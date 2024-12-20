#pragma once
#ifndef AUDIO_PROCESSING_BINDINGS_H
#define AUDIO_PROCESSING_BINDINGS_H

#include <string>

void spleeter_separate(const std::string& input_path, const std::string& output_path);
void openunmix_separate(const std::string& input_path, const std::string& output_path);

#endif // AUDIO_PROCESSING_BINDINGS_H