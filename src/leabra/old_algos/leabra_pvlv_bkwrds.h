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

// use this macro for iterating over either unit groups one-by-one, or the one
// unit group that is the layer->units, and applying 'code' to either
#define UNIT_GP_ITR(lay, code) \
  int gi = 0; \
  if(lay->units.gp.size > 0) { \
    for(gi=0; gi<lay->units.gp.size; gi++) { \
      Unit_Group* ugp = (Unit_Group*)lay->units.gp[gi]; \
      code \
    } \
  } \
  else { \
    Unit_Group* ugp = (Unit_Group*)&(lay->units); \
    code \
  } 
  
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
  bool		err_sb;		// #DEF_true apply exponential soft-bounding to the error learning component

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LearnMixSpec);
  COPY_FUNS(LearnMixSpec, taBase);
  TA_BASEFUNS(LearnMixSpec);
};

class SAvgCorSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER sending average activation correction specifications: affects hebbian learning and netinput computation
public:
  float		cor;		// #DEF_0.4 proportion of correction to apply (0=none, 1=all, .5=half, etc)
  float		thresh;		// #DEF_0.001 threshold of sending average activation below which learning does not occur (prevents learning when there is no input)
  bool		norm_con_n;	// #DEF_false #AKA_div_gp_n in normalizing netinput, divide by the actual number of connections (recv group n), not the overall number of units in the sending layer; this is good when units only receive from a small fraction of sending layer units

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SAvgCorSpec);
  COPY_FUNS(SAvgCorSpec, taBase);
  TA_BASEFUNS(SAvgCorSpec);
};

class LeabraConSpec : public ConSpec {
  // Leabra connection specs
public:
  enum	LRSValue {		// what value to drive the learning rate schedule with
    NO_LRS,			// don't use a learning rate schedule
    EPOCH,			// current epoch counter
    SE_STAT,			// LeabraSE_Stat value at epoch-level (in process above epoch)
    EXT_REW_STAT,		// ExtRew_Stat value at epoch-level (in process above epoch): value is * 100 (0..100) 
    EXT_REW_AVG			// uses average reward computed by ExtRew layer (if present): value is units[0].act_avg (avg_rew) * 100 (0..100) 
  };

  bool		inhib;		// #DEF_false makes the connection inhibitory (to g_i instead of net)
  WtScaleSpec	wt_scale;	// scale weight values, both relative and absolute factors
  WtSigSpec	wt_sig;		// sigmoidal weight function for contrast enhancement: high gain makes weights more binary & discriminative
  float		lrate;		// #DEF_0.01 learning rate -- how fast do the weights change per experience
  float		cur_lrate;	// #READ_ONLY #NO_INHERIT #SHOW current actual learning rate = lrate * lrate_sched current value (* 1 if no lrate_sched)
  LRSValue	lrs_value;	// what value to drive the learning rate schedule with (Important: affects values entered in start_ctr fields of schedule!)
  Schedule	lrate_sched;	// schedule of learning rate over training epochs (multiplies lrate!)
  LearnMixSpec	lmix;		// mixture of hebbian & err-driven learning
  SAvgCorSpec	savg_cor;	// #AKA_fix_savg for Hebbian and netinput computation: correction for sending average act levels (i.e., renormalization); also norm_con_n for normalizing netinput computation
  
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

  virtual void	C_InitWtState_Post(Con_Group*, Connection*, Unit*, Unit*) { };
  // hook for setting other weight-like values after initializing the weight value

  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su) {
    ConSpec::C_InitWtState(cg, cn, ru, su); LeabraCon* lcn = (LeabraCon*)cn;
    lcn->pdw = 0.0f; C_InitWtState_Post(cg, cn, ru, su); }

  void 		C_InitWtDelta(Con_Group* cg, Connection* cn, Unit* ru, Unit* su)
  { ConSpec::C_InitWtDelta(cg, cn, ru, su); ((LeabraCon*)cn)->dwt=0.0f; }

  inline float 	C_Compute_Net(LeabraCon* cn, Unit*, Unit* su);
  inline float 	Compute_Net(Con_Group* cg, Unit* ru);
  // receiver-based net input 

  inline void 	C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, float su_act_eff);
  inline void 	Send_Net(Con_Group* cg, Unit* su);
  // sender-based net input computation

  inline void 	C_Send_Inhib(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, float su_act_eff);
  inline virtual void Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based inhibitiory net input computation

  inline void 	C_Send_NetDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, float su_act_delta_eff);
  inline virtual void Send_NetDelta(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based delta net input computation (send_delta mode only)

  inline void 	C_Send_InhibDelta(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, float su_act_delta_eff);
  inline virtual void Send_InhibDelta(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based delta inhibitiory net input computation (send_delta mode only)

  inline void 	C_Send_ClampNet(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, float su_act_eff);
  inline virtual void Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based net input computation for clamp net

  inline virtual void Compute_SAvgCor(LeabraCon_Group* cg, LeabraUnit* ru);
  // compute hebb correction scaling term for sending average act (cg->savg_cor) based on layer target activity percent

  inline float	C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg,
			       float ru_act, float su_act);
  // compute Hebbian associative learning

  inline float 	C_Compute_Err(LeabraCon*, float ru_act_p, float ru_act_m,
				  float su_act_p, float su_act_m);
  // compute generec error term, sigmoid case

  inline void 	C_Compute_dWt(LeabraCon* cn, LeabraUnit* ru, float heb, float err);
  // combine associative and error-driven weight change, actually update dwt

  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);
  // compute weight change: make new one of these for any C_ change above: hebb, err, dwt

  inline virtual void	B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru);
  // compute bias weight change for netin model of bias weight

  inline void		C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg, 
					LeabraUnit* ru, LeabraUnit* su, LeabraUnitSpec* rus);
  // update weights, if activation regulation is NOT in effect
  inline void		C_Compute_ActReg(LeabraCon* cn, LeabraCon_Group* cg, 
					 LeabraUnit* ru, LeabraUnit* su, LeabraUnitSpec* rus);
  // compute dwt for activation regulation
  inline void		C_UpdateWeightsActReg(LeabraCon* cn, LeabraCon_Group* cg, 
					      LeabraUnit* ru, LeabraUnit* su, LeabraUnitSpec* rus);
  // update weights, if activation regulation is in effect
  inline void		UpdateWeights(Con_Group* cg, Unit* ru);
  inline virtual void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus);

  virtual void	SetCurLrate(int epoch, LeabraTrial* trl);
  // set current learning rate based on schedule given epoch (or error value)

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

  inline void	B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru);

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
  float		savg_cor;	// #NO_SAVE savg correction factor for hebbian learning
  float		net;		// #NO_SAVE netinput to this con_group: not normally computed!

  void	Compute_LinFmWt()  { ((LeabraConSpec*)spec.spec)->Compute_LinFmWt(this); }

  void	Compute_WtFmLin()  { ((LeabraConSpec*)spec.spec)->Compute_WtFmLin(this); }

  void	C_InitWtState_Post(Connection* cn, Unit* ru, Unit* su) { 
    ((LeabraConSpec*)spec.spec)->C_InitWtState_Post(this, cn, ru, su);
  }
  // hook for setting other weight-like values after initializing the weight value

  void	SetCurLrate(int epoch, LeabraTrial* trl) { ((LeabraConSpec*)spec.spec)->SetCurLrate(epoch, trl); }

  inline void 	Send_ClampNet(LeabraUnit* su)
  { ((LeabraConSpec*)spec.spec)->Send_ClampNet(this, su); }

  inline void 	Send_NetDelta(LeabraUnit* su)
  { ((LeabraConSpec*)spec.spec)->Send_NetDelta(this, su); }

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

class ActRegSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activity regulation via weight adjustment
public:
  bool		on;		// whether to activity regulation is on (active) or not
  float		min;		// #CONDEDIT_ON_on:true #DEF_0 increase weights for units below this level of average activation
  float		max;		// #CONDEDIT_ON_on:true #DEF_0.35 decrease weights for units above this level of average activation 
  float		wt_dt;		// #CONDEDIT_ON_on:true #DEF_0.2 pre-lrate rate constant for making weight changes to rectify over-activation (dwt = cur_lrate * wt_dt * wt)

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
    NOISY_LINEAR,		// simple linear output function (still thesholded) convolved with Gaussian noise (noise is nvar)
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
  ActRegSpec	act_reg;	// activity regulation via global scaling of weight values
  NoiseType	noise_type;	// where to add random noise in the processing (if at all)
  Random	noise;		// #CONDEDIT_OFF_noise_type:NO_NOISE distribution parameters for random added noise
  Schedule	noise_sched;	// #CONDEDIT_OFF_noise_type:NO_NOISE schedule of noise variance over settling cycles

  FunLookup	nxx1_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT convolved gaussian and x/x+1 function as lookup table
  FunLookup	noise_conv;	// #HIDDEN #NO_SAVE #NO_INHERIT gaussian for convolution

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
  // force units to external values provided by environment: also optimizes settling by only computing netinput once
  virtual void	Compute_HardClampNoClip(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // hard-clamp units without clipping values to clamp_range (use for second plus phase clamping to settled values)
  virtual bool	Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // soft-clamps unit, returns true if unit is not above .5

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual float	Compute_IThresh(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // compute inhibitory value that would place unit directly at threshold
  virtual float	Compute_IThreshNoAH(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // compute inhibitory value that would place unit directly at threshold, excluding any gc.a, gc.h currents

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
  void		Compute_HardClampNoClip(LeabraLayer* lay, LeabraTrial* trl) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_HardClampNoClip(this, lay, trl); }
  bool		Compute_SoftClamp(LeabraLayer* lay, LeabraTrial* trl) 
  { return ((LeabraUnitSpec*)spec.spec)->Compute_SoftClamp(this, lay, trl); }

  float		Compute_IThresh(LeabraLayer* lay, LeabraTrial* trl)
  { return ((LeabraUnitSpec*)spec.spec)->Compute_IThresh(this, lay, trl); }
  float		Compute_IThreshNoAH(LeabraLayer* lay, LeabraTrial* trl)
  { return ((LeabraUnitSpec*)spec.spec)->Compute_IThreshNoAH(this, lay, trl); }

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
  bool		diff_act_pct;	// #DEF_false if true, use different actual percent activity for overall layer activation
  float		act_pct;	// #CONDEDIT_ON_diff_act_pct:true actual percent activity to put in kwta.pct field of layer

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

class LeabraLayerSpec : public LayerSpec {
  // Leabra layer specs, computes inhibitory input for all units in layer
public:
  enum Compute_I {		// how to compute the inhibition
    KWTA_INHIB,			// between thresholds of k and k+1th most activated units (sets precise k value, should use i_kwta_pt = .25 std)
    KWTA_AVG_INHIB,		// average of top k vs avg of rest (provides more flexibility in actual k value, should use i_kwta_pt = .6 std)
    AVG_MAX_PT_INHIB,		// put inhib value at i_kwta_pt between avg and max values for layer!
    UNIT_INHIB			// unit-based inhibition (g_i from netinput -- requires connections with inhib flag set to provide inhibition)
  };

  enum InhibGroup {
    ENTIRE_LAYER,		// treat entire layer as one inhibitory group (even if subgroups exist)
    UNIT_GROUPS,		// treat sub unit groups as separate inhibitory groups
    LAY_AND_GPS,		// compute inhib over both groups and whole layer, inhibi is max of layer and group inhib
    GPS_THEN_UNITS		// first find top gp_kwta.k most active (on average) groups, then find top kwta.k units within those groups (else all units inhibbed)
  };

  KWTASpec	kwta;		// #CONDEDIT_OFF_inhib_group:UNIT_GROUPS desired activity level over entire layer (NOTE: used to set target activity for UNIT_INHIB, AVG_MAX_PT_INHIB, but not used for actually computing inhib for these cases)
  KWTASpec	gp_kwta;	// #CONDEDIT_OFF_inhib_group:ENTIRE_LAYER desired activity level for units within unit groups (not for ENTIRE_LAYER) (NOTE: used to set target activity for UNIT_INHIB, AVG_MAX_PT_INHIB, but not used for actually computing inhib for these cases)
  InhibGroup	inhib_group;	// what to consider the inhibitory group (layer or unit subgroups, or both)
  Compute_I	compute_i;	// how to compute inhibition (g_i): two forms of kwta or unit-level inhibition
  float		i_kwta_pt;	// [Default: .25 for KWTA_INHIB, .6 for KWTA_AVG, .2 for AVG_MAX_PT_INHIB] point to place inhibition between k and k+1 (or avg and max for AVG_MAX_PT_INHIB)
  float		gp_i_pt;	// #CONDEDIT_ON_compute_i:AVG_MAX_PT_INHIB [Default: .2] for unit groups: point to place inhibition between avg and max for AVG_MAX_PT_INHIB
  AdaptISpec	adapt_i;	// #AKA_adapt_pt adapt the inhibition: either i_kwta_pt point based on diffs between actual and target k level (for avg-based), or g_bar.i for unit-inhib
  ClampSpec	clamp;		// how to clamp external inputs to units (hard vs. soft)
  DecaySpec	decay;		// decay of activity state vars between events, -/+ phase, and 2nd set of phases (if appl)

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
  virtual void	Compute_HardClampPhase2(LeabraLayer* lay, LeabraTrial* trl);
  // prior to settling: hard-clamp inputs (special code for hard clamping in phase 2 based on prior acts)
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
  virtual void	Compute_Inhib_AvgMaxPt(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  // implementation of avg-max-pt inhibition computation
  virtual void	Compute_Inhib_kWTA_Gps(LeabraLayer* lay, LeabraTrial* trl);
  // implementation of GPS_THEN_UNITS kwta on groups

  ////// Stage 3.5: second pass of inhibition to do averaging

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

  virtual LeabraLayer* FindLayerFmSpec(LeabraLayer* lay, int& prjn_idx, TypeDef* layer_spec);
  // find a layer that given layer receives from based on the type of layer spec
  virtual LeabraLayer* FindLayerFmSpecExact(LeabraLayer* lay, int& prjn_idx, TypeDef* layer_spec);
  // find a layer that given layer receives from based on the type of layer spec: uses exact type match, not inherits!
  static  LeabraLayer* FindLayerFmSpecNet(Network* net, TypeDef* layer_spec);
  // find a layer in network based on the type of layer spec

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

class LeabraInhib {
  // holds threshold-computation values, used as a parent class for layers, etc
public:
  LeabraSort 	active_buf;	// #HIDDEN #NO_SAVE list of active units
  LeabraSort 	inact_buf;	// #HIDDEN #NO_SAVE list of inactive units

  AvgMaxVals	netin;		// #READ_ONLY net input values for the layer
  AvgMaxVals	i_thrs;		// #READ_ONLY inhibitory threshold values for the layer
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
  float		stm_gain;	// actual stim gain for soft clamping, can be incremented to ensure clamped units active
  bool		hard_clamped;	// this layer is actually hard clamped
  int		prv_phase;	// #READ_ONLY previous phase value (needed for 2nd plus phases and the like)
  float		dav;		// #READ_ONLY dopamine-like modulatory value (where applicable)
  int		da_updt;	// #READ_ONLY true if da triggered an update (either + to store or - reset)
  int_Array	misc_iar;	// #HIDDEN misc int array of data

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
  int		misc_state1;	// second miscellaneous state variable 
  int		misc_state2;	// third miscellaneous state variable 

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

void LeabraConSpec::C_Send_Inhib(LeabraCon_Group*, LeabraCon* cn, LeabraUnit* ru, float su_act_eff) {
  ru->gc.i += su_act_eff * cn->wt;
}
void LeabraConSpec::Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su) {
  // apply scale based only on first unit in con group: saves lots of redundant mulitplies!
  // LeabraUnitSpec::CheckConfig checks that this is ok.
  Unit* ru = cg->Un(0);
  float su_act_eff = ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act;
  CON_GROUP_LOOP(cg, C_Send_Inhib(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_eff));
}

void LeabraConSpec::C_Send_Net(LeabraCon_Group*, LeabraCon* cn, Unit* ru, float su_act_eff) {
  ru->net += su_act_eff * cn->wt;
}
void LeabraConSpec::Send_Net(Con_Group* cg, Unit* su) {
  // apply scale based only on first unit in con group: saves lots of redundant mulitplies!
  // LeabraUnitSpec::CheckConfig checks that this is ok.
  Unit* ru = cg->Un(0);
  float su_act_eff = ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act;
  if(inhib)
    CON_GROUP_LOOP(cg, C_Send_Inhib((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_eff));
  else {
    CON_GROUP_LOOP(cg, C_Send_Net((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), cg->Un(i), su_act_eff));
  }
}

///////////////////

void LeabraConSpec::C_Send_InhibDelta(LeabraCon_Group*, LeabraCon* cn, LeabraUnit* ru, float su_act_delta_eff) {
  ru->g_i_delta += su_act_delta_eff * cn->wt;
}
void LeabraConSpec::Send_InhibDelta(LeabraCon_Group* cg, LeabraUnit* su) {
  Unit* ru = cg->Un(0);
  float su_act_delta_eff = ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta;
  CON_GROUP_LOOP(cg, C_Send_InhibDelta(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_delta_eff));
}

void LeabraConSpec::C_Send_NetDelta(LeabraCon_Group*, LeabraCon* cn, LeabraUnit* ru, float su_act_delta_eff) {
  ru->net_delta += su_act_delta_eff * cn->wt;
}

void LeabraConSpec::Send_NetDelta(LeabraCon_Group* cg, LeabraUnit* su) {
  Unit* ru = cg->Un(0);
  float su_act_delta_eff = ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta;
  if(inhib)
    CON_GROUP_LOOP(cg, C_Send_InhibDelta(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_delta_eff));
  else {
    CON_GROUP_LOOP(cg, C_Send_NetDelta(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_delta_eff));
  }
}

///////////////////

void LeabraConSpec::C_Send_ClampNet(LeabraCon_Group*, LeabraCon* cn, LeabraUnit* ru, float su_act_eff) {
  ru->clmp_net += su_act_eff * cn->wt;
}
void LeabraConSpec::Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su) {
  Unit* ru = cg->Un(0);
  float su_act_eff = ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act;
  CON_GROUP_LOOP(cg, C_Send_ClampNet(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su_act_eff));
}

//////////////////////////
//     Computing dWt 	//
//////////////////////////

inline void LeabraConSpec::Compute_SAvgCor(LeabraCon_Group* cg, LeabraUnit*) {
  LeabraLayer* fm = (LeabraLayer*)cg->prjn->from;
  float savg = .5f + savg_cor.cor * (fm->kwta.pct - .5f);
  savg = MAX(savg_cor.thresh, savg); // keep this computed value within bounds
  cg->savg_cor = .5f / savg;
}

inline float LeabraConSpec::C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg,
					   float ru_act, float su_act)
{
  // wt is negative in linear form, so using opposite sign of usual here
  return ru_act * (su_act * (cg->savg_cor + cn->wt) +
		   (1.0f - su_act) * cn->wt);
}

// generec error term with sigmoid activation function, and soft bounding
inline float LeabraConSpec::C_Compute_Err
(LeabraCon* cn, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
  float err = (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
  // wt is negative in linear form, so using opposite sign of usual here
  if(lmix.err_sb) {
    if(err > 0.0f)	err *= (1.0f + cn->wt);
    else		err *= -cn->wt;	
  }
  return err;
}

// combine hebbian and error-driven
inline void LeabraConSpec::C_Compute_dWt(LeabraCon* cn, LeabraUnit*, float heb, float err) {
  float dwt = lmix.err * err + lmix.hebb * heb;
  cn->dwt += cur_lrate * dwt;
}

inline void LeabraConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraUnit* lru = (LeabraUnit*)ru;
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
	C_Compute_dWt(cn, lru, 
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
      }
    }
  }
}

inline void LeabraConSpec::C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg,
					   LeabraUnit*, LeabraUnit*, LeabraUnitSpec*)
{
  // no act reg!
  if(cn->dwt != 0.0f) {
    C_Compute_LinFmWt(cg, cn);	// go to linear weights
    cn->wt -= cn->dwt; // wt is now negative in linear form -- signs are reversed!
    // note: this fun sets 0-1 limits automatically!
    C_Compute_WtFmLin(cg, cn);	// go back to nonlinear weights
    // then put in real limits!!
    if(cn->wt < wt_limits.min) cn->wt = wt_limits.min;
    if(cn->wt > wt_limits.max) cn->wt = wt_limits.max;
  }
  cn->pdw = cn->dwt;
  cn->dwt = 0.0f;
}

inline void LeabraConSpec::C_Compute_ActReg(LeabraCon* cn, LeabraCon_Group*,
					    LeabraUnit* ru, LeabraUnit*, LeabraUnitSpec* rus)
{
  // do this in update so inactive units can be reached (no opt_thresh.updt)
  // act_reg.on:
  float dwinc = 0.0f;
  if(ru->act_avg < rus->act_reg.min)
    dwinc = rus->act_reg.wt_dt;
  else if(ru->act_avg > rus->act_reg.max)
    dwinc = -rus->act_reg.wt_dt;
  if(dwinc != 0.0f) {
    float wtval = wt_sig_fun_inv.Eval(cn->wt);
    cn->dwt += cur_lrate * dwinc * wtval; // weight is + !
  }
}

inline void LeabraConSpec::C_UpdateWeightsActReg(LeabraCon* cn, LeabraCon_Group* cg,
						 LeabraUnit* ru, LeabraUnit* su, LeabraUnitSpec* rus)
{
  C_Compute_ActReg(cn, cg, ru, su, rus);
  C_UpdateWeights(cn, cg, ru, su, rus);
}

inline void LeabraConSpec::UpdateWeights(Con_Group* cg, Unit* ru) {
  LeabraUnitSpec* rus = (LeabraUnitSpec*)ru->spec.spec;
  LeabraCon_Group* lcg = (LeabraCon_Group*)cg;
  if(rus->act_reg.on) {		// do this in update so inactive units can be reached (no opt_thresh.updt)
    CON_GROUP_LOOP(cg, C_UpdateWeightsActReg((LeabraCon*)cg->Cn(i), lcg,
					     (LeabraUnit*)ru, (LeabraUnit*)cg->Un(i), rus));
  }
  else {
    CON_GROUP_LOOP(cg, C_UpdateWeights((LeabraCon*)cg->Cn(i), lcg,
				       (LeabraUnit*)ru, (LeabraUnit*)cg->Un(i), rus));
  }
//  ApplyLimits(cg, ru); limits are automatically enforced anyway
}

inline void LeabraConSpec::B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
  float err = ru->act_p - ru->act_m;
  cn->dwt += cur_lrate * err;
}
  
// default is not to do anything tricky with the bias weights
inline void LeabraConSpec::B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus) {
  if(rus->act_reg.on) {		// do this in update so inactive units can be reached (no opt_thresh.updt)
    if(ru->act_avg < rus->act_reg.min)
      cn->dwt += cur_lrate * rus->act_reg.wt_dt;
    else if(ru->act_avg > rus->act_reg.max)
      cn->dwt -= cur_lrate * rus->act_reg.wt_dt;
  }
  cn->pdw = cn->dwt;
  cn->wt += cn->dwt;
  cn->dwt = 0.0f;
  C_ApplyLimits(cn, ru, NULL);
}

