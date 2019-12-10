#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, sys
from Arch_python_func import arg_parse, option_parse, traversal, user_input, genesis


#Program launch parameter and option check
file_to_proc=arg_parse(sys.argv) #Exits if no file/folder or >1 file/folder
sys.argv.remove(file_to_proc)
optionCode=option_parse(sys.argv)

#If the program's parameter is a file, detect whether it is encoded or not
if os.path.isfile(file_to_proc):
	try:
		with open(file_to_proc, "r") as fil:
			for line in fil:
				pass
	except UnicodeDecodeError:	#The file is a binary file--decode it
		if os.system("./Decmpr_Huffman "+file_to_proc+" "+str(optionCode)): #Returns >0 in case of error
			exit()
		if optionCode & (1<<1):
			os.system("rm -f "+file_to_proc)
	else:	#The file is plain text--encode it
		newfile="ENCODED_"+file_to_proc
		if os.system("./Cmpr_Huffman "+file_to_proc+" "+str(optionCode)):
			exit()
		if optionCode & (1<<1):
			os.system("rm -f "+file_to_proc)
		user_input(newfile,optionCode)	#Decode now or later

#The program's parameter is a directory
else:
	A_file_to_proc="Arch_"+file_to_proc+".huf"
	os.system("mkdir Huff_Files_To_Compress; touch Huff_Files_To_Compress/"+A_file_to_proc) #Temporary directory for concatenation file
	os.system("chmod 754 Huff_Files_To_Compress/"+A_file_to_proc)
	traversal(os.getcwd(),file_to_proc,A_file_to_proc) #getcwd() returns current working directory
	#Check if the collected .txt files are actually in plain text
	try:
		with open("Huff_Files_To_Compress/"+A_file_to_proc, "r") as fil:
			for line in fil:
				pass
	except UnicodeDecodeError:
		sys.stderr.write("Erreur: le fichier à compresser contient du code binaire. Opération interrompue.")
		sys.stderr.write("Veuillez modifier l'extension .txt du ou des fichiers binaires présents dans vos dossiers.")
		os.system("mv Huff_Files_To_Compress/"+A_file_to_proc+" .; rm -d Huff_Files_To_Compress")
		exit()
	else:
		pass
	os.system("mv Huff_Files_To_Compress/"+A_file_to_proc+" .; rm -d Huff_Files_To_Compress")

	#Call encoder
	if os.system("./Cmpr_Huffman "+A_file_to_proc+" "+str(optionCode)):
		exit()
	if optionCode & (1<<1):
		os.system("rm -r "+file_to_proc)
	E_file_to_proc="ENCODED_"+A_file_to_proc
	os.system("rm -f "+A_file_to_proc)
	user_input(E_file_to_proc,optionCode)

#Deconcatenation and directory rebuilding
if "E_file_to_proc" in locals():
	concat_file="DECODED_"+E_file_to_proc
elif "newfile" in locals():
	concat_file="DECODED_"+newfile
else:
	concat_file="DECODED_"+file_to_proc
#Check if the file is made from concatenated files
detect=os.popen("tail -n 1 "+concat_file).read(8) #Check for separator characters in the first 8 bytes of the file's last line
if "&!FILE&!" in detect:
	genesis(concat_file)