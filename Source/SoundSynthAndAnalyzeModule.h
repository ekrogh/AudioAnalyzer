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
#include <JuceHeader.h>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SoundSynthAndAnalyzeModule  : public juce::AudioAppComponent,
                                    private juce::Thread
{
public:
    //==============================================================================
    SoundSynthAndAnalyzeModule ();
    ~SoundSynthAndAnalyzeModule() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
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
    double currentSampleRate = 0.0, currentPhase = 0.0, phaseDeltaPerSample = 0.0;
    double maxFrequencyHz = 50.0f;
    double currentFrequencyHz = 50.0f;
    double deltaFrequencyHz = 1.0f;
    double deltaTimeS = 1.0f;  // 1 sec
    double currentTimeToRunS =
        deltaTimeS * (maxFrequencyHz - currentFrequencyHz) / deltaFrequencyHz;
    double audioSamplesSquareSum = 0.0f;
    unsigned int noSamplesInAudioSamplesSquareSum = 0;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Slider> maxFrequency__Slider;
    std::unique_ptr<juce::ToggleButton> run__toggleButton;
    std::unique_ptr<juce::Slider> deltaTime__slider;
    std::unique_ptr<juce::Label> juce__label;
    std::unique_ptr<juce::Label> juce__label2;
    std::unique_ptr<juce::Label> juce__label3;
    std::unique_ptr<juce::Slider> deltaFreq__slider;
    std::unique_ptr<juce::Label> timeToRun__label;
    std::unique_ptr<juce::Label> timeToRunValue__label;
    std::unique_ptr<juce::Label> juce__label4;
    std::unique_ptr<juce::Label> currentFrequency__label;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundSynthAndAnalyzeModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

