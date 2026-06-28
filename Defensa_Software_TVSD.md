# Biblia Definitiva de Defensa de Software - TVSD Formula Student

Este documento es la **Guía Técnica de Referencia** del equipo. Su propósito es diseccionar, analizar y justificar cada línea de código, cada patrón de arquitectura y cada decisión de hardware de nuestro monoplaza eléctrico. Está diseñado para que cualquier miembro del equipo comprenda la lógica del vehículo con un nivel de profundidad absoluto.

---

## Capítulo 1: Metodología de Desarrollo y Ciclo de Vida (SDLC)

El software de automoción requiere un nivel de rigor que penaliza severamente la improvisación. Este capítulo detalla cómo el equipo gestiona, verifica y despliega el código fuente, eliminando el error humano de la ecuación.

### 1.1 Ingeniería de Software en Automoción: Modelos de Trabajo

Históricamente, la industria del motor ha utilizado el **Modelo en V (V-Model)** para el desarrollo de sistemas embebidos críticos (frenos ABS, inyección). Este modelo asume que todos los requisitos funcionales están perfectamente definidos en la fase inicial, bajando por la rama izquierda de la 'V' (Diseño de Arquitectura, Diseño de Componentes, Codificación) y subiendo por la rama derecha (Tests Unitarios, Tests de Integración, Tests de Sistema).

**Rechazo del Modelo en V y Scrum en Formula Student:**
En nuestro entorno, el Modelo en V es inaplicable debido a la volatilidad del diseño de hardware. Si el equipo de *Powertrain* reemplaza un inversor a mitad de temporada, el V-Model obliga a reiniciar todo el ciclo documental, paralizando el avance. Del mismo modo, el framework **Scrum**, aunque ágil, opera mediante ciclos de tiempo bloqueados (*Sprints* de 1 a 4 semanas). Si detectamos un *bug* térmico crítico un jueves durante el *Testing* en pista, no podemos esperar al próximo "Sprint Planning" del lunes para asignarle recursos. El coche debe volver a la pista el viernes.

**La Adopción de Kanban y Lean Manufacturing:**
Nuestra arquitectura se gestiona mediante **Kanban**. Este sistema de "flujo continuo" (Pull System) carece de *Sprints* cerrados y se centra en una métrica crítica: el **Tiempo de Ciclo (Cycle Time)**, que es el tiempo total desde que un ingeniero asume un bug hasta que el parche está flasheado en el coche.
1. **Limitación de WIP (Work In Progress):** Prohibimos que un desarrollador tenga más de dos ramas de Git abiertas simultáneamente. Esto evita el *Context Switching* (sobrecarga cognitiva al saltar entre problemas) que estadísticamente es la causa principal de inyección de bugs lógicos.
2. **Priorización Asimétrica Continua:** El Director Técnico reorganiza el *Backlog* (cola de tareas) en tiempo real. Un fallo en el CAN Bus salta automáticamente al tope de la lista y es absorbido por el primer ingeniero que libera su WIP. Esta es la clave de nuestra velocidad de reacción (Evaluación interna: 8.6/10 de adaptación).

### 1.2 Arquitectura del Pipeline CI/CD (GitHub Actions)

El determinismo de nuestro software no depende de que el programador "escriba código sin errores". Eso es estadísticamente imposible. Depende de un sistema que **prohíbe** físicamente que el código con errores llegue al vehículo.

Nuestra infraestructura confía en un pipeline de **Integración Continua y Despliegue Continuo (CI/CD)** automatizado en la nube mediante *GitHub Actions*.

**Flujo de Aprobación Inquebrantable:**
1. **Push & Trigger:** Cuando un ingeniero sube una rama al repositorio, los servidores de GitHub levantan un contenedor virtual de Linux (Runner) e inicializan un entorno estéril.
2. **Compilación Cruzada (Cross-Compilation):** El Runner instala *PlatformIO* y el *Espressif IoT Development Framework (ESP-IDF)*. Inmediatamente, compila el código fuente traduciéndolo al lenguaje ensamblador de la arquitectura Xtensa32 (el procesador de nuestras placas).
3. **Tratamiento de Warnings como Errores (`-Werror`):** En un proyecto universitario estándar, un *warning* (aviso del compilador) es ignorado. En nuestro pipeline, hemos inyectado la *flag* `-Werror`. Si el compilador detecta una conversión de tipos dudosa (ej: meter un número de 32 bits en una variable de 16 bits), el pipeline lo considera un error fatal (*Exit Code 1*), aborta el proceso, y bloquea el botón de *Merge* (Fusión).
4. **Validación Nativa (Unit Testing):** Tras comprobar la sintaxis, el servidor compila una segunda versión del programa para arquitectura x86_64 (el procesador del servidor) usando librerías falsas (*Mocks*). Ejecuta el framework de pruebas *Unity*, inyectando valores falsos pero extremos a las funciones matemáticas del coche (ej: pedal a fondo, error térmico simulado). Si una sola aserción matemática (`TEST_ASSERT`) falla, la revisión queda vetada.
5. **Certificación del Binario:** Solo si el código compila perfectamente para la MCU, no tiene Warnings de seguridad, y aprueba el 100% de los exámenes matemáticos, el sistema genera el archivo final certificado `firmware.bin`.

