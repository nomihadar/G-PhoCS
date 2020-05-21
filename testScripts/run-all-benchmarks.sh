# script for running test on benchmark data
#!/bin/bash

benchmarkRoot=/home/nomihadar/benchmarks


if [ $# -lt 2 ]; then
   echo "Usage: $0 benchmarkID gphocsBin outDir [numIter]"
   echo " gphocsBinPath:  path to G-PhoCS binary to test + options (e.g. "G-PhoCS-1.3.2 -n 1") - should be in dir gphocsBinDir"
   echo " outDir:      path to output directory"
   echo " numIter:     number of MCMC iterations to use in test (optional)"
   exit
fi

gphocsBinPath=$1     # name of G-PhoCS binary to test + options (e.g. "G-PhoCS-1.3.2 -n 1") - should be in dir $gphocsBinDir (defined above)
outDir=$2        # FULL path to output directory             
numIter=$3       # number of MCMC iterations to use in test (optional)


echo $gphocsBinPath

#for each benchmark 
for benchmark in "$benchmarkRoot"/ctlFiles/*
do
	
	#echo $benchmark
	
	#get benchmark file name without the path
	benchmarkFile="$(basename "$benchmark")"
	
	#get benchmark file name without extension 
	benchmarkID="${benchmarkFile%.*}"
	
	# test id consists of benchmark id, G-PhoCS binary (including options without spaces), number of iterations (if set) and date (yymmdd)
	testID=$benchmarkID
	#-`echo "$gphocsBin" | tr -d [[:space:]]`
	if [ "$numIter" != "" ]; then
	   testID=$testID-mcmc$numIter
	fi
	testID=$testID #-`date +%d.%m.%y`  

	# create new directory for benchmark run
	mkdir -p $outDir/$testID
	cd $outDir/$testID

	# copy control file and replace root directory for sequence file, name of trace file, and optionally also number of MCMC iterations
	cat $benchmarkRoot/ctlFiles/$benchmarkFile | sed 's|BENCHMARK_ROOT|'$benchmarkRoot'|' > $benchmarkFile 
	sed 's/trace-file[[:space:]].*/trace-file   '$testID'-trace.txt/' -i $benchmarkFile

	if [ "$numIter" != "" ]; then
	   sed 's/mcmc-iterations[[:space:]].*/mcmc-iterations   '$numIter'/' -i $benchmarkFile
	fi


	# run G-PhoCS in background
	nohup $gphocsBinPath -v $benchmarkFile 1> $testID.out 2> $testID.err &
  
  
done




