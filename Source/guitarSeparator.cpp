/*
  ==============================================================================

	guitarSeparator.cpp
	Created: 24 Sep 2025 1:27:21am
	Author:  eigil

  ==============================================================================
*/

#include <JuceHeader.h>
#include "guitarSeparator.h"
#include "eksClamp.h"

//==============================================================================
static void medianSmoothTime(const std::deque<std::vector<float>>& hist, std::vector<float>& out)
{
	if (hist.empty()) return;
	const int T = (int)hist.size();
	const int N = (int)hist.front().size();
	out.assign(N,0.0f);
	std::vector<float> tmp; tmp.reserve((size_t)T);
	for (int k =0; k < N; ++k)
	{
		tmp.clear();
		for (int t =0; t < T; ++t) tmp.push_back(hist[t][k]);
		std::nth_element(tmp.begin(), tmp.begin() + tmp.size() /2, tmp.end());
		out[k] = tmp[tmp.size() /2];
	}
}

static void medianSmoothFreq(const std::vector<float>& in, std::vector<float>& out, int halfWidth)
{
	const int N = (int)in.size();
	out.resize(N);
	std::vector<float> tmp; tmp.reserve((size_t)(2 * halfWidth +1));
	for (int k =0; k < N; ++k)
	{
		const int a = jmax(0, k - halfWidth);
		const int b = jmin(N -1, k + halfWidth);
		tmp.clear();
		for (int i = a; i <= b; ++i) tmp.push_back(in[i]);
		std::nth_element(tmp.begin(), tmp.begin() + tmp.size() /2, tmp.end());
		out[k] = tmp[tmp.size() /2];
	}
}

//==============================================================================
// guitarSeparator
//==============================================================================
guitarSeparator::guitarSeparator()
{
	initializeRNNoise();
	initGuitarSeparationDSP();
}

guitarSeparator::~guitarSeparator()
{
	setSource(nullptr);
	releaseResources();

	// Destroy RNNoise state
	if (rnnoiseState != nullptr)
	{
		rnnoise_destroy(rnnoiseState);
		rnnoiseState = nullptr;
	}
}

void guitarSeparator::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
	const ScopedLock sl(callbackLock);

	sampleRate = newSampleRate;
	blockSize = samplesPerBlockExpected;

	resetGuitarSeparationDSP();
	updateGuitarFreqWeight();

	if (source != nullptr)
		source->prepareToPlay(samplesPerBlockExpected, sampleRate);
}


void guitarSeparator::initializeRNNoise()
{
	rnnoiseState = rnnoise_create(nullptr);
	rnnoiseFrameSize = rnnoise_get_frame_size();

	// Pre-size scratch buffers to avoid per-callback allocations
	rnFrameScratch.resize(rnnoiseFrameSize);
	rnTailScratch.resize(rnnoiseFrameSize);
}

void guitarSeparator::initGuitarSeparationDSP()
{
	// Default params are set in header
	fft = std::make_unique<dsp::FFT>(fftOrder);
	analysisWindow.resize(stftSize);
	synthesisWindow.resize(stftSize);

	// Use JUCE's Hann window
	juce::dsp::WindowingFunction<float>::fillWindowingTables(
		analysisWindow.data(), stftSize,
		juce::dsp::WindowingFunction<float>::hann);
	synthesisWindow = analysisWindow; // COLA-compatible with50% overlap

	fftBuffer.assign(2 * stftSize,0.0f);

	inFifo.assign(stftSize,0.0f);
	inFifoIndex =0;

	olaBuffer.assign(stftSize,0.0f);
	olaWritePos =0;

	outQueue.clear();

	magHistory.clear();

	updateGuitarFreqWeight();
}

void guitarSeparator::resetGuitarSeparationDSP()
{
	const ScopedLock sl(callbackLock);
	initGuitarSeparationDSP();
}

// Build a per-bin weight that emphasizes typical guitar fundamentals + overtones (~80–5kHz)
void guitarSeparator::updateGuitarFreqWeight()
{
	const int bins = stftSize /2 +1;
	guitarWeight.assign(bins,1.0f);
	if (sampleRate <=0.0) return;

	const float binHz = (float)sampleRate / (float)stftSize;
	const float low =80.0f;
	const float mid1 =1200.0f;
	const float mid2 =5000.0f;
	const float high =8000.0f; // de-emphasize above this

	for (int k =0; k < bins; ++k)
	{
		const float f = k * binHz;
		float w =0.6f; // base
		if (f >= low && f <= mid1) w =1.4f; // strong boost in fundamental/low mids
		else if (f > mid1 && f <= mid2) w =1.2f; // moderate boost in upper mids
		else if (f > mid2 && f <= high) w =0.9f; // slight cut
		else if (f > high) w =0.6f; // more cut at very high freq
		guitarWeight[k] = w;
	}
}

// Median HPSS + Wiener mask + guitar frequency weighting
void guitarSeparator::computeHarmonicPercussiveMasks(const std::vector<float>& curMag,
	std::vector<float>& harmMask)
{
	// Time median
	std::vector<float> timeMed;
	if (!magHistory.empty())
		medianSmoothTime(magHistory, timeMed);
	else
		timeMed = curMag;

	// Frequency median
	std::vector<float> freqMed;
	medianSmoothFreq(curMag, freqMed, freqSmoothBins);

	// Emphasize guitar band in the harmonic track
	if (guitarWeight.size() == curMag.size())
		for (size_t k =0; k < timeMed.size(); ++k)
			timeMed[k] *= guitarWeight[k];

	// Compute Wiener-style harmonic mask
	harmMask.resize(curMag.size());
	for (size_t k =0; k < curMag.size(); ++k)
	{
		const float H = std::pow(timeMed[k], maskExponent) +1.0e-12f;
		const float P = std::pow(freqMed[k], maskExponent) +1.0e-12f;
		float w = H / (H + P);
		// Gate to remove bins that are not confidently harmonic
		if (w < maskThreshold) w =0.0f;
		harmMask[k] = w;
	}
}

