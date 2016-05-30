# ifndef STRING_ALIGN_ALIGNMENT_MATRIX
# define STRING_ALIGN_ALIGNMENT_MATRIX
/*
=================================
Librería: Matriz de Alineamiento.
=================================

Funciones especializadas para la estructura A_Matrix (y su subsidiaria Entry), que representa una matriz de alineamiento.
Contiene funciones para crear, llenar, imprimir y destruir matrices de alineamiento.

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


Las siguientes son las funciones declaradas aquí:

:A_Matrix *AllocAlignMatrix(const char *str1, const char *str2, const char *type, const char *aligntype, const float *scores):
	Genera espacio para una matriz de alineamiento general con las cadenas de texto Str1 y Str2.

:float *getScores(const char *scoreStr):
	Obtiene los costos de operación expresados en scoreStr.

:void Score(A_Matrix *Al, const int i, const int j, float (*Compare)(const float *numbers, const int size)):
	LLena la entrada i,j de la matriz AlignMatrix utilizando la función Compare para seleccionar la entrada correcta

:void FillAlignMatrix(A_Matrix *AlignMatrix):
	LLena la matriz AlignMatrix con los valores y punteros correspondientes con ayuda de la función Score()

:void PrintAlignMatrixNoPointers(const A_Matrix *AlignMatrix):
	Imprime sólo las entradas de la matriz de alineamiento AlignMatrix (sin punteros)

:void PrintAlignMatrix(const A_Matrix *AlignMatrix):
	Imprime la matriz de alineamiento AlignMatrix, incluyendo los punteros de traceback, estos son: \(diagonal) |(vertical) y _(horizontal)

:void FreeAlignMatrix(A_Matrix *AlignMatrix):
	Libera el espacio de una estructura a_matrix previamente alojada con AllocAlignMatrix().


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
//Definiciones globales
#define DEFAULT_TYPE "min"
#define DEFAULT_ALIGN "global"


/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Funciones especializadas para matrices de alineamiento.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

A_Matrix *AllocAlignMatrix(const char *str1, const char *str2, const char *type, const char *aligntype, const float *scores)
/*
Aloja espacio para crear una matriz de  alineamiento o similaridad de los textos Str1 y Str2.
Srt1 es representada en las columnas y Str2 en las filas.
La matriz resultante tiene una estructura como la que se muestra:
	matrix[i]={fila i},	donde i=0...m,
	matrix[i][j]=elemento(i,j)
	matrix->Str1=Cadena de texto a transformar.
	matrix->Str2=Cadena de texto objetivo.
	matrix->Scores=[M, R, I, D] (arreglo con los costos de operación usados para llenar la matriz
Devuelve un puntero a una estructura de matriz.
 */
{
	int i,j;
	int rows=strlen(str2)+1, cols=strlen(str1)+1;//Inicializa para crear la matriz

	//Crea la estructura donde se aloja la matriz_____________________________
	A_Matrix *A=(A_Matrix *) malloc(sizeof(*A));
	assert(A != NULL);

	//Define las cadenas a alinear____________________________________________
	(A->Str1) = strdup(str1);//Almacena las cadenas en la estructura
	assert((A->Str1) != NULL);
	(A->Str2) = strdup(str2);
	assert((A->Str2) != NULL);

	//Define el tipo de matriz_________________________________________________
	if(type==NULL)
		(A->Type) = strdup(DEFAULT_TYPE);//Define la matriz por default como matriz de distancia (se llena con el mínimo de los valores posibles)
	else
		(A->Type) = strdup(type);
	assert((A->Type) != NULL);

	//Define el tipo de alineamiento___________________________________________
	if(aligntype==NULL)
		(A->AlignType) = strdup(DEFAULT_ALIGN);//Define el alineamiento por defecto
	else
		(A->AlignType) = strdup(aligntype);
	assert((A->AlignType) != NULL);

	//Define los costos de operación__________________________________________
	(A->Scores) = (float *) malloc(4 * sizeof(*scores));
	if(scores==NULL)
	{
		(A->Scores)[0]=0, (A->Scores)[1]=1, (A->Scores)[2]=1, (A->Scores)[3]=1;//Carga los valores por default
	}
	else
	{
		for(i=0; i<4; i++)
			(A->Scores)[i]=scores[i]; //Carga los valores dados
	}

	//Genera las entradas de la matriz________________________________________
	Entry **M=(Entry **) malloc(rows * sizeof(Entry *));//Genera el espacio para la matriz, en específico para los punteros a las filas
	for(i=0; i<rows; i++)
	{
		M[i]=(Entry *) malloc(cols * sizeof(Entry));//Genera espacio para las filas, M[i] es una fila
		for(j=0; j<cols; j++)
		{
			M[i][j].N_Pointers = 0;//Inicializa el número de punteros como nulo
		}
	}
	(A->M)=M;

	return A;
}//___________________________________________________________



