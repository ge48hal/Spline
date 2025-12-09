#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "points/points.h"
#include "inputreader/prep.h"
#include "geom/shoelace.h"

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
}
