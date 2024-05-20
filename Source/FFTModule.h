/*
  ==============================================================================

    FFTModule.h
    Created: 20 May 2024 1:24:42pm
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include "SpectrogramComponent.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "AudioAnalyzerGlobalEnums.h"
#include "Utilities.h"
#include "freqPlotModule.h"
#include <semaphore>
#include <coroutine>

class FFTModule final : public Component
{
public:
	FFTModule
	(
		std::shared_ptr<AudioDeviceManager> SADM
		,
		std::shared_ptr<freqPlotModule> FPM
	);

	~FFTModule() override;

	void paint(Graphics& g) override;

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
	);
	

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
	);
	

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
	);
	

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
	);
	


	void openAudioFile
	(
		juce::ToggleButton* spectrumOfaudioFile__toggleButton
		, juce::ToggleButton* makespectrumOfInput__toggleButton
	);
	

	void switchToMicrophoneInput();

	bool makeWhiteNoise
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		double maxFreq
		,
		unsigned int sampleRate
	);
	

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
	);
	

	bool makeWindows
	(
		unsigned int fftOrder
		,
		unsigned int fftSize
		,
		double maxFreq
		,
		unsigned int sampleRate
	);
	

	Random randomRGB = juce::Random::getSystemRandom();
	void makeGraphAttributes(cmp::GraphAttributeList& ga);

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
	);
	


	void setAutoSwitchToInput(bool autoSwitch);

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
