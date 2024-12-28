#include "SpectrogramComponent.h"
#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>

// Function to set the DLL directory
bool SetDllDirectory(const std::wstring &directory)
{
    return SetDllDirectoryW(directory.c_str()) != 0;
}

// Function to convert std::string to std::wstring
std::wstring stringToWString(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

SpectrogramComponent::SpectrogramComponent(AudioFormatManager &FM, std::shared_ptr<AudioDeviceManager> SADM, FFTModule *FFTMP, std::shared_ptr<freqPlotModule> FPM)
    : formatManager(FM), AudioAppComponent(*SADM), spectrogramImage(Image::RGB, 600, 626, true), ptrFFTModule(FFTMP), module_freqPlot(FPM), Thread("Audio file read and FFT"), sharedAudioDeviceManager(SADM)
{
    // Set the DLL directory
    if (!SetDllDirectory(L"C:\\path\\to\\onnxruntime\\dlls"))
    {
        // Handle error (e.g., log it, throw an exception, etc.)
    }

    // Initialize the ONNX model
    onnx_model_ = ONNXModel(stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/preprocess_model.onnx"),
                            stringToWString("D:/Users/eigil/projects/juceProjs/AudioAnalyzer/onnxThings/post_stft_model.onnx"));
}

void SpectrogramComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    auto numChans = bufferToFill.buffer->getNumChannels();
    auto numSamples = bufferToFill.numSamples;

    if (numChans > 0)
    {
        const auto *channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        auto *channelWritePtr = bufferToFill.buffer->getWritePointer(0);

        // Process audio with RNNoise
        if (useAINoiseRemoval)
        {
            // Add your RNNoise processing code here
        }

        if (numChans >= 2)
        {
            bufferToFill.buffer->copyFrom(1, bufferToFill.startSample, channelData, numSamples);
        }

        for (auto i = 0; i < numSamples; ++i)
            pushNextSampleIntoFifo(channelData[i]);

        // Call processBlock with the audio buffer
        processBlock(*bufferToFill.buffer);

        // bufferToFill.clearActiveBufferRegion(); // Uncomment if you need to clear the buffer
    }
}

void SpectrogramComponent::processBlock(juce::AudioBuffer<float> &buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float *audio_data = buffer.getWritePointer(channel, 0);
        onnx_model_.processAudio(audio_data, buffer.getNumSamples());
    }
}
