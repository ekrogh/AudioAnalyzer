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

#include "AudioSettingsModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
extern AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels = 1, int numOutputChannels = 1);
//[/MiscUserDefs]

//==============================================================================
AudioSettingsModule::AudioSettingsModule ()
{
    //[Constructor_pre] You can add your own custom stuff here..
	setOpaque(true);

	RuntimePermissions::request(RuntimePermissions::recordAudio,
		[this](bool granted)
		{
			int numInputChannels = granted ? 1 : 0;
			getSharedAudioDeviceManager().initialise(numInputChannels, 1, nullptr, true, {}, nullptr);
		});
    //[/Constructor_pre]

    juce__component.reset (new AudioDeviceSelectorComponent (getSharedAudioDeviceManager(), 1, 1, 1, 2, false, false, false, false));
    addAndMakeVisible (juce__component.get());
    juce__component->setName ("new component");

    Disable_OS_audio_preprocesstogglebutton.reset (new juce::ToggleButton ("Disable OS audio preprocesstoggle button"));
    addAndMakeVisible (Disable_OS_audio_preprocesstogglebutton.get());
    Disable_OS_audio_preprocesstogglebutton->setButtonText (TRANS ("Disable OS audio preprocessing"));


    //[UserPreSize]
	deviceSupportsDisableAudioPreprocessing =
		getSharedAudioDeviceManager().getCurrentAudioDevice()->setAudioPreprocessingEnabled(true);
		if (deviceSupportsDisableAudioPreprocessing)
		{
			Disable_OS_audio_preprocesstogglebutton->onClick =
				[this]
				{
					//if (Disable_OS_audio_preprocesstogglebutton->getToggleState())
					if (deviceSupportsDisableAudioPreprocessing)
					{
						getSharedAudioDeviceManager().getCurrentAudioDevice()->
                        setAudioPreprocessingEnabled
                        (
                            !(Disable_OS_audio_preprocesstogglebutton->
                                    getToggleState())
                         );
					}
				};

			Disable_OS_audio_preprocesstogglebutton->setEnabled(true);
			Disable_OS_audio_preprocesstogglebutton->setVisible(true);
		}
		else
		{
			Disable_OS_audio_preprocesstogglebutton->setEnabled(false);
			Disable_OS_audio_preprocesstogglebutton->setVisible(false);
		}
    //[/UserPreSize]

    setSize (600, 700);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

AudioSettingsModule::~AudioSettingsModule()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    juce__component = nullptr;
    Disable_OS_audio_preprocesstogglebutton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
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

    juce__component->setBounds (8, 8, proportionOfWidth (0.9421f), getHeight() - 260);
    Disable_OS_audio_preprocesstogglebutton->setBounds (16, 8 + (getHeight() - 260) - -4, proportionOfWidth (0.4073f), proportionOfHeight (0.0385f));
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

<JUCER_COMPONENT documentType="Component" className="AudioSettingsModule" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="700">
  <BACKGROUND backgroundColour="ff505050"/>
  <GENERICCOMPONENT name="new component" id="bfa59ed472623a36" memberName="juce__component"
                    virtualName="" explicitFocusOrder="0" pos="8 8 94.208% 260M"
                    class="AudioDeviceSelectorComponent" params="getSharedAudioDeviceManager(), 1, 1, 1, 2, false, false, false, false"/>
  <TOGGLEBUTTON name="Disable OS audio preprocesstoggle button" id="3a7e5ff79aea0286"
                memberName="Disable_OS_audio_preprocesstogglebutton" virtualName=""
                explicitFocusOrder="0" pos="16 -4R 40.734% 3.846%" posRelativeY="bfa59ed472623a36"
                buttonText="Disable OS audio preprocessing" connectedEdges="0"
                needsCallback="0" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

