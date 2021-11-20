/* -*- C++ -*- */ 
/*============================================================================= 
// 
// This file is part of the RT++ software package, which is                   // 
// Copyright (C) 2002 Joshua W. Brown and Washington University in St. Louis  // 
//									      // 
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
//      								      // 
// This file is derived from the PDP++ software package, which is             // 
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      // 
//		      James L. McClelland, and Carnegie Mellon University     // 
//     									      // 
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
// IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY OR WASHINGTON UNIVERSITY      // 
// IN ST. LOUIS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR            // 
// CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING     // 
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE           // 
// POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR   // 
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 		      // 
==============================================================================*/ 
 
#define DEBUG 0
 
#ifdef DEBUG 
#define DM(a) {if (DEBUG) {printf(a); fflush(stdout);}} 
#define DM2(a,b) {if (DEBUG) {printf(a,b); fflush(stdout);}} 
#else 
#define DM(a) ; 
#define DM2(a,b) ; 
#endif 
 
#define RT 1 
#define RT_1_0  // version control 
 
#ifndef Rt_h 
#define Rt_h 
 
#include <stdio.h> 
#include <unistd.h>
#include <math.h> 
#include <float.h> 
#include "rns_TA_type.h" 
 
#include <pdp/base.h> 
#include <pdp/netstru.h> 
#include <pdp/sched_proc.h> 
#include <pdp/declare.h> 
#include <pdp/pdpshell.h> 
#include <pdp/procs_extra.h> 
 
class RtCon; 
class RtCon_Group; 
class RtUnit; 
class ModelParam; //holds pointers to parameters of model 
class RtUnitSpec; 
class RtTrial; 
class float_DelayBuffer; 
class RtEvent; 
 
// Macros 
#define MMIN(a,b)       ((a) < (b) ? (a) : (b)) 
#define MMAX(a,b)       ((a) > (b) ? (a) : (b)) 
#define MPLUS(a)        MMAX(a,0) 
//#define MSQUELCH(a,b)   ((a) > (b) ? (a) : 0) 
#define MSQUELCH(a,b)   MPLUS((a) > (b) ? (a) : 0.000001*a) //better param fitting 
 
 
/////////////////////////////// 
// Circular Buffer           // 
/////////////////////////////// 
 
class float_DelayBuffer : public float_Array { 
 // Circular buffer for holding state information 
public: 
  //int		st_idx;		// starting index 
  //int		length;		// logical length of the list 
  int           head;           // JWB -- most recent datum index 
  int           max_len;        // JWB -- longest lag 
 
  void          SetSize(int sz); 
 
  void  PushHead(const float& item) {Set(head++, item); if (head >=max_len) head-=max_len; 
} //JWB 
  float  ReadDelayed(int aDelay) {int idx = head-aDelay; if (idx < 0) idx+=max_len;  
return SafeEl(idx); 
} 
 
  void  Reset(); 
  void 	Initialize(); 
  void	Destroy()		{ }; 
  void 	Copy_(const float_DelayBuffer& cp); 
  COPY_FUNS(float_DelayBuffer, float_Array); 
  TA_BASEFUNS(float_DelayBuffer); 
}; 
 
// example for inline fields 
//  class WtScaleSpec : public taBase { 
//    // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER weight scaling specification 
//  public: 
//    float		abs;		// #DEF_1 absolute scaling (not subject to normalization: directly multiplies weight values) 
//    float		rel;		// [Default: 1] relative scaling (subject to normalization across all other projections into unit) 
 
//    inline float	NetScale() 	{ return abs * rel; } 
 
//    void	Initialize(); 
//    void	Destroy()	{ }; 
//    SIMPLE_COPY(WtScaleSpec); 
//    COPY_FUNS(WtScaleSpec, taBase); 
//    TA_BASEFUNS(WtScaleSpec); 
//  }; 
 
 
/////////////////////////////// 
// Units, Unit Specs         // 
/////////////////////////////// 
 
//#ifdef RT_1_0 
#define BASEFUNS(newclass, base) SIMPLE_COPY(newclass); COPY_FUNS(newclass,base); TA_BASEFUNS(newclass)
 
class NeuroModInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool used;  //Will this be applied? 
  float ex_wt; //#CONDEDIT_ON_used:true multiply signal by this wt and add as excitation 
  float ex_mult; //#CONDEDIT_ON_used:true multiply net_excit by (1+ex_mult*signal)
  float inh_wt; //#CONDEDIT_ON_used:true multiply signal by this wt and add as inhibition 
  float inh_mult; //#CONDEDIT_ON_used:true multiply net_inhib by (1+inh_mult*signal) 
 
  void Initialize () {used = false; ex_wt = 0.0; inh_wt = 0.0; ex_mult=0.0; inh_mult = 0.0;}; 
  void Destroy () {}; 
 
  BASEFUNS(NeuroModInfo, taBase); 
}; 
 
class NMDAInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool used;  //Compute NMDA effects? 
  float NMDA_gain; //#CONDEDIT_ON_used:true multiply net_excit by this if act above thresh 
  float NMDA_thresh; //#CONDEDIT_ON_used:true act threshold for NMDA gain 
 
  void Initialize () {used = false; NMDA_gain = 1.0f; NMDA_thresh = 0.0f;}; 
  void Destroy () {}; 
 
  BASEFUNS(NMDAInfo, taBase); 
}; 
 
class HabitInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool used;  //Will this be applied? 
  float rate; //#CONDEDIT_ON_used:true rate of habituation 
  bool time_const; //#CONDEDIT_ON_used:true overall rate of habituation,recovery 
  float thresh; //#CONDEDIT_ON_used:true activity must exceed thresh to initiate habituation 
 
  void Initialize () {used = false; rate = 1.0f; time_const = 100.0f; thresh = 0.0;}; 
  void Destroy () {}; 
 
  BASEFUNS(HabitInfo, taBase); 
}; 
 
class ActInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  float time_const; //overall rate of change 
  float pass_decay;  //rate of passive decay
  float bias_excit;  //bias excitation signal
  float hyperpol; //lower asymptote (for shunting equation) 
 
  void Initialize () {time_const = 100.0f; pass_decay=1.0f; bias_excit = 0.0f; hyperpol = 0.5f;}; 
  void Destroy () {}; 
 
  BASEFUNS(ActInfo, taBase); 
}; 
 
class ActTraceInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool used; //Will activity trace be computed?
  float rise_rate; //#CONDEDIT_ON_used:true how quickly does trace rise?
  float decay_rate; //#CONDEDIT_ON_used:true how quickly does trace decay?
 
  void Initialize () {used = false; rise_rate = 100.0f; decay_rate = 1.0f;}; 
  void Destroy () {}; 
 
  BASEFUNS(ActTraceInfo, taBase); 
}; 
 
class RecExcInfo : public taBase {  
  // ##INLINE ##NO_TOKENS  inline class to keep track of neuromodulator effects 
public: 
  enum RecurExcitType {  // form of recurrent excitation signal function 
    NONE,                // No recurrent excitation 
    LINEAR,              // f(x) = n*(x-a) if (x-a > 0), else 0 
    STEP,                // f(x) = n if (x >= a), else 0 
    SQUELCH,             // f(x) = nx if (x >=a), else 0 
    XSQUARED,            // f(x) = nx^2 
    SIGMOID              // f(x) = x^n/(x^n+a^n) 
  }; 
 
  bool used; //Compute recurrent excitation term?
  RecurExcitType type;  //#CONDEDIT_ON_used:true  
  float n; //#CONDEDIT_ON_used:true  gain
  float a; //#CONDEDIT_ON_used:true  level
  float wt; //#CONDEDIT_ON_used:true  strength of recurrent excitation
  float re_a_pow_n;  //#HIDDEN #READONLY a^n, for recur_excit sigmoid 
 
  void UpdateAfterEdit() {re_a_pow_n = pow(a,n);  taBase::UpdateAfterEdit();}; 
  void Initialize (){used = false; type = NONE; n = 1.0f; a = 0.0f; wt = 1.0f;}; 
  void Destroy () {}; 
 
  BASEFUNS(RecExcInfo, taBase); 
}; 
 
class SoftClampInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool used; //Should input be soft-clamped?
  float gain; //#CONDEDIT_ON_used:true wt of external input if soft-clamped 
  void Initialize () {used = false; gain = 1.0f;}; 
  void Destroy () {}; 
  BASEFUNS(SoftClampInfo, taBase); 
}; 
 
class NormInfo : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of neuromodulator effects 
public: 
  bool input; //Normalize input weights to a unit?
  bool output; //Normalize output weights from a unit?
  void Initialize () {input = false; output = false;}; 
  void Destroy () {}; 
  BASEFUNS(NormInfo, taBase); 
}; 

class SpikeInfo : public taBase {
   // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of spiking properties
public:
  bool doIntAndFire; //Should cells do integrate-and-fire?
  float iafThresh; //#CONDEDIT_ON_doIntAndFire:true act threshold for spike
  void Initialize() {doIntAndFire = false; iafThresh = 0.2;}
  void Destroy() {};
  BASEFUNS(SpikeInfo, taBase);
};
 
//#endif // #define RT_1_0 
 
class RandLearnInfoSpec : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline class to keep track of postsynaptic learning threshold changes for random sample learning 
public: 
  bool on;  //Should RtUnit sample input stochastically to learn? 
  float prob; //#CONDEDIT_ON_on:true Probability of learning event per cell per second? (e.g. 0.01) 
  float dur; //#CONDEDIT_ON_on:true duration of sampling in seconds 
  float dpost_l; //#CONDEDIT_ON_on:true How much to lower postsynaptic learning threshold? 
 
  void Initialize () {on = false; prob = 0.01; dur = 0.2; dpost_l = 0.3;}; 
  void Destroy () {}; 
  SIMPLE_COPY(RandLearnInfoSpec); 
  COPY_FUNS(RandLearnInfoSpec, taBase); 
  TA_BASEFUNS(RandLearnInfoSpec); 
}; 
 
class VectAssocMapInfo : public taBase { 
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER  if active, causes cells to respond only to certain *ranges* of net_input intensity. (e.g. If response desired ONLY if net_excit in [0.4, 0.6], set peak = 0.5 and hwidth = 0.1)  Creates symmetric triangular tuning curve. 
public: 
  bool active; //Should the unit be selective for a certain intensity range? 
  /* 
  float lower_thresh; //#CONDEDIT_ON_active:true subtract lower_thresh from net_excit and positive rectify 
  float lower_gain;  //#CONDEDIT_ON_active:true multiplies the excitation above lower_thresh 
  float upper_thresh; //#CONDEDIT_ON_active:true subtract upper_thresh from net_excit and positive rectify -- this will inhibit the cell by adding to net_inhib 
  float upper_gain; //#CONDEDIT_ON_active:true multiplies the inhibition 
  */ 
  float peak;  //#CONDEDIT_ON_active:true what net_excit to respond to maximally? 
  float hwidth;//#CONDEDIT_ON_active:true half width of the tuning curve 
  float gain;  //#CONDEDIT_ON_active:true how strongly cell responds to input in range 
 
  void Initialize() {active = false; peak = 0.1; hwidth = 0.02; gain = 200.0;  /* lower_thresh = 0.0; lower_gain = 5.0; upper_thresh = 1.0; upper_gain = 10.0;*/}; 
  void Destroy() {}; 
 
  SIMPLE_COPY(VectAssocMapInfo); 
  COPY_FUNS(VectAssocMapInfo, taBase); 
  TA_BASEFUNS(VectAssocMapInfo); 
}; 
 
class RtUnitSpec : public UnitSpec { 
  // Real-Time Unit Specification 
public: 
 
