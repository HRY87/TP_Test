#ifndef SIMULADORTWEENER_H_INCLUDED
#define SIMULADORTWEENER_H_INCLUDED

#include <stdio.h>
#include "registros.h"
#include "../LruCacheLista/LruCacheLista.h"
#include "../LruCache/LruCache.h"

#define CANTIDAD(v)  (sizeof(v) / sizeof(*(v)))

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
    int      usaCache;                                                                  /* 0: no tiene capacidad */
    void     (*crear)(tCache *c, unsigned capacidad);
    int      (*obtener)(tCache *c, const void *clave, void *dato, unsigned tamDato);   /* distinto de 0 si lo encontro */
    void     (*agregar)(tCache *c, const void *clave, const void *dato, unsigned tamDato);
    tPosteo *(*victima)(tCache *c);                                                     /* posteo que saldra en el proximo agregar; NULL si todavia hay lugar */
    void     (*vaciar)(tCache *c);
    unsigned (*altura)(tCache *c);                                                      /* NULL si no aplica */
} tEstrategia;

typedef struct
{
    unsigned long usuarios;
    unsigned long aciertos;
    unsigned long fallos;
    unsigned long lecturas;
    unsigned altura;
} tContadores;

/* Fuentes de datos de un lote. Los usuarios y los posteos se leen del disco a
   medida que se necesitan (nunca se carga un archivo completo). */
typedef struct
{
    FILE *pfPosteos;
    FILE *pfUsuarios;
} tFuentes;

/* Tiempo */
double obtenerTiempoMs(void);

/* Comparadores */
int cmpClaveInt(const void *a, const void *b);
int cmpDatoCacheInt(const void *a, const void *b);

/* Adaptadores: sin cache */
void scCrear(tCache *c, unsigned cap);
int scObtener(tCache *c, const void *clave, void *dato, unsigned tam);
void scAgregar(tCache *c, const void *clave, const void *dato, unsigned tam);
tPosteo *scVictima(tCache *c);
void scVaciar(tCache *c);

/* Adaptadores: Parte 1 (lista simple) */
void lCrear(tCache *c, unsigned cap);
int lObtener(tCache *c, const void *clave, void *dato, unsigned tam);
void lAgregar(tCache *c, const void *clave, const void *dato, unsigned tam);
tPosteo *lVictima(tCache *c);
void lVaciar(tCache *c);

/* Adaptadores: Parte 2 (lista doble + arbol indice) */
void aCrear(tCache *c, unsigned cap);
int aObtener(tCache *c, const void *clave, void *dato, unsigned tam);
void aAgregar(tCache *c, const void *clave, const void *dato, unsigned tam);
tPosteo *aVictima(tCache *c);
void aVaciar(tCache *c);
unsigned aAltura(tCache *c);

/* Fuentes de datos */
int abrirFuentes(tFuentes *f, const char *nombreUsuarios);
void cerrarFuentes(tFuentes *f);
int leerPosteo(FILE *pf, tPosteo *destino, int id);

/* Simulacion.
   Las caches de este proyecto guardan PUNTEROS a la clave y al valor (no copias),
   asi que cada posteo cacheado necesita un casillero propio que no se pise mientras
   siga en la cache. realizarSimulacion reserva capacidad + 1 casilleros: 'capacidad'
   en uso y uno libre donde se lee el proximo posteo. 'libre' apunta a ese casillero. */
int obtenerPosteo(const tEstrategia *e, tCache *c, const tFuentes *f, tPosteo **libre, int id, tContadores *k);
int procesarUsuario(const tEstrategia *e, tCache *c, const tFuentes *f, tPosteo **libre, const tUsuario *usu, tContadores *k);
int realizarSimulacion(const tEstrategia *e, unsigned capacidad, const tFuentes *f, tContadores *k);
double medirSimulacion(const tEstrategia *e, unsigned capacidad, int repeticiones,
                       const tFuentes *f, tContadores *k);

/* Presentacion */
void mostrarEncabezado(const tEstrategia *e);
void mostrarFila(const tEstrategia *e, unsigned capacidad, double ms, double msBase, const tContadores *k);
int mostrarBloque(const tEstrategia *e, int repeticiones, const tFuentes *f, double *msBase);
int simularLote(const char *tipo, int repeticiones, const tFuentes *f);

/* Simula el lote 'tipo' (concentrado, disperso u ordenado) sin cache, con la
   cache de la Parte 1 y con la de la Parte 2 (cada una en su propia tabla), para
   varias capacidades. Cada simulacion se repite 'repeticiones' veces y se informa
   el promedio. Devuelve SIM_OK o un codigo de error. */
int ejecutarSimulacion(const char *tipo, int repeticiones);

#endif // SIMULADORTWEENER_H_INCLUDED
