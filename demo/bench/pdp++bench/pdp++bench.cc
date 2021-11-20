/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

#ifdef __GNUG__
#pragma implementation
#endif

#include "pdp++bench.h"

//////////////////////////
//  BP:	Con, Spec	//
//////////////////////////

BpConSpec::BpConSpec() {
  lrate = .25;
  momentum_type = BEFORE_LRATE;
  momentum = .9;
  momentum_c = .1;
  decay = 0;
  decay_fun = NULL;
}


//////////////////////////
//  BP:	Unit, Spec	//
//////////////////////////

BpUnitSpec::BpUnitSpec() {
  err_tol = 0.0f;
  err_fun = Bp_Squared_Error;
}

void BpUnitSpec::InitState(Unit* u) {
  UnitSpec::InitState(u);
  BpUnit* bu = (BpUnit*)u;
  bu->err = bu->dEdA = bu->dEdNet = 0.0f;
}

void BpUnitSpec::Compute_Act(Unit* u) { // this does the sigmoid
#ifdef SEND_NET
  if(u->bias != NULL)
    u->net += u->bias->wt;
#endif
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else
    u->act = act_range.min + act_range.range * sig.Eval(u->net);
}

void BpUnitSpec::Compute_Error(BpUnit* u) {
  if(u->ext_flag & Unit::TARG) (*err_fun)(this, u);
}

void BpUnitSpec::Compute_dEdA(BpUnit* u) {
  if(u->ext_flag & Unit::TARG) {
    (*err_fun)(this, u);
  }
  else {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  BpCon_Group* send_gp;
  int g;
  FOR_ITR_GP(BpCon_Group, send_gp, u->send., g) {
    if(!send_gp->prjn->layer->lesion)
      u->dEdA += send_gp->Compute_dEdA(u);
  }
}

void BpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = u->dEdA * sig.gain * (u->act - act_range.min) *
    (act_range.max - u->act) * act_range.scale;
}

void BpUnitSpec::Compute_dWt(Unit* u) {
  if(u->ext_flag & Unit::EXT)  return; // don't compute dwts for clamped units
  UnitSpec::Compute_dWt(u);
  ((BpConSpec*)bias_spec)->B_Compute_dWt((BpCon*)u->bias, (BpUnit*)u);
}

void BpUnitSpec::UpdateWeights(Unit* u) {
  if(u->ext_flag & Unit::EXT)  return; // don't update for clamped units
  UnitSpec::UpdateWeights(u);
  ((BpConSpec*)bias_spec)->B_UpdateWeights((BpCon*)u->bias, (BpUnit*)u);
}

BpUnit::BpUnit() {
  err = dEdA = dEdNet = 0.0f;
}

