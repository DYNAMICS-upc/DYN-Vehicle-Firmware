# Auditoría Firmware V42-V44

Este documento recoge los principales cambios estructurales realizados durante la campaña de refactorización masiva de las versiones V42 a V44 de la rama principal del vehículo.

## PDM (Power Distribution Module)
- Se ha desacoplado la máquina de estados (*FSM*) del ciclo principal.
- Refactorización de constantes codificadas a fuego (*magic numbers*) movidas a `pdm_config.h`.
- Se ha purgado el uso indiscriminado de `EXTRA_COMPONENT_DIRS` en `CMakeLists.txt` a favor del *Library Dependency Finder* nativo de PlatformIO.

## ECU (Engine Control Unit)
- Centralización de pines y macros en `bsp.h`.
- Se ha integrado una utilidad global `EMA_FILTER_SHIFT` para filtrar de forma asíncrona y eficiente (con operaciones de bits) el ruido de los sensores analógicos.
- Estandarización del orquestador `app_run()` sin delays bloqueantes.

## MCU (Motor Control Unit)
- Refactorización masiva: separación de dependencias de la FSM `r2d_manager` hacia `app.cpp`.
- Adopción estricta de paso de estado a través de structs (`vehicle_state_t`), inyectados a las dependencias, en vez de variables globales.
- Cierre del `CMakeLists.txt` confiando también en PlatformIO.

## Steering (Volante)
- Optimización crítica en `nextion_driver`: se ha añadido un cache local en la placa (`s_cached_dash`) para evitar saturar el bus UART con refrescos innecesarios.
- Eliminadas las llamadas DMA inexistentes en arquitectura AVR y arreglados conflictos de flags de compilación (`-Werror`) al redefinir `configSUPPORT_STATIC_ALLOCATION`.
- Se han enlazado todos los displays de telemetría (Temperaturas, Alto Voltaje, SOC, R2D) con la estructura de estado `dashboard_struct_t`.
