#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void spleeter_separate(const std::string& input_path, const std::string& output_path)
{
    py::module_ spleeter = py::module_::import("spleeter.separator");
    py::object Separator = spleeter.attr("Separator");
    py::object separator = Separator("spleeter:2stems");
    separator.attr("separate_to_file")(input_path, output_path);
}

void openunmix_separate(const std::string& input_path, const std::string& output_path)
{
    py::module_ openunmix = py::module_::import("openunmix");
    py::object separate = openunmix.attr("separate");
    separate(input_path, output_path);
}

PYBIND11_MODULE(audio_processing, m)
{
    m.def("spleeter_separate", &spleeter_separate, "Separate audio using Spleeter");
    m.def("openunmix_separate", &openunmix_separate, "Separate audio using OpenUnmix");
}
