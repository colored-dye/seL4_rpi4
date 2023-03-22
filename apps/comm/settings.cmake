include_guard(GLOBAL)

set(project_dir "${CMAKE_CURRENT_LIST_DIR}/../..")
file(GLOB project_modules ${project_dir}/projects/*)
list(
    APPEND
        CMAKE_MODULE_PATH
        ${project_dir}/kernel
        ${project_dir}/tools/seL4/cmake-tool/helpers
        ${project_dir}/tools/seL4/elfloader-tool
        ${project_dir}/tools/camkes
        ${project_modules}
)
# set(POLLY_DIR ${project_dir}/tools/polly CACHE INTERNAL "")

include(application_settings)

# Deal with the top level target-triplet variables.
if(NOT TUT_BOARD)
    message(
        FATAL_ERROR
            "Please select a board to compile for. Choose either pc or zynq7000\n\t`-DTUT_BOARD=<PREFERENCE>`"
    )
endif()

# Set arch and board specific kernel parameters here.
if(${TUT_BOARD} STREQUAL "zynq7000")
    # Do a quick check and warn the user if they haven't set
    # -DARM/-DAARCH32/-DAARCH64.
    if(
        (NOT ARM)
        AND (NOT AARCH32)
        AND ((NOT CROSS_COMPILER_PREFIX) OR ("${CROSS_COMPILER_PREFIX}" STREQUAL ""))
    )
        message(
            WARNING
                "The target machine is an ARM machine. Unless you've defined -DCROSS_COMPILER_PREFIX, you may need to set one of:\n\t-DARM/-DAARCH32/-DAARCH64"
        )
    endif()

    set(KernelArch "arm" CACHE STRING "" FORCE)
    set(KernelSel4Arch "aarch32" CACHE STRING "" FORCE)
    set(KernelPlatform "zynq7000" CACHE STRING "" FORCE)
    ApplyData61ElfLoaderSettings(${KernelPlatform} ${KernelSel4Arch})
elseif(${TUT_BOARD} STREQUAL "rpi4")
    set(KernelArch "arm" CACHE STRING "" FORCE)
    set(KernelSel4Arch "aarch64" CACHE STRING "" FORCE)
    set(KernelPlatform "bcm2711" CACHE STRING "" FORCE)
    ApplyData61ElfLoaderSettings(${KernelPlatform} ${KernelSel4Arch})
elseif(${TUT_BOARD} STREQUAL "qemu-arm-virt")
    set(KernelArch "arm" CACHE STRING "" FORCE)
    set(KernelSel4Arch "aarch64" CACHE STRING "" FORCE)
    set(KernelPlatform "qemu-arm-virt" CACHE STRING "" FORCE)
    ApplyData61ElfLoaderSettings(${KernelPlatform} ${KernelSel4Arch})
elseif(${TUT_BOARD} STREQUAL "ls1046a")
    set(KernelArch "arm" CACHE STRING "" FORCE)
    set(KernelSel4Arch "aarch64" CACHE STRING "" FORCE)
    set(KernelPlatform "ls1046a" CACHE STRING "" FORCE)
    set(KernelPlatformOK1046AC2 ON CACHE BOOL "" FORCE)
    ApplyData61ElfLoaderSettings(${KernelPlatform} ${KernelSel4Arch})
else()
    message(FATAL_ERROR "Unsupported board ${TUT_BOARD}.")
endif()

# include(${project_dir}/kernel/configs/seL4Config.cmake)
set(CapDLLoaderMaxObjects 20000 CACHE STRING "" FORCE)
set(KernelRootCNodeSizeBits 16 CACHE STRING "")

# For the tutorials that do initialize the plat support serial printing they still
# just want to use the kernel debug putchar if it exists
set(LibSel4PlatSupportUseDebugPutChar FALSE CACHE BOOL "" FORCE)

# Just let the regular abort spin without calling DebugHalt to prevent needless
# confusing output from the kernel for a tutorial
set(LibSel4MuslcSysDebugHalt FALSE CACHE BOOL "" FORCE)

# Only configure a single domain for the domain scheduler
set(KernelNumDomains 1 CACHE STRING "" FORCE)

# Export generic timer to user
# TODO: Implement another timer and remove these two later
# set(KernelArmExportPCNTUser TRUE CACHE BOOL "" FORCE)
# set(KernelArmExportPTMRUser TRUE CACHE BOOL "" FORCE)

# Serial output relies on Debug settings
ApplyCommonReleaseVerificationSettings(FALSE FALSE)
