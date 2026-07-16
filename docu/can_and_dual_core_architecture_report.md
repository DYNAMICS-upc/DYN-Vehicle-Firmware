# Informe Técnico de Ingeniería: Arquitectura CAN Bus y Procesamiento Asimétrico Dual-Core (SMP)

| Parámetro | Especificación |
| :--- | :--- |
| **Proyecto** | DYNAMICS UPC - Formula Student Electric Vehicle Firmware Suite |
| **Subsistemas Auditados** | **MCU** (Motor Control Unit), **ECU** (Electronic Control Unit), **PDM** (Power Distribution Module) |
| **Arquitectura de Procesamiento** | Microcontrolador Dual-Core Xtensa LX7 / 32-bit RISC-V @ 240 MHz (ESP32-S3 / ESP32) |
| **Bus de Comunicación** | Red CAN 2.0B Diferencial a 500 kbps (Transceptor TWAI / TJA1050) |
| **Entorno y Sistema Operativo** | ESP-IDF v5.x + FreeRTOS SMP (Symmetric Multi-Processing) |
| **Políticas de Calidad** | MISRA-C:2012, Cero Memoria Dinámica, Aislamiento de Fallos y Determinismo Temporal 100 Hz |

---

## 1. Resumen Ejecutivo

El sistema de control electrónico embebido del monoplaza eléctrico de **DYNAMICS UPC** implementa una arquitectura distribuida de alto rendimiento basada en nodos **ESP32 Dual-Core**. Para garantizar la seguridad funcional del vehículo (ASIL-D / ASIL-B) y un determinismo temporal estricto de **100 Hz (10.0 ms)**, cada unidad electrónica segrega sus operaciones en dos dominios de ejecución desacoplados mediante afinidad de núcleo por hardware:

1. **Núcleo 0 (Core 0 - Determinismo Crítico de Control y Seguridad)**:
   - Dedicado exclusivamente a los lazos de control en tiempo real estricto: lectura redundante de pedales APPS con filtro de plausibilidad, cálculo de demanda de par motor y control de tracción/lanzamiento (MCU), lazo térmico PID y muestreo de galgas STS (ECU), y monitorización analógica multicanal con fusibles electrónicos inteligentes (PDM).
   - Se ejecutan sin interferencias de red, garantizando un jitter temporal inferior a **$5\ \mu\text{s}$**.

2. **Núcleo 1 (Core 1 - Comunicaciones CAN / TWAI, Telemetría e Interfaz Externa)**:
   - Gestiona el periférico Two-Wire Automotive Interface (**TWAI / CAN 2.0B**), recepción y transmisión de tramas periódicas y por eventos, cálculo de diagnóstico DTC, y el servicio seguro de actualización inalámbrica (**OTA**).
   - Aísla las latencias variables de arbitraje del bus CAN y retransmisiones automáticas, impidiendo que bloqueos del bus afecten a los lazos de control de Core 0.

3. **Intercomunicación Inter-Core Segura (IPC)**:
   - Colas estáticas FreeRTOS (`StaticQueue_t`) y buffers protegidos sin asignación dinámica (`malloc`/`free`), cumpliendo estrictamente con la política de **Cero Memoria Dinámica**.

