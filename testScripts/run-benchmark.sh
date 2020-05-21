# script for running test on benchmark data
#!/bin/bash

benchmarkRoot=/home/nomihadar/benchmarks
#gphocsBinDir=/home/nomihadar/G-PhoCS_dev
if [ $# -lt 3 ]; then
   echo "Usage: $0 benchmarkID gphocsBin outDir [numIter]"
   echo " benchmarkID: id of benchmark (e.g. benchmark2b)                  - should be name of control file in ctlFiles" 
   echo " gphocsBinPath:  path to G-PhoCS binary to test + options (e.g. "G-PhoCS-1.3.2 -n 1") - should be in dir gphocsBinDir"
   echo " outDir:      path to output directory"
   echo " numIter:     number of MCMC iterations to use in test (optional)"
   exit
fi

benchmarkID=$1   # id of benchmark (e.g. benchmark2b)                  - should be name of control file in ctlFiles
gphocsBinPath=$2     # name of G-PhoCS binary to test + options (e.g. "G-PhoCS-1.3.2 -n 1") - should be in dir $gphocsBinDir (defined above)
outDir=$3        # path to output directory             
numIter=$4       # number of MCMC iterations to use in test (optional)

gphocsBin="$(basename $gphocsBinPath)"

#gphocsBinPath="$gphocsBinDir/$gphocsBin"
#gphocsBin=`readlink -f  $gphocsBinDir/$2`
echo $gphocsBinPath
echo $gphocsBin
# test id consists of benchmark id, G-PhoCS binary (including options without spaces), number of iterations (if set) and date (yymmdd)
testID=$benchmarkID-`echo "$gphocsBin" | tr -d [[:space:]]`
if [ "$numIter" != "" ]; then
   testID=$testID-mcmc$numIter
fi
testID=$testID-`date +%y%m%d`

# create new directory for benchmark run
mkdir -p $outDir/$testID
cd $outDir/$testID

# copy control file and replace root directory for sequence file, name of trace file, and optionally also number of MCMC iterations
cat $benchmarkRoot/ctlFiles/$benchmarkID.ctl | sed 's|BENCHMARK_ROOT|'$benchmarkRoot'|' > $benchmarkID.ctl 
sed 's/trace-file[[:space:]].*/trace-file   '$testID'-trace.txt/' -i $benchmarkID.ctl

if [ "$numIter" != "" ]; then
   sed 's/mcmc-iterations[[:space:]].*/mcmc-iterations   '$numIter'/' -i $benchmarkID.ctl
fi

# run G-PhoCS in background

nohup $gphocsBinPath -v $benchmarkID.ctl 1> $testID.out 2> $testID.err &
