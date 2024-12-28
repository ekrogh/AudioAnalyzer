#include "ONNXModel.h"

ONNXModel::ONNXModel(const std::wstring& preprocess_model_path, const std::wstring& main_model_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel"), session_options()
{
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    preprocess_session_ = std::make_unique<Ort::Session>(env, preprocess_model_path.c_str(), session_options);
    main_session_ = std::make_unique<Ort::Session>(env, main_model_path.c_str(), session_options);
}

void ONNXModel::processAudio(float* audio_data, int num_samples)
{
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::array<int64_t, 2> input_shape{ 1, num_samples };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, audio_data, num_samples, input_shape.data(), input_shape.size());

    const char* preprocess_input_names[] = { "input" };
    const char* preprocess_output_names[] = { "output" };
    auto preprocess_output_tensors = preprocess_session_->Run(Ort::RunOptions{ nullptr }, preprocess_input_names, &input_tensor, 1, preprocess_output_names, 1);

    Ort::Value& preprocess_output_tensor = preprocess_output_tensors.front();
    float* preprocess_output_data = preprocess_output_tensor.GetTensorMutableData<float>();

    Ort::Value main_input_tensor = Ort::Value::CreateTensor<float>(memory_info, preprocess_output_data, input_shape.size(), input_shape.data(), input_shape.size());
    const char* main_input_names[] = { "input" };
    const char* main_output_names[] = { "output" };
    auto main_output_tensors = main_session_->Run(Ort::RunOptions{ nullptr }, main_input_names, &main_input_tensor, 1, main_output_names, 1);

    Ort::Value& main_output_tensor = main_output_tensors.front();
    const float* main_output_data = main_output_tensor.GetTensorMutableData<float>();
}
