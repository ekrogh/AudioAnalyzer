#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
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
	//if (!SetDllDirectory(L"D:/Users/eigil/projects/machineLearning/onnxruntime/build/Windows/RelWithDebInfo/RelWithDebInfo"))
	//{
	//	std::cerr << "Failed to set DLL directory." << std::endl;
	//	return -1;
	//}

	try
	{

		std::string version = Ort::GetVersionString();
		std::cout << "ONNX Runtime version: " << version << std::endl;

		Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel");
		Ort::SessionOptions session_options;
		std::wstring preprocess_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model_v15.onnx");
		std::wstring main_model_path = stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model_v15.onnx");

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
