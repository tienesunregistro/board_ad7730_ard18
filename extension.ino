// Unidades extension
#define UE_MM 0
#define UE_IN 1

void inicializar_encoders(void)
{
    //vg.PasoHusillo = vgEEprom.PasoHusillo;
    //vg.PasosEncoder = vgEEprom.PasosEncoder;
    //vg.IPE = vgEEprom.IPE;
    LS7366_Inic();
    RecalcularCoeficientes();
}

// Lec. extension en mm
float LecturaExtension(void)
{
    float lectura;
    long pasos = leer_encoder();
    //pasos = vg.pasosEncoder;
    lectura = pasos_a_deformacion(pasos, UE_MM);
    return lectura;
}

float pasos_a_deformacion(long valor, int unidad)
{
    float res;

    switch (unidad)
    {
    default:
        res = 9999;
        break;
    case UE_MM:
        res = valor * vg.CCE_MM * vgEEprom.IPE;
        break;
    case UE_IN:
        res = valor * vg.CCE_IN * vgEEprom.IPE;
        break;
    }

    res = ((long)(res / 0.001)) * 0.001;
    return res;
}
