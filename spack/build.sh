FLAVOR=cpu_only # Build for cpu_only, cuda, or hip
SCRATCH=/tmp/spack # Don't build in your home dir. Specify system scratch

set -e # Exit immediately if a command exits with non-zero status

if [ ! -d "${SCRATCH}" ]; then
   echo "Scratch directory '${SCRATCH}' does not exist"
   exit 1
fi

echo "Updating spack and spack-packages..."
git submodule update --init --recursive

export SPACK_ROOT=$PWD/spack
export SPACK_DISABLE_LOCAL_CONFIG=true
export SPACK_USER_CACHE_PATH=${SCRATCH}/spack
source $SPACK_ROOT/share/spack/setup-env.sh

echo "Activating spack env..."
spack env activate -d $FLAVOR -p
echo "Setting spack-packages location..."
spack repo set --destination ${PWD}/spack-packages builtin
echo "Finding available compilers..."
spack compiler find
echo "Cleaning..."
spack clean --all
echo "Concretizing..."
spack concretize -fU
echo "Installing..."
spack install --fail-fast

#echo "Generating modules..."
#spack module tcl refresh --delete-tree -y
#cd spack/share/spack/modules
#ln -sf linux-$OS-x86_64 ./env
#ln -sf linux-$OS-x86_64_v4 ./env_v4
#cd ../../../../
#
#echo "Setting env permissions (this may take a while)..."
#cd ../..
#chmod -fR 750 ./dev_tools/
#chgrp -fR sapdev ./dev_tools/
