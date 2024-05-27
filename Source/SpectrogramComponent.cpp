/*
  ==============================================================================

	SpectrogramComponent.cpp
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/
#include "NOTCH_50_60_Hz_filter_Coeffs.h"
#include "FFTCtrl.h"
#include "FFTModule.h"
#include "cpEKSNotchFilter.h"
#include "SpectrogramComponent.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>

using namespace NOTCH_50_60_Hz_filter_Coeffs;

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
	, ptrFFTModule(FFTMP)
	, module_freqPlot(FPM)
	, Thread("Audio file read and FFT")
{
	setOpaque(true);

	forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

	initRealTimeFftChartPlot();

	curNumInputChannels = 1;
	setAudioChannels(1, 2);

	curTimerFrequencyHz = 60;
	startTimerHz(curTimerFrequencyHz);

	setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

}

void SpectrogramComponent::switchToMicrophoneInput()
{
	stopTimer();

	// Stop the thread
	signalThreadShouldExit();;
	weSpectrumDataReady.signal();

	showFilters = false;

	resetVariables();

	setAudioChannels(1, 2);

	thisIsNotAudioIOSystem = false;
	notAudioIOSystemIsRunning = false;

	curTimerFrequencyHz = 60;
	startTimerHz(curTimerFrequencyHz);

}

void SpectrogramComponent::switchToNonInput()
{
	stopTimer();

	// Stop the thread
	signalThreadShouldExit();;
	weSpectrumDataReady.signal();

	showFilters = false;

	resetVariables();

	thisIsNotAudioIOSystem = false;
	notAudioIOSystemIsRunning = false;

	curTimerFrequencyHz = 60;

	ptrFFTCtrl->switchUIToSpecialPlots();
	ptrFFTModule->switchToNonInput();
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

void SpectrogramComponent::startShowingFilters()
{

	shutdownAudio();

	stopTimer();

	resetVariables();

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

	// Prepare cartesian plot
	initRealTimeFftChartPlot();

	if (ptrFFTCtrl != nullptr)
	{
		ptrFFTCtrl->updateSampleRate(curSampleRate);
	}

	thisIsNotAudioIOSystem = true;
	notAudioIOSystemIsRunning = true;

	// Start the thread
	startThread();

	startTimerHz(60);

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

		showFilters = false;

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

		setFilterToUse(filterToUse);

		sizeToUseInFreqInRealTimeFftChartPlot
			= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

		// Prepare cartesian plot
		initRealTimeFftChartPlot();

		if (ptrFFTCtrl != nullptr)
		{
			ptrFFTCtrl->updateSampleRate(curSampleRate);
		}

		thisIsNotAudioIOSystem = true;
		notAudioIOSystemIsRunning = true;

		// Start the thread
		startThread();

		auto noSamples = reader->lengthInSamples;
		auto playTime = noSamples / curSampleRate;
		auto noFftBuffers = noSamples / fftSize;
		auto timePerBuffer = playTime / noFftBuffers;

		curTimerInterValMs = static_cast<int>(std::round(timePerBuffer * 1000.0f));

		curTimerFrequencyHz = 1.0 / timePerBuffer;

		startTimer(curTimerInterValMs);
		//startTimerHz(curTimerFrequencyHz);

	}

	return true;
}

SpectrogramComponent::Task SpectrogramComponent::makeFilterPing()
{
	while (showFilters)
	{
		if (filterToUse != noFilter)
		{
			zeromem(fftDataInBuffer, sizeOfFftDataBuffersInBytes);

			auto impulseResponse = theNotchFilter->calculateImpulseResponse(fftSize);

			for (auto i = 0; i < impulseResponse.size(); i++)
			{
				fftDataInBuffer[i] = impulseResponse[i];
			}

			doFFT(fftDataInBuffer, fftSize);

			for (size_t n = 0; n < fftSize; n++)
			{
				if (std::isnan(fftDataInBuffer[n]) || std::isinf(fftDataInBuffer[n]))
				{
					impulseResponse[n] = 0;
				}
			}
		}
		co_await std::suspend_always{};
	}
}

SpectrogramComponent::Task SpectrogramComponent::readerToFftDataCopy()
{
	curNumInputChannels = reader->numChannels;

	auto readerLngth = reader->lengthInSamples;

	for (juce::int64 readerStartSample = 0; readerStartSample < readerLngth; readerStartSample += fftSize)
	{
		theAudioBuffer =
			AudioBuffer<float>::AudioBuffer
			(
				curNumInputChannels
				,
				fftSize
			);
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

		zeromem(fftDataInBuffer, sizeOfFftDataBuffersInBytes);

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

		co_await std::suspend_always{};
	}
}

SpectrogramComponent::Task SpectrogramComponent::setTask()
{
	if (showFilters)
	{
		return makeFilterPing();
	}
	else
	{
		return readerToFftDataCopy();
	}
}

void SpectrogramComponent::run()
{
	Task t = setTask();

	do
	{
		notAudioIOSystemIsRunning = !t.resume();

		fftDataInBufferIndex ^= 1; // Toggle
		fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];
	}
	while (weSpectrumDataReady.wait(500) && !threadShouldExit() && notAudioIOSystemIsRunning);

	doSwitchToMicrophoneInput = (!notAudioIOSystemIsRunning) && autoSwitchToInput;
	doSwitchTNoneInput = !doSwitchToMicrophoneInput;
}

void SpectrogramComponent::timerCallback()
{
	if (nextFFTBlockReady)
	{
		doFFT(fftDataOutBuffer, fftSize);
		drawNextLineOfSpectrogramAndFftPlotUpdate(fftDataOutBuffer, fftSize);
		nextFFTBlockReady = false;
		repaint();
	}
	else if (thisIsNotAudioIOSystem)
	{
		weSpectrumDataReady.signal(); // Task can prepare next buffer of FFT data
		drawNextLineOfSpectrogramAndFftPlotUpdate(fftDataOutBuffer, fftSize);
		repaint();

		fftDataOutBufferIndex ^= 1; // Toggle
		fftDataOutBuffer = fftDataBuffers[fftDataOutBufferIndex];

		thisIsNotAudioIOSystem = notAudioIOSystemIsRunning;
	}
	else if (doSwitchToMicrophoneInput)
	{
		doSwitchToMicrophoneInput = false;
		makespectrumOfInput__toggleButton->
			setToggleState(true, juce::NotificationType::sendNotification);
	}
	else if (doSwitchTNoneInput)
	{
		doSwitchTNoneInput = false;
		switchToNonInput();
	}
}


void SpectrogramComponent::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
	curSampleRate = newSampleRate;

	setFilterToUse(filterToUse);

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

	fillRTChartPlotFrequencyValues();

	if (ptrFFTCtrl != nullptr)
	{
		ptrFFTCtrl->updateSampleRate(curSampleRate);
	}
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
			zeromem(fftDataOutBuffer, sizeOfFftDataBuffersInBytes);
			memcpy(fftDataOutBuffer, fifo, sizeOfFFifoInBytes);

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
	shutDownIO();
}

void SpectrogramComponent::drawNextLineOfSpectrogramAndFftPlotUpdate(float* fftDataBuffer, unsigned int& fftSize)
{
	if (doRealTimeFftChartPlot)
	{
		static bool doneFirst = false;


		//int sizeToUse = fftSize / 100;

		plotValues.clear();
		plotValues.push_back(std::vector<float>
			(fftDataBuffer, fftDataBuffer + sizeToUseInFreqInRealTimeFftChartPlot));
		frequencyValues[0].resize(sizeToUseInFreqInRealTimeFftChartPlot);
		auto pltSize = plotValues[0].size();
		auto tstData = plotValues[0].data();
		auto tstFreq = frequencyValues[0].data();
		for (size_t n = 0; n < plotValues[0].size(); n++)
		{
			if (std::isnan(plotValues[0][n]) || std::isinf(plotValues[0][n]))
			{
				plotValues[0][n] = 0;
			}
		}

		if (doneFirst)
		{
			module_freqPlot->updatePlotRealTime(plotValues, frequencyValues);
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
	auto maxLevel = FloatVectorOperations::findMinAndMax(fftDataBuffer, static_cast<int>(fftSize));

	for (auto y = 1; y < imageHeight; ++y)
	{
		auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, (int)fftSize / 2, (int)(skewedProportionY * (int)fftSize / 2));
		auto level = jmap(fftDataBuffer[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

		//spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSL(level, 1.0f, level + 0.1f, 1.0f));
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level + 0.03f, 1.0f));
	}

}

void SpectrogramComponent::doFFT(float* fftDataBuffer, unsigned int& fftSize)
{
	// Window the data
	juce::dsp::WindowingFunction<float> theKaiserWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::kaiser
	);

	theKaiserWindow.multiplyWithWindowingTable
	(
		fftDataBuffer
		,
		fftSize
	);
	//juce::dsp::WindowingFunction<float> theHannWindow
	//(
	//	fftSize
	//	,
	//	juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	//);

	//theHannWindow.multiplyWithWindowingTable
	//(
	//	fftDataBuffer
	//	,
	//	fftSize
	//);

	forwardFFT->performFrequencyOnlyForwardTransform(fftDataBuffer, true);

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
				theNotchFilter = std::make_unique<cpEKSNotchFilter>(50.0, curSampleRate, curNumInputChannels, 0.1);
				break;
			}
		case filter60Hz:
			{
				theNotchFilter = std::make_unique<cpEKSNotchFilter>(60.0, curSampleRate, curNumInputChannels, 0.1);
				break;
			}
	}

	filterToUse = theFilterType;
}

void SpectrogramComponent::initRealTimeFftChartPlot()
{
	if (doRealTimeFftChartPlot)
	{
		ptrFFTModule->makeGraphAttributes(graph_attributes);
		plotLegend = { "p " + std::to_string(plotLegend.size() + 1) };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		fillRTChartPlotFrequencyValues();

		plotValues = frequencyValues;
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);
	}
}

void SpectrogramComponent::fillRTChartPlotFrequencyValues()
{
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
}

void SpectrogramComponent::setDoRealTimeFftChartPlot(bool doRTFftCP)
{
	shutDownIO();

	doRealTimeFftChartPlot = doRTFftCP;

	initRealTimeFftChartPlot();

	reStartIO();
}

void SpectrogramComponent::setMaxFreqInRealTimeFftChartPlot(double maxFRTFftCP)
{
	shutDownIO();

	maxFreqInRealTimeFftChartPlot = maxFRTFftCP;

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)((maxFreqInRealTimeFftChartPlot / curSampleRate) * fftSize);

	fillRTChartPlotFrequencyValues();

	reStartIO();
}

void SpectrogramComponent::setFftOrderAndFftSize(unsigned int newFftOrder, unsigned int newFftSize)
{
	shutDownIO();

	fftOrder = newFftOrder;
	fftSize = newFftSize;

	fftDataBuffers =
	{
		new float[2 * fftSize]
		,
		new float[2 * fftSize]
	};
	sizeOfFftDataBuffersInBytes = sizeof(float) * fftSize * 2;

	fifo = new float[fftSize] { 0 };
	sizeOfFFifoInBytes = sizeof(float) * fftSize;
	fillRTChartPlotFrequencyValues();

	forwardFFT.reset();
	forwardFFT = std::make_unique<dsp::FFT>(fftOrder);

	resetVariables();

	reStartIO();
}

void SpectrogramComponent::shutDownIO()
{
	shutdownAudio();

	stopTimer();

	// Stop thread
	if (threadWasRunning = isThreadRunning())
	{
		do
		{
			signalThreadShouldExit();

			weSpectrumDataReady.signal();
		}
		while (!waitForThreadToExit(100));
	}

	resetVariables();

}

void SpectrogramComponent::reStartIO()
{
	if (threadWasRunning)
	{
		startThread();
	}
	else
	{
		setAudioChannels(curNumInputChannels, curNumOutputChannels);
	}

	startTimerHz(curTimerFrequencyHz);
}

void SpectrogramComponent::registerFFTCtrl(FFTCtrl* FFTC)
{
	ptrFFTCtrl = FFTC;
	//ptrFFTCtrl = std::unique_ptr<FFTCtrl>(FFTC);
}

void SpectrogramComponent::setShowFilters(bool showF)
{
	showFilters = showF;

	startShowingFilters();
}
