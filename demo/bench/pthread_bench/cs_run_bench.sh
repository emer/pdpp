#!/bin/csh

echo ======================================= >& cs_bench.out
echo 10 unit layers, 500 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

/usr/bin/time cs++ -nogui -f startup.css cs_pt_10_1p.proj.gz 500 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_10_2p.proj.gz 500 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_10_2pnu.proj.gz 500 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 50 unit layers, 50 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

/usr/bin/time cs++ -nogui -f startup.css cs_pt_50_1p.proj.gz 50 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_50_2p.proj.gz 50 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_50_2pnu.proj.gz 50 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 100 unit layers, 5 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

/usr/bin/time cs++ -nogui -f startup.css cs_pt_100_1p.proj.gz 5 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_100_2p.proj.gz 5 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_100_2pnu.proj.gz 5 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 200 unit layers, 2 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

/usr/bin/time cs++ -nogui -f startup.css cs_pt_200_1p.proj.gz 2 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_200_2p.proj.gz 2 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_200_2pnu.proj.gz 2 >>& cs_bench.out

echo " " >>& cs_bench.out
echo ======================================= >>& cs_bench.out
echo 500 unit layers, 1 epochs >>& cs_bench.out
echo ======================================= >>& cs_bench.out

/usr/bin/time cs++ -nogui -f startup.css cs_pt_500_1p.proj.gz 1 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_500_2p.proj.gz 1 >>& cs_bench.out
/usr/bin/time cs++ -nogui -f startup.css cs_pt_500_2pnu.proj.gz 1 >>& cs_bench.out

