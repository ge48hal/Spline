#include "shoelace.h"
//#include <immintrin.h>



// ∫ σ dε over [a,b] with linear σ between (a,sa) and (b,sb)
static inline double seg_area(double a, double b, double sa, double sb) {
    return 0.5 * (sa + sb) * (b - a);
}


// ∫ ε σ(ε) dε over [a,b], σ linear between (a,sa) and (b,sb)
static inline double seg_moment(double a, double b, double sa, double sb) {
    const double h = b - a;
    const double m = (sb - sa) / h;

    // sa * ∫ ε dε + m * ∫ ε(ε-a) dε
    const double I1 = 0.5 * (b*b - a*a);
    const double I2 = (1.0/3.0)*(b*b*b - a*a*a) - 0.5*a*(b*b - a*a);
    return sa * I1 + m * I2;
}

namespace geom {

// n and s must be same size

Shoelace::Shoelace(const Points& pts, Square const& cs)
    : points(pts), cs(cs)
{

    const std::size_t n = points.size();
    area_prefix.assign(n, 0.0);
    momentum_prefix.assign(n, 0.0);

    if (n < 2) return;

    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();

    // epsilon strictly increasing (lower_bound)
    for (std::size_t i = 1; i < n; ++i) {
        if (!(eps[i] > eps[i - 1])) {
            throw std::invalid_argument("Shoelace: epsilon must be strictly increasing.");
        }
    }

    // prefix at knots, starting from origin(=eps[0]) baseline
    // A[i] = ∫_{eps[0]}^{eps[i]} σ dε  
    // M[i] = ∫_{eps[0]}^{eps[i]} ε σ dε
    for (std::size_t i = 1; i < n; ++i) {
        area_prefix[i] = area_prefix[i - 1] + seg_area(eps[i - 1], eps[i], sig[i-1 ], sig[i]);
        momentum_prefix[i] = momentum_prefix[i - 1] + seg_moment(eps[i - 1], eps[i], sig[i-1 ], sig[i]);
    }
}


// Warning.
// !!!! Trapezoid pts input must not be epsilon-sigma but z-sigma !!!!
// !!! Trapezoid pts input must be linspaced before !!!

Shoelace::Shoelace(const Points& pts, Trapezoid const& cs)
    : points(pts), cs(cs)
{
    const std::size_t n = points.size();
    const double dz = cs.height_mm / (n - 1); // later to z_mm[i] - z_mm[i-1] ? 

    area_prefix.assign(n, 0.0);
    momentum_prefix.assign(n, 0.0);

    area_helper.assign(n, 0.0); // helper vector for trapezoid

    if (n < 2) return;

    const auto& z_mm = points.get_epsilon();
    const auto& sig = points.get_sigma();

    // epsilon strictly increasing (lower_bound)
    for (std::size_t i = 1; i < n; ++i) {
        if (!(z_mm[i] > z_mm[i - 1])) {
            throw std::invalid_argument("Shoelace: z_mm must be strictly increasing.");
        }
    }

    for(std::size_t i = 0; i< n ; ++i){
        area_helper[i] = cs.bhi_mm + cs.slope() * z_mm[i];
    }
    // prefix at knots, starting from origin(=eps[0]) baseline
    // A[i] = ∫_{eps[0]}^{eps[i]} σ dε  
    // M[i] = ∫_{eps[0]}^{eps[i]} ε σ dε
    for (std::size_t i = 1; i < n; ++i) {
        const double sigma_mid = 0.5 * (sig[i - 1] + sig[i]);
        const double b_mid     = 0.5 * (area_helper[i - 1] + area_helper[i]);
        area_prefix[i] = area_prefix[i - 1] + sigma_mid * b_mid * dz;
        //momentum_prefix[i] = momentum_prefix[i - 1] + seg_moment(eps[i - 1], eps[i], sig[i-1 ], sig[i]);
    }
}

double Shoelace::calculateArea(double eps_cut, std::pair<std::size_t,double> cut) const
{
    const auto& x   = points.get_epsilon(); // Square: eps, Trapezoid: z_mm
    const auto& sig = points.get_sigma();
    const std::size_t n = points.size();

    if (n < 2) return 0.0;

    const std::size_t idx = cut.first;   // lower_bound index
    const double s_cut    = cut.second;  // interpolated sigma(x_cut)

    if (eps_cut < x.front() || eps_cut > x.back()) {
        throw std::out_of_range("Shoelace::calculateArea: eps_cut out of range.");
    }
    if (eps_cut == x.front()) return 0.0;
    if (idx >= n) throw std::out_of_range("Shoelace::calculateArea: invalid idx.");

    // exact knot hit
    if (x[idx] == eps_cut) {
        return area_prefix[idx];
    }
    if (idx == 0) {
        throw std::logic_error("Shoelace::calculateArea: idx==0 but eps_cut not at x[0].");
    }

    const double a  = x[idx - 1];
    const double sa = sig[idx - 1];

    if (cs.cs_type == CrossSection::CrossSectionType::SQUARE) {
        // ∫ σ dε
        return area_prefix[idx - 1] + seg_area(a, eps_cut, sa, s_cut);
    }
    // TRAPEZOID: last partial segment [a, eps_cut]
    // Use the SAME rule as prefix: avg(sigma) * avg(b) * dz_partial

    const double ba = area_helper[idx - 1];

    const double denom = x[idx] - x[idx - 1];
    if (!(denom > 0.0)) {
        throw std::logic_error("Shoelace::calculateArea: non-increasing x segment.");
    }

    const double t  = (eps_cut - x[idx - 1]) / denom;
    const double bb = area_helper[idx - 1] + t * (area_helper[idx] - area_helper[idx - 1]);

    const double dz_partial = eps_cut - a;

    const double sigma_mid = 0.5 * (sa + s_cut);
    const double b_mid     = 0.5 * (ba + bb);

    return area_prefix[idx - 1] + sigma_mid * b_mid * dz_partial;

}


double Shoelace::calculateMomentum(double eps_cut, std::pair<std::size_t, double> cut) const
{
    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();
    const std::size_t n = points.size();

    if (n < 2) return 0.0;

    const std::size_t idx = cut.first;   // lower_bound index
    const double s_cut    = cut.second;  // already interpolated sigma(eps_cut)

    if (eps_cut < eps.front() || eps_cut > eps.back()) {
        throw std::out_of_range("Shoelace::calculateMomentum: eps_cut out of range.");
    }

    if (eps_cut == eps.front()) return 0.0;

    if (idx >= n) {
        throw std::out_of_range("Shoelace::calculateMomentum: invalid idx.");
    }

    // exact knot hit (preprocess가 {idx, sig[idx]})
    if (eps[idx] == eps_cut) {
        return momentum_prefix[idx];
    }

    if (idx == 0) {
        throw std::logic_error("Shoelace::calculateMomentum: idx==0 but eps_cut not at eps[0].");
    }

    const double a  = eps[idx - 1];
    const double sa = sig[idx - 1];

    return momentum_prefix[idx - 1] + seg_moment(a, eps_cut, sa, s_cut);
}

} // namespace geom