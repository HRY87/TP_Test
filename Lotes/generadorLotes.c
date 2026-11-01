#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Lotes/generadorLotes.h"
#include "../SimuladorTweener/registros.h"
#include "../Lotes/exportador.h"

#define ALFABETO         "abcdefghijklmnopqrstuvwxyz "
#define CANT_PALABRAS    (sizeof(palabras) / sizeof(*palabras))

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

/* Tabla indexada por TEXTO_LETRAS / TEXTO_PALABRAS */
static const tGenerarTexto generadoresTexto[] =
{
    generarTextoLetrasAleatorias,
    generarTextoPalabrasAleatorias
};

/* Devuelve un numero aleatorio en [0, n). Usa dos rand() en sentencias separadas
   para tener orden definido y cubrir rangos mayores a RAND_MAX */
unsigned long aleatorio(unsigned long n)
{
    unsigned long alto = (unsigned long)rand();
    unsigned long bajo = (unsigned long)rand();

    return (alto * 32768UL + bajo) % n;
}

/* ---------- generadores de id (uno por tipo de lote) ---------- */

/* Lote concentrado: elige siempre dentro de un conjunto chico de posteos populares */
int idConcentrado(tCtxLote *ctx)
{
    return *(ctx->pool + aleatorio((unsigned long)ctx->tamPool));
}

/* Lote disperso: cualquier posteo con igual probabilidad */
int idDisperso(tCtxLote *ctx)
{
    (void)ctx;
    return (int)aleatorio(CANT_POSTEOS);
}

/* Lote ordenado: ids estrictamente crecientes en toda la simulacion (requiere total <= CANT_POSTEOS) */
int idOrdenado(tCtxLote *ctx)
{
    int id = (int)((ctx->contador * CANT_POSTEOS) / ctx->total);

    ctx->contador++;

    return id;
}

/* Busca la funcion generadora de un tipo de lote; NULL si no existe */
tGenerarId buscarTipo(const char *nombre)
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
void generarTextoLetrasAleatorias(char *texto)
{
    const char *fin = texto + 1 + aleatorio(MAX_CARACTERES);
    char *p;

    for(p = texto; p < fin; p++)
        *p = *(ALFABETO + aleatorio(strlen(ALFABETO)));
}

/* Elige una palabra al azar de la lista */
const char *elegirPalabra(void)
{
    return *(palabras + aleatorio(CANT_PALABRAS));
}

/* Arma el texto con palabras separadas por un espacio hasta un largo objetivo
   aleatorio de 1 a MAX_CARACTERES. Lleva al menos una palabra y nunca supera
   MAX_CARACTERES; el '\0' final lo deja el memset previo del registro. */
void generarTextoPalabrasAleatorias(char *texto)
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

/* ---------- armado de registros y escritura de archivos ---------- */

/* Completa un posteo: el id es su posicion en el archivo y el texto lo arma el generador elegido */
void armarPosteo(void *registro, int numero, void *contexto)
{
    tPosteo *post = registro;

    (void)contexto;
    post->id = numero;
    (*(generadoresTexto + TEXTO_ELEGIDO))(post->texto);
}

/* Completa un usuario: nombre correlativo y feed con ids del generador del lote */
void armarUsuario(void *registro, int numero, void *contexto)
{
    tUsuario *usu = registro;
    tCtxLote *ctx = contexto;
    int *p;

    snprintf(usu->usuario, sizeof(usu->usuario), "usuario%05u", (unsigned)(numero + 1) % 100000u);

    for(p = usu->feed; p < usu->feed + LARGO_FEED; p++)
        *p = ctx->generar(ctx);
}

/* Escribe 'cantidad' registros en el archivo; cada uno se pone en cero antes de armarlo
   para que el archivo sea reproducible byte a byte */