**El Principio Fundacional:** *"Only verified code is deployed to the vehicle"*. Jamás flasheamos el microcontrolador desde el ordenador personal de un desarrollador. Todo binario que entra al coche ha sido certificado por el servidor.

> [!NOTE] 
> **Referencia de Implementación CI/CD:**
> Puedes auditar la configuración exacta del pipeline de integración en los archivos YAML de GitHub Actions, por ejemplo en [build_mcu.yml](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/.github/workflows/build_mcu.yml). Ahí se observa la bandera `-Werror` bloqueando código defectuoso y el lanzamiento del entorno nativo (`platformio test -e native`) usando el framework *Unity*.

### 1.3 Despliegue Over-The-Air (OTA) por Wi-Fi

Una vez que tenemos el binario `firmware.bin` certificado, debemos introducirlo en el cerebro del monoplaza. El método tradicional implica abrir los compartimentos estancos del monocasco, localizar el puerto micro-USB de la ECU, PDM o MCU, y conectar un cable. Esto es logísticamente inaceptable en un evento de *Endurance* o durante validación dinámica, donde cada minuto con el coche parado nos resta puntos.

**Nuestra Solución (Arquitectura OTA):**
Hemos implementado un sistema de flasheo inalámbrico aprovechando el silicio de telecomunicaciones integrado del ESP32. El ingeniero transfiere el binario desde su portátil, pasando por un router local en los boxes, directamente al coche, completando la reprogramación de una centralita en menos de 10 segundos.

**Profundidad Técnica de la OTA:**
El chip no se sobrescribe a sí mismo (lo cual sería fatal si se corta la conexión Wi-Fi a medias, dejando el coche *brickeado* o inútil). Empleamos un sistema de **Partición Dual Pasiva**:
- La memoria Flash del microcontrolador está dividida lógicamente mediante una tabla de particiones `.csv` en dos sectores masivos: `App0` y `App1`.
- Si el coche está ejecutando el código desde `App0`, el servicio OTA en segundo plano escucha conexiones HTTP.
- Al recibir el nuevo archivo `.bin`, el procesador lo va escribiendo, bloque a bloque, en la partición dormida (`App1`).
- Una vez finalizada la descarga, el ESP32 calcula un *Hash Criptográfico (Checksum)* sobre los megabytes descargados y lo compara con el servidor. Si falta un solo bit por un corte de red, el archivo se considera corrupto, se borra y el coche sigue funcionando con `App0`.
- Si el *Checksum* es válido, el *Bootloader* (gestor de arranque intocable del procesador) actualiza su puntero para que, en el próximo reinicio, el coche despierte ejecutando `App1`.

> [!TIP]
> **Referencia de Código OTA:**
> La lógica matemática para escribir la partición pasiva y validar la integridad con las librerías nativas de Espressif (`esp_ota_ops.h`) está implementada en [ota_service.c](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW/lib/ota_service/ota_service.c). Esta misma arquitectura escalable se replica idénticamente en la PDM y la ECU.

**Justificación del Trade-off OTA:**
Mantener la antena Wi-Fi encendida consume memoria RAM profunda y un 40% de ciclos de reloj de un núcleo del procesador para gestionar el mastodóntico Stack TCP/IP (LwIP). ¿Cómo defendemos este consumo masivo en un vehículo de carreras determinista? Mediante el Aislamiento de Núcleo (Core Pinning), que detallaremos en el siguiente capítulo de Arquitectura de Hardware.


---

## Capítulo 2: Arquitectura de Hardware (El Cerebro)

El software no se ejecuta en el vacío; está indisolublemente atado al silicio que lo procesa. Este capítulo disecciona la Unidad Microcontroladora (MCU) que dota de inteligencia al monoplaza, justificando por qué descartamos los estándares tradicionales en favor de una arquitectura heterogénea moderna.

### 2.1 Microcontroladores en Automoción: Estudio Comparativo

Durante la fase conceptual del diseño electrónico (Concept Design), el equipo de *Electrical Engineering* (EE) debió seleccionar la plataforma de procesamiento para las centralitas principales: Master Control Unit (MCU), Power Distribution Module (PDM) y Electronic Control Unit (ECU).

Las opciones sobre la mesa fueron:
1. **Teensy 4.1 (NXP i.MX RT1062):** Una bestia bruta operando a 600 MHz.
   * *Desventaja:* Es un procesador *Single-Core* (un solo núcleo) sin conectividad inalámbrica de silicio nativa. Implementar telemetría en vivo o despliegues OTA requeriría soldar un módulo externo por bus SPI, añadiendo latencia parasitaria y puntos críticos de fallo de hardware por vibración mecánica.
2. **STM32F4 / F7 (STMicroelectronics):** El estándar de oro en sistemas embebidos de grado automotriz y robótico (basado en arquitectura ARM Cortex-M).
   * *Desventaja:* Aunque su HAL (Hardware Abstraction Layer) es inmensamente fiable, sigue siendo un ecosistema de núcleo único para esta gama de precios. Intercalar los interrupciones del transceptor CAN físico con el envío asíncrono masivo de telemetría es una pesadilla de planificación en un solo procesador de ~168 MHz.
