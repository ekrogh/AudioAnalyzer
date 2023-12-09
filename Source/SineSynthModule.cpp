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

	run__toggleButton->setBounds(8, 56, 144, 24);


	//[UserPreSize]
	//[/UserPreSize]

	setSize(600, 100);


	//[Constructor] You can add your own custom stuff here..
	run__toggleButton->onStateChange = [this]
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
	//[/Constructor]
}

SineSynthModule::~SineSynthModule()
{
	//[Destructor_pre]. You can add your own custom destruction code here..
	shutdownAudio();
	//[/Destructor_pre]

	frequencySlider = nullptr;
	run__toggleButton = nullptr;


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
	//[UserResized] Add your own custom resize handling here..
	//[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SineSynthModule::updateAngleDelta()
{
	auto cyclesPerSample = frequencySlider->getValue() / currentSampleRate;
	phaseDeltaPerSample = cyclesPerSample * juce::MathConstants<double>::twoPi;
}

void SineSynthModule::prepareToPlay(int, double sampleRate)
{
	currentSampleRate = sampleRate;
	updateAngleDelta();
}


void SineSynthModule::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	double gainToUse = 0.125f;

	float* channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
#if (JUCE_WINDOWS && _DEBUG)
	auto outBffrStart = stdext::make_unchecked_array_iterator(channelData);
	auto outBffrEnd = stdext::make_unchecked_array_iterator(channelData + bufferToFill.numSamples);
#else // (JUCE_WINDOWS && _DEBUG)
	auto outBffrStart = channelData;
	auto outBffrEnd = channelData + bufferToFill.numSamples;
#endif // (JUCE_WINDOWS && _DEBUG)

	std::for_each
	(
		outBffrStart, outBffrEnd,
		[this, gainToUse]
		(float& soundSample)
		{
			currentPhase = std::fmod(currentPhase + phaseDeltaPerSample, juce::MathConstants<double>::twoPi);
			soundSample = (float)(gainToUse * std::sin(currentPhase));
		}
	);

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
				 fixedSize="0" initialWidth="600" initialHeight="100">
  <BACKGROUND backgroundColour="ff505050"/>
  <SLIDER name="Frequency Slider" id="3ad3aaa1f69d9a54" memberName="frequencySlider"
		  virtualName="" explicitFocusOrder="0" pos="8 16 27M 24" min="50.0"
		  max="5000.0" int="1.0" style="LinearHorizontal" textBoxPos="TextBoxLeft"
		  textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
		  needsCallback="0"/>
  <TOGGLEBUTTON name="run toggle button" id="3e0da1935c285e8f" memberName="run__toggleButton"
				virtualName="" explicitFocusOrder="0" pos="8 56 144 24" buttonText="Run"
				connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