inline void LeabraBiasSpec::B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
  float err = ru->act_p - ru->act_m;
  if(fabsf(err) >= dwt_thresh)
    cn->dwt += cur_lrate * err;
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
    PLUS_PHASE = 1,		// plus phase
    MINUS_2 = 2,		// second minus phase
    PLUS_2 = 3			// second plus phase
  };

  enum PhaseOrder {
    MINUS_PLUS,			// standard minus-plus (err and assoc)
    PLUS_ONLY,			// only present the plus phase (hebbian-only)
    MINUS_PLUS_NOTHING,		// auto-encoder version with final 'nothing' minus phase
    PLUS_NOTHING,		// just the auto-encoder (no initial minus phase)
    MINUS_PLUS_PLUS,		// two plus phases for gated context layer updating
    MINUS_PLUS_2		// two minus-plus phases (for pfc/bg system)
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

class LeabraGoodStat : public Stat {
  // ##COMPUTE_IN_TrialProcess constraint satisfaction goodness statistic
public:
  bool		subtr_inhib;	// subtract inhibition from harmony?
  StatVal	hrmny;		// harmony = act * netin = a_i sum_j a_j w_ij
  StatVal	strss;		// stress = act * log(act)
  StatVal	gdnss;		// goodness = harmony + stress

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

class LeabraSharpStat : public Stat {
  // ##COMPUTE_IN_TrialProcess layer sharpness statistic: just max / avg
public:
  StatVal	sharp;		// sharpness = max / avg 

  void		InitStat();
  void		Init();
  bool		Crit();

  void		RecvCon_Run(Unit*)	{ }; // don't do these!
  void		SendCon_Run(Unit*)	{ };

  void		Unit_Run(Layer*)	{ }; // no unit stat
  void		Layer_Stat(Layer*);

  void		Network_Init();

  void	Initialize();
  void	Destroy();
  void	Copy_(const LeabraSharpStat& cp);
  COPY_FUNS(LeabraSharpStat, Stat);
  TA_BASEFUNS(LeabraSharpStat);
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

class LeabraPrjnRelNetinStat : public Stat {
  // ##COMPUTE_IN_TrialProcess computes overall relative netinput contributions for the different projections into a layer. Useful for setting wt_scale parameters to achieve desired relative contributions of different inputs.  you MUST set the layer parameter to the layer in question
public:
  StatVal_List	relnet;		// relative netinput contributions for the different projections into units in this layer
  float		recv_act_thr;	// #DEF_0.1 only compute netinput for receiving units that are active above this threshold (prevents dilution by varying numbers of inactive units)

  void		InitStat();
  void		Init();
  bool		Crit();

  void		RecvCon_Run(Unit*)	{ }; // don't do these!
  void		SendCon_Run(Unit*)	{ };
  void		Unit_Run(Layer*)	{ }; // no unit stat

  void		Network_Init();

  void		NameStatVals();
  void		Layer_Stat(Layer*);

  virtual String GetPrjnNm(const char* prjn_nm);

  void	UpdateAfterEdit();

  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	Copy_(const LeabraPrjnRelNetinStat& cp);
  COPY_FUNS(LeabraPrjnRelNetinStat, Stat);
  TA_BASEFUNS(LeabraPrjnRelNetinStat);
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
// 	Misc Special Objects		//
//////////////////////////////////////////

class LeabraLinUnitSpec : public LeabraUnitSpec {
  // a pure linear unit (suitable for an AC unit spec unit)
public:
  void 	Compute_ActFmVm(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);
  
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(LeabraLinUnitSpec);
};

class LeabraNegBiasSpec : public LeabraBiasSpec {
  // only learns negative bias changes, not positive ones (decay restores back to zero)
public:
  float		decay;		// rate of weight decay towards zero 
  bool		updt_immed;	// update weights immediately when weights are changed

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru, LeabraUnitSpec* rus) {
    if(rus->act_reg.on) {		// do this in update so inactive units can be reached (no opt_thresh.updt)
      if(ru->act_avg < rus->act_reg.min)
	cn->dwt += cur_lrate * rus->act_reg.wt_dt;
      else if(ru->act_avg > rus->act_reg.max)
	cn->dwt -= cur_lrate * rus->act_reg.wt_dt;
    }
    if(cn->dwt > 0.0f)		// positive only
      cn->dwt = 0.0f;
    cn->dwt -= decay * cn->wt;
    cn->pdw = cn->dwt;
    cn->wt += cn->dwt;
    cn->dwt = 0.0f;
    C_ApplyLimits(cn, ru, NULL);
  }

  inline void	B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
    LeabraBiasSpec::B_Compute_dWt(cn, ru);
    if(updt_immed) B_UpdateWeights(cn, ru, (LeabraUnitSpec*)ru->spec.spec);
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraNegBiasSpec);
  COPY_FUNS(LeabraNegBiasSpec, LeabraBiasSpec);
  TA_BASEFUNS(LeabraNegBiasSpec);
};

class TrialSynDepCon : public LeabraCon {
  // synaptic depression connection at the trial level (as opposed to cycle level)
public:
  float		effwt;		// #NO_SAVE effective weight value (can be depressed) -- used for sending ativation

  void 	Initialize()		{ effwt = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const TrialSynDepCon& cp) { effwt = cp.effwt; }
  COPY_FUNS(TrialSynDepCon, LeabraCon);
  TA_BASEFUNS(TrialSynDepCon);
};

class SynDepSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for synaptic depression
public:
  float		rec;		// #DEF_1 rate of recovery from depression
  float		depl;		// #DEF_1.1 rate of depletion of synaptic efficacy as a function of sender-receiver activations

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(SynDepSpec);
  COPY_FUNS(SynDepSpec, taBase);
  TA_BASEFUNS(SynDepSpec);
};

class TrialSynDepConSpec : public LeabraConSpec {
  // synaptic depression connection at the trial level (as opposed to cycle level)
public:
  SynDepSpec	syn_dep;	// synaptic depression specifications

  void C_Depress_Wt(TrialSynDepCon* cn, LeabraUnit*, LeabraUnit* su) {
    // NOTE: fctn of sender act and wt: could be just sender or sender*recv.. 
    float deff = syn_dep.rec * (cn->wt - cn->effwt) - syn_dep.depl * su->act_eq * cn->wt;
    cn->effwt += deff;
    if(cn->effwt > cn->wt) cn->effwt = cn->wt;
    if(cn->effwt < wt_limits.min) cn->effwt = wt_limits.min;
  }
  virtual void Depress_Wt(LeabraCon_Group* cg, LeabraUnit* ru) {
    CON_GROUP_LOOP(cg, C_Depress_Wt((TrialSynDepCon*)cg->Cn(i), ru, (LeabraUnit*)cg->Un(i)));
  }

  void Compute_dWt(Con_Group* cg, Unit* ru) {
    LeabraUnit* lru = (LeabraUnit*)ru;
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
	  C_Compute_dWt(cn, lru, 
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	  // depression operates on nonlinear weight!
	  C_Depress_Wt((TrialSynDepCon*)cn, lru, su);
	}
      }
    }
  }

  void C_Reset_EffWt(TrialSynDepCon* cn) {
    cn->effwt = cn->wt;
  }
  virtual void Reset_EffWt(LeabraCon_Group* cg) {
    CON_GROUP_LOOP(cg, C_Reset_EffWt((TrialSynDepCon*)cg->Cn(i)));
  }

  void 	C_InitWtState_Post(Con_Group*, Connection* cn, Unit*, Unit*) {
    TrialSynDepCon* lcn = (TrialSynDepCon*)cn; lcn->effwt = lcn->wt;
  }

  float C_Compute_Net(TrialSynDepCon* cn, Unit*, Unit* su) {
    return cn->effwt * su->act;
  }
  float Compute_Net(Con_Group* cg, Unit* ru) {
    float rval=0.0f;
    CON_GROUP_LOOP(cg, rval += C_Compute_Net((TrialSynDepCon*)cg->Cn(i), ru, cg->Un(i)));
    return ((LeabraCon_Group*)cg)->scale_eff * rval;
  }
  void C_Send_Inhib(LeabraCon_Group* cg, TrialSynDepCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    ru->gc.i += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->effwt;
  }
  void Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su) {
    CON_GROUP_LOOP(cg, C_Send_Inhib(cg, (TrialSynDepCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
  }
  void C_Send_Net(LeabraCon_Group* cg, TrialSynDepCon* cn, Unit* ru, Unit* su) {
    ru->net += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->effwt;
  }
  void Send_Net(Con_Group* cg, Unit* su) {
    if(inhib)
      Send_Inhib((LeabraCon_Group*)cg, (LeabraUnit*)su);
    else {
      CON_GROUP_LOOP(cg, C_Send_Net((LeabraCon_Group*)cg, (TrialSynDepCon*)cg->Cn(i), cg->Un(i), su));
    }
  }
  void C_Send_InhibDelta(LeabraCon_Group* cg, TrialSynDepCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    ru->g_i_delta += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta * cn->effwt;
  }
  void Send_InhibDelta(LeabraCon_Group* cg, LeabraUnit* su) {
    CON_GROUP_LOOP(cg, C_Send_InhibDelta(cg, (TrialSynDepCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
  }
  void C_Send_NetDelta(LeabraCon_Group* cg, TrialSynDepCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    ru->net_delta += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act_delta * cn->effwt;
  }
  void Send_NetDelta(LeabraCon_Group* cg, LeabraUnit* su) {
    if(inhib)
      Send_InhibDelta(cg, su);
    else {
      CON_GROUP_LOOP(cg, C_Send_NetDelta(cg, (TrialSynDepCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
    }
  }
  void C_Send_ClampNet(LeabraCon_Group* cg, TrialSynDepCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    ru->clmp_net += ((LeabraCon_Group*)ru->recv.FastGp(cg->other_idx))->scale_eff * su->act * cn->effwt;
  }
  void Send_ClampNet(LeabraCon_Group* cg, LeabraUnit* su) {
    CON_GROUP_LOOP(cg, C_Send_ClampNet(cg, (TrialSynDepCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
  }

  void 	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(TrialSynDepConSpec);
  COPY_FUNS(TrialSynDepConSpec, LeabraConSpec);
  TA_BASEFUNS(TrialSynDepConSpec);
};

class FastWtCon : public LeabraCon {
  // fast weight connection: standard wt learns fast, but decays toward slow weight value
public:
  float		swt;		// slow weight value
  float		sdwt;		// #NO_SAVE slow weight delta-weight change

  void 	Initialize()		{ swt = sdwt = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const FastWtCon& cp) { swt = cp.swt;  sdwt = cp.sdwt; }
  COPY_FUNS(FastWtCon, LeabraCon);
  TA_BASEFUNS(FastWtCon);
};

class FastWtSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER specificiations for fast weights
public:
  enum DecayMode {
    ALWAYS,			// always decay all weights toward slow weight (swt)
    SU_PROP,			// decay in proportion to the sending unit activation: decay only happens after sending activity dissipates
    SU_THR			// decay only weights with sending unit activations below sending threshold (nom .1): decay only happens after sending activity dissipates
  };

  float		lrate;		// learning rate
  bool		use_lrs;	// #DEF_false use learning rate schedule to modify cur_lrate learning rate?
  float		cur_lrate;	// #READ_ONLY #NO_INHERIT #SHOW current learning rate with lrate schedule factored in
  float		decay;		// rate of decay toward the slow weight values
  bool		slw_sat;	// #DEF_true does fast weight contribute to saturation of slow weights?
  DecayMode	dk_mode;	// how to apply the decay of fast weights back to the slow weight (swt) value

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(FastWtSpec);
  COPY_FUNS(FastWtSpec, taBase);
  TA_BASEFUNS(FastWtSpec);
};

// TODO: the decay in this code cannot be parallelized over dwt's because the dynamics will
// be different!!!
// need to have a dmem small_batch over sequences type mode that does a SyncWts using sum_dwts = false
// and calls UpdateWeights after each trial..

// TODO: following code is not dealing with contrast enhancement on the swt vals!!!!

class FastWtConSpec : public LeabraConSpec {
  // fast weight connection: standard wt learns fast, but decays toward slow weight value
public:
  FastWtSpec	fast_wt;	// fast weight specs: fast weights are added in separately to overall weight value as an increment (

  void 		C_InitWtState_Post(Con_Group*, Connection* cn, Unit*, Unit*) {
    FastWtCon* lcn = (FastWtCon*)cn; lcn->swt = lcn->wt;
  }

  void 		C_InitWtDelta(Con_Group* cg, Connection* cn, Unit* ru, Unit* su)
  { ConSpec::C_InitWtDelta(cg, cn, ru, su); ((FastWtCon*)cn)->sdwt=0.0f; }

  void SetCurLrate(int epoch, LeabraTrial* trl);

  inline float C_Compute_SlowHebb(FastWtCon* cn, LeabraCon_Group* cg,
				  float ru_act, float su_act)
  {
    // swt is always stored positive, so signs are different.
    float swt_eff = (fast_wt.slw_sat) ? -cn->wt : cn->swt;
    return ru_act * (su_act * (cg->savg_cor - swt_eff) - (1.0f - su_act) * swt_eff);
  }

  // todo: somewhat inefficient to do this computation twice..

  // generec error term with sigmoid activation function, and soft bounding
  inline float C_Compute_SlowErr(FastWtCon* cn, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
    float err = (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
    float swt_eff = (fast_wt.slw_sat) ? -cn->wt : cn->swt;
    if(err > 0.0f)	err *= (1.0f - swt_eff);
    else		err *= swt_eff;	
    return err;
  }

  inline void C_Compute_SlowdWt(FastWtCon* cn, LeabraUnit*, float heb, float err) {
    float sdwt = lmix.err * err + lmix.hebb * heb;
    cn->sdwt += cur_lrate * sdwt; // standard lrate is slow one..
  }

  // combine hebbian and error-driven
  inline void C_Compute_dWt(LeabraCon* cn, LeabraUnit*, float heb, float err) {
    float dwt = lmix.err * err + lmix.hebb * heb;
    cn->dwt += fast_wt.cur_lrate * dwt;	// the regular weight is the fast one!
  }

  inline void C_Compute_FastDecay(FastWtCon* cn, LeabraUnit*, LeabraUnit* su) {
    if((fast_wt.dk_mode == FastWtSpec::SU_THR) && (su->act_p > .1f)) return;
    if(fast_wt.dk_mode == FastWtSpec::SU_PROP)
      cn->wt += (1.0f - su->act_p) * fast_wt.decay * (-cn->swt - cn->wt); // decay toward slow weights..
    else
      cn->wt += fast_wt.decay * (-cn->swt - cn->wt); // decay toward slow weights..
  }

  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    LeabraUnit* lru = (LeabraUnit*)ru;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if(((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) {
      for(int i=0; i<cg->size; i++) {
	LeabraUnit* su = (LeabraUnit*)cg->Un(i);
	FastWtCon* cn = (FastWtCon*)cg->Cn(i);
	if(!(su->in_subgp &&
	     (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	  C_Compute_FastDecay(cn, lru, su);
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get weight into linear form
	  C_Compute_dWt(cn, lru, 
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	  C_Compute_SlowdWt(cn, lru, 
			C_Compute_SlowHebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_SlowErr(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
  }

  inline void C_UpdateWeights(FastWtCon* cn, LeabraCon_Group* cg,
			      LeabraUnit*, LeabraUnit*, LeabraUnitSpec*)
  {
    if(cn->sdwt != 0.0f) {
      cn->swt += cn->sdwt; // wt is not negative!
      if(cn->swt < wt_limits.min) cn->swt = wt_limits.min;
      if(cn->swt > wt_limits.max) cn->swt = wt_limits.max;
    }
    if(cn->dwt != 0.0f) {
      // always do this because of the decay term..
      C_Compute_LinFmWt(cg, cn);	// go to linear weights
      cn->wt -= cn->dwt; // wt is now negative in linear form -- signs are reversed!
      cn->wt = MAX(cn->wt, -1.0f);	// limit 0-1
      cn->wt = MIN(cn->wt, 0.0f);
      C_Compute_WtFmLin(cg, cn);	// go back to nonlinear weights
      // then put in real limits!!
      if(cn->wt < wt_limits.min) cn->wt = wt_limits.min;
      if(cn->wt > wt_limits.max) cn->wt = wt_limits.max;
    }
    cn->pdw = cn->dwt;
    cn->dwt = 0.0f;
    cn->sdwt = 0.0f;
  }

  inline void C_UpdateWeightsActReg(FastWtCon* cn, LeabraCon_Group* cg,
				    LeabraUnit* ru, LeabraUnit* su, LeabraUnitSpec* rus)
  {
    C_Compute_ActReg(cn, cg, ru, su, rus);
    C_UpdateWeights(cn, cg, ru, su, rus);
  }

  inline void UpdateWeights(Con_Group* cg, Unit* ru) {
    LeabraUnitSpec* rus = (LeabraUnitSpec*)ru->spec.spec;
    LeabraCon_Group* lcg = (LeabraCon_Group*)cg;
    if(rus->act_reg.on) {		// do this in update so inactive units can be reached (no opt_thresh.updt)
      CON_GROUP_LOOP(cg, C_UpdateWeightsActReg((FastWtCon*)cg->Cn(i), lcg,
					       (LeabraUnit*)ru, (LeabraUnit*)cg->Un(i), rus));
    }
    else {
      CON_GROUP_LOOP(cg, C_UpdateWeights((FastWtCon*)cg->Cn(i), lcg,
					 (LeabraUnit*)ru, (LeabraUnit*)cg->Un(i), rus));
    }
    //  ApplyLimits(cg, ru); limits are automatically enforced anyway
  }

  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(FastWtConSpec);
  COPY_FUNS(FastWtConSpec, LeabraConSpec);
  TA_BASEFUNS(FastWtConSpec);
};

//////////////////////////////////
// 	Scalar Value Layer	//
//////////////////////////////////

// misc todo: items for SUM_BAR:
// to do equivalent of "clamp_value" (e.g., PV units at end of settle), add a special
// mode where asymptotic Vm is computed based on current params, and act from that, etc.

// also possible: unit subgroups that all have the same gc.i value, but different random
// connectivity from inputs: sending units all send to a fixed # (permute) of these 
// group units, producing a random sensory representation.  not really necc. for S2 
// spikes, because of syndep..

class ScalarValSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for scalar values
public:
  enum	RepType {
    GAUSSIAN,			// gaussian bump, with value = weighted average of tuned unit values
    LOCALIST,			// each unit represents a distinct value; intermediate values represented by graded activity of neighbors; overall activity is weighted-average across all units
    SUM_BAR,			// bar of increasing activation threshold across units, with value = sum of overall activation.  This has more natural activation dynamics than GAUSSIAN
  };

  RepType	rep;		// type of representation of scalar value to use
  float		un_width;	// #CONDEDIT_ON_rep:GAUSSIAN sigma parameter of a gaussian specifying the tuning width of the coarse-coded units (in min-max units)
  float		min_net;	// #CONDEDIT_ON_rep:SUM_BAR #DEF_0.1 minimum netinput value for purposes of computing gc.i in sum_bar
  bool		clamp_pat;	// #DEF_false if true, environment provides full set of values to clamp over entire layer (instead of providing single scalar value to clamp on 1st unit, which then generates a corresponding distributed pattern)
  float		min_sum_act;	// #DEF_0.2 minimum total activity of all the units representing a value: when computing weighted average value, this is used as a minimum for the sum that you divide by
  bool		val_mult_lrn;	// #DEF_false for learning, effectively multiply the learning rate by the minus-plus phase difference in overall represented value (i.e., if overall value is the same, no learning takes place)
  float		min;		// #READ_ONLY #NO_SAVE #NO_INHERIT minimum unit value
  float		range;		// #READ_ONLY #NO_SAVE #NO_INHERIT range of unit values
  float		val;		// #READ_ONLY #NO_SAVE #NO_INHERIT current val being represented (implementational, computed in InitVal())
  float		incr;		// #READ_ONLY #NO_SAVE #NO_INHERIT increment per unit (implementational, computed in InitVal())
  int		sb_lt;		// #READ_ONLY #NO_SAVE #NO_INHERIT sum_bar lower threshold index (implementational, computed in InitVal())
  float		sb_ev;		// #READ_ONLY #NO_SAVE #NO_INHERIT sum_bar 'edge' unit value (implementational, computed in InitVal())

  virtual void	InitVal(float sval, int ugp_size, float umin, float urng);
  // initialize implementational values for subsequently computing GetUnitAct to represent scalar val sval over unit group of ugp_size
  virtual float	GetUnitAct(int unit_idx);
  // get activation under current representation for unit at given index: MUST CALL InitVal first!
  virtual float	GetUnitVal(int unit_idx);
  // get target value associated with unit at given index: MUST CALL InitVal first!

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ScalarValSpec);
  COPY_FUNS(ScalarValSpec, taBase);
  TA_BASEFUNS(ScalarValSpec);
};

class ScalarValBias : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER initial bias for given activation value for scalar value units
public:
  enum UnitBias {		// bias on individual units
    NO_UN,			// no unit bias
    GC,				// bias value enters as a conductance in gc.h or gc.a
    BWT				// bias value enters as a bias.wt
  };

  enum BiasShape {		// shape of the bias pattern
    VAL,			// bias as a particular value representation
    NEG_SLP,			// bias as an increasingly negative slope (gc.a or -bwt) as unit values increase
    POS_SLP			// bias as an increasingly positive slope (gc.h or +bwt) as unit values increase
  };

  enum WeightBias {		// bias on weights into units
    NO_WT,			// no weight bias
    WT				// input weights
  };

  UnitBias	un;		// bias on individual units
  BiasShape	un_shp;		// shape of unit bias
  float		un_gain;	// #CONDEDIT_OFF_un:NO_UN #DEF_1 gain multiplier (strength) of bias to apply for units.  WT = .03 as basic weight multiplier
  WeightBias	wt;		// bias on weights: always uses a val-shaped bias
  float		wt_gain;	// #CONDEDIT_OFF_wt:NO_WT #DEF_1 gain multiplier (strength) of bias to apply for weights (gain 1 = .03 wt value)
  float		val;		// value location (center of gaussian bump)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ScalarValBias);
  COPY_FUNS(ScalarValBias, taBase);
  TA_BASEFUNS(ScalarValBias);
};

class ScalarValLayerSpec : public LeabraLayerSpec {
  // represents a scalar value using a coarse-coded distributed code over units.  first unit represents scalar value.
public:
  ScalarValSpec	 scalar;	// specifies how values are represented in terms of distributed patterns of activation across the layer
  MinMaxRange	 unit_range;	// range of values represented across the units; for GAUSSIAN, add extra values above and below true useful range to prevent edge effects.
  ScalarValBias	 bias_val;	// specifies bias for given value (as gaussian bump) 
  MinMaxRange	 val_range;	// #READ_ONLY #NO_INHERIT actual range of values (scalar.min/max taking into account un_range)

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

  virtual void	Compute_WtBias_Val(Unit_Group* ugp, float val);
  virtual void	Compute_UnBias_Val(Unit_Group* ugp, float val);
  virtual void	Compute_UnBias_NegSlp(Unit_Group* ugp);
  virtual void	Compute_UnBias_PosSlp(Unit_Group* ugp);
  virtual void	Compute_BiasVal(LeabraLayer* lay);
  // initialize the bias value 

  void 	InitWtState(LeabraLayer* lay);
  void	Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  // don't include first unit in any of these computations!
  void 	Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);

  virtual void 	Compute_dWtUgp(Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl);
  // compute weight changes just for one unit group
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  virtual void	ReConfig(Network* net, int n_units = -1);
  // #BUTTON reconfigure layer and associated specs for current scalar.rep type; if n_units > 0, changes number of units in layer to specified value

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


//////////////////////////////////
// 	TwoD Value Layer	//
//////////////////////////////////

class TwoDValSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for two-dimensional values
public:
  enum	RepType {
    GAUSSIAN,			// gaussian bump, with value = weighted average of tuned unit values
    LOCALIST			// each unit represents a distinct value; intermediate values represented by graded activity of neighbors; overall activity is weighted-average across all units
  };

  RepType	rep;		// type of representation of scalar value to use
  int		n_vals;		// number of values to represent in layer: layer geom.x must be >= 2 * n_vals because vals are represented in first row of layer!
  float		un_width;	// #CONDEDIT_ON_rep:GAUSSIAN sigma parameter of a gaussian specifying the tuning width of the coarse-coded units (in min-max units)
  bool		clamp_pat;	// #DEF_false if true, environment provides full set of values to clamp over entire layer (instead of providing single scalar value to clamp on 1st unit, which then generates a corresponding distributed pattern)
  float		min_sum_act;	// #DEF_0.2 minimum total activity of all the units representing a value: when computing weighted average value, this is used as a minimum for the sum that you divide by
  float		mn_dst;		// #DEF_0.5 minimum distance factor for reading out multiple bumps: must be at least this times un_width far away from other bumps

  float		x_min;		// #READ_ONLY #NO_SAVE #NO_INHERIT minimum unit value
  float		x_range;	// #READ_ONLY #NO_SAVE #NO_INHERIT range of unit values
  float		y_min;		// #READ_ONLY #NO_SAVE #NO_INHERIT minimum unit value
  float		y_range;	// #READ_ONLY #NO_SAVE #NO_INHERIT range of unit values
  float		x_val;		// #READ_ONLY #NO_SAVE #NO_INHERIT current val being represented (implementational, computed in InitVal())
  float		y_val;		// #READ_ONLY #NO_SAVE #NO_INHERIT current val being represented (implementational, computed in InitVal())
  float		x_incr;		// #READ_ONLY #NO_SAVE #NO_INHERIT increment per unit (implementational, computed in InitVal())
  float		y_incr;		// #READ_ONLY #NO_SAVE #NO_INHERIT increment per unit (implementational, computed in InitVal())
  int		x_size;		// #READ_ONLY #NO_SAVE #NO_INHERIT size of axis
  int		y_size;		// #READ_ONLY #NO_SAVE #NO_INHERIT size of axis

  virtual void	InitVal(float xval, float yval, int xsize, int ysize, float xmin, float xrng, float ymin, float yrng);
  // initialize implementational values for subsequently computing GetUnitAct to represent scalar val sval over unit group of ugp_size
  virtual float	GetUnitAct(int unit_idx);
  // get activation under current representation for unit at given index: MUST CALL InitVal first!
  virtual void	GetUnitVal(int unit_idx, float& x_cur, float& y_cur);
  // get target values associated with unit at given index: MUST CALL InitVal first!

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(TwoDValSpec);
  COPY_FUNS(TwoDValSpec, taBase);
  TA_BASEFUNS(TwoDValSpec);
};

class TwoDValBias : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER initial bias for given activation value for scalar value units
public:
  enum UnitBias {		// bias on individual units
    NO_UN,			// no unit bias
    GC,				// bias value enters as a conductance in gc.h or gc.a
    BWT				// bias value enters as a bias.wt
  };

  enum WeightBias {		// bias on weights into units
    NO_WT,			// no weight bias
    WT				// input weights
  };

  UnitBias	un;		// bias on individual units
  float		un_gain;	// #CONDEDIT_OFF_un:NO_UN #DEF_1 gain multiplier (strength) of bias to apply for units.  WT = .03 as basic weight multiplier
  WeightBias	wt;		// bias on weights: always uses a val-shaped bias
  float		wt_gain;	// #CONDEDIT_OFF_wt:NO_WT #DEF_1 gain multiplier (strength) of bias to apply for weights (gain 1 = .03 wt value)
  float		x_val;		// X axis value location (center of gaussian bump)
  float		y_val;		// Y axis value location (center of gaussian bump)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(TwoDValBias);
  COPY_FUNS(TwoDValBias, taBase);
  TA_BASEFUNS(TwoDValBias);
};

class TwoDValLayerSpec : public LeabraLayerSpec {
  // represents one or more two-d value(s) using a coarse-coded distributed code over units.  first row represents scalar value(s).  one val readout is weighted-average; multiple vals = max bumps over 3x3 local grid
public:
  TwoDValSpec	 twod;		// specifies how values are represented in terms of distributed patterns of activation across the layer
  MinMaxRange	 x_range;	// range of values represented across the X (horizontal) axis; for GAUSSIAN, add extra values above and below true useful range to prevent edge effects.
  MinMaxRange	 y_range;	// range of values represented across the Y (vertical) axis; for GAUSSIAN, add extra values above and below true useful range to prevent edge effects.
  TwoDValBias	 bias_val;	// specifies bias values
  MinMaxRange	 x_val_range;	// #READ_ONLY #NO_INHERIT actual range of values (scalar.min/max taking into account un_range)
  MinMaxRange	 y_val_range;	// #READ_ONLY #NO_INHERIT actual range of values (scalar.min/max taking into account un_range)

  virtual void	ClampValue(Unit_Group* ugp, LeabraTrial* trl, float rescale=1.0f);
  // clamp value in the first unit's ext field to the units in the group
  virtual void	ReadValue(Unit_Group* ugp, LeabraTrial* trl);
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

  virtual void	Compute_WtBias_Val(Unit_Group* ugp, float x_val, float y_val);
  virtual void	Compute_UnBias_Val(Unit_Group* ugp, float x_val, float y_val);
  virtual void	Compute_BiasVal(LeabraLayer* lay);
  // initialize the bias value 

  void 	InitWtState(LeabraLayer* lay);
  void	Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  // don't include first unit in any of these computations!
  void 	Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);
  void 	Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*);

  virtual void 	Compute_dWtUgp(Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl);
  // compute weight changes just for one unit group
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  virtual void	ReConfig(Network* net, int n_units = -1);
  // #BUTTON reconfigure layer and associated specs for current scalar.rep type; if n_units > 0, changes number of units in layer to specified value

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(TwoDValLayerSpec);
  COPY_FUNS(TwoDValLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(TwoDValLayerSpec);
};

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// 	Reinforcement Learning Algorithms (TD/PVLV/BG/PFC)	//
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

class DaModUnit : public LeabraUnit {
  // Leabra unit with dopamine-like modulation of minus phase activation for learning
public:
  float		act_m2;		// second minus phase activation
  float		act_p2;		// second plus phase activation
  float 	p_act_m;	// previous minus phase activation 
  float		p_act_p;	// previous plus phase activation
  float 	dav;		// modulatory dopamine value 

  void	Initialize();
  void	Destroy()	{ };
  void	Copy_(const DaModUnit& cp);
  COPY_FUNS(DaModUnit, LeabraUnit);
  TA_BASEFUNS(DaModUnit);
};

class DaModSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for effects of da-based modulation: plus-phase = learning effects
public:
  enum ModType {
    PLUS_CONT,			// da modulates plus-phase activations (only) in a continuous manner
    PLUS_POST,			// da modulates plus-phase activations (only), at the end of the plus phase
    NEG_DIP			// da dips provide a (decaying) negative (accomodation) current on a trial-wise basis
  };

  bool		on;		// whether to actually modulate activations by da values
  ModType	mod;		// #CONDEDIT_ON_on:true how to apply DA modulation
  float		gain;		// #CONDEDIT_ON_on:true gain multiplier of da values
  float		neg_rec;	// #CONDEDIT_ON_mod:NEG_DIP recovery time constant for NEG_DIP case (how much to decay negative current per trial)
  bool		p_dwt;		// whether units learn based on prior activation states, as in TD and other algorithms (not really to do with DA modulation; just stuck here.. affects Compute_dWt and UpdateWeights calls)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(DaModSpec);
  COPY_FUNS(DaModSpec, taBase);
  TA_BASEFUNS(DaModSpec);
};

class DaModUnitSpec : public LeabraUnitSpec {
  // Leabra unit with temporal-differences error modulation of minus phase activation for learning
public:
  DaModSpec	da_mod;		// da modulation of activations (for da-based learning, and other effects)

  void 		Compute_Conduct(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl);

  void		InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ LeabraUnitSpec::InitState(u); }
  void		Compute_dWt(Unit*) { };
  void 		Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void 		UpdateWeights(Unit* u);
  void 		EncodeState(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  void		DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay);
  void		PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			   LeabraTrial* trl, bool set_both=false);

  bool  CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet=false);

  void	Defaults();
  void	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(DaModUnitSpec);
  COPY_FUNS(DaModUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(DaModUnitSpec);
};

//////////////////////////////////////////
//	External Reward Layer		//
//////////////////////////////////////////

class AvgExtRewSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for computing average external rewards
public:
  bool		sub_avg;	// #DEF_false subtract average reward value in computing rewards
  float		avg_dt;		// #DEF_0.005 time constant for integrating average reward value

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(AvgExtRewSpec);
  COPY_FUNS(AvgExtRewSpec, taBase);
  TA_BASEFUNS(AvgExtRewSpec);
};

class OutErrSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for computing external rewards based on output performance of network
public:
  bool		graded;		// #DEF_false compute a graded reward signal as a function of number of correct output values
  bool		no_off_err;	// #DEF_false do not count a unit wrong if it is off but target says on -- only count wrong units that are on but should be off
  bool		seq_all_cor;	// #DEF_false require that all RewTarg events in a sequence be correct before giving reward (on the last event in sequence);  if graded is checked, this reward is a graded function of % correct

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(OutErrSpec);
  COPY_FUNS(OutErrSpec, taBase);
  TA_BASEFUNS(OutErrSpec);
};

class ExtRewSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for computing external rewards
public:
  float		err_val;	// #DEF_0 reward value for errors (when network does not respond correctly)
  float		norew_val;	// #DEF_0.5 reward value when no feedback information is present
  float		rew_val;	// #DEF_1 reward value for correct responses (positive rewards)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(ExtRewSpec);
  COPY_FUNS(ExtRewSpec, taBase);
  TA_BASEFUNS(ExtRewSpec);
};

class ExtRewLayerSpec : public ScalarValLayerSpec {
  // computes external rewards: minus phase is zero, plus phase is reward value derived from network performance or other inputs (computed at start of 1+)
public:
  enum RewardType {		// how do we get the reward values?
    OUT_ERR_REW,		// get rewards as a function of errors on the output layer ONLY WHEN RewTarg layer act > .5 -- get from markerconspec from output layer(s)
    EXT_REW,			// get rewards as external inputs marked as ext_flag = TARG to the first unit in the layer
    DA_REW			// get rewards from da values on first unit in layer
  };

  RewardType	rew_type;	// how do we get the reward values?
  AvgExtRewSpec	avg_rew;	// average reward computation specifications
  OutErrSpec	out_err;	// #CONDEDIT_ON_rew_type:OUT_ERR_REW how to compute external rewards based on output performance
  ExtRewSpec	rew;		// misc reward computation specifications

  virtual bool	OutErrRewAvail(LeabraLayer* lay, LeabraTrial* trl);
  // figure out if reward is available on this trial (look if target signals are present)
  virtual float	GetOutErrRew(LeabraLayer* lay, LeabraTrial* trl);
  // get reward value based on error at layer with MarkerConSpec connection: 1 = rew (correct), 0 = err, -1 = no info
  virtual void 	Compute_OutErrRew(LeabraLayer* lay, LeabraTrial* trl);
  // get reward value based on external error (put in da val, clamp)
  virtual void 	Compute_ExtRew(LeabraLayer* lay, LeabraTrial* trl);
  // get external rewards from inputs (put in da val, clamp)
  virtual void 	Compute_DaRew(LeabraLayer* lay, LeabraTrial* trl);
  // clamp external rewards as da values (put in da val, clamp)
  virtual void 	Compute_UnitDa(float er, DaModUnit* u, Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl);
  // actually compute the unit da value based on external reward value er
  virtual void	Compute_ZeroAct(LeabraLayer* lay, LeabraTrial* trl);
  // clamp zero activations, for minus phase
  virtual void	Compute_NoRewAct(LeabraLayer* lay, LeabraTrial* trl);
  // clamp norew_val values for when no reward information is present

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(ExtRewLayerSpec);
  COPY_FUNS(ExtRewLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(ExtRewLayerSpec);
};

class ExtRew_Stat : public Stat {
  // ##COMPUTE_IN_TrialProcess ##FINAL_STAT external reward statistic
public:
  StatVal	rew;		// external reward value

  void	RecvCon_Run(Unit*)	{ }; // don't do these!
  void	SendCon_Run(Unit*)	{ };

  void	InitStat();
  void	Init();
  bool	Crit();
  void	Network_Run();	// do everything here

  void 	ComputeAggregates(); // special aggregation to only count when data avail!

  void	NameStatVals();
  
  void	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(ExtRew_Stat);
  COPY_FUNS(ExtRew_Stat, Stat);
  TA_BASEFUNS(ExtRew_Stat);
};

//////////////////////////////////////////////////////////
// 	Standard TD Reinforcement Learning 		//
//////////////////////////////////////////////////////////

class TDRewPredConSpec : public LeabraConSpec {
  // Reward Prediction connections: for TD RewPred Layer, uses TD algorithm for predicting rewards
public:
  inline float C_Compute_Err(LeabraCon* cn, DaModUnit* ru, DaModUnit* su) {
    float err = (ru->act_p - ru->act_m) * su->p_act_p;
    // wt is negative in linear form, so using opposite sign of usual here
    if(lmix.err_sb) {
      if(err > 0.0f)	err *= (1.0f + cn->wt);
      else		err *= -cn->wt;	
    }
    return err;
  }

  // this computes weight changes based on sender at time t-1
  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    DaModUnit* lru = (DaModUnit*)ru;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if(lru->p_act_p >= 0.0f) {
      for(int i=0; i<lcg->size; i++) {
	DaModUnit* su = (DaModUnit*)lcg->Un(i);
	LeabraCon* cn = (LeabraCon*)lcg->Cn(i);
	float orig_wt = cn->wt;
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	C_Compute_dWt(cn, lru, 
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->p_act_p),
		      C_Compute_Err(cn, lru, su));  
	cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
      }
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(TDRewPredConSpec);
  COPY_FUNS(TDRewPredConSpec, LeabraConSpec);
  TA_BASEFUNS(TDRewPredConSpec);
};

//////////////////////////////////////////////////
//	TD Reward Prediction Layer		//
//////////////////////////////////////////////////

class TDRewPredLayerSpec : public ScalarValLayerSpec {
  // predicts rewards: minus phase = clamped prior expected reward V^(t), plus = settles on expectation of future reward V^(t+1)
public:
  virtual void 	Compute_SavePred(Unit_Group* ugp, LeabraTrial* trl); // save current prediction to misc_1 for later clamping
  virtual void 	Compute_ClampPred(Unit_Group* ugp, LeabraTrial* trl); // clamp misc_1 to ext 
  virtual void 	Compute_ClampPrev(LeabraLayer* lay, LeabraTrial* trl);
  // clamp minus phase to previous act value
  virtual void 	Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial* trl);
  // copy ext values to act_p
  virtual void 	Compute_TdPlusPhase_impl(Unit_Group* ugp, LeabraTrial* trl);
  virtual void 	Compute_TdPlusPhase(LeabraLayer* lay, LeabraTrial* trl);
  // compute plus phase activations for learning including the td values

  void	InitState(LeabraLayer* lay);
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
  SIMPLE_COPY(TDRewPredLayerSpec);
  COPY_FUNS(TDRewPredLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(TDRewPredLayerSpec);
};

//////////////////////////////////////////
//	TD Reward Integration Layer	//
//////////////////////////////////////////

class TDRewIntegSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER misc specs for TDRewIntegLayerSpec
public:
  float		discount;	// discount factor for V(t+1) from TDRewPredLayer

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(TDRewIntegSpec);
  COPY_FUNS(TDRewIntegSpec, taBase);
  TA_BASEFUNS(TDRewIntegSpec);
};

class TDRewIntegLayerSpec : public ScalarValLayerSpec {
  // integrates perceived and external rewards: delta over phases = DA td-like signal. minus phase = prev exp rew V^(t), plus phase = extrew (r) + tdrewpred computing V(t+1)
public:
  TDRewIntegSpec	rew_integ;	// misc specs for TDRewIntegLayerSpec

  void 	Compute_Act(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(TDRewIntegLayerSpec);
  COPY_FUNS(TDRewIntegLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(TDRewIntegLayerSpec);
};

//////////////////////////
//	  TdLayer 	//
//////////////////////////

class TdLayerSpec : public LeabraLayerSpec {
  // computes activation = temporal derivative (act_eq - act_m) of sending units in plus phases: note, act will go negative!
public:
  virtual void	Compute_ZeroAct(LeabraLayer* lay, LeabraTrial* trl);
  // compute a zero td value: in minus phase
  virtual void	Compute_Td(LeabraLayer* lay, LeabraTrial* trl);
  // compute the td value based on recv projections: every cycle in 1+ phases
  virtual void	Send_Td(LeabraLayer* lay, LeabraTrial* trl);
  // send the td value to sending projections: every cycle

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void 	Compute_Act(LeabraLayer* lay, LeabraTrial* trl);

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

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//	Pavlovian (PVLV): Perceived Value and Local Value Reward Learning System	//
//////////////////////////////////////////////////////////////////////////////////////////

// LV local value: learns continuously about external rewards (present or absent)
// PV perceives value: learns only at the time of external (expected) rewards, free to fire at time CS's come on
// PVLVDa (VTA/SNc) computes DA signal as: (ER - LV) + (PV - baserate)
// 	(baserate is basal PV value comparison to maintain DA at target value)

// todo: da in minus phase = cur - del; this should modulate learning in *prior*
// time step.  plus phase da modulates minus phase acts.  2nd plus modulates plus phase, etc

//////////////////////////////////////////
//	LV: Local Value			//
//////////////////////////////////////////

// TODO: the syndep in this code cannot be parallelized over dwt's because the dynamics will
// be different!!!
// need to have a dmem small_batch over sequences type mode that does a SyncWts using sum_dwts = false
// and calls UpdateWeights after each trial..
// problem is that this does averaging of dwts;  one soln is to keep the
// orig wt from last wt change, apply dwt every trial, and then subtract wt - origwt ->dwt
// aggretate these dwts as a SUM, apply to wts!

class LVConSpec : public LeabraConSpec {
  // local value connection spec
public:
  inline float C_Compute_Err(LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    float err = (ru->act_p - ru->act_m) * su->act_p;
    // wt is negative in linear form, so using opposite sign of usual here
    if(lmix.err_sb) {
      if(err > 0.0f)	err *= (1.0f + cn->wt);
      else		err *= -cn->wt;	
    }
    return err;
  }

  // this computes weight changes based on sender at time t-1
  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    LeabraUnit* lru = (LeabraUnit*)ru;
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
	  C_Compute_dWt(cn, lru, 
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_Err(cn, lru, su));  
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	}
      }
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LVConSpec);
  COPY_FUNS(LVConSpec, LeabraConSpec);
  TA_BASEFUNS(LVConSpec);
};

class LVSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for learning values
public:
  float		avg_dt;		// #DEF_0.005 time constant for integrating average LV value (in minus phase)
  float		avg_init;	// #DEF_0.5 initial average LV value (use 0 for ExtRew norew val = 0)
  bool		use_pt;		// #DEF_false use thr_avg_pt to put threshold for saying cur trial as an ER between avg LV val for hits (positive rewards) and others (no info)
  float		thr_avg_pt;	// #CONDEDIT_ON_use_pt:true #DEF_0.6 how close to put threshold between average LV values for hit and other (allows floating threshold to adapt to actual LV values)
  float		thr_min;	// #DEF_0.2 minimum threshold on LV reward expectation magnitude required for saying ER (.25 or .3 for use_pt, 0 for !use_pt and ExtRew norew = 0)
  float		thr_max;	// #DEF_0.8 maximum threshold on LV reward expectation magnitude required for saying ER (use .75 for ExtRew norew val = .5, else use .4 for norew = 0)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(LVSpec);
  COPY_FUNS(LVSpec, taBase);
  TA_BASEFUNS(LVSpec);
};

class LVLayerSpec : public ScalarValLayerSpec {
  // local value (LV): continously learns about external reward
public:
  LVSpec	lv;	// local value specs

  virtual float	Compute_ExtRew(LeabraLayer* lay, LeabraTrial* trl, bool& actual_er_avail, bool& lv_est_er_avail);
  // get external reward from ExtRew layer if it is available: actual_er_avail = internal flag from ExtRew layer (cheating), lv_est_er_avail = LV layer's estimate of ER availability based on current ER values vs. LV values (use this for bio authenticity)
  virtual void 	Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial* trl);
  // copy ext values to act_p
  virtual void 	Compute_LVPlusPhaseDwt(LeabraLayer* lay, LeabraTrial* trl);
  // compute plus phase activations as external rewards and change weights

  void	InitWtState(LeabraLayer* lay);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(LVLayerSpec);
  COPY_FUNS(LVLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(LVLayerSpec);
};

//////////////////////////////////////////////////////////
//	Perceived Value Learning Layer: ABL Amygdala	//
//////////////////////////////////////////////////////////

class PVConSpec : public TrialSynDepConSpec {
  // perceived value layer: generalizes to stimuli associated with reward
public:
  inline float C_Compute_Err(LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su) {
    float err = (ru->act_p - ru->act_m) * su->act_p;
    // wt is negative in linear form, so using opposite sign of usual here
    if(lmix.err_sb) {
      if(err > 0.0f)	err *= (1.0f + cn->wt);
      else		err *= -cn->wt;	
    }
    return err;
  }

  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    LeabraUnit* lru = (LeabraUnit*)ru;
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
	  C_Compute_dWt(cn, lru, 
			C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
			C_Compute_Err(cn, lru, su));  
	  cn->wt = orig_wt; // restore original value; note: no need to convert there-and-back for dwt, saves numerical lossage!
	  // depression operates on linear weight!
	  C_Depress_Wt((TrialSynDepCon*)cn, lru, su);
	}
      }
    }
  }

  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(PVConSpec);
  COPY_FUNS(PVConSpec, TrialSynDepConSpec);
  TA_BASEFUNS(PVConSpec);
};

class PVSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for perceived value learning
public:
  enum	LearnMode {
    ER,				// use the external reward value for the training signal
    DA_LV,			// use DA value from LV - ER (ONLY) for training PV: does not include the PV system's own contribution to the DA signal & prevents weird self-feedback effects
    DA				// use global DA value: does include PV system's own contribution to DA, which can cause weird self-feedback effects
  };

  LearnMode	lrn_mode;	// #DEF_ER learning mode: what signal to use to train PV signal (does not determine *when* learning occur)
//   bool		gc_mod;		// #CONDEDIT_OFF_lrn_mode:ER #DEF_false implement learning as a modulation of the gc.h, .a currents instead of clamping value
  float		discount;	// #CONDEDIT_ON_lrn_mode:ER #DEF_0 multiplicative discount factor for external reward (ER) training value: clamp = (1-discount)*ER
  float		da_gain;	// #CONDEDIT_OFF_lrn_mode:ER,ER_LV #DEF_1.5 gain on the learning da signal (also used in gc_mod mode)
  bool		use_er;		// #DEF_false use actual external reward to determine when to learn (cheating), otherwise use LV's estimate of when ER is avail (more realistic)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(PVSpec);
  COPY_FUNS(PVSpec, taBase);
  TA_BASEFUNS(PVSpec);
};

class PVLayerSpec : public ScalarValLayerSpec {
  // learns to perceive value based on inputs that are associated with rewards (fast-learning PV system)
public:
  PVSpec	pv;	// perceived value learning specs

  virtual void 	Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial* trl);
  // copy ext values to act_p
  virtual void 	Compute_DepressWt(Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl);
  // depress weights for units in unit group (only if not doing dwts!)
  virtual void 	Compute_PVPlusPhaseDwt(LeabraLayer* lay, LeabraTrial* trl);
  // compute plus phase activations for learning, and actually change weights if da is above threshold

//   virtual void 	Compute_DaLearnMod(LeabraLayer* lay, Unit_Group* ugp, LeabraInhib* thr, LeabraTrial* trl);
  // compute dynamic da modulation: evaluation modulation
//   void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);

  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(PVLayerSpec);
  COPY_FUNS(PVLayerSpec, ScalarValLayerSpec);
  TA_BASEFUNS(PVLayerSpec);
};

class PVSLayerSpec : public PVLayerSpec {
  // slow version of PV layer spec: learns to perceive value (just a marker for layer; same functionality as PVLayerSpec)
public:

  void 	Initialize()		{ };
  void	Destroy()		{ };
  TA_BASEFUNS(PVSLayerSpec);
};

//////////////////////////
//	  DaLayer 	//
//////////////////////////

class PVLVDaSpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specs for PVLV da parameters
public:
  enum	DaMode {
    PV_IF_ER_LV,		// da = (PVf - PVs) + [if (ext_rew expected or present), ER - LV]
    IF_ER_LV,			// if (ext_rew expected or present), da = ER - LV, else da = PVf - PVs
    LV_PLUS_PV			// da = (ER - LV) + (PVf - PVs)
  };

  DaMode	mode;		// #DEF_PV_IF_ER_LV how to compute da as a function of LV and PV systems
  float		tonic_da;	// #DEF_0 set a tonic 'dopamine' (DA) level (offset to add to da values)
  float		min_pv_s;	// #DEF_0.1 minimum PVslow value, so that a low PVfast value (~0) makes a negative da: DA_pv = PVfast - MAX(PVslow, min_pv_s)
  bool		use_er;		// #DEF_false use actual ext rew avail signal (i.e., cheating); else use the LV activity to determine if ER is avail

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(PVLVDaSpec);
  COPY_FUNS(PVLVDaSpec, taBase);
  TA_BASEFUNS(PVLVDaSpec);
};

