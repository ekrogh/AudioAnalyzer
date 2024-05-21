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
	dataReadWriteThread.addTimeSliceClient(this);
	fftCalculatorThread.addTimeSliceClient(this);
	fftPlotThread.addTimeSliceClient(this);
	spectrumDrawerThread.addTimeSliceClient(this);

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
		dataReadWriteThread.addTimeSliceClient(this);
		fftCalculatorThread.addTimeSliceClient(this);
		fftPlotThread.addTimeSliceClient(this);
		spectrumDrawerThread.addTimeSliceClient(this);

		// Start the threads
		dataReadWriteThread.startThread();
		fftCalculatorThread.startThread();
		fftPlotThread.startThread();
		spectrumDrawerThread.startThread();


	}

	return true;
}


void SpectrogramComponent::switchToMicrophoneInput()
{
	stopTimer();

	doRealTimeFFtPlot = false;

	// Stop the threads
	dataReadWriteThread.signalThreadShouldExit();
	releaseAllSemaphores();
	dataReadWriteThread.stopThread(500);
	dataReadWriteThread.removeAllClients();

	fftCalculatorThread.signalThreadShouldExit();
	releaseAllSemaphores();
	fftCalculatorThread.stopThread(500);
	fftCalculatorThread.removeAllClients();

	fftPlotThread.signalThreadShouldExit();
	releaseAllSemaphores();
	fftPlotThread.stopThread(500);
	fftPlotThread.removeAllClients();

	spectrumDrawerThread.signalThreadShouldExit();
	releaseAllSemaphores();
	spectrumDrawerThread.stopThread(500);
	spectrumDrawerThread.removeAllClients();

	resetVariables();

	setAudioChannels(1, 2);

	thisIsAudioFile = false;
	audioFileReadRunning = false;

	startTimerHz(60);

}


void SpectrogramComponent::releaseAllSemaphores()
{
	for (auto& bfr : dataBuffers)
	{
		bfr.readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread.release();
		bfr.readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread.release();
		bfr.readyToWriteSemaphoreForFftCalculatorThread.release();
		bfr.readyToWriteSemaphoreForFftPlotThread.release();
		bfr.readyToWriteSemaphoreForSpectrumDrawerThread.release();

		bfr.readyToReadSemaphoreForDataReadWriteThread.release();
		bfr.readyToReadSemaphoreForFftCalculatorThread.release();
		bfr.readyToReadSemaphoreForFftPlotThread.release();
		bfr.readyToReadSemaphoreForSpectrumDrawerThread.release();
	}
}

void SpectrogramComponent::resetVariables()
{
	spectrogramImage.clear(spectrogramImage.getBounds());
	for (auto& bfr : dataBuffers)
	{
		while (bfr.readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread.try_acquire()) {};  // Remove all
		while (bfr.readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread.try_acquire()) {};  // Remove all
		while (bfr.readyToWriteSemaphoreForFftCalculatorThread.try_acquire()) {};  // Remove all
		while (bfr.readyToWriteSemaphoreForFftPlotThread.try_acquire()) {};		   // Remove all
		while (bfr.readyToWriteSemaphoreForSpectrumDrawerThread.try_acquire()) {}; // Remove all

		while (bfr.readyToReadSemaphoreForDataReadWriteThread.try_acquire()) {};  // Remove all
		while (bfr.readyToReadSemaphoreForFftCalculatorThread.try_acquire()) {};  // Remove all
		while (bfr.readyToReadSemaphoreForFftPlotThread.try_acquire()) {};        // Remove all
		while (bfr.readyToReadSemaphoreForSpectrumDrawerThread.try_acquire()) {}; // Remove all

		bfr.readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread.release();
		bfr.readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread.release();
		//bfr.readyToWriteSemaphoreForFftCalculatorThread.release();
		//bfr.readyToWriteSemaphoreForFftPlotThread.release();
		//bfr.readyToWriteSemaphoreForSpectrumDrawerThread.release();

	}

	dataReadWriteBufferIndex = 0;
	fftCalculatorBufferIndex = 0;
	fftPlotBufferIndex = 0;
	spectrumPlotBufferIndex = 0;

	while (timerSemaphoreSpectrumPlot.try_acquire()) {}; // Remove all
	while (timerSemaphoreFftPlot.try_acquire()) {};      // Remove all

	dataReadWriteBuffer = dataBuffers[dataReadWriteBufferIndex].dataBuffer;
	fftCalculatorBuffer = dataBuffers[fftCalculatorBufferIndex].dataBuffer;
	fftPlotBuffer = dataBuffers[fftPlotBufferIndex].dataBuffer;
	spectrumPlotBuffer = dataBuffers[spectrumPlotBufferIndex].dataBuffer;

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
		timerSemaphoreSpectrumPlot.release();  // Release the semaphore
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
			zeromem(spectrumPlotBuffer, sizeof(dataBuffers[spectrumPlotBufferIndex].dataBuffer));
			memcpy(spectrumPlotBuffer, fifo, sizeof(fifo));
			if (doRealTimeFFtPlot)
			{
				zeromem(fftPlotBuffer, sizeof(dataBuffers[fftPlotBufferIndex].dataBuffer));
				memcpy(fftPlotBuffer, fifo, sizeof(fifo));
			}

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

	// find the range of values produced, so we can scale our rendering to
	// show up the detail clearly
	auto maxLevel = FloatVectorOperations::findMinAndMax(spectrumPlotBuffer, (int)fftSize);

	for (auto y = 1; y < imageHeight; ++y)
	{
		auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
		auto fftDataIndex = jlimit(0, (int)fftSize / 2, (int)(skewedProportionY * (int)fftSize / 2));
		auto level = jmap(spectrumPlotBuffer[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

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
				dataReadWriteBuffer[sampleNbr] = theSum;
			}

			co_yield true;
		}

		co_yield false;
	}
}

