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

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"


//==============================================================================
class SpectrogramComponent
	: juce::Component
{
public:
	SpectrogramComponent
	(
		AudioFormatManager& FM
		, juce::Image JM
	)
		: spectrogramImage(JM)
		, formatManager(FM)
	{
		setOpaque(true);

		forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

		formatManager.registerBasicFormats();

		//setAudioChannels(2, 2);

		//startTimerHz(60);
	}

	bool loadURLIntoSpectrum(const URL& theUrl)
	{
		if (!(theUrl.isEmpty()))
		{

			const auto source = makeInputSource(theUrl);

			if (source == nullptr)
				return false;

			auto stream = rawToUniquePtr(source->createInputStream());

			if (stream == nullptr)
				return false;

			reader = rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

			if (reader == nullptr)
				return false;

			AudioBuffer<float> theAudioBuffer =
				AudioBuffer<float>::AudioBuffer
				(
					reader->numChannels
					,
					fftSize
				);

			juce::int64 readerLngth = reader->lengthInSamples;
			juce::int64 readerStartSample = 0;

			for
				(
					juce::int64 readerStartSample = 0
					; readerStartSample < readerLngth
					; readerStartSample += fftSize
					)
			{
				reader->read
				(
					&theAudioBuffer
					,
					0
					,
					fftSize
					,
					readerStartSample
					,
					true
					,
					true
				);


				for (auto sampleNbr = 0; sampleNbr < theAudioBuffer.getNumSamples(); sampleNbr++)
				{
					fftData[sampleNbr] = 0.0f;
					for (auto channelNbr = 0; channelNbr < theAudioBuffer.getNumChannels(); channelNbr++)
					{
						fftData[sampleNbr] += theAudioBuffer.getSample(channelNbr, sampleNbr);
					}
				}

				drawNextLineOfSpectrogram();
			}
		}

		return true;
	}


	void drawNextLineOfSpectrogram()
	{
		auto rightHandEdge = spectrogramImage.getWidth() - 1;
		auto imageHeight = spectrogramImage.getHeight();

		// first, shuffle our image leftwards by 1 pixel..
		spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

		// Window the data
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

	//~SpectrogramComponent()
	//{
	//	//transportSource.setSource(nullptr);
	//	//audioSourcePlayer.setSource(nullptr);
	//	//setSource(nullptr);
	//	//deviceManager.removeAudioCallback(&audioSourcePlayer);
	//	//deviceManager.removeAudioCallback(this);
	//}

	void paint(juce::Graphics& g)
	{
		g.fillAll(juce::Colours::black);

		g.setOpacity(1.0f);
		g.drawImage(spectrogramImage, getLocalBounds().toFloat());
	}

private:
	juce::Image& spectrogramImage;
	std::unique_ptr<juce::dsp::FFT> forwardFFT = std::make_unique<dsp::FFT>(fftOrder);


	float fifo[fftSize];
	float fftData[2 * fftSize] = { 0 };
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	AudioFormatManager& formatManager;
	std::unique_ptr<AudioFormatReader> reader;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};


//==============================================================================
class FFTModule final
	: public Component
{
public:
	FFTModule
	(
		std::shared_ptr<AudioDeviceManager> SADM
		,
		std::shared_ptr<freqPlotModule> FPM
	)
		: module_freqPlot(FPM)
		, deviceManager(*SADM)
	{
		spectrogramCmpnt = std::make_unique<SpectrogramComponent>(formatManager, spe);

		setOpaque(true);

		forwardFFT
			= std::make_unique<dsp::FFT>(fftOrder);

		formatManager.registerBasicFormats();

		setSize(700, 500);
	}

	~FFTModule() override
	{
		transportSource.setSource(nullptr);
		audioSourcePlayer.setSource(nullptr);
		deviceManager.removeAudioCallback(&audioSourcePlayer);
	}



	//===================
	// FFTModule
	//===================

	void paint(Graphics& g) override
	{
		g.fillAll(Colours::black);

		g.setOpacity(1.0f);
	}

	void selectFile
	(
		double maxFreq
		,
		juce::TextEditor* fftOrder__textEditor
		,
		juce::TextEditor* Nbr_Samples__textEditor
		,
		juce::Label* fftSizeNbr__label
		,
		juce::TextEditor* Sample_Freq__textEditor
	)
	{
		chooser.launchAsync
		(
			FileBrowserComponent::openMode
			|
			FileBrowserComponent::canSelectFiles
			,
			[
				this
					,
					maxFreq
					,
					fftOrder__textEditor
					,
					Nbr_Samples__textEditor
					,
					fftSizeNbr__label
					,
					Sample_Freq__textEditor
			]
			(const FileChooser& fc) /*mutable*/
		{
			if (fc.getURLResults().size() > 0)
			{
				auto u = fc.getURLResult();

				handleAudioResource
				(
					std::move(u)
					,
					maxFreq
					,
					fftOrder__textEditor
					,
					Nbr_Samples__textEditor
					,
					fftSizeNbr__label
					,
					Sample_Freq__textEditor
				);
			}
		}
		);
	}

	void handleAudioResource
	(
		URL resource
		,
		double maxFreq
		,
		juce::TextEditor* fftOrder__textEditor
		,
		juce::TextEditor* Nbr_Samples__textEditor
		,
		juce::Label* fftSizeNbr__label
		,
		juce::TextEditor* Sample_Freq__textEditor
	)
	{
		if
			(
				!loadURLIntoFFT
				(
					resource
					,
					maxFreq
					,
					fftOrder__textEditor
					,
					Nbr_Samples__textEditor
					,
					fftSizeNbr__label
					,
					Sample_Freq__textEditor
				)
				)
		{
			// Failed to load the audio file!
			jassertfalse;
			return;
		}

		currentAudioFile = std::move(resource);
	}

	bool loadURLIntoFFT
	(
		double maxFreq
		,
		juce::TextEditor* fftOrder__textEditor
		,
		juce::TextEditor* Nbr_Samples__textEditor
		,
		juce::Label* fftSizeNbr__label
		,
		juce::TextEditor* Sample_Freq__textEditor
	)
	{
		if (currentAudioFile != URL())
		{
			return loadURLIntoFFT
			(
				currentAudioFile
				,
				maxFreq
				,
				fftOrder__textEditor
				,
				Nbr_Samples__textEditor
				,
				fftSizeNbr__label
				,
				Sample_Freq__textEditor
			);
		}
		else
		{
			return false;
		}
	}

	bool loadURLIntoFFT
	(
		const URL& audioURL
		,
		double maxFreq
		,
		juce::TextEditor* fftOrder__textEditor
		,
		juce::TextEditor* Nbr_Samples__textEditor
		,
		juce::Label* fftSizeNbr__label
		,
		juce::TextEditor* Sample_Freq__textEditor
	)
	{
		module_freqPlot->clearPlot();

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
		unsigned int fftOrder = static_cast<unsigned int>(std::log2(bufferLengthInSamples));
		unsigned int fftSize = 1 << fftOrder;

		showValues
		(
			fftOrder
			,
			fftSize
			,
			static_cast<unsigned int>(reader->sampleRate)
			,
			fftOrder__textEditor
			,
			Nbr_Samples__textEditor
			,
			fftSizeNbr__label
			,
			Sample_Freq__textEditor
		);

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

		std::unique_ptr<dsp::FFT> forwardFFT =
			std::make_unique<dsp::FFT>(fftOrder);
		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = (float)reader->sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
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

		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}


	void openAudioFile()
	{
		chooser.launchAsync
		(
			FileBrowserComponent::openMode
			|
			FileBrowserComponent::canSelectFiles
			,
			[this]
			(const FileChooser& fc) /*mutable*/
			{
				if (fc.getURLResults().size() > 0)
				{
					juce::URL theUrl = fc.getURLResult();

					// Make spectrum plot
					spectrogramCmpnt->loadURLIntoSpectrum(theUrl);

					const auto source = makeInputSource(theUrl);
					auto stream = juce::rawToUniquePtr(source->createInputStream());
					auto reader =
						juce::rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

					if (reader.get() != nullptr)
					{
						currentAudioFileSource =
							std::make_unique<AudioFormatReaderSource>(reader.release(), true);

						transportSource.setSource
						(
							//currentAudioFileSource.get()
							//, 0			// tells it to buffer this many samples ahead
							//, nullptr	// this is the background thread to use for reading-ahead
							//, currentAudioFileSource->getAudioFormatReader()->sampleRate
							currentAudioFileSource.get()
							, 0   // tells it to buffer this many samples ahead
							, &thread // this is the background thread to use for reading-ahead
							, currentAudioFileSource->getAudioFormatReader()->sampleRate
						);
						audioSourcePlayer.setSource(&transportSource);

						currentAudioDevice = deviceManager.getCurrentAudioDevice();
						currentAudioDeviceType = deviceManager.getCurrentAudioDeviceType();
						currentDeviceTypeObject = deviceManager.getCurrentDeviceTypeObject();

						deviceManager.addAudioCallback(&audioSourcePlayer);
						transportSource.start();

						thread.startThread(Thread::Priority::normal);
					}
				}
			}
		);

	}

	void switchToMicrophoneInput()
	{
		thread.stopThread(100);
		deviceManager.removeAudioCallback(&audioSourcePlayer);
		transportSource.stop();
		transportSource.setSource(nullptr);
		currentAudioFileSource.reset(nullptr);
	}

	bool makeWhiteNoise
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		double maxFreq
		,
		unsigned int sampleRate
	)
	{
		module_freqPlot->clearPlot();

		const unsigned int fftDataSize = fftSize << 1;

		float* fftData = new float[fftDataSize] { 0 };

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

		std::unique_ptr<dsp::FFT> forwardFFT =
			std::make_unique<dsp::FFT>(fftOrder);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = (float)sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
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

		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	bool makeSines
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		double maxFreq
		,
		unsigned int sampleRate
		,
		std::vector<double>& frequencies
	)
	{
		module_freqPlot->clearPlot();

		const unsigned int fftDataSize = fftSize << 1;

		float* fftData = new float[fftDataSize] { 0 };

		for (double freq : frequencies)
		{
			double currentPhase = 0.0f;
			auto cyclesPerSample = freq / sampleRate;
			double phaseDeltaPerSample =
				cyclesPerSample * juce::MathConstants<double>::twoPi;

			for (size_t i = 0; i < fftSize; i++)
			{
				fftData[i] += static_cast<float>(std::sin(currentPhase));
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

		std::unique_ptr<dsp::FFT> forwardFFT =
			std::make_unique<dsp::FFT>(fftOrder);
		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		auto deltaHz = (float)sampleRate / fftSize;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal <= maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

		auto nbrSamplesInPlot = tmpFreqVctr.size();

		std::vector <std::vector<float>>
			plotValues{ { fftData, fftData + nbrSamplesInPlot } };

		cmp::GraphAttributeList graph_attributes(0);
		makeGraphAttributes(graph_attributes);

		cmp::StringVector plotLegend{ { "p " + std::to_string(plotLegend.size() + 1) } };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	bool makeWindows
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		double maxFreq
		,
		unsigned int sampleRate
	)
	{
		module_freqPlot->clearPlot();

		const unsigned int fftDataSize = fftSize << 1;

		// hann window
		float* fftData = new float[fftDataSize] { 0 };

		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::hann
		);

		std::unique_ptr<dsp::FFT> forwardFFT =
			std::make_unique<dsp::FFT>(fftOrder);

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
		std::vector <std::vector<float>> frequencyValues{ tmpFreqVctr };

		auto nbrSamplesInPlot = tmpFreqVctr.size();

		std::vector <std::vector<float>>
			plotValues{ { fftData, fftData + nbrSamplesInPlot } };

		cmp::GraphAttributeList graph_attributes(0);
		makeGraphAttributes(graph_attributes);

		cmp::StringVector plotLegend{ "hann" };

		// hamming Window
		fftData = new float[fftDataSize] { 0 };

		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::hamming
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		frequencyValues.push_back(tmpFreqVctr);
		plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
		makeGraphAttributes(graph_attributes);
		plotLegend.push_back("hamming");

		// blackman Window
		fftData = new float[fftDataSize] { 0 };
		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::blackman
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		frequencyValues.push_back(tmpFreqVctr);
		plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
		makeGraphAttributes(graph_attributes);
		plotLegend.push_back("blackman");

		// blackmanHarris Window
		fftData = new float[fftDataSize] { 0 };
		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::blackmanHarris
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		frequencyValues.push_back(tmpFreqVctr);
		plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
		makeGraphAttributes(graph_attributes);
		plotLegend.push_back("blackmanHarris");

		// flatTop Window
		fftData = new float[fftDataSize] { 0 };
		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::flatTop
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		frequencyValues.push_back(tmpFreqVctr);
		plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
		makeGraphAttributes(graph_attributes);
		plotLegend.push_back("flatTop");

		// kaiser Window
		fftData = new float[fftDataSize] { 0 };
		juce::dsp::WindowingFunction<float>::fillWindowingTables
		(
			fftData
			,
			fftSize
			,
			juce::dsp::WindowingFunction<float>::kaiser
		);

		// then render our FFT data..
		forwardFFT->performFrequencyOnlyForwardTransform(fftData, true);

		frequencyValues.push_back(tmpFreqVctr);
		plotValues.push_back({ fftData, fftData + nbrSamplesInPlot });
		makeGraphAttributes(graph_attributes);
		plotLegend.push_back("kaiser");

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		return true;
	}

	Random randomRGB = juce::Random::getSystemRandom();
	void makeGraphAttributes(cmp::GraphAttributeList& ga)
	{
		cmp::GraphAttribute colourForLine;
		colourForLine.graph_colour = juce::Colour
		(
			static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
			,
			static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
			,
			static_cast<juce::int8>(randomRGB.nextInt(juce::Range(10, 255)))
		);
		ga.push_back(colourForLine);
	}

	void showValues
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		unsigned int sampleFreq
		,
		juce::TextEditor* fftOrder__textEditor
		,
		juce::TextEditor* Nbr_Samples__textEditor
		,
		juce::Label* fftSizeNbr__label
		,
		juce::TextEditor* Sample_Freq__textEditor
	)
	{
		fftOrder__textEditor->setText(String(fftOrder), true);
		Nbr_Samples__textEditor->setText(String(fftSize), true);
		fftSizeNbr__label->setText(String(fftSize), NotificationType::dontSendNotification);
		Sample_Freq__textEditor->setText(String(sampleFreq));
	}

private:
	AudioDeviceManager& deviceManager;

	AudioIODevice* currentAudioDevice = nullptr;
	String currentAudioDeviceType = String();;
	AudioIODeviceType* currentDeviceTypeObject = nullptr;
	std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;
	TimeSliceThread thread{ "audio file preview" };
	AudioTransportSource transportSource;
	juce::AudioSourcePlayer audioSourcePlayer;

	std::unique_ptr<dsp::FFT> forwardFFT;

	float fifo[fftSize];
	float fftData[2 * fftSize] = { 0 };
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	URL currentAudioFile = URL();
	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReader> reader;
	std::unique_ptr<AudioBuffer<float>> theAudioBuffer;

	std::shared_ptr<freqPlotModule> module_freqPlot;

	std::unique_ptr<SpectrogramComponent> spectrogramCmpnt;

	FileChooser chooser
	{
			"File..."
			,
			File::getSpecialLocation
			(
				juce::File::SpecialLocationType::userMusicDirectory
			)
			.getChildFile("recording.wav")
			,
#if JUCE_ANDROID
				"*.*"
#else
				"*.wav;*.flac;*.aif"
#endif
	};


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTModule)
};




