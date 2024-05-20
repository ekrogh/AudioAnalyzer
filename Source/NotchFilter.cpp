/*
  ==============================================================================

	NotchFilter.cpp
	Created: 20 May 2024 2:44:40pm
	Author:  eigil

  ==============================================================================
*/

#include "NotchFilter.h"


NotchFilter::NotchFilter(double baseFrequency, double sampleRate, int numChannels)
{
    // Create a notch filter for each harmonic up to Nyquist frequency
    for (int i = 1; i <= sampleRate / 2 / baseFrequency; ++i)
    {
        for (int j = 0; j < numChannels; ++j)
        {
            auto* filter = new juce::dsp::IIR::Filter<float>();
            filter->coefficients =
                juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, baseFrequency * i);
            filters.add(filter);
        }
    }
}

void NotchFilter::process(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (int i = channel; i < filters.size(); i += buffer.getNumChannels())
        {
            filters[i]->process(context);
        }
    }
}
