class FFTModule final
    : public juce::AudioAppComponent
{
public:
    FFTModule
    (
        std::shared_ptr<AudioDeviceManager> SADM
        ,
        std::shared_ptr<freqPlotModule> FPM
    ) :
        module_freqPlot(FPM)
        ,
        AudioAppComponent(*SADM)
    {
        setOpaque(true);

        formatManager.registerBasicFormats();

        setAudioChannels(2, 2);

        setSize(700, 500);
    }

    ~FFTModule() override
    {
        shutdownAudio();
        transportSource.setSource(nullptr);
        audioSourcePlayer.setSource(nullptr);
        setSource(nullptr);
        deviceManager.removeAudioCallback(&audioSourcePlayer);
        deviceManager.removeAudioCallback(this);
    }

    //=============================
    // AudioAppComponent callbacks
    //=============================
    void prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override
    {
        // (nothing to do here)
    }

    void releaseResources() override
    {
        // (nothing to do here)
    }

    //=====================================
    // AudioSourcePlayer Call backs
    //=====================================

    /** Implementation of the AudioIODeviceCallbackWithContext method. */
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int totalNumInputChannels,
        float* const* outputChannelData,
        int totalNumOutputChannels,
        int numSamples,
        const AudioIODeviceCallbackContext& context) override
    {
        // (nothing to do here)
    }

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceAboutToStart(AudioIODevice* device) override
    {
        // (nothing to do here)
    }

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceStopped() override
    {
        // (nothing to do here)
    }

private:
    // (other private members)...
};
