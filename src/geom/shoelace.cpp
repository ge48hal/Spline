#include "shoelace.h"

namespace geom {

double Shoelace:: calculateArea(const Points& points) {
    double area = 0.0;
    size_t n = points.size();

    if(n < 3) {
        return 0.0; // Invalid input
    }

    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();
    
    for (size_t i = 0; i < n - 1; ++i) {
        size_t j = i + 1; // Next vertex index, wrapping around
        area += eps[i] * sig[j];
        area -= eps[j] * sig[i];
    }

    area += eps[n - 1] * sig[0];
    area -= eps[0] * sig[n - 1];
    
    return std::abs(area) * 0.5;
}

double Shoelace:: calculateMomentum(const Points& points) {
    double momentum = 0.0;
    size_t n = points.size();

    if(n < 3) {
        return 0.0; // Invalid input
    }

    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();
    
    // Python m1: sum over i = 0 to n-2 
    for (size_t i = 0; i < n -1; ++i) {

        momentum += (eps[i] + eps[i+1]) * (eps[i] * sig[i+1] - eps[i+1] * sig[i]);
    }
    
    return std::abs(momentum) / 6.0;
}
    
}