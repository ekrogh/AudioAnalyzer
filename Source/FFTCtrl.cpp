/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.12

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "SpectrogramComponent.h"
//[/Headers]

#include "FFTCtrl.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
FFTCtrl::FFTCtrl (std::shared_ptr<FFTModule> FFT, std::shared_ptr<SpectrogramComponent> SC, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<freqPlotModule> FPlot)
    : ptrFFTModule(FFT),
      ptrSpectrogramComponent(SC),
      sharedAudioDeviceManager(SADM),
      module_freqPlot(FPlot)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    selFile__textButton.reset (new juce::TextButton ("select file button"));
    addAndMakeVisible (selFile__textButton.get());
    selFile__textButton->setExplicitFocusOrder (2);
    selFile__textButton->setButtonText (TRANS ("Select Audio File"));
    selFile__textButton->addListener (this);

    selFile__textButton->setBounds (16, 80, 150, 24);

    makeWhiteNoise__textButton.reset (new juce::TextButton ("makeWhiteNoise button"));
    addAndMakeVisible (makeWhiteNoise__textButton.get());
    makeWhiteNoise__textButton->setExplicitFocusOrder (6);
    makeWhiteNoise__textButton->setButtonText (TRANS ("Make White Noise"));
    makeWhiteNoise__textButton->addListener (this);

    makeWhiteNoise__textButton->setBounds (16, 281, 150, 24);

    makeSines__textButton.reset (new juce::TextButton ("makeSines button"));
    addAndMakeVisible (makeSines__textButton.get());
    makeSines__textButton->setExplicitFocusOrder (8);
    makeSines__textButton->setButtonText (TRANS ("MakeSines"));
    makeSines__textButton->addListener (this);

    makeSines__textButton->setBounds (16, 505, 150, 24);

    frequencies__textEditor.reset (new juce::TextEditor ("frequencies text editor"));
    addAndMakeVisible (frequencies__textEditor.get());
    frequencies__textEditor->setTooltip (TRANS ("Frequencies"));
    frequencies__textEditor->setExplicitFocusOrder (7);
    frequencies__textEditor->setMultiLine (true);
    frequencies__textEditor->setReturnKeyStartsNewLine (true);
    frequencies__textEditor->setReadOnly (false);
    frequencies__textEditor->setScrollbarsShown (true);
    frequencies__textEditor->setCaretVisible (true);
    frequencies__textEditor->setPopupMenuEnabled (true);
    frequencies__textEditor->setText (juce::String());

    frequencies__textEditor->setBounds (16, 360, 150, 136);

    frequencies__label.reset (new juce::Label ("frequencies label",
                                               TRANS ("Frequencies")));
    addAndMakeVisible (frequencies__label.get());
    frequencies__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    frequencies__label->setJustificationType (juce::Justification::centredLeft);
    frequencies__label->setEditable (false, false, false);
    frequencies__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    frequencies__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    frequencies__label->setBounds (16, 325, 150, 24);

    max_freq__textEditor.reset (new juce::TextEditor ("max_freq text editor"));
    addAndMakeVisible (max_freq__textEditor.get());
    max_freq__textEditor->setTooltip (TRANS ("max_freq"));
    max_freq__textEditor->setExplicitFocusOrder (1);
    max_freq__textEditor->setMultiLine (false);
    max_freq__textEditor->setReturnKeyStartsNewLine (false);
    max_freq__textEditor->setReadOnly (false);
    max_freq__textEditor->setScrollbarsShown (true);
    max_freq__textEditor->setCaretVisible (true);
    max_freq__textEditor->setPopupMenuEnabled (true);
    max_freq__textEditor->setText (TRANS ("15000"));

    max_freq__textEditor->setBounds (16, 42, 150, 24);

    max_Freq__label.reset (new juce::Label ("max_Freq label",
                                            TRANS ("Max. frequency")));
    addAndMakeVisible (max_Freq__label.get());
    max_Freq__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    max_Freq__label->setJustificationType (juce::Justification::centredLeft);
    max_Freq__label->setEditable (false, false, false);
    max_Freq__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    max_Freq__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    max_Freq__label->setBounds (16, 13, 150, 24);

    Sample_Freq__label.reset (new juce::Label ("Sample_Freq label",
                                               TRANS ("Sample Freq")));
    addAndMakeVisible (Sample_Freq__label.get());
    Sample_Freq__label->setTooltip (TRANS ("Sample_Freq"));
    Sample_Freq__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    Sample_Freq__label->setJustificationType (juce::Justification::centredLeft);
    Sample_Freq__label->setEditable (false, false, false);
    Sample_Freq__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    Sample_Freq__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    Sample_Freq__label->setBounds (16, 119, 150, 24);

    Sample_Freq__textEditor.reset (new juce::TextEditor ("Sample_Freq text editor"));
    addAndMakeVisible (Sample_Freq__textEditor.get());
    Sample_Freq__textEditor->setTooltip (TRANS ("Sample_Freq"));
    Sample_Freq__textEditor->setExplicitFocusOrder (3);
    Sample_Freq__textEditor->setMultiLine (false);
    Sample_Freq__textEditor->setReturnKeyStartsNewLine (false);
    Sample_Freq__textEditor->setReadOnly (false);
    Sample_Freq__textEditor->setScrollbarsShown (true);
    Sample_Freq__textEditor->setCaretVisible (true);
    Sample_Freq__textEditor->setPopupMenuEnabled (true);
    Sample_Freq__textEditor->setText (TRANS ("44100"));

    Sample_Freq__textEditor->setBounds (16, 151, 150, 24);

    Nbr_samples__label.reset (new juce::Label ("Nbr_samples label",
                                               TRANS ("Nbr samples")));
    addAndMakeVisible (Nbr_samples__label.get());
    Nbr_samples__label->setTooltip (TRANS ("Nbr_samples"));
    Nbr_samples__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    Nbr_samples__label->setJustificationType (juce::Justification::centredLeft);
    Nbr_samples__label->setEditable (false, false, false);
    Nbr_samples__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    Nbr_samples__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    Nbr_samples__label->setBounds (16, 192, 150, 24);

    Nbr_Samples__textEditor.reset (new juce::TextEditor ("Nbr_Samples text editor"));
    addAndMakeVisible (Nbr_Samples__textEditor.get());
    Nbr_Samples__textEditor->setTooltip (TRANS ("Nbr_Samples"));
    Nbr_Samples__textEditor->setExplicitFocusOrder (4);
    Nbr_Samples__textEditor->setMultiLine (false);
    Nbr_Samples__textEditor->setReturnKeyStartsNewLine (false);
    Nbr_Samples__textEditor->setReadOnly (false);
    Nbr_Samples__textEditor->setScrollbarsShown (true);
    Nbr_Samples__textEditor->setCaretVisible (true);
    Nbr_Samples__textEditor->setPopupMenuEnabled (true);
    Nbr_Samples__textEditor->setText (TRANS ("1024"));

    Nbr_Samples__textEditor->setBounds (16, 224, 150, 24);

    clearPlot__textButton.reset (new juce::TextButton ("clearPlot button"));
    addAndMakeVisible (clearPlot__textButton.get());
    clearPlot__textButton->setExplicitFocusOrder (9);
    clearPlot__textButton->setButtonText (TRANS ("Clear Plot"));
    clearPlot__textButton->addListener (this);

    clearPlot__textButton->setBounds (192, 40, 150, 24);

    replot__textButton.reset (new juce::TextButton ("replot button"));
    addAndMakeVisible (replot__textButton.get());
    replot__textButton->setExplicitFocusOrder (10);
    replot__textButton->setButtonText (TRANS ("Replot Audio File"));
    replot__textButton->addListener (this);

    replot__textButton->setBounds (192, 80, 150, 24);

    fftOrder__textEditor.reset (new juce::TextEditor ("fftOrder text editor"));
    addAndMakeVisible (fftOrder__textEditor.get());
    fftOrder__textEditor->setTooltip (TRANS ("FFT Order"));
    fftOrder__textEditor->setExplicitFocusOrder (5);
    fftOrder__textEditor->setMultiLine (false);
    fftOrder__textEditor->setReturnKeyStartsNewLine (false);
    fftOrder__textEditor->setReadOnly (false);
    fftOrder__textEditor->setScrollbarsShown (true);
    fftOrder__textEditor->setCaretVisible (true);
    fftOrder__textEditor->setPopupMenuEnabled (true);
    fftOrder__textEditor->setText (TRANS ("10"));

    fftOrder__textEditor->setBounds (192, 183, 150, 24);

    fftOrder__label.reset (new juce::Label ("fftOrder label",
                                            TRANS ("fftOrder")));
    addAndMakeVisible (fftOrder__label.get());
    fftOrder__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    fftOrder__label->setJustificationType (juce::Justification::centredLeft);
    fftOrder__label->setEditable (false, false, false);
    fftOrder__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    fftOrder__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    fftOrder__label->setBounds (192, 151, 150, 24);

    fftSize__label.reset (new juce::Label ("fftSize label",
                                           TRANS ("FFT Size")));
    addAndMakeVisible (fftSize__label.get());
    fftSize__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    fftSize__label->setJustificationType (juce::Justification::centredLeft);
    fftSize__label->setEditable (false, false, false);
    fftSize__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    fftSize__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    fftSize__label->setBounds (192, 213, 150, 24);

    fftSizeNbr__label.reset (new juce::Label ("fftSizeNbr label",
                                              TRANS ("1024")));
    addAndMakeVisible (fftSizeNbr__label.get());
    fftSizeNbr__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    fftSizeNbr__label->setJustificationType (juce::Justification::centredLeft);
    fftSizeNbr__label->setEditable (false, false, false);
    fftSizeNbr__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    fftSizeNbr__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    fftSizeNbr__label->setBounds (192, 236, 150, 24);

    fftWindows__textButton.reset (new juce::TextButton ("fftWindows button"));
    addAndMakeVisible (fftWindows__textButton.get());
    fftWindows__textButton->setButtonText (TRANS ("FFT of windows"));
    fftWindows__textButton->addListener (this);

    fftWindows__textButton->setBounds (192, 119, 150, 24);

    spectrumOfAudioFile__toggleButton.reset (new juce::ToggleButton ("spectrumOfAudioFile toggle button"));
    addAndMakeVisible (spectrumOfAudioFile__toggleButton.get());
    spectrumOfAudioFile__toggleButton->setButtonText (TRANS ("Make plots of audio file  "));
    spectrumOfAudioFile__toggleButton->setConnectedEdges (juce::Button::ConnectedOnTop);
    spectrumOfAudioFile__toggleButton->setRadioGroupId (1);
    spectrumOfAudioFile__toggleButton->addListener (this);

    spectrumOfAudioFile__toggleButton->setBounds (192, 357, 158, 24);

    makeSpectrumOfInput__toggleButton.reset (new juce::ToggleButton ("makeSpectrumOfInput toggle button"));
    addAndMakeVisible (makeSpectrumOfInput__toggleButton.get());
    makeSpectrumOfInput__toggleButton->setButtonText (TRANS ("Make spectrum of input"));
    makeSpectrumOfInput__toggleButton->setConnectedEdges (juce::Button::ConnectedOnBottom);
    makeSpectrumOfInput__toggleButton->setRadioGroupId (1);
    makeSpectrumOfInput__toggleButton->addListener (this);
    makeSpectrumOfInput__toggleButton->setToggleState (true, juce::dontSendNotification);

    makeSpectrumOfInput__toggleButton->setBounds (192, 280, 150, 24);

    autoSwitchToInput__toggleButton.reset (new juce::ToggleButton ("autoSwitchToInput toggle button"));
    addAndMakeVisible (autoSwitchToInput__toggleButton.get());
    autoSwitchToInput__toggleButton->setButtonText (TRANS ("Auto switch to input"));
    autoSwitchToInput__toggleButton->addListener (this);

    autoSwitchToInput__toggleButton->setBounds (192, 397, 150, 24);

    use50HzFilter__toggleButton.reset (new juce::ToggleButton ("use50HzFilter toggle button"));
    addAndMakeVisible (use50HzFilter__toggleButton.get());
    use50HzFilter__toggleButton->setButtonText (TRANS ("Use 50 Hz filter"));
    use50HzFilter__toggleButton->setConnectedEdges (juce::Button::ConnectedOnBottom);
    use50HzFilter__toggleButton->setRadioGroupId (2);
    use50HzFilter__toggleButton->addListener (this);

    use50HzFilter__toggleButton->setBounds (192, 436, 150, 24);

    use60HzFilter__toggleButton.reset (new juce::ToggleButton ("use60HzFilter toggle button"));
    addAndMakeVisible (use60HzFilter__toggleButton.get());
    use60HzFilter__toggleButton->setButtonText (TRANS ("Use 60 Hz filter"));
    use60HzFilter__toggleButton->setConnectedEdges (juce::Button::ConnectedOnTop | juce::Button::ConnectedOnBottom);
    use60HzFilter__toggleButton->setRadioGroupId (2);
    use60HzFilter__toggleButton->addListener (this);

    use60HzFilter__toggleButton->setBounds (192, 467, 150, 24);

    useNoFilter__toggleButton.reset (new juce::ToggleButton ("useNoFilter toggle button"));
    addAndMakeVisible (useNoFilter__toggleButton.get());
    useNoFilter__toggleButton->setButtonText (TRANS ("Use no filter"));
    useNoFilter__toggleButton->setConnectedEdges (juce::Button::ConnectedOnTop);
    useNoFilter__toggleButton->setRadioGroupId (2);
    useNoFilter__toggleButton->addListener (this);
    useNoFilter__toggleButton->setToggleState (true, juce::dontSendNotification);

    useNoFilter__toggleButton->setBounds (192, 507, 150, 24);

    makeFFtRealTimeChartPlot__toggleButton.reset (new juce::ToggleButton ("makeFFtRealTimeChartPlot toggle button"));
    addAndMakeVisible (makeFFtRealTimeChartPlot__toggleButton.get());
    makeFFtRealTimeChartPlot__toggleButton->setButtonText (TRANS ("Make FFT real time chart plot"));
    makeFFtRealTimeChartPlot__toggleButton->addListener (this);

    makeFFtRealTimeChartPlot__toggleButton->setBounds (192, 315, 183, 24);


    //[UserPreSize]
	max_freq__textEditor->setInputRestrictions(10, "1234567890.,");
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
			fftOrder =
				static_cast<unsigned int>(std::log2(Nbr_Samples__textEditor->getText().getIntValue()));
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};
	Nbr_Samples__textEditor->onFocusLost =
		[this]
		{
			fftOrder =
				static_cast<unsigned int>(std::log2(Nbr_Samples__textEditor->getText().getIntValue()));
			fftSize = 1 << fftOrder;
			setValues(fftOrder, fftSize);
		};
	//Nbr_Samples__textEditor->onTextChange =
	//	[this]
	//	{
	//		fftOrder =
	//			static_cast<unsigned int>(std::log2(Nbr_Samples__textEditor->getText().getIntValue()));
	//		fftSize = 1 << fftOrder;
	//		setValues(fftOrder, fftSize);
	//	};

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
	//fftOrder__textEditor->onTextChange =
	//	[this]
	//	{
	//		fftOrder = fftOrder__textEditor->getText().getIntValue();
	//		fftSize = 1 << fftOrder;
	//		setValues(fftOrder, fftSize);
	//	};

	//max Freq In FFT Chart
	max_freq__textEditor->onReturnKey =
		[this]
		{
			double cur_max_freq__textEditor_value =
				max_freq__textEditor->getText().getDoubleValue();
			if (cur_max_freq__textEditor_value > curSampleFreqHz)
			{
				max_freq__textEditor->
					setText(juce::String(curSampleFreqHz));

				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(curSampleFreqHz);

				maxChartPlotFreq = curSampleFreqHz;
			}
			else
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(cur_max_freq__textEditor_value);

				maxChartPlotFreq = cur_max_freq__textEditor_value;
			}
		};
	max_freq__textEditor->onFocusLost =
		[this]
		{
			double cur_max_freq__textEditor_value =
				max_freq__textEditor->getText().getDoubleValue();
			if (cur_max_freq__textEditor_value > curSampleFreqHz)
			{
				max_freq__textEditor->
					setText(juce::String(curSampleFreqHz));

				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(curSampleFreqHz);

				maxChartPlotFreq = curSampleFreqHz;
			}
			else
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(cur_max_freq__textEditor_value);

				maxChartPlotFreq = cur_max_freq__textEditor_value;
			}
		};

	//max Freq In FFT Chart
	Sample_Freq__textEditor->onReturnKey =
		[this]
		{
			stopStreamAudio();

			double cur_Sample_Freq__textEditor_double_value =
				Sample_Freq__textEditor->getText().getDoubleValue();

			double cur_max_freq__textEditor_double_value =
				max_freq__textEditor->getText().getDoubleValue();

			if (
				cur_max_freq__textEditor_double_value
					>
				cur_Sample_Freq__textEditor_double_value
				)
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(curSampleFreqHz);

				maxChartPlotFreq = curSampleFreqHz;
			}
			else
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(cur_max_freq__textEditor_double_value);

				maxChartPlotFreq = cur_max_freq__textEditor_double_value;
			}
		};
	Sample_Freq__textEditor->onFocusLost =
		[this]
		{
			stopStreamAudio();

			double cur_Sample_Freq__textEditor_double_value =
				Sample_Freq__textEditor->getText().getDoubleValue();

			double cur_max_freq__textEditor_double_value =
				max_freq__textEditor->getText().getDoubleValue();

			if (
				cur_max_freq__textEditor_double_value
					>
				cur_Sample_Freq__textEditor_double_value
				)
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(curSampleFreqHz);

				maxChartPlotFreq = curSampleFreqHz;
			}
			else
			{
				ptrSpectrogramComponent->
					setMaxFreqInRealTimeFftChartPlot(cur_max_freq__textEditor_double_value);

				maxChartPlotFreq = cur_max_freq__textEditor_double_value;
			}
		};

	fftOrder =
		static_cast<unsigned int>(std::log2(Nbr_Samples__textEditor->getText().getIntValue()));
	fftSize = 1 << fftOrder;
	setValues(fftOrder, fftSize);

    //[/UserPreSize]

    setSize (600, 400);


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
    frequencies__textEditor = nullptr;
    frequencies__label = nullptr;
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
    fftWindows__textButton = nullptr;
    spectrumOfAudioFile__toggleButton = nullptr;
    makeSpectrumOfInput__toggleButton = nullptr;
    autoSwitchToInput__toggleButton = nullptr;
    use50HzFilter__toggleButton = nullptr;
    use60HzFilter__toggleButton = nullptr;
    useNoFilter__toggleButton = nullptr;
    makeFFtRealTimeChartPlot__toggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void FFTCtrl::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

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

