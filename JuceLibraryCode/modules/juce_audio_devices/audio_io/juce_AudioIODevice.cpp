/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

void AudioIODeviceCallback::audioDeviceIOCallbackWithContext ([[maybe_unused]] const float* const* inputChannelData,
                                                              [[maybe_unused]] int numInputChannels,
                                                              [[maybe_unused]] float* const* outputChannelData,
                                                              [[maybe_unused]] int numOutputChannels,
                                                              [[maybe_unused]] int numSamples,
                                                              [[maybe_unused]] const AudioIODeviceCallbackContext& context) {}

//==============================================================================
AudioIODevice::AudioIODevice (const String& deviceName, const String& deviceTypeName)
    : name (deviceName), typeName (deviceTypeName)
{
}

AudioIODevice::~AudioIODevice() {}

// eks 17. april. 2024 added
// checkAudioInputAccessPermissions always
// returns 3 (=eksAVAuthorizationStatusAuthorized) on Linux
#if JUCE_LINUX
int AudioIODevice::checkAudioInputAccessPermissions()           { return 3; };
#else // #if JUCE_LINUX
// eks 15. sept. 2020 added checkAudioInputAccessPermissions
int AudioIODevice::checkAudioInputAccessPermissions( )          { return 0; };
#endif // #if JUCE_LINUX

void AudioIODeviceCallback::audioDeviceError (const String&)    {}
bool AudioIODevice::setAudioPreprocessingEnabled (bool)         { return false; }
bool AudioIODevice::hasControlPanel() const                     { return false; }
int  AudioIODevice::getXRunCount() const noexcept               { return -1; }

bool AudioIODevice::showControlPanel()
{
    jassertfalse;    // this should only be called for devices which return true from
                     // their hasControlPanel() method.
    return false;
}

} // namespace juce
