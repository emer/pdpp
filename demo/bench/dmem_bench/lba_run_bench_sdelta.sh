#!/bin/csh -f

# replace this with full path to mpirun if needed
#set mpirn = mpirun
set lba = leabra++
echo " " >& lba_sdelta_bench.out

# echo ======================================= >>& lba_sdelta_bench.out
# echo 10 unit layers, 200 epochs >>& lba_sdelta_bench.out
# echo ======================================= >>& lba_sdelta_bench.out

echo " " >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out
echo 50 unit layers, 50 epochs >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out

echo "--------------------------" >>& lba_sdelta_bench.out
echo "Normal:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .005:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 .005 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .01:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 .01 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .02:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 .02 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .05:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 .05 >>& lba_sdelta_bench.out

echo " " >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out
echo 100 unit layers, 15 epochs >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out

echo "--------------------------" >>& lba_sdelta_bench.out
echo "Normal:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .005:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 .005 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .01:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 .01 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .02:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 .02 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .05:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 .05 >>& lba_sdelta_bench.out

echo " " >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out
echo 200 unit layers, 5 epochs >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out

echo "--------------------------" >>& lba_sdelta_bench.out
echo "Normal:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .005:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 .005 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .01:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 .01 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .02:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 .02 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .05:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 .05 >>& lba_sdelta_bench.out

echo " " >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out
echo 500 unit layers, 1 epochs >>& lba_sdelta_bench.out
echo ======================================= >>& lba_sdelta_bench.out

echo "--------------------------" >>& lba_sdelta_bench.out
echo "Normal:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .005:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 .005 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .01:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 .01 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .02:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 .02 >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
echo "send_delta .05:"  >>& lba_sdelta_bench.out
echo "--------------------------" >>& lba_sdelta_bench.out
$lba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 .05 >>& lba_sdelta_bench.out
