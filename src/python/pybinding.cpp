#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "points/points.h"
#include "inputreader/prep.h"
#include "geom/shoelace.h"
#include "kappamoment/crosssection.h"
#include "kappamoment/sectioncal.h"

namespace py = pybind11;

PYBIND11_MODULE(splinepy, m){
    m.doc() = "Spline / Shoelace bindings";

    py::class_<Points>(m, "Points")
        .def(py::init<>())
        .def(py::init<std::size_t>(), py::arg("n"))
        .def(py::init<std::vector<double>, std::vector<double>>(), py::arg("epsilon_vec"), py::arg("sigma_vec"))
        .def("size", &Points::size)
        .def("add_point", &Points::push_back, py::arg("epsilon"), py::arg("sigma"))
        .def("get_epsilon", &Points::get_epsilon)
        .def("get_sigma", &Points::get_sigma);

    m.def("preprocess", &preprocess::prep, "Preprocess polyline to polygon for Shoelace calculation"
    , py::arg("eps_cut"), py::arg("lm"));

    m.def("cal_area", &geom::Shoelace::calculateArea, "Calculate area using Shoelace formula"
    , py::arg("points"));
    m.def("cal_momentum", &geom::Shoelace::calculateMomentum, "Calculate momentum using Shoelace formula"
    , py::arg("points"));
    m.def("cal_area_momentum", &geom::Shoelace::calculateAreaAndMomentum , "Calculate area and momentum using Shoelace formula"
    , py::arg("points"));
    m.def("cal_area_momentum_simd", &geom::Shoelace::calculateAreaAndMomentum_simd, "Calculate area and momentum using Shoelace formula with SIMD"
    , py::arg("points"));

    py::class_ <CrossSection>(m, "CrossSection")
        .def(py::init<double, double, double, double>(),
        py::arg("h"), py::arg("l") = 160.0, py::arg("b") = 1.0, py::arg("E") = 60000.0)
        .def_readwrite("length_mm", &CrossSection::length_mm)
        .def_readwrite("height_mm", &CrossSection::height_mm)
        .def_readwrite("b_mm", &CrossSection::b_mm)
        .def_readwrite("E_mpa", &CrossSection::E_mpa)
        .def("h_u_mm", &CrossSection::h_u_mm)
        .def("h_d_mm", &CrossSection::h_d_mm)
        .def("I_mm4", &CrossSection::I_mm4)
        .def("W_mm3", &CrossSection::W_mm3);

    py::class_<SectionState>(m, "SectionState")
        .def(py::init<>())
        .def_readwrite("eps_cc", &SectionState::eps_cc)
        .def_readwrite("eps_ft", &SectionState::eps_ft)
        .def_readwrite("h_cc", &SectionState::h_cc)
        .def_readwrite("h_ft", &SectionState::h_ft)
        .def_readwrite("jac_cc", &SectionState::jac_cc)
        .def_readwrite("jac_ft", &SectionState::jac_ft)
        .def_readwrite("f_cc", &SectionState::f_cc)
        .def_readwrite("f_ft", &SectionState::f_ft)
        .def_readwrite("m_ca", &SectionState::m_ca);

    py::class_<SectionCal>(m, "SectionCal")
        .def(py::init<const CrossSection&, const Points&, const Points&>(),
             py::arg("cs"), py::arg("cc"), py::arg("ft"),
             py::keep_alive<1, 2>(),
             py::keep_alive<1, 3>(),
             py::keep_alive<1, 4>())
        .def("forceresidual", &SectionCal::forceresidual,
             py::arg("eps_ca"), py::arg("kappa"))
        .def("moment", &SectionCal::moment,
             py::arg("eps_ca"), py::arg("kappa"))
        .def("eval", &SectionCal::eval,
             py::arg("eps_ca"), py::arg("kappa"));
}
