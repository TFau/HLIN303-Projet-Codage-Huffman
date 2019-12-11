#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, sys


def arg_parse(arg_list) :
	arg=[a for a in arg_list if os.path.exists(a) and a != arg_list[0]]
	if len(arg) < 1:
		sys.stderr.write("Erreur: veuillez indiquer en paramètre du programme un fichier ou dossier existant.\n")
		sys.exit(1)
	elif len(arg) > 1:
		sys.stderr.write("Erreur: veuillez n'indiquer qu'un fichier ou dossier à traiter.\n")
		sys.exit(2)
	else:
		return arg[0]

def user_help() :
	print("Options disponibles. Appuyez sur c pour sortir de l'aide et continuer le programme, ou sur q pour terminer l'exécution du programme.\n")
	print("""\033[1m-n\033[0m\tRenommer le fichier compressé et/ou décompressé. Les fichiers issus
	d'une archive ne seront pas renommés à la décompression.""")		#Triple quotes for printing on multiple lines
	print("""\033[1m-r\033[0m\tEffacer le fichier ou dossier passé en paramètre. Si le programme est
	lancé en compression et que l'utilisateur choisit de décompresser
	pendant la même exécution, le fichier compressé sera également effacé.""")
	print("\033[1m-c\033[0m\tAfficher les caractères du fichier et leurs codes respectifs.\n")
	while True:
		cont=input()
		if cont == 'c':
			return
		elif cont == 'q':
			sys.exit(0)
		else:
			print("Appuyez sur c pour sortir de l'aide et continuer le programme, ou sur q pour terminer l'exécution du programme.")

def option_parse(opt_list) :
	valid_opt=['c', 'n', 'r']
	param=[] #Selected options list
	optionByte=0
	for opt in opt_list:
		if opt == "--help":
			user_help()
		firstchar=opt.find('-')
		if firstchar == 0 and len(opt) > 1 and opt[1] != '-':	#'-' is the first character of the string; lazy evaluation for 3rd clause
			for i in opt:
				if i != '-' and i in valid_opt and i not in param:
					param.append(i)
				elif i != '-' and i not in valid_opt:
					sys.stderr.write("Erreur: option non reconnue.\n")
					sys.exit(3)
	for initial in param:
		if initial == 'n':	#New name
			optionByte|=(1<<0)
		if initial == 'r': #Delete file/folder
			optionByte|=(1<<1)
		if initial == 'c':	#Print codes
			optionByte|=(1<<2)
	return optionByte

def traversal(srcdir, dirpath, destfile) :
	dirlist=os.listdir(dirpath)
	for fil in dirlist:
		if os.access(dirpath+"/"+fil,os.R_OK):	#Path readability
			if not os.path.isdir(dirpath+"/"+fil):	#Don't check file extensions on directories
				ext=fil.split(".")
				if len(ext) > 1 and ext[-1] == "txt":
					os.system("chmod 754 "+dirpath+"/"+fil)
					os.system("cp "+dirpath+"/"+fil+" "+srcdir+"/Huff_Files_To_Compress")
					os.system("echo '\n&!FILE&!"+fil+"&!ADR&!"+dirpath+"&!SEP&!' >> Huff_Files_To_Compress/"+fil) #Newline required
					os.system("cat Huff_Files_To_Compress/"+fil+" >> Huff_Files_To_Compress/"+destfile)
					os.system("rm -f Huff_Files_To_Compress/"+fil)
			else:
				traversal(srcdir,dirpath+"/"+fil,destfile) #Recursive

def user_rename(old_name,intgr) :
	while True:
		if intgr == 0:
			new_name=input("Entrez un nom pour le fichier compressé:\n")
		else:
			new_name=input("Entrez un nom pour le fichier décompressé:\n")
		if new_name == "":
			print("Le nom du fichier doit contenir au moins un caractère.")
		elif os.path.exists(new_name):
			print("Un fichier ou dossier porte déjà ce nom.")
		else:
			break
	os.rename(old_name,new_name)
	return new_name	#To update variable in main program

def user_input(string,optCode) :
	while True:
		cont=input("Voulez-vous décompresser le fichier? y/n\n")
		if cont not in ('y', 'Y', 'n', 'N'):
			print("Entrez le caractère \'y\' pour décompresser ou \'n\' pour quitter le programme.")
		else:
			break
	if cont in ('n', 'N'):
		print("Lancez le programme avec le fichier encodé en paramètre pour le décompresser.")
		sys.exit(0)
	else:
		if os.system("./Decmpr_Huffman "+string+" "+str(optCode)):
			sys.exit()
		if optCode & (1<<1):
			os.system("rm -f "+string)

def genesis(bigfile) :
	newfile=open("Huff_Temp_Name.txt", "w")	#New file to write the first original file
	with open(bigfile, "r+") as fil:
		for line in fil:
			#The newfile will be "split off" with its original name when a separator is read
			if "&!FILE&!" in line:
				file_and_path=re.search("&!FILE&!(.+)&!ADR&!(.+)&!SEP&!", line)
				newfile.close()
				with open("Huff_Temp_Name.txt", "rb+") as eraser:	#Remove the newline added before the separator, "b" required for seek method
					eraser.seek(-1, os.SEEK_END)
					eraser.truncate()
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