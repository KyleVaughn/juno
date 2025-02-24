#pragma once

// We want a test framework with assertions that:
// - Are active in both Debug and Release builds.
// - Work for both host and device code.
// - Work the same, regardless of JUNO_ENABLE_ASSERTS.
//
// Therefore, this framework utilizes <cassert> to implement assertions.
// But, in order for "assert" to function correctly, NDEBUG cannot be defined.
// Hence, we mandate that all JUNO headers are included after this file so that
// we can safely undef NDEBUG.
//
// Overview:
// 1. Use TEST_CASE(name) to define a test case containing one or more 'ASSERT'
// 2. Use MAKE_GPU_KERNEL(name) to create a GPU kernel from a test case,
//    provided that the test case was declared with HOSTDEV.
// 3. Use TEST_SUITE(name) to define a test suite containing one or more
//    TEST(host_test) or TEST_GPU_KERNEL(host_test).
//      - It is assumed MAKE_GPU_KERNEL(host_test) was called before
//          TEST_GPU_KERNEL(host_test).
// 4. Use RUN_TESTS(suite) to run a test suite in the main function.
//
// Additional notes:
// - TEST_HOSTDEV(name) is a shortcut for "TEST(name); TEST_GPU_KERNEL(name)".

#ifndef JUNO_USE_GPU
#  error("test_macros.hpp must be included after any JUNO files since it undefs NDEBUG")
#endif

#undef NDEBUG
#include <Kokkos_Core.hpp> // abort, printf
#include <cassert>

#undef ASSERT
#undef ASSERT_NEAR

#define ASSERT(cond) assert(cond)

#define STATIC_ASSERT(cond) static_assert(cond)

#define ASSERT_NEAR(a, b, eps)                                                           \
  {                                                                                      \
    auto const a_eval = (a);                                                             \
    auto const b_eval = (b);                                                             \
    auto const eps_eval = (eps);                                                         \
    ASSERT(a_eval < b_eval ? b_eval - a_eval <= eps_eval : a_eval - b_eval <= eps_eval); \
  }

#define STATIC_ASSERT_NEAR(a, b, eps)                                                    \
  {                                                                                      \
    auto constexpr a_eval = (a);                                                         \
    auto constexpr b_eval = (b);                                                         \
    auto constexpr eps_eval = (eps);                                                     \
    static_assert(a_eval < b_eval ? b_eval - a_eval <= eps_eval                          \
                                  : a_eval - b_eval <= eps_eval);                        \
  }

// We don't care about putting the entire unit test file in an anonymous namespace
// NOLINTBEGIN(misc-use-anonymous-namespace)
#define TEST_CASE(name) static void name()

#define TEST_SUITE(name) static void name()
// NOLINTEND(misc-use-anonymous-namespace)

#define TEST(name) name()

#if JUNO_USE_GPU

// NOLINTBEGIN(bugprone-macro-parentheses) OK, using the name of the test
#  define MAKE_GPU_KERNEL_1_ARGS(host_test)                                              \
    __global__ void host_test##_gpu_kernel() { host_test(); }

#  define MAKE_GPU_KERNEL_2_ARGS(host_test, T)                                           \
    __global__ void host_test##_gpu_kernel() { host_test<T>(); }

#  define MAKE_GPU_KERNEL_3_ARGS(host_test, T, U)                                        \
    __global__ void host_test##_gpu_kernel() { host_test<T, U>(); }

#  define MAKE_GPU_KERNEL_4_ARGS(host_test, T, U, V)                                     \
    __global__ void host_test##_gpu_kernel() { host_test<T, U, V>(); }

#  define MAKE_GPU_KERNEL_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#  define MAKE_GPU_KERNEL(...)                                                           \
    MAKE_GPU_KERNEL_GET_MACRO(__VA_ARGS__, MAKE_GPU_KERNEL_4_ARGS,                       \
                              MAKE_GPU_KERNEL_3_ARGS, MAKE_GPU_KERNEL_2_ARGS,            \
                              MAKE_GPU_KERNEL_1_ARGS)                                    \
    (__VA_ARGS__)

