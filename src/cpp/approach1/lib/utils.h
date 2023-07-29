#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <cstdlib>
#include <random>

using namespace std;

class Utils
{
public:
    static double random_number(double min, double max)
    {
        static mt19937 rng{random_device{}()};
        uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }
};

#endif