#define UF_N 0
#define UF_KN 1
#define UF_K 2
#define UF_LB 3

/*
#define CCF_N  = 1.0f
#define  CCF_KN  1.0f / 1000.0f
#define  CCF_K   1.0f / 9.81f
#define  CCF_LB  1.0f * 0.225f
*/

void inicializar_celulas(void)
{
  vg.id_celula = identificar_celula();

  configurar_celula(vg.id_celula);

  //Serial.println("inicializar_celulas END");
}

//Detección de celula en A0

// Identificamos 5 células
/*
Con RA = 4K7 conectada a +5 V y a A0, Rx conecta A0 y GND
Cod.
CEL 	       Vmin 	Vnominal               Vmax     Rx
-----------------------------------------------------------
0		            0      0.0   	               0.2    0
1		            0.2	   1.0			             1.5    1K2
2		            1.5	   2.0              		 2.5    3K
3		            2.5	   3.0			             3.5    7K
4		            3.5	   4.0			             5.0    22K
*/

// valores de 0 a ID_MAXIMO_CELULA-1

int identificar_celula(void)
{
  float m_v;
  int pasos;

  pasos = analogRead(A0);

  m_v = pasos * (5000.0 / 1023.0);

  if (m_v < 200)
  {
    alarma_celula_off();
    return 0; // cod. 0
  }

  if (m_v > 200 && m_v <= 1500)
  {
    alarma_celula_off();
    return 1;
  }

  if (m_v > 1500 && m_v <= 2500)
  {
    alarma_celula_off();
    return 2;
  }

  if (m_v > 2500 && m_v <= 3500)
  {
    alarma_celula_off();
    return 3;
  }

  if (m_v > 3500)
  {
    alarma_celula_off();
    return 4;
  }

  return 0;
}

float capacidad_celula_newtons(int id)
{

  if (id < 0 || id >= ID_MAXIMO_CELULA)
    return 0;

  return vgEEprom.celulas[id].cap;
}

float resolucion_celula_newtons(int id)
{

  if (id < 0 || id >= ID_MAXIMO_CELULA)
    return 0;

  return vgEEprom.celulas[id].res;
}

void configurar_celula(int i)
{

  if (i < ID_MAXIMO_CELULA)
  { // celula configurada manualmente
    vg.id_celula = i;
    vg.cap_celula = vgEEprom.celulas[i].cap < 0.001 ? 1 : vgEEprom.celulas[i].cap;
    vg.res_celula = vgEEprom.celulas[i].res < 0.0001 ? 1 : vgEEprom.celulas[i].res;
    vg.pol_celula = vgEEprom.celulas[i].pol;
    vg.gan_celulaPos = vgEEprom.celulas[i].gainpasostoFPos > 0.01 ? vgEEprom.celulas[i].gainpasostoFPos : 1;
    vg.gan_celulaNeg = vgEEprom.celulas[i].gainpasostoFNeg > 0.01 ? vgEEprom.celulas[i].gainpasostoFNeg : 1;
    vg.limite_carga_celPos = vgEEprom.celulas[i].limite_carga_celP < 0.0001 ? 1 : vgEEprom.celulas[i].limite_carga_celP;
    vg.limite_carga_celNeg = vgEEprom.celulas[i].limite_carga_celN < 0.0001 ? 1 : vgEEprom.celulas[i].limite_carga_celN;
    vg.datarate_celula = vgEEprom.celulas[i].datarate < 1 ? 150 : vgEEprom.celulas[i].datarate;

    vg.pasos_limite_carga_celPos = fuerza_a_pasos(vg.limite_carga_celPos, UF_N);
    vg.pasos_limite_carga_celNeg = fuerza_a_pasos(vg.limite_carga_celNeg, UF_N);
  }
  else
  {
    vg.id_celula = 0;
    vg.cap_celula = 1;
    vg.res_celula = 1;
    vg.gan_celulaPos = 1.0;
    vg.gan_celulaNeg = 1.0;
    vg.limite_carga_celPos = 1.0;
    vg.limite_carga_celNeg = 1.0;
    vg.pasos_limite_carga_celPos = 1.0;
    vg.pasos_limite_carga_celNeg = 1.0;
    vg.pol_celula = 1;
    vg.datarate_celula = 150;
  }

  RecalcularCoeficientes();

  /*
    Serial.println(vg.id_celula);
    Serial.println(vg.cap_celula, 3);
    Serial.println(vg.res_celula, 4);
    Serial.println(vg.gan_celulaPos, 3);
    Serial.println(vg.gan_celulaNeg, 3);
    Serial.println(vg.limite_carga_celPos, 3);
    Serial.println(vg.limite_carga_celNeg, 3);

    Serial.println(vg.pasos_limite_carga_celPos);
    Serial.println(vg.pasos_limite_carga_celNeg);
    Serial.println(vg.pol_celula);
    Serial.println(vg.datarate_celula);
  */
}

