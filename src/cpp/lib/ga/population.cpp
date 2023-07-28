#include "population.h"
#include <iostream>

using namespace std;

bool compare_fitness(Individual i1, Individual i2)
{
    return (i1.get_fitness() > i2.get_fitness());
}

Population::Population(int num_of_individuals, Individual *individuals)
{
    Population::num_of_individuals = num_of_individuals;
    Population::individuals.insert(Population::individuals.end(), individuals, individuals + num_of_individuals);
}

Population::Population(vector<Individual>& individuals){
    Population::num_of_individuals = individuals.size();
    Population::individuals.insert(Population::individuals.end(), individuals.begin(), individuals.end());
}

Individual Population::get_individual(int i){
    return individuals[i];
}

void Population::calculate_fitness(uint16_t total_image, float (*fitness_func)(Individual*, uint16_t)){    
    for(int i = 0; i < num_of_individuals; i++){
        individuals[i].set_fitness(fitness_func(&individuals[i], total_image));
    }
}

vector<Individual> Population::get_best_individuals(uint16_t n){
    
    vector<Individual> results(n);

    sort(individuals.begin(), individuals.end(), compare_fitness);

    copy(individuals.begin(), individuals.begin() + n, results.begin());

    return results;
}



 
