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

#ifdef __GNUG__
#pragma implementation
#endif
#include "biobel.h"
#include <pdp/enviro.h>
#include <math.h>
#include <values.h>

//////////////////////////
//  	Con, Spec	//
//////////////////////////

void BioBelConSpec::Initialize() {
  min_obj_type = &TA_BioBelCon_Group;
  min_con_type = &TA_BioBelCon;
  wt_limits.min = 0.0f;
  wt_limits.max = 1.0f;
  wt_limits.sym = true;
  wt_limits.type = WeightLimits::MIN_MAX;
  wt_gain = 1.0f;
  k_gaus = 2.0f;
}

void BioBelConSpec::UpdateAfterEdit() {
  ConSpec::UpdateAfterEdit();
}

//////////////////////////
//  	Unit, Spec	//
//////////////////////////

void BioBelChannels::Initialize() {
  e = l = i = h = a = 0.0f;
}

void BioBelChannels::Copy_(const BioBelChannels& cp) {
  e = cp.e;
  l = cp.l;
  i = cp.i;
  h = cp.h;
  a = cp.a;
}

void VChanSpecs::Initialize() {
  type = NO_CHAN;
  dt = 0.1f;
  thresh = .1f;
  power = 1.0f;
}

void VChanSpecs::Copy_(const VChanSpecs& cp) {
  type = cp.type;
  dt = cp.dt;
  thresh = cp.thresh;
  power = cp.power;
}

void ActFunSpecs::Initialize() {
  thresh = .3f;
  gain = 600.0f;
  v_m_rest = .15f;
  g_e_rest = 0.0f;		// needs to be computed
}

void BioBelUnitSpec::Initialize() {
  min_obj_type = &TA_BioBelUnit;
  vm_range.max = 1.0f;
  vm_range.min = 0.0f;

  netin_fun = DOT_PRODUCT;
  learn_fun = DOT_PRODUCT;
  act_fun = NOISY_XX1;
  bias_wts = BIAS_NETIN;
  vm_dt = 0.7f;
  net_dt = 0.7f;

  g_bar.e = 1.5f;
  g_bar.l = 0.01f;
  g_bar.i = 1.0f;
  g_bar.h = 0.1f;
  g_bar.a = 0.1f;
  e_rev.e = 1.0f;
  e_rev.l = 0.0f;
  e_rev.i = 0.15f;
  e_rev.h = 0.70f;
  e_rev.a = 0.0f;

  hyst.type = VChanSpecs::NO_CHAN;
  hyst.dt = 0.1f;
  hyst.thresh = .31f;
  hyst.power = 1.0f;

  acc_f.type = VChanSpecs::NO_CHAN;
  acc_f.dt = 0.1f;
  acc_f.thresh = .1f;
  acc_f.power = 1.0f;

  acc_s.type = VChanSpecs::NO_CHAN;
  acc_s.dt = 0.01f;
  acc_s.thresh = .31f;
  acc_s.power = 2.0f;
  acc_s_gain = 100.0f;

  noise_type = NO_NOISE;
  noise.type = Random::NONE;
  noise.var = .005f;
  v_m_init.type = Random::UNIFORM;
  v_m_init.mean = .15f;
  v_m_init.var = 0.0f;

  noise_conv.x_range.min = -.05f;
  noise_conv.x_range.max = .05f;
  noise_conv.res = .001f;
  noise_conv.x_range.UpdateAfterEdit();

  nxx1_fun.x_range.min = -.03f;
  nxx1_fun.x_range.max = .20f;
  nxx1_fun.res = .001f;
  nxx1_fun.x_range.UpdateAfterEdit();

  send_thresh = .1f;
}

void BioBelUnitSpec::InitLinks() {
  UnitSpec::InitLinks();
  taBase::Own(vm_range, this);
  taBase::Own(act, this);
  taBase::Own(g_bar, this);
  taBase::Own(e_rev, this);
  taBase::Own(hyst, this);
  taBase::Own(acc_f, this);
  taBase::Own(acc_s, this);
  taBase::Own(v_m_init, this);
  taBase::Own(noise, this);
  taBase::Own(noise_sched, this);
  taBase::Own(nxx1_fun, this);
  taBase::Own(noise_conv, this);
}

void BioBelUnitSpec::UpdateAfterEdit() {
  UnitSpec::UpdateAfterEdit();

  // rest input just balances the leak current to keep at v_m_init
  act.g_e_rest = (g_bar.l * (e_rev.l - act.v_m_rest)) / (act.v_m_rest - e_rev.e);

  if(act_fun == NOISY_XX1)
    CreateNXX1Fun();
}

