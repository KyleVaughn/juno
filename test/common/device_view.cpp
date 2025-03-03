#include <juno/config.hpp>
#include <concepts>
#include <cstdlib>
#include <iostream>


#include <Kokkos_Core.hpp>

using HostSpace = Kokkos::HostSpace;
using HostExecSpace = HostSpace::execution_space;
using HostRangePolicy = Kokkos::RangePolicy<HostExecSpace>;
using DeviceSpace = Kokkos::HIPSpace;
using DeviceExecSpace = DeviceSpace::execution_space;
using DeviceRangePolicy = Kokkos::RangePolicy<DeviceExecSpace>;

// A crude test of Kokkos device views

template <typename T, typename MemSpace>
struct Grid
{
  using Vector = Kokkos::View<T*, Kokkos::LayoutLeft, MemSpace>;
  Vector _x;

  explicit Grid(size_t n) noexcept
    : _x("x", n)
  {};

  template <typename OtherMemSpace>
  auto
  operator=(Grid<T, OtherMemSpace> const & g) noexcept -> Grid<T, MemSpace> &
  {
    if constexpr (std::same_as<MemSpace, OtherMemSpace>) {
      _x = g._x;
    } else {
      Kokkos::deep_copy(_x, g._x);
    }
    return *this;
  }
};

int main(void) 
{
  size_t constexpr n = 1 << 24; 
  int constexpr n32 = static_cast<int>(n);
  int constexpr nrepeat = 1000;

  Kokkos::initialize();

  {
    Grid<float, HostSpace> h_a(n);
    for (int i = 0; i < n32; ++i) {
      h_a._x(i) = 1;
    }
    
    Grid<float, DeviceSpace> d_a(n);
    d_a = h_a;

    // Timer products.
    {
      Kokkos::Timer timer;

      for (int repeat = 0; repeat < nrepeat; ++repeat) {
        float result = 0;
        Kokkos::parallel_reduce("SinReduce", HostRangePolicy(0, n32), 
            KOKKOS_LAMBDA(int const i, float & update ) {
              update += Kokkos::sin(h_a._x(i)); 
            }, result
        );
        // Output result.
        if (repeat == (nrepeat - 1)) {
          std::cout << "result = " << result << '\n';
        }
      }

      // Calculate time.
      double const time = timer.seconds();
      double const Gbytes = 1.0e-9 * static_cast<double>(n);

      std::cout << "Host bandwidth: " << Gbytes * nrepeat / time << '\n';
    }
    {
      Kokkos::Timer timer;
                                                                       
      for (int repeat = 0; repeat < nrepeat; ++repeat) {
        float result = 0;
        Kokkos::parallel_reduce("SinReduce", DeviceRangePolicy(0, n32), 
            KOKKOS_LAMBDA(int const i, float & update ) {
              update += Kokkos::sin(d_a._x(i)); 
            }, result
        );
        // Output result.
        if (repeat == (nrepeat - 1)) {
          std::cout << "result = " << result << '\n';
        }
      }
                                                                       
      // Calculate time.
      double const time = timer.seconds();
      double const Gbytes = 1.0e-9 * static_cast<double>(n);
                                                                       
      std::cout << "Device bandwidth: " << Gbytes * nrepeat / time << '\n';
    }

  }
  Kokkos::finalize();
  return 0;
} // main
