/*!
 * \file Dec_Functions.c
 * \brief Fonctions du programme de décompression
 * \author Troy Fau
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Dec_Functions.h"

/*!
* \brief Fonction de récupération du début de l'index
*
* Récupère le nombre total de caractères sur le flux reader et stocke cette valeur 
* dans total_char, et récupère le nombre de caractères distincts, stocké dans unique_char.
* \param[in] reader: flux de lecture.
* \param[in] total_char: variable du main stockant le nombre total de caractères.
* \param[in] unique_char: variable du main stockant le nombre de caractères distincts.
* \return 0 si la lecture s'est déroulée correctement, 1 ou 2 sinon.
*/
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

/*!
* \brief Fonction d'initialisation du tableau pour le stockage de l'arbre à des valeurs par défaut
*
* \param[in] T: tableau de type struct node, structure dont les attributs permettent de stocker les paramètres de l'arbre, sans l'attribut de fréquence.
* \param[in] size: taille du tableau T.
*/
void initTree(struct node* T, int size)
{
	for(int i = 0; i < size; i++) {
		T[i].symbol=0;
		T[i].parent=-1;
		T[i].child_left=-1;
		T[i].child_right=-1;
	}
}

/*!
* \brief Fonction de décodage d'un bit
*
* Sous-fonction de decIDXmain.
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \return Un entier non signé sur un octet de valeur 0 ou 1.
*/
unsigned char decodeSingle(unsigned char* carrier, int* fill)
{
	unsigned char decode='\0';
	if(*carrier & 1<<(CHAR_BIT-1-*fill))	//if 1 is read at the position
		decode|=(1<<0); //decode set to 0x01
	//else decode is still a nullchar
	(*fill)++;
	return decode;
}

/*!
* \brief Fonction de décodage d'un caractère de l'index
*
* Sous-fonction de decIDXmain. Lit les bits de carrier et les écrit sur T_byte pour 
* retrouver le caractère associé à une feuille. Renvoie T_byte lorsque carrier a été 
* lu en entier ou T_byte est rempli.
* \param[in] carrier: variable du main stockant les bits encodés à décoder.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] T_byte: variable du main stockant le caractère décodé.
* \param[in] code_read: variable du main stockant la quantité de code déjà lu.
* \return Un entier sur un octet non signé correspondant au caractère associé à une feuille.
*/
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

/*!
* \brief Fonction de lecture du fichier compressé
*
* Sous-fonction decIDXmain et decMSGmain. L'octet lu sur le flux reader est stocké 
* dans carrier et fill est remis à zero.
* \param[in] reader: flux de lecture.
* \param[in] carrier: variable du main stockant les bits encodés à décoder.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \return 0 si la lecture s'est déroulée correctement, 1 sinon.
*/
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

/*!
* \brief Fonction de décodage de l'index
*
* Fonction de décodage de l'index. Les octets lus sur le flux reader sont chargés sur 
* l'octet porteur carrier. Lorsque tous les bits de carrier ont été lus, la fonction 
* appelle readChar pour récupérer un nouvel octet sur reader. La fonction appelle 
* decodeSingle pour décoder les noeuds de l'arbre. La lecture du bit 1 indique une feuille, 
* et la fonction appelle decodeIDX pour lire les CHAR_BIT bits suivants et retrouver le 
* caractère associé à la feuille et le stocker dans le tableau de l'arbre de Huffman.
* \param[in] reader: flux de lecture.
* \param[in] T: tableau stockant l'arbre.
* \param[in] carrier: variable du main stockant les bits encodés à décoder.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] size: taille de l'arbre.
* \return 0 si la lecture s'est déroulée correctement, 1, 2 ou 3 sinon.
*/
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

/*!
* \brief Fonction de reconstruction de l'arbre de Huffman
*
* Les arbres de Huffman ne sont pas des arbres complets et donc il n'y a pas une formule unique 
* pour relier parents et enfants. Mais sachant que l'arbre a été encodé de haut en bas et de gauche 
* à droite, à partir d'un niveau de profondeur, on peut connaître le nombre de noeuds à traiter au 
* niveau suivant: 2 pour chaque noeud interne au niveau courant. En tenant compte des noeuds à traiter 
* à un certain niveau, des noeuds déjà traités et des noeuds aux niveau suivant, la fonction calcule 
* et alloue les enfants de chaque noeud interne et ainsi reconstitue l'arbre.
* \param[in] T: tableau stockant l'arbre.
* \param[in] size: taille de l'arbre.
*/
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

