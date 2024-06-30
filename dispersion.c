/*		JAVIER GALANTE GÓMEZ	-	52417330P	-	PA3		*/

#include "dispersion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lee el contenido del fichero hash organizado mediante el método de DISPERSIÓN según los criterios
// especificados en la práctica. Se leen todos los cubos completos tengan registros asignados o no. La
// salida que produce esta función permite visualizar el método de DISPERSIÓN

int leeHash(char *fichHash)
{
    FILE *f;
    tipoCubo cubo;
    regConfig regC;
    int j, nCubo = 0, densidadOcupacion;

    if ((f = fopen(fichHash, "rb")) == NULL)
        return -2;
    fread(&regC, sizeof(regConfig), 1, f);
    fread(&cubo, sizeof(cubo), 1, f);
    while (!feof(f))
    {
        for (j = 0; j < C; j++)
        {
            if (j == 0)
            {
                fflush(stdout);
                printf("-------------------------------------------------------------------------------------- \n"); 
                printf("Cubo %2d (%2d reg. ASIGNADOS)", nCubo, cubo.numRegAsignados);
            }
            else if ((j == 1) && cubo.desbordado)
                printf("DESBORDADO\t\t");
            else
                printf("\t\t\t");
            if (j < cubo.numRegAsignados)
                mostrarReg(&(cubo.reg[j]));
            else
                printf("\n");
        }
        nCubo++;
        fread(&cubo, sizeof(cubo), 1, f);
    }
    fclose(f);
    printf("ORGANIZACIÓN MÉTODO DISPERSIÓN:\n\t %d CUBOS y %d CUBOS DESBORDE con capacidad %d\n",
           regC.nCubos, regC.nCubosDes, C);
    printf("\t Contiene %d registros de los cuales se han desbordado %d\n", regC.numReg, regC.numRegDes);

    densidadOcupacion = 100 * regC.numReg / ((regC.nCubos + regC.nCubosDes) * C);
    printf("Densidad ACTUAL de ocupación: %d \n\t(MÍNIMA permitida %d  MÁXIMA permitida %d)\n",
           densidadOcupacion, regC.densidadMin, regC.densidadMax);

    if (densidadOcupacion > regC.densidadMax)
    {
        printf("No se respeta la densidad máxima de ocupacion\n");
    }

    if (densidadOcupacion < regC.densidadMin)
    {
        printf("No se respeta la densidad mínima de ocupacion\n");
    }
    return 0;
}

int creaHvacio(char *fichHash, regConfig *regC)
{
    FILE *f = fopen(fichHash, "wb");
    if (f == NULL)
    {
        printf("Error al abrir el fichero %s en la funcion creaVacioH\n", fichHash);
        return -2; // Si no ha sido posible abrir el fichero
    }

    // Inicializo un cubo vacío
    tipoCubo cuboVacio;
    memset(&cuboVacio.reg, 0, sizeof(cuboVacio.reg));
    cuboVacio.numRegAsignados = 0;
    cuboVacio.desbordado = 0;
    // Escribo en el fichero el registro de configuración y los cubos vacíos
    fwrite(regC, sizeof(regConfig), 1, f);
    for (int i = 0; i < regC->nCubos + regC->nCubosDes; i++)
    {
        fwrite(&cuboVacio, sizeof(tipoCubo), 1, f);
    }
    fclose(f);
    return 0;
}

