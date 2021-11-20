/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the PDP++ software package.			      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted	      //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//									      //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions. 	HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
// 									      //
// Note that the taString class, which is derived from the GNU String class,  //
// is Copyright (C) 1988 Free Software Foundation, written by Doug Lea, and   //
// is covered by the GNU General Public License, see ta_string.h.             //
// The iv_graphic library and some iv_misc classes were derived from the      //
// InterViews morpher example and other InterViews code, which is             //
// Copyright (C) 1987, 1988, 1989, 1990, 1991 Stanford University             //
// Copyright (C) 1991 Silicon Graphics, Inc.				      //
//									      //
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,         //
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 	      //
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  	      //
// 									      //
// IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY SPECIAL,    //
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES  //
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT     //
// ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY,      //
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        //
// SOFTWARE. 								      //
==============================================================================*/

// bleabra.h 

#ifndef bleabra_h
#ifdef __GNUG__
#pragma interface
#endif
#define bleabra_h

#include <bleabra/biobel.h>
#include <bleabra/bleabra_TA_type.h>

class LeabraCon;
class LeabraConSpec;
class LeabraBiasSpec;
class LeabraCon_Group;

class LeabraUnitSpec;
class LeabraUnit;
class LeabraLayer;
class LeabraLayerSpec;
class LeabraCycle;
class LeabraSettle;
class LeabraTrial;

class LeabraCon : public BioBelCon {
  // LEABRA connection
public:
  float		dwt;		// #NO_VIEW #NO_SAVE resulting net weight change
  float		pdw;		// #NO_SAVE previous delta-weight change
  float		lr;		// connection-specific learning rate parameter

  void 	Initialize()		{ dwt = pdw = lr = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const LeabraCon& cp);
  COPY_FUNS(LeabraCon, BioBelCon);
  TA_BASEFUNS(LeabraCon);
};

class LeabraDBDSpecs : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER delta-bar-delta lrate specs
public:
  float		inc;		// amount to additively increment
  float		dec;		// amount to multiplicatively decrement (multliply by (1.0 - dec))
  float		max;		// maximum learning rate possible
  float		inc_lr;		// #HIDDEN increment times learning rate (must set in CS UAE)
  float		dec_c;		// #HIDDEN complement of dec (for multiplying)

  void	UpdateLrate(LeabraCon* cn) {
    float prod = cn->dwt * cn->pdw;
    if(prod > 0.0f) {
      cn->lr += inc_lr;	cn->lr = MIN(cn->lr, max);
    }
    else if(prod < 0.0f)
      cn->lr *= dec_c;
    // prod of zero means don't change lrate (could have it increase?)
  }

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LeabraDBDSpecs);
  COPY_FUNS(LeabraDBDSpecs, taBase);
  TA_BASEFUNS(LeabraDBDSpecs);
};

class LeabraConSpec : public BioBelConSpec {
  // LEABRA connection specs
public:
  enum SAvgSource {
    SLAYER_TRG_PCT,
    SLAYER_AVG_ACT,
    FIXED_SAVG
  };

  enum HebbType {
    ZSH,			// zero-sum hebbian
    PXY				// probability of x given y
  };

  float		lrate;		// learning rate
  LeabraDBDSpecs dbd;		// delta-bar-delta specifications
  HebbType	hebb_type;	// type of hebbian learning to use
  float		k_hebb;		// proportion of learning due to hebbian component
  float		k_err;		// complementary proportion due to error-driven component
  SAvgSource	savg_src;	// source of sending average act for correcting hebb rule
  float		fixed_savg;	// parameter for fixed sending average act
  float		fx_inc_cor;	// #HIDDEN fixed increase correction
  float		fx_dec_cor;	// #HIDDEN fixed decrease correction

  void 		C_InitWtState(Connection* cn, Unit* ru, Unit* su) {
    BioBelConSpec::C_InitWtState(cn, ru, su); LeabraCon* lcn = (LeabraCon*)cn;
    lcn->pdw = 0.0f; lcn->lr = lrate; }

  void 		C_InitWtDelta(Connection* cn, Unit* ru, Unit* su)
  { BioBelConSpec::C_InitWtDelta(cn, ru, su); ((LeabraCon*)cn)->dwt=0.0f; }

  inline virtual float Compute_SAvgCor(LeabraCon_Group* cg, float& inc_cor, float& dec_cor);
  // compute hebb correction scaling terms for sending average act, returns inc cor, and dec cor

  inline float	C_Compute_Hebb(LeabraCon* cn, float ru_act, float su_act,
			       float savg, float inc_cor, float dec_cor);
  // compute Hebbian associative learning