float *getScores(const char *scoreStr)
/*Obtiene un arreglo con los costos de operación con el formato: [ M, R, I, D] (Match, Replacement, Insertion, Deletion).
 * Recibe una string donde se especifican pares operación costo, por ejemplo, si se requiere que M=1, I=-1, R=0,
 * una cadena apropiada sería "M1I-1R0", no importa el orden en que se especifican los costos y no es necesario
 * especificar valores para todas las operaciones.
 * Si un valor no se especifica, los valores por defecto son: M=0, R=1, I=1, D=1
 */
{
	float *scores=(float *) malloc(4 * sizeof(*scores));//Genera el arreglo con los costos por default
	scores[0]=0, scores[1]=1, scores[2]=1, scores[3]=1;//Carga los valores por default
	//Carga los valores en la secuencia otorgada
	float  *temp;
	int i;
	for(i=0; scoreStr[i]!='\0';)//Busca en el texto completo
	{
		switch(scoreStr[i])//Mira cuál costo se está definiendo
		{
			case 'M'://Se está definiendo el costo de emparejar
			{
				temp=getNum(scoreStr, i+1);//Obtén el costo y la siguiente posición
				scores[0]=temp[0];//El valor numérico del costo
				i=(int)temp[1];//Consigue la siguiente posición
				free(temp);
				break;
			}
			case 'R'://Se está definiendo el costo de un reemplazo
			{
				temp=getNum(scoreStr, i+1);//Obtén el costo y la siguiente posición
				scores[1]=temp[0];//El valor numérico del costo
				i=(int)temp[1];//Consigue la siguiente posición
				free(temp);
				break;
			}
			case 'I'://Se está definiendo el costo de insertar una letra
			{
				temp=getNum(scoreStr, i+1);//Obtén el costo y la siguiente posición
				scores[2]=temp[0];//El valor numérico del costo
				i=(int)temp[1];//Consigue la siguiente posición
				free(temp);
				break;
			}
			case 'D'://Se está definiendo el costo de borrar una letra
			{
				temp=getNum(scoreStr, i+1);//Obtén el costo y la siguiente posición
				scores[3]=temp[0];//El valor numérico del costo
				i=(int)temp[1];//Consigue la siguiente posición
				free(temp);
				break;
			}
			default:
			{
				printf("\nADVERTENCIA: Operación no definida '%c' al obtener costos, por defecto se ha omitido\n", scoreStr[i]);
				i++;
			}
		}
	}
	return scores;
}



