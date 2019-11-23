#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, sys
from Arch_python_func import traversal, user_input, genesis


#Program launch parameter check
if len(sys.argv) < 2:
	sys.stderr.write("Erreur: indiquez en paramètre du programme le fichier ou dossier à traiter.\n")
	exit()
if not os.path.exists(sys.argv[1]):
	sys.stderr.write("Erreur: le dossier ou fichier \'"+sys.argv[1]+"\' n'existe pas.\n")
	exit()

#If the program's parameter is a file, detect whether it is encoded or not
if os.path.isfile(sys.argv[1]):
	try:
		with open(sys.argv[1], "r") as fil:
			for line in fil:
				pass
	except UnicodeDecodeError:	#The file is a binary file--decode it
		os.system("./Decmpr_Huffman "+sys.argv[1])
	else:	#The file is plain text--encode it
		os.system("./Cmpr_Huffman "+sys.argv[1])
		user_input(sys.argv[1])	#Decode now or later

#The program's parameter is a directory
else:
	os.system("mkdir Huff_Files_To_Compress; touch Huff_Files_To_Compress/Huff_To_Compress.txt") #Temporary directory for concatenation file
	os.system("chmod 754 Huff_Files_To_Compress/Huff_To_Compress.txt")
	traversal(os.getcwd(),sys.argv[1]) #getcwd() returns current working directory
	#Check if the collected .txt files are actually in plain text
	try:
		with open("Huff_Files_To_Compress/Huff_To_Compress.txt", "r") as fil:
			for line in fil:
				pass
	except UnicodeDecodeError:
		sys.stderr.write("Erreur: le fichier à compresser contient du code binaire. Opération interrompue.")
		sys.stderr.write("Veuillez modifier l'extension .txt du ou des fichiers binaires présents dans vos dossiers.")
		os.system("mv Huff_Files_To_Compress/Huff_To_Compress.txt .; rm -d Huff_Files_To_Compress")
		exit()
	else:
		pass
	os.system("mv Huff_Files_To_Compress/Huff_To_Compress.txt .; rm -d Huff_Files_To_Compress")

	#Call encoder
	os.system("./Cmpr_Huffman Huff_To_Compress.txt")
	user_input("Huff_To_Compress.txt")

#Deconcatenation and directory rebuilding
if os.path.isdir(sys.argv[1]):
	concat_file="Huff_To_Compress.txt"
else:
	concat_file=sys.argv[1]
#Check if the file is made from concatenated files
detect=os.popen("tail -n 1 "+concat_file).read(8) #Check for separator characters in the first 8 bytes of the file's last line
if "&!FILE&!" in detect:
	genesis(concat_file)