  float          (*govern_eq)(RtUnitSpec* spec, RtUnit* u);  // #LIST_RtConSpecGovEq  
  Script custGovEq; //#CONDEDIT_ON_govern_eq:Rt_Cust_Script  Script for custom objective function -- script should find RtUnitSpec *spec = *owner; access unit as spec->custom_script_u; then set spec->custom_script_da = return value of the equation.
  RtUnit *custom_script_u; //#HIDDEN #READ_ONLY only used with script governing eqn to hold reference to current RtUnit
  float custom_script_da; //#HIDDEN #READ_ONLY only used with script governing eqn's to hold return values.

//#ifdef RT_1_0 
  ActInfo gov;    //governing equation parameters 
  RecExcInfo rec_excit; //recurrent excitation parameters 
 
  NMDAInfo     nmda;  //NMDA receptors
  NeuroModInfo dopa;  //Dopamine 
  NeuroModInfo ne;  //Norepinephrine 
  NeuroModInfo ach; //Acetylcholine 
  NeuroModInfo sero; //Serotonin 
 
  ActTraceInfo act_trace; //Keeps a slowly-decaying trace of max cell activity 
  HabitInfo habit;  //Habituation -- internal var decays with activity 
  SoftClampInfo softclamp; //If external input is soft-clamped 
  NormInfo wt_norm;  //whether to normalize *excitatory* input and output weights 
   
#ifndef RT_1_0 //else 
  enum RecurExcitType {  // form of recurrent excitation signal function 
    NONE,                // No recurrent excitation 
    LINEAR,              // f(x) = n*(x-a) if (x-a > 0), else 0 
    STEP,                // f(x) = n if (x >= a), else 0 
    SQUELCH,             // f(x) = nx if (x >=a), else 0 
    XSQUARED,            // f(x) = nx^2 
    SIGMOID              // f(x) = x^n/(x^n+a^n) 
  }; 
 
  float         re_a_pow_n;  //#HIDDEN #READONLY a^n, for recur_excit sigmoid 
  float         NMDA_thresh; //voltage above which NMDA channels unblock 
  float         NMDA_gain;   //multiplicative effect of NMDA unblocking on input (squelch effect) 
  float         ne_gain; //input gain due to noradrenergic input 
  float         ne_inhib_gain; //gain for additive inhibition from NE 
  float         pass_decay; //Passive decay rate 
  float         bias_excit; //Baseline bias of excitation 
  float         hyperpol; //Hyperpolarization potential 
  float         time_const; //rate of response 
  bool          calc_habituation; //use habituation of postsynaptic eligibility? 
  float         habit_rate;  // #CONDEDIT_ON_calc_habituation:true rate habituation variable declines (habituates) 
  float         habit_time_const; //#CONDEDIT_ON_calc_habituation:true overall rate of change 
  float         habit_thresh;    //#CONDEDIT_ON_calc_habituation:true rectify activity minus this thresh 
  bool          calc_act_trace; //if true, act_trace is slowly-decaying record or recent max activity in unit 
  float         at_rise_rate;  //#CONDEDIT_ON_calc_act_trace:true rate of act_trace tracking upward to current activity 
  float         at_decay_rate;  //#CONDEDIT_ON_calc_act_trace:true rate of act_tracepassive decay to zero 
  bool          doRecurExcit; // Whether to enable recurrent excitation 
  RecurExcitType recur_excit_type; //Type of recurrent excitation 
  float         recur_excit_gain_n; //#CONDEDIT_OFF_recur_excit_type:NONE gain of recurrent excitation transfer function 
  float         recur_excit_level_a; //#CONDEDIT_OFF_recur_excit_type:NONE,XSQUARED threshold of recurrent excitation 
  float         recur_excit_wt;  //#CONDEDIT_OFF_recur_excit_type:NONE strength of recurrent excitation 
 
  bool          soft_clamp;     // should network use ext input as an excitatory input rather than set the unit activity to the value of the external input? 
  float          soft_clamp_gain; // #CONDEDIT_ON_soft_clamp:true multiplier of soft clamped input 
 
  bool          norm_input_wt;  // normalize sum of incoming excit. weights? 
  bool          norm_output_wt; // normalize sum of outgoing excit. weights? 
 
#endif //#define RT_1_0 ============== 
  bool          useDA_NMDA_gain; //DA potentiates NMDA? 
  float         DA_NMDA_gain; //multiplicative effect of DA on NMDA gain 
  float         D2_inhib_habit_thresh; //If (1-habit) > this thresh, then inhibit cell (rect thresh) 
  float         D2_inhib_habit_gain; //If (1-habit) > above thresh, then inhibit cell (rect thresh) with this gain 
 
  Random	initial_act;	 // initial activation value 
  bool          legacy_norm;    // #HIDDEN should set to false unless doing so breaks old sims 
 
  bool          addNoise;       // Should add noise to state var? 
  Random	noise;		// #CONDEDIT_ON_addNoise:true what kind of noise to add to activations 
  float		sqrt_dt; 	// #HIDDEN square-root of dt for noise 
  float		dt;		// #READ_ONLY #HIDDEN set by RtTrial::Run() grain of time computing on (must be in [0..1] range) 
 
  RandLearnInfoSpec samp_learn; //#INLINE supports learning representations by stochastic sampling of input 
#ifndef RT_1_0
  bool          doIntAndFire;    // Should the cell behave like an integrate-and-fire neuron? 
  float         iafThresh;       // #CONDEDIT_ON_doIntAndFire:true activity threshold for integrate-and-fire spike 
#endif
  SpikeInfo Spiking; //Can set cells to spike instead of rate coding

  VectAssocMapInfo  VectorMap;   //#INLINE Makes cells selective for only a specific intensity range of net_excit 
 
  void		InitState(Unit* u); 
  void          Prep_Compute_Net(RtUnit* u, float abs_time);    // Initialize state of net b4 iter 
  void		Compute_Net(Unit* u); 
  void          Compute_Wt_Norm(RtUnit *u); 
  virtual void	Compute_Act_impl(RtUnit* u); 
  void		Compute_Act(Unit* u); 
  void 		Compute_dWt(Unit* u); 
 
  void	        UpdateAfterEdit(); 
  void 	        Initialize(); 
  void	        Destroy()		{ CutLinks(); } 
  void	        InitLinks(); 
 
  SIMPLE_COPY(RtUnitSpec); 
  COPY_FUNS(RtUnitSpec, UnitSpec); 
  TA_BASEFUNS(RtUnitSpec); 
}; 
 
// The following functions are the various possible governing equations 
// #REG_FUN 
float Rt_Shunting(RtUnitSpec* spec, RtUnit* u) 
// #LIST_RtConSpecGovEq Full Shunting Equations 
     ;                            // term here so scanner picks up comment 
// #REG_FUN 
float Rt_Tracking(RtUnitSpec* spec, RtUnit* u) 
// #LIST_RtConSpecGovEq Unit follows linear sum of excit, inhib inputs only 
     ;                            // term here so scanner picks up comment 
// #REG_FUN 
float Rt_Additive(RtUnitSpec* spec, RtUnit* u) 
// #LIST_RtConSpecGovEq Unit simply adds excitation and inhibition 
     ;     
// #REG_FUN 
float Rt_Usher_Mc(RtUnitSpec* spec, RtUnit* u) 
// #LIST_RtConSpecGovEq Usher-McClelland Equations
     ;     
// #REG_FUN 
float Rt_Cust_Script(RtUnitSpec* spec, RtUnit* u) 
// #LIST_RtConSpecGovEq User-specified script governing eqn 
     ;  

float RtCalcRecExcit(RtUnitSpec* sp, RtUnit* u); 
void RtCalcNeuroMod(RtUnitSpec* sp, RtUnit* u, float &ex, float &exgain, float &in, float &ingain);
 
class RtUnit : public Unit { 
  // basic Rt unit 
public: 
  float		da;		// delta-activation (change in activation value) 
  float         net_excit;      // excitatory input to unit 
  float         net_inhib;      // inhibitory input to unit 
  float         net_d1;         // net dopamine D1 signal -- deprecated
  float         net_d2;         // net dopamine D2 signal -- deprecated
  float         net_ne;         // net norepinephrine signal 
  float         net_ach;        // net acetylcholine signal
  float         net_sero;       // net serotonin signal
  float         net_dopa;       // net dopamine signal
  float         sum_in_wt;      // sum of incoming excit. weights, for normaliz'n 
  float         sum_out_wt;     // sum of outgoing excit. weights, for normaliz'n 
  //int           num_in_wt;      // #NO_VIEW #HIDDEN how many norm'd cons from sending layers? 
  //int           num_out_wt;     // #NO_VIEW #HIDDEN how many norm'd cons to receiving layers? 
  float		prv_targ;	// #NO_VIEW #HIDDEN previous target value 
  float		prv_ext;	// #NO_VIEW #HIDDEN previous external input value 
  float		prv_act;	// #NO_VIEW #HIDDEN previous activation value 
  float		act_raw;	// #NO_VIEW current raw activation value 
  float		prv_net;	// #NO_VIEW #HIDDEN previous net-input value 
  float         habit;          //habituation or xmtr depletion -- decays to zero with cell activity, returns to maximum 1.0 with cell inactivity 
  float         act_trace;      // slowly-decaying, rapidly tracks maximum act 
 
  bool isLearnEvent; //#HIDDEN is a stochastic learning sample currently active? 
  float lEvStart;  //#HIDDEN when did the learning event start? 
 
  //These are for modeling axonal delays: 
  float_DelayBuffer ext_flags;	// #NO_VIEW array of external_flag values 
  float_DelayBuffer targs;	// #NO_VIEW array of target values 
  float_DelayBuffer exts;	// #NO_VIEW array of external input values 
  float_DelayBuffer acts;	//  array of activation values 
  bool          volt_clamp;    // artificially clamp act at current level 
  int           max_axon_delay; // longest possible axon delay to receipients, in ticks 
 
  void Prep_Compute_Net(float abs_time)     {((RtUnitSpec*)spec.spec)->Prep_Compute_Net(this, abs_time);} 
 
  void 		InitExterns();	// keep prv_values.. 
  virtual void	StoreState(); 
  void 	        Initialize(); 
  void          UpdateAfterEdit(); 
  void	        Destroy()	{ }; 
  void	        InitLinks(); 
  SIMPLE_COPY (RtUnit); //void	Copy_(const RtUnit& cp); 
  COPY_FUNS(RtUnit, Unit); 
  TA_BASEFUNS(RtUnit); 
}; 
 
 
 
/////////////////////////////// 
// Connections, ConSpec's    // 
/////////////////////////////// 
 
class RtCon : public Connection { 
  // Rt connection 
public: 
  float        dwt; 		// #NO_SAVE Change in weight 
  float        elig;            // #NO_SAVE Eligibility trace for connection 
  bool         isClamp;         // Is the weight fixed for data fitting? 
 
  void 	Initialize()		{ dwt = elig = 0.0f; isClamp = 0.0f;} 
  void 	Destroy()		{ }; 
  void	Copy_(const RtCon& cp)	{dwt = cp.dwt;  elig = cp.elig; isClamp = cp.isClamp;} 
  COPY_FUNS(RtCon, Connection); 
  TA_BASEFUNS(RtCon); 
}; 
 
//--- new revision data structures (1/30/03)---
class SigFuncInfo : public taBase {
   // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER signal function; defines threshold for presynaptic cells
public:
  enum SigFuncType {      // What is the nature of the signal function? 
    RECT         = 0x00, // subtract threshold, and set floor at zero 
    SQUELCH      = 0x01  // set input to zero if below threshold, else no change 
  }; 
  SigFuncType sig_type;  // What is the nature of the signal function?
  float thresh; //threshold of presynaptic activity; meaning depends on sig_type
  void Initialize() {sig_type = RECT; thresh = 0.0;}
  void Destroy() {};
  BASEFUNS(SigFuncInfo, taBase);
};

