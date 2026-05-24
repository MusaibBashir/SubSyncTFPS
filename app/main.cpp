#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

// Command line parser
#include "CLI11.hpp" 

// alass-cpp Engine Headers
#include "alass/audio_tap.hpp"
#include "alass/srt_parser.hpp"
#include "alass/alass_math.hpp"

int main(int argc, char** argv) {
    CLI::App app{"alass-cpp: Blazing Fast Subtitle Synchronization"};

    std::string video_path;
    std::string srt_in_path;
    std::string srt_out_path;

    app.add_option("video", video_path, "Path to the media file")->required()->check(CLI::ExistingFile);
    app.add_option("input_srt", srt_in_path, "Path to the out-of-sync .srt file")->required()->check(CLI::ExistingFile);
    app.add_option("output_srt", srt_out_path, "Path to save the corrected .srt file")->required();

    CLI11_PARSE(app, argc, argv);

    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        std::cout << "[1/4] Extracting audio energy from video...\n";
        auto audio_bins = alass::extract_audio_bins(video_path);

        std::cout << "[2/4] Parsing subtitles...\n";
        auto subtitles = alass::parse_srt(srt_in_path);
        
        std::cout << "[3/4] Running exact ALASS Sweep-Line algorithm...\n";
        // Convert binary audio into continuous ALASS intervals
        auto audio_intervals = alass::bins_to_intervals(audio_bins, alass::DEFAULT_BIN_SIZE, alass::DEFAULT_SEEK_SEC);
        
        // Calculate shift using exact continuous math
        double shift_sec = alass::compute_shift_exact(subtitles, audio_intervals);
        
        std::cout << "      -> Calculated shift: " << std::showpos << shift_sec << " seconds\n" << std::noshowpos;

        std::cout << "[4/4] Writing corrected subtitles...\n";
        alass::write_shifted_srt(srt_out_path, subtitles, shift_sec);

    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "\nSuccess! Synced flawlessly in " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds.\n";
    return 0;
}