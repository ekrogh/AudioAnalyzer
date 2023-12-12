/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.9

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "cmp_plot.h"
#include "PlotModule.h"
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
                              private juce::Thread
{
public:
    //==============================================================================
    SoundProcessorModule (std::shared_ptr<PlotModule> ptr_module_Plot);
    ~SoundProcessorModule() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void updateAngleDelta();
    void updateFrequencyAndAngleDelta();
    void prepareToPlay(int, double sampleRate) override;
    void releaseResources() override {};
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void run() override; // Called from Thread
    void updateCurrentFrequencyLabel();
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    std::shared_ptr<PlotModule> module_Plot;
    CriticalSection rmsLock;
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
    std::vector<float> frequencyValues;
    std::vector<float> rmsValues;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Slider> maxFrequency__Slider;
    std::unique_ptr<juce::ToggleButton> run__toggleButton;
    std::unique_ptr<juce::Slider> deltaTime__slider;
    std::unique_ptr<juce::Label> juce__label2;
    std::unique_ptr<juce::Label> juce__label3;
    std::unique_ptr<juce::Slider> deltaFreq__slider;
    std::unique_ptr<juce::Label> timeToRun__label;
    std::unique_ptr<juce::Label> timeToRunValue__label;
    std::unique_ptr<juce::Label> juce__label4;
    std::unique_ptr<juce::Label> currentFrequency__label;
    std::unique_ptr<juce::Label> juce__label5;
    std::unique_ptr<juce::Slider> minFreq__slider;
    std::unique_ptr<juce::Label> juce__label;
    std::unique_ptr<juce::Label> juce__label6;
    std::unique_ptr<juce::Label> timeToRunTotally__label;
    std::unique_ptr<juce::ToggleButton> pause__toggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundProcessorModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