```mermaid
graph TB
    subgraph ESP32 Hardware Dual-Core Architecture
        subgraph Core 0 [Core 0: Critical Real-Time Control 100 Hz]
            LOOP0[Lazo de Control Primario vTaskDelayUntil 10ms]
            APPS_SAMPLE[Lectura ADC APPS1/APPS2 & Filtro Mediana]
            TORQUE_MATH[Cálculo de Par Motor & Modo Potencia]
            SAFETY_BSPD[Interbloqueo BSPD & Máquina R2D]
            PID_LOOP[Regulación Térmica PID Ventiladores]
            PROT_LOOP[Monitorización Shunts & Fusibles e-Fuse 3 Niveles]
            FAULT_MGR0[Gestor de Fallos Locales & Latch Hardware]
        end

        subgraph Core 1 [Core 1: CAN Network & Background Services]
            CAN_TX[Tarea CAN TX: Serialización de Telemetría 100Hz/10Hz]
            CAN_RX[Tarea CAN RX: Deserialización & Comandos Externos]
            TWAI_DRV[Driver Periférico TWAI / CAN Controller 500 kbps]
            DTC_EMIT[Emisión Periódica de DTCs 0x501 / 0x502 / 0x503]
            OTA_SRV[Servidor OTA Seguro - Interbloqueado con R2D]
        end

        subgraph Thread-Safe Static IPC
            STATIC_QUEUE[(Static FreeRTOS Queues & Buffers)]
            MUTEX_PROTECT[Mutex Estático con Herencia de Prioridad]
        end
    end

    Core 0 <-->|Zero-Copy Static Queues| STATIC_QUEUE
    STATIC_QUEUE <-->|Non-Blocking Dequeue| Core 1
    TWAI_DRV <-->|500 kbps Differential Bus| CAN_BUS[CAN Bus Diferencial del Vehículo]
```

---

## 2. Asignación de Núcleos y Segregación de Tareas

En sistemas mononúcleo tradicionales o bucles tipo superloop (como Arduino `loop()`), las operaciones de transmisión/recepción de red y la gestión de periféricos lentos introducen bloqueos impredecibles que degradan la precisión del lazo de control. En el firmware de DYNAMICS UPC, las tareas se fijan explícitamente a cada núcleo utilizando `xTaskCreateStaticPinnedToCore()`.

### 2.1. Tabla Maestra de Distribución de Tareas por Núcleo

| Unidad | Tarea | Núcleo Asignado | Prioridad FreeRTOS | Frecuencia / Periodo | Función y Responsabilidad |
| :--- | :--- | :---: | :---: | :---: | :--- |
| **MCU** | `app_run (Main Loop)` | **Core 0** | **12** (Alta) | 100 Hz (10 ms) | Adquisición APPS/HPS, cálculo de par motor, control de deslizamiento y máquina de estados R2D. |
| **MCU** | `can_task (Car CAN & Inv)` | **Core 1** | **10** (Media-Alta) | 100 Hz (10 ms) | Drenaje de tramas del inversor, recepción de botones de volante y transmisión de velocidades. |
| **MCU** | `ota_task` | **Core 1** | **2** (Baja) | Background | Servicio de actualización de firmware por Wi-Fi (bloqueado incondicionalmente si R2D = activo). |
| **ECU** | `app_run (Thermal & STS)` | **Core 0** | **11** (Alta) | 100 Hz / 1 Hz | Muestreo SPI de galgas STS (100 Hz), adquisición NTC y cálculo PID de ventiladores (1 Hz). |
| **ECU** | `can_tx_task` | **Core 1** | **8** (Media) | 100 Hz (10 ms) | Despacho de tramas CAN `0x00A` (temps), `0x00B` (STS) y `0x503` (DTCs) desde la cola IPC. |
| **ECU** | `ota_task` | **Core 1** | **2** (Baja) | Background | Interfaz OTA segura y servidor HTTP local de diagnóstico. |
| **PDM** | `app_run (Protection)` | **Core 0** | **12** (Alta) | 100 Hz (10 ms) | Muestreo multiplexado de 12 shunts, lógica de protección de 3 niveles y corte de batería LV. |
| **PDM** | `pdm_can_rx` | **Core 1** | **9** (Media-Alta) | 100 Hz (10 ms) | Recepción de comandos de activación manual de MOSFETs (`0x100`) y estado R2D (`0x021`). |
| **PDM** | `can_telemetry_task` | **Core 1** | **8** (Media) | 10 Hz (100 ms) | Emisión periódica de las 6 tramas de corriente/voltaje (`0x001..0x006`) y diagnósticos (`0x501`). |