class PVLVDaLayerSpec : public LeabraLayerSpec {
  // computes PVLV 'Da' signal: typically if(ER), da = ER-LV, else PVf - PVs
public:
  PVLVDaSpec	da;		// parameters for the pvlv da computation

  virtual void	Compute_ZeroAct(LeabraLayer* lay, LeabraTrial* trl);
  // compute a zero da value: in minus phase
  virtual void	Compute_Da(LeabraLayer* lay, LeabraTrial* trl);
  // compute the da value based on recv projections: every cycle in 1+ phases
  virtual void	Send_Da(LeabraLayer* lay, LeabraTrial* trl);
  // send the da value to sending projections: every cycle

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void 	Compute_Act(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_dWt(LeabraLayer*, LeabraTrial*);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(PVLVDaLayerSpec);
  COPY_FUNS(PVLVDaLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(PVLVDaLayerSpec);
};

////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// 	BG-based PFC Gating/RL learning Mechanism		//
//////////////////////////////////////////////////////////////////

//////////////////////////////////
//	  Patch/SNc	 	//
//////////////////////////////////

class PatchLayerSpec : public PVLayerSpec {
  // simulates Patch as a PV layer: currently no doing anything different than regular PV
public:

  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(PatchLayerSpec);
};

class SNcMiscSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER misc parameters for SNc layer
public:
  enum PatchMode {
    NO_PATCH,			// no patch at all
    PATCH			// use patch: Note currently not supported.
  };

  PatchMode	patch_mode;	// #DEF_NO_PATCH how to run the patch computation
  float		patch_gain;	// #CONDEDIT_ON_patch_mode:PATCH #DEF_0.5 proportion of patch (stripe-specific) da relative to global abl da

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SNcMiscSpec);
  COPY_FUNS(SNcMiscSpec, taBase);
  TA_BASEFUNS(SNcMiscSpec);
};

