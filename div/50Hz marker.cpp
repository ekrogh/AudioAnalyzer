void freqPlotModule::addFrequencyMarker(float frequency, juce::Colour colour)
{
    // Create a vertical line at the given frequency
    juce::Line<float> line (frequency, plotArea.getBottom(), frequency, plotArea.getTop());

    // Add the line to the plot with the given colour
    plot.add(line, colour);
}

void freqPlotModule::markFrequencies()
{
    // Clear any existing markers
    plot.clear();

    // Mark 50 Hz and its harmonics up to the Nyquist frequency
    for (float freq = 50.0f; freq <= sampleRate / 2; freq += 50.0f)
    {
        addFrequencyMarker(freq, juce::Colours::red);
    }

    // Redraw the plot
    plot.repaint();
}


class Plot : public juce::Component
{
    // ... existing code ...

public:
    void addVerticalLine(float x_position, juce::Colour colour)
    {
        // Create a new GraphLine
        auto line = std::make_unique<cmp::GraphLine>();

        // Set the x and y values for the line
        line->setXValues({ x_position, x_position });
        line->setYValues({ m_y_lim.min, m_y_lim.max });

        // Set the colour of the line
        line->setColour(colour);

        // Add the line to the plot
        m_graph_lines->push_back(std::move(line));
    }

    // ... existing code ...
};

class ExtendedPlot : public cmp::Plot
{
public:
    ExtendedPlot(const cmp::Scaling x_scaling = cmp::Scaling::linear,
        const cmp::Scaling y_scaling = cmp::Scaling::linear)
        : cmp::Plot(x_scaling, y_scaling)
    {
    }

    void addVerticalLine(float x_position, juce::Colour colour)
    {
        // Implementation of the method...
    }
};


std::vector<float> frequencies;
for (float freq = 50.0f; freq <= sampleRate / 2; freq += 50.0f)
{
    frequencies.push_back(freq);
}
module_freqPlot->setXTicks(frequencies);

std::vector<std::string> labels;
for (float freq = 50.0f; freq <= sampleRate / 2; freq += 50.0f)
{
    labels.push_back(std::to_string(freq) + " Hz");
}
module_freqPlot->setXTickLabels(labels);

void NotchFilter::process(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sn = 0; sn < buffer.getNumSamples(); ++sn)
        {
            filterXValues.pop_back();
            filterXValues.push_front(static_cast<double>(channelData[sn]));

            double yNew = std::inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, 0.0)
                - std::inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, 0.0);

            filterYValues.pop_back();
            filterYValues.push_front(yNew);

            channelData[sn] = static_cast<float>(yNew);
        }
    }
}
