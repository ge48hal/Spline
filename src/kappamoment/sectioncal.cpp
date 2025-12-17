#include "sectioncal.h"
#include <algorithm>
#include <cmath>

static inline double clamp(double x, double lo, double hi){
    return std::min(std::max(x, lo), hi);
}

SectionState SectionCal::eval(double eps_ca, double kappa) const {
    
    SectionState s;

    const double h_u = cs.h_u_mm();
    const double h_d = cs.h_d_mm();
    const double h   = cs.height_mm;

    s.eps_cc = std::abs(eps_ca - kappa * h_u);
    s.eps_ft = std::abs(eps_ca + kappa * h_d);

    const double eps_dt = s.eps_cc + s.eps_ft;

    s.h_cc = std::abs((s.eps_cc/eps_dt) * h);
    s.h_ft = std::abs((s.eps_ft/eps_dt) * h);

    s.jac_cc = s.h_cc/ s.eps_cc;
    s.jac_ft = s.h_ft/s.eps_ft;

    std::pair<double,double> m_cc = geom::Shoelace::calculateAreaAndMomentum(preprocess::prep(s.eps_cc, cc));
    std::pair<double,double> m_ft = geom::Shoelace::calculateAreaAndMomentum(preprocess::prep(s.eps_ft, ft));

    s.f_cc = m_cc.first * s.jac_cc;
    s.f_ft = m_ft.first * s.jac_ft;

    s.m_ca = m_cc.second * s.jac_cc * s.jac_cc +
             m_ft.second * s.jac_ft * s.jac_ft;

    return s;
}

double SectionCal::forceresidual(double eps_ca, double kappa) const {
    auto s = eval(eps_ca, kappa);
    return s.f_cc - s.f_ft;
}

double SectionCal::moment(double eps_ca, double kappa) const {
    auto s = eval(eps_ca, kappa);
    return s.m_ca;
}