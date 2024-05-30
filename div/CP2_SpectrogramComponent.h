#include <vector>
#include <thread>
#include <future>

class SpectrogramComponent {
    // ...

    std::vector<std::future<void>> fftFutures;

    void processAudioData() {
        // Divide the audio data into chunks
        int numChunks = std::thread::hardware_concurrency();
        int chunkSize = fftSize / numChunks;

        // Process each chunk in a separate thread
        fftFutures.clear();
        for (int i = 0; i < numChunks; ++i) {
            fftFutures.push_back(std::async(std::launch::async, [this, i, chunkSize]() {
                // Copy the chunk of audio data to a local buffer
                std::vector<float> buffer(chunkSize);
                std::copy(fftDataInBuffer.begin() + i * chunkSize,
                          fftDataInBuffer.begin() + (i + 1) * chunkSize,
                          buffer.begin());

                // Perform FFT on the local buffer
                doFFT(buffer.data(), chunkSize);

                // Copy the FFT results back to the main buffer
                std::copy(buffer.begin(), buffer.end(),
                          fftDataInBuffer.begin() + i * chunkSize);
            }));
        }

        // Wait for all threads to finish
        for (auto& future : fftFutures) {
            future.wait();
        }
    }

    // ...
};
