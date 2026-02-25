

long filtroRC(long present_reading) {
  static long last_output = -999999; // Valor centinela
  
  if (last_output == -999999) { 
    last_output = present_reading; // La primera vez, saltamos la rampa
  }

  last_output = (7 * last_output + present_reading) / 8;
  return last_output;
}

//Filtro RC con "Punto Fijo"
long filtroRC2(long present_reading) {
  static double last_output = -999999.0;
  double alpha = 0.2; // Factor de suavizado (1.0 = sin filtro, 0.01 = filtro muy fuerte)

  if (last_output < -900000.0) { 
    last_output = (double)present_reading;
  }

  // Fórmula estándar EMA: last = last + alpha * (actual - last)
  last_output = last_output + alpha * ((double)present_reading - last_output);
  
  return (long)round(last_output);
}

// Filtro de Mediana (Ideal para ruidos bruscos)
long filtroMediana3(long nueva_lectura) {
  static long buf[3];
  buf[0] = buf[1];
  buf[1] = buf[2];
  buf[2] = nueva_lectura;

  // Devolvemos el valor central de los 3 (ordenación simple)
  long a = buf[0], b = buf[1], c = buf[2];
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}


//----------  Media movil

// Filtro media movil
// const int windowSize = 20;
// int32_t circularBuffer[windowSize];
// int32_t * pcircularBuffer = circularBuffer;

// void appendToBuffer(int32_t value)
// {
//   *pcircularBuffer = value;
//   pcircularBuffer++;
//   if (pcircularBuffer >= circularBuffer + windowSize) 
//     pcircularBuffer = circularBuffer;
// }


// int32_t sum;
// int elementCount;
// float mean;
// float filtroMediaMovil(int32_t value)
// {
//   sum -= *pcircularBuffer;
//   sum += value;
//   appendToBuffer(value);

//   if (elementCount < windowSize)
//     ++elementCount;
//   return ((float) (sum / elementCount));
// }



#define SMA_WINDOW 16 // Debe ser potencia de 2 para máxima velocidad

long filtroSMA(long nueva_lectura) {
    static long buffer[SMA_WINDOW];  // Array para almacenar las muestras
    static int indice = 0;           // Puntero al dato más antiguo
    static long long sumaTotal = 0;  // Usamos long long para evitar desbordamiento
    static bool bufferLleno = false;

    // Inicialización: Evita que la lectura empiece en 0 y suba como rampa
    if (!bufferLleno) {
        for (int i = 0; i < SMA_WINDOW; i++) {
            buffer[i] = nueva_lectura;
        }
        sumaTotal = (long long)nueva_lectura * SMA_WINDOW;
        bufferLleno = true;
    }

    // Algoritmo de Media Móvil Eficiente:
    // 1. Restamos el valor más antiguo que va a salir del buffer
    sumaTotal -= buffer[indice];
    
    // 2. Metemos el nuevo valor en esa posición
    buffer[indice] = nueva_lectura;
    
    // 3. Sumamos el nuevo valor al total
    sumaTotal += buffer[indice];

    // 4. Desplazamos el índice de forma circular
    indice++;
    if (indice >= SMA_WINDOW) indice = 0;

    // 5. Devolvemos el promedio
    // Como 16 es potencia de 2, el compilador usará un shift (>> 4) en lugar de división
    return (long)(sumaTotal / SMA_WINDOW);
}