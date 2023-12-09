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

#include "SineSynthModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
extern AudioDeviceManager& getSharedAudioDeviceManager(int numInputChannels = 1, int numOutputChannels = 1);
//[/MiscUserDefs]

//==============================================================================
SineSynthModule::SineSynthModule()
	: AudioAppComponent(getSharedAudioDeviceManager())
{
	//[Constructor_pre] You can add your own custom stuff here..
	//[/Constructor_pre]

	setName("Sine Synth Module");
	frequencySlider.reset(new juce::Slider("Frequency Slider"));
	addAndMakeVisible(frequencySlider.get());
	frequencySlider->setRange(50, 5000, 1);
	frequencySlider->setSliderStyle(juce::Slider::LinearHorizontal);
	frequencySlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);

	run__toggleButton.reset(new juce::ToggleButton("run toggle button"));
	addAndMakeVisible(run__toggleButton.get());
	run__toggleButton->setButtonText(TRANS("Run"));

	newGNDB__toggleButton.reset(new juce::ToggleButton("new getNextDaaBlock toggle button"));
	addAndMakeVisible(newGNDB__toggleButton.get());
	newGNDB__toggleButton->setButtonText(TRANS("new getNextDaaBlock"));

	Ts_label.reset(new juce::Label("Ts_label",
		TRANS("Ts_label")));
	addAndMakeVisible(Ts_label.get());
	Ts_label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	Ts_label->setJustificationType(juce::Justification::centredLeft);
	Ts_label->setEditable(false, false, false);
	Ts_label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	Ts_label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	Ts_label->setBounds(8, 144, 150, 24);


	//[UserPreSize]
	//[/UserPreSize]

	setSize(600, 300);


	//[Constructor] You can add your own custom stuff here..
	run__toggleButton->onStateChange =
		[this]
		{
			if (run__toggleButton->getToggleState())
			{
				setAudioChannels(1, 1); // One input, one output
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

	newGNDB__toggleButton->onStateChange =
		[this]
		{
			newGNDB = newGNDB__toggleButton->getToggleState();
		};

	//[/Constructor]
}

SineSynthModule::~SineSynthModule()
{
	//[Destructor_pre]. You can add your own custom destruction code here..
	eksShutdownAudio();
	//[/Destructor_pre]

	frequencySlider = nullptr;
	run__toggleButton = nullptr;
	newGNDB__toggleButton = nullptr;
	Ts_label = nullptr;


	//[Destructor]. You can add your own custom destruction code here..
	//[/Destructor]
}

//==============================================================================
void SineSynthModule::paint(juce::Graphics& g)
{
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	g.fillAll(juce::Colour(0xff505050));

	//[UserPaint] Add your own custom painting code here..
	//[/UserPaint]
}

void SineSynthModule::resized()
{
	//[UserPreResize] Add your own custom resize code here..
	//[/UserPreResize]

	frequencySlider->setBounds(8, 16, getWidth() - 27, 24);
	run__toggleButton->setBounds(52 - (88 / 2), 56, 88, 24);
	newGNDB__toggleButton->setBounds(104 - (192 / 2), 97, 192, 24);
	//[UserResized] Add your own custom resize handling here..
	//[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SineSynthModule::updateAngleDelta()
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

void SineSynthModule::prepareToPlay(int, double sampleRate)
{
	currentSampleRate = sampleRate;
	updateAngleDelta();
}

void SineSynthModule::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	auto* channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

	for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
	{
		channelData[sample] = (float)std::sin(currentPhase);
		currentPhase += phaseDeltaPerSample;
	}

	currentPhase = std::fmod(currentPhase, juce::MathConstants<double>::twoPi);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

	This is where the Projucer stores the metadata that describe this GUI layout, so
	make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SineSynthModule" componentName="Sine Synth Module"
				 parentClasses="public juce::AudioAppComponent" constructorParams=""
				 variableInitialisers="AudioAppComponent(getSharedAudioDeviceManager())"
				 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
				 fixedSize="0" initialWidth="600" initialHeight="300">
  <BACKGROUND backgroundColour="ff505050"/>
  <SLIDER name="Frequency Slider" id="3ad3aaa1f69d9a54" memberName="frequencySlider"
		  virtualName="" explicitFocusOrder="0" pos="8 16 27M 24" min="50.0"
		  max="5000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
		  textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
		  needsCallback="0"/>
  <TOGGLEBUTTON name="run toggle button" id="3e0da1935c285e8f" memberName="run__toggleButton"
				virtualName="" explicitFocusOrder="0" pos="52c 56 88 24" buttonText="Run"
				connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new getNextDaaBlock toggle button" id="2968f63f06f38605"
				memberName="newGNDB__toggleButton" virtualName="" explicitFocusOrder="0"
				pos="104c 97 192 24" buttonText="new getNextDaaBlock" connectedEdges="0"
				needsCallback="0" radioGroupId="0" state="0"/>
  <LABEL name="Ts_label" id="ac48f9216f8947d3" memberName="Ts_label" virtualName=""
		 explicitFocusOrder="0" pos="8 144 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="Ts_label" editableSingleClick="0" editableDoubleClick="0"
		 focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
		 kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

