#include "prep.h"


namespace preprocess {

// Computes sigma(eps_cut) from the polyline (epsilon[], sigma[])
std::pair<std::size_t,double> _preprocess_polyline(double eps_cut, const Points& lm)
{
    std::size_t n = lm.size();
    if (n < 2) {
        spdlog::error("Interpolation failed: polyline contains fewer than 2 points.");
        return {0, 0.0};
    }


    const auto& eps = lm.get_epsilon();
    const auto& sig = lm.get_sigma();


    // Check valid domain
    if (eps_cut < eps.front() || eps_cut > eps.back()) {
        spdlog::error("eps_cut={} is out of range [{}, {}].",
                eps_cut, eps.front(), eps.back());
        return {0, 0.0};
    }

    // Locate position using binary search 
    auto it = std::lower_bound(eps.begin(), eps.end(), eps_cut);
    std::size_t idx = static_cast<std::size_t>(std::distance(eps.begin(), it));

    // Exact match: return sigma without interpolation
    if (it != eps.end() && *it == eps_cut) {
        return {idx, sig[idx]};
    }

    // Bracketing indices must exist
    if (idx == 0 || idx >= n) {
        spdlog::error("Failed to bracket eps_cut={}, idx={}", eps_cut, idx);
        return {0, 0.0};
    }

    std::size_t i = idx - 1;
    std::size_t j = idx;

    double eps0 = eps[i];
    double eps1 = eps[j];
    double sig0 = sig[i];
    double sig1 = sig[j];

    double t = (eps_cut - eps0) / (eps1 - eps0);
    double result = sig0 + t * (sig1 - sig0);

    return {idx, result};
}


// Preprocessing step equivalent to Python prep()
// Constructs a closed polygon for shoelace: trim, add intersection point,
// drop a vertical segment, and add the origin point.
Points prep(double eps_cut, const Points& lm)
{

    const auto& eps = lm.get_epsilon();
    const auto& sig = lm.get_sigma();

    std::pair<std::size_t,double> interp = _preprocess_polyline(eps_cut, lm);

    if (interp.first == 0u && interp.second == 0.0) {
        spdlog::error("Preprocessing failed: could not compute intersection point.");
        return Points();
    }

    
    Points out(interp.first + 3); // +3 for intersection, projection, and origin

    // 1) Copy all points with epsilon < eps_cut

    out.insert_range(eps, sig, 0, interp.first);

    // 2) Add intersection point [eps_cut, sigma_interp]
    out.push_back(eps_cut, interp.second);

    // 3) Add projection to sigma = 0
    out.push_back(eps_cut, 0.0);

    // 4) Add starting point to close the polygon 
    out.push_back(eps[0], sig[0]);

    return out;
}

}