# script for comparing results of benchmark tests
#!/bin/bash

if [ $# -ne 2 ]; then
   echo "Usage: $0 testRoot testID1 testID2"
   echo " testRoot1:     root directory of output 1"
   echo " testRoot2:     root directory of output 2"
   exit
fi
testRoot1=$1
testRoot2=$2

declare -a benchmarks=("benchmark1" 
				"benchmark2" 
				"benchmark2b" 
				"benchmark2c" 
				"benchmark2d" 
				"benchmark2e" 
				"benchmark2e" 
				"benchmark2f" 
				"benchmark2g" 
				"benchmark3" 
				"benchmark3a" 
				"benchmark3b" 
				"benchmark3c" 
				"benchmark3d" 
				"benchmark3e" 
				"benchmark3f" 
				"benchmark3g")

for benchmark in "${benchmarks[@]}"
do
	echo "$benchmark"

	# id of comparison between run $id1 and run $id2
	#compID=${benchmarkID1}___VS___${benchmarkID2}
	outfile=$benchmark.out
	tmpfile=$benchmark.tmp

	trace1=$testRoot1/$benchmark/$benchmark-trace.txt
	trace2=$testRoot2/$benchmark/$benchmark-trace.txt

	#if [ ! -r $trace1 ] ; then
	#   echo "run $testID1 does not exist in $testRoot"
	#   exit 1
	#fi
	#if [ ! -r $trace2 ] ; then
	#   echo "run $testID2 does not exist in $testRoot"
	#   exit 1
	#fi

	echo "Comparing run $testID1 with run $testID on " > $outfile
	echo >> $outfile

	ncol1=`head -n1 $trace1 | awk '{print NF}'`
	ncol2=`head -n1 $trace2 | awk '{print NF}'`

	if [ $ncol1 -ne $ncol2 ]; then
	   echo "number of trace columns, $testID1: $ncol1, $testID2: $ncol2" >> $outfile
	   # exit 1  DO NOT EXIT !!!!
	else
	   echo "number of trace columns okay ($ncol1)" >> $outfile
	fi

	nrow1=`wc -l $trace1 | cut -f1 -d" "`
	nrow2=`wc -l $trace2 | cut -f1 -d" "`

	if [ $nrow1 -ne $nrow2 ]; then
	   echo "number of trace rows, $testID1: $nrow1, $testID2: $nrow2" >> $outfile
	   # exit 1  DO NOT EXIT !!!!
	else
	   echo "number of trace rows okay ($nrow1)" >> $outfile
	fi


	for i in `seq 1 $ncol1`; do
	   col=`head -n1 $trace1 | cut -f$i`
	   cat $trace1 | tr -s "\t" | cut -f$i  | tail -n+2 > $tmpfile
	   maxdiff=`cat $trace2 | tr -s "\t" | cut -f$i | tail -n+2 | paste $tmpfile - | \
		 awk 'BEGIN{max=0;} \
		 { \
		  if(NF>1) { \
			if($2-$1>max) { max=$2-$1; } \
			if($1-$2>max) { max=$1-$2; } \
		  }\
		 } END{print max;}'`

	   echo "Col $col, max diff = $maxdiff" >> $outfile
	done

	rm $tmpfile

	cat $outfile

   
done

