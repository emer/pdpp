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
  wt_scale = 1.0f;
  wt_gain = 6.0f;
  wt_off = 1.25f;

  wt_sig_fun.x_range.min = 0.0f;
  wt_sig_fun.x_range.max = 1.0f;
  wt_sig_fun.res = .001f;
  wt_sig_fun.x_range.UpdateAfterEdit();
}

void BioBelConSpec::InitLinks() {
  ConSpec::InitLinks();
  taBase::Own(wt_sig_fun, this);
  CreateWtSigFun();
}

void BioBelConSpec::UpdateAfterEdit() {
  ConSpec::UpdateAfterEdit();
  CreateWtSigFun();
}

void BioBelConSpec::CreateWtSigFun() {
  wt_sig_fun.AllocForRange();
  int i;
  for(i=0; i<wt_sig_fun.size; i++) {
    float w = wt_sig_fun.x_range.min + ((float)i * wt_sig_fun.res);
    wt_sig_fun.FastEl(i) = WeightSigFun(w, wt_gain, wt_off);
  }
}

void BioBelConSpec::ApplySymmetry(Con_Group* cg, Unit* ru) {
  if(!wt_limits.sym) return;
  Connection* rc, *su_rc;
  int i;
  for(i=0; i<cg->size;i++) {
    rc = cg->Cn(i);
    su_rc = cg->FindRecipRecvCon(cg->Un(i), ru);
    if(su_rc != NULL) {
      su_rc->wt = rc->wt;	// set other's weight to ours (otherwise no random..)
      ((BioBelCon*)su_rc)->wt_lin = ((BioBelCon*)rc)->wt_lin;
      // get the linear weights too!
    }
  }
}

