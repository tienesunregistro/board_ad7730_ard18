

void store_char(TDatoCanal *pdato, data_buffer *buffer);

void Buffer_init(data_buffer *buffer)
{
  pdatos_buffer = buffer;
  pdatos_buffer->head = 0;
  pdatos_buffer->tail = 0;
}

void store_char(TDatoCanal *pdato, data_buffer *buffer)
{
  // Calculamos la siguiente posición usando AND en lugar de MODULO
  unsigned int next_head = (buffer->head + 1) & BUFFER_MASK;

  if (next_head != buffer->tail)
  {
    buffer->buffer[buffer->head] = *pdato;
    buffer->head = next_head;
  }
  // Si está lleno, el dato se descarta
}

int Buffer_read(TDatoCanal *dato)
{
  noInterrupts(); // Protección atómica para head y tail (16 bits)

  if (pdatos_buffer->head == pdatos_buffer->tail)
  {
    interrupts();
    return -1; // Buffer vacío
  }
  
  *dato = pdatos_buffer->buffer[pdatos_buffer->tail];
  pdatos_buffer->tail = (pdatos_buffer->tail + 1) & BUFFER_MASK;
  
  interrupts();
  return 1;
}

uint16_t IsDataAvailable(void)
{
  uint16_t count;
  noInterrupts();
  // La magia de las potencias de 2: 
  // (head - tail) & mask funciona incluso si head dio la vuelta (wrap-around)
  count = (pdatos_buffer->head - pdatos_buffer->tail) & BUFFER_MASK;
  interrupts();
  return count;
}

void Buffer_Flush() {
  noInterrupts();
  pdatos_buffer->head = 0;
  pdatos_buffer->tail = 0;
  // llenar de ceros para estar seguros
  memset((void*)pdatos_buffer->buffer, 0, sizeof(pdatos_buffer->buffer));
  interrupts();
}