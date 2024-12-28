#include "ONNXModel.h"
#include <iostream>
#include <vector>

ONNXModel::ONNXModel(const std::wstring& preprocess_model_path, const std::wstring& main_model_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "ONNXModel")
    , session_options()
{
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    preprocess_session_ = std::make_unique<Ort::Session>(env, preprocess_model_path.c_str(), session_options);
    main_session_ = std::make_unique<Ort::Session>(env, main_model_path.c_str(), session_options);
}

void ONNXModel::processAudio(float* audio_data, int num_samples)
{
    const char* errmsg;

    try
    {
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        // Adjust input tensor shape for preprocess model
        std::array<int64_t, 3> preprocess_input_shape{ 1, 2, 343980 };
        std::vector<float> extended_audio_data(preprocess_input_shape[1] * preprocess_input_shape[2], 0);
        for (int i = 0; i < num_samples && i < extended_audio_data.size(); ++i)
            extended_audio_data[i] = audio_data[i];

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, extended_audio_data.data(), extended_audio_data.size(), preprocess_input_shape.data(), preprocess_input_shape.size());

        const char* preprocess_input_names[] = { "onnx::ReduceMean_0" };
        const char* preprocess_output_names[] = { "5" };  // Updated with the correct output name
        auto preprocess_output_tensors = preprocess_session_->Run(Ort::RunOptions{ nullptr }, preprocess_input_names, &input_tensor, 1, preprocess_output_names, 1);

        Ort::Value& preprocess_output_tensor = preprocess_output_tensors.front();
        float* preprocess_output_data = preprocess_output_tensor.GetTensorMutableData<float>();

        // Adjust input tensor shape for main model
        std::array<int64_t, 4> main_input_shape{ 1, 4, 1025, 87 };  // Use the shape expected by the main model
        Ort::Value main_input_tensor = Ort::Value::CreateTensor<float>(memory_info, preprocess_output_data, preprocess_output_tensor.GetTensorTypeAndShapeInfo().GetElementCount(), main_input_shape.data(), main_input_shape.size());

        const char* main_input_names[] = { "mix" };
        const char* main_output_names[] = { "560" };  // Updated with the correct output name
        auto main_output_tensors = main_session_->Run(Ort::RunOptions{ nullptr }, main_input_names, &main_input_tensor, 1, main_output_names, 1);

        Ort::Value& main_output_tensor = main_output_tensors.front();
        const float* main_output_data = main_output_tensor.GetTensorMutableData<float>();
    }
    catch (const Ort::Exception& e)
    {
        errmsg = e.what();
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
    }
}
