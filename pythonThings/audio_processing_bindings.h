#pragma once
#ifndef AUDIO_PROCESSING_BINDINGS_H
#define AUDIO_PROCESSING_BINDINGS_H

#include <string>

void spleeter_separate(const float* input_buffer, size_t buffer_size, float* output_buffer);
// void openunmix_separate(const float* input_buffer, size_t buffer_size, float* output_buffer);

#endif // AUDIO_PROCESSING_BINDINGS_H