void PrintAlignMatrixNoPointers(const A_Matrix *AlignMatrix)
//Imprime la matriz de alineamiento AlignMatrix
{
#define STR1	(AlignMatrix->Str1)
#define STR2	(AlignMatrix->Str2)
#define TYPE	(AlignMatrix->Type)
#define ALIGN_TYPE	(AlignMatrix->AlignType)
#define SCORES	(AlignMatrix->Scores)
#define VALUE(i,j)	(AlignMatrix->M)[i][j].Value

	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);//Recupera el tamaño de la matriz
	int i,j;

	//Imprime los costos, el tipo de matriz y las secuencias asociadas
	//Imprime el tipo de matriz y alineamiento:
	printf("\n\t\tAlineamiento %s, Matriz de %s:\n",
		   (equStr(ALIGN_TYPE, "global") ? "Global" : "Local"),
		   (equStr(TYPE, "min") ? "Distancia" : "Similaridad"));
	//Imprime las secuencias:
	printf("\nStr1:\t%s\nStr2:\t%s\n", STR1, STR2);
	//Imprime los costos o scores
	printf("Scores:\tMatch = %f,\tReplacement = %f,\tInsertion = %f,\tDeletion = %f.\n", SCORES[0], SCORES[1], SCORES[2], SCORES[3]);

	//Imprime la matriz
	//Imprime primero la cadena de texto asociada a las columnas
	for(j=-1; j<cols; j++)
		printf("\t%c", (j<=0 ? ' ':STR1[j-1]));
	printf("\n");

	//Imprime las entradas de la matriz
	for(i=0; i<rows; i++)//Lee filas
	{
		printf("\t%c", (i==0 ? ' ':STR2[i-1]));//Imprime la cadena de texto asociada a las filas.
		for(j=0; j<cols; j++)//Lee el j-ésimo elemento de la i-ésima fila.
		{
			printf("\t%f", VALUE(i,j));//Imprime la entrada i,j de la matriz
		}
		printf("\n");
	}
#undef STR1
#undef STR2
#undef TYPE
#undef ALIGN_TYPE
#undef SCORES
#undef VALUE
}//___________________________________________________________



void PrintAlignMatrix(const A_Matrix *AlignMatrix)
//Imprime la matriz de alineamiento AlignMatrix con punteros.
{
#define STR1 (AlignMatrix->Str1)
#define STR2 (AlignMatrix->Str2)
#define TYPE	(AlignMatrix->Type)
#define ALIGN_TYPE	(AlignMatrix->AlignType)
#define SCORES	(AlignMatrix->Scores)
#define MATRIX (AlignMatrix->M)
#define VALUE(i,j) MATRIX[i][j].Value
#define N_POINTERS MATRIX[i][j].N_Pointers
#define POINTER(p) MATRIX[i][j].Pointers[p]
#define DIAG '\\'
#define VERT '|'
#define HORI '-'

	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);//Recupera el tamaño de la matriz
	int i,j,p;

	//Imprime los costos, el tipo de matriz y las secuencias asociadas
	//Imprime el tipo de matriz y alineamiento:
	printf("\n\t\tAlineamiento %s (%s), Matriz de %s (%s):\n",
		   (equStr(ALIGN_TYPE, "global") ? "Global" : "Local"), ALIGN_TYPE,
		   (equStr(TYPE, "min") ? "Distancia" : "Similaridad"), TYPE);
	//Imprime las secuencias:
	printf("\nStr1:\t%s\nStr2:\t%s\n", STR1, STR2);
	//Imprime los costos o scores
	printf("Scores:\tMatch = %f,\tReplacement = %f,\tInsertion = %f,\tDeletion = %f.\n", SCORES[0], SCORES[1], SCORES[2], SCORES[3]);

	//Imprime la matriz
	//Imprime primero la cadena de texto asociada a las columnas
	printf("\t\t");
	for(j=0; j<cols; j++)
		printf("%c\t", (j<=0 ? ' ':STR1[j-1]));
	printf("\n\n\n");
	//Imprime las entradas de la matriz
	for(i=0; i<rows; i++)//Lee filas
	{
		printf("\t%c\t", (i==0 ? ' ':STR2[i-1]));//Imprime la cadena de texto asociada a las filas.
		for(j=0; j<cols; j++)//Lee el j-ésimo elemento de la i-ésima fila.
		{
			printf("%d(", (int)VALUE(i,j));
			for(p=0; p<N_POINTERS; p++)
			{
				printf("%c", POINTER(p));
// 				if(POINTER(p)==DIAG)
// 					printf("\\");
// 				if(POINTER(p)==VERT)
// 					printf("|");
// 				if(POINTER(p)==HORI)
// 					printf("-");
			}
			printf(")\t");
		}
		printf("\n\n\n");
	}
