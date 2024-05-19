// File: FFTModuleCoroutine.h
#include <coroutine>
#include <vector>

class Reader {
public:
    // ... existing code ...

    // This is a placeholder function. Replace it with your actual implementation.
    std::vector<float> readData() {
        // ... read data from the reader ...
        return std::vector<float>();  // placeholder
    }
};

class FFTData {
public:
    // ... existing code ...

    // This is a placeholder function. Replace it with your actual implementation.
    void copyData(const std::vector<float>& data) {
        // ... copy data into the fftData buffer ...
    }
};

class ReaderToFftDataCopyTask {
public:
    ReaderToFftDataCopyTask(Reader& reader, FFTData& fftData)
        : reader(reader), fftData(fftData) {}

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        // Read data from the reader
        auto data = reader.readData();

        // Copy data into the fftData buffer
        fftData.copyData(data);

        // Resume the coroutine
        handle.resume();
    }

    void await_resume() {}

private:
    Reader& reader;
    FFTData& fftData;
};

std::coroutine_handle<> readerToFftDataCopyCoroutine(Reader& reader, FFTData& fftData)
{
    co_await ReaderToFftDataCopyTask(reader, fftData);
}

int useTimeSlice() override
{
    auto currentThreadId = juce::Thread::getCurrentThreadId();

    if (currentThreadId == dataThread.getThreadId())
    {
        // This is the data thread
        readyToWriteSemaphore[writeBufferIndex].acquire();  // Wait for the draw thread to finish reading

        const juce::ScopedLock lock(criticalSections[writeBufferIndex]);  // Lock the critical section for the write buffer

        // Create a coroutine handle for the readerToFftDataCopyCoroutine coroutine
        auto handle = readerToFftDataCopyCoroutine(reader, fftDataWrite);

        // Start or continue the execution of the coroutine
        if (!handle.done())
            handle.resume();

        readyToReadSemaphore[writeBufferIndex].release();  // Signal the draw thread that data is ready

        // switch the write buffer
        writeBufferIndex ^= 1;  // Toggle the write buffer index
        fftDataWrite = fftDataBuffers[writeBufferIndex];
    }
    else if (currentThreadId == drawThread.getThreadId())
    {
        // This is the draw thread
        readyToReadSemaphore[readBufferIndex].acquire();  // Wait for the data thread to finish writing

        const juce::ScopedLock lock(criticalSections[readBufferIndex]);  // Lock the critical section for the read buffer

        // Call drawNextLineOfSpectrogram() with fftData[readBufferIndex] here
        // ...

        readyToWriteSemaphore[readBufferIndex].release();  // Signal the data thread that it can start writing

        // switch the read buffer
        readBufferIndex ^= 1;  // Toggle the read buffer index
        fftData = fftDataBuffers[readBufferIndex];
    }

    return 1000 / 60;  // Return the time to wait until the next call (in milliseconds)
}

#include <experimental/coroutine>

class ReaderToFftDataCopyGenerator
{
public:
    struct promise_type
    {
        ReaderToFftDataCopyGenerator get_return_object() { return this; }
        std::experimental::suspend_never initial_suspend() { return {}; }
        std::experimental::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    ReaderToFftDataCopyGenerator(promise_type* p) : m_promise(p) {}

    bool move_next() { return m_promise->m_coroutine.resume(); }

    // This is a placeholder function. Replace it with your actual implementation.
    float get_value() const
    {
        // ... return the current value ...
        return 0.0f;  // placeholder
    }

private:
    promise_type* m_promise;
};

ReaderToFftDataCopyGenerator readerToFftDataCopyCoroutine(Reader& reader, FFTData& fftData)
{
    for (juce::int64 readerStartSample = 0; readerStartSample < reader.lengthInSamples; readerStartSample += fftSize)
    {
        // Read data from the reader
        auto data = reader.readData();

        // Copy data into the fftData buffer
        fftData.copyData(data);

        co_yield;  // Yield execution back to the caller
    }
}


class SpectrogramComponent
    : public AudioAppComponent
    , private Timer
    , private juce::TimeSliceClient
{
    // ... existing code ...

    struct ReaderToFftDataCopyAwaitable
    {
        SpectrogramComponent* component;
        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) { component->readerToFftDataCopyCoroutine(handle); }
        void await_resume() {}
    };

    ReaderToFftDataCopyAwaitable readerToFftDataCopy()
    {
        return { this };
    }

    void readerToFftDataCopyCoroutine(std::coroutine_handle<> handle)
    {
        AudioBuffer<float> theAudioBuffer(reader->numChannels, fftSize);

        for (juce::int64 readerStartSample = 0; readerStartSample < reader->lengthInSamples; readerStartSample += fftSize)
        {
            reader->read(&theAudioBuffer, 0, fftSize, readerStartSample, true, true);

            for (auto sampleNbr = 0; sampleNbr < theAudioBuffer.getNumSamples(); sampleNbr++)
            {
                fftData[sampleNbr] = 0.0f;
                for (auto channelNbr = 0; channelNbr < theAudioBuffer.getNumChannels(); channelNbr++)
                {
                    fftData[sampleNbr] += theAudioBuffer.getSample(channelNbr, sampleNbr);
                }
            }

            // Yield execution back to the caller
            handle.yield();
        }

        // Resume the coroutine
        handle.resume();
    }

    // ... existing code ...
};

int useTimeSlice() override
{
    // ... existing code ...

    // Call the coroutine
    auto task = readerToFftDataCopy();
    while (!task.await_ready())
    {
        task.await_suspend({});
    }

    // ... existing code ...
}

std::generator<bool> readerToFftDataCopy(Reader& reader, FFTData& fftData)
{
    for (juce::int64 readerStartSample = 0; readerStartSample < reader.lengthInSamples; readerStartSample += fftSize)
    {
        // Read data from the reader
        auto data = reader.readData();

        // Copy data into the fftData buffer
        fftData.copyData(data);

        co_yield true;  // Yield execution back to the caller
    }
}

int useTimeSlice() override
{
    // ... existing code ...

    // Create a generator object for the readerToFftDataCopy coroutine
    auto generator = readerToFftDataCopy(reader, fftDataWrite);

    // Get iterators for the generator
    auto begin = generator.begin();
    auto end = generator.end();

    // Step through the coroutine
    for (auto it = begin; it != end; ++it)
    {
        // The coroutine is suspended at this point
        // You can do other work here if needed
    }

    // ... existing code ...
}
