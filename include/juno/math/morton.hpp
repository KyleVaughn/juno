#pragma once

#include <juno/config.hpp>

#include <juno/common/assert.hpp>

#include <concepts>

//==============================================================================
// MORTON ENCODING/DECODING
//==============================================================================
// This file provides functions for mapping to and from Morton codes.
// https://en.wikipedia.org/wiki/Z-order_curve
//
// unsigned ints:
// mortonEncode(u32, u32) -> u32
// mortonEncode(u32, u32, u32) -> u32
// mortonEncode(u64, u64) -> u64
// mortonEncode(u64, u64, u64) -> u64
//
// floating point (normalized to [0,1]):
// mortonEncode(f32, f32) -> u32
// mortonEncode(f32, f32, f32) -> u32
// mortonEncode(f64, f64) -> u64
// mortonEncode(f64, f64, f64) -> u64
//
// Morton codes can be efficiently encoded and decoded using the BMI2
// instruction set for bit manipulation. If this instruction set is not
// supported (such as on GPU), then we emulate the BMI2 intrinsics using a
// portable algorithm.

#if defined(__BMI2__) 
// Only compile fallbacks for device code
#  define BMI2_SUPPORTED 1
#  include <immintrin.h> // _pdep_u64, _pext_u64, _pdep_u32, _pext_u32
#  if COMPILING_DEVICE 
#    define BMI2_HOSTDEV DEVICE
#  endif
#else
// Compile fallbacks for host and device code
#  define BMI2_SUPPORTED 0
#  define BMI2_HOSTDEV   HOSTDEV
#endif

