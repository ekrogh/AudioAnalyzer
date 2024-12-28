import onnx
from onnx import version_converter, helper

# Load the original model
original_model = onnx.load("preprocess_model.onnx")

# Convert the model to opset 15
converted_model = version_converter.convert_version(original_model, 15)

# Save the converted model
onnx.save(converted_model, "preprocess_model_v15.onnx")

# Load the original model
original_model = onnx.load("post_stft_model.onnx")

# Convert the model to opset 15
converted_model = version_converter.convert_version(original_model, 15)

# Save the converted model
onnx.save(converted_model, "post_stft_model_v15.onnx")
