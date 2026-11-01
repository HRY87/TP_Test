#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "simuladorTweener.h"

#define TAM_BUFFER_USUARIOS  65536

static const tEstrategia estrategias[] =
{
    {"SIN CACHE",                           0, scCrear, scObtener, scAgregar, scVaciar, NULL},
    {"PARTE 1 - lista simple",              1, lCrear,  lObtener,  lAgregar,  lVaciar,  NULL},
    {"PARTE 2 - lista doble + arbol indice", 1, aCrear,  aObtener,  aAgregar,  aVaciar,  aAltura}
};

static const unsigned capacidades[] = {10, 100, 1000, 10000};
static const unsigned sinCapacidad[] = {0};

/* ---------- tiempo ---------- */

/* Devuelve un tiempo monotonico en milisegundos. Se mide tiempo REAL (no de CPU):
   lo que la cache ahorra es espera de disco, que clock() no cuenta en Linux. */
#ifdef _WIN32
double obtenerTiempoMs(void)
{
    LARGE_INTEGER frecuencia;
    LARGE_INTEGER cuenta;

    QueryPerformanceFrequency(&frecuencia);
    QueryPerformanceCounter(&cuenta);

    return (double)cuenta.QuadPart * 1000.0 / (double)frecuencia.QuadPart;
}
#else
double obtenerTiempoMs(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}
#endif

/* ---------- comparadores ---------- */

/* Compara dos claves enteras sin riesgo de desbordamiento */
int cmpClaveInt(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}

/* Compara dos tDatoCache por su clave entera (la Parte 1 compara entradas completas) */
int cmpDatoCacheInt(const void *a, const void *b)
{
    return cmpClaveInt(((const tDatoCache *)a)->clave, ((const tDatoCache *)b)->clave);
}

/* ---------- adaptadores: sin cache (siempre falla y no guarda nada) ---------- */

/* No hay nada que crear */
void scCrear(tCache *c, unsigned cap)
{
    (void)c;
    (void)cap;
}

/* Siempre informa que no encontro el dato */
int scObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    (void)c;
    (void)clave;
    (void)dato;
    (void)tam;

    return TODO_BIEN;
}

/* No guarda nada */
void scAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
    (void)c;
    (void)clave;
    (void)dato;
    (void)tam;
}

/* No hay nada que liberar */
void scVaciar(tCache *c)
{
    (void)c;
}

/* ---------- adaptadores: Parte 1 (lista simple) ---------- */

/* Crea la cache de la Parte 1 */
void lCrear(tCache *c, unsigned cap)
{
    crear_lrucache(&c->lista, cap, sizeof(int), cmpDatoCacheInt);
}

/* Busca en la cache de la Parte 1 */
int lObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    return obtener_lrucache(&c->lista, clave, dato, tam);
}

/* Agrega a la cache de la Parte 1 */
void lAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
    agregar_lrucache(&c->lista, clave, dato, tam);
}

/* Libera la cache de la Parte 1 */
void lVaciar(tCache *c)
{
    vaciar_lrucache(&c->lista);
}

/* ---------- adaptadores: Parte 2 (lista doble + arbol indice) ---------- */

/* Crea la cache de la Parte 2 */
void aCrear(tCache *c, unsigned cap)
{
    crear_lrucache_indice(&c->arbol, cap, sizeof(int), cmpClaveInt);
}

/* Busca en la cache de la Parte 2 */
int aObtener(tCache *c, const void *clave, void *dato, unsigned tam)
{
    return obtener_lrucache_indice(&c->arbol, clave, dato, tam);
}

/* Agrega a la cache de la Parte 2 */
void aAgregar(tCache *c, const void *clave, const void *dato, unsigned tam)
{
    agregar_lrucache_indice(&c->arbol, clave, dato, tam);
}

/* Libera la cache de la Parte 2 */
void aVaciar(tCache *c)
{
    vaciar_lrucache_indice(&c->arbol);
}

/* Altura del arbol indice de la Parte 2 */
unsigned aAltura(tCache *c)
{
    return altura_arbol_indice(&c->arbol);
}

/* ---------- fuentes de datos ---------- */

/* Abre usuarios y posteos y reserva la tabla de casilleros. Deja todo inicializado
   para que cerrarFuentes sea siempre seguro, incluso si esto falla.
   Posteos sin buffer: cada lectura es un registro directo (acceso aleatorio), un
   buffer solo traeria bytes de mas. Usuarios con buffer grande: se leen en orden. */
