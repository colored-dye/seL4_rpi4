
include(${SEL4_TUTORIALS_DIR}/settings.cmake)
# sel4_tutorials_regenerate_tutorial(${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.7.2)
# declare the hello-world CMake project and the languages it is written in (just C)
project(empty C)

# In future tutorials, these setup steps will be replaced with
# sel4_tutorials_setup_roottask_tutorial_environment()
find_package(seL4 REQUIRED)
find_package(elfloader-tool REQUIRED)
find_package(musllibc REQUIRED)
find_package(sel4runtime REQUIRED)
find_package(util_libs REQUIRED)
# find_package(seL4_libs REQUIRED)
# find_package(sel4-tutorials REQUIRED)

sel4_configure_platform_settings()

message("${IMAGE_START_ADDR}")
sel4_import_kernel()
elfloader_import_project()

# This sets up environment build flags and imports musllibc and runtime libraries.
musllibc_setup_build_environment_with_sel4runtime()
sel4_import_libsel4()
util_libs_import_libraries()
# sel4_libs_import_libraries()
# sel4_tutorials_import_libsel4tutorials()
# sel4runtime_import_project()

# Name the executable and list source files required to build it
add_executable(empty src/empty.c)

# List of libraries to link with the application.
target_link_libraries(empty
    sel4_autoconf
    sel4runtime
    sel4
    muslc
    # utils
    # sel4tutorials
    # sel4muslcsys
    # sel4platsupport
    # sel4utils
    # sel4debug
)

# Tell the build system that this application is the root task. 
include(rootserver)
DeclareRootserver(empty)

# utility CMake functions for the tutorials (not required in normal, non-tutorial applications)
# set(FINISH_COMPLETION_TEXT "
# FINISH_COMPLETION_TEXT: Hello, World!
# ")
# set(START_COMPLETION_TEXT "
# START_COMPLETION_TEXT: Hello, World!
# ")
# configure_file(${SEL4_TUTORIALS_DIR}/tools/expect.py ${CMAKE_BINARY_DIR}/check @ONLY)
# include(simulation)
# GenerateSimulateScript()

