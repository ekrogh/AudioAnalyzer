class AudioProcessor : public juce::AudioIODeviceCallback
{
public:
    AudioProcessor(double sampleRate, int numChannels)
        : filter(sampleRate, numChannels)
    {
    }

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
                               float** outputChannelData, int numOutputChannels,
                               int numSamples) override
    {
        // Copy input to output
        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            if (channel < numInputChannels && inputChannelData[channel] != nullptr && outputChannelData[channel] != nullptr)
            {
                juce::FloatVectorOperations::copy(outputChannelData[channel], inputChannelData[channel], numSamples);
            }
        }

        // Apply filter
        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            if (outputChannelData[channel] != nullptr)
            {
                juce::AudioBuffer<float> buffer(outputChannelData + channel, 1, numSamples);
                filter.process(buffer);
            }
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        // You can put code here that needs to run before the device starts
    }

    void audioDeviceStopped() override
    {
        // You can put code here that needs to run after the device stops
    }

private:
    NotchFilter filter;
};


auto* audioDeviceManager = new juce::AudioDeviceManager();
audioDeviceManager->initialiseWithDefaultDevices(2, 2); // initialize for stereo input and output

auto* audioProcessor = new AudioProcessor(audioDeviceManager->getCurrentAudioDevice()->getCurrentSampleRate(),
    audioDeviceManager->getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits());
audioDeviceManager->addAudioCallback(audioProcessor);
