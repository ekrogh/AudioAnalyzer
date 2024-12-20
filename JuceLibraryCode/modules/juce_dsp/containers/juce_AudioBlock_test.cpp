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

namespace juce::dsp
{

#if JUCE_USE_SIMD
template <typename SampleType>
String& operator<< (String& str, SIMDRegister<SampleType>) { return str; }
#endif

template <typename SampleType>
class AudioBlockUnitTests final : public UnitTest
{
public:
    //==============================================================================
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    AudioBlockUnitTests()
        : UnitTest ("AudioBlock", UnitTestCategories::dsp)
    {
        for (auto v : { &data, &otherData })
            for (auto& channel : *v)
                channel = allocateAlignedMemory (numSamples);

        block      = { data.data(),      data.size(),      (size_t) numSamples };
        otherBlock = { otherData.data(), otherData.size(), (size_t) numSamples };

        resetBlocks();
    }

    ~AudioBlockUnitTests() override
    {
        for (auto v : { &data, &otherData })
            for (auto channel : *v)
                deallocateAlignedMemory (channel);
    }

    void runTest() override
    {
        beginTest ("Equality");
        {
            expect (block == block);
            expect (block != otherBlock);
        }

        beginTest ("Constructors");
        {
            expect (block == AudioBlock<SampleType> (data.data(), data.size(), numSamples));
            expect (block == AudioBlock<SampleType> (data.data(), data.size(), (size_t) 0, numSamples));
            expect (block == AudioBlock<SampleType> (block));

            expect (block == AudioBlock<const SampleType> (data.data(), data.size(), numSamples));
            expect (block == AudioBlock<const SampleType> (data.data(), data.size(), (size_t) 0, numSamples));
            expect (block == AudioBlock<const SampleType> (block));
        }

        beginTest ("Swap");
        {
            resetBlocks();

            expect (block != otherBlock);
            expectEquals (block.getSample (0, 0), SampleType (1.0));
            expectEquals (block.getSample (0, 4), SampleType (5.0));
            expectEquals (otherBlock.getSample (0, 0), SampleType (-1.0));
            expectEquals (otherBlock.getSample (0, 3), SampleType (-4.0));

            block.swap (otherBlock);

            expect (block != otherBlock);
            expectEquals (otherBlock.getSample (0, 0), SampleType (1.0));
            expectEquals (otherBlock.getSample (0, 4), SampleType (5.0));
            expectEquals (block.getSample (0, 0), SampleType (-1.0));
            expectEquals (block.getSample (0, 3), SampleType (-4.0));

            block.swap (otherBlock);

            expectEquals (block.getSample (0, 0), SampleType (1.0));
            expectEquals (block.getSample (0, 4), SampleType (5.0));
            expectEquals (otherBlock.getSample (0, 0), SampleType (-1.0));
            expectEquals (otherBlock.getSample (0, 3), SampleType (-4.0));
        }

        beginTest ("Getters and setters");
        {
            resetBlocks();

            expectEquals ((int) block.getNumChannels(), (int) data.size());
            expectEquals ((int) block.getNumSamples(), numSamples);

            expectEquals (block.getChannelPointer (0)[2], SampleType (3.0));
            block.getChannelPointer (0)[2] = SampleType (999.0);
            expectEquals (block.getChannelPointer (0)[2], SampleType (999.0));

            expectEquals (block.getSample (0, 4), SampleType (5.0));
            expectEquals (block.getSample (1, 4), SampleType (11.0));

            expectEquals (block.getSingleChannelBlock (1).getSample (0, 3), block.getSample (1, 3));

            expectEquals (block.getSubsetChannelBlock (0, 2).getSample (1, 3), block.getSample (1, 3));
            expectEquals (block.getSubsetChannelBlock (1, 1).getSample (0, 3), block.getSample (1, 3));

            block.setSample (1, 1, SampleType (777.0));
            expectEquals (block.getSample (1, 1), SampleType (777.0));

            block.addSample (1, 1, SampleType (1.0));
            expectEquals (block.getSample (1, 1), SampleType (778.0));
        }

        beginTest ("Basic copying");
        {
            block.clear();
            expectEquals (block.getSample (0, 2), SampleType (0.0));
            expectEquals (block.getSample (1, 4), SampleType (0.0));

            block.fill ((NumericType) 456.0);
            expectEquals (block.getSample (0, 2), SampleType (456.0));
            expectEquals (block.getSample (1, 4), SampleType (456.0));

            block.copyFrom (otherBlock);
            expect (block != otherBlock);
            expectEquals (block.getSample (0, 2), otherBlock.getSample (0, 2));
            expectEquals (block.getSample (1, 4), otherBlock.getSample (1, 4));

            resetBlocks();

            SampleType testSample1 = block.getSample (0, 2);
            SampleType testSample2 = block.getSample (1, 3);
            expectNotEquals (testSample1, block.getSample (0, 4));
            expectNotEquals (testSample2, block.getSample (1, 5));
            block.move (0, 2);
            expectEquals (block.getSample (0, 4), testSample1);
            expectEquals (block.getSample (1, 5), testSample2);
        }

        beginTest ("Addition");
        {
            resetBlocks();

            block.add ((NumericType) 15.0);
            expectEquals (block.getSample (0, 4), SampleType (20.0));
            expectEquals (block.getSample (1, 4), SampleType (26.0));

            block.add (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (15.0));
            expectEquals (block.getSample (1, 4), SampleType (15.0));

            block.replaceWithSumOf (otherBlock, (NumericType) 9.0);
            expectEquals (block.getSample (0, 4), SampleType (4.0));
            expectEquals (block.getSample (1, 4), SampleType (-2.0));

            resetBlocks();

            block.replaceWithSumOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (0.0));
            expectEquals (block.getSample (1, 4), SampleType (0.0));
        }

