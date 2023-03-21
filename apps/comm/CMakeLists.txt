cmake_minimum_required(VERSION 3.7.2)

include(settings.cmake)

project(comm C ASM)

find_package(camkes-tool REQUIRED)
camkes_tool_setup_camkes_build_environment()

# Use components in `global-components`
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
    INCLUDES include
    SOURCES ${Server_deps}
)

file(
    GLOB
        Decrypt_deps
        components/Decrypt/src/*.c
)
DeclareCAmkESComponent(
    Decrypt
    INCLUDES include
    SOURCES ${Decrypt_deps}
)

file(
    GLOB
        UART_deps
        components/UART/src/*.c
)
DeclareCAmkESComponent(
    UART
    INCLUDES include
    SOURCES ${UART_deps}
)

DeclareCAmkESRootserver(comm.camkes)

GenerateCAmkESRootserver()