int abrirFuentes(tFuentes *f, const char *nombreUsuarios)
{
    long bytes;

    f->casilleros = NULL;
    f->pfUsuarios = fopen(nombreUsuarios, "rb");
    f->pfPosteos = fopen(NOMBRE_POSTEOS, "rb");

    if(!f->pfUsuarios || !f->pfPosteos)
        return SIM_ERROR_ARCHIVO;

    setvbuf(f->pfPosteos, NULL, _IONBF, 0);
    setvbuf(f->pfUsuarios, NULL, _IOFBF, TAM_BUFFER_USUARIOS);

    fseek(f->pfUsuarios, 0, SEEK_END);
    bytes = ftell(f->pfUsuarios);
    rewind(f->pfUsuarios);

    if(bytes < (long)sizeof(tUsuario))
        return SIM_ERROR_ARCHIVO;

    f->casilleros = malloc(CANT_POSTEOS * sizeof(tPosteo));

    if(!f->casilleros)
        return SIM_SIN_MEM;

    /* Se toca toda la tabla ahora para que los fallos de pagina no entren en los tiempos */
    memset(f->casilleros, 0, CANT_POSTEOS * sizeof(tPosteo));

    return SIM_OK;
}

/* Libera todo lo que abrirFuentes haya conseguido */
void cerrarFuentes(tFuentes *f)
{
    free(f->casilleros);

    if(f->pfUsuarios)
        fclose(f->pfUsuarios);

    if(f->pfPosteos)
        fclose(f->pfPosteos);
}

/* Lectura directa (el id es la posicion del registro) en su casillero de la tabla.
   Devuelve el casillero, o NULL si el id es invalido o falla la lectura. */
tPosteo *leerPosteo(FILE *pf, tPosteo *casilleros, int id)
{
    tPosteo *casillero;

    if(id < 0 || id >= CANT_POSTEOS)
        return NULL;

    casillero = casilleros + id;

    if(fseek(pf, (long)id * (long)sizeof(tPosteo), SEEK_SET) != 0
            || fread(casillero, sizeof(*casillero), 1, pf) != 1
            || casillero->id != id)
        return NULL;

    return casillero;
}

/* ---------- simulacion ---------- */

/* Busca un posteo en la cache; si falla lo lee del archivo y lo agrega a la cache.
   Devuelve 0 si no pudo leerlo. Los datos obtenidos se descartan. */
int obtenerPosteo(const tEstrategia *e, tCache *c, const tFuentes *f, int id, tContadores *k)
{
    tPosteo descartado;
    tPosteo *casillero;

    if(e->obtener(c, &id, &descartado, sizeof(descartado)))
    {
        k->aciertos++;
        return 1;
    }

    k->fallos++;
    k->lecturas++;
    casillero = leerPosteo(f->pfPosteos, f->casilleros, id);

    if(!casillero)
        return 0;

    e->agregar(c, &casillero->id, casillero, sizeof(*casillero));

    return 1;
}

/* Procesa todos los posteos del feed de un usuario */
int procesarUsuario(const tEstrategia *e, tCache *c, const tFuentes *f, const tUsuario *usu, tContadores *k)
{
    const int *p = usu->feed;
    const int *fin = usu->feed + LARGO_FEED;
    int ok = 1;

    while(p < fin && ok)
    {
        ok = obtenerPosteo(e, c, f, *p, k);
        p++;
    }

    return ok;
}

/* Realiza una simulacion completa con una cache nueva, leyendo los usuarios de a uno */
int realizarSimulacion(const tEstrategia *e, unsigned capacidad, const tFuentes *f, tContadores *k)
{
    tCache c;
    tUsuario usu;
    int ok = 1;

    memset(k, 0, sizeof(*k));
    rewind(f->pfUsuarios);
    e->crear(&c, capacidad);

    while(ok && fread(&usu, sizeof(usu), 1, f->pfUsuarios) == 1)
    {
        ok = procesarUsuario(e, &c, f, &usu, k);
        k->usuarios++;
    }

    if(e->altura)
        k->altura = e->altura(&c);

    e->vaciar(&c);

    return ok;
}

/* Repite la simulacion y devuelve el tiempo promedio de una en ms (negativo si hubo error).
   Solo se mide la simulacion: la impresion queda afuera. */
