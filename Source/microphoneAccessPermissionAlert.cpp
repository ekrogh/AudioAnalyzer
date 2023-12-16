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

#include "microphoneAccessPermissionAlert.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
microphoneAccessPermissionAlert::microphoneAccessPermissionAlert ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    juce__label.reset (new juce::Label ("new label",
                                        TRANS ("Access to audio input device \\\\n NOT granted!")));
    addAndMakeVisible (juce__label.get());
    juce__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label->setJustificationType (juce::Justification::topLeft);
    juce__label->setEditable (false, false, false);
    juce__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__label2.reset (new juce::Label ("new label",
                                         TRANS ("You might try to\\\\nEnbale AudioAnalyzer in\\\\nSettings -> Privacy -> Microphone\\\\nOr UNinstall\\\\nand REinstall AudioAnalyzer")));
    addAndMakeVisible (juce__label2.get());
    juce__label2->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label2->setJustificationType (juce::Justification::topLeft);
    juce__label2->setEditable (false, false, false);
    juce__label2->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label2->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__textButton.reset (new juce::TextButton ("new button"));
    addAndMakeVisible (juce__textButton.get());
    juce__textButton->setButtonText (TRANS ("Quit"));


    //[UserPreSize]
    juce__label->setText
        (
            TRANS("Access to audio input device\n\nNOT granted!")
            ,
            dontSendNotification
        );
#if (JUCE_IOS)
    juce__label2->setText
    (
        TRANS("You might try to\nEnbale AudioAnalyzer in\nSettings -> Privacy -> Microphone\nOr\nUNinstall and REinstall AudioAnalyzer")
        ,
        dontSendNotification
    );
#else
	juce__label2->setText
	(
		TRANS("You might try to\nEnbale AudioAnalyzer in\nSystem Preferences -> Security & Privacy -> Privacy -> Microphone\nOr\nUNinstall and REinstall AudioAnalyzer")
		,
		dontSendNotification
	);

#endif
	juce__textButton->onClick =
		[this]
		{
//            std::exit (0);
            JUCEApplication::getInstance()->systemRequestedQuit();

//			JUCEApplicationBase::quit();
		};
    //[/UserPreSize]

    setSize (500, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

microphoneAccessPermissionAlert::~microphoneAccessPermissionAlert()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    juce__label = nullptr;
    juce__label2 = nullptr;
    juce__textButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void microphoneAccessPermissionAlert::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void microphoneAccessPermissionAlert::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    juce__label->setBounds ((getWidth() / 2) + -195, (getHeight() / 2) + -156, 468, 48);
    juce__label2->setBounds ((getWidth() / 2) + -195, (getHeight() / 2) + -92, 553, 96);
    juce__textButton->setBounds ((getWidth() / 2) + -187, (getHeight() / 2) + 28, 118, 32);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="microphoneAccessPermissionAlert"
                 componentName="" parentClasses="public juce::Component" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="0" initialWidth="500" initialHeight="400">
  <BACKGROUND backgroundColour="ff505050"/>
  <LABEL name="new label" id="ca182622363d7a28" memberName="juce__label"
         virtualName="" explicitFocusOrder="0" pos="-195C -156C 468 48"
         edTextCol="ff000000" edBkgCol="0" labelText="Access to audio input device \\n NOT granted!"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="9"/>
  <LABEL name="new label" id="800ff60470088085" memberName="juce__label2"
         virtualName="" explicitFocusOrder="0" pos="-195C -92C 553 96"
         edTextCol="ff000000" edBkgCol="0" labelText="You might try to\\nEnbale AudioAnalyzer in\\nSettings -&gt; Privacy -&gt; Microphone\\nOr UNinstall\\nand REinstall AudioAnalyzer"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="9"/>
  <TEXTBUTTON name="new button" id="8fc86f2beaf0fb0f" memberName="juce__textButton"
              virtualName="" explicitFocusOrder="0" pos="-187C 28C 118 32"
              buttonText="Quit" connectedEdges="0" needsCallback="0" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