3. **Espressif ESP32-WROOM-32E (La Elección Definitiva):**
   * Incorpora un procesador Xtensa® Dual-Core 32-bit LX6 operando hasta 240 MHz.
   * Dispone de 520 KB de SRAM interna y hardware de banda base Wi-Fi/Bluetooth 4.2 integrado en el propio dado de silicio.
   * Su *Framework* de desarrollo nativo, ESP-IDF, está apoyado de forma oficial en FreeRTOS.

**Veredicto de Diseño:** Seleccionamos el ESP32 no por su potencia matemática pura, sino por su **topología de doble núcleo**, la cual soluciona el dilema de concurrencia de nuestro vehículo. Nos otorga la capacidad de separar físicamente el mundo del "Tiempo Real Estricto" del mundo de "Conectividad Asíncrona".

### 2.2 Arquitectura Interna del SoC ESP32 y Buses de Memoria

Para defender este hardware ante los auditores, debemos conocer sus intestinos. El ESP32 es un *System on a Chip* (SoC), no un simple microprocesador.

- **CPU y Pipeline:** Los procesadores Xtensa LX6 cuentan con un pipeline de 5 a 7 etapas, capaces de ejecutar instrucciones multiplicativas masivas en un solo ciclo de reloj, esencial para los mapas de Torque y transformaciones matemáticas de ángulos (Filtros Kalman o similares si fuesen aplicados).
- **Controlador DMA (Direct Memory Access):** El ESP32 posee un árbitro de memoria que permite a los periféricos hardware (como los conversores analógico-digitales que leen los pedales, o el controlador CAN que recibe los mensajes) volcar los datos de voltaje de los sensores directamente en la memoria RAM principal, sin despertar a la CPU principal. Esto significa que la CPU no pierde valiosos microsegundos verificando si ha llegado voltaje al pin; la RAM se auto-rellena "mágicamente" en segundo plano.

### 2.3 Asimetría de Núcleos (Core Pinning) y Aislamiento Físico

El problema histórico del software de automoción conectado (Connected Car) es que los paquetes de red arruinan el *Determinismo* mecánico. 

Si usamos un único núcleo, y la placa está calculando cuánta potencia dar al inversor, pero de pronto recibe una solicitud Wi-Fi para actualizar un log, el núcleo tiene que guardar todo lo que está haciendo, procesar el Wi-Fi (que toma milisegundos), y luego volver al cálculo de potencia. En ese *impasse*, el inversor del coche eléctrico podría haberse descontrolado.

**La Solución Estructural: El Aislamiento por Silicio**
Hacemos un *Hard-Pinning* de las responsabilidades. Bloqueamos cada tarea a un núcleo físico específico en su inicialización.

- **Core 0 (PRO_CPU - "El Relacionista Público"):**
  - Todas las tareas que exigen tiempo pero no son vitales para evitar que el coche se estrelle son desterradas a este núcleo.
  - Esto incluye: El servidor HTTP del despliegue OTA, el stack de red LwIP (TCP/UDP), la pila de cifrado mbedTLS para comunicaciones seguras, el formateo de strings pesados para los *Logs* a la tarjeta SD, y la actualización de los pixeles de las pantallas del Dashboard.
  - *Mitigación de Fallo:* Si el Core 0 sufre una saturación (Ej. 10 portátiles del equipo intentan conectar por Wi-Fi simultáneamente al coche), el Core 0 llegará a un 100% de uso de CPU y colapsará. Pero el coche no se parará.

- **Core 1 (APP_CPU - "El Cirujano"):**
  - Este núcleo es un búnker de alta seguridad, desconectado de todo factor externo.
  - Aquí orquestamos: El Controlador TWAI (El Bus CAN físico que maneja 1,500 frames por segundo), la Máquina de Estados Finita (Secuencia R2D, mitigación de fallos térmicos) y las rutinas de implausibilidad del pedal (*Brake Plausibility Check*).
  - *Mitigación de Fallo:* Al no tener que lidiar con la red Wi-Fi, el Core 1 funciona como un metrónomo perfecto, respetando su bucle principal con latencias inferiores a 12 milisegundos. 

Esta segregación de responsabilidades a nivel físico es nuestra armadura más gruesa ante las preguntas de los jueces sobre "¿Por qué un simple micro de IoT gobierna un coche de 80kW?".

> [!IMPORTANT]
> **Referencia de Asimetría (Core Pinning):**
> La asignación estricta de tareas al Core 1 y Core 0 se realiza mediante la función `xTaskCreatePinnedToCore` de FreeRTOS. Esto es visible en la inicialización (setup) de cada placa, como por ejemplo en el [main.cpp de la MCU](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW/src/main.cpp).


---

## Capítulo 3: Sistema Operativo en Tiempo Real y Gestión de Memoria

El control de un vehículo eléctrico requiere ejecutar múltiples procesos (sensado, filtrado, comunicaciones, telemetría) simultáneamente. Para gobernar el procesador sin violar las leyes de la física ni los requerimientos temporales, utilizamos **FreeRTOS**.

### 3.1 Teoría de RTOS y el Planificador (Scheduler)

