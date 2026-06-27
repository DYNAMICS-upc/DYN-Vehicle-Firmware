# Biblia de Defensa de Software - TVSD Formula Student

Este documento es la referencia definitiva para comprender, justificar y defender ante cualquier juez de Formula Student la arquitectura de software de nuestro vehículo. Está diseñado para que cualquier integrante del equipo, independientemente de su nivel inicial, pueda asimilar los conceptos desde su base hasta llegar a la justificación técnica de nivel ingeniero.

---

## Capítulo 1: Fundamentos y Metodología del Equipo

### 1.1 El Entorno y la Metodología: ¿Por qué Kanban?

**Para el Novato:**
Imagina que estás construyendo un coche y tienes un plan detallado de qué hacer cada semana (Waterfall) o cada dos semanas (Scrum). De repente, en medio de la semana, el equipo de Aerodinámica cambia un sensor y tú necesitas reprogramarlo urgentemente. Si usas Waterfall, tu plan se rompe. Si usas Scrum, tienes que esperar a la próxima "sprint" (ciclo de dos semanas) para meter esa tarea. Sin embargo, con Kanban, tienes una pizarra con tareas en columnas ("Por hacer", "En progreso", "Hecho"). Simplemente coges la tarea urgente, la pones en "En progreso" y te pones a ello. Es un flujo continuo.

**Para el Ingeniero (Justificación y Trade-offs):**
En un entorno de Motorsport universitario, la volatilidad de los requisitos es extrema. Evaluamos tres paradigmas de gestión: Waterfall (puntuación 3.4), Scrum (7.8) y Kanban (8.6).
- *Rechazo de Waterfall:* Su rigidez asume que los requisitos de hardware no cambiarán durante el año. En FS, esto es falso; un cambio mecánico de última hora invalida el diseño en cascada.
- *Rechazo de Scrum:* Aunque iterativo, Scrum bloquea el "Sprint Backlog" durante 1-4 semanas. Si el inversor se quema a mitad de sprint y hay que reescribir la lógica de mitigación de picos, el framework Scrum colapsa.
- *Elección de Kanban:* Permite limitar el *Work In Progress (WIP)* para evitar saturar a los desarrolladores, a la vez que permite re-priorizar la cola del *Backlog* en tiempo real. Esto garantiza un **tiempo de ciclo (cycle time)** hiperreducido frente a anomalías de hardware repentinas, otorgándonos la máxima agilidad (adaptabilidad a cambios súbitos).

### 1.2 El Ciclo de Vida del Código: Integración y Despliegue Continuo (CI/CD)

**Para el Novato:**
Imagina que el código del coche es una receta de cocina muy compleja. Si alguien añade sal, no queremos probar el plato directamente en el jurado porque podría estar salado. El CI/CD es un "robot probador" automático. Cada vez que alguien hace un cambio (*Commit*), el robot prepara la receta (*Build*) y la prueba (*Tests*). Solo si el robot dice que está perfecta, la receta se manda mágicamente por Wi-Fi al coche (*Deploy*).

**Para el Ingeniero (Justificación y Trade-offs):**
Nuestro pipeline en GitHub Actions no es un lujo, es la red de seguridad de nuestra arquitectura. Se estructura de la siguiente manera:
`Commit Changes -> Trigger Build -> Build -> Report Build Outcome -> Run Tests -> Report Test Outcome -> Deliver Build -> Deploy Build (OTA Wi-Fi)`

El trade-off clásico de implementar CI/CD es el tiempo que se invierte en configurarlo frente al tiempo ahorrado en debugging. En automoción, un bug que llega al hardware puede significar un cortocircuito, fuego o un accidente. Por lo tanto, el ROI (Retorno de Inversión) del CI/CD es infinito. 

La Ley Fundamental de nuestro repositorio es: **"Only verified code is deployed to the vehicle"**.
El despliegue OTA (Over-The-Air) por Wi-Fi permite flashear la MCU, PDM y ECU directamente desde el portátil al router del box y de ahí al coche, sin tener que desmontar el monocasco para acceder a los puertos USB. Esto reduce el tiempo de iteración en pista de minutos a segundos.

---

## Capítulo 2: La Arquitectura del Sistema (El Hardware y el OS)

### 2.1 El Cerebro: ESP32 y FreeRTOS

