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



//Union para convertir el float a bytes
union conversor
{
    float valorf;
    uint8_t Data[4];
};

//*********************************************************

/*
  Se le pasa "cadena" que contiene campos separados por "|"
 y rellena "params" con los campos, devuelve el numero de campos
 encontrados en "cadena"
 cada parametro tine LONG_MAX_PARAMETRO carateres máximo
 */

 int get_params(char *cadena, char params[][LONG_MAX_PARAMETRO], int nr_max_parametros)
{
    char *token;
    char *p;
    int i = 0;
    char stringBuffer[LONG_COMANDO + 1];

    // 1. Copia segura al buffer temporal
    strncpy(stringBuffer, cadena, LONG_COMANDO);
    stringBuffer[LONG_COMANDO] = '\0'; // Garantizar cierre nulo si la cadena era muy larga

    // 2. Troceado con strtok_r
    token = strtok_r(stringBuffer, "|", &p);
    
    while (token != NULL)
    {
        if (i < nr_max_parametros) // 3. Protección de índice de parámetros
        {
            // 4. COPIA SEGURA DEL PARÁMETRO (strncpy)
            // Copiamos como máximo LONG_MAX_PARAMETRO - 1 (para dejar sitio al '\0')
            strncpy(params[i], token, LONG_MAX_PARAMETRO - 1);
            
            // 5. Garantizar el cierre del nulo manualmente
            params[i][LONG_MAX_PARAMETRO - 1] = '\0'; 

            i++;
        }
        else 
        {
            // Opcional: Si hay más parámetros de los permitidos, salimos
            break; 
        }
        
        token = strtok_r(NULL, "|", &p);
    }

    return i; // Devolvemos cuántos parámetros pudimos procesar con seguridad
}

int get_params_org(char *cadena, char params[][LONG_MAX_PARAMETRO], int nr_max_parametros)
{
    char *token;
    char *p;
    int i = 0;
    char stringBuffer[LONG_COMANDO + 1];

    strncpy(stringBuffer, cadena, LONG_COMANDO);
    stringBuffer[LONG_COMANDO] = '\0'; // Garantizar cierre nulo si la cadena era muy larga

    //Serial.println(stringBuffer);

    token = strtok_r(stringBuffer, "|", &p);
    do
    {
        if (token)
        {
            strncpy(params[i], token, LONG_MAX_PARAMETRO - 1);
            params[i][LONG_MAX_PARAMETRO - 1] = '\0'; // Asegurar terminación nula

            //sprintf(str1, "%d = %s", i, params[i]);
            //Serial.println(str1);

            i++;
            if (i > nr_max_parametros - 1)
            {
                return -1;
            }
        }
    } while ((token = strtok_r(NULL, "|", &p)) != NULL);

    return i;
}

//Pasa el comando a un enumerador
int decodificarComando(char *comando)
{
    if (!strcmp(comando, "R1"))
    {
        return CMD_R1;
    }

    if (!strcmp(comando, "R2"))
    {
        return CMD_R2;
    }

    if (!strcmp(comando, "RP"))
    {
        return CMD_RP;
    }

    if (!strcmp(comando, "WP"))
    {
        return CMD_WP;
    }

    if (!strcmp(comando, "R3"))
    {
        return CMD_R3;
    }

    if (!strcmp(comando, "R4"))
    {
        return CMD_R4;
    }

    if (!strcmp(comando, "R5"))
    {
        return CMD_R5;
    }

    if (!strcmp(comando, "RA"))
    {
        return CMD_RA;
    }

    if (!strcmp(comando, "RB"))
    {
        return CMD_RB;
    }

    if (!strcmp(comando, "RC"))
    {
        return CMD_RC;
    }

    if (!strcmp(comando, "RS"))
    {
        return CMD_RS;
    }

    if (!strcmp(comando, "RX"))
    {
        return CMD_RX;
    }

    if (!strcmp(comando, "RI"))
    {
        return CMD_RI;
    }

    if (!strcmp(comando, "WE"))
    {
        return CMD_WE;
    }

    if (!strcmp(comando, "WT"))
    {
        return CMD_WT;
    }

    if (!strcmp(comando, "WI"))
    {
        return CMD_WI;
    }

    if (!strcmp(comando, "WY"))
    {
        return CMD_WY;
    }

    if (!strcmp(comando, "WZ"))
    {
        return CMD_WZ;
    }

    return CMD_RERROR;
}

//Aqui llega el comando sin el Id de estacion y sin los parametros

