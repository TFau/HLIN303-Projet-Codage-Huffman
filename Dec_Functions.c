#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Dec_Functions.h"

bool oned(unsigned char byte)
{
	bool bit;
	for(int i=7; i > 0; i--) {
		bit=byte&(1<<i);
		if(bit) {
			return false;
		}
	}
	bit=byte&(1<<0);
	if(bit)
		return true;
	else return false;
}

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

unsigned char decodeSingle(unsigned char carrier, int* fill)
{
	unsigned char decode='\0';
	if(carrier & 1<<(7-*fill))	//if 1 is read at the position
		decode|=(1<<0); //decode set to 00000001
	//else decode is still a nullchar
	(*fill)++;
	return decode;
}

void decodeIDX(unsigned char carrier, int* fill, int* code_read, unsigned char* T, int* pos)
{
	while(*code_read < 8 && *fill < 8) {	//To the end of the symbol byte or to the end of the carrier byte
		if(carrier & (1<<(7-*fill))) {
			T[*pos]='1';
		}
		else {
			T[*pos]='0';
		}
		(*pos)++;
		(*fill)++;
		(*code_read)++;
	}	
}

unsigned char stringBuild(unsigned char* buffer, int pos)
{
	//Function always finds 8 bits, no special checks needed
	unsigned char string='\0';
	int string_pos=0;
	for(int i=0; i <= pos; i++) {
		if(buffer[i] & 1) {	//Character '1'
			string|=(1<<(7-string_pos));
		}
		else {	//Character '0'
			string&=(~(1<<(7-string_pos)));
		}
		string_pos++;
	}
	return string;
}

int codeCheck(unsigned char* T, int pos, unsigned char** DT, int size)
{
	int j, length;
	for(int i=0; i < size; i++) {
		j=0;
		if(DT[i]) {		//Not a NULL pointer
			length=strlen(DT[i]);
			while(j < pos && j < length && T[j] == DT[i][j]) {
				j++;
			}
			if(j == length) {
				return i;
			}
		}
	}
	return -1;
}

int decodeMSG(unsigned char carrier, int* fill, unsigned char* T, int* pos, unsigned char** Dec_T, int size)
{
	//No code_read variable: no advance knowledge of the length of the code
	int character;
	while(*fill < 8) {
		if(carrier & (1<<(7-*fill))) {
			T[*pos]='1';
		}
		else {
			T[*pos]='0';
		}
		(*pos)++;
		(*fill)++;
		//Checks codetable after every added bit
		character=codeCheck(T,*pos,Dec_T,size);
		//Returns character linked to code if it exists
		if(character > 0) {
			return character;
		}
	}
	//No code found yet after reading carrier byte
	return -1;
}