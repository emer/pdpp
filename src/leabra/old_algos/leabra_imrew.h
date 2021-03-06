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

// leabra.h 

#ifndef leabra_h
#define leabra_h

#include <pdp/pdpshell.h>	// needed for Wizard
#include <ta_misc/fun_lookup.h>
#include <leabra/leabra_TA_type.h>
#include <math.h>
#include <limits.h>
#include <float.h>

// pre-declare

class LeabraCon;
class LeabraConSpec;
class LeabraBiasSpec;
class LeabraCon_Group;

class LeabraUnitSpec;
class LeabraUnit;
class LeabraUnit_Group;

class LeabraInhib;
class LeabraLayerSpec;
class LeabraLayer;

class LeabraCycle;
class LeabraSettle;
class LeabraTrial;

class LeabraMaxDa;

// _

// The Leabra algorithm:

// Local, Error-driven and Associative, Biologically Realistic Algorithm
// Implements a balance between Hebbian and error-driven learning.

// Hebbian learning is performed using conditional principal
// components analysis (CPCA) algorithm with correction factor for
// sparse expected activity levels.

// Error driven learning is performed using GeneRec, which is a
// generalization of the Recirculation algorithm, and approximates
// Almeida-Pineda recurrent backprop.  The symmetric, midpoint version
// of GeneRec is used, which is equivalent to the contrastive Hebbian
// learning algorithm (CHL).  See O'Reilly (1996; Neural Computation)
// for more details.

// The activation function is a point-neuron approximation with both
// discrete spiking and continuous rate-code output.

// Layer or unit-group level inhibition can be computed directly using
// a k-winners-take-all (KWTA) function, producing sparse distributed
// representations.

// The net input is computed as an average, not a sum, over
// connections, based on normalized, sigmoidaly transformed weight
// values, which are subject to scaling on a connection-group level to
// alter relative contributions.  Automatic scaling is performed to
// compensate for differences in expected activity level in the
// different projections.

// The underlying internal linear weight value upon which learning occurs
// is computed from the nonlinear (sigmoidal) weight value prior to making
// weight changes, and is then converted back in UpdateWeights.  The linear
// weight is always stored as a negative value, so that shared weights or multiple
// weight updates do not try to linearize the already-linear value.  The learning
// rules have been updated to assume that wt is negative (and linear).

class LeabraCon : public Connection {
  // Leabra connection
public:
  float		dwt;		// #NO_VIEW #NO_SAVE resulting net weight change
  float		pdw;		// #NO_SAVE previous delta-weight change

  void 	Initialize()		{ dwt = pdw = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const LeabraCon& cp);
  COPY_FUNS(LeabraCon, Connection);
  TA_BASEFUNS(LeabraCon);
};

class WtScaleSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER weight scaling specification
public:
  float		abs;		// #DEF_1 absolute scaling (not subject to normalization: directly multiplies weight values)
  float		rel;		// [Default: 1] relative scaling (subject to normalization across all other projections into unit)

  inline float	NetScale() 	{ return abs * rel; }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(WtScaleSpec);
  COPY_FUNS(WtScaleSpec, taBase);
  TA_BASEFUNS(WtScaleSpec);
};

class WtSigSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER sigmoidal weight function specification
public:
  float		gain;		// #DEF_6 gain (contrast, sharpness) of the weight contrast function (1 = linear)
  float		off;		// #DEF_1.25 offset of the function (1=centered at .5, >1=higher, <1=lower)

  // function for implementing weight sigmodid
  static inline float	SigFun(float w, float gain, float off) {
    if(w <= 0.0f) return 0.0f;
    if(w >= 1.0f) return 1.0f;
    return 1.0f / (1.0f + powf(off * (1.0f - w) / w, gain));
  }

  // function for implementing inverse of weight sigmoid
  static inline float	SigFunInv(float w, float gain, float off) {
    if(w <= 0.0f) return 0.0f;
    if(w >= 1.0f) return 1.0f;
    return 1.0f / (1.0f + powf((1.0f - w) / w, 1.0f / gain) / off);
  }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(WtSigSpec);
  COPY_FUNS(WtSigSpec, taBase);
  TA_BASEFUNS(WtSigSpec);
};

class LearnMixSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER mixture of learning factors (hebbian vs. error-driven) specification
public:
  float		hebb;		// [Default: .01] amount of hebbian learning (should be relatively small, can be effective at .0001)
  float		err;		// #READ_ONLY #SHOW [Default: .99] amount of error driven learning, automatically computed to be 1-hebb

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LearnMixSpec);
  COPY_FUNS(LearnMixSpec, taBase);
  TA_BASEFUNS(LearnMixSpec);
};

class FixedSAvg : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER provide fixed sending avg activity level for netin computation (and SAvgCor)
public:
  bool		fix;		// #DEF_false use fixed val of sending avg activity level for normalizing netinput
  float		savg;		// #CONDEDIT_ON_fix:true [Default: .25] fixed sending average activity value
  bool		div_gp_n;	// #DEF_false in computing average netin, divide by the recv group n (number of actual connections), not the layer n

  float 	GetSAvg(float slayer_pct)
  { float rval = slayer_pct; if(fix) rval = savg; return rval; }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(FixedSAvg);
  COPY_FUNS(FixedSAvg, taBase);
  TA_BASEFUNS(FixedSAvg);
};

class SAvgCorSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER sending average activation correction specifications
public:
  enum SAvgSource {
    SLAYER_AVG_ACT,		// use actual average activation computed over sending layer 
    SLAYER_TRG_PCT,		// use target activation (from kwta) of sending layer
    FIXED_SAVG,			// use the value specified in the fix_savg.savg
    COMPUTED_SAVG		// compute the average directly over the connections into each unit (EXPENSIVE!)
  };

  float		cor;		// #DEF_0.4 proportion of correction to apply (0=none, 1=all, .5=half, etc)
  SAvgSource	src;		// #DEF_SLAYER_AVG_ACT source of sending average act for renormalizing hebbian learning
  float		thresh;		// #DEF_0.001 threshold of sending average activation below which learning does not occur (prevents learning when there is no input)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SAvgCorSpec);
  COPY_FUNS(SAvgCorSpec, taBase);
  TA_BASEFUNS(SAvgCorSpec);
};

class LeabraConSpec : public ConSpec {
  // Leabra connection specs
public:
  bool		inhib;		// #DEF_false makes the connection inhibitory (to g_i instead of net)
  WtScaleSpec	wt_scale;	// scale weight values, both relative and absolute factors
  WtSigSpec	wt_sig;		// sigmoidal weight function for contrast enhancement: high gain makes weights more binary & discriminative
  float		lrate;		// #DEF_0.01 learning rate -- how fast do the weights change per experience
  float		cur_lrate;	// #READ_ONLY #NO_INHERIT #SHOW current actual learning rate = lrate * lrate_sched current value (* 1 if no lrate_sched)
  Schedule	lrate_sched;	// schedule of learning rate over training epochs (multiplies lrate!)
  LearnMixSpec	lmix;		// mixture of hebbian & err-driven learning
  FixedSAvg	fix_savg;	// how to normalize the computation of average net input: fixed sending avg act value for normalizing netin
  SAvgCorSpec	savg_cor;	// for Hebbian: correction for sending average act levels (i.e., renormalization)
  
  FunLookup	wt_sig_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT computes wt sigmoidal fun
  FunLookup	wt_sig_fun_inv;	// #HIDDEN #NO_SAVE #NO_INHERIT computes inverse of wt sigmoidal fun
  WtSigSpec	wt_sig_fun_lst;	// #HIDDEN #NO_SAVE #NO_INHERIT last values of wt sig parameters for which the wt_sig_fun's were computed; prevents excessive updating
  float		wt_sig_fun_res;	// #HIDDEN #NO_SAVE #NO_INHERIT last values of resolution parameters for which the wt_sig_fun's were computed

  void		C_Compute_WtFmLin(LeabraCon_Group*, LeabraCon* cn)
  { if(cn->wt < 0.0f) cn->wt = wt_sig_fun.Eval(-cn->wt);  }
  // weight is negative if it is in its linear form, only perform if negative
  inline virtual void	Compute_WtFmLin(LeabraCon_Group* gp);
  // compute actual weight value from linear weight value

  void		C_Compute_LinFmWt(LeabraCon_Group*, LeabraCon* cn)
  { if(cn->wt >= 0.0f) cn->wt = - wt_sig_fun_inv.Eval(cn->wt); }
  // weight is negative if it is in its linear form, only perform if positive
  inline virtual void	Compute_LinFmWt(LeabraCon_Group* gp);
  // compute linear weight value from actual weight value

  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su) {
    ConSpec::C_InitWtState(cg, cn, ru, su); LeabraCon* lcn = (LeabraCon*)cn;
    lcn->pdw = 0.0f; }

  void 		C_InitWtDelta(Con_Group* cg, Connection* cn, Unit* ru, Unit* su)
  { ConSpec::C_InitWtDelta(cg, cn, ru, su); ((LeabraCon*)cn)->dwt=0.0f; }

  inline float 	C_Compute_Net(LeabraCon* cn, Unit*, Unit* su);
  inline float 	Compute_Net(Con_Group* cg, Unit* ru);
  // receiver-based net input 

  inline void 	C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, Unit* su);
  inline void 	Send_Net(Con_Group* cg, Unit* su);
  // sender-based net input computation

  inline void 	C_Send_Inhib(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void 	Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based inhibitiory net input computation

  inline void 	C_Send_NetDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void 	Send_NetDelta(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based delta net input computation (send_delta mode only)

  inline void 	C_Send_InhibDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void 	Send_InhibDelta(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based delta inhibitiory net input computation (send_delta mode only)

  inline void 	C_Send_ClampNet(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void 	Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based net input computation for clamp net

  inline void	Compute_SAvgCor(LeabraCon_Group* cg, LeabraUnit* ru);
  // compute hebb correction scaling terms for sending average act, returns inc cor, and dec cor

  inline float	C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg,
			       float ru_act, float su_act);
  // compute Hebbian associative learning

  inline float 	C_Compute_Err(LeabraCon*, float ru_act_p, float ru_act_m,
				  float su_act_p, float su_act_m);
  // compute generec error term, sigmoid case

  inline void 	C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus, float heb, float err);
  // combine associative and error-driven weight change, actually update dwt

  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);
  // compute weight change: make new one of these for any C_ change above: hebb, err, dwt

  inline virtual void	B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru);
  // compute bias weight change for netin model of bias weight

  inline void		C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg, 
					LeabraUnit* ru, LeabraUnit* su);
  inline void		UpdateWeights(Con_Group* cg, Unit* ru);
  inline virtual void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru);

  virtual void	SetCurLrate(int epoch);
  // set current learning rate based on schedule given epoch

  virtual void	CreateWtSigFun(); // create the wt_sig_fun and wt_sig_fun_inv

  virtual void	Defaults();	// #BUTTON #CONFIRM restores default parameter settings: warning -- you will lose any unique parameters you might have set!

  virtual void	GraphWtSigFun(GraphLog* graph_log);
  // #BUTTON #NULL_OK graph the sigmoidal weight contrast enhancement function (NULL = new graph log)

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraConSpec);
  COPY_FUNS(LeabraConSpec, ConSpec);
  TA_BASEFUNS(LeabraConSpec);
};

class LeabraBiasSpec : public LeabraConSpec {
  // Leabra bias-weight connection specs (bias wts are a little bit special)
public:
  float		dwt_thresh;  // #DEF_0.1 don't change if dwt < thresh, prevents buildup of small changes

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru);

  bool	CheckObjectType_impl(TAPtr obj);
  // make sure this can only go with a unitspec, not a project or a congroup

  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraBiasSpec);
  COPY_FUNS(LeabraBiasSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraBiasSpec);
};

class LeabraCon_Group : public Con_Group {
  // Leabra connection group
public:
  float		scale_eff;	// #NO_SAVE effective scale parameter for netin
  float		savg;		// #NO_SAVE sending average activation
  float		max_cor;	// #NO_SAVE savg correction factor for cpca
  float		net; 		// total (averaged) net input from all connections in group
  float		p_savg;		// #NO_SAVE previous sending average activation for time-based learning
  float		p_max_cor;	// #NO_SAVE previous savg correction factor for cpca for time-based learning

  void	Compute_LinFmWt()  { ((LeabraConSpec*)spec.spec)->Compute_LinFmWt(this); }

  void	Compute_WtFmLin()  { ((LeabraConSpec*)spec.spec)->Compute_WtFmLin(this); }

  void	SetCurLrate(int epoch) { ((LeabraConSpec*)spec.spec)->SetCurLrate(epoch); }

  inline void 	Send_ClampNet(LeabraUnit* su);

  inline void 	Send_NetDelta(LeabraUnit* su)
  { ((LeabraConSpec*)spec.spec)->Send_NetDelta(this, su); }

  virtual void	EncodeState(LeabraUnit*)  { p_savg = savg; p_max_cor = max_cor; }
  // encode current state information (for time-based learning)

  void 	Initialize();
  void	Destroy()		{ };
  void	Copy_(const LeabraCon_Group& cp);
  COPY_FUNS(LeabraCon_Group, Con_Group);
  TA_BASEFUNS(LeabraCon_Group);
};

class ActFunSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activation function specifications
public:
  float		thr;		// #DEF_0.25 threshold value Theta (Q) for firing output activation 
  float		gain;		// #DEF_600 gain (gamma) of the sigmoidal rate-coded activation function 
  float		nvar;		// #DEF_0.005 variance of the Gaussian noise kernel for convolving with XX1 in NOISY_XX1
  float		avg_dt;		// #DEF_0.005 time constant for integrating activation average (computed across trials)
  bool		send_delta;	// #DEF_false #READ_ONLY send only changes in activation when it changes beyond opt_thresh.delta: COPIED FROM LeabraSettle!

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActFunSpec);
  COPY_FUNS(ActFunSpec, taBase);
  TA_BASEFUNS(ActFunSpec);
};

class SpikeFunSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER spiking activation function specs
public:
  float		decay;		// #DEF_0.05 exponential decay of activation produced by a spike (act(t+1) = act(t) * (1-decay))
  float		v_m_r;		// #DEF_0 post-spiking membrane potential to reset to, produces refractory effect
  float		eq_gain;	// #DEF_10 gain for computing act_eq relative to actual average: act_eq = eq_gain * (spikes/cycles)
  float		eq_dt;		// #DEF_0.02 if non-zero, eq is computed as a running average with this time constant
  float		hard_gain;	// #DEF_0.4 gain for hard-clamped external inputs, mutliplies ext.  constant external inputs otherwise have too much influence compared to spiking ones: Note: soft clamping is strongly recommended

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SpikeFunSpec);
  COPY_FUNS(SpikeFunSpec, taBase);
  TA_BASEFUNS(SpikeFunSpec);
};

class DepressSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER depressing synapses activation function specs
public:
  enum PSpike {
    P_NXX1,			// probability of spiking is based on NOISY_XX1 f(Vm - Q)
    P_LINEAR			// probability of spiking is based on LINEAR f(Vm - Q)
  };

  PSpike	p_spike;	// how to compute the probability of spiking, which is then mult by amp of spiking
  float		rec;		// #DEF_0.2 rate of recovery of spike amplitude (determines overall time constant of depression function)
  float		asymp_act;	// #DEF_0.5 asymptotic activation value (as proportion of 1) for a fully active unit (determines depl value)
  float		depl;		// #READ_ONLY #SHOW rate of depletion of spike amplitude as a function of activation output (computed from rec, asymp_act)
  float		max_amp;	// #READ_ONLY #SHOW maximum amplitude required to maintain asymptotic firing at normal clamp levels (copied to act_range.max) 

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(DepressSpec);
  COPY_FUNS(DepressSpec, taBase);
  TA_BASEFUNS(DepressSpec);
};

class OptThreshSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER optimization thresholds for faster processing
public:
  float		send;		// #DEF_0.1 don't send activation when act <= send -- greatly speeds processing
  float		delta;		// #DEF_0.005 don't send activation changes until they exceed this threshold: only for when LeabraSettle::send_delta is on!
  float		learn;		// #DEF_0.01 don't learn on recv unit weights when both phase acts <= learn
  bool		updt_wts;	// #DEF_true whether to apply learn threshold to updating weights (otherwise always update)
  float		phase_dif;	// #DEF_0 don't learn when +/- phase difference ratio (- / +) < phase_dif (.8 when used, but off by default)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(OptThreshSpec);
  COPY_FUNS(OptThreshSpec, taBase);
  TA_BASEFUNS(OptThreshSpec);
};

class DtSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER time constants
public:
  float		vm;		// #DEF_0.2 membrane potential time constant -- if units oscillate between 0 and 1, this is too high!  reduce.
  float		net;		// #DEF_0.7 net input time constant -- how fast to update net input (damps oscillations)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(DtSpec);
  COPY_FUNS(DtSpec, taBase);
  TA_BASEFUNS(DtSpec);
};

class LeabraChannels : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in Leabra
public:
  float		e;		// Excitatory (glutamatergic synaptic sodium (Na) channel)
  float		l;		// Constant leak (potassium, K+) channel 
  float		i;		// inhibitory
  float		h;		// hysteresis (Ca)
  float		a;		// accomodation (k)

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const LeabraChannels& cp);
  COPY_FUNS(LeabraChannels, taBase);
  TA_BASEFUNS(LeabraChannels);
};

class VChanSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER voltage gated channel specs
public:
  bool		on;		// #DEF_false true if channel is on
  float		b_dt;		// #CONDEDIT_ON_on:true time constant for integrating basis variable (basis ~ intracellular calcium which builds up slowly as function of activation)
  float		a_thr;		// #CONDEDIT_ON_on:true activation threshold of the channel: when basis > a_thr, conductance starts to build up (channels open)
  float		d_thr;		// #CONDEDIT_ON_on:true deactivation threshold of the channel: when basis < d_thr, conductance diminshes (channels close)
  float		g_dt;		// #CONDEDIT_ON_on:true time constant for changing conductance (activating or deactivating)
  bool		init;		// #CONDEDIT_ON_on:true initialize variables when state is intialized between trials (else with weights)

  void	UpdateBasis(float& basis, bool& on_off, float& gc, float act) {
    if(on) {
      basis += b_dt * (act - basis);
      if(basis > a_thr)
	on_off = true;
      if(on_off && (basis < d_thr))
	on_off = false;
      gc += g_dt * ((float)on_off - gc);
    }
  }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(VChanSpec);
  COPY_FUNS(VChanSpec, taBase);
  TA_BASEFUNS(VChanSpec);
};

class PhaseSharpSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER phase-based sharpening specification: activations are sharper in plus phase
public:
  enum SharpType {
    NONE,			// don't use phase-based sharpening
    SIG,			// sharpen plus phase activation values by passing through sigmoidal function at each point in time
    SIG_END,			// sharpen final plus phase activation values by passing through sigmoidal function: only at end of settling, not during
    NETIN			// sharpen plus phase net input values by increasing effective g_bar.e and .i values in plus phase
  };

  SharpType	type;		// type of sharpening to apply
  float		gain;		// #CONDEDIT_ON_type:SIG,SIG_END #DEF_1.01 gain of sigmoidal function for SIG -- extent above 1.0 determines sharpening
  float		off;		// #CONDEDIT_ON_type:SIG,SIG_END #DEF_1 offset of sigmoidal function for SIG -- >1=offset >.5, etc
  float		delta;		// #CONDEDIT_ON_type:NETIN how much to increase g_bar.e,.i values in plus phase 
  float		i_gain;		// #CONDEDIT_ON_type:NETIN #DEF_1.6 extra gain term for inhibitory netinput -- gain on inhib has smaller impact due to reversal potential dynamics
  float		act_gain;	// #CONDEDIT_ON_type:NETIN #DEF_0.1 extra netin gain as a function of unit activation


  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(PhaseSharpSpec);
  COPY_FUNS(PhaseSharpSpec, taBase);
  TA_BASEFUNS(PhaseSharpSpec);
};

class ActRegSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activity regulation via weight adjustment
public:
  bool		on;		// whether to activity regulation is on (active) or not
  float		min;		// #CONDEDIT_ON_on:true #DEF_0.001 increase weights for units below this level of average activation
  float		max;		// #CONDEDIT_ON_on:true #DEF_0.35 decrease weights for units above this level of average activation 
  float		wt_dt;		// #CONDEDIT_ON_on:true #DEF_0.2 rate constant for making weight changes to rectify over-activation (dwt ~= wt_dt * wt)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActRegSpec);
  COPY_FUNS(ActRegSpec, taBase);
  TA_BASEFUNS(ActRegSpec);
};

class LeabraUnitSpec : public UnitSpec {
  // Leabra unit specifications, point-neuron approximation
public:
  enum ActFun {
    NOISY_XX1,			// x over x plus 1 convolved with Gaussian noise (noise is nvar)
    XX1,			// x over x plus 1, hard threshold, no noise convolution
    LINEAR,			// simple linear output function (still thesholded)
    DEPRESS,			// depressing synapses activation function (rate coded)
    SPIKE			// discrete spiking activations (spike when > thr)
  };

  enum NoiseType {
    NO_NOISE,			// no noise added to processing
    VM_NOISE,			// noise in the value of v_m (membrane potential)
    NETIN_NOISE,		// noise in the net input (g_e)
    ACT_NOISE			// noise in the activations
  };

  ActFun	act_fun;	// activation function to use
  ActFunSpec	act;		// activation function specs
  SpikeFunSpec	spike;		// #CONDEDIT_ON_act_fun:SPIKE spiking function specs (only for act_fun = SPIKE)
  DepressSpec	depress;	// #CONDEDIT_ON_act_fun:DEPRESS depressing synapses activation function specs, note that act_range deterimines range of spk_amp spiking amplitude, max should be > 1
  OptThreshSpec	opt_thresh;	// optimization thresholds for speeding up processing when units are basically inactive
  MinMaxRange	clamp_range;	// range of clamped activation values (min, max, 0, .95 std), don't clamp to 1 because acts can't reach, so .95 instead
  MinMaxRange	vm_range;	// membrane potential range (min, max, 0-1 for normalized, -90-50 for bio-based)
  Random	v_m_init;	// what to initialize the membrane potential to (mean = .15, var = 0 std)
  DtSpec	dt;		// time constants (rate of updating): membrane potential (vm) and net input (net)
  LeabraChannels g_bar;		// [Defaults: 1, .1, 1, .1, .5] maximal conductances for channels
  LeabraChannels e_rev;		// [Defaults: 1, .15, .15, 1, 0] reversal potentials for each channel
  VChanSpec	hyst;		// [Defaults: .05, .8, .7, .1] hysteresis (excitatory) v-gated chan (Ca2+, NMDA)
  VChanSpec	acc;		// [Defaults: .01, .5, .1, .1] accomodation (inhibitory) v-gated chan (K+)
  PhaseSharpSpec phase_sharp;	// phase-based sharpening, makes activations sharper in plus phase as extra unsupervised learning signal
  ActRegSpec	act_reg;	// activity regulation via global scaling of weight values
  NoiseType	noise_type;	// where to add random noise in the processing (if at all)
  Random	noise;		// #CONDEDIT_OFF_noise_type:NO_NOISE distribution parameters for random added noise
  Schedule	noise_sched;	// #CONDEDIT_OFF_noise_type:NO_NOISE schedule of noise variance over settling cycles

  FunLookup	nxx1_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT convolved gaussian and x/x+1 function as lookup table
  FunLookup	noise_conv;	// #HIDDEN #NO_SAVE #NO_INHERIT gaussian for convolution
  float		phase_delta;	// #READ_ONLY external plus-minus phase dif (error signal), applied in post settle

  void 		InitWtState(Unit* u);

  virtual void	SetCurLrate(LeabraUnit* u, LeabraTrial* trl, int epoch);
  // set current learning rate based on epoch

  //////////////////////////////////////////
  //	Stage 0: at start of settling	  // 
  //////////////////////////////////////////

  virtual void	InitDelta(LeabraUnit* u);
  virtual void	InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ UnitSpec::InitState(u); }

  virtual void 	Compute_NetScale(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // compute net input scaling values and input from hard-clamped inputs
  virtual void 	Send_ClampNet(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // compute net input from hard-clamped inputs (sender based)

  //////////////////////////////////
  //	Stage 1: netinput 	  //
  //////////////////////////////////

  void 		Send_Net(Unit* u) { UnitSpec::Send_Net(u); }
  void 		Send_Net(LeabraUnit* u, LeabraLayer* lay);  // add ext input, sender-based

  virtual void 	Send_NetDelta(LeabraUnit* u, LeabraLayer* lay);

  ////////////////////////////////////////////////////////////////
  //	Stage 2: netinput averages and clamping (if necc)	//
  ////////////////////////////////////////////////////////////////

  inline virtual void	Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute netin average
  inline virtual void	Compute_InhibAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute inhib netin average
  virtual void	Compute_HardClamp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  virtual bool	Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // soft-clamps unit, returns true if unit is not above .5

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual float	Compute_IThresh(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // compute inhibitory value that would place unit directly at threshold

  ////////////////////////////////////////
  //	Stage 4: the final activation 	//
  ////////////////////////////////////////

  void		Compute_Act(Unit* u)	{ UnitSpec::Compute_Act(u); }
  virtual void 	Compute_Act(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute the final activation: calls following function steps

  virtual void Compute_Conduct(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute input conductance values in the gc variables
  virtual void Compute_Vm(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute the membrante potential from input conductances
  virtual void Compute_ActFmVm(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute the activation from membrane potential
  virtual void Compute_SelfReg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  // compute self-regulatory currents (hysteresis, accommodation)

  ////////////////////////////////////////
  //	Stage 5: Between Events 	//
  ////////////////////////////////////////

  virtual void	PhaseInit(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // initialize external input flags based on phase
  virtual void	DecayPhase(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay);
  // decay activation states towards initial values: at phase-level boundary
  virtual void	DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay);
  // decay activation states towards initial values: at event-level boundary
  virtual void	ExtToComp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // change external inputs to comparisons (remove input)
  virtual void	TargExtToComp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // change target & external inputs to comparisons (remove targ & input)
  virtual void	PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			   LeabraTrial* trl, bool set_both=false);
  // set stuff after settling is over (set_both = both _m and _p for current)

  ////////////////////////////////////////
  //	Stage 6: Learning 		//
  ////////////////////////////////////////

  void		Compute_dWt(Unit*)	{ };
  virtual void	Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  virtual void	Compute_dWt_impl(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // actually do wt change

  virtual void	Compute_WtFmLin(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // if weights need to be updated from linear values without doing updatewts

  void		UpdateWeights(Unit* u);

  virtual void	EncodeState(LeabraUnit*, LeabraLayer*, LeabraTrial*)	{ };
  // encode current state information (hook for time-based learning)

  virtual void	CreateNXX1Fun();  // create convolved gaussian and x/x+1 

  virtual void	Defaults();	// #BUTTON #CONFIRM restores default parameter settings: warning -- you will lose any unique parameters you might have set!

  virtual void	GraphVmFun(GraphLog* graph_log, float g_i = .5, float min = 0.0, float max = 1.0, float incr = .01);
  // #BUTTON #NULL_OK graph membrane potential (v_m) as a function of excitatory net input (net) for given inhib conductance (g_i) (NULL = new graph log)
  virtual void	GraphActFmVmFun(GraphLog* graph_log, float min = .15, float max = .50, float incr = .001);
  // #BUTTON #NULL_OK graph the activation function as a function of membrane potential (v_m) (NULL = new graph log)
  virtual void	GraphActFmNetFun(GraphLog* graph_log, float g_i = .5, float min = 0.0, float max = 1.0, float incr = .001);
  // #BUTTON #NULL_OK graph the activation function as a function of net input (projected through membrane potential) (NULL = new graph log)

  bool  CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet=false);

  void	UpdateAfterEdit();	// to set _impl sig
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraUnitSpec);
  COPY_FUNS(LeabraUnitSpec, UnitSpec);
  TA_BASEFUNS(LeabraUnitSpec);
};

class VChanBasis : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER basis variables for vchannels
public:
  float		hyst;		// hysteresis
  float		acc;		// fast accomodation
  bool		hyst_on;	// #NO_VIEW binary thresholded mode state variable, hyst
  bool		acc_on;		// #NO_VIEW binary thresholded mode state variable, acc
  float		g_h;		// #NO_VIEW hysteresis conductance
  float		g_a;		// #NO_VIEW accomodation conductance

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const VChanBasis& cp);
  COPY_FUNS(VChanBasis, taBase);
  TA_BASEFUNS(VChanBasis);
};

class LeabraUnitChans : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in Leabra units
public:
  float		l;		// leak
  float		i;		// #DMEM_SHARE_SET_1 inhibitory
  float		h;		// hysteresis (Ca)
  float		a;		// accomodation (K)

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const LeabraUnitChans& cp);
  COPY_FUNS(LeabraUnitChans, taBase);
  TA_BASEFUNS(LeabraUnitChans);
};

class LeabraUnit : public Unit {
  // ##DMEM_SHARE_SETS_5 Leabra unit, point-neuron approximation
public:
  float		act_eq;		// #NO_SAVE rate-code equivalent activity value (time-averaged spikes or just act)
  float		act_avg;	// average activation over long time intervals (dt = act.avg_dt)
  float		act_m;		// minus_phase activation, set after settling, used for learning
  float		act_p;		// plus_phase activation, set after settling, used for learning
  float		act_dif;	// difference between plus and minus phase acts, gives unit err contribution
  float		da;		// #NO_SAVE delta activation: change in act from one cycle to next, used to stop settling
  VChanBasis	vcb;		// voltage-gated channel basis variables
  LeabraUnitChans gc;		// #DMEM_SHARE_SET_1 #NO_SAVE current unit channel conductances
  float		I_net;		// #NO_SAVE net current produced by all channels
  float		v_m;		// #NO_SAVE membrane potential

  bool		in_subgp;	// #READ_ONLY #NO_SAVE determine if unit is in a subgroup
  float		clmp_net;	// #NO_VIEW #NO_SAVE #DETAIL #DMEM_SHARE_SET_4 hard-clamp net input (no need to recompute)
  float		net_scale;	// #NO_VIEW #NO_SAVE #DETAIL total netinput scaling basis
  float		bias_scale;	// #NO_VIEW #NO_SAVE #DETAIL bias weight scaling factor
  float		prv_net;	// #NO_VIEW #NO_SAVE #DETAIL previous net input (for time averaging)
  float		prv_g_i;	// #NO_VIEW #NO_SAVE #DETAIL previous inhibitory conductance value (for time averaging)

  float		act_sent;	// #NO_VIEW #NO_SAVE #DETAIL last activation value sent (only send when diff is over threshold)
  float		act_delta;	// #NO_VIEW #NO_SAVE #DETAIL change in activation to send to other units
  float		net_raw;	// #NO_VIEW #NO_SAVE #DETAIL raw net input received from sending units (increments the deltas in send_delta)
  float		net_delta;	// #NO_VIEW #NO_SAVE #DETAIL #DMEM_SHARE_SET_3 change in netinput received from other units  (send_delta mode only)
  float		g_i_raw;	// #NO_VIEW #NO_SAVE #DETAIL raw inhib net input received from sending units (increments the deltas in send_delta)
  float		g_i_delta;	// #NO_VIEW #NO_SAVE #DETAIL #DMEM_SHARE_SET_3 change in inhibitory netinput received from other units (send_delta mode only)

  float		i_thr;		// #NO_SAVE inhibitory threshold value for computing kWTA
  float		spk_amp;	// amplitude of spiking output (for depressing synapse activation function)
  float		misc_1;		// #NO_VIEW miscellaneous variable for other algorithms that need it (e.g., TdLayerSpec)

  void		InitDelta()
  { ((LeabraUnitSpec*)spec.spec)->InitDelta(this); }
  void		InitState(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->InitState(this, lay); }

  void		SetCurLrate(LeabraTrial* trl, int epoch)
  { ((LeabraUnitSpec*)spec.spec)->SetCurLrate(this, trl, epoch); }

  void		Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetScale(this, lay, trl); }
  void		Send_ClampNet(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->Send_ClampNet(this, lay, trl); }

  void		Send_Net(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Send_Net(this, lay); }
  void		Send_NetDelta(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Send_NetDelta(this, lay); }

  void		Compute_NetAvg(LeabraLayer* lay, LeabraInhib* athr, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetAvg(this, lay, athr, trl); }
  void		Compute_InhibAvg(LeabraLayer* lay, LeabraInhib* athr, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->Compute_InhibAvg(this, lay, athr, trl); }
  void		Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_HardClamp(this, lay, trl); }
  bool		Compute_SoftClamp(LeabraLayer* lay, LeabraTrial* trl) 
  { return ((LeabraUnitSpec*)spec.spec)->Compute_SoftClamp(this, lay, trl); }

  float		Compute_IThresh(LeabraLayer* lay, LeabraTrial* trl)
  { return ((LeabraUnitSpec*)spec.spec)->Compute_IThresh(this, lay, trl); }

  void		Compute_Act()	{ Unit::Compute_Act(); }
  void 		Compute_Act(LeabraLayer* lay, LeabraInhib* athr, LeabraTrial* trl) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_Act(this, lay, athr, trl); }

  void		PhaseInit(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->PhaseInit(this, lay, trl); }
  void		DecayEvent(LeabraLayer* lay, LeabraTrial* trl, float decay)
  { ((LeabraUnitSpec*)spec.spec)->DecayEvent(this, lay, trl, decay); }
  void		DecayPhase(LeabraLayer* lay, LeabraTrial* trl, float decay)
  { ((LeabraUnitSpec*)spec.spec)->DecayPhase(this, lay, trl, decay); }
  void		ExtToComp(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->ExtToComp(this, lay, trl); }
  void		TargExtToComp(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->TargExtToComp(this, lay, trl); }
  void		PostSettle(LeabraLayer* lay, LeabraInhib* athr, LeabraTrial* trl, bool set_both=false)
  { ((LeabraUnitSpec*)spec.spec)->PostSettle(this, lay, athr, trl, set_both); }

  void 		Compute_dWt(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt(this, lay, trl); }	  

  void 		Compute_WtFmLin(LeabraLayer* lay, LeabraTrial* trl) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_WtFmLin(this, lay, trl); }

  void 		EncodeState(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->EncodeState(this, lay, trl); }

  void		GetInSubGp();

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraUnit);
  COPY_FUNS(LeabraUnit, Unit);
  TA_BASEFUNS(LeabraUnit);
};