int procesarComando(char *cmdStr) {
    if (strlen(cmdStr) < 2) return do_cmd_error();

    char prefijo = cmdStr[0];
    char accion = cmdStr[1];

    // Agrupamos por lectura (R) o escritura (W)
    if (prefijo == 'R') {
        switch (accion) {
            case '1': return do_cmd_r1();
            case '2': return do_cmd_r2();
            case '3': return do_cmd_r3();
            case '4': return do_cmd_r4();
            case 'A': return do_cmd_ra();
            case 'B': return do_cmd_rb();
            case 'C': return do_cmd_rc();
            case 'P': return do_cmd_rp();
            case 'S': return do_cmd_rs();
            case 'X': return do_cmd_rx();
            case 'I': return do_cmd_ri();
            default:  return do_cmd_error();
        }
    } 
    else if (prefijo == 'W') {
        switch (accion) {
            case 'P': return do_cmd_wp();
            case 'E': return do_cmd_we();
            case 'I': return do_cmd_wi();
            case 'Y': return do_cmd_wy();
            case 'Z': return do_cmd_wz();
            case 'T': return do_cmd_wt();
            default:  return do_cmd_error();
        }
    }
    
    return do_cmd_error();
}

int procesarComando_org(char *comandox)
{
    int cmd;
    int ok = 0;

    cmd = decodificarComando(comandox);

    switch (cmd)
    {

    case CMD_R1:
        ok = do_cmd_r1();
        break;
    case CMD_R2:
        ok = do_cmd_r2();
        break;

    case CMD_RP:
        ok = do_cmd_rp();
        break;

    case CMD_WP:
        ok = do_cmd_wp();
        break;

    case CMD_R3:
        ok = do_cmd_r3();
        break;

    case CMD_R4:
        ok = do_cmd_r4();
        break;

    case CMD_R5:
        ok = do_cmd_r5();
        break;

    case CMD_RA:
        ok = do_cmd_ra();
        break;

    case CMD_RB:
        ok = do_cmd_rb();
        break;

    case CMD_RC:
        ok = do_cmd_rc();
        break;

    case CMD_RS:
        ok = do_cmd_rs();
        break;

    case CMD_RX:
        ok = do_cmd_rx();
        break;

    case CMD_RI:
        ok = do_cmd_ri();
        break;

    case CMD_WE:
        ok = do_cmd_we();
        break;

    case CMD_WT:
        ok = do_cmd_wt();
        break;

    case CMD_WI:
        ok = do_cmd_wi();
        break;

    case CMD_WY:
        ok = do_cmd_wy();
        break;

    case CMD_WZ:
        ok = do_cmd_wz();
        break;

    case CMD_RERROR:
        ok = do_cmd_error();
        break;
    }
    return ok;
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

//Lectura encoder
int do_cmd_r2()
{
    float value;
    value = LecturaExtension();
    Serial.print(value, 3);
    Serial.write(0xd);
    //Serial.println(vg.PosEncoder);
    return 1;
}

//Lectura analogico CH 1, valor  en PASOS
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
    uint8_t fs2 = 0x55; //0xBB;

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
//Lectura pico máximo de fuerza, autoreseteable
int do_cmd_ra()
{
    float datoF;

    datoF = LecturaFuerzaMaxima();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

//Lectura pico máximo de fuerza en un tramo, autoreseteable
int do_cmd_rb()
{
    float datoF;

    datoF = LecturaFuerzaMaximaTramo();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

//Lectura pico máximo de fuerza en un tramo filtrado, autoreseteable
int do_cmd_rc()
{
    float datoF;

    datoF = LecturaFuerzaMaximaTramoFiltrada();

    Serial.print(datoF, 3);
    Serial.write(0xd);

    return 1;
}

//Alarmas
int do_cmd_rs()
{
    int value = get_alarmas();

    Serial.print(value);
    Serial.write(0xd);
    return 1;
}
//Identificar extensometro
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

//Lanzar el ensayo
int do_cmd_wt()
{

    return 1;
}

/******************************************************
 * WI|Pin|Valor	- Escritura	I/O
 * Pin Nr. del pin de la placa arduino
 * Valor: 0 / 1
 ******************************************************/

int do_cmd_wi()
{
    int pin = atoi(comando.params[1]);
    int estado = atoi(comando.params[2]);
    digitalWrite(pin, estado);
   
    return 1;
}

//Reset hardware del conversor
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

//Cero conversor
int do_cmd_wz()
{
    fuerza_cero();
    return 1;
}

int freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

int do_cmd_error()
{

    Serial.print('?');
    Serial.write(0xd);

    return 1;
}

// ------------------------------ Registro de parametros -----
int do_cmd_rp()
{
    int indice = 0;
    int funcion = atoi(comando.params[2]);

    switch (funcion)
    {

    case 0:
        
        break;

    case 9: //RP|9  // filtro_on_off
        Serial.print(vgEEprom.filtro_on_off);
        Serial.write(0xd);
        break;

    case 10: //RP|10  // IdEstacion
        Serial.print(vgEEprom.idEstacion);
        Serial.write(0xd);
        break;

    case 13: //RP|13 lectura parametros célula en RAM
       
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

    case 14: //RP|14
        Serial.print(vgEEprom.PasoHusillo);
        Serial.write(0xd);
        break;
    case 15: //RP|15
        Serial.print(vgEEprom.PasosEncoder);
        Serial.write(0xd);
        break;
    case 16: //RP|16  //polaridad extension
        Serial.print(vgEEprom.IPE);
        Serial.write(0xd);
        break;
    case 17: //RP|17  //modo quadrature
        Serial.print(vgEEprom.count_mode);
        Serial.write(0xd);
        break;

    case 18: //RP|18

        break;

    case 19: //RP|19|xx lectura parametros célula xxx en EEPROM
        indice = atoi(comando.params[3]);

        indice = atoi(comando.params[3]);
        if (indice < 0 || indice > ID_MAXIMO_CELULA - 1)
            return 0;

        Serial.print(vgEEprom.celulas[indice].idcel);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].cap, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].pol);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].res, 3);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].limite_carga_celP, 3);
        Serial.print('|'); // en unidades de fuerza N
        Serial.print(vgEEprom.celulas[indice].limite_carga_celN, 3);
        Serial.print('|'); // en unidades de fuerza N
        Serial.print(vgEEprom.celulas[indice].gainpasostoFPos, 5);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].gainpasostoFNeg, 5);
        Serial.print('|');
        Serial.print(vgEEprom.celulas[indice].datarate);
        Serial.write(0xd);
        break;

    case 21: //RP|21  //salida de datos continua start stop
        Serial.print(vg.salida_datos_continua_start);
        Serial.write(0xd);
        break;

    case 22: //RP|22  //salida de datos binario
        Serial.print(vg.modo_salida_datos_binario);
        Serial.write(0xd);
        break;

    case 90: //RP|90  //ID maquina
        Serial.print(vgEEprom.id_maquina);
        Serial.write(0xd);
        break;

    case 99: //RP|99 Test memoria RAM
        Serial.print(F("Memoria RAM:"));
        Serial.print(freeRam());
        Serial.write(0xd);

        break;

    default:
        break;
    }
    return 1;
}

