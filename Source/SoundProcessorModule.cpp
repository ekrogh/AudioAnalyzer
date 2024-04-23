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
#include "AudioAnalyzerGlobalEnums.h"
//[/Headers]

#include "SoundProcessorModule.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SoundProcessorModule::SoundProcessorModule (std::shared_ptr<PlotModule> ptr_module_Plot, std::shared_ptr<AudioDeviceManager> SADM, std::shared_ptr<AudioRecorderModule> MAR)
    : module_Plot(ptr_module_Plot),
      AudioAppComponent(*SADM),
      Thread("Freq. shifter"),
      module_AudioRecording(MAR)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    setName ("Sound Synth And Analyze Module");
    runNewMeasurement__toggleButton.reset (new juce::ToggleButton ("run New Measurement toggle button"));
    addAndMakeVisible (runNewMeasurement__toggleButton.get());
    runNewMeasurement__toggleButton->setExplicitFocusOrder (5);
    runNewMeasurement__toggleButton->setButtonText (TRANS ("Run new measurement"));

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

    juce__label3->setBounds (8, 146, 128, 24);

    timeToRun__label.reset (new juce::Label ("time To Run label",
                                             TRANS ("Time remaining to run [Min]:")));
    addAndMakeVisible (timeToRun__label.get());
    timeToRun__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    timeToRun__label->setJustificationType (juce::Justification::centredLeft);
    timeToRun__label->setEditable (false, false, false);
    timeToRun__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    timeToRun__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    qurFreq__label.reset (new juce::Label ("cur. freq. label",
                                           TRANS ("Current frequency [Hz]:")));
    addAndMakeVisible (qurFreq__label.get());
    qurFreq__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    qurFreq__label->setJustificationType (juce::Justification::centredLeft);
    qurFreq__label->setEditable (false, false, false);
    qurFreq__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    qurFreq__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juce__label5.reset (new juce::Label ("new label",
                                         TRANS ("Min. frequency [Hz]")));
    addAndMakeVisible (juce__label5.get());
    juce__label5->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juce__label5->setJustificationType (juce::Justification::centredLeft);
    juce__label5->setEditable (false, false, false);
    juce__label5->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juce__label5->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

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

    pause__toggleButton.reset (new juce::ToggleButton ("pause toggle button"));
    addAndMakeVisible (pause__toggleButton.get());
    pause__toggleButton->setExplicitFocusOrder (6);
    pause__toggleButton->setButtonText (TRANS ("Pause"));

    Deleteoldmeasurements__textButton.reset (new juce::TextButton ("Deleteoldmeasurements__textButton"));
    addAndMakeVisible (Deleteoldmeasurements__textButton.get());
    Deleteoldmeasurements__textButton->setExplicitFocusOrder (7);
    Deleteoldmeasurements__textButton->setButtonText (TRANS ("Delete old measurements"));

    minFreq__textEditor.reset (new juce::TextEditor ("min. freq. text editor"));
    addAndMakeVisible (minFreq__textEditor.get());
    minFreq__textEditor->setTooltip (TRANS ("min. freq."));
    minFreq__textEditor->setExplicitFocusOrder (2);
    minFreq__textEditor->setMultiLine (false);
    minFreq__textEditor->setReturnKeyStartsNewLine (false);
    minFreq__textEditor->setReadOnly (false);
    minFreq__textEditor->setScrollbarsShown (false);
    minFreq__textEditor->setCaretVisible (true);
    minFreq__textEditor->setPopupMenuEnabled (true);
    minFreq__textEditor->setText (juce::String());

    maxFreq__textEditor.reset (new juce::TextEditor ("max freq. text editor"));
    addAndMakeVisible (maxFreq__textEditor.get());
    maxFreq__textEditor->setExplicitFocusOrder (1);
    maxFreq__textEditor->setMultiLine (false);
    maxFreq__textEditor->setReturnKeyStartsNewLine (false);
    maxFreq__textEditor->setReadOnly (false);
    maxFreq__textEditor->setScrollbarsShown (true);
    maxFreq__textEditor->setCaretVisible (true);
    maxFreq__textEditor->setPopupMenuEnabled (true);
    maxFreq__textEditor->setText (juce::String());

    deltaFreq__textEditor.reset (new juce::TextEditor ("delta freq. text editor"));
    addAndMakeVisible (deltaFreq__textEditor.get());
    deltaFreq__textEditor->setExplicitFocusOrder (3);
    deltaFreq__textEditor->setMultiLine (false);
    deltaFreq__textEditor->setReturnKeyStartsNewLine (false);
    deltaFreq__textEditor->setReadOnly (false);
    deltaFreq__textEditor->setScrollbarsShown (true);
    deltaFreq__textEditor->setCaretVisible (true);
    deltaFreq__textEditor->setPopupMenuEnabled (true);
    deltaFreq__textEditor->setText (juce::String());

    deltaFreq__textEditor->setBounds (8, 176, 150, 24);

    deltaTime__textEditor.reset (new juce::TextEditor ("delta time text editor"));
    addAndMakeVisible (deltaTime__textEditor.get());
    deltaTime__textEditor->setExplicitFocusOrder (4);
    deltaTime__textEditor->setMultiLine (false);
    deltaTime__textEditor->setReturnKeyStartsNewLine (false);
    deltaTime__textEditor->setReadOnly (false);
    deltaTime__textEditor->setScrollbarsShown (true);
    deltaTime__textEditor->setCaretVisible (true);
    deltaTime__textEditor->setPopupMenuEnabled (true);
    deltaTime__textEditor->setText (juce::String());

    deltaTime__textEditor->setBounds (8, 240, 150, 24);

    save__textButton.reset (new juce::TextButton ("save button"));
    addAndMakeVisible (save__textButton.get());
    save__textButton->setExplicitFocusOrder (8);
    save__textButton->setButtonText (TRANS ("Save measurements to file"));
    save__textButton->addListener (this);

    save__textButton->setBounds (176, 208, 183, 24);

    read__textButton.reset (new juce::TextButton ("read button"));
    addAndMakeVisible (read__textButton.get());
    read__textButton->setExplicitFocusOrder (9);
    read__textButton->setButtonText (TRANS ("Read measurements from file"));
    read__textButton->addListener (this);

    read__textButton->setBounds (376, 208, 199, 24);

    timeToRunTotally__textEditor.reset (new juce::TextEditor ("time To Run Totally text editor"));
    addAndMakeVisible (timeToRunTotally__textEditor.get());
    timeToRunTotally__textEditor->setMultiLine (false);
    timeToRunTotally__textEditor->setReturnKeyStartsNewLine (false);
    timeToRunTotally__textEditor->setReadOnly (true);
    timeToRunTotally__textEditor->setScrollbarsShown (false);
    timeToRunTotally__textEditor->setCaretVisible (false);
    timeToRunTotally__textEditor->setPopupMenuEnabled (false);
    timeToRunTotally__textEditor->setColour (juce::TextEditor::textColourId, juce::Colours::white);
    timeToRunTotally__textEditor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    timeToRunTotally__textEditor->setText (TRANS ("-"));

    timeToRunTotally__textEditor->setBounds (376, 16, 150, 24);

    timeToRunValue__textEditor.reset (new juce::TextEditor ("time To Run Value"));
    addAndMakeVisible (timeToRunValue__textEditor.get());
    timeToRunValue__textEditor->setMultiLine (false);
    timeToRunValue__textEditor->setReturnKeyStartsNewLine (false);
    timeToRunValue__textEditor->setReadOnly (true);
    timeToRunValue__textEditor->setScrollbarsShown (false);
    timeToRunValue__textEditor->setCaretVisible (false);
    timeToRunValue__textEditor->setPopupMenuEnabled (false);
    timeToRunValue__textEditor->setColour (juce::TextEditor::textColourId, juce::Colours::white);
    timeToRunValue__textEditor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    timeToRunValue__textEditor->setText (TRANS ("-"));

    timeToRunValue__textEditor->setBounds (376, 48, 150, 24);

    currentFrequencyValue__textEditor2.reset (new juce::TextEditor ("current Frequency Value"));
    addAndMakeVisible (currentFrequencyValue__textEditor2.get());
    currentFrequencyValue__textEditor2->setMultiLine (false);
    currentFrequencyValue__textEditor2->setReturnKeyStartsNewLine (false);
    currentFrequencyValue__textEditor2->setReadOnly (true);
    currentFrequencyValue__textEditor2->setScrollbarsShown (false);
    currentFrequencyValue__textEditor2->setCaretVisible (false);
    currentFrequencyValue__textEditor2->setPopupMenuEnabled (false);
    currentFrequencyValue__textEditor2->setColour (juce::TextEditor::textColourId, juce::Colours::white);
    currentFrequencyValue__textEditor2->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    currentFrequencyValue__textEditor2->setText (TRANS ("-"));

    currentFrequencyValue__textEditor2->setBounds (376, 82, 150, 24);

    recordAudio__toggleButton.reset (new juce::ToggleButton ("recordAudio toggle button"));
    addAndMakeVisible (recordAudio__toggleButton.get());
    recordAudio__toggleButton->setButtonText (TRANS ("Record Audio"));
    recordAudio__toggleButton->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 600);


    //[Constructor] You can add your own custom stuff here..
	Deleteoldmeasurements__textButton->onClick =
		[this]
		{
			{ // ScopedLock sl scope begin
				const ScopedLock sl(clearOldMeasuredLock);

				frequencyValues.clear();
				frequencyValues.reserve(0);
				rmsValues.clear();
				rmsValues.reserve(0);
				graph_attributes.clear();
				graph_attributes.reserve(0);
				plotLegend.clear();
				if (runNewMeasurement__toggleButton->getToggleState())
				{
					frequencyValues.push_back(forPush_BackFrequencyVector);
					rmsValues.push_back(forPush_BackRMSVector);
					makeGraphAttributes();
					plotLegend.push_back("p " + std::to_string(plotLegend.size() + 1));
				}
				module_Plot->clearPlot();

			}// ScopedLock sl scope END
		};

	runNewMeasurement__toggleButton->onClick =
		[this]
		{
			if (runNewMeasurement__toggleButton->getToggleState())
			{
				deltaFrequencyHz = deltaFreq__textEditor->getText().getDoubleValue();
				currentFrequencyHz = minFrequencyHz;

				updateAngleDelta();

				//frequencyValues.clear();
				//frequencyValues.reserve(0);
				frequencyValues.push_back(forPush_BackFrequencyVector);
				rmsValues.push_back(forPush_BackRMSVector);
				makeGraphAttributes();
				plotLegend.push_back("p " + std::to_string(plotLegend.size() + 1));

				setAudioChannels(1, 1); // One input, one output

				pause__toggleButton->setEnabled(true);

				if (recordAudio__toggleButton->getToggleState())
				{
					module_AudioRecording->startStopRecording();
				}

				startThread(Priority::low);
			}
			else
			{
				stopThread(100);

				if (recordAudio__toggleButton->getToggleState())
				{
					module_AudioRecording->startStopRecording();
				}

				stopAudio();
			}

			pause__toggleButton->setToggleState(false, dontSendNotification);
		};

	pause__toggleButton->setEnabled(false);
	pause__toggleButton->onClick =
		[this]
		{
			if (runNewMeasurement__toggleButton->getToggleState())
			{
				if (pause__toggleButton->getToggleState())
				{
					stopThread(100);

					shutdownAudio();

					updateAngleDelta();
				}
				else
				{
					updateAngleDelta();

					setAudioChannels(1, 1); // One input, one output

					startThread(Priority::low);
				}
			}
			else
			{
				pause__toggleButton->setToggleState(false, dontSendNotification);
			}
		};

	minFreq__textEditor->setInputRestrictions(10, "1234567890");
	minFreq__textEditor->setSelectAllWhenFocused(true);
	minFreq__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);
	minFreq__textEditor->setText(std::to_string(minFrequencyHz), false);
	minFreq__textEditor->onReturnKey =
		[this]
		{
			minFrequencyHz = minFreq__textEditor->getText().getDoubleValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				maxFreq__textEditor->setText(std::to_string(minFrequencyHz), false);
				maxFrequencyHz = minFrequencyHz;
			}
			updateAngleDelta();
		};
	minFreq__textEditor->onFocusLost =
		[this]
		{
			minFrequencyHz = minFreq__textEditor->getText().getDoubleValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				maxFreq__textEditor->setText(std::to_string(minFrequencyHz), false);
				maxFrequencyHz = minFrequencyHz;
			}
			updateAngleDelta();
		};

	maxFreq__textEditor->setInputRestrictions(10, "1234567890");
	maxFreq__textEditor->setSelectAllWhenFocused(true);
	maxFreq__textEditor->setKeyboardType(juce::TextInputTarget::VirtualKeyboardType::numericKeyboard);
	maxFreq__textEditor->setText(std::to_string(maxFrequencyHz), false);
	maxFreq__textEditor->onReturnKey =
		[this]
		{
			maxFrequencyHz = maxFreq__textEditor->getText().getDoubleValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				minFreq__textEditor->setText(juce::String(maxFrequencyHz), false);
				minFrequencyHz = maxFrequencyHz;
			}
			updateAngleDelta();
		};
	maxFreq__textEditor->onFocusLost =
		[this]
		{
			maxFrequencyHz = maxFreq__textEditor->getText().getDoubleValue();
			if (maxFrequencyHz < minFrequencyHz)
			{
				minFreq__textEditor->setText(std::to_string(maxFrequencyHz), false);
				minFrequencyHz = maxFrequencyHz;
			}
			updateAngleDelta();
		};

	deltaTime__textEditor->setInputRestrictions(10, "1234567890");
	deltaTime__textEditor->setSelectAllWhenFocused(true);
	deltaTime__textEditor->setKeyboardType
	(
		juce::TextInputTarget::VirtualKeyboardType::numericKeyboard
	);
	deltaTime__textEditor->setText(std::to_string(deltaTimeS), false);
	deltaTime__textEditor->onReturnKey =
		[this]
		{
			deltaTimeS = deltaTime__textEditor->getText().getDoubleValue();
			updateAngleDelta();
		};
	deltaTime__textEditor->onFocusLost =
		[this]
		{
			deltaTimeS = deltaTime__textEditor->getText().getDoubleValue();
			updateAngleDelta();
		};

	deltaFreq__textEditor->setInputRestrictions(10, "1234567890");
	deltaFreq__textEditor->setSelectAllWhenFocused(true);
	deltaFreq__textEditor->setKeyboardType
	(
		juce::TextInputTarget::VirtualKeyboardType::numericKeyboard
	);
	deltaFreq__textEditor->setText(std::to_string(deltaFrequencyHz), false);
	deltaFreq__textEditor->onReturnKey =
		[this]
		{
			deltaFrequencyHz = deltaFreq__textEditor->getText().getDoubleValue();
			updateAngleDelta();
		};
	deltaFreq__textEditor->onFocusLost =
		[this]
		{
			deltaFrequencyHz = deltaFreq__textEditor->getText().getDoubleValue();
			updateAngleDelta();
		};

	save__textButton->onClick =
		[this]
		{
			chooser.launchAsync(FileBrowserComponent::saveMode
				| FileBrowserComponent::canSelectFiles
				| FileBrowserComponent::warnAboutOverwriting,
				[this](const FileChooser& c)
				{
					if (juce::File(c.getResult()).exists())
					{
						juce::File(c.getResult()).deleteFile();
					}

					if (const auto outputStream = makeOutputStream(c.getURLResult()))
					{

						outputStream->setPosition(0);
						//outputStream->truncate();

						int vecNo = 0;
						for (auto vec : rmsValues)
						{
							// Y values
							outputStream->writeInt(static_cast<int>(vec.size()));
							for (auto fltVal : vec)
							{
								outputStream->writeFloat(fltVal);
							}

							// X-values
							auto xVec = frequencyValues[vecNo++];
							outputStream->writeInt(static_cast<int>(xVec.size()));
							for (auto fltVal : xVec)
							{
								outputStream->writeFloat(fltVal);
							}
						}

						outputStream->flush();
					}
				});

		};

	read__textButton->onClick =
		[this]
		{
			chooser.launchAsync
			(
				FileBrowserComponent::openMode
				|
				FileBrowserComponent::canSelectFiles
				,
				[this](const FileChooser& c)
				{
					if
                    (
                        const auto inputStream =
                        makeInputSource(c.getURLResult())->createInputStream()
                    )
					{
						//rmsValues.clear();
						//rmsValues.reserve(0);

						inputStream->setPosition(0);

                        // Y values
                        auto yCurVectorSize = inputStream->readInt();
                        while (!(inputStream->isExhausted()))
						{
							// Y values
							std::vector<float> yTmpVec(yCurVectorSize);

							inputStream->read(yTmpVec.data(), yCurVectorSize * sizeof(float));

							rmsValues.push_back(yTmpVec);

							// X values
							auto xCurVectorSize = inputStream->readInt();
							std::vector<float> xTmpVec(xCurVectorSize);

							inputStream->read(xTmpVec.data(), xCurVectorSize * sizeof(float));

							frequencyValues.push_back(xTmpVec);

							makeGraphAttributes();

							plotLegend.push_back("p " + std::to_string(plotLegend.size() + 1));

                            yCurVectorSize = inputStream->readInt();
						}

						inputStream->~InputStream();

						module_Plot->updatePlot(rmsValues, frequencyValues, graph_attributes, plotLegend);
					}
				}
			);

		};

	module_Plot->setTitle("Frequency response [RMS]");
	module_Plot->setXLabel("[Hz]");
	module_Plot->setYLabel("[RMS]");

	updateAngleDelta();
    //[/Constructor]
}

