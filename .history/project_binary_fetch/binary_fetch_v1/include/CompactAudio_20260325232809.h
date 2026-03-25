#pragma once
#include <string>
using namespace std;

class CompactAudio {
public:
    CompactAudio() = default;

    // Audio Output (Speaker / Headphones/)
    string active_audio_output();        // Returns device name
    string active_audio_output_status(); // Returns "(Active)" or "(Inactive)"

    // Audio Input (Microphone)
    string active_audio_input();         // Returns device name
    string active_audio_input_status();  // Returns "(Active)" or "(Inactive)"
};
