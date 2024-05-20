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
{
	setOpaque(true);

	forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

	setAudioChannels(1, 2);

	startTimerHz(60);

	setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

	// Add this instance to the TimeSliceThreads
	dataThread.addTimeSliceClient(this);
	drawThread.addTimeSliceClient(this);

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

		// Prepare cartesian plot
		theFftModule.makeGraphAttributes(graph_attributes);
		plotLegend = { "p " + std::to_string(plotLegend.size() + 1) };

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		auto deltaHz = (float)reader->sampleRate / fftSize;
		auto maxFreq = reader->sampleRate;

		std::vector<float> tmpFreqVctr(0);
		float freqVal = 0.0f;
		while (freqVal < maxFreq)
		{
			tmpFreqVctr.push_back(freqVal);
			freqVal += deltaHz;
		}
		tmpFreqVctr.resize(fftSize / 100);
		frequencyValues = { tmpFreqVctr };

		plotValues = frequencyValues;
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		//doRealTimeChartPlot = true;

		thisIsAudioFile = true;
		audioFileReadRunning = true;

		auto sRate = reader->sampleRate;
		auto noSamples = reader->lengthInSamples;
		auto playTime = noSamples / sRate;
		auto noFftBuffers = noSamples / fftSize;
		auto timePerBuffer = playTime / noFftBuffers;
		auto timerFreq = 1.0 / timePerBuffer;

		startTimerHz(timerFreq);


		// Add this instance to the TimeSliceThreads
		dataThread.addTimeSliceClient(this);
		drawThread.addTimeSliceClient(this);

		// Start the threads
		dataThread.startThread();
		drawThread.startThread();


	}

	return true;
}


void SpectrogramComponent::switchToMicrophoneInput()
{
	stopTimer();

	doRealTimeChartPlot = false;

	// Stop the threads
	dataThread.signalThreadShouldExit();
	releaseAllSemaphores();
	dataThread.stopThread(500);
	dataThread.removeAllClients();

	drawThread.signalThreadShouldExit();
	releaseAllSemaphores();
	drawThread.stopThread(500);
	drawThread.removeAllClients();

	resetVariables();

	setAudioChannels(1, 2);

	thisIsAudioFile = false;
	audioFileReadRunning = false;

	startTimerHz(60);

}


void SpectrogramComponent::releaseAllSemaphores()
{
	timerSemaphore.release();
	readyToReadSemaphore[0].release(); // Set to 1
	readyToReadSemaphore[1].release(); // Set to 1
	readyToWriteSemaphore[0].release(); // Set to 1
	readyToWriteSemaphore[1].release(); // Set to 1
}

void SpectrogramComponent::resetVariables()
{
	spectrogramImage.clear(spectrogramImage.getBounds());
	while (readyToReadSemaphore[0].try_acquire()) {};  // Remove all
	while (readyToReadSemaphore[1].try_acquire()) {};  // Remove all
	while (readyToWriteSemaphore[0].try_acquire()) {};  // Remove all
	while (readyToWriteSemaphore[1].try_acquire()) {};  // Remove all
	readyToWriteSemaphore[0].release(); // Set to 1
	readyToWriteSemaphore[1].release(); // Set to 1
	readBufferIndex = 0;  // Index of the buffer currently being read
	writeBufferIndex = 1;  // Index of the buffer currently being filled
	while (timerSemaphore.try_acquire()) {};  // Remove all
	fftDataDraw = fftDataBuffers[readBufferIndex];
	fifoIndex = 0;
	nextFFTBlockReady = false;
}


