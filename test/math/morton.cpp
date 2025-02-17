#include <juno/math/morton.hpp>

#include <juno/config.hpp>

#include <cstdint>

// NOLINTNEXTLINE(misc-include-cleaner)
#include <concepts>

#include "../test_macros.hpp"

template <std::unsigned_integral U>
HOSTDEV
TEST_CASE(mortonEncode)
{
  U constexpr x0 = 0;
  U constexpr x1 = 1;
  U constexpr x2 = 2;
  U constexpr x3 = 3;

  // 2D
  ASSERT(juno::mortonEncode(x0, x0) == 0);
  ASSERT(juno::mortonEncode(x1, x0) == 1);
  ASSERT(juno::mortonEncode(x0, x1) == 2);
  ASSERT(juno::mortonEncode(x1, x1) == 3);
  ASSERT(juno::mortonEncode(x2, x2) == 12);
  ASSERT(juno::mortonEncode(x3, x3) == 15);

  // 3D
  ASSERT(juno::mortonEncode(x0, x0, x0) == 0);
  ASSERT(juno::mortonEncode(x1, x0, x0) == 1);
  ASSERT(juno::mortonEncode(x0, x1, x0) == 2);
  ASSERT(juno::mortonEncode(x1, x1, x0) == 3);
  ASSERT(juno::mortonEncode(x0, x0, x1) == 4);
  ASSERT(juno::mortonEncode(x1, x0, x1) == 5);
  ASSERT(juno::mortonEncode(x0, x1, x1) == 6);
  ASSERT(juno::mortonEncode(x1, x1, x1) == 7);
  ASSERT(juno::mortonEncode(x2, x0, x0) == 8);
  ASSERT(juno::mortonEncode(x2, x2, x2) == 56);
  ASSERT(juno::mortonEncode(x3, x3, x3) == 63);
}

template <std::unsigned_integral U>
HOSTDEV
TEST_CASE(mortonDecode)
{

  // 2D
  U x;
  U y;
  juno::mortonDecode(static_cast<U>(0), x, y);
  ASSERT(x == 0 && y == 0);
  juno::mortonDecode(static_cast<U>(1), x, y);
  ASSERT(x == 1 && y == 0);
  juno::mortonDecode(static_cast<U>(2), x, y);
  ASSERT(x == 0 && y == 1);
  juno::mortonDecode(static_cast<U>(3), x, y);
  ASSERT(x == 1 && y == 1);
  juno::mortonDecode(static_cast<U>(12), x, y);
  ASSERT(x == 2 && y == 2);
  juno::mortonDecode(static_cast<U>(15), x, y);
  ASSERT(x == 3 && y == 3);

  // 3D
  U z;
  juno::mortonDecode(static_cast<U>(0), x, y, z);
  ASSERT(x == 0 && y == 0 && z == 0);
  juno::mortonDecode(static_cast<U>(1), x, y, z);
  ASSERT(x == 1 && y == 0 && z == 0);
  juno::mortonDecode(static_cast<U>(2), x, y, z);
  ASSERT(x == 0 && y == 1 && z == 0);
  juno::mortonDecode(static_cast<U>(3), x, y, z);
  ASSERT(x == 1 && y == 1 && z == 0);
  juno::mortonDecode(static_cast<U>(4), x, y, z);
  ASSERT(x == 0 && y == 0 && z == 1);
  juno::mortonDecode(static_cast<U>(5), x, y, z);
  ASSERT(x == 1 && y == 0 && z == 1);
  juno::mortonDecode(static_cast<U>(6), x, y, z);
  ASSERT(x == 0 && y == 1 && z == 1);
  juno::mortonDecode(static_cast<U>(7), x, y, z);
  ASSERT(x == 1 && y == 1 && z == 1);
  juno::mortonDecode(static_cast<U>(56), x, y, z);
  ASSERT(x == 2 && y == 2 && z == 2);
}

