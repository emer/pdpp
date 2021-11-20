# MPI_HOME =/home/dan/mpich-1.2.4
# lab
MPI_HOME =/usr/local/mpi/mpich-1.2.4
default:
	g++ -fPIC -O2 -fno-exceptions -fno-rtti -Wall -Winline -DLINUX -c pdp++base.cc -o pdp++base.o -I${MPI_HOME}/include
	g++ -fPIC -O2 -fno-exceptions -fno-rtti -Wall -Winline -DLINUX -c pdp++bench.cc -o pdp++bench.o -I${MPI_HOME}/include
	g++ pdp++base.o pdp++bench.o -o pdp++bench  ${MPI_HOME}/lib/libmpich.a

clean:
	/bin/rm *.o pdp++bench *.log *~

