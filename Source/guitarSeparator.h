/*
  ==============================================================================

    guitarSeparator.h
    Created: 24 Sep 2025 1:27:21am
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

//==============================================================================
class guitarSeparator : public AudioSource,
                        private juce::Thread
{
public:
    guitarSeparator();
    ~guitarSeparator() override;

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock(const AudioSourceChannelInfo&) override;


    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, its releaseResources() method
        will be called after it has been swapped for the new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    void setSource(AudioSource* newSource);

    bool isModelReady() const;
    juce::String getBackendStatusText() const;

private:
    struct ModelManifest
    {
        int sampleRate = 44100;
        int channels = 2;
        int windowFrames = 8192;
        int guitarStemIndex = 4;
        juce::String modelFileName{ "htdemucs_6s_guitar.onnx" };
        juce::String manifestFileName{ "htdemucs_guitar_manifest.json" };
    };

    struct HtDemucsOnnxBackend;

    enum class BackendState
    {
        idle,
        downloadingModel,
        ready,
        fallback,
        failed
    };

    void run() override;

    void initialiseSeparator();
    void configureFallbackFilters();
    void resetProcessingState();
    void updateBackendState(BackendState newState, const juce::String& newStatus);

    juce::File getModelDirectory() const;
    juce::File getModelFile() const;
    juce::File getManifestFile() const;
    juce::String getConfiguredModelUrl() const;
    juce::String getConfiguredManifestUrl() const;

    bool ensureModelAssetsAvailable();
    bool downloadModelAssets();
    bool parseModelManifest();
    bool downloadFileTo(const juce::URL& url, const juce::File& destination) const;
    bool tryLoadHtDemucsBackend();
    bool processAvailableWindow();
    bool processCurrentWindow(std::vector<float>& outputWindow);
    void processFallbackWindow(const std::vector<float>& inputWindow, std::vector<float>& outputWindow);

    void queueInputSamples(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    bool popOutputSamples(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    bool popInputWindow(std::vector<float>& inputWindow);
    void pushOutputWindow(const std::vector<float>& outputWindow);

    AudioSource* source = nullptr;
    double sampleRate = 44100.0;

    CriticalSection callbackLock;
    CriticalSection queueLock;
    mutable CriticalSection stateLock;
    int blockSize = 512;

    ModelManifest modelManifest;
    std::unique_ptr<HtDemucsOnnxBackend> onnxBackend;

    std::vector<float> inputRing;
    std::vector<float> outputRing;
    std::vector<float> processInputWindow;
    std::vector<float> processOutputWindow;
    std::array<std::vector<float>, 2> fallbackChannelScratch;

    int ioChannels = 2;
    int inputCapacityFrames = 0;
    int outputCapacityFrames = 0;
    int inputWriteFrame = 0;
    int inputReadFrame = 0;
    int inputQueuedFrames = 0;
    int outputWriteFrame = 0;
    int outputReadFrame = 0;
    int outputQueuedFrames = 0;

    std::array<juce::IIRFilter, 2> highPassFilters;
    std::array<juce::IIRFilter, 2> lowPassFilters;

    std::atomic<bool> backendReady{ false };
    bool modelAssetsChecked = false;
    bool modelAssetsAvailable = false;
    bool preferDryWhileWarmingUp = true;
    BackendState backendState = BackendState::idle;
    juce::String backendStatusText{ "HTDemucs not initialised" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (guitarSeparator)
};