#undef STR1
#undef STR2
#undef TYPE
#undef ALIGN_TYPE
#undef SCORES
#undef MATRIX
#undef VALUE
#undef N_POINTERS
#undef POINTER
#undef DIAG
#undef VERT
#undef HORI
}//___________________________________________________________



void Score(A_Matrix *Al, const int i, const int j, float (*Compare)(const float *numbers, const int size))
/*Llena el elemento i,j de la matriz de alineamiento Al. (Al->M)[i][j]=MATRIX[i][j] contiene el puntaje y los punteros.
La entrada MATRIX[i][j][0]=ENTRY(i,j) Contiene el puntaje de alineamiento entre Str1[i] y Str2[j], el puntaje se calcula según el esquema contenido en Al->Scores como sigue:
	M=(Al->Scores)[0] es el puntaje por emparejar,
	R=(Al->Scores)[1] es el puntaje por reemplazo,
	I=(Al->Scores)[2] es el puntaje por insertar letras,
	D=(Al->Scores)[3] es el puntaje por eliminar letras.
La entrada MATRIX[i][j].N_Pointers Contiene el número de punteros desde esa entrada
La entrada MATRIX[i][j].Pointers[p] Contiene el p-ésimo puntero, la codificación de los punteros es la siguiente:
	0=DIAG Es un puntero diagonal, 1=VERT Es un puntero vertical, 2=HORI Es un puntero horizontal.
La función Compare() que se pasa como argumento es Min() o Max() dependiendo del tipo de matriz (similaridad o distancia)
*/
{
#define STR1 (Al->Str1)
#define STR2 (Al->Str2)
#define MATRIX (Al->M)
#define N_POINTERS MATRIX[i][j].N_Pointers
#define VALUE(i,j) MATRIX[i][j].Value
#define POINTER(p) MATRIX[i][j].Pointers[p]
#define DIAGP '\\'
#define VERTP '|'
#define HORIP '-'
#define DIAG 0
#define VERT 1
#define HORI 2
#define MM (Al->Scores)[0]
#define R (Al->Scores)[1]
#define I (Al->Scores)[2]
#define D (Al->Scores)[3]
	//Inicializa
	N_POINTERS=0;//Número de punteros inicial nulo.


	if(i==0 || j==0)//Si está a la orilla
	{
		//.................................................
		//------------------------------------------------Condiciones base Score(0,j)=j*D, Score(i,0)=i*I.
		VALUE(i,j)=(i==0 ? j*D:i*I);//Coloca en la distancia inicial(condición base)
		//Coloca el puntero correcto
		if(i!=0)
		{
			POINTER(N_POINTERS)=VERTP;//Coloca el puntero vertical
			N_POINTERS++;//Incrementa la cuenta de punteros
		}
		else if(j!=0)//Coloca el puntero correcto
		{
			POINTER(N_POINTERS)=HORIP;//Coloca el puntero hacia la entrada horizontal
			N_POINTERS++;//Incrementa la cuenta de punteros
		}
	}
	else//Si no, calcula el valor de la relación de recurrencia
	{
		//..................................................................
		//------------------------------------------------------------------Relación de recurrencia
		float pos_dist[3];//Inicializa arreglo de los posibles valores del score
		//Calcula los posibles puntajes
		if(STR1[j-1]!=STR2[i-1])//Calcula el valor de un posible apareamiento o reemplazo
			pos_dist[DIAG]=(VALUE(i-1,j-1) + R);
		else
			pos_dist[DIAG]=(VALUE(i-1, j-1) + MM);
		pos_dist[VERT]=(VALUE(i-1, j) + I);//Calcula el valor de una posible inserción
		pos_dist[HORI]=(VALUE(i, j-1) + D);//Calcula el valor de una posible deleción

		//Finalmente, calcula la verdadera distancia y coloca los punteros adecuados
		VALUE(i,j)=Compare(pos_dist, 3);//Coloca el valor de la distancia en la matriz
		//Coloca los punteros
		if(pos_dist[DIAG]==VALUE(i,j))//Si la distancia fue dada por la entrada diagonal
		{
			POINTER(N_POINTERS)=DIAGP;//Coloca el puntero diagonal
			N_POINTERS++;//Incrementa la cuenta de punteros
		}
		if(pos_dist[VERT]==VALUE(i,j))//Si la distancia provino de la vertical
		{
			POINTER(N_POINTERS)=VERTP;//Coloca el puntero vertical
			N_POINTERS++;//Incrementa la cuenta de punteros
		}
		if(pos_dist[HORI]==VALUE(i,j))//Si la distancia provino de la entrada horizontal
		{
			POINTER(N_POINTERS)=HORIP;//Coloca el puntero hacia la entrada horizontal
			N_POINTERS++;//Incrementa la cuenta de punteros
		}
	}
#undef STR1
#undef STR2
#undef MATRIX
#undef N_POINTERS
#undef VALUE
#undef POINTER
#undef DIAG
#undef VERT
#undef HORI
#undef DIAGP
#undef VERTP
#undef HORIP
#undef MM
#undef R
#undef I
#undef D
}//___________________________________________________________



