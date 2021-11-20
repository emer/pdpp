#!/bin/csh -f

# compiler command
set CC = "g++ -O2 -fno-exceptions -fno-rtti"

$CC -o ptest_mpi ptest_mpi.cc -lm -lmpich -ldl -lgm

echo $CC

# this is to prevent X11 authorization problems when running under mpi
/bin/cp -f $HOME/.ssh/rc.dream $HOME/.ssh/rc

mpirun.ch_gm -np 1 -machinefile machines ./ptest_mpi 50 400000
mpirun.ch_gm -np 2 -machinefile machines ./ptest_mpi 50 400000
mpirun.ch_gm -np 4 -machinefile machines ./ptest_mpi 50 400000
mpirun.ch_gm -np 8 -machinefile machines ./ptest_mpi 50 400000
mpirun.ch_gm -np 16 -machinefile machines ./ptest_mpi 50 400000
mpirun.ch_gm -np 25 -machinefile machines ./ptest_mpi 50 400000

mpirun.ch_gm -np 1 -machinefile machines ./ptest_mpi 100 100000
mpirun.ch_gm -np 2 -machinefile machines ./ptest_mpi 100 100000
mpirun.ch_gm -np 4 -machinefile machines ./ptest_mpi 100 100000
mpirun.ch_gm -np 8 -machinefile machines ./ptest_mpi 100 100000
mpirun.ch_gm -np 16 -machinefile machines ./ptest_mpi 100 100000
mpirun.ch_gm -np 25 -machinefile machines ./ptest_mpi 100 100000

mpirun.ch_gm -np 1 -machinefile machines ./ptest_mpi 500 4000
mpirun.ch_gm -np 2 -machinefile machines ./ptest_mpi 500 4000
mpirun.ch_gm -np 4 -machinefile machines ./ptest_mpi 500 4000
mpirun.ch_gm -np 8 -machinefile machines ./ptest_mpi 500 4000
mpirun.ch_gm -np 16 -machinefile machines ./ptest_mpi 500 4000
mpirun.ch_gm -np 25 -machinefile machines ./ptest_mpi 500 4000

mpirun.ch_gm -np 1 -machinefile machines ./ptest_mpi 1000 1000
mpirun.ch_gm -np 2 -machinefile machines ./ptest_mpi 1000 1000
mpirun.ch_gm -np 4 -machinefile machines ./ptest_mpi 1000 1000
mpirun.ch_gm -np 8 -machinefile machines ./ptest_mpi 1000 1000
mpirun.ch_gm -np 16 -machinefile machines ./ptest_mpi 1000 1000
mpirun.ch_gm -np 25 -machinefile machines ./ptest_mpi 1000 1000

mpirun.ch_gm -np 1 -machinefile machines ./ptest_mpi 5000 40
mpirun.ch_gm -np 2 -machinefile machines ./ptest_mpi 5000 40
mpirun.ch_gm -np 4 -machinefile machines ./ptest_mpi 5000 40
mpirun.ch_gm -np 8 -machinefile machines ./ptest_mpi 5000 40
mpirun.ch_gm -np 16 -machinefile machines ./ptest_mpi 5000 40
mpirun.ch_gm -np 25 -machinefile machines ./ptest_mpi 5000 40

# get rid of it
/bin/rm -f $HOME/.ssh/rc $HOME/.ssh/environment
