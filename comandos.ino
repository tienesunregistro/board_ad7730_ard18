//*************** COMANDOS RS232 *************************
// LECTURA
#define CMD_READ 10

#define CMD_RI CMD_READ + 1
#define CMD_R1 CMD_READ + 2
#define CMD_R2 CMD_READ + 3
#define CMD_R3 CMD_READ + 4
#define CMD_R4 CMD_READ + 5
#define CMD_R5 CMD_READ + 6

#define CMD_RA CMD_READ + 7
#define CMD_RB CMD_READ + 8
#define CMD_RC CMD_READ + 9

#define CMD_RH CMD_READ + 10
#define CMD_RP CMD_READ + 11
#define CMD_RS CMD_READ + 12
#define CMD_RX CMD_READ + 13

#define CMD_RERROR CMD_READ + 40
#define CMD_RS232_ERROR CMD_READ + 41

// ESCRITURA
#define CMD_WRITE 100
#define CMD_WE CMD_WRITE + 1
#define CMD_WT CMD_WRITE + 2
#define CMD_WI CMD_WRITE + 3
#define CMD_WP CMD_WRITE + 4
#define CMD_WK CMD_WRITE + 5
#define CMD_WY CMD_WRITE + 6
#define CMD_WZ CMD_WRITE + 20

// Union para convertir el float a bytes
union conversor
{
    float valorf;
    uint8_t Data[4];
};

//*********************************************************

// Prototipos de funciones para los comandos
int do_cmd_ri();
int do_cmd_r1();
int do_cmd_r2();
int do_cmd_r3();
int do_cmd_r4();
int do_cmd_ra();
int do_cmd_rb();
int do_cmd_rc();
int do_cmd_rs();
int do_cmd_rx();
int do_cmd_rp(int n_params, char params[][LONG_MAX_PARAMETRO]);
int do_cmd_we();
int do_cmd_wt();
int do_cmd_wi(int n_params, char params[][LONG_MAX_PARAMETRO]);
int do_cmd_wy();
int do_cmd_wz();
int do_cmd_wp(int n_params, char params[][LONG_MAX_PARAMETRO]);
int do_cmd_error(const char *error_msg);
int freeRam();

/*
  Se le pasa "cadena" que contiene campos separados por "|"
  y rellena "params" con los campos. Devuelve el numero de campos
  encontrados. Es una versión segura que usa strtok_r y previene overflows.
*/
int get_params(char *cadena, char params[][LONG_MAX_PARAMETRO], int nr_max_parametros)
{
    char *p;
    char *token;
    int i = 0;

    // strtok_r es seguro para interrupciones y no modifica el puntero original
    token = strtok_r(cadena, "|", &p);

    while (token != NULL)
    {
        if (i < nr_max_parametros)
        {
            // Copia segura para evitar desbordamiento del parámetro
            strncpy(params[i], token, LONG_MAX_PARAMETRO - 1);
            params[i][LONG_MAX_PARAMETRO - 1] = '\0'; // Asegurar terminación nula
            i++;
        }
        else
        {
            // Se encontraron más parámetros de los que el buffer puede alojar
            break;
        }
        token = strtok_r(NULL, "|", &p);
    }

    return i;
}