  inline float 	C_Compute_Err(LeabraCon*, float ru_act_p, float ru_act_m,
				  float su_act_p, float su_act_m);
  // compute generec error term, sigmoid case

  inline void 	C_Compute_dWt(LeabraCon* cn, float heb, float err);
  // combine associative and error-driven weight change, actually update dwt

  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);
  // compute weight change

  inline virtual void	B_Compute_dWt_Leak(LeabraCon* cn, LeabraUnit* ru);
  // compute bias weight change for leak model of bias weight
  inline virtual void	B_Compute_dWt_Netin(LeabraCon* cn, LeabraUnit* ru);
  // compute bias weight change for netin model of bias weight

  inline void		C_UpdateWeights(LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void		UpdateWeights(Con_Group* cg, Unit* ru);
  inline virtual void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru);

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraConSpec);
  COPY_FUNS(LeabraConSpec, BioBelConSpec);
  TA_BASEFUNS(LeabraConSpec);
};

class LeabraBiasSpec : public LeabraConSpec {
  // LEABRA bias-weight connection specs (bias wts are a little bit special)
public:
  float		dwt_thresh;
  // don't change wts if dwt < thresh (should be same as learn_thresh)

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru);

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraBiasSpec);
  COPY_FUNS(LeabraBiasSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraBiasSpec);
};

class LeabraCon_Group : public BioBelCon_Group {
  // LEABRA connection group
public:

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraCon_Group);
};

class LeabraUnitSpec : public BioBelUnitSpec {
  // LEABRA unit specs
public:
  float		learn_thresh;	// minimum receiver activity for learning

  void 		InitWtState(Unit* u);

  virtual void	PhaseInit(BioBelUnit* u, BioBelLayer* lay, int phase);
  // initialize external input flags based on phase
  void		PostSettle(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr, int phase);
  // set stuff after settling is over

  virtual void	Compute_dWt_impl(LeabraUnit* u);
  // actually do wt change
  void		Compute_dWt(Unit* u);
  virtual void	Compute_dWt_post(LeabraUnit* u, LeabraLayer* lay);
  // after computing the weight changes (for recon delta)

  virtual void	Compute_AvgAct_dWt(LeabraUnit* u);
  // weight change in bias due to avgerage act being over/under limits

  void		UpdateWeights(Unit* u);

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraUnitSpec);
  COPY_FUNS(LeabraUnitSpec, BioBelUnitSpec);
  TA_BASEFUNS(LeabraUnitSpec);
};

class LeabraUnit : public BioBelUnit {
  // LEABRA unit
public:
  float		act_dif;	// difference between plus and minus phase acts
  float		act_m;		// minus_phase activation
  float		act_p;		// plus_phase activation

  void		PhaseInit(BioBelLayer* lay, int phase)
  { ((LeabraUnitSpec*)spec.spec)->PhaseInit(this, lay, phase); }

  void 		Compute_dWt_post(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt_post(this, lay); }

  void	Initialize();
  void	Destroy()		{ };
  void 	InitLinks();
  SIMPLE_COPY(LeabraUnit);
  COPY_FUNS(LeabraUnit, BioBelUnit);
  TA_BASEFUNS(LeabraUnit);
};

class LeabraLayerSpec : public BioBelLayerSpec {
  // LEABRA layer specifications
public:
  virtual void	PhaseInit(BioBelLayer* lay, int phase);
  // initialize start of a setting phase, set input flags appropriately, etc

  void	PostSettle(BioBelLayer* lay, int phase);
  // after settling, keep track of phase variables, etc.

  virtual void	Compute_dWt_post(LeabraLayer* lay);
  // something to be done after the weights are changed (i.e. update history..)

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraLayerSpec);
};

class LeabraLayer : public BioBelLayer {
  // LEABRA layer 
public:
  AvgMaxVals	acts_p;		// plus-phase activation stats for the layer
  AvgMaxVals	acts_m;		// minus-phase activation stats for the layer

  void	PhaseInit(int phase)	
  { ((LeabraLayerSpec*)spec.spec)->PhaseInit(this, phase); }

  void	Compute_dWt_post()
  { ((LeabraLayerSpec*)spec.spec)->Compute_dWt_post(this); }

  void	Initialize();
  void	Destroy()		{ };
  void 	InitLinks();
  SIMPLE_COPY(LeabraLayer);
  COPY_FUNS(LeabraLayer, BioBelLayer);
  TA_BASEFUNS(LeabraLayer);
};