class EligTraceInfo : public taBase {
   // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER governs synaptic eligibility trace
public:
  bool used; //is eligibility trace computed?
  enum EligType { //What kind of law for eligibility traces? 
    TRACK_ONE,   // The eligibility trace rises to one, regardless of pre*post 
    TRACK_ONE_RAW_PRESYN, // use raw, not cooked presynaptic activity 
    SBA_ELIG      // Use Sutton, Barto, Anderson, eqn, which tracks pre*post 
  };
  EligType      elig_type;   // #CONDEDIT_ON_used:true Eligibility trace law 
  float rise; //#CONDEDIT_ON_used:true rate of eligibility increase
  float decay; //#CONDEDIT_ON_used:true rate of eligibility decrease
  float inhib; //#CONDEDIT_ON_used:true rate of eligibility decrease due to inhibitory input

  void Initialize() {used = false; elig_type = TRACK_ONE; rise = 1.0; decay = 1.0; inhib = 0.0;}
  void Destroy() {};
  BASEFUNS(EligTraceInfo, taBase);
};

class ModLearnInfo : public taBase {
   // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER governs modular learning laws
public:
  enum TermType { //factors for learning equations
    PRExPOST,     //product of pre- and post-synaptic activity
    PRE_ONLY,     //equal to pre-synaptic activity
    POST_ONLY,    //equal to post-synaptic activity
    ELIG,         //equal to synaptic eligibility
    ONE           //equal to unity
  };
  enum NMODType { //neuromodulators can gate learning
    NONE,         //Do not use neuromodulator gating
    DOPA,         //Dopamine
    SERO,         //Serotonin
    ACH,          //Acetylcholine
    NE            //Norepinephrine
  };
  enum aboveBelow { //is quantity above or below spec'd value?
    ABOVE,
    BELOW
  };

  float lim;      //What is the upper or lower bound?
  TermType lx;     //Is the bound modulated (multiplied by term)?
  float rate;     //How fast does synaptic wt approach bound?
  TermType rx;  //Is the rate modulated (multiplied by term)?
  NMODType allX;  //Should learning be gated by a neuromodulator?
  aboveBelow _ ; //#CONDEDIT_OFF_allX:NONE Only allow learning if neuromodulator above or below level?
  float val;     //#CONDEDIT_OFF_allX:NONE Learning rate multiplied by quantity above or below val

  void Initialize() {lim = 1.0; lx = ONE; rate = 1.0; rx = ONE; allX = NONE; _ = ABOVE; val = 0.0;  }
  void Destroy() {};
  BASEFUNS(ModLearnInfo, taBase);
};

class ModLearnDecayInfo : public taBase {
   // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER governs modular learning law passive decay
public:
  bool on;  //should weights decay passively?
  float rate; //#CONDEDIT_ON_on:true if so, at what rate?
  float baseline;  //#CONDEDIT_ON_on:true and toward what baseline?

  void Initialize() {on = false; rate = 1.0; baseline = 0.0; }
  void Destroy() {};
  BASEFUNS(ModLearnDecayInfo, taBase);
};

class ExcludeNormInfo : public taBase {
 // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER keeps track of which weights should be excluded from weight normalization
public:
  bool rwt; //should this connection be excluded from input normalization of postsynaptic weights?
  bool swt; //should this connection be excluded from output normalization of presynaptic weights?

  void Initialize() {rwt = false; swt = false; }
  void Destroy() {};
  BASEFUNS(ExcludeNormInfo, taBase);
};

class RtConSpec : public ConSpec { 
  // Rt connection specifications 
public: 
  // ----- New revision members (1/30/03):
  enum  ConType {        // what kind of connection is it? 
    UNUSED       = 0x00, // Connection has no effect 
    EXCIT        = 0x01, // Pure (Glut-ergic) excitation 
    INHIB        = 0x02, // Pure (GABA-ergic) inhibition 
    D1           = 0x03, // Dopaminergic signal, acts on postsynaptic D1 receptor 
    D2           = 0x04, // Dopaminergic signal, acts on postsynaptic D2 receptor 
    NE           = 0x05, // Noradrenergic signal
    DOPA         = 0x06, // Dopamine signal
    SERO         = 0x07, // Serotonin signal
    ACH          = 0x08, // Acetylcholine signal
  }; 
  ConType con_type; 

  SigFuncInfo  signal_func; //How are signals from the presynaptic cell transformed? 
  float        wt_gain;  // #DEFAULT_PARAM fixed multiplier of connection weights
  float        synapse_sat; //Maximum activation of any one synapse
  float        axon_delay; // axonal delay, in seconds

  enum LearningLaw {     // What kind of learning law? 
    NO_LEARNING  = 0, // Fixed connection, no learning 
    POST_GATE    = 1, // Postsynaptically-gated Hebbian:  dw = lrate*post*(pre-w) 
    PRE_GATE     = 2, // Presynaptically-gated Hebbian: dw = lrate*pre*(post-w) 
    D1_POST_GATE = 3, 
    D2_POST_GATE = 4, 
    D1_PRE_GATE  = 5, 
    D2_PRE_GATE  = 6, 
    HEBB_DECAY   = 7, // Ungated Hebb learning with decay: dw = lrate*pre*post -pass_wt_decay.rate(baseline-w)
    DA_POST_GATE = 8, 
    DA_LEARN_REP = 9, 
    DA_LEARN_REP2 = 10, 
    DA_LEARN_REP_IH = 11, 
    DA_POST_GATE_AT = 12,
    MODULAR = 98, //use modular equation design-- LTP & LTD fields
    SCRIPT = 99 // uses custom learning law script -- script should get RtConSpec *spec = *owner; use spec->curRu,curSu,curCn; then pass dwt to spec->curDwt
  }; 
  LearningLaw learning_law; 

  // CustLearnLaw if learning_law == SCRIPT
  Script CustLearnLaw; //#CONDEDIT_ON_learning_law:SCRIPT if custom learning law script desired

  float 	lrate;		// learning rate 
  // modular learning law if learning_law = MODULAR
  ModLearnInfo LTP; //#CONDEDIT_ON_learning_law:MODULAR Controls rate of wt INCREASE
  ModLearnInfo LTD; //#CONDEDIT_ON_learning_law:MODULAR Controls rate of wt DECREASE
  ModLearnDecayInfo pass_wt_decay; // Controls rate of wt PASSIVE DECAY
  float         post_lthresh;   // Threshold of postsynaptic activity for learning; can be lowered temporarily by samp_learn dpost_l


  //Eligibility 
  EligTraceInfo elig_trace; //governs eligibility traces
  ExcludeNormInfo exclude_norm; //keeps weights from being included in normalization

  int          delay;          // #HIDDEN #READ_ONLY delay between cell & axon terminal, in ticks, calculated from dt and axon_delay
  float        dt;             // #READ_ONLY #HIDDEN set by and from RtTrial::Run() 
  RtCon *curCon;//#HIDDEN #READ_ONLY #NO_SAVE used only to pass con to script learning law
  RtUnit *curSu;//#HIDDEN #READ_ONLY #NO_SAVE used only to pass su to script learning law
  RtUnit *curRu;//#HIDDEN #READ_ONLY #NO_SAVE used only to pass ru to script learning law
  float curDwt; //#HIDDEN #READ_ONLY #NO_SAVE used only to receive dwt from script learning law

  //-------------

#ifndef RT_1_0
  float        thresh;   // The current presynaptic activity threshold 

  enum ThreshType {      // What is the nature of the threshold? 
    RECT         = 0x00, // subtract threshold, and set floor at zero 
    SQUELCH      = 0x01  // set input to zero if below threshold, else no change 
  }; 
  ThreshType thresh_type; 

  enum EligType { //What kind of law for eligibility traces? 
    TRACK_ONE,   // The eligibility trace rises to one, regardless of pre*post 
    TRACK_ONE_RAW_PRESYN, // use raw, not cooked presynaptic activity 
    SBA_ELIG      // Use Sutton, Barto, Anderson, eqn, which tracks pre*post 
  }; 
  bool          decay_wts;      // Should weights decay toward baseline? 
 
  float         baseline_wt;    // #CONDEDIT_ON_decay_wts:true if learning 
  float 	decay;		// #CONDEDIT_ON_decay_wts:true decay rate (before lrate -- decay rate will be multiplied by lrate to determine final decay rate) 
  bool          use_elig;       // Use synaptic eligibility trace for learning? 
  EligType      elig_type;      // #CONDEDIT_ON_use_elig:true Eligibility trace law 
  float         elig_rise_rate; // #CONDEDIT_ON_use_elig:true how fast eligibility appears 
  float         elig_decay_rate;// #CONDEDIT_ON_use_elig:true how fast eligibility decays 
  float         elig_inh_decay_rate; //#CONDEDIT_ON_use_elig:true how fast eligibility decays due to incoming net_inhib 
  bool          excludeRwtNorm; // #CONDEDIT_ON_con_type:EXCIT If normalizing receiving unit wts, should this be excluded from the normalization?  
  bool          excludeSwtNorm; // #CONDEDIT_ON_con_type:EXCIT If normalizing sending unit wts, should this be excluded from the normalization? 

#endif //ifndef RT_1_0

  bool          habit_post_learn; //use habituation of postsynaptic eligibility? (also need to turn on calculation of postsynaptic unit habituation) 
  float         d2n_dw;         // #HIDDEN #CONDEDIT_ON_learning_law:DA_LEARN_REP for DA_LEARN_REP -- how strongly to decrease wts for d2 activity and high eligibility 
  float         d2n_thresh;     // #HIDDEN #CONDEDIT_ON_learning_law:DA_LEARN_REP,DA_LEARN_REP_IH for DA_LEARN_REP -- above what eligibility threshold to begin decreasing wts with d2 activity 
  float         pos_d2wtgain;   // #HIDDEN #CONDEDIT_ON_learning_law:DA_LEARN_REP if punish (d2) and dw > 0, then mult by this 
  bool          useFixedWts;    // #HIDDEN set by trial process for constraint fit 
  bool          legacy_ignore_dt; // #READ_ONLY #HIDDEN set by RtTrial::Run() -- should be true only for old models where dt was not used properly -- to update, mult all RtConSpec lrate's by 1000 and set this to false 

  void updateDt(float dt, bool legacy_ignore_dt);
 
  inline bool   doRWtNorm()      {return ((con_type==EXCIT) && (learning_law != NO_LEARNING) && (!exclude_norm.rwt));} //normalize only excit.   
  inline bool   doSWtNorm()      {return ((con_type==EXCIT) && (learning_law != NO_LEARNING) && (!exclude_norm.swt));} //normalize only excit.   
  // IS THIS FUNCTION NECESSARY? 
  inline void 		Compute_dWt(Con_Group* cg, Unit* ru); 
 
  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su)  
     { ConSpec::C_InitWtState(cg, cn, ru, su); ((RtCon*)cn)->dwt = 0.0f;} 
 
  // Added by Josh 10/26 
  inline float          Compute_Cooked_Act(Connection* cn, RtUnit *ru, RtUnit *su); 
  inline float          ComputeModDwEl(float pre, float post,float elig,RtUnit *ru, ModLearnInfo *p, float wt);
  inline float          ComputeModDw(float pre, float post,float elig,RtUnit *ru, float wt);
  inline float 		C_Compute_Net(Connection* cn, RtUnit* ru, RtUnit* su); 
  inline void           C_Compute_dWt(Connection* cn, RtUnit* ru, RtUnit* su); 
  inline float 	        Compute_Net(Con_Group* cg, Unit* ru); 
 
  void	UpdateAfterEdit(); 
  void	Initialize(); 
  void 	Destroy()		{ }; 
  SIMPLE_COPY(RtConSpec); 
  COPY_FUNS(RtConSpec, ConSpec); 
  TA_BASEFUNS(RtConSpec); 
}; 
 
 
inline void RtConSpec::Compute_dWt(Con_Group* cg, Unit* ru) { 
  CON_GROUP_LOOP(cg,C_Compute_dWt((RtCon*)cg->Cn(i), (RtUnit*)ru, (RtUnit*)cg->Un(i))); 
} 
 
