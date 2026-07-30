# Diff Details

Date : 2026-07-29 15:19:08

Directory c:\\Users\\DBDVU0X\\DYN-Vehicle-Firmware\\PDM

Total : 41 files,  -179 codes, -17 comments, -68 blanks, all -264 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [PDM/PDM_FW/include/pdm_config.h](/PDM/PDM_FW/include/pdm_config.h) | C++ | 11 | 2 | 3 | 16 |
| [PDM/PDM_FW/lib/can_service/can_service.c](/PDM/PDM_FW/lib/can_service/can_service.c) | C | 53 | 6 | 7 | 66 |
| [PDM/PDM_FW/lib/can_service/can_service.h](/PDM/PDM_FW/lib/can_service/can_service.h) | C++ | 10 | 0 | 4 | 14 |
| [PDM/PDM_FW/lib/ipc_manager/ipc.c](/PDM/PDM_FW/lib/ipc_manager/ipc.c) | C | 40 | 0 | 9 | 49 |
| [PDM/PDM_FW/lib/ipc_manager/ipc.h](/PDM/PDM_FW/lib/ipc_manager/ipc.h) | C++ | 22 | 0 | 8 | 30 |
| [PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.c](/PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.c) | C | 77 | 1 | 8 | 86 |
| [PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.h](/PDM/PDM_FW/lib/mosfet_driver/mosfet_driver.h) | C++ | 16 | 1 | 6 | 23 |
| [PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.c](/PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.c) | C | 59 | 1 | 9 | 69 |
| [PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.h](/PDM/PDM_FW/lib/mux_adc_driver/mux_adc_driver.h) | C++ | 12 | 0 | 4 | 16 |
| [PDM/PDM_FW/lib/protection/protection.c](/PDM/PDM_FW/lib/protection/protection.c) | C | 39 | 6 | 12 | 57 |
| [PDM/PDM_FW/lib/protection/protection.h](/PDM/PDM_FW/lib/protection/protection.h) | C++ | 13 | 1 | 5 | 19 |
| [PDM/PDM_FW/platformio.ini](/PDM/PDM_FW/platformio.ini) | Ini | 13 | 0 | 2 | 15 |
| [PDM/PDM_FW/src/app.cpp](/PDM/PDM_FW/src/app.cpp) | C++ | 94 | 6 | 16 | 116 |
| [PDM/PDM_FW/src/app.h](/PDM/PDM_FW/src/app.h) | C++ | 9 | 0 | 4 | 13 |
| [PDM/PDM_FW/src/main.cpp](/PDM/PDM_FW/src/main.cpp) | C++ | 5 | 0 | 2 | 7 |
| [PDM/PDM_FW/test/test_main.cpp](/PDM/PDM_FW/test/test_main.cpp) | C++ | 20 | 0 | 5 | 25 |
| [Steering/Lucy_Dyn10/include/app_log.h](/Steering/Lucy_Dyn10/include/app_log.h) | C++ | -39 | -9 | -9 | -57 |
| [Steering/Lucy_Dyn10/lib/bsp/bsp.c](/Steering/Lucy_Dyn10/lib/bsp/bsp.c) | C | -4 | -1 | -2 | -7 |
| [Steering/Lucy_Dyn10/lib/bsp/bsp.h](/Steering/Lucy_Dyn10/lib/bsp/bsp.h) | C++ | -13 | 0 | -5 | -18 |
| [Steering/Lucy_Dyn10/lib/button_driver/button_driver.c](/Steering/Lucy_Dyn10/lib/button_driver/button_driver.c) | C | -55 | -2 | -9 | -66 |
| [Steering/Lucy_Dyn10/lib/button_driver/button_driver.h](/Steering/Lucy_Dyn10/lib/button_driver/button_driver.h) | C++ | -13 | -1 | -7 | -21 |
| [Steering/Lucy_Dyn10/lib/can_driver/can_driver.cpp](/Steering/Lucy_Dyn10/lib/can_driver/can_driver.cpp) | C++ | -42 | -2 | -10 | -54 |
| [Steering/Lucy_Dyn10/lib/can_driver/can_driver.h](/Steering/Lucy_Dyn10/lib/can_driver/can_driver.h) | C++ | -13 | -1 | -6 | -20 |
| [Steering/Lucy_Dyn10/lib/can_service/can_service.c](/Steering/Lucy_Dyn10/lib/can_service/can_service.c) | C | -48 | -3 | -5 | -56 |
| [Steering/Lucy_Dyn10/lib/can_service/can_service.h](/Steering/Lucy_Dyn10/lib/can_service/can_service.h) | C++ | -11 | 0 | -4 | -15 |
| [Steering/Lucy_Dyn10/lib/ipc_manager/ipc.c](/Steering/Lucy_Dyn10/lib/ipc_manager/ipc.c) | C | -21 | -2 | -7 | -30 |
| [Steering/Lucy_Dyn10/lib/ipc_manager/ipc.h](/Steering/Lucy_Dyn10/lib/ipc_manager/ipc.h) | C++ | -15 | -1 | -6 | -22 |
| [Steering/Lucy_Dyn10/lib/led_driver/led_driver.c](/Steering/Lucy_Dyn10/lib/led_driver/led_driver.c) | C | -25 | 0 | -5 | -30 |
| [Steering/Lucy_Dyn10/lib/led_driver/led_driver.h](/Steering/Lucy_Dyn10/lib/led_driver/led_driver.h) | C++ | -18 | 0 | -5 | -23 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.c](/Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.c) | C | -7 | 0 | -2 | -9 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.h](/Steering/Lucy_Dyn10/lib/nextion_driver/dashboard_struct.h) | C++ | -26 | -5 | -10 | -41 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.cpp](/Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.cpp) | C++ | -60 | -2 | -12 | -74 |
| [Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.h](/Steering/Lucy_Dyn10/lib/nextion_driver/nextion_driver.h) | C++ | -6 | 0 | -2 | -8 |
| [Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.c](/Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.c) | C | -43 | 0 | -6 | -49 |
| [Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.h](/Steering/Lucy_Dyn10/lib/rotary_driver/rotary_driver.h) | C++ | -12 | -1 | -7 | -20 |
| [Steering/Lucy_Dyn10/lib/volante_state/volante_state.h](/Steering/Lucy_Dyn10/lib/volante_state/volante_state.h) | C++ | -22 | -5 | -7 | -34 |
| [Steering/Lucy_Dyn10/platformio.ini](/Steering/Lucy_Dyn10/platformio.ini) | Ini | -15 | -1 | -2 | -18 |
| [Steering/Lucy_Dyn10/src/buttons_app.cpp](/Steering/Lucy_Dyn10/src/buttons_app.cpp) | C++ | -27 | 0 | -6 | -33 |
| [Steering/Lucy_Dyn10/src/buttons_app.h](/Steering/Lucy_Dyn10/src/buttons_app.h) | C++ | -9 | 0 | -4 | -13 |
| [Steering/Lucy_Dyn10/src/main.cpp](/Steering/Lucy_Dyn10/src/main.cpp) | C++ | -44 | -2 | -12 | -58 |
| [Steering/Lucy_Dyn10/test/test_main.cpp](/Steering/Lucy_Dyn10/test/test_main.cpp) | C++ | -84 | -3 | -22 | -109 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details