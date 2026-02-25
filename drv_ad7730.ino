
// ============================================================================
// DRIVER AD7730 - Amplificador de celda de carga de 24 bits con SPI
// ============================================================================

// Constantes de configuracion
#define AD7730_SPI_CLOCK 2000000       // 2 MHz SPI clock
#define AD7730_TIMEOUT 0x4ffff         // Timeout para esperas
#define AD7730_SHIFT_BITS 6            // Bits a descartar (quedar con 18 bits utiles)
#define AD7730_18BIT_MASK (1UL << 17U) // Bit de signo para 18 bits
#define AD7730_18BIT_RANGE (1L << 18)  // Rango de 18 bits con signo
#define AD7730_CMD_READ_FILTER 0x13
#define AD7730_CMD_READ_DAC 0x14
#define AD7730_CMD_SELECT_SINGLE_READ_DAC 0x11
#define AD7730_GLITCH_THRESHOLD 8000   // Umbral para detectar picos

// Variables globales
volatile long old_ad7730_dataconvert = 0;
volatile int state = LOW;

// Configuracion del SPI
static SPISettings spi_ad7730(AD7730_SPI_CLOCK, MSBFIRST, SPI_MODE1);

// Envia datos por SPI con manejo automatico de CS
static void ad7730_spiTransfer(const uint8_t *data, uint8_t len, bool readback)
{
  noInterrupts();
  SPI.beginTransaction(spi_ad7730);
  CS_AD7730_LOW();
  for (uint8_t i = 0; i < len; i++) {
    SPI.transfer(readback ? 0 : data[i]);
  }
  CS_AD7730_HIGH();
  SPI.endTransaction();
  interrupts();
}

static void ad7730_sendByte(uint8_t toSend)
{
  ad7730_spiTransfer(&toSend, 1, false);
}

static void ad7730_send2Bytes(uint8_t first, uint8_t second)
{
  uint8_t data[2] = {first, second};
  ad7730_spiTransfer(data, 2, false);
}

static void ad7730_send3Bytes(uint8_t first, uint8_t second, uint8_t third)
{
  uint8_t data[3] = {first, second, third};
  ad7730_spiTransfer(data, 3, false);
}

// Espera a que el AD7730 este listo (señal RDY baja)
// Retorna true si estuvo listo antes del timeout, false si timeout
static bool waitForReady()
{
  int32_t timeout = AD7730_TIMEOUT;
  while (AD7730_RDY() == 1 && timeout--) {
    // Espera activa
  }
  return (timeout > 0);
}

void AD7730_Reset(void)
{
  RESET_AD7730_HIGH();
  delay(200);
  RESET_AD7730_LOW();
  delay(200);
  RESET_AD7730_HIGH();
  delay(200);
}

