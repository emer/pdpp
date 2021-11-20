/* -*- C++ -*- */ 
/*============================================================================= 
// 
// This file is part of the RNS++ software package, which is                  // 
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
 
// rns.cc 
#include <math.h> 
#define truncf(a) (a > 0.0 ? a-fabs(fmod(a,1.0)) : a+fabs(fmod(a,1.0)))
#include <limits.h> 
#include "rns.h" 
 
////////////////////////// 
//  	Unit		// 
////////////////////////// 
 
void RtUnit::Initialize() { 
  //prv_ext_flag = Unit::NO_EXTERNAL; 
  spec.SetBaseType(&TA_RtUnitSpec); 
  prv_targ = 0.0f; 
  prv_ext = 0.0f; 
  prv_act = 0.0f; 
  act_raw = 0.0f; 
  prv_net = 0.0f; 
  volt_clamp = false; 
  max_axon_delay = 150; //ticks 
  habit = 1.0f; 
  acts.SetSize(max_axon_delay); 
  exts.SetSize(max_axon_delay); 
  acts.InitVals(0.0f, 0, max_axon_delay); 
  exts.InitVals(0.0f, 0, max_axon_delay); 
} 
 
void RtUnit::InitLinks() { 
  Unit::InitLinks(); 
  taBase::Own(ext_flags, this); 
  taBase::Own(targs, this); 
  taBase::Own(exts, this); 
  taBase::Own(acts, this); 
} 
 
 
void RtUnit::InitExterns() { 
  //prv_ext_flag = ext_flag; 
  prv_targ = targ; 
  prv_ext = ext; 
  Unit::InitExterns(); 
} 
 
void RtUnit::StoreState() { 
 
} 
 
void RtUnit::UpdateAfterEdit() { 
  acts.SetSize(max_axon_delay); 
  exts.SetSize(max_axon_delay); 
  Unit::UpdateAfterEdit(); 
} 
 
////////////////////////// 
//  	UnitSpec	// 
////////////////////////// 
 
void RtUnitSpec::Initialize() { 
  min_obj_type = &TA_RtUnit; 
  dt = 0.001f; 
#ifndef RT_1_0 
  time_const = 100.0f;  // 1/e response time = 1000ms/time const 
  calc_habituation = false; 
  habit_rate = 1.0f; 
  habit_time_const = 100.0f; 
  habit_thresh = 0.0f; 
  calc_act_trace = false; 
  at_rise_rate = 100.0f; 
  at_decay_rate = 1.0f; 
  doRecurExcit = true; 
  recur_excit_type = NONE; 
  recur_excit_gain_n = 1.0f; 
  recur_excit_level_a = 0.0f; 
  recur_excit_wt = 1.0f; 
  pass_decay = 1.0f; 
  bias_excit = 0.0f; 
  hyperpol = 0.5f; 
  re_a_pow_n = 0.0f; 
  NMDA_thresh = 0.5f; 
  NMDA_gain = 1.0f; 
  DA_NMDA_gain = 0.0f; 
  useDA_NMDA_gain = false; 
  ne_gain = 0.0f; 
  ne_inhib_gain = 0.0f; 
  soft_clamp = false; 
  soft_clamp_gain = 1.0f; 
  norm_input_wt = false; 
  norm_output_wt = false; 

  doIntAndFire = false; 
  iafThresh = 0.2; 
#endif 
 
  D2_inhib_habit_thresh = 0.5; 
  D2_inhib_habit_gain = 0.0; 
 
  bias_spec.SetBaseType(&TA_RtConSpec); //unused 
  initial_act.type = Random::UNIFORM; 
  initial_act.mean = .5f; 
  initial_act.var = 0.0f; 
 
  legacy_norm = true; 
  noise.type = Random::GAUSSIAN; 
  noise.mean = 0.0f; 
  noise.var = .1; 
  sqrt_dt = sqrtf(dt); 
  addNoise = false; 
  govern_eq = &Rt_Shunting; 

  taBase::Own(custGovEq, this);
  taBase::Own(gov, this);
  taBase::Own(rec_excit, this);
  taBase::Own(nmda, this);
  taBase::Own(dopa, this);
  taBase::Own(ach, this);
  taBase::Own(sero, this);
  taBase::Own(act_trace, this);
  taBase::Own(habit, this);
  taBase::Own(softclamp, this);
  taBase::Own(wt_norm, this);
} 
 
void RtUnitSpec::InitLinks() { 
  UnitSpec::InitLinks(); 
  taBase::Own(initial_act, this); 
} 
 
void RtUnitSpec::InitState(Unit* u) { 
  UnitSpec::InitState(u); 
  RtUnit* ru = (RtUnit*)u; 
  ru->act = initial_act.Gen(); 
  ru->net_excit = 0.0f; 
  ru->net_inhib = 0.0f; 
  ru->net_d1 = 0.0f; 
  ru->net_d2 = 0.0f; 
  ru->net_ne = 0.0f; 
  ru->net_ach = 0.0f;
  ru->net_sero = 0.0f;
  ru->net_dopa = 0.0f;
  ru->sum_in_wt = 0.0f; 
  ru->sum_out_wt = 0.0f; 
  //ru->num_in_wt = 0; 
  //ru->num_out_wt = 0; 
 
  // set initial net value too.. 
  ru->net = 0.0f; //unused 
  ru->prv_act = ru->act; 
  ru->da = 0.0f; 
 
  ru->prv_ext = 0.0f; 
  ru->prv_targ = 0.0f; 
  ru->act_raw = 0.0f; 
  ru->prv_net = 0.0f; 
  /* 
  ru->ext_flags.Reset(); 
  ru->targs.Reset(); 
  ru->exts.Reset(); 
  ru->acts.Reset(); 
  */ 
} 
 
void RtUnitSpec::Prep_Compute_Net(RtUnit *u, float abs_time) { 
  u->sum_in_wt = 0.000001f; u->sum_out_wt = 0.000001f; //must complete before Compute_Net 
  //u->num_in_wt = 0; u->num_out_wt = 0; 
  u->net_excit =0.0f; u->net_inhib =0.0f; u->net_d1 = 0.0f; u->net_d2 = 0.0f; 
  u->net_ne = 0.0f; u->net_ach = 0.0f; u->net_sero = 0.0f; u->net_dopa = 0.0f;
 
 if (u->isLearnEvent) { 
     if ((abs_time - u->lEvStart) > samp_learn.dur) u->isLearnEvent = false;  
     return; 
  } 
 
  float randVal = Random::ZeroOne(); 
 
  if (samp_learn.on && (randVal <= (samp_learn.prob*dt))) { 
    u->isLearnEvent = true; 
    u->lEvStart = abs_time; 
  } 
} 

//#define checkUniqueBit(p, mname, newVal) if (p) {if (p->newVal != newVal) SetUnique(#mname, true); } 

#define checkUniqueBit(p, mname, newVal) if (p) {if (GetUnique(#newVal)) SetUnique(#mname, true); } 

#define checkUnique(p, mname, mfield, newVal) checkUniqueBit(p, mname, newVal); mname.mfield = newVal

void RtUnitSpec::UpdateAfterEdit() { 
  UnitSpec::UpdateAfterEdit();
  if((dt < 0.0f) || (dt > 1.0f)) 
    taMisc::Error("Warning: dt must be in the [0..1] range!"); 
  //if(((recur_excit_type != NONE) || (recur_excit_type != LINEAR)) && ((NMDA_gain != 1.0) || (useDA_NMDA_gain))) taMisc::Error("Warning:  For realism, Recur. Excit. type should be NONE or LINEAR (with zero thresh) if NMDA gain not 1 or DA_NMDA gain used"); 

  sqrt_dt = sqrtf(dt); 

  // Assign old objects to new sub-objects
#ifndef RT_1_0
  int enumTmp;
  RtUnitSpec *p = (RtUnitSpec*) FindParent();

  checkUnique(p, rec_excit, re_a_pow_n, re_a_pow_n); //rec_excit.re_a_pow_n = re_a_pow_n;
  checkUnique(p, rec_excit, used, doRecurExcit); //rec_excit.used = doRecurExcit;
  checkUnique(p, rec_excit, n, recur_excit_gain_n); //rec_excit.n = recur_excit_gain_n;
  checkUnique(p, rec_excit, a, recur_excit_level_a); //rec_excit.a = recur_excit_level_a;
  checkUnique(p, rec_excit, wt, recur_excit_wt); //rec_excit.wt = recur_excit_wt;
  
  checkUniqueBit(p, rec_excit, recur_excit_type);
  enumTmp = (int) recur_excit_type;
  rec_excit.type = (RecExcInfo::RecurExcitType) enumTmp;

  checkUnique(p, gov, time_const, time_const); //gov.time_const = time_const;
  checkUnique(p, gov, pass_decay, pass_decay); //gov.pass_decay = pass_decay;
  checkUnique(p, gov, bias_excit, bias_excit); //gov.bias_excit = bias_excit;
  checkUnique(p, gov, hyperpol, hyperpol); //gov.hyperpol = hyperpol;
  
  nmda.used = true; //default for previously non-existent variable
  checkUnique(p, nmda, NMDA_gain, NMDA_gain); //nmda.NMDA_gain = NMDA_gain;
  checkUnique(p, nmda, NMDA_thresh, NMDA_thresh); //nmda.NMDA_thresh = NMDA_thresh;
  /*  
  >>> neuroModInfo
  da
  ach
  sero
  */
  ne.used = true;
  checkUnique(p, ne, inh_wt, ne_inhib_gain); //ne.inh_wt = ne_inhib_gain;
  checkUnique(p, ne, ex_mult, ne_gain); //ne.ex_mult = ne_gain;

  checkUnique(p, act_trace, used, calc_act_trace); //act_trace.used = calc_act_trace;
  checkUnique(p, act_trace, rise_rate, at_rise_rate); //act_trace.rise_rate = at_rise_rate;
  checkUnique(p, act_trace, decay_rate, at_decay_rate); //act_trace.decay_rate = at_decay_rate;
  
  checkUnique(p, habit, used, calc_habituation); //habit.used = calc_habituation;
  checkUnique(p, habit, rate, habit_rate); //habit.rate = habit_rate;
  checkUnique(p, habit, time_const, habit_time_const); //habit.time_const = habit_time_const;
  checkUnique(p, habit, thresh, habit_thresh); //habit.thresh = habit_thresh;

  checkUnique(p, softclamp, used, soft_clamp); //softclamp.used = soft_clamp;
  checkUnique(p, softclamp, gain, soft_clamp_gain); //softclamp.gain = soft_clamp_gain;

  checkUnique(p, wt_norm, input, norm_input_wt); //wt_norm.input = norm_input_wt;
  checkUnique(p, wt_norm, output, norm_output_wt); //wt_norm.output = norm_output_wt;
#endif
  rec_excit.re_a_pow_n = pow(rec_excit.a,rec_excit.n);

} 
 
void RtUnitSpec::Compute_Net(Unit* u) { 
  Con_Group *recv_gp; 
  RtUnit* ru = (RtUnit*)u; 
  ru->prv_net = ru->net; // save current net as previous 
 
    // sum_in_wt, sum_out_wt initialized in Prep_Compute_Net() 
    int g=0; 
    //printf("About to Iterate Compute_Net()\n"); 
    FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) { 
      Layer* fmlay = recv_gp->prjn->from; 
      if(fmlay->lesion) // || (fmlay->ext_flag & Unit::EXT)) 
	continue;		// don't get from the clamped layers 
      //printf("Calling RtCon_Group::Compute_Net()..."); 
      u->net += ((RtCon_Group*)recv_gp)->Compute_Net((RtUnit*)ru); 
      //u->net is unused here 
    } 
} 
 
void RtUnitSpec::Compute_Wt_Norm(RtUnit *u) { //normalizes both incoming & outgo 
  //Normalize input weights -- must do it here after Compute_Net() completed, 
  //  so that all output weight sums have been computed 
  // What if both pre- and post- layer wts are normalized, and the layers 
  // are different sizes?  Then smaller layers wts can add up to  
  // larger_layer.size/smaller_layer.size 
  // -- This will work only if normalization is just between two layers 
  //printf("About to norm wts..."); fflush(stdout); 
 
  float norm_scale = 1.0; 
  Layer *lay = (Layer *) u->GetOwner(&TA_Layer);  
  //RtUnit *au; 
  int num_units = lay->n_units;  
 
  //printf("num units = %i ",num_units); fflush(stdout); 
    if (wt_norm.input) { 
      Con_Group *recv_gp; 
      int g=0; 
    FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) { 
      RtConSpec* rs = (RtConSpec*)(recv_gp->spec.spec);  
      if (rs->doRWtNorm()) { 
	CON_GROUP_LOOP(recv_gp, { 
	     if (!legacy_norm) norm_scale = recv_gp->size > num_units ? 
		    ((float)recv_gp->size)/((float)num_units) : 1.0; 
	     recv_gp->Cn(i)->wt*=norm_scale/u->sum_in_wt; 
	   } 
        ); 
      } 
    } 
    } 
 
   // Normalize output weights 
   if (wt_norm.output) { 
      Con_Group *send_gp; 
      int g=0; 
      FOR_ITR_GP(Con_Group, send_gp, u->send., g) { 
        RtConSpec* rs = (RtConSpec*)(send_gp->spec.spec);  
        if (rs->doSWtNorm()) { 
	  CON_GROUP_LOOP(send_gp,  { 
	     if (!legacy_norm) norm_scale = send_gp->size > num_units ? ((float)send_gp->size)/((float)num_units) : 1.0;  
	     send_gp->Cn(i)->wt*=norm_scale/u->sum_out_wt; 
	  } 
	  ); 
        } 
      } 
    } 
 
} 
 
void RtUnitSpec::Compute_Act_impl(RtUnit* u) { //computes act 
  float noiseVal = addNoise ? sqrt_dt*noise.Gen() : 0.0f;

  //Vector map transform
  if (VectorMap.active) { 
    u->net_excit = VectorMap.gain*MPLUS(VectorMap.hwidth-
					fabs(VectorMap.peak - u->net_excit)); 
  } 
 
  //get da, the change (delta) of activation for this unit at this time step
  u->da = noiseVal + gov.time_const*dt*(*govern_eq)(this, u); 
 
  // Apply spiking if desired
  if (Spiking.doIntAndFire) { 
    if (u->act == 1.0) u->act = 0.0;      //if just spiked, then reset 
    else if (u->act > Spiking.iafThresh) u->act = 1.0;  // if above threshold, 
                                                        //  then spike 
  } 

  // Update unit activity 
  u->act+=u->da; 
  u->act = act_range.Clip(u->act); 
  u->acts.PushHead(u->act); //store act in circular buffer 
 
  //update unit habituation and activity trace state vars
  if (habit.used) u->habit+= habit.time_const*dt*((1.0f-u->habit) - habit.rate*u->habit*MPLUS(u->act-habit.thresh)); 
  if (act_trace.used) u->act_trace+= dt*(act_trace.rise_rate*MPLUS(u->act- u->act_trace) - act_trace.decay_rate*u->act_trace); 
  else u->act_trace = u->act;
} 
 
