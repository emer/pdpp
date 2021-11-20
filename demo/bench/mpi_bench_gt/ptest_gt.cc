#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// this is a benchmark for computing a neural network activation updates with pointer-based
// encoding of connections (ptest)

// Written by: Randall C. O'Reilly; oreilly@psych.colorado.edu

// this version uses MPI to distribute computation across multiple nodes

////////////////////////////////////////////////////////////////////////////////////
// timing code

#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

class TimeUsed {
  // stores and computes time used for processing information
public:
  bool		rec;		// flag that determines whether to record timing information: OFF by default
  long 		usr;		// user clock ticks used
  long		sys;		// system clock ticks used
  long		tot;		// total time ticks used (all clock ticks on the CPU)
  long		n;		// number of times time used collected using GetUsed

  void 		operator += (const TimeUsed& td)	{ usr += td.usr; sys += td.sys; tot += td.tot; }
  void 		operator -= (const TimeUsed& td)	{ usr -= td.usr; sys -= td.sys; tot -= td.tot; }
  void 		operator *= (const TimeUsed& td)	{ usr *= td.usr; sys *= td.sys; tot *= td.tot; }
  void 		operator /= (const TimeUsed& td)	{ usr /= td.usr; sys /= td.sys; tot /= td.tot; }
  TimeUsed 	operator + (const TimeUsed& td) const;
  TimeUsed 	operator - (const TimeUsed& td) const;
  TimeUsed 	operator * (const TimeUsed& td) const;
  TimeUsed 	operator / (const TimeUsed& td) const;

  void		InitTimes();	// initialize the times
  void		GetTimes();	// get the clock ticks used to this point
  void		GetUsed(const TimeUsed& start);
  // get amount of time used by subtracting start from current time and adding to me, and incrementing n
  void		OutString(FILE* fh);	// output string as seconds and fractions of seconds

  TimeUsed();
};

TimeUsed::TimeUsed() {
  rec = false;
  InitTimes();
}

void TimeUsed::InitTimes() {
  usr = 0; sys = 0; tot = 0; n = 0;
}

void TimeUsed::GetTimes() {
  if(!rec) return;
  struct tms t;
  clock_t tottime = times(&t);
  tot = (long)tottime;
  usr = (long)t.tms_utime;
  sys = (long)t.tms_stime;
}

void TimeUsed::GetUsed(const TimeUsed& start) {
  if(!rec || !start.rec) return;
  TimeUsed end;  end.rec = true;
  end.GetTimes();
  *this += (end - start);
  n++;
}

void TimeUsed::OutString(FILE* fh) {
  if(!rec) return;
  long ticks_per = sysconf(_SC_CLK_TCK);
  float ustr = (float)((double)usr / (double)ticks_per);
  float sstr = (float)((double)sys / (double)ticks_per);
  float tstr = (float)((double)tot / (double)ticks_per);
  fprintf(fh, "usr: %g\t sys: %g\t tot: %g\tn: %ld", ustr, sstr, tstr, n);
}

TimeUsed TimeUsed::operator+(const TimeUsed& td) const {
  TimeUsed rv;
  rv.usr = usr + td.usr; rv.sys = sys + td.sys; rv.tot = tot + td.tot;
  return rv;
}
TimeUsed TimeUsed::operator-(const TimeUsed& td) const {
  TimeUsed rv;
  rv.usr = usr - td.usr; rv.sys = sys - td.sys; rv.tot = tot - td.tot;
  return rv;
}
TimeUsed TimeUsed::operator*(const TimeUsed& td) const {
  TimeUsed rv;
  rv.usr = usr * td.usr; rv.sys = sys * td.sys; rv.tot = tot * td.tot;
  return rv;
}
TimeUsed TimeUsed::operator/(const TimeUsed& td) const {
  TimeUsed rv;
  rv.usr = usr / td.usr; rv.sys = sys / td.sys; rv.tot = tot / td.tot;
  return rv;
}

// timing code
////////////////////////////////////////////////////////////////////////////////////

// SCRAMNet GT version (dmem = distributed memory is generic handle for this)
#include <scgtapi.h>

