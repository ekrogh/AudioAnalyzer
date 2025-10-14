/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioPlaybackModule
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays an audio file.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
				   juce_audio_processors, juce_audio_utils, juce_core,
				   juce_data_structures, juce_events, juce_graphics,
				   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        AudioPlaybackModule

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "Utilities.h"
#include "guitarSeparator.h"


class ModuleThumbnailComp final : public Component,
	public ChangeListener,
	public FileDragAndDropTarget,
	public ChangeBroadcaster,
	private ScrollBar::Listener,
	private Timer
{
public:
	ModuleThumbnailComp(AudioFormatManager& formatManager,
		AudioTransportSource& source,
		Slider& slider)
		: transportSource(source),
		zoomSlider(slider),
		thumbnail(512, formatManager, thumbnailCache)
	{
		thumbnail.addChangeListener(this);

		addAndMakeVisible(scrollbar);
		scrollbar.setRangeLimits(visibleRange);
		scrollbar.setAutoHide(false);
		scrollbar.addListener(this);

		currentPositionMarker.setFill(Colours::white.withAlpha(0.85f));
		addAndMakeVisible(currentPositionMarker);
	}

	~ModuleThumbnailComp() override
	{
		scrollbar.removeListener(this);
		thumbnail.removeChangeListener(this);
	}

	void setURL(const URL& url)
	{
		if (auto inputSource = makeInputSource(url))
		{
			thumbnail.setSource(inputSource.release());

			Range<double> newRange(0.0, thumbnail.getTotalLength());
			scrollbar.setRangeLimits(newRange);
			setRange(newRange);

			startTimerHz(40);
		}
	}

	URL getLastDroppedFile() const noexcept { return lastFileDropped; }

	void setZoomFactor(double amount)
	{
		if (thumbnail.getTotalLength() > 0)
		{
			auto newScale = jmax(0.001, thumbnail.getTotalLength() * (1.0 - jlimit(0.0, 0.99, amount)));
			auto timeAtCentre = xToTime((float)getWidth() / 2.0f);

			setRange({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
		}
	}

	void setRange(Range<double> newRange)
	{
		visibleRange = newRange;
		scrollbar.setCurrentRange(visibleRange);
		updateCursorPosition();
		repaint();
	}

	void setFollowsTransport(bool shouldFollow)
	{
		isFollowingTransport = shouldFollow;
	}

	void paint(Graphics& g) override
	{
		g.fillAll(Colours::darkgrey);
		g.setColour(Colours::lightblue);

		if (thumbnail.getTotalLength() > 0.0)
		{
			auto thumbArea = getLocalBounds();

			thumbArea.removeFromBottom(scrollbar.getHeight() + 4);
			thumbnail.drawChannels(g, thumbArea.reduced(2),
				visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
		}
		else
		{
			g.setFont(14.0f);
			g.drawFittedText("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
		}
	}

	void resized() override
	{
		scrollbar.setBounds(getLocalBounds().removeFromBottom(14).reduced(2));
	}

	void changeListenerCallback(ChangeBroadcaster*) override
	{
		// this method is called by the thumbnail when it has changed, so we should repaint it..
		repaint();
	}

	bool isInterestedInFileDrag(const StringArray& /*files*/) override
	{
		return true;
	}

	void filesDropped(const StringArray& files, int /*x*/, int /*y*/) override
	{
		lastFileDropped = URL(File(files[0]));
		sendChangeMessage();
	}

	void mouseDown(const MouseEvent& e) override
	{
		mouseDrag(e);
	}

	void mouseDrag(const MouseEvent& e) override
	{
		if (canMoveTransport())
			transportSource.setPosition(jmax(0.0, xToTime((float)e.x)));
	}

	void mouseUp(const MouseEvent&) override
	{
		transportSource.start();
	}

	void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel) override
	{
		if (thumbnail.getTotalLength() > 0.0)
		{
			auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
			newStart = jlimit(0.0, jmax(0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

			if (canMoveTransport())
				setRange({ newStart, newStart + visibleRange.getLength() });

			if (!approximatelyEqual(wheel.deltaY, 0.0f))
				zoomSlider.setValue(zoomSlider.getValue() - wheel.deltaY);

			repaint();
		}
	}

private:
	AudioTransportSource& transportSource;
	Slider& zoomSlider;
	ScrollBar scrollbar{ false };

	AudioThumbnailCache thumbnailCache{ 5 };
	AudioThumbnail thumbnail;
	Range<double> visibleRange;
	bool isFollowingTransport = false;
	URL lastFileDropped;

	DrawableRectangle currentPositionMarker;

	float timeToX(const double time) const
	{
		if (visibleRange.getLength() <= 0)
			return 0;

		return (float)getWidth() * (float)((time - visibleRange.getStart()) / visibleRange.getLength());
	}

	double xToTime(const float x) const
	{
		return (x / (float)getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
	}

	bool canMoveTransport() const noexcept
	{
		return !(isFollowingTransport && transportSource.isPlaying());
	}

	void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
	{
		if (scrollBarThatHasMoved == &scrollbar)
			if (!(isFollowingTransport && transportSource.isPlaying()))
				setRange(visibleRange.movedToStartAt(newRangeStart));
	}

	void timerCallback() override
	{
		if (canMoveTransport())
			updateCursorPosition();
		else
			setRange(visibleRange.movedToStartAt(transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
	}

	void updateCursorPosition()
	{
		currentPositionMarker.setVisible(transportSource.isPlaying() || isMouseButtonDown());

		currentPositionMarker.setRectangle(Rectangle<float>(timeToX(transportSource.getCurrentPosition()) - 0.75f, 0,
			1.5f, (float)(getHeight() - scrollbar.getHeight())));
	}
};

//==============================================================================
class AudioPlaybackModule final : public Component,
	private Button::Listener,
	private ChangeListener
{
public:
	AudioPlaybackModule(std::shared_ptr<AudioDeviceManager> SADM)
		: sharedAudioDeviceManager(SADM)
	{
		addAndMakeVisible(zoomLabel);
		zoomLabel.setFont(Font(15.00f, Font::plain));
		zoomLabel.setJustificationType(Justification::centredRight);
		zoomLabel.setEditable(false, false, false);
		zoomLabel.setColour(TextEditor::textColourId, Colours::black);
		zoomLabel.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

		addAndMakeVisible(followTransportButton);
		followTransportButton.onClick = [this] { updateFollowTransportState(); };

		addAndMakeVisible(gainSlider);
		gainSlider.setRange(0, 100.0f, 0);
		gainSlider.onValueChange =
			[this]
			{
				//audioSourcePlayer.setGain(gainSlider.getValue());
				transportSource.setGain(static_cast<float>(gainSlider.getValue()));
			};
		gainSlider.setValue(1.0f);

		addAndMakeVisible(chooseFileButton);
		chooseFileButton.addListener(this);

		addAndMakeVisible(zoomSlider);
		zoomSlider.setRange(0, 1, 0);
		zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor(zoomSlider.getValue()); };
		zoomSlider.setSkewFactor(2);

		thumbnail = std::make_unique<ModuleThumbnailComp>(formatManager, transportSource, zoomSlider);
		addAndMakeVisible(thumbnail.get());
		thumbnail->addChangeListener(this);

		addAndMakeVisible(startStopButton);
		startStopButton.setColour(TextButton::buttonColourId, Colour(0xff79ed7f));
		startStopButton.setColour(TextButton::textColourOffId, Colours::black);
		startStopButton.onClick = [this] { startOrStop(); };

		addAndMakeVisible(useRnNoiseButton);
		useRnNoiseButton.onClick = [this] { updateUseRnNoiseState(); };

		// audio setup
		formatManager.registerBasicFormats();

		thread.startThread(Thread::Priority::normal);

		sharedAudioDeviceManager->addAudioCallback(&audioSourcePlayer);
		audioSourcePlayer.setSource(&transportSource);

		setOpaque(true);
		setSize(500, 500);
	}

	~AudioPlaybackModule() override
	{
		transportSource.setSource(nullptr);
		audioSourcePlayer.setSource(nullptr);

		sharedAudioDeviceManager->removeAudioCallback(&audioSourcePlayer);

		chooseFileButton.removeListener(this);

		thumbnail->removeChangeListener(this);
	}

	void paint(Graphics& g) override
	{
		g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
	}

	void resized() override
	{
		auto r = getLocalBounds().reduced(4);

		auto controls = r.removeFromBottom(90);

		auto gainSliderControlRightBounds = controls.removeFromRight(controls.getWidth() / 6);

		gainSlider.setBounds(gainSliderControlRightBounds.reduced(2));

		auto controlRightBounds = controls.removeFromRight(controls.getWidth() / 3);
		chooseFileButton.setBounds(controlRightBounds.reduced(2));

		auto zoom = controls.removeFromTop(25);
		zoomLabel.setBounds(zoom.removeFromLeft(50));
		zoomSlider.setBounds(zoom);

		followTransportButton.setBounds(controls.removeFromTop(25));
		startStopButton.setBounds(controls.removeFromLeft(controls.getWidth() / 2));

		useRnNoiseButton.setBounds(controls.removeFromRight(controls.getWidth() / 2));

		r.removeFromBottom(6);

		thumbnail->setBounds(r);
	}

private:
	std::shared_ptr<AudioDeviceManager> sharedAudioDeviceManager;

	AudioFormatManager formatManager;
	TimeSliceThread thread{ "audio file preview" };

	FileChooser chooser
	{
			"File..."
			,
			File::getSpecialLocation
			(
				juce::File::SpecialLocationType::userMusicDirectory
			)
			.getChildFile("recording.wav")
			,
#if JUCE_ANDROID
				"*.*"
#else
				"*.wav;*.flac;*.aif;*.mp3"
#endif
	};

	TextButton chooseFileButton{ "Choose Audio File...", "Choose an audio file for playback" };

	URL currentAudioFile;
	AudioSourcePlayer audioSourcePlayer;
	AudioTransportSource transportSource;
	guitarSeparator audioSeparator;
	std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

	std::unique_ptr<ModuleThumbnailComp> thumbnail;
	Label zoomLabel{ {}, "zoom:" };
	Slider zoomSlider{ Slider::LinearHorizontal, Slider::NoTextBox };
	ToggleButton followTransportButton{ "Follow Transport" };
	ToggleButton useRnNoiseButton{ "Use RNNoise" };
	TextButton startStopButton{ "Play/Stop" };
	Slider gainSlider{ Slider::LinearVertical, Slider::TextBoxAbove };

	//==============================================================================
	void showAudioResource(URL resource)
	{
		if (!loadURLIntoTransport(resource))
		{
			// Failed to load the audio file!
			jassertfalse;
			return;
		}

		currentAudioFile = std::move(resource);
		zoomSlider.setValue(0, dontSendNotification);
		thumbnail->setURL(currentAudioFile);
	}

	bool loadURLIntoTransport(const URL& audioURL)
	{
		// unload the previous file source and delete it..
		transportSource.stop();
		transportSource.setSource(nullptr);
		currentAudioFileSource.reset();

		const auto source = makeInputSource(audioURL);

		if (source == nullptr)
			return false;

		auto stream = rawToUniquePtr(source->createInputStream());

		if (stream == nullptr)
			return false;

		auto reader = rawToUniquePtr(formatManager.createReaderFor(std::move(stream)));

		if (reader == nullptr)
			return false;

		currentAudioFileSource = std::make_unique<AudioFormatReaderSource>(reader.release(), true);

		// ..and plug it into our transport source
		transportSource.setSource(currentAudioFileSource.get(),
			32768,                   // tells it to buffer this many samples ahead
			&thread,                 // this is the background thread to use for reading-ahead
			currentAudioFileSource->getAudioFormatReader()->sampleRate);     // allows for sample rate correction

		return true;
	}

	void startOrStop()
	{
		if (transportSource.isPlaying())
		{
			transportSource.stop();
		}
		else
		{
			transportSource.setPosition(0);
			transportSource.start();
		}
	}

	void updateFollowTransportState()
	{
		thumbnail->setFollowsTransport(followTransportButton.getToggleState());
	}

	void updateUseRnNoiseState()
	{
		if (useRnNoiseButton.getToggleState())
		{

			audioSourcePlayer.setSource(&audioSeparator);
			audioSeparator.setSource(&transportSource);
		}
		else
		{
			audioSourcePlayer.setSource(&transportSource);
			audioSeparator.setSource(nullptr);
		}
	}

	void buttonClicked(Button* btn) override
	{
		if (btn == &chooseFileButton)
		{
			chooser.launchAsync
			(
				FileBrowserComponent::openMode
				|
				FileBrowserComponent::canSelectFiles
				,
				[this](const FileChooser& fc) /*mutable*/
				{
					if (fc.getURLResults().size() > 0)
					{
						auto u = fc.getURLResult();

						showAudioResource(std::move(u));
					}
				}
			);
		}
	}

	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == thumbnail.get())
			showAudioResource(URL(thumbnail->getLastDroppedFile()));
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlaybackModule)
};

