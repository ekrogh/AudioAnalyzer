/*
  ==============================================================================

	SpectrogramComponent.cpp
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/

#include "FFTModule.h"
#include "SpectrogramComponent.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>

//==============================================================================
SpectrogramComponent::SpectrogramComponent
(
	AudioFormatManager& FM
	,
	std::shared_ptr<AudioDeviceManager> SADM
	,
	FFTModule* FFTMP
	,
	std::shared_ptr<freqPlotModule> FPM
)
	: formatManager(FM)
	, AudioAppComponent(*SADM)
	, spectrogramImage(Image::RGB, 600, 626, true)
	, theFftModule(*FFTMP)
	, module_freqPlot(FPM)
	, Thread("Audio file read and FFT")
{
	setOpaque(true);

	forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

	setAudioChannels(1, 2);

	startTimerHz(60);

	setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

}


bool SpectrogramComponent::loadURLIntoSpectrum
(
	const URL& theUrl
	, juce::ToggleButton* spectrumOfaudioFile__toggleButtonPtr
	, juce::ToggleButton* makespectrumOfInput__toggleButtonPtr
)
{
	if (!(theUrl.isEmpty()))
	{
		spectrumOfaudioFile__toggleButton = spectrumOfaudioFile__toggleButtonPtr;
		makespectrumOfInput__toggleButton = makespectrumOfInput__toggleButtonPtr;

		shutdownAudio();

		stopTimer();

		const auto source = makeInputSource(theUrl);

		if (source == nullptr)
			return false;

		auto stream = rawToUniquePtr(source->createInputStream());

		if (stream == nullptr)
			return false;

		reader = rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

		if (reader == nullptr)
			return false;


		resetVariables();

		curSampleRate = reader->sampleRate;

		sizeToUseInFreqInRealTimeFftChartPlot
			= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

		// Prepare cartesian plot
		initRealTimeFftChartPlot();

		thisIsAudioFile = true;
		audioFileReadRunning = true;

		// Start the thread
		startThread();

		auto sRate = curSampleRate;
		auto noSamples = reader->lengthInSamples;
		auto playTime = noSamples / sRate;
		auto noFftBuffers = noSamples / fftSize;
		auto timePerBuffer = playTime / noFftBuffers;
		auto timerFreq = 1.0 / timePerBuffer;

		startTimerHz(timerFreq);

	}

	return true;
}


void SpectrogramComponent::switchToMicrophoneInput()
{
	stopTimer();

	// Stop the thread
	signalThreadShouldExit();;
	weSpectrumDataReady.signal();

	resetVariables();

	setAudioChannels(1, 2);

	thisIsAudioFile = false;
	audioFileReadRunning = false;

	startTimerHz(60);

}

void SpectrogramComponent::resetVariables()
{
	spectrogramImage.clear(spectrogramImage.getBounds());

	fftDataInBufferIndex = 0;  // Index of the buffer currently being read
	fftDataOutBufferIndex = 0;  // Index of the buffer currently being filled

	fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];
	fftDataOutBuffer = fftDataBuffers[fftDataOutBufferIndex];

	fifoIndex = 0;
	nextFFTBlockReady = false;
}


void SpectrogramComponent::timerCallback()
{
	if (nextFFTBlockReady)
	{
		doFFT(fftDataOutBuffer, fftSize);
		drawNextLineOfSpectrogramAndFftPlotUpdate();
		nextFFTBlockReady = false;
		repaint();
	}
	else if (thisIsAudioFile)
	{
		weSpectrumDataReady.signal(); // Task can prepare next buffer of FFT data

		drawNextLineOfSpectrogramAndFftPlotUpdate();
		repaint();

		fftDataOutBufferIndex ^= 1; // Toggle
		fftDataOutBuffer = fftDataBuffers[fftDataOutBufferIndex];

		thisIsAudioFile = audioFileReadRunning;
	}
	else if (doSwitchToMicrophoneInput)
	{
		doSwitchToMicrophoneInput = false;
		makespectrumOfInput__toggleButton->setToggleState(true, juce::NotificationType::sendNotification);
	}
}


void SpectrogramComponent::prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/)
{
	// (nothing to do here)
}


void SpectrogramComponent::releaseResources()
{
	// (nothing to do here)
}


void SpectrogramComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
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


void SpectrogramComponent::pushNextSampleIntoFifo(float sample) noexcept
{
	// if the fifo contains enough data, set a flag to say
	// that the next line should now be rendered..
	if (fifoIndex == fftSize)
	{
		if (!nextFFTBlockReady)
		{
			zeromem(fftDataOutBuffer, sizeof(fftDataBuffers[fftDataOutBufferIndex]));
			memcpy(fftDataOutBuffer, fifo, sizeof(fifo));

			nextFFTBlockReady = true;
		}

		fifoIndex = 0;
	}

	fifo[fifoIndex++] = sample;
}


void SpectrogramComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);

	g.setOpacity(1.0f);
	g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}


SpectrogramComponent::~SpectrogramComponent()
{
	shutdownAudio();

	stopTimer();

	// Stop the threads
	//dataThread.stopThread(500);
	//drawThread.stopThread(500);

}


void SpectrogramComponent::drawNextLineOfSpectrogramAndFftPlotUpdate()
{
	if (doRealTimeFftChartPlot)
	{
		static bool doneFirst = false;

		
		//int sizeToUse = fftSize / 100;

		plotValues.clear();
		plotValues.push_back(std::vector<float>
			(fftDataOutBuffer, fftDataOutBuffer + sizeToUseInFreqInRealTimeFftChartPlot));
		frequencyValues[0].resize(sizeToUseInFreqInRealTimeFftChartPlot);
		if (doneFirst)
		{
			module_freqPlot->updatePlotRealTime(plotValues);
		}
		else
		{
			module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);
			doneFirst = true;
		}
	}

	auto rightHandEdge = spectrogramImage.getWidth() - 1;
	auto imageHeight = spectrogramImage.getHeight();

	// first, shuffle our image leftwards by 1 pixel..
	spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

	// find the range of values produced, so we can scale our rendering to
	// show up the detail clearly
	auto maxLevel = FloatVectorOperations::findMinAndMax(fftDataOutBuffer, (int)fftSize);

	for (auto y = 1; y < imageHeight; ++y)
	{
		auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, (int)fftSize / 2, (int)(skewedProportionY * (int)fftSize / 2));
		auto level = jmap(fftDataOutBuffer[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

		//spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSL(level, 1.0f, level + 0.1f, 1.0f));
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level + 0.03f, 1.0f));
	}

}

void SpectrogramComponent::doFFT(float* fftBuffer, auto fftSize)
{
	// Window the data
	juce::dsp::WindowingFunction<float> theHannWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	);

	theHannWindow.multiplyWithWindowingTable
	(
		fftBuffer
		,
		fftSize
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftBuffer, true);
}

SpectrogramComponent::Generator<bool> SpectrogramComponent::readerToFftDataCopy()
{
	while (true)
	{
		theAudioBuffer =
			AudioBuffer<float>::AudioBuffer
			(
				reader->numChannels
				,
				fftSize
			);

		auto readerLngth = reader->lengthInSamples;

		for
			(
				juce::int64 readerStartSample = 0
				;
				readerStartSample < readerLngth
				;
				readerStartSample += fftSize
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

			if (filterToUse != noFilter)
			{
				theNotchFilter->process(theAudioBuffer);
			}

			float theSum = 0.0f;
			for (auto sampleNbr = 0; sampleNbr < theAudioBuffer.getNumSamples(); sampleNbr++)
			{
				theSum = 0.0f;
				for (auto channelNbr = 0; channelNbr < theAudioBuffer.getNumChannels(); channelNbr++)
				{
					theSum += theAudioBuffer.getSample(channelNbr, sampleNbr);
				}
				fftDataInBuffer[sampleNbr] = theSum;
			}

			// Make the FFT
			doFFT(fftDataInBuffer, fftSize);

			co_yield true;
		}

		co_yield false;
	}
}

void SpectrogramComponent::run()
{
	do
	{
		audioFileReadRunning = gen();

		fftDataInBufferIndex ^= 1; // Toggle
		fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];

		weSpectrumDataReady.wait(-1); // Wait for event to be signaled.
	}
	while (!threadShouldExit() && audioFileReadRunning);

	doSwitchToMicrophoneInput = (!audioFileReadRunning) && autoSwitchToInput;
}

void SpectrogramComponent::setAutoSwitchToInput(bool autoSwitch)
{
	autoSwitchToInput = autoSwitch;
}

void SpectrogramComponent::setFilterToUse(filterTypes theFilterType)
{
	switch (theFilterType)
	{
		case filter50Hz:
			{
				theNotchFilter =
					std::make_unique<NotchFilter>(50.0f, reader->sampleRate, reader->numChannels);
				break;
			}
		case filter60Hz:
			{
				theNotchFilter =
					std::make_unique<NotchFilter>(60.0f, reader->sampleRate, reader->numChannels);
				break;
			}
	}

	filterToUse = theFilterType;

}

void SpectrogramComponent::initRealTimeFftChartPlot()
{
	if (doRealTimeFftChartPlot)
	{
		theFftModule.makeGraphAttributes(graph_attributes);
		plotLegend = { "p " + std::to_string(plotLegend.size() + 1) };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		auto deltaHz = (float)curSampleRate / fftSize;
		auto maxFreq = maxFreqInRealTimeFftChartPlot;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal < maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		frequencyValues = { tmpFreqVctr };

		plotValues = frequencyValues;
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);
	}
}

void SpectrogramComponent::setDoRealTimeFftChartPlot(bool doRTFftCP)
{
	bool doRealTimeFftChartPlot = doRTFftCP;

	initRealTimeFftChartPlot();
}

void SpectrogramComponent::setMaxFreqInRealTimeFftChartPlot(double maxFRTFftCP)
{
	maxFreqInRealTimeFftChartPlot = maxFRTFftCP;

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)((maxFreqInRealTimeFftChartPlot / curSampleRate) * fftSize);

	initRealTimeFftChartPlot();
}