int SpectrogramComponent::useTimeSlice()
{
	auto currentThreadId = juce::Thread::getCurrentThreadId();

	if ((currentThreadId == dataReadWriteThread.getThreadId()))
	{
		if (audioFileReadRunning)
		{
			if (doRealTimeFFtPlot)
			{
				// This is the data thread
				if (!(dataBuffers[dataReadWriteBufferIndex]
					.readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread
					.try_acquire_for(std::chrono::milliseconds(200))))
				{
					if (juce::Thread::currentThreadShouldExit())
					{
						return 10;
					}
					return 10;
				}
			}
			if (!(dataBuffers[dataReadWriteBufferIndex]
				.readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread
				.try_acquire_for(std::chrono::milliseconds(200))))
			{
				if (juce::Thread::currentThreadShouldExit())
				{
					return 10;
				}
				return 10;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return 10;
			}

			try
			{
				audioFileReadRunning = gen();
			}
			catch (const std::exception& ex)
			{
				std::cerr << "Exception: " << ex.what() << '\n';
			}
			catch (...)
			{
				std::cerr << "Unknown exception.\n";
			}

			// Signal the draw thread that data is ready
			dataBuffers[dataReadWriteBufferIndex]
				.readyToWriteSemaphoreForFftCalculatorThread.release();
			dataBuffers[dataReadWriteBufferIndex]
				.readyToReadSemaphoreForFftCalculatorThread.release();

			// switch the write buffer
			++dataReadWriteBufferIndex %= numFftBuffers; // Next buffer next time
			dataReadWriteBuffer = dataBuffers[dataReadWriteBufferIndex].dataBuffer;

			return 0;
		}
		else
		{
			return 10; // "Stop" thread
		}
	}
	else if ((currentThreadId == fftCalculatorThread.getThreadId()))
	{
		if (audioFileReadRunning)
		{
			// This is the data thread
			if (!(dataBuffers[fftCalculatorBufferIndex]
				.readyToWriteSemaphoreForFftCalculatorThread
				.try_acquire_for(std::chrono::milliseconds(200))))
			{
				return 10;
			}
			if (!(dataBuffers[fftCalculatorBufferIndex]
				.readyToReadSemaphoreForFftCalculatorThread
				.try_acquire_for(std::chrono::milliseconds(200))))
			{
				return 10;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return 10;
			}

			// Window the data
			juce::dsp::WindowingFunction<float> theHannWindow
			(
				fftSize
				,
				juce::dsp::WindowingFunction<float>::WindowingMethod::hann
			);

			theHannWindow.multiplyWithWindowingTable
			(
				fftCalculatorBuffer
				,
				fftSize
			);

			// then render our FFT data..
			forwardFFT->performFrequencyOnlyForwardTransform(fftCalculatorBuffer, true);


			// Signal the draw thread that data is ready
			if (doRealTimeFFtPlot)
			{
				dataBuffers[fftCalculatorBufferIndex]
					.readyToReadSemaphoreForFftPlotThread.release();
			}
			dataBuffers[fftCalculatorBufferIndex]
				.readyToReadSemaphoreForSpectrumDrawerThread.release();

			// switch the buffer
			++fftCalculatorBufferIndex %= numFftBuffers; // Next buffer next time
			fftCalculatorBuffer = dataBuffers[fftCalculatorBufferIndex].dataBuffer;

			return 0;
		}
		else
		{
			return 10; // "Stop" thread
		}
	}
	else if ((currentThreadId == fftPlotThread.getThreadId()))
	{
		if (doRealTimeFFtPlot && audioFileReadRunning)
		{
			// This is the data thread
			if (!(dataBuffers[fftPlotBufferIndex]
				.readyToReadSemaphoreForFftPlotThread
				.try_acquire_for(std::chrono::milliseconds(200))))
			{
				return 10;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return 10;
			}

			static bool doneFirst = false;

			int sizeToUse = fftSize;
			//int sizeToUse = fftSize / 100;

			plotValues.clear();
			plotValues.push_back(std::vector<float>(fftPlotBuffer, fftPlotBuffer + sizeToUse));
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

			// Signal the draw thread that data is ready
			dataBuffers[fftPlotBufferIndex]
				.readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread.release();

			// switch the write buffer
			++fftPlotBufferIndex %= numFftBuffers; // Next buffer next time
			fftPlotBuffer = dataBuffers[fftPlotBufferIndex].dataBuffer;

			return 10;
		}
		else
		{
			return 10; // "Stop" thread
		}
	}
	else if (currentThreadId == spectrumDrawerThread.getThreadId())
	{
		if (timerSemaphoreSpectrumPlot.try_acquire())  // Try to acquire the semaphore
		{
			// This is the draw thread
			if (!(dataBuffers[spectrumPlotBufferIndex]
				.readyToReadSemaphoreForSpectrumDrawerThread
				.try_acquire_for(std::chrono::milliseconds(200))))
			{
				if (juce::Thread::currentThreadShouldExit())
				{
					return 10;
				}
				if (autoSwitchToInput)
				{
					doSwitchToMicrophoneInput = true;
				}
				return 10;
			}
			if (juce::Thread::currentThreadShouldExit())
			{
				return 10;
			}

			drawNextLineOfSpectrogram();

			MessageManager::callAsync([this]() { repaint(); });

			// Signal the data thread that it can start writing
			dataBuffers[spectrumPlotBufferIndex]
				.readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread.release();

			// switch the read buffer
			++spectrumPlotBufferIndex %= numFftBuffers; // Next buffer next time
			spectrumPlotBuffer = dataBuffers[spectrumPlotBufferIndex].dataBuffer;
		}
		return 10;
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