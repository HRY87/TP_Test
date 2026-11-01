#include <stdio.h>
#include "TestPart1.h"
#include "../LruCacheLista/LruCacheLista.h"

static int cmp_int(const void *a, const void *b) {
    const tDatoCache *dato_a = (const tDatoCache *)a;
    const tDatoCache *dato_b = (const tDatoCache *)b;

    const int *clave_a = (const int *)(dato_a->clave);
    const int *clave_b = (const int *)(dato_b->clave);

    return *clave_a - *clave_b;
}

void ejecutar_tests_lrucache(void) {
    tLruCacheLista cache;
    int valor_obtenido;

    int claves[5] = {0, 1, 2, 3, 4};
    int valores[5] = {0, 10, 20, 30, 40};

    printf("--- INICIANDO TESTS LRU CACHE (PARTE 1) ---\n");

    crear_lrucache(&cache, 3, sizeof(int), cmp_int);
    printf("[OK] Cache inicializada (Capacidad: %d).\n", cache.capacidad);

    agregar_lrucache(&cache, &claves[1], &valores[1], sizeof(int));
    agregar_lrucache(&cache, &claves[2], &valores[2], sizeof(int));
    agregar_lrucache(&cache, &claves[3], &valores[3], sizeof(int));

    if (cache.cantidad == 3) {
        printf("[OK] 3 elementos insertados correctamente.\n");
    } else {
        printf("[ERROR] La cantidad no es 3 (Es %d).\n", cache.cantidad);
    }

    if (obtener_lrucache(&cache, &claves[2], &valor_obtenido, sizeof(int)) && valor_obtenido == 20) {
        printf("[OK] Elemento (2, 20) hallado y promovido al inicio.\n");
    } else {
        printf("[ERROR] Fallo al buscar u obtener el valor correcto de (2).\n");
    }

    agregar_lrucache(&cache, &claves[4], &valores[4], sizeof(int));

    if (!obtener_lrucache(&cache, &claves[1], &valor_obtenido, sizeof(int))) {
        printf("[OK] Politica LRU exitosa: Elemento (1) fue expulsado por desborde.\n");
    } else {
        printf("[ERROR] Politica LRU fallo: Elemento (1) sigue en la cache.\n");
    }

    if (borrar_lrucache(&cache, &claves[3])) {
        printf("[OK] Elemento (3) borrado manualmente.\n");
    } else {
        printf("[ERROR] Fallo al borrar el elemento (3).\n");
    }

    vaciar_lrucache(&cache);
    if (cache.cantidad == 0) {
        printf("[OK] Cache vaciada correctamente (Cantidad: %d).\n", cache.cantidad);
    } else {
        printf("[ERROR] La cache no reseteo su cantidad.\n");
    }

    printf("--- FIN DE TESTS ---\n");
}
