#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "Com_Functions.h"

int freqCalc(int* T, char* textfile)
{
	for(int i=0; i <= UCHAR_MAX; i++)
		T[i]=0;
	FILE* huff=fopen(textfile, "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 1;
	}
	unsigned int counter;	//int needed for EOF
	/* the fgetc function obtains that character as an unsigned char converted to
	an int and advances the associated file position indicator (C11 standard) */
	while((counter=fgetc(huff)) != EOF) {
		//"undechaque" file prints 239,191,189 repeatedly: UTF8 3-byte character �
		T[counter]++;
	}
	if(ferror(huff)) {
		puts("Erreur durant la lecture.");
		return 2;
	}
	else if(feof(huff))
		puts("Fichier lu.");
	fclose(huff);
	return 0;
}

int distinctCalc(int* T, char* textfile, int* unique_char, int* total_char)
{
	for(int i=0; i <= UCHAR_MAX; i++) {
		if(T[i] != 0) {
			(*unique_char)++;
			*total_char+=T[i];
		}
	}
	//Adds a newline if the file contains only one character
	if(*unique_char == 1) {
		T[10]++;
		(*unique_char)++;
		(*total_char)++;
		//Reopens file and adds newline at the end
		FILE* huff=fopen(textfile, "w");
		if(!huff) {
			perror("Echec de la lecture");
			return 1;
		}
		fseek(huff,(long)*total_char,SEEK_SET);	//SEEK_SET offsets from the beginning of the file
		fputc('\n',huff);
		fclose(huff);
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
		T[i].freq=0;
	}
}

void freqTree(struct node* T, int* Table, int unique_char, int total_char)
{
	int outer=0, inner=0;
	while(outer < unique_char) {
		while(inner <= UCHAR_MAX) {
			if(Table[inner] != 0) {
				T[outer].symbol=(unsigned char)inner;
				T[outer].freq=(double)Table[inner]/total_char;
				outer++;
			}
			inner++;
		}
	}
}

