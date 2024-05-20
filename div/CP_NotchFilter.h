class NotchFilter
{
public:
    NotchFilter(double sampleRate, int numChannels)
    {
        // Create a notch filter for each harmonic up to Nyquist frequency
        for (int i = 1; i <= sampleRate / 2 / 50; ++i)
        {
            for (int j = 0; j < numChannels; ++j)
            {
                auto* filter = new juce::dsp::IIR::Filter<float>();
                filter->setType(juce::dsp::IIR::Coefficients<float>::CoefficientsType::notch);
                filter->setCoefficients(*juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, 50.0 * i));
                filters.add(filter);
            }
        }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            for (int i = channel; i < filters.size(); i += buffer.getNumChannels())
            {
                filters[i]->processSamples(buffer.getWritePointer(channel), buffer.getNumSamples());
            }
        }
    }

private:
    juce::OwnedArray<juce::dsp::IIR::Filter<float>> filters;
};
