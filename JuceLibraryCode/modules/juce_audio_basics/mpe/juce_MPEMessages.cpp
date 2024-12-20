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

MidiBuffer MPEMessages::setLowerZone (int numMemberChannels, int perNotePitchbendRange, int masterPitchbendRange)
{
    auto buffer = MidiRPNGenerator::generate (1, zoneLayoutMessagesRpnNumber, numMemberChannels, false, false);

    buffer.addEvents (setLowerZonePerNotePitchbendRange (perNotePitchbendRange), 0, -1, 0);
    buffer.addEvents (setLowerZoneMasterPitchbendRange (masterPitchbendRange), 0, -1, 0);

    return buffer;
}

MidiBuffer MPEMessages::setUpperZone (int numMemberChannels, int perNotePitchbendRange, int masterPitchbendRange)
{
    auto buffer = MidiRPNGenerator::generate (16, zoneLayoutMessagesRpnNumber, numMemberChannels, false, false);

    buffer.addEvents (setUpperZonePerNotePitchbendRange (perNotePitchbendRange), 0, -1, 0);
    buffer.addEvents (setUpperZoneMasterPitchbendRange (masterPitchbendRange), 0, -1, 0);

    return buffer;
}

MidiBuffer MPEMessages::setLowerZonePerNotePitchbendRange (int perNotePitchbendRange)
{
    return MidiRPNGenerator::generate (2, 0, perNotePitchbendRange, false, false);
}

MidiBuffer MPEMessages::setUpperZonePerNotePitchbendRange (int perNotePitchbendRange)
{
    return MidiRPNGenerator::generate (15, 0, perNotePitchbendRange, false, false);
}

MidiBuffer MPEMessages::setLowerZoneMasterPitchbendRange (int masterPitchbendRange)
{
    return MidiRPNGenerator::generate (1, 0, masterPitchbendRange, false, false);
}

MidiBuffer MPEMessages::setUpperZoneMasterPitchbendRange (int masterPitchbendRange)
{
    return MidiRPNGenerator::generate (16, 0, masterPitchbendRange, false, false);
}

MidiBuffer MPEMessages::clearLowerZone()
{
    return MidiRPNGenerator::generate (1, zoneLayoutMessagesRpnNumber, 0, false, false);
}

MidiBuffer MPEMessages::clearUpperZone()
{
    return MidiRPNGenerator::generate (16, zoneLayoutMessagesRpnNumber, 0, false, false);
}

MidiBuffer MPEMessages::clearAllZones()
{
    MidiBuffer buffer;

    buffer.addEvents (clearLowerZone(), 0, -1, 0);
    buffer.addEvents (clearUpperZone(), 0, -1, 0);

    return buffer;
}

