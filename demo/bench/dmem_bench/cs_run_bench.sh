#!/bin/csh -f

# replace this with full path to mpirun if needed
set mpirn = mpirun
set dmlba = /usr/local/bin/dm_cs++
echo " " >& cs_bench.out

# echo ======================================= >>& cs_bench.out
# echo 10 unit layers, 200 epochs >>& cs_bench.out
# echo ======================================= >>& cs_bench.out

# echo "--------------------------" >>& cs_bench.out
# echo "1 process:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 1 $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "2 processes, all-local:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 2 -all-local $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "2 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 2 $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "3 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "4 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "8 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css cs_dmem_10.proj.gz 200 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 50 unit layers, 50 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

echo "--------------------------" >>& cs_bench.out
echo "1 process:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes, all-local:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "3 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "4 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "8 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css cs_dmem_50.proj.gz 50 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 100 unit layers, 15 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

echo "--------------------------" >>& cs_bench.out
echo "1 process:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes, all-local:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "3 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 3 $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "4 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 4 $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#echo "8 processes:"  >>& cs_bench.out
#echo "--------------------------" >>& cs_bench.out
#$mpirn -np 8 $dmlba -nogui -f startup.css cs_dmem_100.proj.gz 15 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 200 unit layers, 5 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

echo "--------------------------" >>& cs_bench.out
echo "1 process:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes, all-local:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "3 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "4 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "8 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup.css cs_dmem_200.proj.gz 5 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 500 unit layers, 1 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

echo "--------------------------" >>& cs_bench.out
echo "1 process:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 1 $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes, all-local:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 -all-local $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
echo "2 processes:"  >>& cs_bench.out
echo "--------------------------" >>& cs_bench.out
$mpirn -np 2 $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "3 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 3 $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "4 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 4 $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# echo "8 processes:"  >>& cs_bench.out
# echo "--------------------------" >>& cs_bench.out
# $mpirn -np 8 $dmlba -nogui -f startup.css cs_dmem_500.proj.gz 1 >>& cs_bench.out
