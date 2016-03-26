/*
*****************************************************************************
                        Librería: ALINEAMIENTOS v0.0                 	*
*****************************************************************************/
/*
***Librería que contiene funciones y estructuras para generar alineamientos a partir de matrices de alineamiento. 
	Las funciones de esta versión se utilizan en global_alignment3.1.c

***La librería contiene:
*/

	//1. Estructuras fundamentales de alineamientos:
		typedef struct Align_struct
		//Estructura que contiene un alineamiento explícito de dos textos
		{
			float Score;
			char *Align1;
			char *Align2;
			
		} Align;
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
		
		

	//2. Funciones para encontrar los alineamientos:
		void GlobalAlignment(const char *str1, const char *str2, const char *type, const float *scores);//Obtiene e imprime todos los alineamientos globales óptimos de str1 y str2 con las características especificadas
		void PathCopy(const Path *p1, Path *p2, const int skip);//Copia el camino en p1 a p2, excepto las últimas skip entradas
		Traceback *AllocTracebackForMatrix(const A_Matrix *AlignMatrix);
		Traceback *TracebackFromMatrixEntry(const A_Matrix *AlignMatrix, const int i, const int j);
		void AllocMorePaths(Traceback *traceback, const int extra_entries, const int path_size);
		void ExtendPath(Path *path, const int extra_entries);
		Align *ExplAlignsFromTraceback(const Traceback *traceback);
		void PrintExplAlign(const Align *align); // Muestra el alineamiento explícito Align
		char *EditTrFromExplAlign(const Align *align); // Genera el código de edición asociado al alineamiento Align
		void PrintEditTr(const char *EditTr); // Muestra el código de edición en el mismo formato en que se muestra el alineamiento en PrintExplAlign().
		void PrintAlignments(const Align *align); // Imprime los alineamientos en el arreglo alignments
		void FreeTraceback(Traceback *traceback); // Libera el espacio ocupado por la estructura traceback previamente alojada por AllocTracebackForMatrix()
		void FreeAligns(Align *alignments); // Libera el espacio ocupado por la estructura alignments previamente alojada por ExplAlignsFromTraceback()

//El código utiliza estructuras y funciones declaradas en general.h y alignment_matrix.h y estos archivos deben ser debidamente incluidos en el programa principal.

/*
Andrés García García @ 28/Feb/'16 (Inicio 19 Oct 2015)
*/

#include<stdio.h>
#include<assert.h>//Para verificar errores con la función assert()
#include<stdlib.h>//Para usar malloc(), calloc(), realloc(), free()
#include<string.h>//Para usar strlen() y strcpy()


int debug; // Variable global para debugear :P


//**********************---------2.--------*************************************************************
//**********************FUNCIONES DE ALINEAMIENTOS *****************************************************

