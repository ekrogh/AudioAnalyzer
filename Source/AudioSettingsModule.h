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
class AudioSettingsModule  : public juce::Component,
                             private ChangeListener
{
public:
    //==============================================================================
    AudioSettingsModule (std::shared_ptr<AudioDeviceManager> SADM);
    ~AudioSettingsModule() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void enableDisableDisable_OS_audio_preprocesstogglebutton();
    void changeListenerCallback(ChangeBroadcaster*) override;
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    std::shared_ptr<AudioDeviceManager> sharedAudioDeviceManager;
    bool deviceSupportsDisableAudioPreprocessing = false;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<AudioDeviceSelectorComponent> juce__component;
    std::unique_ptr<juce::ToggleButton> Disable_OS_audio_preprocesstogglebutton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsModule)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

