/*

       This file is part of the PDP software package.
		 
       Copyright 1987 by James L. McClelland and David E. Rumelhart.
       
       Please refer to licensing information in the file license.txt,
       which is in the same directory with this source file and is
       included here by reference.
*/


/* file: rbp.c

   Fully recurrent back prop through time with activation,
   clamp, target, and other arrays indexed for each time step.
   Uses rpatterns.c for patterns.  

   Includes cascaded function of two types:

   We either time average the activations or the net inputs.

   We correspondingly either time average the 'error' or
   the 'delta' during the back propagation phase.

   serror, sact, sdelta, and snet are arrays that contain
   the time-averaged values that are used by the network.

   Corresponding non-time-averaged things are sometimes 
   necessary and they are kept as well in sdeda, ssignet, scdelta, 
   and scnet, respectively.
   
*/

#include "general.h"
#include "rbp.h"
#include "variable.h"
#include "rweights.h"
#include "crpatterns.h"
#include "command.h"

#include <sys/types.h>
#include <sys/times.h>

char   *Prompt = "ncrbp: ";
char   *Default_step_string = "epoch";
char	grain_string[20] = "pattern";
boolean System_Defined = FALSE;
boolean lflag = 1;
boolean tforce = 0;
int     epochno = 0;
int     tryno = 0;
int     nepochs = 500;
int	nsteps = 0;
int     stepno = 0;
int     cycleno = 0;
int     maxcycles = 1000;
int     bcycleno = 0;
int     maxbcycles = 1000;
int     patno = 0;
FLOAT   tss = 0.0;
FLOAT   terr = 0.0;
FLOAT   pss = 0.0;
FLOAT   perr = 0.0;
FLOAT   tms = 0.0;
FLOAT   pms = 0.0;
FLOAT   *sss;
int     *tcount;
int     ptcount;
int     ttcount;
int     cpcorrect;
int     tpcorrect;
int     apcorrect;
FLOAT   ecrit = 0.0;
FLOAT   tcrit = 100.0;
FLOAT   accrit = 0.001;
FLOAT   dccrit = 0.001;
FLOAT   nonexttarg = 0.0;
FLOAT   zetolerance = 0.0;
FLOAT   ctolerance = 0.0;
FLOAT   cpweight;
FLOAT   dt;
FLOAT   maxtime;
FLOAT   ctime,btime;
FLOAT	grace = 0;
FLOAT   maxctime = 0.0;
FLOAT   maxactchange = 0;
FLOAT   maxdelchange = 0;
FLOAT	drate = .95;
FLOAT   max = (FLOAT)  1.0;
FLOAT   min = (FLOAT)  0.0;
FLOAT  *netinput = NULL;
FLOAT  *activation = NULL;
FLOAT  *delta = NULL;
FLOAT  *error = NULL;
FLOAT  *actchange = NULL;
FLOAT  *delchange = NULL;
FLOAT  **sact = NULL;
FLOAT  **snet = NULL;
FLOAT  **scnet = NULL;
FLOAT  **ssignet = NULL;
FLOAT  **sdeda = NULL;
FLOAT  **serror = NULL;
FLOAT  **starg = NULL;
FLOAT  **sdelta = NULL;
FLOAT  **scdelta = NULL;
FLOAT  **sclamp = NULL;
FLOAT  **dweight = NULL;
FLOAT  *dbias = NULL;
FLOAT   tmax = 1.0;
FLOAT   momentum = 0.9;
FLOAT	dbdinc = 0.1;
FLOAT	dbddec = 0.9;
FLOAT	initact = 0.5 ;
FLOAT   wdrate, wcost, wcdir, wzero; 
int	tallflag = 0;
int     apmode;
int     fpmode;
int     cemode;
int	rampmode;
int     dbdmode;
int     pwmode;
int     tanmode;
int     symmetrize;
int     bptotargets;

extern int read_weights();
extern int write_weights();
extern int read_dweights();
extern int write_dweights();
extern int read_epsilons();
extern int write_epsilons();

cpu_time()
{
  static float last_time = 0.0 ;
  static float this_time = 0.0 ; /* in seconds */
  static struct tms tms;

  times(&tms);
  last_time = this_time;
  this_time = (tms.tms_utime + tms.tms_stime)/100.0;
  printf("Interval: %g Total: %g",this_time-last_time,this_time);
  scs();
  return(CONTINUE);
}

