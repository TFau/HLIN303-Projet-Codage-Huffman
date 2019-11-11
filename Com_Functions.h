#ifndef COM_FUNCTIONS_SET
#define COM_FUNCTIONS_SET

#include <stdbool.h>

struct node 
{
	char symbol;
	int parent;
	int child_left;
	int child_right;
	double freq;
};

int freqCalc(int*, char*);

void initTree(struct node*, int);

void freqTree(struct node*, int*, int, int);

void buildTree(struct node*, int);

unsigned char* extractCode(struct node*, int); //Also in Dec_Functions.h

int codeGen(struct node*, unsigned char**, int); //Also in Dec_Functions.h

bool leftmost(int*, int, int);

unsigned char* binaryChar(char);

void encode(unsigned char*, int*, unsigned char*, int*);

int writeChar(FILE*, unsigned char*, int*, int*);

int encodeIDX(FILE*, struct node*, int, unsigned char*, int*, int*, int, int);

int encodeMSG(FILE*, FILE*, unsigned char**, unsigned char*, int*, int*, int);

int encodeWrite(FILE*, FILE*, char*);

#endif

