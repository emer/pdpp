#!/bin/csh -f

# replace this with full path to mpirun if needed
set mpirn = mpirun
set dmlba = /usr/local/bin/dm_leabra++
echo " " >& lba_sdelta_dmem_bench.out

# echo ======================================= >>& lba_sdelta_dmem_bench.out
# echo 10 unit layers, 200 epochs >>& lba_sdelta_dmem_bench.out
# echo ======================================= >>& lba_sdelta_dmem_bench.out

# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "1 process:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 1 $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "2 processes, all-local:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 2 -all-local $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "2 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 2 $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "3 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "4 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "8 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup_sdelta.css lba_dmem_10.proj.gz 200 >>& lba_sdelta_dmem_bench.out

echo " " >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out
echo 50 unit layers, 50 epochs >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out

echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "1 process:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 1 $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes, all-local:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "3 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "4 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "8 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup_sdelta.css lba_dmem_50.proj.gz 50 >>& lba_sdelta_dmem_bench.out

echo " " >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out
echo 100 unit layers, 15 epochs >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out

echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "1 process:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 1 $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes, all-local:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "3 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "4 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#echo "8 processes:"  >>& lba_sdelta_dmem_bench.out
#echo "--------------------------" >>& lba_sdelta_dmem_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup_sdelta.css lba_dmem_100.proj.gz 15 >>& lba_sdelta_dmem_bench.out

echo " " >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out
echo 200 unit layers, 5 epochs >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out

echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "1 process:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 1 $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes, all-local:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "3 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "4 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "8 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup_sdelta.css lba_dmem_200.proj.gz 5 >>& lba_sdelta_dmem_bench.out

echo " " >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out
echo 500 unit layers, 1 epochs >>& lba_sdelta_dmem_bench.out
echo ======================================= >>& lba_sdelta_dmem_bench.out

echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "1 process:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 1 $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes, all-local:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
echo "2 processes:"  >>& lba_sdelta_dmem_bench.out
echo "--------------------------" >>& lba_sdelta_dmem_bench.out
$mpirn -np 2 $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "3 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "4 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# echo "8 processes:"  >>& lba_sdelta_dmem_bench.out
# echo "--------------------------" >>& lba_sdelta_dmem_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup_sdelta.css lba_dmem_500.proj.gz 1 >>& lba_sdelta_dmem_bench.out