scgtHandle gtHandle;
volatile uint32 *gt32P;
volatile uint32 *gtNumProc32P;
volatile uint32 *gtProcSync32P_1;
volatile uint32 *gtProcSync32P_2;
volatile uint8 *mySync8P_1;
volatile uint8 *mySync8P_2;

volatile float  *gtMyDataP_1;
volatile float  *gtMyDataP_2;
uint32  gtDataOffset_1;
uint32  gtDataOffset_2;
uint32  gtNumBytesW, gtNumBytesR;
uint8   gtSyncIteration = 1;

int dmem_proc;			// mpi processor for this job
int dmem_nprocs;		// total number of processors
const int dmem_max_nprocs = 1024; // maximum number of processors!
const int dmem_max_share = 20000; // hack for non-dynamic array object for dmem type info

class DMemShareVar {
  // definition of one single variable (FLOAT, DOUBLE, INT) that is shared across dmem procs
public:
  /* int		mpi_type; */	// mpi's type for this variable
  int		n_vars;		// total number of variables
  int		max_per_proc;	// maximum number of vars per any one proc
  
  void*		addrs[dmem_max_share];	// addresses for each item to be shared
  int		local_proc[dmem_max_share]; // which proc each guy is local to

  int		n_local[dmem_max_nprocs]; // number of local variables for this proc
  int		recv_idx[dmem_max_nprocs];  // starting indicies into addrs_recv list
  void*		addrs_recv[dmem_max_share]; // addresses in recv format (max_per_proc * nprocs; 000..111..222...)

  float		send_vals[dmem_max_share]; // contiguous array of values, for actual communication (one for each primitive type in real app, with dynamic alloc)
  float		recv_vals[dmem_max_share]; // contiguous array of values, for actual communication (one for each primitive type in real app, with dynamic alloc)

  void	UpdateVar();		// call this after updating the variable info
  void 	SyncVar();		// synchronize variable across procs
};

// in real app, multiple dmemsharevar's can be combined into an aggregate type that
// is shared as a unit across procs

// Now: actual neural network code
// units compute net = sum_j weight_j * act_j for all units that send input into them
// then share this net across nodes
// and then compute activations, and repeat.

class Unit;

class Connection {
  // one connection between units
public:
  float wt;			// connection weight value
  float dwt;			// delta-weight
  Unit* su;			// sending unit
};

class Unit {
  // a simple unit
public:
  float act;			// activation value
  float net;			// net input value
  Connection* recv_wts;		// receiving weights

  // mpi stuff
  int	dmem_local_proc;	// processor on which this unit is local
};

// network configuration information

const int n_layers = 2;
int n_units;			// number of units per layer
int n_cycles;			// number of cycles of updating
Unit* layers[n_layers];		// layers = arrays of units

// construct the network

void MakeNet() {
  layers[0] = new Unit[n_units];
  layers[1] = new Unit[n_units];

  for(int i=0;i<n_units;i++) {
    Unit& un1 = layers[0][i];
    Unit& un2 = layers[1][i];

    // random initial activations [0..1]
    un1.act = (float)rand() / RAND_MAX;
    un2.act = (float)rand() / RAND_MAX;

    // mpi initialization: assign mpi process number
    un1.dmem_local_proc = i % dmem_nprocs;
    un2.dmem_local_proc = i % dmem_nprocs;

    if(un1.dmem_local_proc != dmem_proc) { // don't allocate if not local
      // but we need to keep the random numbers the same as for a single-proc job
      for(int j=0;j<n_units;j++) {
	rand();
	rand();
      }
      continue;
    }

    un1.recv_wts = new Connection[n_units];
    un2.recv_wts = new Connection[n_units];

    // setup reciprocal connections between units
    for(int j=0;j<n_units;j++) {
      Connection& cn1 = un1.recv_wts[j];
      Connection& cn2 = un2.recv_wts[j];
      cn1.su = &(layers[1][j]);
      cn2.su = &(layers[0][j]);

      // random weight values [-2..2]
      cn1.wt = (4.0 * (float)rand() / RAND_MAX) - 2.0;
      cn2.wt = (4.0 * (float)rand() / RAND_MAX) - 2.0;
    }
  }
}

// delete the network

void DeleteNet() {
  for(int l=0;l<n_layers;l++) {
    for(int i=0;i<n_units;i++) {
      Unit& un = layers[l][i];
      delete un.recv_wts;
    }
    delete layers[l];  
  }
}

