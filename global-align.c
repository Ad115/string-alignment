/*
*****************************************************************************
                         ALINEAMIENTO GLOBAL v. 0.0						 	*
*****************************************************************************

***Funcionalidad:
-->	Se introducen 2 palabras a alinearse, junto con el esquema de costos y el tipo de matriz(distancia o similaridad)
-->	Se obtiene la matriz con dichas características y se imprime con punteros y sin punteros
-->	¡¡¡Obtiene todos los alineamientos óptimos desde la última entrada de la matriz!!!
-->	¡¡¡Imprime TODOS los alineamientos óptimos junto con las cadenas de edición asociadas!!!

El programa utiliza las funciones de las librerias alignment_matrix.h, general.h, alignment.h
Andrés García García @ 12/Mar/'16 (Inicio 19/Oct/'15) 
*/

#include<stdio.h>
#include<stdlib.h>//Para usar malloc()
#include<assert.h>//Para verificar errores con la función assert()
#include "general.h"//Incluye las demás librerías de alineamiento de secuencias
#include "alignment_matrix.h"
#include "alignment.h"

int debug; // Para debug :P



int main(int argc, char *argv[])
{
	
	//____________________Inicialización___________________________
	if( argc < 3 )//Debe llamarse con 2 argumentos mínimo!!!
	{
		printf("\nPara llamar el programa escriba en la terminal: %s <texto1> <texto2> [--scores='M<#M>R<#R>I<#I>D<#D>'] [--type=(min | max)]\n", argv[0]);
		printf("Ejemplo: %s vintners writers --scores=M20I-1D-1R-1 --type=max\n\n", argv[0]);
		return 1;
	}

	char *string1=argv[1], *string2=argv[2]; // Obten las secuencias de texto, estas siempre son los dos primeros argumentos
	char ***args = getArgs(argv, argc); // Obten las variables declaradas
	char *type=NULL;
	float *scores=NULL;
	int i;
	for(i=0; args[i] != NULL; i++)
	{
		if(equStr(args[i][0], "type"))
		{
			type=dupStr(args[i][1]);
			assert(type != NULL);
		}
		else
		{
			if(equStr(args[i][0], "scores"))
			{
				scores=getScores(args[i][1]);
			}
		}
	}
	for(i=0; args[i] != NULL; i++)//Libera el espacio que ya no se necesita
	{
		free(args[i][0]);
		free(args[i][1]);
		free(args[i]);
	}
	free(args);
	
	//____________________Operaciones___________________________

	
	//_____________________Resultados____________________________
	GlobalAlignment(string1, string2, type, scores);
	
	return 0;
 }