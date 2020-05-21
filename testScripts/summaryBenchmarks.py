import os, sys
import re
import collections
import pandas as pd

#get path to benchmarks output
if len(sys.argv) > 1:
	benchmarks_output = sys.argv[1]

print
print "Total num of benchmarks: ", len(os.listdir(benchmarks_output))


A = "File"
B = "Errors"
C = "Num MCMC iterations"
D = "Num iterations in trace"
E = "Time"

titles = [A, B, C, D, E]
d = {t:[] for t in titles}


for root, dirs, files in os.walk(benchmarks_output):
	for file in files:

		#get file path
		path = os.path.join(root, file) 
		
		if path.endswith(".err"):
			#if file is not empty
			if os.stat(path).st_size != 0:
				d[B].append("yes")
			else:
				d[B].append("no")
				
		if path.endswith("trace.txt"):
			ncompleted = sum(1 for line in open(path))-1
			d[D].append(ncompleted)

		if path.endswith(".ctl"):
			nIterations = re.search('mcmc-iterations\s*(\d+)', open(path).read()).group(1)
			d[C].append(nIterations)
			
		if path.endswith(".out"):
			d[A].append(file)
			m = re.search('Time used:\s*(.*)', open(path).read())
			if m:
				time = m.group(1)
			else:
				time = 'NA'
			d[E].append(time)
			
			

df = pd.DataFrame(d)
df = df[titles]
df = df.sort_values(by=A)

print df
print 
df.to_csv("benchmarks_summary.csv",index=False)

	
	
	