#!/usr/bin/bash

if [ -e "$PWD/scripts/build-kernel.sh" ]; then
    TOP_DIR=$PWD
elif [ -e "$PWD/build-kernel.sh" ] || [ -d "../kernel" ]; then
    TOP_DIR=`dirname $PWD`
else
    echo "Please run this script in top directory or \`kernel\' directory"
    exit 1
fi

APP="$1"

if [ "$APP" == "" ]; then
    echo "Please enter app name!"
    echo "Usage: ./build-app.sh <app>"
    echo "List of available apps: comm;comm-smaccm;empty;hello;hello-camkes-1;hello-camkes-2"
    exit 1
fi

qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -kernel build/apps/$APP/images/capdl-loader-image-arm-qemu-arm-virt \
    -nographic \
    -m 2G
