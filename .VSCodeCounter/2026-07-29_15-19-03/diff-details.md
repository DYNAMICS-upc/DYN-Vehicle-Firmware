# Diff Details

Date : 2026-07-29 15:19:03

Directory c:\\Users\\DBDVU0X\\DYN-Vehicle-Firmware\\Steering

Total : 42 files,  346 codes, 29 comments, 84 blanks, all 459 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [ECU/ECU_FW/lib/ads8688_driver/ads8688_driver.c](/ECU/ECU_FW/lib/ads8688_driver/ads8688_driver.c) | C | -21 | -2 | -6 | -29 |
| [ECU/ECU_FW/lib/ads8688_driver/ads8688_driver.h](/ECU/ECU_FW/lib/ads8688_driver/ads8688_driver.h) | C++ | -11 | 0 | -4 | -15 |
| [ECU/ECU_FW/lib/bsp/bsp.c](/ECU/ECU_FW/lib/bsp/bsp.c) | C | -7 | 0 | -2 | -9 |
| [ECU/ECU_FW/lib/bsp/bsp.h](/ECU/ECU_FW/lib/bsp/bsp.h) | C++ | -11 | -3 | -6 | -20 |
| [ECU/ECU_FW/lib/can_service/can_service.c](/ECU/ECU_FW/lib/can_service/can_service.c) | C | -49 | 0 | -9 | -58 |
| [ECU/ECU_FW/lib/can_service/can_service.h](/ECU/ECU_FW/lib/can_service/can_service.h) | C++ | -9 | 0 | -4 | -13 |
| [ECU/ECU_FW/lib/fan_driver/fan_driver.c](/ECU/ECU_FW/lib/fan_driver/fan_driver.c) | C | -42 | -2 | -7 | -51 |
| [ECU/ECU_FW/lib/fan_driver/fan_driver.h](/ECU/ECU_FW/lib/fan_driver/fan_driver.h) | C++ | -13 | -1 | -6 | -20 |
| [ECU/ECU_FW/lib/ipc_manager/ipc.c](/ECU/ECU_FW/lib/ipc_manager/ipc.c) | C | -11 | 0 | -5 | -16 |
| [ECU/ECU_FW/lib/ipc_manager/ipc.h](/ECU/ECU_FW/lib/ipc_manager/ipc.h) | C++ | -17 | 0 | -5 | -22 |
| [ECU/ECU_FW/lib/pid_ctrl/pid_ctrl.c](/ECU/ECU_FW/lib/pid_ctrl/pid_ctrl.c) | C | -21 | -1 | -7 | -29 |
| [ECU/ECU_FW/lib/pid_ctrl/pid_ctrl.h](/ECU/ECU_FW/lib/pid_ctrl/pid_ctrl.h) | C++ | -18 | 0 | -5 | -23 |
| [ECU/ECU_FW/platformio.ini](/ECU/ECU_FW/platformio.ini) | Ini | -13 | 0 | -2 | -15 |
| [ECU/ECU_FW/src/app.cpp](/ECU/ECU_FW/src/app.cpp) | C++ | -55 | -3 | -10 | -68 |
| [ECU/ECU_FW/src/app.h](/ECU/ECU_FW/src/app.h) | C++ | -9 | 0 | -4 | -13 |
| [ECU/ECU_FW/src/main.cpp](/ECU/ECU_FW/src/main.cpp) | C++ | -5 | 0 | -2 | -7 |
| [ECU/ECU_FW/test/test_main.cpp](/ECU/ECU_FW/test/test_main.cpp) | C++ | -14 | 0 | -4 | -18 |
| [Steering/Lucy_Dyn10/include/app_log.h](/Steering/Lucy_Dyn10/include/app_log.h) | C++ | 39 | 9 | 9 | 57 |
| [Steering/Lucy_Dyn10/lib/bsp/bsp.c](/Steering/Lucy_Dyn10/lib/bsp/bsp.c) | C | 4 | 1 | 2 | 7 |
| [Steering/Lucy_Dyn10/lib/bsp/bsp.h](/Steering/Lucy_Dyn10/lib/bsp/bsp.h) | C++ | 13 | 0 | 5 | 18 |
| [Steering/Lucy_Dyn10/lib/button_driver/button_driver.c](/Steering/Lucy_Dyn10/lib/button_driver/button_driver.c) | C | 55 | 2 | 9 | 66 |
| [Steering/Lucy_Dyn10/lib/button_driver/button_driver.h](/Steering/Lucy_Dyn10/lib/button_driver/button_driver.h) | C++ | 13 | 1 | 7 | 21 |
| [Steering/Lucy_Dyn10/lib/can_driver/can_driver.cpp](/Steering/Lucy_Dyn10/lib/can_driver/can_driver.cpp) | C++ | 42 | 2 | 10 | 54 |
| [Steering/Lucy_Dyn10/lib/can_driver/can_driver.h](/Steering/Lucy_Dyn10/lib/can_driver/can_driver.h) | C++ | 13 | 1 | 6 | 20 |
| [Steering/Lucy_Dyn10/lib/can_service/can_service.c](/Steering/Lucy_Dyn10/lib/can_service/can_service.c) | C | 48 | 3 | 5 | 56 |
| [Steering/Lucy_Dyn10/lib/can_service/can_service.h](/Steering/Lucy_Dyn10/lib/can_service/can_service.h) | C++ | 11 | 0 | 4 | 15 |
| [Steering/Lucy_Dyn10/lib/ipc_manager/ipc.c](/Steering/Lucy_Dyn10/lib/ipc_manager/ipc.c) | C | 21 | 2 | 7 | 30 |
| [Steering/Lucy_Dyn10/lib/ipc_manager/ipc.h](/Steering/Lucy_Dyn10/lib/ipc_manager/ipc.h) | C++ | 15 | 1 | 6 | 22 |
| [Steering/Lucy_Dyn10/lib/led_driver/led_driver.c](/Steering/Lucy_Dyn10/lib/led_driver/led_driver.c) | C | 25 | 0 | 5 | 30 |
| [Steering/Lucy_Dyn10/lib/led_driver/led_driver.h](/Steering/Lucy_Dyn10/lib/led_driver/led_driver.h) | C++ | 18 | 0 | 5 | 23 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.c](/Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.c) | C | 7 | 0 | 2 | 9 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.h](/Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.h) | C++ | 26 | 5 | 10 | 41 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.cpp](/Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.cpp) | C++ | 60 | 2 | 12 | 74 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.h](/Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.h) | C++ | 6 | 0 | 2 | 8 |
| [Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.c](/Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.c) | C | 43 | 0 | 6 | 49 |
| [Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.h](/Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.h) | C++ | 12 | 1 | 7 | 20 |
| [Steering/Lucy_Dyn10/lib/volante_state/volante_state.h](/Steering/Lucy_Dyn10/lib/volante_state/volante_state.h) | C++ | 22 | 5 | 7 | 34 |
| [Steering/Lucy_Dyn10/platformio.ini](/Steering/Lucy_Dyn10/platformio.ini) | Ini | 15 | 1 | 2 | 18 |
| [Steering/Lucy_Dyn10/src/buttons_app.cpp](/Steering/Lucy_Dyn10/src/buttons_app.cpp) | C++ | 27 | 0 | 6 | 33 |
| [Steering/Lucy_Dyn10/src/buttons_app.h](/Steering/Lucy_Dyn10/src/buttons_app.h) | C++ | 9 | 0 | 4 | 13 |
| [Steering/Lucy_Dyn10/src/main.cpp](/Steering/Lucy_Dyn10/src/main.cpp) | C++ | 44 | 2 | 12 | 58 |
| [Steering/Lucy_Dyn10/test/test_main.cpp](/Steering/Lucy_Dyn10/test/test_main.cpp) | C++ | 84 | 3 | 22 | 109 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details