'''
Compare traces 
'''
import os, sys
import pandas as pd

pd.set_option('display.float_format', lambda x: '%.5f' % x)

#INPUT: path of benchmark A, path of benchmark B
benchA_path = sys.argv[1]
benchB_path = sys.argv[2]

print
print "Trace file A: ", benchA_path
print "Trace file B: ", benchB_path
print 
benchA = pd.read_csv(benchA_path, sep='\s+')
benchB = pd.read_csv(benchB_path, sep='\s+')

print "Length of bench A: ", len(benchA)
print "Length of bench B: ", len(benchB)

if len(benchB) < len(benchA):
	benchA = benchA.head(len(benchB))
	
benchB.rename(columns={'Complete-ld-ln-avg': 'Data-ld-ln', 
			'Data-ld-ln': 'Full-ld-ln'}, inplace=True)
benchB.drop('Gene-ld-ln', 1, inplace=True)

#print (benchA)
#print (benchB)

equal = benchA.equals(benchB)
print "\nAre traces equal?\n", equal

if (not equal):	

	print "\n\nDiffrences: \n"

	bool = (benchA != benchB).stack()  # Create Frame of comparison booleans
	diff = pd.concat([benchA.stack()[bool], benchB.stack()[bool]], axis=1)
	diff.columns=["Trace A", "Trace B"]
	
	print(diff.head(20))
	a = list(set(diff.head(500).index.get_level_values(1)))
	print(a)
	

