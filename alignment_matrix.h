/*
*****************************************************************************
                        Librería: MATRIZ DE ALINEAMIENTO v0.0                 	*
*****************************************************************************/
/*
***Librería que contiene funciones y estructuras implementando matrices de alineamiento. 
	Las funciones de esta versión se utilizan en global_alignment3.1.c

***La librería contiene:
*/
	//1. Estructura de matriz de alineamiento:
		struct a_matrix; // Formato general de las matrices de alineamiento
		
		
	//2. Funciones generales para matrices de alineamiento:		//FUNCIONES ESPECIALIZADAS PARA OBTENER LOS COSTOS, ALOJAR EN MEMORIA, LLENAR LA MATRIZ E IMPRIMIRLA.
		struct a_matrix *AllocAlignMatrix(char *str1, char *str2, char *type, float *scores); //Genera espacio para una matriz de alineamiento general con las cadenas de texto Str1 y Str2
		float *getScores(char *scoreStr); //Obtiene los costos de operación expresados en scoreStr.
		void Score(struct a_matrix *Al, int i, int j, float (*Compare)(float *numbers, int size)); // LLena la entrada i,j de la matriz AlignMatrix utilizando la función Compare para seleccionar la entrada correcta
		void FillAlignMatrix(struct a_matrix *AlignMatrix); // LLena la matriz AlignMatrix con los valores y punteros correspondientes con ayuda de la función Score()
		void PrintAlignMatrixNoPointers(struct a_matrix *AlignMatrix);//Imprime sólo las entradas de la matriz de alineamiento AlignMatrix (sin punteros)
		void PrintAlignMatrix(struct a_matrix *AlignMatrix); //Imprime la matriz de alineamiento AlignMatrix, incluyendo los punteros de traceback, estos son: \(diagonal) |(vertical) y _(horizontal)
	
//El código utiliza estructuras y funciones declaradas en general.h, y éste deberá ser debidamente incluido en el programa principal.
	
/*	
Los algoritmos están basados en los encontrados en el libro:
	Algoritms on Strings, Trees and Sequences-Computer Science and Computational Biology
	Dan Gusfield
	Cambridge University Press (1997)
Andrés García García @ 28/Feb/'16 (Inicio 19 Oct 2015)
*/

#include<stdio.h>
#include<stdlib.h>//Para usar malloc(), calloc(), realloc(), free()
#include<string.h>//Para usar strlen() y strcpy()






//**********************---------1.--------************************************************************
//**********************ESTRUCTURA FUNDAMENTAL: Matriz de alineamiento*********************************

struct a_matrix
//Estructura de la matriz de alineamiento, se compone de ambas secuencias de texto y las entradas de la matriz.
{
	char *Str1;
	char *Str2;
	char *Type;
	float *Scores;
	float ***M;
};//___________________________________________________________




/**********************---------2.--------******************************************************
**********************FUNCIONES GENERALES PARA MATRICES DE ALINEAMIENTO************************/

struct a_matrix *AllocAlignMatrix(char *str1, char *str2, char *type, float *scores)
/*
Aloja espacio para crear una matriz de  alineamiento o similaridad de los textos Str1 y Str2.
Srt1 es representada en las columnas y Str2 en las filas.
La matriz resultante tiene una estructura como la que se muestra:
	matrix[i]={fila i},	donde i=0...m, 
	matrix[i][j]={ elemento(i,j), no. de punteros desde el elemento, <puntero1>, <puntero2>, <puntero3>,...}
	matrix->Str1=Cadena de texto a transformar.
	matrix->Str2=Cadena de texto objetivo.
	matrix->Scores=[M, R, I, D] (arreglo con los costos de operación usados para llenar la matriz
Devuelve un puntero a una estructura de matriz.
 */
{
	struct a_matrix *A=(struct a_matrix *)malloc(sizeof(A));//Crea la estructura donde se aloja la matriz
	
	//Define las cadenas a alinear
	A->Str1=str1; A->Str2=str2;//Almacena las cadenas en la estructura
	
	//Define el tipo de matriz
	if(type==NULL)
		A->Type="min";//Define la matriz por default como matriz de distancia(se llena con el mínimo de los valores posibles)
	else
		A->Type=type;
	
	//Define los costos de operación
	if(scores==NULL)
	{
		float *Scores=(float *)calloc(4, sizeof(float));//Genera el arreglo de costos
		Scores[0]=0, Scores[1]=1, Scores[2]=1, Scores[3]=1;//Carga los valores por default
		A->Scores=Scores;
	}
	else
		A->Scores=scores;
	
	int rows=strlen(str2)+1, cols=strlen(str1)+1;//Inicializa para crear la matriz
	//Genera la matriz en si
	float ***M=(float ***) malloc(rows*sizeof(float **));//Genera el espacio para la matriz, en específico para los punteros a las filas
	int i, j;
	for(i=0; i<rows; i++)
	{
		M[i]=(float **) malloc(cols*sizeof(float *));//Genera espacio para las filas, M[i] es una fila
		for(j=0; j<cols; j++)
		{
			M[i][j]=(float *) malloc(2*sizeof(float));//Inicializa espacio para las entradas de la matriz y el número de punteros.
			M[i][j][1]=0;//Inicializa el número de punteros como nulo
		}
	}
	A->M=M;
	
	return A;
}//___________________________________________________________



