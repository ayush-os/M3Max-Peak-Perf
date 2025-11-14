#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>

#include "utils/tensor.h"
#include "utils/timer.h"

void baseline_matmul(const m3max::Tensor<float> &A,
                     const m3max::Tensor<float> &B, m3max::Tensor<float> &C);

using namespace m3max;

// tolerance
constexpr float EPSILON = 1e-5f;

// --- UTILITY FUNCTIONS ---

/**
 * @brief Fills a tensor with deterministic, non-zero values for testing.
 */
void fill_tensor_deterministic(Tensor<float> &T) {
  for (size_t r = 0; r < T.rows(); ++r) {
    for (size_t c = 0; c < T.cols(); ++c) {
      // Simple pattern: (r + c + 1) / (r + c + 1.0f)
      T(r, c) = (float)(r * 0.1f + c * 0.01f + 1.0f);
    }
  }
}

/**
 * @brief Fills a tensor with random values for performance testing.
 */
void fill_tensor_random(Tensor<float> &T) {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<> distrib(-1.0f, 1.0f);

  for (size_t r = 0; r < T.rows(); ++r) {
    for (size_t c = 0; c < T.cols(); ++c) {
      T(r, c) = distrib(gen);
    }
  }
}

// --- CORRECTNESS TEST HARNESS ---

/**
 * @brief Checks the correctness of a kernel against a known reference tensor.
 * @return bool True if matrices are close within EPSILON, False otherwise.
 */
bool check_correctness(const Tensor<float> &TestC, const Tensor<float> &RefC) {
  if (TestC.rows() != RefC.rows() || TestC.cols() != RefC.cols()) {
    std::cerr << "Error: Result dimensions mismatch!" << std::endl;
    return false;
  }

  for (size_t r = 0; r < RefC.rows(); ++r) {
    for (size_t c = 0; c < RefC.cols(); ++c) {
      float diff = std::fabs(TestC(r, c) - RefC(r, c));
      if (diff > EPSILON) {
        std::cerr << "FAIL: Mismatch at C(" << r << ", " << c << ")."
                  << " Expected: " << RefC(r, c) << ", Got: " << TestC(r, c)
                  << ", Diff: " << diff << std::endl;
        return false;
      }
    }
  }
  return true;
}

/**
 * @brief Runs a correctness check using a small, deterministic MatMul size.
 */
void run_correctness_test() {
  std::cout << "--- 🧪 Running Correctness Test (512x512x512) ---" << std::endl;
  const int M = 512, K = 512, N = 512;

  // 1. Setup Tensors
  Tensor<float> A(M, K);
  Tensor<float> B(K, N);
  Tensor<float> RefC(M, N); // Reference result (calculated by the baseline)
  Tensor<float> TestC(M,
                      N); // Test result (calculated by the kernel being tested)

  // 2. Initialize
  fill_tensor_deterministic(A);
  fill_tensor_deterministic(B);

  // 3. Compute Reference Result (The Baseline is also the Gold Standard)
  baseline_matmul(A, B, RefC);

  // 4. Test the Baseline Kernel (It should pass!)
  baseline_matmul(A, B, TestC);

  std::cout << "Testing Baseline Kernel: ";
  if (check_correctness(TestC, RefC)) {
    std::cout << "\033[32mPASS\033[0m" << std::endl; // Green PASS
  } else {
    std::cout << "\033[31mFAIL\033[0m" << std::endl; // Red FAIL
  }
}

// --- PERFORMANCE TEST HARNESS ---

/**
 * @brief Runs a performance test for a given kernel function.
 */
void run_performance_test(const std::string &kernel_name,
                          void (*kernel_func)(const Tensor<float> &,
                                              const Tensor<float> &,
                                              Tensor<float> &),
                          int M, int K, int N, int num_runs) {
  // 1. Setup Tensors
  Tensor<float> A(M, K);
  Tensor<float> B(K, N);
  Tensor<float> C(M, N);

  // 2. Initialize with random data once
  fill_tensor_random(A);
  fill_tensor_random(B);

  // 3. Warm-up Run (Crucial for consistent measurements on high-perf cores)
  // primes caches and ensure max clock speed.
  kernel_func(A, B, C);

  // 4. Timed Runs
  Timer t;
  uint64_t total_nanos = 0;

  for (int i = 0; i < num_runs; ++i) {
    t.start();
    kernel_func(A, B, C);
    total_nanos += t.stop_and_get_nanos();
  }

  // 5. Reporting and GFLOP/s Calculation
  uint64_t avg_nanos = total_nanos / num_runs;
  double avg_seconds = (double)avg_nanos / 1.0e9;

  // MatMul FLOPs: 2 * M * K * N (M*K*N mults and M*K*N adds)
  uint64_t total_flops = 2ULL * M * K * N;

  // GFLOP/s = (FLOPs / Time) / 10^9
  double gflops = (double)total_flops / avg_seconds / 1.0e9;

  std::cout << std::left << std::setw(20) << kernel_name << ": " << std::right
            << std::setw(10) << std::fixed << std::setprecision(2) << gflops
            << " GFLOP/s"
            << " (" << std::setprecision(3) << avg_seconds * 1000.0 << " ms)"
            << std::endl;
}

void run_all_performance_tests() {
  std::cout << "\n--- ⚡ Running Performance Tests (M=N=K=1024) ---"
            << std::endl;

  const int M = 1024, K = 1024, N = 1024;
  const int NUM_RUNS = 20;

  run_performance_test("Baseline", baseline_matmul, M, K, N, NUM_RUNS);
}

int main() {
  std::cout << "M3 Max Peak Performance Attainment Project" << std::endl;
  std::cout << "------------------------------------------" << std::endl;

  run_correctness_test();
  run_all_performance_tests();

  return 0;
}