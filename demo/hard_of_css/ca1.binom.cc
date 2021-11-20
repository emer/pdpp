// ec->ca1->ec circuit

#include "binom_base.cc"

#ifdef __CSS__
#include "gnuplot.css"
#endif

class main_par : public act_par {
public:
  int 		f_start;		// starting fan-in
  int 		f_end;			// ending fan-in
  int 		f_inc;			// fan-in increment
};

void par_print(main_par& ths, ostream& strm) {
  act_par_print(ths, strm);
  strm << " f_start: " << ths.f_start << " f_end: " << ths.f_end
       << " f_inc: " << ths.f_inc << " log: " << log_name << "\n";
}


void ec_ca1_ec(main_par& ths) {
  log_file << "f\tht\tE_h_ca1\tactao\tf_ec\tht_ec\tao_ec\tp_ec\tE_re_ec\tvar_rec\tP_re_ec\tE_oec\tacth_ec\tact_aec\n";

  thresh_par t_ca1, t_ec;
    
#ifdef __CSS__
  t_ca1 = (act_par)ths;
  t_ec = (act_par)ths;
#else
  (act_par)t_ca1 = (act_par)ths;
  (act_par)t_ec = (act_par)ths;
#endif

  int f_ca1;
  for(f_ca1 = ths.f_start; f_ca1 < ths.f_end; f_ca1 += ths.f_inc) {

    t_ca1.f_i = f_ca1;
    act_par_updt(t_ca1);
    get_thresh(t_ca1);
    if(fabs(t_ca1.act_a_o - t_ca1.a_o) > .002)
      continue;

    act_par_switch(t_ec, t_ca1);	// switch input and output
    t_ec.a_i = t_ca1.act_a_o;
    t_ec.n_i = (int)(t_ec.a_i * t_ec.N_i);
    t_ec.p_i = t_ec.pc_i * t_ec.a_i; 	// update

    get_thresh(t_ec);
//    if(fabs(t_ec.act_a_o - t_ec.a_o) > .005)
//      continue;

    int n_ca1 = (int)(t_ca1.act_a_o * ths.N_o);
//    int n_ec = (int)(t_ec.act_a_o * ths.N_i);
    int n_ec = (int)(t_ec.a_o * ths.N_i);
    // mean number of hits expected from the returning ca1 hits
    // model this as a Poisson lambda
    double E_h_ec = ((double)t_ca1.avg_h * n_ca1) / (double)(n_ec);
    double p_h_ec = E_h_ec / (double)t_ec.f_i;
#ifdef POISSON
    double sd_h_ec = sqrt(E_h_ec);
#else
    double sd_h_ec = sqrt(p_h_ec * (1.0 - p_h_ec) * E_h_ec);
#endif

    double* cumu_p = new double[t_ec.f_i+1];	// cumulative probability
    double* cumu_re_p = new double[t_ec.f_i+1];	// cumulative reactivation probability
    double wt_p_re_ec = (double)n_ec / (double)ths.N_i; // how much to weight p

    int k;
    for(k=0; k <= t_ec.f_i; k++) {
      cumu_p[k] = 0;
      cumu_re_p[k] = 0;
    }

    for(k=t_ec.h_t; k <= t_ec.f_i; k++) {
#ifdef POISSON
      double tmp = poisson_den(k, E_h_ec);
#else
      double tmp = binom_den(t_ec.f_i, k, p_h_ec);
#endif
      cumu_p[k] += wt_p_re_ec * tmp;
      cumu_re_p[k] = tmp;
    }

    // the "off" or "other" ec units
    double E_h_oec = (((double)t_ca1.f_i - t_ca1.avg_h) * n_ca1) / (double)(ths.N_i - n_ec);
    double p_h_oec = E_h_oec / (double)t_ec.f_i;
    double wt_p_oec = (double)(ths.N_i - n_ec) / (double)ths.N_i; // how much to weight p

    for(k=t_ec.h_t; k <= t_ec.f_i; k++) {
#ifdef POISSON
      double tmp = poisson_den(k, E_h_oec);
#else
      double tmp = binom_den(t_ec.f_i, k, p_h_oec);
#endif
      cumu_p[k] += wt_p_oec * tmp;
    }

    double p_re_ec = 0;
    double tot_p = 0;
    for(k=t_ec.f_i; k >= 0; k--) {
      tot_p += cumu_p[k];
      p_re_ec += cumu_re_p[k];
      if(tot_p > t_ec.a_o)
	break;
    }
    int real_h_ec = k;

    log_file << f_ca1 << "\t" << t_ca1.h_t << "\t" << t_ca1.avg_h
	     << "\t" << t_ca1.act_a_o << "\t" << t_ec.f_i << "\t" << t_ec.h_t
	     << "\t" << t_ec.act_a_o << "\t" << t_ec.p_i << "\t" << E_h_ec
	     << "\t" << sd_h_ec << "\t" << p_re_ec << "\t" << E_h_oec
	     << "\t" << real_h_ec << "\t" << tot_p << "\n";
    log_file.flush();
    delete cumu_p;
    delete cumu_re_p;
  }
}


void s_main(int ac, String* av) {
  main_par par;

  accuracy = 1.0e-10;
  log_name = "ca1.dt";

  par.a_i = .0625;
  par.a_o = .024;
  par.N_i = 512;
  par.N_o = 666;		// ca1 = 666, ca3 = 450
  act_par_updt(par);

  par.f_i  = 72;
  par.f_start = 10;
  par.f_end   = 250;
  par.f_inc   = 1;

  if(ac > 1) {
    par.a_i = av[1];
    if(ac > 2)
      par.a_o = av[2];
    if(ac > 3)
      par.N_i = av[3];
    if(ac > 4)
      par.N_o = av[4];
    if(ac > 5)
      par.f_start = av[5];
    if(ac > 6)
      par.f_end = av[6];
    if(ac > 7)
      par.f_inc = av[7];
    if(ac > 8)
      log_name = av[8];
  }

  log_file.open(log_name, ios::out);
  if(log_file.bad()) {
    cout << "could not open file " << log_name << "\n";
    return;
  }
  log_file.width(7);
  log_file.setf(ios::fixed + ios::right + ios::dec);
  log_file.width(7);
  log_file.precision(4);

  act_par_updt(par);
  par_print(par, cout);

  fact_ln(par.N_i);			// prime the fact_ln table

  ec_ca1_ec(par);
  log_file.close();
  return;
}

#ifdef __CSS__
s_main(argc, argv);
#endif