SoundProcessorModule::~SoundProcessorModule()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	eksShutdownAudio();
	stopThread(100);
    //[/Destructor_pre]

    runNewMeasurement__toggleButton = nullptr;
    juce__label2 = nullptr;
    juce__label3 = nullptr;
    timeToRun__label = nullptr;
    qurFreq__label = nullptr;
    juce__label5 = nullptr;
    juce__label = nullptr;
    juce__label6 = nullptr;
    pause__toggleButton = nullptr;
    Deleteoldmeasurements__textButton = nullptr;
    minFreq__textEditor = nullptr;
    maxFreq__textEditor = nullptr;
    deltaFreq__textEditor = nullptr;
    deltaTime__textEditor = nullptr;
    save__textButton = nullptr;
    read__textButton = nullptr;
    timeToRunTotally__textEditor = nullptr;
    timeToRunValue__textEditor = nullptr;
    currentFrequencyValue__textEditor2 = nullptr;
    recordAudio__toggleButton = nullptr;


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

    runNewMeasurement__toggleButton->setBounds (268 - (184 / 2), 82 + 24 - -14, 184, 24);
    juce__label2->setBounds (8, 208, getWidth() - 390, 24);
    timeToRun__label->setBounds (272 - (192 / 2), 48, 192, 24);
    qurFreq__label->setBounds (272 - (192 / 2), 82, 192, 24);
    juce__label5->setBounds (80 - (144 / 2), 82, 144, 24);
    juce__label->setBounds (83 - (150 / 2), 16, 150, 24);
    juce__label6->setBounds (272 - (192 / 2), 16, 192, 24);
    pause__toggleButton->setBounds (416 - (80 / 2), 120, 80, 24);
    Deleteoldmeasurements__textButton->setBounds (268 - (184 / 2), 160, 184, 24);
    minFreq__textEditor->setBounds (83 - (150 / 2), 82 + 24 - -8, 150, 24);
    maxFreq__textEditor->setBounds (83 - (150 / 2), 48, 150, 24);
    recordAudio__toggleButton->setBounds (440 - (127 / 2), 160, 127, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SoundProcessorModule::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == save__textButton.get())
    {
        //[UserButtonCode_save__textButton] -- add your button handler code here..
        //[/UserButtonCode_save__textButton]
    }
    else if (buttonThatWasClicked == read__textButton.get())
    {
        //[UserButtonCode_read__textButton] -- add your button handler code here..
        //[/UserButtonCode_read__textButton]
    }
    else if (buttonThatWasClicked == recordAudio__toggleButton.get())
    {
        //[UserButtonCode_recordAudio__toggleButton] -- add your button handler code here..
        //[/UserButtonCode_recordAudio__toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SoundProcessorModule::startStopRecording()
{
	if (recordAudio__toggleButton->getToggleState())
	{
		module_AudioRecording->startStopRecording();
	}
}

void SoundProcessorModule::makeGraphAttributes()
{
	cmp::GraphAttribute colourForLine;
	colourForLine.graph_colour = juce::Colour
	(
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(100, 255)))
		,
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(100, 255)))
		,
		static_cast<juce::int8>(randomRGB.nextInt(juce::Range(100, 255)))
	);
	graph_attributes.push_back(colourForLine);
}

