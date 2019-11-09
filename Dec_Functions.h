#ifndef DEC_FUNCTIONS_SET
#define DEC_FUNCTIONS_SET

struct node 
{
	char symbol;
	int parent;
	int child_left;
	int child_right;
	//No frequency attribute in decoder
};

bool oned(unsigned char);

unsigned char* extractCode(struct node*, int);

unsigned char decodeSingle(unsigned char, int*);

void decodeIDX(unsigned char, int*, int*, unsigned char*, int*);

unsigned char stringBuild(unsigned char*, int);

int codeCheck(unsigned char*, int, unsigned char**, int);

int decodeMSG(unsigned char, int*, unsigned char*, int*, unsigned char**, int);

#endif