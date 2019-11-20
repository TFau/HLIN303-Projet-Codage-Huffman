#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import os, re, sys

#srcdir to keep track of where to copy the files, created by the program (so no program parameter)
def traversal(srcdir, drctory, comp_list) :
	dirlist=os.listdir(drctory)
	for fil in dirlist:
		if os.access(drctory+"/"+fil,os.R_OK):	#Path readability
			if not os.path.isdir(drctory+"/"+fil):	#Don't check file extensions on directories
				ext=fil.split(".")
				if len(ext) > 1:
					if ext[-1] == "txt":
						comp_list.append(fil)
						os.system("cp "+drctory+"/"+fil+" "+srcdir+"/Huff_Files_To_Compress")
			else:
				traversal(srcdir,drctory+"/"+fil,comp_list)


os.system("mkdir Huff_Files_To_Compress")	#Directory where the files to compress are copied before concatenation
Files_To_Compress=[] #List containing all files obtained from traversal
#getcwd() returns current working directory
traversal(os.getcwd(),sys.argv[1],Files_To_Compress)
#Building command line string from the file list
string=""
for f in Files_To_Compress:
	string+=f+" "
os.system("touch Huff_To_Compress.txt") #Creating concatenation file
os.system("cd Huff_Files_To_Compress; cat "+string+" > Huff_To_Compress.txt") #Concatenate all files into the main file