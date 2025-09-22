/*
  ==============================================================================

	SpectrogramComponent.cpp
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/
#include "FFTCtrl.h"
#include "FFTModule.h"
#include "SpectrogramComponent.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>
#include "eksClamp.h"
#include <rnnoise.h>
#ifdef RNNOISE_PURE_ONNX
#include <rnnoise_pure_onnx.h>
#endif

#include <atomic>

// Global (file-scope) probability state to avoid modifying header
static std::atomic<float> g_rnnoiseActivityProb {0.0f};
static float rnnoiseResidualProb = 0.0f; // kept for backward compatibility with earlier edits

// Helper: choose default ONNX model filename
static juce::File findDefaultOnnxModel() {
	// Look next to executable, then working directory fallback
	auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
	juce::File f = exeDir.getChildFile("model.onnx");
	if (f.existsAsFile()) return f;
	juce::File cwd = juce::File::getCurrentWorkingDirectory().getChildFile("model.onnx");
	if (cwd.existsAsFile()) return cwd;
	return juce::File();
}

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
	, sharedAudioDeviceManager(SADM)
{
	setOpaque(true);

	forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

	// RNNoise / ONNX accumulation buffers
	rnnoiseFrameSize = rnnoise_get_frame_size();
	rnnoiseInAccum.reserve(rnnoiseFrameSize * 2);
	rnnoiseOutAccum.reserve(rnnoiseFrameSize * 2);
	rnnoiseInAccum.clear();
	rnnoiseOutAccum.clear();
	rnnoiseResidualProb = 0.f;

	// Attempt ONNX model load when PURE_ONNX build is linked
#ifdef RNNOISE_PURE_ONNX
	if (rnnoiseState) {
		auto modelFile = findDefaultOnnxModel();
		if (modelFile.existsAsFile()) {
			if (rnnoise_pure_onnx_load(rnnoiseState, modelFile.getFullPathName().toRawUTF8()) != 0) {
				DBG("[RNNoise] Failed to load ONNX model: " << modelFile.getFullPathName());
			}
			else {
				DBG("[RNNoise] Loaded ONNX model: " << modelFile.getFullPathName());
			}
		}
		else {
			DBG("[RNNoise] No model.onnx found alongside executable.");
		}
	}
#endif

	initRealTimeFftChartPlot();

	// Initialize RNNoise
	rnnoiseState = rnnoise_create(nullptr);
	// rnnoiseFrameSize already set above for accumulation; keep for safety
	rnnoiseFrameSize = rnnoise_get_frame_size();

	curNumInputChannels = 1;
	setAudioChannels(1, 2);
	audioSysInit();

	curTimerFrequencyHz = 60;
	yLimNumTimerCallBacks =
		static_cast<int>(std::round(yLimIntervalMs * curTimerFrequencyHz / 1000));
	startTimerHz(curTimerFrequencyHz);

	setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

}

bool SpectrogramComponent::audioSysInit()
{
	juce::AudioDeviceManager::AudioDeviceSetup currentAudioConfig;
	sharedAudioDeviceManager->getAudioDeviceSetup(currentAudioConfig);

	if (currentAudioConfig.bufferSize < rnnoiseFrameSize)
	{
		currentAudioConfig.bufferSize = rnnoiseFrameSize;
	}
	if (currentAudioConfig.sampleRate != sysSampleRate)
	{
		currentAudioConfig.sampleRate = sysSampleRate;
	}
	sharedAudioDeviceManager->setAudioDeviceSetup(currentAudioConfig, true);

	return true;
}

void SpectrogramComponent::switchToMicrophoneInput()
{
	stopTimer();

	// Stop the thread
	stopTheThread();

	autoSwitchToInput = false;
	doSwitchToMicrophoneInput = false;
	doSwitchTNoneInput = false;
	showFilters = false;

	resetVariables();

	clearPlotAndSpectrogram();

	setAudioChannels(1, 2);

	thisIsNotAudioIOSystem = false;
	notAudioIOSystemIsRunning = false;

	drawSemaphore[0].try_acquire();
	drawSemaphore[1].try_acquire();

	curTimerFrequencyHz = 60;
	yLimNumTimerCallBacks =
		static_cast<int>(std::round(yLimIntervalMs * curTimerFrequencyHz / 1000));
	startTimerHz(curTimerFrequencyHz);

}

void SpectrogramComponent::switchToNonInput()
{
	stopTimer();

	// Stop the thread
	stopTheThread();

	showFilters = false;

	resetVariables();

	thisIsNotAudioIOSystem = false;
	notAudioIOSystemIsRunning = false;

	curTimerFrequencyHz = 60;

	ptrFFTCtrl->switchUIToSpecialPlots();
	ptrFFTModule->switchToNonInput();
}

void SpectrogramComponent::clearPlotAndSpectrogram()
{
	spectrogramImage.clear(spectrogramImage.getBounds());

	module_freqPlot->setXTicks({});
	module_freqPlot->setXTickLabels({});

	module_freqPlot->clearPlot();

}

void SpectrogramComponent::resetVariables()
{
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

	// Stop the thread
	stopTheThread();

	resetVariables();

	sizeToUseInFreqInRealTimeFftChartPlot
		= (int)(fftSize * (maxFreqInRealTimeFftChartPlot / curSampleRate));

	// Prepare cartesian plot
	initRealTimeFftChartPlot();

	setXTicksForPowerGridFrequencies();

	if (ptrFFTCtrl != nullptr)
	{
		ptrFFTCtrl->updateSampleRate(curSampleRate);
	}

	thisIsNotAudioIOSystem = true;
	notAudioIOSystemIsRunning = true;

	// Start the thread
	startThread();

	startTimerHz(60);
	yLimNumTimerCallBacks =
		static_cast<int>(std::round(yLimIntervalMs * 60 / 1000));

}

void SpectrogramComponent::setXTicksForPowerGridFrequencies()
{
	module_freqPlot->clearPlot();
	module_freqPlot->setXTicks({});
	module_freqPlot->setXTickLabels({});

	switch (filterToUse)
	{
	case filter50Hz:
	{
		module_freqPlot->setXTicks({ 50.0f, 100.0f, 150.0f });
		module_freqPlot->setXTickLabels({ "50.0", "100.0", "150.0" });
		break;
	}
	case filter60Hz:
	{
		module_freqPlot->setXTicks({ 60.0f, 120.0f, 180.0f });
		module_freqPlot->setXTickLabels({ "60.0", "120.0", "180.0" });
		break;
	}
	}
}

void SpectrogramComponent::stopTheThread()
{
	// Stop the thread
	while (isThreadRunning())
	{
		signalThreadShouldExit();;
		weSpectrumDataReady[0].signal();
		weSpectrumDataReady[1].signal();

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
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

		stopTheThread();

		showFilters = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		reSetFilterToUse(filterToUse);

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

		clearPlotAndSpectrogram();

		curSampleRate = reader->sampleRate;

		//setFilterToUse(filterToUse);

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
		yLimNumTimerCallBacks = static_cast<int>(std::round(yLimIntervalMs / curTimerInterValMs));

		curTimerFrequencyHz = 1.0 / timePerBuffer;

		startTimer(curTimerInterValMs);
		//startTimerHz(curTimerFrequencyHz);

	}

	return true;
}

SpectrogramComponent::Task SpectrogramComponent::makeFilterPing()
{
	prevMinValFromFFT = std::numeric_limits<float>::max();
	prevMaxValFromFFT = std::numeric_limits<float>::min();

	while (showFilters)
	{
		zeromem(fftDataInBuffer, sizeOfFftDataBuffersInBytes);

		if (filterToUse != noFilter)
		{
			// Set impulse
			fftDataInBuffer[0] = 1.0f;

			// Run it through the filter
			theNotchFilter->process(fftDataInBuffer, fftSize);

			// Make the FFT
			doFFT(fftDataInBuffer, fftSize);

			float minVal, maxVal;
			juce::findMinAndMax(fftDataInBuffer, fftSize, minVal, maxVal);

			if ((minVal < prevMinValFromFFT) || (maxVal > prevMaxValFromFFT))
			{
				if (minVal < prevMinValFromFFT)
				{
					prevMinValFromFFT = minVal;
				}
				if (maxVal > prevMaxValFromFFT)
				{
					prevMaxValFromFFT = maxVal;
				}
				module_freqPlot->yLim(prevMinValFromFFT, prevMaxValFromFFT);
				plotYAndXTicks = true;
			}

		}

		co_await std::suspend_always{};
	}
}

SpectrogramComponent::Task SpectrogramComponent::readerToFftDataCopy()
{
	ptrFFTModule->setTransportSourcePosition(0.0f); // Reset player

	prevMinValFromFFT = std::numeric_limits<float>::max();
	prevMaxValFromFFT = std::numeric_limits<float>::min();

	curNumInputChannels = reader->numChannels;

	auto readerLngth = reader->lengthInSamples;

	for (juce::int64 readerStartSample = 0; readerStartSample < readerLngth; readerStartSample += fftSize)
	{
		theAudioBuffer =
			AudioBuffer<float>
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

		float minVal, maxVal;
		juce::findMinAndMax(fftDataInBuffer, fftSize, minVal, maxVal);

		if ((minVal < prevMinValFromFFT) || (maxVal > prevMaxValFromFFT))
		{
			if (minVal < prevMinValFromFFT)
			{
				prevMinValFromFFT = minVal;
			}
			if (maxVal > prevMaxValFromFFT)
			{
				prevMaxValFromFFT = maxVal;
			}
			module_freqPlot->yLim(prevMinValFromFFT, prevMaxValFromFFT);
			plotYAndXTicks = true;
		}

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

	weSpectrumDataReady[fftDataInBufferIndex ^ 1].signal(); // Secure both buffers filled
	weSpectrumDataReady[fftDataInBufferIndex].reset(); // ... but NOT more than that until read

	do
	{
		notAudioIOSystemIsRunning = !t.resume();

		drawSemaphore[fftDataInBufferIndex].release(); // Show buffer fftDataInBufferIndex is ready

		fftDataInBufferIndex ^= 1; // Toggle
		fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];
	}
	while
		(
			weSpectrumDataReady[fftDataInBufferIndex].wait(500)
			&& !threadShouldExit()
			&& notAudioIOSystemIsRunning
			);

	doSwitchToMicrophoneInput = (!notAudioIOSystemIsRunning) && autoSwitchToInput;
	doSwitchTNoneInput = !doSwitchToMicrophoneInput;
}

void SpectrogramComponent::timerCallback()
{
	if (nextFFTBlockReady)
	{
		doFFT(fftDataOutBuffer, fftSize);
		float minVal, maxVal;
		juce::findMinAndMax(fftDataInBuffer, fftSize, minVal, maxVal);

		if ((minVal < prevMinValFromFFT) || (maxVal > prevMaxValFromFFT))
		{
			if (minVal < prevMinValFromFFT)
			{
				prevMinValFromFFT = minVal;
			}
			if (maxVal > prevMaxValFromFFT)
			{
				prevMaxValFromFFT = maxVal;
			}
			module_freqPlot->yLim(prevMinValFromFFT, prevMaxValFromFFT);
			plotYAndXTicks = true;
		}

		drawNextLineOfSpectrogramAndFftPlotUpdate(fftDataOutBuffer, fftSize);
		nextFFTBlockReady = false;
		repaint();
	}
	else if (thisIsNotAudioIOSystem)
	{

		drawSemaphore[fftDataOutBufferIndex].acquire(); // Wait for next buffer to be ready

		drawNextLineOfSpectrogramAndFftPlotUpdate(fftDataOutBuffer, fftSize);

		weSpectrumDataReady[fftDataOutBufferIndex].signal(); // Task can prepare this buffer of FFT data

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

	static int noCallBacksSinceLastYLims = 0;

	if (noCallBacksSinceLastYLims++ >= yLimNumTimerCallBacks)
	{
		noCallBacksSinceLastYLims = 0;
		calculateYLim();
	}
}

void SpectrogramComponent::calculateYLim()
{
	prevMinValFromFFT = std::numeric_limits<float>::max();
	prevMaxValFromFFT = std::numeric_limits<float>::min();
}


void SpectrogramComponent::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
	curSampleRate = newSampleRate;

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

// The wrapped of rnnoise's |rnnoise_process_frame| function so as to make sure its input/outpu is |f32| format.
// Note tha the frame size is fixed 480.
float SpectrogramComponent::rnnoise_process_frame_block(float* frameInOut)
{
	// In-place: scale to expected amplitude, process, scale back.
	for (int i = 0; i < rnnoiseFrameSize; ++i) frameInOut[i] *= 32768.f;
	float prob = rnnoise_process_frame(rnnoiseState, frameInOut, frameInOut);
	for (int i = 0; i < rnnoiseFrameSize; ++i) frameInOut[i] = eks_clamp(frameInOut[i], -32768.f, 32767.f) / 32768.f;
	return prob;
}

void SpectrogramComponent::rnnoise_process_accumulate(float* samples, int numSamples)
{
	if (!rnnoiseState) return;
	int processed = 0;
	while (processed < numSamples) {
		int space = rnnoiseFrameSize - (int)rnnoiseInAccum.size();
		int toCopy = std::min(space, numSamples - processed);
		rnnoiseInAccum.insert(rnnoiseInAccum.end(), samples + processed, samples + processed + toCopy);
		processed += toCopy;
		if ((int)rnnoiseInAccum.size() == rnnoiseFrameSize) {
			// Process one frame
			rnnoiseOutAccum = rnnoiseInAccum; // copy
			float prob = rnnoise_process_frame_block(rnnoiseOutAccum.data());
			rnnoiseResidualProb = prob; // last probability (legacy variable)
			g_rnnoiseActivityProb.store(prob, std::memory_order_relaxed);
			// Commit to output: we overwrite the corresponding samples at the *start* of this frame window.
			int frameStart = processed - rnnoiseFrameSize;
			std::copy(rnnoiseOutAccum.begin(), rnnoiseOutAccum.end(), samples + frameStart);
			rnnoiseInAccum.clear();
		}
	}
}

void SpectrogramComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
	{
	auto numChans = bufferToFill.buffer->getNumChannels();
	auto noSampels = bufferToFill.numSamples;

	if (numChans > 0)
	{
		const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
		auto* channelWritePtr = bufferToFill.buffer->getWritePointer(0);

		// Process audio with RNNoise
		if (useRnNoise)
		{
			// In-place processing accumulating to full frames, minimal latency.
			auto* channelWritePtr2 = bufferToFill.buffer->getWritePointer(0);
			// Ensure input copy for in-place transform
			if (channelWritePtr2 != channelData) {
				std::memmove(channelWritePtr2, channelData, sizeof(float) * noSampels);
			}
			rnnoise_process_accumulate(channelWritePtr2, noSampels);
		}

		if (numChans >= 2)
		{
			bufferToFill.buffer->copyFrom(1, bufferToFill.startSample, channelData, noSampels);
		}

		for (auto i = 0; i < noSampels; ++i)
			pushNextSampleIntoFifo(channelData[i]);

		//bufferToFill.clearActiveBufferRegion();
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

	if (filterToUse == noFilter)
	{
		fifo[fifoIndex++] = sample;
	}
	else
	{
		fifo[fifoIndex++] = theNotchFilter->process(sample);
	}
}


void SpectrogramComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);

	g.setOpacity(1.0f);
	g.drawImage(spectrogramImage, getLocalBounds().toFloat());

	// Draw activity probability meter on the right edge (simple vertical bar)
	const float prob = g_rnnoiseActivityProb.load(std::memory_order_relaxed);
	const auto bounds = getLocalBounds();
	const int meterWidth = 12;
	juce::Rectangle<int> meterArea(bounds.getRight() - meterWidth, bounds.getY(), meterWidth, bounds.getHeight());
	g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
	g.fillRect(meterArea);
	int fillH = (int)std::round(prob * meterArea.getHeight());
	juce::Rectangle<int> fillRect = meterArea.removeFromBottom(fillH);
	juce::Colour topC = juce::Colours::limegreen.interpolatedWith(juce::Colours::red, 1.0f - prob);
	g.setGradientFill(juce::ColourGradient(topC, (float)fillRect.getCentreX(), (float)fillRect.getY(), juce::Colours::black, (float)fillRect.getCentreX(), (float)fillRect.getBottom(), false));
	g.fillRect(fillRect);
	g.setColour(juce::Colours::white);
	g.setFont(10.f);
	g.drawFittedText(juce::String(prob, 2), meterArea.reduced(1), juce::Justification::centredTop, 1);
}


SpectrogramComponent::~SpectrogramComponent()
{
	shutDownIO();

	rnnoise_destroy(rnnoiseState);

}

void SpectrogramComponent::drawNextLineOfSpectrogramAndFftPlotUpdate(float* fftDataBuffer, unsigned int& fftSize)
{
	if (doRealTimeFftChartPlot)
	{
		plotValues.clear();
		plotValues.push_back(std::vector<float>
			(fftDataBuffer, fftDataBuffer + sizeToUseInFreqInRealTimeFftChartPlot));
		frequencyValues[0].resize(sizeToUseInFreqInRealTimeFftChartPlot);

		//module_freqPlot->updatePlotRealTime(plotValues, frequencyValues);
		//module_freqPlot->updatePlotRealTime(plotValues);
		module_freqPlot->updatePlot(plotValues, frequencyValues);
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
	// Stop thread
	bool threadWasRunning = isThreadRunning();
	if (threadWasRunning)
	{
		stopTimer();
		drawSemaphore[0].release();
		drawSemaphore[1].release();

		do
		{
			signalThreadShouldExit();

			weSpectrumDataReady[0].signal();
			weSpectrumDataReady[1].signal();
		}
		while (!waitForThreadToExit(100));

		drawSemaphore[0].try_acquire();
		drawSemaphore[1].try_acquire();

		resetVariables();

	}
	else
	{
		void shutdownAudio();
	}

	reSetFilterToUse(theFilterType);

	filterToUse = theFilterType;

	prevMinValFromFFT = std::numeric_limits<float>::max();
	prevMaxValFromFFT = std::numeric_limits<float>::min();

	if (showFilters)
	{
		startShowingFilters();
	}
	else if (threadWasRunning)
	{
		startThread();
		startTimer(curTimerInterValMs);
	}
	else
	{
		setAudioChannels(curNumInputChannels, curNumOutputChannels);
	}
}

void SpectrogramComponent::reSetFilterToUse(filterTypes theFilterType)
{
	switch (theFilterType)
	{
	case noFilter:
	{
		theNotchFilter = nullptr;
		break;
	}
	case filter50Hz:
	{
		theNotchFilter = std::make_unique<NotchFilter>(50.0, curSampleRate);
		break;
	}
	case filter60Hz:
	{
		theNotchFilter = std::make_unique<NotchFilter>(60.0, curSampleRate);
		break;
	}
	}
}

void SpectrogramComponent::initRealTimeFftChartPlot()
{
	if (doRealTimeFftChartPlot)
	{
		ptrFFTModule->makeGraphAttributes(graph_attributes);
		plotLegend.push_back("p " + std::to_string(plotLegend.size() + 1));

		module_freqPlot->setTitle("Frequency response [FFT]");
		module_freqPlot->setXLabel("[Hz]");
		module_freqPlot->setYLabel("[Magnitude]");

		fillRTChartPlotFrequencyValues();

		clearPlotAndSpectrogram();

		plotValues = frequencyValues;
		module_freqPlot->updatePlot(plotValues, frequencyValues, graph_attributes, plotLegend);

		drawSemaphore[0].try_acquire();
		drawSemaphore[1].try_acquire();
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

	module_freqPlot->xLim(-10.0f, maxFreqInRealTimeFftChartPlot);
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

			weSpectrumDataReady[0].signal();
			weSpectrumDataReady[1].signal();
		}
		while (!waitForThreadToExit(100));
	}

	resetVariables();

	clearPlotAndSpectrogram();

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


void SpectrogramComponent::setUseRnNoises(bool useRnNoiseIn)
{
	useRnNoise = useRnNoiseIn;
}
