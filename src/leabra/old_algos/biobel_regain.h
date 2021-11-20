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

// biobel.h 

#ifndef biobel_h
#ifdef __GNUG__
#pragma interface
#endif
#define biobel_h

#include <pdp/base.h>
#include <pdp/netstru.h>
#include <pdp/sched_proc.h>
#include <ta_misc/fun_lookup.h>
#include <bleabra/bleabra_TA_type.h>
#include <math.h>
#include <values.h>

// pre-declare

class BioBelCon;
class BioBelConSpec;
class BioBelCon_Group;
class BioBelUnitSpec;
class BioBelUnit;
class BioBelUnit_Group;
class BioBelInhib;
class BioBelLayerSpec;
class BioBelLayer;
class BioBelMaxDa;

// _

// Relative Belief in biological terms (BioBel) 
// note that the weights are subject to scaling by wt_scale, but are stored
// in 0-1 normalized form (but wt_eff contains the scaled versions)

class BioBelCon : public Connection {
  // BioBel connection
public:
  float		wt_eff;		// effective weight value wt_scale * sigmoidal fun(wt)

  void 	Initialize()		{ wt_eff = 0.0f; }
  void	Destroy()		{ };
  TA_BASEFUNS(BioBelCon);
};

class BioBelCon_Group : public Con_Group {
  // BioBel relative belief connection group
public:
  float		wt_gain;	// #NO_SAVE most recent weight gain value

  void 	Initialize()		{ wt_gain = 1.0f; }
  void	Destroy()		{ };
  TA_BASEFUNS(BioBelCon_Group);
};

class BioBelConSpec : public ConSpec {
  // BioBel relative belief connection specs
public:
  float		wt_gain;	// gain of the effective weight sigmoidal function
  float		wt_scale;	// strength multiplier for each weight value
  float		regain_thr;	// difference threshold to trigger regain (regain to thresh diff)
  FunLookup	wteff_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT fun gets wt_eff from wt

  static float	WeightGainFun(float w, float gain) { // function for implementing weight gain
    float wrat;
    if(w == 0.0f) wrat = MAXFLOAT;
    else if(w == 1.0f) wrat = 0.0f;
    else wrat = (1.0f - w) / w;
    return 1.0f / (1.0f + powf(wrat, gain));
  }

  void		C_Compute_WtEff(BioBelCon* cn)
  { cn->wt_eff = wteff_fun.Eval(cn->wt); }

  float 	C_Compute_Net(BioBelCon* cn, Unit*, Unit* su)
  { return cn->wt * su->act; }
  float 	Compute_Net(Con_Group* cg, Unit* ru) {
    float rval=0.0f;
    CON_GROUP_LOOP(cg, rval += C_Compute_Net((BioBelCon*)cg->Cn(i), ru, cg->Un(i)));
    return wt_scale * rval;  }

  void 		C_Send_Net(BioBelCon* cn, Unit* ru, Unit* su)
  { ru->net += wt_scale * su->act * cn->wt; }
  virtual void 	Send_Net(Con_Group* cg, Unit* su)
  { CON_GROUP_LOOP(cg, C_Send_Net((BioBelCon*)cg->Cn(i), cg->Un(i), su)); }
  // sender-based net input computation

  void		C_WtMinMax(BioBelCon* cn, float& min_val, float& max_val)
  { min_val = MIN(cn->wt, min_val); max_val = MAX(cn->wt, max_val); }
  virtual float	WtMinMax(BioBelCon_Group* cg, BioBelUnit*, float& max_val) {
    float min_val = 1.0f; max_val = 0.0f;
    CON_GROUP_LOOP(cg, C_WtMinMax((BioBelCon*)cg->Cn(i), min_val, max_val));
    return min_val; }
  // compute min - max of weights in this layer

