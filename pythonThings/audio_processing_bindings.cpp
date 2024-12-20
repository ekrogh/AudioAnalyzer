#include "audio_processing_bindings.h"
#include <spleeter/spleeter.h>
#include <vector>
#include <iostream>

// Function to separate audio using Spleeter
void spleeter_separate(const float* input_buffer, size_t buffer_size, float* output_buffer)
{
    try
    {
        // Initialize Spleeter with the 2 stems model
        spleeter::Spleeter spleeter("spleeter:2stems");

        // Convert input buffer to a vector
        std::vector<float> input_audio(input_buffer, input_buffer + buffer_size);

        // Separate the audio
        auto result = spleeter.separate(input_audio);

        // Extract the separated audio from the result
        const auto& separated_audio = result["vocals"]; // or "accompaniment"
        std::copy(separated_audio.begin(), separated_audio.end(), output_buffer);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in spleeter_separate: " << e.what() << std::endl;
        throw;
    }
}




// #include "audio_processing_bindings.h"
// #include <pybind11/pybind11.h>
// #include <pybind11/numpy.h>
// #include <pybind11/stl.h>
// #include <iostream>

// namespace py = pybind11;

// void spleeter_separate(const float* input_buffer, size_t buffer_size, float* output_buffer)
// {
//     try
//     {
//         py::module_ spleeter = py::module_::import("spleeter.separator");
//         py::object Separator = spleeter.attr("Separator");
//         py::object separator = Separator("spleeter:2stems");

//         // Convert input buffer to numpy array
//         py::array_t<float> input_audio({ static_cast<py::ssize_t>(buffer_size) }, input_buffer);

//         // Call Spleeter separation
//         py::object result = separator.attr("separate")(input_audio);

//         // Extract the separated audio from the result
//         py::array_t<float> separated_audio = result.cast<py::array_t<float>>();
//         std::memcpy(output_buffer, separated_audio.data(), buffer_size * sizeof(float));
//     }
//     catch (const py::error_already_set& e)
//     {
// 		auto rmsg = e.what();

//         //std::cerr << "Error in spleeter_separate: " << e.what() << std::endl;
//         //throw;
//     }
// }

// void openunmix_separate(const float* input_buffer, size_t buffer_size, float* output_buffer)
// {
//     // try
//     // {
//         py::module_ openunmix = py::module_::import("openunmix");
//         py::object separate = openunmix.attr("separate");

//         // Convert input buffer to numpy array
//         py::array_t<float> input_audio({ static_cast<py::ssize_t>(buffer_size) }, input_buffer);

//         // Call OpenUnmix separation
//         py::object result = separate(input_audio);

//         // Extract the separated audio from the result
//         py::array_t<float> separated_audio = result.cast<py::array_t<float>>();
//         std::memcpy(output_buffer, separated_audio.data(), buffer_size * sizeof(float));
//     // }
//     // catch (const py::error_already_set& e)
//     // {
//     //     std::cerr << "Error in openunmix_separate: " << e.what() << std::endl;
//     //     throw;
//     // }
// }

// PYBIND11_MODULE(audio_processing, m)
// {
//     m.def("spleeter_separate", &spleeter_separate, "Separate audio using Spleeter");
//     m.def("openunmix_separate", &openunmix_separate, "Separate audio using OpenUnmix");
// }