float RtCalcRecExcit(RtUnitSpec* sp, RtUnit* u) {//finds recurrent excitation val
  register float re = 0.0f, tmp; 
  if (sp->rec_excit.used != true) return 0.0f; 
 
  switch (sp->rec_excit.type) { 
    case RecExcInfo::NONE: return re; 
    case RecExcInfo::LINEAR: re = (u->act >= sp->rec_excit.a) ? sp->rec_excit.n \
		   *(u->act - sp->rec_excit.a) : 0.0f; break; 
    case RecExcInfo::STEP: re = (u->act >= sp->rec_excit.a) ? sp->rec_excit.n : 0.0f; 
		   break; 
    case RecExcInfo::SQUELCH: re = (u->act >= sp->rec_excit.a) ? \
		   sp->rec_excit.n *u->act : 0.0f; break; 
    case RecExcInfo::XSQUARED: re = u->act*u->act*sp->rec_excit.n; break; 
    case RecExcInfo::SIGMOID: tmp = pow(u->act, sp->rec_excit.n); \
                   re = tmp/(tmp+sp->rec_excit.re_a_pow_n); break; 
  } 
  re*=sp->rec_excit.wt; 
  return re; 
} 

#define APPLY_NMOD(nval, nsp) if (nsp->used) {ex+= nval*nsp->ex_wt; in+=nval*nsp->inh_wt; exgain+=nval*nsp->ex_mult; ingain += nval*nsp->inh_mult;}

void RtCalcNeuroMod(RtUnitSpec* sp, RtUnit* u, float &ex, float &exgain, float &in, float &ingain) {
  exgain = 1.0;
  ingain = 1.0;
  ex = 0.0;
  in = 0.0;
  register float nval;
  NeuroModInfo *nsp;

  nval = u->net_ne; nsp = &(sp->ne);
  APPLY_NMOD(nval, nsp);

  nval = u->net_ach; nsp = &(sp->ach);
  APPLY_NMOD(nval, nsp);

  nval = u->net_sero; nsp = &(sp->sero);
  APPLY_NMOD(nval, nsp);

  nval = u->net_d1+u->net_d2+u->net_dopa; nsp = &(sp->dopa);
  APPLY_NMOD(nval, nsp);

  // dopamine applied as separate mechanism from existing code
} 

#define Rt_CalcEqnTerms \
  float ex, exgain, in, ingain; \
  register float re = RtCalcRecExcit(sp,u);  \
  RtCalcNeuroMod(sp, u, ex, exgain, in, ingain); \
  register float da_inhib = 1.0 - u->habit - sp->D2_inhib_habit_thresh; \
  da_inhib = MPLUS(da_inhib)*sp->D2_inhib_habit_gain; \
  if (u->act > sp->nmda.NMDA_thresh) { \
    exgain *= sp->nmda.NMDA_gain;  \
    if (sp->useDA_NMDA_gain) exgain*=u->net_d1; \
  } //>>> where is DA_NMDA_gain??? 

float Rt_Shunting(RtUnitSpec* sp, RtUnit* u) { 

   Rt_CalcEqnTerms;

   //d_act = (1 - act)*(exgain*input_excit + bias) 
   //        - (act + hyperpol)*ingain*(net_inhib + other_inhib + passive_decay)
   return (   (1.0f-u->act)
	         *(exgain*(ex+u->net_excit+re) + sp->gov.bias_excit)
	    - (u->act+sp->gov.hyperpol)
	         *(ingain*(in+u->net_inhib) + sp->gov.pass_decay + da_inhib)); 
  
} 


float Rt_Tracking(RtUnitSpec* sp, RtUnit* u) {  
  //activity TRACKS simple sum of excitatory and inhibitory inputs
  Rt_CalcEqnTerms;
  return exgain*(u->net_excit+ex+re+sp->gov.bias_excit) -ingain*(u->net_inhib+in+da_inhib+sp->gov.pass_decay) - u->act;
} 
 
float Rt_Additive(RtUnitSpec* sp, RtUnit* u) { 
  //activity CHANGES as simple sum of excitory and inhibitory input
  //    (good for integrate-and-fire)
  Rt_CalcEqnTerms;
  return exgain*(u->net_excit+ex+re+sp->gov.bias_excit) -ingain*(u->net_inhib+in+da_inhib+sp->gov.pass_decay);
} 
 

float Rt_Usher_Mc(RtUnitSpec* sp, RtUnit* u) { // Usher & McClelland (2001) 
  // To implement Usher & McClelland eqn, must also set noise parameter in 
  //     UnitSpec and set UnitSpec act_range.min = 0 
  // Noise adjusted in unit spec and added separately 
  // Tau is 1/time_const in unit spec -- tau scales dx/dt, 
  //     but noise not scaled by 1/sqrt(tau) 
 
  //===== Variable names in eqn below:==== 
#define rho_i exgain*(u->net_excit+ex+re+sp->gov.bias_excit) //excitation
#define kx_i  sp->gov.pass_decay*u->act   //passive decay 
                                          //(passive decay baseline ignored)
#define beta_x_sum_lat_inhib ingain*(u->net_inhib+in+da_inhib) 
                                          // lateral (and other afferent) 
                                          //inhibition (beta adjusted by wt_gain 
                                          //of afferent inhibitory ConSpecs) 
  Rt_CalcEqnTerms;
  return rho_i - kx_i - beta_x_sum_lat_inhib;
} 
 
float Rt_Cust_Script(RtUnitSpec* sp, RtUnit* u) { //implements user-defined script governing equation for RtUnit state update
  sp->custom_script_u = u; //gives script access to RtUnit
  sp->custGovEq.Run();
  return sp->custom_script_da;
}

// -----------------------------------------
void RtUnitSpec::Compute_Act(Unit* u) { 
  RtUnit* rbu = (RtUnit*)u; 
  if (rbu->volt_clamp) {rbu->acts.PushHead(rbu->act); return;}   
                         //no update if artificially clamped, but SAVE STATE! 
  rbu->prv_act = rbu->act;	// save current act as previous 
  if((u->ext_flag & Unit::EXT) && !softclamp.used) { 
      rbu->act = rbu->act_raw = rbu->ext; 
      rbu->net = 0.0f;  
      rbu->exts.PushHead(rbu->ext); //save external inputs 
      return; 
  } 
  else if (u->ext_flag & Unit::EXT) rbu->net_excit += rbu->ext*softclamp.gain; 
 
  Compute_Act_impl(rbu); 
} 
 
void RtUnitSpec::Compute_dWt(Unit* u) { 
  Con_Group *recv_gp = NULL; 
  RtUnit* ru = (RtUnit*)u; 
  int g=0; 
    //printf("About to Iterate Compute_dWt()\n"); 
    FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) { 
      Layer* fmlay = recv_gp->prjn->from; 
      if(fmlay->lesion) // || (fmlay->ext_flag & Unit::EXT)) 
	continue;		// don't get from the clamped layers 
      //printf("Calling RtCon_Group::Compute_dWt()..."); 
      ((RtCon_Group*)recv_gp)->Compute_dWt((RtUnit*)ru); 
    } 
} 
 
 
////////////////////////// 
//  	Con, Spec	// 
////////////////////////// 
 
void RtConSpec::Initialize() { 
  //min_obj_type = &TA_RtCon_Group; 
  //min_obj_type = &TA_Projection; 
  //min_con_type = &TA_RtCon; 
  axon_delay = 0.0;
  delay = 0; 
  lrate = .25; 
  post_lthresh = 0.0f; 
  wt_gain = 1.0; 
 
  synapse_sat = 10000.0f; //default virtually no limit 
  habit_post_learn = false; 
 
  con_type = EXCIT; 
  learning_law = NO_LEARNING; 

  useFixedWts = false; 
  d2n_dw = 4.0; 
  d2n_thresh = 0.5; 
  pos_d2wtgain = 1.0; 

  dt = 0.001; 
  legacy_ignore_dt = true; 

#ifndef RT_1_0
  use_elig = false; 
  elig_type = TRACK_ONE; 
  elig_rise_rate = 1.0; 
  elig_decay_rate = 1.0; 
  elig_inh_decay_rate = 0.0; 
  baseline_wt = 0.5; 
  thresh_type = RECT; 
  decay_wts = false; 
  thresh = 0.0f; 
  decay = 0.0f; 
  excludeRwtNorm = false; 
  excludeSwtNorm = false; 

#endif

  taBase::Own(CustLearnLaw, this);
  taBase::Own(signal_func, this);
  taBase::Own(LTP, this);
  taBase::Own(LTD, this);
  taBase::Own(pass_wt_decay, this);
  taBase::Own(elig_trace, this);
  taBase::Own(exclude_norm, this);
} 


void RtConSpec::UpdateAfterEdit() { 

#ifndef RT_1_0
  int tmp;
  RtConSpec *p = (RtConSpec*) FindParent();

  //set vals ignored unless unique bit set!  the "checkUnique()" macro fixes this
  checkUniqueBit(p, signal_func, thresh_type);
  tmp = (int) thresh_type; signal_func.sig_type = (SigFuncInfo::SigFuncType) tmp;
  checkUnique(p, signal_func, thresh, thresh); //signal_func.thresh = thresh;

  checkUnique(p, pass_wt_decay, on, decay_wts); //pass_wt_decay.on = decay_wts;
  checkUnique(p, pass_wt_decay, rate, decay); // pass_wt_decay.rate = decay;
  checkUnique(p, pass_wt_decay, baseline, baseline_wt); //pass_wt_decay.baseline = baseline_wt;

  checkUnique(p, elig_trace, used, use_elig); //elig_trace.used = use_elig;
  checkUniqueBit(p, elig_trace, elig_type);
  tmp = (int) elig_type;  elig_trace.elig_type = (EligTraceInfo::EligType) tmp;

  checkUnique(p, elig_trace, rise, elig_rise_rate); //elig_trace.rise = elig_rise_rate;
  checkUnique(p, elig_trace, decay, elig_decay_rate); //elig_trace.decay = elig_decay_rate;
  checkUnique(p, elig_trace, inhib, elig_inh_decay_rate); //elig_trace.inhib = elig_inh_decay_rate;
  
  checkUnique(p, exclude_norm, rwt, excludeRwtNorm); //exclude_norm.rwt = excludeRwtNorm;
  checkUnique(p, exclude_norm, swt, excludeSwtNorm); //exclude_norm.swt = excludeSwtNorm;

  axon_delay = dt*(float)delay;
#endif

  // need to compute delay in ticks from axon_delay, dt
  delay = (int) (axon_delay/dt); delay = delay > 0 ? delay : 0; //can't be neg
  if (learning_law == HEBB_DECAY) pass_wt_decay.on = true; 
  ConSpec::UpdateAfterEdit(); 
} 
 
void RtConSpec::updateDt(float adt, bool a_legacy_ignore_dt) {
      dt = adt;  
      legacy_ignore_dt = a_legacy_ignore_dt;   
      delay = (int) (axon_delay/dt); delay = delay > 0 ? delay : 0; //can't be neg
}

void RtCon_Group::Initialize() { 
  spec.SetBaseType(&TA_RtConSpec); 
} 
 
 
////////////////////////// 
//  	DelayBuffer	// 
////////////////////////// 
 
void float_DelayBuffer::Initialize() { 
  //st_idx = 0; 
  //length = 0; 
  head = 0; 
  max_len = 0; 
   
} 
 
void float_DelayBuffer::Copy_(const float_DelayBuffer& cp) { 
  //st_idx = cp.st_idx; 
  //length = cp.length; 
  head = cp.head; 
  max_len = cp.max_len; 
} 
/* 
void float_DelayBuffer::Add(const float& item) { 
  if((st_idx == 0) && (length >= size)) { // must be building up the list, so add it 
    float_Array::Add(item); 
    length++; 
    return; 
  } 
  SafeEl(CircIdx(length++)) = item;	// set it to the element at the end 
} 
*/ 
 
void float_DelayBuffer::SetSize(int sz) { 
  //int diff = sz - size; 
  int i; 
 
  if(size < sz) {  //enlarge 
    for (i=size; i<sz; i++) Add(0.0f);   
    //for (i=size; i<sz; i++) Set(i,0.0);  
  } 
  else if (size > sz) { //contract and reset (wipes out data) 
    for(i=size-1; i>=sz; i--) Remove((uint)i); 
    for (i=0; i < sz; i++) Set(i,0.0); 
    head = 0; 
  } 
 
  max_len = sz; 
  size = sz; 
} 
 
void float_DelayBuffer::Reset() { 
  float_Array::Reset(); 
  //st_idx = 0; 
  //length = 0; 
  head = 0; 
  max_len = 0; 
} 
 
 
 
////////////////////////// 
//  	RtTrial	// 
////////////////////////// 
 
void RtTrial::Initialize() { 
  responseMade = false; 
  responseCorrect = false; 
  responseTime = -1.0f; 
  rewardIfCorrect = false; 
  abs_time = 0.0f; 
  trial_time = 0.0f; 
  dt = 0.001f; 
  legacy_ignore_dt = true; //so that legacy code works 
 
  trialEnded = false; 
  useFixedWts = false; 
 
  log_counter = true; 
  update_period_ticks = 1; 
  update_countdown = 1; 
   
  min_unit = &TA_RtUnit; 
  min_con_group = &TA_RtCon_Group; 
  min_con = &TA_RtCon; 
} 
 
void RtTrial::InitLinks() { 
  TrialProcess::InitLinks(); 
  // changed to apparently create ResponseStat automatically... 
  if(!taMisc::is_loading && (loop_stats.size == 0)) { 
    ResponseStat* st = (ResponseStat*)loop_stats.NewEl(1, &TA_ResponseStat); 
    st->CreateAggregates(Aggregate::COPY); 
    st->UpdateAfterEdit(); 
    // Have responsStat automatically find trial process: 
    taBase::SetPointer((taBase**)&(st->tr), this); 
    //st->tr = this; 
  } 
} 
/* 
void RtTrial::Compute_dWt() {  //Is this necessary? 
  if(network == NULL)	return; 
  network->Compute_dWt(); 
} 
*/ 
 
void RtTrial::UpdateAfterEdit() { 
  //Event_MGroup *events = &(environment->events); 
  /*  
  // Hack 5/10/02 JWB -- problem is that RtEvent.owner isn't properly set 
  // for some reason.  This code goes through and sets it.  This is *really* 
  // klunky 
  RtEvent *theEvent; 
  taLeafItr i; 
  FOR_ITR_EL(RtEvent, theEvent, events-> ,i)  theEvent->eventGpList = events; 
  */ 
 
  TrialProcess::UpdateAfterEdit(); 
} 
 
void RtTrial::Prep_Compute_Net() { 
  Layer* layer; 
  int i; 
  for(i=network->layers.leaves-1; i>= 0; i--) { 
    layer = ((Layer*) network->layers.Leaf(i)); 
    if(!layer->lesion) { 
      RtUnit* u; 
      taLeafItr u_itr; 
      FOR_ITR_EL(RtUnit, u, layer->units., u_itr) { 
	u->Prep_Compute_Net(abs_time); 
      } 
    } 
  } 
} 
 
void RtTrial::Compute_Wt_Norm() { 
  Layer* layer; 
  int i; 
  RtUnitSpec *sp; 
  for(i=network->layers.leaves-1; i>= 0; i--) { 
    layer = ((Layer*) network->layers.Leaf(i)); 
    if(!layer->lesion) { 
      RtUnit* u; 
      taLeafItr u_itr; 
      FOR_ITR_EL(RtUnit, u, layer->units., u_itr) { 
	sp = (RtUnitSpec*)(u->spec.spec); 
	sp->Compute_Wt_Norm(u); 
      } 
    } 
  } 
} 
 
