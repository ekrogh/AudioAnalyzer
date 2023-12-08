/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioSettingsModule
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays information about audio devices.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioSettingsModule

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "Utilities.h"

extern AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels = 1, int numOutputChannels = 1);

//==============================================================================
class AudioSettingsModule final : public Component
{
public:
    AudioSettingsModule()
    {
        setOpaque (true);

        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 1 : 0;
                                         getSharedAudioDeviceManager().initialise(numInputChannels, 1, nullptr, true, {}, nullptr);
                                     });

        audioSetupComp.reset (new AudioDeviceSelectorComponent (getSharedAudioDeviceManager(),
                                                                1, 1, 1, 2, false, false, false, false));
        addAndMakeVisible (audioSetupComp.get());

        setSize (500, 600);
    }

    ~AudioSettingsModule() override
    {
        audioSetupComp = nullptr;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r =  getLocalBounds().reduced (4);
        audioSetupComp->setBounds (r.removeFromTop (proportionOfHeight (0.65f)));
    }


private:

    std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsModule)
};
