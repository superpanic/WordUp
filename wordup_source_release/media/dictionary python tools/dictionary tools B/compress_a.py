## first make a list with each character
## go through one list at a time and create secondary lists
## based on the second character
## go thourgh each of those lists and create third level lists
## based on the third character

import os
import fnmatch
import string,re
import Numeric

def chomp(s): ## cut the trailing linefeeds of a string
    if s[-2:] == '\r\n':
        return s[:-2]
    if s[-1:] == '\r' or s[-1:] == '\n':
        return s[:-1]
    return s


## valid chars
validCharList = [ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M' , 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' ]



##	{				//A
##		{"AAA", "AAB"},		//AA
##		{"ABA", "ABB"}		//AB
##	},
##	{				//B
##		{"BAA", "BAB"},		//BA
##		{"BBA", "BBB"}		//BB
##	},
##	{				//C
##		{"CAA", "CAB"},		//CA
##		{"CBA", "CBB"}		//CB
##	}

baseArray = []

dim1 = 0
dim2 = 0
dim3 = 0
dim4 = 0

##first loop, sort words using first character in charList, then next ...
for i in range( len(validCharList) ):
	f = open('cut.txt')
	## create levelOneArray
	levelOneArray = []
	for line in f.readlines(): ## go through all the lines
		if(line[0] == validCharList[i]): # if the current line starts with current char
			if( dim4 < len( chomp(line) ) ):
				dim4 = len( chomp(line) )
			levelOneArray.append( chomp(line) ) # add to i list

	if(len(levelOneArray)): ## if level one array contains any strings, add it to baseArray
		baseArray.append(levelOneArray)
	f.close()

## go through all lists of baseArray and put the 

for j in range( len(baseArray) ):
	tempArray = []
	for i in range( len(validCharList) ):
		levelTwoArray = []
		for k in range( len(baseArray[j]) ):
			if( baseArray[j][k][1] == validCharList[i] ):
				levelTwoArray.append( baseArray[j][k] )
		if(len(levelTwoArray)):
			if( dim3<len(levelTwoArray) ):
				dim3 = len(levelTwoArray)
			tempArray.append(levelTwoArray)
	baseArray[j] = tempArray

for i in range ( len(baseArray) ):
	if( dim2 < len(baseArray[i]) ):
		dim2 = len(baseArray[i])

dim1 = len(baseArray)

dimensions = "[%d][%d][%d][%d]" % (dim1, dim2, dim3, dim4+1) 
print dimensions

## dim4, +1 one is added for the trailing null character added later in the for loop
header = "const char LEXICON_ARRAY[%d][%d][%d][%d] = {" % (dim1, dim2, dim3, dim4+1)
body = ""
nWords = 0
for x in range ( len(baseArray) ):
	body = body + "{"
	for y in range ( len(baseArray[x]) ):
		body = body + "{"
		for z in range ( len(baseArray[x][y]) ):
			nWords = nWords+1
                        ## print word with trailing null character
                        ## making the array 8 chars long
			# body = body + '"' + baseArray[x][y][z] + '\0' + '"'
			body = body + '"' + baseArray[x][y][z] + '"'

			if(z < len(baseArray[x][y])-1):
				body = body + ","
		body = body + "}"
		if( y < len(baseArray[x])-1 ):
			body = body + "," + "\n"
	if( x < len(baseArray)-1 ):
		body = body + "}," + "\n"
body = body + "}"

footer = r"};"

## make body

superString = header + body + footer

## write superstring to file
fileHandle = open( 'final.txt', 'w' )
fileHandle.write( superString )
fileHandle.close()

##print superString
print(nWords)
print("end")












