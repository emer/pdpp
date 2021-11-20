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
#ifdef __GNUG__
#pragma interface
#endif
#define leabra_h

#include <pdp/netstru.h>
#include <pdp/sched_proc.h>
#include <ta_misc/fun_lookup.h>
#include <xleabra/leabra_TA_type.h>
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

// wt_lin is the underlying internal linear weight value upon which
// learning occurs, and wt is the sigmoidal function of wt_lin for
// computing net input.  Thus, wt is a contrast-enhanced version of
// the actual weight value.

class LeabraCon : public Connection {
  // Leabra connection
public:
  float		wt_lin;		// #NO_VIEW linear (internal) weight value (on which learning occurs)
  float		dwt;		// #NO_VIEW #NO_SAVE resulting net weight change
  float		pdw;		// #NO_SAVE previous delta-weight change

  void 	Initialize()		{ wt_lin = 0.0f; dwt = pdw = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const LeabraCon& cp);
  COPY_FUNS(LeabraCon, Connection);
  TA_BASEFUNS(LeabraCon);
};

class WtScaleSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER weight scaling specification
public:
  float		abs;		// absolute scaling (not subject to normalization)
  float		rel;		// relative scaling (subject to normalization)

  float		NetScale() 	{ return abs * rel; }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(WtScaleSpec);
  COPY_FUNS(WtScaleSpec, taBase);
  TA_BASEFUNS(WtScaleSpec);
};

class WtSigSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER sigmoidal weight function specification
public:
  float		gain;		// gain (contrast, sharpness) of the function
  float		off;		// offset of the function (1=centered at .5, >1=higher)

  // function for implementing weight sigmodid
  static float	SigFun(float w, float gain, float off) {
    float wrat;
    if(w == 0.0f) wrat = MAXFLOAT;
    else if(w == 1.0f) wrat = 0.0f;
    else wrat = off * (1.0f - w) / w;
    return 1.0f / (1.0f + powf(wrat, gain));
  }

  // function for implementing inverse of weight sigmoid
  static float	SigFunInv(float w, float gain, float off) {
    float wrat;
    if(w == 0.0f) wrat = MAXFLOAT;
    else if(w == 1.0f) wrat = 0.0f;
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
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER mixture of learning factors specification
public:
  float		hebb;		// amount of hebbian learning
  float		err;		// amount of error driven learning

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(LearnMixSpec);
  COPY_FUNS(LearnMixSpec, taBase);
  TA_BASEFUNS(LearnMixSpec);
};

class FixedAvg : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER provide fixed average activity level
public:
  bool		fix;		// override automatic computation of avg (and use fixed val)
  float		avg;		// fixed average activity value

  float 	GetAvg(float layer_pct)
  { float rval = layer_pct; if(fix) rval = avg; return rval; }

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(FixedAvg);
  COPY_FUNS(FixedAvg, taBase);
  TA_BASEFUNS(FixedAvg);
};

class AvgCorSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER average activation correction specifications
public:
  enum AvgSource {
    LAYER_AVG_ACT,
    LAYER_TRG_PCT,
    FIXED_AVG,
    COMPUTED_AVG		// average over particular inputs is computed directly
  };

  float		cor;		// amount of correction to apply (0=none, 1=all, .5=half, etc)
  AvgSource	src;		// source of average act for correcting computation of cpca
  float		thresh;		// threshold below which learning does not occur

  inline void Compute_AvgCor(float& cg_avg, float& cg_cor, float fx_avg,
			     LeabraCon_Group* cg, LeabraLayer* lay);

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(AvgCorSpec);
  COPY_FUNS(AvgCorSpec, taBase);
  TA_BASEFUNS(AvgCorSpec);
};

class LeabraConSpec : public ConSpec {
  // Leabra connection specs
public:
  bool		inhib;		// this makes the connection inhibitory (to g_i instead of net)
  WtScaleSpec	wt_scale;	// scale weight values, both relative and absolute factors
  WtSigSpec	wt_sig;		// sigmoidal weight function for contrast enhancement
  float		lrate;		// learning rate
  LearnMixSpec	lmix;		// mixture of learning factors (hebbian, error) 
  FixedAvg	fix_savg;	// fixed sending average activity value (override automatic computation)
  AvgCorSpec	savg_cor;	// correct for sending average activation levels in hebbian
  FixedAvg	fix_ravg;	// fixed receiving average activity value (override automatic computation)
  AvgCorSpec	ravg_cor;	// correct for receiving average activation levels in hebbian

  FunLookup	wt_sig_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT computes wt sigmoidal fun

  void		C_Compute_WtFmLin(LeabraCon_Group*, LeabraCon* cn)
  { cn->wt = wt_sig_fun.Eval(cn->wt_lin);  }
  inline virtual void	Compute_WtFmLin(LeabraCon_Group* gp);
  // compute actual weight value from linear weight value

