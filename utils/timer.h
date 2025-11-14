#pragma once

#include <cstdint>
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>

namespace m3max {

// Global, static storage for the timebase information.
static mach_timebase_info_data_t timebase_info;
static dispatch_once_t once_token = 0;

/**
 * @brief Initializes the mach timebase information exactly once.
 * This conversion factor (numer/denom) is needed to convert raw ticks
 * into wall-clock nanoseconds.
 */
static inline void initialize_timebase() {
  dispatch_once(&once_token, ^{
    kern_return_t kr = mach_timebase_info(&timebase_info);
    if (kr != KERN_SUCCESS) {
      // In a real-world application, this would throw an exception or log a
      // fatal error. For macOS, this should always succeed. We do not throw
      // here to keep the function noexcept-like for dispatch_once.
    }
  });
}

/**
 * @brief Returns the current raw, monotonic tick count.
 */
inline uint64_t get_absolute_time() { return mach_absolute_time(); }

/**
 * @brief Converts a duration in raw ticks to nanoseconds.
 * @param duration_ticks The difference between two mach_absolute_time() calls.
 * @return uint64_t The duration in nanoseconds.
 */
inline uint64_t ticks_to_nanos(uint64_t duration_ticks) {
  initialize_timebase(); // Safely called every time, initializes only once.

  // Conversion formula: (ticks * numerator) / denominator
  // Use 128-bit arithmetic if possible to avoid intermediate overflow,
  // but the following should be safe for typical M3 Max durations.
  return (duration_ticks * timebase_info.numer) / timebase_info.denom;
}

/**
 * @brief A simple class to start and stop timing.
 */
class Timer {
private:
  uint64_t start_time_ = 0;

public:
  /**
   * @brief Records the current absolute time tick count.
   */
  void start() { start_time_ = get_absolute_time(); }

  /**
   * @brief Stops the timer and returns the elapsed time in nanoseconds.
   * @return uint64_t The elapsed time in nanoseconds.
   */
  uint64_t stop_and_get_nanos() {
    uint64_t end_time = get_absolute_time();
    if (end_time < start_time_) {
      // Handle timer wrap-around (highly unlikely but good practice)
      return 0;
    }
    return ticks_to_nanos(end_time - start_time_);
  }
};

} // namespace m3max