#ifndef GENERADORLOTES_H_INCLUDED
#define GENERADORLOTES_H_INCLUDED

#define USUARIOS_MIN     200
#define USUARIOS_MAX     99999
#define POOL_DEFECTO     100

#define MAX_CARACTERES   139            /* "menos de 140" */

/* Elegir aca como se arman los textos de los posteos */
#define TEXTO_LETRAS     0
#define TEXTO_PALABRAS   1
#define TEXTO_ELEGIDO    TEXTO_PALABRAS

/* Estado compartido por los generadores de un lote */
struct sCtxLote;

typedef int (*tGenerarId)(struct sCtxLote *ctx);
typedef void (*tGenerarTexto)(char *texto);

/* Completa el registro numero 'numero' (desde 0) antes de escribirlo al archivo */
typedef void (*tArmarRegistro)(void *registro, int numero, void *contexto);

typedef struct sCtxLote
{
    int *pool;                          /* posteos populares (lote concentrado) */
    int tamPool;
    unsigned long contador;             /* ids ya generados (lote ordenado) */
    unsigned long total;                /* ids totales a generar */
    tGenerarId generar;                 /* generador de ids del tipo de lote */
} tCtxLote;

typedef struct
{
    const char *nombre;
    tGenerarId generar;
} tTipoLote;

/* Aleatorios */
unsigned long aleatorio(unsigned long n);

/* Generadores de id (uno por tipo de lote) */
int idConcentrado(tCtxLote *ctx);
int idDisperso(tCtxLote *ctx);
int idOrdenado(tCtxLote *ctx);
tGenerarId buscarTipo(const char *nombre);

/* Generadores de texto */
void generarTextoLetrasAleatorias(char *texto);
const char *elegirPalabra(void);
void generarTextoPalabrasAleatorias(char *texto);

/* Armado y escritura de archivos */
void armarPosteo(void *registro, int numero, void *contexto);
void armarUsuario(void *registro, int numero, void *contexto);
int escribirArchivoRegistros(const char *nombre, int cantidad, unsigned tamRegistro,
                             tArmarRegistro armar, void *contexto);

/* Preparacion del lote */
int validarLote(tGenerarId generar, int usuarios, int tamPool);
int iniciarContextoLote(tCtxLote *ctx, tGenerarId generar, int usuarios, int tamPool, unsigned long semilla);
int escribirArchivosLote(const char *nombreUsuarios, int usuarios, tCtxLote *ctx);

/* Genera posteos.dat y usuarios_<tipo>.dat (y sus .txt); la misma semilla reproduce el mismo lote.
   tipo: "concentrado", "disperso" u "ordenado". Devuelve SIM_OK o un codigo de error. */
int generarLote(unsigned long semilla, const char *tipo, int usuarios, int tamPool);

#endif // GENERADORLOTES_H_INCLUDED
