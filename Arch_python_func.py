#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, shutil, sys


def arg_parse(arg_list) :
	arg=[a for a in arg_list if os.path.exists(a) and a != arg_list[0]]
	if len(arg) < 1:
		sys.exit("Erreur: veuillez indiquer en paramètre du programme un fichier ou dossier existant.") #Written to stderr
	elif len(arg) > 1:
		sys.exit("Erreur: veuillez n'indiquer qu'un fichier ou dossier à traiter.")
	else:
		return arg[0]

def user_help() :
	print("Options disponibles. Tapez 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour fermer le programme.\n")
	print("""\033[1m-n\033[0m\tRenommer le fichier compressé et/ou décompressé. Les fichiers issus
	d'une archive ne seront pas renommés à la décompression.""")		#Triple quotes for printing on multiple lines
	print("""\033[1m-r\033[0m\tSupprimer le fichier ou dossier passé en paramètre. Si le programme est
	lancé en compression et que l'utilisateur choisit de décompresser
	pendant la même exécution du programme, le fichier compressé sera
	également supprimé.""")
	print("\033[1m-c\033[0m\tAfficher les caractères du fichier et leurs codes respectifs.")
	print("\033[1m-p\033[0m\tAfficher le contenu (non encodé) du fichier.\n")
	while True:
		cont=input()
		if cont == 'c':
			return
		elif cont == 'q':
			sys.exit(0)
		else:
			print("Tapez 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour fermer le programme.")

def option_parse(opt_list) :
	valid_opt=['c', 'n', 'p', 'r']
	param=[] #Selected options list
	optionByte=0
	for opt in opt_list:
		if opt == "--help":
			user_help()
		firstchar=opt.find('-')
		if firstchar == 0 and len(opt) > 1:	#'-' is the first character of the string
			if opt[1] == '-' and opt != "--help":
				sys.exit("Erreur: option non reconnue. Utilisez --help pour afficher les options disponibles.")
			elif opt[1] != '-':
				for i in opt:
					if i != '-' and i in valid_opt and i not in param:
						param.append(i)
					elif i != '-' and i not in valid_opt:
						sys.exit("Erreur: option non reconnue. Utilisez --help pour afficher les options disponibles.")
		else:
			sys.exit("Erreur: caractères non reconnus. Utilisez --help pour afficher les options disponibles.")
	for initial in param:
		if initial == 'n':	#New name
			optionByte|=(1<<0)
		if initial == 'r': #Delete file/folder
			optionByte|=(1<<1)
		if initial == 'c':	#Print codes
			optionByte|=(1<<2)
		if initial == 'p':	#Print text
			optionByte|=(1<<3)
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
				print("Tapez 'y' pour continuer ou 'n' pour quitter le programme.")
			else:
				break
		if cont in ('n', 'N'):
			print("Veuillez modifier l'extension .txt du ou des fichiers binaries contenus dans vos dossiers.")
			sys.exit(0)
		else:
			return 1
	else:
		return 0

def traversal(srcdir,destfile) :
	for dirpath, subdirs, files in os.walk(srcdir): #Traverses directory tree automatically!
		for f in files:
			if f.endswith(".txt"):
				full_f=os.path.join(dirpath,f)
				if not text_check(full_f) and os.stat(full_f).st_size != 0: #Check if the file is actually in plain text and not empty
					os.chmod(full_f,0o755)
					with open(destfile,"a") as catfile: #Append mode
						with open(full_f,"r") as tempfile:
							for line in tempfile:
								catfile.write(line)
						catfile.write("\n&!FILE&!"+f+"&!ADR&!"+dirpath+"&!SEP&!\n") #Newline required for deconcatenation

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
			print("Tapez 'y' pour décompresser ou 'n' pour quitter le programme.")
		else:
			break
	if cont in ('n', 'N'):
		print("Lancez le programme avec le fichier encodé en paramètre pour le décompresser.")
		sys.exit(0)
	else:
		if os.system("./Decmpr_Huffman "+string+" "+str(optCode)):
			sys.exit("Echec de la décompression.")
		if optCode & (1<<1):
			os.remove(string)

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
						os.mkdir(string,0o755)
				if not os.path.exists(string+"/"+file_and_path.group(1)): #For shutil.move, which raises an error if the file already exists
					shutil.move(file_and_path.group(1),string)
				else:
					os.remove(file_and_path.group(1))
				newfile=open("Huff_Temp_Name.txt", "w")	#Newfile reopened to write the next original file
			else:
				newfile.write(line)
	newfile.close()
	os.remove("Huff_Temp_Name.txt")
	os.remove(bigfile)