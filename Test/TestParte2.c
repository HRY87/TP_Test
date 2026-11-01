
#include "TestParte2.h"

// Comparador de claves directo (el árbol ya le pasa los punteros a los enteros gracias al wrapper)
int cmp_int(const void *a, const void *b) {
    const int *int_a = (const int *)a;
    const int *int_b = (const int *)b;
    return *int_a - *int_b;
}

void test_lrucache_parte2(void) {
    tLruCache cache;
    int valor_obtenido;

    // Usamos arreglos para que cada clave/valor tenga su propia dirección en memoria fija
    int claves[5] = {0, 1, 2, 3, 4};
    int valores[5] = {0, 10, 20, 30, 40};

    printf("--- INICIANDO TESTS LRU CACHE (PARTE 2 - ARBOL+LDE) ---\n");

    // 1. Crear caché con capacidad máxima de 3
    crear_lrucache_indice(&cache, 3, sizeof(int), cmp_int);
    printf("[OK] Cache inicializada (Capacidad: %d).\n", cache.capacidad);

    // 2. Insertar elementos: (1, 10), (2, 20), (3, 30)
    agregar_lrucache_indice(&cache, &claves[1], &valores[1], sizeof(int));
    agregar_lrucache_indice(&cache, &claves[2], &valores[2], sizeof(int));
    agregar_lrucache_indice(&cache, &claves[3], &valores[3], sizeof(int));

    if (cache.cantidad == 3) {
        printf("[OK] 3 elementos insertados. Altura del arbol indice: %u\n", altura_arbol_indice(&cache));
    } else {
        printf("[ERROR] La cantidad no es 3 (Es %d).\n", cache.cantidad);
    }

    // 3. Obtener elemento intermedio (Clave 2) para que se promueva al frente en O(1)
    if (obtener_lrucache_indice(&cache, &claves[2], &valor_obtenido, sizeof(int)) && valor_obtenido == 20) {
        printf("[OK] Elemento (2, 20) hallado en ABB y promovido al inicio de LDE.\n");
    } else {
        printf("[ERROR] Fallo al buscar u obtener el valor correcto de (2).\n");
    }

    // 4. Forzar desborde LRU insertando un 4to elemento (Clave 4).
    // Como consultamos el 2 recientemente, el menos usado ahora es el 1.
    agregar_lrucache_indice(&cache, &claves[4], &valores[4], sizeof(int));

    if (!obtener_lrucache_indice(&cache, &claves[1], &valor_obtenido, sizeof(int))) {
        printf("[OK] Politica LRU exitosa: Elemento (1) expulsado del Arbol y la LDE por desborde.\n");
    } else {
        printf("[ERROR] Politica LRU fallo: Elemento (1) sigue en la cache.\n");
    }

    // 5. Borrado manual (Clave 3)
    if (borrar_lrucache_indice(&cache, &claves[3])) {
        printf("[OK] Elemento (3) borrado manualmente de ambas estructuras.\n");
    } else {
        printf("[ERROR] Fallo al borrar el elemento (3).\n");
    }

    // 6. Vaciar y corroborar limpieza total
    vaciar_lrucache_indice(&cache);
    if (cache.cantidad == 0 && altura_arbol_indice(&cache) == 0) {
        printf("[OK] Cache vaciada. Altura del arbol final: %u (Cantidad: %d).\n", altura_arbol_indice(&cache), cache.cantidad);
    } else {
        printf("[ERROR] La cache no se vacio completamente.\n");
    }

    printf("--- FIN DE TESTS ---\n");
}
