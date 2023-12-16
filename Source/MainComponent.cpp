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
    : Thread("Microphone permission checker")
{
    //[Constructor_pre] You can add your own custom stuff here..
	getSharedAudioDeviceManager();

	module_SoundProcessor =
		std::make_unique<SoundProcessorModule>(module_Plot, sharedAudioDeviceManager);
	module_AudioSettings =
		std::make_unique<AudioSettingsModule>(sharedAudioDeviceManager);

	startThread(Priority::high); // Check microphone permission
    //[/Constructor_pre]

    juce__tabbedComponent.reset (new juce::TabbedComponent (juce::TabbedButtonBar::TabsAtTop));
    addAndMakeVisible (juce__tabbedComponent.get());
    juce__tabbedComponent->setTabBarDepth (30);
    juce__tabbedComponent->setCurrentTabIndex (-1);


    //[UserPreSize]
	juce__tabbedComponent->addTab
	(
		TRANS("Plot")
		, juce::Colours::lightgrey
		, module_Plot.get()
		, false
	);
	juce__tabbedComponent->addTab
	(
		TRANS("Sound Processing Control")
		, juce::Colours::lightgrey
		, module_SoundProcessor.get()
		, false
	);
	juce__tabbedComponent->addTab
	(
		TRANS("Audio Settings")
		, juce::Colours::lightgrey
		, module_AudioSettings.get()
		, false
	);
	juce__tabbedComponent->setCurrentTabIndex(1);
    //[/UserPreSize]

    setSize (600, 700);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	module_AudioSettings = nullptr;
	sharedAudioDeviceManager = nullptr;
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

    juce__tabbedComponent->setBounds (0, 0, proportionOfWidth (0.9904f), proportionOfHeight (0.9913f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::run()
{
	// First som permissions requesting
#ifdef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
#ifdef ON_JUCE_MAC
	if (!(SystemStats::getOperatingSystemType() < SystemStats::MacOSX_10_14))
	{
#endif // ON_JUCE_MAC
		AudioIODevice* CurrentAudioDevice = sharedAudioDeviceManager->getCurrentAudioDevice();
		if (CurrentAudioDevice != nullptr)
		{
			switch (CurrentAudioDevice->checkAudioInputAccessPermissions())
			{
			case eksAVAuthorizationStatusDenied:
			{
				startTimer(1000);
				break;
			}
			case eksAVAuthorizationStatusRestricted:
			case eksAVAuthorizationStatusAuthorized:
			{
				break;
			}
			case eksAVAuthorizationStatusNotDetermined:
			{
				startTimer(1000);
				break;
			}
			default:
			{
				break;
			}
			}
		}
#ifdef ON_JUCE_MAC
}
#endif // #ifdef ON_JUCE_MAC
#endif // #ifdef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
}

#ifndef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
void MainComponent::timerCallback()
{}
#else
void MainComponent::timerCallback()
{
#ifdef ON_JUCE_MAC
	if (SystemStats::getOperatingSystemType() >= SystemStats::MacOSX_10_14)
	{
#endif // #ifdef ON_JUCE_MAC
		AudioIODevice* CurrentAudioDevice = getSharedAudioDeviceManager().getCurrentAudioDevice();
		if (CurrentAudioDevice != nullptr)
		{
			switch (CurrentAudioDevice->checkAudioInputAccessPermissions())
			{
			case eksAVAuthorizationStatusDenied:
			{
				stopTimer();
#if JUCE_MODAL_LOOPS_PERMITTED
				juce::AlertWindow::showMessageBox
				(
					juce::AlertWindow::WarningIcon
					, "Access to audio input device\nNOT granted!"
#if (JUCE_IOS)
					, "You might try to\nEnbale guitarFineTune in\nSettings -> Privacy -> Microphone\nOr UNinstall\nand REinstall guitarFineTune"
#else // JUCE_MAC || JUCE_LINUX
					, "You might try to\nEnbale guitarFineTune in\nSystem Preferences -> Security & Privacy -> Privacy -> Microphone\nOr UNinstall\nand REinstall guitarFineTune"
#endif
					, "Quit"
				);
				getSharedAudioDeviceManager().closeAudioDevice();
				JUCEApplication::getInstance()->systemRequestedQuit();
#else //#if JUCE_MODAL_LOOPS_PERMITTED
				juce::AlertWindow::showMessageBoxAsync
				(
					juce::AlertWindow::WarningIcon
					, "Access to audio input device\nNOT granted!"
#if (JUCE_IOS)
					, "You might try to\nEnbale guitarFineTune in\nSettings -> Privacy -> Microphone\nOr UNinstall\nand REinstall guitarFineTune"
#else // JUCE_MAC || JUCE_LINUX
					, "You might try to\nEnbale guitarFineTune in\nSystem Preferences -> Security & Privacy -> Privacy -> Microphone\nOr UNinstall\nand REinstall guitarFineTune"
#endif
					,
					"Quit"
					,
					nullptr
					,
					juce::ModalCallbackFunction::create
					(
						[this](int)
						{
                            sharedAudioDeviceManager->closeAudioDevice();
                            JUCEApplication::getInstance()->shutdown();
//                            JUCEApplicationBase::quit();
                            JUCEApplication::getInstance()->systemRequestedQuit();
						}
					)
				);
//                JUCEApplication::getInstance()->systemRequestedQuit();
                
#endif //#if JUCE_MODAL_LOOPS_PERMITTED
				break;
		}
			case eksAVAuthorizationStatusRestricted:
			case eksAVAuthorizationStatusAuthorized:
			{
				stopTimer();
				break;
			}
			case eksAVAuthorizationStatusNotDetermined:
			{
				break;
			}
			default:
			{
				break;
			}
			}
	}
#ifdef ON_JUCE_MAC
}
#endif // #ifdef ON_JUCE_MAC
}
#endif // #ifdef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX

String MainComponent::getCurrentDefaultAudioDeviceName(AudioDeviceManager& deviceManager, bool isInput)
{
	auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
	jassert(deviceType != nullptr);

	if (deviceType != nullptr)
	{
		auto deviceNames = deviceType->getDeviceNames();
		return deviceNames[deviceType->getDefaultDeviceIndex(isInput)];
	}

	return {};
}


// (returns a shared AudioDeviceManager object that all the demos can use)
AudioDeviceManager& MainComponent::getSharedAudioDeviceManager
(
	int numInputChannels
	,
	int numOutputChannels
)
{
	if (sharedAudioDeviceManager == nullptr)
		sharedAudioDeviceManager.reset(new AudioDeviceManager());

	auto* currentDevice = sharedAudioDeviceManager->getCurrentAudioDevice();

	if (numInputChannels < 0)
		numInputChannels = (currentDevice != nullptr ? currentDevice->getActiveInputChannels().countNumberOfSetBits() : 1);

	if (numOutputChannels < 0)
		numOutputChannels = (currentDevice != nullptr ? currentDevice->getActiveOutputChannels().countNumberOfSetBits() : 2);

	if (numInputChannels > 0 && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
	{
		RuntimePermissions::request(RuntimePermissions::recordAudio,
			[this, numInputChannels, numOutputChannels](bool granted)
			{
				if (granted)
					getSharedAudioDeviceManager(numInputChannels, numOutputChannels);
			});

		numInputChannels = 0;
	}

	if (sharedAudioDeviceManager->getCurrentAudioDevice() != nullptr)
	{
		auto setup = sharedAudioDeviceManager->getAudioDeviceSetup();

		auto numInputs = jmax(numInputChannels, setup.inputChannels.countNumberOfSetBits());
		auto numOutputs = jmax(numOutputChannels, setup.outputChannels.countNumberOfSetBits());

		auto oldInputs = setup.inputChannels.countNumberOfSetBits();
		auto oldOutputs = setup.outputChannels.countNumberOfSetBits();

		if (oldInputs != numInputs || oldOutputs != numOutputs)
		{
			if (oldInputs == 0 && oldOutputs == 0)
			{
				sharedAudioDeviceManager->initialise(numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);
			}
			else
			{
				setup.useDefaultInputChannels = setup.useDefaultOutputChannels = false;

				setup.inputChannels.clear();
				setup.outputChannels.clear();

				setup.inputChannels.setRange(0, numInputs, true);
				setup.outputChannels.setRange(0, numOutputs, true);

				if (oldInputs == 0 && numInputs > 0 && setup.inputDeviceName.isEmpty())
					setup.inputDeviceName = getCurrentDefaultAudioDeviceName(*sharedAudioDeviceManager, true);

				if (oldOutputs == 0 && numOutputs > 0 && setup.outputDeviceName.isEmpty())
					setup.outputDeviceName = getCurrentDefaultAudioDeviceName(*sharedAudioDeviceManager, false);

				sharedAudioDeviceManager->setAudioDeviceSetup(setup, false);
			}
		}
	}
	else
	{
		sharedAudioDeviceManager->initialise(numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);
	}

	return *sharedAudioDeviceManager;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public juce::Component, private juce::Thread, private Timer"
                 constructorParams="" variableInitialisers="Thread(&quot;Microphone permission checker&quot;),&#10;"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="700">
  <BACKGROUND backgroundColour="ff505050"/>
  <TABBEDCOMPONENT name="new tabbed component" id="b42ee76ffd12e39c" memberName="juce__tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="0 0 99.044% 99.134%"
                   orientation="top" tabBarDepth="30" initialTab="-1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...

//[/EndFile]

