/*		JAVIER GALANTE GÓMEZ	-	52417330P	-	PA3		*/
#include "dispersion.h"
#include "asignatura.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int funcionHash(tAsignatura *reg, int nCubos)
{
    return reg->codigo % nCubos;
}

void mostrarReg(tAsignatura *reg)
{
    printf("\t%d %s %c %.1f %.1f %c %c %d / %d\n", reg->codigo, reg->nombre, reg->curso, reg->creditosT, reg->creditosP, reg->tipo, reg->cuatrimestre, reg->numGrT, reg->numGrP);
}

int cmpClave(tAsignatura *reg1, tAsignatura *reg2)
{
    if (reg1->codigo == reg2->codigo)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int buscar(char *fichero, int codigo)
{
    FILE *f = fopen(fichero, "rb");
    if (f == NULL) // Compruebo si se ha podido abrir el fichero
    {
        printf("Error al abrir el fichero %s en la funcion buscar\n", fichero);
        return -2; // Error al abrir el fichero
    }

    tAsignatura asignatura = {0}; // Inicializo la estructura asignatura con todos sus bytes a 0
    tPosicion pos = {-1, -1, -1}; // Inicializo la posición con todos sus elementos a -1
    int i;

    asignatura.codigo = codigo; // Copio el código a la estructura asignatura

    int temp = busquedaHash(f, &asignatura, &pos); // Realizo la búsqueda en el fichero
    fclose(f);

    switch (temp)
    {
    case 0: // Si se ha encontrado el registro, lo muestra por pantalla
        printf("\n --> Registro encontrado:\n");
        mostrarReg(&asignatura);
        printf("\n --> Información de la búsqueda:\n");
        printf("\tCubo: %d\n", pos.cubo);
        printf("\tCubo desborde: %d\n", pos.cuboDes);
        printf("\tPosición: %d\n", pos.posReg);
        return 0;
    case -1:
        printf(" --> No se ha encontrado el registro con código: %d\n\n", asignatura.codigo);
        return -1;
    }
    printf("Error en la función buscar\n\n");
    return -5;
}

int modificar(char *fichero, int codigo, float creditosT, float creditosP)
{
    FILE *f = fopen(fichero, "r+b");
    if (f == NULL) // Compruebo si se ha podido abrir el fichero
    {
        printf("Error al abrir el fichero %s en la funcion modificar\n", fichero);
        return -2; // Error al abrir el fichero
    }

    tAsignatura asignatura = {0}; // Inicializo la estructura asignatura con todos sus bytes a 0
    tPosicion pos = {-1, -1, -1}; // Inicializo la posición con todos sus elementos a -1
    int i;

    asignatura.codigo = codigo; // Copio el código a la estructura asignatura

    int temp = busquedaHash(f, &asignatura, &pos); // Realizo la búsqueda en el fichero
    switch (temp)
    {
    case 0: // Si se ha encontrado el registro, lo modifica
        printf("\n --> Registro encontrado:\n");
        mostrarReg(&asignatura);
        printf("\n --> Cambiando creditos teoricos de %.1f a %.1f\n", asignatura.creditosT, creditosT);
        asignatura.creditosT = creditosT;
        printf("\n --> Cambiando creditos practicos de %.1f a %.1f\n", asignatura.creditosP, creditosP);
        asignatura.creditosP = creditosP;

        temp = modificarReg(f, &asignatura, &pos); // Modifico el registro en el fichero
        if (temp == 0)                             // Si se ha modificado correctamente, muestra el registro modificado
        {
            printf("\n --> Registro modificado:\n");
            mostrarReg(&asignatura);
            fclose(f);
            return 0;
        }
        else // Si no se ha podido modificar, muestra un mensaje de error
        {
            printf("Error al modificar el registro\n");
            fclose(f);
            return -5;
        }
    case -1: // Si no se ha encontrado el registro, muestra un mensaje de error
#ifdef ALUM
        printf(" --> No se ha encontrado el registro con DNI: %s\n\n", alumno.dni);
#endif
#ifdef ASIG
        printf(" --> No se ha encontrado el registro con código: %d\n\n", asignatura.codigo);
#endif
        fclose(f);
        return -1;
    }
    printf("Error en la función modificar\n");
    fclose(f);
    return -5;
}