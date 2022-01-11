"""

create three arrays:
1.  create a list of indexes for each change of the first three characters.
2.  create a second list containing the three characters.
3.  create a list of all words with the first three characters stripped.
if a word has only three charactes, the slot is empty.

"""

#import os
#import fnmatch
import string,re
#import Numeric

def chomp(s): ## cut the trailing linefeeds of a string
	if s[-2:] == '\r\n':
		return s[:-2]
	if s[-1:] == '\r' or s[-1:] == '\n':
		return s[:-1]
		return s

## valid chars
charList = [ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M' , 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' ]

f = open('cut.txt')


# first put all the lines in an array
allWords = []
i = 0
for line in f:
	allWords.append(line)
	i = i+1
f.close()


# make three nested loops
indexArray = []
charsArray = []

cropLength = 2

# first outer loop
wordCounter = 0
indexArray.append(wordCounter)
lastWord = allWords[0]
charsArray.append( lastWord[:cropLength] )

# wordCounter = wordCounter +1

for i in allWords:
	if( i[:cropLength] != lastWord[:cropLength] ):
		indexArray.append(wordCounter)
		lastWord = i
		charsArray.append( lastWord[:cropLength] )
	wordCounter = wordCounter + 1







# create and write the main lexicon array

header = "const char LEXICON_ARRAY[%d][%d] = { " % ( len( allWords ), 8 - cropLength )
body = ""


counter = 0
strippedWords = []
for i in allWords:
	strippedWords.append(i[2:-1])
	body = body + '"' + i[2:-1] + '"'
	if( counter < len(allWords)-1 ):
		body = body + ", "
	counter = counter+1
	
footer = " }"

superString = header + body + footer

fh = open( 'stripped.txt', 'w' )
fh.write( superString )
fh.close()






# create and write the leading characters array

header = "const char LEADING_CHARS_ARRAY[%d][%d] = { " % (  len(charsArray), cropLength + 1 )
body = ""

counter = 0
for i in charsArray:
	body = body + '"' + i + '"'
	if(counter < len(charsArray)-1):
		body = body + ", "
	counter = counter + 1

footer = "}"

superString = header + body + footer

fh = open( 'leadchars.txt', 'w' )
fh.write( superString )
fh.close()







# create and write the index array

header = "const u16 LEXICON_INDEX_ARRAY[%d] = { " % ( len(indexArray) )
body = ""

counter = 0
for i in indexArray:
	body = body + str(i)
	if( counter < len(indexArray)-1 ):
		body = body + ", "
	counter = counter+1

footer = "}"

superString = header + body + footer

fh = open( 'indexarray.txt', 'w' )
fh.write( superString )
fh.close()

print("end")


"""
print( "Total number of words: " + str(len(allWords)) )

print("---")
print("---")
print( str( len( charsArray ) ) )
print( charsArray )
print("---")
print( str( len( indexArray ) ) )
print( indexArray )
print("---")
print("---")
"""
