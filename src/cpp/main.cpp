#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <filesystem>
#include "iostream"
#include <sys/stat.h>
#include <vector>
#include "lib/ga/individual.h"
#include "lib/ga/population.h"
#include <cstdlib>
#include "lib/utils.h"
#include <cmath>

using namespace std;
using namespace cv;
namespace fs = std::__fs::filesystem;

/** Configuration */

int TOTAL_POPULATION = 100;

const uint64_t PRINT_PIXEL_DENSITY = 900; // dpi
const uint64_t GA_PIXEL_DENSITY = 300;    // dpi

int64_t CANVAS_WIDTH_MM = 500;
int64_t CANVAS_HEIGHT_MM = 200;

int64_t GA_CANVAS_WIDTH = (CANVAS_WIDTH_MM / 25.4) * GA_PIXEL_DENSITY;
int64_t GA_CANVAS_HEIGHT = (CANVAS_HEIGHT_MM / 25.4) * GA_PIXEL_DENSITY;

int64_t CANVAS_WIDTH = (CANVAS_WIDTH_MM / 25.4) * PRINT_PIXEL_DENSITY;
int64_t CANVAS_HEIGHT = (CANVAS_HEIGHT_MM / 25.4) * PRINT_PIXEL_DENSITY;

double MUTATION_RATE = 0.2;

/** =================================================================================*/

vector<vector<Point2i>> image_contours;

Mat element = getStructuringElement(
    MORPH_RECT,
    Size(191, 191),
    Point(95, 95));

Scalar color = Scalar(255, 255, 255);

string IMAGE_DIR = "/Users/haus-dev/Documents/auto-arrange-image/assets/images/test1";

vector<string> image_paths;

Point2f rotate_point(const Point2f &p, float rad)
{
    const float x = std::cos(rad) * p.x - std::sin(rad) * p.y;
    const float y = std::sin(rad) * p.x + std::cos(rad) * p.y;

    const Point2f rot_p(x, y);
    return rot_p;
}

void translate_contour(vector<Point2i> &contour, int64_t dx, int64_t dy)
{
    for (size_t i = 0; i < contour.size(); i++)
    {
        contour[i].x += dx;
        contour[i].y += dy;
    }
}

Point2f get_contour_center(vector<Point2i> &contour){
    Moments moments = cv::moments(contour);
    double cx = moments.m10 / moments.m00;
    double cy = moments.m01 / moments.m00;

    return Point2f(cx, cy);
}

void rotate_contour(vector<Point2i> &contour, float rad)
{
    Point2f center = get_contour_center(contour);

    translate_contour(contour, -center.x, -center.y);

    for (size_t i = 0; i < contour.size(); i++)
    {
        contour[i] = rotate_point(contour[i], rad);
    }

    translate_contour(contour, center.x, center.y);
}

void scale_contour(vector<Point2i> &contour, float scale)
{
    for (size_t i = 0; i < contour.size(); i++)
    {
        contour[i].x *= scale;
        contour[i].y *= scale;
    }
}

bool is_contours_intersect(vector<vector<Point2i>> &contours)
{
    // Size canvas_size = Size(CANVAS_HEIGHT, CANVAS_WIDTH);

    // Mat canvas = Mat::zeros(canvas_size, CV_8UC1);

    // drawContours(canvas, contours, -1, color, 10, FILLED);

    // vector<vector<Point2i>> cnts;
    // vector<Vec4i> hierarchy;

    // findContours(canvas, cnts, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // if(cnts.size() != contours.size()){
    //     return true;
    // }

    // return false;

    for (size_t i = 0; i < contours.size(); i++)
    {
        for (size_t j = i + 1; j < contours.size(); j++)
        {
            vector<Point> approxContour1, approxContour2;
            approxPolyDP(contours[i], approxContour1, 0.06 * arcLength(contours[i], true), true);
            approxPolyDP(contours[j], approxContour2, 0.06 * arcLength(contours[j], true), true);

            vector<Point> intersection;
            intersectConvexConvex(approxContour1, approxContour2, intersection);

            if (!intersection.empty())
            {
                return true;
            }
        }
    }

    return false;
}

