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
		struct align_expl; // Contiene textos alineados explícitamente y el puntaje por alinearlos
		struct tracebacks; // Formato general que contiene los posibles alineamientos globales óptimos de dos textos, en términos de la matriz de alineamiento
		struct path; //Estructura que contiene un solo camino y su score

	//2. Funciones para encontrar los alineamientos:
		void GlobalAlignment(char *str1, char *str2, char *type, float *scores);//Obtiene e imprime todos los alineamientos globales óptimos de str1 y str2 con las características especificadas
		void PathCopy(struct path *p1, struct path *p2);
		struct tracebacks *AllocTracebacksForMatrix(struct a_matrix *AlignMatrix);
		struct tracebacks *TracebacksFromMatrix(struct a_matrix *AlignMatrix);
		void AllocMorePaths(struct tracebacks *alignments, int extra_entries, int path_size);
		void ExtendPath(struct path *Path, int extra_entries);
		struct align_expl *ExplAlignsFromTracebacks(struct tracebacks *alignments);
		void PrintExplAlign(struct align_expl *Align); // Muestra el alineamiento explícito Align
		char *EditTrFromExplAlign(struct align_expl *Align); // Genera el código de edición asociado al alineamiento Align
		void PrintEditTr(char *EditTr); // Muestra el código de edición en el mismo formato en que se muestra el alineamiento en PrintExplAlign().
		void PrintAlignments(struct align_expl *alignments); // Imprime los alineamientos en el arreglo alignments

//El código utiliza estructuras y funciones declaradas en general.h y alignment_matrix.h y estos archivos deben ser debidamente incluidos en el programa principal.

/*
Andrés García García @ 28/Feb/'16 (Inicio 19 Oct 2015)
*/

#include<stdio.h>
#include<stdlib.h>//Para usar malloc(), calloc(), realloc(), free()
#include<string.h>//Para usar strlen() y strcpy()






//**********************---------0.--------************************************************************
//**********************ESTRUCTURAS FUNDAMENTALES******************************************************

struct align_expl
//Estructura que contiene un alineamiento explícito de dos textos
{
	float Score;
	char *Align1;
	char *Align2;
};//___________________________________________________________


struct tracebacks
{
	char *Str1;
	char *Str2;
	int nPaths;//Número de caminos inicializados
	int uPaths;//Número de caminos no inicializados
	struct path *Paths;//Arreglo de caminos
};//___________________________________________________________


struct path
{
	int pSize;//Número de entradas inicializadas
	int uSize;//Número de entradas no inicializadas
	float pScore;//Score del camino
	int **Path;

};//___________________________________________________________





//**********************---------3.--------*************************************************************
//**********************FUNCIONES DE ALINEAMIENTOS *****************************************************

void GlobalAlignment(char *str1, char *str2, char *type, float *scores)
/*
 * A partir de las cadenas, costos y el tipo de alineamiento, obtiene los alineamientos globales óptimos
 */
{
	printf("\nAlineamiento Global.\nStr1:\t%s\nStr2:\t%s\n", str1, str2);//Imprime las cadenas
	//Obtiene la matriz de alineamiento
	struct a_matrix *AlignMatrix = AllocAlignMatrix(str1, str2, type, scores);//Genera espacio para la matriz
	FillAlignMatrix(AlignMatrix);//Llena la matriz
	//Obtén los caminos
	//struct tracebacks *tracebacks=TracebacksFromMatrix(AlignMatrix);
	//Libera el espacio ocupado por la matriz
	FreeAlignMatrix(AlignMatrix);
	//Obtén los alineamientos globales óptimos de la matriz
	//struct align_expl *aligns=ExplAlignsFromTracebacks(tracebacks);//El arreglo de alineamientos
	//free(tracebacks);
	//Imprime los alineamientos
	//PrintAlignments(aligns);
	//free(aligns);
}//___________________________________________________________


void PathCopy(struct path *p1, struct path *p2)
/*
 * Copia el camino referenciado por p1 a p2 
 */
{
	int i, j;
	int p2size = (p2->pSize)+(p2->uSize), p1size=(p1->pSize)+(p1->uSize);
	//Verifica si se requiere extender el camino
	if( (p2size < p1size ) )
	{
		ExtendPath(p2, p1size - p2size);
	}
	(p2->pSize)=(p1->pSize);//Copia el tamaño
	(p2->uSize)=(p1->uSize);
	(p2->pScore)=(p1->pScore);//Copia el puntaje
	for(i=0; i < (p1->pSize); i++)//Copia entrada por entrada
	{
		for(j=0; j<3; j++)
			(p2->Path)[i][j]=(p1->Path)[i][j];//Coloca lo correspondiente en la entrada
	}
}//___________________________________________________________



