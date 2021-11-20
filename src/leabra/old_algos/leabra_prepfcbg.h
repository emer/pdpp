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

#include <pdp/netstru.h>
#include <pdp/sched_proc.h>
#include <ta_misc/fun_lookup.h>
#include <leabra/leabra_TA_type.h>
#include <math.h>
#include <values.h>

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
    float wrat;
    if(w <= 0.0f) wrat = MAXFLOAT;
    else if(w >= 1.0f) wrat = 0.0f;
    else wrat = off * (1.0f - w) / w;
    return 1.0f / (1.0f + powf(wrat, gain));
  }

  // function for implementing inverse of weight sigmoid
  static inline float	SigFunInv(float w, float gain, float off) {
    float wrat;
    if(w <= 0.0f) wrat = MAXFLOAT;
    else if(w >= 1.0f) wrat = 0.0f;
    else wrat = (1.0f - w) / w;
    return 1.0f / (1.0f + (1.0f / off) * powf(wrat, 1.0f / gain));
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
  float		thresh;		// #DEF_0.01 threshold of sending average activation below which learning does not occur (prevents learning when there is no input)

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
  float		cur_lrate;	// #READ_ONLY #NO_INHERIT #SHOW current actual learning rate = lrate * lrate_sched current value
  Schedule	lrate_sched;	// schedule of learning rate over training epochs (multiplies lrate!)
  LearnMixSpec	lmix;		// mixture of hebbian & err-driven learning
  FixedSAvg	fix_savg;	// how to normalize the computation of average net input: fixed sending avg act value for normalizing netin
  SAvgCorSpec	savg_cor;	// for Hebbian: correction for sending average act levels (i.e., renormalization)
  
  FunLookup	wt_sig_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT computes wt sigmoidal fun
  FunLookup	wt_sig_fun_inv;	// #HIDDEN #NO_SAVE #NO_INHERIT computes inverse of wt sigmoidal fun

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

  inline void 	C_Compute_dWt(LeabraCon* cn, float heb, float err);
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

  void	Compute_LinFmWt()  { ((LeabraConSpec*)spec.spec)->Compute_LinFmWt(this); }

  void	Compute_WtFmLin()  { ((LeabraConSpec*)spec.spec)->Compute_WtFmLin(this); }

  void	SetCurLrate(int epoch) { ((LeabraConSpec*)spec.spec)->SetCurLrate(epoch); }

  inline void 	Send_ClampNet(LeabraUnit* su);

  virtual void	EncodeState(LeabraUnit*) { };
  // encode current state information (hook for time-based learning)

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

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActFunSpec);
  COPY_FUNS(ActFunSpec, taBase);
  TA_BASEFUNS(ActFunSpec);
};

class SpikeFunSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER spiking activation function specs
public:
  int		dur;		// #DEF_3 spike duration in cycles (models duration of effect on postsynaptic neuron)
  float		v_m_r;		// #DEF_0 post-spiking membrane potential to reset to, produces refractory effect
  float		eq_gain;	// #DEF_10 gain for computing act_eq relative to actual average: act_eq = eq_gain * (spikes/cycles)
  float		ext_gain;	// #DEF_0.4 gain for clamped external inputs, mutliplies ext.  constant external inputs otherwise have too much influence compared to spiking ones

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SpikeFunSpec);
  COPY_FUNS(SpikeFunSpec, taBase);
  TA_BASEFUNS(SpikeFunSpec);
};

class OptThreshSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER optimization thresholds for faster processing
public:
  float		send;		// #DEF_0.1 don't send activation when act <= send -- greatly speeds processing
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

class LeabraUnitSpec : public UnitSpec {
  // Leabra unit specifications, point-neuron approximation
public:
  enum ActFun {
    NOISY_XX1,			// x over x plus 1 convolved with Gaussian noise (noise is nvar)
    XX1,			// x over x plus 1, hard threshold, no noise convolution
    LINEAR,			// simple linear output function (still thesholded)
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
  OptThreshSpec	opt_thresh;	// optimization thresholds for speeding up processing when units are basically inactive
  MinMaxRange	clamp_range;	// range of clamped activation values (min, max, 0, .95 std), don't clamp to 1 because acts can't reach, so .95 instead
  MinMaxRange	vm_range;	// membrane potential range (min, max, 0-1 for normalized, -90-50 for bio-based)
  Random	v_m_init;	// what to initialize the membrane potential to (mean = .15, var = 0 std)
  DtSpec	dt;		// time constants (rate of updating): membrane potential (vm) and net input (net)
  LeabraChannels g_bar;		// [Defaults: 1, .1, 1, .1, .5] maximal conductances for channels
  LeabraChannels e_rev;		// [Defaults: 1, .15, .15, 1, 0] reversal potentials for each channel
  VChanSpec	hyst;		// [Defaults: .05, .8, .7, .1] hysteresis (excitatory) v-gated chan (Ca2+, NMDA)
  VChanSpec	acc;		// [Defaults: .01, .5, .1, .1] accomodation (inhibitory) v-gated chan (K+)
  NoiseType	noise_type;	// where to add random noise in the processing (if at all)
  Random	noise;		// #CONDEDIT_OFF_noise_type:NO_NOISE distribution parameters for random added noise
  Schedule	noise_sched;	// #CONDEDIT_OFF_noise_type:NO_NOISE schedule of noise variance over settling cycles

