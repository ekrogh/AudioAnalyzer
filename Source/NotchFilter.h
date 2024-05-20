/*
  ==============================================================================

    NotchFilter.h
    Created: 20 May 2024 2:44:40pm
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class NotchFilter
{
public:
    NotchFilter(double baseFrequency, double sampleRate, int numChannels);

    void process(juce::AudioBuffer<float>& buffer);

private:
    juce::OwnedArray<juce::dsp::IIR::Filter<float>> filters;
};
