#pragma once

#include <string>
#include <vector>

namespace alass {

    // Default parameters matching your original autosync.c logic
    constexpr int DEFAULT_SAMPLE_RATE = 8000;
    constexpr double DEFAULT_BIN_SIZE = 0.10;
    constexpr int DEFAULT_SEEK_SEC = 300;     // Skip first 5 mins (often credits/intros)
    constexpr int DEFAULT_WINDOW_SEC = 90;    // Analyze 90 seconds of dialogue
    constexpr float ENERGY_THRESH = 1.5f;

    /**
     * Spawns an FFmpeg process, reads raw audio, computes RMS energy, 
     * and returns a binarized array of voice activity (1.0f or 0.0f).
     */
    std::vector<float> extract_audio_bins(
        const std::string& video_path,
        double bin_size_sec = DEFAULT_BIN_SIZE,
        int sample_rate = DEFAULT_SAMPLE_RATE,
        int seek_sec = DEFAULT_SEEK_SEC,
        int window_sec = DEFAULT_WINDOW_SEC
    );

} // namespace alass