// Stream processing using STFT + harmonic mask + OLA
void guitarSeparator::processGuitarStream(const float* in, float* out, int numSamples)
{
	outQueue.reserve((size_t)numSamples + outQueue.size());

	int pos =0;
	while (pos < numSamples)
	{
		const int canCopy = jmin(numSamples - pos, stftSize - inFifoIndex);
		memcpy(inFifo.data() + inFifoIndex, in + pos, (size_t)canCopy * sizeof(float));
		inFifoIndex += canCopy;
		pos += canCopy;

		if (inFifoIndex >= stftSize) // have a full frame
		{
			// Windowed frame -> fftBuffer (real-only FFT format)
			for (int n =0; n < stftSize; ++n)
				fftBuffer[n] = inFifo[n] * analysisWindow[n];
			std::fill(fftBuffer.begin() + stftSize, fftBuffer.begin() +2 * stftSize,0.0f);

			// Forward FFT
			fft->performRealOnlyForwardTransform(fftBuffer.data());

			// Magnitude spectrum
			const int bins = stftSize /2 +1;
			std::vector<float> mag(bins,0.0f);
			mag[0] = std::abs(fftBuffer[0]);
			for (int k =1; k < bins -1; ++k)
			{
				const float re = fftBuffer[2 * k];
				const float im = fftBuffer[2 * k +1];
				mag[k] = std::sqrt(re * re + im * im);
			}
			mag[bins -1] = std::abs(fftBuffer[1]);

			// Compute harmonic mask
			std::vector<float> harmMask;
			computeHarmonicPercussiveMasks(mag, harmMask);

			// Apply mask (keep phase from original)
			fftBuffer[0] *= harmMask[0];
			for (int k =1; k < bins -1; ++k)
			{
				float& re = fftBuffer[2 * k];
				float& im = fftBuffer[2 * k +1];
				re *= harmMask[k];
				im *= harmMask[k];
			}
			fftBuffer[1] *= harmMask[bins -1];

			// Inverse FFT
			fft->performRealOnlyInverseTransform(fftBuffer.data());

			// Overlap-add with synthesis window
			for (int n =0; n < stftSize; ++n)
			{
				const float v = fftBuffer[n] * synthesisWindow[n] / (float)stftSize; // scale iFFT
				olaBuffer[n] += v;
			}

			// Push hopSize samples to outQueue
			outQueue.insert(outQueue.end(), olaBuffer.begin(), olaBuffer.begin() + hopSize);

			// Shift olaBuffer by hop
			std::rotate(olaBuffer.begin(), olaBuffer.begin() + hopSize, olaBuffer.end());
			std::fill(olaBuffer.end() - hopSize, olaBuffer.end(),0.0f);

			// Update mag history (time smoothing) with current frame at back
			magHistory.push_back(std::move(mag));
			if ((int)magHistory.size() > timeSmoothFrames)
				magHistory.pop_front();

			// Shift input FIFO by hop
			std::memmove(inFifo.data(), inFifo.data() + hopSize, (size_t)(stftSize - hopSize) * sizeof(float));
			inFifoIndex = stftSize - hopSize;
		}
	}

	// Pop as many as requested
	const int canOut = jmin(numSamples, (int)outQueue.size());
	if (canOut >0)
	{
		memcpy(out, outQueue.data(), (size_t)canOut * sizeof(float));
		outQueue.erase(outQueue.begin(), outQueue.begin() + canOut);
	}
	if (canOut < numSamples)
	{
		// zero-pad if not enough
		zeromem(out + canOut, (size_t)(numSamples - canOut) * sizeof(float));
	}

	// Clamp output to [-1,1] like the original rnnoise_process path
	for (int i =0; i < numSamples; ++i)
		out[i] = eks_clamp(out[i], -1.0f,1.0f);
}

// The wrapper signature is preserved; we route to the guitar separator.
float guitarSeparator::rnnoise_process(float* pFrameOut, const float* pFrameIn)
{
	processGuitarStream(pFrameIn, pFrameOut, rnnoiseFrameSize);
	// Return a dummy VAD probability (not used). Could compute RMS-based activity if needed.
	return 1.0f;
}

void guitarSeparator::getNextAudioBlock(const AudioSourceChannelInfo& info)
{
	const ScopedLock sl(callbackLock);

	const int numChans = info.buffer->getNumChannels();
	const int numSamples = info.numSamples;

	if ((source != nullptr) && (numChans >0))
	{
		source->getNextAudioBlock(info);

		auto* readPtr = info.buffer->getReadPointer(0, info.startSample);
		auto* writePtr = info.buffer->getWritePointer(0, info.startSample);

		processGuitarStream(readPtr, writePtr, numSamples);

		if (numChans >=2)
			info.buffer->copyFrom(1, info.startSample, writePtr, numSamples);
	}
	else
	{
		info.clearActiveBufferRegion();
	}
}

void guitarSeparator::releaseResources()
{
	const ScopedLock sl(callbackLock);
	if (source != nullptr)
		source->releaseResources();
}

void guitarSeparator::setSource(AudioSource* newSource)
{
	if (source != newSource)
	{
		auto* oldSource = source;

		if (newSource != nullptr && blockSize >0 && sampleRate >0)
			newSource->prepareToPlay(blockSize, sampleRate);

		{
			const ScopedLock sl(readLock);
			source = newSource;
		}

		if (oldSource != nullptr)
			oldSource->releaseResources();
	}
}