bool is_contour_outframe(vector<Point2i> &contour)
{
    int64_t bottom = -9999;
    int64_t right = -9999;

    int64_t top = GA_CANVAS_HEIGHT;
    int64_t left = GA_CANVAS_WIDTH;

    for (size_t i = 0; i < contour.size(); i++)
    {
        if (contour[i].x > right)
            right = contour[i].x;
        if (contour[i].y > bottom)
            bottom = contour[i].y;

        if (contour[i].x < left)
            left = contour[i].x;
        if (contour[i].y < top)
            top = contour[i].y;
    }

    bool is_outframe = (right > GA_CANVAS_WIDTH || bottom > GA_CANVAS_HEIGHT || left < 0 || top < 0);

    return is_outframe;
}

Rect boundingRectContours(vector<vector<Point2i>> &contours)
{
    int64_t bottom = -9999;
    int64_t right = -9999;

    int64_t top = GA_CANVAS_HEIGHT;
    int64_t left = GA_CANVAS_WIDTH;

    for (const auto &contour : contours)
    {
        for (size_t i = 0; i < contour.size(); i++)
        {
            if (contour[i].x > right)
                right = contour[i].x;
            if (contour[i].y > bottom)
                bottom = contour[i].y;

            if (contour[i].x < left)
                left = contour[i].x;
            if (contour[i].y < top)
                top = contour[i].y;
        }
    }

    return Rect(left, top, (right - left), (bottom - top));
}

double contours_area(vector<vector<Point2i>> &contours)
{
    double area = 0;
    for (const auto &contour : contours)
    {
        area += contourArea(contour);
    }
    return area;
}

float determine_fitness(Individual *individual, uint16_t total_image)
{
    float black_area_ratio_score = 0;
    float overlap_score = 1;
    float outframe_score = 1;
    float overall_score = 0;

    vector<vector<Point2i>> contours;

    /** Draw all image contours in canvas */

    Mat canvas = Mat::zeros(Size(GA_CANVAS_WIDTH, GA_CANVAS_HEIGHT), CV_8UC1);

    for (uint16_t i = 0; i < total_image; i++)
    {
        int64_t dx = individual->get_pos_x(i);
        int64_t dy = individual->get_pos_y(i);
        float angle = individual->get_angle(i);

        vector<Point2i> contour;
        contour.insert(contour.end(), image_contours[i].begin(), image_contours[i].end());

        rotate_contour(contour, angle);
        translate_contour(contour, dx, dy);

        contours.push_back(contour);

        if (is_contour_outframe(contour))
        {
            outframe_score = 0;
            break;
        }
    }

    if (is_contours_intersect(contours))
    {
        overlap_score = 0;
    }

    Rect rect = boundingRectContours(contours);

    uint64_t object_area = rect.area();

    black_area_ratio_score = contours_area(contours) / object_area;

    overall_score = black_area_ratio_score + overlap_score + outframe_score;
    overall_score = overall_score * overlap_score * outframe_score;

    return overall_score;
}

void show_result(Individual &individual, uint16_t total_image, uint16_t delayms = 0)
{
    vector<vector<Point2i>> contours;

    /** Draw all image contours in canvas */

    Mat canvas = Mat::zeros(Size(GA_CANVAS_WIDTH, GA_CANVAS_HEIGHT), CV_8UC1);

    for (uint16_t i = 0; i < total_image; i++)
    {
        int64_t dx = individual.get_pos_x(i);
        int64_t dy = individual.get_pos_y(i);
        float angle = individual.get_angle(i);

        vector<Point2i> contour;
        contour.insert(contour.end(), image_contours[i].begin(), image_contours[i].end());

        rotate_contour(contour, angle);
        translate_contour(contour, dx, dy);

        contours.push_back(contour);
    }

    if (is_contours_intersect(contours))
    {
        circle(canvas, Point(0, 0), 1000, color, 10);
    }

    drawContours(canvas, contours, -1, color, 10, FILLED);

    imshow("Canvas", canvas);
    imwrite("Canvas.png", canvas);

    // Wait for any keystroke
    waitKey(delayms);
}

