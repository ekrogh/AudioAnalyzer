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
//[/Headers]

#include "helpPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
aboutPage::aboutPage ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    aboutLabel.reset (new juce::Label ("aboutLabel",
                                       juce::String()));
    addAndMakeVisible (aboutLabel.get());
    aboutLabel->setFont (juce::Font (16.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    aboutLabel->setJustificationType (juce::Justification::centred);
    aboutLabel->setEditable (false, false, false);
    aboutLabel->setColour (juce::Label::textColourId, juce::Colours::white);
    aboutLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    aboutLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    aboutLabel->setBounds (42, 0, 215, 64);

    hyperlinkButton.reset (new juce::HyperlinkButton (TRANS ("http://eksit.dk"),
                                                      juce::URL ("http://eksit.dk")));
    addAndMakeVisible (hyperlinkButton.get());
    hyperlinkButton->setTooltip (TRANS ("http://eksit.dk"));
    hyperlinkButton->setButtonText (TRANS ("http://eksit.dk"));
    hyperlinkButton->setColour (juce::HyperlinkButton::textColourId, juce::Colours::white);

    hyperlinkButton->setBounds (42, 115, 215, 24);

    eigilLabel.reset (new juce::Label ("eigilLabel",
                                       TRANS ("Eigil Krogh Sorensen\n")));
    addAndMakeVisible (eigilLabel.get());
    eigilLabel->setFont (juce::Font (16.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    eigilLabel->setJustificationType (juce::Justification::centred);
    eigilLabel->setEditable (false, false, false);
    eigilLabel->setColour (juce::Label::textColourId, juce::Colours::white);
    eigilLabel->setColour (juce::TextEditor::textColourId, juce::Colours::cornflowerblue);
    eigilLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    eigilLabel->setBounds (42, 92, 215, 24);

    emailButton.reset (new juce::HyperlinkButton (TRANS ("eks@eksit.dk"),
                                                  juce::URL ("mailto://guitarFineTune@eksit.dk")));
    addAndMakeVisible (emailButton.get());
    emailButton->setTooltip (TRANS ("mailto://guitarFineTune@eksit.dk"));
    emailButton->setButtonText (TRANS ("eks@eksit.dk"));
    emailButton->setColour (juce::HyperlinkButton::textColourId, juce::Colours::white);

    emailButton->setBounds (42, 139, 215, 24);

    UsersGuide.reset (new juce::HyperlinkButton (TRANS ("User\'s Guide"),
                                                 juce::URL ("http://eksit.dk/users-guide/")));
    addAndMakeVisible (UsersGuide.get());
    UsersGuide->setTooltip (TRANS ("http://eksit.dk/users-guide/"));
    UsersGuide->setButtonText (TRANS ("User\'s Guide"));
    UsersGuide->setColour (juce::HyperlinkButton::textColourId, juce::Colours::white);

    UsersGuide->setBounds (42, 65, 215, 24);

    juceVer__label.reset (new juce::Label ("juceVer label",
                                           TRANS ("-")));
    addAndMakeVisible (juceVer__label.get());
    juceVer__label->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    juceVer__label->setJustificationType (juce::Justification::centred);
    juceVer__label->setEditable (false, false, false);
    juceVer__label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    juceVer__label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    juceVer__label->setBounds (72, 201, 150, 24);


    //[UserPreSize]
	aboutLabel->setText(std::string(ProjectInfo::projectName)
		+ "\nVer. "
		+ std::string(ProjectInfo::versionString)
#if (JUCE_ANDROID)
		+"." + std::to_string(ANDROID_VERSION_CODE)
#endif
		, juce::NotificationType::dontSendNotification);

	juceVer__label->setText(juce::String(SystemStats::getJUCEVersion()), juce::NotificationType::dontSendNotification);
    //[/UserPreSize]

    setSize (300, 260);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

aboutPage::~aboutPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    aboutLabel = nullptr;
    hyperlinkButton = nullptr;
    eigilLabel = nullptr;
    emailButton = nullptr;
    UsersGuide = nullptr;
    juceVer__label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void aboutPage::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff505050));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void aboutPage::resized()
{
    //[UserPreResize] Add your own custom resize code here..
#if (JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX)
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
#endif // (JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX)
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
#if (JUCE_IOS || JUCE_ANDROID)
void aboutPage::scaleAllComponents()
{
	if (auto parent = findParentComponentOfClass<TabbedComponent>())
	{
		static float scaleUsedLastTime = 1.0f;
		static bool firstCall = true;
		Rectangle<int> workRectangle;

		int tabBarDepth = parent->getTabBarDepth();

		auto curUserArea = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
		float bndsScaleHoriz = (float)(curUserArea.getWidth()) / (float)(widthOfAboutPage);
		float bndsScaleVerti = ((float)(curUserArea.getHeight()) - (float)tabBarDepth) /
			((float)hightOfAboutPage);
		float scaleNow = bndsScaleVerti;
		if (bndsScaleHoriz < bndsScaleVerti)
		{
			scaleNow = bndsScaleHoriz;
		}

		workRectangle.setBounds((300 - 215) / 2, 0, 215, 64);
		aboutLabel->setBounds(workRectangle * scaleUsedLastTime);
		workRectangle.setBounds((300 - 215) / 2, 65, 215, 24);
		UsersGuide->setBounds(workRectangle * scaleUsedLastTime);
		workRectangle.setBounds((300 - 215) / 2, 92, 215, 24);
		eigilLabel->setBounds(workRectangle * scaleUsedLastTime);
		workRectangle.setBounds((300 - 215) / 2, 115, 215, 24);
		hyperlinkButton->setBounds(workRectangle * scaleUsedLastTime);
		workRectangle.setBounds((300 - 215) / 2, 139, 215, 24);
		emailButton->setBounds(workRectangle * scaleUsedLastTime);
		workRectangle.setBounds((300 - 215) / 2 + 0, 169, 215, 24);

		auto curbnds = getBounds();

		if (scaleNow != scaleUsedLastTime)
		{
			float scaleToUse = scaleNow / scaleUsedLastTime;

			curbnds = getBounds();
			curUserArea.setHeight(curUserArea.getHeight() - parent->getTabBarDepth());
			curUserArea.setY(parent->getTabBarDepth());
			setBoundsToFit(curUserArea, Justification::left, false);

			int numChildComponents = this->getNumChildComponents();

			for (int i = 0; i < numChildComponents; ++i)
			{
				if (Component* childComponent = this->getChildComponent(i))
				{
					auto curCpntBnds = childComponent->getBounds();
					auto scaledBounds = curCpntBnds * scaleToUse;
					childComponent->setBounds(scaledBounds);

					auto labelTestComponent = dynamic_cast<juce::Label*> (childComponent);
					if ((labelTestComponent != nullptr) && (scaleToUse > 1) && firstCall)
					{
						Font currentFont = labelTestComponent->getFont();
						currentFont.setHeight(currentFont.getHeightInPoints() * scaleToUse * 0.9);
						labelTestComponent->setFont(currentFont);
					}

					auto textEditorTestComponent = dynamic_cast<juce::TextEditor*> (childComponent);
					if ((textEditorTestComponent != nullptr) && (scaleToUse > 1) && firstCall)
					{
						Font currentFont = textEditorTestComponent->getFont();
						currentFont.setHeight(currentFont.getHeightInPoints() * scaleToUse * 1.3);
						textEditorTestComponent->applyFontToAllText(currentFont);
						textEditorTestComponent->setJustification(Justification::centred);
					}
				}
			}

			scaleUsedLastTime = scaleNow;
			firstCall = false;
		}
	}
}
#endif // (JUCE_IOS || JUCE_ANDROID)
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="aboutPage" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="300" initialHeight="260">
  <BACKGROUND backgroundColour="ff505050"/>
  <LABEL name="aboutLabel" id="6f00272c7df5fc3a" memberName="aboutLabel"
         virtualName="" explicitFocusOrder="0" pos="42 0 215 64" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="16.0" kerning="0.0" bold="0" italic="0" justification="36"/>
  <HYPERLINKBUTTON name="new hyperlink" id="feec564fc78347ef" memberName="hyperlinkButton"
                   virtualName="" explicitFocusOrder="0" pos="42 115 215 24" tooltip="http://eksit.dk"
                   textCol="ffffffff" buttonText="http://eksit.dk" connectedEdges="0"
                   needsCallback="0" radioGroupId="0" url="http://eksit.dk"/>
  <LABEL name="eigilLabel" id="e25fc84f6930e0b1" memberName="eigilLabel"
         virtualName="" explicitFocusOrder="0" pos="42 92 215 24" textCol="ffffffff"
         edTextCol="ff6495ed" edBkgCol="0" labelText="Eigil Krogh Sorensen&#10;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="16.0" kerning="0.0" bold="0"
         italic="0" justification="36"/>
  <HYPERLINKBUTTON name="emailButton" id="a8626cdd7b90c56" memberName="emailButton"
                   virtualName="" explicitFocusOrder="0" pos="42 139 215 24" tooltip="mailto://guitarFineTune@eksit.dk"
                   textCol="ffffffff" buttonText="eks@eksit.dk" connectedEdges="0"
                   needsCallback="0" radioGroupId="0" url="mailto://guitarFineTune@eksit.dk"/>
  <HYPERLINKBUTTON name="UsersGuidehyperlink" id="d98c7b5f8b59379e" memberName="UsersGuide"
                   virtualName="" explicitFocusOrder="0" pos="42 65 215 24" tooltip="http://eksit.dk/users-guide/"
                   textCol="ffffffff" buttonText="User's Guide" connectedEdges="0"
                   needsCallback="0" radioGroupId="0" url="http://eksit.dk/users-guide/"/>
  <LABEL name="juceVer label" id="4a131a76420e3a14" memberName="juceVer__label"
         virtualName="" explicitFocusOrder="0" pos="72 201 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="-" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