struct tracebacks *AllocTracebacksForMatrix(struct a_matrix *AlignMatrix)
/*
 * Genera espacio para una estructura tracebacks y caminos no inicializados con entradas no inicializadas.
 * El número de caminos generados es: DEFAULT_SIZE
 * El número de entradas generadas por camino es DEFAULT_SIZE
 */
#define	STR1	(alignments->Str1)
#define	STR2	(alignments->Str2)
#define	N_PATHS	(alignments->nPaths)
#define	U_PATHS	(alignments->uPaths)
#define PATHARR	(alignments->Paths)
#define DEFAULT_SIZE	(len1+len2)
//DEFAULT_SIZE tiene que ser la misma que en TracebacksFromMatrix!!!!
{
	int len1=strlen(AlignMatrix->Str1), len2=strlen(AlignMatrix->Str2);//Recupera el tamaño de las cadenas y así, el de la matriz
	
	//Crea espacio para la estructura y el arreglo de caminos
	struct tracebacks *alignments=(struct tracebacks *) malloc(sizeof(alignments));//Genera espacio para la estructura
	STR1=(char *) calloc(len1+1, sizeof(char));//Genera espacio para la cadena de texto inicial
	strcpy(STR1, AlignMatrix->Str1);//Obtiene la cadena de texto inicial
	STR2=(char *) calloc(len2+1, sizeof(char));//Genera espacio para la cadena de texto objetivo
	STR2=strcpy(STR2, AlignMatrix->Str2);//Obtiene la cadena de texto objetivo
	PATHARR = (struct path *) malloc(DEFAULT_SIZE * sizeof(struct path));//Inicializa el arreglo de caminos. Tiene DEFAULT_SIZE caminos iniciales.
	U_PATHS = DEFAULT_SIZE;
	N_PATHS = 0;//No hay caminos inicializados
	
	
	//Crea espacio para los caminos
	int i;
	for(i=0; i<DEFAULT_SIZE; i++)
	{
		//Espacio para cada camino
		PATHARR[i].Path = (int **) malloc(DEFAULT_SIZE * sizeof(int *));
		int ii;
		for(ii=0; ii < DEFAULT_SIZE; ii++)
			PATHARR[i].Path[ii]=(int *) calloc(3, sizeof(int));
		PATHARR[i].uSize=DEFAULT_SIZE;//Tiene DEFAULT_SIZE entradas sin inicializar
		PATHARR[i].pSize=0;//No tiene entradas inicializadas
	}
	return alignments;
}
#undef	STR1
#undef	STR2
#undef	N_PATHS
#undef	U_PATHS
#undef	PATHARR
//___________________________________________________________



struct tracebacks *TracebacksFromMatrix(struct a_matrix *AlignMatrix)
/*
 * Genera una estructura tracebacks que contiene un listado de los alineamientos óptimos de los textos asociados a la matriz AlignMatrix
 * la estructura en la variable Paths[i] contiene el iésimo camino, la variable nPaths contiene el número de alineamientos.
 * Cada camino es un listado de la forma Paths[path]->Path={{n,m,POINTER}, {n1,m1,POINTER}, {...}, ... {0,0,-1}}, cuya longitud es Paths[path]->pSize.
 * Para obtenerlos se recorre la matriz siguiendo los punteros en un algoritmo basado en Depth First Search. 
 * Cada vez que se encuentra una bifurcación se copia el alineamiento en construcción tantas veces como caminos nuevos se hayan encontrado.
 * No se usa una cola, en cambio, debido a que todo camino debe terminar n la entrada (0,0) de la matriz, los caminos inconclusos se localizan cuando el último paso en el camino no es {0, 0, -1}
 */

#define	STR1	(alignments->Str1)
#define	STR2	(alignments->Str2)
#define	N_PATHS	(alignments->nPaths)
#define	U_PATHS	(alignments->uPaths)
#define PATHARR	(alignments->Paths)
#define PATH(path)	(PATHARR[path].Path)
#define	PATHSIZE(path)	(PATHARR[path].pSize)
#define	PATH_U_SIZE(path)	(PATHARR[path].uSize)
#define PATHSCORE(path)	(PATHARR[path].pScore)
#define LAST_PATH_ENTRY(path)	PATH(path)[PATHSIZE(path)-1]
#define FILL_LAST_PATH_ENTRY(path, i, j, pointer)	\
			do{LAST_PATH_ENTRY(path)[0]=i;\
			LAST_PATH_ENTRY(path)[1]=j;\
			LAST_PATH_ENTRY(path)[2]=pointer;}while(0)