  void		C_Compute_LinFmWt(LeabraCon_Group*, LeabraCon* cn)
  { cn->wt_lin = WtSigSpec::SigFunInv(cn->wt, wt_sig.gain, wt_sig.off); }
  inline virtual void	Compute_LinFmWt(LeabraCon_Group* gp);
  // compute linear weight value from actual weight value

  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su) {
    ConSpec::C_InitWtState(cg, cn, ru, su); LeabraCon* lcn = (LeabraCon*)cn;
    lcn->pdw = 0.0f; }

  void 		C_InitWtDelta(Con_Group* cg, Connection* cn, Unit* ru, Unit* su)
  { ConSpec::C_InitWtDelta(cg, cn, ru, su); ((LeabraCon*)cn)->dwt=0.0f; }

  inline void	C_InitWtState_post(Con_Group* cg, Connection* cn, Unit* ru, Unit* su);

  inline float 	C_Compute_Net(LeabraCon* cn, Unit*, Unit* su);
  inline float 	Compute_Net(Con_Group* cg, Unit* ru);
  // receiver-based net input 

  inline void 	C_Send_Net(LeabraCon_Group* cg, LeabraCon* cn, Unit* ru, Unit* su);
  inline void 	Send_Net(Con_Group* cg, Unit* su);
  // sender-based net input computation

  inline void 	C_Send_Inhib(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru, LeabraUnit* su);
  inline void 	Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su);
  // sender-based inhibitiory net input computation

  void		ApplySymmetry(Con_Group* cg, Unit* ru);
  // redo for copying wt_lin!

  // all of learning operates on wt_lin now, not wt!
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

  virtual void	CreateWtSigFun(); // create the wt_sig_fun

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
  float		dwt_thresh;
  // don't change wts if dwt < thresh (should be same as learn_thresh)

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru);

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
  float		savg_cor;	// #NO_SAVE savg correction factor for cpca
  float		ravg;		// #NO_SAVE receiving average activation
  float		ravg_cor;	// #NO_SAVE ravg correction factor for cpca

  void	Compute_LinFmWt()  { ((LeabraConSpec*)spec.spec)->Compute_LinFmWt(this); }

  void	Compute_WtFmLin()  { ((LeabraConSpec*)spec.spec)->Compute_WtFmLin(this); }

  void	Copy_Weights(const Con_Group* src)
  { Con_Group::Copy_Weights(src); Compute_LinFmWt(); }
  void	ReadWeights(istream& strm)
  { Con_Group::ReadWeights(strm); Compute_LinFmWt(); }
  void	TransformWeights(const SimpleMathSpec& trans)
  { Con_Group::TransformWeights(trans); Compute_LinFmWt(); }
  void	AddNoiseToWeights(const Random& noise_spec)
  { Con_Group::AddNoiseToWeights(noise_spec); Compute_LinFmWt(); }

  virtual void	EncodeState(LeabraUnit*) { };
  // encode current state information (hook for time-based learning)

  void 	Initialize();
  void	Destroy()		{ };
  void	Copy_(const LeabraCon_Group& cp);
  COPY_FUNS(LeabraCon_Group, Con_Group);
  TA_BASEFUNS(LeabraCon_Group);
};

class LeabraChannels : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in Leabra
public:
  float		e;		// excitatory
  float		l;		// leak
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
  bool		on;		// true if channel is on
  float		b_dt;		// time constant for integrating basis variable
  float		a_thr;		// activation threshold of the channel
  float		d_thr;		// deactivation threshold of the channel
  float		g_dt;		// time constant for incrementing conductance
  bool		init;		// initialize variables when state is intialized (else with weights)

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

class ActFunSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER activation function specifications
public:
  float		thr;		// initial threshold value (prior to adaptation)
  float		gain;		// gain of the activation function (act_eq for spikes)
  float		nvar;		// variance of the gaussian noise for NOISY_XX1

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(ActFunSpec);
  COPY_FUNS(ActFunSpec, taBase);
  TA_BASEFUNS(ActFunSpec);
};

class SpikeFunSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER spiking activation function specs
public:
  int		dur;		// spike duration in cycles (models duration of effect on postsyn)
  float		v_m_r;		// post-spiking membrane potential to reset to (refractory)
  float		eq_gain;	// gain for computing act_eq relative to actual average
  float		ext_gain;	// gain for clamped external inputs (which are constant)

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(SpikeFunSpec);
  COPY_FUNS(SpikeFunSpec, taBase);
  TA_BASEFUNS(SpikeFunSpec);
};

class AdaptThreshSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER adapting threshold based on average act
public:
  float		a_dt;		// time constant for integrating activation average
  float		min;		// minimum avg act, above which no adaptation
  float		max;		// maximum avg act, below which no adaptation
  float		t_dt;		// time constant for integrating threshold changes
  float		mx_d;		// maximum amount to change threshold

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(AdaptThreshSpec);
  COPY_FUNS(AdaptThreshSpec, taBase);
  TA_BASEFUNS(AdaptThreshSpec);
};

class OptThreshSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER optimization thresholds for faster processing
public:
  float		send;		// threshold for sending activation value
  float		learn;		// threshold for learning (activation value)
  bool		updt_wts;	// whether to apply threshold to updating weights

  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(OptThreshSpec);
  COPY_FUNS(OptThreshSpec, taBase);
  TA_BASEFUNS(OptThreshSpec);
};

class LeabraUnitSpec : public UnitSpec {
  // Leabra unit specifications, point-neuron approximation
public:
  enum ActFun {
    NOISY_XX1,			// noisy version of x over x plus 1 (noise is nvar)
    XX1,			// x over x plus 1
    LINEAR,			// simple linear output function 
    SPIKE			// spiking activations
  };

  enum NoiseType {
    NO_NOISE,			// no noise added to processing
    VM_NOISE,			// noise in the value of v_m
    NETIN_NOISE,		// noise in the net input (g_e)
    ACT_NOISE			// noise in the activations
  };

  ActFun	act_fun;	// activation function to use
  ActFunSpec	act;		// specifications of the activation function
  SpikeFunSpec	spike;		// specifications of the spiking activation function
  AdaptThreshSpec adapt_thr;	// adapting threshold specifications
  OptThreshSpec	opt_thresh;	// optimization thresholds for faster processing
  float		phase_dif_thresh;
  // threshold for phase differences (ratio -/+) for learning
  MinMaxRange	clamp_range;	// range of clamped activation values
  MinMaxRange	vm_range;	// membrane potential range
  Random	v_m_init;	// what to initialize the membrane potential to
  float		vm_dt;		// step size to take in integrating the membrane potential
  float		net_dt;		// step size to take in integrating the net input
  LeabraChannels g_bar;		// scales value of conductances
  LeabraChannels e_rev;		// reversal potential associated with each conductance
  VChanSpec	hyst;		// hysteresis v-gated chan (Ca2+, NMDA)
  VChanSpec	acc;		// fast accomodation v-gated chan (delayed rectifier)
  NoiseType	noise_type;	// where to add noise in the processing
  Random	noise;		// what kind of noise?
  Schedule	noise_sched;	// schedule of noise variance over settling cycles
  FunLookup	nxx1_fun;	// #HIDDEN #NO_SAVE #NO_INHERIT convolved gaussian and x/x+1 function as lookup table
  FunLookup	noise_conv;	// #HIDDEN #NO_SAVE #NO_INHERIT gaussian for convolution

  void 		InitWtState(Unit* u);

  virtual void	InitState(LeabraUnit* u, LeabraLayer* lay);
  void		InitState(Unit* u)	{ UnitSpec::InitState(u); }
  virtual void	InitThresh(LeabraUnit* u);
  // initialize the threshold and act_avg back to original starting values

  virtual void	ReConnect_Load(LeabraUnit* u, LeabraLayer* lay);
  // #IGNORE update non-saved variables after loading network

  virtual void	CompToTarg(LeabraUnit* u, LeabraLayer* lay);
  // copy comparison external inputs to target ones
  virtual void 	Compute_NetScaleClamp(LeabraUnit* u, LeabraLayer* lay);
  // compute net input scaling values and input from hard-clamped inputs

