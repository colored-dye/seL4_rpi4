include(${SEL4_TUTORIALS_DIR}/settings.cmake)
# sel4_tutorials_regenerate_tutorial(${CMAKE_CURRENT_SOURCE_DIR})

find_package(sel4-tutorials REQUIRED)

list(
    APPEND
        CMAKE_MODULE_PATH
        ${project_dir}/tools/camkes
)

cmake_minimum_required(VERSION 3.7.2)

project(hello-camkes-1 C ASM)

find_package(camkes-tool REQUIRED)
camkes_tool_setup_camkes_build_environment()

DeclareCAmkESComponent(Client SOURCES components/Client/src/client.c)
DeclareCAmkESComponent(Echo SOURCES components/Echo/src/echo.c)

DeclareCAmkESComponent(EmptyComponent)

DeclareCAmkESRootserver(hello-1.camkes)

GenerateCAmkESRootserver()

# utility CMake functions for the tutorials (not required in normal, non-tutorial applications)
# set(FINISH_COMPLETION_TEXT "Component echo saying: hello world")
# set(START_COMPLETION_TEXT "Booting all finished, dropped to user space")
# configure_file(${SEL4_TUTORIALS_DIR}/tools/expect.py ${CMAKE_BINARY_DIR}/check @ONLY)
# include(simulation)
# GenerateSimulateScript()
