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

MPEValue::MPEValue() noexcept                             {}
MPEValue::MPEValue (int value)  : normalisedValue (value) {}

//==============================================================================
MPEValue MPEValue::from7BitInt (int value) noexcept
{
    jassert (value >= 0 && value <= 127);

    auto valueAs14Bit = value <= 64 ? value << 7
                                    : int (jmap<float> (float (value - 64), 0.0f, 63.0f, 0.0f, 8191.0f)) + 8192;

    return { valueAs14Bit };
}

MPEValue MPEValue::from14BitInt (int value) noexcept
{
    jassert (value >= 0 && value <= 16383);
    return { value };
}

MPEValue MPEValue::fromUnsignedFloat (float value) noexcept
{
    jassert (0.0f <= value && value <= 1.0f);
    return { roundToInt (value * 16383.0f) };
}

MPEValue MPEValue::fromSignedFloat (float value) noexcept
{
    jassert (-1.0f <= value && value <= 1.0f);
    return { roundToInt (((value + 1.0f) * 16383.0f) / 2.0f) };
}

//==============================================================================
MPEValue MPEValue::minValue() noexcept      { return MPEValue::from7BitInt (0); }
MPEValue MPEValue::centreValue() noexcept   { return MPEValue::from7BitInt (64); }
MPEValue MPEValue::maxValue() noexcept      { return MPEValue::from7BitInt (127); }

int MPEValue::as7BitInt() const noexcept
{
    return normalisedValue >> 7;
}

int MPEValue::as14BitInt() const noexcept
{
    return normalisedValue;
}

//==============================================================================
float MPEValue::asSignedFloat() const noexcept
{
    return (normalisedValue < 8192)
           ? jmap<float> (float (normalisedValue), 0.0f, 8192.0f, -1.0f, 0.0f)
           : jmap<float> (float (normalisedValue), 8192.0f, 16383.0f, 0.0f, 1.0f);
}

float MPEValue::asUnsignedFloat() const noexcept
{
    return jmap<float> (float (normalisedValue), 0.0f, 16383.0f, 0.0f, 1.0f);
}

//==============================================================================
bool MPEValue::operator== (const MPEValue& other) const noexcept
{
    return normalisedValue == other.normalisedValue;
}

bool MPEValue::operator!= (const MPEValue& other) const noexcept
{
    return ! operator== (other);
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class MPEValueTests final : public UnitTest
{
public:
    MPEValueTests()
        : UnitTest ("MPEValue class", UnitTestCategories::midi)
    {}

    void runTest() override
    {
        beginTest ("comparison operator");
        {
            MPEValue value1 = MPEValue::from7BitInt (7);
            MPEValue value2 = MPEValue::from7BitInt (7);
            MPEValue value3 = MPEValue::from7BitInt (8);

            expect (value1 == value1);
            expect (value1 == value2);
            expect (value1 != value3);
        }

        beginTest ("special values");
        {
            expectEquals (MPEValue::minValue().as7BitInt(), 0);
            expectEquals (MPEValue::minValue().as14BitInt(), 0);

            expectEquals (MPEValue::centreValue().as7BitInt(), 64);
            expectEquals (MPEValue::centreValue().as14BitInt(), 8192);

            expectEquals (MPEValue::maxValue().as7BitInt(), 127);
            expectEquals (MPEValue::maxValue().as14BitInt(), 16383);
        }

        beginTest ("zero/minimum value");
        {
            expectValuesConsistent (MPEValue::from7BitInt       (0),     0, 0, -1.0f, 0.0f);
            expectValuesConsistent (MPEValue::from14BitInt      (0),     0, 0, -1.0f, 0.0f);
            expectValuesConsistent (MPEValue::fromUnsignedFloat (0.0f),  0, 0, -1.0f, 0.0f);
            expectValuesConsistent (MPEValue::fromSignedFloat   (-1.0f), 0, 0, -1.0f, 0.0f);
        }

        beginTest ("maximum value");
        {
            expectValuesConsistent (MPEValue::from7BitInt       (127),   127, 16383, 1.0f, 1.0f);
            expectValuesConsistent (MPEValue::from14BitInt      (16383), 127, 16383, 1.0f, 1.0f);
            expectValuesConsistent (MPEValue::fromUnsignedFloat (1.0f),  127, 16383, 1.0f, 1.0f);
            expectValuesConsistent (MPEValue::fromSignedFloat   (1.0f),  127, 16383, 1.0f, 1.0f);
        }

        beginTest ("centre value");
        {
            expectValuesConsistent (MPEValue::from7BitInt       (64),   64, 8192, 0.0f, 0.5f);
            expectValuesConsistent (MPEValue::from14BitInt      (8192), 64, 8192, 0.0f, 0.5f);
            expectValuesConsistent (MPEValue::fromUnsignedFloat (0.5f), 64, 8192, 0.0f, 0.5f);
            expectValuesConsistent (MPEValue::fromSignedFloat   (0.0f), 64, 8192, 0.0f, 0.5f);
        }

        beginTest ("value halfway between min and centre");
        {
            expectValuesConsistent (MPEValue::from7BitInt       (32),    32, 4096, -0.5f, 0.25f);
            expectValuesConsistent (MPEValue::from14BitInt      (4096),  32, 4096, -0.5f, 0.25f);
            expectValuesConsistent (MPEValue::fromUnsignedFloat (0.25f), 32, 4096, -0.5f, 0.25f);
            expectValuesConsistent (MPEValue::fromSignedFloat   (-0.5f), 32, 4096, -0.5f, 0.25f);
        }
    }

private:
    //==============================================================================
    void expectValuesConsistent (MPEValue value,
                                 int expectedValueAs7BitInt,
                                 int expectedValueAs14BitInt,
                                 float expectedValueAsSignedFloat,
                                 float expectedValueAsUnsignedFloat)
    {
        expectEquals (value.as7BitInt(), expectedValueAs7BitInt);
        expectEquals (value.as14BitInt(), expectedValueAs14BitInt);
        expectFloatWithinRelativeError (value.asSignedFloat(), expectedValueAsSignedFloat, 0.0001f);
        expectFloatWithinRelativeError (value.asUnsignedFloat(), expectedValueAsUnsignedFloat, 0.0001f);
    }

    //==============================================================================
    void expectFloatWithinRelativeError (float actualValue, float expectedValue, float maxRelativeError)
    {
        const float maxAbsoluteError = jmax (1.0f, std::abs (expectedValue)) * maxRelativeError;
        expect (std::abs (expectedValue - actualValue) < maxAbsoluteError);
    }
};

static MPEValueTests MPEValueUnitTests;

#endif

} // namespace juce
