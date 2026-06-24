#pragma once

#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <thread>
#include <atomic>
#include <condition_variable>

// GuitarSeparator
// - JUCE AudioSource that performs windowed HTDemucs ONNX inference off the audio thread.
// - Model is loaded from BinaryData::htdemucs_fp16weights_onnx (embed via Projucer Binary Resources).

class GuitarSeparator : public juce::AudioSource
{
public:
    GuitarSeparator();
    ~GuitarSeparator() override;

    // AudioSource interface
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, it will be swapped for the
        new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    void setSource(AudioSource* newSource);
    void setUseDelayedDryFallback(bool shouldUse);

    // Optional: call to run a quick smoke test (non-blocking)
    void runStartupDiagnostics();

private:
    // ONNX Runtime
    std::unique_ptr<Ort::Env> ortEnv;
    std::unique_ptr<Ort::Session> ortSession;
    Ort::SessionOptions sessionOptions;

    // Model file written from BinaryData (temp file path)
    juce::File modelTempFile;

    // Worker thread + synchronization
    std::thread workerThread;
    std::atomic<bool> workerRunning{ false };
    std::condition_variable_any workerCv;
    //juce::CriticalSection workerLock;
    std::mutex workerMutex;

    // Streaming buffers (stereo)
    juce::AudioBuffer<float> circularBuffer; // stereo circular buffer
    juce::AbstractFifo fifo;
    int circularCapacity = 0;

    // Overlap-add output buffer (stereo)
    juce::AudioBuffer<float> outputOverlapBuffer;
    juce::CriticalSection outputLock;
    int processedOutputSamples = 0;
    bool hasProcessedOutput = false;
    std::atomic<bool> useDelayedDryFallback{ false };

    // Delayed dry fallback (stereo)
    juce::AudioBuffer<float> dryDelayBuffer;
    int dryDelayWritePos = 0;
    int dryDelaySamples = 0;
    int64_t dryDelaySamplesWritten = 0;

    // MDX-Net segmenting / windowing parameters
    int segmentSize = 0;          // derived from target seconds and sample rate
    int hopSize = 0;              // 50% overlap by default
    int overlapSize = 0;
    std::vector<float> hannWindow;
    std::vector<float> inputSegment;
    std::vector<float> outputSegment;

    // Runtime state
    double currentSampleRate = 44100.0;
    std::atomic<bool> onnxReady{ false };
    int modelInputChannels = 2;
    int modelInputSamples = 0;

    // Simple counters
    int writeIndex = 0;

    // Helpers
    void initOnnxFromFile();
    void startWorker();
    void stopWorker();
    void workerLoop();
    bool runInferenceOnWindow(const std::vector<float>& segment, std::vector<float>& outStereo);
    std::vector<float> makeHannWindow(int size);

    int blockSize = 128;
    juce::AudioSource* source = nullptr;
    //juce::CriticalSection readLock;
    juce::CriticalSection callbackLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarSeparator)
};