init_system() {
  int     strain (), ptrain (), tall (), test_pattern (), reset_weights();
  int	    get_unames(), set_lgrain(), newstart(), reseed();
  int change_lrate(), change_maxtime(), change_dt(), change_nsteps();

  epsilon_menu = SETCONFMENU;

  init_weights();
  init_rpattern_sets();

  install_command("strain", strain, BASEMENU,(int *) NULL);
  install_command("ptrain", ptrain, BASEMENU,(int *) NULL);
  install_command("tall", tall, BASEMENU,(int *) NULL);
  install_command("test", test_pattern, BASEMENU,(int *) NULL);
  install_command("time", cpu_time, BASEMENU,(int *) NULL);
  install_command("reset",reset_weights,BASEMENU,(int *)NULL);
  install_command("newstart",newstart,BASEMENU,(int *)NULL);
  install_command("unames", get_unames, GETMENU,(int *) NULL);
  install_command("patterns", get_rpattern_sets, GETMENU,(int *) NULL);
  install_var("lflag", Int,(int *) & lflag, 0, 0, SETPCMENU);
  install_var("tforce", Int,(int *) & tforce, 0, 0, SETMODEMENU);
  install_var("apmode", Int,(int *) & apmode, 0, 0, SETMODEMENU);
  install_var("fpmode", Int,(int *) & fpmode, 0, 0, SETMODEMENU);
  install_var("cemode", Int,(int *) & cemode, 0, 0, SETMODEMENU);
  install_var("dbdmode", Int,(int *) & dbdmode, 0, 0, SETMODEMENU);
  install_var("rampmode", Int,(int *) & rampmode, 0, 0, SETMODEMENU);
  install_var("pwmode", Int,(int *) & pwmode, 0, 0, SETMODEMENU);
  install_var("tanmode", Int,(int *) & tanmode, 0, 0, SETMODEMENU);
  install_var("bptotargets", Int,(int *) & bptotargets, 0, 0, SETMODEMENU);
  install_var("symmetrize", Int,(int *) &symmetrize, 0, 0, SETMODEMENU);
  install_var("lgrain", String, (int *) grain_string,0, 0,NOMENU);
  install_command("lgrain",set_lgrain,SETMODEMENU,(int *) NULL);
  install_var("nepochs", Int,(int *) & nepochs, 0, 0, SETPCMENU);
  install_var("nsteps", Int,(int *) & nsteps, 0, 0, NOMENU);
  install_command("nsteps",change_nsteps,SETPCMENU,(int *) NULL);
  install_var("maxcycles", Int,(int *) & maxcycles, 0, 0, SETPCMENU);
  install_var("maxbcycles", Int,(int *) & maxbcycles, 0, 0, SETPCMENU);
  install_var("tryno", Int,(int *) & tryno, 0, 0, SETSVMENU);
  install_var("epochno", Int,(int *) & epochno, 0, 0, SETSVMENU);
  install_var("cycleno", Int,(int *) & cycleno, 0, 0, SETSVMENU);
  install_var("bcycleno", Int,(int *) & bcycleno, 0, 0, SETSVMENU);
  install_var("stepno", Int,(int *) & stepno, 0, 0, SETSVMENU);
  install_var("patno", Int,(int *) & patno, 0, 0, SETSVMENU);
  install_var("tss", Float,(int *) & tss, 0, 0, SETSVMENU);
  install_var("terr", Float,(int *) & terr, 0, 0, SETSVMENU);
  install_var("maxactchange", Float,(int *) & maxactchange, 0, 0, SETSVMENU);
  install_var("maxdelchange", Float,(int *) & maxdelchange, 0, 0, SETSVMENU);
  install_var("wcost", Float,(int *) &wcost, 0, 0, SETSVMENU);
  install_var("wcdir", Float,(int *) &wcdir, 0, 0, (int *) NULL);
  install_var("cpweight", Float,(int *) &cpweight, 0, 0, (int *) NULL);
  install_var("pss", Float,(int *) & pss, 0, 0, SETSVMENU);
  install_var("perr", Float,(int *) & perr, 0, 0, SETSVMENU);
  install_var("tms", Float,(int *) & tms, 0, 0, SETSVMENU);
  install_var("pms", Float,(int *) & pms, 0, 0, SETSVMENU);
  install_var("ptcount", Int, (int *) & ptcount, 0, 0, SETSVMENU);
  install_var("ttcount", Int, (int *) & ptcount, 0, 0, SETSVMENU);
  install_var("cpcorrect", Int, (int *) & cpcorrect, 0, 0, SETSVMENU);
  install_var("tpcorrect", Int, (int *) & tpcorrect, 0, 0, SETSVMENU);
  install_var("apcorrect", Int, (int *) & apcorrect, 0, 0, SETSVMENU);
  install_var("momentum", Float,(int *) &momentum,0,0,SETPARAMMENU);
  install_var("dbdinc", Float,(int *) &dbdinc,0,0,SETPARAMMENU);
  install_var("dbddec", Float,(int *) &dbddec,0,0,SETPARAMMENU);
  install_var("initact", Float,(int *) &initact,0,0,SETPARAMMENU);
  install_var("dt", Float,(int *) &dt,0,0,NOMENU);
  install_command("dt",change_dt,SETPCMENU,(int *) NULL);
  install_var("grace", Float,(int *) &grace,0,0,SETPCMENU);
  install_var("ctime", Float,(int *) &ctime,0,0,SETSVMENU);
  install_var("btime", Float,(int *) &btime,0,0,SETSVMENU);
  install_var("tcrit", Float,(int *) &tcrit,0,0,SETPCMENU);
  install_var("maxctime", Float,(int *) &maxctime,0,0,SETSVMENU);
  install_var("max", Float,(int *) &max,0,0,SETPARAMMENU);
  install_var("min", Float,(int *) &min,0,0,SETPARAMMENU);
  install_var("wdrate", Float,(int *) &wdrate,0,0,SETPARAMMENU);
  install_var("wzero", Float,(int *) &wzero,0,0,SETPARAMMENU);
  install_command("lrate", change_lrate, SETPARAMMENU, (int *) NULL);
  install_var("lrate", Float,(int *) & lrate, 0, 0, NOMENU);
  install_command("maxtime", change_maxtime, SETPCMENU, (int *) NULL);
  install_var("maxtime", Float,(int *) & maxtime, 0, 0, NOMENU);
  install_var("ecrit", Float,(int *) & ecrit, 0, 0, SETPCMENU);
  install_var("nonexttarg", Float,(int *) & nonexttarg, 0, 0, SETPCMENU);
  install_var("zetolerance", Float,(int *) & zetolerance, 0, 0, SETPCMENU);
  install_var("ctolerance", Float,(int *) & ctolerance, 0, 0, SETPCMENU);
  install_var("dccrit", Float,(int *) & dccrit, 0, 0, SETPCMENU);
  install_var("accrit", Float,(int *) & accrit, 0, 0, SETPCMENU);
  install_var("tmax", Float,(int *) & tmax, 0, 0, SETPARAMMENU);
}

define_system() {
  register int    i,
  j;
  FLOAT *tmp;

  if (!nunits) {
    put_error("cannot init rbp system, nunits not defined");
    return(FALSE);
  }
  if (maxtime <= (FLOAT) 0.0 || dt <= (FLOAT) 0.0) {
    put_error("cannot run rbp system, maxtime or dt not set");
    return(FALSE);
  }

  if (apmode) {
    nsteps = 1;
  }
  else {
    nsteps = rint( (double) (maxtime/dt) );
  }

  netinput = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("netinput", VFLOAT,(int *) netinput, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) netinput[i] = 0.0;

  activation = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("activation", VFLOAT,(int *) activation, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) activation[i] = 0.0;

  delta = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("delta", VFLOAT,(int *) delta, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) delta[i] = 0.0;

  error = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("error", VFLOAT,(int *) error, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) error[i] = 0.0;

  actchange = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("actchange", VFLOAT,(int *) actchange, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) actchange[i] = 0.0;

  delchange = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("delchange", VFLOAT,(int *) delchange, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) delchange[i] = 0.0;

  sss = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nsteps+1));
  install_var("sss", VFLOAT,(int *) sss, nsteps+1, 0, SETSVMENU);
  for (i = 0; i < nsteps+1; i++) sss[i] = 0.0;

  tcount = (int *) emalloc((unsigned)(sizeof(FLOAT) * nsteps+1));
  install_var("tcount", Vint,(int *) tcount, nsteps+1, 0, SETSVMENU);
  for (i = 0; i < nsteps+1; i++) tcount[i] = 0;

  sact = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("sact",PVFLOAT,(int *)sact,nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    sact[i] = ((FLOAT *)
	       emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) sact[i][j] = 0.0;
  }

  snet = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("snet",PVFLOAT,(int *)snet,nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    snet[i] = ((FLOAT *)
	       emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) snet[i][j] = 0.0;
  }

  scnet = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("scnet",PVFLOAT,(int *)scnet,nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    scnet[i] = ((FLOAT *)
		emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) scnet[i][j] = 0.0;
  }

  ssignet = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("ssignet",PVFLOAT,(int *)ssignet,nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    ssignet[i] = ((FLOAT *)
		  emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) ssignet[i][j] = 0.0;
  }

  sdelta = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("sdelta",PVFLOAT,(int *)sdelta, nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    sdelta[i] = ((FLOAT *)
		 emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) sdelta[i][j] = 0.0;
  }

  scdelta = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("scdelta",PVFLOAT,(int *)scdelta, nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    scdelta[i] = ((FLOAT *)
		  emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) scdelta[i][j] = 0.0;
  }

  serror = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("serror",PVFLOAT,(int *)serror, nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    serror[i] = ((FLOAT *)
		 emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) serror[i][j] = 0.0;
  }

  sdeda = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("sdeda",PVFLOAT,(int *)sdeda, nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    sdeda[i] = ((FLOAT *)
		emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) sdeda[i][j] = 0.0;
  }

  starg = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("starg",PVFLOAT,(int *)starg, nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < (nsteps+1); i++) {
    starg[i] = ((FLOAT *)
		emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) starg[i][j] = min -1.0;
  }

  sclamp = (FLOAT **) 
    emalloc((unsigned)(sizeof(FLOAT *) * (nsteps+1)));
  install_var("sclamp",PVFLOAT,(int *)sclamp,nsteps+1,nunits,SETSVMENU);
    
  for (i = 0; i < nsteps+1; i++) {
    sclamp[i] = ((FLOAT *)
		 emalloc((unsigned)(sizeof(FLOAT)* nunits)));
    for (j = 0; j < nunits; j++) sclamp[i][j] = min - 1.0;
  }

  dweight = ((FLOAT **)
	     emalloc((unsigned)(sizeof(FLOAT *)*nunits)));
  install_var("dweight",PVweight,(int *)dweight,nunits,nunits,SETSVMENU);
    
  for (i = 0; i < nunits; i++) {
    dweight[i] = ((FLOAT *)
		  emalloc((unsigned)(sizeof(FLOAT)*num_weights_to[i])));
    for (j = 0; j < num_weights_to[i]; j++) dweight[i][j] = 0.0;
  }

  dbias = (FLOAT *) emalloc((unsigned)(sizeof(FLOAT) * nunits));
  install_var("dbias", VFLOAT,(int *) dbias, nunits, 0, SETSVMENU);
  for (i = 0; i < nunits; i++) dbias[i] = 0.0;

  System_Defined = TRUE;
  return(TRUE);
}