void RtTrial::Compute_Act() { 
  Prep_Compute_Net(); 
  network->Compute_Net();	// two-stage update of nets and acts 
  network->Compute_Act(); 
  network->Compute_dWt();       //Josh  11/15/02 
  Compute_Wt_Norm(); 
} 
 
 
void RtTrial::StoreState() { 
  Layer* layer; 
  int i; 
  for(i=network->layers.leaves-1; i>= 0; i--) { 
    layer = ((Layer*) network->layers.Leaf(i)); 
    if(!layer->lesion) { 
      RtUnit* u; 
      taLeafItr u_itr; 
      FOR_ITR_EL(RtUnit, u, layer->units., u_itr) 
	u->StoreState(); 
    } 
  } 
} 
 
 
int RtTrial::GetUnitBufSize(bool in_updt_after) { 
  if(network == NULL) return -1; 
  Layer* layer; 
  taLeafItr l_itr; 
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) { 
    if((layer->lesion) || (layer->units.leaves == 0)) 
      continue; 
    RtUnit* u = (RtUnit*)layer->units.Leaf(0); 
    RtUnitSpec* us = (RtUnitSpec*)u->spec.spec; 
    if(us->dt != dt) { 
      dt = us->dt; 
      taMisc::Error("RtTrial: Copying dt from unit spec:", (String)dt); 
      if(!in_updt_after) 
	UpdateAfterEdit();	// get new tick values 
    } 
    return u->ext_flags.max_len; 
  } 
  return -1; 
} 
 
 
void RtTrial::Init_impl() { 
  RtEventSpec *ms; 
  if (cur_event != NULL) { 
    ms = (RtEventSpec*)(cur_event->spec.spec); 
    ms->ResetTriggers(); 
  } 
 
  TrialProcess::Init_impl(); 
  trial_time = 0.0f; 
  responseTime = -1.0f; 
  responseMade = false; 
  responseCorrect = false; 
  trialEnded = false; 
} 
 
void RtTrial::UpdateDt(BaseSpec_MGroup *specs) { 
  BaseSpec *aspec; 
  RtConSpec *rs=NULL; 
  RtUnitSpec *ru=NULL; 
  taLeafItr i; 
 
  // Find list of spec objects, then iterate and set dt in each 
  FOR_ITR_EL(BaseSpec, aspec, specs->, i) { 
    if (aspec->InheritsFrom(&TA_RtConSpec)) { 
      rs = (RtConSpec *)aspec;  
      rs->updateDt(dt, legacy_ignore_dt);
      UpdateDt(&(rs->children)); 
    } 
    else if (aspec->InheritsFrom(&TA_RtUnitSpec)) { 
      ru = (RtUnitSpec *)aspec; ru->dt = dt; 
      UpdateDt(&(ru->children)); 
    } 
  } 
} 
 
void RtTrial::Run() { 
  // update dt in specs if necessary before running 
  BaseSpec_MGroup *specs = &(network->proj->specs); //Get project specs 
  UpdateDt(specs); 
 
  TrialProcess::Run(); 
} 
   
void RtTrial::Loop() { 
  RtEventSpec *ms; 
  bool isEventUpdate; 
  if(cur_event != NULL) {  // an explicit reset is required to define new pats 
    ms = (RtEventSpec*)(cur_event->spec.spec); 
 
    isEventUpdate = ms->CheckTriggers(network,this); 
    // apply events only if event updated or first iteration 
    // This is basically initialization stuff for each new trial: 
    if (isEventUpdate || trial_time <= dt) { 
      ms->ZeroInputLayers(network);  
      cur_event->ApplyPatterns(network);  
      responseMade = false; 
    } 
  } 
   
  Compute_Act(); // updates weights, too 
 
  // Update display? 
  if (!(--update_countdown)) { 
    UpdateDisplays();  
    update_countdown = update_period_ticks >=1 ? update_period_ticks : 1; 
  } 
 
  // increment time 
  abs_time += dt;			 
  trial_time += dt; 
 
  //TrialProcess::Loop(); 
} 
 
void RtTrial::Final() { 
  if(cur_event != NULL) { 
    RtEventSpec *ms = (RtEventSpec*)(cur_event->spec.spec); 
    //RtEvent* ev = (RtEvent *)cur_event; 
    ms->ResetTriggers(); 
    // If running constraint events, then record results 
    /* 
    if (ms->isConstraint) { 
      ev->data.actualRt = responseTime; 
      ev->data.isCorrect = responseCorrect; 
    } 
    */ 
  } 
  TrialProcess::Final(); 
} 
 
void RtTrial::GetCntrDataItems() { 
  if(cntr_items.size >= 2) 
    return; 
  SchedProcess::GetCntrDataItems(); 
  DataItem* it = (DataItem*)cntr_items.New(1,&TA_DataItem); 
  it->SetNarrowName("Abs. Time"); 
  it = (DataItem*)cntr_items.New(1,&TA_DataItem); 
  it->SetNarrowName("Trial Time"); 
} 
   
void RtTrial::GenCntrLog(LogData* ld, bool gen) { 
  TrialProcess::GenCntrLog(ld, gen); 
  if(gen) { 
    if(cntr_items.size < 2) 
      GetCntrDataItems(); 
    ld->AddFloat(cntr_items.FastEl(0), abs_time); 
    ld->AddFloat(cntr_items.FastEl(1), trial_time); 
  } 
} 
 
void RtTrial::setFixedWts(bool isFixed) { //for constraint fitting 
  BaseSpec_MGroup *specs = &(network->proj->specs); //Get project specs 
  BaseSpec *aspec; 
  RtConSpec *rs; 
  taLeafItr i; 
 
  // Find list of spec objects, then iterate and set each for fitting 
  FOR_ITR_EL(BaseSpec, aspec, specs->, i) { 
    if (aspec->InheritsFrom(&TA_RtConSpec)) { 
      rs = (RtConSpec *)aspec; 
      rs->useFixedWts = isFixed; 
    } 
  } 
  useFixedWts = isFixed; 
}   
 
 
 
///////// Response Stat //////////////// 
void ResponseStat::Initialize() { 
  responseSinceWhen = NULL; 
  response_type = THRESHOLD; 
  minResponseTime = 0.0f; 
  threshold = .5f; 
  settle_thresh = 0.001f; 
  trg_lay = NULL; 
  Init(); 
} 
 
void ResponseStat::InitLinks() { 
  Stat::InitLinks(); 
} 
 
void ResponseStat::CutLinks() { 
  Stat::CutLinks(); 
  taBase::DelPointer((TAPtr*)&trg_lay); 
} 
 
void ResponseStat::NameStatVals() { 
  Stat::NameStatVals(); 
  resp.AddDispOption("MIN=0, "); 
  resp.AddDispOption("TEXT, "); 
} 
 
void ResponseStat::InitStat() { 
  //float init_val = InitStatVal(); 
  //resp.InitStat(init_val); 
  //resp_time.InitStat(init_val); 
  
  TrialInit(); 
  Stat::InitStat(); 
} 
 
 
void ResponseStat::TrialInit() {  //for start of trial 
  resp.Init(); 
  resp_time.Init(); 
  resp = 0.0f; 
  resp_time = 0.0f; 
  isResponse = false; 
  isCorrect = false; 
  if (tr != NULL) {tr->responseMade = false; tr->responseCorrect = false;} 
} 
 
 
bool ResponseStat::Crit() { 
  if(!has_stop_crit)    return false; 
  if(n_copy_vals > 0)   return copy_vals.Crit(); 
  return resp.Crit(); 
} 
 
bool ResponseStat::CheckLayerInNet() { 
  bool ok = Stat::CheckLayerInNet(); 
  if(!ok) 
    return false; 
  if(trg_lay == NULL) return true; 
  if(trg_lay->own_net == network) 
    return true; 
 
  taMisc::Error("*** ResponseStat:", name, "layer:", trg_lay->name, 
		 "is not in current network, finding one of same name..."); 
  Layer* nw_lay = (Layer*)network->layers.FindName(trg_lay->name); 
  if(nw_lay == NULL) { 
    taMisc::Error("==> could not find layer with same name in current network, aborting"); 
    return false; 
  } 
  taBase::SetPointer((TAPtr*)&trg_lay, nw_lay); 
  return true; 
} 
  
void ResponseStat::Network_Init() { 
  //InitStat(); 
} 
 
void ResponseStat::Layer_Run() { 
  if(layer != NULL) { 
    if(!CheckLayerInNet()) return; 
    if(layer->lesion)      return; 
    if(trg_lay == NULL) { 
      taMisc::Error("*** ResponseStat:", name, "trg_lay must be set!"); 
      return; 
    } 
    if(tr == NULL) { 
      taMisc::Error("*** ResponseStat:", name, "RtTrial must be set!"); 
      return; 
    } 
    if (tr->trial_time <= tr->dt) TrialInit();  //HACK--reset if starting new trial 
    if(isResponse) return;  //Already got response 
    Layer_Init(layer); 
    Unit_Run(layer); 
    Layer_Stat(layer); 
    return; 
  } 
  taMisc::Error("*** ResponseStat:", name, "layer must be set!"); 
  return; 
} 
 
void ResponseStat::ProcessResponse(Unit *unit) {  //Unit made response 
      float baseline; 
      float interpRt; 
      RtUnit *ru = (RtUnit*)unit; 
 
      if (unit == NULL) return;  //safety check 
      // find relative time of response 
      if (responseSinceWhen != NULL) baseline = responseSinceWhen->whenTriggered; 
      else baseline = 0.0f; 
 
      isResponse = true; 
      isCorrect = (unit->targ > 0.0f) ? true : false; 
      // just exceeded threshold, so log outcome and RT 
      // interpolate backward linearly to estimate exact time of crossing 
      if (response_type == THRESHOLD) { 
	interpRt = tr->trial_time - (ru->act-threshold)*tr->dt/ru->da; 
	lastRt = interpRt - baseline;  
      } else lastRt = tr->trial_time - baseline; //SETTLE 
 
      resp_time = lastRt;  
      resp = isCorrect ? 1.0f:-1.0f; 
      tr->responseMade = true;  
      tr->responseCorrect = isCorrect; 
      tr->responseTime = lastRt; //record response time in trial 
} 
 
void ResponseStat::Unit_Run(Layer* lay) { 
  Unit* unit, *tu; 
  taLeafItr i, ti, whichLayer, whichUnit; 
  Layer *l; 
  Unit *u, *maxU = NULL; 
  RtUnit *ru; 
  float maxAct = -1.0f; //assertion may change 
 
  if (isResponse) return;  //just to be sure 
  if (responseSinceWhen != NULL) if (!(responseSinceWhen->isTriggered)) return; 
  if (responseSinceWhen != NULL) {if ((tr->trial_time - responseSinceWhen->whenTriggered) < minResponseTime) return; } 
  else if (tr->trial_time < minResponseTime) return; 
 
  if (response_type == SETTLE) { 
	// loop over all units and check maximum da < settle_thresh 
       if (layer == NULL) { 
	 FOR_ITR_EL(Layer, l, network->layers., whichLayer) { 
	    FOR_ITR_EL(Unit, u, l->units., whichUnit) { 
	      ru = (RtUnit *) u; 
	      if (ru->da > settle_thresh*tr->dt) return;  //not yet settled 
	    } 
	 } 
       } 
       else { //single layer to check for settling is specified 
            FOR_ITR_EL(Unit, u, layer->units., whichUnit) { 
	      ru = (RtUnit *) u; 
	      if (ru->da > settle_thresh*tr->dt) return;  //not yet settled 
	    }     
        } 
	//responding unit = unit with maximum activity in specified output layer 
	FOR_ITR_EL(Unit, u, lay->units., whichUnit) { 
	      if (u->act > maxAct) {maxAct = u->act; maxU = u;} 
	} 
	ProcessResponse(maxU); 
  } 
  else { //Threshold 
    for(unit = (Unit*)lay->units.FirstEl(i),  
	  tu = (Unit*)trg_lay->units.FirstEl(ti); 
      (unit != NULL) && (tu != NULL); 
      unit = (Unit*)lay->units.NextEl(i), tu = (Unit*)trg_lay->units.NextEl(ti)){ 
        Unit_Init(unit); 
        if(unit->act > threshold)  ProcessResponse(unit);  //response made 
    } 
  } 
} 
 
 
////////////////////// 
// Environment /////// 
////////////////////// 
 
void RtPatternSpec::ApplyPattern(Pattern *pat) 
{ 
  if (needsTrigger)  
  { 
    if (onTrigger != NULL) { if (!(onTrigger->isTrig())) return;} 
  } 
  if (needsVetoTrigger)  
      { if (vetoTrigger != NULL) {if (vetoTrigger->isTrig()) return;}} 
  PatternSpec::ApplyPattern(pat); 
   
} 
 
void RtEventSpec::ResetTriggers() { 
  taLeafItr i; 
  Trigger *tp; 
  FOR_ITR_EL(Trigger, tp, triggers., i) tp->isTriggered = false; 
} 
 
void RtEventSpec::ZeroInputLayers(Network *network) { 
  taLeafItr whichLayer, whichUnit; 
  Layer *l; 
  Unit *u; 
  FOR_ITR_EL(Layer, l, network->layers., whichLayer) { 
    if (l->ext_flag & Unit::EXT)  
      FOR_ITR_EL(Unit, u, l->units., whichUnit) {u->ext = 0.0f; u->targ=0.0f;} 
  } 
} 
 
bool RtEventSpec::CheckTriggers(Network *network, RtTrial *trial) { 
  // run through all trigger conditions for all triggers here 
  // use triggers 
  float trialTime = trial->trial_time; 
  bool isCond = false, allCond, isUpdated = false; 
  //bool isTrig; 
  int numCond; 
  taLeafItr i; 
  Trigger *tp; 
  int condCount; 
  taBase_Group* trigCondGroup; 
  TriggerCondition* cond; 
 
  // iterate over triggers 
  FOR_ITR_EL(Trigger, tp, triggers., i) { 
    if (tp->isTriggered) continue; 
    trigCondGroup = &(tp->trigCondGroup); 
    condCount = 0; 
    numCond = trigCondGroup->size; 
    allCond = true;  //assertion will change to false if any condition fails 
    while (condCount < numCond) { 
      cond = (TriggerCondition *) trigCondGroup->SafeEl(condCount++);  
      switch (cond->trig_cond) { 
      case TriggerCondition::ELAPSED_TRIAL_START : \
	   isCond = trialTime > cond->elapsed_time;  break; 
      case TriggerCondition::ELAPSED_LAST_TRIG : \
           isCond = (cond->other_trigger != NULL) ? ((cond->other_trigger->isTrig()) && (trialTime > cond->other_trigger->whenTriggered + cond->elapsed_time)) : false; break; 
      case TriggerCondition::CORRECT : isCond = trial->responseCorrect; break; 
      case TriggerCondition::INCORRECT : isCond = (trial->responseMade && (!(trial->responseCorrect))); break; 
      case TriggerCondition::LAYER_OVER_THRESH : isCond = trial->responseMade; break; 
      case TriggerCondition::NOT_TRIGGER : \
	   isCond = (cond->other_trigger != NULL) ? !(cond->other_trigger->isTrig()) : false; break; 
      default: isCond = true; 
      } 
      if ((tp->trigger_logic == Trigger::TRIGGER_OR) && isCond) break; 
      else if (tp->trigger_logic==Trigger::TRIGGER_AND)  
	{allCond = allCond && isCond; if (!allCond) break;} 
    } 
    if (((tp->trigger_logic == Trigger::TRIGGER_AND) && allCond)  
	|| ((tp->trigger_logic == Trigger::TRIGGER_OR) && isCond)) { 
      tp->isTriggered = true; 
      isUpdated = true; 
      tp->whenTriggered = trialTime; 
      if ((tp->trig_action == Trigger::TRIAL_END) || (tp->trig_action == Trigger::TRIAL_END_NEXT_SPEC)) 
      { 
	trial->trialEnded=true; 
	if (tp->trig_action == Trigger::TRIAL_END_NEXT_SPEC)  
	  ((RtEpoch *)(trial->epoch_proc))->specNextEvent(((RtEvent*)trial->cur_event)->getNextEvent(trial->responseCorrect)); 
      } 
    } 
  } 
  return isUpdated; 
} 
 
