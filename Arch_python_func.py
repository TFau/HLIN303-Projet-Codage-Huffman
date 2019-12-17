#!/usr/bin/env python3
#-*- coding: utf-8 -*-

## Fonctions de l'archiveur
# @file Arch_python_func.py
# @author Troy Fau

import os, re, shutil, subprocess, sys

## Fonction de recherche du fichier/dossier
#
# La fonction vérifie qu'un et un seul fichier ou dossier existant dans le dossier courant 
# est présent dans arg_list. Si c'est le cas, la chaîne correspondante est renvoyée, sinon 
# une erreur est signalée et le programme arrêté. Affiche l'aide si l'option --help a été 
# sélectionnée.
# @param[in] arg_list La liste des arguments passés à la ligne de commande amputée du programme lui-même.
# @return Le nom du fichier ou dossier à traiter.
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

## Fonction d'affichage de l'aide
#
# Sous-fonction de arg_parse. Affiche une aide sommaire et les options disponibles.
def user_help() :
	#Triple quotes for printing on multiple lines
	print("""\n\033[91mProgramme de compression et d'archivage de fichiers texte par codage de Huffman.\033[0m\n
\033[91mSyntaxe:\033[0m Arch_python.py [\033[3moptions\033[0m]... [\033[3mfichier ou dossier\033[0m] [\033[3moptions\033[0m]...
Doivent être présents dans le même dossier que le script: \033[92mArch_python_func.py\033[0m,
\033[92mCmpr_Huffman\033[0m et \033[92mDecmpr_Huffman\033[0m. Si ces deux derniers n'ont pas été compilés et
que vous avez accès aux fichiers sources, exécutez les commandes 'make com' et
'make dec'.
L'exécution directe du compresseur ou du décompresseur est dépréciée.\n
\033[91mOptions disponibles.\033[0m""")
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
	affiche le contenu du fichier d'archivage.\n""")
	print("Tapez 'c' pour continuer le programme, ou 'q' pour le quitter.")
	while True:
		cont=input()
		if cont == 'c':
			return
		elif cont == 'q':
			sys.exit(0)
		else:
			print("Tapez 'c' pour sortir de l'aide et continuer le programme, ou 'q' pour fermer le programme.")

## Fonction de recherche des options
#
# La fonction traite opt_list pour récupérer les options et vérifier leur validité; une option 
# invalide ou des caractères sauvages déclenchent une erreur et l'arrêt prématuré du programme. 
# Aux 4 options disponibles correspondent les 4 bits de poids faible d'un entier. Un bit est 
# mis à 1 si l'option correspondante a été sélectionnée, et après encodage l'entier est renvoyé.
# @param[in] opt_list La liste des arguments passés à la ligne de commande amputée du programme et du fichier ou dossier à traiter.
# @return L'entier stockant les options.
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

## Fonction de choix d'extension pour l'archivage
#
# Demande à l'utilisateur de saisir l'extension des fichiers texte qu'il veut archiver à partir 
# d'une arborescence de dossier.
# @return L'extension à rechercher.
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

## Fonction de vérification et d'alerte sur la présence de code binaire
#
# Sous-fonction de traversal. Vérifie que file est bien un fichier texte en utilisant la structure de 
# gestion des exceptions. Si file contient du code binaire, la fonction signale à l'utilisateur que le 
# fichier ne sera pas concaténé dans le fichier destiné à la compression et offre la possibilité 
# d'interrompre le programme. Dans ce cas, le fichier d'archivage en cours sera déconcaténé.
# @param[in] file Fichier à vérifier.
# @param[in] ext Extension des fichiers à traiter.
# @param[in] opt Stockage bit à bit des options sélectionnés par l'utilisateur.
# @return 0 si le fichier est bien un fichier texte, 1 si le fichier est un binaire mais que l'utilisateur
# veut continuer le programme en ne copiant pas ce fichier, 2 si l'utilisateur chosit d'arrêter le programme.
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
			return 2
		else:
			return 1
	else:
		return 0