//void pushNextSampleIntoFifo(float sample) noexcept
//{
//	// if the fifo contains enough data, set a flag to say
//	// that the next line should now be rendered..
//	if (fifoIndex == fftSize)
//	{
//		if (!nextFFTBlockReady)
//		{
//			zeromem(fftData, sizeof(fftData));
//			memcpy(fftData, fifo, sizeof(fifo));
//			nextFFTBlockReady = true;
//		}

//		fifoIndex = 0;
//	}

//	fifo[fifoIndex++] = sample;
//}


	//void prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override
	//{
	//	// (nothing to do here)
	//}

	//void releaseResources() override
	//{
	//	// (nothing to do here)
	//}

	//void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
	//{
	//	auto numChans = bufferToFill.buffer->getNumChannels();
	//	auto noSampls = bufferToFill.numSamples;

	//	if (numChans > 0)
	//	{
	//		const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

	//		for (auto i = 0; i < noSampls; ++i)
	//			pushNextSampleIntoFifo(channelData[i]);

	//		bufferToFill.clearActiveBufferRegion();
	//	}
	//}

	//void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
	//	int totalNumInputChannels,
	//	float* const* outputChannelData,
	//	int totalNumOutputChannels,
	//	int numSamples,
	//	const juce::AudioIODeviceCallbackContext& context) override
	//{
	//	if ((totalNumInputChannels > 0) && (numSamples > 0))
	//	{
	//		for (auto i = 0; i < numSamples; ++i)
	//		{
	//			float sum = 0.0f;
	//			for (auto channel = 0; channel < totalNumInputChannels; ++channel)
	//			{
	//				sum += inputChannelData[channel][i];
	//			}
	//			pushNextSampleIntoFifo(sum);
	//		}
	//	}
	//}

	//void audioDeviceAboutToStart(juce::AudioIODevice* device) override
	//{
	//	auto sampleRate = device->getCurrentSampleRate();
	//}

	//void audioDeviceStopped() override
	//{
	//	// (nothing to do here)
	//}

	//void timerCallback() override
	//{
	//	if (nextFFTBlockReady)
	//	{
	//		drawNextLineOfSpectrogram();
	//		nextFFTBlockReady = false;
	//		repaint();
	//	}
	//}
