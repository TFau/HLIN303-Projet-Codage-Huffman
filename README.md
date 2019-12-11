# HLIN303-Projet-Codage-Huffman

I.    INTERFACE & ARCHIVEUR PYTHON
  1. Fonctionnement du programme
  2. Les fonctions
  3. Les options
  
II.   COMPRESSEUR C
  1. Fonctionnement du programme
  2. Les fonctions

III.  DECOMPRESSEUR C
  1. Fonctionnement du programme
  2. Les fonctions



#####################ARCHIVEUR: ARCH_PYTHON.PY#####################

#Modules importés: os, shutil, sys

#Fonctions importées de Arch_python_func.py


1. Fonctionnement du programme

Le programme est lancé par le script 'Arch_python.py'. Un utilisateur peut directement lancer Cmpr_Huffman ou Decmpr_Huffman
avec un fichier en paramètre, mais cet usage est déprécié, et ne permet pas d'accéder aux options du programme ou à la
fonctionnalité d'archivage.

Le script vérifie la validité des arguments passés à la ligne de commande. Il identifie avant toute chose le fichier ou dossier qui doit être traité. C'est l'objet de la fonction arg_parse, qui renvoie le fichier ou dossier à compresser.

Le script vérifie ensuite les potentielles options avec la fonction option_parse, qui signale une erreur et interrompt le
programme si elle lit une option non reconnue, ou renvoie un entier encodant les options sur 3 bits.

Deux cas de figures se présentent ensuite:

CAS 1. Si l'utilisateur a lancé le programme avec un fichier en argument, il faut soit le compresser, soit le décompresser. Le script utilise la structure de traitement des exceptions pour identifier la procédure à suivre: le fichier est ouvert et
lu en mode texte; si la lecture provoque une erreur, le fichier contient du code binaire, donc il est encodé et doit être décompressé. Sinon c'est un fichier texte que l'utilisateur veut compresser. Le script lance alors le compresseur ou le décompresseur.

CAS 2. Si l'utilisateur a passé un dossier en argument, il faut faire appel à la fonction d'archivage, traversal. Celle-ci
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
#Modules importés: os, re, shutil, sys

arg_parse(arg_list)

La fonction crée une liste contenant les éléments de arg_list qui sont des chemins existant dans le dossier courant. La liste
doit contenir exactement un élément--le fichier ou dossier à traiter--pour être validée et son contenu renvoyé, sinon une
erreur est signalée et la sortie du programme est déclenchée.

user_help()

Sous-fonction de option_parse. Affiche les options disponibles.

option_parse(opt_list)

La fonction traite opt_list pour récupérer les options et vérifier leur validité; une option invalide ou des caractères sauvages déclenchent une erreur et l'arrêt prématuré du programme. option_parse appelle user_help si l'utilisateur en a fait la demande. A chaque option correspond un des bits de poids faibles d'un entier. Ceux-ci sont mis à 1 si l'option correspondante a été sélectionnée, et l'entier ainsi encodé est renvoyé.

text_check(file)

Sous-fonction de traversal. Vérifie que file est bien un fichier texte en utilisant la structure de gestion des exceptions. Si file contient du code binaire, la fonction signale à l'utilisateur que le fichier ne sera pas concaténé dans le fichier destiné à la compression et offre la possibilité d'interrompre le programme.

traversal(srcdir,destfile)

La fonction utilise os.walk(srcdir) pour obtenir tous les chemins d'accès, sous-dossiers et fichiers de l'arborescence de
dossiers ayant srcdir comme racine. Tous les fichiers munis de l'extension ".txt" sont alors vérifiés par text_check; si ce
sont bien des fichiers texte leur contenu est copié dans destfile, avec rajout d'une ligne séparatrice spéciale contenant le
nom d'origine du fichier et son chemin d'accès.

user_rename(old_name,intgr)

Si l'utilisateur a sélectionné l'option -n, cette fonction lui propose de renommer le fichier compressé (si intgr=0) ou
décompressé (si intgr=1). Un nom vide ou déjà existant n'est pas accepté, un nom valide est renvoyé.

user_input(string,optCode)

Demande à l'utilisateur s'il veut décompresser le fichier. Si l'utilisateur répond par la négative, le programme est arrêté.
Sinon la fonction lance le décompresseur avec string et optCode comme arguments.