void BioBelCon_Group::Initialize() {
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

void VChanSpec::Initialize() {
  on = false;
  inc = .01f;
  dec = .01f;
  a_thr = .5f;
  d_thr = .1f;
}

void VChanBasis::Initialize() {
  hyst = 0.0f;
  acc = 0.0f;
  hyst_on = false;
  acc_on = false;
}

void VChanBasis::Copy_(const VChanBasis& cp) {
  hyst = cp.hyst;
  acc = cp.acc;
  hyst_on = cp.hyst_on;
  acc_on = cp.acc_on;
}

void ActFunSpec::Initialize() {
  thresh = .3f;
  gain = 600.0f;
  v_m_rest = .15f;
  g_e_rest = 0.0f;		// needs to be computed
}

void AdaptThreshSpec::Initialize() {
  dt = .005f;
  min = .01f;
  max = .35f;
  dthr = .0f;
  err = .0f;
  rnd = false;
  act_thr = .25f;
}

void OptThreshSpec::Initialize() {
  send = .1f;
  learn = 0.0f;
}

void BioBelUnitSpec::Initialize() {
  min_obj_type = &TA_BioBelUnit;
  act_fun = NOISY_XX1;

  vm_range.max = 1.0f;
  vm_range.min = 0.0f;
  v_m_init.type = Random::UNIFORM;
  v_m_init.mean = .15f;
  v_m_init.var = 0.0f;

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
  e_rev.h = 1.0f;
  e_rev.a = 0.0f;

  noise_type = NO_NOISE;
  noise.type = Random::NONE;
  noise.var = .005f;

  noise_conv.x_range.min = -.05f;
  noise_conv.x_range.max = .05f;
  noise_conv.res = .001f;
  noise_conv.x_range.UpdateAfterEdit();

  nxx1_fun.x_range.min = -.03f;
  nxx1_fun.x_range.max = .20f;
  nxx1_fun.res = .001f;
  nxx1_fun.x_range.UpdateAfterEdit();
}

void BioBelUnitSpec::InitLinks() {
  UnitSpec::InitLinks();
  taBase::Own(vm_range, this);
  taBase::Own(act, this);
  taBase::Own(adapt_thr, this);
  taBase::Own(g_bar, this);
  taBase::Own(e_rev, this);
  taBase::Own(hyst, this);
  taBase::Own(acc, this);
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

void BioBelUnitSpec::InitWtState(Unit* u) {
  UnitSpec::InitWtState(u);
  BioBelUnit* bu = (BioBelUnit*)u;
  bu->act_avg = adapt_thr.min + .5f * (adapt_thr.max - adapt_thr.min);
  bu->thr = act.thresh;
}

void BioBelUnitSpec::InitState(BioBelUnit* ru, BioBelLayer*) {
  UnitSpec::InitState(ru);
  ru->clmp_net = 0.0f;
  ru->net_norm = 0.0f;
  ru->prv_net = 0.0f;
  ru->da = 0.0f;
  ru->vcb.hyst = 0.0f;
  ru->vcb.acc = 0.0f;
  ru->vcb.hyst_on = false;
  ru->vcb.acc_on = false;
  ru->gc.l = 0.0f;
  ru->gc.i = 0.0f;
  ru->gc.h = 0.0f;
  ru->gc.a = 0.0f;
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

void BioBelUnitSpec::Compute_ClampNet(BioBelUnit* u, BioBelLayer*) {
  // this is all receiver-based and done only at beginning
  u->clmp_net = 0.0f;
  u->net_norm = 0.0f;
  BioBelCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(BioBelCon_Group, recv_gp, u->recv., g) {
    BioBelLayer* lay = (BioBelLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
    u->net_norm += (float)recv_gp->size; // compute normalization value
    if(!lay->hard_clamped)	continue;
    u->clmp_net += recv_gp->Compute_Net(u);
  }
  // add the bias weight into the netinput
  if(u->bias != NULL) {
    u->clmp_net += u->bias->wt;
    u->net_norm += 1.0f;
  }
}

void BioBelUnitSpec::Compute_Net(BioBelUnit* u, BioBelLayer*) {
  // sigmoid is sender-based (and this fun not called on hard_clamped EXT layers)
  if(u->act > opt_thresh.send) {
    BioBelCon_Group* send_gp;
    int g;
    FOR_ITR_GP(BioBelCon_Group, send_gp, u->send., g) {
      BioBelLayer* lay = (BioBelLayer*) send_gp->prjn->layer;
      if(lay->lesion || lay->hard_clamped)	continue;
      send_gp->Send_Net(u);
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

  float	g_h = hyst.Compute_G(u->vcb.hyst, u->vcb.hyst_on);
  float g_a = acc.Compute_G(u->vcb.acc, u->vcb.acc_on);

  // total conductances
  u->gc.l = g_bar.l;
  u->gc.i = g_bar.i * thr->i_val.g_i;
  u->gc.h = g_bar.h * g_h;
  u->gc.a = g_bar.a * g_a;

  float g_e_net = MAX(u->net, act.g_e_rest);// keep at least rest potential

  float d_v_m = 
    (g_e_net * (e_rev.e - u->v_m)) + (u->gc.l * (e_rev.l - u->v_m)) + 
    (u->gc.i * (e_rev.i - u->v_m)) + (u->gc.h * (e_rev.h - u->v_m)) +
    (u->gc.a * (e_rev.a - u->v_m));

  u->v_m += vm_dt * d_v_m;

  if((noise_type == VM_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->v_m += noise_sched.GetVal(cycle) * noise.Gen();
  }

  u->v_m = vm_range.Clip(u->v_m);

  float new_act = u->v_m - u->thr;
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

  // fast-time scale updated during settling
  hyst.UpdateBasis(u->vcb.hyst, u->vcb.hyst_on, u->act);
  acc.UpdateBasis(u->vcb.acc, u->vcb.acc_on, u->act);
}

float BioBelUnitSpec::Compute_IThresh(BioBelUnit* u) {
  float non_bias_net = u->net;
  if(u->bias != NULL)
    non_bias_net -= (u->bias->wt / u->net_norm);

  // including the ga and gh terms
  return ((non_bias_net * (e_rev.e - act.thresh) + g_bar.l * (e_rev.l - act.thresh)
	   + u->gc.a * (e_rev.a - act.thresh) + u->gc.h * (e_rev.h - act.thresh)) /
	  (act.thresh - e_rev.i));

  // not including the ga and gh terms -- what about using real threshold??
//   return ((non_bias_net * (e_rev.e - act.thresh) + g_bar.l * (e_rev.l - act.thresh)) /
// 	  (act.thresh - e_rev.i));
} 

void BioBelUnitSpec::DecayState(BioBelUnit* u, BioBelLayer*, float decay) {
  u->v_m -= decay * (u->v_m - v_m_init.mean);
  u->act -= decay * u->act;
  u->prv_net -= decay * u->prv_net;
  u->vcb.hyst -= decay * u->vcb.hyst;
  u->vcb.acc -= decay * u->vcb.acc;

  // reset the rest of this stuff just for clarity
  u->net = u->clmp_net = 0.0f;
  u->da = 0.0f;
}

bool BioBelUnitSpec::AdaptThreshTest(BioBelUnit* u, BioBelLayer*, BioBelInhib*, int) {
  u->act_avg += adapt_thr.dt * (u->act - u->act_avg);
  if(((u->act_avg < adapt_thr.min) && (u->act < adapt_thr.act_thr)) ||
     ((u->act_avg > adapt_thr.max) && (u->act > adapt_thr.act_thr)))
    return true;		// need to adapt threshold
  else				 // decay the threshold back towards normal
    u->thr += adapt_thr.dthr * (act.thresh - u->thr);
  return false;
}

void BioBelUnitSpec::AdaptThresh(BioBelUnit* u, BioBelLayer*, BioBelInhib*, int) {
  if(u->act_avg < adapt_thr.min)
    u->thr -= adapt_thr.dthr;
  else if(u->act_avg > adapt_thr.max)
    u->thr += adapt_thr.dthr;
}

void BioBelUnitSpec::PostSettle(BioBelUnit*, BioBelLayer*, BioBelInhib*, int)
{
  // nothing happens here: just a place-holder
}


//////////////////////////
//  	Unit 		//
//////////////////////////


void BioBelUnitChans::Initialize() {
  l = i = h = a = 0.0f;
}

void BioBelUnitChans::Copy_(const BioBelUnitChans& cp) {
  l = cp.l;
  i = cp.i;
  h = cp.h;
  a = cp.a;
}

void BioBelUnit::Initialize() {
  spec.SetBaseType(&TA_BioBelUnitSpec);
  clmp_net = 0.0f;
  net_norm = 0.0f;
  prv_net = 0.0f;
  v_m = 0.0f;
  thr = .30f;
  da = 0.0f;
  act_avg = 0.0f;
}

void BioBelUnit::InitLinks() {
  Unit::InitLinks();
  taBase::Own(vcb, this);
  taBase::Own(gc, this);
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
  gap_q = 1.0f;
  adth_pct = .5f;
}

void KWTAVals::Initialize() {
  k = 12;
  act_k = 12;
  pct = .25f;
  pct_c = .75f;
  Ok_gap = 0.0f;
  max_gap = 0.0f;
  adth_k = 1;
}

void KWTAVals::Copy_(const KWTAVals& cp) {
  k = cp.k;
  act_k = cp.act_k;
  pct = cp.pct;
  pct_c = cp.pct_c;
  Ok_gap = cp.Ok_gap;
  max_gap = cp.max_gap;
  adth_k = cp.adth_k;
}

void KWTAVals::Compute_Pct(int n_units) {
  if(n_units > 0)
    pct = (float)act_k / (float)n_units;
  else
    pct = 0.0f;
  pct_c = 1.0f - pct;
}

void KWTAVals::Compute_AdthK(float adth_pct) {
  adth_k = (int)(adth_pct * (float)k);
  adth_k = MAX(adth_k, 1);
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
  inhib_group = ENTIRE_LAYER;
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

  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
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
    lay->sact.avg += cs->wt_scale * (float)frm->units.leaves * frm->acts.avg;
    n_avg += (float)frm->units.leaves;
    lay->sact.max = MAX(lay->sact.max, cs->wt_scale * frm->acts.max);
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

  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
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
  if(compute_i != KWTA_INHIB) {
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

// inhib between thresholds for k and k+1th units, with gap!
void BioBelLayerSpec::Compute_Inhib_kWTA(BioBelLayer*, Unit_Group* ug, BioBelInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }
  
  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one

  BioBelUnit* u;
  taLeafItr i;
  int j;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    j = 0;
    for(u = (BioBelUnit*)ug->FirstEl(i); u && (j < k_plus_1); u = (BioBelUnit*)ug->NextEl(i), j++)
      thr->active_buf.AddEl_((void*)u);		// add unit to the list
    thr->active_buf.SortByNet(); 	// sort this list by net-input
    thr->inact_buf.size = 0;
    // go through remainder of list, adding 
    for(; u; u = (BioBelUnit*)ug->NextEl(i)) {
      if(u->net <= thr->active_buf.Peek()->net) { // not bigger than smallest one in sort buffer
	thr->inact_buf.AddEl_((void*)u);
	continue;
      }
      int nw_idx = thr->active_buf.FindNewNetPos(u->net);
      thr->active_buf.FastInsertLink((void*)u, nw_idx);
      thr->inact_buf.AddEl_(thr->active_buf.Peek_()); // lowest one gets knocked off
      thr->active_buf.size = k_plus_1; // restore size..
    }
  }
  else {			// starting with existing buf
    thr->active_buf.SortByNet(); // always need to resort buf because it could be diff now
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf.FastEl(j);
      if(u->net <= thr->active_buf.Peek()->net)		// not bigger than smallest one in sort buffer
	continue;
      int nw_idx = thr->active_buf.FindNewNetPos(u->net);
      thr->active_buf.FastInsertLink((void*)u, nw_idx);
      thr->inact_buf.Replace_(j, thr->active_buf.Peek_()); // lowest one gets put on inact list
      thr->active_buf.size = k_plus_1; // restore size..
    }
  }
  int max_gap_i = 0;
  float max_gap = 0.0f;
  for(j=0; j < k_plus_1-1; j++) {
    float tmp = (thr->active_buf.FastEl(j)->net - thr->active_buf.FastEl(j+1)->net);
    if(tmp > max_gap) {
      max_gap_i = j;	max_gap = tmp;
    }
  }

  float Ok_gap = thr->active_buf.FastEl(0)->net - thr->active_buf.Peek()->net;

  thr->kwta.Ok_gap = Ok_gap;
  if(Ok_gap != 0.0f)
    thr->kwta.max_gap = max_gap / Ok_gap;
  else
    thr->kwta.max_gap = 0.0f;
    
  if(thr->kwta.max_gap > kwta.gap_q)
    thr->kwta.act_k = max_gap_i + 1;	// move to position of the maximum gap value
  else
    thr->kwta.act_k = thr->kwta.k;
  // pct should always reflect the actual k value
  thr->kwta.pct = (float)thr->kwta.act_k / (float)ug->leaves;
  thr->kwta.pct_c = 1.0f - thr->kwta.pct;
      
  BioBelUnit* k1_u = thr->active_buf.FastEl(thr->kwta.act_k);
  BioBelUnit* k_u = thr->active_buf.FastEl(thr->kwta.act_k - 1);

  float k1_i = k1_u->Compute_IThresh();
  float k_i = k_u->Compute_IThresh();

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
}

void BioBelSort::FastInsertLink(void* it, int where) {
  if((where >= size) || (where < 0)) {
    AddEl_(it);
    return;
  }
  if(size > 0)
    AddEl_(NULL);
  int i;
  for(i=size-1; i>where; i--)
    el[i] = el[i-1];
  el[where] = it;
}

int BioBelSort::FindNewNetPos(float nw_net) {
  int st = 0;
  int n = size;
  while(true) {
    if(n <= 2) {
      if(FastEl(st)->net < nw_net)
	return st;
      else if((st+1 < size) && (FastEl(st+1)->net < nw_net))
	return st + 1;
      else if((st+2 < size) && (FastEl(st+2)->net < nw_net))
	return st + 2;
      return st;
    }
    int n2 = n / 2;
    if(FastEl(st + n2)->net < nw_net)
      n = n2;			// search in upper 1/2 of list
    else {
      st += n2; n -= n2;	// search in lower 1/2 of list
    }
  }
}

void BioBelSort::SortByNet() {
  if(size <= 1) return;
  // lets do a heap sort since it requires no secondary storage
  int n = size;
  int l,j,ir,i;
  void* tmp;

  l = (n >> 1) + 1;
  ir = n;
  for(;;){
    if(l>1)
      tmp = el[--l -1]; // tmp = ra[--l]
    else {
      tmp = el[ir-1]; // tmp = ra[ir]
      el[ir-1] = el[0]; // ra[ir] = ra[1]
      if(--ir == 1) {
	el[0] = tmp; // ra[1]=tmp
	return;
      }
    }
    i=l;
    j=l << 1;
    while(j<= ir) {
      if(j<ir && (NetCompare((BioBelUnit*)el[j-1], (BioBelUnit*)el[j]) == -1)) j++;
      if(NetCompare((BioBelUnit*)tmp, (BioBelUnit*)el[j-1]) == -1) { // tmp < ra[j]
	el[i-1] = el[j-1]; // ra[i]=ra[j];
	j += (i=j);
      }
      else j = ir+1;
    }
    el[i-1] = tmp; // ra[i] = tmp;
  }
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
  else if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
    // lay->i_val_orig has the maximums
    float max_i = layer_link_gain * (lay->i_val.g_i_orig - thr->i_val.g_i_orig);
    thr->i_val.g_i += max_i;
    // update averages too
    lay->i_val.g_i += max_i / (float)lay->units.gp.size;
  }
}

void BioBelLayerSpec::Compute_Active_K(BioBelLayer* lay) {
  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Active_K_impl(lay, rugp, (BioBelInhib*)rugp);
      // make sure there is a layer-level value..
      lay->kwta.k = lay->kwta.act_k = kwta.k;
      lay->kwta.Compute_Pct(lay->n_units);
      lay->kwta.Compute_AdthK(kwta.adth_pct);
    }
  }
  else {
    Compute_Active_K_impl(lay, &(lay->units), (BioBelInhib*)lay);
  }
}

void BioBelLayerSpec::Compute_Active_K_impl(BioBelLayer* lay, Unit_Group* ug, BioBelInhib* thr) {
  int new_k = 0;
  if(kwta.k_from == KWTASpec::USE_PCT)
    new_k = (int)(kwta.pct * (float)ug->leaves);
  else if((kwta.k_from == KWTASpec::USE_PAT_K) && 
	  ((lay->ext_flag & (Unit::TARG | Unit::COMP)) || (lay->ext_flag & Unit::EXT)))
    new_k = Compute_Pat_K(lay, ug, thr);
  else
    new_k = kwta.k;

  new_k = MIN(ug->leaves - 1, new_k);
  new_k = MAX(1, new_k);

  if(thr->kwta.k != new_k)
    thr->Inhib_ResetSortBuf();

  thr->kwta.k = thr->kwta.act_k = new_k;
  thr->kwta.Compute_Pct(ug->leaves);
  thr->kwta.Compute_AdthK(kwta.adth_pct);
}

int BioBelLayerSpec::Compute_Pat_K(BioBelLayer*, Unit_Group* ug, BioBelInhib*) {
  int pat_k = 0;
  BioBelUnit* u;
  taLeafItr i;
  FOR_ITR_EL(BioBelUnit, u, ug->, i) {
    // use either EXT or TARG information...
    if(u->ext_flag & Unit::EXT) {
      if(u->ext >= kwta.pat_q)
	pat_k++;
    }
    else if(u->ext_flag & (Unit::TARG | Unit::COMP)) {
      if(u->targ >= kwta.pat_q)
	pat_k++;
    }
  }
  return pat_k;
}

//////////////////////////////////////////
//	Stage 3: the final activation 	//
//////////////////////////////////////////

void BioBelLayerSpec::Compute_Act(BioBelLayer* lay, int cycle, int phase) {
  if((cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing

  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
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
  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
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
  // don't do this here: it is done in leabra only in the plus phase..
  // AdaptThreshold(lay, phase);
}

void BioBelLayerSpec::AdaptThreshold(BioBelLayer* lay, int phase) {
  if(lay->ext_flag & (Unit::EXT | Unit::TARG))
    return;			// don't adapt thresholds for clamped layers
  if((inhib_group == UNIT_GROUPS) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      BioBelUnit_Group* rugp = (BioBelUnit_Group*)lay->units.gp.FastEl(g);
      adapt_thr_list.size = 0;

      BioBelUnit* u;
      taLeafItr ui;
      BioBelUnitSpec* us = (BioBelUnitSpec*)((BioBelUnit*)rugp->Leaf(0))->spec.spec;
      if(us->adapt_thr.rnd) {
	FOR_ITR_EL(BioBelUnit, u, rugp->, ui) {
	  if(u->AdaptThreshTest(lay, (BioBelInhib*)rugp, phase))
	    adapt_thr_list.AddEl_((void*)u);
	}
	if(adapt_thr_list.size > rugp->kwta.adth_k)
	  adapt_thr_list.Permute();
	int mxi = MIN(adapt_thr_list.size, rugp->kwta.adth_k);
	int i;
	for(i=0;i<mxi;i++)
	  adapt_thr_list.FastEl(i)->AdaptThresh(lay, (BioBelInhib*)rugp, phase);
      }
      else {
	FOR_ITR_EL(BioBelUnit, u, rugp->, ui) {
	  if(u->AdaptThreshTest(lay, (BioBelInhib*)rugp, phase))
	    u->AdaptThresh(lay, (BioBelInhib*)rugp, phase);
	}
      }
    }
  }
  else {
    adapt_thr_list.size = 0;

    BioBelUnit* u;
    taLeafItr ui;
    BioBelUnitSpec* us = (BioBelUnitSpec*)((BioBelUnit*)lay->units.Leaf(0))->spec.spec;
    if(us->adapt_thr.rnd) {
      FOR_ITR_EL(BioBelUnit, u, lay->units., ui) {
	if(u->AdaptThreshTest(lay, (BioBelInhib*)lay, phase))
	  adapt_thr_list.AddEl_((void*)u);
      }
      if(adapt_thr_list.size > lay->kwta.adth_k)
	adapt_thr_list.Permute();
      int mxi = MIN(adapt_thr_list.size, lay->kwta.adth_k);
      int i;
      for(i=0;i<mxi;i++)
	adapt_thr_list.FastEl(i)->AdaptThresh(lay, (BioBelInhib*)lay, phase);
    }
    else {
      FOR_ITR_EL(BioBelUnit, u, lay->units., ui) {
	if(u->AdaptThreshTest(lay, (BioBelInhib*)lay, phase))
	  u->AdaptThresh(lay, (BioBelInhib*)lay, phase);
      }
    }
  }
  adapt_thr_list.size = 0;
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


