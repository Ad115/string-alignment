# ifndef STRING_ALIGN_GENERAL
# define STRING_ALIGN_GENERAL
/*
==============================
Librería: Funciones generales.
==============================

Funciones de uso general que pueden ser utilizadas en otros programas por su caracter poco específico.


Las siguientes son las funciones declaradas aquí:

:float Min(const float *numbers, const int size):
	Encuentra el mínimo de los valores en numbers, de longitud size.

:float Max(const float *numbers, const int size):
	Encuentra el máximo de los valores en numbers, donde numbers tiene size entradas.

:void swap(char *s, const int a, const int b):
	Intercambia los valores s[a] y s[b] en la cadena de texto str.

:void insert(const char c, const int pos, char *s):
	Inserta el caracter c en la cadena de texto str, de tal manera que str[pos]=c, los otros valores se recorren al final de la cadena.

:float *getNum(const char *str, const int pos):
	Obtiene el valor numérico en la cadena str desde la posición pos, además de la posición del siguiente caracter no numérico.

:char ***getArgs(char **argv, const int argc):
	De la lista de strings argv con argc cadenas, obtiene las variables declaradas como "--var=val".

:char *getVar(const char *str):
	str es una declaración de variable de la forma "--variable=valor". Obtiene "variable".

:char *getVal(const char *str):
	str es una declaración de variable de la forma "--variable=valor". Obtiene "valor".

:int equStr(const char *str1, const char *str2):
	Verdadero (1) si str1 es igual a str2.

char *dupStr(const char *ref_str):
	Genera un duplicado de la cadena de referencia ref_str y devuelve un puntero a la copia.

void copyStr(char *copy, const char *ref_str):
	Copia la cadena de referencia ref_str a copy, caracter por caracter, asume que hay espacio suficiente en copy.

Los prototipos de las funciones y las estructuras están declarados en el archivo alignments_headers.h

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

#define MIN(a, b) ((a) < (b) ? (a) : (b))
float Min(const float *numbers, const int size)
// Numbers es un arreglo de size números, Min(numbers, size) devuelve el menor valor.
{
	int i;
	float min;
	for(i=0, min=numbers[0]; i<size; i++)
		min=MIN(min, numbers[i]);
	return min;
}//___________________________________________________________



#define MAX(a, b) ((a) > (b) ? (a) : (b))
float Max(const float *numbers, const int size)
//Calcula el máximo de los valores en el arreglo numbers, size es el no. de elementos en numbers
{
	int i;
	float max;
	for(i=0, max=numbers[0]; i<size; i++)
		max=MAX(max, numbers[i]);
	return max;
}//___________________________________________________________



void swap(char *s, const int a, const int b)
//Intercambia los elementos s[a] y s[b] de s
{
    char tmp=s[a];
    s[a] = s[b];
    s[b] = tmp;
}//___________________________________________________________



void insert(const char c, const int pos, char *s)
/*
 * Inserta el caracter c en la posición pos de s (resulta en s[pos]=s y recorre los demás caracteres a la derecha)
 * Sólo funciona con strings alojadas con malloc(), calloc() o strcpy().
 */
{
	int i=strlen(s)+1; // El tamaño de s
	s = (char *) realloc(s, (i+1)*sizeof(char)); // Pon espacio en s para un caracter más
	s[i+1] = '\0'; // Pon el caractér final
	for(; i > pos; i--)
		swap(s, i, i-1);//Recorre uno a uno los caracteres hacia la izquierda para insertar el nuevo
	s[i]=c;//Inserta el nuevo caracter
}//___________________________________________________________



float *getNum(const char *str, const int pos)
/*
 * Obtiene el número entero decimal especificado en la cadena str comenzando desde la posición i.
 * Devuelve un arreglo donde la primera entrada es dicho número y la segunda es la posición siguiente de donde termina el número.
 * Ejemplo: getNum("abc123def", 3)={123,6}
 */
{
	float *num=(float *) malloc(2 * sizeof(float));//Inicializa el arreglo del resultado
	int i=pos, sign=1;
	if(str[i]=='+' || str[i]=='-')//Verifica si el número contiene signo
	{
		sign = (str[i]=='+' ? 1 : -1);//Coloca el signo correspondiente
		i++;
	}

	double sum=0, dec=0;
	for(; '0'<=str[i] && str[i]<='9';i++)//Obtén el valor absoluto de la parte entera del número
	{
		sum*=10;
		sum+=str[i]-'0';
	}

	if(str[i]=='.')//Checa si hay una parte fraccionaria
	{
		int j;
		for(j=1; '0'<=str[i+j] && str[i+j]<='9';j++)//Obtén el valor absoluto de la parte decimal
		{
			dec*=10;
			dec+=str[i+j]-'0';
		}
		for(; j>=0; j--)//Recorre el punto decimal
			dec/=10;
		i += j; // La posición siguiente
	}

	num[0]=(sum+dec)*sign;//Coloca el valor numérico con signo
	num[1]=i;//Coloca el índice de la siguiente posición
	return num;
}//___________________________________________________________