int procesarComando(char *cmdStr) {
    // cmdStr ya llega como "RP|9" o "R1" (sin el :ID|)
    int n_params = 0;
    memset(comando.params, 0, sizeof(comando.params));

    char *sep = strchr(cmdStr, '|');
    char cmd_name[5]; // Para RI, RP, R1...

    if (sep != NULL) {
        // Hay parámetros (ej: RP|9)
        int cmdLen = sep - cmdStr;
        strncpy(cmd_name, cmdStr, (cmdLen > 4) ? 4 : cmdLen);
        cmd_name[(cmdLen > 4) ? 4 : cmdLen] = '\0';
        
        // El resto son parámetros
        n_params = get_params(sep + 1, comando.params, NR_MAX_PARAMETROS);
    } else {
        // No hay parámetros (ej: R1)
        strncpy(cmd_name, cmdStr, 4);
        cmd_name[4] = '\0';
        n_params = 0;
    }

    if (strlen(cmd_name) < 2) return do_cmd_error("E|CMD_SHORT");

    char prefijo = cmd_name[0];
    char accion = cmd_name[1];

    if (prefijo == 'R') {
        switch (accion) {
            case '1': return do_cmd_r1();
            case '2': return do_cmd_r2();
            case '3': return do_cmd_r3();
            case '4': return do_cmd_r4();
            case 'A': return do_cmd_ra();
            case 'B': return do_cmd_rb();
            case 'C': return do_cmd_rc();
            case 'P': return do_cmd_rp(n_params, comando.params);
            case 'S': return do_cmd_rs();
            case 'X': return do_cmd_rx();
            case 'I': return do_cmd_ri();
            default:  return do_cmd_error("E|CMD_UNKNOWN");
        }
    } else if (prefijo == 'W') {
        switch (accion) {
            case 'P': return do_cmd_wp(n_params, comando.params);
            case 'E': return do_cmd_we();
            case 'I': return do_cmd_wi(n_params, comando.params);
            case 'Y': return do_cmd_wy();
            case 'Z': return do_cmd_wz();
            case 'T': return do_cmd_wt();
            default:  return do_cmd_error("E|CMD_UNKNOWN");
        }
    }
    return do_cmd_error("E|CMD_UNKNOWN");
}


// Procesa el comando principal, parsea y delega a las funciones do_
int procesarComando_v0(char *cmdStr)
{
    static char cmd_copy[LONG_COMANDO + 1];
    int n_params = 0;

    // Limpiar buffer global de parámetros para no arrastrar datos previos
    memset(comando.params, 0, sizeof(comando.params));

    // 1. Extraer el comando principal (ej: ":1|RP|9" -> "RP")
    // Usamos copia estática para no cargar la pila
    strncpy(cmd_copy, cmdStr, LONG_COMANDO);
    cmd_copy[LONG_COMANDO] = '\0';

    // Parseo determinista formato :ID|CMD|P1|P2...
    char *start = cmd_copy;
    if (*start == ':')
    {
        start++;
    }

    if (*start == '\0')
        return 0;

    // Separador tras ID
    char *sep1 = strchr(start, '|');
    if (sep1 == NULL)
        return do_cmd_error("E|CMD_MISSING");

    *sep1 = '\0'; // ID aislado (no usado aquí)

    // Comando (R1, RI, RP, WP, ...)
    char *cmd_name = sep1 + 1;
    if (*cmd_name == '\0')
        return do_cmd_error("E|CMD_MISSING");

    // Separador tras CMD (si existe, hay parámetros)
    char *sep2 = strchr(cmd_name, '|');
    if (sep2 != NULL)
    {
        *sep2 = '\0';
        char *params_start = sep2 + 1;
        if (*params_start != '\0')
        {
            n_params = get_params(params_start, comando.params, NR_MAX_PARAMETROS);
        }
        else
        {
            n_params = 0;
        }
    }
    else
    {
        n_params = 0;
    }

    // 2. Decodificar y ejecutar con un switch eficiente
    if (strlen(cmd_name) < 2)
        return do_cmd_error("E|CMD_SHORT");

    char prefijo = cmd_name[0];
    char accion = cmd_name[1];

    if (prefijo == 'R')
    {
        switch (accion)
        {
        case '1':
            return do_cmd_r1();
        case '2':
            return do_cmd_r2();
        case '3':
            return do_cmd_r3();
        case '4':
            return do_cmd_r4();
        case 'A':
            return do_cmd_ra();
        case 'B':
            return do_cmd_rb();
        case 'C':
            return do_cmd_rc();
        case 'P':
            return do_cmd_rp(n_params, comando.params);
        case 'S':
            return do_cmd_rs();
        case 'X':
            return do_cmd_rx();
        case 'I':
            return do_cmd_ri();
        default:
            return do_cmd_error("E|CMD_UNKNOWN");
        }
    }
    else if (prefijo == 'W')
    {
        switch (accion)
        {
        case 'P':
            return do_cmd_wp(n_params, comando.params);
        case 'E':
            return do_cmd_we();
        case 'I':
            return do_cmd_wi(n_params, comando.params);
        case 'Y':
            return do_cmd_wy();
        case 'Z':
            return do_cmd_wz();
        case 'T':
            return do_cmd_wt(); // No utilizado, pero mantenido
        default:
            return do_cmd_error("E|CMD_UNKNOWN");
        }
    }

    return do_cmd_error("E|CMD_UNKNOWN");
}