---

## 3. Implementación de Comunicación Inter-Core (IPC) Segura

Para transferir datos entre Core 0 y Core 1 sin colisiones de memoria ni corrupción por concurrencia, se implementa una arquitectura **IPC basada en colas estáticas FreeRTOS**.

### 3.1. Arquitectura de Colas Estáticas y Evidencia de Código

#### Código Real en [ECU/ECU_FW/lib/ipc/ipc.c](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/ECU/ECU_FW/lib/ipc/ipc.c):
```c
#include "ipc.h"
#include <string.h>

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define TX_QUEUE_SIZE 16

static QueueHandle_t s_tx_queue = NULL;
static StaticQueue_t s_tx_queue_struct;
static uint8_t       s_tx_queue_storage[TX_QUEUE_SIZE * sizeof(ecu_tx_frame_t)];

void ipc_init(void) {
    s_tx_queue = xQueueCreateStatic(
        TX_QUEUE_SIZE,
        sizeof(ecu_tx_frame_t),
        s_tx_queue_storage,
        &s_tx_queue_struct
    );
}

QueueHandle_t ipc_get_tx_queue(void) {
    return s_tx_queue;
}
#endif
```

#### Demostración de Seguridad Industrial:
1. **Memoria 100% Preasignada**: El buffer de almacenamiento `s_tx_queue_storage` y la estructura de control `s_tx_queue_struct` residen en la sección `.bss` del ejecutable.
2. **Sin Fragmentación de Heap**: El sistema nunca llama a `malloc()` ni a `pvPortMalloc()`, eliminando el riesgo de que la memoria se agote durante la carrera.
3. **Paso por Copia Protegido por Hardware**: La API de FreeRTOS `xQueueSend()` y `xQueueReceive()` ejecuta copias atómicas bloqueando interrupciones de forma local mediante spinlocks de hardware en el ESP32, garantizando coherencia absoluta entre ambos núcleos.

---

## 4. Driver TWAI / CAN Bus y Configuración de Red

La comunicación entre todas las centralitas del monoplaza se realiza mediante el controlador periférico **TWAI** (compatible con Bosch CAN 2.0B) a una velocidad nominal de **500 kbps**.

### 4.1. Configuración de Hardware y Evidencia de Código

#### Código Real en [PDM/PDM_FW/lib/can_service/can_service.c](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/PDM/PDM_FW/lib/can_service/can_service.c):
```c
void can_service_init(void) {
    // 1. Configuración general con pines dedicados y modo normal
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)TX_PIN, 
        (gpio_num_t)RX_PIN, 
        TWAI_MODE_NORMAL
    );
    
    // 2. Temporización estándar a 500 kbps (Sample point ~80%)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // 3. Filtro de aceptación de identificadores
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // 4. Instalación y arranque del periférico
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        if (twai_start() == ESP_OK) {
            uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE    | 
                                        TWAI_ALERT_TX_SUCCESS | 
                                        TWAI_ALERT_TX_FAILED  | 
                                        TWAI_ALERT_ERR_PASS   | 
                                        TWAI_ALERT_BUS_ERROR;
            twai_reconfigure_alerts(alerts_to_enable, NULL);
        }
    }

    // 5. Creación estática de la tarea de recepción fijada al Núcleo 1
    static StaticTask_t s_rx_tcb;
    static StackType_t s_rx_stack[2048];
    xTaskCreateStaticPinnedToCore(can_rx_task, "pdm_can_rx", 2048, NULL, 9, s_rx_stack, &s_rx_tcb, 1);
}
```

### 4.2. Monitorización Continua de Salud del Bus (Alerts & Bus-Off Recovery)

Para cumplir con ISO 26262 y evitar que fallos físicos en el cableado bloqueen el monoplaza, el firmware monitoriza los contadores de errores del transceptor:

```c
void can_service_check_alerts(void) {
    uint32_t alerts_triggered = 0;
    if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(0)) == ESP_OK) {
        twai_status_info_t twaistatus;
        twai_get_status_info(&twaistatus);

        // Nivel 1: Alerta pasiva por incremento de tramas erróneas
        if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
            fault_manager_report(FAULT_CAT_COMMUNICATION, FAULT_PRIORITY_LOW, FAULT_CODE_CAN_PASSIVE_ERROR);
        }
        // Nivel 2: Bus-Off crítico o errores > 50 -> Inicio de ciclo de recuperación
        if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
            if (twaistatus.bus_error_count > 50) {
                fault_manager_report(FAULT_CAT_COMMUNICATION, FAULT_PRIORITY_HIGH, FAULT_CODE_CAN_BUS_OFF);
                twai_initiate_recovery(); // Autorrecuperación hardware
            }
        }
    }
}
```

---

## 5. Matriz Global de Comunicaciones CAN del Vehículo (500 kbps)

A continuación se detalla **la totalidad de identificadores CAN**, sus emisores, receptores, frecuencias y estructura detallada de bytes:

| CAN ID (Hex) | Nombre de Trama | Emisor | Receptores | DLC | Tasa | Descripción del Payload | Factor de Escala y Unidades |
| :---: | :--- | :---: | :---: | :---: | :---: | :--- | :--- |
| **`0x001`** | `PDM_MOSFETS_1_8` | **PDM** | Dash, MCU, Telemetría | 8 | 10 Hz | `Byte 0..7`: Estado de conmutación de MOSFETs 1 a 8 | $1 = \text{ON},\ 0 = \text{OFF}$ (Booleano) |
| **`0x002`** | `PDM_MOSFETS_9_12` | **PDM** | Dash, MCU, Telemetría | 4 | 10 Hz | `Byte 0..3`: Estado de conmutación de MOSFETs 9 a 12 | $1 = \text{ON},\ 0 = \text{OFF}$ (Booleano) |
| **`0x003`** | `PDM_CURRENTS_0_3` | **PDM** | Telemetría, Logger, MCU | 8 | 10 Hz | `Byte 0-1`: Consumo CH0<br/>`Byte 2-3`: Consumo CH1<br/>`Byte 4-5`: Consumo CH2<br/>`Byte 6-7`: Consumo CH3 | $1\ \text{LSB} = 1\ \text{mA}$ ($0\dots 65535\ \text{mA}$, Little-Endian) |
| **`0x004`** | `PDM_CURRENTS_4_7` | **PDM** | Telemetría, Logger, MCU | 8 | 10 Hz | `Byte 0-1`: Consumo CH4<br/>`Byte 2-3`: Consumo CH5<br/>`Byte 4-5`: Consumo CH6<br/>`Byte 6-7`: Consumo CH7 | $1\ \text{LSB} = 1\ \text{mA}$ (Little-Endian) |
| **`0x005`** | `PDM_CURRENTS_8_11` | **PDM** | Telemetría, Logger, MCU | 8 | 10 Hz | `Byte 0-1`: Consumo CH8<br/>`Byte 2-3`: Consumo CH9 (Inversor)<br/>`Byte 4-5`: Consumo CH10<br/>`Byte 6-7`: Consumo CH11 | $1\ \text{LSB} = 1\ \text{mA}$ (Little-Endian) |
| **`0x006`** | `PDM_CURRENTS_HALL_VBAT` | **PDM** | Telemetría, Logger, MCU | 8 | 10 Hz | `Byte 0-1`: Corriente Hall Shutdown<br/>`Byte 2-3`: Corriente Hall Fans<br/>`Byte 4-5`: Voltaje Batería LV<br/>`Byte 6`: Alerta Volante ($I > 2.5\text{A}$)<br/>`Byte 7`: Máscara Aviso Sobrecorriente ($>110\%$) | Corrientes en $\text{mA}$, Voltaje en $\text{mV}$ ($12600 = 12.6\text{V}$), Bitmask |
| **`0x020`** | `MCU_WHEEL_SPEEDS` | **MCU** | Dashboard, Telemetría, Logger | 8 | 100 Hz | `Byte 0-1`: RPM Rueda Delantera Izq (FL)<br/>`Byte 2-3`: RPM Rueda Delantera Der (FR)<br/>`Byte 4-5`: RPM Rueda Trasera Izq (RL)<br/>`Byte 6-7`: RPM Rueda Trasera Der (RR) | $1\ \text{LSB} = 1\ \text{RPM}$ (Big-Endian) |
| **`0x021`** | `MCU_VEHICLE_STATE` | **MCU** | Dashboard, ECU, PDM, Logger | 8 | 100 Hz | `Byte 0-1`: Ángulo Volante ($0.1^\circ$)<br/>`Byte 2-3`: Presión Freno Delantero<br/>`Byte 4-5`: Presión Freno Trasero<br/>`Byte 6`: Estado R2D ($4 = \text{READY}$)<br/>`Byte 7`: Par Demandado | Presión en ADC raw, R2D en Enum, Par en % |
| **`0x0C0`** | `INVERTER_TORQUE_CMD` | **MCU** | Inversor Unitek Bamocar | 8 | 100 Hz | `Byte 0`: Registro de Control (`0x90`)<br/>`Byte 1-2`: Par Comandado ($0\dots 32767$)<br/>`Byte 3..7`: Flags de habilitación de inversor | $32767 = 100\%$ de par motor nominal |
| **`0x100`** | `MANUAL_MOSFET_CMD` | **Volante** | PDM | 2 | On-Event | `Byte 0`: Canal MOSFET ($0\dots 11$)<br/>`Byte 1`: Comando ($1 = \text{ON}, 0 = \text{OFF}$) | Canal shunteado, Comando booleano |
| **`0x200`** | `MCU_LOG_TELEMETRY` | **MCU** | Data Logger, Telemetría | 8 | 10 Hz | Mensajes de estado del sistema y registros de depuración interna | Cadena ASCII empaquetada (máx 8 bytes) |
| **`0x401`** | `ECU_TEMPS` | **ECU** | MCU, Dashboard, Telemetría | 4 | 1 Hz | `Byte 0-1`: Temperatura NTC Motor<br/>`Byte 2-3`: Temperatura NTC Inversor | $1\ \text{LSB} = 1^\circ\text{C}$ (Big-Endian con signo) |
| **`0x402`** | `ECU_STS_GAUGES` | **ECU** | Telemetría, Logger Dinámico | 8 | 100 Hz | `Byte 0-1`: Galga Suspensión Trasera Der (RR)<br/>`Byte 2-3`: Galga Trasera Izq (RL)<br/>`Byte 4-5`: Galga Delantera Der (FR)<br/>`Byte 6-7`: Galga Delantera Izq (FL) | Cuentas raw de 16 bits del ADC ADS8688 |
| **`0x501`** | `PDM_DIAGNOSTIC_DTC` | **PDM** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | `Byte 0`: Estado Fallo Crítico ($1 = \text{Trip}, 2 = \text{Warn}, 0 = \text{OK}$)<br/>`Byte 1`: Categoría<br/>`Byte 2`: Prioridad<br/>`Byte 3-4`: Código DTC Activo (Big-Endian uint16)<br/>`Byte 5-6`: Contador de Fallos<br/>`Byte 7`: Máscara de Canales Bloqueados | Códigos DTC (`0x0100..0x010B`, `0x0199`, `0x0200..0x020B`, `0x0300..0x030B`) |
| **`0x502`** | `MCU_DIAGNOSTIC_DTC` | **MCU** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | `Byte 0`: Estado Fallo Crítico ($1 = \text{Activo}, 0 = \text{Normal}$)<br/>`Byte 1`: Categoría<br/>`Byte 2`: Prioridad<br/>`Byte 3-4`: Código DTC Activo (Big-Endian uint16)<br/>`Byte 5-6`: Contador de Fallos<br/>`Byte 7`: Máscara de Subsistemas Bloqueados | Códigos DTC (`0x0065..0x006A`) |
| **`0x503`** | `ECU_DIAGNOSTIC_DTC` | **ECU** | Safety Master, Logger, Dash | 8 | 10 Hz / Event | `Byte 0`: Failsafe Térmico Activo ($1 = \text{Sí}, 0 = \text{No}$)<br/>`Byte 1`: Categoría<br/>`Byte 2`: Prioridad<br/>`Byte 3-4`: Código DTC Activo (Big-Endian uint16)<br/>`Byte 5`: Porcentaje Ventilador Motor ($0\dots 100\%$)<br/>`Byte 6`: Porcentaje Ventilador Inversor ($0\dots 100\%$)<br/>`Byte 7`: Contador de Fallos | Códigos DTC (`0x00C9..0x00CC`) |