  FunLookup	nxx1_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT convolved gaussian and x/x+1 function as lookup table
  FunLookup	noise_conv;	// #HIDDEN #NO_SAVE #NO_INHERIT gaussian for convolution
  float		phase_delta;	// #READ_ONLY external plus-minus phase dif (error signal), applied in post settle

  void 		InitWtState(Unit* u);

  virtual void	SetCurLrate(LeabraUnit* u, int epoch);
  // set current learning rate based on epoch

  //////////////////////////////////////////
  //	Stage 0: at start of settling	  // 
  //////////////////////////////////////////

  virtual void	InitDelta(LeabraUnit* u);
  virtual void	InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ UnitSpec::InitState(u); }

  virtual void 	Compute_NetScale(LeabraUnit* u, LeabraLayer* lay);
  // compute net input scaling values and input from hard-clamped inputs
  virtual void 	Send_ClampNet(LeabraUnit* u, LeabraLayer* lay);
  // compute net input from hard-clamped inputs (sender based)

  //////////////////////////////////
  //	Stage 1: netinput 	  //
  //////////////////////////////////

  void 		Send_Net(Unit* u, Layer* tolay) { UnitSpec::Send_Net(u, tolay); }
  void 		Send_Net(LeabraUnit* u, LeabraLayer* lay, LeabraLayer* tolay);
  // add ext input, send-based, only to selected layer

  ////////////////////////////////////////////////////////////////
  //	Stage 2: netinput averages and clamping (if necc)	//
  ////////////////////////////////////////////////////////////////

  inline virtual void	Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // compute netin average
  inline virtual void	Compute_InhibAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // compute inhib netin average
  virtual void	Compute_HardClamp(LeabraUnit* u, LeabraLayer* lay);
  virtual bool	Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay);
  // soft-clamps unit, returns true if unit is not above .5

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual float	Compute_IThresh(LeabraUnit* u);
  // compute inhibitory value that would place unit directly at threshold

  ////////////////////////////////////////
  //	Stage 4: the final activation 	//
  ////////////////////////////////////////

  void		Compute_Act(Unit* u)	{ UnitSpec::Compute_Act(u); }
  virtual void 	Compute_Act(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, int cycle);
  // compute the final activation

  ////////////////////////////////////////
  //	Stage 5: Between Events 	//
  ////////////////////////////////////////

  virtual void	PhaseInit(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl);
  // initialize external input flags based on phase
  virtual void	DecayState(LeabraUnit* u, LeabraLayer* lay, float decay);
  // decay activation states towards initial values
  virtual void	ExtToComp(LeabraUnit* u, LeabraLayer* lay);
  // change external inputs to comparisons (remove input)
  virtual void	TargExtToComp(LeabraUnit* u, LeabraLayer* lay);
  // change target & external inputs to comparisons (remove targ & input)
  virtual void	PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			   LeabraTrial* trl, bool set_both=false);
  // set stuff after settling is over (set_both = both _m and _p for current)

  ////////////////////////////////////////
  //	Stage 6: Learning 		//
  ////////////////////////////////////////

  virtual void	Compute_dWt_impl(LeabraUnit* u, LeabraLayer* lay);
  // actually do wt change
  void		Compute_dWt(Unit*)	{ };
  virtual void	Compute_dWt(LeabraUnit* u, LeabraLayer* lay);

  virtual void	Compute_dWt_post(LeabraUnit* u, LeabraLayer* lay);
  // after computing the weight changes (for recon delta)

  virtual void	Compute_WtFmLin(LeabraUnit* u, LeabraLayer* lay);
  // if weights need to be updated from linear values without doing updatewts

  void		UpdateWeights(Unit* u);

  virtual void	EncodeState(LeabraUnit*, LeabraLayer*)	{ };
  // encode current state information (hook for time-based learning)

  virtual void	CreateNXX1Fun();  // create convolved gaussian and x/x+1 

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
  float		i;		// inhibitory
  float		h;		// hysteresis (Ca)
  float		a;		// accomodation (K)

  void	Initialize();
  void	Destroy()	{ };
  void 	Copy_(const LeabraUnitChans& cp);
  COPY_FUNS(LeabraUnitChans, taBase);
  TA_BASEFUNS(LeabraUnitChans);
};

