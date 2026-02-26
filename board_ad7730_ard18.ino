/*
Placa electrónica: ARD18-V1

Kernel basado en  1.03 y 2.1, se hace una revisión general del código, optimizando
el interface SPI con los dispositivos. Se añade un parametro de datarate para cada célula
y se cambia la opertiva del las funciones de RS232, optimizando la búsqueda
de parámetros. Es un kernel genérico.

Las lecturas del ADLC se hacen por interrupción y se salvan a un buffer.

eL bucle tiene un tiempo de ejecución de 12(us), con tiempo de ciclo de 18 (us).
*/
/*
Revisiones:
1.0 - Inicial, fecha 04-06-2020


*/
#include <SoftwareSerial.h>
#include <SPI.h>
#include "driver_ad7730.h"

#define IDMAQ "GENERIC"
#define IDBOARD "ARD18-V1-2020-v1.0"
#define SKERNEL "KERNEL - 3.1 - 2026-02-27"
#define CLAVE_VERSION 303

#define ID_ESTACION 1
#define ID_MAXIMO_CELULA 5   // CELULAS DE LA 0..4
#define BAUDRATE_RS232 38400 // 115200
#define ADC_USE_INTERRUPT 1

#define LF 0x0a // 10
#define CR 0x0d // 13

// Caracteres de inicio y fin del comando
#define CHAR_INICIO_CMD ':'
#define CHAR_FIN_CMD CR

#define TIMEOUT_CHECK_ALARMAS 200 // ms

// Prototipos de funciones de otros ficheros .ino
int procesarComando(char *cmdStr);
int do_cmd_r4();
void check_alarmas();
void check_rs232_polling();

//-----------------------------------
// Definición de pines
// const int PIN_SCLK = 13;
// const int PIN_MISO = 12;
// const int PIN_MOSI = 11;

// -- Definicion de pines

// Para el RS232
const int PIN_RX_RS232 = 0;
const int PIN_TX_RS232 = 1;

// pines del AD7730
const int PIN_CS_AD7730 = 10;
const int PIN_RDY_AD730 = 2; // Int 0
const int PIN_RESET_AD730 = 8;

// pines del LS7366_CS
const int PIN_CS_LS7366 = 9;

// Pin test debug
const int PIN_TEST_DEBUG = 3;

// pines ALARMAS fuerza positiva y fuerza negativa
const int PIN_ALARMA_FUERZA_POST = 4;
const int PIN_ALARMA_FUERZA_NEG = 5;

//---- ADS7730
#define CS_AD7730_LOW() digitalWrite(PIN_CS_AD7730, LOW)
#define CS_AD7730_HIGH() digitalWrite(PIN_CS_AD7730, HIGH)

#define RESET_AD7730_LOW() digitalWrite(PIN_RESET_AD730, LOW)
#define RESET_AD7730_HIGH() digitalWrite(PIN_RESET_AD730, HIGH)

#define AD7730_RDY() digitalRead(PIN_RDY_AD730)

//-- LS7366
#define CS_LS7366_LOW() digitalWrite(PIN_CS_LS7366, LOW)
#define CS_LS7366_HIGH() digitalWrite(PIN_CS_LS7366, HIGH)

//-- ALARMAS DE SOBRECARGA
#define ALARMA_FUERZA_POST_OFF() digitalWrite(PIN_ALARMA_FUERZA_POST, LOW)
#define ALARMA_FUERZA_POST_ON() digitalWrite(PIN_ALARMA_FUERZA_POST, HIGH)

#define ALARMA_FUERZA_NEG_OFF() digitalWrite(PIN_ALARMA_FUERZA_NEG, LOW)
#define ALARMA_FUERZA_NEG_ON() digitalWrite(PIN_ALARMA_FUERZA_NEG, HIGH)

//-----------------------------------
#define LONG_COMANDO 150
// Parametros
#define NR_MAX_PARAMETROS 13
#define LONG_MAX_PARAMETRO 16

typedef enum DATA_RATE
{
    DATA_RATE_150 = 150,
    DATA_RATE_300 = 300,
    DATA_RATE_600 = 600,
    DATA_RATE_800 = 800,
    DATA_RATE_1000 = 1000,
    DATA_RATE_1200 = 1200
} TDataRate;

typedef struct
{
    int idcel; // Codigos
    float cap; // Capacidad
    int pol;   // Invertir polaridad
    float res; // resolucion
    // maxima carga soportada por la célula
    float limite_carga_celP; // positivo
    float limite_carga_celN; // negativo
    // Ganancia
    float gainpasostoFPos; // Relacion pasos-a-unidades-de-fuerza-positivo * 1000
    float gainpasostoFNeg; // Relacion pasos-a-unidades-de-fuerza-negativo * 1000
    TDataRate datarate;    // Hz data rate al que se ajusta la celula
} TCelula;