namespace juno
{

//==============================================================================
// Maximum coordinate values
//==============================================================================
// In N dimensions with an X-bit morton code, the max bits that may be used to
// represent a coordinate without loss of precision is X / N.
// Therefore, the max coordinate value is 2^(X / N) - 1.

template <std::unsigned_integral U>
inline constexpr U max_2d_morton_coord = (static_cast<U>(1) << (4 * sizeof(U))) - 1;

template <std::unsigned_integral U>
inline constexpr U max_3d_morton_coord = (static_cast<U>(1) << (8 * sizeof(U) / 3)) - 1;

#if BMI2_SUPPORTED && !COMPILING_DEVICE

//==============================================================================
// BMI2 intrinsics
//==============================================================================
// Wrap pdep and pext for portability.

// These can be used depending on compile-time configuration or the user's
// choice, so ignore warnings about unused functions.
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-function"
static inline auto
pdep(uint32_t source, uint32_t mask) noexcept -> uint32_t
{
  return _pdep_u32(source, mask);
}

static inline auto
pdep(uint64_t source, uint64_t mask) noexcept -> uint64_t
{
  return _pdep_u64(source, mask);
}

static inline auto
pext(uint32_t source, uint32_t mask) noexcept -> uint32_t
{
  return _pext_u32(source, mask);
}

static inline auto
pext(uint64_t source, uint64_t mask) noexcept -> uint64_t
{
  return _pext_u64(source, mask);
}
#  pragma GCC diagnostic pop

// Masks for interleaving bits in 2D and 3D morton codes.
template <std::unsigned_integral U>
inline static constexpr U bmi_2d_x_mask = static_cast<U>(0x5555555555555555);

template <std::unsigned_integral U>
inline static constexpr U bmi_2d_y_mask = static_cast<U>(0xAAAAAAAAAAAAAAAA);

template <std::unsigned_integral U>
inline static constexpr U bmi_3d_x_mask = static_cast<U>(0x9249249249249249);

template <std::unsigned_integral U>
inline static constexpr U bmi_3d_y_mask = static_cast<U>(0x2492492492492492);

template <std::unsigned_integral U>
inline static constexpr U bmi_3d_z_mask = static_cast<U>(0x4924924924924924);

//==============================================================================
// Morton encoding/decoding
//==============================================================================

// Encode 2D coordinate to morton code
template <std::unsigned_integral U>
inline auto
mortonEncode(U const x, U const y) noexcept -> U
{
  ASSERT_ASSUME(x <= max_2d_morton_coord<U>);
  ASSERT_ASSUME(y <= max_2d_morton_coord<U>);
  return pdep(x, bmi_2d_x_mask<U>) | pdep(y, bmi_2d_y_mask<U>);
}

// Encode 3D coordinate to morton code
template <std::unsigned_integral U>
inline auto
mortonEncode(U const x, U const y, U const z) noexcept -> U
{
  ASSERT_ASSUME(x <= max_3d_morton_coord<U>);
  ASSERT_ASSUME(y <= max_3d_morton_coord<U>);
  ASSERT_ASSUME(z <= max_3d_morton_coord<U>);
  return pdep(x, bmi_3d_x_mask<U>) | pdep(y, bmi_3d_y_mask<U>) |
         pdep(z, bmi_3d_z_mask<U>);
}

// Decode the morton code to 2D coordinates
template <std::unsigned_integral U>
inline void
mortonDecode(U const morton, U & x, U & y) noexcept
{
  x = pext(morton, bmi_2d_x_mask<U>);
  y = pext(morton, bmi_2d_y_mask<U>);
}

// Decode the morton code to 3D coordinates
template <std::unsigned_integral U>
inline void
mortonDecode(U const morton, U & x, U & y, U & z) noexcept
{
  x = pext(morton, bmi_3d_x_mask<U>);
  y = pext(morton, bmi_3d_y_mask<U>);
  z = pext(morton, bmi_3d_z_mask<U>);
}

#else // BMI2_SUPPORTED && !COMPILING_DEVICE

//==============================================================================
// BMI2 intrinsics emulation
//==============================================================================

// Emulate pdep_u32(x, 0x55555555) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pdep0x55555555(uint32_t x) noexcept -> uint32_t
{
  x = (x | (x << 8)) & 0x00ff00ff;
  x = (x | (x << 4)) & 0x0f0f0f0f;
  x = (x | (x << 2)) & 0x33333333;
  x = (x | (x << 1)) & 0x55555555;
  return x;
}

// Emulate pdep_u64(x, 0x5555555555555555) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pdep0x5555555555555555(uint64_t x) noexcept -> uint64_t
{
  x = (x | (x << 16)) & 0x0000ffff0000ffff;
  x = (x | (x << 8)) & 0x00ff00ff00ff00ff;
  x = (x | (x << 4)) & 0x0f0f0f0f0f0f0f0f;
  x = (x | (x << 2)) & 0x3333333333333333;
  x = (x | (x << 1)) & 0x5555555555555555;
  return x;
}

// Emulate pdep_u32(x, 0x55555555) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pext0x55555555(uint32_t x) noexcept -> uint32_t
{
  x &= 0x55555555;
  x = (x ^ (x >> 1)) & 0x33333333;
  x = (x ^ (x >> 2)) & 0x0f0f0f0f;
  x = (x ^ (x >> 4)) & 0x00ff00ff;
  x = (x ^ (x >> 8)) & 0x0000ffff;
  return x;
}

// Emulate pext_u64(x, 0x5555555555555555) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pext0x5555555555555555(uint64_t x) noexcept -> uint64_t
{
  x &= 0x5555555555555555;
  x = (x ^ (x >> 1)) & 0x3333333333333333;
  x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
  x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
  x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
  x = (x ^ (x >> 16)) & 0x00000000ffffffff;
  return x;
}

// Emulate pdep_u32(x, 0x92492492) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pdep0x92492492(uint32_t x) noexcept -> uint32_t
{
  x = (x | (x << 16)) & 0x030000ff;
  x = (x | (x << 8)) & 0x0300f00f;
  x = (x | (x << 4)) & 0x030c30c3;
  x = (x | (x << 2)) & 0x09249249;
  return x;
}

// Emulate pdep_u64(x, 0x9249249249249249) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pdep0x9249249249249249(uint64_t x) noexcept -> uint64_t
{
  ASSERT_ASSUME(x <= max_3d_morton_coord<uint64_t>);
  x = (x | (x << 32)) & 0x001f00000000ffff;
  x = (x | (x << 16)) & 0x001f0000ff0000ff;
  x = (x | (x << 8)) & 0x100f00f00f00f00f;
  x = (x | (x << 4)) & 0x10c30c30c30c30c3;
  x = (x | (x << 2)) & 0x1249249249249249;
  return x;
}

// Emulate pext_u32(x, 0x92492492) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pext0x92492492(uint32_t x) noexcept -> uint32_t
{
  x &= 0x09249249;
  x = (x ^ (x >> 2)) & 0x030c30c3;
  x = (x ^ (x >> 4)) & 0x0300f00f;
  x = (x ^ (x >> 8)) & 0x030000ff;
  x = (x ^ (x >> 16)) & 0x000003ff;
  return x;
}

// Emulate pext_u64(x, 0x9249249249249249) using portable bit manipulation
BMI2_HOSTDEV static constexpr auto
pext0x9249249249249249(uint64_t x) noexcept -> uint64_t
{
  x &= 0x1249249249249249;
  x = (x ^ (x >> 2)) & 0x10c30c30c30c30c3;
  x = (x ^ (x >> 4)) & 0x100f00f00f00f00f;
  x = (x ^ (x >> 8)) & 0x001f0000ff0000ff;
  x = (x ^ (x >> 16)) & 0x001f00000000ffff;
  x = (x ^ (x >> 32)) & 0x00000000001fffff;
  return x;
}

//==============================================================================
// Morton encoding/decoding
//==============================================================================

// Encode 2D coordinate to morton code
BMI2_HOSTDEV constexpr auto
mortonEncode(uint32_t const x, uint32_t const y) noexcept -> uint32_t
{
  ASSERT_ASSUME(x <= max_2d_morton_coord<uint32_t>);
  ASSERT_ASSUME(y <= max_2d_morton_coord<uint32_t>);
  return pdep0x55555555(x) | (pdep0x55555555(y) << 1);
}

// Encode 2D coordinate to morton code
BMI2_HOSTDEV constexpr auto
mortonEncode(uint64_t const x, uint64_t const y) noexcept -> uint64_t
{
  ASSERT_ASSUME(x <= max_2d_morton_coord<uint64_t>);
  ASSERT_ASSUME(y <= max_2d_morton_coord<uint64_t>);
  return pdep0x5555555555555555(x) | (pdep0x5555555555555555(y) << 1);
}

// Decode the morton code to 2D coordinates
BMI2_HOSTDEV constexpr void
mortonDecode(uint32_t const morton, uint32_t & x, uint32_t & y) noexcept
{
  x = pext0x55555555(morton);
  y = pext0x55555555(morton >> 1);
}

// Decode the morton code to 2D coordinates
BMI2_HOSTDEV constexpr void
mortonDecode(uint64_t const morton, uint64_t & x, uint64_t & y) noexcept
{
  x = pext0x5555555555555555(morton);
  y = pext0x5555555555555555(morton >> 1);
}

// Encode 3D coordinate to morton code
BMI2_HOSTDEV constexpr auto
mortonEncode(uint32_t const x, uint32_t const y, uint32_t const z) noexcept -> uint32_t
{
  ASSERT_ASSUME(x <= max_3d_morton_coord<uint32_t>);
  ASSERT_ASSUME(y <= max_3d_morton_coord<uint32_t>);
  ASSERT_ASSUME(z <= max_3d_morton_coord<uint32_t>);
  return pdep0x92492492(x) | (pdep0x92492492(y) << 1) | (pdep0x92492492(z) << 2);
}

// Encode 3D coordinate to morton code
BMI2_HOSTDEV constexpr auto
mortonEncode(uint64_t const x, uint64_t const y, uint64_t const z) noexcept -> uint64_t
{
  ASSERT_ASSUME(x <= max_3d_morton_coord<uint64_t>);
  ASSERT_ASSUME(y <= max_3d_morton_coord<uint64_t>);
  ASSERT_ASSUME(z <= max_3d_morton_coord<uint64_t>);
  return pdep0x9249249249249249(x) | (pdep0x9249249249249249(y) << 1) |
         (pdep0x9249249249249249(z) << 2);
}

// Decode the morton code to 3D coordinates
BMI2_HOSTDEV constexpr void
mortonDecode(uint32_t const morton, uint32_t & x, uint32_t & y, uint32_t & z) noexcept
{
  x = pext0x92492492(morton);
  y = pext0x92492492(morton >> 1);
  z = pext0x92492492(morton >> 2);
}

// Decode the morton code to 3D coordinates
BMI2_HOSTDEV constexpr void
mortonDecode(uint64_t const morton, uint64_t & x, uint64_t & y, uint64_t & z) noexcept
{
  x = pext0x9249249249249249(morton);
  y = pext0x9249249249249249(morton >> 1);
  z = pext0x9249249249249249(morton >> 2);
}

#endif // BMI2_SUPPORTED && COMPILING_DEVICE

//==============================================================================
// Morton encoding/decoding floats (normalized to [0,1])
//==============================================================================

// Encode 2D floating point coordinate in [0,1] x [0,1] to U morton code
template <std::unsigned_integral U, std::floating_point T>
HOSTDEV auto
mortonEncode(T const x, T const y) noexcept -> U
{
  ASSERT_ASSUME(0 <= x);
  ASSERT_ASSUME(0 <= y);
  ASSERT_ASSUME(x <= 1);
  ASSERT_ASSUME(y <= 1);
  static_assert(!(std::same_as<float, T> && std::same_as<uint64_t, U>),
                "uint64_t -> float conversion can be lossy");
  // Convert x,y in [0,1] to integers in [0 max_2d_morton_coord]
  U const x_m = static_cast<U>(x * max_2d_morton_coord<U>);
  U const y_m = static_cast<U>(y * max_2d_morton_coord<U>);
  return mortonEncode(x_m, y_m);
}

// Decode U morton code to a 2D floating point coordinate in [0,1] x [0,1]
template <std::unsigned_integral U, std::floating_point T>
HOSTDEV void
mortonDecode(U const morton, T & x, T & y) noexcept
{
  U x_m;
  U y_m;
  mortonDecode(morton, x_m, y_m);
  x = static_cast<T>(x_m) / static_cast<T>(max_2d_morton_coord<U>);
  y = static_cast<T>(y_m) / static_cast<T>(max_2d_morton_coord<U>);
}

// Encode 3D floating point coordinate in [0,1] x [0,1] x [0,1] to U morton code
template <std::unsigned_integral U, std::floating_point T>
HOSTDEV auto
mortonEncode(T const x, T const y, T const z) noexcept -> U
{
  ASSERT_ASSUME(0 <= x);
  ASSERT_ASSUME(0 <= y);
  ASSERT_ASSUME(0 <= z);
  ASSERT_ASSUME(x <= 1);
  ASSERT_ASSUME(y <= 1);
  ASSERT_ASSUME(z <= 1);
  U const x_m = static_cast<U>(x * max_3d_morton_coord<U>);
  U const y_m = static_cast<U>(y * max_3d_morton_coord<U>);
  U const z_m = static_cast<U>(z * max_3d_morton_coord<U>);
  return mortonEncode(x_m, y_m, z_m);
}

// Decode U morton code to a 3D floating point coordinate in [0,1] x [0,1] x [0,1]
template <std::unsigned_integral U, std::floating_point T>
HOSTDEV void
mortonDecode(U const morton, T & x, T & y, T & z) noexcept
{
  U x_m;
  U y_m;
  U z_m;
  mortonDecode(morton, x_m, y_m, z_m);
  x = static_cast<T>(x_m) / static_cast<T>(max_3d_morton_coord<U>);
  y = static_cast<T>(y_m) / static_cast<T>(max_3d_morton_coord<U>);
  z = static_cast<T>(z_m) / static_cast<T>(max_3d_morton_coord<U>);
}

} // namespace juno
