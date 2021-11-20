#!/bin/csh -f

# compiler command
set CC = "g++ -O2 -fno-exceptions -fno-rtti"

$CC -o ptest ptest.cc -lm

echo $CC

./ptest 50 400000
./ptest 100 100000
./ptest 500 4000
./ptest 1000 1000
./ptest 5000 40

