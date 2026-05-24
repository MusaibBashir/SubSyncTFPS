#include "alass/audio_tap.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cstdio>

// Cross-platform popen macros
#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

namespace alass {

    std::vector<float> extract_audio_bins(const std::string& video_path, 
                                          double bin_size_sec, 
                                          int sample_rate, 
                                          int seek_sec, 
                                          int window_sec) {
        
        // 1. Construct the FFmpeg command
        // -nostdin: prevents FFmpeg from waiting for user terminal input
        // -y: overwrite without asking (just in case)
        // -v quiet: suppresses all output except the raw audio pipe
        char cmd[2048];
        std::snprintf(cmd, sizeof(cmd),
            "ffmpeg -nostdin -y -ss %d -t %d -i \"%s\" "
            "-ac 1 -ar %d -f f32le -v quiet pipe:1",
            seek_sec, window_sec, video_path.c_str(), sample_rate);

        // 2. Open the process pipe in BINARY read mode
        FILE* pipe = POPEN(cmd, "rb");
        if (!pipe) {
            throw std::runtime_error("Failed to start FFmpeg. Is it installed and added to your PATH?");
        }

        // 3. Read the raw PCM float data
        size_t max_samples = static_cast<size_t>(sample_rate * window_sec);
        std::vector<float> pcm(max_samples, 0.0f);
        
        size_t samples_read = std::fread(pcm.data(), sizeof(float), max_samples, pipe);
        int exit_code = PCLOSE(pipe);

        if (samples_read < static_cast<size_t>(sample_rate * 5)) {
            throw std::runtime_error("FFmpeg returned insufficient audio data. The file might be too short or broken.");
        }

        // Resize vector to actual data read (in case the video was shorter than the window)
        pcm.resize(samples_read);

        // 4. Compute RMS energy in bins (chunks)
        int chunk_samples = static_cast<int>(sample_rate * bin_size_sec);
        size_t n_bins = samples_read / chunk_samples;
        
        std::vector<float> energy(n_bins, 0.0f);

        for (size_t b = 0; b < n_bins; ++b) {
            double sum_sq = 0.0;
            size_t offset = b * chunk_samples;
            
            for (int k = 0; k < chunk_samples; ++k) {
                float sample = pcm[offset + k];
                sum_sq += static_cast<double>(sample) * sample;
            }
            energy[b] = static_cast<float>(std::sqrt(sum_sq / chunk_samples));
        }

        // 5. Binarize the energy (Voice Activity Detection)
        // We make a copy of the energy array to sort it and find the median
        std::vector<float> sorted_energy = energy;
        std::sort(sorted_energy.begin(), sorted_energy.end());
        
        float median_energy = sorted_energy[n_bins / 2];
        float threshold = median_energy * ENERGY_THRESH;

        std::vector<float> audio_bins(n_bins, 0.0f);
        for (size_t i = 0; i < n_bins; ++i) {
            if (energy[i] > threshold) {
                audio_bins[i] = 1.0f;
            }
        }

        return audio_bins;
    }

} // namespace alass