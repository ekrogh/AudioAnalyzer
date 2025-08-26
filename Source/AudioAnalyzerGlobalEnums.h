/*
  ==============================================================================

	AudioAnalyzerGlobalEnums.h
	Created: Thursdays 14. dec. 2023 14:30
	Author:  eks

  ==============================================================================
*/

#pragma once

#include <set>

// Num in- and outputs
enum  numInAndOutputs
{
	defaultNumInputChannels = 1
	, defaultNumOutputChannels = 2
};

enum
{
	eksAVAuthorizationStatusNotDetermined = 0
	, eksAVAuthorizationStatusRestricted = 1
	, eksAVAuthorizationStatusDenied = 2
	, eksAVAuthorizationStatusAuthorized = 3
};

enum defaultFFTValues
{
	fftOrder = 13
	, fftSize = 1 << fftOrder
	, numFftBuffers = 3
};

enum filterTypes
{
	noFilter
	,
	filter50Hz
	,
	filter60Hz
};
