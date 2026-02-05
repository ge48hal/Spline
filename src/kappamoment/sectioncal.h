#pragma once
#include <vector>
#include <boost/math/tools/toms748_solve.hpp>
#include <boost/cstdint.hpp>
#include <limits>

#include "crosssection.h"
#include "points/points.h"
#include "inputreader/prep.h"
#include "geom/shoelace.h"

// one kappa, max_eps_ca given 
struct EpsSolveResult {
    double eps_ca = 0.0;
    double kappa_eff = 0.0;
    double residual = 0.0;
    double moment = 0.0;
    int iters = 0;
    bool success = false;
};

class SectionCal{
    public : 

        SectionCal(const Square&cross_section,const Points& cc,const Points& ft)
            : cs(cross_section), cc(cc), ft(ft), sh_cc(cc,cross_section), sh_ft(ft,cross_section) {}

        SectionCal(const Trapezoid&cross_section,const Points& cc,const Points& ft)
            : cs(cross_section), cc(cc), ft(ft), sh_cc(cc,cross_section), sh_ft(ft,cross_section) {};
        double forceresidual(double eps_ca, double kappa) const;

        double moment(double eps_ca, double kappa) const;

        std::pair<double,double> forceresidual_moment(double eps_ca, double kappa) const;

        
        EpsSolveResult solve_eps_ca_for_kappa(
        double kappa_given,
        double eps_max,
        double rel_tol = 1e-10,
        int max_iter = 80) const;

        std::vector<EpsSolveResult>
        solve_eps_ca_for_kappa_batch(
            const std::vector<double>& kappa_vec,
            double eps_max,
            double rel_tol = 1e-10,
            int max_iter = 80
        ) const;


    private:
        const CrossSection& cs;
        const Points& cc;
        const Points& ft;

        // Shoelace objects keep const reference to cc/ft and store prefix sums
        geom::Shoelace sh_cc;
        geom::Shoelace sh_ft;
};