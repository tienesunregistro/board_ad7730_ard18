# board_ad7730_ard18
Placa de sensores para Arduino uno, AD7730 (ADC 18 bits) y lectura de encoder LS7366R

 ARD18-V1 | Controlador de Célula de Carga y Encoder

Este proyecto consiste en un firmware. Aquí tienes un fichero **README.md (Kernel 3.0) para la placa electrónica **ARD18-V1** basada en Arduino. Está diseñado para aplicaciones** profesionales y detallado, diseñado específicamente para este proyecto. Incluye la arquitectura técnica, la configuración de comunicaciones y el diccionario industriales de ensayo de materiales, control de fuerza y medición de extensión.

## 🚀 Características Principales



# ARD18-V1 Force & Extension Kernel v3.0

Este proyecto contiene el firmware (Kernel) para la placa electrónica **ARD18-V1**, diseñada para aplicaciones de pesaje industrial y ensayos de materiales. El sistema integra una lectura de alta precisión de células de carga (vquisición de Fuerza:** Gestión del conversor AD7730 de 24 bits con filtrado digital avanzado (SMA/RC) y gestión de picos máximos en tiempo real.
- **Medición de Extensión:** Interfaz con elía AD7730) y una lectura de posición mediante encoder (vía LS7366R).

 chip LS7366 para lectura de encoders en cuadratura (X1, X2, X4).
- **Control de Alarmas:** Salidas digitales para sobrecarga positiva y negativa con histéresis programable.
-## 🚀 Características Principales
- **ADC de alta resolución:** Soporte para AD7730 de 24 bits **Gestión de Memoria:** Almacenamiento persistente de calibraciones y parámetros en EEPROM con verificación de integridad mediante Check (muestreo por interrupción).
- **Control de Encoder:** Interfaz con LS7366R para conteo de cuadratura de 32 bits.
- **Buffer Circular:** Implementación optimizada para captura de datos ensum.
- **Kernel Multi-célula:** Capacidad para identificar y autoconfigurar hasta 5 tiempo real.
- **Filtrado Avanzado:** Incluye filtros SMA (Media Móvil) y EMA (Ex células de carga diferentes mediante ID analógico.

---

## 🛠 Configuración RS232

La comunicación seponencial) configurables.
- **Gestión de Alarmas:** Control de sobrecarga positiva/negativa con hist realiza a través del puerto serie principal con los siguientes parámetros:

| Parámetro | Valor |
| :--- |éresis.
- **Memoria No Volátil:** Almacenamiento de calibraciones y parámetros en EEPROM con :--- |
| **Baud Rate** | 38400 bps |
| **Data Bits** | 8 |
| **Parity** | None |
| **Stop Bits** | 1 |
| ** verificación de integridad (Checksum).

---

## 🛠 Configuración de Comunicación (RS232)

LaFlow Control** | None |

---

## 📑 Protocolo de Comandos

Los comandos siguen una estructura de placa utiliza un puerto serie físico (UART) para el control y la transmisión de datos.

- **Baudrate texto plano (ASCII) con marcadores de inicio y fin:

**Formato:** `:ID_ESTACION|:** 38400 bps
- **Data bits:** 8
- **Parity:** None
-COMANDO|PARAM1|PARAM2|...[CR]`

- `:` : Carácter de inicio.
- `ID **Stop bits:** 1
- **Flow Control:** None

### Estructura del Protocolo
Los comandos_ESTACION` : ID del dispositivo (definido en EEPROM, por defecto `1`).
- `| deben seguir el siguiente formato:
`:[ID_ESTACION]|[COMANDO]|[PARAMETRO_1]|[PARAMETRO_` : Separador de campos.
- `[CR]` : Retorno de carro (`0x0DN][CR]`

- **Inicio de comando:** `:` (0x3A)
- **Separador de campos:**` o `\r`) para finalizar el comando.

---

## 🕹 Lista de Comandos

### 1 `|` (0x7C)
- **Fin de comando:** `CR` (Carriage Return, . Comandos de Lectura (Read)

| Comando | Descripción | Respuesta Ejemplo |
| :--- | :---0x0D)

---

## 📑 Diccionario de Comandos

### Comandos de Lectura (Read)

