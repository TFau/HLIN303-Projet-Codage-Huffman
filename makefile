com : Cmpr_Huffman
dec : Decmpr_Huffman
arch : Archive_Huffman
clean: cls

#########

SOURCE = Com_Functions.h Com_Functions.c Com_Huffman.c Dec_Functions.h Dec_Functions.c Dec_Huffman.c \
Arch_python.py Arch_python_func.py README.md makefile

#########

Com_Functions.o : Com_Functions.c
	gcc -c -Wall Com_Functions.c

Com_Huffman.o : Com_Huffman.c
	gcc -c -Wall Com_Huffman.c

Cmpr_Huffman : Com_Functions.o Com_Huffman.o
	gcc -Wall Com_Functions.o Com_Huffman.o -o Cmpr_Huffman

#########

Dec_Functions.o : Dec_Functions.c
	gcc -c -Wall Dec_Functions.c

Dec_Huffman.o : Dec_Huffman.c
	gcc -c -Wall Dec_Huffman.c

Decmpr_Huffman : Dec_Functions.o Dec_Huffman.o
	gcc -Wall Dec_Functions.o Dec_Huffman.o -o Decmpr_Huffman

#########

Archive_Huffman : $(SOURCE)
	tar -zcvf Huffman_Archiver.tar.gz $(SOURCE)

#########

cls :
	rm -f *.o