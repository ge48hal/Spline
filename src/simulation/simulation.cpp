#include "simulation/simulation.h"


std::vector<double> computeAreaTimeslices(std::span<const Points> slices)
{
    const std::size_t size = slices.size();
    std::vector<double> result(size);

    for(size_t i = 0; i< size; ++i){
        result[i] = geom::Shoelace::calculateArea(slices[i]);
    }

    return result;
}

std::vector<double> computeMomentumTimeslices(std::span<const Points> slices)
{
    const std::size_t size = slices.size();
    std::vector<double> result(size);

    for(size_t i = 0; i< size; ++i){
        result[i] = geom::Shoelace::calculateMomentum(slices[i]);
    }

    return result;
}

