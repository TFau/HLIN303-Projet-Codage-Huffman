#ifndef DEC_FUNCTIONS_SET
#define DEC_FUNCTIONS_SET

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

unsigned char decodeSingle(unsigned char*, int*);

unsigned char decodeIDX(unsigned char*, int*, unsigned char*, int*);

int readChar(FILE*, unsigned char*, int*);

int decIDXmain(FILE*, struct node*, unsigned char*, int*, int);

void buildTree(struct node*, int);

unsigned char* extractCode(struct node*, int); //Also in Com_Functions.h

int deCodeGen(struct node*, unsigned char**, int); //Not the same as Com_Functions.h codeGen

char* newFile(char*); //Prefixes file name with "DECODED_"

int codeCheck(unsigned char*, int, unsigned char**, int);

int decodeMSG(unsigned char*, int*, unsigned char*, int*, unsigned char**, int);

int decMSGmain(FILE*, FILE*, unsigned char**, struct node*, unsigned char*, unsigned char, int*, int, int);

#endif