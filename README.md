# HLIN303-Projet-Codage-Huffman

Compilation Compresseur: commande 'make com'

Compilation Décompresseur: commande 'make dec'

Exécution du programme: ./Arch_python.py 'fichier ou dossier'

Option --help pour quelques informations supplémentaires.

#####################


I.    INTERFACE & ARCHIVEUR PYTHON
  1. Fonctionnement du programme
  2. Les options
  3. Les fonctions
  
II.   COMPRESSEUR C
  1. Fonctionnement du programme
  2. Les fonctions

III.  DECOMPRESSEUR C
  1. Fonctionnement du programme
  2. Les fonctions



#####################ARCHIVEUR: ARCH_PYTHON.PY#####################

#Modules importés: os, sys

#Fonctions importées de Arch_python_func.py


 1. Fonctionnement du programme

Le programme est lancé par le script 'Arch_python.py'. Un utilisateur peut directement lancer Cmpr_Huffman ou Decmpr_Huffman
avec un fichier en paramètre, mais cet usage est déprécié, et ne permet pas d'accéder aux options du programme ou à la
fonctionnalité d'archivage.

Le script vérifie la validité des arguments et des options passés à la ligne de commande et arrête le programme s'il identifie une erreur à ce stade.

Deux cas de figures se présentent ensuite:

CAS 1. Si l'utilisateur a lancé le programme avec un fichier en argument, il faut soit le compresser, soit le décompresser. Le script utilise la structure de traitement des exceptions pour identifier la procédure à suivre: le fichier est ouvert et
lu en mode texte; si la lecture provoque une erreur, le fichier contient du code binaire, donc il est encodé et doit être décompressé. Sinon c'est un fichier texte que l'utilisateur veut compresser. Le script lance alors le compresseur ou le décompresseur.

CAS 2. Si l'utilisateur a passé un dossier en argument, le programme demande à l'utilisateur de spécifier l'extension des fichiers qu'il veut archiver. Les fichiers munis de cette extension sont alors concaténés dans un fichier d'archivage, avec comme séparateurs une ligne contenant le nom et le chemin d'accès d'origine du fichier précédent. Si l'option -r a été sélectionnée, les fichiers sont ensuite supprimés. Le script lance ensuite le compresseur avec le fichier d'archivage comme argument.

Une fois le fichier compressé, le script laisse à l'utilisateur le choix d'arrêter le programme ou de procéder immédiatement à la décompression.

Lorsque le fichier est décompressé, le script vérifie si le fichier est le résultat d'une concaténation. Si ce n'est pas le cas, il n'y a rien à faire et le programme se termine. Sinon, le script reconstitue chaque fichier d'origine et les
replace dans l'arborescence de dossiers, celle-ci étant recréée si nécessaire.

###############


 2. Les options
 
 --help

Afficher un bref résumé du programme et les options disponibles.

-n

Renommer le fichier compressé et/ou décompressé. Les fichiers issus d'une archive ne seront pas renommés à la décompression.

-r

Supprimer le fichier passé en argument, ou supprimer les fichiers copiés de l'arborescence du dossier passé en argument. Si l'arborescence du dossier a été entièrement vidé, la supprime également. Si le programme est lancé en compression et que l'utilisateur choisit de décompresser pendant la même exécution du programme, le fichier compressé sera également supprimé.

-c

Afficher les caractères distincts du fichiers et leurs codes respectifs.

-p

Afficher le contenu (non encodé) du fichier. Dans le cas d'une archive, affiche le contenu du fichier de concaténation.

###############


 3. Les fonctions
 
 #Modules importés: os, re, shutil, subprocess, sys

arg_parse(arg_list)

La fonction vérifie qu'un et un seul fichier ou dossier existant dans le dossier courant est présent dans arg_list. Si c'est le cas, la chaîne correspondante est renvoyée, sinon une erreur est signalée et le programme arrêté. Affiche l'aide si l'option --help a été sélectionnée.

user_help()

Sous-fonction de arg_parse. Affiche une aide sommaire et les options disponibles.

option_parse(opt_list)