float *getScores(char *scoreStr)
/*Obtiene un arreglo con los costos de operación con el formato: [ M, R, I, D] (Match, Replacement, Insertion, Deletion).
 * Recibe una string donde se especifican pares operación costo, por ejemplo, si se requiere que M=1, I=-1, R=0,
 * una cadena apropiada sería "M1I-1R0", no importa el orden en que se especifican los costos y no es necesario 
 * especificar valores para todas las operaciones.
 * Si un valor no se especifica, los valores por defecto son: M=0, R=1, I=1, D=1
 */
{
	float *scores=(float *)calloc(4, sizeof(float));//Genera el arreglo con los costos por default
	scores[0]=0, scores[1]=1, scores[2]=1, scores[3]=1;//Carga los valores por default
	//Carga los valores en la secuencia otorgada
	float  *temp;
	int i;
	for(i=0; scoreStr[i]!='\0';)//Busca en todo el texto
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



void PrintAlignMatrixNoPointers(struct a_matrix *AlignMatrix)
//Imprime la matriz de alineamiento AlignMatrix
{
#define STR1 (AlignMatrix->Str1)
#define STR2 (AlignMatrix->Str2)
#define ENTRY(i,j) (AlignMatrix->M)[i][j][0]
	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);//Recupera el tamaño de la matriz
	int i,j;
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
			printf("\t%f", ENTRY(i,j));//Imprime la entrada i,j de la matriz
		}
		printf("\n");
	}
#undef STR1
#undef STR2
#undef MATRIX 
#undef ENTRY
}//___________________________________________________________



void PrintAlignMatrix(struct a_matrix *AlignMatrix)
//Imprime la matriz de alineamiento AlignMatrix con punteros.
{
#define STR1 (AlignMatrix->Str1)
#define STR2 (AlignMatrix->Str2)
#define MATRIX (AlignMatrix->M)
#define ENTRY(i,j) MATRIX[i][j][0]
#define N_POINTERS MATRIX[i][j][1]
#define POINTER(p) MATRIX[i][j][1+p]
#define DIAG 0
#define VERT 1
#define HORI 2
	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);//Recupera el tamaño de la matriz
	int i,j,p;
	//Imprime primero la cadena de texto asociada a las columnas
	printf("\t\t");
	for(j=0; j<cols; j++)
		printf("%c\t\t", (j<=0 ? ' ':STR1[j-1]));
	printf("\n\n\n");
	//Imprime las entradas de la matriz
	for(i=0; i<rows; i++)//Lee filas
	{
		printf("\t%c\t", (i==0 ? ' ':STR2[i-1]));//Imprime la cadena de texto asociada a las filas.
		for(j=0; j<cols; j++)//Lee el j-ésimo elemento de la i-ésima fila.
		{
			printf("%f(", ENTRY(i,j));
			for(p=1; p<=N_POINTERS; p++)
			{
				if(POINTER(p)==DIAG)
					printf("\\");
				if(POINTER(p)==VERT)
					printf("|");
				if(POINTER(p)==HORI)
					printf("_");
			}
			printf(")\t");
		}
		printf("\n\n\n");
	}
#undef STR1
#undef STR2
#undef MATRIX 
#undef ENTRY
#undef N_POINTERS
#undef POINTER
#undef DIAG
#undef VERT
#undef HORI
}//___________________________________________________________