class SNcLayerSpec : public PVLVDaLayerSpec {
  // computes PVLV 'Da' signal, uses inputs from the patch to provide stripe-specific modulation (NOTE: not currently supported); Does not do SNrThal modulation, which is computed directly in Matrix units
public:
  SNcMiscSpec	snc;		// misc SNc specific parameters controlling influence of patch modulation

  void	Compute_Da(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void 	InitLinks();
  SIMPLE_COPY(SNcLayerSpec);
  COPY_FUNS(SNcLayerSpec, PVLVDaLayerSpec);
  TA_BASEFUNS(SNcLayerSpec);
};

//////////////////////////////////
//	  Matrix Con/Units	//
//////////////////////////////////

class MatrixConSpec : public LeabraConSpec {
  // Learning of matrix input connections based on temporal-differences modulation of activation: da happens one time step later!
public:
  enum LearnRule {
    MOTOR_DELTA,		// delta rule for BG_motor: (bg+ - bg-) * s-
    MOTOR_CHL,			// CHL rule for BG_motor: (bg+ * s+) - (bg- * s-)
    PFC   			// PFC learning rule: (bg_p2 - bg_p) * s_p
  };

  LearnRule	learn_rule;	// learning rule to use
  bool		pfc_hebb_nego;	// only allow negative dwts for pfc hebb learning