//IS THIS CLASS NECESSARY?  Maybe for hook to appropriate Compute_Net()? 
class RtCon_Group : public Con_Group {  // group of Rt connections 
public: 
  // these are "convenience" functions for those defined in the spec 
  float Compute_Net(RtUnit* ru) 
    { Unit *u=(Unit*)ru; return ((RtConSpec*)spec.spec)->Compute_Net(this,u); } 
  void Compute_dWt(RtUnit* ru) 
    { Unit *u=(Unit*)ru; ((RtConSpec*)spec.spec)->Compute_dWt(this,u); } 
  void	Initialize(); 
  void 	Destroy()		{ }; 
  COPY_FUNS(RtCon_Group, Con_Group); 
  TA_BASEFUNS(RtCon_Group); 
}; 
 
 
 
class RtTrial : public TrialProcess { 
  // one presentation of an event to Rt 
public: 
  bool          responseMade;   // #HIDDEN did an output unit exceed threshold? 
  bool          responseCorrect;// #HIDDEN whether unit over threshold was correct 
  float         responseTime;   // #HIDDEN response time in task 
  bool          rewardIfCorrect;// whether to reward if response correct 
  bool          trialEnded;     // #HIDDEN stop looping? 
  bool          useFixedWts;    // #HIDDEN whether to substitute fixed weights for computing network -- this is for constraint fitting 
 
  float		abs_time;       // absolute time 
  float         trial_time;     // relative to start of trial 
  // #READ_ONLY #SHOW current time (relative to start of sequence) 
 
  float		dt;             // each "cycle" corresponds to dt (in seconds) 
  bool          legacy_ignore_dt;    // should be set to true, i.e. governing eqns updates are mult'd by dt -- only set to false if necessary to avoid breaking old legacy simulations that were built with false 
  // #READ_ONLY #SHOW this is made to correspond to the dt used by units 
  int           update_period_ticks; //update display after how many cycles? 
  int           update_countdown; //#HIDDEN #READ_ONLY how many cycles till update? 
 
  void          setFixedWts(bool isFixed); 
  void          fixWts()       {setFixedWts(true);}  //for constraint fitting 
  void          unclampWts()   {setFixedWts(false);} //for normal running 
 
  void		Init_impl(); 
  void          Run(); 
  void		Loop(); 
  bool		Crit()          { return trialEnded; } 
  void          Final(); 
 
  void          UpdateDt(BaseSpec_MGroup *specs); 
  //virtual void	Compute_dWt();        //Is this necessary??? 
  void          Prep_Compute_Net(); 
  void		Compute_Act(); 
  void          Compute_Wt_Norm(); 
  
  virtual void	StoreState(); 
  // store current state of network in buffers 
 
  virtual int	GetUnitBufSize(bool in_updt_after = false); 
  // finds first unit in first layer and gets current buffer size of that unit 
 
  void		GetCntrDataItems(); 
  void		GenCntrLog(LogData* ld, bool gen); 
 
  void	        UpdateAfterEdit(); 
  void 	        Initialize(); 
  void	        InitLinks(); 
  void	        Destroy()		{ }; 
  SIMPLE_COPY(RtTrial); 
  COPY_FUNS(RtTrial, TrialProcess); 
  TA_BASEFUNS(RtTrial); 
}; 
 
// Allows next event to be specified 
class RtEpoch : public EpochProcess { 
public: 
  bool isNextSpecified;          //#HIDDEN if specified 
  RtEvent *nextEvent; 
 
  void GetCurEvent();  //overrides parent method of choosing event 
  void specNextEvent(RtEvent *next); 
 
  void  Initialize () {/*EpochProcess::Initialize();*/ nextEvent = NULL; isNextSpecified = false;} 
  void Destroy() {}; 
   
  SIMPLE_COPY(RtEpoch); 
  COPY_FUNS(RtEpoch, EpochProcess); 
  TA_BASEFUNS(RtEpoch); 
}; 
 
 
//------ Inline funcs ------------ 
 
inline float RtConSpec::C_Compute_Net(Connection* _cn, RtUnit* ru, RtUnit* su) { 
  //Computes net input, also does weight update 
  RtCon *cn = (RtCon*)_cn; 
  float cookedAct = Compute_Cooked_Act(cn,(RtUnit *)ru,(RtUnit*) su); 
  float awt = useFixedWts ? 0.5 : cn->wt;  //If fitting constraints, then fix wts.
 
  switch (con_type) { 
     case EXCIT : ru->net_excit += wt_gain*awt *cookedAct; break; 
     case INHIB : ru->net_inhib += wt_gain*awt *cookedAct; break; 
     case D1 : ru->net_d1 += wt_gain*cookedAct; break; 
     case D2 : ru->net_d2 += wt_gain*cookedAct; break; 
     case NE : ru->net_ne += wt_gain*cookedAct; break;
     case DOPA : ru->net_dopa += wt_gain*cookedAct; break;
     case SERO : ru->net_sero += wt_gain*cookedAct; break;
     case ACH : ru->net_ach += wt_gain*cookedAct; break;
     case UNUSED: break; 
  } 
 
  //printf("Compute_Net OK\n"); 
  return 0.0; 
} 

inline float RtConSpec::ComputeModDwEl(float pre, float post,float elig,RtUnit *ru, ModLearnInfo *p, float wt) {
  float mval = 1.0, lval = 1.0, rval = 1.0;
  switch (p->allX) {
     case ModLearnInfo::NONE: break; 
     case ModLearnInfo::DOPA: mval = ru->net_dopa; break;
     case ModLearnInfo::SERO: mval = ru->net_sero; break;
     case ModLearnInfo::ACH: mval = ru->net_ach; break;
     case ModLearnInfo::NE: mval = ru->net_ne; break;
  }
  if (p->allX !=ModLearnInfo::NONE) {
    if (p->_==ModLearnInfo::ABOVE) mval = MPLUS(mval-p->val);
    else mval = MPLUS(p->val-mval);
  }

  switch (p->lx) {
     case ModLearnInfo::ONE:  break;
     case ModLearnInfo::PRExPOST: lval = pre*post; break;
     case ModLearnInfo::PRE_ONLY: lval = pre; break;
     case ModLearnInfo::POST_ONLY:lval = post; break;
     case ModLearnInfo::ELIG: lval = elig; break;
  }
  
  switch (p->rx) {
     case ModLearnInfo::ONE: break;
     case ModLearnInfo::PRExPOST: rval = pre*post; break;
     case ModLearnInfo::PRE_ONLY: rval = pre; break;
     case ModLearnInfo::POST_ONLY:rval = post; break;
     case ModLearnInfo::ELIG: rval = elig; break;
  }

  return (p->lim*lval - wt)*p->rate*rval*mval;
}

inline float RtConSpec::ComputeModDw(float pre, float post,float elig,RtUnit *ru, float wt) {
  float pval, lval;
  pval = ComputeModDwEl(pre, post, elig, ru, &LTP, wt);
  lval = ComputeModDwEl(pre, post, elig, ru, &LTD, wt);
  return lrate*(pval + lval);
}

inline void RtConSpec::C_Compute_dWt(Connection* _cn, RtUnit* ru, RtUnit* su) { 
  // Update weights 
 
  RtUnitSpec *aSpec = (RtUnitSpec *)ru->spec.spec; 
  float dt = aSpec->dt; 
  float tmp = 0.0; 
  RtCon *cn = (RtCon*)_cn;
  float eligVal;
  bool gotDwt; //flag to break out of further computation

  //--- Calculate pre- and post-synaptic activities ----------
  //PRE-SYNAPTIC activity: cookedAct -- accounts for axonal delays, soft_clamp,
  //  synapse_sat and signal function (e.g. threshold)
  float cookedAct = Compute_Cooked_Act(cn,(RtUnit *)ru,(RtUnit*) su); 
 
  //POST-SYNAPTIC activity: post_l_act -- if habit_post_learn, then mult 
  //  postsynaptic act by postsynaptic habituation
  float post_l_act = ru->act*(habit_post_learn?(((RtUnit*)ru)->habit) : 1.0); 

  //squelch resulting postsynaptic act by post_lthresh, then add sampling signal
  post_l_act = MSQUELCH(post_l_act, post_lthresh)+(ru->isLearnEvent ? aSpec->samp_learn.dpost_l : 0.0f); 

  // Update eligibility trace
  if (elig_trace.used) {
    float delig = 0.0; 
    float edecay = elig_trace.decay + ru->net_inhib*elig_trace.inhib; 
    switch (elig_trace.elig_type) { 
      case EligTraceInfo::TRACK_ONE : delig = elig_trace.rise*cookedAct*post_l_act*(1.0f-cn->elig) - edecay*cn->elig; break; 
      case EligTraceInfo::TRACK_ONE_RAW_PRESYN : delig = elig_trace.rise*su->act*post_l_act*(1.0f-cn->elig) - edecay*cn->elig; break; 
      case EligTraceInfo::SBA_ELIG : tmp = cookedAct*post_l_act; delig = elig_trace.rise*tmp*(tmp - cn->elig) - edecay*cn->elig; break; //SBA_ELIG 
    } 
 
    cn->elig+= dt*delig; 
    if (cn->elig < 0.0) cn->elig=0.0; if (cn->elig > 1.0) cn->elig = 1.0;
    eligVal = cn->elig;
  }
  else eligVal = cookedAct*post_l_act; //product of pre- & post-synaptic activity

  //Run learning laws whose form doesn't depend on using eligibility traces
  gotDwt = true; //assertion will change if dwt not computed
  switch (learning_law) {
    case NO_LEARNING : cn->elig = 0.0f; cn->dwt = 0.0f;  break;
    case PRE_GATE : cn->dwt = lrate*(eligVal - cookedAct*cn->wt); break;
    case POST_GATE : cn->dwt = lrate*(eligVal - post_l_act*cn->wt); break; 
    case HEBB_DECAY : cn->dwt = lrate*eligVal; break;//decay added later 
    case SCRIPT : curRu=ru; curSu=su; curCon = cn; CustLearnLaw.Run(); 
                  cn->dwt = curDwt; break;
    case MODULAR :  cn->dwt = 
		ComputeModDw(cookedAct, post_l_act, eligVal, ru, cn->wt); break;
    default: gotDwt = false; break; 
  }

  //Some learning law forms below depend on whether eligibility traces used 
  if (elig_trace.used && !gotDwt) {
    // Eligibility-based learning laws
    switch (learning_law) { 
    case D1_POST_GATE : cn->dwt=lrate*cn->elig*ru->net_d1*(cn->elig-cn->wt); 
                        break; 
    case DA_POST_GATE : cn->dwt = lrate*cn->elig*(ru->net_d1 - ru->net_d2); break;
    case DA_LEARN_REP : //DA_LEARN_REP_IH is deprecated (now AKA DA_LEARN_REP) 
    case DA_LEARN_REP_IH : tmp = ru->net_d1+ru->net_d2;  cn->dwt=lrate*tmp*(-MPLUS(cn->elig-d2n_thresh)+ d2n_dw*MPLUS(cn->elig-pos_d2wtgain)); break; 
    case DA_POST_GATE_AT :  tmp = MPLUS((habit_post_learn? ru->habit:1.0)*(ru->act_trace + (ru->isLearnEvent ? aSpec->samp_learn.dpost_l : 0.0f)) -pos_d2wtgain);  cn->dwt = lrate*((ru->net_d1+ru->net_d2)*tmp*(MPLUS(su->act_trace-signal_func.thresh)-cn->wt)); break; 
    default: break; 
    } 
  }  
  else if (!gotDwt) { 
   switch (learning_law) { 
    case D1_POST_GATE : cn->dwt =lrate*ru->net_d1*post_l_act*(cookedAct - cn->wt);
                        break; 
    case DA_POST_GATE : cn->dwt =lrate*post_l_act*(ru->net_d1*(cookedAct - cn->wt)
			- ru->net_d2*cn->wt);  break; 
    default: break; 
   } 
  } 

  // Apply passive weight decay, if desired
  if ((learning_law != NO_LEARNING) && pass_wt_decay.on) cn->dwt+= lrate*pass_wt_decay.rate*(pass_wt_decay.baseline - cn->wt); 
 
  // Other housekeeping...
  if (useFixedWts || cn->isClamp) return;  //no need to do weight updates below 
  if (!legacy_ignore_dt) cn->dwt *= dt; 
  cn->wt +=cn->dwt; 
  C_ApplyLimits(cn, (Unit *)ru, (Unit *)su); 
 
  if (doRWtNorm())  
    {((RtUnit*)ru)->sum_in_wt+=cn->wt; /*((RtUnit*)ru)->num_in_wt++;*/} 
  if (doSWtNorm()) 
    {((RtUnit*)su)->sum_out_wt+=cn->wt; /*((RtUnit*)ru)->num_out_wt++;*/} 
 
} 
 
