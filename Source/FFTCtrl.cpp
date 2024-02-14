/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.10

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
FFTCtrl::FFTCtrl(std::shared_ptr<FFTModule> ptr_module_FFT, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<freqPlotModule> FPM)
	: sharedAudioDeviceManager(SADM),
	module_FFT(ptr_module_FFT),
	module_freqPlot(FPM)
{
	//[Constructor_pre] You can add your own custom stuff here..
	//[/Constructor_pre]

	selFile__textButton.reset(new juce::TextButton("select file button"));
	addAndMakeVisible(selFile__textButton.get());
	selFile__textButton->setExplicitFocusOrder(2);
	selFile__textButton->setButtonText(TRANS("Select Audio File"));
	selFile__textButton->addListener(this);

	selFile__textButton->setBounds(16, 80, 150, 24);

	makeWhiteNoise__textButton.reset(new juce::TextButton("makeWhiteNoise button"));
	addAndMakeVisible(makeWhiteNoise__textButton.get());
	makeWhiteNoise__textButton->setExplicitFocusOrder(6);
	makeWhiteNoise__textButton->setButtonText(TRANS("Make White Noise"));
	makeWhiteNoise__textButton->addListener(this);

	makeWhiteNoise__textButton->setBounds(16, 281, 150, 24);

	makeSines__textButton.reset(new juce::TextButton("makeSines button"));
	addAndMakeVisible(makeSines__textButton.get());
	makeSines__textButton->setExplicitFocusOrder(8);
	makeSines__textButton->setButtonText(TRANS("MakeSines"));
	makeSines__textButton->addListener(this);

	makeSines__textButton->setBounds(16, 505, 150, 24);

	freqs__textEditor.reset(new juce::TextEditor("freqs text editor"));
	addAndMakeVisible(freqs__textEditor.get());
	freqs__textEditor->setTooltip(TRANS("Frequencies"));
	freqs__textEditor->setExplicitFocusOrder(7);
	freqs__textEditor->setMultiLine(true);
	freqs__textEditor->setReturnKeyStartsNewLine(true);
	freqs__textEditor->setReadOnly(false);
	freqs__textEditor->setScrollbarsShown(true);
	freqs__textEditor->setCaretVisible(true);
	freqs__textEditor->setPopupMenuEnabled(true);
	freqs__textEditor->setText(juce::String());

	freqs__textEditor->setBounds(16, 360, 150, 136);

	freqs__label.reset(new juce::Label("freqs label",
		TRANS("Frequences\n")));
	addAndMakeVisible(freqs__label.get());
	freqs__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	freqs__label->setJustificationType(juce::Justification::centredLeft);
	freqs__label->setEditable(false, false, false);
	freqs__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	freqs__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	freqs__label->setBounds(16, 325, 150, 24);

	max_freq__textEditor.reset(new juce::TextEditor("max_freq text editor"));
	addAndMakeVisible(max_freq__textEditor.get());
	max_freq__textEditor->setTooltip(TRANS("max_freq"));
	max_freq__textEditor->setExplicitFocusOrder(1);
	max_freq__textEditor->setMultiLine(false);
	max_freq__textEditor->setReturnKeyStartsNewLine(false);
	max_freq__textEditor->setReadOnly(false);
	max_freq__textEditor->setScrollbarsShown(true);
	max_freq__textEditor->setCaretVisible(true);
	max_freq__textEditor->setPopupMenuEnabled(true);
	max_freq__textEditor->setText(TRANS("15000"));

	max_freq__textEditor->setBounds(16, 42, 150, 24);

	max_Freq__label.reset(new juce::Label("max_Freq label",
		TRANS("Max. frequency")));
	addAndMakeVisible(max_Freq__label.get());
	max_Freq__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	max_Freq__label->setJustificationType(juce::Justification::centredLeft);
	max_Freq__label->setEditable(false, false, false);
	max_Freq__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	max_Freq__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	max_Freq__label->setBounds(16, 13, 150, 24);

	Sample_Freq__label.reset(new juce::Label("Sample_Freq label",
		TRANS("Sample Freq")));
	addAndMakeVisible(Sample_Freq__label.get());
	Sample_Freq__label->setTooltip(TRANS("Sample_Freq"));
	Sample_Freq__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	Sample_Freq__label->setJustificationType(juce::Justification::centredLeft);
	Sample_Freq__label->setEditable(false, false, false);
	Sample_Freq__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	Sample_Freq__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	Sample_Freq__label->setBounds(16, 119, 150, 24);

	Sample_Freq__textEditor.reset(new juce::TextEditor("Sample_Freq text editor"));
	addAndMakeVisible(Sample_Freq__textEditor.get());
	Sample_Freq__textEditor->setTooltip(TRANS("Sample_Freq"));
	Sample_Freq__textEditor->setExplicitFocusOrder(3);
	Sample_Freq__textEditor->setMultiLine(false);
	Sample_Freq__textEditor->setReturnKeyStartsNewLine(false);
	Sample_Freq__textEditor->setReadOnly(false);
	Sample_Freq__textEditor->setScrollbarsShown(true);
	Sample_Freq__textEditor->setCaretVisible(true);
	Sample_Freq__textEditor->setPopupMenuEnabled(true);
	Sample_Freq__textEditor->setText(TRANS("44100"));

	Sample_Freq__textEditor->setBounds(16, 151, 150, 24);

	Nbr_samples__label.reset(new juce::Label("Nbr_samples label",
		TRANS("Nbr samples")));
	addAndMakeVisible(Nbr_samples__label.get());
	Nbr_samples__label->setTooltip(TRANS("Nbr_samples"));
	Nbr_samples__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	Nbr_samples__label->setJustificationType(juce::Justification::centredLeft);
	Nbr_samples__label->setEditable(false, false, false);
	Nbr_samples__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	Nbr_samples__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	Nbr_samples__label->setBounds(16, 192, 150, 24);

	Nbr_Samples__textEditor.reset(new juce::TextEditor("Nbr_Samples text editor"));
	addAndMakeVisible(Nbr_Samples__textEditor.get());
	Nbr_Samples__textEditor->setTooltip(TRANS("Nbr_Samples"));
	Nbr_Samples__textEditor->setExplicitFocusOrder(4);
	Nbr_Samples__textEditor->setMultiLine(false);
	Nbr_Samples__textEditor->setReturnKeyStartsNewLine(false);
	Nbr_Samples__textEditor->setReadOnly(false);
	Nbr_Samples__textEditor->setScrollbarsShown(true);
	Nbr_Samples__textEditor->setCaretVisible(true);
	Nbr_Samples__textEditor->setPopupMenuEnabled(true);
	Nbr_Samples__textEditor->setText(TRANS("4096"));

	Nbr_Samples__textEditor->setBounds(16, 224, 150, 24);

	clearPlot__textButton.reset(new juce::TextButton("clearPlot button"));
	addAndMakeVisible(clearPlot__textButton.get());
	clearPlot__textButton->setExplicitFocusOrder(9);
	clearPlot__textButton->setButtonText(TRANS("Clear Plot"));
	clearPlot__textButton->addListener(this);

	clearPlot__textButton->setBounds(192, 40, 150, 24);

	replot__textButton.reset(new juce::TextButton("replot button"));
	addAndMakeVisible(replot__textButton.get());
	replot__textButton->setExplicitFocusOrder(10);
	replot__textButton->setButtonText(TRANS("Replot Audio File"));
	replot__textButton->addListener(this);

	replot__textButton->setBounds(192, 80, 150, 24);

	fftOrder__textEditor.reset(new juce::TextEditor("fftOrder text editor"));
	addAndMakeVisible(fftOrder__textEditor.get());
	fftOrder__textEditor->setTooltip(TRANS("FFT Order"));
	fftOrder__textEditor->setExplicitFocusOrder(5);
	fftOrder__textEditor->setMultiLine(false);
	fftOrder__textEditor->setReturnKeyStartsNewLine(false);
	fftOrder__textEditor->setReadOnly(false);
	fftOrder__textEditor->setScrollbarsShown(true);
	fftOrder__textEditor->setCaretVisible(true);
	fftOrder__textEditor->setPopupMenuEnabled(true);
	fftOrder__textEditor->setText(TRANS("12"));

	fftOrder__textEditor->setBounds(192, 224, 150, 24);

	fftOrder__label.reset(new juce::Label("fftOrder label",
		TRANS("fftOrder")));
	addAndMakeVisible(fftOrder__label.get());
	fftOrder__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	fftOrder__label->setJustificationType(juce::Justification::centredLeft);
	fftOrder__label->setEditable(false, false, false);
	fftOrder__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	fftOrder__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	fftOrder__label->setBounds(192, 192, 150, 24);

	fftSize__label.reset(new juce::Label("fftSize label",
		TRANS("FFT Size")));
	addAndMakeVisible(fftSize__label.get());
	fftSize__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	fftSize__label->setJustificationType(juce::Justification::centredLeft);
	fftSize__label->setEditable(false, false, false);
	fftSize__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	fftSize__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	fftSize__label->setBounds(192, 254, 150, 24);

	fftSizeNbr__label.reset(new juce::Label("fftSizeNbr label",
		TRANS("4096")));
	addAndMakeVisible(fftSizeNbr__label.get());
	fftSizeNbr__label->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
	fftSizeNbr__label->setJustificationType(juce::Justification::centredLeft);
	fftSizeNbr__label->setEditable(false, false, false);
	fftSizeNbr__label->setColour(juce::TextEditor::textColourId, juce::Colours::black);
	fftSizeNbr__label->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

	fftSizeNbr__label->setBounds(192, 284, 150, 24);


	//[UserPreSize]
	max_freq__textEditor->setInputRestrictions(10, "1234567890");
	max_freq__textEditor->setSelectAllWhenFocused(true);
	max_freq__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);
	Sample_Freq__textEditor->setInputRestrictions(10, "1234567890");
	Sample_Freq__textEditor->setSelectAllWhenFocused(true);
	Sample_Freq__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);
	Nbr_Samples__textEditor->setInputRestrictions(10, "1234567890");
	Nbr_Samples__textEditor->setSelectAllWhenFocused(true);
	Nbr_Samples__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);
	fftOrder__textEditor->setInputRestrictions(10, "1234567890");
	fftOrder__textEditor->setSelectAllWhenFocused(true);
	fftOrder__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);

	Nbr_Samples__textEditor->onReturnKey =
		[this]
		{
			fftOrder = std::log2(Nbr_Samples__textEditor->getText().getIntValue());
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};
	Nbr_Samples__textEditor->onFocusLost =
		[this]
		{
			fftOrder = std::log2(Nbr_Samples__textEditor->getText().getIntValue());
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};

	fftOrder__textEditor->onReturnKey =
		[this]
		{
			fftOrder = fftOrder__textEditor->getText().getIntValue();
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};
	fftOrder__textEditor->onFocusLost =
		[this]
		{
			fftOrder = fftOrder__textEditor->getText().getIntValue();
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};

	fftOrder = std::log2(Nbr_Samples__textEditor->getText().getIntValue());
	fftSize = 1 << fftOrder;
	setValues(fftOrder, fftSize);
	//[/UserPreSize]

	setSize(600, 400);


	//[Constructor] You can add your own custom stuff here..
	//[/Constructor]
}