Un procesador tradicional lee el código línea por línea. Si le ordenamos leer la SD y luego leer el acelerador, y la SD tarda 100 milisegundos en responder, el acelerador no se lee durante 100 milisegundos. En un *Bare-Metal Super-Loop* (el clásico `while(1)` de Arduino), esto es inevitable a menos que se diseñen máquinas de interrupción extremadamente frágiles.

**La Intervención del Planificador Expropiativo (Preemptive Scheduler):**
FreeRTOS incluye un módulo matemático llamado Planificador que corre a nivel de procesador (tick del sistema, habitualmente configurado a 1000 Hz, es decir, evalúa el sistema cada 1 milisegundo).
- A cada tarea le asignamos una **Prioridad** (un número del 0 al 25, por ejemplo).
- Si la Tarea A (Telemetría Wi-Fi, Prioridad 1) está ejecutándose en el Core 1, y la Tarea B (Freno de Emergencia, Prioridad 20) decide que es su momento de ejecutarse, el Planificador aplica la *Expropiación (Preemption)*.
- Literalmente detiene a la Tarea A a mitad de una instrucción matemática, guarda sus registros de CPU en la pila (*Context Switch*), ejecuta la Tarea B entera, y luego restaura a la Tarea A exactamente donde lo dejó.
- **Defensa ante Jueces:** Esto asegura que las funciones vitales para la dinámica del coche nunca esperen a que las funciones triviales terminen. El determinismo está garantizado por el sistema operativo, no por la buena voluntad del código.

### 3.2 Gestión de Memoria Estática: El Problema de la Fragmentación

En la arquitectura de ordenadores, cuando un programa necesita espacio para guardar una variable grande, solicita memoria al sistema operativo utilizando `malloc()` (Memory Allocate), lo cual extrae memoria del *Heap* (Montículo dinámico). Al terminar, usa `free()` para devolverla.

**El Peligro del Memory Leak y el OOM (Out Of Memory):**
En un coche que funciona ininterrumpidamente durante 30 minutos a temperaturas extremas, si una función olvida hacer `free()`, la memoria se consume gota a gota (*Memory Leak*). Además, el constante pedir y soltar bloques de distintos tamaños deja huecos libres pero dispersos en la RAM. Cuando llega un bloque grande, no cabe en ningún hueco continuo, colapsando el sistema operativo con un *Hard Fault* o *Kernel Panic*.

**Nuestra Solución (Static Allocation Draconiana):**
Hemos purgado absolutamente todas las primitivas dinámicas del código fuente.
- En lugar de `xTaskCreate`, usamos `xTaskCreateStatic`.
- En lugar de `xQueueCreate`, usamos `xQueueCreateStatic`.
- El desarrollador se ve obligado a definir un *Buffer* de bytes de forma global (sección `.bss` de la memoria): `uint8_t rx_buffer[1024];`
- Al hacer esto, **toda la memoria RAM que el coche va a utilizar en su vida útil se bloquea y se reserva durante el segundo cero del arranque**.
- Si el coche arranca sin reportar falta de RAM, es **matemáticamente imposible** que sufra un fallo de memoria durante la carrera. La seguridad estructural justifica sobradamente el hecho de que "despertemos" RAM que a veces no usamos al 100%.

### 3.3 Comunicación Inter-Procesos (IPC) y Sincronización

Con múltiples núcleos y múltiples tareas leyendo y escribiendo al mismo tiempo, nos enfrentamos al terror de los sistemas paralelos: Las **Race Conditions** (Condiciones de Carrera).
Si el Core 0 lee la variable global `velocidad` exactamente en el mismo nanosegundo en que el Core 1 está sobrescribiendo el valor 0 sobre el antiguo 100, el Core 0 podría leer un número corrupto (ej. 50).

**Nuestra Mitigación:**
Ninguna tarea lee ni escribe variables ajenas directamente. Utilizamos los mecanismos de sincronización certificados de FreeRTOS.
1. **Colas Estáticas (Static Queues):** Si la Tarea Sensores tiene el valor del pedal, lo empaqueta en una estructura (Struct) y lo inyecta a una Cola. La Cola bloquea el acceso de escritura internamente (mediante interrupciones críticas o Spinlocks) hasta que la copia atómica ha finalizado. La Tarea Control extrae el paquete de forma limpia.
2. **Mutexes (Mutual Exclusion):** Cuando dos tareas necesitan usar un mismo recurso de hardware (por ejemplo, el bus SPI para hablar con el módulo de termopares), usamos un *Mutex*. Es un "Testigo" de relevos. Si el Core 0 tiene el Mutex, el Core 1 se queda bloqueado esperando en estado pasivo (sin gastar CPU) hasta que el Core 0 suelta el testigo.

Al encapsular el tránsito de variables en el módulo `ipc_manager.c`, garantizamos que no existen lecturas corruptas de sensores.

> [!NOTE]
> **Referencia de Memoria Estática y Colas IPC:**
> Para demostrar a los jueces que no usamos `malloc` y que todas nuestras estructuras de comunicación son estáticas, puedes enseñarles la instanciación de colas (usando `xQueueCreateStatic` con su respectivo buffer `.bss`) en el gestor de comunicación de la MCU en [ipc_manager.c](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW/src/ipc_manager.c).


---

## Capítulo 4: Redes Vehiculares (El Sistema Nervioso CAN)

