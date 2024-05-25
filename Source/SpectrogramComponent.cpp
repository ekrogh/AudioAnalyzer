/*
  ==============================================================================

	SpectrogramComponent.cpp
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/
#include "FFTCtrl.h"
#include <chrono>
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

	resetVariables();

	// For testing purposes
#ifdef LOG_EXECUTION_TIMES
	auto drawLogFile = juce::File::getCurrentWorkingDirectory().getChildFile("drawLog.txt");
	auto drawFileLogger = std::make_unique<juce::FileLogger>(drawLogFile, "drawApplication Log");
	juce::Logger::setCurrentLogger(drawFileLogger.get());
	auto drawRes = static_cast<double>(drawDurations) / static_cast<double>(drawDurationCounts);
	juce::Logger::writeToLog("drawDuration: "
		+ std::to_string(drawRes)
		+ " microseconds");
	juce::Logger::writeToLog("drawDuration: "
		+ std::to_string(drawRes / 1000.0f)
		+ " miliseconds");
	juce::Logger::writeToLog("drawDuration: "
		+ std::to_string(drawRes / (1000.0f * 1000.0f))
		+ " seconds");

	auto readAndFFTLogFile = juce::File::getCurrentWorkingDirectory().getChildFile("readAndFFTLog.txt");
	auto readAndFFTfileLogger = std::make_unique<juce::FileLogger>(readAndFFTLogFile, "readAndFFTApplication Log");
	auto readFFTRes = static_cast<double>(readAndFFTDurations) / static_cast<double>(readAndFFTDurationCounts);
	juce::Logger::setCurrentLogger(readAndFFTfileLogger.get()); juce::Logger::writeToLog("readAndFFTDuration: "
		+ std::to_string(readFFTRes)
		+ " microseconds");
	juce::Logger::setCurrentLogger(readAndFFTfileLogger.get()); juce::Logger::writeToLog("readAndFFTDuration: "
		+ std::to_string(readFFTRes / 1000.0f)
		+ " miliseconds");
	juce::Logger::setCurrentLogger(readAndFFTfileLogger.get()); juce::Logger::writeToLog("readAndFFTDuration: "
		+ std::to_string(readFFTRes / (1000.0f * 1000.0f))
		+ " seconds");
#endif
	// For testing purposes

	setAudioChannels(1, 2);

	thisIsAudioFile = false;
	audioFileReadRunning = false;

	curTimerFrequencyHz = 60;
	startTimerHz(curTimerFrequencyHz);

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

		pFFTCtrl->updateSampleRate(curSampleRate);

		sizeToUseInFreqInRealTimeFftChartPlot
			= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

		// Prepare cartesian plot
		initRealTimeFftChartPlot();

		thisIsAudioFile = true;
		audioFileReadRunning = true;

		// For testing purposes
#ifdef LOG_EXECUTION_TIMES
		drawDurations = 0;
		drawDurationCounts = 0;
		readAndFFTDurations = 0;
		readAndFFTDurationCounts = 0;
#endif
		// For testing purposes

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

void SpectrogramComponent::run()
{
	Task t = readerToFftDataCopy();  // Create a Task that suspends itself 5 times

	do
	{
		// For testing purposes
#ifdef LOG_EXECUTION_TIMES
		auto start = std::chrono::high_resolution_clock::now();
#endif
		// For testing purposes
		audioFileReadRunning = !t.resume();

		fftDataInBufferIndex ^= 1; // Toggle
		fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];
		// For testing purposes
#ifdef LOG_EXECUTION_TIMES
		auto end = std::chrono::high_resolution_clock::now();
		readAndFFTDurations += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		readAndFFTDurationCounts++;
#endif
		// For testing purposes

	}
	while (weSpectrumDataReady.wait(500) && !threadShouldExit() && audioFileReadRunning);

	doSwitchToMicrophoneInput = (!audioFileReadRunning) && autoSwitchToInput;
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
	else if (thisIsAudioFile)
	{
		weSpectrumDataReady.signal(); // Task can prepare next buffer of FFT data

		// For testing purposes
#ifdef LOG_EXECUTION_TIMES
		auto start = std::chrono::high_resolution_clock::now();
#endif
		// For testing purposes
		drawNextLineOfSpectrogramAndFftPlotUpdate(fftDataOutBuffer, fftSize);
		repaint();
		// For testing purposes
#ifdef LOG_EXECUTION_TIMES
		auto end = std::chrono::high_resolution_clock::now();
		drawDurations += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		drawDurationCounts++;
#endif
		// For testing purposes

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


void SpectrogramComponent::prepareToPlay
(
	int samplesPerBlockExpected, double newSampleRate
)
{
	curSampleRate = newSampleRate;

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

	fillRTChartPlotFrequencyValues();

	pFFTCtrl->updateSampleRate(curSampleRate);
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
	juce::dsp::WindowingFunction<float> theHannWindow
	(
		fftSize
		,
		juce::dsp::WindowingFunction<float>::WindowingMethod::hann
	);

	theHannWindow.multiplyWithWindowingTable
	(
		fftDataBuffer
		,
		fftSize
	);

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
				theNotchFilter =
					std::make_unique<NotchFilter>(50.0f, curSampleRate, curNumInputChannels);
				break;
			}
		case filter60Hz:
			{
				theNotchFilter =
					std::make_unique<NotchFilter>(60.0f, curSampleRate, curNumInputChannels);
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

void SpectrogramComponent::registerFFTCtrl
(
	std::shared_ptr<FFTCtrl> PFFTC
)
{
	pFFTCtrl = PFFTC;
}