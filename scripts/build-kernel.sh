#!/usr/bin/bash

if [ -e "$PWD/scripts/build-kernel.sh" ]; then
    TOP_DIR=$PWD
elif [ -e "$PWD/build-kernel.sh" ] || [ -d "../kernel" ]; then
    TOP_DIR=`dirname $PWD`
else
    echo "Please run this script in top directory or \`kernel\' directory"
    exit 1
fi


BUILD_DIR="$TOP_DIR/build"
KERNEL_DIR="$TOP_DIR/kernel"
BUILD_KERNEL_DIR="$BUILD_DIR/kernel"

echo "TOP_DIR: $TOP_DIR"
echo "Kernel build directory: $BUILD_KERNEL_DIR"

rm -rf $BUILD_KERNEL_DIR
mkdir -p $BUILD_KERNEL_DIR
cd $BUILD_KERNEL_DIR

cmake $KERNEL_DIR \
    -GNinja \
    -DKernelArch=arm \
    -DKernelArchARM=1 \
    -DKernelSel4ArchAarch64=1 \
    -DKernelWordSize=64 \
    -DKernelArmCortexA72=1 \
    -DKernelArchArmV8a=1 \
    -Darch=aarch64 \
    -DKernelSel4Arch=aarch64 \
    -DCMAKE_TOOLCHAIN_FILE="$KERNEL_DIR/gcc.cmake" \
    -DKernelPlatform=ls1046a \
    -DCROSS_COMPILER_PREFIX=aarch64-linux-gnu- \
    # -DKernelSel4ArchArmHyp=1 # Temporarily disable hypervisor support

ninja
