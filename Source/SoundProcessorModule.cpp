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

#include "SoundProcessorModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
extern AudioDeviceManager& getSharedAudioDeviceManager
(
	int numInputChannels = numInAndOutputs::defaultNumInputChannels
	,
	int numOutputChannels = numInAndOutputs::defaultNumOutputChannels
);
//[/MiscUserDefs]

//==============================================================================
SoundProcessorModule::SoundProcessorModule (std::shared_ptr<PlotModule> ptr_module_Plot)
    : AudioAppComponent(getSharedAudioDeviceManager()),
      Thread("Freq. shifter"),
      module_Plot(ptr_module_Plot)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    setName ("Sound Synth And Analyze Module");
    maxFrequency__Slider.reset (new juce::Slider ("Max Frequency Slider"));
    addAndMakeVisible (maxFrequency__Slider.get());
    maxFrequency__Slider->setTooltip (TRANS ("Max freq."));
    maxFrequency__Slider->setRange (1, 15000, 1);
    maxFrequency__Slider->setSliderStyle (juce::Slider::LinearHorizontal);
    maxFrequency__Slider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);

    run__toggleButton.reset (new juce::ToggleButton ("run toggle button"));
    addAndMakeVisible (run__toggleButton.get());
    run__toggleButton->setButtonText (TRANS ("Run"));

    deltaTime__slider.reset (new juce::Slider ("delta time slider"));
    addAndMakeVisible (deltaTime__slider.get());
    deltaTime__slider->setTooltip (TRANS ("Delta Time [Sec]"));
    deltaTime__slider->setRange (1, 60, 1);
    deltaTime__slider->setSliderStyle (juce::Slider::LinearHorizontal);
    deltaTime__slider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);

    juce__label2.reset (new juce::Label ("new label",
                                         TRANS ("Delta Time [Sec]")));
    addAndMakeVisible (juce__label2.get());
    juce__label2->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label2->setJustificationType (juce::Justification::centredLeft);
    juce__label2->setEditable (false, false, false);
    juce__label2->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label2->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__label3.reset (new juce::Label ("new label",
                                         TRANS ("Delta freq [Hz]")));
    addAndMakeVisible (juce__label3.get());
    juce__label3->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label3->setJustificationType (juce::Justification::centredLeft);
    juce__label3->setEditable (false, false, false);
    juce__label3->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label3->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    deltaFreq__slider.reset (new juce::Slider ("Delta Freq slider"));
    addAndMakeVisible (deltaFreq__slider.get());
    deltaFreq__slider->setTooltip (TRANS ("Delta Freq [Hz]"));
    deltaFreq__slider->setRange (1, 1000, 1);
    deltaFreq__slider->setSliderStyle (juce::Slider::LinearHorizontal);
    deltaFreq__slider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);

    timeToRun__label.reset (new juce::Label ("time To Run label",
                                             TRANS ("Time remaining to run [Min]:")));
    addAndMakeVisible (timeToRun__label.get());
    timeToRun__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    timeToRun__label->setJustificationType (juce::Justification::centredLeft);
    timeToRun__label->setEditable (false, false, false);
    timeToRun__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    timeToRun__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    timeToRunValue__label.reset (new juce::Label ("Time To Run Value  label",
                                                  TRANS ("Time To Run Value")));
    addAndMakeVisible (timeToRunValue__label.get());
    timeToRunValue__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    timeToRunValue__label->setJustificationType (juce::Justification::centredLeft);
    timeToRunValue__label->setEditable (false, false, false);
    timeToRunValue__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    timeToRunValue__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    timeToRunValue__label->setBounds (216, 336, 150, 24);

    juce__label4.reset (new juce::Label ("new label",
                                         TRANS ("Current frequency [Hz]:")));
    addAndMakeVisible (juce__label4.get());
    juce__label4->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label4->setJustificationType (juce::Justification::centredLeft);
    juce__label4->setEditable (false, false, false);
    juce__label4->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label4->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    currentFrequency__label.reset (new juce::Label ("Current Frequency  label",
                                                    TRANS ("Current Frequency")));
    addAndMakeVisible (currentFrequency__label.get());
    currentFrequency__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    currentFrequency__label->setJustificationType (juce::Justification::centredLeft);
    currentFrequency__label->setEditable (false, false, false);
    currentFrequency__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    currentFrequency__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__label5.reset (new juce::Label ("new label",
                                         TRANS ("Min. frequency [Hz]")));
    addAndMakeVisible (juce__label5.get());
    juce__label5->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label5->setJustificationType (juce::Justification::centredLeft);
    juce__label5->setEditable (false, false, false);
    juce__label5->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label5->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    minFreq__slider.reset (new juce::Slider ("Min. Freq  slider"));
    addAndMakeVisible (minFreq__slider.get());
    minFreq__slider->setTooltip (TRANS ("Min. Freq [Hz]"));
    minFreq__slider->setRange (1, 15000, 1);
    minFreq__slider->setSliderStyle (juce::Slider::LinearHorizontal);
    minFreq__slider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);

    juce__label.reset (new juce::Label ("new label",
                                        TRANS ("Max. freq. [Hz]")));
    addAndMakeVisible (juce__label.get());
    juce__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label->setJustificationType (juce::Justification::centredLeft);
    juce__label->setEditable (false, false, false);
    juce__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__label6.reset (new juce::Label ("new label",
                                         TRANS ("Time to run totally [Min]:")));
    addAndMakeVisible (juce__label6.get());
    juce__label6->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label6->setJustificationType (juce::Justification::centredLeft);
    juce__label6->setEditable (false, false, false);
    juce__label6->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label6->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    timeToRunTotally__label.reset (new juce::Label ("Time To Run Totally  label",
                                                    TRANS ("Time To Run Totally")));
    addAndMakeVisible (timeToRunTotally__label.get());
    timeToRunTotally__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    timeToRunTotally__label->setJustificationType (juce::Justification::centredLeft);
    timeToRunTotally__label->setEditable (false, false, false);
    timeToRunTotally__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    timeToRunTotally__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    pause__toggleButton.reset (new juce::ToggleButton ("pause toggle button"));
    addAndMakeVisible (pause__toggleButton.get());
    pause__toggleButton->setButtonText (TRANS ("Pause"));

    pause__toggleButton->setBounds (120, 408, 150, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 600);


    //[Constructor] You can add your own custom stuff here..
	run__toggleButton->onClick =
		[this]
		{
			if (run__toggleButton->getToggleState())
			{
				startAudio();

				startThread(Priority::low);

			}
			else
			{
				stopThread(10);

				stopAudio();
			}

			pause__toggleButton->setToggleState(false, dontSendNotification);
		};

	pause__toggleButton->setEnabled(false);
	pause__toggleButton->onClick =
		[this]
		{
			if (run__toggleButton->getToggleState())
			{
				if (pause__toggleButton->getToggleState())
				{
					stopThread(10);

					shutdownAudio();

					updateFrequencyAndAngleDelta();
				}
				else
				{
					updateFrequencyAndAngleDelta();

					setAudioChannels(1, 1); // One input, one output

					startThread(Priority::low);
				}
			}
			else
			{
				pause__toggleButton->setToggleState(false, dontSendNotification);
			}
		};

	minFreq__slider->setValue(minFrequencyHz, dontSendNotification);
	minFreq__slider->onValueChange =
		[this]
		{
			minFrequencyHz = minFreq__slider->getValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				maxFrequency__Slider->setValue(minFrequencyHz, dontSendNotification);
				maxFrequencyHz = minFrequencyHz;
			}
			updateFrequencyAndAngleDelta();
		};

	maxFrequency__Slider->setValue(maxFrequencyHz, dontSendNotification);
	maxFrequency__Slider->onValueChange =
		[this]
		{
			maxFrequencyHz = maxFrequency__Slider->getValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				minFreq__slider->setValue(maxFrequencyHz, dontSendNotification);
				minFrequencyHz = maxFrequencyHz;
			}
			updateFrequencyAndAngleDelta();
		};

	deltaTime__slider->setValue(deltaTimeS, dontSendNotification);
	deltaTime__slider->onValueChange =
		[this]
		{
			deltaTimeS = deltaTime__slider->getValue();
			updateFrequencyAndAngleDelta();
		};

	deltaFreq__slider->setValue(deltaFrequencyHz, dontSendNotification);
	deltaFreq__slider->onValueChange =
		[this]
		{
			deltaFrequencyHz = deltaFreq__slider->getValue();
			updateFrequencyAndAngleDelta();
		};

	module_Plot->setTitle("Frequency responce [RMS]");
	module_Plot->setXLabel("[Hz]");
	module_Plot->setYLabel("[RMS]");

	updateFrequencyAndAngleDelta();
    //[/Constructor]
}

