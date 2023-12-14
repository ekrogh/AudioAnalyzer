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
	, defaultNumOutputChannels = 1
};

enum
{
      eksAVAuthorizationStatusNotDetermined    = 0
    , eksAVAuthorizationStatusRestricted       = 1
    , eksAVAuthorizationStatusDenied           = 2
    , eksAVAuthorizationStatusAuthorized       = 3
};
