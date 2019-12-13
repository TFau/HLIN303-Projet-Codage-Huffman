#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, shutil, sys


def arg_parse(arg_list) :
	for a in arg_list:
		if a == "--help":
			user_help()
	arg=[a for a in arg_list if os.path.exists(a) and a != arg_list[0]]
	if len(arg) < 1:
		sys.exit("Erreur: veuillez indiquer en paramètre du programme un fichier ou dossier existant.") #Written to stderr
	elif len(arg) > 1:
		sys.exit("Erreur: veuillez n'indiquer qu'un fichier ou dossier à traiter.")
	else:
		return arg[0]

def user_help() :
	#Triple quotes for printing on multiple lines
	print("""Programme de compression et d'archivage de fichiers texte par codage de Huffman.
Syntaxe: Arch_python.py [options] [fichier ou dossier] [options]
L'exécution directe du compresseur ou du décompresseur est dépréciée.\n
Options disponibles. Tapez 'c' pour sortir de l'aide et continuer le programme,
ou 'q' pour fermer le programme.\n""")
	print("""\033[1m-n\033[0m\tRenommer le fichier compressé et/ou décompressé. Les fichiers issus
	d'une archive ne seront pas renommés à la décompression.""")
	print("""\033[1m-r\033[0m\tSupprimer le fichier passé en argument, ou supprimer les fichiers copiés
	de l'arborescence du dossier passé en argument. Si l'arborescence du
	dossier a été entièrement vidé, la supprime également. Si le programme
	est lancé en compression et que l'utilisateur choisit de décompresser
	pendant la même exécution du programme, le fichier compressé sera
	également supprimé.""")
	print("\033[1m-c\033[0m\tAfficher les caractères distincts du fichier et leurs codes respectifs.")
	print("""\033[1m-p\033[0m\tAfficher le contenu (non encodé) du fichier. Dans le cas d'une archive,
	affiche le contenu du fichier de concaténation.\n""")
	while True:
		cont=input()
		if cont == 'c':
			return
		elif cont == 'q':
			sys.exit(0)
		else:
			print("Tapez 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour fermer le programme.")

def option_parse(opt_list) :
	optionByte=0
	for opt in opt_list: #Removing while iterating is fine here
		if opt == "--help":
			opt_list.remove("--help")
	for opt in opt_list:
		firstchar=opt.find('-')
		if firstchar == 0 and len(opt) > 1:	#'-' is the first character of the string
			for i in opt[1:]: #Read after '-'
				if i == 'n':	#New name
					optionByte|=(1<<0)
				elif i == 'r':	#Delete file/folder
					optionByte|=(1<<1)
				elif i == 'c':	#Print codes
					optionByte|=(1<<2)
				elif i == 'p':	#Print text
					optionByte|=(1<<3)
				else:
					sys.exit("Erreur: option non reconnue. Utilisez --help pour afficher les options disponibles.")
		else:
			sys.exit("Erreur: caractères non reconnus. Utilisez --help pour afficher les options disponibles.")
	return optionByte

def user_choice() :
	while True:
		ext=input("Entrez l'extension des fichiers que vous souhaitez archiver:\n")
		if ext == "":
			print("L'extension ne peut être vide.")
		else:
			break
	if ext[0] != '.':
		ext="."+ext
	return ext

def text_check(file,ext) :
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
			print("Veuillez modifier l'extension '"+ext+"' du ou des fichiers binaires contenus dans vos dossiers.")
			sys.exit(0)
		else:
			return 1
	else:
		return 0

def traversal(srcdir,destfile,ext,optCode) :
	for dirpath, subdirs, files in os.walk(srcdir): #Traverses directory tree automatically!
		for f in files:
			if f.endswith(ext):
				full_f=os.path.join(dirpath,f)
				if not text_check(full_f,ext) and os.stat(full_f).st_size != 0: #Check if the file is actually in plain text and not empty
					os.chmod(full_f,0o755)
					with open(destfile,"a") as catfile: #Append mode
						with open(full_f,"r") as tempfile:
							for line in tempfile:
								catfile.write(line)
						catfile.write("\n&!FILE&!"+f+"&!ADR&!"+dirpath+"&!SEP&!\n") #Newline required for deconcatenation
					if optCode & (1<<1):
						os.remove(full_f)

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

def folder_remove(srcdir) :
	if os.path.isfile(srcdir):
		return False
	elif not os.listdir(srcdir):
		os.rmdir(srcdir)
		return True
	elif all([folder_remove(os.path.join(srcdir,d)) for d in os.listdir(srcdir)]):
		os.rmdir(srcdir)
		return True
	else:
		return False

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