class LeabraUnit : public Unit {
  // Leabra unit, point-neuron approximation
public:
  float		act_eq;		// #NO_SAVE rate-code equivalent activity value (time-averaged spikes or just act)
  float		act_avg;	// average activation over long time intervals (dt = act.avg_dt)
  float		act_m;		// minus_phase activation, set after settling, used for learning
  float		act_p;		// plus_phase activation, set after settling, used for learning
  float		act_dif;	// difference between plus and minus phase acts, gives unit err contribution
  float		da;		// #NO_SAVE delta activation: change in act from one cycle to next, used to stop settling
  VChanBasis	vcb;		// voltage-gated channel basis variables
  LeabraUnitChans gc;		// #NO_SAVE current unit channel conductances
  float		I_net;		// #NO_SAVE net current produced by all channels
  float		v_m;		// #NO_SAVE membrane potential

  bool		in_subgp;	// #READ_ONLY #NO_SAVE determine if unit is in a subgroup
  float		clmp_net;	// #NO_VIEW #NO_SAVE #DETAIL hard-clamp net input (no need to recompute)
  float		net_scale;	// #NO_VIEW #NO_SAVE #DETAIL total netinput scaling basis
  float		bias_scale;	// #NO_VIEW #NO_SAVE #DETAIL bias weight scaling factor
  float		prv_net;	// #NO_VIEW #NO_SAVE #DETAIL previous net input (for time averaging)
  float		prv_g_i;	// #NO_VIEW #NO_SAVE #DETAIL previous inhibitory cond val
  float		i_thr;		// #NO_VIEW #NO_SAVE #DETAIL inhibitory threshold value for computing kWTA

  void		InitDelta()
  { ((LeabraUnitSpec*)spec.spec)->InitDelta(this); }
  void		InitState(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->InitState(this, lay); }

  void		SetCurLrate(int epoch)
  { ((LeabraUnitSpec*)spec.spec)->SetCurLrate(this, epoch); }

  void		Compute_NetScale(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetScale(this, lay); }
  void		Send_ClampNet(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Send_ClampNet(this, lay); }

  void		Send_Net(LeabraLayer* lay, LeabraLayer* tolay)
  { ((LeabraUnitSpec*)spec.spec)->Send_Net(this, lay, tolay); }

  void		Compute_NetAvg(LeabraLayer* lay, LeabraInhib* athr)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetAvg(this, lay, athr); }
  void		Compute_InhibAvg(LeabraLayer* lay, LeabraInhib* athr)
  { ((LeabraUnitSpec*)spec.spec)->Compute_InhibAvg(this, lay, athr); }
  void		Compute_HardClamp(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_HardClamp(this, lay); }
  bool		Compute_SoftClamp(LeabraLayer* lay) 
  { return ((LeabraUnitSpec*)spec.spec)->Compute_SoftClamp(this, lay); }

  float		Compute_IThresh()
  { return ((LeabraUnitSpec*)spec.spec)->Compute_IThresh(this); }

  void		Compute_Act()	{ Unit::Compute_Act(); }
  void 		Compute_Act(LeabraLayer* lay, LeabraInhib* athr, int cycle) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_Act(this, lay, athr, cycle); }

  void		PhaseInit(LeabraLayer* lay, LeabraTrial* trl)
  { ((LeabraUnitSpec*)spec.spec)->PhaseInit(this, lay, trl); }
  void		DecayState(LeabraLayer* lay, float decay)
  { ((LeabraUnitSpec*)spec.spec)->DecayState(this, lay, decay); }
  void		ExtToComp(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->ExtToComp(this, lay); }
  void		TargExtToComp(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->TargExtToComp(this, lay); }
  void		PostSettle(LeabraLayer* lay, LeabraInhib* athr, LeabraTrial* trl, bool set_both=false)
  { ((LeabraUnitSpec*)spec.spec)->PostSettle(this, lay, athr, trl, set_both); }

  void 		Compute_dWt(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt(this, lay); }	  
  void 		Compute_dWt_post(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt_post(this, lay); }

  void 		Compute_WtFmLin(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_WtFmLin(this, lay); }

  void 		EncodeState(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->EncodeState(this, lay); }

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
  float		adth_pct;	// #HIDDEN what pct of k units can adapt threshold at one time (.5 std)

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(KWTASpec);
  COPY_FUNS(KWTASpec, taBase);
  TA_BASEFUNS(KWTASpec);
};