La fonction traite opt_list pour récupérer les options et vérifier leur validité; une option invalide ou des caractères sauvages déclenchent une erreur et l'arrêt prématuré du programme. Aux 4 options disponibles correspondent les 4 bits de poids faible d'un entier. Un bit est mis à 1 si l'option correspondante a été sélectionnée, et après encodage l'entier est renvoyé.

text_check(file,ext)

Sous-fonction de traversal. Vérifie que file est bien un fichier texte en utilisant la structure de gestion des exceptions. Si file contient du code binaire, la fonction signale à l'utilisateur que le fichier d'extension ext ne sera pas concaténé dans le fichier destiné à la compression et offre la possibilité d'interrompre le programme.

traversal(srcdir,destfile,optCode)

La fonction utilise os.walk(srcdir) pour obtenir tous les chemins d'accès, sous-dossiers et fichiers de l'arborescence de
dossiers ayant srcdir comme racine. Tous les fichiers munis de l'extension ".txt" sont alors vérifiés par text_check; si ce
sont bien des fichiers texte leur contenu est copié dans destfile, avec rajout d'une ligne séparatrice spéciale contenant le
nom d'origine du fichier et son chemin d'accès. Si l'option -r a été sélectionnée, le fichier d'origine est supprimé. Si text_check renvoie une valeur signalant l'interruption du programme par l'utilisateur, traversal modifie optCode et le renvoie pour signaler cette interruption au programme principal.

user_rename(old_name,intgr)

Si l'utilisateur a sélectionné l'option -n, cette fonction lui propose de renommer le fichier compressé (si intgr=0) ou
décompressé (si intgr=1). Un nom vide ou déjà existant n'est pas accepté, un nom valide est renvoyé.

user_input(string,optCode)

Demande à l'utilisateur s'il veut décompresser le fichier. Si l'utilisateur répond par la négative, le programme est arrêté.
Sinon la fonction lance le décompresseur avec string et optCode passés en arguments.

folder_remove(srcdir)

Avec l'option -r, tous les fichiers copiés à partir de l'arborescence du dossier passé en argument sont supprimés par la fonction traversal. Au terme de cette phase, la fonction folder_remove vérifie récursivement si le dossier et ses sous-dossiers sont vides, et si c'est le cas, les supprime.

genesis(bigfile)

La fonction ouvre un fichier temporaire et copie le contenu de bigfile jusqu'à la lecture d'une ligne contenant les caractères séparateurs inscrits lors de l'archivage. Le nom d'origine du fichier ainsi que son chemin d'accès sont alors extraits de cette ligne avec re.search. Le fichier temporaire reprend son nom d'origine et la fonction le replace dans son dossier d'origine, en recréant le(s) dossier(s) du chemin d'accès s'ils n'existent pas. Un nouveau fichier temporaire est ouvert et la fonction reprend la lecture de bigfile. Lorsque la lecture de celui-ci est terminée, il est supprimé.



#####################COMPRESSEUR: CMPR_HUFFMAN#####################

#Fichiers sources: Com_Functions.h, Com_Functions.c, Com_Huffman.c


 1. Fonctionnement du programme

Le programme de compression est lancé par le script Python, avec en argument le fichier à traiter et l'entier codant les options.

Le compresseur lit le fichier et compte le nombre d'occurrences de chaque caractère; à partir de cette information se
calculent le nombre de caractères distincts et le nombre total de caractères dans le fichier.

Pour n caractères distincts, le programme crée un tableau de 2n-1 cases qui représentera l'arbre de Huffman: chaque caractère y est inséré comme feuille, sous la forme d'une variable de type struct dont les attributs identifient le caractère par sa valeur numérique, son parent, et sa fréquence dans le texte, ratio du nombre d'occurrences sur le nombre total de caractères.

A partir de ces caractères-feuilles, un algorithme génère l'arbre de Huffman, liant à chaque itération les deux noeuds de plus basses fréquences à un noeud parent dont la fréquence est la somme de celles de ces enfants. Une fois remplie, un second algorithme génère le code de chaque caractère en remontant de sa feuille jusqu'à la racine. Ces codes sont stockés dans un
second tableau.

