import torch
from demucs import pretrained
from demucs.hdemucs import HDemucs

class SimplifiedDemucs(HDemucs):
    def __init__(self, *args, **kwargs):
        super(SimplifiedDemucs, self).__init__(*args, **kwargs)

    def forward(self, x):
        # Simplified forward pass
        length = x.shape[-1]
        stride = 2 ** self.depth
        if length % stride != 0:
            pad = stride - length % stride
            x = torch.nn.functional.pad(x, (0, pad))
        x = self.encode(x)
        x = self.decode(x)
        return x[..., :length]

    def encode(self, x):
        for layer in self.encoder:
            x = layer(x)
        return x

    def decode(self, x):
        for layer in self.decoder:
            x = layer(x)
        return x

# Load the pretrained Demucs model
model = pretrained.get_model('htdemucs_6s')
model.eval()

# Create a simplified version of the model
simplified_model = SimplifiedDemucs(sources=model.sources, audio_channels=model.audio_channels, samplerate=model.samplerate)
simplified_model.load_state_dict(model.state_dict(), strict=False)
simplified_model.eval()

# Dummy input for tracing
dummy_input = torch.randn(1, 2, 44100)  # Ensure the input tensor has 2 channels (stereo)

# Export the simplified model to ONNX format
torch.onnx.export(simplified_model, dummy_input, "simplified_demucs.onnx", opset_version=11)