El *Controller Area Network* (Bus CAN) es la arteria principal del vehículo. Sin él, la ECU es ciega y la MCU es muda. En este capítulo diseccionamos la carga matemática y el comportamiento físico que nos permite asegurar 12ms de latencia.

### 4.1 Capa Física y Mitigación Electromagnética (EMI)

El interior de un monoplaza eléctrico alberga un inversor que conmuta cientos de amperios a frecuencias de Kilohercios. Esto genera un campo electromagnético (EMI) masivo. Si uniésemos las placas con un cable de cobre simple (Single-Ended, como I2C o UART), el ruido induciría voltajes falsos y el coche se volvería loco.

**Señalización Diferencial (CAN-H y CAN-L):**
El estándar CAN transmite la información por dos cables entrelazados (Twisted Pair). No mide el voltaje absoluto de un cable, sino la **diferencia** entre ambos. 
- Si un pulso magnético parásito añade +2V a ambos cables, la diferencia entre ambos sigue siendo la misma. El ruido de "modo común" es ignorado por los transceptores físicos (TJA1050, SN65HVD230).
- La red está terminada en ambos extremos (ECU e Inversor) por resistencias de 120 ohmios para evitar la reflexión de ondas (rebotes de señal cuando la onda choca con el final del cable).

### 4.2 Capa MAC (Medium Access Control) y Arbitraje

A diferencia de una red Ethernet donde hay un *Switch* que organiza el tráfico, el CAN es una red *Multi-Master Broadcast*. Todas las placas gritan sus mensajes al aire a la vez.

**Arbitraje CSMA/CA (Carrier-Sense Multiple Access con Collision Avoidance):**
Si la MCU grita el nivel de batería (ID 0x100) y la PDM grita un fallo térmico masivo (ID 0x010) exactamente a la misma vez, las tramas colisionan. 
- En el bus CAN, el bit 0 se llama *Dominante* (fuerza los cables a un voltaje) y el bit 1 es *Recesivo* (no hace nada).
- Cuando ambas placas hablan, leen el bus al mismo tiempo. La placa que intenta mandar un "1" (recesivo) y lee un "0" (dominante) en el cable, se da cuenta de que otra placa con mayor prioridad está hablando. Inmediatamente se calla y espera al próximo turno.
- **Asignación de IDs:** Sabiendo esto, hemos configurado las IDs más bajas numéricamente (más ceros iniciales) a los comandos críticos del vehículo. Un comando de apagado por *Brake Plausibility* siempre aplastará a un paquete de telemetría de temperatura en el arbitraje físico, sin retrasar el pulso de salvamento.

### 4.3 Modelado Matemático de la Carga de Bus (Bus Load)

El TVSD especifica que debemos gobernar **15 IDs de sensores operando a 100 Hz**. La justificación de esto ante los jueces no puede ser intuitiva, debe ser matemática.

Configuramos el controlador interno del ESP32 (el TWAI, Two-Wire Automotive Interface) a una tasa de baudio de **500 kbps**.

**Ecuación de Tiempo de Bit (Bit Timing):**
A 500 kbps, el tiempo de transmisión de un solo bit en el cobre es:
$T_{bit} = 1 / 500,000 = 0.000002 \text{ segundos (2 microsegundos)}$.

**Cálculo de Trama Máxima (Payload + Overhead):**
Una trama CAN estándar contiene:
- 1 bit de Start (SOF)
- 11 bits de Identificador (ID)
- 1 bit RTR + 1 bit IDE + 1 bit r0 + 4 bits DLC (Data Length Code)
- 64 bits de Datos (8 Bytes de Payload máximo)
- 15 bits de CRC
- 3 bits (CRC Delimiter, ACK, ACK Delimiter)
- 7 bits de End of Frame (EOF)
- 3 bits de Interframe Spacing (IFS)
- Total teórico sin *stuffing*: **111 bits**.

**El Efecto Bit Stuffing:**
El estándar CAN prohíbe que existan más de 5 bits consecutivos del mismo valor (para mantener la sincronización de los relojes). Si hay seis '1's, el hardware inyecta un '0' extra. En el peor escenario (Worst-Case Scenario), el stuffing añade hasta 24 bits extra.
Total *Worst-Case*: **135 bits por trama**.

**Cálculo de Carga Total (Bus Load %):**
- 15 mensajes distintos a 100 Hz = 1,500 tramas por segundo.
- Tiempo por trama = 135 bits × 2 µs/bit = 270 µs (0.000270 s).
- Tiempo total ocupado del bus por segundo = 1,500 tramas × 0.000270 s = **0.405 segundos**.
- **Bus Load Máximo: 40.5%**

**Conclusión de Ingeniería:** La industria de la automoción estipula un umbral de peligro a partir del 60%-70% de *Bus Load*. Nuestro diseño arroja un 40.5%, dejando casi un 60% del ancho de banda libre. Esto garantiza que, frente a un pico masivo de radiación EMI que obligue a los nodos a retransmitir múltiples tramas por errores de CRC (Automatic Retransmission), el bus no se saturará ni perderá su determinismo.

### 4.4 Demostración de la Latencia End-to-End (~12ms)

