#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, sys
from Arch_python_func import traversal, user_input, genesis

###########
# PROGRAM #
###########

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
	os.system("mkdir Huff_Files_To_Compress")	#Directory in which the files to compress are copied before concatenation
	traversal(os.getcwd(),sys.argv[1]) #getcwd() returns current working directory

	dirlist=os.listdir("Huff_Files_To_Compress")
	#Check if the collected .txt files are actually in plain text
	for fil in dirlist:
		try:
			with open("Huff_Files_To_Compress/"+fil, "r") as testfil:
				for line in testfil.readlines():
					pass
		except UnicodeDecodeError:	#The file is a binary file--remove it
			os.system("rm -f Huff_Files_To_Compress/"+fil)
		else:
			pass

	#Build command line string from the file list
	string=""
	for f in dirlist:
		string+=f+" "
	os.system("touch Huff_Files_To_Compress/Huff_To_Compress.txt; chmod 754 Huff_Files_To_Compress/Huff_To_Compress.txt") #Create concatenation file
	os.system("cd Huff_Files_To_Compress; cat "+string+" > Huff_To_Compress.txt") #Concatenate all files into the main file
	os.system("cd Huff_Files_To_Compress; rm -f "+string) #Delete copied files
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