FLOAT  logistic (x)
double  x;
{
    double  exp ();

#ifdef MSDOS    
/* we are conservative under msdos to avoid potential underflow
   problems that may arise from returning extremal values -- jlm */
    if (x > 11.5129)
	return(max - .00001);
      else
	if (x < -11.5129)
	    return(min + .00001);
#else
/* .99999988 is very close to the largest single precis value
   that is resolvably less than 1.0 -- jlm */
      if (x > 15.935773)
	return(max - (FLOAT) .00000012);
      else
	if (x < -15.935773)
	    return(min + (FLOAT) .00000012);
#endif
    else
       return(min+((max-min)/(1.0+(FLOAT) exp((-1.0)*x))));
}

compute_output(s) int s;{
  register int    i,j,b;
  FLOAT *sender, *wt, *end;
  FLOAT net;

  /* sum input */
  for (i = 0; i < nunits; i++) { /* to this unit */
    int nblocks = num_weight_blocks[i] ;
    if (sclamp[s][i] >= min ) continue ;
    net = bias[i];
    wt = weight[i];	/* weight blocks must be contiguous and in order */
    for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
      sender = sact[s-1]+first_weight_to_block[i][b];
      end = sender + num_weights_to_block[i][b];
      for (; sender < end ;){	/* from this unit */
	net += (*sender++)*(*wt++);
      }
    }
    snet[s][i] = scnet[s][i] = netinput[i] = net;
    ssignet[s][i] = (FLOAT) logistic(net);
  }

  /* compute output */
  for (i = 0; i < nunits; i++)
    if (sclamp[s][i] >= min)
      sact[s][i] = activation[i] = ssignet[s][i] = sclamp[s][i];
    else
      sact[s][i] = activation[i] += dt*(ssignet[s][i] - activation[i]);
}

tan_compute_output(s) int s;{
  register int    i,j,b;
  FLOAT *sender, *wt, *end;
  FLOAT net;

  /* sum input */
  for (i = 0; i < nunits; i++) { /* to this unit */
    int nblocks = num_weight_blocks[i] ;
    /* keep running net even if clamped */
    net = bias[i];
    wt = weight[i];	/* weight blocks must be contiguous and in order */
    for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
      sender = sact[s-1]+first_weight_to_block[i][b];
      end = sender + num_weights_to_block[i][b];
      for (; sender < end ;){	/* from this unit */
	net += (*sender++)*(*wt++);
      }
    }
    scnet[s][i] = net;
    snet[s][i] = netinput[i] += dt*(net - netinput[i]);
  }

  /* compute output */
  for (i = 0; i < nunits; i++)
    if (sclamp[s][i] >= min)
      sact[s][i] = activation[i] = ssignet[s][i] = sclamp[s][i];
    else {
      ssignet[s][i] = (FLOAT) logistic(netinput[i]);
      sact[s][i] = activation[i] = ssignet[s][i];
    }
}

/* used in Almeida/Pineda algorithm */

increment_output(s) int s;{
  register int    i,j,b;
  FLOAT *sender, *wt, *end;
  FLOAT net,ac;

  for (i = 0; i < nunits; i++) { /* to this unit */
   int nblocks = num_weight_blocks[i] ;
    if (sclamp[s-1][i] >= min ) {
      sact[s][i] = ssignet[s][i] = sclamp[s-1][i];
    }
    else {
      net = bias[i];
      wt = weight[i];	/* weight blocks must be contiguous and in order */
      for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
	sender = sact[s-1]+first_weight_to_block[i][b];
	end = sender + num_weights_to_block[i][b];
	for (; sender < end ;){	/* from this unit */
	  net += (*sender++)*(*wt++);
	}
      }
      snet[s][i] = scnet[s][i] = netinput[i] = net;
      ssignet[s][i] = logistic(net);
      ac = (ssignet[s][i] - sact[s-1][i]);
      sact[s][i] = sact[s-1][i] + dt * ac;
      actchange[i] = (ac > (FLOAT) 0.0 ? ac : -ac);
    }
    activation[i] = sact[s][i];
  }
  for (i = 0; i < nunits; i++) { /* copyback */
    if (sclamp[s-1][i] < min) {
      sact[s-1][i] = sact[s][i];
    }
  }
}

tan_increment_output(s) int s;{
  register int    i,j,b;
  FLOAT *sender, *wt, *end;
  FLOAT net,ac;

  for (i = 0; i < nunits; i++) { /* to this unit */
   int nblocks = num_weight_blocks[i] ;
    if (sclamp[s-1][i] >= min ) {
      sact[s][i] = ssignet[s][i] = sclamp[s-1][i];
    }
    else {
      net = bias[i];
      wt = weight[i];	/* weight blocks must be contiguous and in order */
      for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
	sender = sact[s-1]+first_weight_to_block[i][b];
	end = sender + num_weights_to_block[i][b];
	for (; sender < end ;){	/* from this unit */
	  net += (*sender++)*(*wt++);
	}
      }
      scnet[s][i] = net;
      ac = net - netinput[i];
      actchange[i] = (ac > (FLOAT) 0.0 ? ac : -ac);
      snet[s][i] = netinput[i] += dt*ac;
      sact[s][i] = ssignet[s][i] = logistic(snet[s][i]);
    }
    activation[i] = sact[s][i];
  }
  for (i = 0; i < nunits; i++) { /* copyback */
    if (sclamp[s-1][i] < min) {
      sact[s-1][i] = sact[s][i];
    }
  }
}