inline float RtConSpec::Compute_Net(Con_Group* cg, Unit* u) { 
  RtUnit *ru = (RtUnit*)u; 
  float rval=0.0f; 
  CON_GROUP_LOOP(cg, rval += C_Compute_Net(cg->Cn(i), ru, (RtUnit*)cg->Un(i))); 
  return rval; 
} 
 
inline float RtConSpec::Compute_Cooked_Act(Connection* cn, RtUnit *ru, RtUnit *su){ 
    float cookedAct; 
    float rawAct; 
    RtUnitSpec *su_spec = (RtUnitSpec*) su->spec.spec; 
 
    if (delay) { 
      if ((su->ext_flag & Unit::EXT) && !(su_spec->softclamp.used)) { 
	if (su->exts.max_len < delay) {  
	  su->exts.SetSize(delay);  
	  su->max_axon_delay = delay; 
	} 
	rawAct = su->exts.ReadDelayed(delay); 
      } 
      else { 
	if (su->acts.max_len < delay) { 
	  su->acts.SetSize(delay); 
	  su->max_axon_delay = delay; 
	} 
	rawAct = su->acts.ReadDelayed(delay); 
      } 
    } 
    else rawAct = su->act; 
    switch (signal_func.sig_type) { 
    case SigFuncInfo::RECT    : cookedAct = MPLUS(rawAct - signal_func.thresh); break; 
    case SigFuncInfo::SQUELCH : cookedAct = MSQUELCH(rawAct, signal_func.thresh); break; 
    default : cookedAct = rawAct; 
  }   
  return MMIN(synapse_sat, cookedAct);  
          //cookedAct > synapse_sat ? synapse_sat : cookedAct; 
} 
 
class Trigger; 
 
//////// Response Stat /////////// 
class ResponseStat : public Stat { 
  // ##COMPUTE_IN_TrialProcess Reports whether a response has been made, and if so, whether correct.  Response occurs when Compare layer unit exceeds threshold.  Response correct when corresponding enviro pattern unit non-zero. 
 
public: 
  enum ResponseType { 
    THRESHOLD,          //response when output cell exceeds threshold 
    SETTLE              //response when unit activities stop changing 
  }; 
  ResponseType response_type; //The criterion for a response 
  Layer*	trg_lay; //unused, should be deleted 
  RtTrial*     tr;   //needed to access trial elapsed time 
  Trigger*      responseSinceWhen; //Response time is relative to time of what trigger? 
  float         minResponseTime;  //Response not registered unless specified time elapses since responseSinceWhen 
  // layer to monitor 
  StatVal	resp;   	// response statistic:  resp.val = 0 when no response, 1 if correct, -1 if incorrect 
  StatVal       resp_time;     // keeps track of response time 
 
  bool          isResponse;   // #READ_ONLY One or more unit currently above threshold 
  bool          isCorrect;    // #READ_ONLY The unit above threshold is correct response 
  float         lastRt;       // #READ_ONLY Response time from trial onset or given trigger, if specified 
 
  float		threshold;	// #CONDEDIT_ON_response_type:THRESHOLD activation value to consider unit being on 
  float         settle_thresh;  // #CONDEDIT_ON_response_type:SETTLE maximum change in activity allowed for any unit when settling complete (as change per second da/dt) 
  void		RecvCon_Run(Unit*)	{ }; // don't do these! 
  void		SendCon_Run(Unit*)	{ }; 
 
  void		InitStat(); 
  void          TrialInit(); //called at start of each trial 
  bool		Crit(); 
  void		Network_Init(); 
  void		Layer_Run();		// only compute on COMP layers 
  void 		Unit_Run(Layer* lay); 
  void          ProcessResponse(Unit *u); 
 
  void		NameStatVals(); 
 
  bool		CheckLayerInNet(); // also check trg_lay 
 
  void	Initialize(); 
  void 	Destroy()		{ CutLinks(); } 
  void	InitLinks(); 
  void	CutLinks(); 
  SIMPLE_COPY(ResponseStat); 
  COPY_FUNS(ResponseStat, Stat); 
  TA_BASEFUNS(ResponseStat); 
}; 
 
 
/////////////////////// 
/// Envionment //////// 
/////////////////////// 
class Trigger; 
 
class TriggerCondition : public taOBase { 
public: 
  enum TrigCondType { 
    ELAPSED_TRIAL_START,       //time elapsed since start of trial 
    ELAPSED_LAST_TRIG,         //time elapsed since a previous trigger 
    LAYER_OVER_THRESH,         //an output unit exceeded a threshold 
    CORRECT,                   //output unit over threshold and correct 
    INCORRECT,                 //output unit over threshold and incorrect 
    NOT_TRIGGER                //a given trigger has not been activated 
  }; 
 
  TrigCondType trig_cond; 
 
  float elapsed_time;          // #USE_RVAL #ENVIROVIEW CONDEDIT_ON_trig_cond:ELAPSED_TRIAL_START,ELAPSED_LAST_TRIG how much time should elapse for trigger? 
  float thresh;                // #USE_RVAL #ENVIROVIEW CONDEDIT_ON_trig_cond:LAYER_OVER_THRESH activity threshold 
  Layer *aLayer;               // #ENVIROVIEW #CONDEDIT_ON_trig_cond:LAYER_OVER_THRESH #NO_NULL not used -- response stat used instead 
  Trigger *other_trigger;      // #ENVIROVIEW #CONDEDIT_ON_trig_cond:NOT_TRIGGER,ELAPSED_LAST_TRIG #NO_NULL depends on which other trigger 
 
  void Initialize () {trig_cond = ELAPSED_TRIAL_START; elapsed_time = 0.0f; thresh = 0.0f; aLayer = (Layer *)NULL; other_trigger = NULL; } 
  void Destroy () {}; 
  void Copy_(const TriggerCondition& cp) {trig_cond = cp.trig_cond; elapsed_time = cp.elapsed_time; thresh = cp.thresh; aLayer = cp.aLayer; other_trigger = cp.other_trigger; } 
 
  COPY_FUNS(TriggerCondition, taOBase); 
  TA_BASEFUNS(TriggerCondition); 
}; 
 
class Trigger : public taNBase { 
public: 
  int index;                   //#READ_ONLY trigger index 
  bool isTriggered;            //has trigger been activated? 
  float whenTriggered;         //#USE_RVAL trigger time with respect to start of trial 
  enum TrigLogic { 
    TRIGGER_AND, 
    TRIGGER_OR 
  }; 
  TrigLogic trigger_logic;     //AND = all trigger conditions must be met; OR = any condition met will suffice 
 
  enum TrigAction { 
    PASSIVE,                   //do nothing; triggers are monitored by other objects 
    TRIAL_END,                 //signals end of trial 
    TRIAL_END_NEXT_SPEC        //current trial is ended, next event is specified 
  }; 
 
  TrigAction trig_action; 
 
  taBase_Group trigCondGroup;  //conditions of trigger 
 
  bool isTrig() {return isTriggered;} //is the Trigger active? 
  void reset() {isTriggered = false;}      //reset trigger to inactive 
 
  void InitLinks() {taNBase::InitLinks(); taBase::Own(trigCondGroup, this);} 
  void Initialize() { trig_action = PASSIVE; index = -1; isTriggered = false; whenTriggered = 0.0f; trigger_logic = TRIGGER_AND; trigCondGroup.SetBaseType(&TA_TriggerCondition);} 
  void Destroy() {}; 
  void Copy_(const Trigger& cp) {trig_action = cp.trig_action; index = cp.index; isTriggered = cp.isTriggered; whenTriggered = cp.whenTriggered;} 
  COPY_FUNS(Trigger, taNBase); 
  TA_BASEFUNS(Trigger); 
}; 
 
  //SpecPtr_of(Trigger); //will this work? 
 
/////////////// Specifying the next event ///////////////// 
class TransitionProb : public taOBase { 
public: 
  RtEvent *nextEvent; //The next event that may be presented 
  float probability;  //The probability that the event may be presented next 
 
  void Initialize () { nextEvent = NULL; probability = 0.0f;} 
  void Destroy () {}; 
  void Copy_(const TransitionProb& cp) {nextEvent = cp.nextEvent; probability = cp.probability;} 
 
  COPY_FUNS(TransitionProb, taOBase); 
  TA_BASEFUNS(TransitionProb); 
}; 
 
class TransitionProb_Group : public taGroup<TransitionProb> {  
  // ##NO_TOKENS ##NO_UPDATE_AFTER group of objects 
public: 
  void	Initialize() 		{ }; 
  void 	Destroy()		{ }; 
  TA_BASEFUNS(TransitionProb_Group); 
}; 
 
class eventTransition : public taNBase { 
public: 
  TransitionProb_Group transitionProbGroup; 
 
  void InitLinks() {taNBase::InitLinks(); taBase::Own(transitionProbGroup, this);} 
  void Initialize() { } 
  void Destroy() {}; 
  void Copy_(const eventTransition& cp) {transitionProbGroup = cp.transitionProbGroup;} 
  COPY_FUNS(eventTransition, taNBase); 
  TA_BASEFUNS(eventTransition); 
}; 
 
/////////// Pattern Specs //////////////////////// 
 
class RtPatternSpec : public PatternSpec { 
public: 
  bool needsTrigger;  //does the pattern require a trigger to be presented? 
  Trigger* onTrigger;  // #CONDEDIT_ON_needsTrigger:true the trigger to enable event on 
  bool needsVetoTrigger; 
  Trigger* vetoTrigger; // #CONDEDIT_ON_needsVetoTrigger:true if triggered, pattern inactive even if onTrigger active 
 
  virtual void ApplyPattern(Pattern *pat); 
 
  void Initialize() {needsTrigger = false; onTrigger = NULL; needsVetoTrigger=false; vetoTrigger = NULL; PatternSpec::Initialize();} 
  void Destroy() {PatternSpec::Destroy();} 
  void Copy_(const RtPatternSpec& cp) {needsTrigger = cp.needsTrigger; needsVetoTrigger = cp.needsVetoTrigger; onTrigger = cp.onTrigger; vetoTrigger = cp.vetoTrigger;} 
  COPY_FUNS(RtPatternSpec, PatternSpec); 
  TA_BASEFUNS(RtPatternSpec); 
}; 
 
class NextEventInfo : public taNBase { 
public: 
  enum NextEventMethod {UNSPEC, DETERMINED, MARKOV, DEPENDS_CORRECT, DEPENDS_RESPONSE, END_EPOCH}; 
 
  NextEventMethod next_event;  
 
  RtEvent *nextEvent;          // #CONDEDIT_ON_next_event:DETERMINED  
  RtEvent *nextEventIfCorrect; // #CONDEDIT_ON_next_event:DEPENDS_CORRECT 
  RtEvent *nextEventIfError;   // #CONDEDIT_ON_next_event:DEPENDS_CORRECT 
 
