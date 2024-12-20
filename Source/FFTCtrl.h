/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.8

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
#include "FFTModule.h"
#include "freqPlotModule.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
	An auto-generated component, created by the Projucer.

	Describe your class and how it works here!
                                                                    //[/Comments]
*/
class FFTCtrl  : public juce::Component,
                 public juce::Button::Listener
{
public:
    //==============================================================================
    FFTCtrl (std::shared_ptr<FFTModule> ptr_module_FFT, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<freqPlotModule> FPM);
    ~FFTCtrl() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void updateSampleRate(double newSampleRate);
    void setValues(unsigned int fftOrder, unsigned int fftSize);
    void switchUIToSpecialPlots();
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SpectrogramComponent& refSpectrogramComponent;
    FFTModule& refModule_FFT;
    //std::shared_ptr<SpectrogramComponent> ptrSpectrogramComponent = nullptr;
    //std::shared_ptr<FFTModule> module_FFT = nullptr;
    unsigned int fftOrder = 0;
	unsigned int fftSize = 0;
	std::shared_ptr<AudioDeviceManager> sharedAudioDeviceManager;
	std::shared_ptr<freqPlotModule> module_freqPlot;
    bool spectrumOfaudioFileTBState = false;
    bool makespectrumOfInputTBState = true;
    bool showFilterTBState = false;
    double curSampleFreqHz = 44100.0f;
    double maxChartPlotFreq = 44100.0f;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::TextButton> selFile__textButton;
    std::unique_ptr<juce::TextButton> makeWhiteNoise__textButton;
    std::unique_ptr<juce::TextButton> makeSines__textButton;
    std::unique_ptr<juce::TextEditor> freqs__textEditor;
    std::unique_ptr<juce::Label> freqs__label;
    std::unique_ptr<juce::TextEditor> max_freq__textEditor;
    std::unique_ptr<juce::Label> max_Freq__label;
    std::unique_ptr<juce::Label> Sample_Freq__label;
    std::unique_ptr<juce::TextEditor> Sample_Freq__textEditor;
    std::unique_ptr<juce::Label> Nbr_samples__label;
    std::unique_ptr<juce::TextEditor> Nbr_Samples__textEditor;
    std::unique_ptr<juce::TextButton> clearPlot__textButton;
    std::unique_ptr<juce::TextButton> replot__textButton;
    std::unique_ptr<juce::TextEditor> fftOrder__textEditor;
    std::unique_ptr<juce::Label> fftOrder__label;
    std::unique_ptr<juce::Label> fftSize__label;
    std::unique_ptr<juce::Label> fftSizeNbr__label;
    std::unique_ptr<juce::TextButton> fftWindows__textButton;
    std::unique_ptr<juce::ToggleButton> spectrumOfaudioFile__toggleButton;
    std::unique_ptr<juce::ToggleButton> makespectrumOfInput__toggleButton;
    std::unique_ptr<juce::ToggleButton> autoSwitchToInput__toggleButton;
    std::unique_ptr<juce::ToggleButton> use50HzFilter__toggleButton;
    std::unique_ptr<juce::ToggleButton> use60HzFilter__toggleButton;
    std::unique_ptr<juce::ToggleButton> useNoFilter__toggleButton;
    std::unique_ptr<juce::ToggleButton> makeFFtRealTimeChartPlot__toggleButton;
    std::unique_ptr<juce::ToggleButton> showFilter__toggleButton;
    std::unique_ptr<juce::ToggleButton> rnnoise_toggleButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTCtrl)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

