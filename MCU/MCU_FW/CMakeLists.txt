cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mcu_fw)

idf_component_register(SRCS 
        "src/main.c"
        "lib/shared_state/shared_state.c"
        "lib/apps_driver/apps_driver.c"
        "lib/brake_driver/brake_driver.c"
        "lib/r2d_manager/r2d_manager.c"
        "lib/torque_ctrl/torque_ctrl.c"
        "lib/launch_ctrl/launch_ctrl.c"
        "lib/can_car_driver/can_car_driver.c"
        "lib/can_inv_driver/can_inv_driver.c"
        "lib/encoder_driver/encoder_driver.c"
        "lib/ipc_manager/ipc_manager.c"
    INCLUDE_DIRS 
        "include" 
        "lib/shared_state"
        "lib/apps_driver"
        "lib/brake_driver"
        "lib/r2d_manager"
        "lib/torque_ctrl"
        "lib/launch_ctrl"
        "lib/can_car_driver"
        "lib/can_inv_driver"
        "lib/encoder_driver"
        "lib/ipc_manager")