compute_output_error(s) int s; {
  register int i;
  FLOAT t,at,ft,targ;
  FLOAT eweight;
  FLOAT range;

  range = max - min;
    
  sss[s] = 0.0;
  tcount[s] = 0;
  cpcorrect = 1;

  if (fpmode) {
    if (ctime <= grace) {
      eweight = 0.0;  cpcorrect = 0;	/* cannot be correct in grace period */
    } else if (rampmode) {
      eweight = dt*(ctime - grace)/(maxtime - grace);
    } else 
      eweight = dt;
  } else if (apmode) eweight = 1.0;
  else eweight = dt;
  if (pwmode) eweight *= cpweight;
  for (i = 0; i < nunits; i++) {
    targ = starg[s][i] ;
    if (targ >= min && eweight > 0.0) {	/* We care about this one */
      if (nonexttarg > 0.0) 
	if (targ == min) {
	  if (sact[s][i] <= nonexttarg-min) targ = sact[s][i] ;
	  else targ = nonexttarg-min ;
	} else if (targ == max) {
	  if (sact[s][i] >= max-nonexttarg) targ = sact[s][i] ;
	  else targ = max-nonexttarg ;
	}
      t = targ - sact[s][i];
      sss[s] += t*t;
      if (cpcorrect && fabs(t) > ctolerance) cpcorrect = 0;
      if (fabs(t) > zetolerance) {
	if (cemode) {
	  at = (sact[s][i] - min)/range;
	  if (targ == max) {
	    sdeda[s][i] = eweight/at;
	    perr -= eweight*flog((double) at);
	  } else if (targ == min) {
	    sdeda[s][i] = -eweight/(1.0 - at);
	    perr -= eweight*flog((double)(1.0 - at));
	  } else {
	    ft = (targ - min)/range;
	    sdeda[s][i] = eweight*(ft/at - (1.0 - ft)/(1.0 - at));
	    perr += eweight*(flog((double)(ft/at))*ft +
			     flog((double)((1.0-ft)/(1.0-at)))*(1.0-ft)) ;
	  }
	} else { /* not cemode */
	  sdeda[s][i] = eweight*2.0*t ;
	  perr       += eweight*t*t;
	} /* cemode */
      } else { /* within tolerance */
	sdeda[s][i] = (FLOAT) 0.0;
      }	/* zetatolerance */
      /* force to actual target even when using nonexttarg */
      if (tforce) sact[s][i] = starg[s][i];
      tcount[s]++;
    } else /* no target */
      sdeda[s][i] = (FLOAT) 0.0;
  } /* units */ 
  pss += sss[s];
  ptcount += tcount[s];
}

#define fprime(x) ( ((x)-min)*(max-(x))/range )

bperror(s) int s; {
  register int i,j,b,first,end;
  FLOAT *wt;
  FLOAT del;
  FLOAT range = max - min;

  /* inject and invert error */
  for (i = 0; i < nunits; i++) {
    error[i] += dt*(sdeda[s][i] - error[i]);
    serror[s][i] = error[i];
    delta[i] = serror[s][i]*fprime(ssignet[s][i]);
    sdelta[s][i] = scdelta[s][i] = delta[i];
  }
  /* distribute error */
  for (i = 0; i < nunits; i++) {
    int nblocks = num_weight_blocks[i] ;
    del = sdelta[s][i];
    wt = weight[i];	/* weight blocks must be contiguous and in order */
    for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
      first = first_weight_to_block[i][b];
      end   = first + num_weights_to_block[i][b];
      for (j = first ; j < end ; j++, wt++){
	/* only bp to units that are not clamped or forced by teacher */
	if (sclamp[s-1][j] < min && (!tforce || (starg[s-1][j] < min)))
	  sdeda[s-1][j] += del * (*wt);
      }
    }
  }
}

tan_bperror(s) int s; {
  register int i,j,b,first,end;
  FLOAT *wt;
  FLOAT del;
  FLOAT range = max - min;

  for (i = 0; i < nunits; i++) {
    serror[s][i] = error[i] = sdeda[s][i];
    scdelta[s][i] = sdeda[s][i] * fprime(sact[s][i]);
    delta[i] += dt*(scdelta[s][i] - delta[i]);
  }
  /* distribute error */
  for (i = 0; i < nunits; i++) {
    int nblocks = num_weight_blocks[i] ;
    del = sdelta[s][i] = delta[i];
    wt = weight[i];	/* weight blocks must be contiguous and in order */
    for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
      first = first_weight_to_block[i][b];
      end   = first + num_weights_to_block[i][b];
      for (j = first ; j < end ; j++, wt++){
	/* only bp to units that are not clamped or forced by teacher */
	if (sclamp[s-1][j] < min && (!tforce || (starg[s-1][j] < min)))
	  sdeda[s-1][j] += del * (*wt);
      }
    }
  }
}

increment_bperror(s) int s; {
  register int i,j,b,first,end;
  FLOAT *wt;
  FLOAT del,dc;
  FLOAT range = max - min;

  for (i = 0; i < nunits; i++) {
    int nblocks = num_weight_blocks[i] ;
    if (sclamp[s-1][i] < min) {
      dc = sdeda[s][i] - error[i];
      serror[s][i] = error[i] += dt*dc;
      delchange[i] = (dc > (FLOAT) 0.0 ? dc : -dc);
      sdelta[s][i] = scdelta[s][i] = delta[i] = 
	serror[s][i]*fprime(ssignet[s][i]);
      wt = weight[i];	/* weight blocks must be contiguous and in order */
      for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
	first = first_weight_to_block[i][b];
	end   = first + num_weights_to_block[i][b];
	for (j = first ; j < end ; j++, wt++){
	  if (sclamp[s-1][j] < min) sdeda[s-1][j] += delta[i]*(*wt);
	}
      }
    }
    else {
      error[i] = serror[s][i] = (FLOAT) 0.0;
      sdelta[s][i] = scdelta[s][i] = delta[i] = (FLOAT) 0.0;
      delchange[i] = (FLOAT) 0.0;
    }
  }
  for (i = 0; i < nunits; i++) { /* copy error info around loop */
    if (starg[s][i] < min) {
      if (sclamp[s-1][i] < min)   sdeda[s][i] =  sdeda[s-1][i];
    }
    else {			/* is this right?? */
      if (bptotargets)
	sdeda[s][i] = sdeda[s-1][i] + starg[s][i] - sact[s][i];
    }
    sdeda[s-1][i] = (FLOAT) 0.0;
  }
}

tan_increment_bperror(s) int s; {
  register int i,j,b,first,end;
  FLOAT *wt;
  FLOAT del,dc;
  FLOAT range = max - min;

  for (i = 0; i < nunits; i++) {
    int nblocks = num_weight_blocks[i] ;
    if (sclamp[s-1][i] < min) {
      error[i] = serror[s][i] = sdeda[s][i];
      scdelta[s][i] = serror[s][i]*fprime(sact[s][i]);
      dc = scdelta[s][i] - delta[i];
      delchange[i] = (dc > (FLOAT) 0.0 ? dc : -dc);
      sdelta[s][i] = delta[i] += dt*dc;
      wt = weight[i];	/* weight blocks must be contiguous and in order */
      for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
	first = first_weight_to_block[i][b];
	end   = first + num_weights_to_block[i][b];
	for (j = first ; j < end ; j++, wt++){
	  if (sclamp[s-1][j] < min) sdeda[s-1][j] += delta[i]*(*wt);
	}
      }
    }
    else {
      error[i] = serror[s][i] = (FLOAT) 0.0;
      sdelta[s][i] = scdelta[s][i] = delta[i] = (FLOAT) 0.0;
      delchange[i] = (FLOAT) 0.0;
    }
  }
  for (i = 0; i < nunits; i++) { /* copy error info around loop */
    if (starg[s][i] < min) {
      if (sclamp[s-1][i] < min)   sdeda[s][i] =  sdeda[s-1][i];
    }
    else {
      if (bptotargets)
	sdeda[s][i] = sdeda[s-1][i] + starg[s][i] - sact[s][i];
    }
    sdeda[s-1][i] = (float) 0.0;
  }
}

