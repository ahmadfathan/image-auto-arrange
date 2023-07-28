#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>

int main() {
    cv::Mat image = cv::imread("/Users/haus-dev/Documents/auto-arrange-image/assets/images/L9_camera.png");

    cv::Point2f center(image.cols / 2.0, image.rows / 2.0);

    double h = image.rows * 1.0;
    double w = image.cols * 1.0;
    
    double s = h / sqrt((h*h) + (w*w));

    double angle = 45;
    double scale = 1;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, s);

    cv::Mat rotatedImage;
    cv::warpAffine(image, rotatedImage, rotationMatrix, cv::Size(image.cols, image.rows));//cv::Size(newImageHeight, newImageWidth));

    cv::imshow("Rotated Image", rotatedImage);
    cv::waitKey(0);

    return 0;
}