#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdlib.h>
#include <string.h>

typedef struct sNodo {
    void *info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

typedef tNodo *tLista;

void crearLista(tLista *l);
int ponerAlComienzo(tLista *l, const void *d, unsigned cantBytes);
int sacarPrimeroLista(tLista *l, void *d, unsigned cantBytes);
int sacarUltimoLista(tLista *l, void *d, unsigned cantBytes);
int sacarPorClave(tLista *l, void *d, unsigned cantBytes, const void *clave, int (*cmp)(const void *, const void *));
void vaciarLista(tLista *l);

#endif // LISTA_H_INCLUDED