void GlobalAlignment(const char *str1, const char *str2, const char *type, const float *scores)
/*
 * A partir de las cadenas, costos y el tipo de alineamiento, obtiene los alineamientos globales óptimos
 */
{
	//char *align_type=strdup("global");
	char align_type[]="global";
	printf("\nAlineamiento Global.\nStr1:\t%s\nStr2:\t%s\n", str1, str2);//Imprime las cadenas
	
	//Obtiene la matriz de alineamiento
	A_Matrix *AlignMatrix = AllocAlignMatrix(str1, str2, type, align_type, scores);//Genera espacio para la matriz
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nGenerada la matriz...Llenando la matriz...\n");
	printf("AlignType: (%s)\n", (AlignMatrix->AlignType));
	/* ...Debug */
	
	FillAlignMatrix(AlignMatrix);//Llena la matriz
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nMatriz llena...Imprimiendo...\n");
	printf("AlignType: (%s)\n", (AlignMatrix->AlignType));
	/* ...Debug */
	
	
	PrintAlignMatrix(AlignMatrix);
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nFinalizado crear matriz.\n");
	printf("\nIniciando traceback...\n");
	/* ...Debug */
	
	//Obtén los caminos
	Traceback *traceback=TracebackFromMatrixEntry(AlignMatrix,-1, -1);
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nFinalizado realizar traceback.\n");
	printf("\nLiberando matriz...Omitido\n");
	/* ...Debug */
	
	
	//Libera el espacio ocupado por la matriz
	FreeAlignMatrix(AlignMatrix);
	
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nLiberada la matriz de alineamiento...\n");
	printf("\nGenerando alineamientos...\n");
	/* ...Debug */
	
	
	//Obtén los alineamientos globales óptimos de la matriz
	Align *aligns=ExplAlignsFromTraceback(traceback);//El arreglo de alineamientos
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nAlineamientos completos...\n");
	printf("\nLiberando tracebacks...\n");
	/* ...Debug */
	
	//Libera el espacio ocupado por los caminos
	FreeTraceback(traceback);
	
	/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nLiberados los tracebacks...\n");
	printf("\nImprimiendo alineamientos...\n");
	/* ...Debug */
	
	//Imprime los alineamientos
	PrintAlignments(aligns);
	
		/* Debug...*/
	printf("\n\tDebug.........................\n\t===============================\nLiberando alineamientos...\n");
	/* ...Debug */
	
	FreeAligns(aligns);
}//___________________________________________________________


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
 * Genera una estructura tracebacks que contiene un listado de los alineamientos óptimos de los textos asociados a la matriz AlignMatrix
 * la estructura en la variable Paths[i] contiene el iésimo camino, la variable nPaths contiene el número de alineamientos.
 * Cada camino es un listado de la forma Paths[path]->Path={{n,m,POINTER}, {n1,m1,POINTER}, {...}, ... {0,0,-1}}, cuya longitud es Paths[path]->pSize.
 * Para obtenerlos se recorre la matriz siguiendo los punteros en un algoritmo basado en Depth First Search. 
 * Cada vez que se encuentra una bifurcación se copia el alineamiento en construcción tantas veces como caminos nuevos se hayan encontrado.
 * No se usa una cola, en cambio, debido a que todo camino debe terminar en la entrada (0,0) de la matriz, los caminos inconclusos se localizan cuando el último paso en el camino no es {0, 0, -1}
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
	
	/* Debug.../
	printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
	printf("Llamando función AllocTracebackForMatrix()...\n");
	/* ...Debug */
	
	//Inicializa la estructura que se llenará con los caminos
	Traceback *traceback=AllocTracebackForMatrix(AlignMatrix);
	assert(traceback != NULL);
	
	/* Debug.../
	printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
	printf("Traceback inicializados...\n");
	/* ...Debug */
	
	int i, j, len1=strlen(STR1), len2=strlen(STR2);//Recupera el tamaño de las cadenas y así, el de la matriz
	if(ii==-1 && jj==-1)
		i=len2, j=len1;//La posición inicial es desde la última entrada de la matriz(Alineamiento global)
	else
		i=ii, j=jj;
	
	/* Debug.../
	printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
	printf("Comenzando búsqueda de caminos desde la entrada (%d, %d), que tiene %d punteros...\n", i, j, N_POINTERS(i,j));
	/* ...Debug */
	
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
		
		/* Debug.../
		printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
		printf("POINTER(pointer,i,j)=%c, i=%d, j=%d, pointer=%d\n", POINTER(pointer, i, j), i, j, pointer);
		/* ...Debug */
		
		
		/* Debug.../
		printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
		printf("Inicializando caminos. Nuevo camino %d, score: %d. Entrada : ( %d, %d, %c)...\n", I_PATHS-1, (int)PATHSCORE(I_PATHS-1), i, j, POINTER(pointer,i,j));
		/* ...Debug */
		
	}
	//Continúa a partir de ellos............................................................................
	for(path=0; path<I_PATHS; path++)//Busca caminos inconclusos 
	{
		if( LAST_PATH_POINTER(path) != NO_POINTERS )//Checa si el camino está inconcluso (si todavía hay punteros desde esa entrada)
		{
			
			/* Debug.../
			printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
			printf("Camino inconcluso detectado...");
			/* ...Debug */
			
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
				
				/* Debug.../
				printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
				printf("Camino lleno. Extendiendo camino %d...\n", path);
				/* ...Debug */
				
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
				
				/* Debug.../
				printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
				printf("Camino completo %d. Entrada %d: ( %d, %d, NO_POINTERS)...\n", path, PATH_I_SIZE(path)-1, i, j);
				/* ...Debug */
				
				/* Debug.../
				printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
				printf("Imprimiendo camino finalizado...\n");
				printf("Camino: %d, Score: %d, Entradas inicializadas: %d, Entradas sin inicializar: %d\n", path, (int)PATHSCORE(path), PATH_I_SIZE(path), PATH_U_SIZE(path));
				for(debug=0; debug<PATH_I_SIZE(path); debug++)
					printf("(%d,%d,%c),", PATHARR[path].Path[debug][0], PATHARR[path].Path[debug][1], PATHARR[path].Path[debug][2]);
				printf("\n");
				/* ...Debug */
				
			}
			else//Si todavía hay punteros desde aquí...
			{
				//El camino todavía está inconcluso, coloca los datos correspondientes para ser llenado
				FILL_NEXT_PATH_ENTRY(path, i, j, POINTER(0, i, j));//Llena la entrada con la posición y el primer puntero
				PATH_I_SIZE(path)++, PATH_U_SIZE(path)--;//Incrementa el tamaño del camino
				
				/* Debug.../
				printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
				printf("Completando camino %d. Entrada añadida %d: ( %d, %d, %c)...\n", path, PATH_I_SIZE(path)-1, i, j, POINTER(0, i, j));
				/* ...Debug */
				
				//Si hay más de un puntero se tienen que inicializar más caminos
				for(pointer=1; pointer<N_POINTERS(i,j); pointer++)
				{
					//Verifica que no se necesite hacer espacio 
					if(U_PATHS==0)
					{
						//Si se requiere espacio
						AllocMorePaths(traceback, DEFAULT_SIZE, DEFAULT_SIZE);
						
						/* Debug.../
						printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
						printf("Se necesita nuveo camino y no hay disponibles. Creando nuevos caminos.....");
						/* ...Debug */
						
					}
					PathCopy(&PATHARR[path], &PATHARR[I_PATHS], 1);//Copia el camino que bifurca hacia el nuevo camino
					
					/* Debug.../
					printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
					printf("Divergencia encontrada. Copiando camino %d en el camino %d...\n", path, I_PATHS);
					/* ...Debug */
					
					FILL_NEXT_PATH_ENTRY(I_PATHS, i, j, POINTER(pointer, i, j));//Llena correctamente la última entrada del camino
					PATH_I_SIZE(I_PATHS)++, PATH_U_SIZE(I_PATHS)--;//Añade la nueva entrada
					I_PATHS++, U_PATHS--;//Incrementa los caminos inicializados
					
					/* Debug.../
					printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
					printf("Completando camino nuevo %d. Entrada añadida %d: ( %d, %d, %c)...\n", I_PATHS-1, PATH_I_SIZE(I_PATHS-1)-1, i, j, POINTER(pointer, i, j));
					/* ...Debug */
					
				}
			}
		path=-1;//No estaba completo, reinicializa por si sigue igual
		}//El camino ya está completo, sigue buscando
	}//Ya no hay caminos inconclusos, ya terminaste! :D
	
	/* Debug...*/
	printf("\n\tDebugeando función TracebackFromMatrixEntry()........\n\t===============================\n");
	printf("Imprimiendo estructura final....\n");
	printf("Str1: %s\nStr2: %s\niPaths: %d, uPaths: %d\nPaths:", STR1, STR2, I_PATHS, U_PATHS);
	for(path=0; path<I_PATHS; path++)
	{
		printf("\nCamino: %d, Score: %d, Entradas inicializadas: %d, Entradas sin inicializar: %d\n", path, (int)PATHSCORE(path), PATH_I_SIZE(path), PATH_U_SIZE(path));
		for(debug=0; debug<PATH_I_SIZE(path); debug++)
			printf("(%d,%d,%c),", PATHARR[path].Path[debug][0], PATHARR[path].Path[debug][1], PATHARR[path].Path[debug][2]);
		printf("\n");
	}
	/* ...Debug */
	
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