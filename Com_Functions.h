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

unsigned char* extractCode(struct node*, int);

bool leftmost(int*, int, int);

unsigned char* binarychar(char);

unsigned char encode(unsigned char, int*, unsigned char*, int*);

#endif