La latencia declarada en el TVSD (12ms) es el resultado de la trazabilidad desde el pedal hasta el motor.
1. Lectura Física (Hardware DMA del ADC): 1 ms.
2. Procesamiento Matemático VCL en Core 1: 1 ms.
3. Movimiento IPC a la tarea Transmisora CAN: < 1 ms.
4. Serialización TWAI y Tiempo en el Cable (Worst Case CSMA/CA Delay): ~3-4 ms.
5. Deserialización interna en el inversor comercial: ~5 ms.

Nuestra arquitectura inter-core contribuye con menos de **3 milisegundos de overhead** absoluto, maximizando la capacidad de respuesta mecánica del vehículo.


---

## Capítulo 5: Lógica de Control de Vehículo (VCL) y Asimetría

El *Hardware* proporciona el músculo; el *FreeRTOS* gestiona los tiempos, y el *CAN Bus* transporta las señales. Pero el alma que toma las decisiones que determinan si el monoplaza avanza o se detiene es la **Vehicle Control Logic (VCL)**.

### 5.1 Modelado Matemático: Máquinas de Estado Finito (FSM)

El principal problema del software escrito por desarrolladores novatos es el "Código Espagueti" (*Spaghetti Code*), caracterizado por anidaciones infinitas de bloques `if-else`. En automoción, un flujo basado en if-else es imposible de verificar porque su *Path Coverage* (combinaciones de ramas posibles) tiende a infinito. 

**Implementación de la Teoría de Autómatas (Modelo de Moore):**
Modelamos el controlador maestro de la dinámica del coche (el módulo *R2D Manager* o secuencia *Lucy Init*) basándonos en la teoría de Máquinas de Estado Finito (FSM). Una FSM confina matemáticamente las transiciones del sistema. Si el coche se halla en un estado determinado, ignora categóricamente cualquier *Input* que no posea una flecha de transición válida, actuando como un cortafuegos contra ráfagas electromagnéticas (Glitches) que inyecten falsas pulsaciones de botones.

**Diseño de la Secuencia FSM Estricta:**
1. `STATE_OFF_STANDBY`: Tractive System (TS) inactivo. A la espera de llave.
2. `STATE_TS_ACTIVE`: HV conectado. Los inversores tienen tensión pero están inhibidos.
3. `STATE_WAITING_BRAKE`: Para que el piloto pueda arrancar, exigimos que presione el pedal de freno. Previene el "Launch" involuntario si el pedal del acelerador estaba enganchado físicamente al fondo.
4. `STATE_WAITING_BUTTON`: Con la presión de freno estabilizada, el piloto pulsa el botón del Dashboard. Un algoritmo de *Debounce* (Antirrebote) filtra las falsas lecturas.
5. `STATE_SOUNDING`: Suena la sirena reglamentaria (RTD Sound) de alerta a mecánicos durante 2 segundos cronometrados mediante `vTaskDelayUntil`.
6. `STATE_READY_TO_DRIVE (R2D)`: La VCL transiciona al inversor a *Torque Enable* y acopla los mapas dinámicos. Tracción viva.

> [!TIP]
> **Referencia de Máquinas de Estado (FSM):**
> Esta filosofía se aplica en todo el monoplaza para evitar bloqueos por Glitches. El código de la Máquina de Estados Dinámica (R2D) se encuentra en la MCU en [r2d_manager.c](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW/src/r2d_manager.c). Por otro lado, la máquina de control de Alta Tensión (*Precharge Sequence*) está confinada en la PDM en [app.cpp](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/PDM/PDM_FW/src/app.cpp).

### 5.2 El Límite Absoluto: Reacción Sub-100ms

El diseño de la VCL no es tolerante. Existen condiciones anómalas dictadas por las reglas de Formula Student (ej. *Brake Plausibility Rule*) que exigen una respuesta tajante si el piloto o el sensor reportan discordancias (ej. el acelerador reporta 30% de profundidad, y el circuito hidráulico reporta un frenado duro simultáneo). 

**La Reacción del Software:**
Cuando la tarea de la VCL procesa una discordancia, debe aislar el sistema de tracción abriendo el *Shutdown Circuit* en un tiempo perentorio inferior a 100 milisegundos.
- **Nuestro Factor de Seguridad Térmica (Factor 10x):** Hemos auditado el tiempo de *Round-Trip* de nuestra lógica. La DMA lee el ADC en 1ms; la VCL en el Core 1 detecta la implausibilidad en su siguiente ciclo de reloj y ejecuta el cálculo en <1ms. Se inyecta la trama CAN prioritaria al bus (3ms). El inversor la procesa (5ms). Tiempo total en tirar a 0 el Torque: **<12ms**. 
- Los 88 milisegundos restantes hasta alcanzar el límite legal sirven como colchón masivo para absorber la inercia electromecánica de los relés de alta potencia que tardan decenas de milisegundos físicos en abrir sus contactos magnéticos.

### 5.3 Asimetría de Prioridades (Steering vs MCU)

Una trampa clásica de los jueces es plantear situaciones de fallos estéticos intentando que admitas que tu coche se apagará, perdiendo los valiosos puntos del *Endurance*. 