class LeabraSort : public taPtrList<LeabraUnit> {
  // ##NO_TOKENS ##NO_UPDATE_AFTER used for sorting units in kwta computation
protected:
  int		El_Compare_(void* a, void* b) const
  { int rval=-1; if(((LeabraUnit*)a)->net < ((LeabraUnit*)b)->net) rval=1;
    else if(((LeabraUnit*)a)->net == ((LeabraUnit*)b)->net) rval=0; return rval; }
  // compare two items for purposes of sorting: descending order by net
public:
  int	FindNewNetPos(float nw_net);	  // find position in list for a new net value
  void	FastInsertLink(void* it, int where); // faster version of insert link fun
};

// misc data-holding structures

class KWTASpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specifies k-winner-take-all parameters
public:
  enum K_From {
    USE_K,			// use the k specified directly
    USE_PCT,			// use the percentage pct to compute the k as a function of layer size
    USE_PAT_K			// use the activity level of the current event pattern (k = # of units > pat_q)
  };

  K_From	k_from;		// how is the active_k determined: directly by k, by pct, or by no. of units where ext > pat_q
  int		k;		// #CONDEDIT_ON_k_from:USE_K desired number of active units in the layer
  float		pct;		// #CONDEDIT_ON_k_from:USE_PCT desired proportion of activity (used to compute a k value based on layer size, .25 std)
  float		pat_q;		// #HIDDEN #DEF_0.5 threshold for pat_k based activity level: add to k if ext > pat_q

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(KWTASpec);
  COPY_FUNS(KWTASpec, taBase);
  TA_BASEFUNS(KWTASpec);
};

class AdaptISpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specifies adaptive kwta specs (esp for avg-based)
public:
  enum AdaptType {
    NONE,			// don't adapt anything
    KWTA_PT,			// adapt kwta point (i_kwta_pt) based on running-average layer activation as compared to target value
    G_BAR_I,			// adapt g_bar.i for unit inhibition values based on layer activation at any point in time
    G_BAR_IL			// adapt g_bar.i and g_bar.l for unit inhibition & leak values based on layer activation at any point in time
  };

  AdaptType	type;		// what to adapt, or none for nothing
  float		tol;		// #CONDEDIT_OFF_type:NONE #DEF_0.02 tolerance around target avg act before changing parameter
  float		p_dt;		// #CONDEDIT_OFF_type:NONE #DEF_0.1 #AKA_pt_dt time constant for changing the parameter (i_kwta_pt or g_bar.i)
  float		mx_d;		// #CONDEDIT_OFF_type:NONE #DEF_0.9 maximum deviation (proportion) from initial parameter setting allowed
  float		l;		// #CONDEDIT_ON_type:G_BAR_IL proportion of difference from target activation to allocate to the leak in G_BAR_IL mode
  float		a_dt;		// #CONDEDIT_ON_type:KWTA_PT #DEF_0.005 time constant for integrating average average activation, which is basis for adapting i_kwta_pt

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(AdaptISpec);
  COPY_FUNS(AdaptISpec, taBase);
  TA_BASEFUNS(AdaptISpec);
};

class ClampSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for clamping 
public:
  bool		hard;		// #DEF_true whether to hard clamp inputs to this layer or not
  float		gain;		// #CONDEDIT_OFF_hard:true #DEF_0.5 starting soft clamp gain factor (net = gain * ext)
  float		d_gain;		// #CONDEDIT_OFF_hard:true [Default: 0] for soft clamp, delta to increase gain when target units not > .5 (0 = off, .1 std when used)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ClampSpec);
  COPY_FUNS(ClampSpec, taBase);
  TA_BASEFUNS(ClampSpec);
};

class DecaySpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds decay values
public:
  float		event;		// #DEF_1 proportion decay of state vars between events
  float		phase;		// #DEF_1 proportion decay of state vars between minus and plus phases 
  float		phase2;		// #DEF_0 proportion decay of state vars between 2nd set of phases (if appl, 0 std)
  bool		clamp_phase2;	// #DEF_false if true, hard-clamp second plus phase activations to prev plus phase (only special layers will then update -- optimizes speed)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(DecaySpec);
  COPY_FUNS(DecaySpec, taBase);
  TA_BASEFUNS(DecaySpec);
};

class LayerLinkSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for linking inhibition across layers
public:
  bool		link;		// #DEF_false whether to link the inhibition across layers (or not)
  float		gain;		// #CONDEDIT_ON_link:true [Default: .5] strength of the inhibition link (how much to move inhib towards that of other layer(s))
  bool		gp_uses_lay_avg; // #DEF_false if using UNIT_GROUPS, use layer average activation for layer target activation percent (only for backward compatiblity)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(LayerLinkSpec);
  COPY_FUNS(LayerLinkSpec, taBase);
  TA_BASEFUNS(LayerLinkSpec);
};

class LeabraLayerSpec : public LayerSpec {
  // Leabra layer specs, computes inhibitory input for all units in layer
public:
  enum Compute_I {		// how to compute the inhibition
    KWTA_INHIB,			// between thresholds of k and k+1th most activated units (sets precise k value, should use i_kwta_pt = .25 std)
    KWTA_AVG_INHIB,		// average of top k vs avg of rest (provides more flexibility in actual k value, should use i_kwta_pt = .6 std)
    UNIT_INHIB,			// unit-based inhibition (g_i from netinput -- requires connections with inhib flag set to provide inhibition)
    UNIT_M_KWTA_P,		// unit-based inhibition in minus phase, kWTA in plus phase
    UNIT_M_KWTA_AVG_P,		// unit-based inhibition in minus phase, kWTA avg-based in plus phase
  };

  enum InhibGroup {
    ENTIRE_LAYER,		// treat entire layer as one inhibitory group (even if subgroups exist)
    UNIT_GROUPS,		// treat sub unit groups as separate inhibitory groups
    LAY_AND_GPS,		// compute inhib over both groups and whole layer, inhibi is max of layer and group inhib
    GPS_THEN_UNITS		// first find top gp_kwta.k most active (on average) groups, then find top kwta.k units within those groups (else all units inhibbed)
  };

  KWTASpec	kwta;		// #CONDEDIT_OFF_inhib_group:UNIT_GROUPS desired activity level over entire layer
  KWTASpec	gp_kwta;	// #CONDEDIT_OFF_inhib_group:ENTIRE_LAYER desired activity level for units within unit groups (not for ENTIRE_LAYER)
  InhibGroup	inhib_group;	// what to consider the inhibitory group (layer or unit subgroups, or both)
  Compute_I	compute_i;	// how to compute inhibition (g_i): two forms of kwta or unit-level inhibition
  float		i_kwta_pt;	// [Default: .25 for KWTA_INHIB, .6 for KWTA_AVG] point to place inhibition between k and k+1
  AdaptISpec	adapt_i;	// #AKA_adapt_pt adapt the inhibition: either i_kwta_pt point based on diffs between actual and target k level (for avg-based), or g_bar.i for unit-inhib
  ClampSpec	clamp;		// how to clamp external inputs to units (hard vs. soft)
  DecaySpec	decay;		// decay of activity state vars between events, -/+ phase, and 2nd set of phases (if appl)
  LayerLinkSpec	layer_link;	// link inhibition between layers (with specified gain), rarely used, linked layers in layer objects

  virtual void	InitWtState(LeabraLayer* lay);
  // initialize weight values and other permanent state

  virtual void	SetCurLrate(LeabraLayer* lay, LeabraTrial* trl, int epoch);
  // set current learning rate based on epoch

  //////////////////////////////////////////
  //	Stage 0: at start of settling	  // 
  //////////////////////////////////////////

  virtual void	Compute_Active_K(LeabraLayer* lay);
  // prior to settling: compute actual activity levels based on spec, inputs, etc
  virtual void	Compute_Active_K_impl(LeabraLayer* lay, Unit_Group* ug,
				      LeabraInhib* thr, KWTASpec& kwtspec);
  virtual int	Compute_Pat_K(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // above are implementation helpers

  virtual void	InitState(LeabraLayer* lay);
  // prior to settling: initialize dynamic state variables
  virtual void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  // prior to settling: hard-clamp inputs
  virtual void	Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl);
  // prior to settling: compute netinput scaling values
  virtual void	Send_ClampNet(LeabraLayer* lay, LeabraTrial* trl);
  // prior to settling: compute input from hard-clamped

  //////////////////////////////////
  //	Stage 1: netinput 	  //
  //////////////////////////////////

  virtual void 	Send_Net(LeabraLayer* lay);
  // compute net inputs
  virtual void 	Send_NetDelta(LeabraLayer* lay);
  // compute net inputs as changes in activation

  ////////////////////////////////////////////////////////////////
  //	Stage 2: netinput averages and clamping (if necc)	//
  ////////////////////////////////////////////////////////////////

  virtual void	Compute_Clamp_NetAvg(LeabraLayer* lay, LeabraTrial* trl);
  // clamp and compute averages of net inputs that were already computed
  virtual void	Compute_NetAvg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  virtual void	Compute_SoftClamp(LeabraLayer* lay, LeabraTrial* trl);
  // soft-clamp inputs by adding to net input

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual void	InitInhib(LeabraLayer* lay);
  // initialize the inhibitory state values
  virtual void	Compute_Inhib(LeabraLayer* lay, LeabraTrial* trl);
  // stage two: compute the inhibition for layer
  virtual void	Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  // implementation of inhibition computation for either layer or unit group
  virtual void	Compute_Inhib_kWTA(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  // implementation of basic kwta inhibition computation
  virtual void	Compute_Inhib_kWTA_Avg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  // implementation of kwta avg-based inhibition computation
  virtual void	Compute_Inhib_kWTA_Gps(LeabraLayer* lay, LeabraTrial* trl);
  // implementation of GPS_THEN_UNITS kwta on groups

  ////// Stage 3.5: second pass of inhibition to do layer_link and averaging

  virtual void	Compute_LinkInhib(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  // compute inhibition after linkages with other layers are factored in
  virtual void 	Compute_InhibAvg(LeabraLayer* lay, LeabraTrial* trl);
  // stage three: compute final activation
  virtual void 	Compute_InhibAvg_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  ////////////////////////////////////////
  //	Stage 4: the final activation 	//
  ////////////////////////////////////////

  virtual void 	Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  virtual void	Compute_ActAvg(LeabraLayer* lay, LeabraTrial* trl);
  // helper function to compute acts.avg from act_eq
  virtual void 	Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  virtual void	Compute_ActMAvg(LeabraLayer* lay, LeabraTrial* trl);
  // helper function to compute acts_m.avg from act_m
  virtual void 	Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  virtual void	Compute_ActPAvg(LeabraLayer* lay, LeabraTrial* trl);
  // helper function to compute acts_p.avg from act_p

  virtual void 	Compute_Act(LeabraLayer* lay, LeabraTrial* trl);
  // stage three: compute final activation
  virtual void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  ////////////////////////////////////////
  //	Stage 5: Between Events 	//
  ////////////////////////////////////////

  virtual void	PhaseInit(LeabraLayer* lay, LeabraTrial* trl);
  // initialize start of a setting phase, set input flags appropriately, etc

  virtual void	DecayEvent(LeabraLayer* lay, LeabraTrial* trl);
  virtual void	DecayPhase(LeabraLayer* lay, LeabraTrial* trl);
  virtual void	DecayPhase2(LeabraLayer* lay, LeabraTrial* trl);

  virtual void	ExtToComp(LeabraLayer* lay, LeabraTrial* trl);
  // change external inputs to comparisons (remove input)
  virtual void	TargExtToComp(LeabraLayer* lay, LeabraTrial* trl);
  // change target & external inputs to comparisons (remove targ & input)
  virtual void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  // after settling, keep track of phase variables, etc.
//   virtual void	NormMPActs(LeabraLayer* lay, LeabraTrial* trl);
//   // normalize minus and plus phase activations to the same average level
  virtual void	AdaptGBarI(LeabraLayer* lay, LeabraTrial* trl);
  // adapt inhibitory conductances based on target activation values relative to current values

  ////////////////////////////////////////
  //	Stage 6: Learning 		//
  ////////////////////////////////////////

  virtual void	AdaptKWTAPt(LeabraLayer* lay, LeabraTrial* trl);
  // adapt the kwta point based on average activity
  virtual void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);
  virtual void	Compute_WtFmLin(LeabraLayer* lay, LeabraTrial* trl);
  // use this if weights will be used again for activations prior to being updated

  virtual void	HelpConfig();	// #BUTTON get help message for configuring this spec
  virtual bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  // check for for misc configuration settings required by different algorithms, including settings on the processes

  virtual void	Defaults();	// #BUTTON #CONFIRM restores default parameter settings: warning -- you will lose any unique parameters you might have set!

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(LeabraLayerSpec);
  COPY_FUNS(LeabraLayerSpec, LayerSpec);
  TA_BASEFUNS(LeabraLayerSpec);
};

SpecPtr_of(LeabraLayerSpec);

class AvgMaxVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds average and max statistics
public:
  float		avg;		// average value
  float		max;		// maximum value
  int 		max_i;		// index of unit with maximum value
  
  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const AvgMaxVals& cp);
  COPY_FUNS(AvgMaxVals, taBase);
  TA_BASEFUNS(AvgMaxVals);
};

class KWTAVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for kwta stuff
public:
  int		k;       	// target number of active units for this collection
  float		pct;		// actual percent activity in group
  float		pct_c;		// #HIDDEN complement of (1.0 - ) actual percent activity in group
  int		adth_k;		// #HIDDEN adapting threshold k value -- how many units can adapt per time
  float		k_ithr;		// inhib threshold for k unit (top k for kwta_avg)
  float		k1_ithr;	// inhib threshold for k+1 unit (other units for kwta_avg)
  float		ithr_r;		// log of ratio of ithr values (indicates signal differentiation)

  void		Compute_Pct(int n_units);
  void		Compute_IThrR(); // compute ithr_r ratio value

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const KWTAVals& cp);
  COPY_FUNS(KWTAVals, taBase);
  TA_BASEFUNS(KWTAVals);
};

class AdaptIVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for adapting kwta stuff
public:
  float		avg_avg;	// average of the average activation in a layer
  float		i_kwta_pt;	// adapting point to place inhibition between k and k+1 for kwta
  float		g_bar_i;	// adapting g_bar.i value 
  float		g_bar_l;	// adapting g_bar.l value 

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const AdaptIVals& cp);
  COPY_FUNS(AdaptIVals, taBase);
  TA_BASEFUNS(AdaptIVals);
};

class InhibVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for inhibition
public:
  float		kwta;		// inhibition due to kwta function
  float		g_i;		// overall value of the inhibition
  float		g_i_orig; 	// #HIDDEN original value of the inhibition (before linking)

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const InhibVals& cp);
  COPY_FUNS(InhibVals, taBase);
  TA_BASEFUNS(InhibVals);
};

class LayerLink : public taOBase {
  // ##NO_TOKENS #NO_UPDATE_AFTER Link strength between layers, affects threshold
public:
  LeabraLayer*	layer;		// layer to link to
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

class LeabraInhib {
  // holds threshold-computation values, used as a parent class for layers, etc
public:
  LeabraSort 	active_buf;	// #HIDDEN #NO_SAVE list of active units
  LeabraSort 	inact_buf;	// #HIDDEN #NO_SAVE list of inactive units

  AvgMaxVals	netin;		// #READ_ONLY net input values for the layer
  AvgMaxVals	acts;		// #READ_ONLY activation values for the layer
  AvgMaxVals	acts_p;		// #READ_ONLY plus-phase activation stats for the layer
  AvgMaxVals	acts_m;		// #READ_ONLY minus-phase activation stats for the layer
  float		phase_dif_ratio; // #READ_ONLY #SHOW phase-difference ratio (acts_m.avg / acts_p.avg)
 
  KWTAVals	kwta;		// #READ_ONLY values for kwta -- activity levels, etc NOTE THIS IS A COMPUTED VALUE: k IS SET IN LayerSpec!
  InhibVals	i_val;		// #READ_ONLY #SHOW inhibitory values computed by kwta
  AvgMaxVals	un_g_i;		// #READ_ONLY average and stdev (not max) values for unit inhib-to-thresh
  AdaptIVals	adapt_i;	// #READ_ONLY #AKA_adapt_pt adapting inhibition values

  void	Inhib_SetVals(float val)	{ i_val.g_i = val; i_val.g_i_orig = val; }
  void	Inhib_ResetSortBuf() 		{ active_buf.size = 0; inact_buf.size = 0; }
  void	Inhib_InitState(LeabraLayerSpec* lay);
  void	Inhib_Initialize();
  void	Inhib_Copy_(const LeabraInhib& cp);
};

class LeabraLayer : public Layer, public LeabraInhib {
  // Leabra Layer: implicit inhibition for soft kWTA behavior
public:
  LeabraLayerSpec_SPtr	spec;	// the spec for this layer: controls all functions of layer
  LayerLink_List layer_links;	// list of layers to link inhibition with
  float		stm_gain;	// actual stim gain for soft clamping, can be incremented to ensure clamped units active
  bool		hard_clamped;	// this layer is actually hard clamped
  int		prv_phase;	// #READ_ONLY previous phase value (needed for 2nd plus phases and the like)
  float		td;		// #READ_ONLY temporal differences delta value (where applicable)
  int		td_updt;	// #READ_ONLY true if td triggered an update (either + to store or - reset)

  void	Build();

  void	InitWtState() 	{ spec->InitWtState(this); }
  void	InitInhib() 	{ spec->InitInhib(this); } // initialize inhibitory state
  void	ModifyState()	{ spec->DecayEvent(this, NULL); } // this is what modify means..

  void	SetCurLrate(LeabraTrial* trl, int epoch) { spec->SetCurLrate(this, trl, epoch); }
  
  void	Compute_Active_K()			{ spec->Compute_Active_K(this); }
  void	InitState() 				{ spec->InitState(this); }

  void	Compute_HardClamp(LeabraTrial* trl) 	{ spec->Compute_HardClamp(this, trl); }
  void	Compute_NetScale(LeabraTrial* trl) 	{ spec->Compute_NetScale(this, trl); }
  void	Send_ClampNet(LeabraTrial* trl) 	{ spec->Send_ClampNet(this, trl); }

  void	Send_Net()				{ spec->Send_Net(this); }
  void	Send_NetDelta()				{ spec->Send_NetDelta(this); }

  void	Compute_Clamp_NetAvg(LeabraTrial* trl)  { spec->Compute_Clamp_NetAvg(this, trl); }

  void	Compute_Inhib(LeabraTrial* trl) 	{ spec->Compute_Inhib(this, trl); }
  void	Compute_InhibAvg(LeabraTrial* trl)	{ spec->Compute_InhibAvg(this, trl); }

  void	Compute_Act()				{ spec->Compute_Act(this, NULL); }
  void	Compute_Act(LeabraTrial* trl) 		{ spec->Compute_Act(this, trl); }

  void	PhaseInit(LeabraTrial* trl)		{ spec->PhaseInit(this, trl); }
  void	DecayEvent(LeabraTrial* trl)		{ spec->DecayEvent(this, trl); } // decay between events
  void	DecayPhase(LeabraTrial* trl)    	{ spec->DecayPhase(this, trl); } // decay between phases
  void	DecayPhase2(LeabraTrial* trl)  		{ spec->DecayPhase2(this, trl); } // decay between 2nd set of phases

  void	ExtToComp(LeabraTrial* trl)		{ spec->ExtToComp(this, trl); }
  void	TargExtToComp(LeabraTrial* trl)		{ spec->TargExtToComp(this, trl); }
  void	PostSettle(LeabraTrial* trl, bool set_both=false) { spec->PostSettle(this, trl, set_both); }

  void	Compute_dWt(LeabraTrial* trl) 		{ spec->Compute_dWt(this, trl); }
  void	Compute_dWt() 				{ spec->Compute_dWt(this, NULL); }
  void	Compute_WtFmLin(LeabraTrial* trl) 	{ spec->Compute_WtFmLin(this, trl); }
  void	UpdateWeights();

  virtual void	ResetSortBuf();

  bool		SetLayerSpec(LayerSpec* sp);
  LayerSpec*	GetLayerSpec()		{ return (LayerSpec*)spec.spec; }
  bool		CheckTypes();

  bool  CheckConfig(TrialProcess* tp, bool quiet=false)
  { return spec->CheckConfig(this, (LeabraTrial*)tp, quiet); }

  void	UpdateAfterEdit();	// reset sort_buf after any edits..

  void	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const LeabraLayer& cp);
  COPY_FUNS(LeabraLayer, Layer);
  TA_BASEFUNS(LeabraLayer);
};

class LeabraUnit_Group : public Unit_Group, public LeabraInhib {
  // for independent subgroups of competing units within a single layer
public:
  int		misc_state;	// miscellaneous state variable

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  void	Copy_(const LeabraUnit_Group& cp);
  COPY_FUNS(LeabraUnit_Group, Unit_Group);
  TA_BASEFUNS(LeabraUnit_Group);
};

//////////////////////////
//	Inlines		// 
//////////////////////////


//////////////////////////
//     WtSigFun 	//
//////////////////////////

void LeabraConSpec::Compute_LinFmWt(LeabraCon_Group* cg) {
  CON_GROUP_LOOP(cg, C_Compute_LinFmWt(cg, (LeabraCon*)cg->Cn(i)));
}

void LeabraConSpec::Compute_WtFmLin(LeabraCon_Group* cg) {
  CON_GROUP_LOOP(cg, C_Compute_WtFmLin(cg, (LeabraCon*)cg->Cn(i)));
}

//////////////////////////
//      Netin   	//
//////////////////////////

float LeabraConSpec::C_Compute_Net(LeabraCon* cn, Unit*, Unit* su) {
  return cn->wt * su->act;
}
float LeabraConSpec::Compute_Net(Con_Group* cg, Unit* ru) {
  float rval=0.0f;
  CON_GROUP_LOOP(cg, rval += C_Compute_Net((LeabraCon*)cg->Cn(i), ru, cg->Un(i)));
  return ((LeabraCon_Group*)cg)->scale_eff * rval;
}

void LeabraConSpec::C_Send_Inhib(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru,
				 LeabraUnit* su) {
  ru->gc.i += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_Inhib(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
}

void LeabraConSpec::C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, Unit* su) {
  ru->net += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_Net(Con_Group* cg, Unit* su) {
  if(inhib)
    Send_Inhib((LeabraCon_Group*)cg, (LeabraUnit*)su);
  else {
    CON_GROUP_LOOP(cg, C_Send_Net((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), cg->Un(i), su));
  }
}

///////////////////

void LeabraConSpec::C_Send_InhibDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su) {
  ru->g_i_delta += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta * cn->wt;
}
void LeabraConSpec::Send_InhibDelta(LeabraCon_Group* cg, LeabraUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_InhibDelta(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
}

void LeabraConSpec::C_Send_NetDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su) {
  ru->net_delta += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta * cn->wt;
}

void LeabraConSpec::Send_NetDelta(LeabraCon_Group* cg, LeabraUnit* su) {
  if(inhib)
    Send_InhibDelta(cg, su);
  else {
    CON_GROUP_LOOP(cg, C_Send_NetDelta(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
  }
}

///////////////////

void LeabraConSpec::C_Send_ClampNet(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru,
				 LeabraUnit* su) {
  ru->clmp_net += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_ClampNet(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
}

void LeabraCon_Group::Send_ClampNet(LeabraUnit* su) {
  ((LeabraConSpec*)spec.spec)->Send_ClampNet(this, su);
}


//////////////////////////
//     Computing dWt 	//
//////////////////////////

inline void LeabraConSpec::Compute_SAvgCor(LeabraCon_Group* cg, LeabraUnit*) {
  LeabraLayer* fm = (LeabraLayer*)cg->prjn->from;
  if(savg_cor.src == SAvgCorSpec::SLAYER_TRG_PCT)
    cg->savg = fm->kwta.pct;
  else if(savg_cor.src == SAvgCorSpec::SLAYER_AVG_ACT)
    cg->savg = fm->acts_p.avg;
  else if(savg_cor.src == SAvgCorSpec::COMPUTED_SAVG) {
    cg->savg = 0.0f;
    for(int i=0; i<cg->size; i++)
      cg->savg += ((LeabraUnit*)cg->Un(i))->act_p;
    if(cg->size > 0)
      cg->savg /= (float)cg->size;
  }
  else
    cg->savg = fix_savg.savg;
  // bias savg based on k_savg_cor
  float savg = .5f + savg_cor.cor * (cg->savg - .5f);
  savg = MAX(savg_cor.thresh, savg); // keep this computed value within bounds
  cg->max_cor = .5f / savg;
}

inline float LeabraConSpec::C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg,
					   float ru_act, float su_act)
{
  // wt is negative in linear form, so using opposite sign of usual here
  return ru_act * (su_act * (cg->max_cor + cn->wt) +
		   (1.0f - su_act) * cn->wt);
}

// generec error term with sigmoid activation function, and soft bounding
inline float LeabraConSpec::C_Compute_Err
(LeabraCon* cn, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
  float err = (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
  // wt is negative in linear form, so using opposite sign of usual here
  if(err > 0.0f)	err *= (1.0f + cn->wt);
  else			err *= -cn->wt;	
  return err;
}

// combine hebbian and error-driven
inline void LeabraConSpec::C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus, float heb, float err) {
  float dwt = lmix.err * err + lmix.hebb * heb;
  if(rus->act_reg.on) {
    if(ru->act_avg <= rus->act_reg.min)
      dwt -= rus->act_reg.wt_dt * cn->wt; // weight is -..
    else if(ru->act_avg >= rus->act_reg.max)
      dwt += rus->act_reg.wt_dt * cn->wt;
  }
  cn->dwt += dwt;
}

inline void LeabraConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraUnit* lru = (LeabraUnit*)ru;
  LeabraUnitSpec* lrus = (LeabraUnitSpec*)ru->spec.spec;
  LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
  Compute_SAvgCor(lcg, lru);
  if(((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) {
    for(int i=0; i<cg->size; i++) {
      LeabraUnit* su = (LeabraUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	float orig_wt = cn->wt;
	C_Compute_LinFmWt(lcg, cn); // get weight into linear form
	C_Compute_dWt(cn, lru, lrus,
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
      }
    }
  }
}

inline void LeabraConSpec::C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg,
					   LeabraUnit*, LeabraUnit*)
{
  if(cn->dwt != 0.0f) {
    C_Compute_LinFmWt(cg, cn);	// go to linear weights
    cn->wt -= cur_lrate * cn->dwt; // wt is now negative in linear form -- signs are reversed!
    cn->wt = MAX(cn->wt, -1.0f);	// limit 0-1
    cn->wt = MIN(cn->wt, 0.0f);
    C_Compute_WtFmLin(cg, cn);	// go back to nonlinear weights
    // then put in real limits!!
    cn->wt = MIN(cn->wt, wt_limits.max);	// limit 0-1
    cn->wt = MAX(cn->wt, wt_limits.min);
  }
  cn->pdw = cn->dwt;
  cn->dwt = 0.0f;
}

inline void LeabraConSpec::UpdateWeights(Con_Group* cg, Unit* ru) {
  LeabraCon_Group* lcg = (LeabraCon_Group*)cg;
  CON_GROUP_LOOP(cg, C_UpdateWeights((LeabraCon*)cg->Cn(i), lcg,
				     (LeabraUnit*)ru, (LeabraUnit*)cg->Un(i)));
//  ApplyLimits(cg, ru); limits are automatically enforced anyway
}

inline void LeabraConSpec::B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
  cn->dwt += (ru->act_p - ru->act_m);
}
  
// default is not to do anything tricky with the bias weights
inline void LeabraConSpec::B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
  cn->pdw = cn->dwt;
  cn->wt += cur_lrate * cn->dwt;
  cn->dwt = 0.0f;
  C_ApplyLimits(cn, ru, NULL);
}

inline void LeabraBiasSpec::B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
  if(fabsf(cn->dwt) < dwt_thresh)
    cn->dwt = 0.0f;
  cn->pdw = cn->dwt;
  cn->wt += cur_lrate * cn->dwt;
  cn->dwt = 0.0f;
  C_ApplyLimits(cn, ru, NULL);
}

