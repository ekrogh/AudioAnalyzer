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

#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"


//==============================================================================
class FFTModule final : public AudioAppComponent,
	private Timer
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
		,
		spectrogramImage(Image::RGB, 512, 512, true)
	{
		setOpaque(true);

		shutdownAudio();

		//forwardFFT
		//	= std::make_unique<dsp::FFT>(fftOrder);

		formatManager.registerBasicFormats();

		//startTimerHz(60);
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

	void handleAudioResource(URL resource, unsigned int maxFreq)
	{
		if (!loadURLIntoFFT(resource, maxFreq))
		{
			// Failed to load the audio file!
			jassertfalse;
			return;
		}

		currentAudioFile = std::move(resource);
	}

	bool loadURLIntoFFT(const URL& audioURL, unsigned int maxFreq)
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

		juce::int64 bufferLengthInSamples = reader->lengthInSamples;
		unsigned int fftOrder = std::log2(bufferLengthInSamples);
		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);
		unsigned int fftSize = 1 << fftOrder;
		const unsigned int fftDataSize = fftSize << 1;

		float* fftData = new float[fftDataSize] { 0 };

		theAudioBuffer =
			std::make_unique<AudioBuffer<float>>
			(
				reader->numChannels
				,
				reader->lengthInSamples
			);

		reader->read
		(
			theAudioBuffer.get()
			,
			0
			,
			(int)reader->lengthInSamples
			,
			0
			,
			true
			,
			true
		);

		auto rdptrs = theAudioBuffer->getArrayOfReadPointers();

		if (rdptrs[0] != theAudioBuffer->getReadPointer(0))
		{
			auto err = 1;
		}
		memcpy(fftData, rdptrs[0], sizeof(float) * fftSize);

		juce::dsp::WindowingFunction<float> theHannWindow
		(
			fftSize
			,
			juce::dsp::WindowingFunction<float>::WindowingMethod::hann
		);

		theHannWindow.multiplyWithWindowingTable
		(
			fftData
			,
			fftSize
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = reader->sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while(freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		tmpFreqVctr.push_back(freqVal);
		std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

		auto nbrSamplesInPlot = tmpFreqVctr.size();

		std::vector <std::vector<float>>
			plotValues{ { fftData, fftData + nbrSamplesInPlot } };
		
		cmp::GraphAttributeList graph_attributes(0);
		makeGraphAttributes(graph_attributes);

		cmp::StringVector plotLegend(0);
		plotLegend.push_back("p " + std::to_string(plotLegend.size() + 1));

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		module_freqPlot->clearPlot();
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	void selectFile
	(
		unsigned int maxFreq
	)
	{
		chooser.launchAsync
		(
			FileBrowserComponent::openMode
			|
			FileBrowserComponent::canSelectFiles
			,
			[this, maxFreq](const FileChooser& fc) /*mutable*/
			{
				if (fc.getURLResults().size() > 0)
				{
					auto u = fc.getURLResult();

					handleAudioResource(std::move(u), maxFreq);
				}
			}
		);
	}

	bool makeWhiteNoise
	(
		unsigned int nbrSamples
		,
		unsigned int maxFreq
		,
		unsigned int sampleRate
	)
	{
		unsigned int fftOrder = std::log2(nbrSamples);
		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);
		unsigned int fftSize = 1 << fftOrder;
		const unsigned int fftDataSize = fftSize << 1;

		float* fftData = new float[fftDataSize] { 0 };

		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);

		juce::Random random;

		for (size_t i = 0; i < fftSize; i++)
		{
			fftData[i] = random.nextFloat() * 0.25f - 0.125f;
		}

		juce::dsp::WindowingFunction<float> theHannWindow
		(
			fftSize
			,
			juce::dsp::WindowingFunction<float>::WindowingMethod::hann
		);

		theHannWindow.multiplyWithWindowingTable
		(
			fftData
			,
			fftSize
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = (double)sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		tmpFreqVctr.push_back(freqVal);
		std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

		auto nbrSamplesInPlot = tmpFreqVctr.size();

		std::vector <std::vector<float>>
			plotValues{ { fftData, fftData + nbrSamplesInPlot } };

		cmp::GraphAttributeList graph_attributes(0);
		makeGraphAttributes(graph_attributes);

		cmp::StringVector plotLegend{ "p " + std::to_string(plotLegend.size() + 1) };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		module_freqPlot->clearPlot();
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	bool makeSines
	(
		unsigned int nbrSamples
		,
		unsigned int maxFreq
		,
		unsigned int sampleRate
		,
		std::vector<double>& frequencies
	)
	{
		unsigned int fftOrder = std::log2(nbrSamples);
		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);
		unsigned int fftSize = 1 << fftOrder;
		const unsigned int fftDataSize = fftSize << 1;

		float* fftData = new float[fftDataSize] { 0 };

		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);


		for (double freq : frequencies)
		{
			double currentPhase = 0.0f;
			auto cyclesPerSample = freq / sampleRate;
			double phaseDeltaPerSample =
				cyclesPerSample * juce::MathConstants<double>::twoPi;

			for (size_t i = 0; i < fftSize; i++)
			{
				fftData[i] += (float)std::sin(currentPhase);
				currentPhase = std::fmod
				(
					currentPhase + phaseDeltaPerSample
					,
					juce::MathConstants<double>::twoPi
				);
			}
		}

		juce::dsp::WindowingFunction<float> theHannWindow
		(
			fftSize
			,
			juce::dsp::WindowingFunction<float>::WindowingMethod::hann
		);

		theHannWindow.multiplyWithWindowingTable
		(
			fftData
			,
			fftSize
		);

		// then render our FFT data..
		//forwardFFT->performFrequencyOnlyForwardTransform(fftData);
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = (float)sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		tmpFreqVctr.push_back(freqVal);
		std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

		auto nbrSamplesInPlot = tmpFreqVctr.size();

		std::vector <std::vector<float>>
			plotValues{ { fftData, fftData + nbrSamplesInPlot/* + 1*/ } };

		cmp::GraphAttributeList graph_attributes(0);
		makeGraphAttributes(graph_attributes);

		cmp::StringVector plotLegend{ { "p " + std::to_string(plotLegend.size() + 1) } };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		module_freqPlot->clearPlot();
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	void makeGraphAttributes(cmp::GraphAttributeList& ga)
	{
		auto randomRGB = juce::Random::getSystemRandom();
		cmp::GraphAttribute colourForLine;
		colourForLine.graph_colour = juce::Colour
		(
			randomRGB.nextInt(juce::Range(100, 255))
			,
			randomRGB.nextInt(juce::Range(100, 255))
			,
			randomRGB.nextInt(juce::Range(100, 255))
		);
		ga.push_back(colourForLine);
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


	//juce::int64 bufferLengthInSamples = 0;

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

	std::shared_ptr<freqPlotModule> module_freqPlot;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTModule)
};