void BioBelUnitSpec::CreateNXX1Fun() {
  // first create the gaussian noise convolver
  noise_conv.AllocForRange();
  int i;
  float var = noise.var * noise.var;
  for(i=0; i < noise_conv.size; i++) {
    float x = noise_conv.x_range.min + ((float)i * noise_conv.res);
    noise_conv.FastEl(i) = expf(-((x * x) / var));
  }

  // normalize it
  float sum = noise_conv.Sum();
  for(i=0; i < noise_conv.size; i++)
    noise_conv.FastEl(i) /= sum;

  // then create the initial function
  FunLookup fun;
  fun.x_range.min = nxx1_fun.x_range.min + noise_conv.x_range.min;
  fun.x_range.max = nxx1_fun.x_range.max + noise_conv.x_range.max;
  fun.res = nxx1_fun.res;
  fun.x_range.UpdateAfterEdit();
  fun.AllocForRange();

  for(i=0; i<fun.size; i++) {
    float x = fun.x_range.min + ((float)i * fun.res);
    float val = 0.0f;
    if(x > 0.0f)
      val = (act.gain * x) / ((act.gain * x) + 1.0f);
    fun.FastEl(i) = val;
  }

  nxx1_fun.Convolve(fun, noise_conv);
}

void BioBelUnitSpec::InitState(BioBelUnit* ru, BioBelLayer*) {
  UnitSpec::InitState(ru);
  ru->clmp_net = 0.0f;
  ru->net_norm = 0.0f;
  ru->prv_net = 0.0f;
  ru->da = 0.0f;
  ru->hyst = 0.0f;
  ru->acc_f = 0.0f;
  ru->acc_s = 0.0f;
  ru->g_a = 0.0f;
  ru->g_l = 0.0f;
  ru->g_i = 0.0f;
  ru->v_m = v_m_init.Gen();
  ru->act = 0.0f;
}

void BioBelUnitSpec::ReConnect_Load(BioBelUnit*, BioBelLayer*) {
  // do nothing..
}

void BioBelUnitSpec::CompToTarg(BioBelUnit* u, BioBelLayer*) {
  if(!(u->ext_flag & Unit::COMP))
    return;
  u->SetExtFlag(Unit::TARG);
}

// todo: what to do about wt_gain in normalizing netinput?

void BioBelUnitSpec::Compute_ClampNet(BioBelUnit* u, BioBelLayer*) {
  // this is all receiver-based and done only at beginning
  u->clmp_net = 0.0f;
  u->net_norm = 0.0f;
  if(netin_fun == DOT_PRODUCT) {
    BioBelCon_Group* recv_gp;
    int g;
    FOR_ITR_GP(BioBelCon_Group, recv_gp, u->recv., g) {
      BioBelLayer* lay = (BioBelLayer*) recv_gp->prjn->from;
      if(lay->lesion)		continue;
      u->net_norm += (float)recv_gp->size; // compute normalization value
      if(!lay->hard_clamped)	continue;
      u->clmp_net += recv_gp->Compute_Net(u);
    }
  }
  else {			// GAUSSIG
    BioBelCon_Group* recv_gp;
    int g;
    FOR_ITR_GP(BioBelCon_Group, recv_gp, u->recv., g) {
      BioBelLayer* lay = (BioBelLayer*) recv_gp->prjn->from;
      if(lay->lesion)		continue;
      u->net_norm += (float)recv_gp->size; // compute normalization value
      if(!lay->hard_clamped)	continue;
      u->clmp_net += recv_gp->Compute_GausSig_Net(u);
    }
  }
  // add the bias weight into the netinput
  if((bias_wts == BIAS_NETIN) && (u->bias != NULL)) {
    u->clmp_net += u->bias->wt;
    u->net_norm += 1.0f;
  }
}

void BioBelUnitSpec::Compute_Net(BioBelUnit* u, BioBelLayer*) {
  if(netin_fun == DOT_PRODUCT) {
    // sigmoid is sender-based (and this fun not called on hard_clamped EXT layers)
    if(u->act > send_thresh) {
      BioBelCon_Group* send_gp;
      int g;
      FOR_ITR_GP(BioBelCon_Group, send_gp, u->send., g) {
	BioBelLayer* lay = (BioBelLayer*) send_gp->prjn->layer;
	if(lay->lesion || lay->hard_clamped)	continue;
	send_gp->Send_Net(u);
      }
    }
  }
  else if(netin_fun == GAUSSIG_SEND) {
    // sender-based gaussig (and this fun not called on hard_clamped EXT layers)
    if(u->act > send_thresh) {
      BioBelCon_Group* send_gp;
      int g;
      FOR_ITR_GP(BioBelCon_Group, send_gp, u->send., g) {
	BioBelLayer* lay = (BioBelLayer*) send_gp->prjn->layer;
	if(lay->lesion || lay->hard_clamped)	continue;
	send_gp->Send_GausSig_Net(u);
      }
    }
  }
  else {			// GAUSSIG
    // receiver based is 100% correct, but slower than using send_thresh
    // don't receive from clamped layers because we already have!
    u->net = 0.0f;
    BioBelCon_Group* recv_gp;
    int g;
    FOR_ITR_GP(BioBelCon_Group, recv_gp, u->recv., g) {
      BioBelLayer* lay = (BioBelLayer*) recv_gp->prjn->from;
      if(lay->lesion || lay->hard_clamped)	continue;
      u->net += recv_gp->Compute_GausSig_Net(u);
    }
  }

  u->net += u->clmp_net;	// add in input from clamped inputs (either send or recv)
}

