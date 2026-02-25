
void cero_adlc(void)
{
  //se hace un cero virtual
  //AD7730_cero_adlc();
}

void inicializar_adlc(void)
{
  AD730_Inic();
}

void reset_adlc(void)
{
  AD7730_Reset();
}


long leer_adlc(void)
{
  static long old_dataconvert;
  TDatoCanal datoCanal;
  long dataconvert;
  int estado = 0;

  if (IsDataAvailable())
  {
    estado = Buffer_read(&datoCanal);
    dataconvert = datoCanal.dato;
    old_dataconvert = dataconvert;
  }
  else
  {
    dataconvert = old_dataconvert;
  }

  if (vg.pol_celula < 0)
    dataconvert = -dataconvert;

  vg.dac_CH1 = dataconvert;
  vg.dac_filtrado_CH1 = filtroRC(dataconvert);

  // detectar maximo en el tramo
  if (vg.dac_CH1 > vg.max_dac_tramo_CH1)
  {
    vg.max_dac_tramo_CH1 = vg.dac_CH1;
  }

  // detectar maximo en el tramo filtrado
  if (vg.dac_filtrado_CH1 > vg.max_dac_tramo_filtrado_CH1)
  {
    vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1;
  }

  // Maximo acumulado
  if (vg.dac_CH1 > vg.max_dac_CH1)
  {
    vg.max_dac_CH1 = vg.dac_CH1;
  }

  return (vg.dac_filtrado_CH1);
}

long leer_adlc_xx(void)
{
  static long old_dataconvert;
  TDatoCanal datoCanal;
  long dataconvert;
  int estado = 0;

  if (IsDataAvailable())
  {
    estado = Buffer_read(&datoCanal);
    if (estado > 0)
    {
      dataconvert = datoCanal.dato;
    }
    else
    {
      dataconvert = old_dataconvert;
    }
    old_dataconvert = dataconvert;
  }
  else
  {
    dataconvert = old_dataconvert;
  }

  if (vg.pol_celula < 0)
    dataconvert = -dataconvert;

  vg.dac_CH1 = dataconvert;
  vg.dac_filtrado_CH1 = filtroRC(dataconvert);

  // detectar maximo en el tramo
  if (vg.dac_CH1 > vg.max_dac_tramo_CH1)
  {
    vg.max_dac_tramo_CH1 = vg.dac_CH1;
  }

  // detectar maximo en el tramo filtrado
  if (vg.dac_filtrado_CH1 > vg.max_dac_tramo_filtrado_CH1)
  {
    vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1;
  }

  // Maximo acumulado
  if (vg.dac_CH1 > vg.max_dac_CH1)
  {
    vg.max_dac_CH1 = vg.dac_CH1;
  }

  return (vg.dac_filtrado_CH1);
}
/*
void setganancia_pga(int gain)
{

  //AD7730_setganancia_pga(gain);
}
*/