#!/bin/csh -f

# replace this with full path to mpirun if needed
set mpirn = "mpirun.ch_gm -machinefile machines --gm-eager 4096"
set dmlba = /usr/local/bin/dm_leabra++
echo " " >& lba_test_500.out

echo " " >>& lba_test_500.out
echo ======================================= >>& lba_test_500.out
echo 500 unit layers, 1 epochs >>& lba_test_500.out
echo ======================================= >>& lba_test_500.out

echo "--------------------------" >>& lba_test_500.out
echo "2 processes:"  >>& lba_test_500.out
echo "--------------------------" >>& lba_test_500.out
$mpirn -np 2 $dmlba -nogui -f startup_trl.css lba_dmem_500.proj.gz 1 >>& lba_test_500.out
echo "--------------------------" >>& lba_test_500.out
echo "8 processes:"  >>& lba_test_500.out
echo "--------------------------" >>& lba_test_500.out
$mpirn -np 8 $dmlba -nogui -f startup_trl.css lba_dmem_500.proj.gz 1 >>& lba_test_500.out
echo "--------------------------" >>& lba_test_500.out
echo "1 process:"  >>& lba_test_500.out
echo "--------------------------" >>& lba_test_500.out
$mpirn -np 1 $dmlba -nogui -f startup_trl.css lba_dmem_500.proj.gz 1 >>& lba_test_500.out
# echo "--------------------------" >>& lba_test_500.out
# echo "3 processes:"  >>& lba_test_500.out
# echo "--------------------------" >>& lba_test_500.out
# $mpirn -np 3 $dmlba -nogui -f startup.css lba_dmem_500.proj.gz 1 >>& lba_test_500.out
# echo "--------------------------" >>& lba_test_500.out
# echo "4 processes:"  >>& lba_test_500.out
# echo "--------------------------" >>& lba_test_500.out
# $mpirn -np 4 $dmlba -nogui -f startup.css lba_dmem_500.proj.gz 1 >>& lba_test_500.out