void show_result_image(Individual &individual)
{
    /** Draw all image contours in canvas */
    Mat canvas = Mat::zeros(Size(CANVAS_WIDTH, CANVAS_HEIGHT), CV_8UC4);

    for (uint16_t i = 0; i < image_paths.size(); i++)
    {
        int64_t dx = individual.get_pos_x(i) * (PRINT_PIXEL_DENSITY / GA_PIXEL_DENSITY);
        int64_t dy = individual.get_pos_y(i) * (PRINT_PIXEL_DENSITY / GA_PIXEL_DENSITY);
        float angle = individual.get_angle(i);

        Mat img = imread(image_paths[i], IMREAD_UNCHANGED);

        // width = 30 mm
        double new_img_width = (30 / 25.4) * PRINT_PIXEL_DENSITY;

        double scale = new_img_width / img.cols;

        resize(img, img, Size(img.cols * scale, img.rows * scale));

        double deg = angle / M_PI * 180.0;

        Point2f center = get_contour_center(image_contours[i]) * (double) (PRINT_PIXEL_DENSITY / GA_PIXEL_DENSITY);

        double h = img.rows * 1.0;
        double w = img.cols * 1.0;

        double s = 0;

        if (h > w)
        {
            s = w / sqrt((h * h) + (w * w));
        }
        else
        {
            s = h / sqrt((h * h) + (w * w));
        }

        Mat rotationMatrix = getRotationMatrix2D(center, -deg, s);
        warpAffine(img, img, rotationMatrix, img.size());

        // resize(img, img, Size(w / s, h / s));

        Mat channels[4];

        split(img, channels);

        Mat mask = channels[3];

        //img.copyTo(canvas(Rect(dx, dy, img.cols, img.rows)), mask);

        scale_contour(image_contours[i], (PRINT_PIXEL_DENSITY / GA_PIXEL_DENSITY));

        rotate_contour(image_contours[i], angle);
        translate_contour(image_contours[i], dx, dy);
    }

    Rect rect = boundingRectContours(image_contours);
    double center_x = rect.x + (rect.width / 2);
    double center_y = rect.y + (rect.height / 2);

    Point center = Point(center_x, center_y);
    Point top_left = Point(rect.x, rect.y);
    Point bottom_right = Point(rect.x + rect.width, rect.y + rect.height);

    rectangle(canvas, top_left, bottom_right, Scalar(255, 0, 0), 10, LINE_8);

    circle(canvas, center, 100, Scalar(0, 0, 255), 10);

    for (uint16_t i = 0; i < image_contours.size(); i++)
    {
        Point2f c = get_contour_center(image_contours[i]);
        line(canvas, c, center, Scalar(0, 0, 255), 10);

        int64_t dx_2 = (center.x - c.x) / 2;
        int64_t dy_2 = (center.y - c.y) / 2;

        //translate_contour(image_contours[i], dx_2, dy_2);
    }

    drawContours(canvas, image_contours, -1, Scalar(0, 255, 0), 10, LINE_AA);

    imshow("Image", canvas);
    imwrite("Image.png", canvas);

    // Wait for any keystroke
    waitKey(0);
}

