#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, shutil, sys
from Arch_python_func import arg_parse, option_parse, traversal, user_rename, user_input, genesis


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
			sys.exit(4)
		if optionCode & (1<<1):
			os.remove(file_to_proc)
	else:	#The file is plain text--encode it
		newfile="ENCODED_"+file_to_proc
		if os.system("./Cmpr_Huffman "+file_to_proc+" "+str(optionCode)):
			sys.exit(5)
		if optionCode & (1<<0):
			newfile=user_rename(newfile,0)
		if optionCode & (1<<1):
			os.remove(file_to_proc)
		user_input(newfile,optionCode)	#Decode now or later

#The program's parameter is a directory
else:
	A_file_to_proc="Arch_"+file_to_proc+".huf"
	os.mkdir("Huff_Files_To_Compress",0o755) #Octal notation using 0o
	open("Huff_Files_To_Compress/"+A_file_to_proc,"a").close()#Create concatenation file
	os.chmod("Huff_Files_To_Compress/"+A_file_to_proc,0o755)
	traversal(os.getcwd(),file_to_proc,A_file_to_proc) #getcwd() returns current working directory
	shutil.move("Huff_Files_To_Compress/"+A_file_to_proc,os.getcwd())
	os.rmdir("Huff_Files_To_Compress") #Empty

	#Call encoder
	if os.system("./Cmpr_Huffman "+A_file_to_proc+" "+str(optionCode)):
		sys.exit(6)
	if optionCode & (1<<1):
		shutil.rmtree(file_to_proc)
	E_file_to_proc="ENCODED_"+A_file_to_proc
	if optionCode & (1<<0):
		E_file_to_proc=user_rename(E_file_to_proc,0)
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
if "&!FILE&!" in os.popen("tail -n 1 "+concat_file).read(8): #Check for separator characters in the first 8 bytes of the file's last line:
	genesis(concat_file)
elif optionCode & (1<<0):
	concat_file=user_rename(concat_file,1)