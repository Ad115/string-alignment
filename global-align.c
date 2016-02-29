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
Andrés García García @ 28/Feb/'16 (Inicio 19/Oct/'15) 
*/

#include<stdio.h>
#include<stdlib.h>//Para usar malloc()

#include "general.h"//Incluye las demás librerías de alineamiento de secuencias
#include "alignment_matrix.h"
#include "alignment.h"



int main(int argc, char *argv[])
{
	
	//____________________Inicialización___________________________
	if( argc < 3 )//Debe llamarse con 2 argumentos mínimo!!!
	{
		printf("\nPara llamar el programa escriba en la terminal: %s <texto1> <texto2> [--scores='M<#M>R<#R>I<#I>D<#D>'] [--type=[min o max]]\n", argv[0]);
		printf("Ejemplo: %s vintners writers --scores=M20I-1D-1R-1 --type=max\n\n", argv[0]);
		return 1;
	}
	
	char *string1=argv[1], *string2=argv[2];//Obten las secuencias de texto, estas siempre son los dos primeros argumentos
	char ***args=getArgs(argv, argc);//Obtén los demás argumentos
	char *type=NULL;
	float *scores=NULL;
	int i; char *var=(char *)calloc(1, sizeof(char));
	for(i=0; args[i][0]!=""; i++)
	{
		if(equStr(args[i][0], "type"))
		{
			type=args[i][1];
		}
		else
		{
			if(equStr(args[i][0], "scores"))
			{
				scores=getScores(args[i][1]);
			}
		}
	}
	free(args);

	
	//____________________Operaciones___________________________

	
	//_____________________Resultados____________________________
	GlobalAlignment(string1, string2, type, scores);
	return 0;
}