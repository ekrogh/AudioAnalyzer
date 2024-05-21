/*
  ==============================================================================

	SpectrogramComponent.h
	Created: 20 May 2024 1:28:57pm
	Author:  eigil

  ==============================================================================
*/

#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>
#include "NotchFilter.h"

class FFTModule;

//==============================================================================
class SpectrogramComponent
	: public AudioAppComponent
	, private Timer
	, private juce::TimeSliceClient

{
public:
	SpectrogramComponent
	(
		AudioFormatManager& FM
		,
		std::shared_ptr<AudioDeviceManager> SADM
		,
		FFTModule* FFTMP
		,
		std::shared_ptr<freqPlotModule> FPM
	);

	~SpectrogramComponent() override;

	bool loadURLIntoSpectrum
	(
		const URL& theUrl
		, juce::ToggleButton* spectrumOfaudioFile__toggleButtonPtr
		, juce::ToggleButton* makespectrumOfInput__toggleButtonPtr
	);

	void switchToMicrophoneInput();

	void releaseAllSemaphores();

	void resetVariables();

	void timerCallback() override;

	void prepareToPlay(int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override;

	void releaseResources() override;

	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

	void pushNextSampleIntoFifo(float sample) noexcept;

	void paint(juce::Graphics& g) override;

	void drawNextLineOfSpectrogram();


	template <typename T>
	struct Generator
	{
		// The class name 'Generator' is our choice and it is not required for coroutine
		// magic. Compiler recognizes coroutine by the presence of 'co_yield' keyword.
		// You can use name 'MyGenerator' (or any other name) instead as long as you include
		// nested struct promise_type with 'MyGenerator get_return_object()' method.
		// Note: You need to adjust class constructor/destructor names too when choosing to
		// rename class.

		struct promise_type;
		using handle_type = std::coroutine_handle<promise_type>;

		struct promise_type
		{ // required
			T value_ = false;
			std::exception_ptr exception_;

			Generator get_return_object()
			{
				return Generator(handle_type::from_promise(*this));
			}
			std::suspend_always initial_suspend() { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }
			void return_void() {}
			//void return_value(bool) {}
			void unhandled_exception() { exception_ = std::current_exception(); } // saving
			// exception

			template <std::convertible_to<T> From> // C++20 concept
			std::suspend_always yield_value(From&& from)
			{
				value_ = std::forward<From>(from); // caching the result in promise
				return {};
			}
		};

		handle_type h_;

		Generator(handle_type h)
			: h_(h)
		{
		}
		~Generator() { h_.destroy(); }
		explicit operator bool()
		{
			fill(); // The only way to reliably find out whether or not we finished coroutine,
			// whether or not there is going to be a next value generated (co_yield)
			// in coroutine via C++ getter (operator () below) is to execute/resume
			// coroutine until the next co_yield point (or let it fall off end).
			// Then we store/cache result in promise to allow getter (operator() below
			// to grab it without executing coroutine).
			return !h_.done();
		}
		T operator()()
		{
			fill();
			full_ = false; // we are going to move out previously cached
			// result to make promise empty again
			return std::move(h_.promise().value_);
		}

	private:

		bool full_ = false;

		void fill()
		{
			if (!full_)
			{
				h_();
				if (h_.promise().exception_)
					std::rethrow_exception(h_.promise().exception_);
				// propagate coroutine exception in called context

				full_ = true;
			}
		}

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Generator)
	};

	Generator<bool> readerToFftDataCopy();
	Generator<bool> gen = readerToFftDataCopy();


	int useTimeSlice() override;

	void setAutoSwitchToInput(bool autoSwitch);

	void setFilterToUse(filterTypes theFilterType);

private:
	std::shared_ptr<freqPlotModule> module_freqPlot;
	std::vector <std::vector<float>> frequencyValues{ { 1 }, { 1 } };
	std::vector <std::vector<float>> plotValues{ { 1 }, { 1 } };
	cmp::GraphAttributeList graph_attributes;
	cmp::StringVector plotLegend{ "1", "2" };

	bool doRealTimeFFtPlot = false;

	FFTModule& theFftModule;

	filterTypes filterToUse = noFilter;
	bool filterToUseChanged = false;

	std::unique_ptr<NotchFilter> theNotchFilter;
	AudioBuffer<float> theAudioBuffer;

	bool autoSwitchToInput = false;

	juce::ToggleButton* spectrumOfaudioFile__toggleButton;
	juce::ToggleButton* makespectrumOfInput__toggleButton;


	struct dataBuffers
	{
		float dataBuffer[2 * fftSize];

		std::binary_semaphore
			readyToWriteSemaphoreForDataReadWriteThreadForFftPlotThread =
				std::binary_semaphore{ 1 };
		std::binary_semaphore
			readyToWriteSemaphoreForDataReadWriteThreadForSpectrumDrawerThread =
				std::binary_semaphore{ 1 };
		std::binary_semaphore readyToWriteSemaphoreForFftCalculatorThread =
			std::binary_semaphore{ 0 };
		std::binary_semaphore readyToWriteSemaphoreForFftPlotThread =
			std::binary_semaphore{ 0 };
		std::binary_semaphore readyToWriteSemaphoreForSpectrumDrawerThread =
			std::binary_semaphore{ 0 };

		std::binary_semaphore readyToReadSemaphoreForDataReadWriteThread =
			std::binary_semaphore{ 0 };
		std::binary_semaphore readyToReadSemaphoreForFftCalculatorThread =
			std::binary_semaphore{ 0 };
		std::binary_semaphore readyToReadSemaphoreForFftPlotThread =
			std::binary_semaphore{ 0 };
		std::binary_semaphore readyToReadSemaphoreForSpectrumDrawerThread =
			std::binary_semaphore{ 0 };
	};

	std::vector<dataBuffers> dataBuffers{ numFftBuffers };

	juce::TimeSliceThread dataReadWriteThread{ "Data Read/Write Thread" };
	juce::TimeSliceThread fftCalculatorThread{ "FFT Calculator Thread" };
	juce::TimeSliceThread fftPlotThread{ "FFT Plot Thread" };
	juce::TimeSliceThread spectrumDrawerThread{ "Spectrum Drawer Thread" };

	std::int_fast8_t dataReadWriteBufferIndex = 0;
	std::int_fast8_t fftCalculatorBufferIndex = 0;  // FFT Calculator Buffer Index
	std::int_fast8_t fftPlotBufferIndex = 0;  // Index fftPlot buffer index
	std::int_fast8_t spectrumPlotBufferIndex = 0;  // Index of the buffer currently being 
	
	float* dataReadWriteBuffer = dataBuffers[dataReadWriteBufferIndex].dataBuffer;
	float* fftCalculatorBuffer = dataBuffers[fftCalculatorBufferIndex].dataBuffer;
	float* fftPlotBuffer =		 dataBuffers[fftPlotBufferIndex].dataBuffer;
	float* spectrumPlotBuffer =  dataBuffers[spectrumPlotBufferIndex].dataBuffer;
	float fifo[fftSize] = { 0 };
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;

	std::binary_semaphore timerSemaphoreFftPlot{ 1 };
	std::binary_semaphore timerSemaphoreSpectrumPlot{ 1 };


	bool thisIsAudioFile = false;
	bool audioFileReadRunning = false;
	bool doSwitchToMicrophoneInput = false;

	juce::Image spectrogramImage;
	std::unique_ptr<juce::dsp::FFT> forwardFFT = std::make_unique<dsp::FFT>(fftOrder);

	AudioFormatManager& formatManager;
	std::unique_ptr<AudioFormatReader> reader;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};