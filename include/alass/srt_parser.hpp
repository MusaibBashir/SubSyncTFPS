#pragma once

#include <string>
#include <vector>

namespace alass {

    // A clean, modern struct to hold our subtitle data
    struct Subtitle {
        int id;
        double start_sec;
        double end_sec;
        std::string text;
    };

    // 1. Reads the file and converts timecodes to seconds
    std::vector<Subtitle> parse_srt(const std::string& filepath);

    // 2. Converts the start/end times into a flat array of 1.0s and 0.0s for the FFT
    std::vector<float> binarize_subtitles(
        const std::vector<Subtitle>& subs, 
        size_t n_bins, 
        double bin_size_sec
    );

    // 3. Applies the calculated shift and writes a brand new .srt file
    void write_shifted_srt(
        const std::string& filepath, 
        const std::vector<Subtitle>& subs, 
        double shift_sec
    );

} // namespace alass