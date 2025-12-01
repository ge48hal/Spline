#include <iostream>
#include <vector>
#include <chrono> // For performance measurement
#include <spdlog/spdlog.h>

#include "points/points.h"
#include "inputreader/prep.h"      // Points prep(double eps_cut, const Points& lm)
#include "geom/shoelace.h"         // geom::Shoelace
#include "simulation/simulation.h" // sim::computeAreaTimeslices, computeMomentumTimeslices

int main() {

    using clock = std::chrono::high_resolution_clock;
    auto start_time = clock::now();

    /////////////////////////////////////////////////////////////////////////
    // 1) Python _cc = [[0/1000,0],[3/1000,180],[10/1000,180]]

    const std::vector<double> epsilon = {0.0 / 1000.0, 3.0 / 1000.0, 10.0 / 1000.0};
    const std::vector<double> sigma   = {0.0,          180.0,        180.0};
    
    Points cc(epsilon, sigma);


    // 2) eps_cc: 0 ~ 10/1000, num_timesteps
    const std::size_t num_timesteps = 200;
    std::vector<double> eps_cc(num_timesteps);

    const double eps_min = 0.0;
    const double eps_max = 10.0 / 1000.0;
    const double step = (eps_max - eps_min) / static_cast<double>(num_timesteps - 1);

    for (std::size_t t = 0; t < num_timesteps; ++t) {
        eps_cc[t] = eps_min + step * static_cast<double>(t);
    }

    std::vector<Points> timeslices;
    timeslices.reserve(num_timesteps);

    for (std::size_t t = 0; t < num_timesteps; ++t) {
        timeslices.push_back(preprocess::prep(eps_cc[t], cc)); 
    }

    /*TODO : INPUT FILE FORMAT */
    /////////////////////////////////////////////////////////////////////////


    std::vector<double> m0cc = computeAreaTimeslices(timeslices);      // Python m0(_cc_pad)
    std::vector<double> m1cc = computeMomentumTimeslices(timeslices);  // Python m1(_cc_pad)

    for (std::size_t t = 0; t < 30; ++t) {
        std::cout << "eps_cc[" << t << "] = " << eps_cc[t]
                << ", m0cc = " << m0cc[t]
                << ", m1cc = " << m1cc[t] << '\n';
    }

    auto end_time = clock::now();
    spdlog::info("Execution time: {} ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());

    return 0;
}
