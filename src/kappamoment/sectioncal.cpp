// sectioncal.cpp
#include "sectioncal.h"
#include <algorithm>
#include <cmath>
#include <limits>

static inline double clamp(double x, double lo, double hi) {
    return std::min(std::max(x, lo), hi);
}

static inline bool is_finite(double x) {
    return std::isfinite(x);
}

struct ClipOut {
    double eps_ca;
    double kappa_eff;
    double kappa_max;
};

static inline ClipOut clip_kappa(double eps_ca, double kappa,
                                 double eps_max,
                                 double h_u, double h_d)
{
    ClipOut r{};
    r.eps_ca = clamp(eps_ca, 0.0, eps_max);

    const double kmax_ft = (eps_max - r.eps_ca) / h_d;
    const double kmax_cc = (eps_max + r.eps_ca) / h_u;
    r.kappa_max = std::max(0.0, std::min(kmax_ft, kmax_cc));

    r.kappa_eff = clamp(kappa, 0.0, r.kappa_max);
    return r;
}

// -----------------------------
// force residual only (Area only)
// -----------------------------
double SectionCal::forceresidual(double eps_ca, double kappa) const
{
    const double h_u = cs.h_u_mm();
    const double h_d = cs.h_d_mm();
    const double h   = cs.height_mm;

    const double eps_cc = std::abs(eps_ca - kappa * h_u);
    const double eps_ft = std::abs(eps_ca + kappa * h_d);
    const double eps_dt = eps_cc + eps_ft;

    if (!(eps_dt > 0.0) || !is_finite(eps_dt) ||
        !(eps_cc > 0.0) || !(eps_ft > 0.0) ||
        !is_finite(eps_cc) || !is_finite(eps_ft))
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double h_cc = std::abs((eps_cc / eps_dt) * h);
    const double h_ft = std::abs((eps_ft / eps_dt) * h);

    const double jac_cc = h_cc / eps_cc;
    const double jac_ft = h_ft / eps_ft;

    // preprocess + AREA only
    auto cut_cc = preprocess::_preprocess_polyline(eps_cc, cc);
    auto cut_ft = preprocess::_preprocess_polyline(eps_ft, ft);

    const double A_cc = sh_cc.calculateArea(eps_cc, cut_cc);
    const double A_ft = sh_ft.calculateArea(eps_ft, cut_ft);

    const double f_cc = A_cc * jac_cc;
    const double f_ft = A_ft * jac_ft;

    return f_cc - f_ft;
}

// -----------------------------
// moment only (Moment integral only)
// -----------------------------
double SectionCal::moment(double eps_ca, double kappa) const
{
    const double h_u = cs.h_u_mm();
    const double h_d = cs.h_d_mm();
    const double h   = cs.height_mm;

    const double eps_cc = std::abs(eps_ca - kappa * h_u);
    const double eps_ft = std::abs(eps_ca + kappa * h_d);
    const double eps_dt = eps_cc + eps_ft;

    if (!(eps_dt > 0.0) || !is_finite(eps_dt) ||
        !(eps_cc > 0.0) || !(eps_ft > 0.0) ||
        !is_finite(eps_cc) || !is_finite(eps_ft))
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double h_cc = std::abs((eps_cc / eps_dt) * h);
    const double h_ft = std::abs((eps_ft / eps_dt) * h);

    const double jac_cc = h_cc / eps_cc;
    const double jac_ft = h_ft / eps_ft;

    // preprocess + MOMENT only
    auto cut_cc = preprocess::_preprocess_polyline(eps_cc, cc);
    auto cut_ft = preprocess::_preprocess_polyline(eps_ft, ft);

    const double M_cc = sh_cc.calculateMomentum(eps_cc, cut_cc);
    const double M_ft = sh_ft.calculateMomentum(eps_ft, cut_ft);

    return M_cc * jac_cc * jac_cc +
           M_ft * jac_ft * jac_ft;
}

std::pair<double,double>
SectionCal::forceresidual_moment(double eps_ca, double kappa) const
{
    // Here we intentionally compute each separately.
    // forceresidual(): area only
    // moment(): moment integral only
    return { this->forceresidual(eps_ca, kappa),
             this->moment(eps_ca, kappa) };
}

// ------------------------------------------------------------
// eps_ca solver implementation
// ------------------------------------------------------------
EpsSolveResult SectionCal::solve_eps_ca_for_kappa(
    double kappa_given,
    double eps_max,
    double rel_tol,
    int max_iter
) const {
    EpsSolveResult out;

    const double h_u = cs.h_u_mm();
    const double h_d = cs.h_d_mm();

    auto r = [&](double eps_ca) -> double {
        const auto ck = clip_kappa(eps_ca, kappa_given, eps_max, h_u, h_d);
        return this->forceresidual(ck.eps_ca, ck.kappa_eff);
    };

    double a = 0.0;
    double b = eps_max;

    double ra = r(a);
    double rb = r(b);

    if (!is_finite(ra) || !is_finite(rb)) {
        out.success = false;
        return out;
    }

    if (ra == 0.0) {
        out.eps_ca = a;
        out.iters = 0;
    } else if (rb == 0.0) {
        out.eps_ca = b;
        out.iters = 0;
    } else {
        double lo = a, hi = b, rlo = ra, rhi = rb;

        if ((ra > 0.0) == (rb > 0.0)) {
            constexpr int Nscan = 64;
            double x_prev = a;
            double r_prev = ra;

            bool found = false;
            for (int i = 1; i <= Nscan; ++i) {
                double x = a + (b - a) * (double(i) / double(Nscan));
                double rx = r(x);

                if (!is_finite(rx) || !is_finite(r_prev)) {
                    x_prev = x;
                    r_prev = rx;
                    continue;
                }

                if ((r_prev > 0.0) != (rx > 0.0)) {
                    lo = x_prev; hi = x;
                    rlo = r_prev; rhi = rx;
                    found = true;
                    break;
                }

                x_prev = x;
                r_prev = rx;
            }

            if (!found) {
                out.success = false;
                return out;
            }
        }

        auto tol = [&](double x, double y) {
            const double w = std::abs(y - x);
            const double s = std::max({1.0, std::abs(x), std::abs(y)});
            return w <= rel_tol * s;
        };

        boost::uintmax_t it = static_cast<boost::uintmax_t>(max_iter);
        auto br = boost::math::tools::toms748_solve(r, lo, hi, rlo, rhi, tol, it);

        out.iters = static_cast<int>(it);
        out.eps_ca = 0.5 * (br.first + br.second);
    }

    // finalize: clip at final eps_ca, store kappa_eff, compute residual+moment separately
    const auto ck = clip_kappa(out.eps_ca, kappa_given, eps_max, h_u, h_d);
    out.eps_ca    = ck.eps_ca;
    out.kappa_eff = ck.kappa_eff;

    out.residual = this->forceresidual(out.eps_ca, out.kappa_eff); // area only
    out.moment   = this->moment(out.eps_ca, out.kappa_eff);        // moment only

    out.success = is_finite(out.residual) && is_finite(out.moment);
    return out;
}

std::vector<EpsSolveResult>
SectionCal::solve_eps_ca_for_kappa_batch(
    const std::vector<double>& kappa_vec,
    double eps_max,
    double rel_tol,
    int max_iter
) const {
    const std::size_t N = kappa_vec.size();
    std::vector<EpsSolveResult> outv(N);

    #pragma omp parallel for
    for (std::size_t i = 0; i < N; ++i) {
        outv[i] = this->solve_eps_ca_for_kappa(
            kappa_vec[i], eps_max, rel_tol, max_iter
        );
    }
    return outv;
}
