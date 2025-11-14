#include "utils/tensor.h"

void baseline_matmul(const m3max::Tensor<float> &A,
                     const m3max::Tensor<float> &B, m3max::Tensor<float> &C) {
  for (size_t i = 0; i < C.rows(); i++) {
    for (size_t j = 0; j < C.cols(); j++) {
      float res = 0.0f;
      for (size_t k = 0; k < A.cols(); k++) {
        res += A(i, k) * B(k, j);
      }
      C(i, j) = res;
    }
  }
}