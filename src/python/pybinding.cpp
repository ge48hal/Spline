#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "points/points.h"
#include "inputreader/prep.h"
#include "geom/shoelace.h"
#include "kappamoment/crosssection.h"
#include "kappamoment/sectioncal.h"

namespace py = pybind11;

PYBIND11_MODULE(splinepy, m) {
    m.doc() = "Spline / Shoelace bindings";

    // ------------------------------------------------------------
    // Points
    // ------------------------------------------------------------
    py::class_<Points>(m, "Points")
        .def(py::init<>())
        .def(py::init<std::size_t>(), py::arg("n"))
        .def(py::init<std::vector<double>, std::vector<double>>(),
             py::arg("epsilon_vec"), py::arg("sigma_vec"))
        .def("size", &Points::size)
        .def("add_point", &Points::push_back, py::arg("epsilon"), py::arg("sigma"))
        .def("get_epsilon", &Points::get_epsilon)
        .def("get_sigma", &Points::get_sigma);

    // ------------------------------------------------------------
    // preprocess
    // ------------------------------------------------------------


    m.def("preprocess_cut_pair",
          &preprocess::_preprocess_polyline,
          "Lightweight preprocessing: returns (index, sigma_at_eps_cut)",
          py::arg("eps_cut"), py::arg("lm"), py::arg("hint") = 1);

    // ------------------------------------------------------------
    // Shoelace
    // ------------------------------------------------------------
    using CutPair = std::pair<std::size_t, double>;

    auto shoelace = py::class_<geom::Shoelace>(m, "Shoelace");

    // member function pointer types (const)
    using AreaMem = double (geom::Shoelace::*)(double, CutPair) const;
    using MomMem  = double (geom::Shoelace::*)(double, CutPair) const;

    // static function pointer types
    using AreaStatic = double (*)(const Points&);
    using MomStatic  = double (*)(const Points&);

    shoelace
        .def(py::init<const Points&>(),
             py::arg("pts"),
             // Shoelace stores a const reference to Points
             py::keep_alive<1, 2>())

        // member overload: (eps_cut, (idx, sigma_cut))
        .def("calculate_area",
             static_cast<AreaMem>(&geom::Shoelace::calculateArea),
             py::arg("eps_cut"), py::arg("eps_cut_pair"))

        .def("calculate_momentum",
             static_cast<MomMem>(&geom::Shoelace::calculateMomentum),
             py::arg("eps_cut"), py::arg("eps_cut_pair"));

    // ------------------------------------------------------------
    // CrossSection
    // ------------------------------------------------------------
    py::class_<CrossSection>(m, "CrossSection")
        .def(py::init<double, double, double, double>(),
             py::arg("h"),
             py::arg("l") = 160.0,
             py::arg("b") = 1.0,
             py::arg("E") = 60000.0)
        .def_readwrite("length_mm", &CrossSection::length_mm)
        .def_readwrite("height_mm", &CrossSection::height_mm)
        .def_readwrite("b_mm", &CrossSection::b_mm)
        .def_readwrite("E_mpa", &CrossSection::E_mpa)
        .def("h_u_mm", &CrossSection::h_u_mm)
        .def("h_d_mm", &CrossSection::h_d_mm)
        .def("I_mm4", &CrossSection::I_mm4)
        .def("W_mm3", &CrossSection::W_mm3);

    // ------------------------------------------------------------
    // EpsSolveResult (NEW)
    // ------------------------------------------------------------
    py::class_<EpsSolveResult>(m, "EpsSolveResult")
        .def(py::init<>())
        .def_readonly("eps_ca",    &EpsSolveResult::eps_ca)
        .def_readonly("kappa_eff", &EpsSolveResult::kappa_eff)
        .def_readonly("residual",  &EpsSolveResult::residual)
        .def_readonly("moment",    &EpsSolveResult::moment)
        .def_readonly("iters",     &EpsSolveResult::iters)
        .def_readonly("success",   &EpsSolveResult::success);

    // ------------------------------------------------------------
    // SectionCal
    // ------------------------------------------------------------
    py::class_<SectionCal>(m, "SectionCal")
        .def(py::init<const CrossSection&, const Points&, const Points&>(),
             py::arg("cs"), py::arg("cc"), py::arg("ft"),
             // SectionCal keeps references to its inputs
             py::keep_alive<1, 2>(),
             py::keep_alive<1, 3>(),
             py::keep_alive<1, 4>())
        .def("forceresidual",
             &SectionCal::forceresidual,
             py::arg("eps_ca"), py::arg("kappa"))
        .def("moment",
             &SectionCal::moment,
             py::arg("eps_ca"), py::arg("kappa"))
        .def("forceresidual_moment",
             &SectionCal::forceresidual_moment,
             py::arg("eps_ca"), py::arg("kappa"))
        .def("solve_eps_ca_for_kappa",
             &SectionCal::solve_eps_ca_for_kappa,
             py::arg("kappa_given"),
             py::arg("eps_max"),
             py::arg("rel_tol") = 1e-10,
             py::arg("max_iter") = 80)
         .def("solve_eps_ca_for_kappa_batch",
              &SectionCal::solve_eps_ca_for_kappa_batch,
              py::arg("kappa_vec"),
              py::arg("eps_max"),
              py::arg("rel_tol") = 1e-10,
              py::arg("max_iter") = 80);



}
