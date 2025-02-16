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

#ifndef JUNO_ENABLE_ASSERTS
#  error("test_macros.hpp must be included after any JUNO files since it undefs NDEBUG")
#endif

#undef NDEBUG
#include <cassert>
#include <cstdio>   // printf

// Overview:
// 1. Use TEST_CASE(name) to define a test case containing one or more 'ASSERT'
// 2. Use TEST_SUITE(name) to define a test suite containing one or more TEST(host_test)
// 3. Use RUN_TESTS(suite) to run a test suite in the main function.

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

// Don't enforce anonymous namespace, since we don't use a TEST close macro
// NOLINTBEGIN(misc-use-anonymous-namespace)
#define TEST_CASE(name) static void name()

#define TEST_SUITE(name) static void name()
// NOLINTEND(misc-use-anonymous-namespace)

#define TEST(name) name()

#define RUN_SUITE(suite)                                                                 \
  printf("Running test suite '%s'\n", #suite);                                           \
  suite();                                                                               \
  printf("Test suite '%s' passed\n", #suite);
