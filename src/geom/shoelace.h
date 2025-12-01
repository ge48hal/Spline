#pragma once

#include <vector>
#include <cmath>
#include "points/points.h"

namespace geom {
    class Shoelace {
    public:
        static double calculateArea(const Points& points);

        static double calculateMomentum(const Points& points);
    };

}

