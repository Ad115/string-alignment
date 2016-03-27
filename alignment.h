# ifndef STRING_ALIGN_ALIGNMENTS
# define STRING_ALIGN_ALIGNMENTS
/*
========================
Librería: Alineamientos.
========================

Funciones principales para crear alineamientos explícitos de cadenas por medio de la estructura Align.
Contiene las funciones principales para crear alineamientos.
Contiene funciones especializadas para crear, llenar, imprimir y destruir alineamientos.
Contiene funciones para crear e imprimir los códigos de edición de los alineamientos.

Esta librería utiliza las siguientes estructuras como base::

		typedef struct Path_struct
		// Elemento fundamental de la estructura Traceback
		{
			int iSize;//Número de entradas inicializadas
			int uSize;//Número de entradas no inicializadas
			float pScore;//Score del camino
			int **Path;//Arreglo de entradas, cada entrada consiste de tres enteros, que comprenden una posición en la matriz y un puntero
		} Path;
		//___________________________________________________________
		
		
		typedef struct Traceback_struct
		//Contiene implícitamente cada alineamiento desde una celda de una matriz de alineamiento A_Matrix
		{
			char *Str1;
			char *Str2;
			int iPaths;//Número de caminos inicializados
			int uPaths;//Número de caminos no inicializados
			Path *Paths;//Arreglo de caminos
			
		} Traceback;
		//___________________________________________________________
		
		
		typedef struct Align_struct
		//Estructura que contiene un alineamiento explícito de dos textos
		{
			float Score;
			char *Align1;
			char *Align2;
			
		} Align;
		//___________________________________________________________


Las siguientes son las funciones declaradas aquí:

:void GlobalAlignment(const char *str1, const char *str2, const char *type, const float *scores):
	Obtiene e imprime todos los alineamientos globales óptimos de str1 y str2 con las características especificadas.
	
:Align *ExplAlignsFromTraceback(const Traceback *traceback):
	Obtiene los alineamientos desde los caminos obtenidos.
	
:void PrintExplAlign(const Align *align):
	Muestra el alineamiento explícito Align.
	
:void PrintAlignments(const Align *align):
	Imprime los alineamientos en el arreglo alignments.
	
:void FreeAligns(Align *alignments):
	Libera el espacio ocupado por la estructura alignments previamente alojada por ExplAlignsFromTraceback().
	
:char *EditTrFromExplAlign(const Align *align):
	Genera el código de edición asociado al alineamiento Align.
	
void PrintEditTr(const char *edit_tr):
	Muestra el código de edición en el mismo formato en que se muestra el alineamiento en PrintExplAlign().


Los prototipos de las funciones y las estructuras están declarados en el archivo alignments_headers.h

Los algoritmos están basados en los encontrados en el libro:
	*Algoritms on Strings, Trees and Sequences-Computer Science and Computational Biology*
	Dan Gusfield
	Cambridge University Press (1997)
	
:Autor:
	Andrés García García @ 28/Feb/'16 (Inicio 19 Oct 2015)
*/

//Librerías
# include <stdio.h>
# include <assert.h>//Para verificar errores con la función assert()
# include <stdlib.h>//Para usar malloc(), calloc(), realloc(), free()
# include <string.h>//Para usar strlen() y strcpy()
# include "alignments_headers.h"

int debug; // Variable global para debugear :P


/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Funciones para recuperar alineamientos.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


void GlobalAlignment(const char *str1, const char *str2, const char *type, const float *scores)
/*
 * A partir de las cadenas, costos y el tipo de alineamiento, obtiene los alineamientos globales óptimos
 */
{
	//char *align_type=strdup("global");
	char align_type[]="global";
	printf("\n====================\nAlineamiento Global.\n====================\n\n");
	printf("Str1:\t%s\nStr2:\t%s\n", str1, str2);//Imprime las cadenas
	printf("Scores:\t[%f, %f, %f, %f]\n", scores[0], scores[1], scores[2], scores[3]);//Imprime los scores
	printf("Alineamiento por %s (%s).\n",
		   (equStr(type, "min") ? "distancia" : "similaridad"), type); // Imprime el modo de encontrar el score
	
	//Obtiene la matriz de alineamiento
	A_Matrix *AlignMatrix = AllocAlignMatrix(str1, str2, type, align_type, scores);//Genera espacio para la matriz
	
	FillAlignMatrix(AlignMatrix);//Llena la matriz
	//PrintAlignMatrix(AlignMatrix);
	
	//Obtén los caminos
	Traceback *traceback=TracebackFromMatrixEntry(AlignMatrix,-1, -1);
	
	//Libera el espacio ocupado por la matriz
	FreeAlignMatrix(AlignMatrix);
	
	//Obtén los alineamientos globales óptimos de la matriz
	Align *aligns=ExplAlignsFromTraceback(traceback);//El arreglo de alineamientos
	
	//Libera el espacio ocupado por los caminos
	FreeTraceback(traceback);
	
	//Imprime los alineamientos
	PrintAlignments(aligns);
	
	FreeAligns(aligns);
}//___________________________________________________________