  void 		Compute_Net(Unit* u) { UnitSpec::Compute_Net(u); }
  void 		Compute_Net(LeabraUnit* u, LeabraLayer* lay);
  // add ext input, send-based
  inline virtual void	Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // compute netin average
  inline virtual void	Compute_InhibAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // compute inhib netin average
  virtual void	Compute_HardClamp(LeabraUnit* u, LeabraLayer* lay);
  virtual bool	Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay);
  // soft-clamps unit, returns true if unit is not above .5

  void		Compute_Act(Unit* u)	{ UnitSpec::Compute_Act(u); }
  virtual void 	Compute_Act(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, int cycle);
  // compute the final activation

  float		Compute_IThresh(LeabraUnit* u);
  // compute inhibitory value that would place unit directly at threshold

  virtual void	DecayState(LeabraUnit* u, LeabraLayer* lay, float decay);
  // decay activation states towards initial values

  virtual void	PhaseInit(LeabraUnit* u, LeabraLayer* lay, int phase);
  // initialize external input flags based on phase
  virtual void	PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			   int phase, bool set_both=false);
  // set stuff after settling is over (set_both = both _m and _p for current)

  virtual bool	AdaptThreshTest(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // compute long-term average activation, and test if thresh needs adapting
  virtual void	AdaptThresh(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr);
  // adapt the firing threshold based on long-term average activation

  virtual void	Compute_dWt_impl(LeabraUnit* u, LeabraLayer* lay);
  // actually do wt change
  void		Compute_dWt(Unit*)	{ };
  virtual void	Compute_dWt(LeabraUnit* u, LeabraLayer* lay);

  virtual void	Compute_dWt_post(LeabraUnit* u, LeabraLayer* lay);
  // after computing the weight changes (for recon delta)

  //  void		AdaptThresh(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, int phase);

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

class LeabraUnitChans : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER channels used in Leabra units
public:
  float		l;		// #NO_VIEW leak
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
  float		act_eq;		// #NO_SAVE equilibrium activity value (accumulates effects of spikes)
  float		act_avg;	// average activation over long time intervals
  float		act_m;		// minus_phase activation
  float		act_p;		// plus_phase activation
  float		act_dif;	// difference between plus and minus phase acts
  float		da;		// #NO_SAVE delta activation
  VChanBasis	vcb;		// voltage-gated channel basis variables
  LeabraUnitChans gc;		// #NO_SAVE current unit channel conductances
  float		I_net;		// #NO_SAVE net current produced by all channels
  float		v_m;		// #NO_SAVE membrane potential
  float		thr;		// threshold for firing (adaptive)

  bool		in_subgp;	// #READ_ONLY #NO_SAVE determine if unit is in a subgroup
  float		clmp_net;	// #NO_VIEW #NO_SAVE hard-clamp net input (no need to recompute)
  float		net_scale;	// #NO_VIEW #NO_SAVE total netinput scaling basis
  float		prv_net;	// #NO_VIEW #NO_SAVE previous net input (for time averaging)
  float		prv_g_i;	// #NO_VIEW #NO_SAVE previous inhibitory cond val

  void		InitDelta() { net = gc.i = 0.0f; }
  void		InitState(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->InitState(this, lay); }
  void		InitThresh()
  { ((LeabraUnitSpec*)spec.spec)->InitThresh(this); }

  void		ReConnect_Load(LeabraLayer* lay)	// #IGNORE
  { ((LeabraUnitSpec*)spec.spec)->ReConnect_Load(this, lay); }
       
  void		CompToTarg(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->CompToTarg(this, lay); }
  void		Compute_NetScaleClamp(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetScaleClamp(this, lay); }

  void		Compute_Net(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Compute_Net(this, lay); }
  void		Compute_NetAvg(LeabraLayer* lay, LeabraInhib* thr)
  { ((LeabraUnitSpec*)spec.spec)->Compute_NetAvg(this, lay, thr); }
  void		Compute_InhibAvg(LeabraLayer* lay, LeabraInhib* thr)
  { ((LeabraUnitSpec*)spec.spec)->Compute_InhibAvg(this, lay, thr); }
  void		Compute_HardClamp(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_HardClamp(this, lay); }
  bool		Compute_SoftClamp(LeabraLayer* lay) 
  { return ((LeabraUnitSpec*)spec.spec)->Compute_SoftClamp(this, lay); }

  void		Compute_Act()	{ Unit::Compute_Act(); }
  void 		Compute_Act(LeabraLayer* lay, LeabraInhib* thr, int cycle) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_Act(this, lay, thr, cycle); }

  float		Compute_IThresh()
  { return ((LeabraUnitSpec*)spec.spec)->Compute_IThresh(this); }

  void		PhaseInit(LeabraLayer* lay, int phase)
  { ((LeabraUnitSpec*)spec.spec)->PhaseInit(this, lay, phase); }

  void		DecayState(LeabraLayer* lay, float decay)
  { ((LeabraUnitSpec*)spec.spec)->DecayState(this, lay, decay); }
  void		PostSettle(LeabraLayer* lay, LeabraInhib* thr, int phase, bool set_both=false)
  { ((LeabraUnitSpec*)spec.spec)->PostSettle(this, lay, thr, phase, set_both); }
  bool		AdaptThreshTest(LeabraLayer* lay, LeabraInhib* thr)
  { return ((LeabraUnitSpec*)spec.spec)->AdaptThreshTest(this, lay, thr); }
  void		AdaptThresh(LeabraLayer* lay, LeabraInhib* thr)
  { ((LeabraUnitSpec*)spec.spec)->AdaptThresh(this, lay, thr); }

  void 		Compute_dWt(LeabraLayer* lay)
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt(this, lay); }	  

  void 		Compute_dWt_post(LeabraLayer* lay) 
  { ((LeabraUnitSpec*)spec.spec)->Compute_dWt_post(this, lay); }

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

