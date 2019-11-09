#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Com_Functions.h"

int main(int argc, char** argv)
{
	if(argc != 2) {
		printf("Erreur: le fichier à lire et compresser doit être passé en paramètre.\n");
		return 1;
	}

	/* Frequency Calc Function */
	int ProcTable[256]; //Extended ASCII
	//File character parsing
	freqCalc(ProcTable,argv[1]);
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

	/* Tree construction */
	int treesize=2*nonzero_count-1;
	printf("%d noeuds dans l'arbre.\n", treesize);
	//Tree node array. Max possible size 511 (2*256 - 1).
	struct node Tree[treesize];
	initTree(Tree,treesize); //Tree initialization with base values
	freqTree(Tree,ProcTable,nonzero_count,sum); //Set character frequencies
	puts("Arbre initialisé...");
	int counter=nonzero_count-1;
	buildTree(Tree,counter); //Linking tree nodes via lowest frequencies
	puts("Arbre construit...");
	puts("");

	/* Code generation */
	unsigned char* CodeTable[256];	//To position characters at index values equal to their numerical values, for easier message encoding later
	for(int i=0; i < 256; i++)
		CodeTable[i]=NULL;	//Otherwise free() causes segfault
	codeGen(Tree,CodeTable,nonzero_count); //Generates the code for each character by traversing the tree
	puts("Codes générés...");
	puts("");
	//Print the code array
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
	unsigned char CarrierByte='\0';
	int fill=0, bits=0;
	//When carrier is full, write it to the file with fputc, reset carrier fill to 0 and set next code bits
	//When code fully read, reset code_read to 0 and read next character
	//When code only partially read and carrier full, set the remainder of the code onto the next carrier

	/* Index size
	For n  distinct characters, 2n-1 bits for the tree nodes, 8n bits for the characters themselves,
	32 bits for the total characters in the message, 8 bits for the number of distinct characters.
	Total: 10n + 39 bits, 1.25n + 4.875 bytes */

	/* Encode the tree */
	FILE* huffwrite;
	huffwrite=fopen("huff_compressed.txt", "wb");
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 2;
	}
	encodeIDX(huffwrite,Tree,treesize,&CarrierByte,&fill,&bits,nonzero_count,sum);
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
	FILE* huff=fopen(argv[1], "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 3;
	}
	//CarrierByte and fill are NOT reset: start from last written bit of current carrier byte
	encodeMSG(huffwrite,huff,CodeTable,&CarrierByte,&fill,&bits,sum);
	//End of message
	if(fill) {
		fputc(CarrierByte,huffwrite);
		bits+=8;	//All 8 bits sent, even if the last part of the code is only written on the most significant ones
	}
	fclose(huffwrite);
	fclose(huff);
	for(int i=0; i < 256; i++)
		free(CodeTable[i]);

	puts("Message codé...");
	printf("%d bits.\n", bits);
	puts("");
	printf("Total: %d bits.\n", index_b+bits);
	printf("Réduction taille: %.3lf%\n", 100.0-(double)(index_b+bits)/(sum*8)*100);
	puts("");
	puts("Compression terminée avec succès.");
	puts("#################################");

	return 0;
}