typedef struct
{

    float CCF_N;
    float CCF_KN;
    float CCF_K;
    float CCF_LB;

    // Datos de la célula en RAM
    byte id_celula; // numero de celula
    float cap_celula;
    float res_celula;
    int pol_celula;
    float gan_celulaPos; // pasos por unidad de fuerza
    float gan_celulaNeg;
    float limite_carga_celPos;
    float limite_carga_celNeg;
    long pasos_limite_carga_celPos;
    long pasos_limite_carga_celNeg;
    int datarate_celula;

    // Encoder
    long PosEncoder; // posición del encoder en pasos
    float CCE_MM;
    float CCE_IN;

    // Datos conversor
    long dac_CH1;
    long dac_CH2;
    long dac_filtrado_CH1;
    long dac_filtrado_CH2;
    long max_dac_CH1;       // máximo acumulado no filtrado
    long max_dac_tramo_CH1; // máximo parcial auto reseteable
    long max_dac_tramo_filtrado_CH1;

    long Cero_canal1;
    long Cero_canal2;

    int salida_datos_continua_start;
    int modo_salida_datos_binario;

} TVarGlobal; // capacidad 226 bytes

typedef struct
{
    int clave;
    int idEstacion;

    int IPE;         // polaridad de la extension
    byte count_mode; // quadrature count mode x1, x2, x4
    float PasoHusillo;
    float PasosEncoder;
    TCelula celulas[ID_MAXIMO_CELULA];

    boolean filtro_on_off;
    float gainpos;
    float gainneg;

    char id_maquina[15];
    uint16_t checksum;
} TVarEEprom;

typedef struct
{
    char params[NR_MAX_PARAMETROS][LONG_MAX_PARAMETRO];

} TComando;

TVarGlobal vg;       // datos en RAM
TVarEEprom vgEEprom; // Datos que recuperan y salvan en EEPROM
TComando comando;

//-- Buffer datos DAC18

#define DATOS_BUFFER_SIZE 32                // Debe ser potencia de 2 (2, 4, 8, 16, 32, 64...)
#define BUFFER_MASK (DATOS_BUFFER_SIZE - 1) // Resultado: 31 (0x1F)

typedef struct _tpDatoCanal
{
    volatile unsigned long secuencia;
    volatile unsigned long t_ms;
    volatile long dato;
} TDatoCanal;

typedef struct
{
    TDatoCanal buffer[DATOS_BUFFER_SIZE];
    volatile unsigned int head;
    volatile unsigned int tail;
} data_buffer;

void store_char(TDatoCanal *pdato, data_buffer *buffer);

data_buffer *pdatos_buffer;
data_buffer datos_buffer = {{0UL, 0UL, 0L}, 0U, 0U};

TDatoCanal datoCanal;
volatile boolean AD730_CONFIGURADO = false;

//------------------------------

long lastTimeAlarmas;

boolean newComando = false;
long baudRate;

SoftwareSerial SerialAux(6, 7); // RX, TX  para debug

void setup()
{
    noInterrupts();
    pinMode(PIN_RX_RS232, INPUT);
    // digitalWrite(PIN_RX_RS232, HIGH); // pull up para evitar ruidos en la entrada

    /*
    baudRate = detRate(PIN_RX_RS232); // La función devuelve alguna de las velocidades estandar que detecta
                                      // 1200,2400,4800,9600,14400,19200,28800,38400,57600,115200
                                      // por el envio del caracter "U" .
                                      // Returns 0 if no detecta o por debajo de 1200 baud
    if (baudRate == 0 || baudRate == 1200)
    {
        baudRate = 38400;
        Serial.begin(baudRate);
        Serial.println();
        Serial.print("Conectado a baudrate ");
        Serial.println(baudRate);
    }
    else
    {
        Serial.begin(baudRate);
        Serial.println("?");
    }

    */

    Serial.begin(BAUDRATE_RS232);
    SerialAux.begin(38400);

    SPI.begin();

    // PINES ADS7730
    pinMode(PIN_RESET_AD730, OUTPUT);
    pinMode(PIN_CS_AD7730, OUTPUT);
    pinMode(PIN_RDY_AD730, INPUT_PULLUP); // INPUT_PULLUP
    digitalWrite(PIN_CS_AD7730, HIGH);
    CS_AD7730_HIGH();

    // LS7366_CS
    pinMode(PIN_CS_LS7366, OUTPUT);
    CS_LS7366_HIGH();

    // ALARMAS
    pinMode(PIN_ALARMA_FUERZA_POST, OUTPUT);
    pinMode(PIN_ALARMA_FUERZA_NEG, OUTPUT);
    ALARMA_FUERZA_POST_OFF();
    ALARMA_FUERZA_NEG_OFF();

    // Pin PIN_TEST_DEBUG
    pinMode(PIN_TEST_DEBUG, OUTPUT);

    Buffer_init(&datos_buffer);

    recuperar_valores_eeprom();
    inicializar_alarmas();
    inicializar_celulas();
    inicializar_encoders();
    inicializar_adlc();
    delay(500);
    Buffer_Flush();
    fuerza_cero();

#if ADC_USE_INTERRUPT
    attachInterrupt(digitalPinToInterrupt(PIN_RDY_AD730), ISR_RDY_ADC7730, FALLING);
#endif

    

    AD730_CONFIGURADO = true;

    interrupts();

    
    vg.salida_datos_continua_start = 0;
    vg.modo_salida_datos_binario = 0;
    // vgEEprom.filtro_on_off = 1;

    lastTimeAlarmas = millis();
}