  // Problem -- need FROM_GROUP source for nextEvent? 
  eventTransition event_trans;  // #CONDEDIT_ON_next_event:MARKOV 
 
 
  RtEvent *getNextEvent(RtUnit *action, bool isReward); 
 
  void Initialize() {next_event = UNSPEC; nextEvent = NULL; nextEventIfCorrect = NULL; nextEventIfError = NULL; }; 
  void InitLinks() {taNBase::InitLinks(); taBase::Own(event_trans, this);}; 
  void Destroy() {}; 
  SIMPLE_COPY(NextEventInfo); 
  COPY_FUNS(NextEventInfo, taNBase); 
  TA_BASEFUNS(NextEventInfo); 
}; 
 
class Trigger_Group : public taGroup<Trigger> {  
  // ##NO_TOKENS ##NO_UPDATE_AFTER group of objects 
public: 
  void	Initialize() 		{ }; 
  void 	Destroy()		{ }; 
  TA_BASEFUNS(Trigger_Group); 
}; 
 
class NextEventInfo_Group : public taGroup<NextEventInfo> {  
  // ##NO_TOKENS ##NO_UPDATE_AFTER group of objects 
public: 
  void	Initialize() 		{ }; 
  void 	Destroy()		{ }; 
  TA_BASEFUNS(NextEventInfo_Group); 
}; 
 
class RtEventSpec : public EventSpec { 
public: 
  Trigger_Group            triggers;          //events within RtEvent 
  NextEventInfo_Group      eventTransitions;  //specifying next RtEvent 
 
  //Constraint Info 
  //bool    isConstraint;  // is the event a constraint for parameter fitting? 
  //enum    ConstraintType {STATIC_STID, STATIC_SEE, DYN_CELL, DYN_RT, DYN_CELL_RT}; 
  //ConstraintType constraint_type; //#CONDEDIT_ON_isConstraint:true  
 
  //Triggers 
  bool    CheckTriggers(Network *network, RtTrial *trial); 
  void    ResetTriggers(); 
  void    ZeroInputLayers(Network *network); 
 
  //Housekeeping 
  void    InitLinks() {EventSpec::InitLinks(); taBase::Own(triggers, this);\
			taBase::Own(eventTransitions, this);} 
  void    Initialize() {}; //{EventSpec::Initialize(); isConstraint = false;  
                           //constraint_type = STATIC_STID;} 
  void    Destroy() { CutLinks(); } 
  //void    Copy(const RtEventSpec& cp) {triggers = cp.triggers;} 
  SIMPLE_COPY(RtEventSpec); 
  TA_BASEFUNS(RtEventSpec); 
}; 
 
/* -- This is obsoleted by separate RtConstraintInfo 
//Need separate constraintEvent class? 
class EventConstraintInfo : public taOBase {  //##INLINE ##NO_TOKENS ##NO_UPDATE_AFTER 
public: 
  float actualRt;                //Reaction time  
  bool isCorrect; 
  float desiredRt; 
 
  void Initialize () {actualRt = -1.0f; desiredRt = -1.0f;}; 
  void Copy_(const EventConstraintInfo& cp) {actualRt = cp.actualRt; isCorrect = cp.isCorrect; desiredRt = cp.desiredRt;} 
  COPY_FUNS(EventConstraintInfo, taOBase); 
  TA_BASEFUNS(EventConstraintInfo); 
}; 
*/ 
 
class RtEvent : public Event { 
public: 
  NextEventInfo *nextEventInfo;  //#FROM_GROUP_nei_mirror 
 
  // Constraint stuff: 
  //EventConstraintInfo data;      //#INLINE #CONDEDIT_ON_isConstraint:true 
 
  // mirrors from spec, to be used by gui 
  NextEventInfo_Group *nei_mirror;  //#HIDDEN #READ_ONLY mirrors spec eventTransitions 
  bool isConstraint;               //#HIDDEN #READ_ONLY mirros spec isConstraint 
 
  RtEvent *getNextEvent(bool isCorrect)    
    {return nextEventInfo->getNextEvent(NULL, isCorrect);} 
 
  void UpdateAfterEdit(); 
  void Initialize() {nextEventInfo = NULL; nei_mirror = (NextEventInfo_Group *)NULL;} 
  void Destroy() { CutLinks(); }; 
 
  SIMPLE_COPY(RtEvent); 
  COPY_FUNS(RtEvent, Event); 
  TA_BASEFUNS(RtEvent); 
}; 
 
///////////////// Constraints and Data fitting /////////////// 
 
class RtConstraintInfo; 
 
//typedef taGroup<RtConstraintInfo> RtConstraintInfo_Group; 
typedef taBase_Group RtConstraintInfo_Group; 
 
class RtConstraintInfo : public BaseSubSpec {  
//For now, only tracks one value, applied to an individual element (unit or con), or else uniformly to the entire group (layer or projection) 
public: 
  enum ConstraintType {INITIAL_CONDITION, CLAMP, SCRIPT}; 
  enum ConstraintObjType {CONSTRAIN_LAYER, CONSTRAIN_PROJECTION}; 
  enum ConstraintSpecific {CONSTRAIN_UNIFORM, CONSTRAIN_INDIVIDUAL}; 
 
  ConstraintType constraint_type; //Set only the initial condition, or maintain clamp? 
  ConstraintObjType constrained_obj; //Is this constraining cell activity or synaptic wts? 
  ScriptProcess apply_script; //#CONDEDIT_ON_constraint_type:SCRIPT 
  Network *whichNetwork; //Network to which the constraint will be applied 
  Layer *whichLayer; //#CONDEDIT_ON_constrained_obj:CONSTRAIN_LAYER if constraining layer 
  Unit_Group *units; //#HIDDEN For TA system -- pointer to units of selected layer 
  RtUnit *whichUnit; //#CONDEDIT_ON_constrained_obj:CONSTRAIN_LAYER #FROM_GROUP_units if constraining layer 
  Projection *whichProjection; //#CONDEDIT_ON_constrained_obj:CONSTRAIN_PROJECTION 
  RtCon *whichCon; //#CONDEDIT_ON_constrained_obj:CONSTRAIN_PROJECTION 
  ConstraintSpecific how_specific; //Constrain all elements in group the same, or per unit? 
  float val; //value to which element will be constrained 
 
  void ReadFromNet(); //#BUTTON read unit activity or connection wts from current net state 
  void ApplyToNet(); //#BUTTON apply this constraint to the network 
  bool isApplied; //#HIDDEN has the constraint been applied but net state not yet restored? 
  void RestoreNet(); //#BUTTON restore network to original state before constraint 
  bool alreadyAppliedThisRun; //#HIDDEN set during application to avoid infinite recursion 
 
  void FindConstraints();   //initializes constraintSource 
  RtConstraintInfo_Group *constraintSource; //#HIDDEN 
  RtConstraintInfo *constraintToLink; //#FROM_GROUP_constraintSource 
 
  RtConstraintInfo_Group sub_constraints; //Other constraints to be applied when this one is applied 
  void LinkConstraint();      //#BUTTON links constraintToLink into sub_constraints 
 
  void UpdateAfterEdit() { if (whichLayer != NULL)  
               taBase::SetPointer((taBase **)&units, &(whichLayer->units));  
               FindConstraints();  
               BaseSubSpec::UpdateAfterEdit(); }; 
 
  void	Initialize(); 
  void  InitLinks(); 
  void 	Destroy()		{ }; 
 
  SIMPLE_COPY(RtConstraintInfo); 
  COPY_FUNS(RtConstraintInfo, BaseSubSpec); 
  TA_BASEFUNS(RtConstraintInfo); 
}; 
 
 
class RtConstraintSpec : public BaseSpec { 
public: 
 
  RtConstraintInfo_Group constraints; //Group of constraints to be applied by RtConstrain process while fitting data 
 
  void	Initialize() 		{constraints.SetBaseType(&TA_RtConstraintInfo);}; 
  void 	Destroy()		{ }; 
  void  InitLinks() {taBase::Own(constraints, this); BaseSpec::InitLinks(); }; 
 
  SIMPLE_COPY(RtConstraintSpec); 
  COPY_FUNS(RtConstraintSpec, BaseSpec); 
  TA_BASEFUNS(RtConstraintSpec);   
}; 
 
////////// Data //////////// 
class FitDataInfo; 
 
typedef taBase_Group FitDataInfo_Group; 
 
/* This class keeps track of one piece of data to be fit by the model.  It specifies the data and also specifies the input/state constraints on the network under which the network should fit the data */ 
class FitDataInfo : public BaseSubSpec { 
public: 
  enum    ConstraintType { 
    STATIC_STID,           //Minimize Square TIme Derivative for unit 
    STATIC_SEE,            //Minimize Squared Equilibrium Error for unit 
    DYN_CELL,              //Fit dynamic cell type activity vs. time curve 
    DYN_RT,                //Fit reaction-time data 
    DYN_CELL_RT,           //Fit both cell type activity and reaction time 
    SCRIPT                 //Obj. Func. defined by "custom" script below 
  }; 
 
  ConstraintType constraint_type; //What kind of data constraint? 
  bool isActive; //Is this data point to be fit currently? 
  Event_MGroup *eventSource; //#HIDDEN points to group of environment events 
  Event *eventToLink;  //#FROM_GROUP_eventSource LinkEvent button will add (link) this to events below 
  taBase_Group events; //Single event, or sequence of events, to be run/applied to generate prediction -- ONLY *LINK* EVENTS TO THIS LIST! (Otherwise, they'll be invisible to the environment) 
  float constraint_wt; //#DETAIL How much does this constraint contribute to obj. func.? (should be >= 0) 
 
  //---- Script-based constraint computation ------ 
  Script custom;   // used to compute custom value for objective function 
  String UserData; // May be used to store names, labels, other info, etc. for this constraint -- especially useful for script constraints 
  float fitEnergyVal; // Script should place raw fit energy here 
 
  //---- Reaction time data ------- 
  float actualRt;  //#CONDEDIT_ON_constraint_type:DYN_RT,DYN_CELL_RT model output 
  float desiredRt; //#CONDEDIT_ON_constraint_type:DYN_RT,DYN_CELL_RT RT to be fit 
  bool isCorrect;  //#CONDEDIT_ON_constraint_type:DYN_RT,DYN_CELL_RT Was the model output "Correct"? 
 
  //---- Static data -------------- 
  float desiredAct; //#CONDEDIT_ON_constraint_type:STATIC_STID,STATIC_SEE 
 
  //---- Static and Cell type data ----------- 
  Layer *whichLayerRecorded; //#CONDEDIT_OFF_constraint_type:DYN_RT to fit cell type data  
  Unit_Group *whichLayerUnits; //#HIDDEN for TA -- points to units of whichLayerRecorded 
  Unit *whichUnitRecorded;   //#CONDEDIT_OFF_constraint_type:DYN_RT #FROM_GROUP_whichLayerUnits to fit celll type data 
 
  //---- Cell type data -------------- 
  // Actual data to be fit 
  float_Array data_time;    //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT 
  float_Array data_act;    //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT  
 
  // Output of model 
  float_Array model_time;    //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT 
  float_Array model_act;    //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT  
  float dataAlignTime; //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT Data alignment point (e.g. if Stimulus presented at t=0.5 in data, then set this to 0.5 to align on stimulus -- t=0.5 will become new t=0 for data) 
  Trigger *align_trigger; //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT Cell type data aligned such that data point at dataAlignTime coincides with this trigger 
  float data_gain;   //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT multiply actual firing rates by this 
 