  inline float C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg, DaModUnit* ru, DaModUnit* su) {
    // wt is negative in linear form, so using opposite sign of usual here
    float rval;
    if((learn_rule == MOTOR_DELTA) || (learn_rule == MOTOR_DELTA))
      rval = ru->act_p * (su->act_p * (cg->savg_cor + cn->wt) + (1.0f - su->act_p) * cn->wt);
    else
      rval = ru->act_p2 * (su->act_p * (cg->savg_cor + cn->wt) + (1.0f - su->act_p) * cn->wt);
    if(pfc_hebb_nego && (rval > 0.0f)) rval = 0.0f;
    return rval;
  }

  inline float C_Compute_Err(LeabraCon* cn, DaModUnit* ru, DaModUnit* su) {
    float err = 0.0f;
    switch(learn_rule) {
    case MOTOR_DELTA:
      err = (ru->act_p  - ru->act_m) * su->act_m;
      break;
    case MOTOR_CHL:
      err = (ru->act_p * su->act_p) - (ru->act_m * su->act_m);
      break;
    case PFC:
      err = (ru->act_p2  - ru->act_p) * su->act_p;
      break;
    }
    // wt is negative in linear form, so using opposite sign of usual here
    if(lmix.err_sb) {
      if(err > 0.0f)	err *= (1.0f + cn->wt);
      else		err *= -cn->wt;	
    }
    return err;
  }