  void		C_ReGainWeights(BioBelCon* cn, float gain)
  { cn->wt = WeightGainFun(cn->wt, gain); }
  virtual void	ReGainWeights(BioBelCon_Group* cg, BioBelUnit* ru) {
    float max_val; float min_val = WtMinMax(cg, ru, max_val);
    min_val = .5f - min_val;  max_val = max_val - .5f;
    if(max_val < regain_thr) {
      max_val = MAX(min_val, max_val) + .5f;  min_val = regain_thr + .5f;
      cg->wt_gain = logf((1.0f - min_val)/min_val) / logf((1.0f - max_val)/max_val);
      CON_GROUP_LOOP(cg, C_ReGainWeights((BioBelCon*)cg->Cn(i), cg->wt_gain));
    }
  }

//   void 		C_InitWtState(Connection* cn, Unit* ru, Unit* su) {
//     ConSpec::C_InitWtState(cn, ru, su); // C_Compute_WtEff((BioBelCon*)cn);
//   }
  void 		InitWtState(Con_Group* cg, Unit* ru) {
    CON_GROUP_LOOP(cg, C_InitWtState(cg->Cn(i), ru, cg->Un(i)));
    InitWtDelta(cg,ru);
    ((BioBelCon_Group*)cg)->wt_gain = 1.0f; // reset this..
    ReGainWeights((BioBelCon_Group*)cg, (BioBelUnit*)ru);
    ApplySymmetry(cg,ru);
  }

  virtual void	CreateWtEffFun(); // create the wteff_fun

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(BioBelConSpec);
  COPY_FUNS(BioBelConSpec, ConSpec);
  TA_BASEFUNS(BioBelConSpec);
};

class BioBelChannels : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in BioBel
public:
  float		e;		// excitatory
  float		l;		// leak
  float		i;		// inhibitory
  float		h;		// hysteresis (Ca)
  float		a;		// accomodation (k)

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const BioBelChannels& cp);
  COPY_FUNS(BioBelChannels, taBase);
  TA_BASEFUNS(BioBelChannels);
};

// this version is based on Abbott-like mechanisms
class VChanSpecs : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER voltage gated channel specs
public:
  bool		on;		// true if channel is on
  float		inc;	// additive increment to basis variable as a fctn of act
  float		dec;	// multiplicative decrement to basis variable 
  float		gain;	// overall gain of this conductance term (or value if thresholded)

  float	Compute_G(float basis) { // get the conductance from this channel
    float rval = 0.0f;
    if(on)
      rval = gain * basis;	// if not thresholded
    return rval;
  }

  void	UpdateBasis(float& basis, float act) {
    basis += act * inc - dec * basis;
  }

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const VChanSpecs& cp);
  COPY_FUNS(VChanSpecs, taBase);
  TA_BASEFUNS(VChanSpecs);
};

class VChanBasis : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER basis variables for vchannels
public:
  float		hyst;		// hysteresis
  float		acc_f;		// fast accomodation

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const VChanBasis& cp);
  COPY_FUNS(VChanBasis, taBase);
  TA_BASEFUNS(VChanBasis);
};

class ActFunSpecs : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activation function specifications
public:
  float		thresh;		// initial threshold value (prior to adaptation)
  float		gain;		// gain of the activation function
  float		v_m_rest;	// resting membrane potential
  float		g_e_rest;	// #HIDDEN excitatory input needed to balance leak at rest

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActFunSpecs);
  COPY_FUNS(ActFunSpecs, taBase);
  TA_BASEFUNS(ActFunSpecs);
};

class AdaptThreshSpecs : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER adapting threshold based on average act
public:
  float		dt;		// time constant for integrating activation average
  float		min;		// minimum avg act, above which no adaptation
  float		max;		// maximum avg act, below which no adaptation
  float		dthr;		// amount to change threshold in correct direction
  float		err;		// amount of error to introduce in order to correct (leabra)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(AdaptThreshSpecs);
  COPY_FUNS(AdaptThreshSpecs, taBase);
  TA_BASEFUNS(AdaptThreshSpecs);
};

