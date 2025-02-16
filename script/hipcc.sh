#!/bin/bash

if ! command -v hipcc &> /dev/null
then
    echo "hipcc could not be found"
    return
fi

export OMP_PROC_BIND=spread
COMPILER=hipcc
BUILD_DIR=build/hipcc
cmake -S . -B $BUILD_DIR \
      -DCMAKE_CXX_COMPILER=$COMPILER \
      -DJUNO_USE_HIP=ON \
      -DKokkos_ENABLE_SERIAL=ON \
      -DKokkos_ENABLE_OPENMP=ON \
      -DKokkos_ENABLE_HIP=ON \
      -DKokkos_ENABLE_TESTS=OFF \
      -DKokkos_ENABLE_ROCTHRUST=OFF \
      -DKokkos_ENABLE_DEBUG=ON \
      -DKokkos_ENABLE_DEBUG_BOUNDS_CHECK=ON \
      -DKokkos_ENABLE_HIP_MULTIPLE_KERNEL_INSTANTIATIONS=ON \
      -DKokkos_ARCH_NATIVE=ON \
&& cmake --build $BUILD_DIR -j \
&& ctest --test-dir $BUILD_DIR
