class SpectrogramComponent : public juce::AudioAppComponent, private juce::Timer, public juce::AudioSourcePlayer
{
public:
    SpectrogramComponent()
        : spectrogramImage(juce::Image::RGB, 512, 512, true)
    {
        setOpaque(true);
        forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);
        startTimerHz(60);
    }

    ~SpectrogramComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override
    {
        // (nothing to do here)
    }

    void releaseResources() override
    {
        // (nothing to do here)
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto numChans = bufferToFill.buffer->getNumChannels();
        auto noSampls = bufferToFill.numSamples;

        if (numChans > 0)
        {
            const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

            for (auto i = 0; i < noSampls; ++i)
                pushNextSampleIntoFifo(channelData[i]);

            bufferToFill.clearActiveBufferRegion();
        }
    }

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int totalNumInputChannels,
        float* const* outputChannelData,
        int totalNumOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override
    {
        if ((totalNumInputChannels > 0) && (numSamples > 0))
        {
            for (auto i = 0; i < numSamples; ++i)
            {
                float sum = 0.0f;
                for (auto channel = 0; channel < totalNumInputChannels; ++channel)
                {
                    sum += inputChannelData[channel][i];
                }
                pushNextSampleIntoFifo(sum);
            }
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        auto sampleRate = device->getCurrentSampleRate();
    }

    void audioDeviceStopped() override
    {
        // (nothing to do here)
    }

    void pushNextSampleIntoFifo(float sample) noexcept
    {
        // (existing code here)...
    }

    void drawNextLineOfSpectrogram()
    {
        // (existing code here)...
    }

    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        g.setOpacity(1.0f);
        g.drawImage(spectrogramImage, getLocalBounds().toFloat());
    }

    enum
    {
        fftOrder = 10,
        fftSize = 1 << fftOrder
    };

private:
    juce::Image spectrogramImage;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    // (other private members related to the spectrogram)...
};


