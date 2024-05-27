class cpEKSNotchFilter
{
public:
    cpEKSNotchFilter(double notchFrequency, double sampleRate, int numChannels, double bandwidth = 0.1)
        : sampleRate(sampleRate), numChannels(numChannels), bandwidth(bandwidth)
    {
        setNotchFrequency(notchFrequency);
    }

    void setNotchFrequency(double notchFrequency)
    {
        this->notchFrequency = notchFrequency;
        updateCoefficients();
    }

    void process(std::vector<std::vector<float>>& buffer)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (size_t i = 0; i < buffer[channel].size(); ++i)
            {
                double input = buffer[channel][i];
                double output = b0 * input + b1 * x1[channel] + b2 * x2[channel] - a1 * y1[channel] - a2 * y2[channel];
                x2[channel] = x1[channel];
                x1[channel] = input;
                y2[channel] = y1[channel];
                y1[channel] = output;
                buffer[channel][i] = static_cast<float>(output);
            }
        }
    }

private:
    void updateCoefficients()
    {
        double R = 1 - 3 * bandwidth;
        double F = 2 * M_PI * notchFrequency / sampleRate;
        double K = (1 - 2 * R * cos(F) + R * R) / (2 - 2 * cos(F));
        a0 = K;
        a1 = -2 * K * cos(F);
        a2 = K;
        b0 = 1;
        b1 = -2 * R * cos(F);
        b2 = R * R;
        x1.assign(numChannels, 0);
        x2.assign(numChannels, 0);
        y1.assign(numChannels, 0);
        y2.assign(numChannels, 0);
    }

    double sampleRate;
    int numChannels;
    double notchFrequency;
    double bandwidth;
    double a0, a1, a2, b0, b1, b2;
    std::vector<double> x1, x2, y1, y2;
};


void SpectrogramComponent::setFilterToUse(filterTypes theFilterType)
{
    switch (theFilterType)
    {
        case filter50Hz:
            {
                theNotchFilter = std::make_unique<cpEKSNotchFilter>(50.0, curSampleRate, curNumInputChannels, 0.1);
                break;
            }
        case filter60Hz:
            {
                theNotchFilter = std::make_unique<cpEKSNotchFilter>(60.0, curSampleRate, curNumInputChannels, 0.1);
                break;
            }
    }

    filterToUse = theFilterType;
}