  inline void Compute_dWt(Con_Group* cg, Unit* ru) {
    DaModUnit* lru = (DaModUnit*)ru;
    LeabraCon_Group* lcg = (LeabraCon_Group*) cg;
    Compute_SAvgCor(lcg, lru);
    if(((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) {
      for(int i=0; i<lcg->size; i++) {
	DaModUnit* su = (DaModUnit*)lcg->Un(i);
	LeabraCon* cn = (LeabraCon*)lcg->Cn(i);
	if(!(su->in_subgp &&
	     (((LeabraUnit_Group*)su->owner)->acts.avg < savg_cor.thresh))) {
	  float orig_wt = cn->wt;
	  C_Compute_LinFmWt(lcg, cn); // get into linear form
	  C_Compute_dWt(cn, lru, 
			C_Compute_Hebb(cn, lcg, lru, su), 
			C_Compute_Err(cn, lru, su));
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

class MatrixBiasSpec : public LeabraBiasSpec {
  // noise bias connection -- holds noise value separate from orig value
public:
  enum LearnRule {
    MOTOR_DELTA,		// delta rule for BG_motor: (bg+ - bg-) * s-
    MOTOR_CHL,			// CHL rule for BG_motor: (bg+ * s+) - (bg- * s-)
    PFC   			// PFC: learn on 2p - p
  };

  LearnRule	learn_rule;	// learning rule to use

  void B_Compute_dWt(LeabraCon* cn, LeabraUnit* ru) {
    DaModUnit* dau = (DaModUnit*)ru;
    float err;
    if(learn_rule == PFC)
      err = dau->act_p2 - dau->act_p;
    else
      err = ru->act_p - ru->act_m;
    if(fabsf(err) >= dwt_thresh)
      cn->dwt += cur_lrate * err;
  }
  
  void 	Initialize();
  void	Destroy()		{ };
  TA_BASEFUNS(MatrixBiasSpec);
};

class MatrixUnitSpec : public DaModUnitSpec {
  // basal ganglia matrix units: fire actions or WM updates. modulated by da signals
public:
  bool	freeze_net;		// #DEF_true freeze netinput (PFC in 2+ phase, MOTOR in 1+ phase) during learning modulation so that learning only reflects DA modulation and not other changes in netin

  void 	Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* trl, LeabraTrial* trl);
  void	PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
		   LeabraTrial* trl, bool set_both=false);

  void	Defaults();

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(MatrixUnitSpec);
  COPY_FUNS(MatrixUnitSpec, DaModUnitSpec);
  TA_BASEFUNS(MatrixUnitSpec);
};

//////////////////////////////////
//	  Matrix Layer Spec	//
//////////////////////////////////

class ContrastSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER contrast enhancement of the GO units
public:
  float		gain;		// #DEF_1 overall gain for da modulation
  float		go_p;		// #DEF_0.5 proportion of da * gate_act for DA+ on GO units: contrast enhancement
  float		go_n;		// #DEF_0.5 proportion of da * gate_act for DA- on GO units: contrast reduction
  float		nogo_p;		// #DEF_0.5 proportion of da * gate_act for DA+ on NOGO units: contrast enhancement
  float		nogo_n;		// #DEF_0.5 proportion of da * gate_act for DA- on NOGO units: contrast reduction

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ContrastSpec);
  COPY_FUNS(ContrastSpec, taBase);
  TA_BASEFUNS(ContrastSpec);
};

class MatrixRndGoSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER matrix random GO firing to encourage exploration for non-productive stripes based on avg_go_da for that stripe (matrix_u->misc_1)
public:
  bool		on;		// #DEF_true whether to use random go firing function or not (if not, over max_mnt Go firing is still used)
  float		go_p;		// #CONDEDIT_ON_on:true #DEF_0.1 probability of actually firing a random Go when all the conditions have been met
  float		rgo_da;		// #CONDEDIT_ON_on:true #DEF_20 strength of DA for activating Go (gc.h) and inhibiting NoGo (gc.a) when max_mnt is up or a random Go is active (applied during learning, after Go/NoGo already computed)
  float		prf_go_p;	// #CONDEDIT_ON_on:true #DEF_1e-04 uniform random performance-only Go firing (not conditional on thresholds; happens randomly; does not cause learning)
  float		prf_rgo_da;	// #CONDEDIT_ON_on:true #DEF_1 strength of DA for activating Go (gc.h) and inhibiting NoGo (gc.a) on a purely random basis (prf_go_p)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(MatrixRndGoSpec);
  COPY_FUNS(MatrixRndGoSpec, taBase);
  TA_BASEFUNS(MatrixRndGoSpec);
};

class MatrixRndGoThrSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER thresholds for matrix random Go firing to encourage exploration for non-productive stripes based on avg_go_da for that stripe (matrix_u->misc_1)
public:
  float		abs_thr;	// #DEF_0 absolute threshold on per stripe avg_go_da value (-1..1) for triggering random GO (if stripe_agd < abs_thr then Go p(go_p))
  float		abs_max;	// #DEF_0.1 absolute maximum on per stripe avg_go_da value (-1..1) above which no random GO can occur (if stripe_agd > abs_max then never Go)
  float		rel;		// #DEF_0.05 relative threshold on per stripe avg_go_da value (-1..1) for triggering random GO (if (stripe_agd - avg_agd) < -agd_rel then Go p(go_p))
  int		mnt;		// #DEF_10 minimum maintenance duration (trials of sustained maintenance/off) for a stripe before a random GO will fire
  float		avgrew;		// #DEF_0.9 threshold on global avg reward value (0..1) below which random GO can fire (uses ExtRew_Stat if avail, else avg value from ExtRewLayer)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(MatrixRndGoThrSpec);
  COPY_FUNS(MatrixRndGoThrSpec, taBase);
  TA_BASEFUNS(MatrixRndGoThrSpec);
};

class MatrixMiscSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER misc specs for the matrix layer
public:
  float		neg_da_bl;	// #DEF_0.0002 negative da baseline in learning condition: this amount subtracted from all da values in learning phase (essentially reinforces nogo)
  float		neg_gain;	// #DEF_1.5 gain for negative DA signals relative to positive ones: neg DA may need to be stronger!
  float		perf_gain;	// #DEF_0 performance effect da gain (in 2- phase for trans, 1+ for gogo)
  int		max_mnt;	// #DEF_50 maximum maintenance duration: if longer than this, NOGO is activated as per rnd_go
  float		avg_go_dt;	// #DEF_0.005 time constant for integrating the average DA value associated with Go firing for each stripe (stored in u->misc_1)
  bool		no_snr_mod;	// #DEF_false disable the Da learning modulation by SNrThal ativation (this is only to demonstrate how important it is)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(MatrixMiscSpec);
  COPY_FUNS(MatrixMiscSpec, taBase);
  TA_BASEFUNS(MatrixMiscSpec);
};

