#include "shoelace.h"
#include <immintrin.h>



static inline double extractsum(__m256d vec) {
    alignas(32) double tmp[4];
    _mm256_store_pd(tmp, vec);
    return tmp[0] + tmp[1] + tmp[2] + tmp[3];
}

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

Shoelace::Shoelace(const Points& pts)
    : points(pts)
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
        const double a  = eps[i - 1];
        const double b  = eps[i];
        const double sa = sig[i - 1];
        const double sb = sig[i];

        area_prefix[i] = area_prefix[i - 1] + seg_area(a, b, sa, sb);
        momentum_prefix[i] = momentum_prefix[i - 1] + seg_moment(a, b, sa, sb);
    }
}



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

// (idx, sigma_cut) + eps_cut
double Shoelace::calculateArea(double eps_cut, std::pair<std::size_t,double> cut) const
{
    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();
    const std::size_t n = points.size();

    if (n < 2) return 0.0;

    const std::size_t idx = cut.first;   // lower_bound index
    const double s_cut    = cut.second;  // already interpolated sigma(eps_cut)

    if (eps_cut < eps.front() || eps_cut > eps.back()) {
        throw std::out_of_range("Shoelace::calculateArea: eps_cut out of range.");
    }

    if (eps_cut == eps.front()) return 0.0;

    // idx must be valid
    if (idx >= n) throw std::out_of_range("Shoelace::calculateArea: invalid idx.");

    // exact knot hit (preprocess {idx, sig[idx]})
    if (eps[idx] == eps_cut) {
        return area_prefix[idx];
    }

    if (idx == 0) {
        // eps_cut > eps.front() 
        throw std::logic_error("Shoelace::calculateArea: idx==0 but eps_cut not at eps[0].");
    }

    const double a  = eps[idx - 1];
    const double sa = sig[idx - 1];

    // seg_area (epsilon a -> epsilon eps_cut)  (sigma sa -> sigma s_cut)
    return area_prefix[idx - 1] + seg_area(a, eps_cut, sa, s_cut);
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

std::pair<double, double> Shoelace:: calculateAreaAndMomentum(const Points& points) {
    double area = 0.0;
    double momentum = 0.0;
    size_t n = points.size();

    if(n < 3) {
        return std::make_pair(0.0, 0.0); // Invalid input
    }

    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();
    
    for (size_t i = 0; i < n - 1; ++i) {
        size_t j = i + 1; // Next vertex index, wrapping around
        double tmp_area = eps[i] * sig[j] - eps[j] * sig[i];
        area += tmp_area;

        momentum += (eps[i] + eps[j]) * tmp_area;
    }

    area = area + eps[n - 1] * sig[0] - eps[0] * sig[n - 1];
    
    area = std::abs(area) * 0.5;
    momentum = std::abs(momentum) / 6.0;

    return std::make_pair(area, momentum);
}

std :: pair<double, double> Shoelace:: calculateAreaAndMomentum_simd(const Points& points) {
    double area = 0.0;
    double momentum = 0.0;
    size_t n = points.size();
    size_t i =0;

    if(n < 3) {
        return std::make_pair(0.0, 0.0); // Invalid input
    }

    const auto& eps = points.get_epsilon();
    const auto& sig = points.get_sigma();

    __m256d area_simd = _mm256_setzero_pd();
    __m256d momentum_simd = _mm256_setzero_pd();
    
    for(i = 0; i + 5 < n; i += 4){
        __m256d eps_i_simd = _mm256_loadu_pd(&eps[i]);
        __m256d sig_i_simd = _mm256_loadu_pd(&sig[i]);

        __m256d eps_j_simd = _mm256_loadu_pd(&eps[i + 1]);
        __m256d sig_j_simd = _mm256_loadu_pd(&sig[i + 1]);

        __m256d term1 = _mm256_mul_pd(eps_i_simd, sig_j_simd);
        __m256d term2 = _mm256_mul_pd(eps_j_simd, sig_i_simd);
        __m256d term3 = _mm256_add_pd(eps_j_simd, eps_i_simd);

        __m256d area_vec = _mm256_sub_pd(term1, term2);

        area_simd = _mm256_add_pd(area_simd, area_vec);

        __m256d momentum_vec = _mm256_mul_pd(term3, area_vec);
        momentum_simd = _mm256_add_pd(momentum_simd, momentum_vec);
    }
    
    area += extractsum(area_simd);
    momentum += extractsum(momentum_simd);

    for (; i < n - 1; ++i) {
        size_t j = i + 1; // Next vertex index, wrapping around
        double tmp_area = eps[i] * sig[j] - eps[j] * sig[i];
        area += tmp_area;

        momentum += (eps[i] + eps[j]) * tmp_area;
    }

    area = area +  eps[n - 1] * sig[0] - eps[0] * sig[n - 1];

    
    area = std::abs(area) * 0.5;
    momentum = std::abs(momentum) / 6.0;

    return std::make_pair(area, momentum);
}

} // namespace geom