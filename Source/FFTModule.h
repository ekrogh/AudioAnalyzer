/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             FFTModule
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple FFT application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
				   juce_audio_processors, juce_audio_utils, juce_core,
				   juce_data_structures, juce_dsp, juce_events, juce_graphics,
				   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FFTModule

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "Utilities.h"


//==============================================================================
class FFTModule final : public AudioAppComponent,
	private Timer
{
public:
	FFTModule(std::shared_ptr<AudioDeviceManager> SADM) :
		AudioAppComponent(*SADM),
		spectrogramImage(Image::RGB, 512, 512, true)
	{
		setOpaque(true);

		//#ifndef JUCE_DEMO_RUNNER
		// RuntimePermissions::request (RuntimePermissions::recordAudio,
		//                              [this] (bool granted)
		//                              {
		//                                  int numInputChannels = granted ? 2 : 0;
		//                                  setAudioChannels (numInputChannels, 2);
		//                              });
		//#else
		// setAudioChannels (2, 2);
		//#endif

		formatManager.registerBasicFormats();

		startTimerHz(60);
		setSize(700, 500);
	}

	~FFTModule() override
	{
		shutdownAudio();
	}

	//==============================================================================
	void prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override
	{
		// (nothing to do here)
	}

	void releaseResources() override
	{
		// (nothing to do here)
	}

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		if (bufferToFill.buffer->getNumChannels() > 0)
		{
			const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

			for (auto i = 0; i < bufferToFill.numSamples; ++i)
				pushNextSampleIntoFifo(channelData[i]);

			bufferToFill.clearActiveBufferRegion();
		}
	}

	//==============================================================================
	void paint(Graphics& g) override
	{
		g.fillAll(Colours::black);

		g.setOpacity(1.0f);
		g.drawImage(spectrogramImage, getLocalBounds().toFloat());
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

	void pushNextSampleIntoFifo(float sample) noexcept
	{
		// if the fifo contains enough data, set a flag to say
		// that the next line should now be rendered..
		if (fifoIndex == fftSize)
		{
			if (!nextFFTBlockReady)
			{
				zeromem(fftData, sizeof(fftData));
				memcpy(fftData, fifo, sizeof(fifo));
				nextFFTBlockReady = true;
			}

			fifoIndex = 0;
		}

		fifo[fifoIndex++] = sample;
	}

	void drawNextLineOfSpectrogram()
	{
		auto rightHandEdge = spectrogramImage.getWidth() - 1;
		auto imageHeight = spectrogramImage.getHeight();

		// first, shuffle our image leftwards by 1 pixel..
		spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData);

		// find the range of values produced, so we can scale our rendering to
		// show up the detail clearly
		auto maxLevel = FloatVectorOperations::findMinAndMax(fftData, (int)fftSize / 2);

		for (auto y = 1; y < imageHeight; ++y)
		{
			auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
			auto fftDataIndex = jlimit(0, (int)fftSize / 2, (int)(skewedProportionY * (int)fftSize / 2));
			auto level = jmap(fftData[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

			spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level, 1.0f));
		}
	}

	void handleAudioResource(URL resource)
	{
		if (!loadURLIntoFFT(resource))
		{
			// Failed to load the audio file!
			jassertfalse;
			return;
		}

		currentAudioFile = std::move(resource);
	}

	bool loadURLIntoFFT(const URL& audioURL)
	{
		const auto source = makeInputSource(audioURL);

		if (source == nullptr)
			return false;

		auto stream = rawToUniquePtr(source->createInputStream());

		if (stream == nullptr)
			return false;

		reader = rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

		if (reader == nullptr)
			return false;

		bufferLengthInSamples = reader->lengthInSamples;
		fftOrder = std::log2(bufferLengthInSamples) /*/ std::log2(2)*/;
		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);
		fftSize = 1 << fftOrder;

		fifo = new float[fftSize];
		fftData = new float[2 * fftSize];
		fifoIndex = 0;
		zeromem(fftData, sizeof(fftData));

		theAudioBuffer =
			std::make_unique<AudioBuffer<float>>
			(
				reader->numChannels
				,
				reader->lengthInSamples
			);
		memcpy(fftData, theAudioBuffer->getReadPointer(0), sizeof(fifo));

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		return true;
	}


	void selectFile()
	{
		chooser.launchAsync
		(
			FileBrowserComponent::openMode
			|
			FileBrowserComponent::canSelectFiles
			,
			[this](const FileChooser& fc) /*mutable*/
			{
				if (fc.getURLResults().size() > 0)
				{
					auto u = fc.getURLResult();

					handleAudioResource(std::move(u));
				}
			}
		);
	}


private:
	std::unique_ptr<dsp::FFT> forwardFFT;
	Image spectrogramImage;

	unsigned int fftOrder = 0;
	unsigned int fftSize = 1 << fftOrder;
	float* fifo;
	float* fftData;
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	FileChooser chooser
	{
			"File..."
			,
			File()
			,
#if JUCE_ANDROID
				"*.*"
#else
				"*.wav;*.flac;*.aif"
#endif
	};

	URL currentAudioFile;
	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReader> reader;
	std::unique_ptr<AudioBuffer<float>> theAudioBuffer;
	juce::int64 bufferLengthInSamples = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTModule)
};
