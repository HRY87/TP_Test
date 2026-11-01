#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#define TODO_BIEN 1
#define SIN_MEM 0
#define CLA_DUP 2

typedef struct sNodoArbol {
    void *info;
    unsigned tamInfo;
    struct sNodoArbol *izq;
    struct sNodoArbol *der;
} tNodoArbol;

typedef tNodoArbol *tArbolBinBusq;

void crearArbolBinBusq(tArbolBinBusq *p);
void eliminarArbol(tArbolBinBusq *pa);
int insertarArbolBinBusq(tArbolBinBusq *p, const void *d, unsigned tam, int (*cmp)(const void*, const void*));
int buscarElemArbolBinBusq(const tArbolBinBusq *p, void *d, unsigned tam, int (*cmp)(const void*, const void*));
int eliminarElemArbolBinBusq(tArbolBinBusq *p, void *d, unsigned tam, int (*cmp)(const void*, const void*));
unsigned alturaArbolBin(const tArbolBinBusq *p);

#endif // ARBOL_H_INCLUDED