class AdaptKWTASpec : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER specifies adaptive kwta specs (esp for avg-based)
public:
  float		a_dt;		// #DEF_0.005 time constant for integrating average average activation
  float		tol;		// #DEF_0.05 tolerance around target avg act before changing i_kwta_pt
  float		pt_dt;		// [Default: 0] time constant for integrating i_kwta_pt changes (0 = no adapt, .1 std when used)
  float		mx_d;		// #DEF_0.2 maximum deviation from initial i_kwta_pt allowed

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(AdaptKWTASpec);
  COPY_FUNS(AdaptKWTASpec, taBase);
  TA_BASEFUNS(AdaptKWTASpec);
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
    UNIT_INHIB			// unit-based inhibition (g_i from netinput -- requires connections with inhib flag set to provide inhibition)
  };

  enum InhibGroup {
    ENTIRE_LAYER,		// treat entire layer as one inhibitory group (even if subgroups exist)
    UNIT_GROUPS,		// treat sub unit groups as separate inhibitory groups
    LAY_AND_GPS			// compute inhib over both groups and whole layer
  };
  
  KWTASpec	kwta;		// #CONDEDIT_OFF_inhib_group:UNIT_GROUPS desired activity level over entire layer
  KWTASpec	gp_kwta;	// #CONDEDIT_OFF_inhib_group:ENTIRE_LAYER desired activity level for units within unit groups (not for ENTIRE_LAYER)
  InhibGroup	inhib_group;	// what to consider the inhibitory group (layer or unit subgroups, or both)
  Compute_I	compute_i;	// how to compute inhibition (g_i): two forms of kwta or unit-level inhibition
  float		i_kwta_pt;	// [Default: .25 for KWTA_INHIB, .6 for KWTA_AVG] point to place inhibition between k and k+1
  float		minus_delta_pt;	// [Default: 0] lowers the i_kwta_pt in the minus phase, producing broader activations that are then sharpened in the plus phase, plus-minus error signal weeds out weak units
  AdaptKWTASpec	adapt_pt;	// adapt the i_kwta_pt point based on diffs between actual and target k level (for avg-based)
  ClampSpec	clamp;		// how to clamp external inputs to units (hard vs. soft)
  DecaySpec	decay;		// decay of activity state vars between events, -/+ phase, and 2nd set of phases (if appl)
  LayerLinkSpec	layer_link;	// link inhibition between layers (with specified gain), rarely used, linked layers in layer objects

  virtual void	InitWtState(LeabraLayer* lay);
  // initialize weight values and other permanent state

  virtual void	SetCurLrate(LeabraLayer* lay, int epoch);
  // set current learning rate based on epoch

  //////////////////////////////////////////
  //	Stage 0: at start of settling	  // 
  //////////////////////////////////////////

  virtual void	InitState(LeabraLayer* lay);
  // prior to settling: initialize dynamic state variables
  virtual void	Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  // prior to settling: hard-clamp inputs
  virtual void	Compute_NetScale(LeabraLayer* lay, int phase);
  // prior to settling: compute netinput scaling values
  virtual void	Send_ClampNet(LeabraLayer* lay, int phase);
  // prior to settling: compute input from hard-clamped

  //////////////////////////////////
  //	Stage 1: netinput 	  //
  //////////////////////////////////

  virtual void 	Send_Net(LeabraLayer* lay, LeabraLayer* tolay, int cycle, int phase);
  // compute net inputs

  ////////////////////////////////////////////////////////////////
  //	Stage 2: netinput averages and clamping (if necc)	//
  ////////////////////////////////////////////////////////////////

  virtual void	Compute_Clamp_NetAvg(LeabraLayer* lay, int cycle, int phase);
  // clamp and compute averages of net inputs that were already computed
  virtual void	Compute_NetAvg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
			       int cycle, int phase);
  virtual void	Compute_SoftClamp(LeabraLayer* lay, int cycle, int phase);
  // soft-clamp inputs by adding to net input

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual void	InitInhib(LeabraLayer* lay);
  // initialize the inhibitory state values
  virtual void	Compute_Inhib(LeabraLayer* lay, int cycle, int phase);
  // stage two: compute the inhibition for layer
  virtual void	Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, int phase);
  // implementation of inhibition computation for either layer or unit group
  virtual void	Compute_Inhib_kWTA(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, int phase);
  // implementation of basic kwta inhibition computation
  virtual void	Compute_Inhib_kWTA_Avg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, int phase);
  // implementation of kwta avg-based inhibition computation
  virtual void	Compute_LinkInhib(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
				   int cycle);
  // compute inhibition after linkages with other layers are factored in
  virtual void	Compute_Active_K(LeabraLayer* lay);
  virtual void	Compute_Active_K_impl(LeabraLayer* lay, Unit_Group* ug,
				      LeabraInhib* thr, KWTASpec& kwtspec);
  virtual int	Compute_Pat_K(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);

  ////////////////////////////////////////
  //	Stage 4: the final activation 	//
  ////////////////////////////////////////

  virtual void 	Compute_Act(LeabraLayer* lay, int cycle, int phase);
  // stage three: compute final activation
  virtual void 	Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
				 int cycle, int phase);

  ////////////////////////////////////////
  //	Stage 5: Between Events 	//
  ////////////////////////////////////////

  virtual void	PhaseInit(LeabraLayer* lay, LeabraTrial* trl);
  // initialize start of a setting phase, set input flags appropriately, etc

  virtual void	DecayEvent(LeabraLayer* lay);
  virtual void	DecayPhase(LeabraLayer* lay);
  virtual void	DecayPhase2(LeabraLayer* lay);

  virtual void	ExtToComp(LeabraLayer* lay);
  // change external inputs to comparisons (remove input)
  virtual void	TargExtToComp(LeabraLayer* lay);
  // change target & external inputs to comparisons (remove targ & input)
  virtual void	PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  // after settling, keep track of phase variables, etc.

  ////////////////////////////////////////
  //	Stage 6: Learning 		//
  ////////////////////////////////////////

  virtual void	AdaptKWTAPt(LeabraLayer* lay);
  // adapt the kwta point based on average activity
  virtual void	Compute_dWt(LeabraLayer* lay);
  virtual void	Compute_dWt_post(LeabraLayer* lay);
  // something to be done after the weights are changed (i.e. update history..)
  virtual void	Compute_WtFmLin(LeabraLayer* lay);
  // use this if weights will be used again for activations prior to being updated

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
  float		avg;	// average value
  float		max;	// maximum value
  
  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const AvgMaxVals& cp) { avg = cp.avg; max = cp.max; }
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
  void		Compute_AdthK(float adth_pct);
  void		Compute_IThrR(); // compute ithr_r ratio value

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const KWTAVals& cp);
  COPY_FUNS(KWTAVals, taBase);
  TA_BASEFUNS(KWTAVals);
};

class AdaptKWTAVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for adapting kwta stuff
public:
  float		avg_avg;	// average of the average activation in a layer
  float		i_kwta_pt;	// adapting point to place inhibition between k and k+1 for kwta

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const AdaptKWTAVals& cp);
  COPY_FUNS(AdaptKWTAVals, taBase);
  TA_BASEFUNS(AdaptKWTAVals);
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

  AvgMaxVals	netin;		// #DETAIL net input values for the layer
  AvgMaxVals	acts;		// #DETAIL activation values for the layer
  AvgMaxVals	acts_p;		// #DETAIL plus-phase activation stats for the layer
  AvgMaxVals	acts_m;		// #DETAIL minus-phase activation stats for the layer
  AvgMaxVals	acts_dif;	// #DETAIL difference between p and m phases
  float		phase_dif_ratio; // phase-difference ratio (-avg / +avg)
 
  KWTAVals	kwta;		// #DETAIL values for kwta -- activity levels, etc
  InhibVals	i_val;		// inhibitory values
  AvgMaxVals	un_g_i;		// #DETAIL average and stdev (not max) values for unit inhib-to-thresh
  AdaptKWTAVals	adapt_pt;	// #DETAIL adapting kwta point values

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
  bool		needs_wt_updt;	// #READ_ONLY needs weight update to be run (i.e., Compute_dWt was called)
  int		prv_phase;	// #READ_ONLY previous phase value (needed for 2nd plus phases and the like)
  float		td;		// #READ_ONLY temporal differences delta value (where applicable)
  int		td_updt;	// #READ_ONLY true if td triggered an update (either + to store or - reset)

  void	Build();
  void	InitState() 	{ spec->InitState(this); }
  void	InitWtState() 	{ spec->InitWtState(this); }
  void	InitInhib() 	{ spec->InitInhib(this); } // initialize inhibitory state
  void	ModifyState()	{ spec->DecayEvent(this); } // this is what modify means..

  void	SetCurLrate(int epoch) { spec->SetCurLrate(this, epoch); }
  
  void	Compute_HardClamp(LeabraTrial* trl) { spec->Compute_HardClamp(this, trl); }
  void	Compute_NetScale(int phase) { spec->Compute_NetScale(this, phase); }
  void	Send_ClampNet(int phase) { spec->Send_ClampNet(this, phase); }

  void	Send_Net(Layer* tolay)		{ spec->Send_Net(this, (LeabraLayer*)tolay, -1, 0); }
  void	Send_Net(LeabraLayer* tolay, int cycle, int phase)
  { spec->Send_Net(this, tolay, cycle, phase); }

  void	Compute_Clamp_NetAvg(int cycle, int phase)
  { spec->Compute_Clamp_NetAvg(this, cycle, phase); }
  void	Compute_Inhib(int cycle, int phase)
  { spec->Compute_Inhib(this, cycle, phase); }
  void	Compute_Act()			{ spec->Compute_Act(this, -1, 0); }
  void	Compute_Act(int cycle, int phase) { spec->Compute_Act(this, cycle, phase); }
  void	Compute_Active_K()		{ spec->Compute_Active_K(this); }

  void	PhaseInit(LeabraTrial* trl)		{ spec->PhaseInit(this, trl); }
  void	DecayEvent()	{ spec->DecayEvent(this); } // decay between events
  void	DecayPhase()    { spec->DecayPhase(this); } // decay between phases
  void	DecayPhase2()  	{ spec->DecayPhase2(this); } // decay between 2nd set of phases

  void	ExtToComp()		{ spec->ExtToComp(this); }
  void	TargExtToComp()		{ spec->TargExtToComp(this); }
  void	PostSettle(LeabraTrial* trl, bool set_both=false)	{ spec->PostSettle(this, trl, set_both); }

  void	Compute_dWt() 		{ spec->Compute_dWt(this); }
  void	Compute_dWt_post()	{ spec->Compute_dWt_post(this); }
  void	Compute_WtFmLin() 	{ spec->Compute_WtFmLin(this); }
  void	UpdateWeights();

  virtual void	ResetSortBuf();

  bool		SetLayerSpec(LayerSpec* sp);
  LayerSpec*	GetLayerSpec()		{ return (LayerSpec*)spec.spec; }

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

  void	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
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
  ru->gc.i += ((LeabraCon_Group*)ru->recv.Gp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_Inhib(cg, (LeabraCon*)cg->Cn(i), (LeabraUnit*)cg->Un(i), su));
}

