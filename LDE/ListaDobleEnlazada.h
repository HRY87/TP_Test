#ifndef LISTADOBLEENLAZADA_H_INCLUDED
#define LISTADOBLEENLAZADA_H_INCLUDED

#include <stdlib.h>
#include <string.h>

typedef struct sNodoD {
    void *info;
    unsigned tamInfo;
    struct sNodoD *sig;
    struct sNodoD *ant;
} tNodoD;

typedef struct {
    tNodoD *primero;
    tNodoD *ultimo;
} tListaDoble;

void crearListaDoble(tListaDoble *l);
void vaciarListaDoble(tListaDoble *l);
void* ponerAlComienzoLDE_Ref(tListaDoble *l, const void *d, unsigned cantBytes);
int sacarUltimoLDE(tListaDoble *l, void *d, unsigned cantBytes);
int eliminarNodoLDE(tListaDoble *l, void *refNodo);
int moverAlPrincipioLDE(tListaDoble *l, void *refNodo);
void* obtenerInfoNodoLDE(void *refNodo);

#endif // LISTADOBLEENLAZADA_H_INCLUDED