compute_wed(s) int s; {
  register int   i,j,b;
  FLOAT *wi, *sender, *end;
  FLOAT del;

  for (i = 0; i < nunits; i++) {
    int nblocks = num_weight_blocks[i] ;
    if (sclamp[s][i] >= min) continue;
    del     = sdelta[s][i];
    bed[i] += del;
    wi = wed[i];	/* weight blocks must be contiguous and in order */
    for (b = 0; b < nblocks ; b++) { /* blocks of contiguous weights */
      sender = sact[s-1]+first_weight_to_block[i][b];
      end = sender + num_weights_to_block[i][b];
      for (;sender < end;) {
	*wi++ += del * (*sender++);
      }
    }
  }
}

clear_wed() {
  register int   i,j;
  register FLOAT *wi, *end;

  for (i = 0; i < nunits; i++) {
    bed[i] = 0.0;
    wi = wed[i];
    end = wi + num_weights_to[i];
    for (; wi < end;) {
      *wi++ = 0.0;
    }
  }
}

#define melt(x) (((FLOAT)2.0 * (x))/(((FLOAT) 1.0 + (x))*((FLOAT) 1.0 + (x))))

decay_weights() {
  register int    i,j;
  register FLOAT *wt,*wi,*end;
  FLOAT sw;

  if (wzero <= (FLOAT) 0.0) {	/* exponential weight decay */
    for (i = 0; i < nunits; i++) {
      wcost  += bias[i]*bias[i];
      bed[i] -= wdrate*bias[i];
      wt = weight[i]; 
      end = wt + num_weights_to[i];
      for (wi = wed[i]; wt < end; wt++, wi++) {
	wcost += (*wt)*(*wt);
	*wi   -= wdrate*(*wt);
      }
    }
  } else {			/* weight melting */
    FLOAT one_over_wzero_sq = (FLOAT) 1.0 / (wzero * wzero) ;
    for (i = 0; i < nunits; i++) {
      /* sw = bias[i]/wzero; */
      sw = bias[i]*bias[i]*one_over_wzero_sq ;
      wcost  += sw / ((FLOAT) 1.0 + sw) ;
      /* replacing "twdr" with "wdrate" (dcp 21jul92) */
      bed[i] -= wdrate*melt(sw);
      wt = weight[i]; 
      end = wt + num_weights_to[i];
      for (wi = wed[i]; wt < end; wt++, wi++) {
	sw = (*wt)*(*wt)*one_over_wzero_sq ;
	wcost += sw / ((FLOAT) 1.0 + sw) ;
	*wi -= wdrate*melt(sw); 		/* also here (dcp) */
      }
    }
  }
  wcost *= wdrate ;
}

change_weights() {
  register int    i,e,j;
  register FLOAT *wt, *wi, *dwt, *epi, *end;
  FLOAT twdr,sw;
  FLOAT wedXwed, dwtXdwt, wedXdwt ;
  FLOAT lrateXdbdinc = lrate*dbdinc ;

  link_sum();
  if (symmetrize) symmetrize_weds();
  wedXwed = dwtXdwt = wedXdwt = 0.0 ;

  wcost = 0.0;
  if (wdrate > (FLOAT) 0.0) decay_weights() ;

  for (i = 0; i < nunits; i++) {
    if (dbdmode && epochno > 1) {
      if (bed[i]*dbias[i]>0) bepsilon[i] += lrateXdbdinc ;
      else                   bepsilon[i] *= dbddec ;
    }
    /* dcp: using epsilon and momentum differently
       dbias[i] = bepsilon[i]*bed[i] + momentum*dbias[i];
       bias[i] += dbias[i];
       */
    dbias[i] = bed[i] + momentum*dbias[i];
    bias[i] += bepsilon[i]*dbias[i];
    wedXwed += bed[i]*bed[i] ;
    dwtXdwt += dbias[i]*dbias[i] ;
    wedXdwt += bed[i]*dbias[i] ;
    bed[i]   = 0.0;
    wt  = weight[i];
    dwt = dweight[i];
    wi  = wed[i];
    epi = epsilon[i];
    end = wt + num_weights_to[i];
    if (dbdmode && epochno > 1)	/* delta-bar-delta */
      for (; wt < end; ) {
	if ((*wi)*(*dwt)>0) (*epi) += lrateXdbdinc ;
	else                (*epi) *= dbddec ;
	/* 
	 * dcp: using epsilon and momentum differently
	 *	*dwt   = (*epi++)*(*wi) + momentum*(*dwt);
	 *	*wt++ += *dwt++;
	 */
	*dwt     = (*wi) + momentum*(*dwt);
	*wt++   += (*epi++)*(*dwt);

	wedXwed += (*wi)*(*wi) ;
	dwtXdwt += (*dwt)*(*dwt) ;
	wedXdwt += (*wi)*(*dwt++) ;
	*wi++    = 0.0;
      } 
    else
      for (; wt < end; ) {
	/* 
	 * dcp: using epsilon and momentum differently
	 *	*dwt   = (*epi++)*(*wi) + momentum*(*dwt);
	 *	*wt++ += *dwt++;
	 */
	*dwt     = (*wi) + momentum*(*dwt);
	*wt++   += (*epi++)*(*dwt);

	wedXwed += (*wi)*(*wi) ;
	dwtXdwt += (*dwt)*(*dwt) ;
	wedXdwt += (*wi)*(*dwt++) ;
	*wi++    = 0.0;
      }
  }				/* units */
  pos_neg_constraints();
  wcdir = (wedXwed==0.0||dwtXdwt==0.0 ? 0.0 : wedXdwt/sqrt(wedXwed*dwtXdwt));
}

constrain_weights() {
  pos_neg_constraints();
  link_constraints();
}

pos_neg_constraints() {
  FLOAT **fpt;

  for (fpt = positive_constraints; fpt && *fpt; fpt++)
    if (**fpt < 0.0)
      **fpt = 0.0;

  for (fpt = negative_constraints; fpt && *fpt; fpt++)
    if (**fpt > 0.0)
      **fpt = 0.0;
}

link_constraints() {
  register int    i,j;
  FLOAT   t;

  for (i = 0; i < nlinks; i++) {
    t = *constraints[i].cvec[0];
    for (j = 1; j < constraints[i].num; j++) {
      *constraints[i].cvec[j] = t;
    }
  }
}

link_sum() {
  register int    i,j;
  FLOAT   ss;

  for (i = 0; i < nlinks; i++) {
    ss = 0.0;
    for (j = 0; j < constraints[i].num; j++) {
      ss += *constraints[i].ivec[j];
    }
    for (j = 0; j < constraints[i].num; j++) {
      *constraints[i].ivec[j] = ss;
    }
  }
}

