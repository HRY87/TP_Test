#ifndef SIMULADORTWEENER_H_INCLUDED
#define SIMULADORTWEENER_H_INCLUDED

/* Simula el lote 'tipo' (concentrado, disperso u ordenado) sin cache, con la
   cache de la Parte 1 y con la de la Parte 2, para varias capacidades.
   Cada simulacion se repite 'repeticiones' veces y se informa el promedio.
   Devuelve SIM_OK o un codigo de error. */
int ejecutarSimulacion(const char *tipo, int repeticiones);

#endif // SIMULADORTWEENER_H_INCLUDED
