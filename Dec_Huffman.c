/*!
* \file Dec_Huffman.c
* \brief Programme principal du décompresseur Huffman
*
* Le programme de decompression est lancé par le script Python, soit à la suite de la compression si l'utilisateur 
* le choisit, soit à la lecture d'un fichier binaire passé en argument.
*
* Le décompresseur ouvre le fichier encodé en lecture binaire et lit tout d'abord l'entier sur 4 octets qui indique 
* le nombre total de caractères, et l'entier sur 1 octet indiquant le nombre de caractères distincts. Ces informations 
* lui permettent de recréer un tableau représentant l'arbre de Huffman du texte encodé.
*
* L'index est décodé et les caractères récupérés sont mis à leur place dans le tableau. Sachant que l'arbre a été 
* encodé en commençant par la racine et en procédant dans un ordre particulier (cf. partie II), le décompresseur 
* reconstruit alors l'arbre en reconstituant les liens parents-enfants d'origine.
*
* Après avoir reconstitué l'arbre, si l'option -c a été sélectionné, le décompresseur régénère les codes pour les afficher.
*
* Le programme s'attaque alors à la décompression du message. L'octet porteur reçoit l'octet lu à partir du fichier 
* encodé, et les bits du porteur sont lus et utilisés pour parcourir l'arbre à partir de sa racine. Lorsque le parcours 
* abouti à une feuille, le caractère associé est alors écrit dans le fichier décodé.
*
* Lorsque le décompresseur a décodé un nombre de caractères égal au nombre total de caractères indiqué par l'index, le 
* programme s'arrête.
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
	puts("Décodage...");
	printf("%d caractères.\n%d caractères distincts.\n\n", sum, nonzero_count);
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
	else
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

	puts("Message décodé...\n");
	puts("Décompression terminée avec succès.\n###################################");

	return 0;
}