**Asimetría de Severidad de Diagnóstico:**
Nuestro firmware implementa un tratamiento de fallos altamente clasista.
- **Microcontroladores Críticos (MCU, ECU, PDM):** Toda anomalía es **Prioridad High**. Si un temporizador pierde el *Deadline*, si un *malloc* (si no estuviese prohibido) falla, o si se pierde una conexión de red con el BMS, el sistema asume que el coche está ciego y sordo, deteniendo instantáneamente el suministro de Alto Voltaje por seguridad de vida.
- **Interfaz Hombre-Máquina (Steering Dashboard):** Toda anomalía aquí es **Prioridad Low**. La pantalla TFT del volante renderiza a 60 fps empujando pixeles a través de un bus SPI. Esta tarea es pesada. Si por temperatura ambiental el chip de la pantalla se satura y se congela, dibujando un socavón negro en lugar de los números de velocidad, el sistema lanza un *Log* interno y continúa operando a plena potencia. El coche se rige por su cerebro ciego y sordo a la estética. Paralizar el tren mecánico y destruir nuestra carrera porque el piloto no sabe a qué velocidad va, es un falso positivo que nuestra arquitectura erradica.


---

## Capítulo 6: Verificación, Validación y Clean Code

La mejor arquitectura del mundo es inútil si el código contiene un error tipográfico en un signo negativo. Para defender la base de código (*Codebase*), debemos aportar pruebas certificadas de su integridad matemática.

### 6.1 El Estándar MISRA-C:2012 contra el Comportamiento Indefinido

El lenguaje de programación C/C++ es una espada de doble filo. Es capaz de controlar los registros del silicio a nivel de nanosegundo, pero asume que el programador jamás se equivoca. No avisa si un número rebasa el límite de memoria, simplemente sobrescribe el dato vecino. A esto se le conoce como *Undefined Behavior* (UB), y es el causante del 90% de catástrofes de software embebido.

**Nuestra Aplicación del MISRA-C:**
Integramos la herramienta `cppcheck` en el flujo de integración continua, configurada con el estándar aeroespacial MISRA-C:2012.
1. **Regla 21.3 (Prohibición Dinámica):** Ya abordada. El `cppcheck` rastrea todo el árbol en busca de un solo `malloc` y lo reporta como fallo de seguridad crítico, abortando la compilación.
2. **Fuertemente Tipado Estático:** Exigimos el uso de bibliotecas de ancho fijo (`<stdint.h>`). Una variable para temperatura jamás se define como `int temp;` (cuyo tamaño varía dependiendo del procesador), sino que debe definirse explícitamente como `int16_t temp;`, asegurando que siempre tendrá 16 bits y jamás provocará un *Buffer Overflow* imprevisto en memorias compactas.
3. **Bloaters y Complejidad Ciclomática:** Eliminamos el código inalcanzable (*Dead Code*). Además, limitamos las anidaciones complejas de bucles y la excesiva Programación Orientada a Objetos (Virtual Tables / Polimorfismos dinámicos), dado que complican predecir exactamente el tiempo que tardará una función en terminar de ejecutarse (*Worst-Case Execution Time* - WCET).

### 6.2 Unit Testing y el Muro del 90% Code Coverage

El TVSD especifica que debemos apuntar a un **90% de Cobertura de Código**.
Defender un 90% en software embebido de automoción frente a un juez hostil requiere un enfoque pragmático y honesto. Intentar simular el 100% de los drivers de interrupción de hardware del proveedor (*Hardware Abstraction Layer* - HAL) es una mentira estadística en entornos de CI sin laboratorios robóticos *Hardware-In-the-Loop* (HIL).

**Defensa de la Cobertura (VCL Target):**
Nuestra argumentación afirma que el objetivo de cobertura del 90% aplica con rigor obsesivo a nuestra **Vehicle Control Logic (VCL)**, excluyendo bibliotecas comerciales certificadas (`esp_wifi.h`, `FreeRTOS.h`).

**Arquitectura de Testing Unitario:**
Utilizamos el framework `Unity` (de ThrowTheSwitch) en un entorno nativo simulado (x86).
- Aislamos funciones matemáticas críticas, como por ejemplo `mosfet_driver_check_fault()` de la PDM, encargada del filtrado anti-picos de corriente (Inrush limits).
- Inyectamos valores anómalos usando bucles en el código del test. Sometemos el filtro Inrush a corrientes de 900A, verificamos el contador interno ciclo a ciclo simulando la latencia del hardware, y aseguramos que el sistema "rompe" exactamente en el quinto ciclo usando la aserción macro `TEST_ASSERT_TRUE`.
- En otro módulo, atacamos el algoritmo de *Brake Plausibility*. Comprobamos la interpolación de los mapas de torque, forzando cortes de cable, cortocircuitos a masa y ruidos aleatorios. 
- Solo cuando cada rincón lógico (Path) y límite matemático (Boundary Values) de la MCU responde con la mitigación correcta, el robot automático da el visto bueno al despliegue OTA del firmware.

