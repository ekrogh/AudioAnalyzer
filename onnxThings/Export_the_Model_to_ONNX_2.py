import torch
import torch.nn.functional as F
from demucs.pretrained import get_model
from pathlib import Path

# Custom preprocessing function outside ONNX
def preprocess_audio(x):
    batch_size, channels, length = x.shape
    stft_outputs = []
    for i in range(channels):
        stft_output = torch.stft(x[:, i, :], n_fft=2048, hop_length=512, win_length=2048, window=torch.hann_window(2048), return_complex=True)
        stft_outputs.append(stft_output)
    return torch.stack(stft_outputs, dim=1)

# Custom function to split real and imaginary parts
def split_real_imag(stft_output):
    real_part = stft_output.real
    imag_part = stft_output.imag
    return torch.cat([real_part, imag_part], dim=1)

# Custom 1D padding function
def custom_pad1d(input, pad, mode='constant', value=0):
    if mode == 'constant':
        return F.pad(input, pad, mode=mode, value=value)
    else:
        raise NotImplementedError(f"Padding mode '{mode}' is not supported for 1D inputs")

# Set the directory for torch.hub
torch.hub.set_dir('D:/Users/eigil/projects/pythonProjs/demucs/demucs/remote/')

# Download the main model using torch.hub
model_name = 'htdemucs_6s'
main_model = get_model(model_name)
main_model.eval()

# Save the main model's state dictionary (if needed)
model_save_path = Path('D:/Users/eigil/projects/pythonProjs/demucs/demucs/remote/htdemucs_6s.pth')
if not model_save_path.exists():
    torch.save(main_model.state_dict(), model_save_path)
    print(f"Model saved to {model_save_path}")

# Define a wrapper class for the post-STFT part of the main model
class PostSTFTModelWrapper(torch.nn.Module):
    def __init__(self, model):
        super(PostSTFTModelWrapper, self).__init__()
        self.model = model
        self.hop_length = 512  # Define hop_length directly here

    def forward(self, x):
        with torch.no_grad():
            tracks = []
            for sub_model in self.model.models:
                valid_length = sub_model.valid_length(x.shape[-1])
                padding = valid_length - x.shape[-1]
                x_padded = custom_pad1d(x, (padding, padding), mode="constant")
                y = sub_model(x_padded)
                y = y[..., :x.shape[-1]]
                tracks.append(y)
            return torch.stack(tracks)

    def _spec(self, mix):
        pad = self.hop_length  # Use the hop_length defined here
        hl = self.hop_length
        le = int(torch.ceil(mix.shape[-1] / hl))
        mix = custom_pad1d(mix, (pad, pad + le * hl - mix.shape[-1]), mode="constant")
        return mix

    def forward(self, mix):
        z = self._spec(mix)
        return z  # Replace with actual forward operations in the model

# Instantiate the post-STFT model
post_stft_model = PostSTFTModelWrapper(main_model)

# Define a dummy input with the appropriate shape
dummy_input = torch.randn(1, 2, 44100)  # Example input shape for stereo audio with 44100 samples

# Perform STFT preprocessing
stft_output = preprocess_audio(dummy_input)
print(stft_output.shape)  # Verify the shape to ensure correctness

# Split real and imaginary parts for ONNX export
stft_split_output = split_real_imag(stft_output)
print(stft_split_output.shape)  # Verify the shape of the split output

# Export the post-STFT model to ONNX
post_stft_onnx_path = "post_stft_model.onnx"
torch.onnx.export(post_stft_model, stft_split_output, post_stft_onnx_path, opset_version=17)
print(f"Post-STFT model exported to {post_stft_onnx_path}")