int do_cmd_ri()
{
    Serial.print(F(IDBOARD));
    Serial.print(F(" / "));
    Serial.print(F(IDMAQ));
    Serial.print(F(" / "));
    Serial.print(F(SKERNEL));
    Serial.write(0xd);

    return 1;
}

int do_cmd_rh()
{
    Serial.write(0xd);
    return 1;
}

// Lectura de fuerza en N, con compensación de cero
int do_cmd_r1()
{
    float datoF;

    if (vgEEprom.filtro_on_off)
    {
        datoF = LecturaFuerzaFiltrada();
    }
    else
    {
        datoF = LecturaFuerzaNoFiltrada();
    }

    if (abs(datoF) > 10000)
    {
        Serial.print(datoF, 1);
        Serial.write(0xd);
    }
    else
    {
        Serial.print(datoF, 3);
        Serial.write(0xd);
    }

    return 1;
}

// Lectura encoder
int do_cmd_r2()
{
    float value;
    value = LecturaExtension();
    Serial.print(value, 3);
    Serial.write(0xd);
    // Serial.println(vg.PosEncoder);
    return 1;
}

// Lectura analogico CH 1, valor  en PASOS
int do_cmd_r3()
{
    long value;

    if (vgEEprom.filtro_on_off)
    {
        value = LecturaFuerzaPasosFiltrada();
    }
    else
    {
        value = LecturaFuerzaPasosNoFiltrada();
    }

    Serial.print(value);
    Serial.write(0xd);
    return 1;
}

void float32_to_serial_binary(float datoF)
{
    int i;
    union conversor valor;
    valor.valorf = datoF;
    for (i = 0; i < 4; i++)
    {
        Serial.write(valor.Data[3 - i]);
    }
}

// devuel en binario el pakete de datos de fuerza y extensión
int do_cmd_r4()
{
    float datoF;

    // Datos de cabecera
    uint8_t fs1 = 0xAA;
    uint8_t fs2 = 0x55; // 0xBB;

    uint8_t nrBytes = 8; // 4 bytes * 2 variables

    if (vgEEprom.filtro_on_off)
    {
        datoF = LecturaFuerzaFiltrada();
    }
    else
    {
        datoF = LecturaFuerzaNoFiltrada();
    }

    if (!vg.modo_salida_datos_binario)
    {
        if (abs(datoF) > 10000)
        {
            Serial.print(datoF, 1);
            Serial.write('|');
        }
        else
        {
            Serial.print(datoF, 3);
            Serial.write('|');
        }
    }
    else
    {

        Serial.write(fs1);
        Serial.write(fs2);
        Serial.write(nrBytes);
        float32_to_serial_binary(datoF);
    }

    datoF = LecturaExtension();

    if (!vg.modo_salida_datos_binario)
    {
        Serial.print(datoF, 3);
        Serial.write(0xd);
        Serial.write(0xa); // !!!!!! OJO so para test con el grafchart
    }
    else
    {
        float32_to_serial_binary(datoF);
    }

    return 1;
}

int do_cmd_r5()
{

    return 1;
}

