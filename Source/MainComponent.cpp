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

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    juce__tabbedComponent.reset (new juce::TabbedComponent (juce::TabbedButtonBar::TabsAtTop));
    addAndMakeVisible (juce__tabbedComponent.get());
    juce__tabbedComponent->setTabBarDepth (30);
    juce__tabbedComponent->addTab (TRANS ("Plot"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS ("Freq. Control"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->addTab (TRANS ("Audio Settings"), juce::Colours::lightgrey, 0, false);
    juce__tabbedComponent->setCurrentTabIndex (0);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    juce__tabbedComponent = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    juce__tabbedComponent->setBounds (0, 0, proportionOfWidth (0.9900f), proportionOfHeight (0.9919f));
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

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff505050"/>
  <TABBEDCOMPONENT name="new tabbed component" id="b42ee76ffd12e39c" memberName="juce__tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="0 0 99.003% 99.194%"
                   orientation="top" tabBarDepth="30" initialTab="0">
    <TAB name="Plot" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Freq. Control" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Audio Settings" colour="ffd3d3d3" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

