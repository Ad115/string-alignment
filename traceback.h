# ifndef STRING_ALIGN_TRACEBACK
# define STRING_ALIGN_TRACEBACK
/*
==============================================
Librería: Recuperación de caminos (Traceback).
==============================================

Funciones especializadas para la estructura Traceback y su subsidiaria Path.
Contiene funciones para crear y llenar tracebacks a partir de la matriz de alineamiento, así como para destruirlos cuando ya no se usen.
Contiene funciones para crear, copiar y extender estructuras Path.

Esta librería utiliza las siguientes estructuras como base::

		typedef struct Entry_struct
		 // Estructura correspondiente a cada entrada de la matriz
		{
			float Value;
			int N_Pointers;
			char Pointers[3];
			
		} Entry;
		//___________________________________________________________
		
		
		typedef struct A_Matrix_struct
		// Estructura de la matriz de alineamiento, se compone de ambas secuencias de texto y las entradas de la matriz.
		{
			char *Str1;
			char *Str2;
			char *Type;
			char *AlignType;
			float *Scores;
			Entry **M;
			
		} A_Matrix;
		//___________________________________________________________
		
		
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

:Traceback *AllocTracebackForMatrix(const A_Matrix *AlignMatrix):
	Genera espacio para una estructura Traceback con caminos no inicializados con entradas no inicializadas.
	El número de caminos generados es: DEFAULT_SIZE.
	El número de entradas generadas por camino es DEFAULT_SIZE.

:Traceback *TracebackFromMatrixEntry(const A_Matrix *AlignMatrix, const int i, const int j):
	Genera una estructura tracebacks que contiene un listado de los alineamientos óptimos de los textos asociados a la matriz AlignMatrix.
	La estructura en la variable Paths[i] contiene el iésimo camino. 
	La variable nPaths contiene el número de alineamientos completos o a medio completar.
	Cada camino es un listado de la forma Paths[path]->Path={{n,m,POINTER}, {n1,m1,POINTER}, {...}, ... {0,0,-1}}, cuya longitud es Paths[path]->pSize.
	Para obtenerlos se recorre la matriz siguiendo los punteros en un algoritmo basado en Depth First Search. 
	Cada vez que se encuentra una bifurcación se copia el alineamiento en construcción tantas veces como caminos nuevos se hayan encontrado.
	No se usa una cola, en cambio, debido a que cada camino debe terminar en la entrada (0,0) de la matriz,
	si el último paso en el camino no es {0, 0, NO_POINTERS}, entonces el camino está inconcluso.

:void FreeTraceback(Traceback *traceback):
	Libera el espacio ocupado por la estructura traceback previamente alojada por AllocTracebackForMatrix()

:void AllocMorePaths(Traceback *traceback, const int extra_entries, const int path_size):
	Haz espacio para extra_entries nuevos caminos en la estructura alignments.
	Cada camino tiene path_size entradas.

:void PathCopy(const Path *p1, Path *p2, const int skip):
	Copia el camino en p1 a p2, excepto las últimas skip entradas.

:void ExtendPath(Path *path, const int extra_entries):
	Añade extra_entries entradas al camino Path, inicializa el espacio correspondiente a cada entrada.



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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Funciones para rastreo de caminos en matrices de alineamiento.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/



#define DEFAULT_SIZE	(len1+len2)
Traceback *AllocTracebackForMatrix(const A_Matrix *AlignMatrix)
/*
 * Genera espacio para una estructura Traceback con caminos no inicializados con entradas no inicializadas.
 * El número de caminos generados es: DEFAULT_SIZE
 * El número de entradas generadas por camino es DEFAULT_SIZE
 */