//////////////////////////
// 	Processes	//
//////////////////////////

class LeabraCycle : public CycleProcess {
  // one Leabra cycle of activation updating
public:
  LeabraSettle*	leabra_settle;
  // #NO_SUBTYPE #READ_ONLY #NO_SAVE pointer to parent settle proc

  void		Loop();		// compute activations
  bool 		Crit()		{ return true; } // executes loop only once
  
  virtual void	Compute_Net();
  virtual void	Compute_Clamp_NetAvg();
  virtual void	Compute_Inhib();
  virtual void	Compute_InhibAvg();
  virtual void	Compute_Act();

  void 	UpdateAfterEdit();		// modify to update the leabra_trial
  void 	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(LeabraCycle);
  COPY_FUNS(LeabraCycle, CycleProcess);
  TA_BASEFUNS(LeabraCycle);
};

class LeabraSettle : public SettleProcess {
  // Leabra settling phase of activation updating
public:
  LeabraTrial* 	leabra_trial;
  // #NO_SUBTYPE #READ_ONLY #NO_SAVE pointer to parent phase trial
  int		min_cycles;	// #DEF_15 minimum number of cycles to settle for
  int		min_cycles_phase2; // #DEF_15 minimum number of cycles to settle for in second phase
  int		netin_mod;	// #DEF_1 net input computation modulus: how often to compute netinput vs. activation update (2 = faster)
  bool		send_delta;	// #DEF_false send netin deltas instead of raw netin: more efficient (automatically sets corresponding unitspec flag)

  void		Init_impl();	// initialize start of settling
  void		Final();	// update acts at end of settling

  virtual void	Compute_Active_K();
  virtual void	DecayEvent();
  virtual void	DecayPhase();
  virtual void	DecayPhase2();
  virtual void	PhaseInit();
  virtual void	ExtToComp();
  virtual void	TargExtToComp();
  virtual void	Compute_HardClamp();
  virtual void	Compute_NetScale();
  virtual void	Send_ClampNet();
  virtual void	PostSettle();
  virtual void	PostSettle_NStdLay();
  virtual void	Compute_dWt_NStdLay(); // on non-nstandard layers
  virtual void	Compute_dWt();		// on all layers
  virtual void	Compute_WtFmLin();
  virtual void	UpdateWeights();

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
  // Leabra trial process, iterates over phases
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
    MINUS_PLUS,			// standard minus-plus (err and assoc)
    PLUS_ONLY,			// only present the plus phase (hebbian-only)
    MINUS_PLUS_NOTHING,		// auto-encoder version with final 'nothing' minus phase
    PLUS_NOTHING,		// just the auto-encoder (no initial minus phase)
    MINUS_PLUS_PLUS		// two plus phases for gated context layer updating
  };

  enum FirstPlusdWt {
    NO_FIRST_DWT,		// for three phase cases: don't change weights after first plus
    ONLY_FIRST_DWT,		// for three phase cases: only change weights after first plus
    ALL_DWT			// for three phase cases: change weights after *both* post-minus phases
  };

  PhaseOrder	phase_order;	// [Default: MINUS_PLUS] number and order of phases to present
  Counter	phase_no;	// Current phase number
  Phase		phase;		// Type of current phase: minus or plus
  StateInit	trial_init;	// #DEF_DECAY_STATE how to initialize network state at start of trial
  bool		no_plus_stats;	// #DEF_true don't do stats/logging in the plus phase
  bool		no_plus_test; 	// #DEF_true don't run the plus phase when testing
  FirstPlusdWt	first_plus_dwt;	// #CONDEDIT_ON_phase_order:MINUS_PLUS_PLUS how to change weights on first plus phase if 2 plus phases (applies only to standard leabralayer specs -- others must decide on their own!)
  int		cycle;		// #READ_ONLY #NO_SAVE current cycle value as copied from settle process ONLY VALID DURING PROCESSING

  void		C_Code();	// modify to use the no_plus_stats flag
  
  void		Init_impl();
  void		Loop();
  void		UpdateState();
  void		Final();
  bool		Crit()		{ return SchedProcess::Crit(); }

  virtual void	InitState();
  virtual void	SetCurLrate();
  virtual void	DecayState();
  virtual void	EncodeState();
  virtual void	Compute_dWt_NStdLay(); // on non-nstandard layers
  virtual void	Compute_dWt();
  virtual void	UpdateWeights();
  virtual void	Compute_WtFmLin();

  void		GetCntrDataItems();
  void		GenCntrLog(LogData* ld, bool gen);

  bool		CheckNetwork();
  bool		CheckUnit(Unit* ck);

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  void	Copy_(const LeabraTrial& cp);
  COPY_FUNS(LeabraTrial, TrialProcess);
  TA_BASEFUNS(LeabraTrial);
};

//////////////////////////
//	Unit NetAvg   	//
//////////////////////////

void LeabraUnitSpec::Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib*, LeabraTrial* trl) {
  if(act.send_delta) {
    u->net_raw += u->net_delta;
    u->net += u->clmp_net + u->net_raw;
  }
  u->net = u->prv_net + dt.net * (u->net - u->prv_net);
  u->prv_net = u->net;
  if((noise_type == NETIN_NOISE) && (noise.type != Random::NONE) && (trl->cycle >= 0)) {
    u->net += noise_sched.GetVal(trl->cycle) * noise.Gen();
  }
  u->i_thr = Compute_IThresh(u, lay, trl);
}

void LeabraUnitSpec::Compute_InhibAvg(LeabraUnit* u, LeabraLayer*, LeabraInhib* thr, LeabraTrial*) {
  if(act.send_delta) {
    u->g_i_raw += u->g_i_delta;
    u->gc.i = u->g_i_raw;
  }
  if(thr->i_val.g_i > 0.0f)
    u->gc.i = thr->i_val.g_i; // add in inhibition from global inhib fctn
  else {
    u->gc.i = u->prv_g_i + dt.net * (u->gc.i - u->prv_g_i);
  }
  u->prv_g_i = u->gc.i;
  // don't add -- either or!
  //  u->gc.i += g_bar.i * thr->i_val.g_i; // add in inhibition from global inhib fctn
}


//////////////////////////
// 	Stats 		//
//////////////////////////

class LeabraMaxDa : public Stat {
  // ##COMPUTE_IN_SettleProcess ##LOOP_STAT stat that computes when equilibrium is
public:
  enum dAType {
    DA_ONLY,			// just use da
    INET_ONLY,			// just use inet
    INET_DA			// use inet if no activity, then use da
  };

  LeabraSettle* settle_proc;	// #READ_ONLY #NO_SAVE the settle process
  dAType	da_type;	// #DEF_INET_DA type of activation change measure to use
  float		inet_scale;	// #DEF_1 how to scale the inet measure to be like da
  float		lay_avg_thr;	// #DEF_0.01 threshold for layer average activation to switch to da fm Inet
  StatVal	da;		// absolute value of activation change

  void		RecvCon_Run(Unit*)	{}; // don't do these!
  void		SendCon_Run(Unit*)	{};

  void		InitStat();
  void		Init();
  bool		Crit();
  void		Network_Init();

  void 		Unit_Stat(Unit* unit);
  void		Network_Stat();

  void	UpdateAfterEdit();
  void 	Initialize();		// set minimums
  void	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(LeabraMaxDa);
  COPY_FUNS(LeabraMaxDa, Stat);
  TA_BASEFUNS(LeabraMaxDa);
};

class LeabraSE_Stat : public SE_Stat {
  // squared error for leabra, controls when to compute SE 
public:
  LeabraTrial* 	trial_proc;	// #READ_ONLY #NO_SAVE the trial process to get phase info
  Unit::ExtType	targ_or_comp;	// when to compute SE: targ = 1st minus, comp = 2nd minus, both = both
  bool		no_off_err;	// do not count a unit wrong if it is off but target says on -- only count wrong units that are on but should be off

  void		Network_Run();	// only run in certain phases based on targ_or_comp
  void		Init();		// only init at the right time too
  void 		Unit_Stat(Unit* unit);
  void		NameStatVals();
  
  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	CutLinks();
  SIMPLE_COPY(LeabraSE_Stat);
  COPY_FUNS(LeabraSE_Stat, SE_Stat);
  TA_BASEFUNS(LeabraSE_Stat);
};

class LeabraAeSE_Stat : public LeabraSE_Stat {
  // squared error for leabra auto-encoder: legacy object does same thing as LeabraSE_Stat
public:
  void	Initialize() { };
  void	Destroy()			{ };
  TA_BASEFUNS(LeabraAeSE_Stat);
};

class LeabraGoodStat : public Stat {
  // ##COMPUTE_IN_TrialProcess constraint satisfaction goodness statistic
public:
  bool		subtr_inhib;	// subtract inhibition from harmony?
  StatVal	hrmny;
  StatVal	strss;
  StatVal	gdnss;

  void		InitStat();
  void		Init();
  bool		Crit();

  void		RecvCon_Run(Unit*)	{ }; // don't do these!
  void		SendCon_Run(Unit*)	{ };

  void		Network_Init();
  void		Unit_Stat(Unit* un);
  void		Network_Stat();

  void	Initialize();
  void	Destroy();
  void	Copy_(const LeabraGoodStat& cp);
  COPY_FUNS(LeabraGoodStat, Stat);
  TA_BASEFUNS(LeabraGoodStat);
};

class WrongOnStat : public Stat {
  // ##COMPUTE_IN_TrialProcess Reports an error if a unit is on when it shouldn't have been (for multiple output cases)
public:
  Layer*	trg_lay;
  // target layer, containing activation pattern for all possible correct responses
  StatVal	wrng;		// wrong on error statistic
  float		threshold;	// activation value to consider unit being on

  void		RecvCon_Run(Unit*)	{ }; // don't do these!
  void		SendCon_Run(Unit*)	{ };

  void		InitStat();
  void		Init();
  bool		Crit();
  void		Network_Init();
  void		Layer_Run();		// only compute on TARG layers
  void 		Unit_Run(Layer* lay);

  void		NameStatVals();

  bool		CheckLayerInNet(); // also check trg_lay

  void	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(WrongOnStat);
  COPY_FUNS(WrongOnStat, Stat);
  TA_BASEFUNS(WrongOnStat);
};

//////////////////////////////////////////
// 	Reinforcement Learning		//
//////////////////////////////////////////

// RL here uses minus/plus phases to span the gap between time steps
// with learning at time t focused on predicting the reward at time t+1
// so the minus phase reflects v(t) and the plus phase v(t+1).
// A simple somewhat degenerate case of this uses a discount factor
// of 1 for all time steps up to the point of reward, when it goes to 0.
// Reward is assumed to be sparse and essentially binary, meaning that 
// on most time steps, r(t) = 0 (during which expectations are computed
// for v(t+1)), and then at some point r(t) > 0 (typically 1), and 
// then v(t+1) == r(t).  At the next time step, v(t) is reset to 0.
// Because of the 1 discount factor, the precise time at which later
// reward will occur is not coded, but the *reliablity* of the reward
// prediction should be encoded, which, in a noisy environment, will
// definitely be a function of time.  The reliability is encoded
// in that if a given state does not ultimately result in reward, 
// this will eventually trickle back and be reflected in the
// v(t+1) for that state.

// act_m = v_est(t): estimate of value function at time t,
// 		is clamped to be prior v_est(t+1)
// act_p = r(t+1) or v_est(t+1): either what actually happend (on targ)
// 		or estimate computed by std activation update

// act_dif here gives the td error, which can be used to drive
// the context gating mechanism.

// Note that the standard error-driven learning mechanisms (together
// with the hebbian component) will operate as expected on the 
// act_m, act_p values, even as they backpropagate, in order to 
// learn to produce the right outputs.

// To drive learning in the representational units themselves and
// not to specifically tune the AC pathway, the AC layerspec
// has a pointer to a unitspec so that it can modulate the gain
// parameter as a function of act_diff in AC.

class ACRewSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for reward computing stuff
public:
  float		discount;	// discount factor for future rewards
  float		inv_disc;	// #READ_ONLY inverse of discount factor
  bool		reset;		// whether to reset to 0 after external reward received
  float		val;		// #CONDEDIT_ON_reset:true value to reset to after reward
  bool		immediate;	// rewards are provided immediately for each pattern

  void	UpdateAfterEdit();
  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ACRewSpec);
  COPY_FUNS(ACRewSpec, taBase);
  TA_BASEFUNS(ACRewSpec);
};

class LeabraACLayerSpec : public LeabraLayerSpec {
  // Layer with 1 unit as the AC (Adaptive Critic) for Reinforcement Learning
public:
  enum TDModType {
    MOD_NOTHING,		// don't modulate anything
    MOD_ERR,			// modulate unit err
    MOD_GAIN			// modulate unit gain
  };
  
  ACRewSpec	rew;		// reward specs: discount factor (1), reset reward after ext rec'd (true), val to reset to (.1), immediate = rewards on each trial
  TDModType	mod_type;	// how to modulate units
  UnitSpec_SPtr	un_gain_spec;	// #CONDEDIT_OFF_mod_type:MOD_NOTHING UnitSpec to modulate err, gain on
  float		err_delta;	// #CONDEDIT_ON_mod_type:MOD_ERR maximum err delta for MOD_ERR
  MinMaxRange	un_gain_range;	// #CONDEDIT_ON_mod_type:MOD_GAIN range of gain values to modulate for unit

  void	InitState(LeabraLayer* lay);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  // clamp during minus phase, and during plus phase with reward

  void	ExtToComp(LeabraLayer* lay, LeabraTrial* trl);