symmetrize_weds() {
  int i,j,wi,wj,bi,bj,di,dj;
  FLOAT ijval;
  FLOAT jival;
  for (i = 0; i < nunits; i++) {
    for (bi = 0, di = 0; bi < num_weight_blocks[i]; bi++) {
      int fi = first_weight_to_block[i][bi];
      int ni = num_weights_to_block[i][bi];
      for (j = 0; j < nunits; j++) {
	for (bj = 0, dj = 0; bj < num_weight_blocks[j] ; bj++) { 
	  int fj = first_weight_to_block[j][bj];
	  int nj = num_weights_to_block[j][bj];
	  if ((wj=j-fi) >= 0 && wj < ni &&
	      (wi=i-fj) >= 0 && wi < nj) {
	    ijval = wed[i][wj+dj];
	    jival = wed[j][wi+di];
	    wed[i][wj+dj] = wed[j][wi+di] = ijval + jival;
	  }
	  dj += nj;
	}
      }
      di += ni;
    }
  }
}

symmetrize_weights() {
  int i,j,wi,wj,bi,bj,di,dj;
  FLOAT ijval;
  FLOAT jival;
  for (i = 0; i < nunits; i++) {
    for (bi = 0, di = 0; bi < num_weight_blocks[i]; bi++) {
      int fi = first_weight_to_block[i][bi];
      int ni = num_weights_to_block[i][bi];
      for (j = 0; j < nunits; j++) {
	for (bj = 0, dj = 0; bj < num_weight_blocks[j] ; bj++) { 
	  int fj = first_weight_to_block[j][bj];
	  int nj = num_weights_to_block[j][bj];
	  if ((wj=j-fi) >= 0 && wj < ni &&
	      (wi=i-fj) >= 0 && wi < nj) {
	    /* this assumes weights are random - otherwise should average */
	    weight[i][wj+dj] = weight[j][wi+di];
	  }
	  dj += nj;
	}
      }
      di += ni;
    }
  }
}

trial() {
  int s,i,laststep;
  setup_pattern();
  pms = pss = perr = 0.0;
  ptcount = 0;
  if (apmode) {			/* almeida-pineda algorithm */
    for (cycleno = 0; cycleno < maxcycles; ) {
      cycleno++;
      ctime = dt*cycleno;
      if (tanmode) tan_increment_output(1);
      else increment_output(1);
      maxactchange = (FLOAT) 0.0;
      for (i = 0; i < nunits; i++) {
	if (actchange[i] > maxactchange) {
	  maxactchange = actchange[i];
	}
      }
      if (step_size == CYCLE) {
	update_display();
	if (single_flag) {
	  if (contin_test() == BREAK) return(BREAK);
	}
      }
      if (maxactchange < accrit) break;
    }
    if (cycleno == maxcycles) {
      tss = -1.0;
      return(CONTINUE);
    }
    compute_output_error(1);
    for (bcycleno = 0; bcycleno < maxbcycles; ) {
      bcycleno++;
      if (tanmode) tan_increment_bperror(1);
      else increment_bperror(1);
      maxdelchange = (FLOAT) 0.0;
      for (i = 0; i < nunits; i++) {
	if (delchange[i] > maxdelchange) {
	  maxdelchange = delchange[i];
	}
      }
      if (step_size == CYCLE) {
	update_display();
	if (single_flag) {
	  if (contin_test() == BREAK) return(BREAK);
	}
      }
      if (maxdelchange < dccrit) break;
      if (lflag) {
	compute_wed(1);
      }
    }
    if (bcycleno == maxbcycles) {
      tss = -1.0;
      return(CONTINUE);
    }
  }
  else {
    for (i = 0; i < nunits; i++)
      if (sclamp[0][i] >= min)
	activation[i] = sact[0][i] = ssignet[0][i] = sclamp[0][i];
      else sact[0][i] = initact ;
    if (tforce) {
      for (i = 0; i < nunits; i++) {
	if (starg[0][i] >= min) sact[0][i] = starg[0][i];
      }
    }
    for (stepno = 1; stepno <= nsteps; stepno++) {
      laststep = s = stepno;
      ctime = stepno*dt;
      if (tanmode) tan_compute_output(s);
      else compute_output(s);
      compute_output_error(s);
      if (step_size == CYCLE) {
	update_display();
	if (single_flag) {
	  if (contin_test() == BREAK) return(BREAK);
	}
      }
      if (fpmode && cpcorrect) break;
    }
    for (i = 0; i < nunits; i++) error[i] = 0.0 ;
    for (s = laststep; s > 0; s--) {
      btime = s*dt;
      if (tanmode) tan_bperror(s);
      else bperror(s);
      if (lflag) compute_wed(s);
    }
  }
  if (ctime > maxctime) maxctime = ctime;
  pms = pss/ptcount;
  if (cpcorrect) tpcorrect++;
  else apcorrect = 0;
  tss += pss;
  tms += pms;
  terr += perr;
  return (CONTINUE);
}

ptrain() {
  return(train('p'));
}

strain() {
  return(train('s'));
}

train(c) char c; {
  int     t,i,old,npat;
  char    *str;

  if (!System_Defined)
    if (!define_system())
      return(BREAK);

  if (!tallflag) clear_wed();

  for (t = 0; t < nepochs; t++) {
    if (!tallflag) epochno++;
    for (i = 0; i < npatterns; i++)
      used[i] = i;
    if (c == 'p') {
      for (i = 0; i < npatterns; i++) {
	npat = rnd() * (npatterns - i) + i;
	old = used[i];
	used[i] = used[npat];
	used[npat] = old;
      }
    }
    ttcount = terr = tss = tms = 0.0;
    maxctime = 0.0;
    apcorrect = 1;
    tpcorrect = 0;
    for (i = 0; i < npatterns; i++) {
      patno = used[i];
      if (trial() == BREAK) return (BREAK);
      if (tss < (FLOAT) 0.0) break;
      if (lflag && grain_string[0] == 'p') {
	change_weights();
      }
      if (step_size <= PATTERN) {
	update_display();
	if (single_flag) {
	  if (contin_test() == BREAK) return(BREAK);
	}
      }
      if (Interrupt) {
	Interrupt_flag = 0;
	update_display();
	if (contin_test() == BREAK) return(BREAK);
      }
    }
    if (lflag && grain_string[0] == 'e') {
      change_weights();
    }
    if (step_size == EPOCH) {
      update_display();
      if (single_flag) {
	if (contin_test() == BREAK) return(BREAK);
      }
    }
    if (fpmode) {
      if (apcorrect && tss < ecrit && maxctime < tcrit) break;
    }
    else if (tss < ecrit) break;
  }
  if (step_size == NEPOCHS) {
    update_display();
  }
  return(CONTINUE);
}

tall() {
  int save_lflag;
  int save_single_flag;
  int save_nepochs;
  int save_step_size;
  
  save_lflag = lflag;  lflag = 0;
  save_single_flag = single_flag; 
  if (in_stream == stdin) single_flag = 1;
  save_step_size = step_size; 
  if (step_size > PATTERN) step_size = PATTERN;
  save_nepochs = nepochs;  nepochs = 1;
  tallflag = 1;
  train('s');
  tallflag = 0;
  lflag = save_lflag;
  nepochs = save_nepochs;
  single_flag = save_single_flag;
  step_size = save_step_size;
  return(CONTINUE);
}
  
test_pattern() {
  char   *str;
  int save_single_flag;
  int save_step_size;

  if (!System_Defined)
    if (!define_system())
      return(BREAK);

  tss = terr = 0.0;

  str = get_command("Test which pattern? ");
  if(str == NULL) return(CONTINUE);
  if ((patno = get_rpattern_number(str)) < 0) {
    return(put_error("Invalid pattern specification."));
  }
  trial();
  update_display();
  return(CONTINUE);
}

