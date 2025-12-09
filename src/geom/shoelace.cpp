#include "shoelace.h"
#include <immintrin.h>



static inline double extractsum(__m256d vec) {
    alignas(32) double tmp[4];
    _mm256_store_pd(tmp, vec);
    return tmp[0] + tmp[1] + tmp[2] + tmp[3];
}

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