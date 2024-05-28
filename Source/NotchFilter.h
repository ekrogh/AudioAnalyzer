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
    NotchFilter(double baseFrequency, double sampleRate);

    void newBaseFrequencyOrSampleRate(double baseFrequency, double sampleRate);
    void clearfilterXAndYValues();

    void process(juce::AudioBuffer<float>& buffer);
    void process(std::vector<float>& buffer);
    void process(float* buffer, unsigned int numSamples);
    float process(float sample);

private:
    unsigned int cNotchNoOfCoeffs = 0;
    unsigned int dNotchNoOfCoeffs = 0;
    std::vector<double>::const_iterator cNotchCoeffsBegin;
    std::vector<double>::const_iterator dNotchCoeffsBegin;
    std::deque<double> filterXValues;
    std::deque<double> filterYValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotchFilter)
};