//----- Lectura de picos máximos realtime
// Estos comandos resetean el valor una vez leido
// Lectura pico máximo de fuerza, autoreseteable
int do_cmd_ra()
{
    float datoF;

    datoF = LecturaFuerzaMaxima();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

// Lectura pico máximo de fuerza en un tramo, autoreseteable
int do_cmd_rb()
{
    float datoF;

    datoF = LecturaFuerzaMaximaTramo();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

// Lectura pico máximo de fuerza en un tramo filtrado, autoreseteable
int do_cmd_rc()
{
    float datoF;

    datoF = LecturaFuerzaMaximaTramoFiltrada();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

// Alarmas
int do_cmd_rs()
{
    int value = get_alarmas();

    Serial.print(value);
    Serial.write(0xd);
    return 1;
}
// Identificar extensometro
int do_cmd_rx()
{
    int value;
    value = identificar_celula();
    Serial.print(value);
    Serial.write(0xd);
    return 1;
}

// Cero de extension
int do_cmd_we()
{
    cero_encoder();
    leer_encoder();
    return 1;
}

// Lanzar el ensayo
int do_cmd_wt()
{

    return 1;
}

/******************************************************
 * WI|Pin|Valor	- Escritura	I/O
 * Pin Nr. del pin de la placa arduino
 * Valor: 0 / 1
 ******************************************************/

int do_cmd_wi(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 2)
    {
        return do_cmd_error("E|PARAM_COUNT");
    }
    int pin = atoi(params[0]);
    int estado = atoi(params[1]);

    // Validar que el pin sea uno de los permitidos para escritura
    if (pin != PIN_ALARMA_FUERZA_POST && pin != PIN_ALARMA_FUERZA_NEG && pin != PIN_TEST_DEBUG)
    {
        return do_cmd_error("E|PIN_INVALID");
    }

    digitalWrite(pin, estado);

    return 1;
}

// Reset hardware del conversor
int do_cmd_wy()
{
    AD730_CONFIGURADO = false;
    reset_adlc();
    delay(100);
    inicializar_adlc();
    delay(100);
    cero_adlc();
    AD730_CONFIGURADO = true;
    return 1;
}

// Cero conversor
int do_cmd_wz()
{
    fuerza_cero();
    return 1;
}

int do_cmd_rp(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 1)
    {
        return do_cmd_error("E|PARAM_COUNT");
    }

    int indice = 0;
    int funcion = atoi(params[0]);

    switch (funcion)
    {
    case 0:
        break;

    case 9: // RP|9  // filtro_on_off
        Serial.print(vgEEprom.filtro_on_off);
        Serial.write(0xd);
        break;

    case 10: // RP|10  // IdEstacion
        Serial.print(vgEEprom.idEstacion);
        Serial.write(0xd);
        break;

    case 13: // RP|13 lectura parametros célula en RAM
        Serial.print(vg.id_celula);
        Serial.print('|');
        Serial.print(vg.cap_celula, 3);
        Serial.print('|');
        Serial.print(vg.pol_celula);
        Serial.print('|');
        Serial.print(vg.res_celula, 3);
        Serial.print('|');
        Serial.print(vg.limite_carga_celPos, 3);
        Serial.print('|');
        Serial.print(vg.limite_carga_celNeg, 3);
        Serial.print('|');
        Serial.print(vg.gan_celulaPos, 3);
        Serial.print('|');
        Serial.print(vg.gan_celulaNeg, 3);
        Serial.print('|');
        Serial.print(vg.datarate_celula);
        Serial.write(0xd);
        break;

    case 14: // RP|14
        Serial.print(vgEEprom.PasoHusillo);
        Serial.write(0xd);
        break;

    case 15: // RP|15
        Serial.print(vgEEprom.PasosEncoder);
        Serial.write(0xd);
        break;

    case 16: // RP|16 // polaridad extension
        Serial.print(vgEEprom.IPE);
        Serial.write(0xd);
        break;

    case 17: // RP|17 // modo quadrature
        Serial.print(vgEEprom.count_mode);
        Serial.write(0xd);
        break;

    case 18: // RP|18
        break;

    case 19: // RP|19|xx lectura parametros célula xxx en EEPROM
        if (n_params < 2)
            return do_cmd_error("E|PARAM_COUNT");

        indice = atoi(params[1]);
        if (indice < 0 || indice > ID_MAXIMO_CELULA - 1)
            return do_cmd_error("E|CELL_ID_OOR");

        Serial.print(vgEEprom.celulas[indice].idcel);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].cap, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].pol);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].res, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].limite_carga_celP, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].limite_carga_celN, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].gainpasostoFPos, 5);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].gainpasostoFNeg, 5);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].datarate);
        Serial.write(0xd);
        break;

    case 21: // RP|21 // salida de datos continua start stop
        Serial.print(vg.salida_datos_continua_start);
        Serial.write(0xd);
        break;

    case 22: // RP|22 // salida de datos binario
        Serial.print(vg.modo_salida_datos_binario);
        Serial.write(0xd);
        break;

    case 90: // RP|90 // ID maquina
        Serial.print(vgEEprom.id_maquina);
        Serial.write(0xd);
        break;

    case 99: // RP|99 Test memoria RAM
        Serial.print(F("Memoria RAM:"));
        Serial.print(freeRam());
        Serial.write(0xd);
        break;

    default:
        return do_cmd_error("E|PARAM_UNKNOWN");
    }

    return 1;
}

