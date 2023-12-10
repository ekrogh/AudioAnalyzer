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
class SoundSynthAndAnalyzeModule  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    SoundSynthAndAnalyzeModule ();
    ~SoundSynthAndAnalyzeModule() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void updateAngleDelta();
    void prepareToPlay(int, double sampleRate) override;
    void releaseResources() override {};
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    double currentSampleRate = 0.0, currentPhase = 0.0, phaseDeltaPerSample = 0.0;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Slider> frequencySlider;
    std::unique_ptr<juce::ToggleButton> run__toggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundSynthAndAnalyzeModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