bool BioBelUnitSpec::Compute_SoftClamp(BioBelUnit* u, BioBelLayer* lay) {
  bool inc_gain = false;
  if(u->ext_flag & Unit::EXT) {
    if((u->ext > .5f) && (u->act < .5f))
      inc_gain = true;	// must increase gain because targ unit is < .5..

    u->net += u->ext * lay->stm_gain;
  }
  return inc_gain;
}

void BioBelUnitSpec::Compute_Act(BioBelUnit* u, BioBelLayer*, BioBelInhib* thr,
				  int cycle)
{
  // automatic scaling of offset for individual sigmoid above lower limit in indiv_sig
  if((noise_type == NETIN_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->net += noise_sched.GetVal(cycle) * noise.Gen();
  }

  // the bias weight is used to affect the level of the leak current
  float bias_val = 0.0f;
  if((bias_wts == BIAS_LEAK) && (u->bias != NULL))
    bias_val = u->bias->wt;

  float g_a = acc_f.Compute_G(u->acc_f) + acc_s_gain * acc_s.Compute_G(u->acc_s);

  // total conductances
  u->g_l = g_bar.l * (1.0f + bias_val);
  u->g_i = g_bar.i * thr->i_val.g_i;
  u->g_a = g_bar.a * g_a;
  u->g_h = g_bar.h * hyst.Compute_G(u->hyst);

  float g_e_net = MAX(u->net, act.g_e_rest);// keep at least rest potential

  float d_v_m = 
    (g_e_net * (e_rev.e - u->v_m)) + (u->g_l * (e_rev.l - u->v_m)) + 
    (u->g_i * (e_rev.i - u->v_m)) + (u->g_h * (e_rev.h - u->v_m)) +
    (u->g_a * (e_rev.a - u->v_m));

  u->v_m += vm_dt * d_v_m;

  if((noise_type == VM_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->v_m += noise_sched.GetVal(cycle) * noise.Gen();
  }

  u->v_m = vm_range.Clip(u->v_m);

  float new_act = u->v_m - act.thresh;
  if(act_fun == NOISY_XX1) {
    if(new_act <= nxx1_fun.x_range.min)
      new_act = 0.0f;
    else if(new_act >= nxx1_fun.x_range.max) {
      new_act *= act.gain;
      new_act = new_act / (new_act + 1.0f);
    }
    else {
      new_act = nxx1_fun.Eval(new_act);
      if(new_act < .01f)
	new_act = 0.0f;
    }
  }
  else {
    if(new_act < 0.0f)
      new_act = 0.0f;
    else {
      new_act *= act.gain;
      if(act_fun == X_OVER_X_1)
	new_act = new_act / (new_act + 1.0f);
    }
  }

  u->da = new_act - u->act;	// pre noise

  if((noise_type == ACT_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    new_act += noise_sched.GetVal(cycle) * noise.Gen();
  }

  u->act = act_range.Clip(new_act);

  hyst.UpdateBasis(u->hyst, u->v_m);
  acc_f.UpdateBasis(u->acc_f, u->v_m);
  acc_s.UpdateBasis(u->acc_s, u->v_m);
}

float BioBelUnitSpec::Compute_IThresh(BioBelUnit* u) {
  // don't include g_a -- just balance the net input and leak
  float non_bias_net = u->net;
  if((bias_wts == BIAS_NETIN) && (u->bias != NULL))
    non_bias_net -= (u->bias->wt / u->net_norm);

  return ((non_bias_net * (e_rev.e - act.thresh) + g_bar.l * (e_rev.l - act.thresh)) /
	  (act.thresh - e_rev.i));
} 

void BioBelUnitSpec::DecayState(BioBelUnit* u, BioBelLayer*, float decay) {
  u->v_m -= decay * (u->v_m - v_m_init.mean);
  u->act -= decay * u->act;
  u->prv_net -= decay * u->prv_net;
  u->hyst -= decay * u->hyst;
  u->acc_f -= decay * u->acc_f;
  u->acc_s -= decay * u->acc_s;

  // reset the rest of this stuff just for clarity
  u->net = u->clmp_net = 0.0f;
  u->da = 0.0f;
}

void BioBelUnitSpec::PostSettle(BioBelUnit*, BioBelLayer*, BioBelInhib*, int) {
  // do nothing, this is a place-holder
}

void BioBelUnit::Initialize() {
  spec.SetBaseType(&TA_BioBelUnitSpec);
  clmp_net = 0.0f;
  net_norm = 0.0f;
  prv_net = 0.0f;
  v_m = 0.0f;
  da = 0.0f;
  hyst = 0.0f;
  acc_f = 0.0f;
  acc_s = 0.0f;
  g_a = 0.0f;
  g_l = 0.0f;
  g_i = 0.0f;
}

void BioBelUnit::InitLinks() {
  Unit::InitLinks();
}

//////////////////////////
//  	Layer, Spec	//
//////////////////////////

void LayerLink::Initialize() {
  layer = NULL;
  link_wt = 1.0f;
}

void LayerLink::CutLinks() {
  taOBase::CutLinks();
  taBase::DelPointer((TAPtr*)&layer);
}

void LayerLink::Copy_(const LayerLink& cp) {
  taBase::SetPointer((TAPtr*)&layer, cp.layer);
  link_wt = cp.link_wt;
}
  
void BioBelDecays::Initialize() {
  event = 1.0f;
  phase = 1.0f;
  ae = 0.0f;
}

void KWTASpec::Initialize() {
  k_from = USE_PCT;
  k = 12;
  pct = .23f;
  pat_q = .5f;
}

void KWTAVals::Initialize() {
  k = 12;
  pct = .25f;
  pct_c = .75f;
}

void KWTAVals::Copy_(const KWTAVals& cp) {
  k = cp.k;
  pct = cp.pct;
  pct_c = cp.pct_c;
}

void ActInhibSpec::Initialize() {
  dt = 0.7f;
  gain = 1.9f;
}

void InhibVals::Initialize() {
  kwta = 0.0f;
  sact = 0.0f;
  ract = 0.0f;
  g_i = 0.0f;
  g_i_orig = 0.0f;
}

void InhibVals::Copy_(const InhibVals& cp) {
  kwta = cp.kwta;
  sact = cp.sact;
  ract = cp.ract;
  g_i = cp.g_i;
  g_i_orig = cp.g_i_orig;
}

void BioBelLayerSpec::Initialize() {
  min_obj_type = &TA_BioBelLayer;
  compute_i = KWTA_INHIB;
  i_kwta_pt = .25f;
  hard_clamp = true;
  clamp_range.min = .0f;
  clamp_range.max = .95f;
  stm_gain = 3.0f;
  d_stm_gain = 1.0f;
  layer_link = NO_LINK;
  layer_link_gain = 0.5f;
}

void BioBelLayerSpec::UpdateAfterEdit() {
  LayerSpec::UpdateAfterEdit();
}

void BioBelLayerSpec::InitLinks() {
  LayerSpec::InitLinks();
  taBase::Own(kwta, this);
  taBase::Own(i_sact, this);
  taBase::Own(i_ract, this);
  taBase::Own(clamp_range, this);
  taBase::Own(decay, this);
}

void BioBelLayerSpec::CutLinks() {
  LayerSpec::CutLinks();
}

//////////////////////////////////
//	at start of settling	// 
//////////////////////////////////

void BioBelLayerSpec::CompToTarg(BioBelLayer* lay) {
  if(!(lay->ext_flag & Unit::COMP))	// only process comparison
    return;
  lay->SetExtFlag(Unit::TARG);
    
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i)
    u->CompToTarg(lay);
}

void BioBelLayerSpec::Compute_HardClamp(BioBelLayer* lay, int) {
  lay->input_dist = -1;		// reset input distance to unknown

  if(!(hard_clamp && (lay->ext_flag & Unit::EXT))) {
    lay->hard_clamped = false;
    return;
  }
  lay->hard_clamped = true;	// cache this flag

  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      rugp->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs
    }
  }

  float tmp_avg = 0.0f;
  float tmp_max = clamp_range.min;
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
    u->net = u->prv_net = u->ext * stm_gain;
    u->act = u->v_m = clamp_range.Clip(u->ext);
    tmp_avg += u->act;
    tmp_max = MAX(tmp_max, u->act);
    u->da = 0.0f;
  }
  lay->acts.avg = tmp_avg / (float)lay->units.leaves;
  lay->acts.max = tmp_max;
}

