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

#include "SoundSynthAndAnalyzeModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
extern AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels = 1, int numOutputChannels = 1);
//[/MiscUserDefs]

//==============================================================================
SoundSynthAndAnalyzeModule::SoundSynthAndAnalyzeModule ()
    : AudioAppComponent(getSharedAudioDeviceManager())
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    setName ("Sound Synth And Analyze Module");
    frequencySlider.reset (new juce::Slider ("Frequency Slider"));
    addAndMakeVisible (frequencySlider.get());
    frequencySlider->setRange (50, 5000, 1);
    frequencySlider->setSliderStyle (juce::Slider::LinearHorizontal);
    frequencySlider->setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 20);

    run__toggleButton.reset (new juce::ToggleButton ("run toggle button"));
    addAndMakeVisible (run__toggleButton.get());
    run__toggleButton->setButtonText (TRANS ("Run"));

    Ts_label.reset (new juce::Label ("Ts_label",
                                     TRANS ("Ts_label")));
    addAndMakeVisible (Ts_label.get());
    Ts_label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    Ts_label->setJustificationType (juce::Justification::centredLeft);
    Ts_label->setEditable (false, false, false);
    Ts_label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    Ts_label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    Ts_label->setBounds (8, 102, 150, 24);

    GNAB__toggleButton.reset (new juce::ToggleButton ("Get Next Audio Block toggle button"));
    addAndMakeVisible (GNAB__toggleButton.get());
    GNAB__toggleButton->setButtonText (TRANS ("Use new for_each"));

    GNAB__toggleButton->setBounds (8, 144, 150, 24);

    measured_ts__label.reset (new juce::Label ("Measured ts label",
                                               TRANS ("Measured Ts")));
    addAndMakeVisible (measured_ts__label.get());
    measured_ts__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    measured_ts__label->setJustificationType (juce::Justification::centredLeft);
    measured_ts__label->setEditable (false, false, false);
    measured_ts__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    measured_ts__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    measured_ts__label->setBounds (8, 184, 150, 24);

    forDurationRms__label.reset (new juce::Label ("for Duration Rms label",
                                                  TRANS ("RMS for Duration")));
    addAndMakeVisible (forDurationRms__label.get());
    forDurationRms__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    forDurationRms__label->setJustificationType (juce::Justification::centredLeft);
    forDurationRms__label->setEditable (false, false, false);
    forDurationRms__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    forDurationRms__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    forDurationRms__label->setBounds (8, 224, 255, 24);

    for_eachDurationRms__label.reset (new juce::Label ("for_each Duration Rms  abel",
                                                       TRANS ("for_each Duration Rms")));
    addAndMakeVisible (for_eachDurationRms__label.get());
    for_eachDurationRms__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    for_eachDurationRms__label->setJustificationType (juce::Justification::centredLeft);
    for_eachDurationRms__label->setEditable (false, false, false);
    for_eachDurationRms__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    for_eachDurationRms__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    for_eachDurationRms__label->setBounds (8, 272, 266, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
	run__toggleButton->onClick =
		[this]
		{
			if (run__toggleButton->getToggleState())
			{
				setAudioChannels(1, 1); // One input, one output

				getNextAudioBlockDurationAvgSum = 0.0f;
				noSmplsGetNextAudioBlockDurationAvgSum = 0;
				getNextAudioBlockDurationSum = 0;
				getNextAudioBlockNoCalls = 0;
			}
			else
			{
				shutdownAudio();
			}
		};

	frequencySlider->setSkewFactorFromMidPoint(500.0);
	frequencySlider->onValueChange = [this]
		{
			if (currentSampleRate > 0.0)
				updateAngleDelta();
		};

	GNAB__toggleButton->onClick =
		[this]
		{
			if (newGNDB = GNAB__toggleButton->getToggleState())
			{
				forDurationRms__label->setText
				(
					std::to_string
					(
						//std::sqrt
						//(
							getNextAudioBlockDurationAvgSum / (double)noSmplsGetNextAudioBlockDurationAvgSum
						//)
					)
					+ " [mS RMS] (for)"
					, dontSendNotification
				);
			}
			else
			{
				for_eachDurationRms__label->setText
				(
					std::to_string
					(
						//std::sqrt
						//(
							getNextAudioBlockDurationAvgSum / (double)noSmplsGetNextAudioBlockDurationAvgSum
						//)
					)
					+ " [mS RMS] (for_each)"
					, dontSendNotification
				);
			}
			getNextAudioBlockDurationAvgSum = 0.0f;
			noSmplsGetNextAudioBlockDurationAvgSum = 0;
			getNextAudioBlockDurationSum = 0;
			getNextAudioBlockNoCalls = 0;

		};

    //[/Constructor]
}