#define	STR1	(traceback->Str1)
#define	STR2	(traceback->Str2)
#define	I_PATHS	(traceback->iPaths)
#define	U_PATHS	(traceback->uPaths)
#define PATHARR	(traceback->Paths)
//DEFAULT_SIZE tiene que ser la misma que en TracebacksFromMatrix!!!!
{
	int len1=strlen(AlignMatrix->Str1), len2=strlen(AlignMatrix->Str2);//Recupera el tamaño de las cadenas y así, el de la matriz
	
	//Crea espacio para la estructura y el arreglo de caminos
	Traceback *traceback=(Traceback *) malloc(sizeof(*traceback));//Genera espacio para la estructura
	assert(traceback != NULL);
	STR1 = dupStr(AlignMatrix->Str1);//Copia la cadena de texto inicial
	assert(STR1 != NULL);//Verifica que se haya copiado
	STR2 = dupStr(AlignMatrix->Str2);//Copia la cadena de texto objetivo
	assert(STR2 != NULL);//Verifica que se haya copiado
	PATHARR = (Path *) malloc(DEFAULT_SIZE * sizeof(Path));//Inicializa el arreglo de caminos. Tiene DEFAULT_SIZE caminos iniciales.
	assert(PATHARR != NULL);
	U_PATHS = DEFAULT_SIZE;
	I_PATHS = 0;//No hay caminos inicializados
	
	
	//Crea espacio para los caminos
	int i, ii;
	for(i=0; i<DEFAULT_SIZE; i++)
	{
		//Espacio para cada camino
		PATHARR[i].Path = (int **) malloc(DEFAULT_SIZE * sizeof(int *));
		assert(PATHARR[i].Path != NULL);
		for(ii=0; ii < DEFAULT_SIZE; ii++)
		{
			PATHARR[i].Path[ii] = (int *) calloc(3, sizeof(int));
			assert(PATHARR[i].Path[ii] != NULL);
		}
		PATHARR[i].uSize=DEFAULT_SIZE;//Tiene DEFAULT_SIZE entradas sin inicializar
		PATHARR[i].iSize=0;//No tiene entradas inicializadas
	}
	
	return traceback;
}
#undef	STR1
#undef	STR2
#undef	I_PATHS
#undef	U_PATHS
#undef	PATHARR
//___________________________________________________________



Traceback *TracebackFromMatrixEntry(const A_Matrix *AlignMatrix, const int ii, const int jj)
/*
 * Genera una estructura tracebacks que contiene un listado de los alineamientos óptimos de los textos asociados a la matriz AlignMatrix.
 * La estructura en la variable Paths[i] contiene el iésimo camino. 
 * La variable nPaths contiene el número de alineamientos completos o a medio completar.
 * Cada camino es un listado de la forma Paths[path]->Path={{n,m,POINTER}, {n1,m1,POINTER}, {...}, ... {0,0,-1}}, cuya longitud es Paths[path]->pSize.
 * Para obtenerlos se recorre la matriz siguiendo los punteros en un algoritmo basado en Depth First Search. 
 * Cada vez que se encuentra una bifurcación se copia el alineamiento en construcción tantas veces como caminos nuevos se hayan encontrado.
 * No se usa una cola, en cambio, debido a que cada camino debe terminar en la entrada (0,0) de la matriz,
 * si el último paso en el camino no es {0, 0, NO_POINTERS}, entonces el camino está inconcluso.
 */

#define	STR1	(traceback->Str1)
#define	STR2	(traceback->Str2)
#define	I_PATHS	(traceback->iPaths)
#define	U_PATHS	(traceback->uPaths)
#define PATHARR	(traceback->Paths)
#define	PATH_I_SIZE(path)	(PATHARR[path].iSize)
#define	PATH_U_SIZE(path)	(PATHARR[path].uSize)
#define PATHSCORE(path)	(PATHARR[path].pScore)
#define LAST_PATH_ENTRY(path)	((PATHARR[path].Path)[PATH_I_SIZE(path)-1])
#define NEXT_PATH_ENTRY(path)	((PATHARR[path].Path)[PATH_I_SIZE(path)])
#define FILL_NEXT_PATH_ENTRY(path, i, j, pointer)	\
			do{NEXT_PATH_ENTRY(path)[0]=i;\
			NEXT_PATH_ENTRY(path)[1]=j;\
			NEXT_PATH_ENTRY(path)[2]=pointer;}while(0)