FFTCtrl::~FFTCtrl()
{
	//[Destructor_pre]. You can add your own custom destruction code here..
	//[/Destructor_pre]

	selFile__textButton = nullptr;
	makeWhiteNoise__textButton = nullptr;
	makeSines__textButton = nullptr;
	freqs__textEditor = nullptr;
	freqs__label = nullptr;
	max_freq__textEditor = nullptr;
	max_Freq__label = nullptr;
	Sample_Freq__label = nullptr;
	Sample_Freq__textEditor = nullptr;
	Nbr_samples__label = nullptr;
	Nbr_Samples__textEditor = nullptr;
	clearPlot__textButton = nullptr;
	replot__textButton = nullptr;
	fftOrder__textEditor = nullptr;
	fftOrder__label = nullptr;
	fftSize__label = nullptr;
	fftSizeNbr__label = nullptr;


	//[Destructor]. You can add your own custom destruction code here..
	//[/Destructor]
}

//==============================================================================
void FFTCtrl::paint(juce::Graphics& g)
{
	//[UserPrePaint] Add your own custom painting code here..
	//[/UserPrePaint]

	g.fillAll(juce::Colour(0xff505050));

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

void FFTCtrl::buttonClicked(juce::Button* buttonThatWasClicked)
{
	//[UserbuttonClicked_Pre]
	//[/UserbuttonClicked_Pre]

	if (buttonThatWasClicked == selFile__textButton.get())
	{
		//[UserButtonCode_selFile__textButton] -- add your button handler code here..
		module_FFT->selectFile
		(
			max_freq__textEditor->getText().getIntValue()       //unsigned int maxFreq
		);
		//[/UserButtonCode_selFile__textButton]
	}
	else if (buttonThatWasClicked == makeWhiteNoise__textButton.get())
	{
		//[UserButtonCode_makeWhiteNoise__textButton] -- add your button handler code here..
		module_FFT->makeWhiteNoise
		(
			fftOrder                                            //unsigned int fftOrder
			,
			fftSize                                             //unsigned int fftSize
			,
			max_freq__textEditor->getText().getIntValue()       //unsigned int maxFreq
			,
			Sample_Freq__textEditor->getText().getIntValue()    //unsigned int sampleRate
		);
		//[/UserButtonCode_makeWhiteNoise__textButton]
	}
	else if (buttonThatWasClicked == makeSines__textButton.get())
	{
		//[UserButtonCode_makeSines__textButton] -- add your button handler code here..
		StringArray theStrings = StringArray::fromLines(freqs__textEditor->getText());

		std::vector<double> dblFreqs;
		for (String str : theStrings)
		{
			dblFreqs.push_back(str.getDoubleValue());
		}

		module_FFT->makeSines
		(
			fftOrder                                            //unsigned int fftOrder
			,
			fftSize                                             //unsigned int fftSize
			,
			max_freq__textEditor->getText().getIntValue()       //unsigned int maxFreq
			,
			Sample_Freq__textEditor->getText().getIntValue()    //unsigned int sampleRate
			,
			dblFreqs
		);
		//[/UserButtonCode_makeSines__textButton]
	}
	else if (buttonThatWasClicked == clearPlot__textButton.get())
	{
		//[UserButtonCode_clearPlot__textButton] -- add your button handler code here..
		module_freqPlot->clearPlot();
		//[/UserButtonCode_clearPlot__textButton]
	}
	else if (buttonThatWasClicked == replot__textButton.get())
	{
		//[UserButtonCode_replot__textButton] -- add your button handler code here..
		module_FFT->loadURLIntoFFT
		(
			max_freq__textEditor->getText().getIntValue()
		);
		//[/UserButtonCode_replot__textButton]
	}

	//[UserbuttonClicked_Post]
	//[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void FFTCtrl::setValues(unsigned int fftOrder, unsigned int fftSize)
{
	fftOrder__textEditor->setText(String(fftOrder), false);
	Nbr_Samples__textEditor->setText(String(fftSize), false);
	fftSizeNbr__label->setText(String(fftSize), NotificationType::dontSendNotification);
}
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
			  virtualName="" explicitFocusOrder="2" pos="16 80 150 24" buttonText="Select Audio File"
			  connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="makeWhiteNoise button" id="5a27d84fb1070544" memberName="makeWhiteNoise__textButton"
			  virtualName="" explicitFocusOrder="6" pos="16 281 150 24" buttonText="Make White Noise"
			  connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="makeSines button" id="2c40b1d469aa1ae7" memberName="makeSines__textButton"
			  virtualName="" explicitFocusOrder="8" pos="16 505 150 24" buttonText="MakeSines"
			  connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="freqs text editor" id="ced3cc68bea5afad" memberName="freqs__textEditor"
			  virtualName="" explicitFocusOrder="7" pos="16 360 150 136" tooltip="Frequencies"
			  initialText="" multiline="1" retKeyStartsLine="1" readonly="0"
			  scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="freqs label" id="90bb1f07be81f053" memberName="freqs__label"
		 virtualName="" explicitFocusOrder="0" pos="16 325 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="Frequences&#10;" editableSingleClick="0"
		 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
		 fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="max_freq text editor" id="3d92ad17fbd24469" memberName="max_freq__textEditor"
			  virtualName="" explicitFocusOrder="1" pos="16 42 150 24" tooltip="max_freq"
			  initialText="15000" multiline="0" retKeyStartsLine="0" readonly="0"
			  scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="max_Freq label" id="795ee54aec39c02d" memberName="max_Freq__label"
		 virtualName="" explicitFocusOrder="0" pos="16 13 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="Max. frequency" editableSingleClick="0"
		 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
		 fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="Sample_Freq label" id="e27bd8b628d8b373" memberName="Sample_Freq__label"
		 virtualName="" explicitFocusOrder="0" pos="16 119 150 24" tooltip="Sample_Freq"
		 edTextCol="ff000000" edBkgCol="0" labelText="Sample Freq" editableSingleClick="0"
		 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
		 fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="Sample_Freq text editor" id="a2ed2cb609099863" memberName="Sample_Freq__textEditor"
			  virtualName="" explicitFocusOrder="3" pos="16 151 150 24" tooltip="Sample_Freq"
			  initialText="44100" multiline="0" retKeyStartsLine="0" readonly="0"
			  scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="Nbr_samples label" id="b4b3deb5d8c534b2" memberName="Nbr_samples__label"
		 virtualName="" explicitFocusOrder="0" pos="16 192 150 24" tooltip="Nbr_samples"
		 edTextCol="ff000000" edBkgCol="0" labelText="Nbr samples" editableSingleClick="0"
		 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
		 fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="Nbr_Samples text editor" id="67840ece2249397f" memberName="Nbr_Samples__textEditor"
			  virtualName="" explicitFocusOrder="4" pos="16 224 150 24" tooltip="Nbr_Samples"
			  initialText="4096" multiline="0" retKeyStartsLine="0" readonly="0"
			  scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="clearPlot button" id="655c3dd1794570bf" memberName="clearPlot__textButton"
			  virtualName="" explicitFocusOrder="9" pos="192 40 150 24" buttonText="Clear Plot"
			  connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="replot button" id="414fc8d4a90aefb7" memberName="replot__textButton"
			  virtualName="" explicitFocusOrder="10" pos="192 80 150 24" buttonText="Replot Audio File"
			  connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="fftOrder text editor" id="21a1b23e829f6d24" memberName="fftOrder__textEditor"
			  virtualName="" explicitFocusOrder="5" pos="192 224 150 24" tooltip="FFT Order"
			  initialText="12" multiline="0" retKeyStartsLine="0" readonly="0"
			  scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="fftOrder label" id="3e3a0c0ab0f7ce39" memberName="fftOrder__label"
		 virtualName="" explicitFocusOrder="0" pos="192 192 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="fftOrder" editableSingleClick="0" editableDoubleClick="0"
		 focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
		 kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="fftSize label" id="c26736a857041fd5" memberName="fftSize__label"
		 virtualName="" explicitFocusOrder="0" pos="192 254 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="FFT Size" editableSingleClick="0" editableDoubleClick="0"
		 focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
		 kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="fftSizeNbr label" id="f6dc9c671d2b180c" memberName="fftSizeNbr__label"
		 virtualName="" explicitFocusOrder="0" pos="192 284 150 24" edTextCol="ff000000"
		 edBkgCol="0" labelText="4096" editableSingleClick="0" editableDoubleClick="0"
		 focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
		 kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

