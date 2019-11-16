#ifndef DEC_FUNCTIONS_SET
#define DEC_FUNCTIONS_SET

#include <stdbool.h>

struct node 
{
	unsigned char symbol;
	int parent;
	int child_left;
	int child_right;
	//No frequency attribute in decoder
};

int readStart(FILE*, int*, int*);

void initTree(struct node*, int);

bool oned(unsigned char);

unsigned char decodeSingle(unsigned char*, int*);

void decodeIDX(unsigned char*, int*, int*, unsigned char*, int*);

unsigned char stringBuild(unsigned char*, int);

int readChar(FILE*, unsigned char*, int*);

int decIDXmain(FILE*, unsigned char*, unsigned char*, int*, int, int);

void arraytoTree(unsigned char*, struct node*, int, int);

void buildTree(struct node*, int);

unsigned char* extractCode(struct node*, int); //Also in Com_Functions.h

int deCodeGen(struct node*, unsigned char**, int); //Not the same as Com_Functions.h codeGen

int codeCheck(unsigned char*, int, unsigned char**, int);

int decodeMSG(unsigned char*, int*, unsigned char*, int*, unsigned char**, int);

int decMSGmain(FILE*, FILE*, unsigned char**, struct node*, unsigned char*, int*, int, int);

int decodeWrite(FILE*, FILE*, char*);

#endif