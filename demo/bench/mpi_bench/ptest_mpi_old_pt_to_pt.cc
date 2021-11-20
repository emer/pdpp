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

// Now: actual neural network code
// units compute net = sum_j weight_j * act_j for all units that send input into them
// then share this net across nodes
// and then compute activations, and repeat.

// mpi version (dmem = distributed memory is generic handle for this)
#include <mpi.h>
int dmem_proc;			// mpi processor for this job
int dmem_nprocs;		// total number of processors
const int dmem_max_nprocs = 1024; // maximum number of processors!

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

// mpi type info
int dmem_unit_net_type;		// type for one Unit's net input
int dmem_all_net_types[dmem_max_nprocs]; // type for all of the allocated units, per processor
void* dmem_addr_zero;

void DMem_MakeUnitNetType() {
  int mx_typ = 10;

  int tidx = 0;
  int primitives[mx_typ];
  int byte_offsets[mx_typ];
  int block_lengths[mx_typ];

//   primitives[tidx] = (int)MPI_LB;
//   byte_offsets[tidx] = 0;
//   block_lengths[tidx++] = 1;

  primitives[tidx] = MPI_FLOAT;
  byte_offsets[tidx] = (int)&Unit::net;
  //  printf("byte offset: %d\n", byte_offsets[tidx]);
  block_lengths[tidx++] = 1;

//   primitives[tidx] = (int)MPI_UB;
//   byte_offsets[tidx] = sizeof(Unit);
//   block_lengths[tidx++] = 1;

  MPI_Type_struct(tidx, block_lengths, byte_offsets, primitives, &dmem_unit_net_type);
  MPI_Type_commit(&dmem_unit_net_type);
}

void DMem_MakeAllNetType() {
  int max_local = ((n_units * n_layers) / dmem_nprocs) + 1;
  int offsets[max_local];
  int block_size[max_local];
  int mpi_type[max_local];

  dmem_addr_zero = (void*)&(layers[0][0]);

  for(int lproc=0; lproc < dmem_nprocs; lproc++) {
    int num_units = 0;

    for(int l=0;l<n_layers;l++) {
      for(int i=0;i<n_units;i++) {
	Unit& un = layers[l][i];

	if(un.dmem_local_proc != lproc) continue; // don't run if not local

	block_size[num_units] = 1;
	offsets[num_units] = (ulong)&un - (ulong)dmem_addr_zero;
	mpi_type[num_units] = dmem_unit_net_type;
	num_units++;
      }
    }
    MPI_Type_struct(num_units, block_size, offsets, mpi_type, &(dmem_all_net_types[lproc]));
    MPI_Type_commit(&(dmem_all_net_types[lproc]));
  }
}

void SyncNets() {
  // basic computation here is to send all of my stuff to all other nodes
  // and for them to send all of their stuff to me
  
  //////////////////////////////////////////////////////////////////////////////////
  // this is the optimal version -- lets communciation occur when ready.

  MPI_Request requests[2 * dmem_nprocs];
  MPI_Status status[2 * dmem_nprocs];

  int send_typ = dmem_all_net_types[dmem_proc];	// my type

  int request_ctr = 0;
  for(int send_rank = 0; send_rank < dmem_nprocs; send_rank++) {
    if(send_rank == dmem_proc) continue;
    MPI_Isend(dmem_addr_zero, 1, send_typ, send_rank, 101, MPI_COMM_WORLD, &(requests[request_ctr]));
    request_ctr++;
  }

  for(int recv_rank = 0; recv_rank < dmem_nprocs; recv_rank++) {
    if(recv_rank == dmem_proc) continue;
    int typ = dmem_all_net_types[recv_rank];
    MPI_Irecv(dmem_addr_zero, 1, typ, recv_rank, 101, MPI_COMM_WORLD, &(requests[request_ctr]));
    request_ctr++;
  }

  int waiterr = MPI_Waitall(request_ctr, requests, status);
  if(waiterr != MPI_SUCCESS) {
    if(waiterr == MPI_ERR_IN_STATUS) {
      for(int i=0;i<request_ctr;i++) {
	if(status[i].MPI_ERROR != MPI_SUCCESS) {
	  printf("   DMEM request id: %d failed with code: %d\n", i, status[i].MPI_ERROR);
	}
      }
    }
    else {
      printf("DMEM WaitAll error\n");
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("must have 2 args:\n\t<n_units>\tnumber of units in each of 2 layers\n");
    printf("\t<n_cycles>\tnumber of cycles\n");
    return 1;
  }

  srand(56);			// always start with the same seed!

  n_units = (int)strtol(argv[1], NULL, 0);
  n_cycles = (int)strtol(argv[2], NULL, 0);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &dmem_nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &dmem_proc);

  MakeNet();

  DMem_MakeUnitNetType();
  DMem_MakeAllNetType();

  FILE* logfile;
  if(dmem_proc == 0)
    logfile = fopen("ptest_mpi.log", "w");

  TimeUsed time_used;	time_used.rec = true;
  TimeUsed start;  	start.rec = true;
  start.GetTimes();

  // this is the key computation loop
  for(int cyc = 0; cyc < n_cycles; cyc++) {
    ComputeNets();
    SyncNets();
    float tot_act = ComputeActs();
    if(dmem_proc == 0)
      fprintf(logfile,"%d\t%g\n", cyc, tot_act);
  }

  time_used.GetUsed(start);

  if(dmem_proc == 0) {
    fclose(logfile);

//     time_used.OutString(stdout);
//     printf("\n");

    double tot_time = time_used.tot / 1.0e2;

    double n_wts = n_units * n_units * 2.0f;   // 2 fully connected layers
    double n_con_trav = n_wts * n_cycles;
    double con_trav_sec = ((double)n_con_trav / tot_time) / 1.0e6;

    printf("procs, %d, \tunits, %d, \tweights, %g, \tcycles, %d, \tcon_trav, %g, \tsecs, %g, \tMcon/sec, %g\n",
	   dmem_nprocs, n_units, n_wts, n_cycles, n_con_trav, tot_time, con_trav_sec);
  }

  DeleteNet();
  MPI_Finalize();
}