SoundProcessorModule::~SoundProcessorModule()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	eksShutdownAudio();
	stopThread(10);
    //[/Destructor_pre]

    maxFrequency__Slider = nullptr;
    run__toggleButton = nullptr;
    deltaTime__slider = nullptr;
    juce__label2 = nullptr;
    juce__label3 = nullptr;
    deltaFreq__slider = nullptr;
    timeToRun__label = nullptr;
    timeToRunValue__label = nullptr;
    juce__label4 = nullptr;
    currentFrequency__label = nullptr;
    juce__label5 = nullptr;
    minFreq__slider = nullptr;
    juce__label = nullptr;
    juce__label6 = nullptr;
    timeToRunTotally__label = nullptr;
    pause__toggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SoundProcessorModule::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SoundProcessorModule::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    maxFrequency__Slider->setBounds (16 + 0, 112, (getWidth() - 40) - 0, 24);
    run__toggleButton->setBounds ((((16 + 0) + 0) + 0) + 0, 408, 88, 24);
    deltaTime__slider->setBounds (((16 + 0) + 0) + 0, 256, (((getWidth() - 40) - 0) - 0) - 0, 24);
    juce__label2->setBounds ((((16 + 0) + 0) + 0) + 0, 224, getWidth() - 446, 24);
    juce__label3->setBounds (((16 + 0) + 0) + 0, 152, 176, 24);
    deltaFreq__slider->setBounds ((16 + 0) + 0, 184, ((getWidth() - 40) - 0) - 0, 24);
    timeToRun__label->setBounds (112 - (192 / 2), 336, 192, 24);
    juce__label4->setBounds ((((16 + 0) + 0) + 0) + 96 - (192 / 2), 370, 192, 24);
    currentFrequency__label->setBounds ((((16 + 0) + 0) + 0) + 200, 370, 150, 24);
    juce__label5->setBounds (16 + 0, 16, getWidth() - 352, 24);
    minFreq__slider->setBounds (16, 48, getWidth() - 40, 24);
    juce__label->setBounds ((16 + 0) + 0, 88, 150, 24);
    juce__label6->setBounds (112 - (192 / 2), 304, 192, 24);
    timeToRunTotally__label->setBounds (292 - (152 / 2), 304, 152, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SoundProcessorModule::startAudio()
{
	currentFrequencyHz = minFrequencyHz;

	updateFrequencyAndAngleDelta();

	frequencyValues.clear();
	rmsValues.clear();

	setAudioChannels(1, 1); // One input, one output

	pause__toggleButton->setEnabled(true);
}

void SoundProcessorModule::stopAudio()
{
	shutdownAudio();

	currentFrequencyHz = 0;

	updateFrequencyAndAngleDelta();

	pause__toggleButton->setEnabled(false);
}

void SoundProcessorModule::updateAngleDelta()
{
	if (currentSampleRate > 0.0)
	{
		auto cyclesPerSample = currentFrequencyHz / currentSampleRate;
		phaseDeltaPerSample = cyclesPerSample * juce::MathConstants<double>::twoPi;
	}

}

void SoundProcessorModule::updateFrequencyAndAngleDelta()
{
	updateAngleDelta();

	if (deltaFrequencyHz > 0.0f)
	{
		auto totalTimeToRunS = deltaTimeS * (maxFrequencyHz - minFrequencyHz) / deltaFrequencyHz;

		timeToRunTotally__label->setText
		(
			std::to_string
			(
				totalTimeToRunS / 60.0f
			)
			, dontSendNotification
		);

		currentTimeToRunS = deltaTimeS * (maxFrequencyHz - currentFrequencyHz) / deltaFrequencyHz;

		timeToRunValue__label->setText
		(
			std::to_string
			(
				currentTimeToRunS / 60.0f
			)
			, dontSendNotification
		);
	}

	currentFrequency__label->setText
	(
		std::to_string
		(
			currentFrequencyHz
		)
		+" [Hz]"
		, dontSendNotification
	);

}

void SoundProcessorModule::prepareToPlay(int, double sampleRate)
{
	currentSampleRate = sampleRate;
	updateFrequencyAndAngleDelta();
}

void SoundProcessorModule::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	const ScopedLock sl(rmsLock);

	// Sum squares
	//auto tst = bufferToFill.buffer->getRMSLevel(0, 0, bufferToFill.numSamples);
	auto* channelRead = bufferToFill.buffer->getReadPointer(0);
	double curSample;
	for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
	{
		curSample = channelRead[sample];
		audioSamplesSquareSum += curSample * curSample;
	}
	noSamplesInAudioSamplesSquareSum += bufferToFill.numSamples;

	auto* channelWrite = bufferToFill.buffer->getWritePointer(0);
	for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
	{
		channelWrite[sample] = (float)std::sin(currentPhase);
		currentPhase += phaseDeltaPerSample;
	}
	currentPhase = std::fmod(currentPhase, juce::MathConstants<double>::twoPi);

}