void SpectrogramComponent::timerCallback()
{
	if (doSwitchToMicrophoneInput)
	{
		doSwitchToMicrophoneInput = false;
		makespectrumOfInput__toggleButton->setToggleState(true, juce::NotificationType::sendNotification);
		//spectrumOfaudioFile__toggleButton->setToggleState(false, juce::NotificationType::dontSendNotification);
		//switchToMicrophoneInput();
	}
	else if (thisIsAudioFile)
	{
		timerSemaphore.release();  // Release the semaphore
	}
	else if (nextFFTBlockReady)
	{
		drawNextLineOfSpectrogram();
		nextFFTBlockReady = false;
		repaint();
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
			zeromem(fftDataDraw, sizeof(fftDataDraw));
			memcpy(fftDataDraw, fifo, sizeof(fifo));
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


void SpectrogramComponent::drawNextLineOfSpectrogram()
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
		fftDataDraw
		,
		fftSize
	);

	// then render our FFT data..
	forwardFFT->performFrequencyOnlyForwardTransform(fftDataDraw, true);

	if (doRealTimeChartPlot)
	{
		static bool doneFirst = false;

		int sizeToUse = fftSize;
		//int sizeToUse = fftSize / 100;

		plotValues.clear();
		plotValues.push_back(std::vector<float>(fftDataDraw, fftDataDraw + sizeToUse));
		frequencyValues[0].resize(sizeToUse);
		if (doneFirst)
		{
			module_freqPlot->updatePlotRealTime(plotValues);
			//module_freqPlot->updatePlotRealTime(plotValues, frequencyValues);
		}
		else
		{
			module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);
			doneFirst = true;
		}
	}

	// find the range of values produced, so we can scale our rendering to
	// show up the detail clearly
	auto maxLevel = FloatVectorOperations::findMinAndMax(fftDataDraw, (int)fftSize);

	for (auto y = 1; y < imageHeight; ++y)
	{
		auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, (int)fftSize / 2, (int)(skewedProportionY * (int)fftSize / 2));
		auto level = jmap(fftDataDraw[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

		//spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSL(level, 1.0f, level + 0.1f, 1.0f));
		spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, level + 0.03f, 1.0f));
	}

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

			if (filterToUseChanged)
			{
				switch (filterToUse)
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
				filterToUseChanged = false;
			}

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
				fftDataWrite[sampleNbr] = theSum;
			}

			co_yield true;
		}

		co_yield false;
	}
}

int SpectrogramComponent::useTimeSlice()
{
	auto currentThreadId = juce::Thread::getCurrentThreadId();

	if ((currentThreadId == dataThread.getThreadId()))
	{
		if (audioFileReadRunning)
		{
			// This is the data thread
			if (!(readyToWriteSemaphore[writeBufferIndex]
				  .try_acquire_for(std::chrono::milliseconds(200))))
			{
				if (juce::Thread::currentThreadShouldExit())
				{
					return -1;
				}
				return -1;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return -1;
			}

			//const juce::ScopedLock lock(criticalSections[writeBufferIndex]);  // Lock the critical section for the write buffer

			audioFileReadRunning = gen();

			readyToReadSemaphore[writeBufferIndex].release();  // Signal the draw thread that data is ready

			// switch the write buffer
			writeBufferIndex ^= 1;  // Toggle the write buffer index
			fftDataWrite = fftDataBuffers[writeBufferIndex];

			return 0;
		}
		else
		{
			return -1; // "Stop" thread
		}
	}
	else if (currentThreadId == drawThread.getThreadId())
	{
		if (timerSemaphore.try_acquire())  // Try to acquire the semaphore
		{
			// This is the draw thread
			if (!(readyToReadSemaphore[readBufferIndex]
				  .try_acquire_for(std::chrono::milliseconds(200))))
			{
				if (juce::Thread::currentThreadShouldExit())
				{
					return -1;
				}
				if (autoSwitchToInput)
				{
					doSwitchToMicrophoneInput = true;
				}
				return -1;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return -1;
			}

			//const juce::ScopedLock lock(criticalSections[readBufferIndex]);  // Lock the critical section for the read buffer

			drawNextLineOfSpectrogram();

			MessageManager::callAsync([this]() { repaint(); });

			readyToWriteSemaphore[readBufferIndex].release();  // Signal the data thread that it can start writing

			// switch the read buffer
			readBufferIndex ^= 1;  // Toggle the read buffer index
			fftDataDraw = fftDataBuffers[readBufferIndex];
		}
		return 0;
	}

}

void SpectrogramComponent::setAutoSwitchToInput(bool autoSwitch)
{
	autoSwitchToInput = autoSwitch;
}

void SpectrogramComponent::setFilterToUse(filterTypes theFilterType)
{
	filterToUse = theFilterType;
	filterToUseChanged = true;
}