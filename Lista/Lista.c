#include "lista.h"

#define minimo(X, Y) ((X) <= (Y) ? (X) : (Y))

/**
* Crea la lista apuntando a NULL
*/
void crearLista(tLista *l) {
    *l = NULL;
}

/**
* 1.Reservar memoria para el nuevo nodo y copiar la información
* 2. El siguiente del nuevo nodo, apunta a lista, es decir nue->sig = *l
* 3. La lista apunta al nuevo elemento, *l = nue
*/
int ponerAlComienzo(tLista *l, const void *d, unsigned cantBytes) {
    tNodo *nue;
    if ((nue = (tNodo *)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(cantBytes)) == NULL) {
        free(nue);
        return 0;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;

    nue->sig = *l;
    *l = nue;
    return 1;
}
/**
* 1. Validamos que la lista tenga elementos.
* 2. La lista apunta al eliminar siguiente.
* 3. Copia el dato a eliminar, en el buffer.
* 4. Libera memoria del dato y tNodo.
*/
int sacarPrimeroLista(tLista *l, void *d, unsigned cantBytes) {
    tNodo *elim = *l;

    if (*l == NULL) return 0;

    *l = elim->sig;

    memcpy(d, elim->info, minimo(cantBytes, elim->tamInfo));

    free(elim->info);
    free(elim);
    return 1;
}

/**
* 1. Valido que la lista tenga elementos.
* 2. Movemos lista hasta el ultimo elemento.
* 3. Copiamos el elemento en el buffer.
* 4. Liberamos memoria dinamica.
* 5. La lista quedo apunta al siguiente del ultimo elementos, entonces debe valer NULL
*/
int sacarUltimoLista(tLista *l, void *d, unsigned cantBytes) {
    if (*l == NULL) return 0;

    while ((*l)->sig) {
        l = &(*l)->sig;
    }

    memcpy(d, (*l)->info, minimo(cantBytes, (*l)->tamInfo));

    free((*l)->info);
    free(*l);

    *l = NULL;
    return 1;
}
/**
* 1.Recorremos la lista y evaluamos, cada elemento con la funcion de comparación, Si el elemento es encontrado:
* 1.1 Creamo un auxiliar "Elim" que apunta al valor de l
* 1.2 Actualizar el valor de l, con lo que apunta elim siguiente.
* 1.3 copiamos el elemento, en el buffer.
* 1.4 liberamos memoria.
* 1.5 Retornamos 1.
* 2. retornamos 0, no se encontro el elemento
*/
int sacarPorClave(tLista *l, void *d, unsigned cantBytes, const void *clave, int (*cmp)(const void *, const void *)) {
    while (*l) {

        if (cmp((*l)->info, clave) == 0) {
            tNodo *elim = *l;

            *l = elim->sig;
            memcpy(d, elim->info, minimo(cantBytes, elim->tamInfo));
            free(elim->info);
            free(elim);
            return 1;
        }

        l = &(*l)->sig;
    }
    return 0;
}

void vaciarLista(tLista *l)
{
    tNodo *elim;

    while (*l)
    {
        elim = *l;
        *l = elim->sig;

        free(elim->info);
        free(elim);
    }
}
