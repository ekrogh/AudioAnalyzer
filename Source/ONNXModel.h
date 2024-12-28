#pragma once

#include <onnxruntime_cxx_api.h>
#include <memory>
#include <string>

class ONNXModel
{
public:
    ONNXModel() = default;
    ONNXModel(const std::wstring& preprocess_model_path, const std::wstring& main_model_path);
    void processAudio(float* audio_data, int num_samples);

private:
    Ort::Env env{ ORT_LOGGING_LEVEL_WARNING, "ONNXModel" };
    Ort::SessionOptions session_options;
    std::unique_ptr<Ort::Session> preprocess_session_;
    std::unique_ptr<Ort::Session> main_session_;
};
