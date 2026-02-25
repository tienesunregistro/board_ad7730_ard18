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
#define ID_MAXIMO_CELULA 5 // CELULAS DE LA 0..4

#define LF 0x0a // 10
#define CR 0x0d //13

// Caracteres de inicio y fin del comando
#define CHAR_INICIO_CMD ':'
#define CHAR_FIN_CMD CR

#define TIMEOUT_CHECK_ALARMAS 200 // ms
//-----------------------------------
// Definición de pines
//const int PIN_SCLK = 13;
//const int PIN_MISO = 12;
//const int PIN_MOSI = 11;

// -- Definicion de pines

// Para el RS232
const int PIN_RX_RS232 = 0;
const int PIN_TX_RS232 = 1;

//pines del AD7730
const int PIN_CS_AD7730 = 10;
const int PIN_RDY_AD730 = 2; //Int 0
const int PIN_RESET_AD730 = 8;

//pines del LS7366_CS
const int PIN_CS_LS7366 = 9;

// Pin test debug
const int PIN_TEST_DEBUG = 3;

//pines ALARMAS fuerza positiva y fuerza negativa
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
    //maxima carga soportada por la célula
    float limite_carga_celP; // positivo
    float limite_carga_celN; // negativo
    // Ganancia
    float gainpasostoFPos; // Relacion pasos-a-unidades-de-fuerza-positivo * 1000
    float gainpasostoFNeg; // Relacion pasos-a-unidades-de-fuerza-negativo * 1000
    TDataRate datarate;    //Hz data rate al que se ajusta la celula
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
    float gan_celulaPos; //pasos por unidad de fuerza
    float gan_celulaNeg;
    float limite_carga_celPos;
    float limite_carga_celNeg;
    long pasos_limite_carga_celPos;
    long pasos_limite_carga_celNeg;
    int datarate_celula;

    //Encoder
    long PosEncoder; // posición del encoder en pasos
    float CCE_MM;
    float CCE_IN;

    //Datos conversor
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
    byte count_mode; //quadrature count mode x1, x2, x4
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
    int nrparametros;
    char receivedChars[LONG_COMANDO + 1]; // uno mas para el nulo
    char params[NR_MAX_PARAMETROS][LONG_MAX_PARAMETRO] = {0};

} TComando;

TVarGlobal vg;       // datos en RAM
TVarEEprom vgEEprom; // Datos que recuperan y salvan en EEPROM
TComando comando;

//-- Buffer datos DAC18

#define DATOS_BUFFER_SIZE 32      // Debe ser potencia de 2 (2, 4, 8, 16, 32, 64...)
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
    //digitalWrite(PIN_RX_RS232, HIGH); // pull up para evitar ruidos en la entrada
   
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
    
    Serial.begin(38400);
    //Serial.begin(115200);
    SerialAux.begin(38400);

    SPI.begin();

    //PINES ADS7730
    pinMode(PIN_RESET_AD730, OUTPUT);
    pinMode(PIN_CS_AD7730, OUTPUT);
    pinMode(PIN_RDY_AD730, INPUT_PULLUP); //INPUT_PULLUP
    digitalWrite(PIN_CS_AD7730, HIGH);
    CS_AD7730_HIGH();

    //LS7366_CS
    pinMode(PIN_CS_LS7366, OUTPUT);
    CS_LS7366_HIGH();

    //ALARMAS
    pinMode(PIN_ALARMA_FUERZA_POST, OUTPUT);
    pinMode(PIN_ALARMA_FUERZA_NEG, OUTPUT);
    ALARMA_FUERZA_POST_OFF();
    ALARMA_FUERZA_NEG_OFF();

    //Pin PIN_TEST_DEBUG
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

    attachInterrupt(digitalPinToInterrupt(PIN_RDY_AD730), ISR_RDY_ADC7730, FALLING);

    AD730_CONFIGURADO = true;

    interrupts();

    vg.salida_datos_continua_start = 0;
    vg.modo_salida_datos_binario = 0;
    //vgEEprom.filtro_on_off = 1;

    lastTimeAlarmas = millis();
}

void loop()
{
    //digitalWrite(PIN_TEST_DEBUG, 1);

    int rta = 0;
    unsigned long currentMillis = millis();

    if (IsDataAvailable())
    {
        leer_adlc();
    }
    //leer_encoder();

    if (vg.salida_datos_continua_start)
    {
        do_cmd_r4();
    }

    recvComandoConMarcadoresInicioFinal();
    comando.nrparametros = 0;
    if (newComando == true)
    {
        comando.nrparametros = get_params((char *)comando.receivedChars, comando.params, NR_MAX_PARAMETROS);

        if (atoi(comando.params[0]) == vgEEprom.idEstacion)
        {
            rta = procesarComando(comando.params[1]);
        }
        newComando = false;
    }

    if ((currentMillis - lastTimeAlarmas) >= TIMEOUT_CHECK_ALARMAS)
    {
        check_alarmas();
        lastTimeAlarmas = millis();
    }

    //digitalWrite(PIN_TEST_DEBUG, 0);
}

void recvComandoConMarcadoresInicioFinal()
{
    static boolean recvEnProgreso = false;
    static byte ndx = 0;
    char marcadorInicio = CHAR_INICIO_CMD;
    char marcadorFinal = CHAR_FIN_CMD;
    char rc;

    while (Serial.available() > 0 && newComando == false)
    {
        rc = Serial.read();

        if (recvEnProgreso == true)
        {
            if (rc != marcadorFinal)
            {
                comando.receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= LONG_COMANDO)
                {
                    ndx = LONG_COMANDO - 1;
                }
            }
            else
            {
                comando.receivedChars[ndx] = '\0'; // terminate the string
                recvEnProgreso = false;
                ndx = 0;
                newComando = true;
            }
        }
        else if (rc == marcadorInicio)
        {
            recvEnProgreso = true;
        }
    }
}