genesis(bigfile)

La fonction ouvre un fichier temporaire et copie le contenu de bigfile jusqu'à la lecture d'une ligne contenant les caractères séparateurs inscrits lors de l'archivage. Le nom d'origine du fichier ainsi que son chemin d'accès sont alors extraits de cette ligne. Le fichier temporaire reprend son nom d'origine et la fonction le replace dans son dossier d'origine, en recréant les dossiers s'ils n'existent pas. Un nouveau fichier temporaire est ouvert et la fonction reprend la lecture de bigfile. Lorsque la lecture de celui-ci est terminée, il est supprimé.

###############


3. Les options

--help

Afficher les options disponibles.

-n

Renommer le fichier compressé ou décompressé. Les fichiers issus de la déconcaténation d'un fichier-archive décompressé ne peuvent pas être renommés.

-r

Supprimer le fichier ou dossier d'origine. Si la décompression est faite en suivant la compression, le fichier compressé sera également effacé. Le fichier de concaténation est toujours supprimé après déconcaténation.

-c

Afficher les caractères distincts du fichiers et leurs codes respectifs.



#####################COMPRESSEUR: CMPR_HUFFMAN#####################

#Bibliothèques utilisées: limits, stdbool, stdio, stdlib, string


1. Fonctionnement du programme

Le programme de compression est lancé par le script Python, avec en argument le fichier à traiter et l'entier codant les options.

Le compresseur lit le fichier et compte le nombre d'occurrences de chaque caractère; à partir de cette information se
calculent le nombre de caractères distincts et le nombre total de caractères dans le fichier.

Pour n caractères distincts, un tableau de 2n-1 cases est créé. Ce tableau représentera l'arbre de Huffman: chaque caractère
y est inséré comme feuille, sous la forme d'une variable de type struct dont les attributs identifient le caractère par sa
valeur numérique, son parent, et sa fréquence dans le texte, ratio du nombre d'occurrences sur le nombre total de caractères.

A partir de ces caractères-feuilles, un algorithme génère l'arbre de Huffman, liant à chaque itération les deux noeuds de plus basses fréquences à un noeud parent dont la fréquence est la somme de celles de ces enfants. Une fois remplie, un second algorithme génère le code de chaque caractère en remontant de sa feuille jusqu'à la racine. Ces codes sont stockés dans un
second tableau.

Si l'option -c a été sélectionné à l'exécution du script Python, les caractères et leurs codes respectifs sont affichés.

Commence alors la phase de compression proprement dite, dans un nouveau fichier. L'encodage se fait bit par bit sur un octet, qui est écrit dans le fichier lorsque ses CHAR_BIT (défini par l'implémentation, mais typiquement 8) bits ont été fixés. L'écriture reprend ensuite sur le même octet.

Avant l'encodage du fichier sont encodés: un entier sur 4 octets contenant le nombre total de caractères dans le texte, un entier sur un octet contenant le nombre de caractères distincts, puis l'arbre lui-même, sous la forme d'un bit par noeud et feuille, et pour chaque feuille les 8 bits du caractère associé.

Après encodage de l'index, le contenu du fichier est encodé. Le programme récupère le code de chaque caractère lu dans le fichier d'origine et retranscrit ce code bit par bit sur l'octet porteur. Lorsque tous les caractères ont été lus et encodés, le programme se termine.

###############


2. Les fonctions

int freqCalc(int* T, char* textfile)

La fonction ouvre le fichier indiqué par textfile en lecture. A la lecture d'un caractère, la valeur de la case du tableau
d'occurrences T correspondant au caractère est incrémenté.

int distinctCalc(int* T, char* textfile, int* unique_char, int* total_char)

La fonction calcule à partir de T le nombre total de caractères, stocké dans total_char, et le nombre de caractères distincts, stocké dans unique_char. Si ce dernier est égal à un, le fichier textfile est ouvert en ajout-écriture et un caractère <newline> est rajouté à la fin (cf. norme POSIX.1-2017 3.206).
  
void initTree(struct node* Tree, int size)

Fonction d'initialisation à des valeurs par défaut des attributs de chaque case du tableau Tree. Size sert au comptage.

void freqTree(struct node* T, int* Table, int total_char)

La fonction attribue les premières cases de T aux caractères de Table ayant au moins une occurrence, et calcule la fréquence de ces caractères. total_char sert au comptage.