newstart() {
  reseed();
  reset_weights();
}

reseed() {
  random_seed = time(0);
	
}

reset_epsilons() {
  register int    i,j;

  if (epsilon != NULL) 
    for (i = 0; i < nunits; i++) 
      for (j = 0; j < num_weights_to[i]; j++) 
	if (epsilon[i][j] != 0.0) epsilon[i][j] = lrate;
  if (bepsilon != NULL)
    for (i = 0; i < nunits; i++) 
      if (bepsilon[i] != 0.0) bepsilon[i] = lrate;
}

reset_weights() {
  register int    i,j,num;
  char ch;
    
  epochno = 0;
  tryno++;
  cpname[0] = '\0';
  srand(random_seed);

  if (!System_Defined)
    if (!define_system())
      return(BREAK);

  reset_epsilons() ;	/* dcp: in case dbdmode has messed them up */

  for (j = 0; j < nunits; j++) {
    num = num_weights_to[j];
    for (i = 0; i < num; i++) {
      wed[j][i] = dweight[j][i] = 0.0;
      ch = wchar[j][i];
      if (isupper(ch)) ch = tolower(ch);
      if (ch == '.') {
	weight[j][i] = 0.0;	    
      }
      else {
	if (constants[ch - 'a'].random) {
	  if (constants[ch - 'a'].positive) {
	    weight[j][i] = wrange * rnd();
	  }
	  else
	    if (constants[ch - 'a'].negative) {
	      weight[j][i] = wrange * (rnd() - 1);
	    }
	    else
	      weight[j][i] = wrange * (rnd() -.5);
	}
	else {
	  weight[j][i] = constants[ch - 'a'].value;
	}
      }
    }
    bed[j] = dbias[j] = 0.0;
    ch = bchar[j];
    if (isupper(ch)) ch = tolower(ch);
    if (ch == '.') {
      bias[j] = 0;
    }
    else {
      if (constants[ch - 'a'].random) {
	if (constants[ch - 'a'].positive) {
	  bias[j] = wrange * rnd();
	}
	else
	  if (constants[ch - 'a'].negative) {
	    bias[j] = wrange * (rnd() - 1);
	  }
	  else
	    bias[j] = wrange * (rnd() -.5);
      }
      else {
	bias[j] = constants[ch - 'a'].value;
      }
    }
  }
  constrain_weights();
  if (symmetrize) symmetrize_weights();
  clear_arrays();
  /*
    Chad Dawson Jan 10th 1991
    */
  ResetScales();
  

  update_display();
  return(CONTINUE);
}

clear_arrays() {
  int i,s;
  
  ttcount = tss = terr = tms = (FLOAT) 0.0;
  cycleno = bcycleno = 0;
  for (i = 0; i < nunits; i++) {
    actchange[i] = delchange[i] = netinput[i] = activation[i] = error[i] = 
      (FLOAT) 0.0;
    for (s = 0; s < nsteps +1; s++) {
      sclamp[s][i] = ssignet[s][i] = sact[s][i] = starg[s][i] = serror[s][i] = 
	sdeda[s][i] = sdelta[s][i] = scdelta[s][i] = snet[s][i] = 
	  scnet[s][i] = (FLOAT) 0.0;
    }
  }
  for (s = 0; s < nsteps + 1; s++) {
    sss[s] = (FLOAT) 0.0;
  }
}

set_lgrain() {
  char old_grain_string[STRINGLENGTH];
  struct Variable *vp, *lookup_var();
    
  strcpy(old_grain_string,grain_string);

  vp = lookup_var("lgrain");
  change_variable("lgrain",vp);
    
  if (startsame(grain_string,"epoch")) strcpy(grain_string,"epoch");
  else if (startsame(grain_string,"pattern"))
    strcpy(grain_string,"pattern");
  else {
    strcpy(grain_string,old_grain_string);
    return(put_error("urecognized grain -- not changed."));
  }
  return(CONTINUE);
}

change_maxtime() {
  struct Variable *vp, *lookup_var();
  FLOAT old_maxtime;
  old_maxtime = maxtime;
    
  vp = lookup_var("maxtime");
  change_variable("maxtime",vp);
    
  if (maxtime != old_maxtime) {
    maxtime = old_maxtime;
    return(put_error("maxtime cannot change."));
  }
  return(CONTINUE);
}

change_nsteps() {
  struct Variable *vp, *lookup_var();
  int old_nsteps;
  old_nsteps = nsteps;
    
  vp = lookup_var("nsteps");
  change_variable("nsteps",vp);
    
  if (nsteps != old_nsteps) {
    nsteps = old_nsteps;
    return(put_error("change maxtime to change nsteps."));
  }
  return(CONTINUE);
}

change_dt() {
  struct Variable *vp, *lookup_var();
  FLOAT old_dt;
  old_dt = dt;
    
  vp = lookup_var("dt");
  change_variable("dt",vp);
    
  if (dt != old_dt) {
    dt = old_dt;
    return(put_error("dt cannot be changed"));
  }
  return(CONTINUE);
}

init_weights() {
  int define_rbp_network();
  install_command("network", define_rbp_network,GETMENU,(int *) NULL);
  install_command("weights", read_weights, GETMENU,(int *) NULL);
  install_command("weights", write_weights, SAVEMENU,(int *) NULL);
  install_command("dweights", read_dweights, GETMENU,(int *) NULL);
  install_command("dweights", write_dweights, SAVEMENU,(int *) NULL);
  install_command("epsilons", read_epsilons, GETMENU,(int *) NULL);
  install_command("epsilons", write_epsilons, SAVEMENU,(int *) NULL);
  install_var("nunits", Int,(int *) & nunits, 0, 0, SETCONFMENU);
  install_var("ninputs", Int,(int *) & ninputs, 0, 0, SETCONFMENU);
  install_var("noutputs", Int,(int *) & noutputs, 0, 0, SETCONFMENU);
  install_var("wrange",Float,(int *) &wrange,0,0, SETPARAMMENU);
}

define_rbp_network() {
  define_bp_network();
  if (symmetrize) symmetrize_weights();
}

