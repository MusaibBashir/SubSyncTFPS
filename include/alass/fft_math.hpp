#pragma once

#include <vector>

namespace alass {

    /**
     * Computes the optimal time shift to align subtitle bins with audio bins
     * using Fast Fourier Transform (FFT) cross-correlation.
     * * @param audio_bins Binarized audio energy (1.0 = voice, 0.0 = silence)
     * @param sub_bins Binarized subtitle presence (1.0 = text, 0.0 = no text)
     * @param bin_size_sec The duration of each bin in seconds (e.g., 0.10s)
     * @return The calculated shift in seconds. Positive means subtitles need to be delayed.
     */
    double compute_shift(const std::vector<float>& audio_bins, 
                         const std::vector<float>& sub_bins, 
                         double bin_size_sec);

} // namespace alass