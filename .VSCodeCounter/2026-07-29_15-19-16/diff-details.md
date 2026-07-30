# Diff Details

Date : 2026-07-29 15:19:16

Directory c:\\Users\\DBDVU0X\\DYN-Vehicle-Firmware\\MCU

Total : 39 files,  -1 codes, -2 comments, 29 blanks, all 26 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [MCU/MCU_FW/lib/apps_driver/apps_driver.c](/MCU/MCU_FW/lib/apps_driver/apps_driver.c) | C | 59 | 1 | 9 | 69 |
| [MCU/MCU_FW/lib/apps_driver/apps_driver.h](/MCU/MCU_FW/lib/apps_driver/apps_driver.h) | C++ | 14 | 1 | 6 | 21 |
| [MCU/MCU_FW/lib/brake_driver/brake_driver.c](/MCU/MCU_FW/lib/brake_driver/brake_driver.c) | C | 32 | 1 | 7 | 40 |
| [MCU/MCU_FW/lib/brake_driver/brake_driver.h](/MCU/MCU_FW/lib/brake_driver/brake_driver.h) | C++ | 11 | 0 | 4 | 15 |
| [MCU/MCU_FW/lib/can_car_driver/can_car_driver.c](/MCU/MCU_FW/lib/can_car_driver/can_car_driver.c) | C | 18 | 2 | 6 | 26 |
| [MCU/MCU_FW/lib/can_car_driver/can_car_driver.h](/MCU/MCU_FW/lib/can_car_driver/can_car_driver.h) | C++ | 29 | 0 | 6 | 35 |
| [MCU/MCU_FW/lib/control_algorithms/launch_ctrl.c](/MCU/MCU_FW/lib/control_algorithms/launch_ctrl.c) | C | 20 | 5 | 7 | 32 |
| [MCU/MCU_FW/lib/control_algorithms/launch_ctrl.h](/MCU/MCU_FW/lib/control_algorithms/launch_ctrl.h) | C++ | 11 | 0 | 5 | 16 |
| [MCU/MCU_FW/lib/control_algorithms/r2d_manager.c](/MCU/MCU_FW/lib/control_algorithms/r2d_manager.c) | C | 42 | 3 | 6 | 51 |
| [MCU/MCU_FW/lib/control_algorithms/r2d_manager.h](/MCU/MCU_FW/lib/control_algorithms/r2d_manager.h) | C++ | 19 | 0 | 5 | 24 |
| [MCU/MCU_FW/lib/control_algorithms/torque_ctrl.c](/MCU/MCU_FW/lib/control_algorithms/torque_ctrl.c) | C | 21 | 3 | 10 | 34 |
| [MCU/MCU_FW/lib/control_algorithms/torque_ctrl.h](/MCU/MCU_FW/lib/control_algorithms/torque_ctrl.h) | C++ | 11 | 0 | 4 | 15 |
| [MCU/MCU_FW/lib/ipc_manager/ipc_manager.c](/MCU/MCU_FW/lib/ipc_manager/ipc_manager.c) | C | 15 | 0 | 5 | 20 |
| [MCU/MCU_FW/lib/ipc_manager/ipc_manager.h](/MCU/MCU_FW/lib/ipc_manager/ipc_manager.h) | C++ | 17 | 0 | 5 | 22 |
| [MCU/MCU_FW/lib/shared_state/shared_state.c](/MCU/MCU_FW/lib/shared_state/shared_state.c) | C | 23 | 0 | 5 | 28 |
| [MCU/MCU_FW/lib/shared_state/shared_state.h](/MCU/MCU_FW/lib/shared_state/shared_state.h) | C++ | 19 | 0 | 5 | 24 |
| [MCU/MCU_FW/lib/utils/math_utils.c](/MCU/MCU_FW/lib/utils/math_utils.c) | C | 10 | 0 | 3 | 13 |
| [MCU/MCU_FW/lib/utils/math_utils.h](/MCU/MCU_FW/lib/utils/math_utils.h) | C++ | 10 | 0 | 4 | 14 |
| [MCU/MCU_FW/platformio.ini](/MCU/MCU_FW/platformio.ini) | Ini | 13 | 0 | 2 | 15 |
| [MCU/MCU_FW/src/app.cpp](/MCU/MCU_FW/src/app.cpp) | C++ | 61 | 6 | 18 | 85 |
| [MCU/MCU_FW/src/app.h](/MCU/MCU_FW/src/app.h) | C++ | 9 | 0 | 4 | 13 |
| [MCU/MCU_FW/src/main.cpp](/MCU/MCU_FW/src/main.cpp) | C++ | 5 | 0 | 2 | 7 |
| [MCU/MCU_FW/test/test_main.cpp](/MCU/MCU_FW/test/test_main.cpp) | C++ | 23 | 0 | 5 | 28 |
| [PDM/PDM_FW/include/pdm_config.h](/PDM/PDM_FW/include/pdm_config.h) | C++ | -11 | -2 | -3 | -16 |
| [PDM/PDM_FW/lib/can_service/can_service.c](/PDM/PDM_FW/lib/can_service/can_service.c) | C | -53 | -6 | -7 | -66 |
| [PDM/PDM_FW/lib/can_service/can_service.h](/PDM/PDM_FW/lib/can_service/can_service.h) | C++ | -10 | 0 | -4 | -14 |
| [PDM/PDM_FW/lib/ipc_manager/ipc.c](/PDM/PDM_FW/lib/ipc_manager/ipc.c) | C | -40 | 0 | -9 | -49 |
| [PDM/PDM_FW/lib/ipc_manager/ipc.h](/PDM/PDM_FW/lib/ipc_manager/ipc.h) | C++ | -22 | 0 | -8 | -30 |
| [PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.c](/PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.c) | C | -77 | -1 | -8 | -86 |
| [PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.h](/PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.h) | C++ | -16 | -1 | -6 | -23 |
| [PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.c](/PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.c) | C | -59 | -1 | -9 | -69 |
| [PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.h](/PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.h) | C++ | -12 | 0 | -4 | -16 |
| [PDM/PDM_FW/lib/protection/protection.c](/PDM/PDM_FW/lib/protection/protection.c) | C | -39 | -6 | -12 | -57 |
| [PDM/PDM_FW/lib/protection/protection.h](/PDM/PDM_FW/lib/protection/protection.h) | C++ | -13 | -1 | -5 | -19 |
| [PDM/PDM_FW/platformio.ini](/PDM/PDM_FW/platformio.ini) | Ini | -13 | 0 | -2 | -15 |
| [PDM/PDM_FW/src/app.cpp](/PDM/PDM_FW/src/app.cpp) | C++ | -94 | -6 | -16 | -116 |
| [PDM/PDM_FW/src/app.h](/PDM/PDM_FW/src/app.h) | C++ | -9 | 0 | -4 | -13 |
| [PDM/PDM_FW/src/main.cpp](/PDM/PDM_FW/src/main.cpp) | C++ | -5 | 0 | -2 | -7 |
| [PDM/PDM_FW/test/test_main.cpp](/PDM/PDM_FW/test/test_main.cpp) | C++ | -20 | 0 | -5 | -25 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details