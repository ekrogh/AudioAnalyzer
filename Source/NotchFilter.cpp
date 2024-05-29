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
			filterXValues.push_front(static_cast<double>(channelDataIn[sn]));

			yNew =
				std::inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, 0.0)
				- std::inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, 0.0);

			filterYValues.pop_back();
			filterYValues.push_front(yNew);

			channelDataOut[sn] = static_cast<float>(yNew);
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
		filterXValues.push_front(static_cast<double>(vectorData[sn]));

		yNew =
			std::inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, 0.0)
			- std::inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, 0.0);

		filterYValues.pop_back();
		filterYValues.push_front(yNew);

		vectorData[sn] = static_cast<float>(yNew);
	}
}

void NotchFilter::process(float* buffer, unsigned int numSamples)
{
	double yNew = 0.0f;

	for (int sn = 0; sn < numSamples; sn++)
	{
		filterXValues.pop_back();
		filterXValues.push_front(static_cast<double>(buffer[sn]));

		yNew =
			std::inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, 0.0)
			- std::inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, 0.0);

		filterYValues.pop_back();
		filterYValues.push_front(yNew);

		buffer[sn] = static_cast<float>(yNew);
	}
}

float NotchFilter::process(float sample)
{
	filterXValues.pop_back();
	filterXValues.push_front(static_cast<double>(sample));

	double yNew =
		std::inner_product(filterXValues.begin(), filterXValues.end(), dNotchCoeffsBegin, 0.0)
		- std::inner_product(filterYValues.begin(), filterYValues.end(), cNotchCoeffsBegin, 0.0);

	filterYValues.pop_back();
	filterYValues.push_front(yNew);

	return static_cast<float>(yNew);
}