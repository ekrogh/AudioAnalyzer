/*
  ==============================================================================

    guitarSeparator.h
    Created: 24 Sep 2025 1:27:21am
    Author:  eigil

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class guitarSeparator  : public juce::Component
{
public:
    guitarSeparator();
    ~guitarSeparator() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (guitarSeparator)
};