// the thread
void SoundProcessorModule::run()
{
	// First som permissions requesting
#ifdef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
#ifdef ON_JUCE_MAC
	if (!(SystemStats::getOperatingSystemType() < SystemStats::MacOSX_10_14))
	{
#endif // ON_JUCE_MAC
		AudioIODevice* CurrentAudioDevice = getSharedAudioDeviceManager().getCurrentAudioDevice();
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


	// Then go on
	while ((!threadShouldExit()) && (currentFrequencyHz <= maxFrequencyHz))
	{
		{
			const MessageManagerLock mml;
			updateFrequencyAndAngleDelta();
		}

		wait((int)(1000.0f * deltaTimeS)); // sleep ms

		if (!threadShouldExit())
		{
			{ // ScopedLock sl scope begin
				const ScopedLock sl(rmsLock);

				copyOfAudioSamplesSquareSum = audioSamplesSquareSum;
				audioSamplesSquareSum = 0.0f;

				copyOfNoSamplesInAudioSamplesSquareSum = noSamplesInAudioSamplesSquareSum;
				noSamplesInAudioSamplesSquareSum = 0;

				copyOfCurrentFrequencyHz = currentFrequencyHz;
				currentFrequencyHz += deltaFrequencyHz;
			}  // ScopedLock sl scope end

			frequencyValues.push_back(copyOfCurrentFrequencyHz);

			auto curRMS = std::sqrt(copyOfAudioSamplesSquareSum / copyOfNoSamplesInAudioSamplesSquareSum);
			rmsValues.push_back(curRMS);

			module_Plot->updatePlot(frequencyValues, rmsValues);
		}
	}

	if (currentFrequencyHz >= maxFrequencyHz)
	{
		const MessageManagerLock mml;
		stopAudio();
		run__toggleButton->setToggleState(false, dontSendNotification);
	}

}

#ifndef JUCE_IOS_or_JUCE_MAC_or_JUCE_LINUX
void SoundProcessorModule::timerCallback()
{}
#else
void SoundProcessorModule::timerCallback()
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
				);
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