int BioBelLayerSpec::Compute_InputDist(BioBelLayer* lay, int phase) {
  if(lay->ext_flag & Unit::EXT) {
    lay->input_dist = 0;
    return 0;
  }

  if(lay->input_dist == -1)	// if haven't started to compute it yet
    lay->input_dist = -2;	// this means "being computed"
  int nw_dist = 10000;		// computing using MIN, so start with large

  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, lay->projections.,i) {
    BioBelLayer* frm = (BioBelLayer*)p->from;
    if((frm == lay) || (frm->input_dist == -2))
      continue; // ignore self-projections and undetermined distances
    if(frm->input_dist < 0)
      frm->Compute_InputDist(phase);
    if(frm->input_dist == -2)	continue; // didn't get it..
    int tmp_dist = frm->input_dist + 1;
    nw_dist = MIN(nw_dist, tmp_dist); // closest distance is computed
  }
  if(nw_dist == 10000) {
    return -2;			// indicates uncomputable case
  }
  int diff = nw_dist - lay->input_dist;
  lay->input_dist = nw_dist;
  if(diff != 0)
    return -1;			// indicates change in input_dist
  return lay->input_dist;	// else return final stable value
}
 
void BioBelLayerSpec::Compute_Cascade(BioBelLayer* lay, int, int depth) {
  if(lay->input_dist == 0) return; // don't touch these ones..
  if(lay->input_dist <= depth)
    lay->hard_clamped = false;	// if within the depth, don't set hard clamp
  else
    lay->hard_clamped = true;	// use hard clamp flag to "clamp" init acts
}