Si l'option -c a été sélectionné à l'exécution du script Python, les caractères et leurs codes respectifs sont affichés.

Commence alors la phase de compression proprement dite, dans un nouveau fichier. L'encodage se fait bit par bit sur un octet, qui est écrit dans le fichier lorsque ses CHAR_BIT (défini par l'implémentation, mais typiquement 8) bits ont été fixés. L'écriture reprend ensuite sur le même octet.

Avant l'encodage du fichier sont encodés: un entier sur 4 octets contenant le nombre total de caractères dans le texte, un entier sur 1 octet contenant le nombre de caractères distincts, puis l'arbre lui-même, sous la forme d'un bit par noeud et feuille, et pour chaque feuille les CHAR_BIT bits du caractère associé.

Après encodage de l'index, le contenu du fichier est encodé. Le compresseur récupère le code de chaque caractère lu dans le fichier d'origine et le retranscrit par la méthode décrite plus haut dans un nouveau fichier. Lorsque tous les caractères ont été lus et encodés, le programme se termine.

###############


 2. Les fonctions

int freqCalc(int* T, char* textfile, unsigned char Opt)

La fonction ouvre le fichier textfile en lecture. A la lecture d'un caractère, la valeur de la case du tableau d'occurrences T correspondant au caractère est incrémenté. Si l'option -p a été sélectionnée, affiche le caractère lu sur stdout.

void distinctCalc(int* T, int* unique_char, int* total_char)

La fonction calcule à partir de T le nombre total de caractères, stocké dans total_char, et le nombre de caractères distincts, stocké dans unique_char.

void initTree(struct node* T, int size)

Fonction d'initialisation à des valeurs par défaut des attributs de chaque case du tableau T. Size sert au comptage.

void freqTree(struct node* T, int* Table, int total_char)

La fonction attribue les premières cases de T aux caractères de Table ayant au moins une occurrence, et calcule la fréquence de ces caractères. total_char sert au comptage.

void buildTree(struct node* T, int counter)

Algorithme de construction de l'arbre de Huffman. Les deux noeuds de plus basses fréquences sont liés à un parent, placé
dans une case vide de T à partir de l'indice correspondant au nombre de caractères distincts. La fréquence du parent est la somme des fréquences de ces deux enfants. Chaque nouvelle itération prendra en compte le nouveau noeud ainsi créé. Lorsqu'on ne trouve plus deux noeuds n'ayant pas de parent, l'algorithme s'arrête. counter sert au comptage.

unsigned char* extractCode(struct node* T, int i)

Sous-fonction de codeGen. Une chaîne de caractères vide est créée. A partir de la feuille i de l'arbre stocké dans T, l'algorithme remonte jusqu'à la racine. A chaque déplacement d'un enfant vers un parent, si l'enfant était à gauche le caractère '0' est ajouté à la chaîne, sinon le caractère '1' est ajouté. Lorsque la racine a été atteinte, la chaîne est réallouée dans une chaîne de taille adaptée et renvoyée. Attribue le code 1 par défaut si la feuille est racine, dans le cas où le fichier ne contient qu'un caractère distinct.

int codeGen(struct node* T, unsigned char** Table, int unique_char)

Fonction de génération des codes pour chaque caractère de T, avec un nombre d'appels à extractCode égal à unique_char. Les codes sont stockés dans Table.

char* newFile(char* oldFilename)

La fonction rajoute le préfixe "ENCODED_" à oldFilename et renvoie la chaîne résultante.

bool leftmost(int* T, int size, int n)

Sous-fonction de encodeIDX. Les indices de T correspondent aux noeuds de l'arbre de Huffman. Teste la valeur du noeud n dans T en la comparant à toutes les autres valeurs positives. Si cette valeur est la plus petite de T, n est le noeud le plus à gauche au niveau de profondeur actuel de l'arbre dans encodeIDX, et la fonction renvoie vrai. Sinon la fonction renvoie faux.

void encode(unsigned char* carrier, int* fill, unsigned char* code, int* code_read)

