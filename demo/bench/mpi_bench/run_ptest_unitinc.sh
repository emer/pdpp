#!/bin/sh -f

# compiler command
#CC = "g++ -O2 -fno-exceptions -fno-rtti"

#$CC -o ptest_mpi ptest_mpi.cc -lm -lmpich -ldl -lgm

#echo $CC

# this is to prevent X11 authorization problems when running under mpi
/bin/cp -f $HOME/.ssh/rc.dream $HOME/.ssh/rc

# cpu count is the arg
#cpuCnt=$1
minUnitCnt=20
maxUnitCnt=501
incUnitCnt=5
for (( i = $minUnitCnt; i < $maxUnitCnt; i = i + $incUnitCnt )); do
    mpirun.ch_gm -np $1 -machinefile machines ./ptest_mpi $i 40000
done
 
# get rid of it
/bin/rm -f $HOME/.ssh/rc $HOME/.ssh/environment