MidiBuffer MPEMessages::setZoneLayout (MPEZoneLayout layout)
{
    MidiBuffer buffer;

    buffer.addEvents (clearAllZones(), 0, -1, 0);

    auto lowerZone = layout.getLowerZone();
    if (lowerZone.isActive())
        buffer.addEvents (setLowerZone (lowerZone.numMemberChannels,
                                        lowerZone.perNotePitchbendRange,
                                        lowerZone.masterPitchbendRange),
                          0, -1, 0);

    auto upperZone = layout.getUpperZone();
    if (upperZone.isActive())
        buffer.addEvents (setUpperZone (upperZone.numMemberChannels,
                                        upperZone.perNotePitchbendRange,
                                        upperZone.masterPitchbendRange),
                          0, -1, 0);

    return buffer;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class MPEMessagesTests final : public UnitTest
{
public:
    MPEMessagesTests()
        : UnitTest ("MPEMessages class", UnitTestCategories::midi)
    {}

    void runTest() override
    {
        beginTest ("add zone");
        {
            {
                MidiBuffer buffer = MPEMessages::setLowerZone (7);

                const uint8 expectedBytes[] =
                {
                    0xb0, 0x64, 0x06, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x07, // set up zone
                    0xb1, 0x64, 0x00, 0xb1, 0x65, 0x00, 0xb1, 0x06, 0x30, // per-note pbrange (default = 48)
                    0xb0, 0x64, 0x00, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x02  // master pbrange (default = 2)
                };

                testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
            }
            {
                MidiBuffer buffer = MPEMessages::setUpperZone (5, 96, 0);

                const uint8 expectedBytes[] =
                {
                    0xbf, 0x64, 0x06, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x05, // set up zone
                    0xbe, 0x64, 0x00, 0xbe, 0x65, 0x00, 0xbe, 0x06, 0x60, // per-note pbrange (custom)
                    0xbf, 0x64, 0x00, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x00  // master pbrange (custom)
                };

                testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
            }
        }

        beginTest ("set per-note pitchbend range");
        {
            MidiBuffer buffer = MPEMessages::setLowerZonePerNotePitchbendRange (96);

            const uint8 expectedBytes[] = { 0xb1, 0x64, 0x00, 0xb1, 0x65, 0x00, 0xb1, 0x06, 0x60 };

            testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
        }


        beginTest ("set master pitchbend range");
        {
            MidiBuffer buffer = MPEMessages::setUpperZoneMasterPitchbendRange (60);

            const uint8 expectedBytes[] = { 0xbf, 0x64, 0x00, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x3c };

            testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
        }

        beginTest ("clear all zones");
        {
            MidiBuffer buffer = MPEMessages::clearAllZones();

            const uint8 expectedBytes[] = { 0xb0, 0x64, 0x06, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x00, // clear lower zone
                                            0xbf, 0x64, 0x06, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x00  // clear upper zone
                                          };

            testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
        }

        beginTest ("set complete state");
        {
            MPEZoneLayout layout;

            layout.setLowerZone (7, 96, 0);
            layout.setUpperZone (7);

            MidiBuffer buffer = MPEMessages::setZoneLayout (layout);

            const uint8 expectedBytes[] = {
                0xb0, 0x64, 0x06, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x00,  // clear lower zone
                0xbf, 0x64, 0x06, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x00,  // clear upper zone
                0xb0, 0x64, 0x06, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x07,  // set lower zone
                0xb1, 0x64, 0x00, 0xb1, 0x65, 0x00, 0xb1, 0x06, 0x60,  // per-note pbrange (custom)
                0xb0, 0x64, 0x00, 0xb0, 0x65, 0x00, 0xb0, 0x06, 0x00,  // master pbrange (custom)
                0xbf, 0x64, 0x06, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x07,  // set upper zone
                0xbe, 0x64, 0x00, 0xbe, 0x65, 0x00, 0xbe, 0x06, 0x30,  // per-note pbrange (default = 48)
                0xbf, 0x64, 0x00, 0xbf, 0x65, 0x00, 0xbf, 0x06, 0x02   // master pbrange (default = 2)
            };

            testMidiBuffer (buffer, expectedBytes, sizeof (expectedBytes));
        }
    }

private:
    //==============================================================================
    void testMidiBuffer (MidiBuffer& buffer, const uint8* expectedBytes, int expectedBytesSize)
    {
        uint8 actualBytes[128] = { 0 };
        extractRawBinaryData (buffer, actualBytes, sizeof (actualBytes));

        expectEquals (std::memcmp (actualBytes, expectedBytes, (std::size_t) expectedBytesSize), 0);
    }

    //==============================================================================
    void extractRawBinaryData (const MidiBuffer& midiBuffer, const uint8* bufferToCopyTo, std::size_t maxBytes)
    {
        std::size_t pos = 0;

        for (const auto metadata : midiBuffer)
        {
            const uint8* data = metadata.data;
            std::size_t dataSize = (std::size_t) metadata.numBytes;

            if (pos + dataSize > maxBytes)
                return;

            std::memcpy ((void*) (bufferToCopyTo + pos), data, dataSize);
            pos += dataSize;
        }
    }
};

static MPEMessagesTests MPEMessagesUnitTests;

#endif

} // namespace juce
