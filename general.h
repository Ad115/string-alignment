/*
*****************************************************************************
                        Librería de funciones generales v0.0                 	*
*****************************************************************************/
/*
***Librería que contiene funciones no especializadas.
	Las funciones de esta versión se utilizan en global_alignment3.1.c

***La librería contiene:
*/
	//1. Funciones generales:				//NO ESPECIALIZADAS, PUEDEN SER USADAS EN OTROS PROGRAMAS
		float Min(float *numbers, int size); // Encuentra el mínimo de los valores en numbers, de longitud size
		float Max(float *numbers, int size); // Encuentra el máximo de los valores en numbers, donde numbers tiene size entradas
		void swap(char *s, int a, int b); // Intercambia los valores s[a] y s[b] en la cadena de texto str
		void insert(char c, int pos, char *s); //Inserta el caracter c en la cadena de texto str, de tal manera que str[pos]=c, los otros valores se recorren al final de la cadena
		float *getNum(char *str, int pos); //Obtiene el valor numérico en la cadena str desde la posición pos, además de la posición del siguiente caracter no numérico.
		char ***getArgs(char **argv, int argc); //De la lista de strings argv con argc cadenas, obtiene las variables declaradas como "--var=val".
		char *getVar(char *str); //str es una declaración de variable de la forma "--variable=valor". Obtiene "variable".
		char *getVal(char *str); //str es una declaración de variable de la forma "--variable=valor". Obtiene "valor". 
		int equStr(char *str1, char *str2); //Verdadero (1) si str1 es igual a str2.
	
/*	
Andrés García García @ 28/Feb/'16 (Inicio 19 Oct 2015)
*/

#include<stdio.h>
#include<stdlib.h>//Para usar malloc(), calloc(), realloc(), free()
#include<string.h>//Para usar strlen() y strcpy()



//**********************---------1.--------******************************************************
//**********************FUNCIONES GENERALES******************************************************

#define MIN(a, b) (a<b ? a : b)

float Min(float *numbers, int size)
//Calcula el mínimo de los valores en el arreglo numbers, size es el no. de elementos en numbers
{
	int i;
	float min;
	for(i=0, min=numbers[0]; i<size; i++)
		min=MIN(min, numbers[i]);
	return min;
}



#define MAX(a, b) (a>b ? a : b)

float Max(float *numbers, int size)
//Calcula el máximo de los valores en el arreglo numbers, size es el no. de elementos en numbers
{
	int i;
	float max;
	for(i=0, max=numbers[0]; i<size; i++)
		max=MAX(max, numbers[i]);
	return max;
}



void swap(char *s, int a, int b)
//Intercambia los elementos s[a] y s[b] de s
{
    char tmp=s[a];
    s[a] = s[b];
    s[b] = tmp;
}//___________________________________________________________



void insert(char c, int pos, char *s)
//Inserta el caracter c en la posición pos de s (resulta en s[pos]=s y recorre los demás caracteres a la derecha)
//Sólo funciona con strings alojadas con malloc(), calloc() o strcpy().
{
	int i=strlen(s)+1;
	s=(char *) realloc(s, (i+1)*sizeof(char));
	for(; i>pos; i--)
		swap(s, i, i-1);
	s[i]=c;
}//___________________________________________________________



float *getNum(char *str, int pos)
/*
 * Obtiene el número entero decimal especificado en la cadena str comenzando desde la posición i.
 * Devuelve un arreglo donde la primera entrada es dicho número y la segunda es la posición siguiente de donde termina el número.
 * Ejemplo: getNum("abc123def", 3)={123,6}
 */
{
	float *num=(float *)calloc(2, sizeof(float));//Inicializa el arreglo del resultado
	int i=pos, sign=1;
	if(str[pos]=='+' || str[pos]=='-')//Verifica si el número contiene signo 
	{
		sign = (str[pos]=='+' ? 1 : -1);//Coloca el signo correspondiente
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
		for(; j-1>0; j--)
			dec/=10;
	}
	
	num[0]=(sum+dec)*sign;//Coloca el valor numérico con signo
	num[1]=i;//Coloca el índice de la siguiente posición
	return num;
}//___________________________________________________________



char *getVar(char *str)
/*
 * De la cadena de caracteres str, con una variable de la forma "--variable=valor", obtiene la cadena "variable"
 */
{
	char *var=(char *)calloc(1, sizeof(char));
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




char *getVal(char *str)
/*
 * De la cadena de caracteres str, con una variable de la forma "--variable=valor", obtiene la cadena "valor"
 */
{
	char *val=(char *)calloc(1, sizeof(char));
	int i, j;
	for(i=0; str[i]!='=' && str[i]!='\0'; i++)
		;
	if(str[i] == '=')
	{
		for(j=0, i++; str[i]!='\0'; i++, j++)
		{
			val=(char *)realloc(val, (j+1)*sizeof(char));
			val[j]=str[i];
		}
		val=(char *)realloc(val, (j+1)*sizeof(char));
		val[j]='\0';
	}
	else
		val=NULL;
	
	return val;
}//___________________________________________________________




 char ***getArgs(char **argv, int argc)
/*
 * De la lista de strings argv con argc cadenas, obtiene las variables declaradas como "--var=val".
 * Las devuelve en una lista de pares, donde en cada par está el nombre de la variable y el valor, guardados como cadenas de texto;
 * el útlimo par contiene apuntadores nulos para marcar el fin de la lista de pares.
 * El formato de los pares es: args[i][0]="var", args[i][1]="val".
 */
{
	int i, nargs=0;
	char ***args=(char ***) calloc(1, sizeof(char **));
	for(i=0; i<argc; i++)
	{
		if(argv[i][0]=='-' && argv[i][1]=='-')//Si el argumento comienza con "--"
		{
			nargs++;//Incrementa la cuenta de pares
			args=(char ***)realloc(args, nargs*sizeof(char **));//Haz espacio para el nuevo par
			args[nargs-1]=(char **)calloc(2, sizeof(char *));
			args[nargs-1][0]=getVar(argv[i]);
			args[nargs-1][1]=getVal(argv[i]);
		}
	}
	args=(char ***)realloc(args, (nargs+1)*sizeof(char **));
	args[nargs]=(char **)calloc(2, sizeof(char *));
	args[nargs][0]="", args[nargs][1]="";
	
	return args;
}//___________________________________________________________



int equStr(char *str1, char *str2)
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
