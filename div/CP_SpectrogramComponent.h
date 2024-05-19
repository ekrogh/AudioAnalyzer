class SpectrogramComponent
    : public AudioAppComponent
    , private Timer
    , private juce::TimeSliceClient
{
public:
    // ...

    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
            repaint();
            timerSemaphore.release();  // Release the semaphore
        }
    }

    int useTimeSlice() override
    {
        auto currentThreadId = juce::Thread::getCurrentThreadId();

        if (currentThreadId == dataThread.getThreadId())
        {
            // This is the data thread
            readyToWriteSemaphore[writeBufferIndex].acquire();  // Wait for the draw thread to finish reading

            const juce::ScopedLock lock(criticalSections[writeBufferIndex]);  // Lock the critical section for the write buffer

            // Read data from the reader to fftData[writeBufferIndex] here
            // ...

            readyToReadSemaphore[writeBufferIndex].release();  // Signal the draw thread that data is ready

            // switch the write buffer
            writeBufferIndex ^= 1;  // Toggle the write buffer index
            fftDataWrite = fftDataBuffers[writeBufferIndex];

            if (timerSemaphore.try_acquire())  // Try to acquire the semaphore
            {
                drawNextLineOfSpectrogram();
            }
        }

        return 0;  // Return the time slice to use
    }

private:
    std::binary_semaphore timerSemaphore{0};  // Add this line to declare the semaphore
};
