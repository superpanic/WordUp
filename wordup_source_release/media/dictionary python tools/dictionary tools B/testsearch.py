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

for i in allWords:
	if wordCounter == 102:
		print chomp(i)
		print wordCounter
		print " "
	"""
	if chomp(i) == "TEA":
		print "TEA"
		print wordCounter
	"""
	wordCounter = wordCounter + 1




