#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Lotes/generadorLotes.h"
#include "../SimuladorTweener/registros.h"
#include "../Lotes/exportador.h"

#define ALFABETO         "abcdefghijklmnopqrstuvwxyz "
#define CANT_PALABRAS    (sizeof(palabras) / sizeof(*palabras))

typedef struct
{
    int *pool;                          /* posteos populares (lote concentrado) */
    int tamPool;
    unsigned long contador;             /* ids ya generados (lote ordenado) */
    unsigned long total;                /* ids totales a generar */
} tCtxLote;

typedef int (*tGenerarId)(tCtxLote *ctx);
typedef void (*tGenerarTexto)(char *texto);

typedef struct
{
    const char *nombre;
    tGenerarId generar;
} tTipoLote;

static const char *palabras[] =
{
    /* saludos y expresiones */
    "hola", "chau", "gracias", "buenas", "genial", "bueno", "claro", "dale", "perfecto", "vamos",
    /* sobre el dia a dia */
    "hoy", "manana", "ayer", "ahora", "siempre", "nunca", "temprano", "tarde", "noche", "semana",
    "dia", "tiempo", "fin", "lunes", "viernes", "sabado", "domingo", "vacaciones", "trabajo", "casa",
    /* personas */
    "amigos", "familia", "gente", "todos", "nadie", "alguien", "mama", "papa", "hermano", "vecino",
    /* comida y bebida */
    "cafe", "mate", "pizza", "asado", "pan", "dulce", "almuerzo", "cena", "desayuno", "helado",
    /* lugares y clima */
    "calle", "ciudad", "playa", "plaza", "parque", "sol", "lluvia", "frio", "calor", "viento",
    /* actividades */
    "musica", "pelicula", "serie", "libro", "juego", "partido", "futbol", "viaje", "paseo", "fiesta",
    "estudio", "clase", "examen", "tarea", "proyecto", "reunion", "foto", "video", "baile", "carrera",
    /* adjetivos */
    "lindo", "feliz", "tranquilo", "cansado", "contento", "nuevo", "grande", "chico", "rapido", "facil",
    "dificil", "mejor", "peor", "bien", "mal", "todo", "nada", "mucho", "poco", "mas",
    /* verbos y conectores */
    "quiero", "puedo", "necesito", "pienso", "creo", "salgo", "vuelvo", "comparto", "miro", "escucho",
    "que", "para", "con", "sin", "pero", "porque", "tambien", "ya", "hay", "es"
};

/* Numero aleatorio en [0, n). Dos rand() en sentencias separadas: orden definido y rango > RAND_MAX */
static unsigned long aleatorio(unsigned long n)
{
    unsigned long alto = (unsigned long)rand();
    unsigned long bajo = (unsigned long)rand();

    return (alto * 32768UL + bajo) % n;
}

/* ---------- generadores de id (uno por tipo de lote) ---------- */

/* Lote concentrado: elige siempre dentro de un conjunto chico de posteos populares */
static int idConcentrado(tCtxLote *ctx)
{
    return *(ctx->pool + aleatorio((unsigned long)ctx->tamPool));
}

/* Lote disperso: cualquier posteo con igual probabilidad */
static int idDisperso(tCtxLote *ctx)
{
    (void)ctx;
    return (int)aleatorio(CANT_POSTEOS);
}

/* Lote ordenado: estrictamente creciente en toda la simulacion (requiere total <= CANT_POSTEOS) */
static int idOrdenado(tCtxLote *ctx)
{
    int id = (int)((ctx->contador * CANT_POSTEOS) / ctx->total);

    ctx->contador++;

    return id;
}

/* Busca la funcion generadora de un tipo de lote; NULL si no existe */
static tGenerarId buscarTipo(const char *nombre)
{
    static const tTipoLote tipos[] =
    {
        {"concentrado", idConcentrado},
        {"disperso",    idDisperso},
        {"ordenado",    idOrdenado}
    };
    const tTipoLote *p = tipos;
    const tTipoLote *fin = tipos + sizeof(tipos) / sizeof(*tipos);

    while(p < fin && strcmp(p->nombre, nombre) != 0)
        p++;

    return p < fin ? p->generar : NULL;
}

/* ---------- generadores de texto ---------- */

/* Llena el texto con entre 1 y MAX_CARACTERES letras al azar */
static void generarTextoLetrasAleatorias(char *texto)
{
    const char *fin = texto + 1 + aleatorio(MAX_CARACTERES);
    char *p;

    for(p = texto; p < fin; p++)
        *p = *(ALFABETO + aleatorio(strlen(ALFABETO)));
}

/* Elige una palabra al azar de la lista */
static const char *elegirPalabra(void)
{
    return *(palabras + aleatorio(CANT_PALABRAS));
}

/* Arma el texto con palabras separadas por un espacio, hasta un largo objetivo
   aleatorio de 1 a MAX_CARACTERES. Siempre lleva al menos una palabra y nunca
   supera MAX_CARACTERES; el '\0' final queda por el memset previo. */
