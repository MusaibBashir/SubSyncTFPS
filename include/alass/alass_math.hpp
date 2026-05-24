#pragma once
#include "alass/srt_parser.hpp" // We need the Subtitle struct
#include <vector>

namespace alass {

    struct Interval {
        double start_sec;
        double end_sec;
    };

    // Converts our raw binary audio bins into continuous time intervals
    std::vector<Interval> bins_to_intervals(const std::vector<float>& bins, 
                                            double bin_size_sec, 
                                            double seek_sec);

    // The pure ALASS Sweep-Line algorithm (O(NM log NM))
    double compute_shift_exact(const std::vector<Subtitle>& subs, 
                               const std::vector<Interval>& audio_intervals);

} // namespace alass