char *getVar(const char *str)
/*
 * De la cadena de caracteres str, con una variable de la forma "--variable=valor", obtiene la cadena "variable"
 */
{
	char * var = (char *) calloc(1, sizeof(char));
	int i, j;
	for(i=2, j=0; str[i]!='=' && str[i]!='\0'; i++, j++)
	{
		var=(char *)realloc(var, (j+1)*sizeof(char));
		var[j]=str[i];
	}
	var=(char *)realloc(var, (j+1)*sizeof(char));
	var[j]='\0';

	return var;
}//___________________________________________________________




char *getVal(const char *str)
/*
 * De la cadena de caracteres str, con una variable de la forma "--variable=valor", obtiene la cadena "valor"
 */
{
	char *val=(char *)calloc(1, sizeof(char));
	assert(val != NULL);
	int i, j;
	for(i=0; str[i]!='=' && str[i]!='\0'; i++)
		;
	if(str[i] == '=')
	{
		for(j=0, i++; str[i]!='\0'; i++, j++)
		{
			val=(char *)realloc(val, (j+1)*sizeof(char));
			assert(val != NULL);
			val[j]=str[i];
		}
		val=(char *)realloc(val, (j+1)*sizeof(char));
		assert(val != NULL);
		val[j]='\0';
	}
	else
	{
		free(val);
		val=NULL;
	}

	return val;
}//___________________________________________________________


char *searchArg(char *arg_name, char ***args)
// Busca la variable arg_name entre los las variables contenidas en args.
// args contiene pares (nombre, valor) de las variables pasadas como argumentos desde terminal.
// Devuelve el valor de la variable encontrada.
{
   int i;
   for(i=0; args[i] != NULL; i++)
   {
	   // Si la variable fué declarada...
	   if(equStr(args[i][0], arg_name))
	   {
		   return args[i][1]; // Regresa su valor (como string)
	   }
   }
   return NULL;
}//____________________________________________


 char ***getArgs(char **argv, const int argc)
/*
 * De la lista de strings argv con argc cadenas, obtiene las variables declaradas como "--var=val".
 * Las devuelve en una lista de pares, donde en cada par está el nombre de la variable y el valor, guardados como cadenas de texto.
 * El formato de los pares es: args[i][0]="var", args[i][1]="val".
 * args[i]=NULL marca el final de la lista.
 */
{
	char ***args=(char ***) calloc(1, sizeof(char **));
	assert(args != NULL);
	int i, nargs;
	for(i=0, nargs=0; i<argc; i++)
	{
		if(argv[i][0]=='-' && argv[i][1]=='-')//Si el argumento comienza con "--"
		{
			nargs++;//Incrementa la cuenta de pares
			args=(char ***)realloc(args, nargs*sizeof(char **));//Haz espacio para el nuevo par
			assert(args != NULL);
			args[nargs-1]=(char **)calloc(2, sizeof(char *));
			assert(args[nargs-1] != NULL);
			args[nargs-1][0]=getVar(argv[i]);
			args[nargs-1][1]=getVal(argv[i]);
		}
	}
	args = (char ***)realloc(args, (nargs+1)*sizeof(char **));
	args[nargs] = NULL;

	return args;
}//___________________________________________________________


void freeArgs(char ***args)
// Libera el espacio ocupado por el arreglo args.
// Se asume que args fué declarado con la función getArgs()
{
	int i;
   for(i=0; args[i] != NULL; i++)//Libera el espacio que ya no se necesita
   {
	   free(args[i][0]);
	   free(args[i][1]);
	   free(args[i]);
   }
   free(args);
}//__________________________________________


int equStr(const char *str1, const char *str2)
/*
 * Compara las cadenas de texto str1 y str2 caracter por caracter, regresando 1 o "TRUE" si son iguales y 0 o "FALSE" si son distintas
 */
{
	int i;
	for(i=0; str1[i]!='\0'; i++)
	{
		if(str1[i] != str2[i])
		   return 0;
	}
	if(str2[i]=='\0')
		return 1;
}//___________________________________________________________



char *dupStr(const char *ref_str)
/*
 * Genera un duplicado de la cadena de referencia ref_str y devuelve un puntero a la copia
 */
{
	char *copy = (char *) malloc( (strlen(ref_str)+1) * sizeof(char) );//Haz espacio para la copia
	if (copy != NULL)//Si no hubo problemas de espacio, procede a copiar la cadena
		copyStr(copy, ref_str);
	return copy;//Devuelve la nueva cadena
}//___________________________________________________________



void copyStr(char *copy, const char *ref_str)
/*
 * Copia la cadena de referencia ref_str a copy, caracter por caracter, asume que hay espacio suficiente en copy
 */
{
	int i;
	for(i=0; ref_str[i] != '\0'; i++)
		copy[i] = ref_str[i];
	copy[i] = '\0';
}//___________________________________________________________

# endif
