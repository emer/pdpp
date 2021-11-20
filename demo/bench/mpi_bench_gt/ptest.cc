#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// this is a benchmark for computing a neural network activation updates with pointer-based
// encoding of connections (ptest)

// Written by: Randall C. O'Reilly; oreilly@psych.colorado.edu

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

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("must have 2 args:\n\t<n_units>\tnumber of units in each of 2 layers\n");
    printf("\t<n_cycles>\tnumber of cycles\n");
    return 1;
  }

  srand(56);			// always start with the same seed!

  n_units = (int)strtol(argv[1], NULL, 0);
  n_cycles = (int)strtol(argv[2], NULL, 0);

  MakeNet();

  FILE* logfile;
  logfile = fopen("ptest.log", "w");

  TimeUsed time_used;	time_used.rec = true;
  TimeUsed start;  	start.rec = true;
  start.GetTimes();

  // this is the key computation loop
  for(int cyc = 0; cyc < n_cycles; cyc++) {
    ComputeNets();
    float tot_act = ComputeActs();
    fprintf(logfile,"%d\t%g\n", cyc, tot_act);
  }

  time_used.GetUsed(start);

  fclose(logfile);

//   time_used.OutString(stdout);
//   printf("\n");

  double tot_time = time_used.tot / 1.0e2;

  double n_wts = n_units * n_units * 2.0f;   // 2 fully connected layers
  double n_con_trav = n_wts * n_cycles;
  double con_trav_sec = ((double)n_con_trav / tot_time) / 1.0e6;

  printf("procs, %d, \tunits, %d, \tweights, %g, \tcycles, %d, \tcon_trav, %g, \tsecs, %g, \tMcon/sec, %g\n",
	 1, n_units, n_wts, n_cycles, n_con_trav, tot_time, con_trav_sec);

  DeleteNet();
}

