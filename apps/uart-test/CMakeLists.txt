cmake_minimum_required(VERSION 3.7.2)

include(settings.cmake)

project(uart-test C ASM)

find_package(camkes-tool REQUIRED)
camkes_tool_setup_camkes_build_environment()

DeclareCAmkESComponent(UART_Driver SOURCES components/UART/src/uart.c)
DeclareCAmkESRootserver(uart-test.camkes)

GenerateCAmkESRootserver()
