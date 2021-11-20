#!/bin/csh

echo ======================================= >& lba_bench.out
echo 10 unit layers, 200 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

/usr/bin/time leabra++ -nogui -f startup.css lba_pt_10_1p.proj.gz 200 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_10_2p.proj.gz 200 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_10_2pnu.proj.gz 200 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 50 unit layers, 50 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

/usr/bin/time leabra++ -nogui -f startup.css lba_pt_50_1p.proj.gz 50 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_50_2p.proj.gz 50 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_50_2pnu.proj.gz 50 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 100 unit layers, 15 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

/usr/bin/time leabra++ -nogui -f startup.css lba_pt_100_1p.proj.gz 15 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_100_2p.proj.gz 15 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_100_2pnu.proj.gz 15 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 200 unit layers, 5 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

/usr/bin/time leabra++ -nogui -f startup.css lba_pt_200_1p.proj.gz 5 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_200_2p.proj.gz 5 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_200_2pnu.proj.gz 5 >>& lba_bench.out

echo " " >>& lba_bench.out
echo ======================================= >>& lba_bench.out
echo 500 unit layers, 1 epochs >>& lba_bench.out
echo ======================================= >>& lba_bench.out

/usr/bin/time leabra++ -nogui -f startup.css lba_pt_500_1p.proj.gz 1 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_500_2p.proj.gz 1 >>& lba_bench.out
/usr/bin/time leabra++ -nogui -f startup.css lba_pt_500_2pnu.proj.gz 1 >>& lba_bench.out

