#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, sys

#srcdir to keep track of where to copy the files; created by the program
def traversal(srcdir, dirpath, comp_list) :
	dirlist=os.listdir(dirpath)
	for fil in dirlist:
		if os.access(dirpath+"/"+fil,os.R_OK):	#Path readability
			if not os.path.isdir(dirpath+"/"+fil):	#Don't check file extensions on directories
				ext=fil.split(".")
				if len(ext) > 1:
					if ext[-1] == "txt":
						comp_list.append(fil)
						os.system("cp "+dirpath+"/"+fil+" "+srcdir+"/Huff_Files_To_Compress")
			else:
				traversal(srcdir,dirpath+"/"+fil,comp_list) #Recursive

def user_input() :
	while True:
		cont=input("Voulez-vous décompresser le fichier? y/n\n")
		if cont not in ('y', 'Y', 'n', 'N'):
			print("Entrez le caractère y pour décompresser, n pour quitter le programme.")
		else:
			break
	if cont == 'n' or 'N':
		exit()
	else:
		os.system("./Decmpr_Huffman Huff_To_Compress.txt")


#If the program's parameter is a file, detect whether it is encoded or not
if os.path.isfile(sys.argv[1]):
	try:
		with open(sys.argv[1], "r") as fil:
			for line in fil.readlines():
				pass
	except UnicodeDecodeError:	#The file is encoded--decode it
		os.system("./Decmpr_Huffman Huff_To_Compress.txt")
	else:	#The file is plain text--encode it
		os.system("./Cmpr_Huffman "+sys.argv[1])
		user_input()	#Decode now or later

#The program's parameter is a directory
else:
	os.system("mkdir Huff_Files_To_Compress")	#Directory in which the files to compress are copied before concatenation
	Files_To_Compress=[] #List to store all files obtained from traversal
	#getcwd() returns current working directory
	traversal(os.getcwd(),sys.argv[1],Files_To_Compress)

	#Add separator at the end of each copied file
	dirlist=os.listdir("Huff_Files_To_Compress")
	for fil in dirlist:
		os.system("chmod 754 Huff_Files_To_Compress/"+fil+"; echo '&!FILE&!SEP&!' >> Huff_Files_To_Compress/"+fil)
	#Build command line string from the file list
	string=""
	for f in Files_To_Compress:
		string+=f+" "
	Files_To_Compress=[]
	os.system("touch Huff_Files_To_Compress/Huff_To_Compress.txt; chmod 754 Huff_Files_To_Compress/Huff_To_Compress.txt") #Creating concatenation file
	os.system("cd Huff_Files_To_Compress; cat "+string+" > Huff_To_Compress.txt") #Concatenate all files into the main file
	os.system("cd Huff_Files_To_Compress; rm -f "+string) #Delete copied files
	os.system("mv Huff_Files_To_Compress/Huff_To_Compress.txt .; rm -d Huff_Files_To_Compress")

	#Call encoder
	os.system("./Cmpr_Huffman Huff_To_Compress.txt")
	user_input()

#Need to delete Huff_To_Compress.txt after compression