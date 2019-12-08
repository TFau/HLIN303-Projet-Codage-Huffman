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

int freqCalc(int*, char*);

int distinctCalc(int*, char*, int*, int*);

void initTree(struct node*, int);

void freqTree(struct node*, int*, int);

void buildTree(struct node*, int);

unsigned char* extractCode(struct node*, int); //Also in Dec_Functions.h

int codeGen(struct node*, unsigned char**, int); //Not the same as Dec_Functions.h deCodeGen

char* newFilename(char*); //Prefixes file name with "ENCODED_"

bool leftmost(int*, int, int);

unsigned char* binaryChar(unsigned char);

void encode(unsigned char*, int*, unsigned char*, int*);

int writeChar(FILE*, unsigned char*, int*, int*);

int encodeIDX(FILE*, struct node*, int, unsigned char*, int*, int*, int, int);

int encodeMSG(FILE*, FILE*, unsigned char**, unsigned char*, int*, int*, int);

#endif

