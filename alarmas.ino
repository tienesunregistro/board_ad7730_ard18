// Define 16 alarmas como máximo 0..15
#define a_celula 0    // no detecta ninguna célula
#define a_fuerzapos 1 // sobrecarga pos
#define a_fuerzaneg 2 // sobrecarga neg
#define a_cero 3      // cero superior al 20 por ciento de la fmax
#define a_fuerza 4    // sobrecarga pos o neg

uint8_t _alarmas_;
uint8_t _status_;

#define alarma_celula_on() bitSet(_alarmas_, a_celula)
#define alarma_celula_off() bitClear(_alarmas_, a_celula)
#define alarma_celula() bitRead(_alarmas_, a_celula)

#define alarma_fuerzapos_on() bitSet(_alarmas_, a_fuerzapos)
#define alarma_fuerzapos_off() bitClear(_alarmas_, a_fuerzapos)
#define alarma_fuerzapos() bitRead(_alarmas_, a_fuerzapos)

#define alarma_fuerzaneg_on() bitSet(_alarmas_, a_fuerzaneg)
#define alarma_fuerzaneg_off() bitClear(_alarmas_, a_fuerzaneg)
#define alarma_fuerzaneg() bitRead(_alarmas_, a_fuerzaneg)

#define alarma_fuerza_on() bitSet(_alarmas_, a_fuerza)
#define alarma_fuerza_off() bitClear(_alarmas_, a_fuerza)
#define alarma_fuerza() bitRead(_alarmas_, a_fuerza)

#define HYSTERESIS_FACTOR 0.05

void check_alarmas()
{
  float lFuerza = LecturaFuerzaFiltrada();
  float limPos = fabs(vg.limite_carga_celPos);
  float limNeg = fabs(vg.limite_carga_celNeg);
  float limPosOff = limPos * (1.0 - HYSTERESIS_FACTOR);
  float limNegOff = limNeg * (1.0 - HYSTERESIS_FACTOR);

  if (!alarma_fuerzapos() && (lFuerza > 0.0) && (fabs(lFuerza) > limPos))
  {
    alarma_fuerzapos_on();
  }
  else if (alarma_fuerzapos() && (fabs(lFuerza) < limPosOff))
  {
    alarma_fuerzapos_off();
  }

  if (!alarma_fuerzaneg() && (lFuerza < 0.0) && (fabs(lFuerza) > limNeg))
  {
    alarma_fuerzaneg_on();
  }
  else if (alarma_fuerzaneg() && (fabs(lFuerza) < limNegOff))
  {
    alarma_fuerzaneg_off();
  }

  alarma_fuerza_off();
  if (alarma_fuerzapos() || alarma_fuerzaneg())
  {
    alarma_fuerza_on();
  }

  if (alarma_fuerzapos())
  {
    ALARMA_FUERZA_POST_ON();
  }
  else
  {
    ALARMA_FUERZA_POST_OFF();
  }

  if (alarma_fuerzaneg())
  {
    ALARMA_FUERZA_NEG_ON();
  }
  else
  {
    ALARMA_FUERZA_NEG_OFF();
  }
}

void mostrar_alarmas(void)
{

  if (!hay_alarmas())
  {
    return;
  }
}

void inicializar_alarmas(void)
{
  _alarmas_ = 0;
  _status_ = 0;
}

bool hay_alarmas(void)
{
  return _alarmas_;
}

uint8_t get_alarmas(void)
{
  return _alarmas_;
}

uint8_t get_status(void)
{
  return _status_;
}