  virtual void 	UpdateUnitGain(LeabraUnitSpec* us, float new_gain);
  // update the unit spec after new value copied
  virtual void 	UpdateUnitErr(LeabraUnitSpec* us, float new_err);
  // update the unit spec after new value copied

  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);

  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void  InitLinks();
  void	CutLinks();
  SIMPLE_COPY(LeabraACLayerSpec);
  COPY_FUNS(LeabraACLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(LeabraACLayerSpec);
};

class LeabraLinUnitSpec : public LeabraUnitSpec {
  // a pure linear unit (suitable for an AC unit spec unit)
public:
  void 	Compute_ActFmVm(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraLinUnitSpec);
};

//////////////////////////////////////////
// 	Context Layer for Sequential	//
//////////////////////////////////////////

class CtxtUpdateSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER context updating specifications
public:
  float		fm_hid;		// from hidden (inputs to context layer)
  float		fm_prv;		// from previous context layer values (maintenance)
  float		to_out;		// outputs from context layer

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(CtxtUpdateSpec);
  COPY_FUNS(CtxtUpdateSpec, taBase);
  TA_BASEFUNS(CtxtUpdateSpec);
};

class LeabraContextLayerSpec : public LeabraLayerSpec {
  // context layer that copies from its recv projection (like an input layer)
public:
  CtxtUpdateSpec updt;		// ctxt updating constants: from hidden, from previous values (hysteresis), outputs from context (n/a on simple gate layer)

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  // clamp from act_p values of sending layer

  virtual void Compute_Context(LeabraLayer* lay, LeabraUnit* u, LeabraTrial* trl);
  // get context source value for given context unit

  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);

  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraContextLayerSpec);
  COPY_FUNS(LeabraContextLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(LeabraContextLayerSpec);
};

class GateSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER gating parameters
public:
  enum MaintMode {
    EXTRACELL,			// maintenance performed by extracellular mechanisms (collaterals)
    INTRACELL			// also intracellular maintenance 
  };

  enum OutMod {
    NO_OUT,			// don't modulate output con strength
    TD_MOD			// modulate as function of TD err just like inputs
  };

  MaintMode	maint;		// how to maintain activity: extracellular vs intracellular
  OutMod	out;		// how to modulate output connections
  bool		reset;		// reset activations after receiving external reward

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(GateSpec);
  COPY_FUNS(GateSpec, taBase);
  TA_BASEFUNS(GateSpec);
};

class GateNoiseSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER noise in the gating signal parameters
public:
  float		p_pos;		// probability of a positive noise value
  float		pos_var;	// variance of the positive noise distribution
  float		neg_var;	// variance of the negative noise distribution
  float		pos_thr;	// threshold for always updating the state: positive td
  float		neg_thr;	// threshold for always updating the state: negative td

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(GateNoiseSpec);
  COPY_FUNS(GateNoiseSpec, taBase);
  TA_BASEFUNS(GateNoiseSpec);
};

class LeabraGatedCtxLayerSpec : public LeabraContextLayerSpec {
  // gated context layer: gate based on AC unit, need a different spec (& assoc con specs) for each such layer, also set LeabraTrial phase_order = MINUS_PLUS_PLUS
public:
  CtxtUpdateSpec base;		// base values for the gating constants
  GateSpec	gate;		// gating parameters: maintance by extracellular only, or plus intracellular,  out = modulate output values?, reset = reset after reward?
  GateNoiseSpec	gate_noise;	// noise in the gating term
  ConSpec_SPtr	in_con_spec;	// incoming connections to modulate using fm_hid (DYNAMIC)
  ConSpec_SPtr	maint_con_spec;	// maintenance (lateral) cons mod using fm_prv (reset when td is neg)
  ConSpec_SPtr	out_con_spec;	// output cons to modulate using to_out based on out_mod

  virtual int	FindACLayer(LeabraLayer* lay);
  // find the projection from the AC layer
  virtual float	Get_TD(LeabraLayer* lay, int ac_prjn_idx);
  // get the td error value from AC unit on 2nd connection into 1st unit in layer
  virtual bool	Get_RewReset(LeabraLayer* lay, int ac_prjn_idx);
  // get the reward reset from AC unit on 2nd connection into 1st unit in layer
  virtual void	UpdateConSpec(LeabraConSpec* cs, float new_gain);
  // update the connection spec wt_scale.abs with new gain value
  virtual void	UndoHardClamp(LeabraLayer* lay);
  // undo hard clamp settings so that layer will settle normally

  void		InitState(LeabraLayer* lay);
  void		Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void		Compute_Context(LeabraLayer* lay, LeabraUnit* u, LeabraTrial* trl);

  void		PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  // update hysteresis based on td value

  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(LeabraGatedCtxLayerSpec);
  COPY_FUNS(LeabraGatedCtxLayerSpec, LeabraContextLayerSpec);
  TA_BASEFUNS(LeabraGatedCtxLayerSpec);
};

class LeabraACMaintLayerSpec : public LeabraLayerSpec {
  // intrinsic maintenance (gc.h) driven by AC TD signals: must receive from AC Layer
public:
  float		td_clear_thresh; // Threshold td value for clearing previous intracel. maint currents
  bool		ignore_td;	// #DEF_false do not pay attention to td signal at all
  float		min_ac_pred;	// minimum AC prediction value in minus phase: AC must always predict SOME amount of reward
  bool          clear_bwt_pos_td; // clear bias weights when a positive (> td_clear_thresh) TD signal is received
  int		ac_prjn_idx;	// #READ_ONLY projection that has the AC layer

  virtual void	FindACLayer(LeabraLayer* lay);
  // find the projection from the AC layer, set ac_prjn_idx
  virtual float	Get_TD(LeabraLayer* lay);
  // get the td error value from AC unit on 2nd connection into 1st unit in layer
  virtual bool	Get_RewReset(LeabraLayer* lay);
  // get the reward reset from AC unit on 2nd connection into 1st unit in layer

  void		PhaseInit(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);

  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  SIMPLE_COPY(LeabraACMaintLayerSpec);
  COPY_FUNS(LeabraACMaintLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(LeabraACMaintLayerSpec);
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
    MINUS_ONLY,			// only present minus
    PLUS_ONLY			// only present plus
  };

  PhaseOrder	phase_order;	// order to present phases of stimuli to network

  void	Initialize();
  void	Destroy() 	{ };
  void	Copy_(const PhaseOrderEventSpec& cp);
  COPY_FUNS(PhaseOrderEventSpec, EventSpec);
  TA_BASEFUNS(PhaseOrderEventSpec);
};

//////////////////////////////////////////
// 	Time-Based Learning		//
//////////////////////////////////////////

class LeabraTimeUnit;
class LeabraTimeUnitSpec;

class LeabraTimeCon_Group : public LeabraCon_Group {
  // Leabra time connection group: just for backwards compatibility -- everything is in the base group now!
public:
  void 	Initialize()		{ };
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraTimeCon_Group);
};

class LeabraTimeUnit : public LeabraUnit {
  // Leabra unit with an single step activation history
public:
  float 	p_act_m;	// previous minus phase activation 
  float		p_act_p;	// previous plus phase activation

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LeabraTimeUnit);
  COPY_FUNS(LeabraTimeUnit, LeabraUnit);
  TA_BASEFUNS(LeabraTimeUnit);
};

class LeabraTimeUnitSpec : public LeabraUnitSpec {
  // adapts weights based on one step of activation history 
public:
  void		InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ LeabraUnitSpec::InitState(u); }
  void		Compute_dWt(Unit*) { };
  void		Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void 		UpdateWeights(Unit* u);

  void		EncodeState(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraTimeUnitSpec);
  COPY_FUNS(LeabraTimeUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(LeabraTimeUnitSpec);
};

class TimeMixSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER how much each point in time contributes
public:
  float		prv;		// contribution from previous point in time
  float		cur;		// #READ_ONLY #SHOW contribution from current point in time

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(TimeMixSpec);
  COPY_FUNS(TimeMixSpec, taBase);
  TA_BASEFUNS(TimeMixSpec);
};

class LeabraTimeConSpec : public LeabraConSpec {
  // computes weight change based on recv current and send previous acts: It is essential to set UnitSpec->opt_thresh.learn to -1 to get proper p_savg, p_max_cor values!
public:
  TimeMixSpec	time_mix;	// how much to learn based on previous versus current state information