void BioBelLayerSpec::Compute_ClampNet(BioBelLayer* lay, int) {
  if(lay->hard_clamped) return;

  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i)
    u->Compute_ClampNet(lay);
}

//////////////////////////////////
//	Stage 0: netinput 	//
//////////////////////////////////

void BioBelLayerSpec::Compute_Net(BioBelLayer* lay, int, int) {
  // hard-clamped input layers are already computed in the clmp_net value
  if(lay->hard_clamped) return;

  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i)
    u->Compute_Net(lay);
}

//////////////////////////////////////////////////////////////////
//	Stage 1: netinput averages and clamping (if necc)	//
//////////////////////////////////////////////////////////////////

void BioBelLayerSpec::Compute_Clamp_NetAvg(BioBelLayer* lay, int cycle, int phase) {
  if(lay->hard_clamped) return;

  if(lay->ext_flag & Unit::EXT)
    Compute_SoftClamp(lay, cycle, phase);

  lay->netin.avg = 0.0f;
  lay->netin.max = 0.0f;

  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      Compute_NetAvg(lay, rugp, (BioBelInhib*)rugp, cycle, phase);
      // keep maximums for linking purposes
      lay->netin.avg = MAX(lay->netin.avg, rugp->netin.avg);
      lay->netin.max = MAX(lay->netin.max, rugp->netin.max);
    }
  }
  else {
    Compute_NetAvg(lay, &(lay->units), (BioBelInhib*)lay, cycle, phase);
  }

  Compute_sAct(lay, cycle, phase);
}

void BioBelLayerSpec::Compute_NetAvg(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr,
				    int, int)
{
  thr->netin.avg = 0.0f;
  thr->netin.max = -MAXFLOAT;
  int n_in_avg = 0;
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, ug->, i) {
    u->Compute_NetAvg(lay, thr);
    thr->netin.avg += u->net;
    thr->netin.max = MAX(thr->netin.max, u->net);
    n_in_avg++;
  }

  if(n_in_avg > 0)
    thr->netin.avg /= (float)n_in_avg;	// turn it into an average
}

void BioBelLayerSpec::Compute_sAct(BioBelLayer* lay, int, int) {
  lay->sact.avg = 0.0f;
  lay->sact.max = -MAXFLOAT;
  float n_avg = 0.0f;
  Projection* prjn;
  taLeafItr i;
  FOR_ITR_EL(Projection, prjn, lay->projections., i) {
    BioBelConSpec* cs = (BioBelConSpec*) prjn->con_spec.spec;
    BioBelLayer* frm = (BioBelLayer*) prjn->from;
    lay->sact.avg += cs->wt_gain * (float)frm->units.leaves * frm->acts.avg;
    n_avg += (float)frm->units.leaves;
    lay->sact.max = MAX(lay->sact.max, cs->wt_gain * frm->acts.max);
  }
  lay->sact.avg /= n_avg;	// keep sact.avg normalized in same way netin is
}

void BioBelLayerSpec::Compute_SoftClamp(BioBelLayer* lay, int cycle, int) {
  BioBelUnit* u;
  taLeafItr i;
  bool inc_gain = false;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
    if(!inc_gain)
      inc_gain = u->Compute_SoftClamp(lay);
    else
      u->Compute_SoftClamp(lay);
  }
  if(inc_gain && (cycle > 1)) // only increment after it has a chance to activate
    lay->stm_gain += d_stm_gain;
}