int creaHash(char *fichEntrada, char *fichHash, regConfig *regC)
{
    printf(" ... Creando fichero %s\n", fichHash);

    FILE *fEntrada = fopen(fichEntrada, "rb");
    if (fEntrada == NULL)
    {
        printf("Error al abrir el fichero %s en la funcion creaHash\n", fichEntrada);
        return -1;
    }

    if (creaHvacio(fichHash, regC) != 0)
    {
        printf("Error al crear el fichero vacío %s en la funcion creaHash\n", fichHash);
        fclose(fEntrada);
        return -2;
    }

    FILE *fHash = fopen(fichHash, "r+b");
    if (fHash == NULL)
    {
        printf("Error al abrir el fichero %s en la funcion creaHash\n", fichHash);
        fclose(fEntrada);
        return -2;
    }

    tipoReg registro;
    int posicionCubo;
    tipoCubo cubo;

    while (fread(&registro, sizeof(tipoReg), 1, fEntrada))
    {
        posicionCubo = funcionHash(&registro, regC->nCubos);                         // Calculo el cubo en el que posicionar el registro según la función hash
        fseek(fHash, sizeof(regConfig) + posicionCubo * sizeof(tipoCubo), SEEK_SET); // Coloco el puntero al inicio del cubo en el que posicionar el registro

        fread(&cubo, sizeof(tipoCubo), 1, fHash); // Leo el cubo para ver si tiene espacio para el registro
        if (cubo.numRegAsignados < C)             // Escribo el registro en el primer hueco libre del cubo correspondiente
        {
            cubo.reg[cubo.numRegAsignados] = registro; // Añado el registro en el cubo
            cubo.numRegAsignados++;                    // Incremento el número de registros asignados al cubo
            fseek(fHash, -sizeof(tipoCubo), SEEK_CUR); // Coloco el puntero en el lugar adecuado para la escritura del cubo en el fichero
            fwrite(&cubo, sizeof(tipoCubo), 1, fHash); // Escribo el cubo con el registro añadido modificado en el fichero
            regC->numReg++;                            // Incremento el número de registros del fichero en el registro de configuración
        }
        else
        {
            if (cubo.desbordado != 1) // Si aun no hemos indicado que el cubo se ha desbordado
            {
                cubo.desbordado = 1;                       // Indico que el cubo está desbordado
                fseek(fHash, -sizeof(tipoCubo), SEEK_CUR); // Coloco el puntero en el lugar adecuado para la escritura del cubo en el fichero
                fwrite(&cubo, sizeof(tipoCubo), 1, fHash); // Escribo el cubo con la variable desbordado modificada en el fichero
            }

            if (desborde(fHash, &registro, regC) != 0)
            {
                printf("Error al desbordar el fichero %s en la funcion creaHash\n", fichHash);
                fclose(fEntrada);
                fclose(fHash);
                return -5;
            }
            regC->numRegDes++; // Incremento el número de registros del fichero en el registro de configuración
            regC->numReg++;    // Incremento el número de registros del fichero en el registro de configuración
        }
    }

    fseek(fHash, 0, SEEK_SET);                 // Coloco el puntero al inicio del fichero para poder escribir el registro de configuración
    fwrite(regC, sizeof(regConfig), 1, fHash); // Actualizo el registro de configuración con el nuevo número de registros y registros desbordados

    fclose(fEntrada);
    fclose(fHash);
    printf("  --> Fichero %s creado con éxito\n", fichHash);
    return 0;
}

int desborde(FILE *fHash, tipoReg *reg, regConfig *regC)
{
    fseek(fHash, sizeof(regConfig) + regC->nCuboDesAct * sizeof(tipoCubo), SEEK_SET); // Coloco el pntero al inicio del cubo de desborde activo
    tipoCubo cubo;
    if (fread(&cubo, sizeof(tipoCubo), 1, fHash) == 1) // Leo el cubo de desborde activo y compruebo si tiene espacio para el registro
    {
        if (cubo.numRegAsignados < C) // Escribo el registro en el primer hueco libre del cubo de desborde activo
        {
            cubo.reg[cubo.numRegAsignados] = *reg;     // Añado el registro en el cubo
            cubo.numRegAsignados++;                    // Incremento el número de registros asignados al cubo
            fseek(fHash, -sizeof(tipoCubo), SEEK_CUR); // Coloco el puntero en el lugar adecuado para la escritura en el fichero
            fwrite(&cubo, sizeof(tipoCubo), 1, fHash); // Escribo el cubo con el registro añadido modificado en el fichero
        }
        else
        {
            regC->nCuboDesAct++;                       // Incremento el número del cubo de desborde activo
            fseek(fHash, 0, SEEK_SET);                 // Coloco el puntero al inicio del fichero para poder escribir el registro de configuración
            fwrite(regC, sizeof(regConfig), 1, fHash); // Actualizo el registro de configuración con el nuevo número de cubo de desborde actual
            desborde(fHash, reg, regC);                // Llamada recursiva para desbordar el siguiente cubo de desborde
        }
        return 0;
    }
    else
    {
        printf("Error al leer el cubo de desborde activo el la funcion desborde\n");
        return -1; // Error al leer el cubo de desborde activo
    }
}

