#include "alass/alass_math.hpp"
#include <algorithm>

namespace alass {

    std::vector<Interval> bins_to_intervals(const std::vector<float>& bins, double bin_size_sec, double seek_sec) {
        std::vector<Interval> intervals;
        bool in_speech = false;
        double start = 0.0;
        
        for (size_t i = 0; i < bins.size(); ++i) {
            if (bins[i] > 0.5f && !in_speech) {
                // Map the array index back to an absolute timestamp in the movie
                start = (i * bin_size_sec) + seek_sec;
                in_speech = true;
            } else if (bins[i] <= 0.5f && in_speech) {
                double end = (i * bin_size_sec) + seek_sec;
                intervals.push_back({start, end});
                in_speech = false;
            }
        }
        if (in_speech) {
            intervals.push_back({start, (bins.size() * bin_size_sec) + seek_sec});
        }
        return intervals;
    }

    struct Event {
        double x;
        int slope_change;
        
        // Sort events chronologically by shift value
        bool operator<(const Event& other) const {
            return x < other.x;
        }
    };

    double compute_shift_exact(const std::vector<Subtitle>& subs, const std::vector<Interval>& audio_intervals) {
        std::vector<Event> events;
        
        // Pre-allocate memory to prevent reallocation lag (4 events per pair)
        events.reserve(4 * subs.size() * audio_intervals.size());
        
        for (const auto& sub : subs) {
            double A = sub.start_sec;
            double B = sub.end_sec;
            
            for (const auto& aud : audio_intervals) {
                double C = aud.start_sec;
                double D = aud.end_sec;
                
                // The 4 mathematical points where the overlap slope changes
                events.push_back({C - B,  1});
                events.push_back({C - A, -1});
                events.push_back({D - B, -1});
                events.push_back({D - A,  1});
            }
        }
        
        // Sort the millions of events (C++ does this in milliseconds)
        std::sort(events.begin(), events.end());
        
        double current_y = 0.0;
        double current_slope = 0.0;
        double last_x = events.empty() ? 0.0 : events[0].x;
        
        double max_y = -1.0;
        double best_shift = 0.0;
        
        // Sweep across the events to perfectly reconstruct the overlap graph
        for (const auto& ev : events) {
            double dx = ev.x - last_x;
            current_y += current_slope * dx;
            current_slope += ev.slope_change;
            last_x = ev.x;
            
            if (current_y > max_y) {
                max_y = current_y;
                best_shift = ev.x;
            }
        }
        
        return best_shift;
    }

} // namespace alass