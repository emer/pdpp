
cpuCnt=4
minUnitCnt=21
maxUnitCnt=200
incUnitCnt=1
for (( i = $minUnitCnt; i < $maxUnitCnt; i = i + $incUnitCnt )); do
./gt_run.sh -np $cpuCnt -machinefile gtmachines ./ptest_gt $i 40000
done
 