int do_cmd_wp(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 1)
    {
        return do_cmd_error("E|PARAM_COUNT");
    }
    int indice = 0;
    int funcion = atoi(params[0]);

    switch (funcion)
    {

    case 0:
        if (n_params != 1)
            return do_cmd_error("E|PARAM_COUNT");
        salvar_valores();
        break;

    case 9: // WP|9|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.filtro_on_off = atoi(params[1]);
        break;

    case 10: // WP|10|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.idEstacion = atoi(params[1]);
        break;

    case 14: // WP|14|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.PasoHusillo = atof(params[1]);
        RecalcularCoeficientes();
        break;
    case 15: // WP|15|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.PasosEncoder = atof(params[1]);
        RecalcularCoeficientes();
        break;
    case 16: // WP|16|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.IPE = atoi(params[1]);
        RecalcularCoeficientes();
        break;

    case 17: // WP|17|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vgEEprom.count_mode = atoi(params[1]);
        inicializar_encoders();
        RecalcularCoeficientes();
        break;

    case 18: // WP|18|xx

        break;

    case 19:
        // WP|19|idcel|cap|pol|res|limite_carga_celP|limite_carga_celN|gainpasostoFPos|gainpasostoFNeg|datarate
        if (n_params != 10)
            return do_cmd_error("E|PARAM_COUNT_19");

        indice = atoi(params[1]);
        if (indice < 0 || indice >= ID_MAXIMO_CELULA)
            return do_cmd_error("E|CELL_ID_OOR");

        vgEEprom.celulas[indice].idcel = atoi(params[1]);
        vgEEprom.celulas[indice].cap = atof(params[2]);
        vgEEprom.celulas[indice].pol = atoi(params[3]);
        vgEEprom.celulas[indice].res = atof(params[4]);
        vgEEprom.celulas[indice].limite_carga_celP = abs(atof(params[5]));
        vgEEprom.celulas[indice].limite_carga_celN = abs(atof(params[6]));
        vgEEprom.celulas[indice].gainpasostoFPos = atof(params[7]);
        vgEEprom.celulas[indice].gainpasostoFNeg = atof(params[8]);
        vgEEprom.celulas[indice].datarate = (TDataRate)atoi(params[9]);

        ValidarLimites(indice);
        configurar_celula(indice);

        break;

    case 20: // WP|20|xx activar celula por software
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        indice = atoi(params[1]);
        if (indice < 0 || indice >= ID_MAXIMO_CELULA)
            return do_cmd_error("E|CELL_ID_OOR");

        configurar_celula(indice);
        break;

    case 21: // WP|21|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vg.salida_datos_continua_start = atoi(params[1]);
        break;

    case 22: // WP|22|xx
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        vg.modo_salida_datos_binario = atoi(params[1]);
        break;

    case 90: // WP|90|xx texto de 14 caracteres
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        strncpy(vgEEprom.id_maquina, params[1], 14);
        vgEEprom.id_maquina[14] = '\0'; // Asegurar terminación
        break;

    case 999:
        if (n_params != 1)
            return do_cmd_error("E|PARAM_COUNT");
        SetValorPordefecto();
        salvar_valores();
        break;

    default:
        return do_cmd_error("E|PARAM_UNKNOWN");
    }

    return 1;
}

int do_cmd_error(const char *error_msg)
{
    Serial.println(error_msg);
    return 0; // Devolvemos 0 para indicar fallo
}

int freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
