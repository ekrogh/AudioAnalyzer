/*
  ==============================================================================

    guitarSeparator.h
    Created: 24 Sep 2025 1:27:21am
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class guitarSeparator : public AudioSource
{
public:
    guitarSeparator();
    ~guitarSeparator() override;

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock(const AudioSourceChannelInfo&) override;


    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, its releaseResources() method
        will be called after it has been swapped for the new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    void setSource(AudioSource* newSource);

private:
    CriticalSection readLock;

    AudioSource* source = nullptr;
    double sampleRate = 0;
    int bufferSize = 0;

    CriticalSection callbackLock;
    int blockSize = 128, readAheadBufferSize = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (guitarSeparator)
};