int escribirArchivoRegistros(const char *nombre, int cantidad, unsigned tamRegistro,
                             tArmarRegistro armar, void *contexto)
{
    FILE *pf = fopen(nombre, "wb");
    void *registro = malloc(tamRegistro);
    int i;
    int ok = 1;

    if(!pf || !registro)
    {
        if(pf)
            fclose(pf);

        free(registro);
        return pf ? SIM_SIN_MEM : SIM_ERROR_ARCHIVO;
    }

    for(i = 0; i < cantidad && ok; i++)
    {
        memset(registro, 0, tamRegistro);
        armar(registro, i, contexto);
        ok = fwrite(registro, tamRegistro, 1, pf) == 1;
    }

    if(fclose(pf) != 0)
        ok = 0;

    free(registro);

    return ok ? SIM_OK : SIM_ERROR_ARCHIVO;
}

/* ---------- preparacion del lote ---------- */

/* Verifica tipo, cantidad de usuarios y pool; informa el motivo si algo es invalido */
int validarLote(tGenerarId generar, int usuarios, int tamPool)
{
    if(!generar || usuarios < USUARIOS_MIN || usuarios > USUARIOS_MAX
            || tamPool < 1 || tamPool > CANT_POSTEOS)
    {
        printf("Parametros invalidos (usuarios %d a %d, pool 1 a %d)\n",
               USUARIOS_MIN, USUARIOS_MAX, CANT_POSTEOS);
        return SIM_PARAM_INVALIDO;
    }

    if(generar == idOrdenado && (unsigned long)usuarios * LARGO_FEED > CANT_POSTEOS)
    {
        printf("Lote ordenado: usuarios * %d no puede superar %d (maximo %d usuarios)\n",
               LARGO_FEED, CANT_POSTEOS, CANT_POSTEOS / LARGO_FEED);
        return SIM_PARAM_INVALIDO;
    }

    return SIM_OK;
}

/* Inicializa el contexto: siembra el generador y arma el pool de posteos populares.
   El pool se sortea siempre (aunque el lote no lo use) para que una misma semilla
   produzca siempre los mismos archivos. */
int iniciarContextoLote(tCtxLote *ctx, tGenerarId generar, int usuarios, int tamPool, unsigned long semilla)
{
    int *p;

    ctx->generar = generar;
    ctx->total = (unsigned long)usuarios * LARGO_FEED;
    ctx->contador = 0;
    ctx->tamPool = tamPool;
    ctx->pool = malloc(tamPool * sizeof(int));

    if(!ctx->pool)
        return SIM_SIN_MEM;

    srand((unsigned)semilla);

    for(p = ctx->pool; p < ctx->pool + tamPool; p++)
        *p = (int)aleatorio(CANT_POSTEOS);

    return SIM_OK;
}

/* Escribe los dos binarios del lote y sus versiones .txt (en ese orden) */
int escribirArchivosLote(const char *nombreUsuarios, int usuarios, tCtxLote *ctx)
{
    int res = escribirArchivoRegistros(NOMBRE_POSTEOS, CANT_POSTEOS, sizeof(tPosteo), armarPosteo, NULL);

    if(res == SIM_OK)
        res = escribirArchivoRegistros(nombreUsuarios, usuarios, sizeof(tUsuario), armarUsuario, ctx);

    if(res == SIM_OK)
        res = exportarATexto(NOMBRE_POSTEOS, sizeof(tPosteo), formatearPosteo);

    if(res == SIM_OK)
        res = exportarATexto(nombreUsuarios, sizeof(tUsuario), formatearUsuario);

    return res;
}

/* Genera posteos.dat y usuarios_<tipo>.dat con sus .txt; la misma semilla reproduce el mismo lote */
int generarLote(unsigned long semilla, const char *tipo, int usuarios, int tamPool)
{
    tGenerarId generar = buscarTipo(tipo);
    char nombreUsu[64];
    tCtxLote ctx;
    int res = validarLote(generar, usuarios, tamPool);

    if(res != SIM_OK)
        return res;

    res = iniciarContextoLote(&ctx, generar, usuarios, tamPool, semilla);

    if(res != SIM_OK)
    {
        printf("Sin memoria\n");
        return res;
    }

    snprintf(nombreUsu, sizeof(nombreUsu), FORMATO_USUARIOS, tipo);
    res = escribirArchivosLote(nombreUsu, usuarios, &ctx);
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