void ComputeNets() {
  // compute net input = activation times weights
  // this is the "inner loop" that takes all the time!
  for(int l=0;l<n_layers;l++) {
  
    for(int i=0;i<n_units;i++) {
      Unit& un = layers[l][i];
      un.net = 0.0f;
      if(un.dmem_local_proc != dmem_proc) continue; // don't run if not local

      for(int j=0;j<n_units;j++) {
	un.net += un.recv_wts[j].wt * un.recv_wts[j].su->act;
//	un.net ++;
      }
    }
  }
}

float ComputeActs() {
  // compute activations (only order number of units)
  float tot_act = 0.0f;
  for(int l=0;l<n_layers;l++) {
    for(int i=0;i<n_units;i++) {
      Unit& un = layers[l][i];
      un.act = 1.0f / (1.0f + expf(-un.net));
      tot_act += un.act;
    }
  }
  return tot_act;
}

////////////////////////////////
// mpi/dmem code

DMemShareVar dmem_net_var;	// netinput variable shared across all procs

void DMem_MakeNetType() {
  /* dmem_net_var.mpi_type = MPI_FLOAT; */
  int tot_cnt = 0;
  for(int l=0;l<n_layers;l++) {
    for(int i=0;i<n_units;i++, tot_cnt++) {
      Unit& un = layers[l][i];
      dmem_net_var.addrs[tot_cnt] = (void*)&(un.net);
      dmem_net_var.local_proc[tot_cnt] = un.dmem_local_proc;
    }
  }
  dmem_net_var.n_vars = tot_cnt;
  dmem_net_var.UpdateVar();
}

void DMemShareVar::UpdateVar() {
  // initialize counts
  for(int i=0;i<dmem_nprocs;i++) {
    n_local[i] = 0;
  }

  for(int i=0;i<n_vars;i++) {
    n_local[local_proc[i]]++;	// increment counts
  }

  // find max
  max_per_proc = 0;
  for(int i=0;i<dmem_nprocs;i++) {
    if(n_local[i] > max_per_proc) max_per_proc = n_local[i];
  }

  static int proc_ctr[dmem_max_nprocs];
  for(int i=0;i<dmem_nprocs;i++) {
    proc_ctr[i] = 0;
    recv_idx[i] = max_per_proc * i;
  }

  // allocate the addrs_recv array (max_per_proc * nprocs; 000..111..222...)
  for(int i=0;i<n_vars;i++) {
    int lproc = local_proc[i];
    addrs_recv[recv_idx[lproc] + proc_ctr[lproc]] = addrs[i];
    proc_ctr[lproc]++;
  }
}

// what we want is an allgather
// need to have a separate buffer for getting the results: needs to be memory-mapped
// as an array of all the same elements.  i.e., recv_type and send_type need to be the
// same for all procs!  as do the counts!

// 0: send: 0000
// 1: send: 1111
// 2: send: 2222
// all recv: 0000 1111 2222


/* SYNC using GT memory barrier */
  #define DMA_WRITE_TRESHOLD (1024)
  #define DMA_READ_TRESHOLD (64)