Fonction d'encodage appelée par encodeIDX et encodeMSG. carrier est l'octet porteur sur lequel sont encodés les caractères de la chaîne 'code', bits par bits. code_read indique la taille du code déjà lu et permet de calculer la taille du code restant à encoder en effectuant la différence avec 'code'. L'encodage s'arrête lorsque carrier est rempli (indiqué par fill) ou lorsqu'il n'y a plus de code à écrire.

void encodeCh(unsigned char* carrier, int* fill, unsigned char* symbol, int* code_read)

Fonction d'encodage appelée par encodeIDX pour encoder les caractères associés à chaque feuille de l'arbre de Huffman. Lit les bits de symbol et les encode sur carrier tant que fill et code_read sont inférieurs à CHAR_BIT.

int writeChar(FILE* writer, unsigned char* carrier, int* fill, int* bits)

Fonction d'écriture appelée par encodeIDX et encodeMSG. L'octet carrier est écrit sur le flux writer, fill est remis à zero et bits est incrémenté de CHAR_BIT.

int encodeIDX(FILE* writer, struct node* Tr, int size, unsigned char* carrier, int* fill, int* bits, int unique_char, int total_char)

Fonction d'encodage de l'index. Envoie tout d'abord le nombre total de caractères sur un entier de 4 octets, puis le nombre de caractères distincts sur 1 octet. Un tableau auxiliaire est créé à l'usage de la fonction leftmost, avec toutes ses valeurs initialisées à -1 sauf la racine de Tr à 0. L'algorithme part de la racine et encode les noeuds de l'arbre par niveau de profondeur, et de gauche à droite. Les noeuds sont encodés sur un bit par la fonction encode: 0 pour un noeud interne, 1 pour une feuille. Après le bit d'une feuille sont encodés les CHAR_BIT bits du caractère correspondant par appel à la fonction encodeCh. Lorsque fill=CHAR_BIT, l'octet carrier est rempli et la fonction appelle writeChar pour écrire sur le flux writer.

int encodeMSG(FILE* writer, FILE* reader, unsigned char** Table, unsigned char* carrier, int* fill, int* bits, int total_char)

Fonction d'encodage du message. A la lecture d'un caractère sur le flux reader, la fonction récupère le code correspondant dans Table, et appelle la fonction encode pour écrire ce code bit à bit sur carrier. Lorsque carrier est rempli, la fonction appelle writeChar pour écrire sur le flux writer.



#####################DECOMPRESSEUR: DECMPR_HUFFMAN#####################

#Fichiers sources: Dec_Functions.h, Dec_Functions.c, Dec_Huffman.c


 1. Fonctionnement du programme

Le programme de decompression est lancé par le script Python, soit à la suite de la compression si l'utilisateur le choisit, soit à la lecture d'un fichier binaire passé en argument.

Le décompresseur ouvre le fichier encodé en lecture binaire et lit tout d'abord l'entier sur 4 octets qui indique le nombre total de caractères, et l'entier sur 1 octet indiquant le nombre de caractères distincts. Ces informations lui permettent de recréer un tableau représentant l'arbre de Huffman du texte encodé.

L'index est décodé et les caractères récupérés sont mis à leur place dans le tableau. Sachant que l'arbre a été encodé en commençant par la racine et en procédant dans un ordre particulier (cf. partie II), le décompresseur reconstruit alors l'arbre en reconstituant les liens parents-enfants d'origine.

Après avoir reconstitué l'arbre, si l'option -c a été sélectionné, le décompresseur régénère les codes pour les afficher.

Le programme s'attaque alors à la décompression du message. L'octet porteur reçoit l'octet lu à partir du fichier encodé, et les bits du porteur sont lus et utilisés pour parcourir l'arbre à partir de sa racine. Lorsque le parcours abouti à une feuille, le caractère associé est alors écrit dans le fichier décodé.

Lorsque le décompresseur a décodé un nombre de caractères égal au nombre total de caractères indiqué par l'index, le programme s'arrête.

###############


 2. Les fonctions
 
int readStart(FILE* reader, int* total_char, int* unique_char)
 
