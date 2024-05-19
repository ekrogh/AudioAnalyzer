class SpectrogramComponent
    : public juce::AudioAppComponent
    , private juce::TimeSliceClient
{
public:
    SpectrogramComponent
    (
        juce::AudioFormatManager& FM
        ,
        std::shared_ptr<juce::AudioDeviceManager> SADM
    )
        : formatManager(FM)
        , AudioAppComponent(*SADM)
        , spectrogramImage(juce::Image::RGB, 600, 626, true)
    {
        setOpaque(true);

        forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

        setAudioChannels(1, 2);

        startTimerHz(60);  // Start a timer to call drawNextLineOfSpectrogram() at 60 Hz

        setSize(spectrogramImage.getWidth(), spectrogramImage.getHeight());

        // Start the threads
        dataThread.startThread();
        drawThread.startThread();
    }

    ~SpectrogramComponent() override
    {
        // Stop the threads
        dataThread.stopThread(500);
        drawThread.stopThread(500);
    }

    // ...

    int useTimeSlice() override  // This method is called when the thread starts
    {
        // Read data from the reader to fftData here
        // ...

        // Signal the draw thread that data is ready
        dataReady.signal();

        return 1000 / 60;  // Return the time to wait until the next call (in milliseconds)
    }

    void timerCallback() override  // This method is called at 60 Hz
    {
        // Wait for the data thread to signal that data is ready
        dataReady.wait();

        drawNextLineOfSpectrogram();
    }

    // ...

private:
    juce::TimeSliceThread dataThread{ "Data Thread" };
    juce::TimeSliceThread drawThread{ "Draw Thread" };
    juce::WaitableEvent dataReady;
};


int useTimeSlice() override
{
    auto currentThreadId = juce::Thread::getCurrentThreadId();

    if (currentThreadId == dataThread.getThreadId())
    {
        // This is the data thread
        // ...
    }
    else if (currentThreadId == drawThread.getThreadId())
    {
        // This is the draw thread
        // ...
    }

    return 1000 / 60;  // Return the time to wait until the next call (in milliseconds)
}

class SpectrogramComponent
    : public juce::AudioAppComponent
    , private juce::TimeSliceClient
{
public:
    SpectrogramComponent
    (
        juce::AudioFormatManager& FM
        ,
        std::shared_ptr<juce::AudioDeviceManager> SADM
    )
        : formatManager(FM)
        , AudioAppComponent(*SADM)
        , spectrogramImage(juce::Image::RGB, 600, 626, true)
    {
        // ...

        // Add this instance to the TimeSliceThreads
        dataThread.addTimeSliceClient(this);
        drawThread.addTimeSliceClient(this);

        // Start the threads
        dataThread.startThread();
        drawThread.startThread();
    }

    ~SpectrogramComponent() override
    {
        // Remove this instance from the TimeSliceThreads
        dataThread.removeTimeSliceClient(this);
        drawThread.removeTimeSliceClient(this);

        // Stop the threads
        dataThread.stopThread(500);
        drawThread.stopThread(500);
    }

    int useTimeSlice() override
    {
        // This is the code that will be run by the TimeSliceThreads
        // ...
    }

    // ...
};
 
class SpectrogramComponent
    : public juce::AudioAppComponent
    , private juce::TimeSliceClient
{
public:
    // ...

    int useTimeSlice() override
    {
        auto currentThreadId = juce::Thread::getCurrentThreadId();

        if (currentThreadId == dataThread.getThreadId())
        {
            // This is the data thread
            const juce::ScopedLock lock(criticalSection);  // Lock the critical section

            // Read data from the reader to fftData[writeBufferIndex] here
            // ...

            // After filling a buffer, increment writeBufferIndex and wrap it around if necessary
            writeBufferIndex = (writeBufferIndex + 1) % fftData.size();
        }
        else if (currentThreadId == drawThread.getThreadId())
        {
            // This is the draw thread
            const juce::ScopedLock lock(criticalSection);  // Lock the critical section

            // Call drawNextLineOfSpectrogram() with fftData[readBufferIndex] here
            // ...

            // After reading a buffer, increment readBufferIndex and wrap it around if necessary
            readBufferIndex = (readBufferIndex + 1) % fftData.size();
        }

        return 1000 / 60;  // Return the time to wait until the next call (in milliseconds)
    }

    // ...

private:
    juce::CriticalSection criticalSection;
    std::vector<float[2 * fftSize]> fftData;  // Vector of fftData buffers
    std::size_t writeBufferIndex = 0;  // Index of the buffer currently being filled
    std::size_t readBufferIndex = 0;  // Index of the buffer currently being read
    // ...
};

#include <semaphore>

class SpectrogramComponent
    : public juce::AudioAppComponent
    , private juce::TimeSliceClient
{
public:
    // ...

    int useTimeSlice() override
    {
        auto currentThreadId = juce::Thread::getCurrentThreadId();

        if (currentThreadId == dataThread.getThreadId())
        {
            // This is the data thread
            readyToWriteSemaphore[writeBufferIndex].wait();  // Wait for the draw thread to finish reading

            const juce::ScopedLock lock(criticalSections[writeBufferIndex]);  // Lock the critical section for the write buffer

            // Read data from the reader to fftData[writeBufferIndex] here
            // ...

            readyToReadSemaphore[writeBufferIndex].release();  // Signal the draw thread that data is ready
        }
        else if (currentThreadId == drawThread.getThreadId())
        {
            // This is the draw thread
            readyToReadSemaphore[readBufferIndex].wait();  // Wait for the data thread to finish writing

            const juce::ScopedLock lock(criticalSections[readBufferIndex]);  // Lock the critical section for the read buffer

            // Call drawNextLineOfSpectrogram() with fftData[readBufferIndex] here
            // ...

            readyToWriteSemaphore[readBufferIndex].release();  // Signal the data thread that it can start writing
        }

        return 1000 / 60;  // Return the time to wait until the next call (in milliseconds)
    }

    // ...

private:
    std::array<juce::CriticalSection, 2> criticalSections;  // Array of two CriticalSection objects
    std::array<float[2 * fftSize], 2> fftData;  // Array of two fftData buffers
    std::array<std::binary_semaphore, 2> readyToReadSemaphore = { std::binary_semaphore{0}, std::binary_semaphore{0} };  // Semaphores to signal when data is ready to be read
    std::array<std::binary_semaphore, 2> readyToWriteSemaphore = { std::binary_semaphore{1}, std::binary_semaphore{1} };  // Semaphores to signal when data is ready to be written
    std::size_t readBufferIndex = 0;  // Index of the buffer currently being read
    std::size_t writeBufferIndex = 1;  // Index of the buffer currently being filled
    // ...
};
