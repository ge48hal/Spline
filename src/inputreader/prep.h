#pragma once

#include <utility>
#include <stdexcept>
#include <algorithm>
#include <spdlog/spdlog.h>
#include "points/points.h"

// Arguments:
//   eps_cut : epsilon value at which the original polyline is trimmed
//   lm      : original polyline (x: epsilon, y: sigma), assumed to have x sorted in ascending order
// Returns:
//   A new polygon consisting of:
//     - all points with epsilon < eps_cut,
//     - the interpolated point [eps_cut, sig_interp],
//     - the vertical projection [eps_cut, 0],
//     - and the initial point [x0, y0] to close the polygon.

namespace preprocess{
    std::pair<std::size_t,double> _preprocess_polyline(double eps_cut, const Points& lm);
    
    Points prep(double eps_cut, const Points& lm);
}
