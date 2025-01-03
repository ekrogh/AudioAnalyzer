#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>
#include <iostream>

// Function to convert std::string to std::wstring
std::wstring stringToWString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

// Function to modify the PATH environment variable
void addDllDirectoryToPath(const std::wstring& dllPath)
{
    wchar_t currentPath[MAX_PATH];
    if (GetEnvironmentVariable(L"PATH", currentPath, MAX_PATH))
    {
        std::wstring newPath = dllPath + L";" + currentPath;
        SetEnvironmentVariable(L"PATH", newPath.c_str());
    }
    else
    {
        SetEnvironmentVariable(L"PATH", dllPath.c_str());
    }
}

int main()
{
    auto tst = SetDefaultDllDirectories(
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR
    );
    // Add the custom DLL directory to the PATH
    //std::wstring dllPath = L"D:/Users/eigil/projects/machineLearning/onnxruntime/build/Windows/RelWithDebInfo/RelWithDebInfo";
    //addDllDirectoryToPath(dllPath);

    // Load the ONNX Runtime DLL explicitly
    HMODULE hModule = LoadLibrary(L"onnxruntime.dll");
    if (hModule == NULL)
    {
        std::cerr << "Failed to load ONNX Runtime DLL: " << GetLastError() << std::endl;
        return -1;
    }

    try
    {
        // Verify the correct version of the ONNX Runtime is being used
        std::string version = Ort::GetVersionString();
        std::cout << "ONNX Runtime version: " << version << std::endl;

        hModule = GetModuleHandle(L"onnxruntime.dll");
        if (hModule != NULL)
        {
            wchar_t path[MAX_PATH];
            if (GetModuleFileName(hModule, path, MAX_PATH))
            {
                std::wcout << L"ONNX Runtime DLL path: " << path << std::endl;
            }
            else
            {
                std::cerr << "Failed to get ONNX Runtime DLL path: " << GetLastError() << std::endl;
            }
        }
        else
        {
            std::cerr << "ONNX Runtime DLL not loaded: " << GetLastError() << std::endl;
        }

        // Initialize the ONNX Runtime environment
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel");
        Ort::SessionOptions session_options;
        std::wstring preprocess_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model.onnx");
        //std::wstring preprocess_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model_v15.onnx");
        std::wstring main_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model.onnx");
        //std::wstring main_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model_v15.onnx");

        Ort::Session preprocess_session(env, preprocess_model_path.c_str(), session_options);
        Ort::Session main_session(env, main_model_path.c_str(), session_options);

        std::cout << "ONNX Runtime environment initialized successfully." << std::endl;
    }
    catch (const Ort::Exception& e)
    {
        std::cerr << "ONNX Runtime initialization failed: " << e.what() << std::endl;
        return -1;
    }

    // Free the loaded DLL
    if (!FreeLibrary(hModule))
    {
        std::cerr << "Failed to free the ONNX Runtime DLL: " << GetLastError() << std::endl;
    }

    return 0;
}
