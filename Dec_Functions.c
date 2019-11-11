#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Dec_Functions.h"

void initTree(struct node* T, int size)
{
	for(int i = 0; i < size; i++) {
		T[i].symbol=-1;
		T[i].parent=-1;
		T[i].child_left=-1;
		T[i].child_right=-1;
	}
}

int readStart(FILE* reader, int* total_char, int* unique_char)
{
	unsigned int buf[1]; //4 byte storage
	fread(buf,sizeof(buf),1,reader);
	*total_char=buf[0];
	if(ferror(reader)) {
		puts("Erreur durant la lecture.");
		return 1;
	}
	*unique_char=fgetc(reader);
	if(ferror(reader)) {
		puts("Erreur durant la lecture.");
		return 2;
	}
	return 0;
}

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

unsigned char decodeSingle(unsigned char* carrier, int* fill)
{
	unsigned char decode='\0';
	if(*carrier & 1<<(7-*fill))	//if 1 is read at the position
		decode|=(1<<0); //decode set to 00000001
	//else decode is still a nullchar
	(*fill)++;
	return decode;
}

void decodeIDX(unsigned char* carrier, int* fill, int* code_read, unsigned char* T, int* pos)
{
	while(*code_read < 8 && *fill < 8) {	//To the end of the symbol byte or to the end of the carrier byte
		if(*carrier & (1<<(7-*fill))) {
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

int readChar(FILE* reader, unsigned char* carrier, int* fill)
{
	*carrier=fgetc(reader);
	if(ferror(reader)) {
		puts("Erreur durant la lecture.");
		return 1;
	}
	*fill=0;
	return 0;
}

int decIDXmain(FILE* reader, unsigned char* treeArray, unsigned char* carrier, int* fill, int size, int unique_char)
{
	int counter=0, temp_pos=0, code_read=0;
	unsigned char buftemp[256]; //Char array to hold bits for decoding
	//The decoder must extract the node and symbol bits, and place each node or symbol into a byte of the buffer for easier processing
	*carrier=fgetc(reader);
	if(ferror(reader)) {
		puts("Erreur durant la lecture.");
		return 1;
	}
	while(counter < size+unique_char) {
		treeArray[counter]=decodeSingle(carrier,fill);	//Node code, single bit (0 or 1)
		if(*fill == 8) {
			if(readChar(reader,carrier,fill)) {
				return 2;
			}
		}
		if(oned(treeArray[counter])) {	//Leaf code
			while(code_read < 8) {
				//Temp array for piece of character byte
				decodeIDX(carrier,fill,&code_read,buftemp,&temp_pos);
				if(*fill == 8) {
					if(readChar(reader,carrier,fill)) {
						return 3;
					}
				}
			}
			code_read=0;
			counter++; //To place leaf character 1 cell after the leaf code
			treeArray[counter]=stringBuild(buftemp,temp_pos);
		}
		temp_pos=0;
		counter++;
	}
	return 0;
}

void arraytoTree(unsigned char* treeArray, struct node* T, int size, int unique_char)
{
	int counter=0, nodes_count=0;
	while(counter < size+unique_char) {
		if(treeArray[counter] == '\0') {
			nodes_count++;
		}
		else if(oned(treeArray[counter])) {
			T[nodes_count].symbol=treeArray[counter+1];
			nodes_count++;
		}
		counter++;
	}
}

void buildTree(struct node* T, int size)
{
	int counter=0, nodes_next=1, nodes_work, nodes_work_fix;
	while(counter < size) {
		nodes_work_fix=nodes_next;
		nodes_work=0;
		nodes_next=0;
		while(nodes_work < nodes_work_fix) {
			if(T[counter].symbol == -1) {
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

int deCodeGen(struct node* T, unsigned char** Table, int size)
{
	for(int i=0; i < size; i++) {	//Leaves throughout tree, unlike in encoder tree
		if(T[i].symbol != -1) {
			Table[i]=extractCode(T,i);
			if(Table[i] == NULL) {
				puts("Erreur: mémoire insuffisante.");
				return 1;
			}
		}
	}
	return 0;
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

int decodeMSG(unsigned char* carrier, int* fill, unsigned char* T, int* pos, unsigned char** Dec_T, int size)
{
	//No code_read variable: no advance knowledge of the length of the code
	int character;
	while(*fill < 8) {
		if(*carrier & (1<<(7-*fill))) {
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

int decMSGmain(FILE* reader, FILE* writer, unsigned char** Table, struct node* T, unsigned char* carrier, int* fill, int size, int total_char)
{
	int counter=0, temp_pos=0, char_found=-1;
	unsigned char buftemp[256];
	while(counter < total_char) {
		while(char_found == -1) {
			char_found=decodeMSG(carrier,fill,buftemp,&temp_pos,Table,size);
			if(*fill == 8) {
				if(readChar(reader,carrier,fill)) {
					return 2;
				}
			}
		}
		fputc(T[char_found].symbol,writer);
		if(ferror(writer)) {
			puts("Erreur durant l'écriture.");
			return 3;
		}
		counter++;
		temp_pos=0;
		char_found=-1;
	}
	return 0;
}

int decodeWrite(FILE* temp, FILE* writer, char* textfile)
{
	rewind(temp); //Changes to read mode in tmpfile
	writer=fopen(textfile, "w");
	if(!writer) {
		perror("Echec de la lecture");
		return 1;
	}
	int counter;
	while((counter=fgetc(temp)) != EOF) {
		fputc(counter,writer);
		if(ferror(writer)) {
			puts("Erreur durant l'écriture.");
			return 2;
		}
	}
	return 0;
}