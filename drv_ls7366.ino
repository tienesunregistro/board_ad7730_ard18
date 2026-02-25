//Comandos para el reg. de instruccion
#define SELECT_MDR0 0x88 //escribir en el reg MDR0
#define SELECT_MDR1 0x90 //escribir en el reg MDR1
#define READ_CNTR 0x60
#define LOAD_DTR 0x98
#define LOAD_CNTR 0xE0
#define READ_MDR0 0x48
#define READ_MDR1 0x50

#define CONF_MDR0 0x01 //cuadratura x1 free running
#define CONF_MDR1 0X00 //32 BITS

#define CERO_ENCODER (long)(1L << 31)
//#define CERO_ENCODER         0xDF0000UL

#define COUNT_MODE_X4 0x03
#define COUNT_MODE_X2 0x02
#define COUNT_MODE_X1 0x01

static SPISettings spi_ls7366(2000000, MSBFIRST, SPI_MODE0); //8000000, 4000000, 2000000 or 1000000

void LS7366_Inic(void)
{
    LS7366_SetMultiplicador(vgEEprom.count_mode);
    noInterrupts();
    SPI.beginTransaction(spi_ls7366);
    CS_LS7366_LOW(); // cs=0
    SPI.transfer(SELECT_MDR1);
    SPI.transfer(CONF_MDR1);
    CS_LS7366_HIGH();
    SPI.endTransaction();
    interrupts();
    cero_encoder();
}

void LS7366_SetMultiplicador(byte multiplicador)
{
    byte MDR0;

    switch (multiplicador)
    {
    case 1:
        MDR0 = COUNT_MODE_X1;
        break;
    case 2:
        MDR0 = COUNT_MODE_X2;
        break;
    case 4:
        MDR0 = COUNT_MODE_X4;
        break;
    default:
        MDR0 = COUNT_MODE_X1;
    }
    noInterrupts();
    SPI.beginTransaction(spi_ls7366);
    CS_LS7366_LOW(); // cs=0
    SPI.transfer(SELECT_MDR0);
    SPI.transfer(MDR0);
    CS_LS7366_HIGH();
    SPI.endTransaction();
    interrupts();
}

void cero_encoder(void)
{
    escribir_encoder(CERO_ENCODER);
}

void escribir_encoder(unsigned long valor)
{
    unsigned long bTmp[4];

    noInterrupts();

    SPI.beginTransaction(spi_ls7366);
    CS_LS7366_LOW(); // cs=0

    SPI.transfer(LOAD_DTR);

    bTmp[0] = valor & 0x000000ff;
    bTmp[1] = (valor >> 8) & 0x000000ff;
    bTmp[2] = (valor >> 16) & 0x000000ff;
    bTmp[3] = (valor >> 24) & 0x000000ff;

    SPI.transfer(bTmp[3]);
    SPI.transfer(bTmp[2]);
    SPI.transfer(bTmp[1]);
    SPI.transfer(bTmp[0]);

    CS_LS7366_HIGH();
    SPI.endTransaction();

    SPI.beginTransaction(spi_ls7366);
    CS_LS7366_LOW();
    SPI.transfer(LOAD_CNTR);
    CS_LS7366_HIGH();
    SPI.endTransaction();

    interrupts();
}

long leer_encoder()
{
    // Initialize temporary variables for SPI read
    unsigned int count_1, count_2, count_3, count_4;
    long count_value;
    
    noInterrupts();
  
    SPI.beginTransaction(spi_ls7366);
    CS_LS7366_LOW();
    SPI.transfer(READ_CNTR);   // peticion de lectura
    count_1 = SPI.transfer(0); // byte de mas peso
    count_2 = SPI.transfer(0);
    count_3 = SPI.transfer(0);
    count_4 = SPI.transfer(0); // byte de menos peso
    CS_LS7366_HIGH();
    SPI.endTransaction();
    
    interrupts();
  
    
    // Calculate encoder count
    count_value = (count_1 << 8) + count_2;
    count_value = (count_value << 8) + count_3;
    count_value = (count_value << 8) + count_4;

    count_value = CERO_ENCODER - count_value;
    //Serial.println(count_value);
    vg.PosEncoder = count_value;
    return count_value;
}