int main()
{
    cout << "Canvas Width: " << GA_CANVAS_WIDTH << endl;
    cout << "Canvas Height: " << GA_CANVAS_HEIGHT << endl;

    TickMeter tm;

    tm.start();

    struct stat sb;

    for (const auto &entry : fs::directory_iterator(IMAGE_DIR))
    {
        // Converting the path to const char * in the
        // subsequent lines
        fs::path outfilename = entry.path();
        string outfilename_str = outfilename.string();

        string extension = outfilename.extension();
        if (extension == ".png")
        {
            image_paths.push_back(outfilename_str);
        }
    }

    uint16_t TOTAL_IMAGES = image_paths.size();

    Mat original_images[TOTAL_IMAGES];
    Mat alpha_images[TOTAL_IMAGES];

    /** Load image */

    for (uint16_t i = 0; i < TOTAL_IMAGES; i++)
    {
        original_images[i] = imread(image_paths[i], IMREAD_UNCHANGED);

        Mat channels[4];

        split(original_images[i], channels);

        alpha_images[i] = channels[3];

        /** Find image contours */

        // Dilate image
        dilate(alpha_images[i], alpha_images[i], element);

        vector<vector<Point2i>> contours;
        vector<Vec4i> hierarchy;
        findContours(alpha_images[i], contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

        double new_img_width = (30 / 25.4) * GA_PIXEL_DENSITY;

        double scale = new_img_width / original_images[i].cols;

        scale_contour(contours[0], scale);
        image_contours.push_back(contours[0]);
    }

INITIAL_POPULATION:
    /** Generate initial population*/

    Individual individuals[TOTAL_POPULATION];

    const int64_t max_x = GA_CANVAS_WIDTH;
    const int64_t max_y = GA_CANVAS_HEIGHT;
    const float max_angle = M_PI;

    for (int i = 0; i < TOTAL_POPULATION; i++)
    {

        int64_t pos_x[TOTAL_IMAGES];
        int64_t pos_y[TOTAL_IMAGES];
        float angle[TOTAL_IMAGES];

        for (int j = 0; j < TOTAL_IMAGES; j++)
        {
            uint64_t margin_left = 200;
            uint64_t margin_right = 200;
            uint64_t margin_top = 200;
            uint64_t margin_bottom = 200;

            pos_x[j] = Utils::random_number(margin_left, (max_x - margin_right)); // max_x - 2000);
            pos_y[j] = Utils::random_number(margin_top, (max_y - margin_bottom)); // max_y - 2000);
            angle[j] = Utils::random_number((float)0, max_angle);
        }

        individuals[i] = Individual(TOTAL_IMAGES, pos_x, pos_y, angle);
    }

    Population population(TOTAL_POPULATION, individuals);
    population.calculate_fitness(TOTAL_IMAGES, &determine_fitness);

    vector<Individual> best_individuals = population.get_best_individuals(50);

    int gen_num = 0;
    while (true)
    {
        vector<Individual> new_individuals;

        // while (new_individuals.size() < TOTAL_POPULATION){
        //  Crossover
        for (int i = 0; i < best_individuals.size(); i += 2)
        {
            // Got 25 new individuals
            Individual i1 = best_individuals[i].crossover(best_individuals[i + 1]);
            // Got 25 new individuals
            Individual m1 = best_individuals[i].mutate(MUTATION_RATE, GA_CANVAS_WIDTH, GA_CANVAS_HEIGHT);
            // Got 25 new individuals
            Individual m2 = best_individuals[i + 1].mutate(MUTATION_RATE, GA_CANVAS_WIDTH, GA_CANVAS_HEIGHT);
            // Got 25 new individuals
            Individual m3 = i1.mutate(MUTATION_RATE, GA_CANVAS_WIDTH, GA_CANVAS_HEIGHT);

            new_individuals.push_back(m1);
            new_individuals.push_back(m2);
            new_individuals.push_back(m3);
            new_individuals.push_back(i1);
        }

        // cout << "Generation\t: " << new_individuals.size() << endl;
        //}

        population = Population(new_individuals);
        population.calculate_fitness(TOTAL_IMAGES, &determine_fitness);

        best_individuals = population.get_best_individuals(50);

        cout << "Generation\t: " << gen_num << endl;
        cout << "Best Fitness\t: " << best_individuals[0].get_fitness() << endl;

        if (best_individuals[0].get_fitness() > 2.1)
        {
            break;
        }

        // if (best_individuals[0].get_fitness() < 2)
        // {
        //     goto INITIAL_POPULATION;
        // }

        // show_result(best_individuals[0], TOTAL_IMAGES, 1);

        gen_num++;
    }

    tm.stop();
    cout << "Execution Time\t: " << tm.getTimeSec() << " second(s)\n";
    cout << "Generation Num\t: " << gen_num << endl;
    cout << "Best Fitness\t: " << best_individuals[0].get_fitness() << endl;

    show_result_image(best_individuals[0]);

    return 0;
}