void Score(struct a_matrix *Al, int i, int j, float (*Compare)(float *numbers, int size))
/*Llena el elemento i,j de la matriz de alineamiento Al. (Al->M)[i][j]=MATRIX[i][j] contiene el puntaje y los punteros.
La entrada MATRIX[i][j][0]=ENTRY(i,j) Contiene el puntaje de alineamiento entre Str1[i] y Str2[j], el puntaje se calcula según el esquema contenido en Al->Scores como sigue:
	M=(Al->Scores)[0] es el puntaje por emparejar,
	R=(Al->Scores)[1] es el puntaje por reemplazo,
	I=(Al->Scores)[2] es el puntaje por insertar letras,
	D=(Al->Scores)[3] es el puntaje por eliminar letras.
La entrada MATRIX[i][j][1]=N_POINTERS Contiene el número de punteros desde esa entrada
La entrada MATRIX[i][j][1+p]=POINTER(p) Contiene el p-ésimo puntero, la codificación de los punteros es la siguiente:
	0=DIAG Es un puntero diagonal, 1=VERT Es un puntero vertical, 2=HORI Es un puntero horizontal.
La función Compare() que se pasa como argumento es Min() o Max() dependiendo del tipo de matriz (similaridad o distancia)
*/
{
#define STR1 (Al->Str1)
#define STR2 (Al->Str2)
#define MATRIX (Al->M) 
#define N_POINTERS (int)MATRIX[i][j][1]
#define ENTRY(i,j) MATRIX[i][j][0]
#define POINTER(p) MATRIX[i][j][1+p]
#define DIAG 0
#define VERT 1
#define HORI 2
#define m (Al->Scores)[0]
#define R (Al->Scores)[1]
#define I (Al->Scores)[2]
#define D (Al->Scores)[3]
	//NOTA: La matriz sin llenar tiene por default el número de punteros en cada entrada como 0;
	if(i==0 || j==0)//Si está a la orilla
	{
		//.................................................
		//------------------------------------------------Condiciones base Score(0,j)=j*D, Score(i,0)=i*I.
		ENTRY(i,j)=(i==0 ? j*D:i*I);//Coloca en la distancia inicial(condición base)
		//Coloca el puntero correcto
		if(i!=0)
		{
			N_POINTERS++;//Incrementa la cuenta de punteros
			MATRIX[i][j]=realloc(MATRIX[i][j], (N_POINTERS+2)*sizeof(int));
			POINTER(N_POINTERS)=VERT;//Coloca el puntero vertical
		}
		else if(j!=0)//Coloca el puntero correcto
		{
			N_POINTERS++;//Incrementa la cuenta de punteros
			MATRIX[i][j]=realloc(MATRIX[i][j], (N_POINTERS+2)*sizeof(int));
			POINTER(N_POINTERS)=HORI;//Coloca el puntero hacia la entrada horizontal
		}
	}
	else//Si no, calcula el valor de la relación de recurrencia
	{
		//..................................................................
		//------------------------------------------------------------------Relación de recurrencia
		float pos_dist[3];//Inicializa arreglo de los posibles valores del score		
		//Calcula los posibles puntajes
		if(STR1[j-1]!=STR2[i-1])//Calcula el valor de un posible apareamiento o reemplazo
			pos_dist[DIAG]=(ENTRY(i-1,j-1) + R);
		else
			pos_dist[DIAG]=(ENTRY(i-1, j-1) + m);
		pos_dist[VERT]=(ENTRY(i-1, j) + I);//Calcula el valor de una posible inserción
		pos_dist[HORI]=(ENTRY(i, j-1) + D);//Calcula el valor de una posible deleción
		
		//Finalmente, calcula la verdadera distancia y coloca los punteros adecuados
		ENTRY(i,j)=Compare(pos_dist, 3);//Coloca el valor de la distancia en la matriz
		//Coloca los punteros
		if(pos_dist[DIAG]==ENTRY(i,j))//Si la distancia fue dada por la entrada diagonal
		{
			N_POINTERS++;//Incrementa la cuenta de punteros
			MATRIX[i][j]=realloc(MATRIX[i][j], (N_POINTERS+2)*sizeof(int));
			POINTER(N_POINTERS)=DIAG;//Coloca el puntero diagonal
		}
		if(pos_dist[VERT]==ENTRY(i,j))//Si la distancia provino de la vertical
		{
			N_POINTERS++;//Incrementa la cuenta de punteros
			MATRIX[i][j]=realloc(MATRIX[i][j], (N_POINTERS+2)*sizeof(int));
			POINTER(N_POINTERS)=VERT;//Coloca el puntero vertical
		}
		if(pos_dist[HORI]==ENTRY(i,j))//Si la distancia provino de la entrada horizontal
		{
			N_POINTERS++;//Incrementa la cuenta de punteros
			MATRIX[i][j]=realloc(MATRIX[i][j], (N_POINTERS+2)*sizeof(int));
			POINTER(N_POINTERS)=HORI;//Coloca el puntero hacia la entrada horizontal
		}
	}
#undef STR1
#undef STR2
#undef MATRIX 
#undef N_POINTERS
#undef ENTRY
#undef POINTER
#undef DIAG
#undef VERT
#undef HORI
#undef M
#undef R
#undef I
#undef D
}//___________________________________________________________



void FillAlignMatrix(struct a_matrix *AlignMatrix)
/*Llena la matriz de de alineamiento A con las los puntajes obtenidos con la función Score
*La llena una fila a la vez.
*/
{
#define STR1 (AlignMatrix->Str1)
#define STR2 (AlignMatrix->Str2)
#define DEFAULT_COMPARE Min
	//Recupera el tamaño de la matriz
	int rows=(strlen(STR2)+1), cols=(strlen(STR1)+1);
	//Recupera el tipo de matriz, si de distancia (mínimo puntaje) o de similaridad (máximo puntaje)
	float (*Compare)(float *numbers, int size);//Puntero a función que seleccionará si se calcula el mínimo o máximo de los puntajes
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
		printf("\nADVERTENCIA: Función de comparación no es ni min o max \"%s\" al obtener costos, por defecto se ha colocado DEFAULT_COMPARE\n", AlignMatrix->Type); 
		Compare=DEFAULT_COMPARE;//Si no es min o max, asigna la opción por default.
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
#undef DEFAULT_COMPARE
}//___________________________________________________________

