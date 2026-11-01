#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simuladorTweener.h"
#include "registros.h"
#include "../LruCacheLista/LruCacheLista.h"
#include "../LruCache/LruCache.h"

/* Las dos caches comparten espacio: en cada simulacion se usa solo una */
typedef union
{
    tLruCacheLista lista;
    tLruCache arbol;
} tCache;

/* Cada variante se maneja con las mismas operaciones */
typedef struct
{
    const char *nombre;
    void     (*crear)(tCache *c, unsigned capacidad);
    int      (*obtener)(tCache *c, const void *clave, void *dato, unsigned tamDato);   /* distinto de 0 si lo encontro */
    void     (*agregar)(tCache *c, const void *clave, const void *dato, unsigned tamDato);
    void     (*vaciar)(tCache *c);
    unsigned (*altura)(tCache *c);                                                      /* NULL si no aplica */
} tEstrategia;

typedef struct
{
    unsigned long aciertos;
    unsigned long fallos;
    unsigned long lecturas;
    unsigned altura;
} tContadores;

/* Recursos compartidos por todas las simulaciones de un lote.
   Las caches de este proyecto guardan PUNTEROS a la clave y al valor (no copias),
   asi que ese almacenamiento lo pone la simulacion: 'tabla' tiene un casillero por
   id de posteo. Un casillero solo se escribe cuando su id NO esta en la cache (fallo),
   por lo que nunca se pisa un dato que la cache este referenciando. La tabla no se
   consulta nunca para decidir aciertos: eso lo decide solo la cache. */
typedef struct
{
    FILE *pf;
    tPosteo *tabla;
    tUsuario *usu;
    size_t cant;
} tEscenario;

/* ---------- comparadores ---------- */

/* Compara dos claves enteras sin riesgo de desbordamiento */
static int cmpClaveInt(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}

/* Compara dos tDatoCache por su clave entera (la Parte 1 compara entradas completas).
   Si tu Parte 1 llama al comparador con claves crudas, usar cmpClaveInt en lCrear. */
static int cmpDatoCacheInt(const void *a, const void *b)
{
    return cmpClaveInt(((const tDatoCache *)a)->clave, ((const tDatoCache *)b)->clave);
}

/* ---------- adaptadores: sin cache (siempre falla y no guarda nada) ---------- */

static void scCrear(tCache *c, unsigned cap)
{
}

static int scObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    return 0;
}

static void scAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
}

static void scVaciar(tCache *c)
{
}

/* ---------- adaptadores: Parte 1 (lista simple) ---------- */

static void lCrear(tCache *c, unsigned cap)
{
    crear_lrucache(&c->lista, cap, sizeof(int), cmpDatoCacheInt);
}

static int lObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    return obtener_lrucache(&c->lista, clave, dato, tam);
}

static void lAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
    agregar_lrucache(&c->lista, clave, dato, tam);
}

static void lVaciar(tCache *c)
{
    vaciar_lrucache(&c->lista);
}

/* ---------- adaptadores: Parte 2 (lista doble + arbol indice) ---------- */

static void aCrear(tCache *c, unsigned cap)
{
    crear_lrucache_indice(&c->arbol, cap, sizeof(int), cmpClaveInt);
}

static int aObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    return obtener_lrucache_indice(&c->arbol, clave, dato, tam);
}

static void aAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
    agregar_lrucache_indice(&c->arbol, clave, dato, tam);
}

static void aVaciar(tCache *c)
{
    vaciar_lrucache_indice(&c->arbol);
}

static unsigned aAltura(tCache *c)
{
    return altura_arbol_indice(&c->arbol);
}

static const tEstrategia estrategias[] =
{
    {"sin cache", scCrear, scObtener, scAgregar, scVaciar, NULL},
    {"parte 1",   lCrear,  lObtener,  lAgregar,  lVaciar,  NULL},
    {"parte 2",   aCrear,  aObtener,  aAgregar,  aVaciar,  aAltura}
};

/* ---------- recursos del lote ---------- */

/* Lee todo el archivo de usuarios en memoria; NULL si falla o esta vacio */
static tUsuario *cargarUsuarios(const char *nombre, size_t *cant)
{
    FILE *pf = fopen(nombre, "rb");
    tUsuario *usu = NULL;
    long bytes;

    *cant = 0;

    if(!pf)
        return NULL;

    fseek(pf, 0, SEEK_END);
    bytes = ftell(pf);
    rewind(pf);

    *cant = (size_t)bytes / sizeof(tUsuario);

    if(*cant > 0)
        usu = malloc(*cant * sizeof(tUsuario));

    if(usu && fread(usu, sizeof(tUsuario), *cant, pf) != *cant)
    {
        free(usu);
        usu = NULL;
    }

    fclose(pf);

    return usu;
}

/* Prepara usuarios, archivo de posteos y tabla de casilleros. Deja todo inicializado
   para que cerrarEscenario sea siempre seguro, incluso si esto falla. */
static int abrirEscenario(tEscenario *esc, const char *nombreUsuarios)
{
    esc->pf = NULL;
    esc->tabla = NULL;
    esc->usu = cargarUsuarios(nombreUsuarios, &esc->cant);

    if(!esc->usu)
        return SIM_ERROR_ARCHIVO;

    esc->pf = fopen(NOMBRE_POSTEOS, "rb");

    if(!esc->pf)
        return SIM_ERROR_ARCHIVO;

    esc->tabla = malloc(CANT_POSTEOS * sizeof(tPosteo));

    if(!esc->tabla)
        return SIM_SIN_MEM;

    /* Se toca toda la tabla ahora para que los fallos de pagina no entren en los tiempos */
    memset(esc->tabla, 0, CANT_POSTEOS * sizeof(tPosteo));

    return SIM_OK;
}