  inline float	C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group*,
			       float ru_act, float su_act, float max_cor) {
    // wt is negative in linear form, so using opposite sign of usual here
    return ru_act * (su_act * (max_cor + cn->wt) +
		     (1.0f - su_act) * cn->wt);
  }
    
  inline void 	C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus, float heb, float err, float k_mult) {
    float dwt = lmix.err * err + lmix.hebb * heb;
    if(rus->act_reg.on) {
      if(ru->act_avg <= rus->act_reg.min)
	dwt -= rus->act_reg.wt_dt * cn->wt; // weight is -..
      else if(ru->act_avg >= rus->act_reg.max)
	dwt += rus->act_reg.wt_dt * cn->wt;
    }
    cn->dwt += k_mult * dwt;
  }

  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    LeabraTimeUnit* lru = (LeabraTimeUnit*)ru;
    LeabraUnitSpec* lrus = (LeabraUnitSpec*)ru->spec.spec;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    // this pass is for the sender_(t-1), recv_(t) weight change
    if((lru->p_act_p >= 0.0f) && (lcg->p_savg >= savg_cor.thresh) && (time_mix.prv > 0.0f)) {
      for(int i=0; i<cg->size; i++) {
	LeabraTimeUnit* su = (LeabraTimeUnit*)cg->Un(i);
	LeabraCon* cn = (LeabraCon*)cg->Cn(i);
	if(!(su->in_subgp &&	// NOTE: this is based on *current* acts_p.avg: no val for prev
	     (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get into linear form
	  C_Compute_dWt(cn, lru, lrus,
			C_Compute_Hebb(cn, lcg, lru->act_p, su->p_act_p, lcg->p_max_cor),
			C_Compute_Err(cn, lru->act_p, lru->act_m, su->p_act_p, su->p_act_m),
			time_mix.prv);
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
    // this is for the sender_(t), recv_(t) weight change
    if((((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) && (time_mix.cur > 0.0f)) {
      for(int i=0; i<cg->size; i++) {
	LeabraTimeUnit* su = (LeabraTimeUnit*)cg->Un(i);
	LeabraCon* cn = (LeabraCon*)cg->Cn(i);
	if(!(su->in_subgp &&
	     (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get into linear form
	  C_Compute_dWt(cn, lru, lrus,
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p, lcg->max_cor),
			C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m),
			time_mix.cur);
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
  }

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraTimeConSpec);
  COPY_FUNS(LeabraTimeConSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraTimeConSpec);
};


//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////

class LeabraNegBiasSpec : public LeabraBiasSpec {
  // only learns negative bias changes, not positive ones (decay restores back to zero)
public:
  float		decay;		// rate of weight decay towards zero 
  bool		updt_immed;	// update weights immediately when weights are changed

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
    if((fabsf(cn->dwt) < dwt_thresh) || (cn->dwt > 0.0f))
      cn->dwt = 0.0f;
    cn->dwt -= decay * cn->wt;
    cn->pdw = cn->dwt;
    cn->wt += cur_lrate * cn->dwt;
    cn->dwt = 0.0f;
    C_ApplyLimits(cn, ru, NULL);
  }

  inline void	B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
    LeabraBiasSpec::B_Compute_dWt(cn, ru);
    if(updt_immed) B_UpdateWeights(cn, ru);
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraNegBiasSpec);
  COPY_FUNS(LeabraNegBiasSpec, LeabraBiasSpec);
  TA_BASEFUNS(LeabraNegBiasSpec);
};

class LeabraMaintConSpec : public LeabraConSpec {
  // switches on hysteresis for sending act > maint_thresh, recv must have input = prjn[0] using NetConSpec
public:
  float		maint_thresh;  // when sending act > than this thresh, maint turned on in recv unit

  inline void 	C_Switch_Maint(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, Unit* su) {
    ru->vcb.g_h = ((LeabraCon_Group*)ru->recv.gp[0])->net * cn->wt * su->act;
  }

  inline void 	Send_Net(Con_Group* cg, Unit* su) {
    LeabraConSpec::Send_Net(cg,su);
    if (su->act>=maint_thresh){
      CON_GROUP_LOOP(cg, C_Switch_Maint((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i),
					(LeabraUnit*)cg->Un(i), su));
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraMaintConSpec);
  COPY_FUNS(LeabraMaintConSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraMaintConSpec);
};

class LeabraNetConSpec : public LeabraConSpec {
  // Saves the net-input value on a per-congroup basis
public:
  inline void	C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, Unit* su) {
    LeabraCon_Group* ru_gp = (LeabraCon_Group*)ru->recv.FastGp(cg->other_idx);
    float tmp_net = ru_gp->scale_eff * su->act * cn->wt;
    ru->net += tmp_net;
    ru_gp->net += tmp_net;
  }

  inline void 	Send_Net(Con_Group* cg, Unit* su) {
    if(inhib)
      Send_Inhib((LeabraCon_Group*)cg, (LeabraUnit*)su);
    else {
      CON_GROUP_LOOP(cg, C_Send_Net((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), cg->Un(i), su));
    }
  }

  inline float 	Compute_Net(Con_Group* cg, Unit* ru) {
    float rval=0.0f;
    CON_GROUP_LOOP(cg, rval += C_Compute_Net((LeabraCon*)cg->Cn(i), ru, cg->Un(i)));
    float temp_net = ((LeabraCon_Group*)cg)->scale_eff * rval;
    ((LeabraCon_Group*)cg)->net = temp_net;
    return temp_net;
  }

  inline void 	C_Send_ClampNet(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru,
				LeabraUnit* su) {
    LeabraCon_Group* ru_gp = (LeabraCon_Group*)ru->recv.FastGp(cg->other_idx);
    float tmp_net = ru_gp->scale_eff * su->act * cn->wt;
    ru->clmp_net += tmp_net;
    ru_gp->net += tmp_net;
  }

  inline void	Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su) {
    CON_GROUP_LOOP(cg, C_Send_ClampNet(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
  }

  void 	Initialize()		{ };
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraNetConSpec);
};


class PhaseDWtConSpec : public LeabraConSpec {
  // specifies that weight changes should only take place for certain plus-phases (in multi-plus phase cases): only works with PhaseDWtUnitSpec
public:
  enum dWtPhase {
    NO_FIRST_DWT,		// for three phase cases: don't change weights after first plus
    ONLY_FIRST_DWT,		// for three phase cases: only change weights after first plus
    ALL_DWT			// for three phase cases: change weights after *both* post-minus phases
  };

  dWtPhase		dwt_phase; // what phase to change weights in

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(PhaseDWtConSpec);
  COPY_FUNS(PhaseDWtConSpec, LeabraConSpec);
  TA_BASEFUNS(PhaseDWtConSpec);
};

class PhaseDWtUnitSpec : public LeabraUnitSpec {
  // looks for phase dwt con specs and applies their dwt preferences
public:
  enum dWtPhase {
    NO_FIRST_DWT,		// for three phase cases: don't change weights after first plus
    ONLY_FIRST_DWT,		// for three phase cases: only change weights after first plus
    ALL_DWT			// for three phase cases: change weights after *both* post-minus phases
  };

  dWtPhase		dwt_phase; // what phase to change weights in: for the bias weights

  void		Compute_dWt_impl(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  
  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(PhaseDWtUnitSpec);
  COPY_FUNS(PhaseDWtUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(PhaseDWtUnitSpec);
};

class LeabraTabledConSpec : public LeabraConSpec {
  // Leabra connection spec with table driven learning, supporting one and two phase learning rules
public:
  enum TableType {
    ONE_PHASE,			// hebbian-like learning based only on plus-phase activations
    ONE_PHASE_WITH_WT,		// one-phase learning rule that incldues dependency on current weight value
    TWO_PHASE,			// error-driven-style learning rule based on minus and plus phase activations
    TWO_PHASE_WITH_WT		// two-phase learning rule that incldues dependency on current weight value
  };

  enum Squashing {
    SQUASH_WTS,
    DONT_SQUASH_WTS
  };

  FunLookupND	dwt_table;	// #HIDDEN #AKA_mesh the lookup table for the learning rule implemented
  String	cur_table;	// #READ_ONLY #SHOW name of last table loaded
  TableType  	table_type; 	// what type of dwt table we have: hebbian (one phase) or hebb + error-driven (two phase)
  Squashing 	squashing; 	// squash the table-driven weight change values to keep them within 0-1 range (multiply inc by 1-w and dec by w)
  float       	table_mix;      // what fraction of the wt change shoud be calculated using the lookup table (1 - table_mix) is the standard leabra dwt computationa

  inline void  C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus,
			     LeabraUnit* su, float heb, float err);
  inline void  Compute_dWt(Con_Group* cg, Unit* ru);

  virtual float LookupDWt(float su_act_m, float ru_act_m, float su_act_p, float ru_act_p, float wt);
  // #BUTTON #USE_RVAL get dwt value for given set of parameters using current table
  virtual void	LoadDWtTable(istream& is);
  // #BUTTON #EXT_tab loads weight-change lookup table from a file
  virtual void  ListTable(ostream& strm = cout);
  // #BUTTON #ARGC_0 #CONFIRM output the lookup table in text format
  virtual void  ShiftNorm(float desired_mean);
  // #BUTTON normalize the function values via an additive shift to achieve overall desired mean
  virtual void  MulNorm(float desired_mean);
  // #BUTTON normalize the function values by multiplying positive and negative values by separate scaling factors to achieve desired mean

  virtual void	Graph2DTable(GraphLog* disp_log);
  // #BUTTON #NULL_OK graph a two-d table (doesn't work for higher dim tables)

  void        Defaults();
  void        Initialize();
  void        Destroy()               { };
  void        InitLinks();
  SIMPLE_COPY(LeabraTabledConSpec);
  COPY_FUNS(LeabraTabledConSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraTabledConSpec);
};

// combine table with hebbian and error-driven 
inline void LeabraTabledConSpec::C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus,
					       LeabraUnit* su, float heb, float err)
{
  float table = LookupDWt(su->act_m, ru->act_m, su->act_p, ru->act_p, -1.0f*cn->wt);
  if(squashing == SQUASH_WTS) {
    if(table > 0.0f) {
      table *= 1.0f + cn->wt; // since cn->wt has its sign fliped, equiv to *= 1-cn->wt
    }
    else {
      table *= -1.0f * cn->wt; // due to sign flip, equiv to *= cn->wt
    } 
  }

  float dwt; 
  if((table_type == ONE_PHASE) || (table_type == ONE_PHASE_WITH_WT)) {
    float heb_tab = table_mix * table + (1.0f - table_mix) * heb;
    dwt = lmix.hebb * heb_tab + lmix.err * err;
  }
  else { // Table represents a two phase learning rule 
    float analytic_dwt = lmix.err * err + lmix.hebb * heb;
    dwt = table_mix * table + (1.0f - table_mix) * analytic_dwt;
  }

  if(rus->act_reg.on) {
    if(ru->act_avg <= rus->act_reg.min)
      dwt -= rus->act_reg.wt_dt * cn->wt; // weight is -..
    else if(ru->act_avg >= rus->act_reg.max)
      dwt += rus->act_reg.wt_dt * cn->wt;
  }
  cn->dwt += dwt;
}

inline void LeabraTabledConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraUnit* lru = (LeabraUnit*)ru;
  LeabraUnitSpec* lrus = (LeabraUnitSpec*)ru->spec.spec;
  LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
  Compute_SAvgCor(lcg, lru);
  if(((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) {
    for(int i=0; i<cg->size; i++) {
      LeabraUnit* su = (LeabraUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	float orig_wt = cn->wt;
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	C_Compute_dWt(cn, lru, lrus, su,
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
      }
    }
  }
}

//////////////////////////////////
// 	Scalar Value Layer	//
//////////////////////////////////

class ScalarValSpec : public MinMaxRange {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for scalar values
public:
  float		un_width;	// sigma parameter of a gaussian specifying the tuning width of the coarse-coded units (in min-max units)
  bool		clamp_full_pat;	// #DEF_false if true, environment provides full set of values to clamp over entire layer (instead of providing single scalar value to clamp on 1st unit, which then generates a corresponding distributed pattern)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ScalarValSpec);
  COPY_FUNS(ScalarValSpec, MinMaxRange);
  TA_BASEFUNS(ScalarValSpec);
};

class ScalarValLayerSpec : public LeabraLayerSpec {
  // represents a scalar value using a coarse-coded distributed code over units.  first unit represents scalar value.
public:
  ScalarValSpec	 scalar;	// specifies the range of values represented.  add extra values above and below true useful range to prevent edge effects.
  MinMaxRange	 val_range;	// #READ_ONLY actual range of values (scalar.min/max taking into account un_range)

  virtual void	ClampValue(Unit_Group* ugp, LeabraTrial* trl, float rescale=1.0f);
  // clamp value in the first unit's ext field to the units in the group
  virtual float	ClampAvgAct(int ugp_size);
  // computes the average activation for a clamped unit pattern (for computing rescaling)
  virtual float	ReadValue(Unit_Group* ugp, LeabraTrial* trl);
  // read out current value represented by activations in layer
  virtual void	ResetAfterClamp(LeabraLayer* lay, LeabraTrial* trl);
  // reset activation of first unit(s) after hard clamping
  virtual void	HardClampExt(LeabraLayer* lay, LeabraTrial* trl);
  // hard clamp current ext values (on all units, after ClampValue called) to all the units

  virtual void	LabelUnits_impl(Unit_Group* ugp);
  // label units with their underlying values
  virtual void	LabelUnits(LeabraLayer* lay);
  // #BUTTON label units in given layer with their underlying values
  virtual void	LabelUnitsNet(Network* net);
  // #BUTTON label all layers in given network using this spec

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  // don't include first unit in any of these computations!
  void 	Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(ScalarValLayerSpec);
  COPY_FUNS(ScalarValLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(ScalarValLayerSpec);
};

class ScalarValSelfPrjnSpec : public ProjectionSpec {
  // special projection for making self-connection that establishes neighbor similarity in scalar val
public:
  int	width;			// width of neighborhood, in units (i.e., connect width units to the left, and width units to the right)
  float	wt_width;		// width of the sigmoid for providing initial weight values
  float	wt_max;			// maximum weight value (of 1st neighbor -- not of self unit!)

  virtual void	Connect_UnitGroup(Unit_Group* gp, Projection* prjn);
  void		Connect_impl(Projection* prjn);
  void		C_InitWtState(Projection* prjn, Con_Group* cg, Unit* ru);
  // uses weight values as specified in the tesselel's

  void	Initialize();
  void 	Destroy()		{ };
  SIMPLE_COPY(ScalarValSelfPrjnSpec);
  COPY_FUNS(ScalarValSelfPrjnSpec, ProjectionSpec);
  TA_BASEFUNS(ScalarValSelfPrjnSpec);
};

//////////////////////////////////////////////////////////////////
// 	BG-based PFC Gating/RL learning Mechanism		//
//////////////////////////////////////////////////////////////////


class MarkerConSpec : public LeabraConSpec {
  // connection spec that marks special projections: doesn't send netin or adapt weights
public:
  // don't send regular net inputs or learn!
  inline float 	Compute_Net(Con_Group*, Unit*) { return 0.0f; }
  inline void 	Send_Net(Con_Group*, Unit*) { };
  inline void 	Send_NetDelta(LeabraCon_Group*, LeabraUnit*) { };
  inline void 	Compute_dWt(Con_Group*, Unit*) { };
  inline void	UpdateWeights(Con_Group*, Unit*) { };

  bool	 DMem_AlwaysLocal() { return true; }
  // these connections always need to be there on all nodes..

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(MarkerConSpec);
};

class TdModUnit : public LeabraUnit {
  // Leabra unit with temporal-differences error modulation of minus phase activation for learning
public:
  float		act_pp;		// second plus phase activations
  float 	td;		// modulatory temporal-differences signal to apply to minus phase act: computed by sending TdLayerSpec
  float 	p_act_m;	// previous minus phase activation 
  float		p_act_p;	// previous plus phase activation

  void	Initialize();
  void	Destroy()	{ };
  void	Copy_(const TdModUnit& cp);
  COPY_FUNS(TdModUnit, LeabraUnit);
  TA_BASEFUNS(TdModUnit);
};

class TdModUnitSpec : public LeabraUnitSpec {
  // Leabra unit with temporal-differences error modulation of minus phase activation for learning
public:
  void		InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ LeabraUnitSpec::InitState(u); }
  void		Compute_dWt(Unit*) { };
  void 		Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void 		UpdateWeights(Unit* u);
  void 		EncodeState(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void		DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay);
  void		PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			   LeabraTrial* trl, bool set_both=false);

  void	Defaults();

  void	Initialize();
  void	Destroy()		{ };
//    SIMPLE_COPY(TdModUnitSpec);
//    COPY_FUNS(TdModUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(TdModUnitSpec);
};

class TdLearnSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for td-based learning of normal network connections
public:
  float		gain;		// #DEF_1 gain multiplier of td values
  float		lmix;		// #DEF_0.2 proportion td contributes to overall learning
  float		lmix_c;		// #READ_ONLY 1-lmix
  float		min_lrate;	// #DEF_0.2 minimum td modulated lrate -- td modulates lrate between this value and 1.0, so if its 1.0 then no modulation

  void	UpdateAfterEdit();
  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(TdLearnSpec);
  COPY_FUNS(TdLearnSpec, taBase);
  TA_BASEFUNS(TdLearnSpec);
};

class TdModConSpec : public LeabraConSpec {
  // connections that use td (temporal differences) modulation
public:
  TdLearnSpec	td;		// td parameters: gain, contribution to learning

  inline float C_Compute_Err(LeabraCon* cn, float ru_td, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
    float err = (td.lmix_c * ((ru_act_p * su_act_p) - (ru_act_m * su_act_m))) +
      td.lmix * (ru_td * (ru_act_m * su_act_m));
    
    // wt is negative in linear form, so using opposite sign of usual here
    if(err > 0.0f)	err *= (1.0f + cn->wt);
    else			err *= -cn->wt;	
    return err;
  }

  inline void C_Compute_dWt(LeabraCon* cn, TdModUnit* ru, LeabraUnitSpec* rus, float heb, float err) {
    float dwt = lmix.err * err + lmix.hebb * heb;
    if(rus->act_reg.on) {
      if(ru->act_avg <= rus->act_reg.min)
	dwt -= rus->act_reg.wt_dt * cn->wt; // weight is -..
      else if(ru->act_avg >= rus->act_reg.max)
	dwt += rus->act_reg.wt_dt * cn->wt;
    }
    float mlr = td.min_lrate + fabs(ru->td);
    mlr = MIN(1.0f, mlr);
    cn->dwt += mlr * dwt;
  }

  // this computes weight changes based on sender at time t-1
  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    TdModUnit* lru = (TdModUnit*)ru;
    LeabraUnitSpec* lrus = (LeabraUnitSpec*)ru->spec.spec;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if(((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) {
      for(int i=0; i<cg->size; i++) {
	LeabraUnit* su = (LeabraUnit*)cg->Un(i);
	LeabraCon* cn = (LeabraCon*)cg->Cn(i);
	if(!(su->in_subgp &&
	     (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get into linear form
	  C_Compute_dWt(cn, lru, lrus,
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_Err(cn, lru->td, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
  }

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(TdModConSpec);
  COPY_FUNS(TdModConSpec, ConSpec);
  TA_BASEFUNS(TdModConSpec);
};

//////////////////////////////////
//	Immediate Rew Pred	//
//////////////////////////////////

class ImRewPredSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for immediate reward computing stuff
public:
  bool	graded;			// #DEF_false compute a graded error signal as a function of number of correct output values
  bool	no_off_err;		// #DEF_false do not count a unit wrong if it is off but target says on -- only count wrong units that are on but should be off

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ImRewPredSpec);
  COPY_FUNS(ImRewPredSpec, taBase);
  TA_BASEFUNS(ImRewPredSpec);
};

class ImRewPredLayerSpec : public ScalarValLayerSpec {
  // THIS IS DEPRECATED: DO NOT USE! predicts immediate rewards: gets rew from MarkerConSpec layer error; minus phase = prediction of reward; plus phase = actual reward; difference = delta_r = what drives subsequent TD learning
public:
  MinMaxRange	pred_range;	// [Default: .1, 1] restrict predictions (in minus phase) to this range: ensures some continued error and reward values even if no prediction
  ImRewPredSpec	rew;		// additional reward computation and prediction parameters

  virtual bool	RewardAvail(LeabraLayer* lay, LeabraTrial* trl);
  // figure out if reward is available on this trial (look if target signals are present)
  virtual float	GetReward(LeabraLayer* lay, LeabraTrial* trl);
  // get reward value based on error at layer with MarkerConSpec connection: 1 = rew (correct), 0 = err, -1 = no info

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(ImRewPredLayerSpec);
  COPY_FUNS(ImRewPredLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(ImRewPredLayerSpec);
};

class ErrRateSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER compute error rate in td layer misc_1
public:
  float	td_err_thr;		// #DEF_0.2 threshold td magnitude for an error (if td < -td_err_thr then err, else no err)
  float	rate_dt;		// time constant for averaging errors over time to compute err rate

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ErrRateSpec);
  COPY_FUNS(ErrRateSpec, taBase);
  TA_BASEFUNS(ErrRateSpec);
};

class TdLayerSpec : public LeabraLayerSpec {
  // computes activation = avg act_dif of sending units in plus phase: note, act will go negative!
public:
  ErrRateSpec	err_rate;	// specs for computing rate of errors in unit misc_1

  virtual void	Compute_ZeroAct(LeabraLayer* lay, LeabraTrial* trl);
  virtual void	Compute_Td(LeabraLayer* lay, LeabraTrial* trl);
  virtual void	Send_Td(LeabraLayer* lay, LeabraTrial* trl);

  void	InitWtState(LeabraLayer* lay);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	ExtToComp(LeabraLayer* lay, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);

  void	Compute_dWt(LeabraLayer*, LeabraTrial*) { }; // nop

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(TdLayerSpec);
  COPY_FUNS(TdLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(TdLayerSpec);
};


//////////////////////////////////
//	Delayed Rew Pred: NAc	//
//////////////////////////////////

class RewPredConSpec : public LeabraConSpec {
  // Reward Prediction connections: learn to predict subsequent reward based on state at time t-1 based on reward at time t: needs to span 1 time step gap
public:
  inline float C_Compute_Err(LeabraCon* cn, float ru_act_p, float ru_act_m, float su_act_p) {
    float err = (ru_act_p - ru_act_m) * su_act_p;
    // wt is negative in linear form, so using opposite sign of usual here
    if(err > 0.0f)	err *= (1.0f + cn->wt);
    else		err *= -cn->wt;	
    return err;
  }

  // this computes weight changes based on sender at time t-1
  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    TdModUnit* lru = (TdModUnit*)ru;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if(lru->p_act_p >= 0.0f) {
      for(int i=0; i<lcg->size; i++) {
	TdModUnit* su = (TdModUnit*)lcg->Un(i);
	LeabraCon* cn = (LeabraCon*)lcg->Cn(i);
	float orig_wt = cn->wt;
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	// no Hebbian!
	cn->dwt += C_Compute_Err(cn, lru->act_p, lru->act_m, su->p_act_p);
	cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
      }
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(RewPredConSpec);
};

class RewPredSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for reward computing stuff
public:
  float		discount;	// [Default .8] discount factor for future rewards
  float		gain;		// #DEF_0.5 gain (multiplier) of the reward signal
  bool		graded;		// #DEF_false compute a graded reward signal as a function of number of correct output values
  bool		no_off_err;	// #DEF_false do not count a unit wrong if it is off but target says on -- only count wrong units that are on but should be off
  bool		sub_avg;	// subtract average reward value in computing rewards
  float		avg_dt;		// time constant for integrating average reward value

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(RewPredSpec);
  COPY_FUNS(RewPredSpec, taBase);
  TA_BASEFUNS(RewPredSpec);
};

class RewPredLayerSpec : public ScalarValLayerSpec {
  // predicts delayed rewards: minus phase = prior expected reward, plus = discounted expectation of future reward + current reward
public:
  enum RewardType {		// how do we get the reward values?
    TD_VAL,			// learn based on td values received from TDLayer (e.g., via ImRewPredLayerSpec or another RewPredLayerSpec)
    OUT_ERR_REW,		// get rewards directly as a function of errors on the output layer ONLY WHEN RewTarg layer act > .5 -- get from markerconspec from output layer(s)
    EXT_REW			// get rewards directly as external inputs marked as ext_flag = TARG to the first unit in the layer
  };

  RewardType	rew_type;	// how do we get the reward values?
  MinMaxRange	pred_range;	// [Default: -1, 1] restrict predictions of future reward (in plus phase) to this range
  RewPredSpec	rew;		// reward prediction specifications

  virtual bool	OutErrRewAvail(LeabraLayer* lay, LeabraTrial* trl);
  // figure out if reward is available on this trial (look if target signals are present)
  virtual float	GetOutErrRew(LeabraLayer* lay, LeabraTrial* trl);
  // get reward value based on error at layer with MarkerConSpec connection: 1 = rew (correct), 0 = err, -1 = no info
  virtual void 	Compute_ExtRew(LeabraLayer* lay, LeabraTrial* trl);
  // get any external rewards and put them in the td val

  virtual void 	Compute_SavePred(Unit_Group* ugp, LeabraTrial* trl); // save current prediction to misc_1 for later clamping
  virtual void 	Compute_ClampPred(Unit_Group* ugp, LeabraTrial* trl); // clamp misc_1 to ext 
  virtual void 	Compute_ClampPrev(LeabraLayer* lay, LeabraTrial* trl);
  // clamp minus phase to previous act value
  virtual void 	Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial* trl);
  // copy ext values to act_p
  virtual void 	Compute_TdPlusPhase(Unit_Group* ugp, LeabraTrial* trl);
  // clamp plus phase based on td computation of immediate reward plus discounted future reward

  void	InitState(LeabraLayer* lay);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	ExtToComp(LeabraLayer* lay, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(RewPredLayerSpec);
  COPY_FUNS(RewPredLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(RewPredLayerSpec);
};

//////////////////////////
//	  Patch	 	//
//////////////////////////

class PatchLearnSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER how to train up the patch units
public:
  enum LearnMode {
    JUST_TD,			// just use straight td signal
    PROT_NEG			// protect negative td signals as fctn of prior raw prediction (act_m)
  };

  LearnMode	lrn_mod;	// #DEF_PROT_NEG how to modulate learning 
  float		td_gain;	// #DEF_1 multiplier for temporal-differences (dopamine, from vta)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(PatchLearnSpec);
  COPY_FUNS(PatchLearnSpec, taBase);
  TA_BASEFUNS(PatchLearnSpec);
};

class PatchLayerSpec : public RewPredLayerSpec {
  // patch layer: computes reward expectation only for cases where corresp matrix units are gating
public:
  PatchLearnSpec	learn;		// how to train up patch units: 

  virtual LeabraLayer* 	FindMatrixLayer(LeabraLayer* lay, int& matrix_prjn_idx);
  // find the projection for the matrix layer

  virtual void Compute_ZeroAct(Unit_Group* ugp, LeabraTrial* trl);
  // zero out activations when not in upstate

  virtual void 	Compute_SavePredFmActP(Unit_Group* ugp, LeabraTrial* trl); // save current prediction to misc_1 for later clamping

  void Compute_TdPlusPhase(Unit_Group* ugp, LeabraTrial* trl, float snc);
  // clamp plus phase based on td computation of immediate reward plus discounted future reward

  void 	InitWtState(LeabraLayer* lay);
  void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void 	InitLinks();
  SIMPLE_COPY(PatchLayerSpec);
  COPY_FUNS(PatchLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(PatchLayerSpec);
};

class SNcLayerSpec : public TdLayerSpec {
  // computes activation = previous raw reward prediction (act_m) for stripe from patch, sends to matrix for protect_neg
public:
  void	Compute_Td(LeabraLayer* lay, LeabraTrial* trl);
  void	Send_Td(LeabraLayer* lay, LeabraTrial* trl);

  void 	Initialize();
  void	Destroy()		{ };
//    void 	InitLinks();
//    SIMPLE_COPY(SNcLayerSpec);
//    COPY_FUNS(SNcLayerSpec, TdLayerSpec);
  TA_BASEFUNS(SNcLayerSpec);
};

//////////////////////////
//	  Matrix 	//
//////////////////////////

class MatrixConSpec : public LeabraConSpec {
  // Learning of matrix input connections based on temporal-differences modulation of activation: td happens one time step later!
public:
  inline float C_Compute_Err(LeabraCon* cn, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
    float err = (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
    // wt is negative in linear form, so using opposite sign of usual here
    if(err > 0.0f)	err *= (1.0f + cn->wt);
    else		err *= -cn->wt;	
    return err;
  }

  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    TdModUnit* lru = (TdModUnit*)ru;
    LeabraUnitSpec* lrus = (LeabraUnitSpec*)ru->spec.spec;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if((lru->p_act_p >= 0.0f) && (lcg->p_savg >= savg_cor.thresh)) {
      for(int i=0; i<lcg->size; i++) {
	TdModUnit* su = (TdModUnit*)lcg->Un(i);
	LeabraCon* cn = (LeabraCon*)lcg->Cn(i);
	if(!(su->in_subgp &&
	     (((LeabraUnit_Group*)su->owner)->acts.avg < savg_cor.thresh))) {
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get into linear form
	  C_Compute_dWt(cn, lru, lrus,
			C_Compute_Hebb(cn, lcg, lru->p_act_p, su->p_act_p),
			C_Compute_Err(cn, lru->p_act_p, lru->p_act_m, su->p_act_p, su->p_act_m));
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  TA_BASEFUNS(MatrixConSpec);
};

class NoiseBiasCon : public LeabraCon {
  // noise bias connection weight -- holds baseline value (zwt) separate from baseline + noise (wt)
public:
  float		zwt;		// 'zero' bias weight value -- baseline offset

  void 	Initialize()		{ zwt = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const NoiseBiasCon& cp);
  COPY_FUNS(NoiseBiasCon, LeabraCon);
  TA_BASEFUNS(NoiseBiasCon);
};

class NoiseBiasSpec : public LeabraBiasSpec {
  // noise bias connection -- holds noise value separate from orig value
public:
  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su) {
    LeabraBiasSpec::C_InitWtState(cg, cn, ru, su); NoiseBiasCon* lcn = (NoiseBiasCon*)cn;
    lcn->zwt = lcn->wt; }

  void B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
    TdModUnit* tdu = (TdModUnit*)ru;
    float err = tdu->td * tdu->act_eq;
    cn->dwt += err;
  }

  void B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
    NoiseBiasCon* nbc = (NoiseBiasCon*)cn;
    if(fabsf(cn->dwt) < dwt_thresh)
      cn->dwt = 0.0f;
    cn->pdw = cn->dwt;
    nbc->zwt += cur_lrate * cn->dwt; // update the zwt
    cn->dwt = 0.0f;
    // C_ApplyLimits(cn, ru, NULL);
  }

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(NoiseBiasSpec);
};

class MatrixUnitSpec : public TdModUnitSpec {
  // basal ganglia matrix units: fire actions or WM updates. modulated by td signals from NAc/Vta, Patch/SNc
public:
  void	Compute_dWt(Unit*)	{ };
  void	Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_dWt_impl(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // allow bias weights to learn even when clamped
  void	UpdateWeights(Unit* u);

  void	Defaults();

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  TA_BASEFUNS(MatrixUnitSpec);
};

class UpDnGradSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER gradient for allocating up/down currents across groups
public:
  float		start;		// starting value for bias in first groups
  int		gps_per;	// groups of stripes per value
  float		incr;		// increment in bias for next group of stripes

  inline float	GetG(int gp) {	// get current for given group number
    int lvl = gp / gps_per;
    return start + (float)lvl * incr;
  }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(UpDnGradSpec);
  COPY_FUNS(UpDnGradSpec, taBase);
  TA_BASEFUNS(UpDnGradSpec);
};

class MaintDurSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER maintenance duration specification for allocating different durations across groups
public:
  int	start;			// starting duration for the first set of groups
  int	gps_per;		// number of groups per given duration level
  int	incr;			// increment in duration level between sets of groups

  int	GetDur(int gp) {	// get duration for given group
    int lvl = gp / gps_per;
    return start + lvl * incr;
  }

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(MaintDurSpec);
  COPY_FUNS(MaintDurSpec, taBase);
  TA_BASEFUNS(MaintDurSpec);
};

class MatrixGateSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER matrix gating specs
public:
  float		down_g_a;	// #DEF_1 accommodation current for units in the "down" state: ineligible
  bool		no_patch;	// there is no patch layer represented: update my own gates!
  bool		go_toggle;	// #DEF_true a GO signal in 2nd plus phase when already maintaining = OFF! else instant update..
  float		snc_gain;	// #DEF_1 gain multiplier on the SNc modulation signal (0 = don't use SNc at all!)
  float		snc_err_thr;	// #DEF_0.1 error rate threshold for using SNc signal to modulate the td signal: only listen to SNc when err rate is below this level

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(MatrixGateSpec);
  COPY_FUNS(MatrixGateSpec, taBase);
  TA_BASEFUNS(MatrixGateSpec);
};

class MatrixLayerSpec : public LeabraLayerSpec {
  // basal ganglia matrix layer: fire actions/WM updates, or no-op; 1st two phases are clamped to prior, settles in 2nd plus for new gating action
public:
  enum	GateSignal {
    GATE_GO = 0,		// gate GO unit fired 
    GATE_NOGO = 1,		// gate NOGO unit fired
  };

  Random	bias_noise;	// noise in bias weights (facilitates exploration)
  UpDnGradSpec	nop_up_g_h;	// hysteresis current gradients for NOP action when it is in up state (on, i.e., after a gate-on)
  MaintDurSpec	max_maint_dur;	// maximum maintenance durations for different stripes 
  MatrixGateSpec gate;		// matrix gating parameters

  virtual GateSignal	GetGateSignal(LeabraLayer* lay, int gp);
  // get gating signal from minus phase activations of units in given group (eq = use act_eq instead of act_p)

  virtual void 	Compute_ClampMinus(LeabraUnit_Group* ugp, LeabraTrial* trl);
  virtual void	UpdateGates(LeabraLayer* lay, LeabraTrial* trl);
  // update the gating info based on current firing states

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);
  void 	InitWtState(LeabraLayer* lay);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(MatrixLayerSpec);
  COPY_FUNS(MatrixLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(MatrixLayerSpec);
};

class PFCMaintSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER pfc maintenance specifications
public:
  enum TDModGCType {
    NO_TD_MOD,			// no modulation of td values (from VTA) on gc maintenance currents
    MAINT_ONLY,			// only modulate units that have already had maintenance switched on
    ALL_UNITS			// modulate all units
  };

  bool 		off_accom;	// whether to turn on accommodation for off-going units (keep them off, at end of trial)
  TDModGCType	td_mod_gc;	// how td (from VTA) modulates maintenance currents
  float		td_gain;	// multiplier for td values as they affect gc

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(PFCMaintSpec);
  COPY_FUNS(PFCMaintSpec, taBase);
  TA_BASEFUNS(PFCMaintSpec);
};

class PFCLayerSpec : public LeabraLayerSpec {
  // Prefrontal cortex layer: gets gating signal from matrix, gate updates before each plus phase (toggle off, toggle on)
public:
  PFCMaintSpec	maint;		// various parameters affecting maintenance currents in pfc units

  virtual LeabraLayer*	FindMatrixLayer(LeabraLayer* lay, int& matrix_prjn_idx);
  // find the projection from the matrix layer, set matrix_prjn_idx

  virtual void 	Compute_GatePPhase12(LeabraLayer* lay, LeabraTrial* trl, LeabraLayer* matrix_lay, MatrixLayerSpec* mls);
  // compute the gating signal in plus phases 1 and 2

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  // don't hard clamp: keep 2nd plus phase free running..

  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);

  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();
  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  TA_BASEFUNS(PFCLayerSpec);
};

//////////////////////////////////
//	Leabra Wizard		//
//////////////////////////////////

class LeabraWiz : public Wizard {
  // Leabra-specific wizard for automating construction of simulation objects
public:
  virtual void 	StdNetwork(Network* net = NULL);

  virtual void	StdLayerSpecs(Network* net);
  // #MENU_BUTTON #MENU_ON_Network #MENU_SEP_BEFORE make standard layer specs for a basic Leabra network (KWTA_AVG 25% for hiddens, KWTA PAT_K for input/output)

  virtual void	SRNContext(Network* net);
  // #MENU_BUTTON configure a simple-recurrent-network context layer in the network

  virtual void	UnitInhib(Network* net, int n_inhib_units=10);
  // #MENU_BUTTON configures unit-based inhibition for all layers in the network

  virtual void 	BgPFC(Network* net, bool make_patch_snc=true, bool out_to_pfc_cons=true, int n_stripes=8);
  // #MENU_BUTTON #MENU_SEP_BEFORE configure all the layers and specs for doing basal-ganglia based gating of the pfc layer!  can exclude patch_snc system, output to pfc cons

  virtual void SetPFCStripes(Network* net, int n_stripes, int n_units=-1);
  // #MENU_BUTTON set number of "stripes" (unit groups) throughout the entire set of pfc/bg layers (n_units = -1 = use current # of units)

  void 	Initialize();
  void 	Destroy()	{ };
//   SIMPLE_COPY(LeabraWiz);
//   COPY_FUNS(LeabraWiz, Wizard);
  TA_BASEFUNS(LeabraWiz);
};

#endif // leabra_h