        beginTest ("Subtraction");
        {
            resetBlocks();

            block.subtract ((NumericType) 15.0);
            expectEquals (block.getSample (0, 4), SampleType (-10.0));
            expectEquals (block.getSample (1, 4), SampleType (-4.0));

            block.subtract (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (-5.0));
            expectEquals (block.getSample (1, 4), SampleType (7.0));

            block.replaceWithDifferenceOf (otherBlock, (NumericType) 9.0);
            expectEquals (block.getSample (0, 4), SampleType (-14.0));
            expectEquals (block.getSample (1, 4), SampleType (-20.0));

            resetBlocks();

            block.replaceWithDifferenceOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (10.0));
            expectEquals (block.getSample (1, 4), SampleType (22.0));
        }

        beginTest ("Multiplication");
        {
            resetBlocks();

            block.multiplyBy ((NumericType) 10.0);
            expectEquals (block.getSample (0, 4), SampleType (50.0));
            expectEquals (block.getSample (1, 4), SampleType (110.0));

            block.multiplyBy (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (-250.0));
            expectEquals (block.getSample (1, 4), SampleType (-1210.0));

            block.replaceWithProductOf (otherBlock, (NumericType) 3.0);
            expectEquals (block.getSample (0, 4), SampleType (-15.0));
            expectEquals (block.getSample (1, 4), SampleType (-33.0));

            resetBlocks();

            block.replaceWithProductOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (-25.0));
            expectEquals (block.getSample (1, 4), SampleType (-121.0));
        }

        beginTest ("Multiply add");
        {
            resetBlocks();

            block.addProductOf (otherBlock, (NumericType) -1.0);
            expectEquals (block.getSample (0, 4), SampleType (10.0));
            expectEquals (block.getSample (1, 4), SampleType (22.0));

            block.addProductOf (otherBlock, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (35.0));
            expectEquals (block.getSample (1, 4), SampleType (143.0));
        }

        beginTest ("Negative abs min max");
        {
            resetBlocks();
            otherBlock.negate();

            block.add (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (10.0));
            expectEquals (block.getSample (1, 4), SampleType (22.0));

            block.replaceWithNegativeOf (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (-5.0));
            expectEquals (block.getSample (1, 4), SampleType (-11.0));

            block.clear();
            otherBlock.negate();
            block.replaceWithAbsoluteValueOf (otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (5.0));
            expectEquals (block.getSample (1, 4), SampleType (11.0));

            resetBlocks();
            block.replaceWithMinOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (-5.0));
            expectEquals (block.getSample (1, 4), SampleType (-11.0));

            resetBlocks();
            block.replaceWithMaxOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), SampleType (5.0));
            expectEquals (block.getSample (1, 4), SampleType (11.0));

            resetBlocks();
            auto range = block.findMinAndMax();
            expectEquals (SampleType (range.getStart()), SampleType (1.0));
            expectEquals (SampleType (range.getEnd()), SampleType (12.0));
        }

        beginTest ("Operators");
        {
            resetBlocks();
            block += (NumericType) 10.0;
            expectEquals (block.getSample (0, 4), SampleType (15.0));
            expectEquals (block.getSample (1, 4), SampleType (21.0));
            block += otherBlock;
            expectEquals (block.getSample (0, 4), SampleType (10.0));
            expectEquals (block.getSample (1, 4), SampleType (10.0));

            resetBlocks();
            block -= (NumericType) 10.0;
            expectEquals (block.getSample (0, 4), SampleType (-5.0));
            expectEquals (block.getSample (1, 4), SampleType (1.0));
            block -= otherBlock;
            expectEquals (block.getSample (0, 4), SampleType (0.0));
            expectEquals (block.getSample (1, 4), SampleType (12.0));

            resetBlocks();
            block *= (NumericType) 10.0;
            expectEquals (block.getSample (0, 4), SampleType (50.0));
            expectEquals (block.getSample (1, 4), SampleType (110.0));
            block *= otherBlock;
            expectEquals (block.getSample (0, 4), SampleType (-250.0));
            expectEquals (block.getSample (1, 4), SampleType (-1210.0));
        }

        beginTest ("Process");
        {
            resetBlocks();
            AudioBlock<SampleType>::process (block, otherBlock, [] (SampleType x) { return x + (NumericType) 1.0; });
            expectEquals (otherBlock.getSample (0, 4), SampleType (6.0));
            expectEquals (otherBlock.getSample (1, 4), SampleType (12.0));
        }

        beginTest ("Copying");
        {
            resetBlocks();
            copyingTests();
        }

        beginTest ("Smoothing");
        {
            resetBlocks();
            smoothedValueTests();
        }
    }