int busquedaHash(FILE *fHash, tipoReg *reg, tPosicion *posicion)
{
    int i, j, cuboHash;
    fseek(fHash, 0, SEEK_SET); // Coloco el puntero al inicio del fichero
    regConfig config;
    fread(&config, sizeof(regConfig), 1, fHash); // Leo el registro de configuración

    cuboHash = funcionHash(reg, config.nCubos);                              // Calculo el cubo en el que se encuentra el registro según la función hash
    fseek(fHash, sizeof(regConfig) + cuboHash * sizeof(tipoCubo), SEEK_SET); // Coloco el puntero al inicio del cubo que me ha indicado la función hash

    tipoCubo cubo;
    fread(&cubo, sizeof(tipoCubo), 1, fHash); // Leo el cubo que me ha indicado la funcion hash

    for (i = 0; i < cubo.numRegAsignados; i++) // Busqueda secuencial del registro en el cubo indicado por la funcion hash
    {
        if (cmpClave(&cubo.reg[i], reg) == 0) // Comparo el registro con el que busco
        {
            *reg = cubo.reg[i];        // Copio el registro encontrado en el registro de entrada
            posicion->cubo = cuboHash; // Indico que el registro se encuentra en el cubo que me ha indicado la función hash
            posicion->posReg = i;      // Indico la posición del registro en el cubo
            posicion->cuboDes = -1;    // Indico que el registro no se encuentra en un cubo de desborde
            return 0;                  // Registro encontrado
        }
    }
    // Si no ha encontrado el registro en el cubo indicado por la función hash, busca en los cubos de desborde
    if (cubo.desbordado)
    {
        fseek(fHash, sizeof(regConfig) + config.nCubos * sizeof(tipoCubo), SEEK_SET); // Coloco el puntero al inicio del primer cubo de desborde
        for (i = config.nCubos; i < config.nCubos + config.nCubosDes; i++)            // Recorro los cubos de desborde
        {
            fread(&cubo, sizeof(tipoCubo), 1, fHash);  // Leo el cubo de desborde
            for (j = 0; j < cubo.numRegAsignados; j++) // Busqueda secuencial del registro en el cubo de desborde
            {
                if (cmpClave(&cubo.reg[j], reg) == 0) // Comparo el registro con el que busco
                {
                    *reg = cubo.reg[j];                    // Copio el registro encontrado en el registro de entrada
                    posicion->cubo = i;             // Indico que el registro se debería encontrar en el cubo que me ha indicado la función hash
                    posicion->posReg = j;                  // Indico la posición del registro en el cubo
                    posicion->cuboDes = i - config.nCubos; // Indico que el registro se encuentra en uno de los cubos de desborde
                    return 0;                              // Registro encontrado
                }
            }
        }
    }
#ifdef ALUM
    printf("No se ha encontrado el registro con DNI: %s en el fichero\n", reg->dni); // Si no ha encontrado el registro en el fichero
#endif
#ifdef ASIG
    printf("No se ha encontrado el registro con código: %d en el fichero\n", reg->codigo); // Si no ha encontrado el registro en el fichero
#endif
    return -1; // Registro no encontrado
}

int modificarReg(FILE *fHash, tipoReg *reg, tPosicion *posicion)
{
    size_t escrito;
    fseek(fHash, sizeof(regConfig) + posicion->cubo * sizeof(tipoCubo) + posicion->posReg * sizeof(tipoReg), SEEK_SET); // Coloco el puntero al inicio del registro a modificar
    escrito = fwrite(reg, sizeof(tipoReg), 1, fHash);                                                                   // Escribo el registro modificado en el fichero
    if (escrito != 1)
    {
        printf("Error al escribir el registro en la función modificarReg\n");
        return -5; // Error al escribir el registro
    }
    return 0; // Modificación exitosa
}
