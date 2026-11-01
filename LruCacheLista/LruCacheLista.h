#ifndef LRUCACHELISTA_H_INCLUDED
#define LRUCACHELISTA_H_INCLUDED

#include "../Lista/Lista.h"

typedef int (*tCmp)(const void *clave1, const void *clave2);

typedef struct {
    tLista lista;
    unsigned cantidad;
    unsigned capacidad;
    unsigned tamClave;
    tCmp cmp;
} tLruCacheLista;

typedef struct {
    void * clave;
    void * valor;
    unsigned tamValor;
} tDatoCache;

void crear_lrucache(tLruCacheLista *c, unsigned capacidad, unsigned tamClave, tCmp cmp);
int agregar_lrucache(tLruCacheLista *c, const void *clave, const void *valor, unsigned tamValor);
int obtener_lrucache(tLruCacheLista *c, const void *clave, void *valor, unsigned tamValor);
int borrar_lrucache(tLruCacheLista *c, const void *clave);
int vaciar_lrucache(tLruCacheLista *c);

#endif // LRUCACHELISTA_H_INCLUDED