class MatrixLayerSpec : public LeabraLayerSpec {
  // basal ganglia matrix layer: fire actions/WM updates, or nogo; PFC = gate in 1+ and 2+, MOTOR = gate in -
public:
  enum 	BGType {       		// which type of basal ganglia circuit is this?
    MOTOR,			// matrix that gates motor cortex responses
    PFC				// matrix that gates pfc working memory updating
  };

  BGType		bg_type;	// type of basal ganglia/frontal system: motor or pfc
  MatrixMiscSpec 	matrix;		// misc parameters for the matrix layer
  ContrastSpec 	 	contrast;	// contrast enhancement effects of da/dopamine neuromodulation
  MatrixRndGoSpec	rnd_go;		// matrix random GO firing to encourage exploration for non-productive stripes based on avg_go_da for that stripe (matrix_u->misc_1)
  MatrixRndGoThrSpec	rnd_go_thr;	// #CONDEDIT_ON_rnd_go.on:true random Go firing thresholds: when to fire a random Go that drives learning

  virtual void 	Compute_RndGo(LeabraLayer* lay, LeabraTrial* trl);
  // compute random GO signal for stripes that have been dormant a longish time
  virtual void 	Compute_ClearRndGo(LeabraLayer* lay, LeabraTrial* trl);
  // clear the rnd go signal
  virtual void 	Compute_DaModUnit_NoContrast(DaModUnit* u, float dav, int go_no);
  // apply given dopamine modulation value to the unit, based on whether it is a go (0) or nogo (1); no contrast enancement based on activation
  virtual void 	Compute_DaModUnit_Contrast(DaModUnit* u, float dav, float gating_act, int go_no);
  // apply given dopamine modulation value to the unit, based on whether it is a go (0) or nogo (1); contrast enhancement based on activation (gating_act)
  virtual void 	Compute_DaTonicMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib* thr, LeabraTrial* trl);
  // compute tonic da modulation (for pfc gating units in first two phases)
  virtual void 	Compute_DaPerfMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib* thr, LeabraTrial* trl);
  // compute dynamic da modulation; performance modulation, not learning (second minus phase)
  virtual void 	Compute_DaLearnMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib* thr, LeabraTrial* trl);
  // compute dynamic da modulation: evaluation modulation, which is sensitive to GO/NOGO firing and activation in action phase
  virtual void 	Compute_AvgGoDa(LeabraLayer* lay, LeabraTrial* trl);
  // compute average da present when stripes fire a Go (stored in u->misc_1); used to modulate rnd_go firing

  void	InitWtState(LeabraLayer* lay);
  void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl);
  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  void	Compute_dWt(LeabraLayer* lay, LeabraTrial* trl);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(MatrixLayerSpec);
  COPY_FUNS(MatrixLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(MatrixLayerSpec);
};

//////////////////////////////////////////////////////////////////
//	  SNrThalLayer: Integrate Matrix and compute Gating 	//
//////////////////////////////////////////////////////////////////

class SNrThalLayerSpec : public LeabraLayerSpec {
  // computes activation = GO - NOGO from MatrixLayerSpec
public:
  float		avg_net_dt;	// time-averaged netinput computation integration rate

  virtual void	Compute_GoNogoNet(LeabraLayer* lay, LeabraTrial* trl);
  // compute netinput as GO - NOGO on matrix layer

  void 	Compute_Clamp_NetAvg(LeabraLayer* lay, LeabraTrial* trl);
  void	Compute_dWt(LeabraLayer*, LeabraTrial*);

  void	HelpConfig();	// #BUTTON get help message for configuring this spec
  bool  CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet=false);
  void	Defaults();

  void 	Initialize();
  void	Destroy()		{ };
  void  InitLinks();
  SIMPLE_COPY(SNrThalLayerSpec);
  COPY_FUNS(SNrThalLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(SNrThalLayerSpec);
};

//////////////////////////////////
//	PFC Layer Spec		//
//////////////////////////////////

class PFCGateSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER basal ganglia gating specifications
public:
  enum	GateSignal {
    GATE_GO = 0,		// gate GO unit fired 
    GATE_NOGO = 1,		// gate NOGO unit fired
  };
  
  enum	GateState {		// what happened on last gating action, stored in misc_state1 on unit group
    EMPTY_GO,			// stripe was empty, got a GO
    EMPTY_NOGO,			// stripe was empty, got a NOGO
    LATCH_GO,			// stripe was already latched, got a GO
    LATCH_NOGO,			// stripe was already latched, got a NOGO
    LATCH_GOGO,			// stripe was already latched, got a GO then another GO
    NO_GATE,			// no gating took place
    RANDOM_GO,			// matrix random-Go for stripes firing lots of NoGo (causes learning)
    PERF_RND_GO			// performance-only unconditional matrix random-Go 
  };

  float		go_thr;		// #DEF_0.1 threshold in snrthal activation required to trigger a GO
  float		off_accom;	// #DEF_0 how much of the maintenance current to apply to accommodation after turning a unit off
  bool		updt_reset_sd;	// #DEF_true reset synaptic depression when units are updated
  float		snr_tie_thr;	// #DEF_0.2 how high does the average netinput need to be to detect a tie-deadlock among a set of snrthal units all sending equally strong GO signals, such that the net activation is < go_thr?
  bool		snr_tie_max;	// #DEF_false choose the max active units to win in the case of a snrthal tie deadlock 

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(PFCGateSpec);
  COPY_FUNS(PFCGateSpec, taBase);
  TA_BASEFUNS(PFCGateSpec);
};

class PFCLayerSpec : public LeabraLayerSpec {
  // Prefrontal cortex layer: gets gating signal from matrix, gate updates before each plus phase (toggle off, toggle on)
public:
  enum MaintUpdtAct {
    NO_UPDT,			// no update action
    STORE,			// store current activity state in maintenance currents
    CLEAR,			// clear current activity state from maintenance currents
    RESTORE,			// restore prior maintenance currents (after transient input activation)
    TMP_STORE,			// temporary store of current activity state (for default maintenance of last state)
    TMP_CLEAR			// temporary clear of current maintenance state (for transient representation in second plus)
  };

  PFCGateSpec	gate;		// parameters controlling the gating of pfc units

  virtual void 	ResetSynDep(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // reset synaptic depression for sending cons from unit that was just toggled off in plus phase 1
  virtual void 	Compute_MaintUpdt(LeabraUnit_Group* ugp, MaintUpdtAct updt_act, LeabraLayer* lay, LeabraTrial* trl);
  // update maintenance state variables (gc.h, misc_1) based on given action
  virtual void	SendGateStates(LeabraLayer* lay, LeabraTrial* trl);
  // send misc_state gating state variables to the snrthal and matrix layers
  virtual void 	Compute_TmpClear(LeabraLayer* lay, LeabraTrial* trl);
  // temporarily clear the maintenance of pfc units to prepare way for transient acts
  virtual void 	Compute_GatingTrans(LeabraLayer* lay, LeabraTrial* trl);
  // compute the gating signal based on SNrThal layer: TRANS model
  virtual void 	Compute_GatingGOGO(LeabraLayer* lay, LeabraTrial* trl);
  // compute the gating signal based on SNrThal layer: GOGO model

  void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
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

  virtual void 	TD(Network* net, bool bio_labels = false, bool td_mod_all = false);
  // #MENU_BUTTON #MENU_SEP_BEFORE configure standard TD reinforcement learning layers; bio_labels = use biologically-based labels for layers, else functional; td_mod_all = have td value modulate all the regular units in the network

  virtual void 	PVLV(Network* net, bool bio_labels = false, bool localist_val = true, bool fm_hid_cons=true, bool fm_out_cons=false, bool da_mod_all = false);
  // #MENU_BUTTON configure PVLV (pavlovian perceived value and local value) learning layers in a network; bio_labels = use biologically-based labels for layers, else functional; localist_val = use localist value representations for pvlv layers; da_mod_all = have da value modulate all the regular units in the network

  virtual void 	BgPFC(Network* net, bool bio_labels = false, bool localist_val = true, int n_stripes=4, bool fm_hid_cons=true, bool fm_out_cons=false, bool mat_fm_pfc_full = false, bool make_patch=false, bool nolrn_pfc=false, bool da_mod_all = false, bool lr_sched = true);
  // #MENU_BUTTON #MENU_SEP_BEFORE configure all the layers and specs for doing basal-ganglia based gating of the pfc layer; bio_labels = label layers with biological, else functional, names; localist_val = use localist value representations for pvlv layers; fm_hid_cons = make cons to pfc/bg fm hidden layers; fm_out_cons = from output layers; mat_fm_pfc_full = make pfc -> matrix prjn full (else stripe-specific); patch per-stripe learning system optional; nolrn_pfc = pfc does not learn -- just copies input acts directly; da_mod_all = have da value modulate all the regular units in the network; lr_sched = make a learning rate schedule on BG learn cons

  virtual void SetPFCStripes(Network* net, int n_stripes, int n_units=-1);
  // #MENU_BUTTON set number of "stripes" (unit groups) throughout the entire set of pfc/bg layers (n_units = -1 = use current # of units)

  void 	Initialize();
  void 	Destroy()	{ };
//   SIMPLE_COPY(LeabraWiz);
//   COPY_FUNS(LeabraWiz, Wizard);
  TA_BASEFUNS(LeabraWiz);
};

#endif // leabra_h