//////////////////////////
//     Computing dWt 	//
//////////////////////////


inline float LeabraConSpec::Compute_SAvgCor(LeabraCon_Group* cg, float& inc_cor, float& dec_cor) {
  LeabraLayer* fm = (LeabraLayer*)cg->prjn->from;
  float savg;
  if(savg_src == SLAYER_TRG_PCT) {
    savg = fm->kwta.pct;
    inc_cor = sqrtf((1.0f - savg) / savg);
    dec_cor = 1.0f / inc_cor;
  }
  else if(savg_src == SLAYER_AVG_ACT) {
    savg = fm->acts.avg;
    inc_cor = sqrtf((1.0f - savg) / savg);
    dec_cor = 1.0f / inc_cor;
  }
  else {
    savg = fixed_savg;
    inc_cor = fx_inc_cor;
    dec_cor = fx_dec_cor;
  }
  return savg;
}

inline float LeabraConSpec::C_Compute_Hebb(LeabraCon* cn, float ru_act, float su_act,
					   float savg, float inc_cor, float dec_cor)
{
  float hebb;
  if(hebb_type == ZSH)
    hebb = (su_act > savg) ? (ru_act * inc_cor * (su_act - savg)) : 
			     (ru_act * dec_cor * (su_act - savg));
  else
    hebb = ru_act * (inc_cor * su_act * (1.0f - cn->wt) -
		     dec_cor * (1.0f - su_act) * cn->wt);
  return hebb;
}

// generec error term with sigmoid activation function
inline float LeabraConSpec::C_Compute_Err
(LeabraCon*, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
  return (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
}

// combine associative and error-driven
inline void LeabraConSpec::C_Compute_dWt(LeabraCon* cn, float heb, float err) {
  cn->dwt += k_err * err + k_hebb * heb;
}

inline void LeabraConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraUnit* lru = (LeabraUnit*)ru;
  float inc_cor, dec_cor;
  float savg = Compute_SAvgCor((LeabraCon_Group*)cg, inc_cor, dec_cor);
  for(int i=0; i<cg->size; i++) {
    LeabraUnit* su = (LeabraUnit*)cg->Un(i);
    LeabraCon* cn = (LeabraCon*)cg->Cn(i);
    C_Compute_dWt(cn,
		  C_Compute_Hebb(cn, lru->act_p, su->act_p, savg, inc_cor, dec_cor),
		  C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
  }
}

inline void LeabraConSpec::C_UpdateWeights(LeabraCon* cn, LeabraUnit*, LeabraUnit*) {
  dbd.UpdateLrate(cn);		// lrate change takes effect *before* wt change..
  cn->wt += cn->lr * cn->dwt;
  cn->wt = MAX(cn->wt, 0.0f);	// limit 0-1
  cn->wt = MIN(cn->wt, 1.0f);
  cn->pdw = cn->dwt;
  cn->dwt = 0.0f;
}

inline void LeabraConSpec::UpdateWeights(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_UpdateWeights((LeabraCon*)cg->Cn(i), (LeabraUnit*)ru, 
				     (LeabraUnit*)cg->Un(i)));
//  ApplyLimits(cg, ru); limits are automatically enforced anyway
}

// bias weights, are in the denominator, negative of error
// might want to set lrate to equal  glbar?
inline void LeabraConSpec::B_Compute_dWt_Leak(LeabraCon* cn, LeabraUnit* ru) {
  cn->dwt += (ru->act_m - ru->act_p);
}

inline void LeabraConSpec::B_Compute_dWt_Netin(LeabraCon* cn, LeabraUnit* ru) {
  cn->dwt += (ru->act_p - ru->act_m);
}
  
// default is not to do anything tricky with the bias weights
inline void LeabraConSpec::B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
  cn->pdw = cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dwt = 0.0f;
  C_ApplyLimits(cn, ru, NULL);
}

inline void LeabraBiasSpec::B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
  if(fabsf(cn->dwt) < dwt_thresh)
    cn->dwt = 0.0f;
  cn->pdw = cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dwt = 0.0f;
  C_ApplyLimits(cn, ru, NULL);
}


//////////////////////////
// 	Processes	//
//////////////////////////

class LeabraCycle : public CycleProcess {
  // one Leabra relative belief cycle of activation updating
public:
  LeabraSettle*	leabra_settle;
  // #NO_SUBTYPE #READ_ONLY #NO_SAVE pointer to parent settle proc

  void		Loop();		// compute activations
  bool 		Crit()		{ return true; } // executes loop only once
  