int do_cmd_wp()
{
    int indice = 0;
    int funcion = atoi(comando.params[2]);

    switch (funcion)
    {

    case 0:
        salvar_valores();
        break;

    case 9: //WP|9|xx
        vgEEprom.filtro_on_off = atoi(comando.params[3]);
        break;

    case 10: //WP|10|xx
        vgEEprom.idEstacion = atoi(comando.params[3]);
        break;

    case 14: //WP|14|xx
        vgEEprom.PasoHusillo = atof(comando.params[3]);
        RecalcularCoeficientes();
        break;
    case 15: //WP|15|xx
        vgEEprom.PasosEncoder = atof(comando.params[3]);
        RecalcularCoeficientes();
        break;
    case 16: //WP|16|xx
        vgEEprom.IPE = atoi(comando.params[3]);
        RecalcularCoeficientes();
        break;

    case 17: //WP|17|xx
        vgEEprom.count_mode = atoi(comando.params[3]);
        inicializar_encoders();
        RecalcularCoeficientes();
        break;

    case 18: //WP|18|xx

        break;

    case 19:
        // WP|19|idcel|cap|pol|res|limite_carga_celP|limite_carga_celN|gainpasostoFPos|gainpasostoFNeg|datarate

        indice = atoi(comando.params[3]);
        if (indice < 0 || indice > ID_MAXIMO_CELULA - 1 || comando.nrparametros > 12)
            return 0;

        vgEEprom.celulas[indice].idcel = atoi(comando.params[3]);
        vgEEprom.celulas[indice].cap = atof(comando.params[4]);
        vgEEprom.celulas[indice].pol = atoi(comando.params[5]);
        vgEEprom.celulas[indice].res = atof(comando.params[6]);
        vgEEprom.celulas[indice].limite_carga_celP = abs(atof(comando.params[7]));
        vgEEprom.celulas[indice].limite_carga_celN = abs(atof(comando.params[8]));
        vgEEprom.celulas[indice].gainpasostoFPos = atof(comando.params[9]);
        vgEEprom.celulas[indice].gainpasostoFNeg = atof(comando.params[10]);
        vgEEprom.celulas[indice].datarate = (TDataRate)atoi(comando.params[11]);

        //Serial.print("Indx: ");
        //Serial.println(indice);
        ValidarLimites(indice);
        configurar_celula(indice);

        break;

    case 20: //WP|20|xx activar celula por software

        indice = atoi(comando.params[3]);
        if (indice < 0 || indice > ID_MAXIMO_CELULA - 1)
            return 0;

        configurar_celula(indice);
        break;

    case 21: //WP|21|xx
        vg.salida_datos_continua_start = atoi(comando.params[3]);
        break;

    case 22: //WP|22|xx
        vg.modo_salida_datos_binario = atoi(comando.params[3]);
        break;

    case 90: //WP|99|xx texto de 10 caracteres
        strncpy(vgEEprom.id_maquina, comando.params[3], strlen(comando.params[3]));
        break;

    case 999: 
        SetValorPordefecto();
        salvar_valores();
        //Serial.println(F("FACTORY_RESET_OK"));
        break;    

    default:
        break;
    }

    return 1;
}

