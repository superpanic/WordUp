# this script is made for parsing a textfile from 
# words and strip all but the first word on each line.
# Then sort the result and write to a file.

import os
import fnmatch
import string,re

l = []
f = open('words.txt')
for line in f.readlines():
	l.append( line.split(',')[0] )
	
f.close()
print len(l)

fileHandle = open( 'eng.txt', 'w' )

for i in range( len(l) ):
	fileHandle.write( l[i] + "\n" )

fileHandle.close()
print "end"