// Bucle principal
void loop0()
{
    // Recepción RS232 por sondeo directo (más robusto en este escenario)
    check_rs232_polling();

    // Consumir muestras del ADC desde el buffer circular
    // (imprescindible para actualizar vg.dac_CH1 y vg.dac_filtrado_CH1)
    if (IsDataAvailable())
    {
        leer_adlc();
    }

    // Comprobar y actualizar estado de alarmas
    static unsigned long last_check = 0;
    if (millis() - last_check > TIMEOUT_CHECK_ALARMAS)
    {
        check_alarmas();
        last_check = millis();
    }

    // Gestionar transmisión continua si está activa
    if (vg.salida_datos_continua_start)
    {
        do_cmd_r4();
    }
}

void loop()
{
    // Lectura NO bloqueante
    check_rs232_non_blocking();

    // Consumir muestras del ADC
    if (IsDataAvailable())
    {
        leer_adlc();
    }

    // Alarmas (sin cambios)
    static unsigned long last_check = 0;
    if (millis() - last_check > TIMEOUT_CHECK_ALARMAS)
    {
        check_alarmas();
        last_check = millis();
    }

    // Transmisión continua
    if (vg.salida_datos_continua_start)
    {
        do_cmd_r4();
    }
}

void check_rs232_polling()
{
    if (Serial.available() <= 0)
    {
        return;
    }

    static char comando_local[LONG_COMANDO + 1];
    int n = Serial.readBytesUntil(CHAR_FIN_CMD, comando_local, LONG_COMANDO);
    if (n <= 0)
    {
        return;
    }

    comando_local[n] = '\0';

    // Si viene LF residual al final, recortarlo
    while (n > 0 && (comando_local[n - 1] == '\n' || comando_local[n - 1] == '\r'))
    {
        comando_local[n - 1] = '\0';
        n--;
    }

    // Saltar posibles separadores al inicio
    int i = 0;
    while (comando_local[i] == '\r' || comando_local[i] == '\n' || comando_local[i] == ' ')
    {
        i++;
    }

    if (comando_local[i] == '\0')
    {
        return;
    }

    procesarComando(&comando_local[i]);
}


// Variables globales nuevas para el manejo del puerto
char inputBuffer[LONG_COMANDO];
int bufferIndex = 0;

void check_rs232_non_blocking() {
    while (Serial.available() > 0) {
        char inChar = (char)Serial.read();

        if (inChar == CHAR_INICIO_CMD) {
            bufferIndex = 0;
            continue;
        }

        if (inChar == CHAR_FIN_CMD || inChar == LF) {
            if (bufferIndex > 0) {
                inputBuffer[bufferIndex] = '\0';

                char *separator = strchr(inputBuffer, '|');
                if (separator != NULL) {
                    *separator = '\0'; // Cortamos la cadena: antes es ID, después es CMD
                    int idRecibido = atoi(inputBuffer);
                    char *ptrCmd = separator + 1;

                    // FILTRO DE IDENTIDAD
                    if (idRecibido == vgEEprom.idEstacion || idRecibido == 0) {
                        procesarComando(ptrCmd); 
                    }
                }
            }
            bufferIndex = 0;
        } else if (bufferIndex < (LONG_COMANDO - 1)) {
            if (inChar >= 32) inputBuffer[bufferIndex++] = inChar;
        }
    }
}

// ID máquina un caracter
void check_rs232_non_blocking_v0() {
    while (Serial.available() > 0) {
        char inChar = (char)Serial.read();

        // Ignorar caracteres de control innecesarios al inicio
        if (bufferIndex == 0 && (inChar == LF || inChar == ' ')) continue;

        // Si encontramos el carácter de fin (CR)
        if (inChar == CHAR_FIN_CMD) {
            inputBuffer[bufferIndex] = '\0'; // Cerrar cadena
            if (bufferIndex > 0) {
                procesarComando(inputBuffer);
            }
            bufferIndex = 0; // Reset para el siguiente comando
        } 
        else {
            // Añadir al buffer si hay espacio
            if (bufferIndex < LONG_COMANDO - 1) {
                // Solo añadir si no es un LF (para limpiar el par CR/LF)
                if (inChar != LF) {
                    inputBuffer[bufferIndex++] = inChar;
                }
            } else {
                // Buffer lleno: reset por seguridad
                bufferIndex = 0;
            }
        }
    }
}