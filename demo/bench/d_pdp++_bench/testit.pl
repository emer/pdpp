#!/usr/bin/perl

@NP_TEST_SET =
(1, 2, 3, 4);

@N_UNITS_TEST_SET =
(50, 100, 200, 400, 800, 1600);

foreach $n_units (@N_UNITS_TEST_SET) {
foreach $np (@NP_TEST_SET) {
   system(
       "echo >> test.log;". 
       "echo n_units: $n_units >> test.log;".
       "echo procs: $np >> test.log;".
       "mpirun -np $np pdp++bench $n_units 10 >> test.log");
}}
