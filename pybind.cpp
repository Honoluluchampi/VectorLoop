#include "VectorLoop.hpp"
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

template <typename T>
auto vector2pyarray(const std::vector<T>& vec) -> pybind11::array_t<T>
{ return py::array_t<T>({ vec.size() }, { sizeof(T) }, vec.data()); }

template <typename T>
auto polyrize_svg_wrapper(const std::string& file_path, int div_count)
{ return vector2pyarray(VL::polyrize_svg<T>(file_path, div_count)); }

PYBIND11_MODULE(VectorLoop, m) {
  m.def("polyrize_svg_float32", &polyrize_svg_wrapper<float>, py::arg("file_path"), py::arg("div_count"));
  m.def("polyrize_svg_float64", &polyrize_svg_wrapper<double>, py::arg("file_path"), py::arg("div_count"));
}