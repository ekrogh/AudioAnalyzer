/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.10

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "AudioRecorderModule.h"
#include "cmp_plot.h"
#include "PlotModule.h"
#include "Utilities.h"
#include <JuceHeader.h>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
	An auto-generated component, created by the Projucer.

	Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SoundProcessorModule  : public juce::AudioAppComponent,
                              private juce::Thread,
                              public juce::Button::Listener
{
public:
    //==============================================================================
    SoundProcessorModule (std::shared_ptr<PlotModule> ptr_module_Plot, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<AudioRecorderModule> MAR);
    ~SoundProcessorModule() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void startStopRecording();
	void makeGraphAttributes();
	void stopAudio();
	void updateAngleDelta();
	void updateInfoTextEditors();
	void prepareToPlay(int, double sampleRate) override;
	void releaseResources() override {};
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void run() override; // Called from Thread
	void updateCurrentFrequencyLabel();
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
	std::shared_ptr<PlotModule> module_Plot;
	CriticalSection rmsLock;
	CriticalSection clearOldMeasuredLock;
	double currentSampleRate = 0.0, currentPhase = 0.0, phaseDeltaPerSample = 0.0;
	double minFrequencyHz = 1.0f;
	double maxFrequencyHz = 50.0f;
	double currentFrequencyHz = 0.0f;
	double deltaFrequencyHz = 1.0f;
	double deltaTimeS = 1.0f;  // 1 sec
	double currentTimeToRunS =
		deltaTimeS * (maxFrequencyHz - currentFrequencyHz) / deltaFrequencyHz;
	long double audioSamplesSquareSum = 0.0f;
	unsigned long long noSamplesInAudioSamplesSquareSum = 0;

	double copyOfCurrentFrequencyHz = 0.0f;
	long double copyOfAudioSamplesSquareSum = 0.0f;
	unsigned long long copyOfNoSamplesInAudioSamplesSquareSum = 0;
	std::vector <std::vector<float>> frequencyValues;
	std::vector<float> forPush_BackFrequencyVector; // To be used for insert to frequencyValues
	std::vector <std::vector<float>> rmsValues;
	std::vector<float> forPush_BackRMSVector; // To be used for insert to rmsValues
	cmp::GraphAttributeList graph_attributes;
	FileChooser chooser
	{
		"File..."
		,
		File::getSpecialLocation
		(
			juce::File::SpecialLocationType::userMusicDirectory
		)
		.getChildFile("measurements.aua"), "*.aua"
	};
	cmp::StringVector plotLegend;
    std::shared_ptr<AudioRecorderModule> module_AudioRecording;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::ToggleButton> runNewMeasurement__toggleButton;
    std::unique_ptr<juce::Label> juce__label2;
    std::unique_ptr<juce::Label> juce__label3;
    std::unique_ptr<juce::Label> timeToRun__label;
    std::unique_ptr<juce::Label> qurFreq__label;
    std::unique_ptr<juce::Label> juce__label5;
    std::unique_ptr<juce::Label> juce__label;
    std::unique_ptr<juce::Label> juce__label6;
    std::unique_ptr<juce::ToggleButton> pause__toggleButton;
    std::unique_ptr<juce::TextButton> Deleteoldmeasurements__textButton;
    std::unique_ptr<juce::TextEditor> minFreq__textEditor;
    std::unique_ptr<juce::TextEditor> maxFreq__textEditor;
    std::unique_ptr<juce::TextEditor> deltaFreq__textEditor;
    std::unique_ptr<juce::TextEditor> deltaTime__textEditor;
    std::unique_ptr<juce::TextButton> save__textButton;
    std::unique_ptr<juce::TextButton> read__textButton;
    std::unique_ptr<juce::TextEditor> timeToRunTotally__textEditor;
    std::unique_ptr<juce::TextEditor> timeToRunValue__textEditor;
    std::unique_ptr<juce::TextEditor> currentFrequencyValue__textEditor2;
    std::unique_ptr<juce::ToggleButton> recordAudio__toggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundProcessorModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