#define N_POINTERS(i, j)	(AlignMatrix->M)[i][j].N_Pointers
#define POINTER(p, i, j)	(AlignMatrix->M)[i][j].Pointers[p]
#define VALUE(i, j) (AlignMatrix->M)[i][j].Value
#define LAST_PATH_POINTER(path)	LAST_PATH_ENTRY(path)[2]
#define NO_POINTERS -1
#define DIAG 92
#define VERT '|'
#define HORI '-'
{
	//Inicializa la estructura que se llenará con los caminos
	Traceback *traceback=AllocTracebackForMatrix(AlignMatrix);
	assert(traceback != NULL);

	int i, j, len1=strlen(STR1), len2=strlen(STR2);//Recupera el tamaño de las cadenas y así, el de la matriz
	if(ii==-1 && jj==-1)
		i=len2, j=len1;//La posición inicial es desde la última entrada de la matriz(Alineamiento global)
	else
		i=ii, j=jj;

	//Comienza a buscar caminos...
	int path, pointer, temp;//Inicializa contadores
	//Inicializa los primeros caminos.......................................................................
	for(pointer=0; pointer<N_POINTERS(i,j); pointer++)//Haz tantos caminos como bifurcaciones haya desde la última entrada de la matriz
	{
		PATHSCORE(I_PATHS)=VALUE(i,j);//El score del camino
		FILL_NEXT_PATH_ENTRY(I_PATHS, i, j, POINTER(pointer,i,j));//Llena correctamente la entrada del camino
		PATH_I_SIZE(I_PATHS)++;//El nuevo camino tiene 1 entrada
		PATH_U_SIZE(I_PATHS)--;//El nuevo camino tiene una entrada menos sin inicializar
		I_PATHS++;//Incrementa los caminos inicializados
		U_PATHS--;//Decrementa los caminos no inicializados
	}
	//Continúa a partir de ellos............................................................................
	for(path=0; path<I_PATHS; path++)//Busca caminos inconclusos 
	{
		if( LAST_PATH_POINTER(path) != NO_POINTERS )//Checa si el camino está inconcluso (si todavía hay punteros desde esa entrada)
		{
			//El camino path está inconcluso, complétalo
			i=LAST_PATH_ENTRY(path)[0], j=LAST_PATH_ENTRY(path)[1];//Obtiene las i y j donde el camino se quedó
			//Encuentra los siguientes valores de i y j
			switch(LAST_PATH_POINTER(path))//Los siguientes i y j dependen del puntero correspondiente
			{
				case DIAG:
				{
					i--, j--;
					break;
				}
				case VERT:
				{
					i--;
					break;
				}
				case HORI:
				{
					j--;
					break;
				}
				default:
					printf("\nPuntero desconocido en camino %d, posición %d : %c=%d.\n", path, PATH_I_SIZE(path)-1, LAST_PATH_POINTER(path), LAST_PATH_POINTER(path));
			}
			//Coloca la siguiente entrada del camino
			//Verifica que no se necesite hacer espacio para más entradas...
			if(PATH_U_SIZE(path)==0)
			{
				//Se requiere extender el camino
				ExtendPath(&PATHARR[path], DEFAULT_SIZE);
			}
			//Antes de colocar los datos, verifica si es un final de camino
			if(N_POINTERS(i,j)==0)//Si el número de punteros desde esa posición es 0....
			{
				//Llegaste al final del camino!
				//Llena la entrada con la posición i,j y el marcador NO_POINTERS que indica el fin del camino
				FILL_NEXT_PATH_ENTRY(path, i, j, NO_POINTERS);
				PATH_I_SIZE(path)++, PATH_U_SIZE(path)--;//Incrementa el tamaño del camino
			}
			else//Si todavía hay punteros desde aquí...
			{
				//El camino todavía está inconcluso, coloca los datos correspondientes para ser llenado
				FILL_NEXT_PATH_ENTRY(path, i, j, POINTER(0, i, j));//Llena la entrada con la posición y el primer puntero
				PATH_I_SIZE(path)++, PATH_U_SIZE(path)--;//Incrementa el tamaño del camino
				//Si hay más de un puntero se tienen que inicializar más caminos
				for(pointer=1; pointer<N_POINTERS(i,j); pointer++)
				{
					//Verifica que no se necesite hacer espacio 
					if(U_PATHS==0)
					{
						//Si se requiere espacio
						AllocMorePaths(traceback, DEFAULT_SIZE, DEFAULT_SIZE);
					}
					PathCopy(&PATHARR[path], &PATHARR[I_PATHS], 1);//Copia el camino que bifurca hacia el nuevo camino
					FILL_NEXT_PATH_ENTRY(I_PATHS, i, j, POINTER(pointer, i, j));//Llena correctamente la última entrada del camino
					PATH_I_SIZE(I_PATHS)++, PATH_U_SIZE(I_PATHS)--;//Añade la nueva entrada
					I_PATHS++, U_PATHS--;//Incrementa los caminos inicializados
				}
			}
		path=-1;//No estaba completo, reinicializa por si sigue igual
		}//El camino ya está completo, sigue buscando
	}//Ya no hay caminos inconclusos, ya terminaste! :D

	return traceback;//Devuelve los resultados. 
#undef	STR1
#undef	STR2
#undef	I_PATHS
#undef	U_PATHS
#undef	PATHARR
#undef	PATH_I_SIZE
#undef	PATH_U_SIZE
#undef	PATHSCORE
#undef	LAST_PATH_ENTRY
#undef	FILL_LAST_PATH_ENTRY
#undef N_POINTERS
#undef POINTER
#undef SCORE
#undef LAST_PATH_POINTER
#undef NO_POINTERS
#undef DIAG 
#undef VERT 
#undef HORI
}//___________________________________________________________
#undef DEFAULT_SIZE