## Fonction de recherche et d'archivage des fichiers
#
# La fonction utilise os.walk(srcdir) pour obtenir tous les chemins d'accès, sous-dossiers et fichiers 
# de l'arborescence de dossiers ayant srcdir comme racine. Tous les fichiers munis de l'extension ".txt" 
# sont alors vérifiés par text_check; si ce sont bien des fichiers texte leur contenu est copié dans 
# destfile, avec rajout d'une ligne séparatrice spéciale contenant le nom d'origine du fichier et son 
# chemin d'accès. Si l'option -r a été sélectionnée, le fichier d'origine est supprimé.
# @param[in] srcdir Répertoire racine de la recherche.
# @param[in] destfile Fichier de concaténation.
# @param[in] ext Extension des fichiers à traiter.
# @param[in,out] optCode Stockage bit à bit des options sélectionnés par l'utilisateur.
# @return L'entier optCode stockant les options, potentiellement modifié si l'utilisateur a choisit
# d'arrêter le programme.
def traversal(srcdir,destfile,ext,optCode) :
	for dirpath, subdirs, files in os.walk(srcdir): #Traverses directory tree automatically!
		for f in files:
			if f.endswith(ext):
				full_f=os.path.join(dirpath,f)
				interrupt=text_check(full_f,ext)
				if not interrupt and os.stat(full_f).st_size != 0: #Check if the file is actually in plain text and not empty
					os.chmod(full_f,0o755)
					with open(destfile,"a") as catfile: #Append mode
						with open(full_f,"r") as tempfile:
							for line in tempfile:
								catfile.write(line)
						catfile.write("\n&!FILE&!"+f+"&!ADR&!"+dirpath+"&!SEP&!\n") #Newline required for deconcatenation
					if optCode & (1<<1):
						os.remove(full_f)
				elif interrupt == 2:
					optCode|=(1<<4)	#Cancel archiving signal
					return optCode
	return optCode


##Fonction de renommage de fichier
#
# Si l'utilisateur a sélectionné l'option -n, cette fonction lui propose de renommer le fichier compressé 
# (si intgr=0) ou décompressé (si intgr=1). Un nom vide ou déjà existant n'est pas accepté, un nom valide 
# est renvoyé.
# @param[in] old_name Nom du fichier à renommer.
# @param[in] intgr Indique s'il s'agit d'un fichier compressé ou décompressé.
# @return Le nouveau nom du fichier.
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

##Fonction de relance
#
# Demande à l'utilisateur s'il veut décompresser le fichier. Si l'utilisateur répond par la négative, le 
# programme est arrêté. Sinon la fonction lance le décompresseur avec string et optCode passés en arguments.
# @param[in] string Nom du fichier.
# @param[in] optCode Stockage bit à bit des options sélectionnés par l'utilisateur. 
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
		subprocess.run([os.getcwd()+"/Decmpr_Huffman",string,str(optCode)],check=True)
		if optCode & (1<<1):
			os.remove(string)

##Fonction de suppression des dossiers vides
#
# Avec l'option -r, tous les fichiers copiés à partir de l'arborescence du dossier passé en argument sont 
# supprimés par la fonction traversal. Au terme de cette phase, la fonction folder_remove vérifie récursivement 
# si le dossier et ses sous-dossiers sont vides, et si c'est le cas, les supprime.
# @param[in] srcdir Répertoire racine de la recherche.
# @return Un booléen: vrai si l'élément est un dossier vide ou dont tous les sous-dossiers vérifiés
# récursivement sont vides, faux sinon.
def folder_remove(srcdir) :
	if os.path.isfile(srcdir):
		return False
	elif not os.listdir(srcdir):
		os.rmdir(srcdir)
		return True
	elif all([folder_remove(os.path.join(srcdir,d)) for d in os.listdir(srcdir)]): #Recursive list comprehension
		os.rmdir(srcdir)
		return True
	else:
		return False

##Fonction de désarchivage et déplacement des fichiers
#
# La fonction ouvre un fichier temporaire et copie le contenu de bigfile jusqu'à la lecture d'une ligne contenant 
# les caractères séparateurs inscrits lors de l'archivage. Le nom d'origine du fichier ainsi que son chemin d'accès 
# sont alors extraits de cette ligne avec re.search. Le fichier temporaire reprend son nom d'origine et la fonction 
# le replace dans son dossier d'origine, en recréant le(s) dossier(s) du chemin d'accès s'ils n'existent pas. Un 
# nouveau fichier temporaire est ouvert et la fonction reprend la lecture de bigfile. Lorsque la lecture de celui-ci 
# est terminée, il est supprimé.
# @param[in] bigfile Fichier d'archive décompressé à déconcaténer.
def genesis(bigfile) :
	newfile=open("Huff_Temp_Name.txt", "w")	#New file to write the first original file
	with open(bigfile, "r+") as file:
		for line in file:
			#The newfile will be "split off" with its original name when a separator is read
			if "&!FILE&!" in line:
				file_and_path=re.search("&!FILE&!(.+)&!ADR&!(.+)&!SEP&!",line)
				newfile.close()
				with open("Huff_Temp_Name.txt","rb+") as eraser:	#Remove the newline added before the separator, "b" required for seek method
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
				newfile=open("Huff_Temp_Name.txt","w")	#Newfile reopened to write the next original file
			else:
				newfile.write(line)
	newfile.close()
	os.remove("Huff_Temp_Name.txt")
	os.remove(bigfile)