static void generarTextoPalabrasAleatorias(char *texto)
{
    unsigned objetivo = 1 + (unsigned)aleatorio(MAX_CARACTERES);
    unsigned largo = 0;
    const char *palabra = elegirPalabra();
    unsigned tam = (unsigned)strlen(palabra);

    do
    {
        if(largo > 0)
            *(texto + largo++) = ' ';

        memcpy(texto + largo, palabra, tam);
        largo += tam;

        palabra = elegirPalabra();
        tam = (unsigned)strlen(palabra);
    }
    while(largo + 1 + tam <= objetivo);
}

/* Tabla indexada por TEXTO_LETRAS / TEXTO_PALABRAS */
static const tGenerarTexto generadoresTexto[] =
{
    generarTextoLetrasAleatorias,
    generarTextoPalabrasAleatorias
};

/* ---------- archivos binarios ---------- */

/* Escribe el archivo de posteos: el id coincide con la posicion del registro */
static int generarPosteos(const char *nombre)
{
    tGenerarTexto generarTexto = *(generadoresTexto + TEXTO_ELEGIDO);
    FILE *pf = fopen(nombre, "wb");
    tPosteo post;
    int id;
    int ok = 1;

    if(!pf)
        return SIM_ERROR_ARCHIVO;

    for(id = 0; id < CANT_POSTEOS && ok; id++)
    {
        memset(&post, 0, sizeof(post));      /* archivo reproducible byte a byte */
        post.id = id;
        generarTexto(post.texto);
        ok = fwrite(&post, sizeof(post), 1, pf) == 1;
    }

    if(fclose(pf) != 0)
        ok = 0;

    return ok ? SIM_OK : SIM_ERROR_ARCHIVO;
}

/* Escribe el archivo de usuarios, cada feed con ids del generador del lote */
static int generarUsuarios(const char *nombre, int usuarios, tGenerarId generar, tCtxLote *ctx)
{
    FILE *pf = fopen(nombre, "wb");
    tUsuario usu;
    int i;
    int *p;
    int ok = 1;

    if(!pf)
        return SIM_ERROR_ARCHIVO;

    for(i = 1; i <= usuarios && ok; i++)
    {
        memset(&usu, 0, sizeof(usu));
        snprintf(usu.usuario, sizeof(usu.usuario), "usuario%05u", (unsigned)i % 100000u);

        for(p = usu.feed; p < usu.feed + LARGO_FEED; p++)
            *p = generar(ctx);

        ok = fwrite(&usu, sizeof(usu), 1, pf) == 1;
    }

    if(fclose(pf) != 0)
        ok = 0;

    return ok ? SIM_OK : SIM_ERROR_ARCHIVO;
}

/* Genera posteos.dat y usuarios_<tipo>.dat con sus .txt; la misma semilla reproduce el mismo lote */
int generarLote(unsigned long semilla, const char *tipo, int usuarios, int tamPool)
{
    tGenerarId generar = buscarTipo(tipo);
    char nombreUsu[64];
    tCtxLote ctx;
    int *p;
    int res;

    if(!generar || usuarios < USUARIOS_MIN || usuarios > USUARIOS_MAX
            || tamPool < 1 || tamPool > CANT_POSTEOS)
    {
        printf("Parametros invalidos (usuarios %d a %d, pool 1 a %d)\n",
               USUARIOS_MIN, USUARIOS_MAX, CANT_POSTEOS);
        return SIM_PARAM_INVALIDO;
    }

    ctx.total = (unsigned long)usuarios * LARGO_FEED;
    ctx.contador = 0;
    ctx.tamPool = tamPool;

    if(generar == idOrdenado && ctx.total > CANT_POSTEOS)
    {
        printf("Lote ordenado: usuarios * %d no puede superar %d (maximo %d usuarios)\n",
               LARGO_FEED, CANT_POSTEOS, CANT_POSTEOS / LARGO_FEED);
        return SIM_PARAM_INVALIDO;
    }

    ctx.pool = malloc(tamPool * sizeof(int));

    if(!ctx.pool)
    {
        printf("Sin memoria\n");
        return SIM_SIN_MEM;
    }

    srand((unsigned)semilla);

    for(p = ctx.pool; p < ctx.pool + tamPool; p++)
        *p = (int)aleatorio(CANT_POSTEOS);

    snprintf(nombreUsu, sizeof(nombreUsu), FORMATO_USUARIOS, tipo);

    res = generarPosteos(NOMBRE_POSTEOS);

    if(res == SIM_OK)
        res = generarUsuarios(nombreUsu, usuarios, generar, &ctx);

    if(res == SIM_OK)
        res = exportarATexto(NOMBRE_POSTEOS, sizeof(tPosteo), formatearPosteo);

    if(res == SIM_OK)
        res = exportarATexto(nombreUsu, sizeof(tUsuario), formatearUsuario);

    free(ctx.pool);

    if(res != SIM_OK)
    {
        printf("Error al escribir los archivos\n");
        return res;
    }

    printf("Lote '%s' (semilla %lu): %d posteos en %s, %d usuarios en %s (con sus .txt)\n",
           tipo, semilla, CANT_POSTEOS, NOMBRE_POSTEOS, usuarios, nombreUsu);

    return SIM_OK;
}
