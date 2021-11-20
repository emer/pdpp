// basic parameters and functions for the analytical models
// binomial distribution

#ifndef __CSS__
#include <css/hard_of_css.h>
#endif

double 	accuracy;	// accuracy cutoff
String 	log_name;	// log-file name
fstream	log_file;	// log-file itself

class act_par {
public:
  double 	a_i;			// input alpha (activity level)
  double 	a_o;        		// output alpha
  int 		N_i;			// N in input
  int 		N_o;			// N in output
  int		n_i;			// number of active inputs (computed)
  int		n_o;			// number of active outputs (computed)
  int		f_i;			// fan-in from the input
  int		f_o;			// fan-in from the output(computed) 
  double	pc_i;			// probability of contact from i unit (computed)
  double	pc_o;			// probability of contact from o unit (computed)
  double	p_i;			// probability from an active input
  double	p_o;			// probability from an active output
};

void act_par_print(act_par& ths, ostream& strm) {
  strm << "\na_i: " << ths.a_i << " a_o: " << ths.a_o
       << " N_i: " << ths.N_i << " N_o: " << ths.N_o
       << " n_i: " << ths.n_i << " n_o: " << ths.n_o
       << " f_i: " << ths.f_i << " f_o: " << ths.f_o
       << " pc_i: " << ths.pc_i << " pc_o: " << ths.pc_o
       << " p_i: " << ths.p_i << " p_o: " << ths.p_o;
}

void  act_par_updt(act_par& ths) {
  ths.n_i = (int)(ths.a_i * ths.N_i);
  ths.n_o = (int)(ths.a_o * ths.N_o);
  ths.pc_i = (double)ths.f_i / ths.N_i;
  ths.f_o = (int)((double)ths.N_o * ths.pc_i); // ths.f_i / ths.N_i
  ths.pc_o = (double)ths.f_o / ths.N_o;
  ths.p_i = ths.a_i * ths.pc_i;
  ths.p_o = ths.a_o * ths.pc_o;
}

// switch input and output

void act_par_switch(act_par& ths, act_par& src) {
  ths.N_i = src.N_o;
  ths.N_o = src.N_i;
  ths.a_i = src.a_o;
  ths.a_o = src.a_i;
  ths.n_i = src.n_o;
  ths.n_o = src.n_i;
  ths.f_i = src.f_o;
  ths.f_o = src.f_i;
  ths.pc_i = src.pc_o;
  ths.pc_o = src.pc_i;
  ths.p_i = src.p_o;
  ths.p_o = src.p_i;
}

  
// determine threshold empirically by computing sum of hyperg distribution

class thresh_par : public act_par {
public:
  int		h_t;		// threshold number of hits
  double 	act_a_o;	// actual a_o (based on integer threshold)
  int 		max_h;		// maximum h (based on accuracy cutoff)
  double 	avg_h;		// mean number of hits in dist above threshold
};


int get_thresh(thresh_par& ths) {
  ths.max_h = MIN(ths.n_i, ths.f_i);	// max number of hits possible
  double act_a_o = 0;
  double avg_h = 0;
  int h_a;
  double p_a;
  for(h_a = ths.max_h; h_a >= 0; h_a--) {
    p_a = binom_den(ths.N_i, h_a, ths.p_i);
    avg_h += p_a * (double)h_a;
    if(p_a < accuracy)
      ths.max_h = h_a;
    act_a_o += p_a;
    if(act_a_o >= ths.a_o)		// in tail and above threshold
      break;
  }
  ths.avg_h = avg_h / act_a_o;		// normalize
  ths.act_a_o = act_a_o;
  ths.h_t = h_a;
  return h_a;
}