RtEvent *NextEventInfo::getNextEvent(RtUnit *action, bool isReward) { 
  // If next transition is random: 
  float pval = Random::ZeroOne(); 
  float pcum = 0.0f; 
  TransitionProb *pt, *pt_hit = NULL; 
  taLeafItr i; 
 
  switch (next_event) { 
    case (UNSPEC) : return (RtEvent *) NULL; 
    case (DETERMINED) : return nextEvent; 
    case (MARKOV) :  
         // Assumes that all transition probabilities add up to 1.0 
         FOR_ITR_EL(TransitionProb, pt, event_trans.transitionProbGroup., i) { 
            pcum+= pt->probability; 
            if (pcum > pval) {pt_hit = pt; break;} 
         } 
         return pt_hit->nextEvent; 
    case (DEPENDS_CORRECT) : return isReward ? nextEventIfCorrect : \
			     nextEventIfError; 
 
    case (DEPENDS_RESPONSE) : return (RtEvent*) NULL; //not yet implemented 
    case (END_EPOCH) : return (RtEvent *) NULL; //not yet implemented 
    default : return (RtEvent *) NULL; 
  } 
} 
 
void RtEpoch::GetCurEvent() { 
  if (isNextSpecified && (nextEvent != NULL)) { 
    taBase::SetPointer((TAPtr*)&cur_event, nextEvent); 
    isNextSpecified = false; //reset flag 
  } 
  else EpochProcess::GetCurEvent(); 
  return; 
} 
 
void RtEpoch::specNextEvent(RtEvent *next) { 
  if (next) {  //NULL value means no next event specified 
    isNextSpecified = true; 
    nextEvent = next; 
  } 
} 
 
void RtEvent::UpdateAfterEdit() {  //keep track of mirrors from spec for gui 
  RtEventSpec *ms = (RtEventSpec*)spec.spec; 
  if ((ms!= NULL) && (ms->InheritsFrom(&TA_RtEventSpec))) { 
    //isConstraint = ms->isConstraint; 
    nei_mirror = &(ms->eventTransitions); 
  } 
  Event::UpdateAfterEdit(); 
} 
 
 
//------------ Model Parameter methods ----------- 
 
float ModelParamEntry::getVal() { 
  //int i; 
  float *tmp_pval; 
  char *sname; 
  Project* prj = (Project*)owner->GetOwner(&TA_Project); 
  BaseSpec_MGroup*     	specs = &(prj->specs); 
  //MemberSpace *ms; 
  //cout << "ModelParamEntry::getVal() " << SpecName << MemberName; 
  if(specs == NULL) 
     {taMisc::Error("ModelParamEntry::getVal() --specs not found\n"); return 0.0f;} 
  TAPtr aSpec = specs->FindSpecName(SpecName); 
  if(aSpec == NULL) 
     {taMisc::Error("ModelParamEntry::getVal() --spec not found\n"); return 0.0f;} 
  sname = MemberName; 
  MemberDef *md = aSpec->FindMember(sname); 
  if(md==NULL) 
     {taMisc::Error("ModelParamEntry::getVal() --member not found\n"); return 0.0f;} 
  if (subGroup.isSubGroup) { 
    TAPtr inlineObj = (taBase *) md->GetOff((void*)aSpec); //points to inlined object 
    MemberSpace *subMs = &(inlineObj->GetTypeDef()->members); 
    MemberDef *subMd = subMs->FindName(subGroup.SubMemberName); 
    if (subMd == NULL)  
      {taMisc::Error("ModelParamEntry::getVal() --sub member not found\n"); return 0.0f;} 
    tmp_pval = (float*)subMd->GetOff((void*)inlineObj); 
  } 
  else tmp_pval = (float*)md->GetOff((void*)aSpec); 
 
  val = *tmp_pval; //update ModelParamEntry internal 
  return val; 
} 
 
void ModelParamEntry::setVal(float aVal) { 
  //int i; 
  float *tmp_pval; 
  char *sname; 
  Project* prj = (Project*)owner->GetOwner(&TA_Project); 
  BaseSpec_MGroup*     	specs = &(prj->specs); 
 
  val=aVal; //update ModelParamEntry internal 
  TAPtr aSpec = specs->FindSpecName(SpecName); 
  if(aSpec == NULL) 
     {taMisc::Error("ModelParamEntry::setVal() --spec not found\n"); return; } 
  sname = MemberName; 
  MemberDef *md = aSpec->FindMember(sname); 
  if(md==NULL) 
     {taMisc::Error("ModelParamEntry::setVal() --member not found\n"); return; } 
  if (subGroup.isSubGroup) { 
    TAPtr inlineObj = (taBase*)md->GetOff((void*)aSpec); //points to inlined object 
    MemberSpace *subMs = &(inlineObj->GetTypeDef()->members); 
    MemberDef *subMd = subMs->FindName(subGroup.SubMemberName); 
    if (subMd == NULL)  
      {taMisc::Error("ModelParamEntry::setVal() --sub member not found\n"); return;} 
    tmp_pval = (float *)subMd->GetOff((void*)inlineObj); 
  } 
  else tmp_pval = (float*)md->GetOff((void*)aSpec); 
  // got pointer, so now enforce bounds and store 
  aVal = aVal > limits.upper ? limits.upper : aVal; 
  aVal = aVal < limits.lower ? limits.lower : aVal; 
  *tmp_pval = aVal; 
  aSpec->UpdateAfterEdit(); 
  tabMisc::NotifyEdits(aSpec); 
} 
 
void ModelParam::updateParamList() { 
  Project* prj = (Project*)owner->GetOwner(&TA_Project); 
  BaseSpec_MGroup*     	specs = &(prj->specs); 
  int count;  
  ModelParamEntry *mp; 
 
  // Mark entries as asserted orphans 
  for (count=0; count < paramMembers.size; count++) { 
    mp = paramMembers.SafeEl(count); 
    mp->isOrphan=true; 
  } 
 
  // Do the update 
  updateParamList_impl(specs); 
 
  // Delete the orphans 
  for (count=0; count < paramMembers.size; count++) { 
    mp = paramMembers.SafeEl(count); 
    if (mp->isOrphan==true) paramMembers.Remove(count); 
  } 
} 
 
void ModelParam::updateParamList_impl(BaseSpec_MGroup *source) { 
  BaseSpec *kid; 
  taLeafItr i; 
 
  // Update valid entries from existing specs 
  FOR_ITR_EL(BaseSpec, kid, source->, i) { 
    updateParamsFromSpec(kid); //should be *, not instance? 
    updateParamList_impl(&(kid->children)); 
  } 
} 
 
 
ModelParamEntry *ModelParam::findParam(String SpecName, String MemberName, String SubMemberName) { 
     ModelParamEntry *mp = NULL; 
     char *smn = SubMemberName;
     int count; 
     for (count=0; count < paramMembers.size; count++) { 
	   mp = paramMembers.SafeEl(count); 
	   if (mp->SpecName == SpecName) if (mp->MemberName == MemberName) { 
	     if ((smn) && (strcmp(smn, "0"))) { //submember exists 
	       if (mp->subGroup.SubMemberName == SubMemberName) return mp; 
	     }  
	     else { //not a submember 
	       return mp; 
	     } 
	   } 
     } 
     return NULL;   //entry not found 
} 
 
//Updates value of specified param, or else creates new one if param entry doesn't exist 
void ModelParam::updateParamsFromObj(String SpecName, String MemberName, String SubMemberName, float *pval, bool isDefaultParam) { 
  char *smn = SubMemberName;
  ModelParamEntry *mp = findParam(SpecName, MemberName, SubMemberName); 
  if (mp) { mp->val = *pval; mp->isOrphan = false;} 
  else { 
     mp = paramMembers.NewEl(1, NULL); 
     taBase::Own(mp, &paramMembers); // REMOVAL WITHOUT DISOWN COULD ->CRASH? 
     //printf("UpdateParamsFromSpec(): getting spec group\n"); 
     //Project* prj = (Project*)owner->GetOwner(&TA_Project); 
     //BaseSpec_MGroup*     	specs = &(prj->specs); 
     //taBase::SetPointer((taBase**)&(mp->specs), specs); 
     mp->SpecName = SpecName; 
     mp->MemberName = MemberName; 
     if (smn != NULL) { 
       if (strcmp(smn, "0"))  {  
	 mp->subGroup.isSubGroup = true;  
	 mp->subGroup.SubMemberName = SubMemberName; 
       }  
     } 
     else mp->subGroup.isSubGroup = false; 
     mp->fit = false; 
     mp->pval = pval; //unused 
     mp->val = *pval; 
     mp->limits.upper = 2.0*mp->val; 
     mp->limits.lower = 0.0f; 
     mp->isOrphan = false; 
  } 
} 
 
void ModelParam::updateParamsFromSpec(TAPtr aSpec) { 
  int i, count; 
  float *p_mbval; 
  MemberDef *md, *subMd; 
  MemberSpace *ms = &(aSpec->GetTypeDef()->members); 
  MemberSpace *subMs; 
  BaseSpec* bSpec = (BaseSpec*)aSpec; 
  //debugging: 
 
  if (aSpec->InheritsFrom(&TA_ModelParam)) return; //Don't copy from self! 
 
  //look through all members/entries in aSpec 
  for (i=0; i < ms->size; i++) { 
    md = ms->SafeEl(i); 
    if (md->HasOption("NO_SAVE")) continue; //shouldn't be a parameter

    // find corresponding ModelParamEntry instance, or create new  
    //if none exists -- do this only if entry is "unique" or else top level 
    // (want to avoid needlessly setting non-unique child entries) 
    if ((bSpec->FindParent() != NULL)  
	   && !(bSpec->GetUnique((char *)md->name.chars()))) continue; 
 
    // update parameters from inlined objects -- important to exclude taStrings, 
    // which have no float members anyway, and will cause program to crash while 
    // trying to get TypeDefs 
    if (md->type->InheritsFormal(&TA_class) &&  
	(!(md->type->InheritsFrom(&TA_taString))) 
	/*&& md->HasOption("PARAM_OPT")*/) { 
      // recurse on subobject for inline members 
       TAPtr inlineObj = (taBase*)md->GetOff((void*)aSpec); //points to inlined object 
       subMs = &(inlineObj->GetTypeDef()->members); //object's memberspace 
       for (count=0; count < subMs->size; count++) { 
	 subMd = subMs->SafeEl(count); 
	 if (subMd->type->InheritsFrom(&TA_float)) { 
	     p_mbval = (float*)subMd->GetOff((void*)inlineObj); 
	     updateParamsFromObj(((BaseSpec*)aSpec)->name, md->name, subMd->name, p_mbval, md->HasOption("DEFAULT_PARAM")); 
	 } 
       } 
    } 
 
    // update parameters from float objects 
    else if(md->type->InheritsFrom(&TA_float) /*&& md->HasOption("PARAM_OPT")*/) { 
       p_mbval = (float*)md->GetOff((void*)aSpec);  // points to float aSpec member 
           updateParamsFromObj(bSpec->name, md->name, NULL, p_mbval, md->HasOption("DEFAULT_PARAM")); 
    } 
 
    /* else if(md->type->InheritsFrom(&TA_int)){ 
       mbval = (float) *((int *) md->GetOff((void*)ptrs.FastEl(i)));*/ 
     
  } 
} 
 
void ModelParam::UpdateAfterEdit() { 
  // look through list and assign val to *pval 
  int count; 
  //bool isChange = false; 
  ModelParamEntry *mp; 
 
  /*for (count=0; count < paramMembers.size; count++) { 
    mp = paramMembers.SafeEl(count); 
    if (mp->pval != NULL) *(mp->pval) = mp->val; 
    }*/ 
 
  //If ModelParam edited, then assume parameters to be fit may have changed 
  changeCount++; 
  fitIndices.Reset(); 
  for (count=0; count < paramMembers.size; count++) { 
    mp = paramMembers.SafeEl(count); 
    if (mp->fit) fitIndices.Add(count); 
  } 
   
} 
 
// ----------- Model Constraints for fitting ----------- 
 
void RtConstraintInfo::Initialize() { 
  Project *proj = GET_MY_OWNER(Project); 
 
  if (proj == NULL) whichNetwork = NULL; 
  else whichNetwork = (Network*)proj->networks.DefaultEl(); 
 
  constraint_type = INITIAL_CONDITION; 
  constrained_obj = CONSTRAIN_LAYER; 
  whichLayer = NULL; 
  units = NULL; 
  whichUnit = NULL; 
  whichProjection = NULL; 
  whichCon = NULL; 
  how_specific = CONSTRAIN_UNIFORM; 
  val = 0.0f; 
  isApplied = false; 
  alreadyAppliedThisRun = false; 
  constraintSource = NULL; 
  constraintToLink = NULL; 
  taBase::Own(apply_script,this); 
} 
 
void RtConstraintInfo::ReadFromNet() { 
  RtUnit *u = NULL; 
  RtCon *c = NULL; 
  RtCon_Group *g = NULL; 
  /* 
  if (whichNetwork == NULL) { 
    Project *proj = GET_MY_OWNER(Project);  
    whichNetwork = (Network*)proj->networks.DefaultEl();  
    if (whichNetwork == NULL) {taMisc::Error("No network found!"); return;} 
  } 
  */ 
 
  if (constrained_obj == CONSTRAIN_LAYER) { 
   if (whichLayer != NULL) { 
     u = whichUnit; 
     if (u == NULL)  u = (RtUnit*) whichLayer->units.DefaultEl();  
     val = u->act; 
   } else {taMisc::Error("No layer specified"); return;} 
  } 
  else { //CONSTRAIN_PROJECTION -- projection's layer is the RECIPIENT of projection 
    if (whichProjection != NULL) { 
        c = whichCon; 
	if (whichCon == NULL) { 
	  u = (RtUnit*)whichProjection->layer->units.DefaultEl(); 
	  if (u!= NULL) g = (RtCon_Group*)u->recv.DefaultEl(); 
	  if (g!= NULL) c = (RtCon*)g->DefaultEl(); 
	} 
	if (c != NULL) val = c->wt; 
    } else {taMisc::Error("No projection specified"); return;} 
  } 
} 
 
void RtConstraintInfo::ApplyToNet() { 
  RtUnit *u = NULL; 
  RtCon *c = NULL; 
  RtCon_Group *g = NULL; 
  taLeafItr i, count, citr; 
  RtConstraintInfo *rc; 
  bool doClamp;  
 
  //printf("Checking if already applied...\n"); 
  if (alreadyAppliedThisRun) return; 
  if (constraint_type == SCRIPT)  
    {/*printf("Applying Script...\n");*/ if (apply_script.mod.flag) apply_script.Run();  return; } 
  //printf("Computing doClamp...\n"); 
  doClamp = (constraint_type == CLAMP); 
  if (constrained_obj == CONSTRAIN_LAYER) { 
   if (whichLayer != NULL) { 
     u = whichUnit; 
     if ((u == NULL) || (how_specific == CONSTRAIN_UNIFORM)) { //set all units in layer 
       FOR_ITR_EL(RtUnit, u, whichLayer->units., i)  
	  {u->act = val; u->volt_clamp = doClamp; } 
     } 
     else {u->act = val; u->volt_clamp = doClamp;} 
   } else {taMisc::Error("No layer specified"); return;} 
  } 
  else { //CONSTRAIN_PROJECTION -- projection's layer is the RECIPIENT of projection 
    if (whichProjection != NULL) { 
        c = whichCon; 
	if ((c == NULL) || (how_specific == CONSTRAIN_UNIFORM)) {//set all cons in proj'n 
	  FOR_ITR_EL(RtUnit, u, whichProjection->layer->units., i) { 
	    FOR_ITR_EL(RtCon_Group, g, u->recv., count) { 
	      if (g->prjn != whichProjection) continue; 
	      FOR_ITR_EL(RtCon, c, g->, citr) { 
		c->wt = val; c->isClamp = doClamp; 
	      } 
	    } 
	  } 
	} 
	else {c->wt = val; c->isClamp = doClamp;} 
    } else {taMisc::Error("No projection specified"); return;} 
  } 
   
  alreadyAppliedThisRun = true; //Needed to avoid possible infinite recursion below: 
  FOR_ITR_EL(RtConstraintInfo, rc, sub_constraints., i) rc->ApplyToNet(); 
  alreadyAppliedThisRun = false; 
} 
 