void LeabraConSpec::C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, Unit* su) {
  ru->net += ((LeabraCon_Group*)ru->recv.Gp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_Net(Con_Group* cg, Unit* su) {
  if(inhib)
    Send_Inhib((LeabraCon_Group*)cg, (LeabraUnit*)su);
  else {
    CON_GROUP_LOOP(cg, C_Send_Net((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), cg->Un(i), su));
  }
}



void LeabraConSpec::C_Send_ClampNet(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru,
				 LeabraUnit* su) {
  ru->clmp_net += ((LeabraCon_Group*)ru->recv.Gp(cg->other_idx))->scale_eff * su->act * cn->wt;
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
inline void LeabraConSpec::C_Compute_dWt(LeabraCon* cn, float heb, float err) {
  float dwt = lmix.err * err + lmix.hebb * heb;
  cn->dwt += dwt;
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
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	C_Compute_dWt(cn,
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
      }
    }
  }
}

inline void LeabraConSpec::C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg,
					   LeabraUnit*, LeabraUnit*)
{
  if(cn->wt < 0.0f) {
    // wt is negative in linear form, so using opposite sign of usual here
    cn->wt -= cur_lrate * cn->dwt;
    cn->wt = MAX(cn->wt, -1.0f);	// limit 0-1
    cn->wt = MIN(cn->wt, 0.0f);
    C_Compute_WtFmLin(cg, cn);	// go back to nonlinear weights
    // then put in real limits!!
    cn->wt = MIN(cn->wt, wt_limits.max);	// limit 0-1
    cn->wt = MAX(cn->wt, wt_limits.min);
    cn->pdw = cn->dwt;
    cn->dwt = 0.0f;
  }
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
//	Unit NetAvg   	//
//////////////////////////

void LeabraUnitSpec::Compute_NetAvg(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  u->net *= g_bar.e;		// multiply by gbar..
  u->net = u->prv_net + dt.net * (u->net - u->prv_net);
  u->prv_net = u->net;
  u->i_thr = Compute_IThresh(u);
}

void LeabraUnitSpec::Compute_InhibAvg(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  u->gc.i *= g_bar.i;		// multiply by gbar..
  u->gc.i = u->prv_g_i + dt.net * (u->gc.i - u->prv_g_i);
  u->prv_g_i = u->gc.i;
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
  virtual void	Compute_dWt();
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
    NO_DWT,
    AC_ONLY_DWT,
    ALL_DWT,
    ONLY_FIRST_DWT
  };

  PhaseOrder	phase_order;	// [Default: MINUS_PLUS] number and order of phases to present
  Counter	phase_no;	// Current phase number
  Phase		phase;		// Type of current phase: minus or plus
  StateInit	trial_init;	// #DEF_DECAY_STATE how to initialize network state at start of trial
  bool		no_plus_stats;	// #DEF_true don't do stats/logging in the plus phase
  bool		no_plus_test; 	// #DEF_true don't run the plus phase when testing
  FirstPlusdWt	first_plus_dwt;	// #CONDEDIT_ON_phase_order:MINUS_PLUS_NOTHING,MINUS_PLUS_PLUS how to change weights on first plus phase if 2 plus phases (applies only to standard leabralayer specs -- others must decide on their own!)

  void		C_Code();	// modify to use the no_plus_stats flag
  
  void		Init_impl();
  void		UpdateState();
  void		Final();
  bool		Crit()		{ return SchedProcess::Crit(); }

  virtual void	InitState();
  virtual void	SetCurLrate();
  virtual void	DecayState();
  virtual void	EncodeState();
  virtual void	Compute_dWt_post();
  virtual void	Compute_dWt();
  virtual void	UpdateWeights();

  void		GetCntrDataItems();
  void		GenCntrLog(LogData* ld, bool gen);

  void	UpdateAfterEdit();
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

  void	ExtToComp(LeabraLayer* lay);

  virtual void 	UpdateUnitGain(LeabraUnitSpec* us, float new_gain);
  // update the unit spec after new value copied
  virtual void 	UpdateUnitErr(LeabraUnitSpec* us, float new_err);
  // update the unit spec after new value copied

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
  void		Compute_Act(Unit* u)	{ UnitSpec::Compute_Act(u); }
  void 	Compute_Act(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, int cycle);
  
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

  virtual void Compute_Context(LeabraLayer* lay, LeabraUnit* u, int phase);
  // get context source value for given context unit

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
  int		ac_prjn_idx;	// #READ_ONLY projection that has the AC layer

  virtual void	FindACLayer(LeabraLayer* lay);
  // find the projection from the AC layer, set ac_prjn_idx
  virtual float	Get_TD(LeabraLayer* lay);
  // get the td error value from AC unit on 2nd connection into 1st unit in layer
  virtual bool	Get_RewReset(LeabraLayer* lay);
  // get the reward reset from AC unit on 2nd connection into 1st unit in layer
  virtual void	UpdateConSpec(LeabraConSpec* cs, float new_gain);
  // update the connection spec wt_scale.abs with new gain value
  virtual void	UndoHardClamp(LeabraLayer* lay);
  // undo hard clamp settings so that layer will settle normally

  void		InitState(LeabraLayer* lay);
  void		Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl);
  void		Compute_Context(LeabraLayer* lay, LeabraUnit* u, int phase);

  void		PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both=false);
  // update hysteresis based on td value

  void 	Initialize();
  void	Destroy()		{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(LeabraGatedCtxLayerSpec);
  COPY_FUNS(LeabraGatedCtxLayerSpec, LeabraContextLayerSpec);
  TA_BASEFUNS(LeabraGatedCtxLayerSpec);
};


