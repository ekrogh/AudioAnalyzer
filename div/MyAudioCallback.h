class MyAudioCallback : public juce::AudioIODeviceCallback
{
public:
    void audioDeviceIOCallbackWithContext (const float** inputChannelData,
                                           int numInputChannels,
                                           float** outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override
    {
        // This is where you handle the incoming and outgoing audio data.
        // For example, you might just copy the input to the output to create a pass-through:
        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            if (outputChannelData[channel] != nullptr)
            {
                const float* inputData = (channel < numInputChannels && inputChannelData[channel] != nullptr)
                                            ? inputChannelData[channel]
                                            : nullptr;

                if (inputData != nullptr)
                    memcpy (outputChannelData[channel], inputData, sizeof (float) * numSamples);
                else
                    memset (outputChannelData[channel], 0, sizeof (float) * numSamples);
            }
        }
    }

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override
    {
        // This is where you can prepare any resources you need before the audio starts playing.
    }

    void audioDeviceStopped() override
    {
        // This is where you can release any resources you no longer need after the audio has stopped.
    }
};


juce::AudioDeviceManager deviceManager;
MyAudioCallback callback;

// Initialise the device manager with default settings
deviceManager.initialiseWithDefaultDevices(2, 2);

// Set our callback as the audio callback
deviceManager.addAudioCallback(&callback);

// ...later, when you're done with the audio...
deviceManager.removeAudioCallback(&callback);



class CustomAudioSourcePlayer : public juce::AudioSourcePlayer
{
public:
    void audioDeviceIOCallback(const float** inputChannelData,
        int numInputChannels,
        float** outputChannelData,
        int numOutputChannels,
        int numSamples) override
    {
        // This is where you process the audio data.
        // For example, you might just copy the input to the output to create a pass-through:
        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            if (outputChannelData[channel] != nullptr)
            {
                const float* inputData = (channel < numInputChannels && inputChannelData[channel] != nullptr)
                    ? inputChannelData[channel]
                    : nullptr;

                if (inputData != nullptr)
                    memcpy(outputChannelData[channel], inputData, sizeof(float) * numSamples);
                else
                    memset(outputChannelData[channel], 0, sizeof(float) * numSamples);
            }
        }
    }
};

juce::AudioDeviceManager deviceManager;
CustomAudioSourcePlayer player;

// Initialise the device manager with default settings
deviceManager.initialiseWithDefaultDevices(2, 2);

// Set our player as the audio callback
deviceManager.addAudioCallback(&player);

// Set the source for the player
juce::AudioTransportSource transportSource;
player.setSource(&transportSource);

// ...later, when you're done with the audio...
player.setSource(nullptr);
deviceManager.removeAudioCallback(&player);
