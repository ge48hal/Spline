#include "simulation/simulation.h"
#include <omp.h>


std::vector<double> computeAreaTimeslices(std::span<const Points> slices)
{
    const std::size_t size = slices.size();
    std::vector<double> result(size);

    #pragma omp parallel for 
    // Use dynamic scheduling to balance workload across threads
    for(size_t i = 0; i< size; ++i){
        result[i] = geom::Shoelace::calculateArea(slices[i]);
    }

    return result;
}

std::vector<double> computeMomentumTimeslices(std::span<const Points> slices)
{
    const std::size_t size = slices.size();
    std::vector<double> result(size);

    #pragma omp parallel for 
    // Use dynamic scheduling to balance workload across threads
    for(size_t i = 0; i< size; ++i){
        result[i] = geom::Shoelace::calculateMomentum(slices[i]);
    }

    return result;
}

std::vector<std::pair<double,double>> computeAreaAndMomentumTimeslices(std::span<const Points> slices)
{
    const std::size_t size = slices.size();
    std::vector<std::pair<double,double>> result(size);

    #pragma omp parallel for 
    // Use dynamic scheduling to balance workload across threads
    for(size_t i = 0; i< size; ++i){
        result[i] = geom::Shoelace::calculateAreaAndMomentum(slices[i]);
    }

    return result;
}