template <std::unsigned_integral U, std::floating_point T>
HOSTDEV
TEST_CASE(mortonEncodeFloat)
{
  T const zero = static_cast<T>(0);
  T const one = static_cast<T>(1);
  ASSERT((juno::mortonEncode<U, T>(zero, zero) == static_cast<U>(0x0000000000000000)));
  ASSERT((juno::mortonEncode<U, T>(one, zero) == static_cast<U>(0x5555555555555555)));
  ASSERT((juno::mortonEncode<U, T>(zero, one) == static_cast<U>(0xAAAAAAAAAAAAAAAA)));
  ASSERT((juno::mortonEncode<U, T>(one, one) == static_cast<U>(0xFFFFFFFFFFFFFFFF)));

  // 3D
  U value = 100;
  ASSERT(
      (juno::mortonEncode<U, T>(zero, zero, zero) == static_cast<U>(0x0000000000000000)));
  value = juno::mortonEncode<U, T>(one, one, one);
  if constexpr (std::same_as<uint32_t, U>) {
    ASSERT(value == static_cast<U>(0x3fffffff));
  } else {
    ASSERT(value == static_cast<U>(0x7fffffffffffffff));
  }
  value = juno::mortonEncode<U, T>(one, zero, zero);
  if constexpr (std::same_as<uint32_t, U>) {
    ASSERT(value == static_cast<U>(0x9249249));
  } else {
    ASSERT(value == static_cast<U>(0x1249249249249249));
  }
}

template <std::unsigned_integral U, std::floating_point T>
HOSTDEV
TEST_CASE(mortonDecodeFloat)
{
  T constexpr eps = static_cast<T>(1e-6);

  T x;
  T y;
  juno::mortonDecode<U, T>(static_cast<U>(0x0000000000000000), x, y);
  ASSERT_NEAR(x, static_cast<T>(0), eps);
  ASSERT_NEAR(y, static_cast<T>(0), eps);
  juno::mortonDecode<U, T>(static_cast<U>(0xFFFFFFFFFFFFFFFF), x, y);
  ASSERT_NEAR(x, static_cast<T>(1), eps);
  ASSERT_NEAR(y, static_cast<T>(1), eps);
  juno::mortonDecode<U, T>(static_cast<U>(0x5555555555555555), x, y);
  ASSERT_NEAR(x, static_cast<T>(1), eps);
  ASSERT_NEAR(y, static_cast<T>(0), eps);
  juno::mortonDecode<U, T>(static_cast<U>(0xAAAAAAAAAAAAAAAA), x, y);
  ASSERT_NEAR(x, static_cast<T>(0), eps);
  ASSERT_NEAR(y, static_cast<T>(1), eps);

  // 3D
  T z;
  juno::mortonDecode<U, T>(static_cast<U>(0x0000000000000000), x, y, z);
  ASSERT_NEAR(x, static_cast<T>(0), eps);
  ASSERT_NEAR(y, static_cast<T>(0), eps);
  ASSERT_NEAR(z, static_cast<T>(0), eps);
  if constexpr (std::same_as<uint32_t, U>) {
    juno::mortonDecode<U, T>(static_cast<U>(0x3fffffff), x, y, z);
  } else {
    juno::mortonDecode<U, T>(static_cast<U>(0x7fffffffffffffff), x, y, z);
  }
  ASSERT_NEAR(x, static_cast<T>(1), eps);
  ASSERT_NEAR(y, static_cast<T>(1), eps);
  ASSERT_NEAR(z, static_cast<T>(1), eps);
}

#if JUNO_USE_GPU
template <std::unsigned_integral U>
MAKE_GPU_KERNEL(mortonEncode, U);

template <std::unsigned_integral U>
MAKE_GPU_KERNEL(mortonDecode, U);

template <std::unsigned_integral U, std::floating_point T>
MAKE_GPU_KERNEL(mortonEncodeFloat, U, T);

template <std::unsigned_integral U, std::floating_point T>
MAKE_GPU_KERNEL(mortonDecodeFloat, U, T);
#endif

template <std::unsigned_integral U>
TEST_SUITE(morton)
{
  TEST_HOSTDEV(mortonEncode, U);
  TEST_HOSTDEV(mortonDecode, U);
}

template <std::unsigned_integral U, std::floating_point T>
TEST_SUITE(mortonFloat)
{
  TEST_HOSTDEV(mortonEncodeFloat, U, T);
  TEST_HOSTDEV(mortonDecodeFloat, U, T);
}

auto
main() -> int
{
  RUN_SUITE(morton<uint32_t>);
  RUN_SUITE(morton<uint64_t>);
  RUN_SUITE((mortonFloat<uint32_t, float>));
  RUN_SUITE((mortonFloat<uint32_t, double>));
  RUN_SUITE((mortonFloat<uint64_t, double>));
  return 0;
}
