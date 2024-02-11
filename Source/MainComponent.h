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
#include "freqPlotModule.h"
#include "FFTCtrl.h"
#include "FFTModule.h"
#include "AudioPlaybackModule.h"
#include "AudioRecorderModule.h"
#include "microphoneAccessPermissionAlert.h"
#include "AudioAnalyzerGlobalEnums.h"
#include "AudioSettingsModule.h"
#include "SoundProcessorModule.h"
#include "PlotModule.h"
#include "cmp_plot.h"
#include <JuceHeader.h>


// For microphone permissions checks =================
#if (JUCE_IOS || JUCE_MAC || JUCE_LINUX)
#define JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
#else
#undef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
#endif

#if (JUCE_MAC)
#define ON_JUCE_MAC
#else
#undef ON_JUCE_MAC
#endif

//// For debug/edit in visual studio
 //#define JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
// #define ON_JUCE_MAC
//================================================
//
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
	An auto-generated component, created by the Projucer.

	Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent ();
    ~MainComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    String getCurrentDefaultAudioDeviceName(AudioDeviceManager& deviceManager, bool isInput);

    AudioDeviceManager& getSharedAudioDeviceManager
    (
        int numInputChannels = defaultNumInputChannels
        ,
        int numOutputChannels = defaultNumOutputChannels
    );
    bool checkMicrophoneAccessPermission();

    std::shared_ptr<AudioDeviceManager> sharedAudioDeviceManager;
	std::shared_ptr<PlotModule> module_Plot =
		std::make_shared<PlotModule>();
    std::shared_ptr<SoundProcessorModule> module_SoundProcessor;
    std::shared_ptr<AudioSettingsModule> module_AudioSettings;
    std::shared_ptr<microphoneAccessPermissionAlert> module_microphoneAccessPermissionAlert;
    std::shared_ptr<AudioRecorderModule> module_AudioRecording;
    std::shared_ptr<AudioPlaybackModule> module_AudioPlayback;
    std::shared_ptr<FFTModule> module_FFT;
    std::shared_ptr<FFTCtrl> module_FFTCtrl;
    std::shared_ptr<freqPlotModule> module_freqPlot =
        std::make_shared<freqPlotModule>();


    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TabbedComponent> juce__tabbedComponent;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

