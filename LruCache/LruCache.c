#include "LruCache.h"

// Adaptadores encapsulados
static void obtenerValorNodoLDE(void *refNodo, void *valor, unsigned tamValor) {
    tDatoCache_indice *dato = (tDatoCache_indice *)obtenerInfoNodoLDE(refNodo);
    if(dato) memcpy(valor, dato->valor, (tamValor < dato->tamValor) ? tamValor : dato->tamValor);
}

static void actualizarValorNodoLDE(void *refNodo, const void *valor, unsigned tamValor) {
    tDatoCache_indice *dato = (tDatoCache_indice *)obtenerInfoNodoLDE(refNodo);
    if(dato) {
        dato->valor = (void *)valor;
        dato->tamValor = tamValor;
    }
}

static int cmp_indice_arbol(const void *a, const void *b) {
    const tIndiceArbol *ia = (const tIndiceArbol *)a;
    const tIndiceArbol *ib = (const tIndiceArbol *)b;
    return ia->cmp(ia->clave, ib->clave);
}

void crear_lrucache_indice(tLruCache *c, unsigned capacidad, unsigned tamClave, tCmp cmp) {
    crearListaDoble(&c->lista);
    crearArbolBinBusq(&c->arbol);
    c->cantidad = 0;
    c->capacidad = capacidad;
    c->tamClave = tamClave;
    c->cmp = cmp;
}

int agregar_lrucache_indice(tLruCache *c, const void *clave, const void *valor, unsigned tamValor) {
    tIndiceArbol indice_busq = {(void *)clave, NULL, c->cmp};

    if (buscarElemArbolBinBusq(&c->arbol, &indice_busq, sizeof(tIndiceArbol), cmp_indice_arbol)) {
        actualizarValorNodoLDE(indice_busq.refNodoLista, valor, tamValor);
        moverAlPrincipioLDE(&c->lista, indice_busq.refNodoLista);
        return 1;
    }

    if (c->cantidad >= c->capacidad) {
        tDatoCache_indice dato_viejo;
        sacarUltimoLDE(&c->lista, &dato_viejo, sizeof(tDatoCache_indice));
        tIndiceArbol indice_a_borrar = {dato_viejo.clave, NULL, c->cmp};
        eliminarElemArbolBinBusq(&c->arbol, &indice_a_borrar, sizeof(tIndiceArbol), cmp_indice_arbol);
        c->cantidad--;
    }

    tDatoCache_indice nue_dato = {(void *)clave, (void *)valor, tamValor};
    void *refNodo = ponerAlComienzoLDE_Ref(&c->lista, &nue_dato, sizeof(tDatoCache_indice));

    tIndiceArbol nue_indice = {(void *)clave, refNodo, c->cmp};
    insertarArbolBinBusq(&c->arbol, &nue_indice, sizeof(tIndiceArbol), cmp_indice_arbol);

    c->cantidad++;
    return 1;
}

int obtener_lrucache_indice(tLruCache *c, const void *clave, void *valor, unsigned tamValor) {
    tIndiceArbol indice_busq = {(void *)clave, NULL, c->cmp};

    if (!buscarElemArbolBinBusq(&c->arbol, &indice_busq, sizeof(tIndiceArbol), cmp_indice_arbol)) {
        return 0;
    }

    moverAlPrincipioLDE(&c->lista, indice_busq.refNodoLista);
    obtenerValorNodoLDE(indice_busq.refNodoLista, valor, tamValor);
    return 1;
}

int borrar_lrucache_indice(tLruCache *c, const void *clave) {
    tIndiceArbol indice_busq = {(void *)clave, NULL, c->cmp};

    if (!eliminarElemArbolBinBusq(&c->arbol, &indice_busq, sizeof(tIndiceArbol), cmp_indice_arbol)) {
        return 0;
    }

    eliminarNodoLDE(&c->lista, indice_busq.refNodoLista);
    c->cantidad--;
    return 1;
}

int vaciar_lrucache_indice(tLruCache *c) {
    eliminarArbol(&c->arbol);
    vaciarListaDoble(&c->lista);
    c->cantidad = 0;
    return 1;
}

unsigned altura_arbol_indice(tLruCache *c) {
    return alturaArbolBin(&c->arbol);
}
