#include "alass/fft_math.hpp"
#include "pocketfft_hdronly.h"
#include <complex>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace alass {

    double compute_shift(const std::vector<float>& audio_bins, 
                         const std::vector<float>& sub_bins, 
                         double bin_size_sec) {
                             
        size_t n_audio = audio_bins.size();
        size_t n_sub = sub_bins.size();

        if (n_audio == 0 || n_sub == 0) {
            throw std::invalid_argument("Input arrays for FFT cannot be empty.");
        }

        // 1. Zero-pad to the next power of 2 (>= n_audio + n_sub - 1)
        // This makes the FFT exponentially faster and prevents circular cross-correlation overlap
        size_t target_size = n_audio + n_sub - 1;
        size_t padded_size = 1;
        while (padded_size < target_size) {
            padded_size <<= 1; 
        }

        using cplx = std::complex<double>;
        std::vector<cplx> audio_c(padded_size, {0.0, 0.0});
        std::vector<cplx> sub_c(padded_size, {0.0, 0.0});

        for (size_t i = 0; i < n_audio; ++i) audio_c[i] = {audio_bins[i], 0.0};
        for (size_t i = 0; i < n_sub; ++i)   sub_c[i]   = {sub_bins[i], 0.0};

        // pocketfft setup
        pocketfft::shape_t shape{padded_size};
        pocketfft::stride_t stride{sizeof(cplx)};
        pocketfft::shape_t axes{0};

        // 2. Forward FFT on both arrays
        pocketfft::c2c(shape, stride, stride, axes, pocketfft::FORWARD, audio_c.data(), audio_c.data(), 1.0);
        pocketfft::c2c(shape, stride, stride, axes, pocketfft::FORWARD, sub_c.data(), sub_c.data(), 1.0);

        // 3. Multiply Audio by the Complex Conjugate of Subtitles
        for (size_t i = 0; i < padded_size; ++i) {
            audio_c[i] *= std::conj(sub_c[i]);
        }

        // 4. Inverse FFT to get the cross-correlation array
        pocketfft::c2c(shape, stride, stride, axes, pocketfft::BACKWARD, audio_c.data(), audio_c.data(), 1.0 / padded_size);

        // 5. Find the peak peak (highest correlation)
        double max_val = -1.0;
        size_t peak_idx = 0;

        for (size_t i = 0; i < padded_size; ++i) {
            if (audio_c[i].real() > max_val) {
                max_val = audio_c[i].real();
                peak_idx = i;
            }
        }

        // 6. Convert the array index to a physical time shift
        // If the peak is in the second half of the array, it represents a negative shift
        int shift_bins = static_cast<int>(peak_idx);
        if (shift_bins > static_cast<int>(padded_size / 2)) {
            shift_bins -= static_cast<int>(padded_size);
        }

        return shift_bins * bin_size_sec;
    }

} // namespace alass