void RtConstraintInfo::RestoreNet() { 
  //taMisc::Error("RestoreNet() not yet functional"); 
  //for now, just removes all clamps from network... 
  RtUnit *u = NULL; 
  RtCon *c = NULL; 
  RtCon_Group *g = NULL; 
  taLeafItr i, count; 
  RtConstraintInfo *rc; 
 
  if (alreadyAppliedThisRun) return; 
  if (constraint_type == SCRIPT) return; 
 
  if (constrained_obj == CONSTRAIN_LAYER) { 
   if (whichLayer != NULL) { 
     u = whichUnit; 
     if ((u == NULL) || (how_specific == CONSTRAIN_UNIFORM)) { //set all units in layer 
       FOR_ITR_EL(RtUnit, u, whichLayer->units., i)  
	  {u->volt_clamp = false; } 
     } 
     else { u->volt_clamp = false;} 
   } else {taMisc::Error("No layer specified"); return;} 
  } 
  else { //CONSTRAIN_PROJECTION -- projection's layer is the RECIPIENT of projection 
    if (whichProjection != NULL) { 
        c = whichCon; 
	if ((c == NULL) || (how_specific == CONSTRAIN_UNIFORM)) {//set all cons in proj'n 
	  FOR_ITR_EL(RtUnit, u, whichProjection->layer->units., i) { 
	    FOR_ITR_EL(RtCon_Group, g, u->recv., count) { 
	      if (g->prjn != whichProjection) continue; 
	      c->isClamp = false; 
	    } 
	  } 
	} 
	else c->isClamp = false; 
    } else {taMisc::Error("No projection specified"); return;} 
  } 
   
  alreadyAppliedThisRun = true; //Needed to avoid possible infinite recursion below: 
  FOR_ITR_EL(RtConstraintInfo, rc, sub_constraints., i) rc->ApplyToNet(); 
  alreadyAppliedThisRun = false; 
} 
 
void RtConstraintInfo::InitLinks() { 
  taBase::Own(sub_constraints, this);  
  FindConstraints(); 
  BaseSubSpec::InitLinks(); 
} 
 
void RtConstraintInfo::FindConstraints() {   
  Project *proj = GET_MY_OWNER(Project);    
  RtConstraintSpec *Rtcs = (RtConstraintSpec*) proj->specs.FindSpecType(&TA_RtConstraintSpec); 
  if (Rtcs != NULL) constraintSource = &(Rtcs->constraints);  
} 
 
void RtConstraintInfo::LinkConstraint() { 
  sub_constraints.LinkUnique(constraintToLink); 
} 
 
// ----------- Fit Data Methods ------------------------ 
 
void FitDataInfo::Initialize() { 
  constraint_type = STATIC_STID; 
  taBase::Own(custom,this); 
  constraint_wt = 1.0; //default 
  fitEnergyVal = 0.0; 
  actualRt = 0.0f; 
  desiredRt = 0.0f; 
  isCorrect = true; 
  desiredAct = 0.0f; 
  whichLayerRecorded = NULL; 
  whichLayerUnits = NULL; 
  whichUnitRecorded = NULL; 
  dataAlignTime = 0.0f; 
  align_trigger = NULL; 
  data_gain = 1.0f; 
  clampUnitToData = false; 
  dataDriveWhichLayer = NULL; 
  dataDriveUnits = NULL; 
  dataDriveWhichUnit = NULL; 
  net_state = NULL; 
  net_state_group = NULL; 
  events.SetBaseType(&TA_RtEvent); 
  eventSource = NULL; 
  eventToLink = NULL; 
} 
 
void  FitDataInfo::InitLinks() { 
  Project *proj = GET_MY_OWNER(Project); 
  Environment *env = (Environment*) proj->environments.SafeEl(0); 
  //taBase::SetPointer(&eventSource, env->events); 
  if (env != NULL) eventSource = &(env->events); 
  //get pointer to constraints in RtConstraintSpec, if it exists 
  FindConstraints(); 
 
  taBase::Own(data_time, this); taBase::Own(data_act,this); 
  taBase::Own(model_time, this); taBase::Own(model_act, this);  
  taBase::Own(events,this); BaseSubSpec::InitLinks(); 
} 
 
void FitDataInfo::FindConstraints() {   
  Project *proj = GET_MY_OWNER(Project);    
  RtConstraintSpec *Rtcs = (RtConstraintSpec*) proj->specs.FindSpecType(&TA_RtConstraintSpec); 
  if (Rtcs != NULL) net_state_group = &(Rtcs->constraints);  
} 
 
float FitDataInfo::LinearInterpModel(float curModelTime) { 
  int i; 
  float t0,tm1, a0, am1; 
  for (i=0; i < model_time.size; i++) if (model_time.SafeEl(i) >= curModelTime) break; 
  if (model_time.SafeEl(i) == curModelTime) return model_act.SafeEl(i); 
  t0 = model_time.SafeEl(i); tm1 = model_time.SafeEl(i-1); 
  a0 = model_act.SafeEl(i);  am1 = model_act.SafeEl(i-1); 
  return am1 + (a0-am1)*(curModelTime-tm1)/(t0-tm1); //Linear interpolation 
} 
 
float FitDataInfo::CalcCellTypeFitError() { 
  float val = 0.0f, tmp, curDataTime, curDataAct, curModelTime, curModelAct; 
  float modelOffsetFromData = align_trigger->whenTriggered - dataAlignTime; 
  int i; 
 
  // Loop through data points, and interpolate model output 
  for (i=0; i < data_time.size; i++) { 
    curDataTime = data_time.SafeEl(i); 
    curDataAct = data_act.SafeEl(i); 
    curModelTime = modelOffsetFromData +  curDataTime; 
    if ((curModelTime < model_time.SafeEl(0)) || //ignore before start/after end of trial 
	(curModelTime > model_time.SafeEl(model_time.size-1))) continue;  
    curModelAct = LinearInterpModel(curModelTime); 
    tmp = curModelAct - curDataAct; 
    val+=tmp*tmp; 
  } 
  return val; 
} 
 
float FitDataSpec::CalcRSquare() { 
  int i; 
  FitDataInfo *fp; 
  float dvar=0.0, avar=0.0, cvar=0.0, dmean=0.0, amean=0.0; 
  float adiff, ddiff; 
  float numVals = (float)(data.size); 
 
  for (i=0; i < data.size; i++) { 
    fp = (FitDataInfo *)data.SafeEl(i); 
    dmean+= fp->desiredRt; 
    amean+= fp->actualRt; 
  } 
  dmean/=numVals; amean/=numVals; 
 
  for (i=0; i < data.size; i++) { 
    fp = (FitDataInfo *)data.SafeEl(i); 
    adiff = fp->actualRt - amean; 
    ddiff = fp->desiredRt - dmean; 
    dvar+= ddiff*ddiff; 
    avar+= adiff*adiff; 
    cvar+= adiff*ddiff; 
  } 
  return cvar*cvar/(avar*dvar); 
} 
 
void FitDataSpec::MakeRandEv() { //Makes all existing NON-SCRIPT constraints into DYN_RT with uniformly sampled random events from the default environment and zero constraint wt. 
  int i; 
  FitDataInfo *fp=NULL; 
  int numEv = 0; 
  float randval = 0.0f; 
  int randEvNum = 0; 
 
  for (i=0; i < data.size; i++) { 
    fp = (FitDataInfo *)data.SafeEl(i); 
    if (fp->constraint_type != FitDataInfo::SCRIPT) { 
      //link a random event 
      numEv = fp->eventSource->size; 
      randval = (float)numEv * Random::ZeroOne(); 
      randEvNum = (int)floor(randval); randEvNum = MMIN(randEvNum,numEv-1); 
      fp->eventToLink = (Event*) fp->eventSource->SafeEl(randEvNum); 
      fp->LinkEvent(); 
 
      fp->constraint_type = FitDataInfo::DYN_RT; 
      fp->constraint_wt = 0.0f; 
      fp->isActive = true; 
    } 
  } 
} 
 
void FitDataSpec::DumpData() { //need to make this function handle all constraint types 
  int i; 
  FitDataInfo *fp=NULL; 
  float aval, dval; 
  int isCor; 
  char *s, *nm;
 
  FILE *out = fopen("rt.out","w"); 
  if (out == NULL) { taMisc::Error("Could not open rt.out for writing"); return;} 
  fprintf(out, "Desired  Actual\n"); 
 
  for (i=0; i < data.size; i++) { 
    fp = (FitDataInfo *)data.SafeEl(i); 
    if ((fp->constraint_type == FitDataInfo::DYN_RT) ||  
	(fp->constraint_type == FitDataInfo::DYN_CELL_RT)){ 
      dval= fp->desiredRt; 
      aval= fp->actualRt;
      s = fp->UserData;
      nm = fp->eventToLink->name;
      isCor = fp->isCorrect; 
      fprintf(out, "%f %f %i %s %s\n", dval, aval, isCor, s, nm); 
    } 
  } 
 
  fclose (out); 
} 
 
void RtDistFitMgr::Initialize() {
  used = false;
  master = false;
  numSlaves = 0;  //default
  numOverlap = 0;
  slaveID = 1;
  numParams = 0;
  waitLastScript = false;
  taBase::Own(p, this);
}

void RtDistFitMgr::DirectSlaves() {
  int i, numPerSlave = numConstr/numSlaves, first, last=numOverlap;
  char buf[512];
  FILE *test, *out = fopen("_fitmgr_params","w");
  if (out == NULL) fprintf(stderr,"Could not open output file to slaves!\n");
  
  (*getParams)(data, &p);
  fprintf(out,"%i\n", numParams);
  for (i=0; i < numParams; i++) fprintf(out, "%e\n", p.SafeEl(i));
  // divide up labor:  entry is: <slave #> <first constr> <last constr>
  for (i=0; i < numSlaves; i++) {
    first = last-numOverlap;
    last = first+numPerSlave+numOverlap;
    if (i == (numSlaves-1)) last = numConstr - (waitLastScript ? 2 : 1);
			      //don't lose constraints to rounding
    fprintf(out, "%i %i %i\n", i, first, last);
  }
  fclose(out);

  //now remove slaves' previous reports, if they exist 
  //-- this cues slaves to receive orders
  for (i=0; i < numSlaves; i++) {
    sprintf(buf, "_fitmgr_data_%i", i);
    if ((test=fopen(buf,"r")) != NULL) {fclose(test); unlink(buf);}
  }
}

void RtDistFitMgr::ReceiveOrders() {
  char buf[512];
  int i, tmpfirst=-1, tmplast;
  float ftmp;
  FILE *in, *test;
  sprintf(buf,"_fitmgr_data_%i", slaveID);
  //slaves should wait until master's orders ready, signaled by removal
  // of previous report from slave
  while ((test = fopen(buf,"r")) != NULL) {fclose(test); sleep(1);} //wait 
  //OK, orders should be ready now, but wait if necessary
  while ((in = fopen("_fitmgr_params","r")) == NULL) sleep(1); //wait for orders
  //if (in == NULL) fprintf(stderr,"Could not open input file from master!\n");

  fscanf(in,"%i\n", &numParams);  //should already be set!
  for (i=0; i < numParams; i++) {fscanf(in,"%e\n", &ftmp); p.Set(i,ftmp);}
  (*setParams)(data,&p); //params now updated

  while (!feof(in)) {
    fscanf(in,"%i %i %i", &i, &tmpfirst, &tmplast);
    if (i==slaveID) {firstConstr = tmpfirst; lastConstr = tmplast; break;}
  }
  if (tmpfirst == -1) fprintf(stderr,"Error: No orders for slave %i!\n",slaveID);
  fclose(in);
}

void RtDistFitMgr::ReportToMaster(FitDataInfo_Group *data) {
  int i, numPts = lastConstr-firstConstr+1;
  char buf[512];
  FitDataInfo *fd;
  FILE *out;
  sprintf(buf, "_fitmgr_data_%i",slaveID);
  if ((out = fopen(buf,"w")) == NULL) {
    fprintf(stderr,"Slave %i: Could not open report file\n", slaveID); return;
  }
  for (i=0; i < numPts; i++) {
    fd = (FitDataInfo *) data->SafeEl(i+firstConstr);
    fprintf(out, "%i %f %f %i\n", i+firstConstr, fd->fitEnergyVal, fd->actualRt, fd->isCorrect);
  }
  fclose(out);
}

void RtDistFitMgr::GetFromSlaves(FitDataInfo_Group *data) {
  FILE *in;
  char buf[512];
  int i,count, whichConstr, isCor;
  float enval, rtval;
  FitDataInfo *fd;
  for (i=1; i < numSlaves; i++) {  //don't read master's data -- already there
    sprintf(buf,"_fitmgr_data_%i", i);
 DM2("Reading from slave file: %s... ", buf);
    while ((in = fopen(buf,"r")) == NULL) {DM("waiting "); sleep(1);}; //wait for slaves
    // fprintf(stderr,"Slave %i: Could not open report file\n", slaveID);
    count = 0;
    while (!feof(in)) {
	fscanf(in,"%i %f %f %i\n", &whichConstr, &enval, &rtval, &isCor);
	if ((count >= numOverlap) && (whichConstr >=numOverlap)) {
	  fd = (FitDataInfo *) data->SafeEl(whichConstr);
	  fd->fitEnergyVal = enval;
	  fd->actualRt = rtval;
	  fd->isCorrect = isCor;
	}
        count++;
    }
    fclose(in);
DM("Done reading slave file\n");
  }
}

 
// ---------- RtConstrain ----------- 
// Constraints are in a group --  
// static constrains in top level, and dynamic in subgroups 
void RtConstrain::Initialize() { 
  paramSetChangeCount = -1; //initially force init 
  errorPenalty = 0.0f; 
  min_delta_p = 0.001f;  
  max_delta_p = 10.0; 
  fit_method = OPT_SIMPLEX; 
  dp_bounding = DP_ABS; 
  loopDumpParams = true;
  paramDumpFile = "params.out";
 
  op.getParams = &(RtGetParams); 
  op.setParams = &(RtSetParams); 
  op.objFunc = &(RtFitObjFunc); 
  op.calcGrad = &(RtFitCalcGrad); 
  op.setDataObj((void *)this); 
 
  simp.getParams = &(RtGetParams); 
  simp.setParams = &(RtSetParams); 
  simp.objFunc = &(RtFitObjFunc); 
  simp.setDataObj((void *)this); 

  subp.getParams = &(RtGetParams);
  subp.setParams = &(RtSetParams);
  subp.objFunc = &(RtFitObjFunc);
  subp.setDataObj((void *)this);
  subp.saveParams = &(RtSaveParams);

  dFit.getParams = &(RtGetParams);
  dFit.setParams = &(RtSetParams);
  dFit.setDataObj((void *)this);
 
  taBase::Own(op, this); 
  taBase::Own(simp, this); 
  taBase::Own(subp, this);
  taBase::Own(dFit, this);
} 
 