Align *ExplAlignsFromTraceback(const Traceback *traceback)
/*
 * Genera un arreglo de alineaminetos explícitos a partir de los caminos contenidos en traceback
 */
{
#define	STR1	(traceback->Str1)
#define	STR2	(traceback->Str2)
#define PATHSCORE(align)	((traceback->Paths)[align].pScore)
#define	POINTER(align, align_pos)	((traceback->Paths)[align].Path)[align_pos][2]
#define	NO_POINTERS	-1
#define ALIGN1(align)	(aligns[align].Align1)
#define ALIGN2(align)	(aligns[align].Align2)
#define ALIGNSCORE(align)	(aligns[align].Score)
#define DIAG	'\\'
#define VERT	'|'
#define HORI	'-'
	//Inicializa variables importantes
	int align, n_aligns=(traceback->iPaths), endS1, endS2, pos,i , j;
	//Inicializa el espacio para el arreglo de alineamientos
	Align *aligns=(Align *) malloc((n_aligns+1) * sizeof(Align));
	assert(aligns != NULL);
	//Obtén los alineamientos
	for(align=0; align < n_aligns; align++)//Itera sobre cada camino
	{
		//Obtén ambos alineamientos en reversa
		endS1=strlen(STR1), endS2=strlen(STR2);//Obtén las dimensiones de los textos
		//Inicializa el alineamineto número align
		ALIGNSCORE(align)=PATHSCORE(align);//Coloca el score correspondiente
		ALIGN1(align) = dupStr(STR1);//Copia la primera cadena
		assert(ALIGN1(align) != NULL);
		ALIGN2(align) = dupStr(STR2);//Copia la segunda cadena
		assert(ALIGN2(align) != NULL);
		//Arma los alineamientos
		for(pos=0; POINTER(align, pos) != NO_POINTERS; pos++)
		{
			switch(POINTER(align, pos))//Dependiendo del tipo de puntero
			{
				case DIAG ://Hubo una coincidencia o reemplazo
				{
					break;
				}
				case HORI ://Hubo la eliminación de una letra de STR1 en STR2
				{
					insert('_', endS2-pos, ALIGN2(align));
					endS2++;
					break;
				}
				case VERT ://Hubo una inserción de una letra en STR1 para transformarla en STR2
				{
					insert('_', endS1-pos, ALIGN1(align));
					endS1++;
					break;
				}
			}
		}
	}
	//Marca el final del arreglo con un alineamiento nulo.
	ALIGN1(n_aligns)=NULL, ALIGN2(n_aligns)=NULL;
	return aligns;
#undef STR1
#undef STR2
#undef PATHSCORE
#undef POINTER
#undef NO_POINTERS
#undef ALIGN1
#undef ALIGN2
#undef ALIGNSCORE
#undef DIAG
#undef VERT
#undef HORI
}//___________________________________________________________



void PrintExplAlign(const Align *align)
{
#define ALIGN1 (align->Align1)
#define ALIGN2 (align->Align2)
#define SCORE (align->Score)
	printf("Score:\t%f\n", SCORE);
	int len=strlen(ALIGN1),i;
	printf("Str1:\t");
	for(i=0; i<len; i++)
		printf("%c ", ALIGN1[i]);
	printf("\nStr2:\t");
	for(i=0; i<len; i++)
		printf("%c ", ALIGN2[i]);
	printf("\n");
#undef ALIGN1
#undef ALIGN2
}//___________________________________________________________



char *EditTrFromExplAlign(const Align *align)
{
#define ALIGN1 (align->Align1)
#define ALIGN2 (align->Align2)
	int i;
	char *EditTr=(char *) calloc(1, sizeof(char));
	EditTr=strcpy(EditTr, "");
	for(i=strlen(ALIGN1)-1; i>=0; i--)
	{
		if(ALIGN1[i]=='_')
			insert('I', 0, EditTr);
 		else if(ALIGN2[i]=='_')
 			insert('D', 0, EditTr);
 		else if(ALIGN1[i]==ALIGN2[i])
 			insert('M', 0, EditTr);
 		else
 			insert('R', 0, EditTr);
	}
	return EditTr;
#undef ALIGN1
#undef ALIGN2
}//___________________________________________________________



void PrintEditTr(const char *EditTr)
{
	int len=strlen(EditTr),i;
	printf("EditTr:\t");
	for(i=0; i<len; i++)
		printf("%c ", EditTr[i]);
	printf("\n");
}//___________________________________________________________



void PrintAlignments(const Align *alignments)
/*
 * 
 */
{
	int i;
	for(i=0; (alignments[i].Align1)!=NULL; i++)
	{
		printf("\nAlineamiento número %d:\n", i+1);
		PrintExplAlign(&alignments[i]);
		PrintEditTr(EditTrFromExplAlign(&alignments[i]));
	}
}//___________________________________________________________



void FreeAligns(Align *alignments)
/*
 * Libera el espacio ocupado por los alineamientos
 */
{
	int i;
	for(i=0; (alignments[i].Align1)!=NULL; i++)
	{
		free(alignments[i].Align1);
		free(alignments[i].Align2);
	}
	free(alignments);
}//___________________________________________________________

# endif