class CustomAudioSource : public juce::AudioSource
{
public:
    CustomAudioSource(juce::AudioSource* source, FFTModule& fftModule)
        : audioSource(source), fftModule(fftModule)
    {
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        audioSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override
    {
        audioSource->releaseResources();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        audioSource->getNextAudioBlock(bufferToFill);

        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                fftModule.pushNextSampleIntoFifo(channelData[i]);
        }
    }

private:
    juce::AudioSource* audioSource;
    FFTModule& fftModule;
};
