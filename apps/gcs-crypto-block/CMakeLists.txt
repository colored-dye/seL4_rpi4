cmake_minimum_required(VERSION 3.7.2)

set(project_dir "${CMAKE_CURRENT_LIST_DIR}/../..")
list(
    APPEND
        CMAKE_MODULE_PATH
        ${project_dir}/GEC-block-linux
)

set(CMAKE_BUILD_TYPE "Debug")

project(gcs-crypto C)

find_package(GEC REQUIRED)
GEC_import_library()

message(STATUS "GEC path: ${GEC_DIR}")

add_executable(gcs-crypto src/main.c)

target_include_directories(
    gcs-crypto PUBLIC ${GEC_DIR}/include ${project_dir}/mavlink/include
)

target_link_libraries(
    gcs-crypto GEC pthread
)
