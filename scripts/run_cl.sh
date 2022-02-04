
srcdir=$(dirname $0)
workdir=$(pwd)

cd $srcdir/CMSSW_10_2_13/src
eval `scram runtime -sh`
cd $workdir

echo "run_cl with command \""$*"\""
time $*
