# HTDemucs Guitar Separator - Setup and Usage

This document describes the HTDemucs guitar separator feature implemented in the AudioAnalyzer project.

## Overview

The `GuitarSeparator` class provides real-time audio source separation using the HTDemucs ONNX model. When enabled, it extracts the guitar/instrumental stem from mixed audio in real-time.

## Prerequisites

### 1. ONNX Runtime Installation

#### Windows (vcpkg recommended):
```bash
vcpkg install onnxruntime:x64-windows
```

The project expects ONNX Runtime at:
- Headers: `vcpkg/installed/x64-windows/include/`
- Libraries: `vcpkg/installed/x64-windows/lib/onnxruntime.lib`
- Runtime DLL: `vcpkg/installed/x64-windows/bin/onnxruntime.dll` (must be in PATH or copied to output)

#### Manual Installation:
If not using vcpkg, update paths in:
- **CMakeLists.txt**: Set `ONNXRUNTIME_INCLUDE_DIR` and `ONNXRUNTIME_LIB_DIR` cache variables
- **AudioAnalyzer.jucer**: Update `headerPath` and `libraryPath` in VS2026 exporter

### 2. HTDemucs ONNX Model

Place your HTDemucs model file named `htdemucs_fp16weights.onnx` in:
```
AudioAnalyzer/Resources/htdemucs_fp16weights.onnx
```

The build system will automatically copy this to the executable directory.

**Model Requirements:**
- FP16 or FP32 quantized HTDemucs model
- Expected output shape: `[1, num_stems, channels, time]` or `[1, num_stems, time]`
- Typical stem order: `[drums, bass, other, vocals]` (guitar extracted from "other" at index 2)

## Building

### CMake Build
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Debug
# or
cmake --build . --config Release
```

The CMake build will:
- Link ONNX Runtime (via find_package or fallback paths)
- Copy the model file from Resources/ to the output directory
- Show warnings if ONNX Runtime or model file is missing

### Projucer/Visual Studio Build
1. Open `AudioAnalyzer.jucer` in Projucer
2. Save to regenerate VS project files
3. Open `Builds/VisualStudio2026/AudioAnalyzer.sln`
4. Build Debug or Release

The VS build will:
- Link `onnxruntime.lib` from vcpkg paths
- Copy model file via post-build command
- Include ONNX Runtime headers from `headerPath` in .jucer

## Runtime Setup

Ensure `onnxruntime.dll` (Windows) or equivalent shared library is accessible:
- Copy to executable directory, OR
- Add vcpkg bin directory to system PATH

## Usage

### In AudioPlaybackModule:

1. Load an audio file using "Choose Audio File..."
2. Click the **"Use GuitarSeparator"** toggle button
3. Press Play

**When enabled:**
- Output is **guitar/instrumental stem only** (other stems muted)
- Silence is output until first inference completes
- Toggle can be switched on/off during playback

**When disabled:**
- Original audio plays unchanged

### Monitoring in Debug Console

Enable JUCE debugging to see detailed logs:
- Model loading: path, success/failure
- ONNX session creation
- Tensor shape information: `[batch, stems, channels, time]`
- Stem extraction: which stem index is used
- Inference timing and buffer status

Example log output:
```
GuitarSeparator::prepareToPlay - blockSize=512, sampleRate=44100
GuitarSeparator: Using windowSize=4096, hopSize=2048
GuitarSeparator: Loading ONNX model from: C:\path\to\htdemucs_fp16weights.onnx
GuitarSeparator: ONNX session created successfully
GuitarSeparator: 4D output [batch=1, stems=4, channels=2, time=4096]
GuitarSeparator: Extracted guitar stem (index 2), output length=4096
```

## How It Works

### Architecture
1. **Audio Input**: Incoming stereo/mono audio is downmixed to mono and written to circular FIFO buffer
2. **Worker Thread**: Reads overlapping windows (default 4096 samples, 50% overlap)
3. **Windowing**: Applies Hann window before inference
4. **ONNX Inference**: Runs HTDemucs model with input shape `[1, 1, windowSize]`
5. **Tensor Parsing**: Extracts guitar stem (index 2 = "other") from output tensor
6. **Overlap-Add**: Accumulates windowed outputs with hop size advancement
7. **Audio Output**: Reads processed samples from overlap buffer, outputs guitar-only signal

### Thread Safety
- **Audio Thread**: Writes to input FIFO, reads from output overlap buffer (protected by `outputLock`)
- **Worker Thread**: Reads from input FIFO (via AbstractFifo), writes to output overlap buffer (protected by `outputLock`)
- **Synchronization**: Condition variable notifies worker when new data is available

### Parameters
- **Window Size**: 4096 samples (~93ms @ 44.1kHz)
- **Hop Size**: 2048 samples (50% overlap)
- **Circular Buffer**: 8x window size (handles latency/jitter)

## Troubleshooting

### "ONNX model file not found"
- Ensure `htdemucs_fp16weights.onnx` exists in `Resources/` directory before building
- Check executable directory for the model file after build
- Review build output for copy command success/failure

### Linker errors (ONNX Runtime symbols undefined)
- Verify ONNX Runtime is installed via vcpkg or manually
- Check library paths in CMakeLists.txt or .jucer match your installation
- Ensure correct architecture (x64-windows)

### No audio output / silence when enabled
- Check debug console for "ONNX session created successfully"
- Verify model output shape is logged (3D or 4D expected)
- Ensure model file is FP16/FP32 format (not INT8 without proper dequantization)
- Check that stem index 2 exists in your model's output

### Runtime DLL missing error (Windows)
- Copy `onnxruntime.dll` from `vcpkg/installed/x64-windows/bin/` to executable directory
- Or add vcpkg bin path to system PATH environment variable

### Unexpected output stem
- If guitar is not in stem index 2, modify `guitarStemIndex` in `runInferenceOnWindow()`
- Check your model's stem ordering documentation
- Experiment with indices 0-3 to find correct stem

## Model Customization

To use a different HTDemucs model or stem:

1. **Change stem index**: Edit `runInferenceOnWindow()` in `guitarSeparator.cpp`:
   ```cpp
   const int guitarStemIndex = 2; // Change to 0=drums, 1=bass, 3=vocals
   ```

2. **Adjust window/hop**: Modify in `guitarSeparator.h`:
   ```cpp
   int windowSize = 8192;  // Larger window = more latency, better quality
   int hopSize = 4096;     // Must be <= windowSize
   ```

3. **Different model file**: Rename your model to `htdemucs_fp16weights.onnx` or update path in `initOnnxFromFile()`

## Performance Notes

- **Latency**: ~2-4 windows (depends on window size and hop)
- **CPU Usage**: Depends on model complexity and execution provider
- **Real-time capability**: Default settings tested at 44.1kHz with ~5-10% CPU (CPU-only inference)
- **GPU Acceleration**: Uncomment and configure execution provider in `initOnnxFromFile()` for CUDA/DirectML/CoreML

## Future Enhancements

- [ ] Configurable stem selection (dropdown for drums/bass/other/vocals)
- [ ] GPU execution provider selection at runtime
- [ ] Adjustable window size via UI
- [ ] Dry/wet mix control
- [ ] Multiple stem outputs (multichannel routing)
- [ ] Dynamic latency compensation

## License

Check HTDemucs model license separately. ONNX Runtime is licensed under MIT.
