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

class IIRFilter;

//==============================================================================
/**
    A set of coefficients for use in an IIRFilter object.

    @see IIRFilter

    @tags{Audio}
*/
class JUCE_API  IIRCoefficients
{
public:
    //==============================================================================
    /** Creates a null set of coefficients (which will produce silence). */
    IIRCoefficients() noexcept;

    /** Directly constructs an object from the raw coefficients.
        Most people will want to use the static methods instead of this, but
        the constructor is public to allow tinkerers to create their own custom
        filters!
    */
    IIRCoefficients (double c1, double c2, double c3,
                     double c4, double c5, double c6) noexcept;

    /** Creates a copy of another filter. */
    IIRCoefficients (const IIRCoefficients&) noexcept;
    /** Creates a copy of another filter. */
    IIRCoefficients& operator= (const IIRCoefficients&) noexcept;
    /** Destructor. */
    ~IIRCoefficients() noexcept;

    //==============================================================================
    /** Returns the coefficients for a low-pass filter. */
    static IIRCoefficients makeLowPass (double sampleRate,
                                        double frequency) noexcept;

    /** Returns the coefficients for a low-pass filter with variable Q. */
    static IIRCoefficients makeLowPass (double sampleRate,
                                        double frequency,
                                        double Q) noexcept;

    //==============================================================================
    /** Returns the coefficients for a high-pass filter. */
    static IIRCoefficients makeHighPass (double sampleRate,
                                         double frequency) noexcept;

    /** Returns the coefficients for a high-pass filter with variable Q. */
    static IIRCoefficients makeHighPass (double sampleRate,
                                         double frequency,
                                         double Q) noexcept;

    //==============================================================================
    /** Returns the coefficients for a band-pass filter. */
    static IIRCoefficients makeBandPass (double sampleRate, double frequency) noexcept;

    /** Returns the coefficients for a band-pass filter with variable Q. */
    static IIRCoefficients makeBandPass (double sampleRate,
                                         double frequency,
                                         double Q) noexcept;

    //==============================================================================
    /** Returns the coefficients for a notch filter. */
    static IIRCoefficients makeNotchFilter (double sampleRate, double frequency) noexcept;

    /** Returns the coefficients for a notch filter with variable Q. */
    static IIRCoefficients makeNotchFilter (double sampleRate,
                                            double frequency,
                                            double Q) noexcept;

    //==============================================================================
    /** Returns the coefficients for an all-pass filter. */
    static IIRCoefficients makeAllPass (double sampleRate, double frequency) noexcept;

    /** Returns the coefficients for an all-pass filter with variable Q. */
    static IIRCoefficients makeAllPass (double sampleRate,
                                        double frequency,
                                        double Q) noexcept;

    //==============================================================================
    /** Returns the coefficients for a low-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the low frequencies are multiplied by, so values
        greater than 1.0 will boost the low frequencies, values less than 1.0 will
        attenuate them.
    */
    static IIRCoefficients makeLowShelf (double sampleRate,
                                         double cutOffFrequency,
                                         double Q,
                                         float gainFactor) noexcept;

    /** Returns the coefficients for a high-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the high frequencies are multiplied by, so values
        greater than 1.0 will boost the high frequencies, values less than 1.0 will
        attenuate them.
    */
    static IIRCoefficients makeHighShelf (double sampleRate,
                                          double cutOffFrequency,
                                          double Q,
                                          float gainFactor) noexcept;

    /** Returns the coefficients for a peak filter centred around a
        given frequency, with a variable Q and gain.

        The gain is a scale factor that the centre frequencies are multiplied by, so
        values greater than 1.0 will boost the centre frequencies, values less than
        1.0 will attenuate them.
    */
    static IIRCoefficients makePeakFilter (double sampleRate,
                                           double centreFrequency,
                                           double Q,
                                           float gainFactor) noexcept;

    //==============================================================================
    /** The raw coefficients.
        You should leave these numbers alone unless you really know what you're doing.
    */
    float coefficients[5];
};

//==============================================================================
/**
    An IIR filter that can perform low, high, or band-pass filtering on an
    audio signal.

    @see IIRCoefficient, IIRFilterAudioSource

    @tags{Audio}
*/
template <typename Mutex>
class JUCE_API  IIRFilterBase
{
public:
    //==============================================================================
    /** Creates a filter.

        Initially the filter is inactive, so will have no effect on samples that
        you process with it. Use the setCoefficients() method to turn it into the
        type of filter needed.
    */
    IIRFilterBase() noexcept;

    /** Creates a copy of another filter. */
    IIRFilterBase (const IIRFilterBase&) noexcept;

    //==============================================================================
    /** Clears the filter so that any incoming data passes through unchanged. */
    void makeInactive() noexcept;

    /** Applies a set of coefficients to this filter. */
    void setCoefficients (const IIRCoefficients& newCoefficients) noexcept;

    /** Returns the coefficients that this filter is using. */
    IIRCoefficients getCoefficients() const noexcept    { return coefficients; }

    //==============================================================================
    /** Resets the filter's processing pipeline, ready to start a new stream of data.

        Note that this clears the processing state, but the type of filter and
        its coefficients aren't changed. To put a filter into an inactive state, use
        the makeInactive() method.
    */
    void reset() noexcept;

    /** Performs the filter operation on the given set of samples. */
    void processSamples (float* samples, int numSamples) noexcept;

    /** Processes a single sample, without any locking or checking.

        Use this if you need fast processing of a single value, but be aware that
        this isn't thread-safe in the way that processSamples() is.
    */
    float processSingleSampleRaw (float sample) noexcept;

protected:
    //==============================================================================
    Mutex processLock;
    IIRCoefficients coefficients;
    float v1 = 0, v2 = 0;
    bool active = false;

    // The exact meaning of an assignment operator would be ambiguous since the filters are
    // stateful. If you want to copy the coefficients, then just use setCoefficients().
    IIRFilter& operator= (const IIRFilter&) = delete;

    JUCE_LEAK_DETECTOR (IIRFilter)
};

/**
    An IIR filter that can perform low, high, or band-pass filtering on an
    audio signal, and which attempts to implement basic thread-safety.

    This class synchronises calls to some of its member functions, making it
    safe (although not necessarily real-time-safe) to reset the filter or
    apply new coefficients while the filter is processing on another thread.
    In most cases this style of internal locking should not be used, and you
    should attempt to provide thread-safety at a higher level in your program.
    If you can guarantee that calls to the filter will be synchronised externally,
    you could consider switching to SingleThreadedIIRFilter instead.

    @see SingleThreadedIIRFilter, IIRCoefficient, IIRFilterAudioSource

    @tags{Audio}
*/
class IIRFilter : public IIRFilterBase<SpinLock>
{
public:
    using IIRFilterBase::IIRFilterBase;
};

/**
    An IIR filter that can perform low, high, or band-pass filtering on an
    audio signal, with no thread-safety guarantees.

    You should use this class if you need an IIR filter, and don't plan to
    call its member functions from multiple threads at once.

    @see IIRFilter, IIRCoefficient, IIRFilterAudioSource

    @tags{Audio}
*/
class SingleThreadedIIRFilter : public IIRFilterBase<DummyCriticalSection>
{
public:
    using IIRFilterBase::IIRFilterBase;
};

} // namespace juce
