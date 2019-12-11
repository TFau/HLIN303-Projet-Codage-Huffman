# HLIN303-Projet-Codage-Huffman

I.    INTERFACE & ARCHIVEUR PYTHON
  1. Le fonctionnement du programme
  2. Les fonctions
  3. Les options
  
II.   COMPRESSEUR C
III.  DECOMPRESSEUR C


###############################
###ARCHIVEUR: ARCH_PYTHON.PY###
###############################

#Modules importés: os, shutil, sys
#Fonctions importées de Arch_python_func.py

1. Le fonctionnement du programme

Le programme est lancé par le script 'Arch_python.py'. Un utilisateur peut directement lancer Cmpr_Huffman ou Decmpr_Huffman
avec un fichier en paramètre, mais cet usage est déprécié, et ne permet pas d'accéder aux options du programme ou à la
fonctionnalité d'archivage.

Le script vérifie la validité des arguments passés à la ligne de commande. Il identifie avant toute chose le fichier ou dossier
qui doit être traité. C'est l'objet de la fonction arg_parse, qui renvoie le fichier ou dossier à compresser.

Le script vérifie ensuite si les potentielles options avec la fonction option_parse, qui signale une erreur et interrompt le
programme si elle lit une option non reconnue, ou renvoie un entier encodant les options sur 3 bits.

Deux cas de figures se présentent ensuite:

CAS 1. Si l'utilisateur a lancé le programme avec un fichier en argument, il faut soit le compresser, soit le décompresser. Le
script utilise la structure de traitement des exceptions pour identifier la procédure à suivre: le fichier est ouvert et lu en
mode texte; si la lecture provoque une erreur le fichier contient du code binaire, donc il est encodé et doit être décompressé.
Sinon c'est un fichier texte que l'utilisateur veut compresser. Le script lance alors le compresseur ou le décompresseur.

CAS 2. Si l'utilisateur a spécifié un dossier en argument, il faut faire appel à la fonction d'archivage, traversal. Celle-ci
récupère tous les fichiers du dossier et de ses sous-dossiers et les concatène dans un fichier, en les séparant d'une ligne
contenant leur nom et leur chemin d'accès d'origine. Le script lance ensuite le compresseur avec le fichier concaténé comme
argument.

Une fois le fichier compressé, l'utilisateur peut arrêter le programme ou procéder immédiatement à la décompression. La
décision est prise dans le cadre de la fonction user_input.

Une fois le fichier décompressé, le script vérifie si le fichier est le résultat d'une concaténation. Si ce n'est pas le cas,
il n'y a rien à faire et le programme se termine. Sinon, la fonction genesis reconstitue chaque fichier d'origine et les
replace dans l'arborescence de dossiers, celle-ci étant recréée si nécessaire.

###############


2. Les fonctions
Modules importés: os, re, shutil, sys

arg_parse(arg_list)
###################
La fonction crée une liste contenant les éléments de arg_list qui sont des chemins existant dans le dossier courant. La liste
doit contenir exactement un élément--le fichier ou dossier à traiter--pour être validée et son contenu renvoyé, sinon une
erreur est signalée et la sortie du programme est déclenchée.

user_help()
######################
Sous-fonction de option_parse. Affiche les options disponibles.

option_parse(opt_list)
######################
La fonction traite opt_list pour récupérer les options et vérifier leur validité; une option invalide ou des caractères sauvages
déclenchent une erreur et l'arrêt prématuré du programme. option_parse appelle user_help si l'utilisateur en a fait la demande.
A chaque option correspond un des bits de poids faibles d'un entier. Ceux-ci sont mis à 1 si l'option correspondante a été
sélectionnée, et l'entier ainsi encodé est renvoyé.

text_check(file)
################
Sous-fonction de traversal. Vérifie que file est bien un fichier texte en utilisant la structure de gestion des exceptions. Si
file contient du code binaire, la fonction signale à l'utilisateur que le fichier ne sera pas concaténé dans le fichier destiné
à la compression et offre d'interrompre le programme ou non.

traversal(srcdir,destfile)
##########################
La fonction utilise os.walk(srcdir) pour obtenir tous les chemins d'accès, sous-dossiers et fichiers de l'arborescence de
dossiers ayant srcdir comme racine. Tous les fichiers munis de l'extension ".txt" sont alors vérifiés par text_check; si ce
sont bien des fichiers texte leur contenu est copié dans destfile, avec rajout de la ligne séparatrice spéciale contenant le
nom d'origine du fichier et son chemin d'accès.

user_rename(old_name,intgr)
###########################
Si l'utilisateur a sélectionné l'option -n, cette fonction lui propose de renommer le fichier compressé (si intgr=0) ou
décompressé (si intgr=1). Un nom vide ou déjà existant n'est pas accepté. Un nom valide est renvoyé.

user_input(string,optCode)
##########################
Demande à l'utilisateur s'il veut décompresser le fichier. Si l'utilisateur répond par la négative, le programme est arrêté.
Sinon la fonction lance le décompresseur avec string et optCode comme arguments.

genesis(bigfile)
################
La fonction ouvre un fichier temporaire et copie le contenu de bigfile jusqu'à la lecture d'une ligne contenant les caractères
séparateurs inscrits lors de l'archivage. Le nom d'origine du fichier ainsi que son chemin d'accès sont alors extraits de cette
ligne. Le fichier temporaire reprend son nom d'origine et la fonction le replace dans son dossier d'origine, en recréant les
dossiers s'ils n'existent pas. Un nouveau fichier temporaire est ouvert et la fonction reprend la lecture de bigfile. Lorsque
la lecture de bigfile est terminé, il est supprimé.

###############


3. Les options



