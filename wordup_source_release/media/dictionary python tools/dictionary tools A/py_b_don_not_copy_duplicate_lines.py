# this script is made for parsing a textfile that is
# already sorted and delete any duplicate words.
# Then write to a file.

import os
import fnmatch
import string,re

l = []
word=""
f = open('eng.txt')
for line in f.readlines():
	if(word != line):
		word = line
		l.append(word)
f.close()

fileHandle = open( 'out.txt', 'w' )
for i in range( len(l) ):
	fileHandle.write( l[i] )

fileHandle.close()
print "end"

