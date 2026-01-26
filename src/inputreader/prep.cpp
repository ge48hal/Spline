#include "prep.h"


namespace preprocess {

// Computes sigma(eps_cut) from the polyline (epsilon[], sigma[])
std::pair<std::size_t,double> _preprocess_polyline(double eps_cut, const Points& lm, size_t hint)
{
    std::size_t n = lm.size();
    if (n < 2) {
        //spdlog::error("Interpolation failed: polyline contains fewer than 2 points.");
        return {0, 0.0};
    }


    const auto& eps = lm.get_epsilon();
    const auto& sig = lm.get_sigma();


    // Check valid domain
    if (eps_cut < eps.front() || eps_cut > eps.back()) {
        //spdlog::error("eps_cut={} is out of range [{}, {}].",
        //        eps_cut, eps.front(), eps.back());
        return {0, 0.0};
    }

    const std::size_t i0   = (hint < 1 ? 1 : hint);
    const std::size_t i_max = std::min(i0 + 4, n - 1);

    for (std::size_t i = i0; i <= i_max; ++i) {
        if (eps[i] == eps_cut) return {i, sig[i]};   // exact

        if (eps[i] > eps_cut) {                      // bracket: (i-1, i)
            const std::size_t j = i - 1;
            const double t = (eps_cut - eps[j]) / (eps[i] - eps[j]);
            return {i, sig[j] + t * (sig[i] - sig[j])};
        }
    }

    // Locate position using binary search 
    auto it = std::lower_bound(eps.begin(), eps.end(), eps_cut);
    std::size_t idx = static_cast<std::size_t>(std::distance(eps.begin(), it));

    // Exact match: return sigma without interpolation
    if (it != eps.end() && *it == eps_cut) {
        return {idx, sig[idx]};
    }

    // // Bracketing indices must exist
    // if (idx == 0 || idx >= n) {
    //     //spdlog::error("Failed to bracket eps_cut={}, idx={}", eps_cut, idx);
    //     return {0, 0.0};
    // }

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

}