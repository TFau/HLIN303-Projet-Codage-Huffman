/*!
 * \file Com_Functions.h
 * \brief En-tête des fonctions du programme de compression
 * \author Troy Fau
 */

#ifndef COM_FUNCTIONS_SET
#define COM_FUNCTIONS_SET

#include <stdbool.h>

struct node 
{
	unsigned char symbol;
	int parent;
	int child_left;
	int child_right;
	double freq;
};

int freqCalc(int*, char*, unsigned char);

void distinctCalc(int*, int*, int*);

void initTree(struct node*, int);

void freqTree(struct node*, int*, int);

void buildTree(struct node*, int);

unsigned char* extractCode(struct node*, int); //Also in Dec_Functions.h

int codeGen(struct node*, unsigned char**, int); //Not the same as Dec_Functions.h deCodeGen

char* newFile(char*); //Prefixes file name with "ENCODED_"

bool leftmost(int*, int, int);

void encode(unsigned char*, int*, unsigned char*, int*);

void encodeCh(unsigned char*, int*, unsigned char*, int*);

int writeChar(FILE*, unsigned char*, int*, int*);

int encodeIDX(FILE*, struct node*, int, unsigned char*, int*, int*, int, int);

int encodeMSG(FILE*, FILE*, unsigned char**, unsigned char*, int*, int*, int);

#endif