//////////////////////////////////////////
// 	Duration  Environment		//
//////////////////////////////////////////

class DurEvent : public Event {
  // an event which lasts for a particular amount of time
public:
  float		duration;	// #ENVIROVIEW length of time (cycles) event should be presented

  void 	Initialize();
  void	Destroy()		{ };
  void	Copy_(const DurEvent& cp);
  COPY_FUNS(DurEvent, Event);
  TA_BASEFUNS(DurEvent);
};  

// the following are not in regular use, and are considered "demoted"

//////////////////////////////////////////
// 	Encode/Learn Environment	//
//////////////////////////////////////////

class EncLrnEventSpec : public EventSpec {
  // explicit encoding and learning event spec for controling learning
public:
  enum EncLrnMode {
    IGNORE,			// do not encode or learn on this event
    ENCODE,			// encode this event (store activations) but don't learn
    LEARN,			// learn from this event, but don't encode new state
    LRN_ENC			// both learn and encode from this event
  };

  EncLrnMode	enc_lrn;	// what to do with this event: ignore, encode or learn

  void	Initialize();
  void	Destroy()	{ };
  void	Copy_(const EncLrnEventSpec& cp);
  COPY_FUNS(EncLrnEventSpec, EventSpec);
  TA_BASEFUNS(EncLrnEventSpec);
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

class LeabraTimeConSpec : public LeabraConSpec {
  // computes weight change based on recv current and send previous acts
public:
  float		k_prv;		// how much to learn from the previous senders
  float		k_cur;		// how much to learn from the current senders

  float		C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group*,
			       float ru_act, float su_act, float max_cor) {
    // wt is negative in linear form, so using opposite sign of usual here
    return ru_act * (su_act * (max_cor + cn->wt) +
		     (1.0f - su_act) * cn->wt);
  }
    
  inline void 	C_Compute_dWt(LeabraCon* cn, float heb, float err, float k_mult) {
    float dwt = lmix.err * err + lmix.hebb * heb;
    cn->dwt += k_mult * dwt;
  }

  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraTimeConSpec);
  COPY_FUNS(LeabraTimeConSpec, LeabraConSpec);
  TA_BASEFUNS(LeabraTimeConSpec);
};

class LeabraTimeCon_Group : public LeabraCon_Group {
  // one step of history learning
public:
  float		p_savg;		// #NO_SAVE previous sending average activation
  float		p_max_cor;	// #NO_SAVE previous savg correction factor for cpca

  void		EncodeState(LeabraUnit*)  { p_savg = savg; p_max_cor = max_cor; }