float RtConstrain::fitEnergy() { //computes objective function 
  taLeafItr citr, eitr; 
  FitDataInfo *fi; 
  float tmp, val = 0.0f; //holds energy (return value) 
  RtEvent *ev; 
  bool doRecord; 
  RtUnit *u; 
  int ctr = -1; //constraint counter
 
  if (dFit.used) { //used distributed objective function computation
    dFit.setNumParams(params->fitIndices.size);
    dFit.setNumConstr(fitData->data.size);
    if (dFit.master) dFit.DirectSlaves();
    dFit.ReceiveOrders(); //both master and slaves work
  }

  // loop over all constraints 
  FOR_ITR_EL(FitDataInfo, fi, fitData->data., citr) { 
    ctr++;
    if (!fi->isActive) continue; 
    if (dFit.used) //doing a subset for distributed objective function calc?
      if ((ctr < dFit.firstConstr) || (ctr > dFit.lastConstr)) continue;

    fi->fitEnergyVal = 0.0; 
    if (fi->net_state != NULL) fi->net_state->ApplyToNet(); //Apply constraints 
    if (fi->constraint_type == FitDataInfo::SCRIPT) { 
      fi->custom.Run();  //script should place value in fitEnergyVal 
      val+= fi->fitEnergyVal*fi->constraint_wt; 
    } 
    else if ((fi->constraint_type == FitDataInfo::STATIC_STID) ||  
	(fi->constraint_type == FitDataInfo::STATIC_SEE)) { //STATIC, but STID only now 
      //ev = fi->events.DefaultEl(); 
      //taBase::SetPointer((TAPtr*)&(trial_proc->cur_event), ev); 
      fi->whichUnitRecorded->act = fi->desiredAct; 
      trial_proc->Step(); //Calc's da 
      u = (RtUnit*)fi->whichUnitRecorded; tmp = u->da; 
      fi->fitEnergyVal+= tmp*tmp; 
      val+=fi->fitEnergyVal*fi->constraint_wt; 
    }  
    else { //DYNAMIC 
      if ((fi->constraint_type == FitDataInfo::DYN_CELL) ||  
	  (fi->constraint_type == FitDataInfo::DYN_CELL_RT))  {//CELL 
	FOR_ITR_EL(RtEvent, ev, fi->events., eitr) { 
	  if (eitr.i >= (fi->events.size-1)) doRecord = true; else doRecord = false; 
 
	  //if trial_proc reinit flag set, will pull event from this epoch  
	  //process.  Therefore must set cur_event both in trial_proc and in  
	  //epoch process (i.e., this) 
	  taBase::SetPointer((TAPtr*)&(trial_proc->cur_event), ev); 
	  taBase::SetPointer((TAPtr*)&cur_event, ev);  
 
	  trial_proc->ReInit();  //Is this necessary? Maybe to reset stats? 
	  if (doRecord) { //Is this the last event in the sequence? 
	    fi->model_time.Reset(); fi->model_act.Reset(); 
	    while (!trial_proc->Crit()) { 
	      trial_proc->Step(); //Run event subject to constraints 
	      fi->model_time.Add(trial_proc->trial_time); 
	      fi->model_act.Add(fi->whichUnitRecorded->act); 
	    } 
	    fi->fitEnergyVal+= fi->CalcCellTypeFitError(); 
	    val+=fi->fitEnergyVal*fi->constraint_wt; 
            if (fi->constraint_type == FitDataInfo::DYN_CELL_RT) { 
	       fi->actualRt = trial_proc->responseTime; // set by last event 
	       // if no response, then set obj func rt to 10x desired -- this 
	       // allows fitting process to penalize response failures 
	       tmp = trial_proc->responseTime; 
	       tmp = tmp > 0.0 ? (fi->actualRt - fi->desiredRt)  
		                 : 10.0*fi->desiredRt; 
	       fi->fitEnergyVal+=tmp*tmp; 
	       if (!trial_proc->responseCorrect) fi->fitEnergyVal+=errorPenalty; 
	       val+=fi->fitEnergyVal*fi->constraint_wt; 
	       fi->isCorrect = trial_proc->responseCorrect; 
	    } 
	  } //If this is not the last event in the sequence, then don't record any output 
	  else trial_proc->Run(); 
	} 
      } 
      else if (fi->constraint_type == FitDataInfo::DYN_RT) { //RT only 
	FOR_ITR_EL(RtEvent, ev, fi->events., eitr) { 
	  //trial_proc->ReInit(); 
	  taBase::SetPointer((TAPtr*)&(trial_proc->cur_event), ev); 
	  taBase::SetPointer((TAPtr*)&cur_event, ev);  
	  trial_proc->Run(); //Run event subject to constraints 
	  fi->actualRt = trial_proc->responseTime; //finally set by last event 
          tmp = fi->actualRt - fi->desiredRt; 
	  fi->fitEnergyVal+=tmp*tmp; 
          if (!trial_proc->responseCorrect) fi->fitEnergyVal+=errorPenalty; 
	  val+=fi->fitEnergyVal*fi->constraint_wt; 
	  fi->isCorrect = trial_proc->responseCorrect; 
	} 
      } 
    } //end if constraint type 
    if (fi->net_state != NULL) fi->net_state->RestoreNet(); 
  } //end loop over all constraints 

  if (dFit.used) {
    if (dFit.master) {  //retrieve data from slaves
      dFit.GetFromSlaves(&(fitData->data));
      if (dFit.waitLastScript) { //run final script, if it exists
	fi = (FitDataInfo*)fitData->data.SafeEl(fitData->data.size-1); //last element
	if (fi->constraint_type == FitDataInfo::SCRIPT)  
	  fi->custom.Run();  //script should place value in fitEnergyVal 
      }
      //now redo sum for final objective function
      val = 0.0;
      FOR_ITR_EL(FitDataInfo, fi, fitData->data., citr) { //calc all vals
	val+=fi->fitEnergyVal*fi->constraint_wt;
      }
    }
    else dFit.ReportToMaster(&(fitData->data));  //slave, so send data
  }
  return val;  //if dFit.used, this value is meaningless for slaves
}
 
void RtConstrain::findTrialProc() { 
  trial_proc = (RtTrial*)FindSubProc(&TA_RtTrial); 
  if(trial_proc == NULL)  
    taMisc::Error("RtConstrain process cannot find RtTrial sub-process"); 
} 
 
void RtConstrain::PrepDoFit() { 
  int i, idx; 
  ModelParamEntry *mpe; 
 
  if (fit_method == OPT_BFGS) op.isRun = true; 
  else if (fit_method == OPT_SIMPLEX) simp.isRun = true; 
  else if (fit_method == OPT_SUBPLEX) subp.isRun = true;
 
  // set fit ModelParamEntry test_delta_p to -1.0 
  for (i=0; i < params->fitIndices.size; i++) { 
    idx = params->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
    mpe = params->paramMembers.SafeEl(idx); //get ModelParamEntry 
    mpe->test_delta_p = -1.0; 
  } 
} 
 
void RtConstrain::Loop() { 
  int i, idx; 
  ModelParamEntry *mpe; 
  bool keepGoing = false; //assertion may change 
 
  if ((dFit.used) && !(dFit.master)) {float dummy = fitEnergy(); return;} //slave

  // ----- 
  //trial_proc->fixWts(); 
  //calcDE_DP(); 
  //updateParams(); 
  //trial_proc->unclampWts(); 
 
  //check for changes to ModelParam 
  if (params->changeCount != paramSetChangeCount) { //need to update list of which params to fit? 
    paramSetChangeCount = params->changeCount;  //mark as updated 
    op.setNumParams(params->fitIndices.size); 
    simp.setNumParams(params->fitIndices.size);
    subp.setNumParams(params->fitIndices.size);
    //op.Init(); already called by setNumParams() 
  } 
 
  // crank through an iteration 
  if (fit_method == OPT_BFGS) { 
      op.doOptStep(); 
      if (!op.isRun) { //only allow convergence if delta_p's not decreasing 
	for (i=0; i < params->fitIndices.size; i++) { 
	  idx = params->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
	  mpe = params->paramMembers.SafeEl(idx); //get ModelParamEntry 
	  if ((mpe->test_delta_p < 0.0) || (mpe->test_delta_p > mpe->delta_p)) {   
	    //no candidate convergence yet, since delta_p may be decreasing 
	    keepGoing = true; mpe->test_delta_p = mpe->delta_p; 
	  } 
	} 
	if (keepGoing) op.isRun = true; //override BFGS convergence signal 
	else if (op.retries > 0) {op.isRun = true; op.retries--;} 
      } 
  } 
  else if (fit_method == OPT_SIMPLEX) { 
      simp.doOptStep(); 
  }
  else subp.doOptStep();
} 

void RtConstrain::SaveParams() {
  RtSaveParams(this);
}
 
void RtSaveParams(void *data) {
  RtConstrain *rtc = (RtConstrain *)data; 
  if (!(rtc->loopDumpParams)) return;
  ModelParam *mp = rtc->params; 
  int i, idx; 
  ModelParamEntry *mpe;
  const char *buf = rtc->paramDumpFile;
  char *sname, *mname, *subname;
  FILE *out = fopen(buf,"w");
  if (out == NULL) {
    fprintf(stderr,"Error:  could not save params to %s\n",buf); return;
  }
 
  for (i=0; i < mp->fitIndices.size; i++) { 
    idx = mp->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
    mpe = mp->paramMembers.SafeEl(idx); //get ModelParamEntry 
    sname = mpe->SpecName; mname = mpe->MemberName; 
    if (mpe->subGroup.isSubGroup) {
      subname = mpe->subGroup.SubMemberName;
      fprintf(out,"%s %s %s %f\n", sname, mname, subname, mpe->getVal());
    }
    else
      fprintf(out, "%s %s %f\n", sname, mname, mpe->getVal());
    //p->Set(i, mpe->getVal());  //parameter is its value 
  } 
  fclose(out);
}

void RtGetParams(void *data, float_Array *p) { 
  RtConstrain *rtc = (RtConstrain *)data; 
  //optim *op = &(rtc->op); 
  ModelParam *mp = rtc->params; 
  int i, idx; 
  //float val; 
  ModelParamEntry *mpe; 
 
  for (i=0; i < mp->fitIndices.size; i++) { 
    idx = mp->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
    mpe = mp->paramMembers.SafeEl(idx); //get ModelParamEntry 
    p->Set(i, mpe->getVal());  //parameter is its value 
  } 
 
} 
 
void RtSetParams(void *data, float_Array *p) { 
  RtConstrain *rtc = (RtConstrain *)data; 
  //optim *op = &(rtc->op); 
  ModelParam *mp = rtc->params; 
  int i, idx; 
  ModelParamEntry *mpe; 
 
  for (i=0; i < mp->fitIndices.size; i++) { 
    idx = mp->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
    mpe = mp->paramMembers.SafeEl(idx); //get ModelParamEntry 
    mpe->setVal(p->SafeEl(i));   //set ModelParamEntry value to parameter 
  } 
 
} 
 
float RtFitObjFunc(void *data, float_Array *p) { 
  RtConstrain *rtc = (RtConstrain *)data; 
  //optim *op = &(rtc->op); 
  RtSetParams(data,p); 
  return rtc->fitEnergy(); 
} 
 
// calcs gradient and adjusts delta_p for finite diff. gradient approximation 
void RtFitCalcGrad(void *data, float_Array *p, float_Array *g) { 
  RtConstrain *rtc = (RtConstrain *)data; 
  //optim *op = &(rtc->op); 
  ModelParam *mp = rtc->params; 
  ModelParamEntry *pe; 
  int i, idx; 
  float origParam, gval; 
 
  //get baseline 
  RtSetParams(data,p); 
  float baseline = rtc->fitEnergy(); 
 
  //calc gradient, one parameter at a time 
  for (i=0; i < mp->fitIndices.size; i++) { 
    idx = mp->fitIndices.SafeEl(i);  //find index of ModelParamEntry 
    pe = mp->paramMembers.SafeEl(idx); //get ModelParamEntry 
 
    // adjust delta_p downward toward min if possible 
    if (pe->delta_p > rtc->min_delta_p)  pe->delta_p*=0.5; //exponential decay 
    if (rtc->dp_bounding == RtConstrain::DP_ABS) { //absolute 
      pe->delta_p = MMAX(pe->delta_p, rtc->min_delta_p); //enforce lower bound 
    } else { //relative 
      pe->delta_p = MMAX(pe->delta_p, rtc->min_delta_p*pe->getVal()); 
    } 
    //safeguard against infinite loop below: 
    pe->delta_p = MMAX(pe->delta_p, 1.0e-8); //shouldn't ever need smaller? 
 
    //calc gradient, doubling (exponential increase) delta_p if needed 
    // user should not set max_delta_p too high, or this will iterate a long time 
    // also, if min_delta_p too low, may not hit stopping criteria 
    while (1) { 
      origParam = pe->getVal(); 
      pe->setVal(origParam + pe->delta_p); 
      gval = (rtc->fitEnergy() - baseline)/pe->delta_p; 
      // Is there any point to another iteration? 
      if ((gval > 0.0) ||(pe->delta_p >= rtc->max_delta_p)) break;  //no 
      pe->delta_p*=2.0;       //otherwise, try increasing delta_p 
      if (rtc->dp_bounding == RtConstrain::DP_ABS) { //absolute 
        pe->delta_p = MMIN(pe->delta_p, rtc->max_delta_p); //enforce upper bound 
      } else { //relative 
        pe->delta_p = MMIN(pe->delta_p, rtc->max_delta_p*pe->getVal()); 
      } 
    }; 
 
    g->Set(i, gval); 
    pe->setVal(origParam);   //restore original value 
  } 
} 
 
// ----- Optimization code: --------------------------------- 
// Class optim implements the BFGS version of DFP minimization 
 
void optim::Initialize() { 
  maxit = 100; 
  machEps = 3.0e-8; 
  optim_tolx = 4.0*machEps; 
  bfgs_max_step = 100.0; 
  ns_tol = 1.0e-7; 
  small_f = 1.0e-4; 
 
  objFunc = NULL; 
  calcGrad = NULL; 
  gtol = 0.001; //max delta of any gradient element at convergence 
 
  forceInitNextRun = false; 
  retries = 0; 
} 
 
void optim::setNumParams(int num) { 
  numParams = num; 
  int i,j; 
 
  p.Reset(); 
  pnew.Reset(); 
  g.Reset(); 
  delta_g.Reset(); 
  hdelta_g.Reset(); 
  xi.Reset(); 
  hessian.Reset(); 
 
  //Rebuild array and matrix sizes 
  for (i=0; i < numParams; i++) { 
    p.Add(0.0); pnew.Add(0.0); g.Add(0.0); delta_g.Add(0.0); hdelta_g.Add(0.0);xi.Add(0.0); 
    for (j=0; j<numParams; j++) hessian.Add(0.0); 
  } 
 
  forceInitNextRun = true; 
} 
 
void optim::Init() 
{ 
    int i,j; 
    float tmpsum = 0.0, tmp; 
 
    //need to get parameters (initial condition) into p 
    (*getParams)(data, &p); 
 
    //Calculate starting function value & grad 
    if (objFunc) {fp=objFunc(data, &p); its++;} 
    else taMisc::Error("No objective function specified!"); 
    //printf("Initial error = %f", fp); fflush(stdout); 
    if (calcGrad) {calcGrad(data, &p,&g); its+=numParams+1; } 
    else taMisc::Error("No gradient function specified!"); 
 
    for (i=0;i<numParams;i++)  
      {   //and initialize the inverse Hessian to the unit matrix.  
	  for (j=0;j<numParams;j++) setmat(hessian,i,j, 0.0);  
	  setmat(hessian,i,i,1.0);  
	  xi.Set(i, -g.SafeEl(i)); //Initial line direction.  
	  tmp= p.SafeEl(i); 
	  tmpsum += tmp*tmp;  
      }  
    stpmax=bfgs_max_step*MMAX(sqrt(tmpsum),(double)numParams); 
 
    forceInitNextRun = false; 
    isRun = true; 
    its = 0;  
} 
 
