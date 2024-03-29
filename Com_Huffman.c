/*!
* \file Com_Huffman.c
* \brief Programme principal du compresseur Huffman
*
*Le compresseur lit le fichier et compte le nombre d'occurrences de chaque caractère; à partir de cette information se calculent le 
*nombre de caractères distincts et le nombre total de caractères dans le fichier.
*
*Pour n caractères distincts, le programme crée un tableau de 2n-1 cases qui représentera l'arbre de Huffman: chaque caractère y est 
*inséré comme feuille, sous la forme d'une variable de type struct dont les attributs identifient le caractère par sa valeur numérique, 
*son parent, et sa fréquence dans le texte, ratio du nombre d'occurrences sur le nombre total de caractères.
*
*A partir de ces caractères-feuilles, un algorithme génère l'arbre de Huffman, liant à chaque itération les deux noeuds de plus basses 
*fréquences à un noeud parent dont la fréquence est la somme de celles de ces enfants. Une fois remplie, un second algorithme génère 
*le code de chaque caractère en remontant de sa feuille jusqu'à la racine. Ces codes sont stockés dans un
*second tableau.
*
*Si l'option -c a été sélectionné à l'exécution du script Python, les caractères et leurs codes respectifs sont affichés.
*
*Commence alors la phase de compression proprement dite, dans un nouveau fichier. L'encodage se fait bit par bit sur un octet, qui est 
*écrit dans le fichier lorsque ses CHAR_BIT (défini par l'implémentation, mais typiquement 8) bits ont été fixés. L'écriture reprend 
*ensuite sur le même octet.
*
*Avant l'encodage du fichier sont encodés: un entier sur 4 octets contenant le nombre total de caractères dans le texte, un entier sur 1 
*octet contenant le nombre de caractères distincts, puis l'arbre lui-même, sous la forme d'un bit par noeud et feuille, et pour chaque 
*feuille les CHAR_BIT bits du caractère associé.
*
*Après encodage de l'index, le contenu du fichier est encodé. Le compresseur récupère le code de chaque caractère lu dans le fichier 
*d'origine et le retranscrit par la méthode décrite plus haut dans un nouveau fichier. Lorsque tous les caractères ont été lus et 
*encodés, le programme se termine.
* \author Troy Fau
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Com_Functions.h"


int main(int argc, char** argv)
{
	if(argc < 2) {
		fputs("Erreur: le fichier à lire et à compresser doit être passé en paramètre.\n", stderr); //writes to stdout and adds a newline, fputs does not
		return 1;
	}
	unsigned char optByte='\0';
	if(argc == 3)
		optByte=atoi(argv[2]);

	/* Frequency Calc Function */
	int ProcTable[UCHAR_MAX+1];
	//File character parsing
	if(freqCalc(ProcTable,argv[1],optByte)) //Returns 0 on success, 1 in case of failed realloc
		return 2;
	//Distinct character count and total character count
	int nonzero_count=0, sum=0;
	distinctCalc(ProcTable,&nonzero_count,&sum);
	printf("%d caractères: %d bits.\n%d caractères distincts.\n\n", sum, sum*8, nonzero_count);

	/* Tree construction */
	int treesize=2*nonzero_count-1;
	printf("%d noeuds dans l'arbre.\n", treesize);
	//Tree node array. Max possible size 511 (2*256 - 1).
	struct node Tree[treesize];
	initTree(Tree,treesize); //Tree initialization with base values
	freqTree(Tree,ProcTable,sum); //Set character frequencies
	int counter=nonzero_count-1;
	buildTree(Tree,counter); //Linking tree nodes via lowest frequencies
	puts("Arbre initialisé...\nArbre construit...\n");

	/* Code generation */
	unsigned char* CodeTable[UCHAR_MAX+1];	//To position characters at index values equal to their numerical values, for easier message encoding later
	for(int i=0; i <= UCHAR_MAX; i++)
		CodeTable[i]=NULL;	//Otherwise free() causes segfault
	//Generates the code for each character by traversing the tree
	if(codeGen(Tree,CodeTable,nonzero_count)) //0 on success, 1 in case of failed realloc
		return 3; //Error message printed by codeGen
	puts("Codes générés...\n");
	//Print the code array if -c option used
	if(optByte & (1<<2)) {
		counter=0; //Reuse
		for(int i=0; i <= UCHAR_MAX; i++) {
			if(Tree[counter].symbol == i) {
				printf("%d. %c\t%s\n", i, Tree[counter].symbol, CodeTable[i]);
				counter++;
			}
		}
		puts("");
	}
	else
		puts("Utilisez l'option -c pour afficher les caractères et leurs codes respectifs.\n");

	//////////////////
	/* Compression */
	/////////////////
	unsigned char CarrierByte='\0';
	int fill=0, bits=0;
	//When the carrier is full, write it to the file with fputc, reset carrier fill to 0 and set next code bits
	//When the code is fully read, reset code_read to 0 and read next character
	//When the code is only partially read and the carrier is full, set the remainder of the code onto the next carrier

	/* Index size
	For n  distinct characters, 2n-1 bits for the tree nodes, 8n bits for the characters themselves,
	32 bits for the total characters in the message, 8 bits for the number of distinct characters.
	Total: 10n + 39 bits, 1.25n + 4.875 bytes */

	/* Encode the tree */
	char* filename=newFile(argv[1]);
	FILE* huffwrite=fopen(filename, "wb");
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 4;
	}
	if(encodeIDX(huffwrite,Tree,treesize,&CarrierByte,&fill,&bits,nonzero_count,sum)) //0 on success, 1 or more in case of failure
		return 5; //Error message printed by encodeIDX
	/* The carrier with the last part of the index may not be full, and may also hold some of the coded message itself. This is not
	a problem as long as it is properly written to the file. The decoder will be able to read the carrier up to the last bit of the index,
	stop reading, rebuild the tree and generate the codes, then resume reading. */
	if(fill)
		bits+=fill;
	int index_b=bits;
		printf("Arbre codé...\n%d bits.\n\n", bits);

	/* Encode the message */
	FILE* huff=fopen(argv[1], "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 6;
	}
	//CarrierByte and fill are NOT reset; start from last written bit of current carrier byte
	if(encodeMSG(huffwrite,huff,CodeTable,&CarrierByte,&fill,&bits,sum)) //0 on success, 1 or more in case of failure
		return 7; //Error message printed by encodeMSG
	//End of message
	if(fill) {
		fputc(CarrierByte,huffwrite);
		bits+=CHAR_BIT;	//All 8 bits sent, even if the last part of the code is only written on the most significant ones
	}
	fclose(huff);
	fclose(huffwrite);
	for(int i=0; i <= UCHAR_MAX; i++)
		free(CodeTable[i]);
	free(filename);

		printf("Message codé...\n%d bits.\n%.3f bits par caractère.\n\n", bits, (float)bits/sum);
		printf("Total: %d bits, soit %d octets.\n", index_b+bits, (index_b+bits)/8);
		printf("Réduction taille: %.3f%\n\n", 100.0-(float)(index_b+bits)/(sum*8)*100);
		puts("Compression terminée avec succès.\n#################################");

	return 0;
}