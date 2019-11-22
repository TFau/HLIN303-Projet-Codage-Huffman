#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re

#srcdir to keep track of where to copy the files; created by the program
def traversal(srcdir, dirpath) :
	dirlist=os.listdir(dirpath)
	for fil in dirlist:
		if os.access(dirpath+"/"+fil,os.R_OK):	#Path readability
			if not os.path.isdir(dirpath+"/"+fil):	#Don't check file extensions on directories
				ext=fil.split(".")
				if len(ext) > 1 and ext[-1] == "txt":
					os.system("chmod 754 "+dirpath+"/"+fil)
					os.system("cp "+dirpath+"/"+fil+" "+srcdir+"/Huff_Files_To_Compress")
					os.system("echo '&!FILE&!"+fil+"&!ADR&!"+dirpath+"&!SEP&!' >> Huff_Files_To_Compress/"+fil)
			else:
				traversal(srcdir,dirpath+"/"+fil) #Recursive

def user_input(string) :
	while True:
		cont=input("Voulez-vous décompresser le fichier? y/n\n")
		if cont not in ('y', 'Y', 'n', 'N'):
			print("Entrez le caractère \'y\' pour décompresser ou \'n\' pour quitter le programme.")
		else:
			break
	if cont in ('n', 'N'):
		print("Lancez le programme avec le fichier encodé en paramètre pour le décompresser.")
		exit()
	else:
		os.system("./Decmpr_Huffman "+string)

def genesis(bigfile) :
	newfile=open("Huff_Temp_Name.txt", "w")	#New file to write the first original file
	with open(bigfile, "r+") as fil:
		for line in fil:
			#The newfile will be "split off" with its original name when the terminating character is read
			if "&!FILE&!" in line:
				file_and_path=re.search("&!FILE&!(.+)&!ADR&!(.+)&!SEP&!", line)
				newfile.close()
				os.system("mv Huff_Temp_Name.txt "+file_and_path.group(1))
				#Moving the original files to their original positions, creating directories if necessary
				path_list=file_and_path.group(2).split("/")
				string=""
				for dirpath in path_list:
					string+=dirpath+"/"
					if not os.path.exists(string):
						os.system("mkdir "+string)
				os.system("mv "+file_and_path.group(1)+" "+string)
				newfile=open("Huff_Temp_Name.txt", "w")	#Newfile reopened to write the next original file
			else:
				newfile.write(line)
	newfile.close()
	os.system("rm -f Huff_Temp_Name.txt "+bigfile)