/* Libera todo lo que abrirEscenario haya conseguido */
static void cerrarEscenario(tEscenario *esc)
{
    free(esc->tabla);
    free(esc->usu);

    if(esc->pf)
        fclose(esc->pf);
}

/* ---------- simulacion ---------- */

/* Lectura directa (el id es la posicion del registro) en su casillero de la tabla.
   Devuelve el casillero, o NULL si el id es invalido o falla la lectura. */
static tPosteo *leerPosteo(FILE *pf, tPosteo *tabla, int id)
{
    tPosteo *casillero;

    if(id < 0 || id >= CANT_POSTEOS)
        return NULL;

    casillero = tabla + id;

    if(fseek(pf, (long)id * (long)sizeof(tPosteo), SEEK_SET) != 0
            || fread(casillero, sizeof(*casillero), 1, pf) != 1
            || casillero->id != id)
        return NULL;

    return casillero;
}

/* Busca cada posteo del feed en la cache y, si falla, en el archivo (y lo agrega a la cache) */
static int procesarUsuario(const tEstrategia *e, tCache *c, const tEscenario *esc,
                           const tUsuario *usu, tContadores *k)
{
    const int *p = usu->feed;
    const int *fin = usu->feed + LARGO_FEED;
    tPosteo descartado;
    tPosteo *casillero;
    int ok = 1;

    while(p < fin && ok)
    {
        if(e->obtener(c, p, &descartado, sizeof(descartado)))
            k->aciertos++;
        else
        {
            k->fallos++;
            k->lecturas++;
            casillero = leerPosteo(esc->pf, esc->tabla, *p);
            ok = casillero != NULL;

            if(ok)
                e->agregar(c, &casillero->id, casillero, sizeof(*casillero));
        }

        p++;
    }

    return ok;
}

/* Realiza una simulacion completa con una cache nueva. Los datos leidos se descartan. */
static int realizarSimulacion(const tEstrategia *e, unsigned capacidad,
                              const tEscenario *esc, tContadores *k)
{
    tCache c;
    const tUsuario *u = esc->usu;
    const tUsuario *fin = esc->usu + esc->cant;
    int ok = 1;

    memset(k, 0, sizeof(*k));
    e->crear(&c, capacidad);

    while(u < fin && ok)
    {
        ok = procesarUsuario(e, &c, esc, u, k);
        u++;
    }

    if(e->altura)
        k->altura = e->altura(&c);

    e->vaciar(&c);

    return ok;
}

/* Repite la simulacion, promedia los tiempos e imprime una fila de la tabla */
static int medirYMostrarFila(const tEstrategia *e, unsigned capacidad, int repeticiones,
                             const tEscenario *esc)
{
    tContadores k;
    clock_t ini = clock();
    double ms;
    int i;
    int ok = 1;

    for(i = 0; i < repeticiones && ok; i++)
        ok = realizarSimulacion(e, capacidad, esc, &k);

    ms = (double)(clock() - ini) * 1000.0 / CLOCKS_PER_SEC / repeticiones;

    if(!ok)
    {
        printf("%-10s error durante la simulacion\n", e->nombre);
        return 0;
    }

    printf("%-10s ", e->nombre);

    if(capacidad)
        printf("%8u ", capacidad);
    else
        printf("%8s ", "-");

    printf("%10.2f %12.2f %11.3f %9lu %7lu %9lu ",
           ms,
           ms * 1000.0 / (double)esc->cant,
           ms * 1000.0 / ((double)esc->cant * LARGO_FEED),
           k.aciertos, k.fallos, k.lecturas);

    if(e->altura)
        printf("%6u\n", k.altura);
    else
        printf("%6s\n", "-");

    return 1;
}

/* Ejecuta todas las variantes y capacidades sobre un lote y muestra la tabla */
int ejecutarSimulacion(const char *tipo, int repeticiones)
{
    static const unsigned capacidades[] = {10, 100, 1000, 10000};
    const unsigned *pc;
    const unsigned *finC = capacidades + sizeof(capacidades) / sizeof(*capacidades);
    const tEstrategia *pe;
    const tEstrategia *finE = estrategias + sizeof(estrategias) / sizeof(*estrategias);
    char nombre[64];
    tEscenario esc;
    int res;
    int ok;

    if(repeticiones < 1)
        repeticiones = 1;

    snprintf(nombre, sizeof(nombre), FORMATO_USUARIOS, tipo);
    res = abrirEscenario(&esc, nombre);

    if(res != SIM_OK)
    {
        if(res == SIM_SIN_MEM)
            printf("Sin memoria\n");
        else
            printf("No se pudo leer %s o %s (generalos con las opciones 1 a 3)\n", nombre, NOMBRE_POSTEOS);

        cerrarEscenario(&esc);
        return res;
    }

    printf("\n--- Lote %s: %u usuarios x %d posteos, %d repeticiones ---\n",
           tipo, (unsigned)esc.cant, LARGO_FEED, repeticiones);
    printf("%-10s %8s %10s %12s %11s %9s %7s %9s %6s\n",
           "variante", "capacid.", "total(ms)", "us/usuario", "us/posteo",
           "aciertos", "fallos", "lecturas", "altura");

    ok = medirYMostrarFila(estrategias, 0, repeticiones, &esc);   /* sin cache */

    for(pc = capacidades; pc < finC && ok; pc++)
    {
        for(pe = estrategias + 1; pe < finE && ok; pe++)
            ok = medirYMostrarFila(pe, *pc, repeticiones, &esc);
    }

    cerrarEscenario(&esc);

    return ok ? SIM_OK : SIM_ERROR_ARCHIVO;
}
