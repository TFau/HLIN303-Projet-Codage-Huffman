#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Com_Functions.h"

unsigned char* extractCode(struct node* T, int x)
{
	int i=x, j, k;
	unsigned char* code=malloc(256*sizeof(unsigned char));
	code[0]='\0';
	while(T[i].parent != -1) {
		j=i;
		i=T[i].parent;
		k=strlen(code);	//when used as array indice returns null character
		while(k >= 0) {
			code[k+1]=code[k];
			k--;
		}
		if(T[i].child_left == j) {
			code[0]='0';
		}
		else {
			code[0]='1';
		}
	}
	//Optimization: reduce string size
	unsigned char* codeOpt=realloc(code,(strlen(code)+1)*sizeof(unsigned char));
	//No need to free(code), realloc frees code itself
	//"Causes double free or corruption (out)" error and crash
	if(!(codeOpt))
		free(code);
	else
		code=NULL;
	return codeOpt;	//NULL pointer if realloc failed
}

bool leftmost(int* T, int size, int n)
{
	int testval=T[n];
	//Smallest positive value is leftmost node
	for(int i=0; i < size; i++) {
		if(T[i] >= 0 && T[i] < testval) {
			return false;
		}
	}
	return true;
}

unsigned char* binarychar(char n)
{
	//Convert char numerical value to binary value held in 8-char array
	unsigned char *T=malloc(9*sizeof(unsigned char));	//Array with null terminator cell
	int r;
	for(int i=7; i >=0; i--) {
		if(n > 0) {
			r=n%2;
			if(r & 1) {
				T[i]='1';
			}
			else {
				T[i]='0';
			}
			n/=2;
		}
		else {
			T[i]='0';	//Fill out most significant bits with 0's if needed
		}
	}
	T[8]='\0';
	return T;
}

//Could pass the carrier as a pointer, and return void
unsigned char encode(unsigned char carrier, int* fill, unsigned char* code, int* code_read)
{
	int size=strlen(code)-*code_read;	//Code to write, without null terminator
	//8 encodeable bits
	while(size > 0 && *fill < 8) {	//While there is still code to write and the carrier byte isn't full
		//Starts reading code array after already written code
		if(code[*code_read] & 1) {	//'1' is coded as 49, odd number with least significant bit 1
			carrier|=(1<<(7-*fill));
		}
		else {
			carrier&=(~(1<<(7-*fill)));
		}
		size--;
		(*fill)++;	//Without parentheses, ++ increments the pointer itself (operator has higher priority than *)
		(*code_read)++;
	}
	return carrier;
}