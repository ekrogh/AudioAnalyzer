import onnx

preprocess_model_path = "D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model_v15.onnx"
preprocess_model = onnx.load(preprocess_model_path)
for input in preprocess_model.graph.input:
    print(f"Input name: {input.name}")
    print(f"Input shape: {[dim.dim_value for dim in input.type.tensor_type.shape.dim]}")
for output in preprocess_model.graph.output:
    print(f"Output name: {output.name}")

post_stft_model_path = "D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model_v15.onnx"
post_stft_model = onnx.load(post_stft_model_path)
for input in post_stft_model.graph.input:
    print(f"Input name: {input.name}")
    print(f"Input shape: {[dim.dim_value for dim in input.type.tensor_type.shape.dim]}")
for output in post_stft_model.graph.output:
    print(f"Output name: {output.name}")


post_stft_model_path = "D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model_v15.onnx"
post_stft_model = onnx.load(post_stft_model_path)
for output in post_stft_model.graph.output:
    print(f"Output name: {output.name}")
