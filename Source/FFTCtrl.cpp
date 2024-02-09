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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "FFTCtrl.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
FFTCtrl::FFTCtrl (std::shared_ptr<FFTModule> ptr_module_FFT, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<freqPlotModule> FPM)
    : sharedAudioDeviceManager(SADM),
      module_FFT(ptr_module_FFT),
      module_freqPlot(FPM)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    selFile__textButton.reset (new juce::TextButton ("select file button"));
    addAndMakeVisible (selFile__textButton.get());
    selFile__textButton->setButtonText (TRANS ("Select Audio File"));
    selFile__textButton->addListener (this);

    selFile__textButton->setBounds (16, 16, 150, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

FFTCtrl::~FFTCtrl()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    selFile__textButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void FFTCtrl::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void FFTCtrl::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void FFTCtrl::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == selFile__textButton.get())
    {
        //[UserButtonCode_selFile__textButton] -- add your button handler code here..
        module_FFT->selectFile();
        //[/UserButtonCode_selFile__textButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="FFTCtrl" componentName=""
                 parentClasses="public juce::Component" constructorParams="std::shared_ptr&lt;FFTModule&gt; ptr_module_FFT, std::shared_ptr&lt;AudioDeviceManager&gt; SADM, std::shared_ptr&lt;freqPlotModule&gt; FPM"
                 variableInitialisers="sharedAudioDeviceManager(SADM)&#10;module_FFT(ptr_module_FFT)&#10;module_freqPlot(FPM)&#10;"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff505050"/>
  <TEXTBUTTON name="select file button" id="919b4d6b7887d05b" memberName="selFile__textButton"
              virtualName="" explicitFocusOrder="0" pos="16 16 150 24" buttonText="Select Audio File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

