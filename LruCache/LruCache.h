#ifndef LRUCACHE_H_INCLUDED
#define LRUCACHE_H_INCLUDED

#include "../Arbol/Arbol.h"
#include "../LDE/ListaDobleEnlazada.h"

typedef int (*tCmp)(const void *clave1, const void *clave2);

typedef struct {
    void *clave;
    void *valor;
    unsigned tamValor;
} tDatoCache_indice;

typedef struct {
    void *clave;
    void *refNodoLista;
    tCmp cmp;
} tIndiceArbol;

typedef struct {
    tListaDoble lista;
    tArbolBinBusq arbol;
    unsigned cantidad;
    unsigned capacidad;
    unsigned tamClave;
    tCmp cmp;
} tLruCache;

void crear_lrucache_indice(tLruCache *c, unsigned capacidad, unsigned tamClave, tCmp cmp);
int agregar_lrucache_indice(tLruCache *c, const void *clave, const void *valor, unsigned tamValor);
int obtener_lrucache_indice(tLruCache *c, const void *clave, void *valor, unsigned tamValor);
int borrar_lrucache_indice(tLruCache *c, const void *clave);
int vaciar_lrucache_indice(tLruCache *c);
unsigned altura_arbol_indice(tLruCache *c);

#endif // LRUCACHE_H_INCLUDED