/*!
* \brief Fonction de génération de code pour une feuille
*
* Sous-fonction de codeGen. Une chaîne de caractères vide est créée. A partir de la 
* feuille i de l'arbre stocké dans T, l'algorithme remonte jusqu'à la racine. A chaque 
* déplacement d'un enfant vers un parent, si l'enfant était à gauche le caractère '0' 
* est ajouté à la chaîne, sinon le caractère '1' est ajouté. Lorsque la racine a été 
* atteinte, la chaîne est réallouée dans une chaîne de taille adaptée et renvoyée. Attribue 
* le code 1 par défaut si la feuille est racine, dans le cas où le fichier ne contient 
* qu'un caractère distinct.
* \param[in] T: tableau stockant l'arbre.
* \param[in] i: indice du tableau correspondant au caractère dont on génère le code.
* \return La chaîne de caractère du code correspondant au caractère i.
*/
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

/*!
* \brief Fonction de génération des codes
*
* Fonction de génération des codes pour chaque caractère de T, parcourant la totalité de l'arbre.
* \param[in] T: tableau stockant l'arbre.
* \param[in] Table: tableau de chaînes de caractères stockant les codes.
* \param[in] size: taille de l'arbre.
* \return 0 si la génération des codes a fonctionné correctement, 1 si une erreur
* d'allocation mémoire est survenue.
*/
int deCodeGen(struct node* T, unsigned char** Table, int size)
{
	for(int i=0; i < size; i++) {	//Leaves throughout tree, unlike in encoder tree
		if(T[i].symbol) {
			Table[i]=extractCode(T,i);
			if(!Table[i]) {
				fputs("Erreur: mémoire insuffisante.\n", stderr);
				return 1;
			}
		}
	}
	return 0;
}

/*!
* \brief Fonction de baptême du fichier décompressé
*
* \param[in] oldFilename: nom du fichier d'origine.
* \return Le nouveau nom du fichier.
*/
char* newFile(char* oldFilename)
{
	char* new_name=malloc((strlen(oldFilename)+9)*sizeof(char));
	char* prefix="DECODED_"; //String literal is a constant, cannot be directly concatenated
	strcpy(new_name,prefix);
	return strcat(new_name,oldFilename);
}

/*!
* \brief Fonction de décodage d'un caractère du message
*
* Sous-fonction de decMSGmain. Lis les bits de carrier tant que fill est inférieur à CHAR_BIT 
* et parcourt l'arbre de Huffman stocké dans T en fonction des bits lus, en utilisant pos comme 
* indice. Si une feuille est rencontrée avant la fin de la lecture de carrier, le caractère 
* associé est renvoyé, sinon la fonction retourne -1.
* \param[in] carrier: variable du main stockant les bits encodés à décoder.
* \param[in] T: tableau stockant l'arbre.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] pos: variable du main stockant la position dans l'arbre entre itérations de la fonction.
* \return Si une feuille de l'arbre de Huffman a été atteinte par le parcours de la fonction,
* renvoie le caractère associé à cette feuille, sinon renvoie -1.
*/
int decodeMSG(unsigned char* carrier, struct node* T, int* fill, int* pos)
{
	while(*fill < CHAR_BIT) {
		if(T[*pos].symbol) {
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

/*!
* \brief Fonction de décodage du message
*
* Fonction de décodage du message. A la lecture d'un caractère sur le flux reader, appelle 
* la fonction decodeMSG en lui passant l'octer carrier pour parcourir l'arbre de Huffman 
* stocké dans Tree et retrouver les caractères encodés. Lorsque tous les bits de carrier 
* ont été lu, appelle readChar pour récuperer un nouvel octet de reader. Lorsque decodeMSG 
* renvoie une valeur positive, la fonction l'écrit sur le flux writer. Si l'option -p a été 
* sélectionnée, le caractère est également affiché sur stdout.
* \param[in] reader: flux de lecture.
* \param[in] writer: flux d'écriture sur le fichier décompressé.
* \param[in] Tree: tableau stockant l'arbre.
* \param[in] carrier: variable du main stockant les bits encodés à décoder.
* \param[in] Opt: octet encodant les options sélectionnés par l'utilisateur.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] total_char: nombre total de caractères.
* \return 0 si la lecture s'est déroulée correctement, 1 ou 2 sinon.
*/
int decMSGmain(FILE* reader, FILE* writer, struct node* Tree, unsigned char* carrier, unsigned char Opt, int* fill, int total_char)
{
	int counter=0, temp_pos=0, char_found=-1;
	while(counter++ < total_char) {
		while(char_found == -1) {
			char_found=decodeMSG(carrier,Tree,fill,&temp_pos);
			if(*fill == CHAR_BIT) {
				if(readChar(reader,carrier,fill)) {
					return 1;
				}
			}
		}
		fputc(char_found,writer);
		if(Opt & (1<<3)) {
			putchar(char_found);
		}
		if(ferror(writer)) {
			fputs("Erreur durant l'écriture.\n", stderr);
			return 2;
		}
		temp_pos=0;
		char_found=-1;
	}
	return 0;
}