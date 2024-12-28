#include <onnxruntime_cxx_api.h>
#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>
#include <iostream>

// Function to set the DLL directory
bool SetDllDirectory(const std::wstring& directory)
{
    return SetDllDirectoryW(directory.c_str()) != 0;
}

// Function to convert std::string to std::wstring
std::wstring stringToWString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

int main()
{
    // Set the DLL directory
    if (!SetDllDirectory(L"D:/Users/eigil/projects/machineLearning/onnxruntime-win-x64-1.20.1/lib/"))
    {
        std::cerr << "Failed to set DLL directory." << std::endl;
        return -1;
    }

    try
    {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel");
        Ort::SessionOptions session_options;
        std::wstring preprocess_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model.onnx");
        std::wstring main_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model.onnx");

        Ort::Session preprocess_session(env, preprocess_model_path.c_str(), session_options);
        Ort::Session main_session(env, main_model_path.c_str(), session_options);

        std::cout << "ONNX Runtime environment initialized successfully." << std::endl;
    }
    catch (const Ort::Exception& e)
    {
        std::cerr << "ONNX Runtime initialization failed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