void Bp_Squared_Error(BpUnitSpec* spec, BpUnit* u) {
  u->dEdA = u->targ - u->act;
  if(fabs(u->dEdA) < spec->err_tol) {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  else
    u->err = u->dEdA * u->dEdA;
}

void Bp_CrossEnt_Error(BpUnitSpec* spec, BpUnit* u) {
  u->dEdA = u->targ - u->act;
  if(fabs(u->dEdA) < spec->err_tol) {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  else {
    u->dEdA /= (u->act - spec->act_range.min) * (spec->act_range.max - u->act)
      * spec->act_range.scale;
    float a = spec->sig.Clip((u->act - spec->act_range.min) * spec->act_range.scale);
    float t = (u->targ - spec->act_range.min) * spec->act_range.scale;
    u->err = (t * logf(a) + (1.0f - t) * logf(1.0f - a));
  }
}


//////////////////////////////////////////////////////////
// 		actually run the simulation!		//
//////////////////////////////////////////////////////////

#ifdef NO_TIME
  extern "C" {
    long clock();
  }
#define CLOCKS_PER_SEC 100
#else
#include <time.h>
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

#define RPT_SZ(x,ex) printf("%s:\t%d\t%d\t%d\n", #x, (int)sizeof(x), ex, (int)(ex - sizeof(x)))
#define RPT_SZT(x,ex) printf("%s:\t\t%d\t%d\t%d\n", #x, (int)sizeof(x), ex, (int)(ex - sizeof(x)))

void ApplyPats(Network& net) {
  Layer* lay;
  taLeafItr li;
  FOR_ITR_EL(Layer, lay, net.layers., li) {
    if(li.i == 0)
      lay->ext_flag = Unit::EXT;
    else if(li.i == net.layers.size-1)
      lay->ext_flag = Unit::TARG;
    else
      continue;
    Unit* un;
    taLeafItr ui;
    FOR_ITR_EL(Unit, un, lay->units., ui) {
      if(li.i == 0) {
	un->ext_flag = Unit::EXT;
	un->ext = drand48();
      }
      else if(li.i==net.layers.size-1) {
	un->ext_flag = Unit::TARG;
	un->targ = drand48();
      }
    }
  }
}

void Proximity(Network* net) {
  printf("Proximity\n");
  int l;
  for(l=0;l<net->layers.size;l++) {
    Layer* lay = (Layer*)net->layers[l];
    printf("Layer:%d\n",l);	
    int j;
    for(j=0;j<lay->units.size;j++) {
      Unit* u = (Unit*)lay->units[j];
      long u_ad = (long)u;
      int k;
      if(u->recv.gp.size == 0) continue;
      printf("%d:\t", j);
      Con_Group* cg = (Con_Group*)u->recv.gp[0];
      long cg_ad = (long)cg;
      printf("%ld %ld ", cg_ad - u_ad, (long)cg->el - u_ad);
      for(k=0;k<cg->size;k++) {
	Connection* cn = (Connection*)cg->Cn(k);
	long cn_ad = (long)cn;
	long diff = cn_ad - u_ad;
	if(diff ==  24)
	  printf(".");
	else
	  printf("%ld ", diff);
	u_ad = cn_ad;		// rolling offset
      }
      printf("\n");
    }
  }
}

void RSProximity(Network* net) {
  printf("Proximity\n");
  int l;
  for(l=0;l<net->layers.size;l++) {
    Layer* lay = (Layer*)net->layers[l];
    printf("Layer:%d\n",l);	
    int j;
    for(j=0;j<lay->units.size;j++) {
      Unit* u = (Unit*)lay->units[j];
      long u_ad = (long)u;
      int k;
      if(u->recv.gp.size == 0) continue;
      printf("%d:\t", j);
      Con_Group* cg = (Con_Group*)u->recv.gp[0];
      long cg_ad = (long)cg;
      printf("%ld %ld ", cg_ad - u_ad, (long)cg->el - u_ad);
      for(k=0;k<cg->size;k+=10) {
	Connection* cn = cg->Cn(k);
	Unit* su = cg->Un(k);
	long cn_su_d = (long)su - (long)cn;
	long su_el_d = (long)su - (long)cg->units.el;
	long cn_el_d = (long)cn - (long)cg->el;
	printf("< %ld, %ld, %ld> ", cn_su_d, su_el_d, cn_el_d);
      }
      printf("\n");
    }
  }
}


void OrderRecv(Network* net) {
  printf("Recv Ordering\n");
  int l;
  for(l=0;l<net->layers.size;l++) {
    Layer* lay = (Layer*)net->layers[l];
    printf("Layer:%d\n",l);	
    int j;
    for(j=0;j<lay->units.size;j+=10) {
      Unit* u = (Unit*)lay->units[j];
      int k;
      if(u->recv.gp.size == 0) continue;
      printf("%d:\t", j);
      Con_Group* cg = (Con_Group*)u->recv.gp[0];
      for(k=0;k<cg->size;k+=10) {
	Connection* cn = (Connection*)cg->Cn(k);
	Unit* su = (Unit*)cg->Un(k);
	Con_Group* scg = (Con_Group*)su->send.gp[0];
	int suidx = scg->units.Find(u);
	int cnidx = scg->Find(cn);
	printf("<%d, %d, %d> ", k, suidx, cnidx);
      }
      printf("\n");
    }
  }
}

void OrderSend(Network* net) {
  printf("Send Ordering\n");
  int l;
  for(l=0;l<net->layers.size;l++) {
    Layer* lay = (Layer*)net->layers[l];
    printf("Layer:%d\n",l);	
    int j;
    for(j=0;j<lay->units.size;j+=10) {
      Unit* u = (Unit*)lay->units[j];
      int k;
      if(u->send.gp.size == 0) continue;
      printf("%d:\t", j);
      Con_Group* cg = (Con_Group*)u->send.gp[0];
      for(k=0;k<cg->size;k+=10) {
	Connection* cn = (Connection*)cg->Cn(k);
	Unit* ru = (Unit*)cg->Un(k);
	Con_Group* rcg = (Con_Group*)ru->recv.gp[0];
	int ruidx = rcg->units.Find(u);
	int cnidx = rcg->Find(cn);
	printf("<%d, %d, %d> ", k, ruidx, cnidx);
      }
      printf("\n");
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("must have 2 args:\n\t<n_units>\tnumber of units in each of 3 layers\n");
    printf("\t<n_pats>\tnumber of pattern presentations\n");

    printf("\nSizes:\n");
    RPT_SZT(taBase, 8);
    RPT_SZ(taNBase, 16);
    RPT_SZ(taPtrList_impl, 20);
    RPT_SZ(taList_impl, 48);
    RPT_SZ(taGroup_impl, 108);
    RPT_SZ(BaseSpec, 176);
    RPT_SZ(Connection, 12);
    RPT_SZ(ConSpec, 228);
    RPT_SZ(Con_Group, 192);
    RPT_SZ(UnitSpec, 228);
    RPT_SZT(Unit, 468);
    RPT_SZ(Projection, 108);
    RPT_SZT(Layer, 400);
    RPT_SZ(Network, 424);
    RPT_SZT(BpCon, 20);
    RPT_SZ(BpConSpec, 252);
    RPT_SZ(BpCon_Group, 192);
    RPT_SZ(BpUnitSpec, 252);
    RPT_SZT(BpUnit, 480);
    return 1;
  }

  int n_units = (int)strtol(argv[1], NULL, 0);
  int n_pats = (int)strtol(argv[2], NULL, 0);
  int n_lays = 3;

  BpUnitSpec us;
  BpConSpec cs;
  us.bias_spec = &cs;
  
  Network net;
  int l;
  for(l=0; l<n_lays; l++) {
    Layer* lay = new Layer;
    net.layers.Add_(lay);
    lay->units.Alloc(n_units);
    if(l == 0)
      lay->ext_flag = Unit::EXT;
    else if(l == n_lays-1)
      lay->ext_flag = Unit::TARG;
    if(l > 0) {
      Projection* prjn = new Projection;
      lay->projections.Add_(prjn);
      prjn->layer = lay;
      prjn->from = (Layer*)net.layers.FastEl(l-1);
    }
  }
  Layer* lay;
  taLeafItr li;
  FOR_ITR_EL(Layer, lay, net.layers., li) {
    int u;
    for(u=0; u<n_units; u++) {
      BpUnit* un = new BpUnit;
      lay->units.Add_(un);
      un->spec = &us;
      un->bias = new BpCon;
      un->spec->InitState(un);
#ifndef LOAD_STYLE		// loading allocates inline
    }
#else
      if(li.i > 0) {
	BpCon_Group* cgp = new BpCon_Group;
	un->recv.gp.Add_(cgp);
	cgp->prjn = (Projection*)lay->projections.FastEl(0);
	cgp->spec = &cs;
#if !defined(SEND_BASED) || !defined(NO_PRE_ALLOC)
	cgp->Alloc(n_units);
#endif
#ifndef SEND_BASED
	int j;
	for(j=0; j<n_units; j++) {
#ifdef RANDOM_SPACE
	  if(drand48() < .05)
	    new char[500];
#endif
	  cgp->Add_(new BpCon);
	}
#endif
      }
      if(li.i != 2) {
	BpCon_Group* cgp = new BpCon_Group;
	un->send.gp.Add_(cgp);
	Layer* rl = (Layer*)net.layers.el[l+1];
	cgp->prjn = (Projection*)rl->projections.FastEl(0);
	cgp->spec = &cs;
#if defined(SEND_BASED) || !defined(NO_PRE_ALLOC)
	cgp->Alloc(n_units);
#endif
#ifdef SEND_BASED
	int j;
	for(j=0; j<n_units; j++)
#ifdef RANDOM_SPACE
	  if(drand48() < .05)
	    new char[500];
#endif
	  cgp->Add_(new BpCon);
#endif
      }
    }
#endif
  }

#ifdef EXTRA_SPACE
  char* stuff = new char[8000000];
#endif

  // now connect them up: first pass, allocate
#ifndef LOAD_STYLE
  FOR_ITR_EL(Layer, lay, net.layers., li) {
    Unit* un;
    taLeafItr ui;
    FOR_ITR_EL(Unit, un, lay->units., ui) {
      if(li.i > 0) {
	BpCon_Group* cgp = new BpCon_Group;
	un->recv.gp.Add_(cgp);
	cgp->prjn = (Projection*)lay->projections.FastEl(0);
	cgp->spec = &cs;
#if !defined(SEND_BASED) || !defined(NO_PRE_ALLOC)
	cgp->Alloc(n_units);
#endif
#ifndef SEND_BASED
	int j;
	for(j=0; j<n_units; j++) {
#ifdef RANDOM_SPACE
	  if(drand48() < .05)
	    new char[500];
#endif
	  cgp->Add_(new BpCon);
	}
#endif
      }
      if(li.i != 2) {
	BpCon_Group* cgp = new BpCon_Group;
	un->send.gp.Add_(cgp);
	Layer* rl = (Layer*)net.layers.el[li.i+1];
	cgp->prjn = (Projection*)rl->projections.FastEl(0);
	cgp->spec = &cs;
#if defined(SEND_BASED) || !defined(NO_PRE_ALLOC)
	cgp->Alloc(n_units);
#endif
#ifdef SEND_BASED
	int j;
	for(j=0; j<n_units; j++) {
#ifdef RANDOM_SPACE
	  if(drand48() < .05)
	    new char[500];
#endif
	  cgp->Add_(new BpCon);
	}
#endif
      }
    }
  }
#endif

  // then connect
  FOR_ITR_EL(Layer, lay, net.layers., li) {
    if(li.i == 0) continue;
    Projection* prjn = (Projection*)lay->projections[0];
    Unit* ru, *su;
    taLeafItr ru_itr, su_itr;
#ifndef SEND_BASED
#ifdef LOAD_STYLE
    FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
      FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	recv_gp->units.Add_(su);
      }  
    }
    FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	send_gp->units.Add_(ru);
	Connection* con = recv_gp->Cn(recv_gp->units.Find(su));
	send_gp->Add_(con);
      }  
    }
