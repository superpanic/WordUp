# this script is made for parsing a textfile
# and look for word containing strange characters
# Copy all good words that does NOT contain strange characters
# Write those to a new text file.

import string

# valid chars
goodChars = [ ' ', '\n', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M' , 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' ]

# minimum number of chars allowed
minChars = 3

# maximum number of chars allowed
maxChars = 7

l = []

counter_a = 0
counter_b = 0

f = open('out.txt')
for line in f.readlines():

	counter_a = counter_a + 1
	wordvalid = 1

	# test 1
	if( len(line)-1 < minChars or len(line)-1 > maxChars ):
		wordvalid = 0
		#print(line)
	# test 2
	for char in line:
		# if one of the characters in the
		# line (word) was not in the goodChars
		# list set word valid to 0
		if( not goodChars.count(char) ):
			wordvalid = 0
			#print(char)

	if( wordvalid ):
		counter_b = counter_b + 1
		l.append(  string.upper( line )  )

f.close()

fileHandle = open('cut.txt', 'w')
for i in range( len(l) ):
	fileHandle.write( l[i] )

fileHandle.close()
print "Original word count: ", counter_a
print "Final word count: ", counter_b
print "end"
