/*
  ==============================================================================

	NotchFilter.cpp
	Created: 20 May 2024 2:44:40pm
	Author:  eigil

  ==============================================================================
*/

#include "NOTCH_50_60_Hz_filter_Coeffs.h"
#include "NotchFilter.h"

using namespace NOTCH_50_60_Hz_filter_Coeffs;

NotchFilter::NotchFilter(double baseFrequency, double sampleRate)
{
	newBaseFrequencyOrSampleRate(baseFrequency, sampleRate);
}

void NotchFilter::newBaseFrequencyOrSampleRate(double baseFrequency, double sampleRate)
{
	// The right coefficients
	/*std::tuple<int, int, std::vector<double>::const_iterator, std::vector<double>::const_iterator>*/
	auto theCoefficients = notchFiltersCoefficientsMap[baseFrequency][sampleRate];
	cNotchNoOfCoeffs = std::get<0>(theCoefficients);
	dNotchNoOfCoeffs = std::get<1>(theCoefficients);
	cNotchCoeffsBegin = std::get<2>(theCoefficients);
	dNotchCoeffsBegin = std::get<3>(theCoefficients);

	// Clear NOTCH filters input and output deque
	clearfilterXAndYValues();
}

void NotchFilter::clearfilterXAndYValues()
{
	// Clear NOTCH filters input and output deque
	filterXValues.clear();
	filterXValues.resize(dNotchNoOfCoeffs, 0);

	filterYValues.clear();
	filterYValues.resize(cNotchNoOfCoeffs, 0);
}

void NotchFilter::process(juce::AudioBuffer<float>& buffer)
{
	double yNew = 0.0f;

	for (int channel = 0; channel < buffer.getNumChannels(); channel++)
	{
		auto channelDataIn = buffer.getReadPointer(channel);
		auto channelDataOut = buffer.getWritePointer(channel);

		for (int sn = 0; sn < buffer.getNumSamples(); sn++)
		{
			filterXValues.pop_back();
			filterXValues.push_front((double)channelDataIn[sn]);

			yNew =
				inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, (double)0.0)
				- inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, (double)0.0);

			filterYValues.pop_back();
			filterYValues.push_front(yNew);

			channelDataOut[sn] = (float)yNew;
		}
	}
}

void NotchFilter::process(std::vector<float>& buffer)
{
	double yNew = 0.0f;

	auto vectorData = buffer.data();

	for (int sn = 0; sn < buffer.size(); sn++)
	{
		filterXValues.pop_back();
		filterXValues.push_front((double)vectorData[sn]);

		yNew =
			inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, (double)0.0)
			- inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, (double)0.0);

		filterYValues.pop_back();
		filterYValues.push_front(yNew);

		vectorData[sn] = (float)yNew;
	}
}

void NotchFilter::process(float* buffer, unsigned int numSamples)
{
	double yNew = 0.0f;

	for (int sn = 0; sn < numSamples; sn++)
	{
		filterXValues.pop_back();
		filterXValues.push_front((double)buffer[sn]);

		yNew =
			inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, (double)0.0)
			- inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, (double)0.0);

		filterYValues.pop_back();
		filterYValues.push_front(yNew);

		buffer[sn] = (float)yNew;
	}
}

float NotchFilter::process(float sample)
{
	filterXValues.pop_back();
	filterXValues.push_front((double)sample);

	double yNew =
		inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, (double)0.0)
		- inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, (double)0.0);

	filterYValues.pop_back();
	filterYValues.push_front(yNew);

	return (float)yNew;
}