  virtual void	Compute_Net();
  virtual void	Compute_Clamp_NetAvg();
  virtual void	Compute_Inhib();
  virtual void	Compute_Act();

  void 	UpdateAfterEdit();		// modify to update the leabra_trial
  void 	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(LeabraCycle);
  COPY_FUNS(LeabraCycle, CycleProcess);
  TA_BASEFUNS(LeabraCycle);
};

class CascadeParams : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER Holds cascading parameters
public:
  int		cycles;		// number of cycles per cascade
  int		depth;
  // #IV_READ_ONLY #SHOW depth of cascading of information through net
  int		max; 		// maximum depth of network

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(CascadeParams);
  COPY_FUNS(CascadeParams, taBase);
  TA_BASEFUNS(CascadeParams);
};

class LeabraSettle : public SettleProcess {
  // Leabra relative belief settling phase of activation updating
public:
  LeabraTrial* 	leabra_trial;
  // #NO_SUBTYPE #READ_ONLY #NO_SAVE pointer to parent phase trial
  int		min_cycles;	// minimum number of cycles to settle for
  bool		use_cascade; 	// whether to use cascading activations or not
  CascadeParams	cascade;
  // #CONTROL_PANEL parameters controling cascading of info through net

  void		Init_impl();	// initialize start of settling
  void		Loop();		// do cascading in loop
  void		Final();	// update acts at end of settling

  virtual void	Compute_Active_K();
  virtual void	DecayEvent();
  virtual void	DecayPhase();
  virtual void	DecayAE();
  virtual void	PhaseInit();
  virtual void	CompToTarg();	// move COMPARE to TARGET (for + phase)
  virtual void	ExtToComp();	// move all env input to COMPARE (for final minus)
  virtual void	Compute_HardClamp();
  virtual void	Compute_InputDist();
  virtual void	Compute_Cascade();
  virtual void	Compute_ClampNet();
  virtual void	PostSettle();
  virtual void	Compute_dWt();

  void 	UpdateAfterEdit();		// modify to update the leabra_trial
  void 	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(LeabraSettle);
  COPY_FUNS(LeabraSettle, SettleProcess);
  TA_BASEFUNS(LeabraSettle);
};

class LeabraTrial : public TrialProcess {
  // Leabra relative belief trial process, iterates over phases
public:
  enum StateInit {		// ways of initializing the state of the network
    DO_NOTHING,			// do nothing
    INIT_STATE,			// initialize state
    DECAY_STATE			// decay the state
  };
    
  enum Phase {
    MINUS_PHASE = 0,		// minus phase
    PLUS_PHASE = 1		// plus phase
  };

  enum PhaseOrder {
    PLUS_ONLY,			// only present the plus phase (associative-only)
    MINUS_PLUS,			// standard minus-plus (err and assoc)
    MINUS_PLUS_NOTHING		// auto-encoder version with final 'nothing' minus phase
  };

  PhaseOrder	phase_order;	// number and order of phases to present
  Counter	phase_no;	// Current phase number
  Phase		phase;		// state variable for phase
  StateInit	trial_init;	// how to initialize network state at start of trial
  bool		no_plus_stats;	// don't do stats/logging in the plus phase
  bool		no_plus_test; 	// don't run the plus phase when testing

  void		C_Code();	// modify to use the no_plus_stats flag
  
  void		Init_impl();
  void		UpdateState();
  void		Final();
  bool		Crit()		{ return SchedProcess::Crit(); }

  virtual void	InitInhib();
  // #BUTTON #CONFIRM initialize the inhibitory feedback parameters

  virtual void	InitState();
  virtual void	DecayState();
  virtual void	Compute_dWt();

  void		GetCntrDataItems();
  void		GenCntrLog(LogData* ld, bool gen);

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  void	Copy_(const LeabraTrial& cp);
  COPY_FUNS(LeabraTrial, TrialProcess);
  TA_BASEFUNS(LeabraTrial);
};

//////////////////////////
// 	Stats 		//
//////////////////////////

class BioBelMaxDa : public Stat {
  // ##COMPUTE_IN_SettleProcess ##LOOP_STAT stat that computes when equilibrium is
public:
  LeabraSettle* settle_proc;	// #READ_ONLY #NO_SAVE the settle process
  StatVal	da;		// delta-activation

  void		RecvCon_Run(Unit*)	{}; // don't do these!
  void		SendCon_Run(Unit*)	{};

  void		InitStat();
  void		Init();
  bool		Crit();
  void		Network_Init();

  void 		Unit_Stat(Unit* unit);
  void		Network_Stat();	// check cascade depth and set to above thresh