void SoundProcessorModule::stopAudio()
{
	shutdownAudio();

	currentFrequencyHz = 0;

	updateAngleDelta();

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

void SoundProcessorModule::updateInfoTextEditors()
{
	if (deltaFrequencyHz > 0.0f)
	{
		auto totalTimeToRunS = deltaTimeS * (maxFrequencyHz - minFrequencyHz) / deltaFrequencyHz;

		timeToRunTotally__textEditor->setText
		(
			std::to_string
			(
				totalTimeToRunS / 60.0f
			)
			, false
		);

		currentTimeToRunS = deltaTimeS * (maxFrequencyHz - currentFrequencyHz) / deltaFrequencyHz;

		timeToRunValue__textEditor->setText
		(
			std::to_string
			(
				currentTimeToRunS / 60.0f
			)
			, false
		);
	}

	currentFrequencyValue__textEditor2->setText
	(
		std::to_string
		(
			currentFrequencyHz
		)
		, false
	);
}

void SoundProcessorModule::prepareToPlay(int, double sampleRate)
{
	currentSampleRate = sampleRate;
	updateAngleDelta();
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
	while ((!threadShouldExit()) && (currentFrequencyHz <= maxFrequencyHz))
	{
		updateAngleDelta();
		MessageManager::callAsync([this]() { updateInfoTextEditors(); });

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

			{ // ScopedLock sl scope begin
				const ScopedLock sl(clearOldMeasuredLock);

				frequencyValues[frequencyValues.size() - 1]
					.push_back(static_cast<float>(copyOfCurrentFrequencyHz));

				auto curRMS = std::sqrt(copyOfAudioSamplesSquareSum / copyOfNoSamplesInAudioSamplesSquareSum);
				rmsValues[rmsValues.size() - 1]
					.push_back(static_cast<float>(curRMS));

				module_Plot->updatePlot(rmsValues, frequencyValues, graph_attributes, plotLegend);
			}
		}
	}

	if (currentFrequencyHz >= maxFrequencyHz)
	{
		MessageManager::callAsync([this]() { startStopRecording(); });

		const MessageManagerLock mml;
		stopAudio();
		runNewMeasurement__toggleButton->setToggleState(false, dontSendNotification);
	}

}