class BioBelUnitSpec : public UnitSpec {
  // BioBel relative belief unit specifications
public:
  enum ActFun {
    NOISY_XX1,			// noisy version of x over x plus 1 (uses noise params)
    X_OVER_X_1,			// x over x plus 1
    LINEAR			// simple linear output function 
  };

  enum BiasWts {
    BIAS_NETIN,			// as an additive term in netinput
    BIAS_LEAK			// as a divisive term in the leak current
  };

  enum NoiseType {
    NO_NOISE,			// no noise added to processing
    VM_NOISE,			// noise in the value of v_m
    NETIN_NOISE,		// noise in the net input (g_e)
    ACT_NOISE			// noise in the activations
  };

  ActFun	act_fun;	// activation function to use
  BiasWts	bias_wts;	// how to model the effect of bias weights
  ActFunSpecs	act;		// specifications of the activation function
  AdaptThreshSpecs adapt_thr;	// adapting threshold specifications
  float		send_thresh;	// sending activation threshold
  MinMaxRange	vm_range;	// membrane potential range
  Random	v_m_init;	// what to initialize the membrane potential to
  float		vm_dt;		// step size to take in integrating the membrane potential
  float		net_dt;		// step size to take in integrating the net input
  BioBelChannels g_bar;		// scales value of conductances
  BioBelChannels e_rev;		// reversal potential associated with each conductance
  VChanSpecs	hyst;		// hysteresis v-gated chan (Ca2+, NMDA)
  VChanSpecs	acc_f;		// fast accomodation v-gated chan (delayed rectifier)
  NoiseType	noise_type;	// where to add noise in the processing
  Random	noise;		// what kind of noise?
  Schedule	noise_sched;	// schedule of noise variance over settling cycles
  FunLookup	nxx1_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT convolved gaussian and x/x+1 function as lookup table
  FunLookup	noise_conv;	// #HIDDEN #NO_SAVE #NO_INHERIT gaussian for convolution

  void 		InitWtState(Unit* u);

  virtual void	InitState(BioBelUnit* u, BioBelLayer* lay);
  void		InitState(Unit* u)	{ UnitSpec::InitState(u); }

  virtual void	ReConnect_Load(BioBelUnit* u, BioBelLayer* lay);
  // #IGNORE update non-saved variables after loading network

  virtual void	CompToTarg(BioBelUnit* u, BioBelLayer* lay);
  // copy comparison external inputs to target ones
  virtual void 	Compute_ClampNet(BioBelUnit* u, BioBelLayer* lay);
  // compute net input from hard-clamped inputs

  void 		Compute_Net(Unit* u) { UnitSpec::Compute_Net(u); }
  void 		Compute_Net(BioBelUnit* u, BioBelLayer* lay);
  // add ext input, send-based
  inline virtual void	Compute_NetAvg(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr);
  // compute netin average, also perform any post-avg netin increments (e.g. bias wts)
  virtual bool	Compute_SoftClamp(BioBelUnit* u, BioBelLayer* lay);
  // soft-clamps unit, returns true if unit is not above .5

  void		Compute_Act(Unit* u)	{ UnitSpec::Compute_Act(u); }
  virtual void 	Compute_Act(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr, int cycle);
  // compute the final activation

  float		Compute_IThresh(BioBelUnit* u);
  // compute inhibitory value that would place unit directly at threshold

  virtual void	DecayState(BioBelUnit* u, BioBelLayer* lay, float decay);
  // decay activation states towards initial values

  virtual void	AdaptThreshold(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr, int phase);
  // adapt the firing threshold based on long-term average activation

  virtual void	PostSettle(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr, int phase);
  // set stuff after settling is over (place holder)

  virtual void	CreateNXX1Fun();  // create convolved gaussian and x/x+1 