void FreeTraceback(Traceback *traceback)
/*
 * Libera el espacio ocupado por el camino referenciado por traceback
 */
#define	PATH(p)	(traceback->Paths)[p]
{
	free(traceback->Str1);
	free(traceback->Str2);
	int i, j;
	for(i=0; i < (traceback->iPaths) + (traceback->uPaths); i++)
	{
		for(j=0; j < (PATH(i).iSize)+(PATH(i).uSize); j++)
			free(PATH(i).Path[j]);
		free(PATH(i).Path);
	}
	free(traceback->Paths);
	free(traceback->Str1);
	free(traceback->Str2);
	free(traceback);
#undef PATH
}//___________________________________________________________




void AllocMorePaths(Traceback *traceback, const int extra_entries, const int path_size)
/*
 * Haz espacio para extra_entries nuevos caminos en la estructura alignments.
 * Cada camino tiene path_size entradas.
 */
#define	I_PATHS	(traceback->iPaths)
#define	U_PATHS	(traceback->uPaths)
#define PATHARR	(traceback->Paths)
{
	//Incrementa el número de caminos
	PATHARR=(Path *) realloc(PATHARR, (I_PATHS+U_PATHS+extra_entries) * sizeof(Path));
	int prev_uninit=U_PATHS;//El número anterior de caminos sin inicializar
	U_PATHS+=extra_entries;//Ahora hay más caminos sin inicializar
	
	//Haz espacio para cada camino nuevo, los caminos nuevos empiezan a partir del N_PATHS+prev_uninit
	int i, ii;
	for(i=I_PATHS+prev_uninit; i < I_PATHS+U_PATHS; i++)
	{
		//Espacio para cada camino
		PATHARR[i].Path = (int **) malloc(path_size * sizeof(int *));//Haz espacio para el camino con path_size entradas
		assert(PATHARR[i].Path != NULL);
		for(ii=0; ii < path_size; ii++)
		{
			PATHARR[i].Path[ii]=(int *) calloc(3, sizeof(int));//Cada entrada consta de tres enteros
			assert(PATHARR[i].Path[ii] != NULL);
		}
		PATHARR[i].uSize=path_size;//Tiene path_size entradas sin inicializar
		PATHARR[i].iSize=0;//No tiene entradas inicializadas
	}
}
#undef N_PATHS
#undef U_PATHS
#undef PATHARR
//___________________________________________________________



void PathCopy(const Path *p1, Path *p2, const int skip)
/*
 * Copia el camino referenciado por p1 a p2, saltándose las últimas skip entradas.
 */
{
	int i, j;
	int p2size = (p2->iSize)+(p2->uSize), p1size=(p1->iSize)+(p1->uSize);
	//Verifica si se requiere extender el camino
	if( (p2size < p1size ) )
	{
		ExtendPath(p2, p1size - p2size);
		p2size = (p2->iSize)+(p2->uSize);//Recalcula el tamaño del camino
	}
	
	//Prepara para copiar...
	for(i=0; i < (p1->iSize) - skip; i++)//Copia entrada por entrada
	{
		for(j=0; j<3; j++)
			(p2->Path)[i][j]=(p1->Path)[i][j];//Coloca lo correspondiente en la entrada
	}
	(p2->iSize) = (p1->iSize) - skip;//Copia el tamaño
	(p2->uSize) = p2size-(p2->iSize);//Recalcula las entradas no inicializadas
	(p2->pScore) = (p1->pScore);//Copia el puntaje
}//___________________________________________________________



void ExtendPath(Path *path, const int extra_entries)
/*
 * Añade extra_entries entradas al camino Path, inicializa el espacio correspondiente a cada entrada
 */
{
	int old_size=(path->uSize)+(path->iSize);
	(path->Path)=(int **) realloc((path->Path), old_size+extra_entries);//Genera espacio
	int ii;
	for(ii=old_size; ii < old_size+extra_entries; ii++)
		(path->Path)[ii]=(int *) calloc(3, sizeof(int));//Cada entrada consta de tres enteros
	(path->uSize)+=extra_entries;//Tiene extra_entries entradas nuevas sin inicializar
}//___________________________________________________________

# endif