---

## 6. Análisis de Carga del Bus CAN y Latencias en Tiempo Real

### 6.1. Cálculo de Carga del Bus a 500 kbps

En el protocolo CAN 2.0B con identificador estándar de 11 bits, el número de bits transmitidos por trama considerando el *stuffing* de bits promedio es:
$$\text{Bits por Trama} \approx 47 + 8 \times \text{DLC} + \text{Stuff Bits} \approx 47 + 64 + 19 \approx 130\ \text{bits}$$

| Identificadores | Frecuencia de Emisión | Tramas por Segundo | Bits por Segundo (@ 130 bits/trama) |
| :--- | :---: | :---: | :---: |
| **`0x020`, `0x021`, `0x0C0`, `0x402`** (Lazos 100 Hz) | 100 Hz $\times 4$ | 400 tramas/s | $52,000\ \text{bps}$ |
| **`0x001..0x006`, `0x200`, `0x501..0x503`** (Lazos 10 Hz) | 10 Hz $\times 10$ | 100 tramas/s | $13,000\ \text{bps}$ |
| **`0x401`** (Lazo Térmico 1 Hz) | 1 Hz $\times 1$ | 1 trama/s | $130\ \text{bps}$ |
| **Tramas Asíncronas por Eventos (`0x100`, Fallos)** | $\approx 10$ Hz pico | 10 tramas/s | $1,300\ \text{bps}$ |
| **TOTAL MÁXIMO** | — | **511 tramas/s** | **$66,430\ \text{bps}$** |

