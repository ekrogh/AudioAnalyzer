cmake_minimum_required(VERSION 3.12)
project(AudioProcessingBindings)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)

# Find Python and pybind11
find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

# Set pybind11_DIR to the directory containing pybind11Config.cmake
# You can replace the path below with the output of `python -m pybind11 --cmakedir`
set(pybind11_DIR "D:/Users/eigil/.conda/envs/spleeter_env/lib/site-packages/pybind11/share/cmake/pybind11")

find_package(pybind11 REQUIRED)

# Include directories for Python and pybind11
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${pybind11_INCLUDE_DIRS})

# Add the library
add_library(audio_processing_bindings STATIC
    pythonThings/audio_processing_bindings.cpp
)

# Link libraries
target_link_libraries(audio_processing_bindings ${PYTHON_LIBRARIES} pybind11::module)

# Set the output directory for the static library
set_target_properties(audio_processing_bindings PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    PREFIX ""
    SUFFIX ".lib"
)
