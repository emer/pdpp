#!/bin/csh -f

# replace this with full path to mpirun if needed
set mpirn = "mpirun.ch_gm -machinefile machines"
set dmlba = /usr/local/bin/dm_leabra++
echo " " >& lba_bench.out

# this is to prevent X11 authorization problems when running under mpi
/bin/cp -f $HOME/.ssh/rc.dream $HOME/.ssh/rc

# echo ======================================= >>& lba_bench.out
# echo 10 unit layers, 200 epochs >>& lba_bench.out
# echo ======================================= >>& lba_bench.out

# $mpirn -np 1 $dmlba -nogui -f startup_pbatch.css lba_dmem_10.proj.gz 200 >>& lba_bench.out
# $mpirn -np 2 $dmlba -nogui -f startup_pbatch.css lba_dmem_10.proj.gz 200 >>& lba_bench.out
# $mpirn -np 5 $dmlba -nogui -f startup_pbatch.css lba_dmem_10.proj.gz 200 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 50 unit layers, 100 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

$mpirn -np 1 $dmlba -nogui -f startup_pbatch.css lba_dmem_50.proj.gz 100 >>& lba_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_pbatch.css lba_dmem_50.proj.gz 100 >>& lba_bench.out
$mpirn -np 5 $dmlba -nogui -f startup_pbatch.css lba_dmem_50.proj.gz 100 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 100 unit layers, 25 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

$mpirn -np 1 $dmlba -nogui -f startup_pbatch.css lba_dmem_100.proj.gz 25 >>& lba_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_pbatch.css lba_dmem_100.proj.gz 25 >>& lba_bench.out
$mpirn -np 5 $dmlba -nogui -f startup_pbatch.css lba_dmem_100.proj.gz 25 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 200 unit layers, 6 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

$mpirn -np 1 $dmlba -nogui -f startup_pbatch.css lba_dmem_200.proj.gz 6 >>& lba_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_pbatch.css lba_dmem_200.proj.gz 6 >>& lba_bench.out
$mpirn -np 5 $dmlba -nogui -f startup_pbatch.css lba_dmem_200.proj.gz 6 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 500 unit layers, 1 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

$mpirn -np 1 $dmlba -nogui -f startup_pbatch.css lba_dmem_500.proj.gz 1 >>& lba_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_pbatch.css lba_dmem_500.proj.gz 1 >>& lba_bench.out
$mpirn -np 5 $dmlba -nogui -f startup_pbatch.css lba_dmem_500.proj.gz 1 >>& lba_bench.out

# get rid of it
/bin/rm -f $HOME/.ssh/rc $HOME/.ssh/environment
