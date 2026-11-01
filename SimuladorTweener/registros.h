#ifndef REGISTROS_H_INCLUDED
#define REGISTROS_H_INCLUDED

#define CANT_POSTEOS     50000
#define LARGO_TEXTO      141
#define LARGO_USUARIO    15
#define LARGO_FEED       20

#define NOMBRE_POSTEOS   "posteos.dat"
#define FORMATO_USUARIOS "usuarios_%s.dat"   /* %s = concentrado, disperso u ordenado */

/* Codigos de retorno del simulador (propios para no chocar con los de Arbol.h) */
#define SIM_OK               0
#define SIM_SIN_MEM          1
#define SIM_PARAM_INVALIDO   2
#define SIM_ERROR_ARCHIVO    3

typedef struct
{
    int id;                       /* coincide con la posicion del registro */
    char texto[LARGO_TEXTO];
} tPosteo;

typedef struct
{
    char usuario[LARGO_USUARIO];
    int feed[LARGO_FEED];
} tUsuario;

#endif // REGISTROS_H_INCLUDED
