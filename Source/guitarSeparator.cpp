/*
  ==============================================================================

    guitarSeparator.cpp
    Created: 24 Sep 2025 1:27:21am
    Author:  eigil

  ==============================================================================
*/

#include <JuceHeader.h>
#include "guitarSeparator.h"

//==============================================================================
guitarSeparator::guitarSeparator()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

guitarSeparator::~guitarSeparator()
{
}

void guitarSeparator::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
    const ScopedLock sl(callbackLock);

    sampleRate = newSampleRate;
    blockSize = samplesPerBlockExpected;

    if (source != nullptr)
        source->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void guitarSeparator::setSource(AudioSource* newSource)
{
    if (source != newSource)
    {
        auto* oldSource = source;

        if (newSource != nullptr && bufferSize > 0 && sampleRate > 0)
            newSource->prepareToPlay(bufferSize, sampleRate);

        {
            const ScopedLock sl(readLock);
            source = newSource;
        }

        if (oldSource != nullptr)
            oldSource->releaseResources();
    }
}