void optim::doOptStep() 
{ 
    int i,j; 
    float test, temp; 
 
    if (!isRun) return; 
    if (forceInitNextRun) Init(); 
    if (numParams == 0) { 
      taMisc::Error("Warning:  No parameters selected for fitting!"); 
      finishOpt(); 
      return; 
    } 
	  doLineSrch(numParams,&p,fp,&g,&xi,&pnew,&curf,stpmax,&check);  
	  //printf(".");//printf("Saving parameters after iteration %i\n", its); 
	  printf("Parameter fit:  Objective function = %f\n", curf); 
	  fflush(stdout); 
 
	  //Enforce bounds 
          (*setParams)(data,&pnew);  //update parameter object 
	  (*getParams)(data,&pnew);  // enforces legal bounds on parameters 
 
	  if (curf > fp) fprintf(stderr,"Error!  obj. func. increasing!\n"); 
	  fp = curf;  
	  for (i=0;i<numParams;i++)  
	  {  
	    xi.Set(i,pnew.SafeEl(i)-p.SafeEl(i)); 
	    p.Set(i,pnew.SafeEl(i)); 
	  }  
	  test=0.0; //Test for convergence on x.  
	  for (i=0;i<numParams;i++)  
	  {  
	      temp=fabs(xi.SafeEl(i))/MMAX(fabs(p.SafeEl(i)),1.0);  
	      if (temp > test) test=temp;  
	  }  
	  if (test < optim_tolx) { finishOpt(); return; }  
	  for (i=0;i<numParams;i++) delta_g.Set(i, g.SafeEl(i)); //Save the old gradient,  
	  (*calcGrad)(data,&p,&g); its+=numParams+1;//and get the new gradient.  
	  test=0.0; //will test for convergence 
	  den=MMAX(curf,1.0);  
	  for (i=0;i<numParams;i++)  
	  {  
	      temp=fabs(g.SafeEl(i))*MMAX(fabs(p.SafeEl(i)),1.0)/den;  
	      if (temp > test) test=temp;  
	  }  
	  if (test < gtol) { finishOpt(); return; }  
	  for (i=0;i<numParams;i++) delta_g.Set(i, g.SafeEl(i)-delta_g.SafeEl(i)); 
	  for (i=0;i<numParams;i++)  
	  {  
	      hdelta_g.Set(i, 0.0);  
	      for (j=0;j<numParams;j++)  
		hdelta_g.Set(i, hdelta_g.SafeEl(i)  
		      + getmat(hessian,i,j)*delta_g.SafeEl(j));  
	  }  
	  bfgs_a=bfgs_c=sumdelta_g=sumxi=0.0;  
	  for (i=0;i<numParams;i++)  
	  { 
	      float tmp; 
	      bfgs_a += delta_g.SafeEl(i)*xi.SafeEl(i);  
	      bfgs_c += delta_g.SafeEl(i)*hdelta_g.SafeEl(i);  
	      tmp = delta_g.SafeEl(i); sumdelta_g += tmp*tmp;  
	      tmp = xi.SafeEl(i); sumxi += tmp*tmp;  
	  }  
	  if (bfgs_a > sqrt(machEps*sumdelta_g*sumxi))  
	  {   //Skip update if bfgs_a not sufficiently positive.  
	      bfgs_a=1.0/bfgs_a;  
	      bfgs_b=1.0/bfgs_c;  
	      for (i=0;i<numParams;i++) delta_g.Set(i, bfgs_a*xi.SafeEl(i)-bfgs_b*hdelta_g.SafeEl(i));  
	      for (i=0;i<numParams;i++)  
	      {  
 
		  for (j=i;j<numParams;j++)  
		  {  
		      setmat(hessian,i,j, (getmat(hessian,i,j) 
			    +bfgs_a*xi.SafeEl(i)*xi.SafeEl(j)  
			   -bfgs_b*hdelta_g.SafeEl(i)*hdelta_g.SafeEl(j)  
			    +bfgs_c*delta_g.SafeEl(i)*delta_g.SafeEl(j))); 
		      setmat(hessian,j,i, (getmat(hessian,i,j))); 
		  }  
	      }  
	  }  
	  for (i=0;i<numParams;i++)  
	  {  
	      // Find next update direction  
	      xi.Set(i,0.0); //xi[i]=0.0;  
	      for (j=0;j<numParams;j++) //xi[i] -= hessian[i][j]*g[j];  
		xi.Set(i, (xi.SafeEl(i)-g.SafeEl(j)*getmat(hessian,i,j))); 
	  }  
 
	  //its++;  
 
          if (its > maxit) {printf("fit:  maxitt exceeded\n"); finishOpt();} 
} 
 
void optim::doLineSrch(int n, float_Array *xold, float fold, float_Array *gloc, float_Array *ploc, float_Array *x, float *fval3, float max_step, int *vfy)  
{ 
    int i;  
    float a,nsval,ns2=0.0,nsmin,b,disc,fval2=0.0; //assigning a value keeps  
                   //compiler from whining, even though values aren't used 
    float term_ns1,term_ns2,slope,sum,temp,test,tmpns,tmp; 
    float min_f = fold, min_ns = 0.0;   
    float ns0 = 1.0;  
    *vfy=0;  
    for (sum=0.0,i=0;i<n;i++) {tmp = ploc->SafeEl(i); sum += tmp*tmp; } 
    sum=sqrt(sum);  
    if (sum > max_step) for (i=0;i<n;i++) ploc->SafeEl(i) *= max_step/sum;  
           //Scale if attempted step is too big.  
    for (slope=0.0,i=0;i<n;i++) slope += gloc->SafeEl(i)*ploc->SafeEl(i);  
    if (slope >= 0.0) printf("doLineSrch roundoff problem");  
    test=0.0; //Compute  min.  
    for (i=0;i<n;i++)  
    { 
        temp=fabs(ploc->SafeEl(i))/MMAX(fabs(xold->SafeEl(i)),1.0);  
	if (temp > test) test=temp;  
    }  
    nsmin=ns_tol/test;  
    nsval=ns0; // try Newton step  
    for (;;)  
    {   //Start of iteration loop.  
        for (i=0;i<n;i++) x->Set(i,(xold->SafeEl(i)+nsval*ploc->SafeEl(i)));  
	*fval3=(*objFunc)(data, x); its++; 
	if (*fval3 < min_f) { min_f = *fval3; min_ns = nsval;} //Hack (Josh 7/17/02) 
	if (nsval < nsmin)   //Convergence? 
	{  
	    *vfy=1;  
	    if (*fval3 > min_f) { 
	      //Hack added 7/17/02 Josh --guarantees that obj func won't decrease 
	      //check for objective function increasing; roll back to min if so 
              for (i=0;i<n;i++) x->Set(i, (xold->SafeEl(i)+min_ns*ploc->SafeEl(i)));  
	      *fval3 = min_f; 
	    } 
	    return;  
	}  
	else if (*fval3 <= fold+small_f*nsval*slope) { //Sufficient function decrease.  
	  if (*fval3 > min_f) { 
	    //Hack added 7/17/02 Josh -- guarantees that obj func won't decrease 
	    //check for objective function increasing; roll back to min if so 
            for (i=0;i<n;i++) x->Set(i, (xold->SafeEl(i)+min_ns*ploc->SafeEl(i)));  
	    *fval3 = min_f; 
	  } 
	  return; 
	}                  
	else  
	{  
	    //Backtrack 
	    if (nsval == ns0) tmpns = -slope/(2.0*(*fval3-fold-slope));  
	    //First backtrack 
	    else  
	    {  
	        //Subsequent backtracks 
	        term_ns1 = *fval3-fold-nsval*slope;  
		term_ns2=fval2-fold-ns2*slope;  
		a=(term_ns1/(nsval*nsval)-term_ns2/(ns2*ns2))/(nsval-ns2);  
		b=(-ns2*term_ns1/(nsval*nsval)+nsval*term_ns2/(ns2*ns2))/(nsval-ns2);  
		if (a == 0.0) tmpns = -slope/(2.0*b);  
		else  
		{  
		    disc=b*b-3.0*a*slope;  
		    if (disc < 0.0) tmpns=0.5*nsval;  
		    else if (b <= 0.0) tmpns=(-b+sqrt(disc))/(3.0*a);  
		    else tmpns=-slope/(b+sqrt(disc));  
		}  
		if (tmpns > 0.5*nsval) tmpns=0.5*nsval; 
	    }  
	}  
	ns2=nsval;  
	fval2 = *fval3;  
	nsval=MAX(tmpns,0.1*nsval); 
    } 
} 
 
//--------------- Simplex Algorithm ------------------- 
// Implements Nelder-Mead (1965) simplex gradient descent 
 
void simplex::Initialize() { 
  maxitt = 5000;      //maximum allowed objective function evaluations 
  ftol = 1.0e-6;      //default, arbitrary 
  ftol_test = 0.0;    // will change 
  //#IGNORE init_len = 0.1;   //characteristic length; determines initial size of simplex 
  init_dp = 0.1;      //allow default initial 10% variation in parameters to make simplex 
  min_init_dp = 0.001; //don't let simplex start off too small 
  its = 0; 
  isRun = false; 
  forceStop = false; 
  forceInitNextRun = true; 
  retries = 1; 
  f_low = 1.0e10; 
  rep_f_low_min = false;
  //doFLoAvg = false;
 
  objFunc = NULL;     //flag function pointers as null until explicitly set 
  getParams = NULL; 
  setParams = NULL; 
  data = NULL; 
 
  taBase::Own(p, this);  taBase::Own(psum, this); taBase::Own(fvals, this); 
  taBase::Own(s, this);  taBase::Own(ap, this);
} 
 
 
void simplex::setNumParams(int num) { 
  numParams = num;
  numActiveParams = num;
  forceInitNextRun = true; 
  setAllParamsActive(); //default
} 

void simplex::resetActiveParams() {
  numActiveParams = 0;
  ap.Reset();
}

void simplex::setAllParamsActive () {
  int i;
  ap.Reset();
  for (i=0; i < numParams; i++) ap.Add(i);
  numActiveParams = numParams;
}

void simplex::setParamActive (int which) {
  if (ap.Find(which) >= 0) return; //already active
  ap.Add(which);
  numActiveParams++;
}
 
void simplex::Init() { 
  
  InitSimplex();
} 
 
void simplex::InitSimplex() {  //assumes model params reflect current best -- IS THIS TRUE???
  int i, j; 
  spts = numActiveParams+1; 
  float_Array ptmp; 
 
  p.Reset(); 
  s.Reset(); 
  psum.Reset(); 
  fvals.Reset(); 
  ptmp.Reset(); 

  //first get p matrix 
  for (i=0; i < numParams; i++) p.Add(0.0); 
  (*getParams)(data, &p); 
 
  //Rebuild array and matrix sizes 
  for (i=0; i < numParams; i++) { 
    for (j=0; j < spts; j++) s.Add(0.0); 
    psum.Add(0.0); 
  } 
 
  //Assume initial guess is in p, so calculate initial remaining vectors in p and their objective function values 
   
  //make numActiveParams+1 copies of p vector  
  for (i=0; i < spts; i++) { 
    for (j=0; j < numParams; j++) setpmat(s,i,j, p.SafeEl(j)); 
  } 
 
  //add characteristic length (delta_p) to all but first param vector 
  for (i=0; i < numActiveParams; i++) setpmat(s,i+1,ap.SafeEl(i), (p.SafeEl(ap.SafeEl(i))+initStepSize(ap.SafeEl(i)))); 
   
  //calculate objective function values for vertices of simplex 
  for (i=0; i < spts; i++) { 
    ptmp.Reset(); 
    for (j=0; j < numParams; j++) ptmp.Add(getpmat(s,i,j)); 
    if (objFunc) {fvals.Add((*objFunc)(data, &ptmp));  its++;} 
  } 
  
  //clean up -- restore p to ModelParam
  (*setParams)(data, &p);

  // Set up for optimization 
  calcPsum(); 
  its = 0; 
  isRun = true; 
  //f_low = 1.0e10; 
 
  forceInitNextRun = false; 
}

void simplex::checkInitSimplex() {
   if (forceInitNextRun && !forceStop) InitSimplex();
}

void simplex::sortFVals() {
  int i;
  // find highest, next highest, and lowest y 
  idx_lo = 0; 
  if (fvals.SafeEl(0) > fvals.SafeEl(1)) {idx_hi = 0; idx_nhi = 1;} 
  else {idx_hi = 1; idx_nhi = 0;} 
 
  for (i=0; i < spts; i++) { 
    if (fvals.SafeEl(i) < fvals.SafeEl(idx_lo)) idx_lo = i; 
 
    if (fvals.SafeEl(i) > fvals.SafeEl(idx_hi)) { idx_nhi = idx_hi; idx_hi = i;} 
    else if ((fvals.SafeEl(i) > fvals.SafeEl(idx_nhi)) && (i != idx_hi)) idx_nhi = i; 
  } 
  f_low = fvals.SafeEl(idx_lo); 
}

bool simplex::checkForceStop() {
  if (forceStop) { 
    putBestPtInP();   
    if (setParams) (*setParams)(data, &p); //this should be redundant, since 
                                         // objFunc() below sets parameters, too 
    if (saveParams) (*saveParams)(data); //dump to file if set
    if (objFunc) (*objFunc)(data, &p);  //make sure fit data reflects best fit 
    isRun = false; 
    forceStop = false; 
    return true; 
  }
  else return false;
}

bool simplex::checkCrit() {
  if (simplexCrit()) { //possible convergence, so either restart or return 
    putBestPtInP(); 
    if (setParams) (*setParams)(data, &p);
    if (saveParams) (*saveParams)(data); //dump to file if set
    if (retries) { //still have some retries left, so restart at current best pt 
      retries--;  
      forceInitNextRun = true;    
    } 
    else { 
      // save best parameters and quit 
      if (objFunc) (*objFunc)(data, &p);  //make sure fit data reflects best fit 
      isRun = false; 
    }
    return true;
  } 
 
  if (its > maxitt) { 
      taMisc::Error("Simplex Optimization:  Maximum Iterations Exceeded"); 
      putBestPtInP(); 
      if (setParams) (*setParams)(data, &p); 
      if (saveParams) (*saveParams)(data); //dump to file if set
      if (objFunc) (*objFunc)(data, &p);  //make sure fit data reflects best fit 
      isRun = false; 
      return true; 
  }
  return false; //criterion not yet reached
}

