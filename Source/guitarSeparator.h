/*
  ==============================================================================

    guitarSeparator.h
    Created: 24 Sep 2025 1:27:21am
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "rnnoise.h"
#include <vector>
#include <deque>

//==============================================================================
/*
*/
class guitarSeparator : public AudioSource
{
public:
    guitarSeparator();
    ~guitarSeparator() override;

    void initializeRNNoise();

    // Replaced implementation: this now runs our guitar-only spectral separator.
    // It preserves the signature to avoid changing call sites.
    float rnnoise_process(float* pFrameOut, const float* pFrameIn);

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

private:
    // RNNoise state (kept for compatibility)
    DenoiseState* rnnoiseState = nullptr;
    int rnnoiseFrameSize = 0;

    // Scratch buffers reused every process call to avoid allocations
    std::vector<float> rnFrameScratch; // per-frame temporary buffer
    std::vector<float> rnTailScratch;  // remainder frame buffer (zero-padded)

    // === Guitar-only spectral separator (HPSS-like) ===
    void initGuitarSeparationDSP();
    void resetGuitarSeparationDSP();
    void processGuitarStream(const float* in, float* out, int numSamples);

    // STFT parameters
    int stftSize = 1024; // FFT size (power of two)
    int hopSize = 512; //50% overlap for Hann COLA
    int fftOrder = 10; // log2(stftSize)

    // Windows and FFT
    std::vector<float> analysisWindow;
    std::vector<float> synthesisWindow;
    std::vector<float> fftBuffer; // size =2 * stftSize (JUCE real-only format)
    std::unique_ptr<dsp::FFT> fft;

    // Streaming buffers
    std::vector<float> inFifo; // size = stftSize
    int inFifoIndex = 0; // number of new samples in FIFO

    std::vector<float> olaBuffer; // overlap-add buffer size = stftSize
    int olaWritePos = 0; // position within hop for writing out

    std::vector<float> outQueue; // produced samples waiting to be read

    // Magnitude history for time median smoothing
    std::deque<std::vector<float>> magHistory; // each frame has N/2+1 mags

    // Tunables
    int timeSmoothFrames = 7; // median window length across time
    int freqSmoothBins = 5; // half-width (bins) for freq median
    float maskExponent = 2.0f; // Wiener mask exponent
    float maskThreshold = 0.55f; // gate to remove non-guitar bins

    // Guitar spectral emphasis (per-bin weights)
    std::vector<float> guitarWeight; // size = bins
    void updateGuitarFreqWeight();

    // Helper to compute smoothed magnitudes and masks
    void computeHarmonicPercussiveMasks(const std::vector<float>& curMag,
                                         std::vector<float>& harmMask);

    CriticalSection readLock;

    AudioSource* source = nullptr;
    double sampleRate = 0;
    int bufferSize = 0;

    CriticalSection callbackLock;
    int blockSize = 128, readAheadBufferSize = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (guitarSeparator)
};
