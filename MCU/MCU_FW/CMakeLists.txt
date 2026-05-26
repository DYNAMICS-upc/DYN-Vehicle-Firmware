cmake_minimum_required(VERSION 3.16)

# MCU_FW – PlatformIO + ESP-IDF CMakeLists
# This file tells ESP-IDF's build system where to find all component sources.

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/apps_driver"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/brake_driver"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/can_car_driver"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/control_algorithms"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/ipc_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/shared_state"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(MCU_FW)
