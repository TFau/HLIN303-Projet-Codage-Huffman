#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, sys


def arg_parse(arg_list) :
	f=""
	for arg in sys.argv:
		if os.path.exists(arg) and arg != arg_list[0]:
			f=arg
	if f == "":
		sys.stderr.write("Erreur: dossier ou fichier inexistent.\n")
		exit()
	arg_list.remove(f)
	return f

def option_parse(opt_list) :
	param=[] #Selected options list
	optionByte=0
	for opt in opt_list:
		firstchar=opt.find('-')
		if firstchar == 0:	#'-'' is the first character of the string, i.e. the string is an option
			for i in opt:
				if i != '-' and i in ('c', 'n', 'r'): #Add future options
					param.append(i)
	for initial in param:
		if initial == 'n':	#Option new file
			optionByte|=(1<<0)
		if initial == 'r': #Option delete folder
			optionByte|=(1<<1)
		if initial == 'c':	#Option print codes
			optionByte|=(1<<2)
	return optionByte

def traversal(srcdir, dirpath) :
	dirlist=os.listdir(dirpath)
	for fil in dirlist:
		if os.access(dirpath+"/"+fil,os.R_OK):	#Path readability
			if not os.path.isdir(dirpath+"/"+fil):	#Don't check file extensions on directories
				ext=fil.split(".")
				if len(ext) > 1 and ext[-1] == "txt":
					os.system("chmod 754 "+dirpath+"/"+fil)
					os.system("cp "+dirpath+"/"+fil+" "+srcdir+"/Huff_Files_To_Compress")
					os.system("echo '\n&!FILE&!"+fil+"&!ADR&!"+dirpath+"&!SEP&!' >> Huff_Files_To_Compress/"+fil) #Newline required
					os.system("cat Huff_Files_To_Compress/"+fil+" >> Huff_Files_To_Compress/Huff_To_Compress.txt")
					os.system("rm -f Huff_Files_To_Compress/"+fil)
			else:
				traversal(srcdir,dirpath+"/"+fil) #Recursive

def user_input(string,optionCode) :
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
		os.system("./Decmpr_Huffman "+string+" "+str(optionCode))

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