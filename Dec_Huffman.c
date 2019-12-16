/*!
 * \file Dec_Huffman.c
 * \brief Programme principal du décompresseur Huffman
 * \author Troy Fau
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dec_Functions.h"


int main(int argc, char** argv)
{
	if(argc < 2) {
		fputs("Erreur: le fichier à lire et à décompresser doit être passé en paramètre.\n", stderr);
		return 1;
	}
	unsigned char optByte='\0';
	if(argc == 3)
		optByte=atoi(argv[2]);

	FILE* huff=fopen(argv[1], "rb");
	if(!huff) {
		perror("Echec de la lecture");
		return 2;
	}
	
	/* Get message size */
	int sum, nonzero_count;
	if(readStart(huff,&sum,&nonzero_count)) //Returns 0 on success, 1 in case of failure
		return 3; //Error message printed in readStart
	if(!(optByte & (1<<4))) {	//Do not print when compressing/decompressing concatenation file when archiving interrupted
		puts("Décodage...");
		printf("%d caractères.\n%d caractères distincts.\n\n", sum, nonzero_count);
	}
	int treesize=2*nonzero_count-1;
	struct node Tree[treesize];
	initTree(Tree,treesize); //Tree initialization (no frequencies)

	/* Decode index and rebuild tree */
	int fill=0;
	unsigned char CarrierByte;
	//Decode characters
	if(decIDXmain(huff,Tree,&CarrierByte,&fill,treesize)) //0 on success, 1 or more in case of failure
		return 4; //Error message printed by decIDXmain
	//Rebuild tree
	buildTree(Tree,treesize);
	if(!(optByte & (1<<4)))
		puts("Arbre reconstruit...\n");

	/* Code regeneration with -c option */
	if(optByte & (1<<2)) {
		unsigned char* DecodeTable[treesize];
		for(int i=0; i < treesize; i++) {
			DecodeTable[i]=NULL; //Otherwise free() causes segfault
		}
		//Generates the code for each character by traversing the tree
		if(deCodeGen(Tree,DecodeTable,treesize)) { //0 on success, 1 in case of failed realloc
			return 5; //Error message printed by codeGen
		}
		if(!(optByte & (1<<4)))
			puts("Codes régénérés...\n");
		//Print the code array
		for(size_t i=0; i < treesize; i++) {
			if(Tree[i].symbol != 0) {
				printf("%ld. %c\t%s\n", i, Tree[i].symbol, DecodeTable[i]);
			}
		}
		for(int i=0; i < treesize; i++) {
			free(DecodeTable[i]);
		}
		puts("");
	}
	else if(!(optByte & (1<<4)))
		puts("Utilisez l'option -c pour afficher les caractères et leurs codes respectifs.\n");

	/* Decode message */
	char* filename=newFile(argv[1]);
	FILE* huffwrite=fopen(filename, "w");
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 6;
	}
	//CarrierByte and fill are NOT reset; start from last written bit of current carrier byte
	if(decMSGmain(huff,huffwrite,Tree,&CarrierByte,optByte,&fill,sum)) //0 on success, 1 or more in case of failure
		return 7; //Error message printed by decMSGmain
	if(optByte & (1<<3))
		puts("");

	fclose(huff);
	fclose(huffwrite);
	free(filename);

	if(!(optByte & (1<<4))) {
		puts("Message décodé...\n");
		puts("Décompression terminée avec succès.\n###################################");
	}

	return 0;
}