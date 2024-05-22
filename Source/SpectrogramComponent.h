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
	, private Thread
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

	void drawNextLineOfSpectrogramAndFftPlotUpdate();

	void doFFT(float* fftBuffer, auto fftSize);

	void run() override; // Called from Thread

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

	void setAutoSwitchToInput(bool autoSwitch);

	void setFilterToUse(filterTypes theFilterType);

	void setDoRealTimeFftChartPlot(bool doRTFftCP);
	void initRealTimeFftChartPlot();

	void setMaxFreqInRealTimeFftChartPlot(double axFITFftCP);

private:
	WaitableEvent weSpectrumDataReady;

	std::shared_ptr<freqPlotModule> module_freqPlot;
	std::vector <std::vector<float>> frequencyValues{ { 1 }, { 1 } };
	std::vector <std::vector<float>> plotValues{ { 1 }, { 1 } };
	cmp::GraphAttributeList graph_attributes;
	cmp::StringVector plotLegend{ "1", "2" };

	bool doRealTimeFftChartPlot = false;
	double maxFreqInRealTimeFftChartPlot = 22050.0f;
	int sizeToUseInFreqInRealTimeFftChartPlot = fftSize;

	FFTModule& theFftModule;

	filterTypes filterToUse = noFilter;

	std::unique_ptr<NotchFilter> theNotchFilter;
	AudioBuffer<float> theAudioBuffer;

	bool autoSwitchToInput = false;

	juce::ToggleButton* spectrumOfaudioFile__toggleButton;
	juce::ToggleButton* makespectrumOfInput__toggleButton;


	std::array<float[2 * fftSize], 2> fftDataBuffers;  // Array of two fftData buffers
	int fftDataInBufferIndex = 0;  // Index of the buffer currently being filled
	int fftDataOutBufferIndex = 0;  // Index of the buffer currently being read

	float* fftDataInBuffer = fftDataBuffers[fftDataInBufferIndex];
	float* fftDataOutBuffer = fftDataBuffers[fftDataOutBufferIndex];

	float fifo[fftSize] = { 0 };
	int fifoIndex = 0;
	bool nextFFTBlockReady = false;
	double curSampleRate = 44100.0f; // Hz

	bool thisIsAudioFile = false;
	bool audioFileReadRunning = false;
	bool doSwitchToMicrophoneInput = false;

	juce::Image spectrogramImage;
	std::unique_ptr<juce::dsp::FFT> forwardFFT = std::make_unique<dsp::FFT>(fftOrder);

	AudioFormatManager& formatManager;
	std::unique_ptr<AudioFormatReader> reader;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};