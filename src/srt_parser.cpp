#include "alass/srt_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <iomanip>

namespace alass {

    // --- Helper function to strip whitespace and \r ---
    static void trim_string(std::string& s) {
        s.erase(s.find_last_not_of(" \n\r\t") + 1);
        s.erase(0, s.find_first_not_of(" \n\r\t"));
    }

    std::vector<Subtitle> parse_srt(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open input SRT file: " + filepath);
        }

        std::vector<Subtitle> subs;
        std::string line;
        Subtitle current_sub;
        int state = 0; // 0 = expecting ID, 1 = expecting timecode, 2 = expecting text

        while (std::getline(file, line)) {
            trim_string(line);

            // Empty line means the current subtitle block is finished
            if (line.empty()) {
                if (state == 2 && !current_sub.text.empty()) {
                    subs.push_back(current_sub);
                    current_sub.text.clear();
                }
                state = 0;
                continue;
            }

            if (state == 0) {
                // Parse Block ID
                try {
                    current_sub.id = std::stoi(line);
                    state = 1;
                } catch (...) {
                    // Ignore junk data between blocks
                }
            } else if (state == 1) {
                // Parse Timecode: 00:01:23,450 --> 00:01:28,900
                int h1, m1, s1, ms1, h2, m2, s2, ms2;
                if (std::sscanf(line.c_str(), "%d:%d:%d,%d --> %d:%d:%d,%d",
                                &h1, &m1, &s1, &ms1, &h2, &m2, &s2, &ms2) == 8) {
                    
                    current_sub.start_sec = h1 * 3600.0 + m1 * 60.0 + s1 + ms1 / 1000.0;
                    current_sub.end_sec   = h2 * 3600.0 + m2 * 60.0 + s2 + ms2 / 1000.0;
                    state = 2;
                }
            } else if (state == 2) {
                // Parse Text (append if multi-line)
                if (!current_sub.text.empty()) {
                    current_sub.text += "\n";
                }
                current_sub.text += line;
            }
        }

        // Catch the last subtitle if the file doesn't end with a blank line
        if (state == 2 && !current_sub.text.empty()) {
            subs.push_back(current_sub);
        }

        return subs;
    }

    std::vector<float> binarize_subtitles(const std::vector<Subtitle>& subs, size_t n_bins, double bin_size_sec) {
        // Initialize an array of zeroes
        std::vector<float> bins(n_bins, 0.0f);

        for (const auto& sub : subs) {
            size_t start_bin = static_cast<size_t>(sub.start_sec / bin_size_sec);
            size_t end_bin   = static_cast<size_t>(sub.end_sec / bin_size_sec);

            // Clamp to array bounds to prevent segfaults
            if (start_bin >= n_bins) continue; 
            if (end_bin >= n_bins) end_bin = n_bins - 1;

            // Fill the active subtitle window with 1.0f
            for (size_t i = start_bin; i <= end_bin; ++i) {
                bins[i] = 1.0f;
            }
        }

        return bins;
    }

    // --- Helper function to format seconds back to HH:MM:SS,mmm ---
    static std::string format_time(double total_seconds) {
        int h = static_cast<int>(total_seconds / 3600.0);
        total_seconds -= h * 3600.0;
        int m = static_cast<int>(total_seconds / 60.0);
        total_seconds -= m * 60.0;
        int s = static_cast<int>(total_seconds);
        int ms = static_cast<int>((total_seconds - s) * 1000.0);

        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%03d", h, m, s, ms);
        return std::string(buffer);
    }

    void write_shifted_srt(const std::string& filepath, const std::vector<Subtitle>& subs, double shift_sec) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open output SRT file for writing: " + filepath);
        }

        int new_id = 1;

        for (const auto& sub : subs) {
            double new_start = sub.start_sec + shift_sec;
            double new_end   = sub.end_sec + shift_sec;

            // If a subtitle is shifted so far back it falls before the movie starts, drop it
            if (new_end <= 0.0) continue; 
            
            // If the start is before zero, clamp it to 00:00:00,000
            if (new_start < 0.0) new_start = 0.0;

            file << new_id++ << "\n";
            file << format_time(new_start) << " --> " << format_time(new_end) << "\n";
            file << sub.text << "\n\n";
        }
    }

} // namespace alass