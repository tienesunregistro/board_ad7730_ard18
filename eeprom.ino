#include <EEPROM.h>

// Calcula un checksum XOR simple para asegurar la integridad de los datos
uint16_t calcularChecksum(TVarEEprom *datos) {
    uint16_t checksum = 0;
    uint8_t *p = (uint8_t *)datos;
    // Calculamos sobre toda la estructura excepto el campo checksum (los últimos 2 bytes)
    for (unsigned int i = 0; i < sizeof(TVarEEprom) - sizeof(uint16_t); i++) {
        checksum += p[i];
    }
    return checksum;
}

void SetValorPordefecto(void)
{
    memset(&vgEEprom, 0, sizeof(TVarEEprom)); // Limpiar todo primero
    
    vgEEprom.clave = CLAVE_VERSION;
    vgEEprom.idEstacion = ID_ESTACION;
    vgEEprom.IPE = 1;
    vgEEprom.count_mode = 1;
    vgEEprom.PasoHusillo = 1.0;
    vgEEprom.PasosEncoder = 1.0;
    
    for (int i = 0; i < ID_MAXIMO_CELULA; i++)
    {
        vgEEprom.celulas[i].idcel = i;
        vgEEprom.celulas[i].cap = 1.0;
        vgEEprom.celulas[i].pol = 1;
        vgEEprom.celulas[i].res = 1.0;
        vgEEprom.celulas[i].limite_carga_celP = 1.0;
        vgEEprom.celulas[i].limite_carga_celN = 1.0;
        vgEEprom.celulas[i].gainpasostoFPos = 1.0;
        vgEEprom.celulas[i].gainpasostoFNeg = 1.0;
        vgEEprom.celulas[i].datarate = DATA_RATE_300;
    }

    vgEEprom.filtro_on_off = 1;    
    vgEEprom.gainpos = 1.0;
    vgEEprom.gainneg = 1.0;
    strncpy(vgEEprom.id_maquina, "NUEVA", 14);

    // No calculamos el checksum aquí, se hará al salvar
}

void recuperar_valores_eeprom(void)
{
    // 1. Leer la estructura completa
    EEPROM.get(0, vgEEprom);
    
    // 2. Verificar Clave de Versión
    if (vgEEprom.clave != CLAVE_VERSION) {
        Serial.println(F("EEPROM: Clave incorrecta. Cargando defaults..."));
        SetValorPordefecto();
        salvar_valores();
        return;
    }

    // 3. Verificar Integridad (Checksum)
    uint16_t checksumCalculado = calcularChecksum(&vgEEprom);
    if (vgEEprom.checksum != checksumCalculado) {
        Serial.println(F("EEPROM: Error de Checksum! Datos corruptos."));
        SetValorPordefecto();
        // No salvamos automáticamente para permitir al usuario revisar si fue un error puntual
    } else {
        Serial.println(F("EEPROM: Datos cargados correctamente."));
    }
}

void salvar_valores(void)
{
    // 1. Actualizar el checksum antes de guardar
    vgEEprom.checksum = calcularChecksum(&vgEEprom);
    
    // 2. EEPROM.put utiliza internamente "update", lo que significa que
    // solo sobrescribe los bytes que hayan cambiado físicamente.
    // Esto alarga la vida de la EEPROM drásticamente.
    EEPROM.put(0, vgEEprom);
    
    Serial.println(F("EEPROM: Configuración salvada."));
}

void FormatearEEPROM()
{
    // No es necesario llenar de ceros, simplemente invalidamos la clave
    vgEEprom.clave = 0;
    EEPROM.put(0, vgEEprom);
    Serial.println(F("EEPROM: Formateada (clave invalidada)."));
}