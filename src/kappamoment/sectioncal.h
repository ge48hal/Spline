#pragma once
#include <vector>
#include "crosssection.h"
#include "points/points.h"
#include "inputreader/prep.h"
#include "geom/shoelace.h"

// one kappa, max_eps_ca given 

struct SectionState {
    double eps_cc = 0.0;
    double eps_ft = 0.0;
    double h_cc   = 0.0;
    double h_ft   = 0.0;
    double jac_cc = 0.0;
    double jac_ft = 0.0;
    double f_cc   = 0.0;
    double f_ft   = 0.0;
    double m_ca   = 0.0;
};

class SectionCal{
    public : 
        SectionCal(const CrossSection&cs, Points cc, Points ft);

        double forceresidual(double eps_ca, double kappa) const;

        double moment(double eps_ca, double kappa) const;

        SectionState eval(double eps_ca, double kappa) const;
    private:
        const CrossSection& cs;
        Points cc;
        Points ft;

};