  bool clampUnitToData;  //#CONDEDIT_ON_constraint_type:DYN_CELL,DYN_CELL_RT should the actual firing rate data be reflected in a model cell (for comparison)? 
  Layer *dataDriveWhichLayer; //#CONDEDIT_ON_clampUnitToData:true which layer should reflect data 
  Unit_Group *dataDriveUnits; //#HIDDEN for TA -- points to units of dataDriveWhichLayer 
  Unit *dataDriveWhichUnit;   //#CONDEDIT_ON_clampUnitToData:true #FROM_GROUP_dataDriveUnits which unit in the layer should reflect data 
  RtConstraintInfo_Group *net_state_group; //#HIDDEN points to set of possible constraints 
  RtConstraintInfo *net_state; //#FROM_GROUP_net_state_group specifies state of network for data fitting 
 
  void FindConstraints(); 
 
  void LinkEvent() {events.Link(eventToLink);}; //#BUTTON add eventToLink to events 
 
  float LinearInterpModel(float curModelTime); 
  float CalcCellTypeFitError(); 
 
  void  UpdateAfterEdit() { 
  if(dataDriveWhichLayer != NULL) { 
      if ((TAPtr) dataDriveUnits != (TAPtr) &(dataDriveWhichLayer->units))  
	taBase::SetPointer((taBase**) &(dataDriveUnits),  
			   &(dataDriveWhichLayer->units));  
  } 
    if(whichLayerRecorded != NULL) { 
      if ((TAPtr) whichLayerUnits != (TAPtr) &(whichLayerRecorded->units))  
	taBase::SetPointer((taBase**) &whichLayerUnits,  
			   &(whichLayerRecorded->units)); 
  } 
  FindConstraints(); BaseSubSpec::UpdateAfterEdit();}; 
 
  void  InitLinks(); 
  void	Initialize(); 
  void 	Destroy()		{ }; 
 
  SIMPLE_COPY(FitDataInfo); 
  COPY_FUNS(FitDataInfo, BaseSubSpec); 
  TA_BASEFUNS(FitDataInfo);   
}; 
 
class FitDataSpec : public BaseSpec { 
public: 
  FitDataInfo_Group data; //Set of data points to be fit 
 
  void	Initialize() 		{data.SetBaseType(&TA_FitDataInfo); }; 
  void 	Destroy()		{ }; 
  void  InitLinks() {taBase::Own(data, this); BaseSpec::InitLinks(); }; 
 
  float CalcRSquare(); //#BUTTON #USE_RVAL 
  void  DumpData(); //#BUTTON  
  void  MakeRandEv(); //#BUTTON  
 
  SIMPLE_COPY(FitDataSpec); 
  COPY_FUNS(FitDataSpec, BaseSpec); 
  TA_BASEFUNS(FitDataSpec);   
}; 
 
//The optim class encapsulates the BFGS version of DFP nonlinear optimization 
//Required parameters/inputs are: 
//   void  (*getParams)(void *data, float_Array *p); //retrieves parameters 
//   void  (*setParams)(void *data, float_Array *p); //sets parameters 
//   float (*objFunc)(void *data, float_Array *p);  //returns objective function 
//   void  (*calcGrad)(void *data, float_Array *p, float_Array *g); //calcs grad 
//   float numParams; 
// 
//Optional parameters/inputs are: 
//   void *data;                                     //typically a class object 
// 
//   float getParam(void *data, int which); 
//   void  setParam(void *data, int which, float val); 
//   calc 
// This class should be relatively portable, except for the dependence on  
//  float_Array types.  At least it should port easily enough to other pdp++  
//  apps. 
 
//#REG_FUN 
void RtSetParams(void *data, float_Array *p); 
//#REG_FUN 
void RtGetParams(void *data, float_Array *p); 
//#REG_FUN 
float RtFitObjFunc(void *data, float_Array *p); 
//#REG_FUN 
void RtFitCalcGrad(void *data, float_Array *p, float_Array *g); 
//#REG_FUN
void RtSaveParams(void *data);
 
#define getmat(a,b,c) a.SafeEl(b*numParams+c) //for accessing hessian 
#define setmat(a,b,c,val) a.Set(b*numParams+c, val) 
 
class optim : public taBase { 
public: 
  int maxit;         //Maximum allowed iterations 
  float machEps;     //#DETAIL limit of machine precision 
  float optim_tolx;  //#DETAIL Convergence criterion on x-values 
  float bfgs_max_step; //#DETAIL Scaled maximum step length allowed in line searches.  
  float ns_tol;     //#DETAIL convergence criterion on x for line search (default 1e-7) 
  float small_f;         //#DETAIL ensures sufficient decrease in function value (default 1e-4) 
 
  float_Array p;     //Parameters 
  float_Array pnew;  //Updated parameters 
  float_Array g;     //gradient of objFunc in parameter space 
  float_Array delta_g;    //diff between old and new gradient vectors 
  float_Array hdelta_g;   //dg*hessian 
  float_Array xi;    //direction of line search (may differ from gradient) 
  float_Array hessian; 
 
  void *data;        //#HIDDEN typically a class object 
  int numParams;     //#READ_ONLY How many parameters are being simultaneously fit? 
  bool isRun;       // is the optimization in process? 
 
  float den;        //#HIDDEN 
  float bfgs_a;        //#HIDDEN 
  float bfgs_b;        //#HIDDEN 
  float bfgs_c;        //#HIDDEN 
  float stpmax;     //#HIDDEN 
  float sumdelta_g; //#HIDDEN 
  float sumxi;      //#HIDDEN 
  float gtol;       //#HIDDEN 
  float fp;      //last accepted objective function value 
  int check;        //#HIDDEN whether minimum might be local 
  int its;       //Current number of iterations 
  float curf;    //current search of objective function (may not be minimum) 
  bool forceInitNextRun; //if true, causes Init to recalc gradient before stepping (this is useful if the gradient takes a long time to compute, so you don't have to press Init, wait, and then press Run) 
  int retries;   //Convergence may be spurious, so ignore convergence signal and keep on going this many times (counts down to zero) 
 
  float (*objFunc)(void *data, float_Array* p);  //#HIDDEN #NO_TOKENS returns objective function 
  void  (*calcGrad)(void *data, float_Array* p, float_Array *g); //#HIDDEN #NO_TOKENS calculates gradient of objective function -- default can be overridden 
 
  //float (*getParam)(void *data, int which); 
  //void  (*setParam)(void *data, int which, float val); 
 
  void  (*getParams)(void *data, float_Array *p); //#HIDDEN 
  void  (*setParams)(void *data, float_Array *p); //#HIDDEN 
 
  void setNumParams(int num); 
  void setDataObj(void *theData) {data = theData;}; 
  void Init();   //initialize the optimization algorithm 
  void ReInit() {forceInitNextRun = true;}; //#BUTTON  
  void finishOpt() { if (!isRun) return; isRun = 0; (*setParams)(data, &p); }; 
  void doOptStep();  //The optimization engine; calls doLineSrch 
  void doLineSrch(int n, float_Array *xold, float fold, float_Array *g, float_Array *p, float_Array *x, float *f, float stpmax, int *check);   
 
  void Initialize(); 
  // housekeeping for taBase 
  SIMPLE_COPY(optim); 
  COPY_FUNS(optim, taBase); 
  TA_BASEFUNS(optim); 
}; 
 
 
// -------- Simplex algorithm ------------------------------ 
#define getpmat(a,b,c) a.SafeEl((b)*numParams+(c)) //a = array, b = which vertex, c = which param element 
#define setpmat(a,b,c,val) a.Set((b)*numParams+(c), val) //a = array, b = which vertex, c = which param element, val = value to set 
 
//Inputs:  p (starting point, derived by getParams()), init_len (characteristic length), numParams (number of dimensions, float *objFunc(void *data, float_Array *p), *getParams(), *setParams(), void *data 
 
class simplex : public taBase { 
public: 
  int numParams;           //number of parameters 
  int numActiveParams;     // number of active parameters, in case only a subset should be optimized at a given time (e.g. for subplex)
  int_Array  ap; //active params -- indexes the subset of parameters to be optimized -- by default, it is an array of [0..numParams-1]
  float init_dp;           //#DETAIL characteristic length (used to init simplex vertices: delta p = max(p*init_dp,min_dp)) 
  float min_init_dp;       //#DETAIL minimum allowed delta p (only when initializing simplex -- delta p may shrink further after that) 
  float ftol;              //convergence criterion 
  float ftol_test;         //actual convergence metric of current step; will terminate if less than/equal to ftol 
 
  int maxitt;              //maximum allowed number of iterations 
  int its;                 //number of iterations so far 
  float_Array p;           //array of parameters 
  float_Array psum;        //sum of a given parameter element across each vertex of simplex 
  float_Array fvals;       //objective function values at each of the numParams+1 vertices of the simplex 
  float_Array s;           //matrix -- simplex of parameter vectors 
  void *data;              //#HIDDEN typically a class object 
  bool isRun;              // is the optimization in process? 
  bool forceStop;          //#HIDDEN if true, halt optimization and set model to current best parameter set found 
  bool forceInitNextRun;   // #HIDDEN if true, causes Init to recalc gradient before stepping (this is useful if the gradient takes a long time to compute, so you don't have to press Init, wait, and then press Run) 
  int retries;   //Convergence may be spurious, so ignore convergence signal and keep on going this many times (counts down to zero) 
 
  // Internal vars 
  float f_low;              //lowest current value of objective function 
  //bool  doFLoAvg;           //if true, replicate and average the best f value (subplex) -- useful for noisy objective functions, but somewhat slower
  float fsave;             //#HIDDEN internal temporary variable 
  float ftry;              //latest attempted function evaluation 
  bool rep_f_low_min;      //should f_low be replicated, and the min value used?
  int idx_hi, idx_lo, idx_nhi, spts; //#HIDDEN internal variables 
 
  void Initialize(); 
  void setNumParams(int num); 
  void resetActiveParams(); 
  void setAllParamsActive();
  void setParamActive(int which);
  void setDataObj(void *theData) {data = theData;}; 
  virtual void Init(); 
  virtual void ReInit() {forceInitNextRun = true; forceStop = false;};     //#BUTTON Manually flags the optimization algorithm to re-init at start of next run 
  virtual void InitSimplex();
  virtual void checkInitSimplex(); //will call InitSimplex() if necessary
  void sortFVals();  //finds low, hi, nhi, f_low
  virtual bool checkCrit();
  bool checkForceStop();
  virtual void doOptStep();  //The optimization engine 
  void putBestPtInP(); //saves the best (with lowest obj func) parameter set 
  virtual bool simplexCrit(); //did simplex converge?
  float (*objFunc)(void *data, float_Array* p);  //#HIDDEN #NO_TOKENS returns objective function 
  void  (*getParams)(void *data, float_Array *p); //#HIDDEN 
  void  (*setParams)(void *data, float_Array *p); //#HIDDEN 
  void  (*saveParams)(void *data); //#HIDDEN
 
  //helper functions 
  virtual float initStepSize(int whichParam); 
  void calcPsum(); 
  float stepSimplex(float fac); 
 
  SIMPLE_COPY(simplex); 
  COPY_FUNS(simplex, taBase); 
  TA_BASEFUNS(simplex); 
}; 
 
class subplex : public simplex { 
// set stepsizes 
// set subspaces 
// iterate NMS for each subspace 
 
public: 
  float psi; 
  float omega; 
  int nsmin;  //minimum subspace dimension 
  int nsmax;  //maximum subspace dimension 
  int nsp; //number of subspaces 

  int subplexState; //-1 = initializing; 0..nsp = which subspace to optimize
  bool isSimplexCrit; //Did the current subspace optimization terminate?
  int ssoff; //offset of current subspace first element in perm vector

  bool isFirstSimplexIter; //if so, need to calculate convergence crit
  float simp_tol; //#HIDDEN convergence criterion for each subspace optimization -- set by algorithm
  float subp_tol; //convergence criterion for subplex algorithm
 
