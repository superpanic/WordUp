l = []

f = open('2of12inf.txt')
for line in f.readlines():
	word = line.rstrip('% \n')
#	word = filter( isAcceptableChar, line )
	l.append( word + '\n' )

f.close()

fileHandle = open( '2of12inf_out.txt', 'w' )
for i in range( len(l) ):
	fileHandle.write( l[i] )

fileHandle.close()

print "end"
