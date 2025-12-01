#pragma once
#include <vector>
#include <spdlog/spdlog.h>

struct Points {

private: // To make sure that epsilon and sigma are always of the same size
        std::vector<double> epsilon;
        std::vector<double> sigma;

public:
    Points() = default;

    Points(std::size_t n) {
        epsilon.reserve(n);
        sigma.reserve(n);
    }

    Points(std::vector<double> eps, std::vector<double> sig) {
        if (eps.size() != sig.size()) {
            spdlog::error("Points constructor error: epsilon and sigma vectors must be of the same size.");
            return;
        }
        epsilon = std::move(eps);
        sigma = std::move(sig);
    }

    std::size_t size() const {
        return epsilon.size();
    }

    void clear() {
        epsilon.clear();
        sigma.clear();
    }

    void push_back(double eps, double sig) {
        epsilon.push_back(eps);
        sigma.push_back(sig);
    }

    const std::vector<double>& get_epsilon() const {
        return epsilon;
    }

    const std::vector<double>& get_sigma() const {
        return sigma;
    }

    // push back a range of points from given vectors
    void insert_range(const std::vector<double>& eps, const std::vector<double>& sig, std::size_t start, std::size_t end)
    {

        // check for valid range
        if (eps.size() != sig.size())
        {
            // invalid range -> return without inserting
            spdlog::warn("insert_range: invalid range ignored");
            return;
        }
        
        epsilon.insert(epsilon.end(), eps.begin() + start, eps.begin() + end);
        sigma.insert(sigma.end(), sig.begin() + start, sig.begin() + end);
    }


    void change_point(std::size_t index, double eps, double sig) {
        if (index < epsilon.size()) {
            epsilon[index] = eps;
            sigma[index] = sig;
        }
    }

    void print() const {
        spdlog::info("Points:");
        for (std::size_t i = 0; i < epsilon.size(); ++i) {
            spdlog::info("  ({}, {})", epsilon[i], sigma[i]);
        }
    }

    ~Points() = default;
    //delete points not implemented.
};
