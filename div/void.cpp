#include "spleeterpp/Spleeter.hpp"
#include <vector>
#include <iostream>

// Function to separate audio using Spleeter
void spleeter_separate(const float* input_buffer, size_t buffer_size, float* output_buffer)
{
    try
    {
        // Initialize Spleeter with the 2 stems model
        spleeterpp::Spleeter spleeter("spleeter:2stems");

        // Convert input buffer to a vector
        std::vector<float> input_audio(input_buffer, input_buffer + buffer_size);

        // Separate the audio
        auto result = spleeter.separate(input_audio);

        // Extract the separated audio from the result
        const auto& separated_audio = result["vocals"]; // or "accompaniment"
        std::copy(separated_audio.begin(), separated_audio.end(), output_buffer);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in spleeter_separate: " << e.what() << std::endl;
        throw;
    }
}

int main()
{
    // Example usage
    const size_t buffer_size = 44100; // Example buffer size
    float input_buffer[buffer_size] = { /* Your input audio data */ };
    float output_buffer[buffer_size] = { 0 };

    spleeter_separate(input_buffer, buffer_size, output_buffer);

    // Process the output_buffer as needed

    return 0;
}
