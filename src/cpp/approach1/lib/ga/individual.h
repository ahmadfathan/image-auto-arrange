#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <string>
#include <vector>
#include <cmath>
#include "../utils.h"

using namespace std;

class Individual
{
public:
    Individual();
    Individual(
        int num_of_images,
        int64_t *pos_x,
        int64_t *pos_y,
        float *angle);

    int64_t get_pos_x(int i);
    int64_t get_pos_y(int i);
    float get_angle(int i);

    vector<int64_t> get_pos_x();
    vector<int64_t> get_pos_y();
    vector<float> get_angle();

    void set_num_of_images(int num_of_images);
    void set_pos_x(vector<int64_t> pos_x);
    void set_pos_y(vector<int64_t> pos_y);
    void set_angle(vector<float> angle);

    void set_fitness(float fitness);
    float get_fitness(void);

    Individual crossover(Individual &mate);
    Individual mutate(float mutation_rate, uint64_t max_x, uint64_t max_y);

private:
    int num_of_images;
    vector<int64_t> pos_x;
    vector<int64_t> pos_y;
    vector<float> angle; // in radian

    float fitness;
};

#endif