| Comando | Descripción | Respuesta |
| :--- | :--- | :--- |
| **RI | :--- |
| `RI` | Información del sistema (Placa, Máquina, Kernel) | `ARD18-V1 / GENERIC / KERNEL 3.0` |
| `R1` | Lect** | Información de la placa y versión de Kernel | `ARD18-V1 / GENERIC / KERNEL-ura de fuerza actual (Newton) | `125.450` |
| `R2` | Lectura3.0` |
| **R1** | Lectura de fuerza actual (Newton) | `[valor de extensión actual (mm) | `10.025` |
| `R3` | Lect_fuerza]` (ej: 125.305) |
| **R2**ura de fuerza bruta (Pasos del ADC) | `123350` |
| `R4 | Lectura de extensión/posición (mm) | `[valor_extension]` |
| **R3** | Lect` | Lectura continua (Fuerza y Extensión) | `125.45\|10.0ura de fuerza bruta en pasos del ADC | `[pasos_long]` |
| **R4** | Salida25` |
| `RA` | Pico máximo de fuerza (Autoreseteable) | `50 continua de datos (Fuerza \| Extensión) | `[Fuerza]\|[Extensión]` (o0.200` |
| `RB` | Pico máximo de tramo (Autoreseteable) Binario si se activa) |
| **RA** | Pico máximo de fuerza (autoreseteable) | `[valor | `450.120` |
| `RC` | Pico máximo de tramo filtrado_max]` |
| **RB** | Pico máximo de fuerza en tramo actual | `[valor_max_tram | `448.900` |
| `RS` | Estado de alarmas (bitmask) | o]` |
| **RC** | Pico máximo de fuerza en tramo filtrado | `[valor_max`0` (Sin alarmas) |
| `RX` | Identificar código de célula conectada | `2_tramo_f]` |
| **RS** | Estado de alarmas (bitmask) | `[byte` |
| `RP\|9` | Leer estado del filtro (0: OFF, 1: ON) |_alarmas]` (0=OK, 1=Sobrecarga, etc.) |
| **RX** | `1` |
| `RP\|13`| Leer parámetros de la célula activa en RAM | `id\| Identificar código de célula conectada (A0) | `[id_celula_0-4]` |
| **cap\|pol\|res\|limP\|limN...` |

### 2. Comandos de Escritura (Write)

| Comando | Descripción | Parámetros |
| :--- | :--- | :--- |
|RP\|9** | Consultar estado del filtro (1: ON, 0: OFF) | `1` o `0 `WZ` | Cero de fuerza (Tara) | N/A |
| `WE` | Cero de` |
| **RP\|13** | Leer parámetros de calibración actuales en RAM | `ID\|CAP extensión | N/A |
| `WY` | Reset de hardware del ADC AD7730 | N/A\|POL\|RES\|LIM_P\|LIM_N\|...` |
| **RP\|99**| |
| `WI` | Escritura en pin digital | `WI\|Pin\|Estado(0-1)` |
 Test de memoria RAM libre | `Memoria RAM: [bytes]` |

### Comandos de Escritura y Acción (Write)| `WP\|0` | **Salvar cambios en EEPROM** | N/A |
| `WP\|9

| Comando | Descripción | Parámetros / Notas |
| :--- | :--- | :--- |
| **WZ` | Configurar filtro ON/OFF | `WP\|9\|1` (Activar) |
| `WP\|** | Cero de fuerza (Tara) | Resetea picos y captura offset actual. |
| **WE**14`| Configurar paso de husillo (mm) | `WP\|14\|5.0` |
| | Cero de extensión | Resetea el contador del encoder a la posición de inicio. |
| **WY** | Reset `WP\|15`| Configurar pulsos del encoder | `WP\|15\|1000 Hardware del ADC | Reinicializa y calibra el AD7730. |
| **WI\|` |
| `WP\|19`| Configurar datos de célula en EEPROM | `WP\|19\|idx\|cap\|pol\|res...` |
| `WP\|999`| **Reset de fábrica (Defaults)** | N/A |

---

## ⚠️ Alarmas y Seguridad

El sistema monitoriza constantemente los límites de carga definidos para la célula activa:
- **Alarma Fuerza Positiva:** Se activa si `FP\|V** | Escritura en pines I/O auxiliares | `P`: Pin, `V`: Estado (0 o 1). |
| **WP\|0** | **Salvar en EEPROM** | Guarda la configuración de RAM a EEPROM permanentemente. |
| **WP\|9\|V** | Activar/Desactivuerza > Limite_Carga_Pos`.
- **Alarma Fuerza Negativa:** Se activa si `absar Filtro digital | `V=1` (Activo), `V=0` (Passthrough). |
|(Fuerza) > Limite_Carga_Neg`.
- **Histéresis:** Las alarmas tienen un **WP\|14\|V**| Configurar paso de husillo | `V`: Valor en mm. |
| ** factor de histéresis del 5% para evitar oscilaciones en el relé de seguridad.

---

WP\|19\|...**| Configurar celda de carga | `ID\|Cap\|Pol\|Res\|LimP\|## 📂 Estructura del Proyecto

- `main.ino`: Bucle principal y despachador de comandosLimN\|GainP\|GainN\|Rate` |
| **WP\|21\|V**| In.
- `buffer.cpp/h`: Gestión del buffer circular optimizado para interrupciones.
- `driver_iciar/Parar transmisión continua | `V=1` (Start), `V=0` (Stop). |
|ad7730.cpp/h`: Comunicación SPI y gestión del ADC de 24 bits.
- `driver **WP\|999** | **Factory Reset** | Carga valores por defecto y formatea EEPROM._ls7366.cpp/h`: Control del contador de encoder.
- `eeprom_manager.cpp |

---

## 🎛 Alarmas y Seguridad
El sistema monitoriza constantemente los límites de carga defin/h`: Persistencia de datos con validación Checksum.

---

## 🛠 Instalación y Compilación

1.idos para cada célula:
- **Alarma Fuerza Positiva:** Activa `PIN_ALARMA_FUERZA Clonar el repositorio.
2. Asegurarse de tener instaladas las librerías `SPI` y _POST` si se excede `limite_carga_celP`.
- **Alarma Fuerza Negativa:**`EEPROM` (estándar de Arduino).
3. Configurar la placa en el IDE de Arduino como ** Activa `PIN_ALARMA_FUERZA_NEG` si se excede `limite_carga_Arduino Nano** o **Arduino Uno** (ATmega328P).
4. Compilar y subir.celN`.
- **Histéresis:** Las alarmas se desactivan automáticamente cuando la fuerza baja del 5% del

---
*Desarrollado para la placa ARD18-V1 - Versión Kernel 3.0*