void SoundProcessorModule::updateCurrentFrequencyLabel()
{
	const MessageManagerLock mml;
	currentFrequency__label->setText
	(
		std::to_string
		(
			currentFrequencyHz
		)
		+" [Hz]"
		, dontSendNotification
	);

}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SoundProcessorModule" componentName="Sound Synth And Analyze Module"
                 parentClasses="public juce::AudioAppComponent, private juce::Thread, private Timer"
                 constructorParams="std::shared_ptr&lt;PlotModule&gt; ptr_module_Plot"
                 variableInitialisers="AudioAppComponent(getSharedAudioDeviceManager())&#10;Thread(&quot;Freq. shifter&quot;)&#10;module_Plot(ptr_module_Plot)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="600">
  <BACKGROUND backgroundColour="ff505050"/>
  <SLIDER name="Max Frequency Slider" id="3ad3aaa1f69d9a54" memberName="maxFrequency__Slider"
          virtualName="" explicitFocusOrder="0" pos="0 112 0M 24" posRelativeX="887447af4b675ddb"
          posRelativeW="887447af4b675ddb" tooltip="Max freq." min="1.0"
          max="15000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="0"/>
  <TOGGLEBUTTON name="run toggle button" id="3e0da1935c285e8f" memberName="run__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="0 408 88 24" posRelativeX="3f78bae238bae958"
                buttonText="Run" connectedEdges="0" needsCallback="0" radioGroupId="0"
                state="0"/>
  <SLIDER name="delta time slider" id="3f78bae238bae958" memberName="deltaTime__slider"
          virtualName="" explicitFocusOrder="0" pos="0 256 0M 24" posRelativeX="27e8662d217379e4"
          posRelativeW="27e8662d217379e4" tooltip="Delta Time [Sec]" min="1.0"
          max="60.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="0"/>
  <LABEL name="new label" id="69170c72758aeb9e" memberName="juce__label2"
         virtualName="" explicitFocusOrder="0" pos="0 224 446M 24" posRelativeX="3f78bae238bae958"
         edTextCol="ff000000" edBkgCol="0" labelText="Delta Time [Sec]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="30871f40ff4c0cbe" memberName="juce__label3"
         virtualName="" explicitFocusOrder="0" pos="0 152 176 24" posRelativeX="27e8662d217379e4"
         edTextCol="ff000000" edBkgCol="0" labelText="Delta freq [Hz]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <SLIDER name="Delta Freq slider" id="27e8662d217379e4" memberName="deltaFreq__slider"
          virtualName="" explicitFocusOrder="0" pos="0 184 0M 24" posRelativeX="3ad3aaa1f69d9a54"
          posRelativeW="3ad3aaa1f69d9a54" tooltip="Delta Freq [Hz]" min="1.0"
          max="1000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="0"/>
  <LABEL name="time To Run label" id="88f16d9da9e68c20" memberName="timeToRun__label"
         virtualName="" explicitFocusOrder="0" pos="112c 336 192 24" posRelativeW="93293acbba06ebfc"
         edTextCol="ff000000" edBkgCol="0" labelText="Time remaining to run [Min]:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="Time To Run Value  label" id="37068ae926a0a595" memberName="timeToRunValue__label"
         virtualName="" explicitFocusOrder="0" pos="216 336 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Time To Run Value" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="7a68dc774966fe8" memberName="juce__label4"
         virtualName="" explicitFocusOrder="0" pos="96c 370 192 24" posRelativeX="3f78bae238bae958"
         edTextCol="ff000000" edBkgCol="0" labelText="Current frequency [Hz]:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="Current Frequency  label" id="fb908497d89dec02" memberName="currentFrequency__label"
         virtualName="" explicitFocusOrder="0" pos="200 370 150 24" posRelativeX="3f78bae238bae958"
         edTextCol="ff000000" edBkgCol="0" labelText="Current Frequency"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="d890f1fd96613a14" memberName="juce__label5"
         virtualName="" explicitFocusOrder="0" pos="0 16 352M 24" posRelativeX="887447af4b675ddb"
         edTextCol="ff000000" edBkgCol="0" labelText="Min. frequency [Hz]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <SLIDER name="Min. Freq  slider" id="887447af4b675ddb" memberName="minFreq__slider"
          virtualName="" explicitFocusOrder="0" pos="16 48 40M 24" tooltip="Min. Freq [Hz]"
          min="1.0" max="15000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="0"/>
  <LABEL name="new label" id="b25ba8f932e8325" memberName="juce__label"
         virtualName="" explicitFocusOrder="0" pos="0 88 150 24" posRelativeX="3ad3aaa1f69d9a54"
         edTextCol="ff000000" edBkgCol="0" labelText="Max. freq. [Hz]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="93293acbba06ebfc" memberName="juce__label6"
         virtualName="" explicitFocusOrder="0" pos="112c 304 192 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Time to run totally [Min]:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="Time To Run Totally  label" id="e16bd418eeb7240e" memberName="timeToRunTotally__label"
         virtualName="" explicitFocusOrder="0" pos="292c 304 152 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Time To Run Totally" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="pause toggle button" id="880f5cc9c6966415" memberName="pause__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="120 408 150 24" buttonText="Pause"
                connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