SoundSynthAndAnalyzeModule::~SoundSynthAndAnalyzeModule()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	eksShutdownAudio();
    //[/Destructor_pre]

    frequencySlider = nullptr;
    run__toggleButton = nullptr;
    Ts_label = nullptr;
    GNAB__toggleButton = nullptr;
    measured_ts__label = nullptr;
    forDurationRms__label = nullptr;
    for_eachDurationRms__label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SoundSynthAndAnalyzeModule::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SoundSynthAndAnalyzeModule::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    frequencySlider->setBounds (8, 16, getWidth() - 27, 24);
    run__toggleButton->setBounds (52 - (88 / 2), 56, 88, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SoundSynthAndAnalyzeModule::updateAngleDelta()
{
	auto cyclesPerSample = frequencySlider->getValue() / currentSampleRate;
	phaseDeltaPerSample = cyclesPerSample * juce::MathConstants<double>::twoPi;

	auto bufl = deviceManager.getAudioDeviceSetup().bufferSize;

	Ts_label->setText
	(
		std::to_string
		(
			(double)bufl * 1000.0f / currentSampleRate
		)
		+ " [mS / buffer]"
		, dontSendNotification
	);
}

void SoundSynthAndAnalyzeModule::prepareToPlay(int, double sampleRate)
{
	currentSampleRate = sampleRate;
	updateAngleDelta();
}

void SoundSynthAndAnalyzeModule::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	// For when read audio input
	auto tst = bufferToFill.buffer->getRMSLevel(0, 0, bufferToFill.numSamples);

	auto start = std::chrono::high_resolution_clock::now();

	if (newGNDB)
	{
		auto* channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		std::ranges::for_each
		(
			channelData, channelData + bufferToFill.numSamples,
			[this]
			(float& soundSample)
			{
				soundSample = (float)(std::sin(currentPhase));
				currentPhase += phaseDeltaPerSample;
			}
		);
	}
	else
	{
		auto* channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			//*channelData++ = (float)std::sin(currentPhase);
			channelData[sample] = (float)std::sin(currentPhase);
			currentPhase += phaseDeltaPerSample;
		}
	}
	currentPhase = std::fmod(currentPhase, juce::MathConstants<double>::twoPi);

	auto end = std::chrono::high_resolution_clock::now();

	getNextAudioBlockDurationSum +=
		std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	if (++getNextAudioBlockNoCalls == 100)
	{
		auto getNextAudioBlockDurationMsAvg =
			(double)getNextAudioBlockDurationSum / (double)100000.0f;

		getNextAudioBlockDurationAvgSum +=
			getNextAudioBlockDurationMsAvg/* * getNextAudioBlockDurationMsAvg*/;
		noSmplsGetNextAudioBlockDurationAvgSum++;

		auto DurationMsAvgStr = std::to_string
		(
			getNextAudioBlockDurationMsAvg
		);

		{
			const MessageManagerLock mml;
			measured_ts__label->setText
			(
				DurationMsAvgStr
				+ " [mS]"
				, dontSendNotification
			);
		}
		getNextAudioBlockNoCalls = getNextAudioBlockDurationSum = 0;
	}
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SoundSynthAndAnalyzeModule"
                 componentName="Sound Synth And Analyze Module" parentClasses="public juce::AudioAppComponent"
                 constructorParams="" variableInitialisers="AudioAppComponent(getSharedAudioDeviceManager())"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff505050"/>
  <SLIDER name="Frequency Slider" id="3ad3aaa1f69d9a54" memberName="frequencySlider"
          virtualName="" explicitFocusOrder="0" pos="8 16 27M 24" min="50.0"
          max="5000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="0"/>
  <TOGGLEBUTTON name="run toggle button" id="3e0da1935c285e8f" memberName="run__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="52c 56 88 24" buttonText="Run"
                connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
  <LABEL name="Ts_label" id="ac48f9216f8947d3" memberName="Ts_label" virtualName=""
         explicitFocusOrder="0" pos="8 102 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Ts_label" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="Get Next Audio Block toggle button" id="616fb3e92292bc06"
                memberName="GNAB__toggleButton" virtualName="" explicitFocusOrder="0"
                pos="8 144 150 24" buttonText="Use new for_each" connectedEdges="0"
                needsCallback="0" radioGroupId="0" state="0"/>
  <LABEL name="Measured ts label" id="6f4fdf68a21f2957" memberName="measured_ts__label"
         virtualName="" explicitFocusOrder="0" pos="8 184 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Measured Ts" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="for Duration Rms label" id="c503ef8cb681f3cb" memberName="forDurationRms__label"
         virtualName="" explicitFocusOrder="0" pos="8 224 255 24" edTextCol="ff000000"
         edBkgCol="0" labelText="RMS for Duration" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="for_each Duration Rms  abel" id="6f7bc55edde8550d" memberName="for_eachDurationRms__label"
         virtualName="" explicitFocusOrder="0" pos="8 272 266 24" edTextCol="ff000000"
         edBkgCol="0" labelText="for_each Duration Rms" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

