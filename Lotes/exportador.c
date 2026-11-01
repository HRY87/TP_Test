#include <stdlib.h>
#include <string.h>
#include "../Lotes/exportador.h"
#include "../SimuladorTweener/registros.h"

/* Arma en txt el nombre de bin cambiando la extension por .txt; 0 si no entra */
static int armarNombreTexto(const char *bin, char *txt, size_t tam)
{
    const char *punto = strrchr(bin, '.');
    size_t largo = punto ? (size_t)(punto - bin) : strlen(bin);

    if(largo + 5 > tam)                 /* ".txt" y el terminador */
        return 0;

    memcpy(txt, bin, largo);
    strcpy(txt + largo, ".txt");

    return 1;
}

/* Lee el binario registro a registro y escribe una linea de texto por cada uno */
int exportarATexto(const char *nombreBin, unsigned tamRegistro, tFormatoRegistro formatear)
{
    char nombreTxt[64];
    FILE *pb = NULL;
    FILE *pt = NULL;
    void *reg = malloc(tamRegistro);
    long numero = 0;
    int res = SIM_OK;

    if(!reg)
        res = SIM_SIN_MEM;
    else if(!armarNombreTexto(nombreBin, nombreTxt, sizeof(nombreTxt)))
        res = SIM_PARAM_INVALIDO;
    else
    {
        pb = fopen(nombreBin, "rb");
        pt = pb ? fopen(nombreTxt, "w") : NULL;

        if(!pt)
            res = SIM_ERROR_ARCHIVO;
    }

    while(res == SIM_OK && fread(reg, tamRegistro, 1, pb) == 1)
    {
        formatear(pt, reg, numero);
        numero++;
    }

    if(pb)
        fclose(pb);

    if(pt)
        fclose(pt);

    free(reg);

    return res;
}

/* Una linea por posteo: posicion, id y texto */
void formatearPosteo(FILE *pt, const void *registro, long numero)
{
    const tPosteo *post = registro;

    fprintf(pt, "%6ld | id %6d | %.*s\n", numero, post->id, LARGO_TEXTO, post->texto);
}

/* Una linea por usuario: posicion, nombre y los ids de su feed */
void formatearUsuario(FILE *pt, const void *registro, long numero)
{
    const tUsuario *usu = registro;
    const int *p;

    fprintf(pt, "%6ld | %.*s |", numero, LARGO_USUARIO, usu->usuario);

    for(p = usu->feed; p < usu->feed + LARGO_FEED; p++)
        fprintf(pt, " %d", *p);

    fputc('\n', pt);
}
