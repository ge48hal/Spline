#pragma once

#include <vector>
#include <cmath>
#include <spdlog/spdlog.h>
#include <omp.h>
#include "points/points.h"
#include "kappamoment/crosssection.h"

namespace geom {

    class Shoelace {
    
    private:
        const Points& points;
        const CrossSection& cs;

        std::vector<double> area_helper; // used to store intermediate values for trapezoide
        std::vector<double> area_prefix;
        std::vector<double> momentum_prefix;
        
    public:
        Shoelace(const Points& pts, const Square& cs);
        Shoelace(const Points& pts, const Trapezoid& cs);

        double calculateArea(double eps_cut, std::pair<size_t, double> eps_cut_pair) const;

        double calculateMomentum(double eps_cut, std::pair<size_t, double> eps_cut_pairs) const;

    };

}