void FillAlignMatrix(A_Matrix *AlignMatrix)
/*Llena la matriz de de alineamiento A con las los puntajes obtenidos con la función Score
*La llena una fila a la vez.
*/
{
#define STR1 (AlignMatrix->Str1)
#define STR2 (AlignMatrix->Str2)
	//Recupera el tamaño de la matriz
	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);
	//Recupera el tipo de matriz, si de distancia (mínimo puntaje) o de similaridad (máximo puntaje)
	float (*Compare)(const float *numbers, const int size);//Puntero a función que seleccionará si se calcula el mínimo o máximo de los puntajes
	//Dependiendo del tipo de matriz(distancia o similaridad) elije la función para comparar
	if(equStr(AlignMatrix->Type, "min"))//Si es de distancia
	{
		Compare=Min;//Asigna la función de comparación como la función Min()
	}
	else if(equStr(AlignMatrix->Type, "max"))//Si es de similaridad
	{
		Compare=Max;//Asigna la función de comparación como la función Max()
	}
	else
	{
		printf("\nADVERTENCIA: Función de comparación no es ni min o max \"%s\" al obtener costos, se ha colocado la opción por defecto \"%s\"\n", (AlignMatrix->Type), DEFAULT_TYPE);
		Compare= (equStr(DEFAULT_TYPE, "min") ? Min : Max);//Si no es min o max, asigna la opción por default.
	}

	int i,j;
	//Primero coloca los elementos correspondientes a las condiciones base, i.e, con i=0 o j=0
	for(i=0; i<rows; i++)//En la primera columna
		Score(AlignMatrix, i, 0, Compare);
	for(j=0; j<cols; j++)//Llena la primera fila
		Score(AlignMatrix, 0, j, Compare);
	//Llena los demás elementos
	for(i=1; i<rows; i++)//Fila por fila
	{
		for(j=1; j<cols; j++)//Coloca el j-ésimo elemento de la i-ésima fila.
		{
			Score(AlignMatrix, i, j, Compare);
		}
	}
#undef STR1
#undef STR2
}//___________________________________________________________



void FreeAlignMatrix(A_Matrix *AlignMatrix)
/*
 * Libera el espacio de una estructura a_matrix previamente alojada con AllocAlignMatrix()
 */
{

	int i, rows=(strlen(AlignMatrix->Str2)+1);
	for(i=0; i<rows; i++)
	{
		free((AlignMatrix->M)[i]);
	}
	free(AlignMatrix->M);
	free(AlignMatrix->Str1);
	free(AlignMatrix->Str2);
	free(AlignMatrix->Scores);
	free(AlignMatrix->Type);
	free(AlignMatrix->AlignType);
	free(AlignMatrix);
}//___________________________________________________________
#undef DEFAULT_TYPE
#undef DEFAULT_ALIGN

# endif