La fonction récupère le nombre total de caractères sur le flux reader et stocke cette valeur dans total_char, et récupère le nombre de caractères distincts, stocké dans unique_char.

void initTree(struct node* T, int size)

Fonction d'initialisation à des valeurs par défaut des attributs de chaque case du tableau T. La structure node dans le décompresseur ne possède pas d'attribut stockant la fréquence. Size sert au comptage.

unsigned char decodeSingle(unsigned char* carrier, int* fill)

Sous-fonction de decIDXmain. Fonction simplifiée décodant un bit de carrier.

unsigned char decodeIDX(unsigned char* carrier, int* fill, unsigned char* T_byte, int* code_read)

Sous-fonction de decIDXmain. Lit les bits de carrier et les écrit sur T_byte pour retrouver le caractère associé à une feuille. Renvoie T_byte lorsque carrier a été lu en entier ou T_byte est rempli.

int readChar(FILE* reader, unsigned char* carrier, int* fill)

Fonction de lecture appelée par decIDXmain et decMSGmain. L'octet lu sur le flux reader est stocké dans carrier et fill est remis à zero.

int decIDXmain(FILE* reader, struct node* T, unsigned char* carrier, int* fill, int size)

Fonction de décodage de l'index. Les octets lus sur le flux reader sont chargés sur l'octet porteur carrier. Lorsque tous les bits de carrier ont été lus, la fonction appelle readChar pour récupérer un nouvel octet sur reader. La fonction appelle decodeSingle pour décoder les noeuds de l'arbre. La lecture du bit 1 indique une feuille, et la fonction appelle decodeIDX pour lire les CHAR_BIT bits suivants et retrouver le caractère associé à la feuille et le stocker dans le tableau de l'arbre de Huffman.

void buildTree(struct node* T, int size)

Fonction de reconstruction de l'arbre de Huffman. Les arbres de Huffman ne sont pas des arbres complets et donc il n'y a pas une formule unique pour relier parents et enfants. Mais sachant que l'arbre a été encodé de haut en bas et de gauche à droite, à partir d'un niveau de profondeur, on peut connaître le nombre de noeuds à traiter au niveau suivant: 2 pour chaque noeud interne au niveau courant. En tenant compte des noeuds à traiter à un certain niveau, des noeuds déjà traités et des noeuds aux niveau suivant, la fonction calcule et alloue les enfants de chaque noeud interne et ainsi reconstitue l'arbre.

unsigned char* extractCode(struct node* T, int i)

Sous-fonction de codeGen. Identique à la fonction extractCode du compresseur.

int deCodeGen(struct node* T, unsigned char** Table, int size)

Fonction de génération des codes pour chaque caractère de T, avec un nombre d'appels à extractCode égal à size de T (légère différence avec la fonction codeGen du compresseur). Les codes sont stockés dans Table. N'est appelée qu'avec l'option -c.

char* newFile(char* oldFilename)

La fonction rajoute le préfixe "DECODED_" à oldFilename et renvoie la chaîne résultante.

int decodeMSG(unsigned char* carrier, struct node* T, int* fill, int* pos)

Sous-fonction de decMSGmain. Lis les bits de carrier tant que fill est inférieur à CHAR_BIT et parcourt l'arbre de Huffman stocké dans T en fonction des bits lus, en utilisant pos comme indice. Si une feuille est rencontrée avant la fin de la lecture de carrier, le caractère associé est renvoyé, sinon la fonction retourne -1.

int decMSGmain(FILE* reader, FILE* writer, struct node* Tree, unsigned char* carrier, unsigned char Opt, int* fill, int total_char)

Fonction de décodage du message. A la lecture d'un caractère sur le flux reader, appelle la fonction decodeMSG en lui passant l'octer carrier pour parcourir l'arbre de Huffman stocké dans Tree et retrouver les caractères encodés. Lorsque tous les bits de carrier ont été lu, appelle readChar pour récuperer un nouvel octet de reader. Lorsque decodeMSG renvoie une valeur positive, la fonction l'écrit sur le flux writer. Si l'option -p a été sélectionnée, le caractère est également affiché sur stdout.



