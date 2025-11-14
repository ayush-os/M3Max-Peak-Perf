#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <stdexcept>

namespace m3max {

/**
 * @brief Allocates memory aligned to the specified boundary.
 *
 * Uses std::aligned_alloc (C11/C++17) to ensure the returned pointer
 * meets the required alignment, preventing split cache-line accesses.
 *
 * @param alignment The required alignment (e.g., 64 for cache line alignment).
 * @param size The total size of the memory block to allocate in bytes.
 * @return void* A pointer to the aligned memory block.
 * @throws std::bad_alloc if allocation fails (std::aligned_alloc returns NULL).
 */
inline void *aligned_alloc(std::size_t alignment, std::size_t size) {
  // Check if alignment is a power of 2 and is at least sizeof(void*)
  // Although std::aligned_alloc handles some checks, being explicit is good.
  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    // Alignment must be a power of two
    throw std::invalid_argument("Alignment must be a power of two.");
  }
  if (size == 0) {
    return nullptr; // Standard behavior for zero-sized allocation
  }

  void *ptr = std::aligned_alloc(alignment, size);

  if (ptr == nullptr) {
    throw std::bad_alloc();
  }

  return ptr;
}

/**
 * @brief Frees memory previously allocated by aligned_alloc.
 *
 * Uses the C standard free() function, as required by std::aligned_alloc.
 *
 * @param ptr The pointer to the memory block to free.
 */
inline void aligned_free(void *ptr) noexcept { std::free(ptr); }

struct AlignedDeleter {
  void operator()(void *ptr) const { m3max::aligned_free(ptr); }
};

template <typename T>
using aligned_unique_ptr = std::unique_ptr<T, AlignedDeleter>;

} // namespace m3max
