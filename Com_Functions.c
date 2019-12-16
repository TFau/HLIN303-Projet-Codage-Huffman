/*!
 * \file Com_Functions.c
 * \brief Fonctions du programme de compression
 * \author Troy Fau
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Com_Functions.h"


/*! 
* \brief Fonction de lecture du fichier texte comptant le nombre d'occurrences de chaque caractère
*
* La fonction ouvre le fichier textfile en lecture. A la lecture d'un caractère, la valeur de la case du
* tableau d'occurrences T correspondant au caractère est incrémenté. Si l'option -p a été sélectionnée,
* affiche le caractère lu sur stdout.
* \param[in] T: tableau stockant les occurrences des caractères.
* \param[in] textfile: nom du fichier à lire.
* \param[in] Opt: byte d'options.
*/
int freqCalc(int* T, char* textfile, unsigned char Opt)
{
	for(int i=0; i <= UCHAR_MAX; i++)
		T[i]=0;
	FILE* huff=fopen(textfile, "r");
	if(!huff) {
		perror("Echec de la lecture");
		return 1;
	}
	unsigned int counter;	//int needed for EOF
	/* "the fgetc function obtains that character as an unsigned char converted to
	an int and advances the associated file position indicator" (C11 standard) */
	while((counter=fgetc(huff)) != EOF) {
		T[counter]++; //"undechaque" file prints 239,191,189 repeatedly: UTF8 3-byte character �
		if(Opt & (1<<3)) {
			putchar(counter);
		}
	}
	if(ferror(huff)) {
		fputs("Erreur durant la lecture.\n", stderr);
		return 2;
	}
	else if(feof(huff)) {
		if(Opt & (1<<3)) {
			puts("");
		}
		if(!(Opt & (1<<4))) {
			puts("Fichier lu.");
		}
	}
	fclose(huff);
	return 0;
}

/*!
* \brief Fonction de comptage du nombre total de caractères et du nombre de caractères distincts
*
* \param[in] T: tableau stockant les occurrences des caractères.
* \param[in] unique_char: variable du main stockant le nombre de caractères distincts.
* \param[in] total_char: variable du main stockant le nombre total de caractères.
*/
void distinctCalc(int* T, int* unique_char, int* total_char)
{
	for(int i=0; i <= UCHAR_MAX; i++) {
		if(T[i]) {
			(*unique_char)++;
			*total_char+=T[i];
		}
	}
}

/*!
* \brief Fonction d'initialisation du tableau pour le stockage de l'arbre à des valeurs par défaut
*
* \param[in] T: tableau de type struct node, structure dont les attributs permettent de stocker les paramètres de l'arbre.
* \param[in] size: taille du tableau T.
*/
void initTree(struct node* T, int size)
{
	for(int i=0; i < size; i++) {
		T[i].symbol=0;
		T[i].parent=-1;
		T[i].child_left=-1;
		T[i].child_right=-1;
		T[i].freq=0;
	}
}

/*!
* \brief Fonction de calcul des fréquences d'apparition des caractères
*
* \param[in] T: tableau stockant l'arbre.
* \param[in] Table: tableau stockant les occurrences des caractères.
* \param[in] total_char: nombre total de caractères.
*/
void freqTree(struct node* T, int* Table, int total_char)
{
	int j=0;
	for(int i=0; i <= UCHAR_MAX; i++) {
		if(Table[i]) {
			T[j].symbol=(unsigned char)i;
			T[j].freq=(double)Table[i]/total_char;
			j++;
		}
	}
}

