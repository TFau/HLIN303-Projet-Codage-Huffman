#!/usr/bin/env python3
#-*- coding: utf-8 -*-

## Programme principal de l'archiveur/interface utilisateur
# @file Arch_python.py
# @author Troy Fau

import os, subprocess, sys
from Arch_python_func import arg_parse, option_parse, user_choice, traversal, user_rename, user_input, folder_remove, genesis


#Program launch parameter and option check
file_to_proc=arg_parse(sys.argv) #Exits if no file/folder or >1 file/folder
del sys.argv[0]
sys.argv.remove(file_to_proc)
optionCode=option_parse(sys.argv)

#If the program's parameter is a file, detect whether it is encoded or not
if os.path.isfile(file_to_proc):
	if os.stat(file_to_proc).st_size == 0:
		sys.exit("Erreur: Le fichier passé en argument du programme est vide.")
	try:
		with open(file_to_proc, "r") as fil:
			for line in fil:
				pass
	except UnicodeDecodeError:	#The file is a binary file--decode it
		subprocess.run([os.getcwd()+"/Decmpr_Huffman",file_to_proc,str(optionCode)],check=True)
		if optionCode & (1<<1):
			os.remove(file_to_proc)
	else:	#The file is plain text--encode it
		newfile="ENCODED_"+file_to_proc
		subprocess.run([os.getcwd()+"/Cmpr_Huffman",file_to_proc,str(optionCode)],check=True)
		if optionCode & (1<<0):
			newfile=user_rename(newfile,0)
		if optionCode & (1<<1):
			os.remove(file_to_proc)
		user_input(newfile,optionCode)	#Decode now or later

#The program's parameter is a directory
else:
	ext_proc=user_choice()
	A_file_to_proc="Arch_"+file_to_proc+".huf"
	open(A_file_to_proc,"a").close()#Create concatenation file
	os.chmod(A_file_to_proc,0o755) #Octal notation using 0o
	optionCode=traversal(file_to_proc,A_file_to_proc,ext_proc,optionCode)
	if optionCode & (1<<4):
		print("Annulation de l'archivage...")
		genesis(A_file_to_proc)
		sys.exit(0)
	if os.stat(A_file_to_proc).st_size == 0: #Final check
		os.remove(A_file_to_proc)
		sys.exit("Erreur: le dossier passé en argument ne contient pas de fichiers d'extension '"+ext_proc+"' ou "
		"uniquement des fichiers d'extension '"+ext_proc+"' vides.")
	#Call encoder
	subprocess.run([os.getcwd()+"/Cmpr_Huffman",A_file_to_proc,str(optionCode)],check=True)
	E_file_to_proc="ENCODED_"+A_file_to_proc
	if optionCode & (1<<0):
		E_file_to_proc=user_rename(E_file_to_proc,0)
	if optionCode & (1<<1):
		folder_remove(file_to_proc)
	os.remove(A_file_to_proc)
	user_input(E_file_to_proc,optionCode)

#Deconcatenation and directory rebuilding
if "E_file_to_proc" in locals():
	concat_file="DECODED_"+E_file_to_proc
elif "newfile" in locals():
	concat_file="DECODED_"+newfile
else:
	concat_file="DECODED_"+file_to_proc
#Check if the file is made from concatenated files
if "&!FILE&!" in os.popen("tail -n 1 \""+concat_file+"\"").read(8): #Check for separator characters in the first 8 bytes of the file's last line
	genesis(concat_file)
elif optionCode & (1<<0):
	concat_file=user_rename(concat_file,1)