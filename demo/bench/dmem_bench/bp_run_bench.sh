#!/bin/csh -f

# replace this with full path to mpirun if needed
set mpirn = mpirun
set dmlba = /usr/local/bin/dm_bp++

echo " " >& bp_bench.out

echo ======================================= >>& bp_bench.out
echo 10 unit layers, 2000 epochs >>& bp_bench.out
echo ======================================= >>& bp_bench.out

echo "--------------------------" >>& bp_bench.out
echo "1 process:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes, all-local:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "3 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "4 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "8 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css bp_dmem_10.proj.gz 2000 >>& bp_bench.out

echo " " >>& bp_bench.out
echo ======================================= >>& bp_bench.out
echo 50 unit layers, 500 epochs >>& bp_bench.out
echo ======================================= >>& bp_bench.out

echo "--------------------------" >>& bp_bench.out
echo "1 process:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes, all-local:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "3 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "4 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "8 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css bp_dmem_50.proj.gz 500 >>& bp_bench.out

echo " " >>& bp_bench.out
echo ======================================= >>& bp_bench.out
echo 100 unit layers, 150 epochs >>& bp_bench.out
echo ======================================= >>& bp_bench.out

echo "--------------------------" >>& bp_bench.out
echo "1 process:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes, all-local:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "3 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "4 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#echo "8 processes:"  >>& bp_bench.out
#echo "--------------------------" >>& bp_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css bp_dmem_100.proj.gz 150 >>& bp_bench.out

echo " " >>& bp_bench.out
echo ======================================= >>& bp_bench.out
echo 200 unit layers, 50 epochs >>& bp_bench.out
echo ======================================= >>& bp_bench.out

echo "--------------------------" >>& bp_bench.out
echo "1 process:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes, all-local:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "3 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "4 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "8 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup.css bp_dmem_200.proj.gz 50 >>& bp_bench.out

echo " " >>& bp_bench.out
echo ======================================= >>& bp_bench.out
echo 500 unit layers, 10 epochs >>& bp_bench.out
echo ======================================= >>& bp_bench.out

echo "--------------------------" >>& bp_bench.out
echo "1 process:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes, all-local:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
echo "2 processes:"  >>& bp_bench.out
echo "--------------------------" >>& bp_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "3 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "4 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# echo "8 processes:"  >>& bp_bench.out
# echo "--------------------------" >>& bp_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup.css bp_dmem_500.proj.gz 10 >>& bp_bench.out