class LeabraDecays : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds decay values
public:
  float		event;  // how much to decay between events
  float		phase;  // how much to decay between phases
  float		ae;
  // how much to decay for auto-encoder mode (between plus and nothing phases)

  void	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const LeabraDecays& cp) { event = cp.event; phase = cp.phase; ae = cp.ae; }
  COPY_FUNS(LeabraDecays, taBase);
  TA_BASEFUNS(LeabraDecays);
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
  float		pat_q;		// #HIDDEN threshold for pat_k based activity level
  float		adth_pct;	// #HIDDEN what pct of k units can adapt threshold at one time

  void	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(KWTASpec);
  COPY_FUNS(KWTASpec, taBase);
  TA_BASEFUNS(KWTASpec);
};

class KWTAVals : public taBase {
  // #INLINE ##NO_TOKENS #NO_UPDATE_AFTER holds values for kwta stuff
public:
  int		k;       	// target number of active units for this collection
  float		pct;		// actual percent activity in group
  float		pct_c;		// #HIDDEN complement of (1.0 - ) actual percent activity in group
  int		adth_k;		// #HIDDEN adapting threshold k value -- how many units can adapt per time

  void		Compute_Pct(int n_units);
  void		Compute_AdthK(float adth_pct);

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
  float		stdev;		// inhibition due to standard deviation model
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

class LeabraLayerSpec : public LayerSpec {
  // Leabra layer specs, computes inhibitory input for all units in layer
public:
  enum Compute_I {		// how to compute the inhibition
    KWTA_INHIB,			// between thresholds of k and k+1th
    KWTA_AVG_INHIB,		// average of top k vs avg of rest
    STDEV_INHIB,		// as function of standard deviation of inhib within layer
    SACT_INHIB,			// from sending activation
    RACT_INHIB,			// from recurrent activation
    SRACT_INHIB,		// from sending and recurrent activation
    UNIT_INHIB,			// unit-based inhibition (g_i from netinput)
  };

  enum InhibGroup {
    ENTIRE_LAYER,		// treat entire layer as one inhibitory group (even if subgroups exist)
    UNIT_GROUPS,		// treat sub unit groups as separate inhibitory groups
    LAY_AND_GPS			// compute inhib over both groups and whole layer
  };
  
  enum LayerLinking {
    NO_LINK,			// no linking
    LINK_INHIB			// link the inhibition
  };

  KWTASpec	kwta;		// how to calculate the desired activity level
  KWTASpec	gp_kwta;	// desired activity level for the unit groups (if applicable)
  Compute_I	compute_i;	// how to compute the inhibition
  float		i_kwta_pt;	// point to place inhibition between k and k+1 for kwta
  float		i_stdev_gain;	// gain of pct term to get inhib from area under normal above thresh
  ActInhibSpec 	i_sact;		// parameters for sending act based inhbition
  ActInhibSpec 	i_ract;		// parameters for recurrent act based inhbition
  bool		hard_clamp;	// whether to hard clamp any inputs to this layer or not
  float		stm_gain;	// minimum stimulus gain factor (when not hard clamping)
  float		d_stm_gain;	// delta to increase when target units not > .5
  LeabraDecays	decay;		// how much to decay the prior (towards initial_act)
  InhibGroup	inhib_group;	// what to consider the inhibitory group (layer or subgroups)
  LayerLinking	layer_link;	// how to link the layers (if links exist)
  float		layer_link_gain;
  // gain of the layer linking function, how much layer's thresh depends on others
  LeabraSort	adapt_thr_list;	// #HIDDEN #NO_SAVE #NO_INHERIT list of units needing thresh adapt

  virtual void	InitThresh(LeabraLayer* lay);
  // initialize the adapting threshold state values

  //////////////////////////////////////////
  //	Stage 0: at start of settling	  // 
  //////////////////////////////////////////

  virtual void	Compute_HardClamp(LeabraLayer* lay, int phase);
  // prior to settling: hard-clamp inputs
  virtual int	Compute_InputDist(LeabraLayer* lay, int phase);
  // prior to settling: compute layer's distance from input
  virtual void	Compute_Cascade(LeabraLayer* lay, int phase, int depth);
  // compute cascading of activation based on target depth from input
  virtual void	Compute_NetScaleClamp(LeabraLayer* lay, int phase);
  // prior to settling: compute netinput scaling values and input from hard-clamped

  //////////////////////////////////
  //	Stage 1: netinput 	  //
  //////////////////////////////////

  virtual void 	Compute_Net(LeabraLayer* lay, int cycle, int phase);
  // compute net inputs

  ////////////////////////////////////////////////////////////////
  //	Stage 2: netinput averages and clamping (if necc)	//
  ////////////////////////////////////////////////////////////////