float pasos_a_fuerza(long valor, int unidad)
{
    double res; // Usamos double para mayor precisión en cálculos intermedios
    float IPF;
    float res_fuerza = vg.res_celula;

    // Selección de ganancia por polaridad
    // Aseguramos que IPF no sea 0 para evitar divisiones por cero (bloqueo)
    if (valor >= 0) {
        IPF = (vg.gan_celulaPos > 0) ? vg.gan_celulaPos : 1.0f;
    } else {
        IPF = (vg.gan_celulaNeg > 0) ? vg.gan_celulaNeg : 1.0f;
    }

    // Cálculo base según unidad
    // Multiplicamos por el coeficiente de conversión
    switch (unidad)
    {
        case UF_N:   res = (double)valor * vg.CCF_N; break;
        case UF_KN:  res = (double)valor * vg.CCF_KN; break;
        case UF_K:   res = (double)valor * vg.CCF_K; break;
        case UF_LB:  res = (double)valor * vg.CCF_LB; break;
        default:     return 9999.0f;
    }

    // Aplicar ganancia de calibración
    res = res / (double)IPF;

    // Redondeo a la resolución configurada (CRÍTICO)
    // En lugar de truncar con (long), redondeamos al múltiplo más cercano
    if (res_fuerza > 0.0f) {
        // Ejemplo: Si res=4.97 y res_fuerza=0.1 -> round(49.7) * 0.1 = 5.0
        res = round(res / (double)res_fuerza) * (double)res_fuerza;
    }

    return (float)res;
}

long fuerza_a_pasos(float valor, int unidad)
{
  long res;

  float IPF;

  if (valor >= 0)
  {
    IPF = vg.gan_celulaPos;
  }
  else
  {
    IPF = vg.gan_celulaNeg;
  }

  switch (unidad)
  {
  default:
    res = 9999;
    break;
  case UF_N:
    res = (long)(valor / vg.CCF_N * IPF);
    break;
  case UF_KN:
    res = (long)(valor / vg.CCF_KN * IPF);
    break;
  case UF_K:
    res = (long)(valor / vg.CCF_K * IPF);
    break;
  case UF_LB:
    res = (long)(valor / vg.CCF_LB * IPF);
    break;
  }
  /*
  Serial.println(valor,3);
  Serial.println(IPF,3);
  Serial.println(res);
  */
  return res;
}

// Coeficientes para el paso de unidades de N a otras unidades
void RecalcularCoeficientes(void)
{
  vg.CCF_N = 1;
  vg.CCF_KN = 1 / 1000.0;
  vg.CCF_K = 1 / 9.81;
  vg.CCF_LB = 1 * 0.225;

  vg.CCE_MM = vgEEprom.PasoHusillo / vgEEprom.PasosEncoder;
  vg.CCE_IN = (vgEEprom.PasoHusillo / vgEEprom.PasosEncoder) * 0.039;
}

