/***************************************************************************
 * Program: pneural
 * Author: Daniel Cer (Daniel.Cer@cs.colorado.edu) & Randy O'Reilly (oreilly@psych.colorado.edu)
 * benchmark for nnet simulation under petsc/mpi on clusters
 ***************************************************************************/

#include "petscsles.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <math.h>

/***************************************************************************
 * Constants
 ***************************************************************************/

/* Defines the 'help message' displayed for users when this 
 * program is invoked with a command line argument of 
 * -h or -help */
static char help[] = "pneural - benchmark for nnet simulation under petsc/mpi on clusters\n\
Flags:\n\
   -n : the number of neural units (default: 2000)  \n\
   -a : (roughly) percent of units initially active (default: 25) \n\
   -c : cycles per learning trial (default: 100)\n\
   -t : trials of learning (default: 10)\n\
   -v : if followed by a non-zero value --> display network activation states\n\
        during itterations (default: 0)\n\
\n\
Example Invocations:\n\
\n\
   mpirun -np (# processors) pneural -n 4000 -a 25 -c 100 -t 10\n";

#define DEFAULT_N 2000;  
#define DEFAULT_A 25;
#define DEFAULT_C 100;
#define DEFAULT_T 20;

/* #define TEST 1 */		/*   if defined do various tests to make sure things are working.. */

