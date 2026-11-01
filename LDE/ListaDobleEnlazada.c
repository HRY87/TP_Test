#include "ListaDobleEnlazada.h"

#define minimo(X, Y) ((X) <= (Y) ? (X) : (Y))

void crearListaDoble(tListaDoble *l) {
    l->primero = NULL;
    l->ultimo = NULL;
}

void vaciarListaDoble(tListaDoble *l) {
    tNodoD *elim;
    while (l->primero != NULL) {
        elim = l->primero;
        l->primero = elim->sig;
        free(elim->info);
        free(elim);
    }
    l->ultimo = NULL;
}

void* ponerAlComienzoLDE_Ref(tListaDoble *l, const void *d, unsigned cantBytes) {
    tNodoD *nue = (tNodoD *)malloc(sizeof(tNodoD));
    if (nue == NULL || (nue->info = malloc(cantBytes)) == NULL) {
        free(nue);
        return NULL;
    }
    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = l->primero;
    nue->ant = NULL;

    if (l->primero != NULL) {
        l->primero->ant = nue;
    } else {
        l->ultimo = nue;
    }
    l->primero = nue;
    return nue;
}

int sacarUltimoLDE(tListaDoble *l, void *d, unsigned cantBytes) {
    if (l->ultimo == NULL) return 0;
    tNodoD *elim = l->ultimo;
    memcpy(d, elim->info, minimo(cantBytes, elim->tamInfo));

    if (elim->ant != NULL) {
        elim->ant->sig = NULL;
        l->ultimo = elim->ant;
    } else {
        l->primero = NULL;
        l->ultimo = NULL;
    }
    free(elim->info);
    free(elim);
    return 1;
}

int eliminarNodoLDE(tListaDoble *l, void *refNodo) {
    tNodoD *elim = (tNodoD *)refNodo;
    if (elim == NULL) return 0;

    if (elim->ant != NULL) {
        elim->ant->sig = elim->sig;
    } else {
        l->primero = elim->sig;
    }
    if (elim->sig != NULL) {
        elim->sig->ant = elim->ant;
    } else {
        l->ultimo = elim->ant;
    }
    free(elim->info);
    free(elim);
    return 1;
}

int moverAlPrincipioLDE(tListaDoble *l, void *refNodo) {
    tNodoD *nodo = (tNodoD *)refNodo;
    if (nodo == NULL || l->primero == nodo) return 1;

    if (nodo->ant != NULL) nodo->ant->sig = nodo->sig;
    if (nodo->sig != NULL) nodo->sig->ant = nodo->ant;
    else l->ultimo = nodo->ant;

    nodo->sig = l->primero;
    nodo->ant = NULL;
    if (l->primero != NULL) l->primero->ant = nodo;
    l->primero = nodo;
    return 1;
}

void* obtenerInfoNodoLDE(void *refNodo) {
    tNodoD *nodo = (tNodoD *)refNodo;
    return nodo ? nodo->info : NULL;
}