void buildTree(struct node* T, int counter)
{
	int minA, minB;
	double lowfreqA, lowfreqB;
	do {
		minA=-1; minB=-1;
		lowfreqA=1; lowfreqB=1;
		for(int i=0; i <= counter; i++) {
			if(T[i].parent == -1 && T[i].freq > 0 && T[i].freq < lowfreqA) {
				lowfreqA=T[i].freq;
				minA=i;
			}
		}
		for(int i=0; i <= counter; i++) {
			if(T[i].parent == -1 && T[i].freq > 0 && i != minA && T[i].freq < lowfreqB) {
				lowfreqB=T[i].freq;
				minB=i;
			}
		}
		counter++;
		if(minA != -1 && minB != -1) {
			T[counter].freq=lowfreqA+lowfreqB;
			T[minA].parent=counter;
			T[minB].parent=counter;
			T[counter].child_left=minA;
			T[counter].child_right=minB;
		}
	}
	while(minA != -1 && minB != -1);
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

int codeGen(struct node* T, unsigned char** Table, int unique_char)
{
	int counter=0;
	for(int i=0; i < unique_char; i++) {	//All leaves
		counter=T[i].symbol;	//To get numerical value
		Table[counter]=extractCode(T,i);
		if(Table[counter] == NULL) {
			puts("Erreur: mémoire insuffisante.");
			return 1;
		}
	}
	return 0;
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

unsigned char* binaryChar(unsigned char n)
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

void encode(unsigned char* carrier, int* fill, unsigned char* code, int* code_read)
{
	int size=strlen(code)-*code_read;	//Code to write, without null terminator
	//8 encodeable bits
	while(size > 0 && *fill < CHAR_BIT) {	//While there is still code to write and the carrier byte isn't full
		//Starts reading code array after already written code
		if(code[*code_read] & 1) {	//'1' is coded as 49, odd number with least significant bit 1
			*carrier|=(1<<(CHAR_BIT-1-*fill));
		}
		else {
			*carrier&=(~(1<<(CHAR_BIT-1-*fill)));
		}
		size--;
		(*fill)++;	//Without parentheses, ++ increments the pointer itself (operator has higher priority than *)
		(*code_read)++;
	}
}

int writeChar(FILE* writer, unsigned char* carrier, int* fill, int* bits)
{
	fputc(*carrier,writer);
	if(ferror(writer)) {
		puts("Erreur durant l'écriture.");
		return 1;
	}
	*bits+=CHAR_BIT;
	*fill=0;
	return 0;
}

int encodeIDX(FILE* writer, struct node* Tr, int size, unsigned char* carrier, int* fill, int* bits, int unique_char, int total_char)
{
	int code_read=0;
	unsigned int buf0[1]={total_char};	//4 byte storage
	*bits+=32+CHAR_BIT;
	unsigned char buf1[2]={'0','\0'}; //1 random byte and null character--null character needed for proper strlen calc in encoding function
	unsigned char *symbol; //Char array to hold 8-bit char as 8-char array and null terminator
	fwrite(buf0,sizeof(buf0),1,writer); //Write total number of characters
	if(ferror(writer)) {
		puts("Erreur durant l'écriture.");
		return 1;
	}
	fputc(unique_char,writer); //Write number of distinct characters (0-255)
	if(ferror(writer)) {
		puts("Erreur durant l'écriture.");
		return 2;
	}
	//Tree is encoded by depth level, from top to bottom, and from left to right on each level
	int counter=size; //Total nodes in tree
	int search=size-1, child_pos=1, nodes_next=1, nodes_work;
	int T[size];	//Auxiliary array for node ordering
	for(int i=0; i < size-1; i++)
		T[i]=-1;
	T[size-1]=0;	//Root
	while(counter > 0) {
		nodes_work=nodes_next;		//Nodes to work on depending on depth, minus leaves' non-children from previous level
		nodes_next=0;
		search=size-1;
		while(search >= 0 && nodes_work > 0) {
			if(Tr[search].parent == -1 && Tr[search].freq != 0 && leftmost(T,size,search)) {
				if(Tr[search].child_left == -1 && Tr[search].child_right == -1) {		//Leaf
					buf1[0]='1';
					encode(carrier,fill,buf1,&code_read); //Sets bit to 1
					if(*fill == CHAR_BIT) {
						if(writeChar(writer,carrier,fill,bits)) {
							return 3;	//Error message printed by writeChar
						}
					}
					code_read=0; //Only 1 bit set, doesn't need comparison with code length
					symbol=binaryChar(Tr[search].symbol); //Converts 1-byte char into 8-byte char array of the char's binary value
					while(code_read < CHAR_BIT) {
						encode(carrier,fill,symbol,&code_read); //Bit-encodes the previous array onto the carrier byte
						if(*fill == CHAR_BIT) {
							if(writeChar(writer,carrier,fill,bits)) {
								return 4;
							}
						}
					}
					free(symbol);
					code_read=0;
				}
				else {	//Internal node
					buf1[0]='0';
					encode(carrier,fill,buf1,&code_read); //Sets bit to 0
					if(*fill == CHAR_BIT) {
						if(writeChar(writer,carrier,fill,bits)) {
							return 5;
						}
					}
					code_read=0; //Only 1 bit set
					//"Deleting" node
					Tr[Tr[search].child_left].parent=-1;
					T[Tr[search].child_left]=child_pos;	//Leftmost, smaller number in auxiliary table
					child_pos++;
					nodes_next++;
					Tr[Tr[search].child_right].parent=-1;
					T[Tr[search].child_right]=child_pos;
					child_pos++;
					nodes_next++;
				}
				Tr[search].freq=0;	//Neutralizes node in if condition, even with parent==-1
				T[search]=-1; //Neutralizes node in auxiliary table, will not be considered by leftmost algorithm
				nodes_work--;	//One less node to work on at this depth
				counter--;	//One less node to work on overall
				search=size-1;	//Reset search to recheck parentless nodes now that current one has been worked on
			}
			search--;	//Loop counter
		}
	}
	return 0;
}

int encodeMSG(FILE* writer, FILE* reader, unsigned char** Table, unsigned char* carrier, int* fill, int* bits, int total_char)
{
	int counter=0, code_read=0;
	*bits=0-*fill;
	int length;
	unsigned char Xchar='\0';
	unsigned char* buftemp;
	while(counter < total_char) {	//total_char==total characters
		Xchar=fgetc(reader);
		if(ferror(reader)) {
			puts("Erreur durant la lecture.");
			return 1;
		}
		buftemp=malloc((strlen(Table[Xchar])+1)*sizeof(unsigned char));	//Allocate size of code+nullchar
		strcpy(buftemp,Table[Xchar]);	//strcpy copies the nullchar
		length=strlen(buftemp);
		while(code_read < length) {
			encode(carrier,fill,buftemp,&code_read);
			if(*fill == CHAR_BIT) {
				if(writeChar(writer,carrier,fill,bits)) {
					return 2;	//Error message printed by writeChar
				}
			}
		}
		code_read=0;
		counter++;
		free(buftemp);
	}
	return 0;
}

int encodeWrite(FILE* temp, FILE* writer, char* textfile)
{
	rewind(temp); //Changes to read mode in tmpfile
	writer=fopen(textfile, "wb");
	if(!writer) {
		perror("Echec de l'écriture");
		return 1;
	}
	//Must use a special int buffer and fread/fwrite to properly transfer int value
	unsigned int buf0[1];
	fread(buf0,sizeof(buf0),1,temp);
	fwrite(buf0,sizeof(buf0),1,writer);
	int counter;
	while((counter=fgetc(temp)) != EOF) {
		fputc(counter,writer);
		if(ferror(writer)) {
			puts("Erreur durant l'écriture.");
			return 2;
		}
	}
	fclose(writer);
	return 0;
}