void FFTCtrl::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == selFile__textButton.get())
    {
        //[UserButtonCode_selFile__textButton] -- add your button handler code here..
		ptrFFTModule->selectFile
		(
			max_freq__textEditor->getText().getIntValue()       //unsigned int maxFreq
			,
			fftOrder__textEditor.get()
			,
			Nbr_Samples__textEditor.get()
			,
			fftSizeNbr__label.get()
		);
        //[/UserButtonCode_selFile__textButton]
    }
    else if (buttonThatWasClicked == makeWhiteNoise__textButton.get())
    {
        //[UserButtonCode_makeWhiteNoise__textButton] -- add your button handler code here..
		stopStreamAudio();

		ptrFFTModule->makeWhiteNoise
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
		stopStreamAudio();

		StringArray theStrings = StringArray::fromLines(frequencies__textEditor->getText());

		std::vector<double> dblFreqs;
		for (String str : theStrings)
		{
			dblFreqs.push_back(str.getDoubleValue());
		}

		ptrFFTModule->makeSines
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
		stopStreamAudio();

		ptrFFTModule->loadURLIntoFFT
		(
			max_freq__textEditor->getText().getDoubleValue()    //double maxFreq
			,
			fftOrder__textEditor.get()
			,
			Nbr_Samples__textEditor.get()
			,
			fftSizeNbr__label.get()
		);
        //[/UserButtonCode_replot__textButton]
    }
    else if (buttonThatWasClicked == fftWindows__textButton.get())
    {
        //[UserButtonCode_fftWindows__textButton] -- add your button handler code here..
		stopStreamAudio();

		ptrFFTModule->makeWindows
		(
			fftOrder                                            //unsigned int fftOrder
			,
			fftSize                                             //unsigned int fftSize
			,
			max_freq__textEditor->getText().getDoubleValue()    //double maxFreq
			,
			Sample_Freq__textEditor->getText().getIntValue()    //unsigned int sampleRate
		);
        //[/UserButtonCode_fftWindows__textButton]
    }
    else if (buttonThatWasClicked == spectrumOfAudioFile__toggleButton.get())
    {
        //[UserButtonCode_spectrumOfAudioFile__toggleButton] -- add your button handler code here..
        //[/UserButtonCode_spectrumOfAudioFile__toggleButton]
    }
    else if (buttonThatWasClicked == makeSpectrumOfInput__toggleButton.get())
    {
        //[UserButtonCode_makeSpectrumOfInput__toggleButton] -- add your button handler code here..
        //[/UserButtonCode_makeSpectrumOfInput__toggleButton]
    }
    else if (buttonThatWasClicked == autoSwitchToInput__toggleButton.get())
    {
        //[UserButtonCode_autoSwitchToInput__toggleButton] -- add your button handler code here..
		ptrSpectrogramComponent->
			setAutoSwitchToInput(autoSwitchToInput__toggleButton->getToggleState());
        //[/UserButtonCode_autoSwitchToInput__toggleButton]
    }
    else if (buttonThatWasClicked == use50HzFilter__toggleButton.get())
    {
        //[UserButtonCode_use50HzFilter__toggleButton] -- add your button handler code here..
		if (use50HzFilter__toggleButton->getToggleState())
		{
			ptrSpectrogramComponent->setFilterToUse(filter50Hz);
		}
        //[/UserButtonCode_use50HzFilter__toggleButton]
    }
    else if (buttonThatWasClicked == use60HzFilter__toggleButton.get())
    {
        //[UserButtonCode_use60HzFilter__toggleButton] -- add your button handler code here..
		if (use60HzFilter__toggleButton->getToggleState())
		{
			ptrSpectrogramComponent->setFilterToUse(filter60Hz);
		}
        //[/UserButtonCode_use60HzFilter__toggleButton]
    }
    else if (buttonThatWasClicked == useNoFilter__toggleButton.get())
    {
        //[UserButtonCode_useNoFilter__toggleButton] -- add your button handler code here..
		if (useNoFilter__toggleButton->getToggleState())
		{
			ptrSpectrogramComponent->setFilterToUse(noFilter);
		}
        //[/UserButtonCode_useNoFilter__toggleButton]
    }
    else if (buttonThatWasClicked == makeFFtRealTimeChartPlot__toggleButton.get())
    {
        //[UserButtonCode_makeFFtRealTimeChartPlot__toggleButton] -- add your button handler code here..
		ptrSpectrogramComponent->
			setDoRealTimeFftChartPlot(makeFFtRealTimeChartPlot__toggleButton->getToggleState());
        //[/UserButtonCode_makeFFtRealTimeChartPlot__toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void FFTCtrl::updateSampleRate(double newSampleRate)
{
	// Update the TextEditor or Label with the new sample rate
	Sample_Freq__textEditor->setText(juce::String(newSampleRate), juce::NotificationType::dontSendNotification);

	curSampleFreqHz = newSampleRate;

	if (max_freq__textEditor->getText().getDoubleValue() > curSampleFreqHz)
	{
		max_freq__textEditor->
			setText
			(
				juce::String(curSampleFreqHz)
				,
				juce::NotificationType::dontSendNotification
			);
	}
}

void FFTCtrl::getFftOrderAndFftSize(unsigned int& fftOrderNew, unsigned int& fftSizeNew)
{
	fftOrderNew = fftOrder;
	fftSizeNew = fftSize;
}

void FFTCtrl::setValues(unsigned int fftOrder, unsigned int fftSize)
{
	if (ptrSpectrogramComponent != nullptr)
	{
		ptrSpectrogramComponent->setFftOrderAndFftSize(fftOrder, fftSize);
	}

	fftOrder__textEditor->setText(String(fftOrder), false);
	Nbr_Samples__textEditor->setText(String(fftSize), false);
	fftSizeNbr__label->setText(String(fftSize), NotificationType::dontSendNotification);
}

void FFTCtrl::stopStreamAudio()
{
	ptrSpectrogramComponent->shutDownIO();
	ptrFFTModule->shutDownIO();

	spectrumOfAudioFile__toggleButton->
		setRadioGroupId(0);
	makeSpectrumOfInput__toggleButton->
		setRadioGroupId(0);
	spectrumOfAudioFile__toggleButton->
		setToggleState(false, false);
	makeSpectrumOfInput__toggleButton->
		setToggleState(false, false);
}

void FFTCtrl::radioButtonEnable()
{
	spectrumOfAudioFile__toggleButton->
		setRadioGroupId(1);
	makeSpectrumOfInput__toggleButton->
		setRadioGroupId(1);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="FFTCtrl" componentName=""
                 parentClasses="public juce::Component" constructorParams="std::shared_ptr&lt;FFTModule&gt; FFT, std::shared_ptr&lt;SpectrogramComponent&gt; SC, std::shared_ptr&lt;AudioDeviceManager&gt; SADM, std::shared_ptr&lt;freqPlotModule&gt; FPlot"
                 variableInitialisers="ptrFFTModule(FFT)&#10;ptrSpectrogramComponent(SC)&#10;sharedAudioDeviceManager(SADM)&#10;module_freqPlot(FPlot)&#10;"
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
  <TEXTEDITOR name="frequencies text editor" id="ced3cc68bea5afad" memberName="frequencies__textEditor"
              virtualName="" explicitFocusOrder="7" pos="16 360 150 136" tooltip="Frequencies"
              initialText="" multiline="1" retKeyStartsLine="1" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="frequencies label" id="90bb1f07be81f053" memberName="frequencies__label"
         virtualName="" explicitFocusOrder="0" pos="16 325 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Frequencies" editableSingleClick="0"
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
              initialText="1024" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="clearPlot button" id="655c3dd1794570bf" memberName="clearPlot__textButton"
              virtualName="" explicitFocusOrder="9" pos="192 40 150 24" buttonText="Clear Plot"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="replot button" id="414fc8d4a90aefb7" memberName="replot__textButton"
              virtualName="" explicitFocusOrder="10" pos="192 80 150 24" buttonText="Replot Audio File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="fftOrder text editor" id="21a1b23e829f6d24" memberName="fftOrder__textEditor"
              virtualName="" explicitFocusOrder="5" pos="192 183 150 24" tooltip="FFT Order"
              initialText="10" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="fftOrder label" id="3e3a0c0ab0f7ce39" memberName="fftOrder__label"
         virtualName="" explicitFocusOrder="0" pos="192 151 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="fftOrder" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="fftSize label" id="c26736a857041fd5" memberName="fftSize__label"
         virtualName="" explicitFocusOrder="0" pos="192 213 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="FFT Size" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="fftSizeNbr label" id="f6dc9c671d2b180c" memberName="fftSizeNbr__label"
         virtualName="" explicitFocusOrder="0" pos="192 236 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="1024" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="fftWindows button" id="9c0c45ece1379aa3" memberName="fftWindows__textButton"
              virtualName="" explicitFocusOrder="0" pos="192 119 150 24" buttonText="FFT of windows"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="spectrumOfAudioFile toggle button" id="1275ad335d939b3e"
                memberName="spectrumOfAudioFile__toggleButton" virtualName=""
                explicitFocusOrder="0" pos="192 357 158 24" buttonText="Make plots of audio file  "
                connectedEdges="4" needsCallback="1" radioGroupId="1" state="0"/>
  <TOGGLEBUTTON name="makeSpectrumOfInput toggle button" id="a4f23f06626db462"
                memberName="makeSpectrumOfInput__toggleButton" virtualName=""
                explicitFocusOrder="0" pos="192 280 150 24" buttonText="Make spectrum of input"
                connectedEdges="8" needsCallback="1" radioGroupId="1" state="1"/>
  <TOGGLEBUTTON name="autoSwitchToInput toggle button" id="f79b3224172a8f2d"
                memberName="autoSwitchToInput__toggleButton" virtualName="" explicitFocusOrder="0"
                pos="192 397 150 24" buttonText="Auto switch to input" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="use50HzFilter toggle button" id="86160e89f8af914" memberName="use50HzFilter__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="192 436 150 24" buttonText="Use 50 Hz filter"
                connectedEdges="8" needsCallback="1" radioGroupId="2" state="0"/>
  <TOGGLEBUTTON name="use60HzFilter toggle button" id="8fe0a8bef11d661e" memberName="use60HzFilter__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="192 467 150 24" buttonText="Use 60 Hz filter"
                connectedEdges="12" needsCallback="1" radioGroupId="2" state="0"/>
  <TOGGLEBUTTON name="useNoFilter toggle button" id="a4acb1f6eeda074a" memberName="useNoFilter__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="192 507 150 24" buttonText="Use no filter"
                connectedEdges="4" needsCallback="1" radioGroupId="2" state="1"/>
  <TOGGLEBUTTON name="makeFFtRealTimeChartPlot toggle button" id="e0ba875e06b99110"
                memberName="makeFFtRealTimeChartPlot__toggleButton" virtualName=""
                explicitFocusOrder="0" pos="192 315 183 24" buttonText="Make FFT real time chart plot"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

