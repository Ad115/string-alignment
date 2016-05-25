/*
================================================
Proyecto: Alineamiento de cadenas (string-align)
================================================

Implementación de alineamiento local y global en C.
Hasta ahora sólo se han implementado los alineamientos locales.

 - Compilar:
 	```
	make global-align
 	```
    o bien, con:
 	```
	gcc global-align.c -o global-align
 	```

 - Ejecutar:
   + Comando de ejemplo:
	```
    global-align vintners writers --scores=M20I-1D-1R-1 --type=max
	```
   + Salida de ejemplo:
	```
      Alineamiento Global.
      --------------------

      Str1:   vintners
      Str2:   writers
      Scores: [20.000000, -1.000000, -1.000000, -1.000000]
      Alineamiento por similaridad (max).

      Alineamiento número 1:
      Score:  96.000000
      Str1:   _ v i n t n e r s
      Str2:   w r i _ t _ e r s
      EditTr: I R M D M D M M M

      Alineamiento número 2:
      Score:  96.000000
      Str1:   v _ i n t n e r s
      Str2:   w r i _ t _ e r s
      EditTr: R I M D M D M M M
	```

 - Próximos cambios:
  * Mejorar documentación.
  * Posibilidad de añadir las cadenas desde archivos externos.
  * Implementar alineamientos locales.


Funcionalidad
-------------

- Se introducen 2 palabras a alinearse, junto con el esquema de costos y el tipo de matriz (distancia o similaridad).

- Se obtiene la matriz con dichas características.

- ¡¡¡Obtiene todos los alineamientos óptimos desde la última entrada de la matriz!!!

- ¡¡¡Imprime **todos** los alineamientos óptimos junto con las cadenas de edición asociadas!!!


El programa utiliza las funciones de las siguientes librerias:
- **general.h**
- **alignment_matrix.h**
- **traceback.h**
- **alignment.h**, todas las anteriores se incluyen mediante el archivo de declaraciones **alignments_headers.h**

Autor
-----

**Andrés García García** @ Sab 26/Mar/'16 (Inicio del proyecto: 19/Oct/'15)
*/

# include <stdio.h>
# include <stdlib.h>//Para usar malloc()
# include <assert.h>//Para verificar errores con la función assert()
//Incluye las librerías de alineamiento de secuencias
# include "alignments_headers.h"
# include "general.h"
# include "alignment_matrix.h"
# include "traceback.h"
# include "alignment.h"

int debug; // Para debug :P

//===============>> INICIO DE FUNCIÓN MAIN <<=====================

int main(int argc, char *argv[])
{

	//____________________Inicialización___________________________
	if( argc < 3 )//Debe llamarse con 2 argumentos mínimo!!!
	{
		printf("\nPara llamar el programa escriba en la terminal:\n");
		printf("%s <texto1> <texto2> [--scores='M<#M>R<#R>I<#I>D<#D>'] [--type=(min | max)]\n", argv[0]);
		printf("Ejemplo: %s vintners writers --scores=M20I-1D-1R-1 --type=max\n\n", argv[0]);
		return 1;
	}

	// Obten las secuencias de texto, estas siempre son los dos primeros argumentos.
	char *string1=argv[1], *string2=argv[2];
	 // Obten las variables declaradas.
	char ***args = getArgs(argv, argc);
	char *type=NULL; float *scores=NULL; int i;
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

 //===============>> FIN DE FUNCIÓN MAIN <<=====================
