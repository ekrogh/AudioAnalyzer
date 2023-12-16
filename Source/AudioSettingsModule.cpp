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
#include "AudioAnalyzerGlobalEnums.h"
//[/Headers]

#include "AudioSettingsModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AudioSettingsModule::AudioSettingsModule (std::shared_ptr<AudioDeviceManager> SADM)
    : sharedAudioDeviceManager(SADM)
{
    //[Constructor_pre] You can add your own custom stuff here..
	setOpaque(true);

//	RuntimePermissions::request(RuntimePermissions::recordAudio,
//		[this](bool granted)
//		{
//			int numInputChannels = granted ? 1 : 0;
//			sharedAudioDeviceManager->initialise(numInputChannels, 1, nullptr, true, {}, nullptr);
//		});
    //[/Constructor_pre]

    juce__component.reset (new AudioDeviceSelectorComponent (*sharedAudioDeviceManager, 1, 1, 1, 2, false, false, false, false));
    addAndMakeVisible (juce__component.get());
    juce__component->setName ("new component");

    Disable_OS_audio_preprocesstogglebutton.reset (new juce::ToggleButton ("Disable OS audio preprocesstoggle button"));
    addAndMakeVisible (Disable_OS_audio_preprocesstogglebutton.get());
    Disable_OS_audio_preprocesstogglebutton->setButtonText (TRANS ("Disable OS audio preprocessing"));


    //[UserPreSize]
	Disable_OS_audio_preprocesstogglebutton->onClick =
		[this]
		{
			//if (Disable_OS_audio_preprocesstogglebutton->getToggleState())
			if (deviceSupportsDisableAudioPreprocessing)
			{
				sharedAudioDeviceManager->getCurrentAudioDevice()->
					setAudioPreprocessingEnabled
					(
						!(Disable_OS_audio_preprocesstogglebutton->getToggleState())
					);
			}
		};

	enableDisableDisable_OS_audio_preprocesstogglebutton();
    //[/UserPreSize]

    setSize (600, 700);


    //[Constructor] You can add your own custom stuff here..
	sharedAudioDeviceManager->addChangeListener(this);
    //[/Constructor]
}

AudioSettingsModule::~AudioSettingsModule()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    juce__component = nullptr;
    Disable_OS_audio_preprocesstogglebutton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
	sharedAudioDeviceManager->removeChangeListener(this);
    //[/Destructor]
}

//==============================================================================
void AudioSettingsModule::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioSettingsModule::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    juce__component->setBounds (8, 8, proportionOfWidth (0.9769f), proportionOfHeight (0.5099f));
    Disable_OS_audio_preprocesstogglebutton->setBounds (8 + 0, 8 + proportionOfHeight (0.5099f) - -12, proportionOfWidth (0.3359f), proportionOfHeight (0.0375f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void AudioSettingsModule::enableDisableDisable_OS_audio_preprocesstogglebutton()
{
	deviceSupportsDisableAudioPreprocessing =
		sharedAudioDeviceManager->getCurrentAudioDevice()->
		setAudioPreprocessingEnabled
		(
			!(Disable_OS_audio_preprocesstogglebutton->getToggleState())
		);

	if (deviceSupportsDisableAudioPreprocessing)
	{

		Disable_OS_audio_preprocesstogglebutton->setEnabled(true);
		Disable_OS_audio_preprocesstogglebutton->setVisible(true);
	}
	else
	{
		Disable_OS_audio_preprocesstogglebutton->setEnabled(false);
		Disable_OS_audio_preprocesstogglebutton->setVisible(false);
	}
}

void AudioSettingsModule::changeListenerCallback(ChangeBroadcaster*)
{
	enableDisableDisable_OS_audio_preprocesstogglebutton();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioSettingsModule" componentName=""
                 parentClasses="public juce::Component, private ChangeListener"
                 constructorParams="std::shared_ptr&lt;AudioDeviceManager&gt; SADM"
                 variableInitialisers="sharedAudioDeviceManager(SADM)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="600" initialHeight="700">
  <BACKGROUND backgroundColour="ff505050"/>
  <GENERICCOMPONENT name="new component" id="bfa59ed472623a36" memberName="juce__component"
                    virtualName="" explicitFocusOrder="0" pos="8 8 97.678% 50.99%"
                    class="AudioDeviceSelectorComponent" params="*sharedAudioDeviceManager, 1, 1, 1, 2, false, false, false, false"/>
  <TOGGLEBUTTON name="Disable OS audio preprocesstoggle button" id="3a7e5ff79aea0286"
                memberName="Disable_OS_audio_preprocesstogglebutton" virtualName=""
                explicitFocusOrder="0" pos="0 -12R 33.607% 3.713%" posRelativeX="bfa59ed472623a36"
                posRelativeY="bfa59ed472623a36" buttonText="Disable OS audio preprocessing"
                connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

