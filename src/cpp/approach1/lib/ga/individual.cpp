#include "individual.h"

Individual::Individual()
{
}

Individual::Individual(int num_of_images, int64_t *pos_x, int64_t *pos_y, float *angle)
{

    Individual::num_of_images = num_of_images;

    Individual::pos_x.insert(Individual::pos_x.end(), pos_x, pos_x + num_of_images);
    Individual::pos_y.insert(Individual::pos_y.end(), pos_y, pos_y + num_of_images);
    Individual::angle.insert(Individual::angle.end(), angle, angle + num_of_images);
}

void Individual::set_num_of_images(int num_of_images)
{
    Individual::num_of_images = num_of_images;
}

void Individual::set_pos_x(vector<int64_t> pos_x)
{
    Individual::pos_x = pos_x;
}

void Individual::set_pos_y(vector<int64_t> pos_y)
{
    Individual::pos_y = pos_y;
}

void Individual::set_angle(vector<float> angle)
{
    Individual::angle = angle;
}

int64_t Individual::get_pos_x(int i)
{
    return pos_x[i];
}

int64_t Individual::get_pos_y(int i)
{
    return pos_y[i];
}

float Individual::get_angle(int i)
{
    return angle[i];
}

vector<int64_t> Individual::get_pos_x()
{
    return pos_x;
}

vector<int64_t> Individual::get_pos_y()
{
    return pos_y;
}

vector<float> Individual::get_angle()
{
    return angle;
}

void Individual::set_fitness(float fitness)
{
    Individual::fitness = fitness;
}

float Individual::get_fitness(void)
{
    return fitness;
}

Individual Individual::crossover(Individual &mate)
{
    Individual new_individual;

    new_individual.set_num_of_images(num_of_images);
    new_individual.set_pos_x(pos_x);
    new_individual.set_pos_y(mate.get_pos_y());
    new_individual.set_angle(angle);

    return new_individual;
}

Individual Individual::mutate(float mutation_rate, uint64_t max_x, uint64_t max_y)
{
    int64_t pos_x[num_of_images];
    int64_t pos_y[num_of_images];
    float angle[num_of_images];

    for (int j = 0; j < num_of_images; j++)
    {
        uint64_t margin_left = 200;
        uint64_t margin_right = 200;
        uint64_t margin_top = 200;
        uint64_t margin_bottom = 200;

        // pos_x[j] = Individual::get_pos_x(j) + Utils::random_number(margin_left, (max_x - margin_right)) * mutation_rate;
        // pos_y[j] = Individual::get_pos_x(j) + Utils::random_number(margin_top, (max_y - margin_bottom)) * mutation_rate;
        // angle[j] = Individual::get_pos_x(j) + Utils::random_number((float)0, M_PI) * mutation_rate;

        pos_x[j] = (max_x / 2) + Utils::random_number(-(max_x / 2), (max_x / 2)); 
        pos_y[j] = (max_y / 2) + Utils::random_number(-(max_y / 2), (max_y / 2));
        angle[j] = Utils::random_number((float)0, M_PI);
        
        if (pos_x[j] > (max_x - margin_left))
            pos_x[j] = pos_x[j] % (max_x - margin_left);
        if (pos_y[j] > (max_y - margin_bottom))
            pos_y[j] = pos_y[j] % (max_y - margin_bottom);
        if (angle[j] > M_PI)
            angle[j] = fmod(angle[j],M_PI); 
    }

    return Individual(num_of_images, pos_x, pos_y, angle);
}
