#pragma once
#include <string>

class CompactAudio {
public:
    CompactAudio() = default;

    // Audio Output (Speaker / Headphones)
    std::string active_audio_output();        // Returns device name
    std::string active_audio_output_status(); // Returns "(Active)" or "(Inactive)"

    // Audio Input (Microphone)
    std::string active_audio_input();         // Returns device name
    std::string active_audio_input_status();  // Returns "(Active)" or "(Inactive)"
};
