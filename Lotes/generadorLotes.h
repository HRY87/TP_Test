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

/* Genera posteos.dat y usuarios_<tipo>.dat (y sus .txt); la misma semilla reproduce el mismo lote.
   tipo: "concentrado", "disperso" u "ordenado". Devuelve SIM_OK o un codigo de error. */
int generarLote(unsigned long semilla, const char *tipo, int usuarios, int tamPool);


#endif // GENERADORLOTES_H_INCLUDED
