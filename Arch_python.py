#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, sys

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

#If the decoded file was made from concatenated files, split it back into the original files
if os.path.isdir(sys.argv[1]):
	concat_file="Huff_To_Compress.txt"
else:
	concat_file=sys.argv[1]
detect=os.popen("tail -n 1 "+concat_file).read(8) #Check for separator characters in the first 8 bytes of the file's last line
if "&!FILE&!" in detect:
	newfile=open("Huff_Temp_Name.txt", "w")	#New file to write the first original file
	with open(concat_file, "r+") as fil:
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
	os.system("rm -f Huff_Temp_Name.txt Huff_To_Compress.txt")