  void	UpdateAfterEdit();
  void 	Initialize();		// set minimums
  void	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(BioBelMaxDa);
  COPY_FUNS(BioBelMaxDa, Stat);
  TA_BASEFUNS(BioBelMaxDa);
};

class LeabraAeSE_Stat : public SE_Stat {
  // squared error for leabra auto-encoder
public:
  LeabraTrial* 	trial_proc;	// #READ_ONLY #NO_SAVE the trial process to get phase info
  Unit::ExtType	targ_or_comp;	// which unit values to compute SE of

  void		Network_Run();	// only run in certain phases based on targ_or_comp
  void		Init();		// only init at the right time too
  void 		Unit_Stat(Unit* unit);
  void		NameStatVals();
  
  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(LeabraAeSE_Stat);
  COPY_FUNS(LeabraAeSE_Stat, SE_Stat);
  TA_BASEFUNS(LeabraAeSE_Stat);
};


//////////////////////////////////////////
// 	Phase-Order  Environment	//
//////////////////////////////////////////

class PhaseOrderEventSpec : public EventSpec {
  // event specification including order of phases
public:
  enum PhaseOrder {
    MINUS_PLUS,			// minus phase, then plus phase
    PLUS_MINUS,			// plus phase, then minus phase
    PREV_PLUS_MINUS,		// use previous plus phase, only present minus
    PREV_MINUS_PLUS		// use previous minus phase, only present plus
  };

  PhaseOrder	phase_order;	// order to present phases of stimuli to network

  void	Initialize();
  void	Destroy();
  void	Copy_(const PhaseOrderEventSpec& cp);
  COPY_FUNS(PhaseOrderEventSpec, EventSpec);
  TA_BASEFUNS(PhaseOrderEventSpec);
};

//////////////////////////////////////////
// 	History-Based Learning		//
//////////////////////////////////////////

class LeabraHistUnit;
class LeabraHistUnitSpec;

class LeabraHistConSpec : public LeabraConSpec {
  // computes weight change based on recv current and send previous acts
public:
  float		hist_gain;	// strength of the history component of learning

  inline void 	C_Compute_dWt(LeabraCon* cn, float heb, float err);
  // combine associative and error-driven weight change, actually update dwt

  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraHistConSpec);
  COPY_FUNS(LeabraHistConSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraHistConSpec);
};

class LeabraHistUnitSpec : public LeabraUnitSpec {
  // adapts weights based on one step of activation history 
public:
  // apply learn thresh to history state too...
  void		InitState(BioBelUnit* u, BioBelLayer* lay);
  void		InitState(Unit* u)	{ LeabraUnitSpec::InitState(u); }
  void		Compute_dWt(Unit* u);
  void		Compute_dWt_post(LeabraUnit* u, LeabraLayer* lay);
  // update history values at point of weight update

  void	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraHistUnitSpec);
  COPY_FUNS(LeabraHistUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(LeabraHistUnitSpec);
};

class LeabraHistUnit : public LeabraUnit {
  // LEABRA unit with an single step activation history
public:
  float 	act_m_h;	// #NO_VIEW minus phase activation history
  float		act_p_h;	// #NO_VIEW plus phase activation history
  int		hist_cnt;	// number of times history has been updated

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LeabraHistUnit);
  COPY_FUNS(LeabraHistUnit, LeabraUnit);
  TA_BASEFUNS(LeabraHistUnit);
};

// combine associative and error-driven
inline void LeabraHistConSpec::C_Compute_dWt(LeabraCon* cn, float heb, float err) {
  cn->dwt = hist_gain * (k_err * err + k_hebb * heb);
}

inline void LeabraHistConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraUnit* lru = (LeabraUnit*)ru;
  float inc_cor, dec_cor;
  float savg = Compute_SAvgCor((LeabraCon_Group*)cg, inc_cor, dec_cor);
  // this pass is for the compute sender_(t-1), recv_(t) weight change
  for(int i=0; i<cg->size; i++) {
    LeabraHistUnit* su = (LeabraHistUnit*)cg->Un(i);
    LeabraCon* cn = (LeabraCon*)cg->Cn(i);
    if(su->hist_cnt >= 1) {	// only do history computation if history is there
      C_Compute_dWt(cn, 
		    C_Compute_Hebb(cn, lru->act_p, su->act_p_h, savg, inc_cor, dec_cor),
		    C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p_h, su->act_m_h));
    }
  }
  LeabraConSpec::Compute_dWt(cg, ru);
}

//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////

#endif // bleabra_h

