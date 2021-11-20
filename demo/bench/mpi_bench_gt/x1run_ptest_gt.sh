
echo there are only 4 machines thus test for 8 is not a valid one
./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 56 40000
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 56 40000
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 56 40000
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 56 40000

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 112 20000
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 112 20000
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 112 20000
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 112 20000

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 560 400
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 560 400
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 560 400
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 560 400

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 1120 100
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 1120 100
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 1120 100
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 1120 100

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 4480 40
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 4480 40
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 4480 40
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 4480 40

echo
echo now the test exactly as done for other networks
echo 8  run two proceses per machine
echo

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 50 40000
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 50 40000
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 50 40000
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 50 40000

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 100 10000
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 100 10000
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 100 10000
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 100 10000

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 500 400
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 500 400
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 500 400
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 500 400

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 1000 100
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 1000 100
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 1000 100
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 1000 100

./gt_run.sh -np 1 -machinefile gtmachines ./ptest_gt 5000 40
./gt_run.sh -np 2 -machinefile gtmachines ./ptest_gt 5000 40
./gt_run.sh -np 4 -machinefile gtmachines ./ptest_gt 5000 40
./gt_run.sh -np 8 -machinefile gtmachines ./ptest_gt 5000 40
