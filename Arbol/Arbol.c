#include "Arbol.h"

#define reservarMemoriaNodo(X, Y, Z, W) ( ((X) = (typeof(X))malloc(Y)) == NULL || ((Z) = malloc(W)) == NULL ? free(X), 0 : 1 )
#define MINIMO(X, Y) ((X) <= (Y) ? (X) : (Y))

tNodoArbol ** buscarNodoArbolBinBusq(const tArbolBinBusq *p, const void *d, int (*cmp)(const void*, const void*));
tNodoArbol ** mayorNodoArbolBinBusq(const tArbolBinBusq *p);
tNodoArbol ** menorNodoArbolBinBusq(const tArbolBinBusq *p);
int eliminarRaizArbolBinBusq(tArbolBinBusq *p);

void crearArbolBinBusq(tArbolBinBusq *p)
{
    *p = NULL;
}

int insertarArbolBinBusq(tArbolBinBusq *p, const void *d, unsigned tam, int (*cmp)(const void*, const void*))
{
    tNodoArbol *nue;
    int rc;
    while (*p)
    {
        if ((rc = cmp(d, (*p)->info)) < 0) p = &(*p)->izq;
        else if (rc > 0) p = &(*p)->der;
        else return CLA_DUP;
    }
    if (!reservarMemoriaNodo(nue, sizeof(tNodoArbol), nue->info, tam)) return SIN_MEM;
    nue->tamInfo = tam;
    memcpy(nue->info, d, tam);
    nue->der = nue->izq = NULL;
    *p = nue;
    return TODO_BIEN;
}

int buscarElemArbolBinBusq(const tArbolBinBusq *p, void *d, unsigned tam, int (*cmp)(const void*, const void*))
{
    if (!(p = buscarNodoArbolBinBusq(p, d, cmp))) return 0;
    memcpy(d, (*p)->info, MINIMO(tam, (*p)->tamInfo));
    return 1;
}

int eliminarElemArbolBinBusq(tArbolBinBusq *p, void *d, unsigned tam, int (*cmp)(const void*, const void*))
{
    if (!(p = buscarNodoArbolBinBusq(p, d, cmp))) return 0;
    memcpy(d, (*p)->info, MINIMO(tam, (*p)->tamInfo));
    return eliminarRaizArbolBinBusq(p);
}

void eliminarArbol(tArbolBinBusq *pa)
{
    if (*pa)
    {
        eliminarArbol(&(*pa)->izq);
        eliminarArbol(&(*pa)->der);
        free((*pa)->info);
        free(*pa);
        *pa = NULL;
    }
}

unsigned alturaArbolBin(const tArbolBinBusq *p)
{
    int hi, hd;
    if (!*p) return 0;
    hi = alturaArbolBin(&(*p)->izq);
    hd = alturaArbolBin(&(*p)->der);
    return (hi > hd ? hi : hd) + 1;
}

tNodoArbol ** buscarNodoArbolBinBusq(const tArbolBinBusq *p, const void *d, int (*cmp)(const void*, const void*))
{
    int rc;
    while (*p && (rc = cmp(d, (*p)->info)))
    {
        if (rc < 0) p = &(*p)->izq;
        else p = &(*p)->der;
    }
    if (!*p) return NULL;
    return (tNodoArbol **)p;
}

tNodoArbol ** mayorNodoArbolBinBusq(const tArbolBinBusq *p)
{
    if (!*p) return NULL;
    while ((*p)->der) p = &(*p)->der;
    return (tNodoArbol **)p;
}

tNodoArbol ** menorNodoArbolBinBusq(const tArbolBinBusq *p)
{
    if (!*p) return NULL;
    while ((*p)->izq) p = &(*p)->izq;
    return (tNodoArbol **)p;
}

int eliminarRaizArbolBinBusq(tArbolBinBusq *p)
{
    tNodoArbol **remp, *elim;
    if (!*p) return 0;
    free((*p)->info);
    if (!(*p)->izq && !(*p)->der)
    {
        free(*p);
        *p = NULL;
        return 1;
    }
    remp = alturaArbolBin(&(*p)->izq) > alturaArbolBin(&(*p)->der) ?
           mayorNodoArbolBinBusq(&(*p)->izq) : menorNodoArbolBinBusq(&(*p)->der);
    elim = *remp;
    (*p)->info = elim->info;
    (*p)->tamInfo = elim->tamInfo;
    *remp = elim->izq ? elim->izq : elim->der;
    free(elim);
    return 1;
}