// Validar los límites de la config. de la celula
void ValidarLimites(int i)
{
  if (i < ID_MAXIMO_CELULA)
  { // celula configurada manualmente
    vgEEprom.celulas[i].cap = vgEEprom.celulas[i].cap < 0.0001 ? 1.0 : vgEEprom.celulas[i].cap;
    vgEEprom.celulas[i].res = vgEEprom.celulas[i].res < 0.0001 ? 1.0 : vgEEprom.celulas[i].res;
    vgEEprom.celulas[i].pol = (vgEEprom.celulas[i].pol >= -1 && vgEEprom.celulas[i].pol <= 1) ? vgEEprom.celulas[i].pol : 1;

    vgEEprom.celulas[i].gainpasostoFPos = vgEEprom.celulas[i].gainpasostoFPos < 0.01 ? 1.0 : vgEEprom.celulas[i].gainpasostoFPos;
    vgEEprom.celulas[i].gainpasostoFNeg = vgEEprom.celulas[i].gainpasostoFNeg < 0.01 ? 1.0 : vgEEprom.celulas[i].gainpasostoFNeg;
    vgEEprom.celulas[i].limite_carga_celP = (vgEEprom.celulas[i].limite_carga_celP < 0.01 || vgEEprom.celulas[i].limite_carga_celP > vgEEprom.celulas[i].cap) ? vgEEprom.celulas[i].cap : vgEEprom.celulas[i].limite_carga_celP;
    vgEEprom.celulas[i].limite_carga_celN = (vgEEprom.celulas[i].limite_carga_celN < 0.01 || vgEEprom.celulas[i].limite_carga_celN > vgEEprom.celulas[i].cap) ? vgEEprom.celulas[i].cap : vgEEprom.celulas[i].limite_carga_celN;
    if ((vgEEprom.celulas[i].datarate != 150) && (vgEEprom.celulas[i].datarate != 300) && (vgEEprom.celulas[i].datarate != 600) && (vgEEprom.celulas[i].datarate != 1000) && (vgEEprom.celulas[i].datarate != 1200))
    {
      vgEEprom.celulas[i].datarate = DATA_RATE_300; //150
    }
  }
}

//Lectura de fuerza en pasos sin compensación de cero y filtrada
long LecturaFuerzaPasosFiltrada()
{
  long value;

  //value = leer_adlc();
  value = vg.dac_filtrado_CH1;

  //Serial.println(value);
  return value;
}

// en unidades de fuerza N
float LecturaFuerzaFiltrada()
{
  long value;
  float datoF;

  value = LecturaFuerzaPasosFiltrada();
  value = value - vg.Cero_canal1;

  datoF = pasos_a_fuerza(value, UF_N);

  //Serial.println (datoF, 3);
  return datoF;
}

//Lectura de fuerza en pasos sin compensación de cero y filtrada
long LecturaFuerzaPasosNoFiltrada()
{
  long value;

  //value = leer_adlc();
  value = vg.dac_CH1;

  //Serial.println(value);
  return value;
}

// en unidades de fuerza N
float LecturaFuerzaNoFiltrada()
{
  long value;
  float datoF;

  value = LecturaFuerzaPasosNoFiltrada();
  value = value - vg.Cero_canal1;

  datoF = pasos_a_fuerza(value, UF_N);

  //Serial.println (datoF, 3);
  return datoF;
}

// en unidades de fuerza N
float LecturaFuerzaMaximaTramo()
{
  long value;
  float datoF;
  
  noInterrupts();
  value = vg.max_dac_tramo_CH1 - vg.Cero_canal1;
  vg.max_dac_tramo_CH1 = vg.dac_CH1; // reseteo el pico
  interrupts();
  datoF = pasos_a_fuerza(value, UF_N);
  
  //Serial.println (datoF, 3);
  return datoF;
}

// en unidades de fuerza N
float LecturaFuerzaMaximaTramoFiltrada()
{
  long value;
  float datoF;
  
  noInterrupts();
  value = vg.max_dac_tramo_filtrado_CH1 - vg.Cero_canal1;
  vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1; // reseteo el pico
  interrupts();
  datoF = pasos_a_fuerza(value, UF_N);

  //Serial.println (datoF, 3);
  return datoF;
}

// en unidades de fuerza N
float LecturaFuerzaMaxima()
{
  long value;
  float datoF;

  noInterrupts();
  value = vg.max_dac_CH1 - vg.Cero_canal1;
  vg.max_dac_CH1 = vg.dac_CH1; // reseteo el pico
  interrupts();
  datoF = pasos_a_fuerza(value, UF_N);
  
  //Serial.println (datoF, 3);
  return datoF;
}

void fuerza_Max_Reset() {
  noInterrupts();
  vg.max_dac_CH1 = vg.dac_CH1;
  vg.max_dac_tramo_CH1 = vg.dac_CH1;
  vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1;
  interrupts();
}

//F. Cero canal 1
void fuerza_cero(void)
{
  noInterrupts();
  vg.Cero_canal1 = vg.dac_filtrado_CH1; // Captura el cero
  vg.max_dac_CH1 = vg.dac_CH1;          // Reset inmediato de picos
  vg.max_dac_tramo_CH1 = vg.dac_CH1;
  vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1;
  interrupts();
}