double medirSimulacion(const tEstrategia *e, unsigned capacidad, int repeticiones,
                       const tFuentes *f, tContadores *k)
{
    double inicio = obtenerTiempoMs();
    int i;
    int ok = 1;

    for(i = 0; i < repeticiones && ok; i++)
        ok = realizarSimulacion(e, capacidad, f, k);

    return ok ? (obtenerTiempoMs() - inicio) / repeticiones : -1.0;
}

/* ---------- presentacion ---------- */

/* Imprime el titulo del bloque y los nombres de las columnas */
void mostrarEncabezado(const tEstrategia *e)
{
    printf("\n[%s]\n", e->nombre);
    printf("%8s %10s %12s %11s %9s %7s %9s",
           "capacid.", "total(ms)", "us/usuario", "us/posteo", "aciertos", "fallos", "lecturas");

    if(e->altura)
        printf(" %6s", "altura");

    printf(" %8s\n", "mejora");
}

/* Imprime una fila. Los promedios por usuario y por posteo salen del tiempo total
   (un posteo solo puede durar menos que la resolucion del reloj). 'mejora' compara
   contra el tiempo sin cache. */
void mostrarFila(const tEstrategia *e, unsigned capacidad, double ms, double msBase, const tContadores *k)
{
    if(capacidad)
        printf("%8u ", capacidad);
    else
        printf("%8s ", "-");

    printf("%10.2f %12.2f %11.3f %9lu %7lu %9lu",
           ms,
           ms * 1000.0 / (double)k->usuarios,
           ms * 1000.0 / ((double)k->usuarios * LARGO_FEED),
           k->aciertos, k->fallos, k->lecturas);

    if(e->altura)
        printf(" %6u", k->altura);

    printf(" %7.2fx\n", msBase / ms);
}

/* Muestra la tabla completa de una variante (una fila por capacidad).
   *msBase guarda el tiempo sin cache: la primera fila medida lo fija. */
int mostrarBloque(const tEstrategia *e, int repeticiones, const tFuentes *f, double *msBase)
{
    const unsigned *p = e->usaCache ? capacidades : sinCapacidad;
    const unsigned *fin = p + (e->usaCache ? CANTIDAD(capacidades) : CANTIDAD(sinCapacidad));
    tContadores k;
    double ms;
    int ok = 1;

    mostrarEncabezado(e);

    while(p < fin && ok)
    {
        ms = medirSimulacion(e, *p, repeticiones, f, &k);
        ok = ms >= 0.0;

        if(ok)
        {
            if(*msBase <= 0.0)
                *msBase = ms;

            mostrarFila(e, *p, ms, *msBase, &k);
        }

        p++;
    }

    if(!ok)
        printf("%s: error durante la simulacion\n", e->nombre);

    return ok;
}

/* Corre todas las variantes de un lote, cada una en su propia tabla.
   Antes hace una pasada sin medir para que el disco ya este en cache del sistema
   y la variante que se mide primero no quede en desventaja. */
int simularLote(const char *tipo, int repeticiones, const tFuentes *f)
{
    const tEstrategia *pe = estrategias;
    const tEstrategia *fin = estrategias + CANTIDAD(estrategias);
    tContadores k;
    double msBase = 0.0;
    int ok = realizarSimulacion(estrategias, 0, f, &k);

    if(ok)
        printf("\n--- Lote %s: %lu usuarios x %d posteos, %d repeticiones ---\n",
               tipo, k.usuarios, LARGO_FEED, repeticiones);

    while(pe < fin && ok)
    {
        ok = mostrarBloque(pe, repeticiones, f, &msBase);
        pe++;
    }

    return ok ? SIM_OK : SIM_ERROR_ARCHIVO;
}

/* Ejecuta todas las variantes y capacidades sobre un lote y muestra las tablas */
int ejecutarSimulacion(const char *tipo, int repeticiones)
{
    char nombre[64];
    tFuentes f;
    int res;

    if(repeticiones < 1)
        repeticiones = 1;

    snprintf(nombre, sizeof(nombre), FORMATO_USUARIOS, tipo);
    res = abrirFuentes(&f, nombre);

    if(res == SIM_OK)
        res = simularLote(tipo, repeticiones, &f);
    else if(res == SIM_SIN_MEM)
        printf("Sin memoria\n");
    else
        printf("No se pudo leer %s o %s (generalos con las opciones 1 a 3)\n", nombre, NOMBRE_POSTEOS);

    cerrarFuentes(&f);

    return res;
}
