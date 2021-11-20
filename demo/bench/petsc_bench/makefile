all: pneural

include ${PETSC_DIR}/bmake/common/base

LINK_ARGS = -rdynamic -Wl,-rpath,/usr/local/petsc/petsc-2.1.3/lib/libO/linux-gnu -rdynamic -Wl,-rpath,/usr/lib:.:/usr/local/mpi/mpich-1.2.4/lib -L/usr/local/petsc/petsc-2.1.3/lib/libO/linux-gnu -L/usr/local/mpi/mpich-1.2.4/lib

LIBS = -lpetscmat -lpetscvec -lpetsc -llapack -lblas -lg2c -lmpich -ldl -L/usr/X11R6/lib -lX11 -lm

pneural: pneural.o
	gcc -O -o pneural pneural.o ${LINK_ARGS} ${LIBS} 

pneural.o: pneural.c
	gcc -O -c -I/usr/local/mpi/mpich-1.2.4/include -I${PETSC_DIR}/include -I${PETSC_DIR}/bmake/linux-gnu -I${PETSC_DIR}/include -DPETSC_HAVE_X11 -DPETSC_USE_LOG -DPETSC_USE_BOPT_O pneural.c

clean:
	/bin/rm pneural.o pneural
