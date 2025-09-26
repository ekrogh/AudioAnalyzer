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
guitarSeparator::guitarSeparator()
{
	initializeRNNoise();
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

// The wrapper of rnnoise's |rnnoise_process_frame| function so as to make sure its input/output is |f32| format.
// Note that the frame size is fixed 480.
float guitarSeparator::rnnoise_process(float* pFrameOut, const float* pFrameIn)
{
	// Use reusable scratch buffer
	float* buffer = rnFrameScratch.data();

	// Scale input to int16 range expected internally
	std::transform(
		pFrameIn,
		pFrameIn + rnnoiseFrameSize,
		buffer,
		[](float x)
		{
			return x * 32768.0f;
		});

	const float vadProb = rnnoise_process_frame(rnnoiseState, buffer, buffer);

	// Scale back to [-1, 1]
	std::transform(
		buffer,
		buffer + rnnoiseFrameSize,
		pFrameOut,
		[](float x)
		{
			return eks_clamp(x, -32768.0f, 32767.0f) / 32768.0f;
		});

	return vadProb;
}

void guitarSeparator::getNextAudioBlock(const AudioSourceChannelInfo& info)
{
	const ScopedLock sl(callbackLock);

	auto numChans = info.buffer->getNumChannels();
	auto noSampels = info.numSamples;

	if ((source != nullptr) && (numChans > 0))
	{
		source->getNextAudioBlock(info);
		const auto* channelData = info.buffer->getReadPointer(0, info.startSample);

		// Process audio with RNNoise
		auto channelWritePtr = info.buffer->getWritePointer(0, info.startSample);
		auto iStop = noSampels - rnnoiseFrameSize;
		int i = 0;
		for (; i <= iStop; i += rnnoiseFrameSize)
		{
			rnnoise_process(&channelWritePtr[i], &channelData[i]);
			// Add logging to check the processed data
			DBG("Processed frame starting at sample " << i);
		}

		// Process remaining samples
		if (i < noSampels)
		{
			// Reuse tail scratch buffer
			float* tail = rnTailScratch.data();
			std::fill(tail, tail + rnnoiseFrameSize, 0.0f);
			std::copy(&channelData[i], &channelData[noSampels], tail);

			rnnoise_process(tail, tail);
			std::copy(tail, tail + (noSampels - i), &channelWritePtr[i]);
			DBG("Processed remaining samples starting at sample " << i);
		}

		if (numChans >= 2)
		{
			// Mirror processed channel 0 into channel 1
			info.buffer->copyFrom(1, info.startSample, channelWritePtr, noSampels);
		}

		//for (auto i = 0; i < noSampels; ++i)
		//	pushNextSampleIntoFifo(channelData[i]);

		//info.clearActiveBufferRegion();

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

		if (newSource != nullptr && blockSize > 0 && sampleRate > 0)
			newSource->prepareToPlay(blockSize, sampleRate);

		{
			const ScopedLock sl(readLock);
			source = newSource;
		}

		if (oldSource != nullptr)
			oldSource->releaseResources();
	}
}