void DMemShareVar::SyncVar() {
  // basic computation here is to send all of my stuff to all other nodes
  // and for them to send all of their stuff to me
  
  int i;
  uint32 term = 1;
  uint32 syncI32;
  uint8 remaining;
  volatile uint8 *mySync8P;
  volatile uint32 *gtProcSync32P;
  volatile float *gtMyDataP;
  uint32 gtDataOffset;

  /****************** pick the area to work with  ****************/
  if((gtSyncIteration % 2)==0)
  {
     mySync8P = mySync8P_1;
     gtProcSync32P = gtProcSync32P_1;
     gtMyDataP = gtMyDataP_1;
     gtDataOffset = gtDataOffset_1;
  }
  else 
  {
     mySync8P = mySync8P_2;
     gtProcSync32P = gtProcSync32P_2;
     gtMyDataP = gtMyDataP_2;
     gtDataOffset = gtDataOffset_2;
  }


  /************************ write our data ****************************/
  int my_idx = recv_idx[dmem_proc];
  int my_n = n_local[dmem_proc];
  if (gtNumBytesW < DMA_WRITE_TRESHOLD)                 /* PIO prefered */
  {
      for(i=0;i<my_n;i++, my_idx++)
         gtMyDataP[i] = *((float*)addrs_recv[my_idx]);         
  }
  else                                   /* DMA */
  {
      for(i=0;i<my_n;i++, my_idx++)      /* first copy our values into the send array */
          send_vals[i] = *((float*)addrs_recv[my_idx]); // switch on type in real fun

      scgtWrite(&gtHandle, gtDataOffset + (dmem_proc * max_per_proc), send_vals, gtNumBytesW, 0, NULL, NULL);
  }
  mySync8P[0] = gtSyncIteration;     /* signal write completion */ 


  /********* wait until everyone wrote this iteration *********/
  // first wait for (dmem_nprocs / 4) procs to be done (integer devision)
  syncI32 = (gtSyncIteration << 24) | (gtSyncIteration << 16) | (gtSyncIteration << 8) | (gtSyncIteration);
  for (i = 0; i < (dmem_nprocs / 4);)
  {
      if ((term++ % 4096) == 0)
          if ((*gtNumProc32P) == 0)    /* remote termination */
              exit(-2);
  
      if (gtProcSync32P[i] == syncI32)     /* next! */
          i++;
  }
  
  // now wait for remaining (dmem_nprocs % 4)
  if ((remaining = dmem_nprocs % 4))
  {
      /* build syncI32 to show only remaining spots */
      syncI32 >>= ((4 - remaining) * 8);
      while (gtProcSync32P[i] != syncI32) 
      {
         if ((term++ % 4096) == 0)
             if ((*gtNumProc32P) == 0)    /* remote termination */
                 exit(-2);      
      } 
  }
  /****************** done waiting for write ***********************/

  /************************ read new data ****************************/
  scgtRead(&gtHandle, gtDataOffset, recv_vals, gtNumBytesR,(gtNumBytesR < DMA_READ_TRESHOLD)? SCGT_RW_PIO : 0, NULL);

  // now copy stuff back out
  for(int proc=0;proc<dmem_nprocs;proc++) {
    int p_idx = recv_idx[proc];
    if(proc == dmem_proc) continue; // done!
    for(int i=0;i<n_local[proc];i++, p_idx++) {
      *((float*)addrs_recv[p_idx]) = recv_vals[p_idx]; // switch on type in real fun
    }
  }
  gtSyncIteration++;  
}


void SyncNets() {
  dmem_net_var.SyncVar();
}


int main(int argc, char* argv[]) {

  uint32 gtUnit;
  uint32 gtOffset;
  volatile uint32 *gtProcSync32P;

  if (argc < 6) {
    printf("must have 5 args:\n\t<n_units>\tnumber of units in each of 2 layers\n");
    printf("\t<n_cycles>\tnumber of cycles\n");
    printf("\t<gt unit>\t GT unit number\n");
    printf("\t<gt offset>\tstarting memory offset in GT memory (32-bit aligned)\n");
    printf("\t<proc ID>\tproc ID (starting from 0)\n");
    return 1;
  }

  srand(56);			// always start with the same seed!

  n_units = (int)strtol(argv[1], NULL, 0);
  n_cycles = (int)strtol(argv[2], NULL, 0);

#if 0
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &dmem_nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &dmem_proc);
#endif

  gtUnit = (int)strtol(argv[3], NULL, 0);
  gtOffset = (int)strtol(argv[4], NULL, 0);
  dmem_proc = (int)strtol(argv[5], NULL, 0);

  if (scgtOpen(gtUnit, &gtHandle))
  {
      fprintf(stderr, "Failed to open GT unit 0\n");
      return 2;
  }
  
  if ((gt32P = (uint32 *) scgtMapMem(&gtHandle)) == NULL)
  {
      fprintf(stderr, "Failed to map GT memory\n");
      return 3;
  }

  gtNumProc32P = gt32P + (gtOffset / 4);  /* startup script writes number of procs here */
                                          /* write 0 to this location to terminate early */
  dmem_nprocs = gtNumProc32P[0];     
  gtProcSync32P = gtNumProc32P + 4;      /* here is our read/write syncronization table */
                                         /* the 4 could be 1... 4 is nice for gtmem display to debug */
  gtProcSync32P_1 = &gtProcSync32P[16];   /* things are spaced to accomodate up to 64 CPUs */
  gtProcSync32P_2 = &gtProcSync32P[32];
  mySync8P_1 = ((uint8 *) gtProcSync32P_1) + dmem_proc;
  mySync8P_2 = ((uint8 *) gtProcSync32P_2) + dmem_proc;


  MakeNet();

  DMem_MakeNetType();

  gtNumBytesW = sizeof(float) * dmem_net_var.n_local[dmem_proc];
  gtNumBytesR = sizeof(float) * dmem_net_var.max_per_proc * dmem_nprocs;

  gtDataOffset_1 = gtOffset + 4096;  // data starts 4k after offset
  gtDataOffset_2 = gtDataOffset_1 + (((gtNumBytesR + 64) / 0x1000 + 1) * 0x1000);  // second data section a 4k later

  gtMyDataP_1 = (float *) (((uint8 *) gt32P) + gtDataOffset_1 + (dmem_proc * dmem_net_var.max_per_proc * sizeof(float)));
  gtMyDataP_2 = (float *) (((uint8 *) gt32P) + gtDataOffset_2 + (dmem_proc * dmem_net_var.max_per_proc * sizeof(float)));


  printf("%i:%i-%i ", dmem_proc, gtNumBytesR, gtNumBytesW);
  fflush(stdout);
  
  FILE* logfile = NULL;
  if(dmem_proc == 0)
    logfile = fopen("ptest_gt.log", "w");