void SoundProcessorModule::updateCurrentFrequencyLabel()
{
	const MessageManagerLock mml;
	currentFrequencyValue__textEditor2->setText
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
                 parentClasses="public juce::AudioAppComponent, private juce::Thread"
                 constructorParams="std::shared_ptr&lt;PlotModule&gt; ptr_module_Plot, std::shared_ptr&lt;AudioDeviceManager&gt; SADM, std::shared_ptr&lt;AudioRecorderModule&gt; MAR"
                 variableInitialisers="module_Plot(ptr_module_Plot)&#10;AudioAppComponent(*SADM)&#10;Thread(&quot;Freq. shifter&quot;)&#10;module_AudioRecording(MAR)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="600">
  <BACKGROUND backgroundColour="ff505050"/>
  <TOGGLEBUTTON name="run New Measurement toggle button" id="3e0da1935c285e8f"
                memberName="runNewMeasurement__toggleButton" virtualName="" explicitFocusOrder="5"
                pos="268c -14R 184 24" posRelativeX="3f78bae238bae958" posRelativeY="7a68dc774966fe8"
                buttonText="Run new measurement" connectedEdges="0" needsCallback="0"
                radioGroupId="0" state="0"/>
  <LABEL name="new label" id="69170c72758aeb9e" memberName="juce__label2"
         virtualName="" explicitFocusOrder="0" pos="8 208 390M 24" posRelativeX="3f78bae238bae958"
         edTextCol="ff000000" edBkgCol="0" labelText="Delta Time [Sec]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="30871f40ff4c0cbe" memberName="juce__label3"
         virtualName="" explicitFocusOrder="0" pos="8 146 128 24" posRelativeX="27e8662d217379e4"
         edTextCol="ff000000" edBkgCol="0" labelText="Delta freq [Hz]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="time To Run label" id="88f16d9da9e68c20" memberName="timeToRun__label"
         virtualName="" explicitFocusOrder="0" pos="272c 48 192 24" posRelativeW="93293acbba06ebfc"
         edTextCol="ff000000" edBkgCol="0" labelText="Time remaining to run [Min]:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="cur. freq. label" id="7a68dc774966fe8" memberName="qurFreq__label"
         virtualName="" explicitFocusOrder="0" pos="272c 82 192 24" posRelativeX="3f78bae238bae958"
         edTextCol="ff000000" edBkgCol="0" labelText="Current frequency [Hz]:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="d890f1fd96613a14" memberName="juce__label5"
         virtualName="" explicitFocusOrder="0" pos="80c 82 144 24" posRelativeW="93293acbba06ebfc"
         edTextCol="ff000000" edBkgCol="0" labelText="Min. frequency [Hz]"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="new label" id="b25ba8f932e8325" memberName="juce__label"
         virtualName="" explicitFocusOrder="0" pos="83c 16 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Max. freq. [Hz]" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="93293acbba06ebfc" memberName="juce__label6"
         virtualName="" explicitFocusOrder="0" pos="272c 16 192 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Time to run totally [Min]:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="pause toggle button" id="880f5cc9c6966415" memberName="pause__toggleButton"
                virtualName="" explicitFocusOrder="6" pos="416c 120 80 24" buttonText="Pause"
                connectedEdges="0" needsCallback="0" radioGroupId="0" state="0"/>
  <TEXTBUTTON name="Deleteoldmeasurements__textButton" id="737540671aa3e4af"
              memberName="Deleteoldmeasurements__textButton" virtualName=""
              explicitFocusOrder="7" pos="268c 160 184 24" buttonText="Delete old measurements"
              connectedEdges="0" needsCallback="0" radioGroupId="0"/>
  <TEXTEDITOR name="min. freq. text editor" id="873b782b66348908" memberName="minFreq__textEditor"
              virtualName="" explicitFocusOrder="2" pos="83c -8R 150 24" posRelativeX="3f78bae238bae958"
              posRelativeY="7a68dc774966fe8" tooltip="min. freq." initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="max freq. text editor" id="af1a81047b946cdb" memberName="maxFreq__textEditor"
              virtualName="" explicitFocusOrder="1" pos="83c 48 150 24" posRelativeW="93293acbba06ebfc"
              initialText="" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="delta freq. text editor" id="5d300af57711f33c" memberName="deltaFreq__textEditor"
              virtualName="" explicitFocusOrder="3" pos="8 176 150 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="delta time text editor" id="e8d7367e7e6f54a5" memberName="deltaTime__textEditor"
              virtualName="" explicitFocusOrder="4" pos="8 240 150 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTBUTTON name="save button" id="1684acb62ecea24c" memberName="save__textButton"
              virtualName="" explicitFocusOrder="8" pos="176 208 183 24" buttonText="Save measurements to file"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="read button" id="da2707a302e9a9f0" memberName="read__textButton"
              virtualName="" explicitFocusOrder="9" pos="376 208 199 24" buttonText="Read measurements from file"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="time To Run Totally text editor" id="b4ebcb1bfdd1d5" memberName="timeToRunTotally__textEditor"
              virtualName="" explicitFocusOrder="0" pos="376 16 150 24" textcol="ffffffff"
              bkgcol="0" initialText="-" multiline="0" retKeyStartsLine="0"
              readonly="1" scrollbars="0" caret="0" popupmenu="0"/>
  <TEXTEDITOR name="time To Run Value" id="8979b9eb1ff875e4" memberName="timeToRunValue__textEditor"
              virtualName="" explicitFocusOrder="0" pos="376 48 150 24" textcol="ffffffff"
              bkgcol="0" initialText="-" multiline="0" retKeyStartsLine="0"
              readonly="1" scrollbars="0" caret="0" popupmenu="0"/>
  <TEXTEDITOR name="current Frequency Value" id="812a5fde336055f8" memberName="currentFrequencyValue__textEditor2"
              virtualName="" explicitFocusOrder="0" pos="376 82 150 24" textcol="ffffffff"
              bkgcol="0" initialText="-" multiline="0" retKeyStartsLine="0"
              readonly="1" scrollbars="0" caret="0" popupmenu="0"/>
  <TOGGLEBUTTON name="recordAudio toggle button" id="2d551d06cdcfaafe" memberName="recordAudio__toggleButton"
                virtualName="" explicitFocusOrder="0" pos="439.5c 160 127 24"
                buttonText="Record Audio" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

