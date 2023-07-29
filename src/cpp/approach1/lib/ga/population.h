#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"
#include <vector>

using namespace std;

class Population {
    public:
        Population(int num_of_individuals, Individual *individuals);
        Population(vector<Individual>& individuals);

        Individual get_individual(int i);
        void calculate_fitness(uint16_t total_image, float (*fitness_func)(Individual*, uint16_t));
        vector<Individual> get_best_individuals(uint16_t n);

    private:
        int num_of_individuals;
        vector<Individual> individuals;
};

#endif