  void 	Initialize();
  void	Destroy()		{ };
  void	Copy_(const LeabraTimeCon_Group& cp);
  COPY_FUNS(LeabraTimeCon_Group, LeabraCon_Group);
  TA_BASEFUNS(LeabraTimeCon_Group);
};

class LeabraTimeUnitSpec : public LeabraUnitSpec {
  // adapts weights based on one step of activation history 
public:
  void		InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ LeabraUnitSpec::InitState(u); }
  void		Compute_dWt(Unit*) { };
  void		Compute_dWt(LeabraUnit* u, LeabraLayer* lay);

  void		EncodeState(LeabraUnit* u, LeabraLayer* lay);

  void	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraTimeUnitSpec);
  COPY_FUNS(LeabraTimeUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(LeabraTimeUnitSpec);
};

class LeabraTimeUnit : public LeabraUnit {
  // LEABRA unit with an single step activation history
public:
  float 	p_act_m;	// previous minus phase activation 
  float		p_act_p;	// previous plus phase activation

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LeabraTimeUnit);
  COPY_FUNS(LeabraTimeUnit, LeabraUnit);
  TA_BASEFUNS(LeabraTimeUnit);
};

inline void LeabraTimeConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  LeabraTimeUnit* lru = (LeabraTimeUnit*)ru;
  LeabraTimeCon_Group* lcg = (LeabraTimeCon_Group*) cg;
  // this pass is for the sender_(t-1), recv_(t) weight change
  if((lru->p_act_p >= 0.0f) && (lcg->p_savg >= savg_cor.thresh) && (k_prv != 0.0f)) {
    for(int i=0; i<cg->size; i++) {
      LeabraTimeUnit* su = (LeabraTimeUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	C_Compute_dWt(cn, 
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->p_act_p, lcg->p_max_cor),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->p_act_p, su->p_act_m),
		      k_prv);
      }
    }
  }
  // this is for the sender_(t), recv_(t) weight change
  Compute_SAvgCor(lcg, lru);
  float use_k_cur = k_cur;
  if(lru->p_act_p < 0.0f)	use_k_cur = 1.0f; // use full current if no encoded
  if((((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) && (k_cur != 0.0f)) {
    for(int i=0; i<cg->size; i++) {
      LeabraTimeUnit* su = (LeabraTimeUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh))) {
	C_Compute_LinFmWt(lcg, cn); // get into linear form
	C_Compute_dWt(cn, 
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p, lcg->max_cor),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m),
		      use_k_cur);
      }
    }
  }
}

//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////

class LeabraNegBiasSpec : public LeabraBiasSpec {
  // only learns negative bias changes, not positive ones (decay restores back to zero)
public:
  float		decay;		// rate of weight decay towards zero 

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
    if((fabsf(cn->dwt) < dwt_thresh) || (cn->dwt > 0.0f))
      cn->dwt = 0.0f;
    cn->dwt -= decay * cn->wt;
    cn->pdw = cn->dwt;
    cn->wt += cur_lrate * cn->dwt;
    cn->dwt = 0.0f;
    C_ApplyLimits(cn, ru, NULL);
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
    LeabraCon_Group* ru_gp = (LeabraCon_Group*)ru->recv.Gp(cg->other_idx);
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
    LeabraCon_Group* ru_gp = (LeabraCon_Group*)ru->recv.Gp(cg->other_idx);
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

//////////////////////////////////
// 	Activity Regulation	//
//////////////////////////////////

class ActRegSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activity regulation via bias weight adjustment
public:
  float		min;		// #CONDEDIT_OFF_bias_dt:0 minimum avg act, above which no adaptation (.01 std)
  float		max;		// #CONDEDIT_OFF_bias_dt:0 maximum avg act, below which no adaptation (.35 std)
  float		bias_dt;	// time constant for integrating bias changes (.1 std)
  float		dt_var;		// #CONDEDIT_OFF_bias_dt:0 variance in dt (gaussian stdev) to prevent units from doing same thing (.01 std)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActRegSpec);
  COPY_FUNS(ActRegSpec, taBase);
  TA_BASEFUNS(ActRegSpec);
};

class LeabraActRegUnitSpec : public LeabraUnitSpec {
  // activity regulation leabra unit spec
public:
  ActRegSpec 	act_reg;	// activity regulation: min (.01), max (.35) = no-adapt values of act_avg, bias_dt = dt for bias wt (0=no adapt), dt_var = variance in dt for differentiation (.01)

  void		Compute_dWt(Unit*) { };
  void		Compute_dWt(LeabraUnit* u, LeabraLayer* lay);
  
  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(LeabraActRegUnitSpec);
  COPY_FUNS(LeabraActRegUnitSpec, LeabraUnitSpec);
  TA_BASEFUNS(LeabraActRegUnitSpec);
};

#endif // leabra_h