private:
    //==============================================================================
    void copyingTests()
    {
        if constexpr (std::is_scalar_v<SampleType>)
        {
            auto unchangedElement1 = block.getSample (0, 4);
            auto unchangedElement2 = block.getSample (1, 1);

            AudioBuffer<SampleType> otherBuffer (otherData.data(), (int) otherData.size(), numSamples);

            block.copyFrom (otherBuffer, 1, 2, 2);

            expectEquals (block.getSample (0, 4), unchangedElement1);
            expectEquals (block.getSample (1, 1), unchangedElement2);
            expectEquals (block.getSample (0, 2), otherBuffer.getSample (0, 1));
            expectEquals (block.getSample (1, 3), otherBuffer.getSample (1, 2));

            resetBlocks();

            unchangedElement1 = otherBuffer.getSample (0, 4);
            unchangedElement2 = otherBuffer.getSample (1, 3);

            block.copyTo (otherBuffer, 2, 1, 2);

            expectEquals (otherBuffer.getSample (0, 4), unchangedElement1);
            expectEquals (otherBuffer.getSample (1, 3), unchangedElement2);
            expectEquals (otherBuffer.getSample (0, 1), block.getSample (0, 2));
            expectEquals (otherBuffer.getSample (1, 2), block.getSample (1, 3));
        }
       #if JUCE_USE_SIMD
        else
        {
            auto numSIMDElements = SIMDRegister<NumericType>::SIMDNumElements;
            AudioBuffer<NumericType> numericData ((int) block.getNumChannels(),
                                                  (int) (block.getNumSamples() * numSIMDElements));

            for (int c = 0; c < numericData.getNumChannels(); ++c)
                std::fill_n (numericData.getWritePointer (c), numericData.getNumSamples(), (NumericType) 1.0);

            numericData.applyGainRamp (0, numericData.getNumSamples(), (NumericType) 0.127, (NumericType) 17.3);

            auto lastUnchangedIndexBeforeCopiedRange = (int) ((numSIMDElements * 2) - 1);
            auto firstUnchangedIndexAfterCopiedRange = (int) ((numSIMDElements * 4) + 1);
            auto unchangedElement1 = numericData.getSample (0, lastUnchangedIndexBeforeCopiedRange);
            auto unchangedElement2 = numericData.getSample (1, firstUnchangedIndexAfterCopiedRange);

            block.copyTo (numericData, 1, 2, 2);

            expectEquals (numericData.getSample (0, lastUnchangedIndexBeforeCopiedRange), unchangedElement1);
            expectEquals (numericData.getSample (1, firstUnchangedIndexAfterCopiedRange), unchangedElement2);
            expect (SampleType (numericData.getSample (0, 2 * (int) numSIMDElements)) == block.getSample (0, 1));
            expect (SampleType (numericData.getSample (1, 3 * (int) numSIMDElements)) == block.getSample (1, 2));

            numericData.applyGainRamp (0, numericData.getNumSamples(), (NumericType) 15.1, (NumericType) 0.7);

            auto unchangedSIMDElement1 = block.getSample (0, 1);
            auto unchangedSIMDElement2 = block.getSample (1, 4);

            block.copyFrom (numericData, 1, 2, 2);

            expect (block.getSample (0, 1) == unchangedSIMDElement1);
            expect (block.getSample (1, 4) == unchangedSIMDElement2);
            expectEquals (block.getSample (0, 2).get (0), numericData.getSample (0, (int) numSIMDElements));
            expectEquals (block.getSample (1, 3).get (0), numericData.getSample (1, (int) (numSIMDElements * 2)));

            if (numSIMDElements > 1)
            {
                expectEquals (block.getSample (0, 2).get (1), numericData.getSample (0, (int) (numSIMDElements + 1)));
                expectEquals (block.getSample (1, 3).get (1), numericData.getSample (1, (int) ((numSIMDElements * 2) + 1)));
            }
        }
       #endif
    }

    //==============================================================================
    void smoothedValueTests()
    {
        if constexpr (std::is_scalar_v<SampleType>)
        {
            block.fill ((SampleType) 1.0);
            SmoothedValue<SampleType> sv { (SampleType) 1.0 };
            sv.reset (1, 4);
            sv.setTargetValue ((SampleType) 0.0);

            block.multiplyBy (sv);
            expect (block.getSample (0, 2) < (SampleType) 1.0);
            expect (block.getSample (1, 2) < (SampleType) 1.0);
            expect (block.getSample (0, 2) > (SampleType) 0.0);
            expect (block.getSample (1, 2) > (SampleType) 0.0);
            expectEquals (block.getSample (0, 5), (SampleType) 0.0);
            expectEquals (block.getSample (1, 5), (SampleType) 0.0);

            sv.setCurrentAndTargetValue (-1.0f);
            sv.setTargetValue (0.0f);
            otherBlock.fill (-1.0f);
            block.replaceWithProductOf (otherBlock, sv);
            expect (block.getSample (0, 2) < (SampleType) 1.0);
            expect (block.getSample (1, 2) < (SampleType) 1.0);
            expect (block.getSample (0, 2) > (SampleType) 0.0);
            expect (block.getSample (1, 2) > (SampleType) 0.0);
            expectEquals (block.getSample (0, 5), (SampleType) 0.0);
            expectEquals (block.getSample (1, 5), (SampleType) 0.0);
        }
    }

    //==============================================================================
    void resetBlocks()
    {
        auto value = SampleType (1.0);

        for (size_t c = 0; c < block.getNumChannels(); ++c)
        {
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                block.setSample ((int) c, (int) i, value);
                value += SampleType (1.0);
            }
        }

        otherBlock.replaceWithNegativeOf (block);
    }

    //==============================================================================
    static SampleType* allocateAlignedMemory (int numSamplesToAllocate)
    {
        auto alignmentLowerBound = std::alignment_of_v<SampleType>;
       #if ! JUCE_WINDOWS
        alignmentLowerBound = jmax (sizeof (void*), alignmentLowerBound);
       #endif
        auto alignmentOrder = std::ceil (std::log2 (alignmentLowerBound));
        auto requiredAlignment = (size_t) std::pow (2, alignmentOrder);

        auto size = (size_t) numSamplesToAllocate * sizeof (SampleType);

       #if JUCE_WINDOWS
        auto* memory = _aligned_malloc (size, requiredAlignment);
       #else
        void* memory;
        auto result = posix_memalign (&memory, requiredAlignment, size);

        if (result != 0)
        {
            jassertfalse;
            return nullptr;
        }
       #endif

        return static_cast<SampleType*> (memory);
    }

    void deallocateAlignedMemory (void* address)
    {
       #if JUCE_WINDOWS
        _aligned_free (address);
       #else
        free (address);
       #endif
    }

    //==============================================================================
    static constexpr int numChannels = 2, numSamples = 6;
    std::array<SampleType*, numChannels> data, otherData;
    AudioBlock<SampleType> block, otherBlock;
};

static AudioBlockUnitTests<float> audioBlockFloatUnitTests;
static AudioBlockUnitTests<double> audioBlockDoubleUnitTests;

#if JUCE_USE_SIMD
static AudioBlockUnitTests<SIMDRegister<float>> audioBlockSIMDFloatUnitTests;
static AudioBlockUnitTests<SIMDRegister<double>> audioBlockSIMDDoubleUnitTests;
#endif

} // namespace juce::dsp
