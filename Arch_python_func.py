#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, shutil, sys


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
	print("Options disponibles. Entrez le caractère 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour terminer l'exécution du programme.\n")
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
			print("Entrez le caractère 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour terminer l'exécution du programme.")

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

def text_check(file) :
	try:
		with open(file, "r") as test:
			for line in test:
				pass
	except UnicodeDecodeError:
		sys.stderr.write("Attention: Le fichier '"+file+"' contient du code binaire. Il ne sera pas compressé.\n")
		while True:
			cont=input("Souhaitez-vous continuer l'exécution du programme? y/n\n")
			if cont not in ('y', 'Y', 'n', 'N'):
				print("Entrez le caractère 'y' pour continuer ou 'n' pour quitter le programme.")
			else:
				break
		if cont in ('n', 'N'):
			print("Veuillez modifier l'extension .txt du ou des fichiers binaries contenus dans vos dossiers.")
			sys.exit(0)
		else:
			return 1
	else:
		return 0

def traversal(basedir,srcdir,destfile) :
	for dirpath, subdirs, files in os.walk(srcdir): #Traverses directory tree automatically!
		for f in files:
			if f.endswith(".txt"):
				full_f=os.path.join(dirpath,f)
				if not text_check(full_f): #Check if the file is actually in plain text
					os.chmod(full_f,0o755)
					shutil.copy2(full_f,basedir+"/Huff_Files_To_Compress") #Copy with metadata
					with open("Huff_Files_To_Compress/"+f,"a") as tempfile: #Append mode
						tempfile.write("\n&!FILE&!"+f+"&!ADR&!"+dirpath+"&!SEP&!\n") #Newline required for deconcatenation
					with open("Huff_Files_To_Compress/"+destfile,"a") as catfile:
						with open("Huff_Files_To_Compress/"+f,"r") as tempfile:
							for line in tempfile:
								catfile.write(line)
					os.remove("Huff_Files_To_Compress/"+f)

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
			print("Entrez le caractère 'y' pour décompresser ou 'n' pour quitter le programme.")
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
	with open(bigfile, "r+") as file:
		for line in file:
			#The newfile will be "split off" with its original name when a separator is read
			if "&!FILE&!" in line:
				file_and_path=re.search("&!FILE&!(.+)&!ADR&!(.+)&!SEP&!", line)
				newfile.close()
				with open("Huff_Temp_Name.txt", "rb+") as eraser:	#Remove the newline added before the separator, "b" required for seek method
					eraser.seek(-1, os.SEEK_END)
					eraser.truncate()
				os.rename("Huff_Temp_Name.txt",file_and_path.group(1))
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