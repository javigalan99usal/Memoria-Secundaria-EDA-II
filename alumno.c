/*		JAVIER GALANTE GÓMEZ	-	52417330P	-	PA3		*/
#include "dispersion.h"
#include "alumno.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int funcionHash(tAlumno *reg, int nCubos)
{
    int dni = atoi(reg->dni);
    return dni % nCubos;
}

void mostrarReg(tAlumno *reg)
{
    printf("\t%s %s %s %s %s\n", reg->dni, reg->nombre, reg->ape1, reg->ape2, reg->provincia);
}

int cmpClave(tAlumno *reg1, tAlumno *reg2)
{
    return strcmp(reg1->dni, reg2->dni);
}

int buscar(char *fichero, char *dni)
{
    FILE *f = fopen(fichero, "rb");
    if (f == NULL) // Compruebo si se ha podido abrir el fichero
    {
        printf("Error al abrir el fichero %s en la funcion buscar\n", fichero);
        return -2; // Error al abrir el fichero
    }

    tAlumno alumno = {0};         // Inicializo la estructura alumno con todos sus bytes a 0
    tPosicion pos = {-1, -1, -1}; // Inicializo la posición con todos sus elementos a -1
    int i;

    for (i = 0; i < strlen(dni); i++) // Copio el DNI a la estructura alumno
    {
        alumno.dni[i] = dni[i];
    }

    int temp = busquedaHash(f, &alumno, &pos); // Realizo la búsqueda en el fichero
    fclose(f);

    switch (temp)
    {
    case 0: // Si se ha encontrado el registro, lo muestra por pantalla
        printf("\n --> Registro encontrado:\n");
        mostrarReg(&alumno);
        printf("\n --> Información de la búsqueda:\n");
        printf("\tCubo: %d\n", pos.cubo);
        printf("\tCubo desborde: %d\n", pos.cuboDes);
        printf("\tPosición: %d\n", pos.posReg);
        return 0;
    case -1:
        printf(" --> No se ha encontrado el registro con DNI: %s\n\n", alumno.dni);
        return -1;
    }
    printf("Error en la función buscar\n\n");
    return -5;
}

int modificar(char *fichero, char *dni, char *provincia)
{
    FILE *f = fopen(fichero, "r+b");
    if (f == NULL)
    {
        printf("Error al abrir el fichero %s en la funcion modificar\n", fichero);
        return -2;
    }

    tAlumno alumno = {0};         // Inicializo la estructura alumno con todos sus bytes a 0
    tPosicion pos = {-1, -1, -1}; // Inicializo la posición con todos sus elementos a -1
    int i, temp;

    for (i = 0; i < strlen(dni); i++) // Copio el DNI a la estructura alumno
    {
        alumno.dni[i] = dni[i];
    }

    temp = busquedaHash(f, &alumno, &pos); // Realizo la búsqueda en el fichero
    switch (temp)
    {
    case 0: // Si se ha encontrado el registro, lo modifica
        printf("\n --> Registro encontrado:\n");
        mostrarReg(&alumno);
        printf("\n --> Cambiando provincia de %s a %s\n", alumno.provincia, provincia);
        for (i = 0; i < strlen(provincia); i++) // Copio la nueva provincia a la estructura alumno
        {
            alumno.provincia[i] = provincia[i];
        }
        temp = modificarReg(f, &alumno, &pos); // Modifico el registro en el fichero
        if (temp == 0)                         // Si se ha modificado correctamente, muestra el registro modificado
        {
            printf("\n --> Registro modificado:\n");
            mostrarReg(&alumno);
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
        printf(" --> No se ha encontrado el registro con DNI: %s\n\n", alumno.dni);
        fclose(f);
        return -1;
    }
    printf("Error en la función modificar\n");
    fclose(f);
    return -5;
}