// We want to synchronize before timing begins because of unequal CPU speeds 
// and various process start times (ssh conections setup, etc.).
// You can remove this part and things will still work.
// The 0x77 value was picked so it stands out (can be any non-zero 8-bit value).
// We write it starting 32 words after the synchronization table..  
// We could use the sync table but we wanted to see it seperately.

  ((volatile uint8 *)&gtProcSync32P[0])[dmem_proc]= 0x77;      // byte 1 of my 32-bit location 
  int term, i;
  for (i = 0, term = 0; i < dmem_nprocs; i++)
  {
      while (((volatile uint8 *)gtProcSync32P)[i] != 0x77)
      {
          /* early termination code */
          if ((term++ % 4096) == 0)
              if ((*(gtNumProc32P)) == 0)    /* convenience remote termination code */
                  exit(-2);                     
      }
  }

// end of sync before time part



  TimeUsed time_used;	time_used.rec = true;
  TimeUsed start;  	start.rec = true;
  start.GetTimes();

//  double mpi_start = MPI_Wtime();


  // this is the key computation loop
  for(int cyc = 0; cyc < n_cycles; cyc++) {
    ComputeNets();
    SyncNets();
    float tot_act = ComputeActs();
    //    if(dmem_proc == 0)
    //      fprintf(logfile,"%d\t%g\n", cyc, tot_act);
  }

  time_used.GetUsed(start);

//  double mpi_stop = MPI_Wtime();

  if(dmem_proc == 0) {
    fclose(logfile);

//     time_used.OutString(stdout);
//     printf("\n");

//    double tot_time = time_used.tot / 1.0e3;
    double tot_time = time_used.tot / 1.0e2;

//    double mpi_time = mpi_stop - mpi_start;

    double n_wts = n_units * n_units * 2.0f;   // 2 fully connected layers
    double n_con_trav = n_wts * n_cycles;
    double con_trav_sec = ((double)n_con_trav / tot_time) / 1.0e6;

//    printf("procs, %d, \tunits, %d, \tweights, %g, \tcycles, %d, \tcon_trav, %g, \tsecs, %g, \tmpisecs, %g, \tMcon/sec, %g\n",
//	   dmem_nprocs, n_units, n_wts, n_cycles, n_con_trav, tot_time, mpi_time, con_trav_sec);


    printf("\nprocs, %d, \tunits, %d, \tweights, %g, \tcycles, %d, \tcon_trav, %g, \tsecs, %g, \tMcon/sec, %g\n",
	   dmem_nprocs, n_units, n_wts, n_cycles, n_con_trav, tot_time, con_trav_sec);

  }

  DeleteNet();

  scgtUnmapMem(&gtHandle);
  scgtClose(&gtHandle);
}