  void	UpdateAfterEdit();	// to set _impl sig
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(BioBelUnitSpec);
  COPY_FUNS(BioBelUnitSpec, UnitSpec);
  TA_BASEFUNS(BioBelUnitSpec);
};

class BioBelUnitChans : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in BioBel units
public:
  float		l;		// leak
  float		i;		// inhibitory
  float		h;		// hysteresis (Ca)
  float		a;		// accomodation (k)

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const BioBelUnitChans& cp);
  COPY_FUNS(BioBelUnitChans, taBase);
  TA_BASEFUNS(BioBelUnitChans);
};

class BioBelUnit : public Unit {
  // BioBel relative belief unit
public:
  float		clmp_net;	// #NO_VIEW hard-clamp net input (no need to recompute)
  float		net_norm;	// #NO_VIEW normalizer for netinput (number of conns)
  float		prv_net;	// #NO_VIEW previous net input (for time averaging)
  float		v_m;		// #NO_SAVE membrane potential
  float		thr;		// threshold for firing (adaptive)
  float		da;		// #NO_SAVE delta-activation (actually memb pot)
  VChanBasis	vcb;		// voltage-gated channel basis variables
  BioBelUnitChans gc;		// #NO_SAVE current unit channel conductances
  float		act_avg;	// average activation over long time intervals

  void		InitState(BioBelLayer* lay)
  { ((BioBelUnitSpec*)spec.spec)->InitState(this, lay); }

  void		ReConnect_Load(BioBelLayer* lay)	// #IGNORE
  { ((BioBelUnitSpec*)spec.spec)->ReConnect_Load(this, lay); }
       
  void		CompToTarg(BioBelLayer* lay)
  { ((BioBelUnitSpec*)spec.spec)->CompToTarg(this, lay); }
  void		Compute_ClampNet(BioBelLayer* lay)
  { ((BioBelUnitSpec*)spec.spec)->Compute_ClampNet(this, lay); }

  void		Compute_Net(BioBelLayer* lay)
  { ((BioBelUnitSpec*)spec.spec)->Compute_Net(this, lay); }
  void		Compute_NetAvg(BioBelLayer* lay, BioBelInhib* thr)
  { ((BioBelUnitSpec*)spec.spec)->Compute_NetAvg(this, lay, thr); }
  bool		Compute_SoftClamp(BioBelLayer* lay) 
  { return ((BioBelUnitSpec*)spec.spec)->Compute_SoftClamp(this, lay); }

  void		Compute_Act()	{ Unit::Compute_Act(); }
  void 		Compute_Act(BioBelLayer* lay, BioBelInhib* thr, int cycle) 
  { ((BioBelUnitSpec*)spec.spec)->Compute_Act(this, lay, thr, cycle); }

  float		Compute_IThresh()
  { return ((BioBelUnitSpec*)spec.spec)->Compute_IThresh(this); }

  void		DecayState(BioBelLayer* lay, float decay)
  { ((BioBelUnitSpec*)spec.spec)->DecayState(this, lay, decay); }
  void		PostSettle(BioBelLayer* lay, BioBelInhib* thr, int phase)
  { ((BioBelUnitSpec*)spec.spec)->PostSettle(this, lay, thr, phase); }

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(BioBelUnit);
  COPY_FUNS(BioBelUnit, Unit);
  TA_BASEFUNS(BioBelUnit);
};

typedef taPtrList<BioBelUnit> BioBelSort; // use this for sorting..

class LayerLink : public taOBase {
  // ##NO_TOKENS #NO_UPDATE_AFTER Link strength between layers, affects threshold
public:
  BioBelLayer*	layer;		// layer to link to
  float		link_wt;	// strength of the Link

  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void	CutLinks();
  void 	Copy_(const LayerLink& cp);
  COPY_FUNS(LayerLink, taOBase);
  TA_BASEFUNS(LayerLink);
};