void buildTree(struct node* T, int counter)

Algorithme de construction de l'arbre de Huffman. Les deux noeuds de plus basses fréquences sont liés à un parent, placé
dans une case vide de T à partir de l'indice correspondant au nombre de caractères distincts. La fréquence du parent est la somme des fréquences de ces deux enfants. Chaque nouvelle itération prendra en compte le nouveau noeud ainsi créé. Lorsqu'on ne trouve plus deux noeuds n'ayant pas de parent, l'algorithme s'arrête. counter sert au comptage.

unsigned char* extractCode(struct node* T, int i)

Sous-fonction de codeGen. Une chaîne de caractères vide est créée. A partir de la feuille i de l'arbre stocké dans T, l'algorithme remonte jusqu'à la racine. A chaque déplacement d'un noeud x vers un noeud y, si x est l'enfant gauche de y le caractère '0' est ajouté à la chaîne, si x est l'enfant droit le caractère '1' est ajouté. Lorsque la racine a été atteinte, la chaîne est réalloué dans une chaîne de taille adaptée et renvoyée.

int codeGen(struct node* T, unsigned char** Table, int unique_char)

Fonction de génération des codes pour chaque caractère de T, avec un nombre d'appels à extractCode égal à unique_char. Les codes sont stockés dans Table.

char* newFile(char* oldFilename)

La fonction rajoute le préfixe "ENCODED_" à oldFilename et renvoie la chaîne résultante.

bool leftmost(int* T, int size, int n)

Sous-fonction de encodeIDX. Les indices de T correspondent aux noeuds de l'arbre de Huffman. Teste la valeur du noeud n dans T en la comparant à toutes les autres valeurs positives. Si cette valeur est la plus petite de T, n est le noeud le plus à gauche au niveau de profondeur actuel de l'arbre dans encodeIDX, et la fonction renvoie vrai. Sinon la fonction renvoie faux.

unsigned char* binaryChar(unsigned char n)

Sous-fonction de encodeIDX. Convertit le caractère n en chaîne de caractères stockant sa valeur binaire pour qu'il puisse être lu par la fonction d'encodage.

void encode(unsigned char* carrier, int* fill, unsigned char* code, int* code_read)

Fonction d'encodage appelée par encodeIDX et encodeMSG. carrier est l'octet porteur sur lequel sont encodés les caractères de la chaîne 'code', bits par bits. fill mesure le taux de remplissage de carrier. code_read indique la taille du code déjà lu et permet de calculer la taille du code restant à encoder en effectuant la différence avec 'code'.

int writeChar(FILE* writer, unsigned char* carrier, int* fill, int* bits)

Fonction d'écriture appelée par encodeIDX et encodeMSG. L'octet carrier est écrit sur le flux writer, fill est remis à zero et bits est incrémenté de CHAR_BIT.

int encodeIDX(FILE* writer, struct node* Tr, int size, unsigned char* carrier, int* fill, int* bits, int unique_char, int total_char)

Fonction d'encodage de l'index. Envoie tout d'abord le nombre total de caractères sur un entier de 4 octets, puis le nombre de caractères distincts sur un octet. Un tableau auxiliaire est créé à l'usage de la fonction leftmost, avec toutes ses valeurs initialisées à -1 sauf la racine de Tr à 0. L'algorithme part de la racine et encode les noeuds de l'arbre par niveau de profondeur, et de gauche à droite. Tous les encodages se font par appel à la fonction encode. Les noeuds sont encodés sur un bit: 0 pour un noeud interne, 1 pour une feuille. Après le bit d'une feuille sont encodés les CHAR_BIT bits du caractère correspondant, après conversion du caractère en chaîne par binaryChar. Lorsque fill=CHAR_BIT, l'octet carrier est rempli et la fonction appelle writeChar pour écrire sur le flux writer.

int encodeMSG(FILE* writer, FILE* reader, unsigned char** Table, unsigned char* carrier, int* fill, int* bits, int total_char)

Fonction d'encodage du message. A la lecture d'un caractère sur le flux reader, la fonction récupère le code correspondant dans Table, et fait appel à encode pour écrire ce code bit à bit sur carrier. Lorsque carrier est rempli, la fonction appelle writeChar pour écrire sur le flux writer.