**Para el Novato:**
El coche necesita un cerebro electrónico. Podríamos usar Arduino, pero Arduino solo puede hacer una cosa a la vez (como leer un sensor, y luego encender una luz). Nosotros usamos el ESP32, que tiene dos "cerebros" (núcleos) trabajando a la vez. Para organizarlos, usamos un director de orquesta llamado FreeRTOS, que decide qué tarea es más importante (frenar) y cuál puede esperar (encender un LED).

**Para el Ingeniero (Justificación y Trade-offs):**
La arquitectura electrónica pivota sobre microcontroladores **ESP32** ejecutando el RTOS de código abierto **FreeRTOS**.
- *Hardware (ESP32):* Frente a alternativas como STM32, el ESP32 nos ofrece arquitectura dual-core de 240 MHz a muy bajo coste, con periféricos de red (Wi-Fi/Bluetooth) integrados en el propio silicio, lo que posibilita nuestra OTA. Es compatible con toolchains modernos de C/C++ (ESP-IDF).
- *OS (FreeRTOS):* Elegimos FreeRTOS por encima de esquemas *Bare-Metal* (Super-Loop) o RTOS privativos (VxWorks). El Bare-Metal sufre para mantener el determinismo cuando se mezclan tareas pesadas (matemáticas de Torque) con interrupciones de red. FreeRTOS nos provee de *preemption* (las tareas críticas interrumpen a las de baja prioridad al instante) y cuenta con una vasta comunidad activa que acelera el tiempo de desarrollo.

### 2.2 Los 3 Pilares de Robustez

La fiabilidad del coche descansa sobre tres pilares inquebrantables. Violar uno de ellos es corromper la seguridad estructural del firmware.

#### Pilar 1: Determinismo Temporal
**Para el Novato:** Si pisas el freno en tu coche de calle y este tarda 2 segundos en reaccionar, te estrellas. El determinismo significa que el coche garantiza reaccionar a una orden *exactamente* en el mismo tiempo, siempre, sin sorpresas.
**Para el Ingeniero:** Nuestro vehículo exige sistemas deterministas de tiempo real. Diseñamos nuestras máquinas de estado (ej. la secuencia de arranque o *Lucy Init*) de manera estrictamente secuencial (`Input -> State 1 -> State 2 -> State n -> Output`). La MCU tiene un umbral rígido: **debe responder en < 100 ms**. Todo error interno de la MCU (Hardware, Comms, Resources, Timing) desencadena una alerta de prioridad *High*. El determinismo salva vidas cortando la tracción milisegundos antes de que un fallo derive en una catástrofe termodinámica.

#### Pilar 2: Gestión Estática (La cruzada contra la Memoria Dinámica)
**Para el Novato:** Imagina que tienes una biblioteca (Memoria RAM) donde la gente saca y mete libros todo el rato de diferente tamaño (Memoria Dinámica). Al final del día, los huecos que quedan están tan desordenados que no cabe un libro grande, aunque sumando los huecos pequeños sí habría espacio. Eso bloquea la biblioteca. En nuestro coche, compramos estanterías a medida desde el principio (Memoria Estática) y nunca cambiamos su tamaño. Así nunca nos quedamos sin espacio.
**Para el Ingeniero:** Hemos instaurado una política de **Cero Asignación Dinámica**. El uso de `malloc()`, `free()`, o la creación dinámica de tareas y colas en FreeRTOS (`xQueueCreate`, `xTaskCreate`) está terminantemente **prohibido**. Todo se provisiona en tiempo de compilación empleando las API estáticas (`xQueueCreateStatic`, `xTaskCreateStatic`). El trade-off es que perdemos flexibilidad para redimensionar memoria en ejecución y consumimos el máximo de RAM desde el byte 0, pero ganamos inmunidad absoluta contra la fragmentación del *Heap* y el temido *Out-Of-Memory (OOM) Panic*. Las tareas son persistentes en bucles `while(true)`.

#### Pilar 3: Clean-Code y Testing
**Para el Novato:** El código limpio es aquel que puede leer un ingeniero que no estuvo en el equipo cuando se escribió, y entenderlo a la primera.
**Para el Ingeniero:** Nuestro código se somete a los estándares aeronáuticos **MISRA-C**. Promovemos el *Zero Dead Code/Bloaters* y rechazamos la Programación Orientada a Objetos (OOP) innecesaria, priorizando un enfoque procedimental/estructural más predecible en C. En el CI/CD exigimos un **90% de Code Coverage**, validando agresivamente la Lógica de Control del Vehículo. El flujo es estricto e innegociable: `Push -> Code Review -> Aprobación del Project Manager (PM) -> Merge a Main`. Sin review manual + checks automáticos verdes, el código jamás pisa la MCU.