class LayerLink_List : public taList<LayerLink> {
  // ##NO_TOKENS ##NO_UPDATE_AFTER list of LayerLink objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(LayerLink_List);
};

// misc data-holding structures

class AvgMaxVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds average and max statistics
public:
  float		avg;	// average value
  float		max;	// maximum value
  
  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const AvgMaxVals& cp) { avg = cp.avg; max = cp.max; }
  COPY_FUNS(AvgMaxVals, taBase);
  TA_BASEFUNS(AvgMaxVals);
};

class BioBelDecays : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds decay values
public:
  float		event;  // how much to decay between events
  float		phase;  // how much to decay between phases
  float		ae;
  // how much to decay for auto-encoder mode (between plus and nothing phases)

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const BioBelDecays& cp) { event = cp.event; phase = cp.phase; ae = cp.ae; }
  COPY_FUNS(BioBelDecays, taBase);
  TA_BASEFUNS(BioBelDecays);
};

class KWTASpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds parameters specifying kwta params
public:
  enum K_From {
    USE_K,			// use the k specified directly
    USE_PCT,			// use the percentage to compute the k
    USE_PAT_K			// use the activity level of the event pattern (>thresh)
  };

  K_From	k_from;		// how is the active_k determined?
  int		k;		// number of active units in the layer
  float		pct;		// desired proportion of activity 
  float		pat_q;		// threshold for pat_k based activity level

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(KWTASpec);
  COPY_FUNS(KWTASpec, taBase);
  TA_BASEFUNS(KWTASpec);
};

class KWTAVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for kwta stuff
public:
  int		k;       	// number of active units for this collection
  float		pct;		// actual percent activity in group
  float		pct_c;		// #HIDDEN complement of (1.0 - ) actual percent activity in group

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const KWTAVals& cp);
  COPY_FUNS(KWTAVals, taBase);
  TA_BASEFUNS(KWTAVals);
};

class ActInhibSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds parameters for act-based inhibition
public:
  float		dt;		// rate constant for integrating inhibition
  float		gain;		// gain of activation to give inhibition

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ActInhibSpec);
  COPY_FUNS(ActInhibSpec, taBase);
  TA_BASEFUNS(ActInhibSpec);
};

class InhibVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for inhibition
public:
  float		kwta;		// inhibition due to kwta function
  float		sact;		// inhibition due to sending activation
  float		ract;		// inhibition due to sending activation
  float		g_i;		// overall value of the inhibition
  float		g_i_orig; 	// #HIDDEN original value of the inhibition (before linking)

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const InhibVals& cp);
  COPY_FUNS(InhibVals, taBase);
  TA_BASEFUNS(InhibVals);
};

class BioBelLayerSpec : public LayerSpec {
  // BioBel relative belief layer specs, computes inhibitory input for all units in layer
public:
  enum Compute_I {		// how to compute the inhibition
    KWTA_INHIB,			// set inhibition betwen thresholds of k and k+1th
    SACT_INHIB,			// set inhibition from sending activation
    RACT_INHIB,			// set inhibition from recurrent activation
    SRACT_INHIB			// set inhibition from sending and recurrent activation
  };
  
  enum LayerLinking {
    NO_LINK,			// no linking
    LINK_INHIB			// link the inhibition
  };

  KWTASpec	kwta;		// how to calculate the desired activity level
  Compute_I	compute_i;	// how to compute the inhibition
  float		i_kwta_pt;	// point to place inhibition between k and k+1 for kwta
  ActInhibSpec 	i_sact;		// parameters for sending act based inhbition
  ActInhibSpec 	i_ract;		// parameters for recurrent act based inhbition
  bool		hard_clamp;	// whether to hard clamp any inputs to this layer or not
  MinMaxRange	clamp_range;	// range of clamp activation values
  float		stm_gain;	// minimum stimulus gain factor (when not hard clamping)
  float		d_stm_gain;	// delta to increase when target units not > .5
  BioBelDecays	decay;		// how much to decay the prior (towards initial_act)
  LayerLinking	layer_link;	// how to link the layers (if links exist)
  float		layer_link_gain;
  // gain of the layer linking function, how much layer's thresh depends on others

