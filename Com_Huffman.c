#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Com_Functions.h"

int main(int argc, char** argv)
{
	////////////////////////////////
	/* Probability Calc Function */
	///////////////////////////////
	//Extended ASCII
	int ProcTable[256];
	for(size_t i=0; i < 256; i++)
		ProcTable[i]=0;

	FILE* huff=fopen("huff_file1.txt", "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 1;
	}
	int counter;	//int needed for EOF
	while((counter=fgetc(huff)) != EOF)		//fgetc equivalent to getc
		ProcTable[counter]++;
	if(ferror(huff)) {
		puts("Erreur de lecture.");		//writes to stdout and adds a newline to the string, fputs does not
		return 2;
	}
	else if(feof(huff))
		puts("Fichier lu.");
	fclose(huff);

	//Distinct character count and total character count
	int nonzero_count=0;
	int sum=0;
	for(int i=0; i < 256; i++) {
		if(ProcTable[i] != 0) {
			nonzero_count++;
			sum+=ProcTable[i];
		}
	}
	printf("%d caractères: %d bits.\n", sum, sum*8);
	printf("%d caractères distincts.\n", nonzero_count);
	puts("");

	////////////////////////
	/* Tree construction */
	///////////////////////
	int treesize=2*nonzero_count-1;
	printf("%d noeuds dans l'arbre.\n", treesize);
	//Tree node array. Max possible size 511 (2*256 - 1).
	struct node Tree[treesize];
	//Initialization with base values
	for(size_t i = 0; i < treesize; i++) {
		Tree[i].symbol=-1;
		Tree[i].parent=-1;
		Tree[i].child_left=-1;
		Tree[i].child_right=-1;
		Tree[i].freq=0;
	}
	//Initialization with single character frequencies
	int outer=0, inner=0;
	while(outer < nonzero_count) {
		while(inner < 256) {
			if(ProcTable[inner] != 0) {
				Tree[outer].symbol=(char)inner;
				Tree[outer].freq=(double)ProcTable[inner]/sum;
				outer++;
			}
			inner++;
		}
	}
	puts("Arbre initialisé...");
	counter=nonzero_count-1; //Reuse
	int minA, minB;
	double lowfreqA, lowfreqB;
	do {
		minA=-1; minB=-1;
		lowfreqA=1; lowfreqB=1;
		for(size_t i=0; i <= counter; i++) {
			if(Tree[i].parent == -1 && Tree[i].freq > 0 && Tree[i].freq < lowfreqA) {
				lowfreqA=Tree[i].freq;
				minA=i;
			}
		}
		for(size_t i=0; i <= counter; i++) {
			if(Tree[i].parent == -1 && Tree[i].freq > 0 && Tree[i].freq != lowfreqA && Tree[i].freq < lowfreqB) {
				lowfreqB=Tree[i].freq;
				minB=i;
			}
		}
		counter++;
		if(minA != -1 && minB != -1) {
			Tree[counter].freq=lowfreqA+lowfreqB;
			Tree[minA].parent=counter;
			Tree[minB].parent=counter;
			Tree[counter].child_left=minA;
			Tree[counter].child_right=minB;
		}
	}
	while(minA != -1 && minB != -1);
	puts("Arbre construit...");
	puts("");

	//////////////////////
	/* Code generation */
	/////////////////////
	unsigned char* CodeTable[256];	//To position characters at index values equal to their numerical values, for easier message encoding later
	for(int i=0; i < 256; i++)
		CodeTable[i]=NULL;	//Otherwise free() causes segfault
	counter=0; //Reuse
	for(int i=0; i < nonzero_count; i++) {	//All leaves
		counter=Tree[i].symbol;	//To get numerical value
		CodeTable[counter]=extractCode(Tree,i);
		if(CodeTable[counter] == NULL) {
			puts("Erreur: mémoire insuffisante.");
			return 3;
		}
	}
	//Print the code array
	puts("Codes générés...");
	puts("");
	counter=0; //Reuse
	for(int i=0; i < 256; i++) {
		if(Tree[counter].symbol == i) {
			printf("%d. %c\t%s\n", i, Tree[counter].symbol, CodeTable[i]);
			counter++;
		}
	}
	puts("");

	//////////////////
	/* Compression */
	/////////////////
	/* Byte encoding */
	unsigned char CarrierByte='\0';
	int fill=0; //Send pointer (&fill) in function calls
	int code_read=0; //Send pointer (&code_read) in function calls
	int bits=0;
	//When carrier is full, load it into char buffer and fwrite buffer, reset carrier fill to 0 and write next code bits
	//When code fully read, reset code_read to 0 and read next character
	//When code only partially read and carrier full, write the remainder of the code onto the next carrier

	/* Index size
	For n  distinct characters, 2n-1 bits for the tree nodes, 8n bits for the characters themselves, 32 bits for the total characters in the message,
	8 bits for the number of distinct characters. Total: 10n + 39 bits, 1.25n + 4.875 bytes */

	/* Encode the tree */
	FILE* huffwrite;
	huffwrite=fopen("huff_compressed.txt", "wb");
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 4;
	}
	unsigned int buf0[1]={sum};	//4 byte storage
	unsigned char buf1[1]={nonzero_count};	//1 byte storage !NO NULLCHAR!
	bits+=40;
	unsigned char buf2[2]={'0','\0'}; //1 random byte and null character--null character needed for proper strlen calc in encoding function
	unsigned char Xchar='\0';
	unsigned char *symbol; //Char array to hold 8-bit char as 8-char array and null terminator
	fwrite(buf0,sizeof(buf0),1,huffwrite); //Write total number of characters
	fwrite(buf1,sizeof(buf1),1,huffwrite); //Write number of distinct characters (0-255)
	//Tree is encoded by depth level, from top to bottom, and from left to right on each level
	counter=treesize; //Reuse
	int search=treesize-1, child_pos=1, nodes_next=1, nodes_work;
	int T[treesize];	//Auxiliary array for node ordering
	for(int i=0; i < treesize-1; i++)
		T[i]=-1;
	T[treesize-1]=0;	//Root
	while(counter > 0) {
		nodes_work=nodes_next;		//Nodes to work on depending on depth, minus leaves' non-children from previous level
		nodes_next=0;
		search=treesize-1;
		while(search >= 0 && nodes_work > 0) {
			if(Tree[search].parent == -1 && Tree[search].freq != 0 && leftmost(T,treesize,search)) {
				buf1[0]='\0';
				if(Tree[search].child_left == -1 && Tree[search].child_right == -1) {		//Leaf
					buf2[0]='1';
					CarrierByte=encode(CarrierByte,&fill,buf2,&code_read); //Sets bit to 1
					if(fill == 8) {
						Xchar=CarrierByte; //Loads carrier into buffer
						fputc(Xchar,huffwrite); //Writes buffer to file
						bits+=8;
						fill=0;
					}
					code_read=0; //Only 1 bit set, doesn't need comparison with code length
					symbol=binarychar(Tree[search].symbol); //Converts 1-byte char into 8-byte char array of the char's binary value
					while(code_read < 8) {
						CarrierByte=encode(CarrierByte,&fill,symbol,&code_read); //Bit-encodes the previous array onto the carrier byte
						if(fill == 8) {
							Xchar=CarrierByte;
							fputc(Xchar,huffwrite); //Writes symbol
							bits+=8;
							fill=0;
						}
					}
					free(symbol);
					code_read=0;
				}
				else {	//Internal node
					buf2[0]='0';
					CarrierByte=encode(CarrierByte,&fill,buf2,&code_read); //Sets bit to 0
					if(fill == 8) {
						Xchar=CarrierByte;
						fputc(Xchar,huffwrite); //Writes nullchar
						bits+=8;
						fill=0;
					}
					code_read=0; //Only 1 bit set
					//"Deleting" node
					Tree[Tree[search].child_left].parent=-1;
					T[Tree[search].child_left]=child_pos;	//Leftmost, smaller number in auxiliary table
					child_pos++;
					nodes_next++;
					Tree[Tree[search].child_right].parent=-1;
					T[Tree[search].child_right]=child_pos;
					child_pos++;
					nodes_next++;
				}
				Tree[search].freq=0;	//Neutralizes node in if condition, even with parent==-1
				T[search]=-1; //Neutralizes node in auxiliary table, will not be considered by leftmost algorithm
				nodes_work--;	//One less node to work on at this depth
				counter--;	//One less node to work on overall
				search=treesize-1;	//Reset search to recheck parentless nodes now that current one has been worked on
			}
			search--;	//Loop counter
		}
	}
	/* The carrier with the last part of the index may not be full, and may also hold some of the coded message itself. This is not
	a problem as long as it is properly written to the file. The decoder will be able to read the carrier up to the last bit of the index,
	stop reading, rebuild the tree and generate the codes, then resume reading. */
	if(fill)
		bits+=fill;
	int index_b=bits;
	puts("Arbre codé...");
	printf("%d bits.\n", bits);
	puts("");

	/* Encode the message */
	huff=fopen("huff_file1.txt", "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 5;
	}
	counter=0; //Reuse
	//CarrierByte and fill are NOT reset: start from last written bit of current carrier byte
	code_read=0; bits=0-fill;
	int length;
	unsigned char* enc_buftemp;
	while(counter < sum) {	//sum=total characters
		Xchar=fgetc(huff);	//Reads sizeof(buf2)-1 characters, i.e. 1 character
		enc_buftemp=malloc((strlen(CodeTable[Xchar])+1)*sizeof(unsigned char));	//Allocate size of code+nullchar
		strcpy(enc_buftemp,CodeTable[Xchar]);	//strcpy copies the nullchar
		length=strlen(enc_buftemp);
		while(code_read < length) {
			CarrierByte=encode(CarrierByte,&fill,enc_buftemp,&code_read);
			if(fill == 8) {
				fputc(CarrierByte,huffwrite);
				bits+=8;
				fill=0;
			}
		}
		code_read=0;
		counter++;
		free(enc_buftemp);
	}
	//End of message
	if(fill) {
		fputc(CarrierByte,huffwrite);
		bits+=8;	//All 8 bits sent, even if last part of code is only written on the most significant ones
		fill=0;
	}
	for(int i=0; i < 256; i++)
		free(CodeTable[i]);
	puts("Message codé...");
	printf("%d bits.\n", bits);
	puts("");
	printf("Total: %d bits.\n", index_b+bits);
	printf("Compression: %.3lf%\n", 100.0-(double)(index_b+bits)/(sum*8)*100);
	puts("");
	fclose(huffwrite);
	fclose(huff);

	return 0;
}