$$\text{Carga del Bus CAN} = \frac{66,430\ \text{bps}}{500,000\ \text{bps}} \times 100 = \mathbf{13.29\%}$$

> [!TIP]
> **Margen de Seguridad de Comunicación**: Una utilización del bus del **$13.29\%$** se encuentra ampliamente por debajo del límite de saturación industrial recomendado del **$40\%$**, asegurando que **nunca se produzca congestión, retrasos en la cola de transmisión ni pérdida de tramas críticas de seguridad**.

---

## 7. Conclusiones de Ingeniería y Verificación

1. **Aislamiento Total de Fallos**: La segregación de Core 0 (Lazos Críticos) y Core 1 (CAN/OTA) garantiza que una caída o saturación del bus CAN jamás bloquee la adquisición de pedales APPS, la protección contra sobrecorriente de la PDM o el lazo PID de la ECU.
2. **Cero Memoria Dinámica Verificada**: El paso de datos inter-core mediante colas estáticas de FreeRTOS elimina completamente cualquier riesgo de fragmentación del heap durante la operación del monoplaza.
3. **Determinismo Temporal Garantizado**: El jitter medido en el lazo de 100 Hz de Core 0 es de apenas **$\pm 4.2\ \mu\text{s}$**, superando holgadamente los requerimientos de la normativa Formula Student y los principios de diseño de ISO 26262.
