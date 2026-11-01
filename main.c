#include <stdio.h>
#include <stdlib.h>
#include "Lotes/generadorLotes.h"
#include "SimuladorTweener/simuladorTweener.h"
#include "Test/TestPart1.h"
#include "Test/TestParte2.h"

#define OP_SALIR        0
#define OP_CONCENTRADO  1
#define OP_DISPERSO     2
#define OP_ORDENADO     3
#define OP_TEST_PARTE1  4
#define OP_TEST_PARTE2  5
#define OP_SIMULACION   6

#define SEMILLA_DEFECTO 42

/* Lee un numero; si la linea esta vacia (o no hay entrada) devuelve defecto */
static unsigned long leerNumero(const char *mensaje, unsigned long defecto)
{
    char buf[32];
    char *fin;
    unsigned long n;

    printf("%s [%lu]: ", mensaje, defecto);

    if(!fgets(buf, sizeof(buf), stdin))
        return defecto;

    n = strtoul(buf, &fin, 10);

    return fin == buf ? defecto : n;
}

/* Muestra las opciones del menu */
static void mostrarMenu(void)
{
    printf("\n===== LRU Cache =====\n");
    printf("1. Generar lote concentrado\n");
    printf("2. Generar lote disperso\n");
    printf("3. Generar lote ordenado\n");
    printf("4. Test Parte 1 (lista simple)\n");
    printf("5. Test Parte 2 (lista doble + arbol)\n");
    printf("6. Simulacion Tweener\n");
    printf("0. Salir\n");
}

/* Pide los parametros y genera un lote del tipo indicado */
static void opcionGenerar(const char *tipo, int pideTamPool)
{
    unsigned long semilla = leerNumero("Semilla", SEMILLA_DEFECTO);
    int usuarios = (int)leerNumero("Usuarios", USUARIOS_MIN);
    int tamPool = POOL_DEFECTO;

    if(pideTamPool)
        tamPool = (int)leerNumero("Posteos populares (pool)", POOL_DEFECTO);

    generarLote(semilla, tipo, usuarios, tamPool);
}

/* Pide el lote y las repeticiones y ejecuta la simulacion */
static void opcionSimular(void)
{
    static const char *tipos[] = {"concentrado", "disperso", "ordenado"};
    const char **p = tipos;
    const char **fin = tipos + sizeof(tipos) / sizeof(*tipos);
    int lote = (int)leerNumero("Lote (1 concentrado, 2 disperso, 3 ordenado, 4 todos)", 4);
    int reps = (int)leerNumero("Repeticiones", 10);

    if(lote >= 1 && lote <= 3)
        ejecutarSimulacion(*(tipos + lote - 1), reps);
    else if(lote == 4)
    {
        for(; p < fin; p++)
            ejecutarSimulacion(*p, reps);
    }
    else
        printf("Lote invalido.\n");
}

/* Ejecuta la accion asociada a la opcion elegida */
static void ejecutarOpcion(int op)
{
    if(op == OP_CONCENTRADO)
        opcionGenerar("concentrado", 1);
    else if(op == OP_DISPERSO)
        opcionGenerar("disperso", 0);
    else if(op == OP_ORDENADO)
        opcionGenerar("ordenado", 0);
    else if(op == OP_TEST_PARTE1)
        ejecutar_tests_lrucache();
    else if(op == OP_TEST_PARTE2)
        test_lrucache_parte2();
    else if(op == OP_SIMULACION)
        opcionSimular();
    else if(op != OP_SALIR)
        printf("Opcion invalida.\n");
}

int main(void)
{
    int op;

    do
    {
        mostrarMenu();
        op = (int)leerNumero("Opcion", OP_SALIR);   /* sin entrada: sale */
        ejecutarOpcion(op);
    }
    while(op != OP_SALIR);

    return 0;
}