int main(int argc,char **args)
{
  Mat wt_mat;			/* the weight matrix local to each proc*/
  Vec act_vec;			/* activation vector local to proc */
  Vec net_vec;			/* netinput vector local to proc */
  Vec act_vec_global;		/* global activation vector */

  int n_units;
  int n_act;			/* number active */
  float pct_act;		/* pct active */
  int act_thresh;		/* threshold for rand() for activity */
  float avg_act;		/* computed average activations */
  float	avg_net;		/* computed average netins */
  int cycles;
  int trials;
  int verbose;
  float lrate = 0.01;
  float mcons_raw;		/* millions of connections processed, total */
  float mcons_k;		/* millions of connections processed, only active */

  PetscScalar *net_array;
  PetscScalar *act_array;
  PetscScalar *act_array_global;
  
  int Istart, Iend;		/* local unit indicies */
  PetscScalar zero;
  PetscScalar one;
  int trl, cyc, suj, rui;
  
  int numprocs;
  PetscTruth foundOption;
  int ierr;
  int numLocalRows;
  IS is;
  VecScatter ctx;

  struct tms start_time, end_time;
  clock_t start_time_tot, end_time_tot;
  float secs_tot;
  float secs_usr;
  float secs_sys;

  PetscInitialize(&argc,&args,(char *)0,help);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

  ierr = PetscOptionsGetInt(PETSC_NULL, "-n", &n_units, &foundOption); CHKERRQ(ierr);
  if (!foundOption) n_units = DEFAULT_N;

  ierr = PetscOptionsGetInt(PETSC_NULL, "-a", &n_act, &foundOption); CHKERRQ(ierr);
  if (!foundOption) n_act = DEFAULT_A;
 
  ierr = PetscOptionsGetInt(PETSC_NULL, "-c", &cycles, &foundOption); CHKERRQ(ierr);
  if (!foundOption) cycles = DEFAULT_C;

  ierr = PetscOptionsGetInt(PETSC_NULL, "-t", &trials, &foundOption); CHKERRQ(ierr);
  if (!foundOption) trials = DEFAULT_T;

  ierr = PetscOptionsGetInt(PETSC_NULL, "-v", &verbose, &foundOption); CHKERRQ(ierr);
  if (!foundOption) verbose = 0;

  pct_act = n_act/100.0;
  mcons_raw = (float)((float)n_units * (float)n_units * (float)cycles * (float)trials) / 100000.0;
  mcons_k = (float)((pct_act * (float)n_units) * (pct_act * (float)n_units) * (float)cycles * (float)trials) / 100000.0;

  ierr = PetscPrintf(PETSC_COMM_WORLD,
		     "units: %d, act: %g, cycs: %d, trls: %d, Mcon: %g, MconK: %g",
		     n_units, pct_act, cycles, trials, mcons_raw, mcons_k);
  CHKERRQ(ierr);

  ierr = VecCreate(PETSC_COMM_WORLD, &act_vec); CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) act_vec, "Activations"); CHKERRQ(ierr);
  ierr = VecSetSizes(act_vec, PETSC_DECIDE, n_units); CHKERRQ(ierr);
  ierr = VecSetFromOptions(act_vec); CHKERRQ(ierr);

  ierr = VecDuplicate(act_vec, &net_vec); CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)net_vec, "Net Input"); CHKERRQ(ierr);

  ierr = VecCreateSeq(PETSC_COMM_SELF, n_units, &act_vec_global);
  ierr = ISCreateStride(PETSC_COMM_SELF, n_units,0,1,&is); CHKERRQ(ierr);
  ierr = VecScatterCreate(act_vec, is, act_vec_global, is, &ctx); CHKERRQ(ierr);

  ierr = MatCreateMPIDense(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,
			   n_units, n_units,PETSC_NULL, &wt_mat); CHKERRQ(ierr);
  ierr = MatSetFromOptions(wt_mat); CHKERRQ(ierr);

  ierr = MatGetOwnershipRange(wt_mat, &Istart, &Iend);

  numLocalRows = Iend - Istart;

  act_thresh = (1.0-pct_act)*RAND_MAX;
  zero = 0.0;
  one = 0.5;
  for (rui = Istart; rui<Iend; rui++) {
#ifdef TEST
    if (rui % 4 == 0) {
#else
    if (rand() > act_thresh) {
#endif
      ierr = VecSetValues(act_vec, 1, &rui, &one, INSERT_VALUES);
      CHKERRQ(ierr);
    } else {
      ierr = VecSetValues(act_vec, 1, &rui, &zero, INSERT_VALUES);
      CHKERRQ(ierr);
    }
    for (suj = 0; suj<n_units; suj++) {
      PetscScalar W_ij;
#ifdef TEST
      W_ij = 1.0;
#else
      W_ij = ((float)rand())/RAND_MAX - .5f;
#endif
      ierr = MatSetValues(wt_mat, 1, &rui, 1, &suj, &W_ij, INSERT_VALUES);
      CHKERRQ(ierr);
    }
  }

  /* The way the code is written right now, the following
  * four lines are completely unnecessary */
  ierr = VecAssemblyBegin(act_vec); CHKERRQ(ierr);
  ierr = MatAssemblyBegin(wt_mat, MAT_FINAL_ASSEMBLY);
  ierr = VecAssemblyEnd(act_vec); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(wt_mat, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
/*    ierr = PetscPrintf(PETSC_COMM_WORLD, "Starting"); CHKERRQ(ierr); */

  start_time_tot = times(&start_time);

  for (trl = 0; trl < trials; trl++) {
    if (!(trl % 5)) PetscPrintf(PETSC_COMM_WORLD, ".");
    for (cyc = 0; cyc < cycles; cyc++) {
      ierr = MatMult(wt_mat, act_vec, net_vec); CHKERRQ(ierr);
      ierr = VecGetArray(net_vec, &net_array); CHKERRQ(ierr);
      ierr = VecGetArray(act_vec, &act_array); CHKERRQ(ierr);
      avg_act = 0.0;
      avg_net = 0.0;
      for (rui = 0; rui<numLocalRows; rui++) {
 	act_array[rui] = 1.0/(1.0+exp(-1.0*net_array[rui])); 
#ifdef TEST
	if (rui % 4 != 0) {
#else
	if (rand() < act_thresh) {
#endif
	  act_array[rui] = 0.0;
	}
	avg_act += act_array[rui];
	avg_net += net_array[rui];
      }
      ierr = VecRestoreArray(net_vec, &net_array); CHKERRQ(ierr);
      ierr = VecRestoreArray(act_vec, &act_array); CHKERRQ(ierr);
      avg_act /= (float)numLocalRows;
      avg_net /= (float)numLocalRows;
      if (verbose) {
	ierr = PetscPrintf(PETSC_COMM_WORLD, "Trial: %d, cycle: %d, avg_act: %g, avg_net: %g, n_units: %d\n", trl, cyc, avg_act, avg_net, numLocalRows); 
	CHKERRQ(ierr);
	/*  	  ierr = VecView(act_vec, PETSC_VIEWER_STDOUT_WORLD); CHKERRQ(ierr); */
      }
    }
    {
      ierr = VecScatterBegin(act_vec, act_vec_global, INSERT_VALUES, SCATTER_FORWARD, ctx); CHKERRQ(ierr);
      ierr = VecScatterEnd(act_vec, act_vec_global, INSERT_VALUES, SCATTER_FORWARD, ctx); CHKERRQ(ierr);
      ierr = VecGetArray(act_vec_global, &act_array_global); CHKERRQ(ierr);
      for (rui = Istart; rui<Iend; rui++) {
	PetscScalar *wt_mat_row;
	/* PetscScalar *W_colsnum; */
	int ncols;
	ierr = MatGetRow(wt_mat, rui, &ncols, PETSC_NULL, &wt_mat_row); CHKERRQ(ierr); 
	for (suj = 0; suj<n_units; suj++) {
	  PetscScalar W_ij;
	  W_ij += wt_mat_row[suj] + lrate * act_array_global[rui] * (act_array_global[suj] - W_ij);
	  ierr = MatSetValues(wt_mat, 1, &rui, 1, &suj, &W_ij, INSERT_VALUES); 
	  CHKERRQ(ierr);
	}
	ierr = MatAssemblyBegin(wt_mat, MAT_FINAL_ASSEMBLY);
	ierr = MatAssemblyEnd(wt_mat, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
	ierr = MatRestoreRow(wt_mat, rui, &ncols, PETSC_NULL, &wt_mat_row); CHKERRQ(ierr); 
      }
      ierr = VecRestoreArray(act_vec_global, &act_array_global); CHKERRQ(ierr); 
    }
  }

  end_time_tot = times(&end_time);

  {
    long ticks_per = sysconf(_SC_CLK_TCK);
    secs_usr = (float)((double)(end_time.tms_utime - start_time.tms_utime) / (double)ticks_per);
    secs_sys = (float)((double)(end_time.tms_stime - start_time.tms_stime) / (double)ticks_per);
    secs_tot = (float)((double)(end_time_tot - start_time_tot) / (double)ticks_per);
  }

  mcons_raw /= secs_tot;
  mcons_k /= secs_tot;

  ierr = PetscPrintf(PETSC_COMM_WORLD, "\n\tsec: %g, Mcon/s: %g, MconK/s: %g (usr: %g, sys: %g)\n",
		     secs_tot, mcons_raw, mcons_k, secs_usr, secs_sys); 
  ierr = VecDestroy(act_vec); CHKERRQ(ierr);
  ierr = VecDestroy(net_vec); CHKERRQ(ierr);
  ierr = MatDestroy(wt_mat); CHKERRQ(ierr);

  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