> [!TIP]
> **Referencia de Testing Unitario Avanzado (Unity):**
> Para demostrar cómo inyectamos fallos eléctricos simulados a la lógica del coche de forma determinista en el servidor de Integración Continua, referenciar las aserciónes matemáticas en los archivos de test principales: [test_main.cpp de la MCU](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/MCU/MCU_FW/test/test_main.cpp) y [test_main.cpp de la PDM](file:///c:/Users/DBDVU0X/DYN-Vehicle-Firmware/PDM/PDM_FW/test/test_main.cpp).


---

## Capítulo 7: Arsenal de Defensa de Diseño (Q&A Nivel Juez)

Este bloque proporciona argumentaciones blindadas para defender la tesis frente a los ataques directos de los Jueces en el Design Event. Cada respuesta es un resumen ejecutivo de las decenas de páginas teóricas desarrolladas en los capítulos anteriores.

### 7.1 Carga Algorítmica y Overhead (FreeRTOS)
**Q1: "Afirmáis tener un determinismo crítico, pero un RTOS añade *Context Switching Overhead*. ¿Acaso un Super-Loop puramente en hardware (Bare-Metal) no sería más rápido?"**
> **Respuesta Oficial:** Un Super-Loop Bare-Metal es marginalmente más rápido en vacío, pero carece de *Preemption*. Si la rutina de parseo de datos de telemetría de 2 kilobytes bloquea la CPU durante 15ms, el cálculo de freno de emergencia sufre una latencia de 15ms. Inaceptable. FreeRTOS asume un pequeño overhead computacional (~2 microsegundos), pero provee control expropiativo basado en prioridades. La tarea de seguridad detiene, expropia y relega a la telemetría en el nanosegundo que llega su ISR, garantizando latencias predecibles inferiores a 12ms pase lo que pase.

### 7.2 Elección del SoC y Core Pinning
**Q2: "Usar un ESP32 es típico de proyectos aficionados del IoT. En la industria del motor manda Infineon AURIX o STM32 ASIL. ¿Os importa tan poco la seguridad estructural?"**
> **Respuesta Oficial:** Validamos componentes frente a las dinámicas del vehículo, no por prestigio de marca. La familia ESP fue seleccionada porque su topología Xtensa Dual-Core asimétrica aborda nativamente nuestro mayor desafío: La Concurrencia de Redes vs. Tracción Real-Time. Al emplear *Core Pinning*, atrincheramos todo el driver del Bus CAN y los controladores cinemáticos matemáticos en el Core 1 (VCL), desterrando el voluminoso stack Wi-Fi LwIP, el Servidor OTA HTTP y la telemetría al Core 0. Un microcontrolador comercial monocore requeriría técnicas de multiplexación de hardware fragilísimas para simular lo que nosotros obtenemos de base a nivel de silicio asimétrico.

### 7.3 Arquitectura de Memoria (El Terror del Heap)
**Q3: "Leo en el TVSD la erradicación completa de la memoria dinámica. Si no usáis malloc, y las colas IPC (Inter-Procesos) acarrean datos pesados, causaréis un Kernel Panic por desbordamiento de Buffer."**
> **Respuesta Oficial:** Esa asunción asimila memorias estáticas a buffer descontrolado. No instanciamos buffers desnudos, orquestamos mediante la API Thread-Safe `xQueueCreateStatic`. Reservamos el `.bss` completo (*Worst-Case Execution Memory*) al arrancar. Cuando una anomalía inunda una cola (Ráfaga Asíncrona de errores del Inversor), el RTOS entra en modo mitigación por descarte. En telemetría aplicamos *Drop-Oldest*, protegiendo la barrera de RAM a la vez que inyectamos el primer error fresco a la FSM para actuar en milisegundos.

### 7.4 Tolerancias Asimétricas y Steering
**Q4: "¿Qué ocurre con el vehículo si el módulo gráfico del Steering (Volante) que renderiza a 60fps falla, la RAM gráfica del TFT colapsa, y el piloto no puede ver nada en la pantalla?"**
> **Respuesta Oficial:** Absolutamente nada. Sigue compitiendo. En el diseño de automoción hay que desvincular la estética de la cinética. Implementamos una Asimetría de Prioridades. Un *Timing Deadline Miss* o *Resources Error* en la HMI (Dashboard) se cataloga de prioridad baja (*Low*). El coche se gobierna íntegramente por su VCL ciego en el Core 1. Acelera y mitiga en red CAN pura. Apagar el coche y perder los 300 puntos de *Endurance* por un píxel congelado es ingeniería negligente y asustadiza. Nuestro diseño tolera amputaciones cosméticas sin sacrificar la seguridad de vida del tractive system.

### 7.5 Validación Continua y Regresiones Rápidas
**Q5: "Si detectáis que la constante del PID del control de torque del inversor es inestable a 5 minutos del cierre del box, ¿cómo podéis auditar el código tan rápido y estar seguros de no empeorar el problema?"**
> **Respuesta Oficial:** Combinando OTA + CI/CD + Kanban. Editamos el factor, comiteamos, y empujamos. En 10 segundos, nuestro CI cross-compila, pasa Cppcheck para MISRA, y bombardea el código con 30 Test Unitarios extremos inyectados en Unity validando la nueva variable. Si el CI lo aprueba, pulsamos desplegar vía OTA. El ESP32 embebido levanta su doble partición paralela y en 5 segundos estamos flasheados. Reducimos el riesgo del error humano obligando a un robot determinista en la nube a certificar cada byte. Nuestro Cycle-Time de regresión es el más implacable de la parrilla.

