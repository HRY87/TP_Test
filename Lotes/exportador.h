#ifndef EXPORTADOR_H_INCLUDED
#define EXPORTADOR_H_INCLUDED

#include <stdio.h>

/* Escribe en pt una linea de texto para el registro (numero parte de 0) */
typedef void (*tFormatoRegistro)(FILE *pt, const void *registro, long numero);

/* Lee nombreBin registro a registro y crea un .txt con el mismo nombre */
int exportarATexto(const char *nombreBin, unsigned tamRegistro, tFormatoRegistro formatear);

void formatearPosteo(FILE *pt, const void *registro, long numero);
void formatearUsuario(FILE *pt, const void *registro, long numero);

#endif // EXPORTADOR_H_INCLUDED