#define N_POINTERS(i, j)	(AlignMatrix->M)[i][j][1]
#define POINTER(p, i, j)	(int)(AlignMatrix->M)[i][j][1+p]
#define SCORE(i, j) (AlignMatrix->M)[i][j][0]
#define LAST_PATH_POINTER(path)	LAST_PATH_ENTRY(path)[2]
#define NO_POINTERS -1
#define DIAG 0
#define VERT 1
#define HORI 2
{
	//Inicializa la estructura que se llenará con los caminos
	struct tracebacks *alignments=AllocTracebacksForMatrix(AlignMatrix);
	
	//Comienza a buscar caminos...
	int len1=strlen(STR1), len2=strlen(STR2);//Recupera el tamaño de las cadenas y así, el de la matriz
	int i, j, path, pointer, temp;//Inicializa contadores
	//Inicializa los primeros caminos.......................................................................
	i=len2, j=len1;//La posición inicial es desde la última entrada de la matriz(Alineamiento global)
	for(pointer=1; pointer<=N_POINTERS(i,j); pointer++)//Haz tantos caminos como bifurcaciones haya desde la última entrada de la matriz
	{
		//Verifica que no se necesite hacer espacio para más caminos...
		if(U_PATHS==0)
		{
			//Si se requieren más caminos, has espacio para ellos
			AllocMorePaths(alignments, DEFAULT_SIZE, DEFAULT_SIZE);
		}
		PATHSIZE(N_PATHS)++;//El nuevo camino tiene 1 entrada
		PATH_U_SIZE(N_PATHS)--;//El nuevo camino tiene una entrada menos sin inicializar
		PATHSCORE(N_PATHS)=SCORE(i,j);//El score del camino
		FILL_LAST_PATH_ENTRY(N_PATHS, i, j, POINTER(pointer, i, j));//Llena correctamente la entrada del camino
		N_PATHS++;//Incrementa los caminos inicializados
		U_PATHS--;//Decrementa los caminos no inicializados
	}
	//Continúa a partir de ellos............................................................................
	for(path=0; path<N_PATHS; path++)//Busca caminos inconclusos 
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
			}
			//Coloca la siguiente entrada del camino
			//Verifica que no se necesite hacer espacio para más entradas...
			if(U_PATHS==0)
			{
				//Se requiere extender el camino
				ExtendPath(&PATHARR[path], DEFAULT_SIZE);
			}
			PATHSIZE(path)++, PATH_U_SIZE(path)--;//Incrementa el tamaño del camino
			//Antes de colocar los datos en ella, verifica si es un final de camino
			if(N_POINTERS(i,j)==0)//Si el número de punteros desde esa posición es 0....
			{
				//Llegaste al final del camino!
				//Llena la entrada con la posición i,j y el marcador NO_POINTERS que indica el fin del camino
				FILL_LAST_PATH_ENTRY(path, i, j, NO_POINTERS);
			}
			else//Si todavía hay punteros desde aquí...
			{
				//El camino todavía está inconcluso, coloca los datos correspondientes para ser llenado
				FILL_LAST_PATH_ENTRY(path, i, j, POINTER(1, i, j));//Llena la entrada con la posición y el primer puntero
				//Si hay más de un puntero se tienen que inicializar más caminos
				for(pointer=2; pointer<=N_POINTERS(i,j); pointer++)
				{
					//Verifica que no se necesite hacer espacio 
					if(U_PATHS==0)
					{
						//Si se requiere espacio
						AllocMorePaths(alignments, DEFAULT_SIZE, DEFAULT_SIZE);
					}
					PathCopy(&PATHARR[path], &PATHARR[N_PATHS]);//Copia el camino que bifurca hacia el nuevo camino
					PATHSIZE(N_PATHS)++, PATH_U_SIZE(N_PATHS)--;//Añade la nueva entrada
					FILL_LAST_PATH_ENTRY(N_PATHS, i, j, POINTER(pointer, i, j));//Llena correctamente la última entrada del camino
					N_PATHS++, U_PATHS--;//Incrementa los caminos inicializados					
				}
			}
		path=-1;//No estaba completo, reinicializa por si sigue igual
		}//El camino ya está completo, sigue buscando
	}//Ya no hay caminos inconclusos, ya terminaste! :D
	
	return alignments;//Devuelve los resultados. 
#undef	STR1
#undef	STR2
#undef	N_PATHS
#undef	PATHARR
#undef	PATH
#undef	PATHSIZE
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
#undef DEFAULT_SIZE
}//___________________________________________________________



void AllocMorePaths(struct tracebacks *alignments, int extra_entries, int path_size)
/*
 * Haz espacio para extra_entries nuevos caminos en la estructura alignments.
 * Cada camino tiene path_size entradas.
 */