void simplex::doOptStep() { 
  int i,j; 
  float tmp;
  float newf_lo;
 
  if (!isRun) return; 
  if (checkForceStop()) return;
  checkInitSimplex();
  sortFVals();
 
  // Did the algorithm converge?  
  if (checkCrit()) return;
 
  // if not stopping, then proceed with simplex algorithm step: 
  ftry = stepSimplex(-1.0); 
  if (ftry <= fvals.SafeEl(idx_lo))  //better than best point, so try stepping further 
    ftry = stepSimplex(2.0); 
  else if (ftry >= fvals.SafeEl(idx_nhi)) { //worse than second-highest, so contract 
    fsave = fvals.SafeEl(idx_hi); 
    ftry = stepSimplex(0.5); 
    if (ftry >= fsave) { //then contract around lowest point
DM("Contracting...\n"); 
      for (i=0; i < spts; i++) { 
	if (checkForceStop()) return; //otherwise, contraction takes a while
	if (i != idx_lo) { 
	  // initialize with all params, then contract only active params
	  for (j=0; j < numParams; j++) psum.Set(j, getpmat(s,i,j)); //psum used as temp var
	  for (j=0; j < numActiveParams; j++) { 
	    tmp = 0.5*(getpmat(s,i,ap.SafeEl(j)) + getpmat(s,idx_lo,ap.SafeEl(j))); 
	    psum.Set(ap.SafeEl(j), tmp);  //psum used as temp var
	    setpmat(s,i,ap.SafeEl(j), tmp); 
	  }
	  fvals.Set(i,(*objFunc)(data, &psum)); its++;
	} 
	else if (rep_f_low_min) { // evaluate and keep the minimum value?
	  newf_lo = (*objFunc)(data, &psum); its++;
	  if (newf_lo < f_low) fvals.Set(i, f_low);
	}
      }
      calcPsum(); 
    }
  }

  //putBestPtInP(); -- unnecessary?
  //(*setParams)(data, &p);
} 
 
float simplex::initStepSize(int whichParam) { 
  return MMAX(p.SafeEl(whichParam)*init_dp, min_init_dp); 
} 
 
void simplex::calcPsum() {  //only active parameters are summed; others undefined
  int i, j; 
  float sum; 
  for (j=0; j < numActiveParams; j++) { 
    for (sum=0.0, i = 0; i <=spts; i++) sum+=getpmat(s,i,ap.SafeEl(j)); 
    psum.Set(ap.SafeEl(j), sum); 
  } 
} 
 
float simplex::stepSimplex(float factor) { 
  int j; 
  float term1, term2;  
  float_Array ptry; 
 
  term1 = (1.0-factor)/(float)numParams; 
  term2 = term1-factor; 
 
  ptry.Reset();
  //default is current p set for inactive params
  for (j=0; j < numParams; j++) ptry.Add(p.SafeEl(j)); 
  //adjust active params in ptry
  for (j=0; j < numActiveParams; j++) ptry.Set(ap.SafeEl(j), psum.SafeEl(ap.SafeEl(j))*term1 -getpmat(s,idx_hi,ap.SafeEl(j))*term2); 
 
  ftry = (*objFunc)(data, &ptry);  its++; 
 
  if (ftry < fvals.SafeEl(idx_hi)) { //if better than highest, replace highest 
    fvals.Set(idx_hi, ftry); 
    for (j=0; j < numActiveParams; j++) { //update psum and param matrix s 
      psum.Set(ap.SafeEl(j), (psum.SafeEl(ap.SafeEl(j)) + ptry.SafeEl(ap.SafeEl(j))-getpmat(s,idx_hi,ap.SafeEl(j)) )); 
      setpmat(s,idx_hi,ap.SafeEl(j),ptry.SafeEl(ap.SafeEl(j))); 
    } 
  } 
 
  return ftry; 
} 
 
void simplex::putBestPtInP() { //store the best simplex vertex in array p 
  int i; 
  for (i=0; i < numParams; i++) p.Set(i, getpmat(s,idx_lo, i)); 
  f_low = fvals.SafeEl(idx_lo); 
} 

bool simplex::simplexCrit() { //did simplex converge
  ftol_test = 2.0*fabs(fvals.SafeEl(idx_hi)-fvals.SafeEl(idx_lo))/(fabs(fvals.SafeEl(idx_hi)) + fabs(fvals.SafeEl(idx_lo)) + 1.0e-10); 
  return (ftol_test < ftol);
} 

//---------------------------- Subplex -------------- 
// Implements Rowan (1990) subplex optimization algorithm
void subplex::Initialize() { 
  simplex::Initialize(); 
  psi = .25; 
  omega = .1; 
  subp_tol = 1.0e-4f;
  isFirstSimplexIter = true;
  subplexState = -1;
  isSimplexCrit = false;
  ssoff = 0;
  nsmin = 2; 
  nsmax = 5; 
  // initialize step
  // initialize dx
  taBase::Own(step, this); taBase::Own(dx,this); taBase::Own(perm, this);
  taBase::Own(lastxbest, this); taBase::Own(subsp_dims, this); 

} 

void subplex::ReInit() {
  int i;
  if (isRun) return; //{taMisc::Error("Cannot ReInit while running -- press stop first"); return;} //should only reinit if stopped
  subplexState = -1;
  f_low = 1.0e10;
  for (i=0; i < fvals.size; i++) fvals.Set(i, f_low);
  simplex::ReInit();
}

void subplex::Init() {
  int i;
DM("Subplex::Init()\n");
  subplexState = -1; //initializing
  isSimplexCrit = false;

  //first get p matrix 
  for (i=0; i < numParams; i++) p.Add(0.0); 
  (*getParams)(data, &p);

  step.Reset(); dx.Reset(); subsp_dims.Reset(); lastxbest.Reset();
  for (i=0; i < numParams; i++) {
    step.Add(simplex::initStepSize(i)); 
    dx.Add(0.0); 
    subsp_dims.Add(0);
    lastxbest.Add(p.SafeEl(i)); // save a copy
  }
  //(*setParams) (data, &p); //restore best parameter set, so simplex::InitSimplex() gets best param values -- this line fixes bug (1/3/03)

  //useSubspace(0);
  //InitSimplex();
}

void subplex::useSubspace(int which) {  //which is base first-element-is-zero
  int i, spSize;
  resetActiveParams();
  for (i=0, ssoff =0; i < which; i++) ssoff+=subsp_dims.SafeEl(i);
  spSize = subsp_dims.SafeEl(which);
  for (i=ssoff; i < (ssoff+spSize); i++) {
      setParamActive(perm.SafeEl(i));
  }
  
}

float subplex::initStepSize(int which) {
  return step.SafeEl(which);
}

void subplex::InitSimplex() { //overrides simplex::InitSimplex()
  float f_lo_tmp = f_low; //preserve lowest f val
  isFirstSimplexIter = true; // will calculate convergence crit
  retries = 0; //no restarts
DM("Initializing subplex simplex\n");
  simplex::InitSimplex();
  //f_low = f_lo_tmp; //restore
  //idx_lo = 0; //simplex::InitSimplex() puts current best point in position zero
  sortFVals(); //finds best newly calc'd f_low
  fvals.Set(idx_lo, MMIN(f_low, f_lo_tmp)); //if new init, uses newly calc'd f_low. Otherwise, if ongoing, uses previous best f_low, if lower than newly calc'd.
}

// step sizes

float L1Norm (float_Array *array) {
  int i;
  float norm = 0;
  for (i=0; i < array->size; i++) norm+= fabs(array->SafeEl(i));
  return norm;
}

int L1Norm (int_Array *array) {
  int i;
  int norm = 0;
  for (i=0; i < array->size; i++) norm+= abs(array->SafeEl(i));
  return norm;
}

float L2Norm(float_Array *array) {
  int i;
  float norm = 0;
  for (i=0; i < array->size; i++) norm+= (array->SafeEl(i))*(array->SafeEl(i));
  return norm;
}

void subplex::updateStepVec() {
  float step_factor =0.0, stepval;
  float stepL1Norm = L1Norm(&step);
  int i;

  stepL1Norm = stepL1Norm > 0.0f ? stepL1Norm : 1.0e-6; //avoid divide by zero

  if (nsp > 1)
    step_factor = MMIN(MMAX(L1Norm(&dx)/stepL1Norm,omega),1.0/omega);
  else step_factor = psi;
DM2("step_factor = %f\n", step_factor);
  for (i=0; i < numParams; i++) {
    stepval = step.SafeEl(i)*step_factor;
    if (dx.SafeEl(i) == 0.0) stepval = -stepval;
    else stepval = fabs(stepval)*(dx.SafeEl(i) > 0.0 ? 1.0 : -1.0);
    step.Set(i,stepval);
  }
}

// set subspaces
void subplex::setSubsp() {
  bool crit = false;
  bool warn;
  //nleft is number of params remaining to be grouped into subspaces
  int nleft, nused; 
  int i,j, itmp; 
  float tmp, gap, gapmax, as1, as2, asleft, as1max = 0.0f; 
  int ns1, ns2;
  for (i=0, perm.Reset(); i < numParams; i++) {
    perm.Add(i); //perm= [0..numParams]
    dx.Set(i,fabs(dx.SafeEl(i))); //want magnitude
  }
  subsp_dims.Reset();

  // sort dx descending, keeping permutation vector
  // use bubble sort -- easy to implement
  for (i=0; i < numParams-1; i++)
    for (j=0; j < numParams-1-i; j++)
      if (dx.SafeEl(j) < dx.SafeEl(j+1)) { //swap
	     tmp = dx.SafeEl(j); dx.Set(j, dx.SafeEl(j+1)); dx.Set(j+1,tmp);
	     itmp = perm.SafeEl(j); perm.Set(j, perm.SafeEl(j+1)); perm.Set(j+1,itmp);
      }

  // num_subpsaces
  nsp = 0; //reset
  nused = 0;
  nleft = numParams;
  for (i=0, asleft = 0.0; i < numParams; i++) asleft+=dx.SafeEl(i);

  while ((nused < numParams) && !crit) {
    nsp++; 
    subsp_dims.Add(nsmin); warn = true; //default should always be changed below
DM2("subsp_dims size = %i\n", subsp_dims.size);
    //sum up to but not including nsmin
    for (as1=0.0, i=1; i < nsmin; i++) as1+=dx.SafeEl(perm.SafeEl(i+nused-1));
    gapmax = -1.0;
    for (ns1 = nsmin; ns1 <= MMIN(nsmax,nleft); ns1++) {
      as1 += dx.SafeEl(perm.SafeEl(nused+ns1-1));
      ns2 = nleft - ns1;
      if (ns2 > 0) {
	//itmp = nsmin* (int) truncf((float)(ns2-1)/(float)(nsmax+1));
	itmp = nsmin*(1+ (int) truncf(((float)ns2-1.0)/(float)nsmax));
	if (ns2 >= itmp) {
	  as2 = asleft-as1;
	  gap = as1/(float)ns1 - as2/(float)ns2;
	  if (gap > gapmax) {
	    gapmax = gap; 
	    subsp_dims.Set(nsp-1, ns1); //ns1 is size (# dims) of n-th subspace
	    warn = false;
	    as1max = as1;
	  }
	}
      } else {
	if ((as1/(float)ns1) > gapmax) { 
	  subsp_dims.Set(nsp-1,ns1); warn = false; crit = true; break;
	}
      }
    } //end for
    if (warn) {
      fprintf(stderr,"Warning: Subplex failed to properly partition subspace:  using min subspace size: dx, perm = \n");
      for(j=0; j < dx.size; j++) fprintf(stderr,"%f, %i\n",dx.SafeEl(j), perm.SafeEl(j));
    }
    nused += subsp_dims.SafeEl(nsp-1);
    if (nused > numParams) {subsp_dims.Set(nsp-1,subsp_dims.SafeEl(nsp-1)-nused+numParams); if (numParams >= nsmin) fprintf(stderr,"Warning:  truncated last subspace due to partitioning algorithm failure\n"); }//hack -- shouldn't ever happen
DM2("nused = %i\n", nused);
    nleft = numParams - nused;
    asleft -= as1max;
  } //end while

  // now subsp_dims has nsp entries, each with the number of dims in each subsp
  
}

float subplex::computeSimplexSize() {  //used for subplex simplex convergence criterion
  int i;
  float tmp = 0.0f, sval=0.0;
  
  for (i=0; i < numParams; i++) p.Set(i, getpmat(s,idx_lo, i)); 

  for (i=0; i < numActiveParams; i++) {
    tmp = getpmat(s,idx_hi,ap.SafeEl(i))-getpmat(s,idx_lo,ap.SafeEl(i));
    sval+=tmp*tmp;
  }
  return sval;
}

bool subplex::simplexCrit() { //overrides simplex::simplexCrit()
  // criterion is true if the L2 distance between hi & lo simplex vertices (in parameter space) decreases by a factor of psi.

  if (isFirstSimplexIter) {
    simp_tol =psi*psi*computeSimplexSize(); 
    isFirstSimplexIter = false; 
    DM2("init:  simplex crit = %e\n",simp_tol);
    return false;
  }
  else {
    DM2("simplex test = %e\n",computeSimplexSize());
    if ((computeSimplexSize() <= simp_tol) || (its > maxitt)) return true; 
    else return false;
  }

}

bool subplex::subplexOuterCrit() {
  int i;
  float crit;
  float maxDx= -1.0f, maxX = -1.0f, maxStep = -1.0f;
  for (i=0; i < numParams; i++) {
    maxDx = MMAX(fabs(dx.SafeEl(i)),maxDx);
    maxX =  MMAX(fabs(p.SafeEl(i)),maxX);
    maxStep = MMAX(fabs(step.SafeEl(i)),maxStep);
  }

  crit = MMAX(maxDx,maxStep*psi)/MMAX(maxX,1.0f);
  return crit < subp_tol;
}

bool subplex::checkCrit() {
 if (simplexCrit()) { //possible convergence, so either restart or return 
    if (its > maxitt) { fprintf(stderr,
	"Simplex Optimization:  Maximum Iterations Exceeded"); fflush(stderr); }
    putBestPtInP(); 
    if (setParams) (*setParams)(data, &p); 
    if (saveParams) (*saveParams)(data); //dump to file if set
    if (objFunc) (*objFunc)(data, &p);  //make sure fit data reflects best fit 
    return true;
  }
 return false;
}

void subplex::doOptStep() {
  int i;
  if (!isRun) return;

  //check for forced resets

  //check for SUBplex initialization
  if (subplexState == -1) { //end of loop or initializing
    // check for convergence, then check if subplex initialization needed
DM("Checking subplex outer crit");

    if (!forceInitNextRun) {
      putBestPtInP(); 
      if (setParams) (*setParams)(data, &p);
      if (saveParams) (*saveParams)(data); //dump to file if set

      //update dx, lastxbest, then check for subplex convergence
      for (i=0; i < numParams; i++) {
        dx.Set(i,(p.SafeEl(i)-lastxbest.SafeEl(i)));
        lastxbest.Set(i,p.SafeEl(i));
      }
      if (subplexOuterCrit()) { //save and quit
	if (objFunc) (*objFunc)(data, &p); //make sure fit data reflects best fit
	isRun = false; 
	return;
      }
    }
    else {DM("Init()"); Init(); forceInitNextRun = false;} 

 if (numParams < 1) {taMisc::Error("No parameters specified!"); isRun= false; return;} 

 //DM("UpdateStepVec()\n");
    updateStepVec();
 //DM("setSubsp()");
    setSubsp();
 //DM("done setSubsp()");
    subplexState = 0; // go on to do first subspace optimization next
    isSimplexCrit = false;
    ssoff = 0; //necessary?
    useSubspace(subplexState);
    InitSimplex(); //Initializes the first simplex subspace
 //DM("done useSubspace\n");
    return; // good enough for now; will start optimizations on next loop
  }

  // check for SIMplex subspace initialization
  if (isSimplexCrit) { //current subspace converged; move on to next
    isSimplexCrit = false;

    subplexState++;
    if (subplexState >= nsp) { subplexState = -1; return;} //finished subplex iter
    useSubspace(subplexState);

    //Be sure that best parameters are in ModelParam for InitSimplex()
    putBestPtInP(); 
    if (setParams) (*setParams)(data, &p);
    if (saveParams) (*saveParams)(data); //dump to file if set

    InitSimplex();
    DM2("isFirstSimplexIter = %i\n", isFirstSimplexIter);
    return; //good enough for now; will step simplex the next iteration
  }

  //In the middle of optimizing a subspace, so step it
  simplex::doOptStep();
  if (simplexCrit()) isSimplexCrit = true;
}