#else
    FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
      FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	send_gp->units.Add_(ru);
	recv_gp->units.Add_(su);
	Connection* con = recv_gp->Cn(recv_gp->units.size-1);
	send_gp->Add_(con);
      }  
    }
#endif
#else
#ifdef LOAD_STYLE
    FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	send_gp->units.Add_(ru);
      }  
    }
    FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
      FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	recv_gp->units.Add_(su);
	Connection* con = send_gp->Cn(send_gp->units.Find(ru));
	recv_gp->Add_(con);
      }  
    }
#else
    FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
	Con_Group* recv_gp = (Con_Group*)ru->recv.gp[0];
	Con_Group* send_gp = (Con_Group*)su->send.gp[0];
	send_gp->units.Add_(ru);
	recv_gp->units.Add_(su);
	Connection* con = send_gp->Cn(send_gp->units.size-1);
	recv_gp->Add_(con);
      }  
    }
#endif
#endif
  }

  // this clears all external inputs!
  net.InitWtState();

  // so put them back in..
  ApplyPats(net);

  FILE* logfile = fopen("pdp++bench.log", "w");
  // now run the thing!

#ifdef NO_TIME
  long 	st_time, ed_time;
#else
  clock_t st_time, ed_time;
#endif
  st_time = clock();

  int pats;
  for(pats = 0; pats < n_pats; pats++) {
    ApplyPats(net);

#ifdef SEND_NET
    net.InitDelta();
#endif
    FOR_ITR_EL(Layer, lay, net.layers., li) {
      if(lay->lesion)	continue;
#ifndef SEND_NET
      if(!(lay->ext_flag & Unit::EXT))
	lay->Compute_Net();
#endif
      lay->Compute_Act();
#ifdef SEND_NET
      lay->Send_Net();
#endif
    }

    float sse = 0.0f;
    Layer* lay;
    taLeafItr li;
    FOR_ITR_EL(Layer, lay, net.layers., li) {
      if(!(lay->ext_flag & Unit::TARG))
	continue;
      Unit* un;
      taLeafItr ui;
      FOR_ITR_EL(Unit, un, lay->units., ui) {
	float tmp = un->targ - un->act;
	sse += tmp * tmp;
      }
    }

    if((pats % 10) == 0)
      fprintf(logfile,"%d\t%g\n", pats, sse);

    for(l=net.layers.size-1; l>= 0; l--) {
      lay = (Layer*) net.layers.El(l);
      if(lay->lesion || (lay->ext_flag & Unit::EXT)) // dont compute err on inputs
	continue;

      BpUnit* u;
      taLeafItr u_itr;
      FOR_ITR_EL(BpUnit, u, lay->units., u_itr)
	u->Compute_dEdA_dEdNet();
	//	u->Compute_dEdNet();
    }

    net.Compute_dWt();
    net.UpdateWeights();
  }

  ed_time = clock();

  fclose(logfile);

  double tot_time = (double)(ed_time - st_time) / (double)CLOCKS_PER_SEC;

  double n_wts = n_units * n_units * 2.0f;   // 2 fully connected layers
  double n_con_trav = 3.0f * n_wts * n_pats;
  double con_trav_sec = ((double)n_con_trav / tot_time) / 1.0e6;

  printf("weights: %g \tpats: %d \tcon_trav: %g\n", n_wts, n_pats, n_con_trav);
  printf("secs:\tMcon/sec:\n");
  printf("%g\t%g\n", tot_time, con_trav_sec);

#ifdef EXTRA_SPACE
  delete stuff;
#endif

}