setup_pattern() {
  int a,i,s,u;
  struct pset *setp;
  struct pspec *sp;
  struct aspec *ap;
  FLOAT *vp;
  FLOAT rval,cval;
  setp = set+patno;
  strcpy(cpname,setp->pname);
  cpweight = setp->pweight;
  for (u = 0; u < nunits; u++) {
    delta[u] = (FLOAT) 0.0;
    activation[u] = initact;
    netinput[u] = (FLOAT) 0.0;
    error[u] = (FLOAT) 0.0;
  }
  if (apmode) {
    for (u = 0; u < nunits; u++) {
      actchange[u] = delchange[u] = (FLOAT) 0.0;
    }
  }
  for(s = 0; s < nsteps+1; s++) {
    for (u = 0; u < nunits; u++) {
      sclamp[s][u] = min -1.0;
      starg[s][u]  = min -1.0;
      sdelta[s][u] = scdelta[s][u] = (FLOAT) 0.0;
      snet[s][u] = scnet[s][u] = (FLOAT) 0.0;
      ssignet[s][u] = sact[s][u] = initact;
      sdeda[s][u] = serror[s][u] = (FLOAT) 0.0;
    }
  }
  for (sp = setp->pspec,i = 0; i < setp->nispecs; i++,sp++) {
    for (s = sp->startstep; s < sp->startstep+sp->nsteps;s++){
      for(vp=sp->val,u=sp->startunit; u<sp->startunit+sp->nunits;u++,vp++) {
	sclamp[s][u] = *vp;
      }
    }
  }
  for (u = 0; u < nunits; u++) {
    if (sclamp[0][u] >= min) {
      ssignet[0][u] = sact[0][u] = sclamp[0][u];
    }
    else {
      ssignet[0][u] = sact[0][u] = initact;
    }
  }
  if (setp->nalts > 1) {
    rval = rnd();
    for (ap = setp->aspec,a=0,cval = 0.0; a < setp->nalts; a++,ap++) {
      cval += ap->tprob;
      if (rval < cval) break;
    }
    ctalt = a;
  }
  else {
    ctalt = a = 0;
    ap = setp->aspec;
  }
  for (sp = ap->pspec,i = 0; i < ap->ntspecs; i++,sp++) {
    for (s = sp->startstep; s < (sp->startstep)+(sp->nsteps);s++){
      for(vp=sp->val,u=sp->startunit;u<(sp->startunit)+(sp->nunits);u++,vp++) {
	starg[s][u] = *vp;
      }
    }
  }
}

/*****************************************************************************/
/* write/read dweights and epsilons
/*****************************************************************************/

write_dweights() {
  int     i,j,end;
  char   *str = NULL;
  char fname[BUFSIZ];
  char *star_ptr;
  char tstr[40];
  FILE * iop;

  if (dweight == NULL) {
    return(put_error("cannot save undefined network"));
  }

 nameagain:
  str = get_command("dweight file name: ");
  if (str == NULL) return(CONTINUE);
  strcpy(fname,str);
  if ( (star_ptr = pdp_index(fname,'*')) != NULL) {
    strcpy(tstr,star_ptr+1);
    sprintf(star_ptr,"%d",epochno);
    strcat(fname,tstr);
  }
  if ((iop = fopen(fname, "r")) != NULL) {
    fclose(iop);
    get_command("file exists -- clobber? ");
    if (str == NULL || str[0] != 'y') {
      goto nameagain;
    }
  }
  if ((iop = fopen(fname, "w")) == NULL) {
    return(put_error("cannot open file for output"));
  }

  for (i = 0; i < nunits; i++) {
    for (j = 0; j < num_weights_to[i]; j++) {
      fprintf(iop, "%f\n", dweight[i][j]);
    }
  }

  if (dbias) {
    for (i = 0; i < nunits; i++) {
      fprintf(iop, "%f\n", dbias[i]);
    }
  }

  (void) fclose(iop);
  return(CONTINUE);
}

read_dweights() {
  int     i,j,end;
  char   *str = NULL;
  FILE * iop;

  if(!System_Defined)
    if(!define_system())
      return(BREAK);

  if (dweight == NULL) {
    return(put_error("cannot restore undefined network"));
  }

  if((str = get_command("File name for stored dweights: ")) == NULL)
    return(CONTINUE);

  if ((iop = fopen(str, "r")) == NULL) {
    sprintf(err_string,"Cannot open dweight file %s.",str);
    return(put_error(err_string));
  }

  for (i = 0; i < nunits; i++) {
    if(num_weights_to[i] == 0) continue;
    for (j = 0; j < num_weights_to[i]; j++) {
      if (fscanf(iop, "%lf", &scanf_var) == EOF) {
	fclose(iop);
	return(put_error("dweight file is not correct for this network"));
      }
      dweight[i][j] = (FLOAT) scanf_var;
    }
  }

  end = nunits;
    
  if (dbias != NULL) {
    for (i = 0; i < end; i++) {
      if (fscanf(iop, "%lf", &scanf_var) == EOF) {
	fclose(iop);
	return(put_error("dweight file is not correct for this network"));
      }
      dbias[i] = scanf_var;
    }
  }

 read_finish:
  (void) fclose(iop);
  update_display();
  return(CONTINUE);
}

write_epsilons() {
  int     i,j,end;
  char   *str = NULL;
  char fname[BUFSIZ];
  char *star_ptr;
  char tstr[40];
  FILE * iop;

  if (epsilon == NULL) {
    return(put_error("cannot save undefined network"));
  }

 nameagain:
  str = get_command("epsilons file name: ");
  if (str == NULL) return(CONTINUE);
  strcpy(fname,str);
  if ( (star_ptr = pdp_index(fname,'*')) != NULL) {
    strcpy(tstr,star_ptr+1);
    sprintf(star_ptr,"%d",epochno);
    strcat(fname,tstr);
  }
  if ((iop = fopen(fname, "r")) != NULL) {
    fclose(iop);
    get_command("file exists -- clobber? ");
    if (str == NULL || str[0] != 'y') {
      goto nameagain;
    }
  }
  if ((iop = fopen(fname, "w")) == NULL) {
    return(put_error("cannot open file for output"));
  }
  fprintf(iop, "%f\n", lrate);
  for (i = 0; i < nunits; i++) {
    for (j = 0; j < num_weights_to[i]; j++) {
      fprintf(iop, "%f\n", epsilon[i][j]/lrate);
    }
  }

  if (bepsilon) {
    for (i = 0; i < nunits; i++) {
      fprintf(iop, "%f\n", bepsilon[i]/lrate);
    }
  }

  (void) fclose(iop);
  return(CONTINUE);
}

read_epsilons() {
  int     i,j,end;
  char   *str = NULL;
  FLOAT	epsi_lrate ;
  FILE * iop;

  if(!System_Defined)
    if(!define_system())
      return(BREAK);

  if (epsilon == NULL) {
    return(put_error("cannot restore undefined network"));
  }

  if((str = get_command("File name for stored epsilons: ")) == NULL)
    return(CONTINUE);

  if ((iop = fopen(str, "r")) == NULL) {
    sprintf(err_string,"Cannot open epsilons file %s.",str);
    return(put_error(err_string));
  }

  if (fscanf(iop, "%lf", &scanf_var) == EOF) {
    fclose(iop);
    return(put_error("epsilons file is not correct for this network"));
  }
  epsi_lrate = (FLOAT) scanf_var;
  if (epsi_lrate != lrate)
    put_error("WARNING: lrate changed (%f to %f)", epsi_lrate, lrate);

  for (i = 0; i < nunits; i++) {
    if (num_weights_to[i] == 0) continue;
    for (j = 0; j < num_weights_to[i]; j++) {
      if (fscanf(iop, "%lf", &scanf_var) == EOF) {
	fclose(iop);
	return(put_error("epsilons file is not correct for this network"));
      }
      epsilon[i][j] = lrate * (FLOAT) scanf_var;
    }
  }

  end = nunits;
    
  if (bepsilon != NULL) {
    for (i = 0; i < end; i++) {
      if (fscanf(iop, "%lf", &scanf_var) == EOF) {
	fclose(iop);
	return(put_error("epsilons file is not correct for this network"));
      }
      bepsilon[i] = lrate * (FLOAT) scanf_var;
    }
  }

 read_finish:
  (void) fclose(iop);
  update_display();
  return(CONTINUE);
}