#  define GPU_KERNEL_POST_TEST                                                           \
    gpuError_t const sync_error = gpuDeviceSynchronize();                                \
    if (sync_error != gpuSuccess) {                                                      \
      Kokkos::printf("GPU error: %s\n", gpuGetErrorString(sync_error));                  \
      Kokkos::abort("GPU error");                                                        \
    }                                                                                    \
    gpuError_t const error = gpuGetLastError();                                          \
    if (error != gpuSuccess) {                                                           \
      Kokkos::printf("GPU error: %s\n", gpuGetErrorString(error));                       \
      Kokkos::abort("GPU error");                                                        \
    }

#  define TEST_GPU_KERNEL_1_ARGS(host_test)                                              \
    {                                                                                    \
      host_test##_gpu_kernel<<<1, 1>>>();                                                \
      GPU_KERNEL_POST_TEST                                                               \
    }

#  define TEST_GPU_KERNEL_2_ARGS(host_test, T)                                           \
    {                                                                                    \
      host_test##_gpu_kernel<T><<<1, 1>>>();                                             \
      GPU_KERNEL_POST_TEST                                                               \
    }

#  define TEST_GPU_KERNEL_3_ARGS(host_test, T, U)                                        \
    {                                                                                    \
      host_test##_gpu_kernel<T, U><<<1, 1>>>();                                          \
      GPU_KERNEL_POST_TEST                                                               \
    }

#  define TEST_GPU_KERNEL_4_ARGS(host_test, T, U, V)                                     \
    {                                                                                    \
      host_test##_gpu_kernel<T, U, V><<<1, 1>>>();                                       \
      GPU_KERNEL_POST_TEST                                                               \
    }

#  define TEST_GPU_KERNEL_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#  define TEST_GPU_KERNEL(...)                                                           \
    TEST_GPU_KERNEL_GET_MACRO(__VA_ARGS__, TEST_GPU_KERNEL_4_ARGS,                       \
                              TEST_GPU_KERNEL_3_ARGS, TEST_GPU_KERNEL_2_ARGS,            \
                              TEST_GPU_KERNEL_1_ARGS)                                    \
    (__VA_ARGS__)

#else // JUNO_USE_GPU
#  define MAKE_GPU_KERNEL(...)
#  define TEST_GPU_KERNEL(...)
#endif // JUNO_USE_GPU

#define RUN_SUITE(suite)                                                                 \
  Kokkos::printf("Running test suite '%s'\n", #suite);                                   \
  suite();                                                                               \
  Kokkos::printf("Test suite '%s' passed\n", #suite);

#define TEST_HOSTDEV_1_ARGS(host_test)                                                   \
  TEST(host_test);                                                                       \
  TEST_GPU_KERNEL(host_test);

#define TEST_HOSTDEV_2_ARGS(host_test, T)                                                \
  TEST((host_test<T>));                                                                  \
  TEST_GPU_KERNEL(host_test, T);

#define TEST_HOSTDEV_3_ARGS(host_test, T, U)                                             \
  TEST((host_test<T, U>));                                                               \
  TEST_GPU_KERNEL(host_test, T, U);

#define TEST_HOSTDEV_4_ARGS(host_test, T, U, V)                                          \
  TEST((host_test<T, U, V>));                                                            \
  TEST_GPU_KERNEL(host_test, T, U, V);
// NOLINTEND(bugprone-macro-parentheses)

#define TEST_HOSTDEV_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define TEST_HOSTDEV(...)                                                                \
  TEST_HOSTDEV_GET_MACRO(__VA_ARGS__, TEST_HOSTDEV_4_ARGS, TEST_HOSTDEV_3_ARGS,          \
                         TEST_HOSTDEV_2_ARGS, TEST_HOSTDEV_1_ARGS)                       \
  (__VA_ARGS__)
