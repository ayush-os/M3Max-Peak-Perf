#pragma once

#include "aligned_memory.h"
#include <stdexcept>

namespace m3max {

template <typename T> class Tensor {
private:
  size_t rows_;
  size_t cols_;
  aligned_unique_ptr<T> data_;

public:
  Tensor() : rows_(0), cols_(0), data_(nullptr) {}

  Tensor(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
    size_t size_bytes = rows * cols * sizeof(T);

    void *raw_ptr = m3max::aligned_alloc(64, size_bytes);
    data_ = aligned_unique_ptr<T>(static_cast<T *>(raw_ptr));
  }

  // Default destructor handles cleanup via aligned_unique_ptr

  // --- Accessors ---

  // Access operators for use inside tight loops (fast, no bounds check)
  // Assuming Row-Major layout for the MatMul baseline (C[i*N + j])
  inline T &operator()(size_t r, size_t c) noexcept {
    return data_.get()[r * cols_ + c];
  }

  inline const T &operator()(size_t r, size_t c) const noexcept {
    return data_.get()[r * cols_ + c];
  }

  // Safe access with bounds checking
  T &at(size_t r, size_t c) {
    if (r >= rows_ || c >= cols_) {
      throw std::out_of_range("Tensor index out of bounds.");
    }
    return (*this)(r, c);
  }

  // --- Getters ---

  size_t rows() const noexcept { return rows_; }
  size_t cols() const noexcept { return cols_; }

  // Function to get the raw pointer for kernel interfaces (Essential!)
  T *data() noexcept { return data_.get(); }
  const T *data() const noexcept { return data_.get(); }
};

} // namespace m3max