#define	N_PATHS	(alignments->nPaths)
#define	U_PATHS	(alignments->uPaths)
#define PATHARR	(alignments->Paths)
{
	//Incrementa el número de caminos
	PATHARR=(struct path *) realloc(PATHARR, (N_PATHS+U_PATHS+extra_entries)*sizeof(struct path));
	int prev_uninit=U_PATHS;//El número anterior de caminos sin inicializar
	U_PATHS+=extra_entries;//Ahora hay más caminos sin inicializar
	
	//Haz espacio para cada camino nuevo, los caminos nuevos empiezan a partir del N_PATHS+prev_uninit
	int i;
	for(i=N_PATHS+prev_uninit; i < N_PATHS+U_PATHS; i++)
	{
		//Espacio para cada camino
		PATHARR[i].Path = (int **) malloc(path_size * sizeof(int *));//Haz espacio para el camino con path_size entradas
		int ii;
		for(ii=0; ii < path_size; ii++)
			PATHARR[i].Path[ii]=(int *) calloc(3, sizeof(int));//Cada entrada consta de tres enteros
		PATHARR[i].uSize=path_size;//Tiene path_size entradas sin inicializar
		PATHARR[i].pSize=0;//No tiene entradas inicializadas
	}
}
#undef N_PATHS
#undef U_PATHS
#undef PATHARR
//___________________________________________________________



void ExtendPath(struct path *Path, int extra_entries)
/*
 * Añade extra_entries entradas al camino Path, inicializa el espacio correspondiente a cada entrada
 */
{
	int old_size=(Path->uSize)+(Path->pSize);
	(Path->Path)=(int **) realloc((Path->Path), old_size+extra_entries);//Genera espacio
	int ii;
	for(ii=old_size; ii < old_size+extra_entries; ii++)
		(Path->Path)[ii]=(int *) calloc(3, sizeof(int));//Cada entrada consta de tres enteros
	(Path->uSize)+=extra_entries;//Tiene extra_entries entradas nuevas sin inicializar
}//___________________________________________________________



struct align_expl *ExplAlignsFromTracebacks(struct tracebacks *alignments)
/*
 * Genera un arreglo de alineaminetos explícitos desde el arreglo desde los tracebacks contenidos en alignments
 */
{
#define	STR1	(alignments->Str1)
#define	STR2	(alignments->Str2)
#define PATHSCORE(align)	((alignments->Paths)[align].pScore)
#define	POINTER(align, align_pos)	(int)((alignments->Paths)[align].Path)[align_pos][2]
#define	NO_POINTERS	-1
#define ALIGN1(align)	(aligns[align].Align1)
#define ALIGN2(align)	(aligns[align].Align2)
#define ALIGNSCORE(align)	(aligns[align].Score)
#define DIAG	0
#define VERT	1
#define HORI	2
	//Inicializa variables importantes
	int align, n_aligns=(alignments->nPaths), endS1, endS2, pos,i , j;
	//Inicializa el espacio para el arreglo de alineamientos
	struct align_expl *aligns=(struct align_expl *) calloc(n_aligns+1, sizeof(struct align_expl));
	//Obtén los alineamientos
	for(align=0; align < n_aligns; align++)//Itera sobre cada camino
	{
		//Obtén ambos alineamientos en reversa
		endS1=strlen(STR1), endS2=strlen(STR2);//Obtén las dimensiones de los textos
		//Inicializa el alineamineto número align
		ALIGNSCORE(align)=PATHSCORE(align);//Coloca el score correspondiente
		ALIGN1(align)=(char *) calloc((endS1+1), sizeof(char));//Copia la primera cadena
		strcpy(ALIGN1(align), STR1);
		ALIGN2(align)=(char *) calloc((endS2+1), sizeof(char));//Copia la segunda cadena
		strcpy(ALIGN2(align), STR2);
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



void PrintExplAlign(struct align_expl *Align)
{
#define ALIGN1 (Align->Align1)
#define ALIGN2 (Align->Align2)
#define SCORE (Align->Score)
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



char *EditTrFromExplAlign(struct align_expl *Align)
{
#define ALIGN1 (Align->Align1)
#define ALIGN2 (Align->Align2)
	int i;
	char *EditTr=(char *) calloc(1, sizeof(char));
	EditTr=strcpy(EditTr, "\0");
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



void PrintEditTr(char *EditTr)
{
	int len=strlen(EditTr),i;
	printf("EditTr:\t");
	for(i=0; i<len; i++)
		printf("%c ", EditTr[i]);
	printf("\n");
}//___________________________________________________________



void PrintAlignments(struct align_expl *alignments)
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
