#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Dec_Functions.h"

int main(int argc, char** argv)
{
	FILE* huff=fopen("huff_compressed.txt", "rb");
	if(!huff) {
		perror("Echec de la lecture");
		return 1;
	}
	unsigned int buf0[1]; //4 byte storage
	unsigned char buf1[1]; //1 byte storage !NO NULLCHAR!
	fread(buf0,sizeof(buf0),1,huff); //Get total characters
	int de_total=buf0[0];
	puts("Décodage...");
	printf("%d caractères.\n", de_total);
	fread(buf1,sizeof(buf1),1,huff); //Get distinct characters
	int de_char=buf1[0];
	printf("%d caractères distincts.\n", de_char);
	puts("");
	int de_treesize=2*de_char-1;
	struct node DeTree[de_treesize];
	for(int i=0; i < de_treesize; i++) {
		DeTree[i].symbol=-1;
		DeTree[i].parent=-1;
		DeTree[i].child_left=-1;
		DeTree[i].child_right=-1;
	}

	/* Decode index */
	int de_fill=0, temp_pos=0, de_code_read=0, dec_count=0;
	unsigned char buftree[de_treesize+de_char]; //Leaves + symbols + internal nodes, each to a byte
	unsigned char buftemp[256]; //Char array to hold bits for decoding
	//The decoder must extract the node and symbol bits, and place each node or symbol into a byte of the buffer for easier processing
	unsigned char Xchar=fgetc(huff);
	unsigned char CarrierByte=Xchar;
	while(dec_count < de_treesize+de_char) {
		buftree[dec_count]=decodeSingle(CarrierByte,&de_fill);	//Node code, single bit (0 or 1)
		if(de_fill == 8) {
			Xchar=fgetc(huff);
			CarrierByte=Xchar;
			de_fill=0;
		}
		if(oned(buftree[dec_count])) {	//Leaf code
			while(de_code_read < 8) {
				//Temp array for piece of character byte
				decodeIDX(CarrierByte,&de_fill,&de_code_read,buftemp,&temp_pos);
				if(de_fill == 8) {
					Xchar=fgetc(huff);
					CarrierByte=Xchar;
					de_fill=0;
				}
			}
			de_code_read=0;
			dec_count++; //To place leaf character 1 cell after the leaf code
			buftree[dec_count]=stringBuild(buftemp,temp_pos);
		}
		temp_pos=0;
		dec_count++;
	}

	//Extract nodes from buffer into tree
	int de_nodes_count=0;
	dec_count=0;
	while(dec_count < de_treesize+de_char) {
		if(buftree[dec_count] == '\0') {
			de_nodes_count++;
		}
		else if(oned(buftree[dec_count])) {
			DeTree[de_nodes_count].symbol=buftree[dec_count+1];
			de_nodes_count++;
		}
		dec_count++;
	}
	dec_count=0;
	int de_nodes_next=1, de_nodes_work, de_nodes_work_fix;
	//Rebuild tree
	while(dec_count < de_treesize) {
		de_nodes_work_fix=de_nodes_next;
		de_nodes_work=0;
		de_nodes_next=0;
		while(de_nodes_work < de_nodes_work_fix) {
			if(DeTree[dec_count].symbol == -1) {
				DeTree[dec_count].child_left=dec_count+de_nodes_next+(de_nodes_work_fix-de_nodes_work);
				DeTree[DeTree[dec_count].child_left].parent=dec_count;
				de_nodes_next++;
				DeTree[dec_count].child_right=dec_count+de_nodes_next+(de_nodes_work_fix-de_nodes_work);
				DeTree[DeTree[dec_count].child_right].parent=dec_count;
				de_nodes_next++;
				de_nodes_work++;
			}
			else {
				de_nodes_work++;
			}
			dec_count++;
		}
	}
	puts("Arbre reconstruit...");
	puts("");

	/* Generate codes from decoded tree */
	unsigned char* DecodeTable[de_treesize];
	for(int i=0; i < de_treesize; i++) {
		if(DeTree[i].symbol != -1) {
			DecodeTable[i]=extractCode(DeTree,i);
		}
		else {
			DecodeTable[i]=NULL;	//Otherwise free() causes segfault
		}
	}
	//Print the code array
	puts("Codes régénérés...");
	puts("");
	for(size_t i=0; i < de_treesize; i++) {
		if(DeTree[i].symbol != -1) {
			printf("%ld. %c\t%s\n", i, DeTree[i].symbol, DecodeTable[i]);
		}
	}
	puts("");

	/* Decode message */
	FILE* huffwrite=fopen("huff_decompressed.txt", "w");
	if(!huffwrite) {
		perror("Echec de l'écriture");
		return 2;
	}
	dec_count=0; //Reuse
	temp_pos=0; //Reuse
	int char_found=-1;
	//CarrierByte and de_fill NOT reset, start from last written bit of current carrier byte
	while(dec_count < de_total) {
		while(char_found == -1) {
			char_found=decodeMSG(CarrierByte,&de_fill,buftemp,&temp_pos,DecodeTable,de_treesize);
			if(de_fill == 8) {
				Xchar=fgetc(huff);
				CarrierByte=Xchar;
				de_fill=0;
			}
		}
		fputc(DeTree[char_found].symbol,huffwrite);
		dec_count++;
		temp_pos=0;
		char_found=-1;
	}
	puts("Message décodé...");
	puts("");
	fclose(huff);
	fclose(huffwrite);
	for(int i=0; i < de_treesize; i++)
		free(DecodeTable[i]);
	return 0;
}