  virtual void	Compute_HardClamp(BioBelLayer* lay, int phase);
  // prior to settling: hard-clamp inputs
  virtual int	Compute_InputDist(BioBelLayer* lay, int phase);
  // prior to settling: compute layer's distance from input
  virtual void	Compute_Cascade(BioBelLayer* lay, int phase, int depth);
  // compute cascading of activation based on target depth from input
  virtual void	Compute_ClampNet(BioBelLayer* lay, int phase);
  // prior to settling: compute netinput for all units from hard-clamped

  virtual void 	Compute_Net(BioBelLayer* lay, int cycle, int phase);
  // stage zero: compute net inputs

  virtual void	Compute_Clamp_NetAvg(BioBelLayer* lay, int cycle, int phase);
  // stage one: clamp and compute averages of net inputs that were already computed
  virtual void	Compute_NetAvg(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr,
			       int cycle, int phase);
  virtual void	Compute_sAct(BioBelLayer* lay, int cycle, int phase);
  // compute avg and max of sending activations into layer
  virtual void	Compute_SoftClamp(BioBelLayer* lay, int cycle, int phase);
  // soft-clamp inputs by adding to net input

  void		InitInhib(BioBelLayer* lay);
  // initialize the inhibitory state values
  void		Compute_Inhib(BioBelLayer* lay, int cycle, int phase);
  // stage two: compute the inhibition for layer
  virtual void	Compute_Inhib_impl(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr);
  // implementation of inhibition computation for either layer or unit group
  virtual void	Compute_Inhib_kWTA(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr);
  // implementation of max kwta inhibition computation
  virtual void	Compute_Inhib_sAct(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr);
  // implementation of sending activation based inhibition
  virtual void	Compute_Inhib_rAct(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr);
  // implementation of recurrent activation based inhibition
  virtual void	Compute_LinkInhib(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr,
				   int cycle);
  // compute inhibition after linkages with other layers are factored in
  void		Compute_Active_K(BioBelLayer* lay);
  virtual void	Compute_Active_K_impl(BioBelLayer* lay, Unit_Group* ug,
				      BioBelInhib* thr);
  virtual int	Compute_Pat_K(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr);

  virtual void 	Compute_Act(BioBelLayer* lay, int cycle, int phase);
  // stage three: compute final activation (including scaling)
  virtual void 	Compute_Act_impl(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr,
				 int cycle, int phase);

  virtual void	DecayEvent(BioBelLayer* lay);
  virtual void	DecayPhase(BioBelLayer* lay);
  virtual void	DecayAE(BioBelLayer* lay);

  virtual void	CompToTarg(BioBelLayer* lay);
  // change comparison external inputs into target external inputs
  virtual void	PostSettle(BioBelLayer* lay, int phase);
  // after settling, keep track of phase variables, etc.

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(BioBelLayerSpec);
  COPY_FUNS(BioBelLayerSpec, LayerSpec);
  TA_BASEFUNS(BioBelLayerSpec);
};

SpecPtr_of(BioBelLayerSpec);

class BioBelInhib {
  // holds threshold-computation values, used as a parent class for layers, etc
public:
  BioBelSort 	active_buf;	// #HIDDEN #NO_SAVE list of active units
  BioBelSort 	inact_buf;	// #HIDDEN #NO_SAVE list of inactive units

  AvgMaxVals	netin;		// net input values for the layer
  AvgMaxVals	acts;		// activation values for the layer
 
  KWTAVals	kwta;		// values for kwta -- activity levels, etc
  InhibVals	i_val;		// inhibitory values

