#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Dec_Functions.h"

int readStart(FILE* reader, int* total_char, int* unique_char)
{
	unsigned int buf[1]; //4 byte storage
	fread(buf,sizeof(buf),1,reader);
	*total_char=buf[0];
	if(ferror(reader)) {
		fputs("Erreur durant la lecture.\n", stderr);
		return 1;
	}
	*unique_char=fgetc(reader);
	if(ferror(reader)) {
		fputs("Erreur durant la lecture.\n", stderr);
		return 2;
	}
	return 0;
}

void initTree(struct node* T, int size)
{
	for(int i = 0; i < size; i++) {
		T[i].symbol=0;
		T[i].parent=-1;
		T[i].child_left=-1;
		T[i].child_right=-1;
	}
}

unsigned char decodeSingle(unsigned char* carrier, int* fill)
{
	unsigned char decode='\0';
	if(*carrier & 1<<(CHAR_BIT-1-*fill))	//if 1 is read at the position
		decode|=(1<<0); //decode set to 0x01
	//else decode is still a nullchar
	(*fill)++;
	return decode;
}

unsigned char decodeIDX(unsigned char* carrier, int* fill, unsigned char* T_byte, int* code_read)
{
	while(*code_read < CHAR_BIT && *fill < CHAR_BIT) {	//To the end of the symbol byte or to the end of the carrier byte
		if(*carrier & (1<<(CHAR_BIT-1-*fill))) {
			*T_byte|=(1<<(CHAR_BIT-1-*code_read));
		}
		else {
			*T_byte&=(~(1<<(CHAR_BIT-1-*code_read)));
		}
		(*code_read)++;
		(*fill)++;
	}
	return *T_byte;	
}

int readChar(FILE* reader, unsigned char* carrier, int* fill)
{
	*carrier=fgetc(reader);
	if(ferror(reader)) {
		fputs("Erreur durant la lecture.\n", stderr);
		return 1;
	}
	*fill=0;
	return 0;
}

int decIDXmain(FILE* reader, struct node* T, unsigned char* carrier, int* fill, int size)
{
	int counter=0, code_read=0;
	unsigned char node='\0', character='\0'; //Bytes used to hold decoded bit or character
	*carrier=fgetc(reader);
	if(ferror(reader)) {
		fputs("Erreur durant la lecture.\n", stderr);
		return 1;
	}
	while(counter < size) {
		node=decodeSingle(carrier,fill);	//Node code, single bit (0 or 1)
		if(*fill == CHAR_BIT) {
			if(readChar(reader,carrier,fill)) {
				return 2;
			}
		}
		if(node && !(node & (node-1))) { //Power of two (i.e single bit) check
			while(code_read < CHAR_BIT) {
				T[counter].symbol=decodeIDX(carrier,fill,&character,&code_read);
				if(*fill == CHAR_BIT) {
					if(readChar(reader,carrier,fill)) {
						return 3;
					}
				}
			}
			code_read=0;
		}
		counter++;
	}
	return 0;
}

void buildTree(struct node* T, int size)
{
	int counter=0, nodes_next=1, nodes_work, nodes_work_fix;
	while(counter < size) {
		nodes_work_fix=nodes_next;
		nodes_work=0;
		nodes_next=0;
		while(nodes_work < nodes_work_fix) {
			if(T[counter].symbol == 0) {
				T[counter].child_left=counter+nodes_next+(nodes_work_fix-nodes_work);
				T[T[counter].child_left].parent=counter;
				nodes_next++;
				T[counter].child_right=counter+nodes_next+(nodes_work_fix-nodes_work);
				T[T[counter].child_right].parent=counter;
				nodes_next++;
				nodes_work++;
			}
			else {
				nodes_work++;
			}
			counter++;
		}
	}
}

unsigned char* extractCode(struct node* T, int i)
{
	int j, k;
	unsigned char* code=malloc((UCHAR_MAX+1)*sizeof(unsigned char));
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
	if(code[0] == '\0') {
		code[1]='\0';
		code[0]='1';
	}
	//Optimization: reduce string size
	unsigned char* codeOpt=realloc(code,(strlen(code)+1)*sizeof(unsigned char));
	//No need to free(code), realloc frees code itself
	//"Causes double free or corruption (out)" error and crash
	if(!codeOpt)
		free(code);
	else
		code=NULL;
	return codeOpt;	//NULL pointer if realloc failed
}

int deCodeGen(struct node* T, unsigned char** Table, int size)
{
	for(int i=0; i < size; i++) {	//Leaves throughout tree, unlike in encoder tree
		if(T[i].symbol != 0) {
			Table[i]=extractCode(T,i);
			if(!Table[i]) {
				fputs("Erreur: mémoire insuffisante.\n", stderr);
				return 1;
			}
		}
	}
	return 0;
}

char* newFile(char* oldFilename)
{
	char* new_name=malloc((strlen(oldFilename)+9)*sizeof(char));
	char* prefix="DECODED_"; //String literal is a constant, cannot be directly concatenated
	strcpy(new_name,prefix);
	return strcat(new_name,oldFilename);
}

int decodeMSG(unsigned char* carrier, struct node* T, int* fill, int* pos)
{
	while(*fill < CHAR_BIT) {
		if(T[*pos].symbol > 0) {
			return T[*pos].symbol;
		}
		else {
			if(*carrier & (1<<(CHAR_BIT-1-*fill))) {
				(*pos)=T[*pos].child_right;
			}
			else {
				(*pos)=T[*pos].child_left;
			}
			(*fill)++;
		}
	}
	return -1;
}

int decMSGmain(FILE* reader, FILE* writer, struct node* Tree, unsigned char* carrier, unsigned char Opt, int* fill, int total_char)
{
	int counter=0, temp_pos=0, char_found=-1;
	while(counter++ < total_char) {
		while(char_found == -1) {
			char_found=decodeMSG(carrier,Tree,fill,&temp_pos);
			if(*fill == CHAR_BIT) {
				if(readChar(reader,carrier,fill)) {
					return 2;
				}
			}
		}
		fputc(char_found,writer);
		if(Opt & (1<<3)) {
			putchar(char_found);
		}
		if(ferror(writer)) {
			fputs("Erreur durant l'écriture.\n", stderr);
			return 3;
		}
		temp_pos=0;
		char_found=-1;
	}
	return 0;
}