# ifndef STRING_ALIGN_ALIGNMENT_HEADERS
# define STRING_ALIGN_ALIGNMENT_HEADERS
/*
===============================================================
Declaraciones básicas para programa de alineamiento de cadenas.
===============================================================

Aquí se encuentran los prototipos de funciones y declaracion de estructuras necesarias para el alineamiento de cadenas,
este es usado en global-align.c donde se encuentra la función main().
Las declaraciones se dividen en secciones, la primera sección corresponde a las estructuras, las siguientes secciones corresponden
a los prototipos de funciones, agrupadas de acuerdo al archivo donde se encuentra el código desarrollado de la función.

Andrés García García @ Sab 26/Mar/2016. (Proyecto inició 19 Oct '15)



	~~~~~~~~~~~~~~~~~~~~~~~
	Estructuras principales
	~~~~~~~~~~~~~~~~~~~~~~~
	Declaración de las estructuras básicas utilizadas en el programa.
::*/
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



/*
	~~~~~~~~~~~~~~~~~~~
	Funciones generales
	~~~~~~~~~~~~~~~~~~~
	Fuente: general.h
	Funciones de uso general que pueden ser utilizadas en otros programas por su caracter poco específico.
::*/
		float Min(const float *numbers, const int size); // Encuentra el mínimo de los valores en numbers, de longitud size
		float Max(const float *numbers, const int size); // Encuentra el máximo de los valores en numbers, donde numbers tiene size entradas
		void swap(char *s, const int a, const int b); // Intercambia los valores s[a] y s[b] en la cadena de texto str
		void insert(const char c, const int pos, char *s); //Inserta el caracter c en la cadena de texto str, de tal manera que str[pos]=c, los otros valores se recorren al final de la cadena
		float *getNum(const char *str, const int pos); //Obtiene el valor numérico en la cadena str desde la posición pos, además de la posición del siguiente caracter no numérico.
		char ***getArgs(char **argv, const int argc); //De la lista de strings argv con argc cadenas, obtiene las variables declaradas como "--var=val".
		char *getVar(const char *str); //str es una declaración de variable de la forma "--variable=valor". Obtiene "variable".
		char *getVal(const char *str); //str es una declaración de variable de la forma "--variable=valor". Obtiene "valor". 
		int equStr(const char *str1, const char *str2); //Verdadero (1) si str1 es igual a str2.
		char *dupStr(const char *ref_str); //Genera un duplicado de la cadena de referencia ref_str y devuelve un puntero a la copia
		void copyStr(char *copy, const char *ref_str); // Copia ref_str a copy, asumiendo que el espacio es suficiente hasta el primer caracter nulo



/*
	~~~~~~~~~~~~~~~~~~~~~~
	Matriz de alineamiento
	~~~~~~~~~~~~~~~~~~~~~~
	Fuente: alignment_matrix.h
	Funciones especializadas para la estructura A_Matrix (y su subsidiaria Entry), que representa una matriz de alineamiento.
	Contiene funciones para crear, llenar, imprimir y destruir matrices de alineamiento.
::*/
		A_Matrix *AllocAlignMatrix(const char *str1, const char *str2, const char *type, const char *aligntype, const float *scores); //Genera espacio para una matriz de alineamiento general con las cadenas de texto Str1 y Str2
		float *getScores(const char *scoreStr); //Obtiene los costos de operación expresados en scoreStr.
		void Score(A_Matrix *Al, const int i, const int j, float (*Compare)(const float *numbers, const int size)); // LLena la entrada i,j de la matriz AlignMatrix utilizando la función Compare para seleccionar la entrada correcta
		void FillAlignMatrix(A_Matrix *AlignMatrix); // LLena la matriz AlignMatrix con los valores y punteros correspondientes con ayuda de la función Score()
		void PrintAlignMatrixNoPointers(const A_Matrix *AlignMatrix);//Imprime sólo las entradas de la matriz de alineamiento AlignMatrix (sin punteros)
		void PrintAlignMatrix(const A_Matrix *AlignMatrix); //Imprime la matriz de alineamiento AlignMatrix, incluyendo los punteros de traceback, estos son: \(diagonal) |(vertical) y _(horizontal)
		void FreeAlignMatrix(A_Matrix *AlignMatrix); //  Libera el espacio de una estructura a_matrix previamente alojada con AllocAlignMatrix()



/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Recuperación de caminos (Traceback)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Fuente: traceback.h
	Funciones especializadas para la estructura Traceback y su subsidiaria Path.
	Contiene funciones para crear y llenar tracebacks a partir de la matriz de alineamiento, así como para destruirlos cuando ya no se usen.
	Contiene funciones para crear, copiar y extender estructuras Path.
::*/
		Traceback *AllocTracebackForMatrix(const A_Matrix *AlignMatrix);
		Traceback *TracebackFromMatrixEntry(const A_Matrix *AlignMatrix, const int i, const int j);
		void FreeTraceback(Traceback *traceback); // Libera el espacio ocupado por la estructura traceback previamente alojada por AllocTracebackForMatrix()
		void AllocMorePaths(Traceback *traceback, const int extra_entries, const int path_size);
		void ExtendPath(Path *path, const int extra_entries);
		void PathCopy(const Path *p1, Path *p2, const int skip);//Copia el camino en p1 a p2, excepto las últimas skip entradas



/*
	~~~~~~~~~~~~~~~~~~~~~~~
	Alineamiento de cadenas
	~~~~~~~~~~~~~~~~~~~~~~~
	Fuente: alignment.h
	Funciones principales para crear alineamientos explícitos de cadenas por medio de la estructura Align.
	Contiene las funciones principales para crear alineamientos.
	Contiene funciones especializadas para crear, llenar, imprimir y destruir alineamientos.
	Contiene funciones para crear e imprimir los códigos de edición de los alineamientos.
::*/
		void GlobalAlignment(const char *str1, const char *str2, const char *type, const float *scores); // Obtiene e imprime todos los alineamientos globales óptimos de str1 y str2 con las características especificadas.
		Align *ExplAlignsFromTraceback(const Traceback *traceback); // Obtiene los alineamientos desde los caminos obtenidos.
		void PrintExplAlign(const Align *align); // Muestra el alineamiento explícito Align.
		void PrintAlignments(const Align *align); // Imprime los alineamientos en el arreglo alignments.
		void FreeAligns(Align *alignments); // Libera el espacio ocupado por la estructura alignments previamente alojada por ExplAlignsFromTraceback().
		char *EditTrFromExplAlign(const Align *align); // Genera el código de edición asociado al alineamiento Align.
		void PrintEditTr(const char *edit_tr); // Muestra el código de edición en el mismo formato en que se muestra el alineamiento en PrintExplAlign().

# endif