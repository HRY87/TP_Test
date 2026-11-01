#include "LruCacheLista.h"



/**
 * crear_lrucache
 * Inicializa la lista y los parámetros base de la caché.
 */
void crear_lrucache(tLruCacheLista *c, unsigned capacidad, unsigned tamClave, tCmp cmp)
{
    crearLista(&c->lista);
    c->cantidad = 0;
    c->capacidad = capacidad;
    c->tamClave = tamClave;
    c->cmp = cmp;
}

/**
* 1. Se busca el elemento en la lista, creando un dummy con la clave, para poder comparar tDatoCache.
* 2. Si lo encuentra al elemento, es eliminado. entonces decrementa la cantidad.
* 3. Si la cantidad de elemento es igual a la capacidad, elimina el ultimo elemento y reducimos la cantidad.
* 4. Crea el nuevo elemento y llama al metodo ponerAlComienzo en la pila, incrementando la cantidad.
*/
int agregar_lrucache(tLruCacheLista *c, const void *clave, const void *valor, unsigned tamValor)
{
    tDatoCache dato;
    tDatoCache clave_busq = {(void *)clave, NULL, 0};

    if(sacarPorClave(&c->lista, &dato, sizeof(tDatoCache), &clave_busq, c->cmp))
    {
        c->cantidad -= 1;
    }

    if (c->cantidad == c->capacidad)
    {
        sacarUltimoLista(&c->lista, &dato, sizeof(tDatoCache));
        c->cantidad -= 1;
    }

    tDatoCache nue = {(void *)clave, (void *)valor, tamValor};
    ponerAlComienzo(&c->lista, &nue, sizeof(tDatoCache));

    c->cantidad += 1;

    return 1;
}

/**
* 1. Creamos el dummy de la clave.
* 2. Sacamos el elemento, si no existe, retornamos 0
* 3. Si el elemento existe, lo copiamos en el buffer "valor"
* 4. volvemos a cargar el dato al comienzo.
*/
int obtener_lrucache(tLruCacheLista *c, const void *clave, void *valor, unsigned tamValor)
{
    tDatoCache dato;
    tDatoCache clave_busq = {(void *)clave, NULL, 0};

    if(!sacarPorClave(&c->lista, &dato, sizeof(tDatoCache), &clave_busq, c->cmp)) return 0;

    memcpy(valor, dato.valor, (tamValor < dato.tamValor) ? tamValor : dato.tamValor);

    ponerAlComienzo(&c->lista, &dato, sizeof(tDatoCache));

    return 1;
}

/**
* 1. Creamos el dummy de la clave
* 2. Si el elemento no existe, retornamos 0.
* 3. Si el elemento existe, decrementamos la cantidad.
*/
int borrar_lrucache(tLruCacheLista *c, const void *clave)
{
    tDatoCache dato;
    tDatoCache clave_busq = {(void *)clave, NULL, 0};

    if(!sacarPorClave(&c->lista, &dato, sizeof(tDatoCache), &clave_busq, c->cmp)) return 0;

    c->cantidad -=1;
    return 1;
}

/**
* 1. Llamamos a vaciar lista
* 2. Limpiamos todas las variables
*/
int vaciar_lrucache(tLruCacheLista *c)
{
    vaciarLista(&c->lista);
    c->cantidad = 0;

    return 1;
}
