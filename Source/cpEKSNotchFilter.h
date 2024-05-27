#include <JuceHeader.h>
#include <corecrt_math_defines.h>

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

	void process(juce::AudioBuffer<float>& buffer)
	{
		for (int channel = 0; channel < numChannels; ++channel)
		{
			for (int i = 0; i < buffer.getNumSamples(); ++i)
			{
				float input = buffer.getSample(channel, i);
				double output = b0 * input + b1 * x1[channel] + b2 * x2[channel] - a1 * y1[channel] - a2 * y2[channel];

				// Check for NaN and replace with 0
				if (std::isnan(output) || std::isinf(output))
				{
					output = 0;
				}

				x2[channel] = x1[channel];
				x1[channel] = input;
				y2[channel] = y1[channel];
				y1[channel] = output;
				buffer.setSample(channel, i, static_cast<float>(output));
			}
		}
	}

	std::vector<double> calculateImpulseResponse(const int numSamples = defaultFFTValues::fftSize)
	{
		std::vector<double> impulseResponse(numSamples, 0.0);
		impulseResponse[0] = 1.0; // Set the first sample to 1 (impulse)

		for (int n = 1; n < numSamples; ++n)
		{
			impulseResponse[n] =
				(n >= 2) ?
				(b0 * impulseResponse[n - 0] +
					b1 * impulseResponse[n - 1] +
					b2 * impulseResponse[n - 2] -
					a1 * impulseResponse[n - 1] -
					a2 * impulseResponse[n - 2]) :
				(n == 1) ?
				(b0 * impulseResponse[n - 0] +
					b1 * impulseResponse[n - 1] -
					a1 * impulseResponse[n - 1]) :
				(b0 * impulseResponse[n - 0]);

				if (std::isnan(impulseResponse[n]) || std::isinf(impulseResponse[n]))
				{
					impulseResponse[n] = 0;
				}

		}

		return impulseResponse;
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