//////////////////////////////////////////////////////////////////
//	Stage 2: inhibition					//
//////////////////////////////////////////////////////////////////

void BioBelLayerSpec::InitInhib(BioBelLayer*) {
}

void BioBelLayerSpec::Compute_Inhib(BioBelLayer* lay, int, int) {
  if(lay->hard_clamped)	return;	// say no more..

  if(lay->units.gp.size > 0) {
    lay->Inhib_SetVals(0.0f);
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Inhib_impl(lay, rugp, (BioBelInhib*)rugp);
      // i_val keeps the averages...
      lay->i_val.g_i += rugp->i_val.g_i;
      // i_val_orig keeps the maximums for linking purposes..
      lay->i_val.g_i_orig = MAX(lay->i_val.g_i_orig, rugp->i_val.g_i);
    }
    lay->i_val.g_i /= (float)g;
 }
  else {
    Compute_Inhib_impl(lay, &(lay->units), (BioBelInhib*)lay);
  }
}

void BioBelLayerSpec::Compute_Inhib_impl(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr) {
  // for the time being, always compute kwta just to have it around..
  Compute_Inhib_kWTA(lay, ug, thr);
  if(compute_i != KWTA_INHIB) {	// always compute these, just for kicks..
    Compute_Inhib_sAct(lay, ug, thr);
    Compute_Inhib_rAct(lay, ug, thr);
  }

  switch(compute_i) {
  case KWTA_INHIB:
    thr->i_val.g_i = thr->i_val.kwta;
    break;
  case SACT_INHIB:
    thr->i_val.g_i = thr->i_val.sact;
    break;
  case RACT_INHIB:
    thr->i_val.g_i = thr->i_val.ract;
    break;
  case SRACT_INHIB:
    thr->i_val.g_i = thr->i_val.ract + thr->i_val.sact;
    break;
  }

  thr->i_val.g_i_orig = thr->i_val.g_i;	// retain original values..
}

// inhib between thresholds for k and k+1th units
void BioBelLayerSpec::Compute_Inhib_kWTA(BioBelLayer*, Unit_Group* ug, BioBelInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }
  
  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one

  float net_k1 = MAXFLOAT;
  int k1_idx = 0;
  BioBelUnit* u;
  taLeafItr i;
  int j;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    j = 0;
    for(u = (BioBelUnit*)ug->FirstEl(i); u && (j < k_plus_1);
	u = (BioBelUnit*)ug->NextEl(i), j++)
    {
      thr->active_buf.Add(u);		// add unit to the list
      if(u->net < net_k1) {
	net_k1 = u->net;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (BioBelUnit*)ug->NextEl(i)) {
      if(u->net <=  net_k1) {	// not bigger than smallest one in sort buffer
	thr->inact_buf.Add(u);
	continue;
      }
      thr->inact_buf.Add(thr->active_buf.FastEl(k1_idx)); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->net;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf.FastEl(j)->net;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf.FastEl(j);
      if(u->net <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf.FastEl(k1_idx));	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->net;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }

  // active_buf now has k+1 most active units, get the next-highest one
  int  k_idx = -1;
  float net_k = MAXFLOAT;
  for(j=0; j < k_plus_1; j++) {
    float tmp = thr->active_buf.FastEl(j)->net;
    if((tmp < net_k) && (tmp != net_k1)) {
      net_k = tmp;		k_idx = j;
    }
  }
  if(k_idx == -1) {		// we didn't find the next one
    k_idx = k1_idx;
    net_k = net_k1;
  }

  BioBelUnit* k1_u = (BioBelUnit*)thr->active_buf.FastEl(k1_idx);
  BioBelUnit* k_u = (BioBelUnit*)thr->active_buf.FastEl(k_idx);

  float k1_i = k1_u->Compute_IThresh();
  float k_i = k_u->Compute_IThresh();

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
    
  // make sure its above 0
  thr->i_val.kwta = MAX(thr->i_val.kwta, 0.0f);
}

void BioBelLayerSpec::Compute_Inhib_sAct(BioBelLayer* lay, Unit_Group*, BioBelInhib* thr) {
  float new_sact = i_sact.gain * lay->sact.avg;
  thr->i_val.sact += i_sact.dt * (new_sact - thr->i_val.sact);
}

void BioBelLayerSpec::Compute_Inhib_rAct(BioBelLayer* lay, Unit_Group*, BioBelInhib* thr) {
  float new_ract = i_ract.gain * lay->acts.avg;
  thr->i_val.ract += i_ract.dt * (new_ract - thr->i_val.ract);
}

