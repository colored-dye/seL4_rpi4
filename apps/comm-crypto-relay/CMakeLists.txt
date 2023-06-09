#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.7.2)
include(settings.cmake)
project(comm C)

if(${VM_LINUX})
    find_package(camkes-arm-vm REQUIRED)
    camkes_arm_vm_setup_arm_vm_environment()
    add_subdirectory("vm-linux")
else()
    find_package(camkes-tool REQUIRED)
    find_package(global-components REQUIRED)
    camkes_tool_setup_camkes_build_environment()
    global_components_import_project()
endif()

# GEC crypto library
find_package(GEC REQUIRED)
GEC_import_library()

message(STATUS "GEC_DIR: ${GEC_DIR}")
set(mavlink_include_path "${project_dir}/mavlink/include" CACHE STRING "" FORCE)
set(gec_include_path "${GEC_DIR}/include" CACHE STRING "" FORCE)

DeclareCAmkESComponent(
    Pixhawk_RX
    INCLUDES include ${gec_include_path}
    SOURCES components/Pixhawk_RX/src/Pixhawk_RX.c
)

DeclareCAmkESComponent(
    Telemetry_RX
    INCLUDES include ${gec_include_path}
    SOURCES components/Telemetry_RX/src/Telemetry_RX.c
)

DeclareCAmkESComponent(
    Decrypt
    INCLUDES include ${mavlink_include_path} ${gec_include_path}
    SOURCES components/Decrypt/src/Decrypt.c
    LIBS GEC
)

DeclareCAmkESComponent(
    Encrypt
    INCLUDES include ${mavlink_include_path} ${gec_include_path}
    SOURCES components/Encrypt/src/Encrypt.c
    LIBS GEC
)

if(${VM_LINUX})
    AddCamkesCPPFlag(cpp_flags CONFIG_VARS VmEmmc2NoDMA VmVUSB VM_LINUX)

    DeclareCAmkESRootserver(comm.camkes
        CPP_FLAGS
        ${cpp_flags}
        CPP_INCLUDES
        ${CAMKES_VM_DIR}/components/VM_Arm
    )
else()
    DeclareCAmkESRootserver(comm.camkes)
endif()

GenerateCAmkESRootserver()