  void		Inhib_SetVals(float val)	{ i_val.g_i = val; i_val.g_i_orig = val; }
  void		Inhib_ResetSortBuf() 		{ active_buf.size = 0; inact_buf.size = 0; }
  void		Inhib_InitState(BioBelLayerSpec* lay);
  void	        Inhib_Initialize();
};

class BioBelLayer : public Layer, public BioBelInhib {
  // BioBel Layer: implicit inhibition for soft kWTA behavior
public:
  AvgMaxVals	sact;		// sending activation values (avg and max, weighted by wt_scale)
  BioBelLayerSpec_SPtr	spec;	// the spec for this layer
  LayerLink_List layer_links;	// list of layers to link inhibition with
  float		stm_gain;	// actual stim gain (failsafe stm_gain for soft clamping)
  bool		hard_clamped;	// this layer is actually hard clamped
  int	        input_dist;	// number of layers away from external input

  void		Build();
  void		InitState();  	// initialize layer state too
  void		InitWtState();
  void		InitInhib() 	{ spec->InitInhib(this); } // initialize inhibitory state
  void		ModifyState()	{ spec->DecayEvent(this); } // this is what modify means..

  void		ReConnect_Load(); // #IGNORE also call stuff at unit level

  void	Compute_HardClamp(int phase)  	{ spec->Compute_HardClamp(this, phase); }
  int	Compute_InputDist(int phase)	{ return spec->Compute_InputDist(this, phase); }
  void	Compute_Cascade(int phase, int depth) { spec->Compute_Cascade(this, phase, depth); }
  void	Compute_ClampNet(int phase)  	{ spec->Compute_ClampNet(this, phase); }

  void	Compute_Net()			{ spec->Compute_Net(this, -1, 0); }
  void	Compute_Net(int cycle, int phase)
  { spec->Compute_Net(this, cycle, phase); }

  void	Compute_Clamp_NetAvg(int cycle, int phase)
  { spec->Compute_Clamp_NetAvg(this, cycle, phase); }
  void	Compute_Inhib(int cycle, int phase)
  { spec->Compute_Inhib(this, cycle, phase); }
  void	Compute_Act()			{ spec->Compute_Act(this, -1, 0); }
  void	Compute_Act(int cycle, int phase) { spec->Compute_Act(this, cycle, phase); }
  void	Compute_Active_K()		{ spec->Compute_Active_K(this); }

  void	DecayEvent()	{ spec->DecayEvent(this); } // decay between events
  void	DecayPhase()    { spec->DecayPhase(this); } // decay between phases
  void	DecayAE()  	{ spec->DecayAE(this); }    // decay for auto-encoder

  void	CompToTarg()		{ spec->CompToTarg(this); }
  void	PostSettle(int phase)	{ spec->PostSettle(this, phase); }

  virtual void	ResetSortBuf();

  bool		SetLayerSpec(LayerSpec* sp);
  LayerSpec*	GetLayerSpec()		{ return (LayerSpec*)spec.spec; }

  void	UpdateAfterEdit();	// reset sort_buf after any edits..

  void	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(BioBelLayer);
  COPY_FUNS(BioBelLayer, Layer);
  TA_BASEFUNS(BioBelLayer);
};

class BioBelUnit_Group : public Unit_Group, public BioBelInhib {
  // for independent subgroups of competing units within a single layer
public:

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  TA_BASEFUNS(BioBelUnit_Group);
};

// inlines

// time-average netinput accumulation
void BioBelUnitSpec::Compute_NetAvg(BioBelUnit* u, BioBelLayer*, BioBelInhib*) {
  u->net /= u->net_norm;	// normalize the netinput
  u->net *= g_bar.e;		// then multiply by gbar..
  u->net = u->prv_net + net_dt * (u->net - u->prv_net);
  u->prv_net = u->net;
}


//////////////////////////
// 	Processes	//
//////////////////////////

// see bleabra.h,cc for examples of BioBel processes

#endif // biobel_h