void BioBelLayerSpec::Compute_LinkInhib(BioBelLayer* lay, Unit_Group*, BioBelInhib* thr,
					int)
{
  if(lay->layer_links.size > 0)	{
    float max_i = 0.0f;
    int i;
    LayerLink* llk;
    for(i=0; i<lay->layer_links.size; i++) {
      llk = (LayerLink*)lay->layer_links.FastEl(i);
      if((llk->layer == NULL) || llk->layer->lesion ||
         (llk->layer->i_val.g_i_orig <= lay->i_val.g_i_orig))
        continue;
      float i_del = llk->link_wt * layer_link_gain * 
        (llk->layer->i_val.g_i_orig - lay->i_val.g_i_orig);
      max_i = MAX(max_i, i_del);
    }
    lay->i_val.g_i += max_i;
  }
  else if(lay->units.gp.size > 0) {
    // lay->i_val_orig has the maximums
    float max_i = layer_link_gain * (lay->i_val.g_i_orig - thr->i_val.g_i_orig);
    thr->i_val.g_i += max_i;
    // update averages too
    lay->i_val.g_i += max_i / (float)lay->units.gp.size;
  }
}

void BioBelLayerSpec::Compute_Active_K(BioBelLayer* lay) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Active_K_impl(lay, rugp, (BioBelInhib*)rugp);
      // make sure there is a layer-level value..
      lay->kwta.k = kwta.k;
      if((kwta.k_from != KWTASpec::USE_PCT) && (lay->n_units > 0))
	lay->kwta.pct = (float)kwta.k / (float)lay->n_units;
      lay->kwta.pct_c = 1.0f - lay->kwta.pct;
    }
  }
  else {
    Compute_Active_K_impl(lay, &(lay->units), (BioBelInhib*)lay);
  }
}

void BioBelLayerSpec::Compute_Active_K_impl(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr) {
  int new_k = 0;
  if(kwta.k_from == KWTASpec::USE_PCT)
    new_k = (int)((float)ug->leaves * kwta.pct);
  else if((kwta.k_from == KWTASpec::USE_PAT_K) && ((lay->ext_flag & Unit::TARG) ||
				       (lay->ext_flag & Unit::EXT)))
    new_k = Compute_Pat_K(lay, ug, thr);
  else
    new_k = kwta.k;

  new_k = MIN(ug->leaves - 1, new_k);
  new_k = MAX(1, new_k);

  if(thr->kwta.k != new_k)
    thr->Inhib_ResetSortBuf();
  thr->kwta.k = new_k;
  if(kwta.k_from == KWTASpec::USE_PCT)
    thr->kwta.pct = kwta.pct;
  else if(ug->leaves > 0)
    thr->kwta.pct = (float)new_k / (float)ug->leaves;
  thr->kwta.pct_c = 1.0f - thr->kwta.pct;
}

int BioBelLayerSpec::Compute_Pat_K(BioBelLayer*, Unit_Group* ug, BioBelInhib*) {
  int cutoff_k = 0;
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, ug->, i) {
    // use either EXT or TARG information...
    if(u->ext_flag & Unit::EXT) {
      if(u->ext >= kwta.pat_q)
	cutoff_k++;
    }
    else if(u->ext_flag & Unit::TARG) {
      if(u->targ >= kwta.pat_q)
	cutoff_k++;
    }
  }
  return cutoff_k;
}

//////////////////////////////////////////
//	Stage 3: the final activation 	//
//////////////////////////////////////////

void BioBelLayerSpec::Compute_Act(BioBelLayer* lay, int cycle, int phase) {
  if((cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing

  if(lay->units.gp.size > 0) {
    lay->acts.max = 0.0f; 
    float lay_acts_avg = 0.0f;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Act_impl(lay, rugp, (BioBelInhib*)rugp, cycle, phase);
      lay_acts_avg += rugp->acts.avg * (float)rugp->size; // weight by no of units
      lay->acts.max = MAX(lay->acts.max, rugp->acts.max);
    }
    lay->acts.avg = lay_acts_avg / (float)lay->units.leaves;
  }
  else {
    Compute_Act_impl(lay, &(lay->units), (BioBelInhib*)lay, cycle, phase);
  }
}

void BioBelLayerSpec::Compute_Act_impl(BioBelLayer* lay, Unit_Group* ug,
				      BioBelInhib* thr, int cycle, int)
{
  if(layer_link == LINK_INHIB)
    Compute_LinkInhib(lay, ug, thr, cycle);

  float tmp_max = 0.0f;
  float tmp_avg = 0.0f;
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, ug->, i) {
    u->Compute_Act(lay,thr,cycle);
    tmp_max = MAX(tmp_max, u->act);
    tmp_avg += u->act;
  }
  thr->acts.max = tmp_max;
  if(ug->size > 0)
    thr->acts.avg = tmp_avg / (float)ug->size;
  else
    thr->acts.avg = 0.0f;
}

