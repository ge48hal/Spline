#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <chrono>

#include <spdlog/spdlog.h>

// Project headers
#include "kappamoment/crosssection.h"
#include "points/points.h"
#include "kappamoment/sectioncal.h"

// ------------------------------------------------------------
// Minimal CSV reader
// Assumptions:
//  - first line is header
//  - comma-separated
//  - no quoted fields
// ------------------------------------------------------------
static std::vector<double>
read_csv_column_double(const std::string& path, const std::string& col)
{
    std::ifstream fin(path);
    if (!fin) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::string header;
    if (!std::getline(fin, header)) {
        throw std::runtime_error("Empty CSV file: " + path);
    }

    std::vector<std::string> cols;
    {
        std::stringstream ss(header);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            cols.push_back(tok);
        }
    }

    int idx = -1;
    for (int i = 0; i < static_cast<int>(cols.size()); ++i) {
        if (cols[i] == col) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        throw std::runtime_error("Column not found: " + col);
    }

    std::vector<double> out;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string tok;
        int j = 0;
        while (std::getline(ss, tok, ',')) {
            if (j == idx) {
                out.push_back(std::stod(tok));
                break;
            }
            ++j;
        }
    }

    return out;
}

int main(int argc, char** argv)
{
    try {
        // ------------------------------------------------------------
        // 1) Input definition (matches the Python reference)
        // ------------------------------------------------------------
        // CrossSection(h=300 mm, l=160 mm, b=1 mm, E=60000 MPa)
        CrossSection cs(/*h=*/300.0, /*l=*/160.0, /*b=*/1.0, /*E=*/60000.0);

        // Concrete compression curve:
        // cc = [[0/1000,0],[3/1000,180],[10/1000,180]]
        std::vector<double> cc_eps = {
            0.0 / 1000.0,
            3.0 / 1000.0,
            10.0 / 1000.0
        };
        std::vector<double> cc_sig = {
            0.0,
            180.0,
            180.0
        };

        // Fiber tension curve:
        // ft = [[0/1000,0],[2/1000,50],[4/1000,50],[8/1000,75]]
        std::vector<double> ft_eps = {
            0.0 / 1000.0,
            2.0 / 1000.0,
            4.0 / 1000.0,
            8.0 / 1000.0
        };
        std::vector<double> ft_sig = {
            0.0,
            50.0,
            50.0,
            75.0
        };

        // Construct Points objects
        // NOTE: adjust this if your Points API differs
        Points cc(cc_eps, cc_sig);
        Points ft(ft_eps, ft_sig);

        SectionCal cal(cs, cc, ft);

        // Maximum eps_ca (same as Python: eps_ca_max * 0.9999)
        const double eps_max = 0.008 * 0.9999;

        // ------------------------------------------------------------
        // 2) Load kappa vector
        // ------------------------------------------------------------
        std::vector<double> K;
        if (argc >= 2) {
            const std::string csv_path = argv[1];
            K = read_csv_column_double(csv_path, "k");
            spdlog::info("Loaded {} kappas from {}", K.size(), csv_path);
        } else {
            // Fallback dummy values
            K = {0.0, 1e-6, 2e-6, 5e-6, 1e-5};
            spdlog::warn("No CSV provided. Using dummy K ({} entries).", K.size());
        }

        // ------------------------------------------------------------
        // 3) Repeated execution to ensure long runtime (~10 seconds)
        // ------------------------------------------------------------
        using clock = std::chrono::steady_clock;
        const double target_seconds = 10.0;

        std::size_t total_iterations = 0;
        auto t_start = clock::now();

        std::vector<EpsSolveResult> sols;

        while (true) {
            // Run one full batch solve
            sols = cal.solve_eps_ca_for_kappa_batch(
                K,
                eps_max,
                /*rel_tol=*/1e-10,
                /*max_iter=*/80
            );

            ++total_iterations;

            auto t_now = clock::now();
            double elapsed =
                std::chrono::duration_cast<std::chrono::duration<double>>(t_now - t_start).count();

            if (elapsed >= target_seconds) {
                spdlog::info(
                    "Stopping after {:.3f} seconds ({} batch iterations)",
                    elapsed,
                    total_iterations
                );
                break;
            }
        }

        // ------------------------------------------------------------
        // 4) Print summary of the *last* iteration
        // ------------------------------------------------------------
        int ok = 0, fail = 0;
        for (const auto& s : sols) {
            if (s.success) ++ok;
            else ++fail;
        }

        spdlog::info("Last iteration: success={}, fail={}", ok, fail);

        std::cout << std::fixed << std::setprecision(12);
        std::cout << "idx,kappa,success,iters,eps_ca,kappa_eff,residual,moment\n";

        const std::size_t nprint = std::min<std::size_t>(sols.size(), 30);
        for (std::size_t i = 0; i < nprint; ++i) {
            const auto& s = sols[i];
            std::cout
                << i << ","
                << K[i] << ","
                << (s.success ? 1 : 0) << ","
                << s.iters << ","
                << s.eps_ca << ","
                << s.kappa_eff << ","
                << s.residual << ","
                << s.moment
                << "\n";
        }

        return 0;
    }
    catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}
