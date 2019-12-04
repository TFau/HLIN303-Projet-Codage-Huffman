#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Dec_Functions.h"


int main(int argc, char** argv)
{
	if(argc < 2) {
		fputs("Erreur: le fichier à lire et à décompresser doit être passé en paramètre.\n", stderr);
		return 1;
	}
	unsigned char optByte;
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
	puts("Décodage...");
	printf("%d caractères.\n%d caractères distincts.\n\n", sum, nonzero_count);
	int treesize=2*nonzero_count-1;
	struct node Tree[treesize];
	initTree(Tree,treesize); //Tree initialization (no frequencies)

	/* Decode index and rebuild tree */
	int fill=0;
	unsigned char CarrierByte;
	unsigned char buftree[treesize+nonzero_count]; //Leaves + symbols + internal nodes, each to a byte
	if(decIDXmain(huff,buftree,&CarrierByte,&fill,treesize,nonzero_count)) //0 on success, 1 or more in case of failure
		return 4; //Error message printed by decIDXmain
	//Extract nodes from buffer into tree
	arraytoTree(buftree,Tree,treesize,nonzero_count);
	//Rebuild tree
	buildTree(Tree,treesize);
	puts("Arbre reconstruit...\n");

	/* Code regeneration */
	unsigned char* DecodeTable[treesize];
	for(int i=0; i < treesize; i++)
		DecodeTable[i]=NULL; //Otherwise free() causes segfault
	//Generates the code for each character by traversing the tree
	if(deCodeGen(Tree,DecodeTable,treesize)) //0 on success, 1 in case of failed realloc
		return 5; //Error message printed by codeGen
	puts("Codes régénérés...\n");
	//Print the code array if -c option used
	if(argc == 3 && (optByte & (1<<2))) {
		for(size_t i=0; i < treesize; i++) {
			if(Tree[i].symbol != 0) {
				printf("%ld. %c\t%s\n", i, Tree[i].symbol, DecodeTable[i]);
			}
		}
		puts("");
	}
	else puts("Utilisez l'option -c pour afficher les caractères et leurs codes respectifs.\n");

	/* Decode message */
	FILE* huffwrite=tmpfile();
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 6;
	}
	//CarrierByte and fill are NOT reset; start from last written bit of current carrier byte
	if(decMSGmain(huff,huffwrite,DecodeTable,Tree,&CarrierByte,&fill,treesize,sum)) //0 on success, 1 or more in case of failure
		return 7; //Error message printed by decMSGmain
	for(int i=0; i < treesize; i++)
		free(DecodeTable[i]);

	/* Write the decoded message */
	fclose(huff);
	if(decodeWrite(huffwrite,huff,argv[1])) //0 on success, 1 in case of failure
		return 8; //Error message printed by decodeWrite
	fclose(huffwrite);

	puts("Message décodé...\n");
	puts("Décompression terminée avec succès.\n###################################");

	return 0;
}