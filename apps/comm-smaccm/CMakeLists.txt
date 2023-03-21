cmake_minimum_required(VERSION 3.7.2)

include(settings.cmake)

# find_package(sel4-tutorials REQUIRED)

project(comm C ASM)

find_package(camkes-tool REQUIRED)
camkes_tool_setup_camkes_build_environment()

find_file(GLOBAL_COMPONENTS_PATH global-components.cmake PATHS ${project_dir}/projects/global-components/ CMAKE_FIND_ROOT_PATH_BOTH)
mark_as_advanced(FORCE GLOBAL_COMPONENTS_PATH)
if("${GLOBAL_COMPONENTS_PATH}" STREQUAL "GLOBAL_COMPONENTS_PATH-NOTFOUND")
    message(FATAL_ERROR "Failed to find global-components.cmake. Consider cmake -DGLOBAL_COMPONENTS_PATH=/path/to/global-components.cmake")
endif()
include(${GLOBAL_COMPONENTS_PATH})

CAmkESAddImportPath(${CMAKE_CURRENT_LIST_DIR}/interfaces)

file(
    GLOB
    Server_deps
        components/Server/src/*.c
)
DeclareCAmkESComponent(
    Server
    INCLUDES
        include components/Server/include
    SOURCES
        ${Server_deps}
)

DeclareCAmkESComponent(
    dispatch_periodic
    INCLUDES include
    SOURCES components/dispatch_periodic/src/dispatch_periodic.c
)

DeclareCAmkESComponent(
    Encrypt
    INCLUDES include
    SOURCES components/Encrypt/src/Encrypt.c
)

file(
    GLOB
    Decrypt_deps
        components/Decrypt/src/*.c
)

DeclareCAmkESComponent(
    Decrypt
    INCLUDES include components/Decrypt/include
    SOURCES ${Decrypt_deps}
)

DeclareCAmkESComponent(
    Encrypt_server2self_Monitor
    INCLUDES include
    SOURCES components/Monitors/Encrypt_server2self_Monitor/src/Encrypt_server2self_Monitor.c
)

DeclareCAmkESComponent(
    Server_decrypt2self_Monitor
    INCLUDES include
    SOURCES components/Monitors/Server_decrypt2self_Monitor/src/Server_decrypt2self_Monitor.c
)

DeclareCAmkESRootserver(comm.camkes)

GenerateCAmkESRootserver()