  float_Array step; //vector of stepsizes, initially set by initStepSize() 
  float_Array lastxbest; //keeps record of last best value of obj func.
  float_Array dx; // progress vector 
  int_Array subsp_dims; //keeps track of which subspace [0.. nsp] each element belongs to 
  int_Array perm; //permutation vector, to keep track of which references to subpsaces
   
 
  void Initialize(); 
  virtual void Init();
  virtual void ReInit(); 
  virtual void checkInitSimplex() {}; 
  void useSubspace(int which); 
  virtual void InitSimplex();
  void updateStepVec();
  void setSubsp();
  float computeSimplexSize();
  virtual bool simplexCrit();
  bool subplexOuterCrit();
  virtual float initStepSize(int which); //returns step
  virtual bool checkCrit();
  virtual void doOptStep(); //The main loop
 
  SIMPLE_COPY(subplex); 
  COPY_FUNS(subplex, simplex); 
  TA_BASEFUNS(subplex); 
}; 
 
// RtDistFitMgr -- a lightweight method of distributing constraint fitting runs
// across multiple processors.  The processes must share a common file system.
// Each process is configured manually as master or slave and slave ID.
// The master writes a file called _fitmgr_params, which the slaves read.  This
// file contains the parameters to be evaluated and a breakdown of which slave
// processes which constraints.  File format is: #params\n, <params, 1 per line>,
// then one line per slave -- each line is: 
// <slave #> <start constraint> <end constraint>
// Slaves then write files ~fitmgr_data_#, where # is the slaveID.  Each slave
// output file has one line per constraint:
// <constraint #> <fitEnergyVal> <actualRt> <isCorrect>
// (Need to add other output data later)
// Final script constraints are processed last by master, once slaves report
// - appearance of directive file + removal of slave's previous report file 
//   cues slaves to respond
// - appearance of slave's report cues master to respond (need other ready cue,
//   to avoid opening partially-written files?)
class RtDistFitMgr : public taNBase {
public:
  bool used; //use distributed fit functions?
  bool master; //if true, runs the show -- otherwise, works as slave
  int numSlaves; //#CONDEDIT_ON_master:true how many slave processors?  Only master needs to know
  int numOverlap; //#CONDEDIT_ON_master:true discard first how many constraint in a sub-run (for excluding startup issues with sequential effects)
  int  slaveID; // must be numbered sequentially 1, 2, 3, ... (master is zero)
  int numParams; //#HIDDEN #READ_ONLY set automatically when run
  int numConstr; //#HIDDEN #READ_ONLY set automatically when run
  float_Array p; //#HIDDEN array of parameters
  int firstConstr; //#HIDDEN #READ_ONLY the first constraint to process
  int lastConstr;  //#HIDDEN #READ_ONLY the last constraint to process
  bool waitLastScript; //#CONDEDIT_ON_master:true set true if last constraint is a script that requires
                       // all previous constraints to have been run
  // Initialization
  void Initialize();
  void setNumParams(int num) {int i; numParams = num; p.Reset(); for (i=0; i < numParams; i++) p.Add(0.0);};
  void setNumConstr(int num) {numConstr = num;};
  void *data; //#HIDDEN #READ_ONLY
  void setDataObj(void *theData) {data = theData;}; 
  void  (*getParams)(void *data, float_Array *p); //#HIDDEN #READ_ONLY
  void  (*setParams)(void *data, float_Array *p); //#HIDDEN #READ_ONLY

  //program interface
  

  //Inter-process communication
  void DirectSlaves();  //master sends control signals to slaves
  void ReceiveOrders();  // slaves respond and start working
  void ReportToMaster(FitDataInfo_Group *data);    // slaves report to master
  void GetFromSlaves(FitDataInfo_Group *data);//master processes data from slaves

  SIMPLE_COPY(RtDistFitMgr);
  COPY_FUNS(RtDistFitMgr, taNBase);
  TA_BASEFUNS(RtDistFitMgr);
};

//RtConstrain either steps once for static constraints or else passes  
// groups of trials for sequential presentation 
// THIS PROCESS IGNORES NEXT-EVENT SPECIFICATIONS 
class RtConstrain : public RtEpoch {   
public: 
  enum opt_method_type {OPT_BFGS, OPT_SIMPLEX, OPT_SUBPLEX}; 
  opt_method_type fit_method;    // which optimization algorithm to use for data fitting 
 
  ModelParam    *params;         // object holding pointers to model parameters 
  int paramSetChangeCount;       // #HIDDEN how many edits to model param accounted for? 
  //Environment   *constraint_env; // Environment object holding constraints -- replaced by FitDataSpec 
  FitDataSpec   *fitData;        // Holds data to be fit -- will be evaluated in order listed 
  optim op;                     //  BFGS optimization object 
  simplex simp;                 // Nelder-Mead simplex optimization object 
  subplex subp;                 // Rowan (1990) subplex optimization object
  RtDistFitMgr dFit;            // Manages multi-processor obj func computation
  enum dp_bound_type {DP_ABS,     //spec'd min/max delta_p are absolute  
		      DP_FRACTION //spec'd min/max delta_p are fractions of the corresponding parameter 
  }; 
  dp_bound_type dp_bounding;    // Are delta_p bounds absolute or relative? 
  float min_delta_p;            // #CONDEDIT_ON_opt_method_type:OPT_BFGS delta_p is added to each parameter in turn to calc finite difference approximation to dE/dp.  min_delta_p provides best accuracy and is used unless it results in zero gradient (this is roughly the precision of the final parameter fit) 
  float max_delta_p;            // #CONDEDIT_ON_opt_method_type:OPT_BFGS if zero gradient, increase delta_p for offending parameter up to max_delta_p 
  RtTrial *trial_proc;          // used for static constraints 
  float errorPenalty;           // how much to penalize model for incorrect responses -- this value is added to objective function for each error 
  bool loopDumpParams;         //Should the best parameter set be saved to a file at each step?
  String paramDumpFile;        //#CONDEDIT_ON_loopDumpParams:true filename to save parameters at each step
 
  float fitEnergy(); // Calculates objective function; changes network state 
  //void calcDE_DP(); // calculates energy gradient in parameter space, calls fitEnergy() 
  void SaveParams(); //save params to file
  void updateParams(); //#IGNORE uses DE_DP to update parameters 
  void findTrialProc(); //#IGNORE find RtTrial instance down sub_proc hierarchy 
  void GetCurEvent() {};  //Disable parent method, which interferes with fit events 
 
  void PrepDoFit(); 
  void Run() { PrepDoFit(); RtEpoch::Run();};    //reset stopping criteria 
  void Step() {PrepDoFit();  RtEpoch::Step();}; 
  void Stop() {if (fit_method == OPT_BFGS) op.isRun = false; \
               else if (simp.isRun) simp.forceStop = true;
               else if (subp.isRun) subp.forceStop = true;}; // soft stop 
  void ForceStop() { RtEpoch::Stop(); }; //#BUTTON #GHOST_ON_running 
  float CalcObjFunc() { return fitEnergy(); }; //#BUTTON #USE_RVAL #GHOST_OFF_running 
  void Loop(); // Don't want to use standard engine 
  void Init() {findTrialProc(); RtEpoch::Init(); op.ReInit(); simp.ReInit(); subp.ReInit();} 
  bool  Crit() {if (fit_method == OPT_BFGS) return !(op.isRun); else if (fit_method == OPT_SIMPLEX) return !(simp.isRun); else return !(subp.isRun); }; 
  void Initialize(); 
  void	Destroy()		{ }; 
  SIMPLE_COPY(RtConstrain); 
  COPY_FUNS(RtConstrain, RtEpoch); 
  TA_BASEFUNS(RtConstrain); 
}; 
 
 
class MPESubNameSpec : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline subclass specification for ModelParamEntry 
public: 
  bool isSubGroup; //#READ_ONLY Is the parameter part of an inlined class in the spec? 
  String SubMemberName; //#CONDEDIT_ON_isSubGroup:true Name of inlined class member 
  void Initialize () {}; 
  void Destroy () {}; 
  SIMPLE_COPY(MPESubNameSpec); 
  COPY_FUNS(MPESubNameSpec, taBase); 
  TA_BASEFUNS(MPESubNameSpec); 
}; 
 
class MPELimitSpec : public taBase {  
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER inline subclass specification for ModelParamEntry 
public: 
  float upper;  //Upper Limit bound for parameter fitting 
  float lower;  //Lower Limit bound for parameter fitting 
  void Initialize () {}; 
  void Destroy () {}; 
  SIMPLE_COPY(MPELimitSpec); 
  COPY_FUNS(MPELimitSpec, taBase); 
  TA_BASEFUNS(MPELimitSpec); 
}; 
 
//Note:  String members below don't display if marked READ_ONLY 
class ModelParamEntry : public taOBase {  //##NO_UPDATE_AFTER 
public: 
  bool     isOrphan;  //#HIDDEN 
  //BaseSpec_MGroup *specs; //#HIDDEN #NO_SAVE holds reference to specs 
  //MemberDef* member;  //#HIDDEN 
  //TAPtr theSpec; 
  String SpecName;   //From which spec? 
  String MemberName; //Member name? 
  MPESubNameSpec subGroup; //Used if the parameter is part of an inlined subclass 
  bool fit; 
  float *pval;      //#HIDDEN #NO_SAVE pointer to actual parameter 
  float delta_p;     //#HIDDEN used to calculate gradient via finite difference 
  float test_delta_p; //#HIDDEN Mark delta_p's when BFGS says convergence, then run again and see if delta_p has decreased further.  If not, then assume converged  
  float val;       //#CONDEDIT_ON_fit:true 
  MPELimitSpec limits; //#CONDEDIT_ON_fit:true 
 
  void setVal(float val); 
  float getVal(); 
 
  void Initialize()  
    {/*specs = NULL;*/ isOrphan = false; fit = false; pval =NULL; delta_p =0.001; test_delta_p=-1.0;}; 
  void UpdateAfterEdit() {if (fit) setVal(val);} 
  void InitLinks() {taBase::Own(subGroup, this); taBase::Own(limits, this); taOBase::InitLinks();}; 
  void Destroy() {CutLinks();}; 
  SIMPLE_COPY(ModelParamEntry); 
  COPY_FUNS(ModelParamEntry, taOBase); 
  TA_BASEFUNS(ModelParamEntry); 
}; 
 
class ModelParamEntry_Group : public taGroup<ModelParamEntry> {  
  // ##NO_TOKENS ##NO_UPDATE_AFTER group of objects 
public: 
  void  UpdateAfterEdit()       {owner->UpdateAfterEdit();}; 
  void	Initialize() 		{ }; 
  void 	Destroy()		{ }; 
  TA_BASEFUNS(ModelParamEntry_Group); 
}; 
 
class ModelParam : public BaseSpec { 
public: 
  int changeCount;        //#HIDDEN How many times param entries been edited? 
                          // This allows other processes to poll and update  
                          // themselves when changeCount increments 
  int_Array fitIndices; //#HIDDEN Which param entries are actively being fit? 
 
  ModelParamEntry_Group paramMembers; 
 
  ModelParamEntry *findParam(String SpecName, String MemberName, String SubMemberName); 
  void updateParamsFromObj(String SpecName, String MemberName, String SubMemberName, float *pval, bool isDefaultParam); 
 
  void updateParamList(); //#BUTTON  gets parameter list from specs 
  void updateParamList_impl(BaseSpec_MGroup *source); 
  void updateParamsFromSpec(TAPtr aSpec); 
 
  void UpdateAfterEdit(); 
 
  void InitLinks() { taBase::Own(paramMembers, this);   
                     taBase::Own(fitIndices, this); BaseSpec::InitLinks();}; 
  void Initialize() {changeCount = 0;}; 
  void Destroy() { CutLinks(); }; 
  SIMPLE_COPY(ModelParam); 
  COPY_FUNS(ModelParam, BaseSpec); 
  TA_BASEFUNS(ModelParam); 
}; 
 
#endif // Rt_h 
 