  virtual void	Compute_Clamp_NetAvg(LeabraLayer* lay, int cycle, int phase);
  // clamp and compute averages of net inputs that were already computed
  virtual void	Compute_NetAvg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
			       int cycle, int phase);
  virtual void	Compute_sAct(LeabraLayer* lay, int cycle, int phase);
  // compute avg and max of sending activations into layer
  virtual void	Compute_SoftClamp(LeabraLayer* lay, int cycle, int phase);
  // soft-clamp inputs by adding to net input

  ////////////////////////////////////////
  //	Stage 3: inhibition		//
  ////////////////////////////////////////

  virtual void	InitInhib(LeabraLayer* lay);
  // initialize the inhibitory state values
  virtual void	Compute_Inhib(LeabraLayer* lay, int cycle, int phase);
  // stage two: compute the inhibition for layer
  virtual void	Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of inhibition computation for either layer or unit group
  virtual void	Compute_Inhib_kWTA(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of kwta inhibition computation
  virtual void	Compute_Inhib_StDev(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of stdev inhibition computation
  virtual void	Compute_Inhib_sAct(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of sending activation based inhibition
  virtual void	Compute_Inhib_rAct(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of recurrent activation based inhibition
  virtual void	Compute_Inhib_Unit(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of inhibition based on inhibitory unit input
  virtual void	Compute_Inhib_kWTA_Avg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr);
  // implementation of kwta avg inhibition computation
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

  virtual void	PhaseInit(LeabraLayer* lay, int phase);
  // initialize start of a setting phase, set input flags appropriately, etc

  virtual void	DecayEvent(LeabraLayer* lay);
  virtual void	DecayPhase(LeabraLayer* lay);
  virtual void	DecayAE(LeabraLayer* lay);

  virtual void	CompToTarg(LeabraLayer* lay);
  // change comparison external inputs into target external inputs
  virtual void	PostSettle(LeabraLayer* lay, int phase, bool set_both=false);
  // after settling, keep track of phase variables, etc.
  virtual void	AdaptThreshold(LeabraLayer* lay);
  // adapt the thresholds of units based on average activity

  ////////////////////////////////////////
  //	Stage 6: Learning 		//
  ////////////////////////////////////////

  virtual void	Compute_dWt_post(LeabraLayer* lay);
  // something to be done after the weights are changed (i.e. update history..)

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

class LeabraContextLayerSpec : public LeabraLayerSpec {
  // context layer that copies from its recv projection (like an input layer)
public:
  float		hysteresis;	 // hysteresis factor: (1-hyst)*new + hyst*old
  float		hysteresis_c;	 // #READ_ONLY complement of hysteresis

  void	Compute_HardClamp(LeabraLayer* lay, int phase);
  // clamp from act_p values of sending layer

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy()		{ };
  SIMPLE_COPY(LeabraContextLayerSpec);
  COPY_FUNS(LeabraContextLayerSpec, LeabraLayerSpec);
  TA_BASEFUNS(LeabraContextLayerSpec);
};


class LeabraInhib {
  // holds threshold-computation values, used as a parent class for layers, etc
public:
  LeabraSort 	active_buf;	// #HIDDEN #NO_SAVE list of active units
  LeabraSort 	inact_buf;	// #HIDDEN #NO_SAVE list of inactive units

  AvgMaxVals	netin;		// net input values for the layer
  AvgMaxVals	acts;		// activation values for the layer
  AvgMaxVals	acts_p;		// plus-phase activation stats for the layer
  AvgMaxVals	acts_m;		// minus-phase activation stats for the layer
  AvgMaxVals	acts_dif;	// difference between p and m phases
  float		phase_dif_ratio; // phase-difference ratio (-avg / +avg)
 
  KWTAVals	kwta;		// values for kwta -- activity levels, etc
  InhibVals	i_val;		// inhibitory values
  AvgMaxVals	un_g_i;		// average and stdev (not max) values for unit inhib-to-thresh

  void	Inhib_SetVals(float val)	{ i_val.g_i = val; i_val.g_i_orig = val; }
  void	Inhib_ResetSortBuf() 		{ active_buf.size = 0; inact_buf.size = 0; }
  void	Inhib_InitState(LeabraLayerSpec* lay);
  void	Inhib_Initialize();
  void	Inhib_Copy_(const LeabraInhib& cp);
};

class LeabraLayer : public Layer, public LeabraInhib {
  // Leabra Layer: implicit inhibition for soft kWTA behavior
public:
  AvgMaxVals	sact;		// sending activation values (avg and max, weighted by wt_scale)
  LeabraLayerSpec_SPtr	spec;	// the spec for this layer
  LayerLink_List layer_links;	// list of layers to link inhibition with
  float		stm_gain;	// actual stim gain (failsafe stm_gain for soft clamping)
  bool		hard_clamped;	// this layer is actually hard clamped
  int	        input_dist;	// number of layers away from external input

  void	Build();
  void	InitState();  	// initialize layer state too
  void	InitWtState();
  void	InitThresh() 	{ spec->InitThresh(this); } // initialize adapting thresholds
  void	InitInhib() 	{ spec->InitInhib(this); } // initialize inhibitory state
  void	ModifyState()	{ spec->DecayEvent(this); } // this is what modify means..
	
  void	ReConnect_Load(); // #IGNORE also call stuff at unit level

  void	Compute_HardClamp(int phase)  	{ spec->Compute_HardClamp(this, phase); }
  int	Compute_InputDist(int phase)	{ return spec->Compute_InputDist(this, phase); }
  void	Compute_Cascade(int phase, int depth) { spec->Compute_Cascade(this, phase, depth); }
  void	Compute_NetScaleClamp(int phase) { spec->Compute_NetScaleClamp(this, phase); }

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

  void	PhaseInit(int phase)		{ spec->PhaseInit(this, phase); }
  void	DecayEvent()	{ spec->DecayEvent(this); } // decay between events
  void	DecayPhase()    { spec->DecayPhase(this); } // decay between phases
  void	DecayAE()  	{ spec->DecayAE(this); }    // decay for auto-encoder

  void	CompToTarg()		{ spec->CompToTarg(this); }
  void	PostSettle(int phase, bool set_both=false)	{ spec->PostSettle(this, phase, set_both); }

  void	Compute_dWt();
  void	Compute_dWt_post()	{ spec->Compute_dWt_post(this); }

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

void LeabraConSpec::C_InitWtState_post(Con_Group* cg, Connection* cn, Unit*, Unit*) {
  C_Compute_LinFmWt((LeabraCon_Group*)cg, (LeabraCon*)cn);
}

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

void LeabraConSpec::C_Send_Inhib(LeabraCon_Group* cg, LeabraCon* cn, LeabraUnit* ru,
				 LeabraUnit* su) {
  ru->gc.i += ((LeabraCon_Group*)ru->recv.Gp(cg->other_idx))->scale_eff * su->act * cn->wt;
}
void LeabraConSpec::Send_Inhib(LeabraCon_Group* cg, LeabraUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_Inhib((LeabraCon_Group*)cg, (LeabraCon*)cg->Cn(i), 
				  (LeabraUnit*)cg->Un(i), su));
}

//////////////////////////
//     Computing dWt 	//
//////////////////////////

inline void AvgCorSpec::Compute_AvgCor(float& cg_avg, float& cg_cor, float fx_avg,
				       LeabraCon_Group* cg, LeabraLayer* lay)
{
  if(src == LAYER_TRG_PCT)
    cg_avg = lay->kwta.pct;
  else if(src == LAYER_AVG_ACT)
    cg_avg = lay->acts_p.avg;
  else if(src == COMPUTED_AVG) {
    cg_avg = 0.0f;
    for(int i=0; i<cg->size; i++)
      cg_avg += ((LeabraUnit*)cg->Un(i))->act_p;
    if(cg->size > 0)
      cg_avg /= (float)cg->size;
  }
  else
    cg_avg = fx_avg;
  // apply amount of correction specified
  float avgc = .5f + cor * (cg_avg - .5f);
  avgc = MAX(thresh, avgc); // keep this computed value within bounds
  cg_cor = .5f / avgc;
}

inline float LeabraConSpec::C_Compute_Hebb(LeabraCon* cn, LeabraCon_Group* cg,
					   float ru_act, float su_act)
{
  // symmetric!
  return .5f * (ru_act * (su_act * (cg->savg_cor - cn->wt_lin) -
			  (1.0f - su_act) * cn->wt_lin) +
		su_act * (ru_act * (cg->ravg_cor - cn->wt_lin) -
			  (1.0f - ru_act) * cn->wt_lin));
}

// generec error term with sigmoid activation function, and soft bounding
inline float LeabraConSpec::C_Compute_Err
(LeabraCon* cn, float ru_act_p, float ru_act_m, float su_act_p, float su_act_m) {
  float err = (ru_act_p * su_act_p) - (ru_act_m * su_act_m);
  if(err > 0.0f)	err *= (1.0f - cn->wt_lin);
  else			err *= cn->wt_lin;	
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
  LeabraLayer* fmlay = (LeabraLayer*)cg->prjn->from;
  LeabraLayer* tolay = (LeabraLayer*)cg->prjn->layer;
  savg_cor.Compute_AvgCor(lcg->savg, lcg->savg_cor, fix_savg.avg, lcg, fmlay);
  ravg_cor.Compute_AvgCor(lcg->ravg, lcg->ravg_cor, fix_ravg.avg, lcg, tolay);
  if(fmlay->acts_p.avg >= savg_cor.thresh) {
    for(int i=0; i<cg->size; i++) {
      LeabraUnit* su = (LeabraUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh)))
	C_Compute_dWt(cn,
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m));  
    }
  }
}

inline void LeabraConSpec::C_UpdateWeights(LeabraCon* cn, LeabraCon_Group* cg,
					   LeabraUnit*, LeabraUnit*)
{
  if(cn->dwt != 0.0f) {
    cn->wt_lin += lrate * cn->dwt;
    cn->wt_lin = MAX(cn->wt_lin, 0.0f);	// limit 0-1
    cn->wt_lin = MIN(cn->wt_lin, 1.0f);
    C_Compute_WtFmLin(cg, cn);
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
//	Unit NetAvg   	//
//////////////////////////

void LeabraUnitSpec::Compute_NetAvg(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  u->net *= g_bar.e;		// multiply by gbar..
  u->net = u->prv_net + net_dt * (u->net - u->prv_net);
  u->prv_net = u->net;
}

void LeabraUnitSpec::Compute_InhibAvg(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  u->gc.i *= g_bar.i;		// multiply by gbar..
  u->gc.i = u->prv_g_i + net_dt * (u->gc.i - u->prv_g_i);
  u->prv_g_i = u->gc.i;
}


//////////////////////////
// 	Processes	//
//////////////////////////

class LeabraCycle : public CycleProcess {
  // Leabra cycle of activation updating
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
  // Leabra settling phase, iterates over cycles of updating
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
  virtual void	Compute_NetScaleClamp();
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
  // Leabra trial process, iterates over phases of settling
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
    PLUS_NOTHING		// just the auto-encoder (no initial minus phase)
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

  virtual void	InitThresh();
  // #BUTTON #CONFIRM initialize the adapting activation thresholds

  virtual void	InitState();
  virtual void	DecayState();
  virtual void	EncodeState();
  virtual void	Compute_dWt_post();
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

class LeabraMaxDa : public Stat {
  // ##COMPUTE_IN_SettleProcess ##LOOP_STAT stat that computes when equilibrium is
public:
  enum dAType {
    DA_ONLY,			// just use da
    INET_ONLY,			// just use inet
    INET_DA			// use inet if no activity, then use da
  };

  LeabraSettle* settle_proc;	// #READ_ONLY #NO_SAVE the settle process
  dAType	da_type;	// type of activation change measure to use
  float		inet_scale;	// how to scale the inet measure to be like da
  float		lay_avg_thr;	// threshold for layer average activation to switch to da fm Inet
  StatVal	da;		// absolute value of activation change

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
  SIMPLE_COPY(LeabraMaxDa);
  COPY_FUNS(LeabraMaxDa, Stat);
  TA_BASEFUNS(LeabraMaxDa);
};

class LeabraAeSE_Stat : public SE_Stat {
  // squared error for leabra auto-encoder: targ = minus, comp = nothing, both = both
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
    PREV_PLUS_MINUS,		// use previous plus phase, only present minus
    PREV_MINUS_PLUS		// use previous minus phase, only present plus
  };

  PhaseOrder	phase_order;	// order to present phases of stimuli to network

  void	Initialize();
  void	Destroy() 	{ };
  void	Copy_(const PhaseOrderEventSpec& cp);
  COPY_FUNS(PhaseOrderEventSpec, EventSpec);
  TA_BASEFUNS(PhaseOrderEventSpec);
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
    return ru_act * (su_act * (max_cor - cn->wt_lin) -
		     (1.0f - su_act) * cn->wt_lin);
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

  // todo: this is not right...
  void		EncodeState(LeabraUnit*)  { p_savg = savg; p_max_cor = p_max_cor; }

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
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh)))
	C_Compute_dWt(cn, 
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->p_act_p, lcg->p_max_cor),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->p_act_p, su->p_act_m),
		      k_prv);
    }
  }
  // this is for the sender_(t), recv_(t) weight change
				//  todo: fix this: Compute_SAvgCor(lcg, lru);
  float use_k_cur = k_cur;
  if(lru->p_act_p < 0.0f)	use_k_cur = 1.0f; // use full current if no encoded
  if((((LeabraLayer*)cg->prjn->from)->acts_p.avg >= savg_cor.thresh) && (k_cur != 0.0f)) {
    for(int i=0; i<cg->size; i++) {
      LeabraTimeUnit* su = (LeabraTimeUnit*)cg->Un(i);
      LeabraCon* cn = (LeabraCon*)cg->Cn(i);
      if(!(su->in_subgp &&
	   (((LeabraUnit_Group*)su->owner)->acts_p.avg < savg_cor.thresh)))
	C_Compute_dWt(cn,	// todo: should be something other than p_max_cor..
		      C_Compute_Hebb(cn, lcg, lru->act_p, su->act_p, lcg->p_max_cor),
		      C_Compute_Err(cn, lru->act_p, lru->act_m, su->act_p, su->act_m),
		      use_k_cur);
    }
  }
}

//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////


#endif // leabra_h