/*!
* \brief Fonction de construction de l'arbre de Huffman
*
* L'algorithme parcourt les noeuds existants. Les deux noeuds de plus basses
* fréquences sont liés à un parent, placé dans une case vide de T à partir de l'indice
* correspondant au nombre de caractères distincts. La fréquence du parent est la somme
* des fréquences de ces deux enfants. Chaque nouvelle itération prendra en compte le 
* nouveau noeud ainsi créé. Lorsqu'on ne trouve plus deux noeuds n'ayant pas de parent, 
* l'algorithme s'arrête.
* \param[in] T: tableau stockant l'arbre.
* \param[in] counter: variable de comptage.
*/
void buildTree(struct node* T, int counter)
{
	int minA, minB;
	double lowfreqA, lowfreqB;
	do {
		minA=minB=-1;
		lowfreqA=lowfreqB=1;
		for(int i=0; i <= counter; i++) {
			if(T[i].parent == -1 && T[i].freq < lowfreqA) {
				lowfreqA=T[i].freq;
				minA=i;
			}
		}
		for(int i=0; i <= counter; i++) {
			if(T[i].parent == -1 && i != minA && T[i].freq < lowfreqB) {
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
* Fonction de génération des codes pour chaque caractère de T, avec un nombre d'appels 
* à extractCode égal à unique_char.
* \param[in] T: tableau stockant l'arbre.
* \param[in] Table: tableau de chaînes de caractères stockant les codes.
* \param[in] unique_char: nombre de caractères distincts.
*/
int codeGen(struct node* T, unsigned char** Table, int unique_char)
{
	int counter=0;
	for(int i=0; i < unique_char; i++) {	//All leaves
		counter=T[i].symbol;	//To get numerical value
		Table[counter]=extractCode(T,i);
		if(!Table[counter]) {
			fputs("Erreur: mémoire insuffisante.\n", stderr);
			return 1;
		}
	}
	return 0;
}

/*!
* \brief Fonction de baptême du fichier compressé
*
* \param[in] oldFilename: nom du fichier d'origine.
*/
char* newFile(char* oldFilename)
{
	char* new_name=malloc((strlen(oldFilename)+9)*sizeof(char));
	char* prefix="ENCODED_";
	strcpy(new_name,prefix);
	return strcat(new_name,oldFilename);
}

/*!
* \brief Fonction de test de noeud
*
* Sous-fonction de encodeIDX. Teste la valeur du noeud n dans T en la 
* comparant à toutes les autres valeurs positives. Si cette valeur est 
* la plus petite de T, n est le noeud le plus à gauche au niveau de 
* profondeur actuel de l'arbre dans encodeIDX, et la fonction renvoie vrai. 
* Sinon la fonction renvoie faux.
* \param[in] T: tableau auxiliaire stockant des valeurs constamment incrémentées pour chaque enfant d'un noeud traité.
* \param[in] size: taille de T.
* \param[in] n: noeud à tester, correspondant à un indice de T.
*/
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

/*!
* \brief Fonction d'encodage
*
* Sous-fonction de encodeIDX et encodeMSG. Lit la chaîne de caractères du code et encode 
* l'octet porteur en fonction du caractère lu. L'encodage s'arrête lorsque carrier est 
* rempli ou lorsqu'il n'y a plus de code à écrire.
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] code: code à encoder.
* \param[in] code_read: variable du main stockant la quantité de code déjà écrit.
*/
void encode(unsigned char* carrier, int* fill, unsigned char* code, int* code_read)
{
	int size=strlen(code)-*code_read;	//Code to write, without null terminator
	//8 encodeable bits
	while(size-- > 0 && *fill < CHAR_BIT) {	//While there is still code to write and the carrier byte isn't full
		//Starts reading code array after already written code
		if(code[*code_read] & 1) {	//'1' is coded as 49, odd number with least significant bit 1
			*carrier|=(1<<(CHAR_BIT-1-*fill));
		}
		else {
			*carrier&=(~(1<<(CHAR_BIT-1-*fill)));
		}
		(*fill)++;	//Without parentheses, ++ increments the pointer itself (operator has higher priority than *)
		(*code_read)++;
	}
}

/*!
* \brief Fonction d'encodage des caractères de l'index
*
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] symbol: caractère à encoder.
* \param[in] code_read: variable du main stockant la quantité de code déjà écrit.
*/
void encodeCh(unsigned char* carrier, int* fill, unsigned char* symbol, int* code_read)
{
	while(*code_read < CHAR_BIT && *fill < CHAR_BIT) {	//To the end of the symbol byte or to the end of the carrier byte
		if(*symbol & (1<<(CHAR_BIT-1-*code_read))) {
			*carrier|=(1<<(CHAR_BIT-1-*fill));
		}
		else {
			*carrier&=(~(1<<(CHAR_BIT-1-*fill)));
		}
		(*code_read)++;
		(*fill)++;
	}
}

/*!
* \brief Fonction d'écriture sur le fichier compressé
*
* Sous-fonction de encodeIDX et encodeMSG. L'octet carrier est écrit sur le 
* flux writer, fill est remis à zero et bits est incrémenté de CHAR_BIT.
* \param[in] writer: flux d'écriture.
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] bits: variable du main stockant le nombre total de bits écrits.
*/
int writeChar(FILE* writer, unsigned char* carrier, int* fill, int* bits)
{
	fputc(*carrier,writer);
	if(ferror(writer)) {
		fputs("Erreur durant l'écriture.\n", stderr);
		return 1;
	}
	*bits+=CHAR_BIT;
	*fill=0;
	return 0;
}

/*!
* \brief Fonction d'encodage de l'index
*
* Envoie tout d'abord le nombre total de caractères sur un entier de 4 octets, 
* puis le nombre de caractères distincts sur 1 octet. Un tableau auxiliaire 
* est créé à l'usage de la fonction leftmost, avec toutes ses valeurs initialisées 
* à -1 sauf la racine de Tr à 0. L'algorithme part de la racine et encode les 
* noeuds de l'arbre par niveau de profondeur, et de gauche à droite. Les noeuds sont 
* encodés sur un bit par la fonction encode: 0 pour un noeud interne, 1 pour une 
* feuille. Après le bit d'une feuille sont encodés les CHAR_BIT bits du caractère 
* correspondant par appel à la fonction encodeCh. Lorsque fill=CHAR_BIT, l'octet 
* carrier est rempli et la fonction appelle writeChar pour écrire sur le flux writer.
* \param[in] writer: flux d'écriture sur le fichier compressé.
* \param[in] Tr: tableau stockant l'arbre.
* \param[in] size: taille de l'arbre.
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] bits: variable du main stockant le nombre total de bits écrits.
* \param[in] unique_char: nombre de caractères distincts.
* \param[in] total_char: nombre total de caractères.
*/
int encodeIDX(FILE* writer, struct node* Tr, int size, unsigned char* carrier, int* fill, int* bits, int unique_char, int total_char)
{
	int code_read=0;
	unsigned int buf0[1]={total_char};	//4 byte storage
	*bits+=32+CHAR_BIT;
	unsigned char buf1[2]={'0','\0'}; //1 random byte and null character--null character needed for proper strlen calc in encoding function
	unsigned char leaf_char='\0'; //Character to encode after a leaf code
	fwrite(buf0,sizeof(buf0),1,writer); //Write total number of characters
	if(ferror(writer)) {
		fputs("Erreur durant l'écriture.\n", stderr);
		return 1;
	}
	fputc(unique_char,writer); //Write number of distinct characters (0-255)
	if(ferror(writer)) {
		fputs("Erreur durant l'écriture.\n", stderr);
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
					leaf_char=Tr[search].symbol;
					while(code_read < CHAR_BIT) {
						encodeCh(carrier,fill,&leaf_char,&code_read); //Bit-encodes the leaf character onto the carrier byte
						if(*fill == CHAR_BIT) {
							if(writeChar(writer,carrier,fill,bits)) {
								return 4;
							}
						}
					}
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
					Tr[Tr[search].child_right].parent=-1;
					T[Tr[search].child_right]=child_pos;
					child_pos++;
					nodes_next+=2;
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

/*!
* \brief Fonction d'encodage du message
*
* \param[in] writer: flux d'écriture sur le fichier compressé.
* \param[in] reader: flux de lecture du fichier d'origine.
* \param[in] Table: tableau de chaînes de caractères stockant les codes.
* \param[in] carrier: variable du main stockant les bits encodés à écrire.
* \param[in] fill: variable du main stockant le taux de remplissage de carrier.
* \param[in] bits: variable du main stockant le nombre total de bits écrits.
* \param[in] total_char: nombre total de caractères.
*/
int encodeMSG(FILE* writer, FILE* reader, unsigned char** Table, unsigned char* carrier, int* fill, int* bits, int total_char)
{
	int counter=0, code_read=0;
	*bits=0-*fill;
	int length;
	unsigned char Xchar='\0';
	unsigned char* buftemp;
	while(counter++ < total_char) {	//total_char==total characters
		Xchar=fgetc(reader);
		if(ferror(reader)) {
			fputs("Erreur durant la lecture.\n", stderr);
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
		free(buftemp); buftemp=NULL;
	}
	return 0;
}