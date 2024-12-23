/*
  ==============================================================================

	SpectrogramComponent.cpp
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/
#include <string>
#include <Windows.h> // Include Windows.h for OutputDebugString

#include <vector>
#include "FFTCtrl.h"
#include "FFTModule.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>
#include "eksClamp.h"
#include <cstdlib> // for _putenv
#include "SpectrogramComponent.h"


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

	initRealTimeFftChartPlot();

	curNumInputChannels = 1;
	setAudioChannels(1, 2);
	audioSysInit();

	curTimerFrequencyHz = 60;
	yLimNumTimerCallBacks =
		static_cast<int>(std::round(yLimIntervalMs * curTimerFrequencyHz / 1000));
	startTimerHz(curTimerFrequencyHz);

	setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

	// Set environment variables
	_putenv("PYTHONHOME=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv");
	_putenv("PYTHONPATH=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/Lib;D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/site-packages;D:/Users/eigil/projects/juceProjs/AudioAnalyzer/Source");

	// Add DLL path to PATH and set DLL directory
	std::string dllPath = "D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv";
	std::string pathEnv = "PATH=" + dllPath + ";" + std::getenv("PATH");
	_putenv(pathEnv.c_str());
	SetDllDirectory(dllPath.c_str());

	OutputDebugString("PYTHONHOME: ");
	OutputDebugString(std::getenv("PYTHONHOME"));
	OutputDebugString("\nPYTHONPATH: ");
	OutputDebugString(std::getenv("PYTHONPATH"));
	OutputDebugString("\nPATH: ");
	OutputDebugString(std::getenv("PATH"));
	OutputDebugString("\n");

	//try
	//{
	//	//static py::scoped_interpreter guard{};
	//	OutputDebugString("Python interpreter initialized successfully.\n");

	//	py::object main = py::module::import("__main__");
	//	global = main.attr("__dict__");

	//	// Redirect stdout and stderr
	//	py::exec(R"(
 //           import sys
 //           class CatchOutErr:
 //               def __init__(self):
 //                   self.value = ''
 //               def write(self, txt):
 //                   self.value += txt
 //               def flush(self):
 //                   pass
 //           sys.stdout = CatchOutErr()   # redirect std out
 //           sys.stderr = CatchOutErr()   # redirect std err
 //       )", global);

	//	py::module sys = py::module::import("sys");
	//	OutputDebugString("Python sys module imported successfully.\n");
	//	py::exec("print(sys.path)", global);

	//	std::string stdout_str = py::str(global["sys"].attr("stdout").attr("value"));
	//	OutputDebugString("Python sys.path: ");
	//	OutputDebugString(stdout_str.c_str());
	//	OutputDebugString("\n");

	//	// Import the separation module
	//	OutputDebugString("Attempting to import separation module...\n");
	//	separation = py::module::import("separation");
	//	OutputDebugString("Python separation module imported successfully.\n");

	//	separate_guitar_buffer = separation.attr("separate_guitar_buffer");
	//}
	//catch (const py::error_already_set& e)
	//{
	//	OutputDebugString("Error initializing Python interpreter: ");
	//	OutputDebugString(e.what());
	//	OutputDebugString("\n");
	//}

	//// Set environment variables
	//_putenv("PYTHONHOME=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv");
	//_putenv("PYTHONPATH=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/Lib;D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/site-packages;D:/Users/eigil/projects/juceProjs/AudioAnalyzer/Source");

	//// Add DLL path to PATH and set DLL directory
	//std::string dllPath = "D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv";
	//std::string pathEnv = "PATH=" + dllPath + ";" + std::getenv("PATH");
	//_putenv(pathEnv.c_str());
	//SetDllDirectory(dllPath.c_str());

	//OutputDebugString("PYTHONHOME: ");
	//OutputDebugString(std::getenv("PYTHONHOME"));
	//OutputDebugString("\nPYTHONPATH: ");
	//OutputDebugString(std::getenv("PYTHONPATH"));
	//OutputDebugString("\nPATH: ");
	//OutputDebugString(std::getenv("PATH"));
	//OutputDebugString("\n");

	//try
	//{
	//	static py::scoped_interpreter guard{}; // Start the Python interpreter
	//	OutputDebugString("Python interpreter initialized successfully.\n");

	//	py::object main = py::module::import("__main__");
	//	py::object global = main.attr("__dict__");

	//	// Redirect stdout and stderr
	//	py::exec(R"(
 //           import sys
 //           class CatchOutErr:
 //               def __init__(self):
 //                   self.value = ''
 //               def write(self, txt):
 //                   self.value += txt
 //               def flush(self):
 //                   pass
 //           sys.stdout = CatchOutErr()   # redirect std out
 //           sys.stderr = CatchOutErr()   # redirect std err
 //       )", global);

	//	py::module sys = py::module::import("sys");
	//	OutputDebugString("Python sys module imported successfully.\n");
	//	py::exec("print(sys.path)", global);

	//	std::string stdout_str = py::str(global["sys"].attr("stdout").attr("value"));
	//	OutputDebugString("Python sys.path: ");
	//	OutputDebugString(stdout_str.c_str());
	//	OutputDebugString("\n");

	//	// Import the separation module
	//	OutputDebugString("Attempting to import separation module...\n");
	//	py::module separation = py::module::import("separation");
	//	OutputDebugString("Python separation module imported successfully.\n");

	//	// Convert std::vector<float> to pybind11::array_t<float>
	//	std::vector<float> buffer(1024, 0.0f); // Example buffer data, replace with actual data
	//	py::array_t<float> buffer_array(buffer.size(), buffer.data());

	//	// Call the separate_guitar_buffer function
	//	py::object separate_guitar_buffer = separation.attr("separate_guitar_buffer");
	//	py::array_t<float> result = separate_guitar_buffer(buffer_array);
	//	OutputDebugString("Guitar track separated successfully.\n");

	//	// Capture the print output from Python
	//	std::string python_stdout = py::str(global["sys"].attr("stdout").attr("value"));
	//	OutputDebugString("Python stdout: ");
	//	OutputDebugString(python_stdout.c_str());
	//	OutputDebugString("\n");

	//	std::string python_stderr = py::str(global["sys"].attr("stderr").attr("value"));
	//	OutputDebugString("Python stderr: ");
	//	OutputDebugString(python_stderr.c_str());
	//	OutputDebugString("\n");

	//	// Convert result to C++ data structure if needed
	//	auto buffer_info = result.request();
	//	float* ptr = static_cast<float*>(buffer_info.ptr);
	//	std::vector<float> guitar_track(ptr, ptr + buffer_info.size);  // Use size instead of size()

	//	// Process the separated guitar track as needed

	//}
	//catch (const py::error_already_set& e)
	//{
	//	OutputDebugString("Python error: ");
	//	OutputDebugString(e.what());
	//	OutputDebugString("\n");
	//}
	//catch (const std::exception& e)
	//{
	//	OutputDebugString("Error initializing Python interpreter: ");
	//	OutputDebugString(e.what());
	//	OutputDebugString("\n");
	//}

	auto ph = std::getenv("PYTHONHOME");
	auto pp = std::getenv("PYTHONPATH");
}

bool SpectrogramComponent::audioSysInit()
{
	juce::AudioDeviceManager::AudioDeviceSetup currentAudioConfig;
	sharedAudioDeviceManager->getAudioDeviceSetup(currentAudioConfig);

	if ((currentAudioConfig.bufferSize < frameSize) != 0)
	{
		currentAudioConfig.bufferSize = frameSize;
		sharedAudioDeviceManager->setAudioDeviceSetup(currentAudioConfig, true);
	}

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

// The wrapped of noiseRemoval's |noiseRemoval_process_frame| function so as to make sure its input/outpu is |f32| format.
// Note tha the frame size is fixed 480.
float SpectrogramComponent::noiseRemoval_process(float* pFrameOut, const float* pFrameIn)
{
	float vadProb;
	float* buffer = new float[frameSize];

	// Note: Be careful for the format of the input data.
	std::transform
	(
		&pFrameIn[0]
		, &pFrameIn[frameSize]
		, &buffer[0]
		, [](float x)
		{
			return x * 32768.0f;
		}
	);

	//vadProb = process_frame(rnnoiseState, &buffer[0], &buffer[0]);

	std::transform
	(
		&buffer[0]
		, &buffer[frameSize]
		, &pFrameOut[0]
		, [](float x)
		{
			return eks_clamp(x, -32768, 32767) / 32768.0f;
		}
	);

	return vadProb;
}

std::vector<float> SpectrogramComponent::separateGuitarSounds(const std::vector<float>& inputBuffer)
{
	_putenv("PYTHONHOME=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv");
	_putenv("PYTHONPATH=D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/Lib;D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv/site-packages;D:/Users/eigil/projects/juceProjs/AudioAnalyzer/Source");

	// Add DLL path to PATH and set DLL directory
	std::string dllPath = "D:/Program_Files/Python/miniconda3/envs/guitarStringSoundsEnv";
	std::string pathEnv = "PATH=" + dllPath + ";" + std::getenv("PATH");
	_putenv(pathEnv.c_str());
	SetDllDirectory(dllPath.c_str());

	OutputDebugString("PYTHONHOME: ");
	OutputDebugString(std::getenv("PYTHONHOME"));
	OutputDebugString("\nPYTHONPATH: ");
	OutputDebugString(std::getenv("PYTHONPATH"));
	OutputDebugString("\nPATH: ");
	OutputDebugString(std::getenv("PATH"));
	OutputDebugString("\n");

	try
	{
		static py::scoped_interpreter guard{}; // Start the Python interpreter
		OutputDebugString("Python interpreter initialized successfully.\n");

		py::object main = py::module::import("__main__");
		py::object global = main.attr("__dict__");

		// Redirect stdout and stderr
		py::exec(R"(
           import sys
           class CatchOutErr:
               def __init__(self):
                   self.value = ''
               def write(self, txt):
                   self.value += txt
               def flush(self):
                   pass
           sys.stdout = CatchOutErr()   # redirect std out
           sys.stderr = CatchOutErr()   # redirect std err
       )", global);

		py::module sys = py::module::import("sys");
		OutputDebugString("Python sys module imported successfully.\n");
		py::exec("print(sys.path)", global);

		std::string stdout_str = py::str(global["sys"].attr("stdout").attr("value"));
		OutputDebugString("Python sys.path: ");
		OutputDebugString(stdout_str.c_str());
		OutputDebugString("\n");

		// Import the separation module
		OutputDebugString("Attempting to import separation module...\n");
		py::module separation = py::module::import("separation");
		OutputDebugString("Python separation module imported successfully.\n");

		// Convert std::vector<float> to pybind11::array_t<float>
		std::vector<float> buffer(1024, 0.0f); // Example buffer data, replace with actual data
		py::array_t<float> buffer_array(buffer.size(), buffer.data());

		// Call the separate_guitar_buffer function
		py::object separate_guitar_buffer = separation.attr("separate_guitar_buffer");
		py::array_t<float> result = separate_guitar_buffer(buffer_array);
		OutputDebugString("Guitar track separated successfully.\n");

		// Capture the print output from Python
		std::string python_stdout = py::str(global["sys"].attr("stdout").attr("value"));
		OutputDebugString("Python stdout: ");
		OutputDebugString(python_stdout.c_str());
		OutputDebugString("\n");

		std::string python_stderr = py::str(global["sys"].attr("stderr").attr("value"));
		OutputDebugString("Python stderr: ");
		OutputDebugString(python_stderr.c_str());
		OutputDebugString("\n");

		// Convert result to C++ data structure if needed
		auto buffer_info = result.request();
		float* ptr = static_cast<float*>(buffer_info.ptr);
		std::vector<float> guitar_track(ptr, ptr + buffer_info.size);  // Use size instead of size()

		// Process the separated guitar track as needed
		return guitar_track;

	}
	catch (const py::error_already_set& e)
	{
		OutputDebugString("Python error: ");
		OutputDebugString(e.what());
		OutputDebugString("\n");
	}
	catch (const std::exception& e)
	{
		OutputDebugString("Error initializing Python interpreter: ");
		OutputDebugString(e.what());
		OutputDebugString("\n");
	}

}
//{
//	try
//	{
//		static py::scoped_interpreter guard{};
//
//		py::gil_scoped_acquire acquire;
//
//		// Convert std::vector<float> to pybind11::array_t<float>
//		py::array_t<float> buffer_array(inputBuffer.size(), inputBuffer.data());
//
//		// Call the separate_guitar_buffer function
//		OutputDebugString("Calling separate_guitar_buffer function...\n");
//		py::array_t<float> result = separate_guitar_buffer(buffer_array);
//		OutputDebugString("Guitar track separated successfully.\n");
//
//		// Capture the print output from Python
//		std::string python_stdout = py::str(global["sys"].attr("stdout").attr("value"));
//		OutputDebugString("Python stdout: ");
//		OutputDebugString(python_stdout.c_str());
//		OutputDebugString("\n");
//
//		std::string python_stderr = py::str(global["sys"].attr("stderr").attr("value"));
//		OutputDebugString("Python stderr: ");
//		OutputDebugString(python_stderr.c_str());
//		OutputDebugString("\n");
//
//		// Convert result to C++ data structure if needed
//		auto buffer_info = result.request();
//		float* ptr = static_cast<float*>(buffer_info.ptr);
//		std::vector<float> guitar_track(ptr, ptr + buffer_info.size);
//
//		return guitar_track;
//
//	}
//	catch (const py::error_already_set& e)
//	{
//		OutputDebugString("Python error: ");
//		OutputDebugString(e.what());
//		OutputDebugString("\n");
//		return {};
//	}
//	catch (const std::runtime_error& e)
//	{
//		OutputDebugString("Runtime error: ");
//		OutputDebugString(e.what());
//		OutputDebugString("\n");
//		return {};
//	}
//	catch (const std::exception& e)
//	{
//		OutputDebugString("Error during separation: ");
//		OutputDebugString(e.what());
//		OutputDebugString("\n");
//		return {};
//	}
//	catch (...)
//	{
//		OutputDebugString("Unknown error occurred during separation.\n");
//		return {};
//	}
//}

void SpectrogramComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	auto numChans = bufferToFill.buffer->getNumChannels();
	auto noSamples = bufferToFill.numSamples;

	if (numChans > 0)
	{
		const auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
		auto* channelWritePtr = bufferToFill.buffer->getWritePointer(0);

		// Process audio with RNNoise
		if (useAINoiseRemoval)
		{
			std::vector<float> inputBuffer(channelData, channelData + bufferToFill.numSamples);
			auto outputBuffer = separateGuitarSounds(inputBuffer);
			std::copy(outputBuffer.begin(), outputBuffer.end(), channelWritePtr);
		}

		if (numChans >= 2)
		{
			bufferToFill.buffer->copyFrom(1, bufferToFill.startSample, channelData, noSamples);
		}

		for (auto i = 0; i < noSamples; ++i)
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
}


SpectrogramComponent::~SpectrogramComponent()
{
	shutDownIO();
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
	useAINoiseRemoval = useRnNoiseIn;
}