// Inicializa y calibra el AD7730
// Configura datarate segun vg.datarate_celula, modo continuo, calibracion interna
void AD730_Inic(void)
{
  AD7730_Reset();

  // ===== Configuracion del Filtro =====
  ad7730_sendByte(CR_SINGLE_WRITE | CR_FILTER_REGISTER);

  // Seleccionar datarate segun configuracion de celula (normal: 300 Hz)
  // Alternativas: 150, 300, 600, 800, 1000, 1200 Hz

  if (vg.datarate_celula == 150)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_150, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else if (vg.datarate_celula == 300)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_300, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else if (vg.datarate_celula == 600)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_600, FR1_SKIP_ON | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else if (vg.datarate_celula == 800)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_800, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else if (vg.datarate_celula == 1000)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_1000, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else if (vg.datarate_celula == 1200)
  {
    ad7730_send3Bytes(FR2_DATA_RATE_1200, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }
  else
  {
    ad7730_send3Bytes(FR2_DATA_RATE_150, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF);
  }

  delay(30);

  // ===== Configuracion DAC =====
  ad7730_sendByte(CR_SINGLE_WRITE | CR_DAC_REGISTER);
  ad7730_sendByte(DACR_OFFSET_SIGN_POSITIVE | DACR_OFFSET_NONE);

  delay(30);

  // ===== Calibracion Interna Zero-Scale =====

  ad7730_sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
  ad7730_send2Bytes(MR1_MODE_INTERNAL_ZERO_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS);
  waitForReady();

  // ===== Calibracion Interna Full-Scale =====
  ad7730_sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
  ad7730_send2Bytes(MR1_MODE_INTERNAL_FULL_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS);
  waitForReady();

  // ===== Calibracion Sistema Zero-Scale =====
  ad7730_sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
  ad7730_send2Bytes(MR1_MODE_SYSTEM_ZERO_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS);
  waitForReady();

  // ===== Modo Continuo (operacion normal) =====
  ad7730_sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
  ad7730_send2Bytes(MR1_MODE_CONTINUOUS | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS);
  waitForReady();
}

uint8_t AD730_ReadDACReg(void)
{
  ad7730_sendByte(AD7730_CMD_READ_DAC);
  noInterrupts();
  SPI.beginTransaction(spi_ad7730);
  CS_AD7730_LOW();
  uint8_t dat = SPI.transfer(0);
  CS_AD7730_HIGH();
  SPI.endTransaction();
  interrupts();
  return dat;
}

long AD730_ReadFilterReg(void)
{
  
  ad7730_sendByte(AD7730_CMD_READ_FILTER);
  noInterrupts();
  SPI.beginTransaction(spi_ad7730);
  CS_AD7730_LOW();
  uint8_t result1 = SPI.transfer(0); // byte mas significativo
  uint8_t result2 = SPI.transfer(0);
  uint8_t result3 = SPI.transfer(0); // byte menos significativo
  CS_AD7730_HIGH();
  SPI.endTransaction();
  interrupts();
  
  long result = result3 + (result2 << 8) + (result1 << 16);
  return result;
}

// Lee datos de conversion continua del AD7730 (3 bytes de 24 bits)
// Convierte a 18 bits con signo (two's complement) eliminando LSBs de ruido
int32_t AD730_ReadConversionDataContinua(void)
{
  // Esperamos si el ADC no esta configurado
  if (!AD730_CONFIGURADO) {
    if (!waitForReady()) {
      // Timeout: retornar ultimo valor conocido
      return old_ad7730_dataconvert;
    }
  }

  // Lectura de 3 bytes de datos (24 bits brutos)
  noInterrupts();
  SPI.beginTransaction(spi_ad7730);
  CS_AD7730_LOW();
  uint8_t result1 = SPI.transfer(0); // byte mas significativo
  uint8_t result2 = SPI.transfer(0);
  uint8_t result3 = SPI.transfer(0); // byte menos significativo
  CS_AD7730_HIGH();
  SPI.endTransaction();
  interrupts();
  // Seleccionar modo single read DAC
  ad7730_sendByte(AD7730_CMD_SELECT_SINGLE_READ_DAC);

 // 1. Reconstrucción correcta en 32 bits (sin desbordamiento)
  // Usamos OR (|) en lugar de SUMA (+) para evitar problemas de signo intermedio
  uint32_t rawData = ((uint32_t)result1 << 16) | ((uint32_t)result2 << 8) | (uint32_t)result3;

  // 2. Convertir de Offset Binary a Two's Complement (Solo si el ADC está en modo Bipolar)
  // Esto hace que 0x800000 (0V) pase a ser 0x000000
  rawData ^= 0x800000;

  // 3. Ahora manejamos el signo para extenderlo a 32 bits (Sign Extension)
  int32_t lDat;
  if (rawData & 0x800000) {
    lDat = (int32_t)(rawData | 0xFF000000); // Es negativo, rellenamos con FFs
  } else {
    lDat = (int32_t)rawData; // Es positivo
  }

  // 4. Aplicar el shift si deseas reducir resolución/ruido
  // Si desplazas un número negativo a la derecha, el compilador C++ 
  // mantiene el signo (Arithmetic Shift), lo cual es correcto.
  lDat = lDat >> AD7730_SHIFT_BITS;
  return lDat;
}

// Lee datos del ADC con filtrado de picos (glitches)
// Si detecta un cambio > threshold en un ciclo, rechaza lectura hasta 5 intentos
long AD7730_leer_adlc()
{
  long dataconvert = AD730_ReadConversionDataContinua();
  static int rejectionCounter = 0;
  
  // Calcular diferencia con ultima lectura valida
  long difference = abs(dataconvert) - abs(old_ad7730_dataconvert);
  difference = abs(difference);

  // Detectar y rechazar picos (glitches > threshold)
  if (difference > AD7730_GLITCH_THRESHOLD) {
    rejectionCounter++;
    if (rejectionCounter > 5) {
      rejectionCounter = 0;
      old_ad7730_dataconvert = dataconvert; // Aceptar tras 5+ rechazos consecutivos
    } else {
      dataconvert = old_ad7730_dataconvert; // Rechazar, usar ultima valida
    }
  } else {
    rejectionCounter = 0;
    old_ad7730_dataconvert = dataconvert;
  }

  return dataconvert;
}

// Servicio de interrupcion para lectura del ADC cuando RDY baja
// Mide tiempo en PIN_TEST_DEBUG para analisis de timing
void ISR_RDY_ADC7730()
{
  TDatoCanal datoCanalCelula;
  static unsigned long secuencia = 0;

  digitalWrite(PIN_TEST_DEBUG, HIGH); // Marcador de inicio ISR
  
  if (!AD730_CONFIGURADO) {
    return; // ADC no configurado, ignorar
  }

  // Lectura con glitch filtering
  datoCanalCelula.dato = AD7730_leer_adlc();
  datoCanalCelula.t_ms = millis();
  datoCanalCelula.secuencia = secuencia++;
  
  // Almacenar en buffer circular
  store_char(&datoCanalCelula, pdatos_buffer);

  digitalWrite(PIN_TEST_DEBUG, LOW); // Marcador de fin ISR
}