//////////////////////////////////////////
//	Decaying, transitions	 	//
//////////////////////////////////////////

void BioBelLayerSpec::DecayEvent(BioBelLayer* lay) {
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
    u->DecayState(lay, decay.event);
  }
}
  
void BioBelLayerSpec::DecayPhase(BioBelLayer* lay) {
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
    u->DecayState(lay, decay.phase);
  }
}

void BioBelLayerSpec::DecayAE(BioBelLayer* lay) {
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
    u->DecayState(lay, decay.ae);
  }
}
  
void BioBelLayerSpec::PostSettle(BioBelLayer* lay, int phase) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);

      BioBelUnit* u;
      taLeafItr i;
      FOR_ITR_EL(BioBelUnit, u, rugp->, i)
	u->PostSettle(lay, (BioBelInhib*)rugp, phase);
    }
  }
  else {
    BioBelUnit* u;
    taLeafItr i;
    FOR_ITR_EL(BioBelUnit, u, lay->units., i)
      u->PostSettle(lay, (BioBelInhib*)lay, phase);
  }
}

  

//////////////////////////
// 	BioBelLayer	//
//////////////////////////

  
void AvgMaxVals::Initialize() {
  avg = max = 0.0f;
}

void BioBelInhib::Inhib_Initialize() {
  kwta.k = 1;
  kwta.pct = .25;
  kwta.pct_c = .75;
  i_val.Initialize();
}

void BioBelInhib::Inhib_InitState(BioBelLayerSpec*) {
  i_val.Initialize();
  netin.Initialize();
  acts.Initialize();
}

void BioBelLayer::Initialize() {
  Inhib_Initialize();
  stm_gain = 3.0f;
  hard_clamped = false;
  input_dist = -1;		// indicates unknown
  units.el_typ = &TA_BioBelUnit;
  units.gp.SetBaseType(&TA_BioBelUnit_Group);
  unit_spec.SetBaseType(&TA_BioBelUnitSpec);
  layer_links.SetBaseType(&TA_LayerLink);
}  

void BioBelLayer::InitLinks() {
  Layer::InitLinks();
  taBase::Own(netin, this);
  taBase::Own(acts, this);

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);

  taBase::Own(sact, this);
  taBase::Own(layer_links, this);
  spec.SetDefaultSpec(this);
  units.gp.SetBaseType(&TA_BioBelUnit_Group);
}

void BioBelLayer::CutLinks() {
  Layer::CutLinks();
  spec.CutLinks();
  layer_links.Reset();		// get rid of the links...
}

void BioBelLayer::ResetSortBuf() {
  Inhib_ResetSortBuf();		// reset sort buf after any edit..
  if(units.gp.size > 0) {
    int g;
    for(g=0; g<units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)units.gp.FastEl(g);
      rugp->Inhib_ResetSortBuf();
    }
  }
}

void BioBelLayer::UpdateAfterEdit() {
  Layer::UpdateAfterEdit();
  ResetSortBuf();
}

bool BioBelLayer::SetLayerSpec(LayerSpec* sp) {
  if(sp == NULL)	return false;
  if(sp->CheckObjectType(this))
    spec.SetSpec((BioBelLayerSpec*)sp);
  else
    return false;
  return true;
} 

void BioBelLayer::Build() {
  ResetSortBuf();
  Layer::Build();
}

void BioBelLayer::ReConnect_Load() {
  Layer::ReConnect_Load();
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, units., i)
    u->ReConnect_Load(this);
}

void BioBelLayer::InitState() {
  ext_flag = Unit::NO_EXTERNAL;
  stm_gain = 3.0f;
  hard_clamped = false;
  sact.Initialize();
  input_dist = -1;		// indicates unknown
  if(spec.spec != NULL)
    stm_gain = spec->stm_gain;
  else
    stm_gain = 3.0f;
  ResetSortBuf();
  Compute_Active_K();		// need kwta.pct for init
  Inhib_InitState(spec.spec);
  if(units.gp.size > 0) {
    int g;
    for(g=0; g<units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)units.gp.FastEl(g);
      rugp->Inhib_InitState(spec.spec);
    }
  }
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, units., i)
    u->InitState(this);
}

void BioBelLayer::InitWtState() {
  Compute_Active_K();		// need kwta.pct for init
  Layer::InitWtState();
  if(spec.spec != NULL)
    spec.spec->InitInhib(this);	// initialize inhibition at start..
}

void BioBelUnit_Group::Initialize() {
  Inhib_Initialize();
}

void BioBelUnit_Group::InitLinks() {
  Unit_Group::InitLinks();
  taBase::Own(netin, this);
  taBase::Own(acts, this);

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);
}


