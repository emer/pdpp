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

#include "leabra.h"
#include <pdp/enviro.h>
#include <pdp/procs_extra.h>

//////////////////////////
//  	Con, Spec	//
//////////////////////////

void LeabraCon::Copy_(const LeabraCon& cp) {
  dwt = cp.dwt;
  pdw = cp.pdw;
}

void WtScaleSpec::Initialize() {
  rel = 1.0f;
  abs = 1.0f;
}

void WtSigSpec::Initialize() {
  gain = 6.0f;
  off = 1.25f;
}

void LearnMixSpec::Initialize() {
  hebb = .01f;
  err = 1.0f - hebb;
}
void LearnMixSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  err = 1.0f - hebb;
}

void FixedSAvg::Initialize() {
  fix = false;
  div_gp_n = false;
  savg = .25f;
}

void SAvgCorSpec::Initialize() {
  cor = 1.0f;
  src = SLAYER_AVG_ACT;
  thresh = .01f;
}

void LeabraConSpec::Initialize() {
  min_obj_type = &TA_LeabraCon_Group;
  min_con_type = &TA_LeabraCon;
  wt_limits.min = 0.0f;
  wt_limits.max = 1.0f;
  wt_limits.sym = true;
  wt_limits.type = WeightLimits::MIN_MAX;
  inhib = false;

  wt_sig_fun.x_range.min = 0.0f;
  wt_sig_fun.x_range.max = 1.0f;
  wt_sig_fun.res = .001f;
  wt_sig_fun.x_range.UpdateAfterEdit();

  wt_sig_fun_inv.x_range.min = 0.0f;
  wt_sig_fun_inv.x_range.max = 1.0f;
  wt_sig_fun_inv.res = .001f;
  wt_sig_fun_inv.x_range.UpdateAfterEdit();

  rnd.mean = .5f;
  rnd.var = .25f;
  lrate = .01f;
  cur_lrate = .01f;
}

void LeabraConSpec::InitLinks() {
  ConSpec::InitLinks();
  taBase::Own(wt_scale, this);
  taBase::Own(wt_sig, this);
  taBase::Own(lrate_sched, this);
  taBase::Own(lmix, this);
  taBase::Own(fix_savg, this);
  taBase::Own(savg_cor, this);
  taBase::Own(wt_sig_fun, this);
  taBase::Own(wt_sig_fun_inv, this);
  CreateWtSigFun();
}

void LeabraConSpec::UpdateAfterEdit() {
  ConSpec::UpdateAfterEdit();
  lrate_sched.UpdateAfterEdit();
  CreateWtSigFun();
  lmix.UpdateAfterEdit();
}

void LeabraConSpec::SetCurLrate(int epoch) {
  cur_lrate = lrate * lrate_sched.GetVal(epoch);
}

void LeabraConSpec::CreateWtSigFun() {
  wt_sig_fun.AllocForRange();
  int i;
  for(i=0; i<wt_sig_fun.size; i++) {
    float w = wt_sig_fun.Xval(i);
    wt_sig_fun.FastEl(i) = WtSigSpec::SigFun(w, wt_sig.gain, wt_sig.off);
  }
  wt_sig_fun_inv.AllocForRange();
  for(i=0; i<wt_sig_fun_inv.size; i++) {
    float w = wt_sig_fun_inv.Xval(i);
    if(w > .9999f)
      w = 1.0f;
    wt_sig_fun_inv.FastEl(i) = WtSigSpec::SigFunInv(w, wt_sig.gain, wt_sig.off);
  }
}

void LeabraCon_Group::Initialize() {
  spec.SetBaseType(&TA_LeabraConSpec);
  el_typ = &TA_LeabraCon;
  scale_eff = 0.0f;
  savg = 0.0f;
  max_cor = 1.0f;
}

void LeabraCon_Group::Copy_(const LeabraCon_Group& cp) {
  scale_eff = cp.scale_eff;
  savg = cp.savg;
  max_cor = cp.max_cor;
}

void LeabraBiasSpec::Initialize() {
  min_obj_type = &TA_LeabraUnitSpec;
  SetUnique("rnd", true);
  SetUnique("wt_limits", true);
  SetUnique("wt_scale", true);
  rnd.mean = 0.0f;
  rnd.var = 0.0f;
  wt_limits.min = -1.0f;
  wt_limits.max = 5.0f;
  wt_limits.sym = false;
  wt_limits.type = WeightLimits::NONE;
  dwt_thresh = .1f;
}

bool LeabraBiasSpec::CheckObjectType_impl(TAPtr obj) {
  // don't allow anything to point to a biasspec except the unitspec!
  if(!obj->InheritsFrom(TA_BaseSpec) &&
     !obj->InheritsFrom(TA_LeabraCon)) return false;
  return true;
}

void LeabraNegBiasSpec::Initialize() {
  decay = 0.0f;
}

void LeabraMaintConSpec::Initialize() {
  maint_thresh=.9f;
}


//////////////////////////
//  	Unit, Spec	//
//////////////////////////

void LeabraChannels::Initialize() {
  e = l = i = h = a = 0.0f;
}

void LeabraChannels::Copy_(const LeabraChannels& cp) {
  e = cp.e;
  l = cp.l;
  i = cp.i;
  h = cp.h;
  a = cp.a;
}

void VChanSpec::Initialize() {
  on = false;
  b_dt = .01f;
  a_thr = .5f;
  d_thr = .1f;
  g_dt = .1f;
  init = true;
}

void VChanBasis::Initialize() {
  hyst = acc = 0.0f;
  hyst_on = acc_on = false;
  g_h = g_a = 0.0f;
}

void VChanBasis::Copy_(const VChanBasis& cp) {
  hyst = cp.hyst;
  acc = cp.acc;
  hyst_on = cp.hyst_on;
  acc_on = cp.acc_on;
  g_h = cp.g_h;
  g_a = cp.g_a;
}

void ActFunSpec::Initialize() {
  thr = .25f;
  gain = 600.0f;
  nvar = .005;
}

void SpikeFunSpec::Initialize() {
  dur = 3;
  v_m_r = 0.0f;
  eq_gain = 10.0f;
  ext_gain = .4f;
  // dt of .25, and vm_noise var .002 should also be used
}

void AdaptThreshSpec::Initialize() {
  a_dt = .005f;
  min = .01f;
  max = .35f;
  //  t_dt = .1f;
  t_dt = 0.0f;
  mx_d = .04f;
}

void OptThreshSpec::Initialize() {
  send = .1f;
  learn = 0.0f;
  updt_wts = true;
  phase_dif = 0.0f;		// .8 also useful
}

void DtSpec::Initialize() {
  vm = 0.2f;
  net = 0.7f;
}

void LeabraUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraUnit;
  bias_con_type = &TA_LeabraCon;
  bias_spec.SetBaseType(&TA_LeabraConSpec);

  act_fun = NOISY_XX1;

  clamp_range.min = .0f;
  clamp_range.max = .95f;

  vm_range.max = 1.0f;
  vm_range.min = 0.0f;
  v_m_init.type = Random::UNIFORM;
  v_m_init.mean = .15f;
  v_m_init.var = 0.0f;

  g_bar.e = 1.0f;
  g_bar.l = 0.1f;
  g_bar.i = 1.0f;
  g_bar.h = 0.1f;
  g_bar.a = 0.5f;
  e_rev.e = 1.0f;
  e_rev.l = 0.15f;
  e_rev.i = 0.15f;
  e_rev.h = 1.0f;
  e_rev.a = 0.0f;

  hyst.b_dt = .05f;
  hyst.a_thr = .8f;
  hyst.d_thr = .7f;

  noise_type = NO_NOISE;
  noise.type = Random::GAUSSIAN;
  noise.var = .001f;

  noise_conv.x_range.min = -.05f;
  noise_conv.x_range.max = .05f;
  noise_conv.res = .001f;
  noise_conv.x_range.UpdateAfterEdit();

  nxx1_fun.x_range.min = -.03f;
  nxx1_fun.x_range.max = .20f;
  nxx1_fun.res = .001f;
  nxx1_fun.x_range.UpdateAfterEdit();

  phase_delta = 0.0f;
}

void LeabraUnitSpec::InitLinks() {
  bias_spec.type = &TA_LeabraBiasSpec;
  UnitSpec::InitLinks();
  taBase::Own(clamp_range, this);
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

void LeabraUnitSpec::UpdateAfterEdit() {
  UnitSpec::UpdateAfterEdit();
  noise_sched.UpdateAfterEdit();
  CreateNXX1Fun();
}

void LeabraUnitSpec::CreateNXX1Fun() {
  // first create the gaussian noise convolver
  noise_conv.AllocForRange();
  int i;
  float var = act.nvar * act.nvar;
  for(i=0; i < noise_conv.size; i++) {
    float x = noise_conv.Xval(i);
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
    float x = fun.Xval(i);
    float val = 0.0f;
    if(x > 0.0f)
      val = (act.gain * x) / ((act.gain * x) + 1.0f);
    fun.FastEl(i) = val;
  }

  nxx1_fun.Convolve(fun, noise_conv);
}

void LeabraUnitSpec::InitWtState(Unit* u) {
  UnitSpec::InitWtState(u);
  LeabraUnit* lu = (LeabraUnit*)u;
  InitThresh(lu);
  if(!hyst.init) {
    lu->vcb.hyst = lu->vcb.g_h = 0.0f;
    lu->vcb.hyst_on = false;
  }
  if(!acc.init) {
    lu->vcb.acc = lu->vcb.g_a = 0.0f;
    lu->vcb.acc_on = false;
  }
  phase_delta = 0.0f;
}

void LeabraUnitSpec::InitThresh(LeabraUnit* u) {
  u->act_avg = adapt_thr.min + .5f * (adapt_thr.max - adapt_thr.min);
  u->thr = act.thr;
}

void LeabraUnitSpec::SetCurLrate(LeabraUnit* u, int epoch) {
  ((LeabraConSpec*)bias_spec.spec)->SetCurLrate(epoch);
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g)
    recv_gp->SetCurLrate(epoch);
}

void LeabraUnitSpec::InitState(LeabraUnit* ru, LeabraLayer*) {
  UnitSpec::InitState(ru);
  //  ru->clmp_net = 0.0f;
  ru->net_scale = 0.0f;
  ru->bias_scale = 0.0f;
  ru->prv_net = 0.0f;
  ru->prv_g_i = 0.0f;
  if(hyst.init) {
    ru->vcb.hyst = ru->vcb.g_h = 0.0f;
    ru->vcb.hyst_on = false;
  }
  if(acc.init) {
    ru->vcb.acc = ru->vcb.g_a = 0.0f;
    ru->vcb.acc_on = false;
  }
  ru->gc.l = 0.0f;
  ru->gc.i = 0.0f;
  ru->gc.h = 0.0f;
  ru->gc.a = 0.0f;
  ru->I_net = 0.0f;
  ru->v_m = v_m_init.Gen();
  ru->da = 0.0f;
  ru->act = 0.0f;
  ru->act_eq = 0.0f;
  ru->act_p = ru->act_m = ru->act_dif = 0.0f;
  // always reset threshold to spec value if not adapting
  if(adapt_thr.t_dt == 0.0f)
    ru->thr = act.thr;
}

void LeabraUnitSpec::ExtToComp(LeabraUnit* u, LeabraLayer*) {
  if(!(u->ext_flag & Unit::EXT))
    return;
  u->ext_flag = Unit::COMP;
  u->targ = u->ext;
  u->ext = 0.0f;
}

void LeabraUnitSpec::TargExtToComp(LeabraUnit* u, LeabraLayer*) {
  if(!(u->ext_flag & Unit::TARG_EXT))
    return;
  u->ext_flag = Unit::COMP;
  u->targ = u->ext;
  u->ext = 0.0f;
}

void LeabraUnitSpec::Compute_NetScale(LeabraUnit* u, LeabraLayer*) {
  // this is all receiver-based and done only at beginning of settling
  u->clmp_net = 0.0f;
  u->net_scale = 0.0f;	// total of scale values for this unit's inputs

  int n_cons = 0;
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
     // this is the normalization value: takes into account target activity of layer
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    WtScaleSpec& wt_scale = cs->wt_scale;
    // this gets either lay->kwta.pct or fixed value, depending on fix toggle
    float savg = cs->fix_savg.GetSAvg(lay->kwta.pct);
    if(cs->fix_savg.div_gp_n)	// sometimes it makes sense to just do it by the group n
      recv_gp->scale_eff = wt_scale.NetScale() * (1.0f / ((float)recv_gp->size * savg));
    else
      recv_gp->scale_eff = wt_scale.NetScale() * (1.0f / ((float)lay->units.leaves * savg));
    u->net_scale += wt_scale.rel;
    n_cons += recv_gp->size;
  }
  // add the bias weight into the netinput, scaled by 1/n
  if(u->bias != NULL) {
    LeabraConSpec* bspec = (LeabraConSpec*)bias_spec.spec;
    u->bias_scale = bspec->wt_scale.abs;  // still have absolute scaling if wanted..
    if(n_cons > 0)
      u->bias_scale /= (float)n_cons; // one over n scaling for bias!
    u->clmp_net += u->bias_scale * u->bias->wt;
  }
  // now renormalize
  if(u->net_scale == 0.0f) return;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
    recv_gp->scale_eff /= u->net_scale; // normalize by total connection scale
  }
}

void LeabraUnitSpec::Send_ClampNet(LeabraUnit* u, LeabraLayer*) {
  if(u->act > opt_thresh.send) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      if(!send_gp->prjn->layer->lesion)
	send_gp->Send_ClampNet(u);
    }
  }
}

void LeabraUnitSpec::Compute_Net(LeabraUnit* u, LeabraLayer*) {
  // sigmoid is sender-based (and this fun not called on hard_clamped EXT layers)
  if(u->act > opt_thresh.send) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* lay = (LeabraLayer*) send_gp->prjn->layer;
      if(lay->lesion || lay->hard_clamped)	continue;
      send_gp->Send_Net(u);
    }
  }
  u->net += u->clmp_net;	// add in input from clamped inputs (either send or recv)
}

void LeabraUnitSpec::Compute_HardClamp(LeabraUnit* u, LeabraLayer* lay) {
  u->net = u->prv_net = u->ext * lay->stm_gain;
  u->act_eq = clamp_range.Clip(u->ext);
  if(act_fun == SPIKE)
    u->act = spike.ext_gain * u->act_eq;
  else
    u->act = u->act_eq;
  if(u->act_eq == 0.0f)
    u->v_m = e_rev.l;
  else
    u->v_m = u->thr + u->act_eq / act.gain;
  u->da = u->I_net = 0.0f;
}

bool LeabraUnitSpec::Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay) {
  bool inc_gain = false;
  if(u->ext_flag & Unit::EXT) {
    if((u->ext > .5f) && (u->act < .5f))
      inc_gain = true;	// must increase gain because targ unit is < .5..

    u->net += u->ext * lay->stm_gain;
  }
  return inc_gain;
}

void LeabraUnitSpec::Compute_Act(LeabraUnit* u, LeabraLayer*, LeabraInhib* thr,
				  int cycle)
{
  if((noise_type == NETIN_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->net += noise_sched.GetVal(cycle) * noise.Gen();
  }

  // total conductances
  u->gc.l = g_bar.l;
  u->gc.i += g_bar.i * thr->i_val.g_i; // add in inhibition from global inhib fctn
  u->gc.h = g_bar.h * u->vcb.g_h;
  u->gc.a = g_bar.a * u->vcb.g_a;

  u->I_net = 
    (u->net * (e_rev.e - u->v_m)) + (u->gc.l * (e_rev.l - u->v_m)) + 
    (u->gc.i * (e_rev.i - u->v_m)) + (u->gc.h * (e_rev.h - u->v_m)) +
    (u->gc.a * (e_rev.a - u->v_m));

  u->v_m += dt.vm * u->I_net;

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
    }
  }
  else if(act_fun != SPIKE) {
    if(new_act < 0.0f)
      new_act = 0.0f;
    else {
      new_act *= act.gain;
      if(act_fun == XX1)
	new_act = new_act / (new_act + 1.0f);
    }
  }
  else {			// spiking
    float spike_act = 0.0f;
    if(u->act > .9f) {		// keep track of the length of spike in the residuals
      int len = 1 + (int)((1.0f - u->act) * 100.0f);
      if(len == 1)		// just spiked
	u->v_m = spike.v_m_r;
      len++;
      if(len > spike.dur) {
	new_act = 0.0f;
      }
      else {
	new_act = 1.0f - ((float)len / 100.0f);
      }
    }
    else if(u->v_m > u->thr) {
      new_act = 1.0f;
      spike_act = 1.0f;
    }
    else
      new_act = 0.0f;

    u->act = new_act;
    float new_eq = u->act_eq / spike.eq_gain;
    if(cycle > 0)
      new_eq *= (float)cycle;
    new_eq = act_range.Clip(spike.eq_gain * (new_eq + spike_act) / (float)(cycle+1));
    u->da = new_eq - u->act_eq;	// da is on equilibrium activation
    u->act_eq = new_eq;
  }

  if(act_fun != SPIKE) {
    u->da = new_act - u->act;
    if((noise_type == ACT_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
      new_act += noise_sched.GetVal(cycle) * noise.Gen();
    }
    u->act = u->act_eq = act_range.Clip(new_act);
  }

  // fast-time scale updated during settling
  hyst.UpdateBasis(u->vcb.hyst, u->vcb.hyst_on, u->vcb.g_h, u->act_eq);
  acc.UpdateBasis(u->vcb.acc, u->vcb.acc_on, u->vcb.g_a, u->act_eq);
}

float LeabraUnitSpec::Compute_IThresh(LeabraUnit* u) {
  float non_bias_net = u->net;
  if(u->bias != NULL) {
    non_bias_net -= u->bias_scale * u->bias->wt;
  }

  // including the ga and gh terms
  return ((non_bias_net * (e_rev.e - act.thr) + g_bar.l * (e_rev.l - act.thr)
	   + u->gc.a * (e_rev.a - act.thr) + u->gc.h * (e_rev.h - act.thr)) /
	  (act.thr - e_rev.i));

  // not including the ga and gh terms -- what about using real threshold??
//   return ((non_bias_net * (e_rev.e - act.thr) + g_bar.l * (e_rev.l - act.thr)) /
// 	  (act.thr - e_rev.i));
} 

void LeabraUnitSpec::DecayState(LeabraUnit* u, LeabraLayer*, float decay) {
  u->v_m -= decay * (u->v_m - v_m_init.mean);
  u->act -= decay * u->act;
  u->act_eq -= decay * u->act_eq;
  u->prv_net -= decay * u->prv_net;
  u->prv_g_i -= decay * u->prv_g_i;
  u->vcb.hyst -= decay * u->vcb.hyst;
  u->vcb.acc -= decay * u->vcb.acc;

  // reset the rest of this stuff just for clarity
  u->net = 0.0f;
  u->net_scale = u->bias_scale = 0.0f;
  u->da = u->I_net = 0.0f;
}

bool LeabraUnitSpec::AdaptThreshTest(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  u->act_avg += adapt_thr.a_dt * (u->act_eq - u->act_avg);
  if((u->act_avg < adapt_thr.min) || (u->act_avg > adapt_thr.max))
    return true;		// need to adapt threshold
  else				 // decay the threshold back towards normal
    u->thr += adapt_thr.t_dt * (act.thr - u->thr);
  return false;
}

void LeabraUnitSpec::AdaptThresh(LeabraUnit* u, LeabraLayer*, LeabraInhib*) {
  if(u->act_avg < adapt_thr.min)
    u->thr += adapt_thr.t_dt * ((act.thr - adapt_thr.mx_d) - u->thr);
  else if(u->act_avg > adapt_thr.max)
    u->thr += adapt_thr.t_dt * ((act.thr + adapt_thr.mx_d) - u->thr);
}

void LeabraUnitSpec::PhaseInit(LeabraUnit* u, LeabraLayer*, int phase) {
  if(!(u->ext_flag & Unit::TARG))
    return;

  if(phase == LeabraTrial::MINUS_PHASE) {
    u->ext = 0.0f;
    u->UnSetExtFlag(Unit::EXT);
  }
  else {
    u->ext = u->targ;
    u->SetExtFlag(Unit::EXT);
  }
}

void LeabraUnitSpec::PostSettle(LeabraUnit* u, LeabraLayer*, LeabraInhib*,
				LeabraTrial* trl, bool set_both)
{
  if(set_both) {
    u->act_m = u->act_p = u->act_eq;
    u->act_dif = 0.0f;
  }
  else {
    if(trl->phase == LeabraTrial::MINUS_PHASE)
      u->act_m = u->act_eq;
    else {
      u->act_p = u->act_eq;
      if(phase_delta != 0.0f)
	u->act_p = act_range.Clip(u->act_p * (1.0f + phase_delta));
    }
    u->act_dif = u->act_p - u->act_m;
  }
}

void LeabraUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay) {
  if((u->act_p <= opt_thresh.learn) && (u->act_m <= opt_thresh.learn))
    return;
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay);
}

void LeabraUnitSpec::Compute_dWt_impl(LeabraUnit* u, LeabraLayer*) {
  // don't adapt bias weights on clamped inputs..
  if(!((u->ext_flag & Unit::EXT) && !(u->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
  }
  UnitSpec::Compute_dWt(u);
}

void LeabraUnitSpec::Compute_dWt_post(LeabraUnit*, LeabraLayer*) {
}

void LeabraUnitSpec::Compute_WtFmLin(LeabraUnit* u, LeabraLayer*) {
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)	continue;
    recv_gp->Compute_WtFmLin();
  }
}

void LeabraUnitSpec::UpdateWeights(Unit* u) {
  LeabraUnit* lu = (LeabraUnit*)u;
  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
  }
  if(opt_thresh.updt_wts && 
     ((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)))
    return;
  UnitSpec::UpdateWeights(lu);
}


//////////////////////////
//  	Unit 		//
//////////////////////////


void LeabraUnitChans::Initialize() {
  l = i = h = a = 0.0f;
}

void LeabraUnitChans::Copy_(const LeabraUnitChans& cp) {
  l = cp.l;
  i = cp.i;
  h = cp.h;
  a = cp.a;
}

void LeabraUnit::Initialize() {
  spec.SetBaseType(&TA_LeabraUnitSpec);
  recv.spec.SetBaseType(&TA_LeabraConSpec);
  send.spec.SetBaseType(&TA_LeabraConSpec);

  act_eq = 0.0f;
  act_avg = 0.1f;
  act_p = act_m = act_dif = 0.0f;
  da = 0.0f;
  I_net = 0.0f;
  v_m = 0.0f;
  thr = .25f;

  in_subgp = false;
  clmp_net = 0.0f;
  net_scale = 0.0f;
  bias_scale = 0.0f;
  prv_net = 0.0f;
  prv_g_i = 0.0f;
}

void LeabraUnit::InitLinks() {
  Unit::InitLinks();
  taBase::Own(vcb, this);
  taBase::Own(gc, this);
  GetInSubGp();
}

void LeabraUnit::GetInSubGp() {
  Unit_Group* ownr = (Unit_Group*)owner;  
  if((ownr != NULL) && (ownr->owner != NULL) && ownr->owner->InheritsFrom(TA_taSubGroup))
    in_subgp = true;
  else
    in_subgp = false;
}

//////////////////////////
//  	Layer, Spec	//
//////////////////////////

void KWTASpec::Initialize() {
  k_from = USE_PCT;
  k = 12;
  pct = .25f;
  pat_q = .5f;
  adth_pct = .5f;
}

void AdaptKWTASpec::Initialize() {
  a_dt = .005f;			// time averaging
  tol = .05f;			// allow to be off by .05 before taking action
  //  pt_dt = .01f;			// take reasonably quick action..
  pt_dt = 0.0f;			// deactivate by default
  mx_d = .2f;			// move this far in either direction
}

void ClampSpec::Initialize() {
  hard = true;
  gain = .5f;
  d_gain = 0.0f;
}

void DecaySpec::Initialize() {
  event = 1.0f;
  phase = 1.0f;
  phase2 = 0.0f;
}

void LayerLinkSpec::Initialize() {
  link = false;
  gain = 0.5f;
}
  
void LeabraLayerSpec::Initialize() {
  min_obj_type = &TA_LeabraLayer;
  inhib_group = ENTIRE_LAYER;
  compute_i = KWTA_INHIB;
  i_kwta_pt = .25f;
}

void LeabraLayerSpec::UpdateAfterEdit() {
  LayerSpec::UpdateAfterEdit();
}

void LeabraLayerSpec::InitLinks() {
  LayerSpec::InitLinks();
  taBase::Own(kwta, this);
  taBase::Own(gp_kwta, this);
  taBase::Own(adapt_pt, this);
  taBase::Own(clamp, this);
  taBase::Own(decay, this);
  taBase::Own(layer_link, this);
}

void LeabraLayerSpec::CutLinks() {
  LayerSpec::CutLinks();
}

void LeabraLayerSpec::InitThresh(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->InitThresh();
}

void LeabraLayerSpec::InitWtState(LeabraLayer* lay) {
  Compute_Active_K(lay);	// need kwta.pct for init
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->InitWtState();		// this also does InitThresh
  InitInhib(lay);		// initialize inhibition at start..
}

void LeabraLayerSpec::SetCurLrate(LeabraLayer* lay, int epoch) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->SetCurLrate(epoch);
}

//////////////////////////////////////////
//	Stage 0: at start of settling	// 
//////////////////////////////////////////

void LeabraLayerSpec::InitState(LeabraLayer* lay) {
  lay->ext_flag = Unit::NO_EXTERNAL;
  lay->stm_gain = clamp.gain;
  lay->hard_clamped = false;
  lay->prv_phase = LeabraTrial::MINUS_PHASE;
  lay->ResetSortBuf();
  Compute_Active_K(lay);	// need kwta.pct for init
  lay->Inhib_InitState(this);
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      rugp->Inhib_InitState(this);
    }
  }
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->InitState(lay);
}

void LeabraLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial*) {
  if(!(clamp.hard && (lay->ext_flag & Unit::EXT))) {
    lay->hard_clamped = false;
    return;
  }
  lay->hard_clamped = true;	// cache this flag

  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  if(lay->units.gp.size > 0) {
    float lay_avg = 0.0f;
    float lay_max = 0.0f;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      float tmp_avg = 0.0f;
      float tmp_max = 0.0f;
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      rugp->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs
      LeabraUnit* u;
      taLeafItr i;
      FOR_ITR_EL(LeabraUnit, u, rugp->, i) {
	u->Compute_HardClamp(lay);
	tmp_avg += u->act_eq;
	tmp_max = MAX(tmp_max, u->act_eq);
      }
      lay_avg += tmp_avg;
      lay_max = MAX(tmp_max, lay_max);
      if(rugp->size > 0)
	rugp->acts.avg = tmp_avg / (float)rugp->size;
      rugp->acts.max = tmp_max;
    }
    lay->acts.avg = lay_avg / (float)lay->units.leaves;
    lay->acts.max = lay_max;
  }
  else {
    float tmp_avg = 0.0f;
    float tmp_max = 0.0f;
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
      u->Compute_HardClamp(lay);
      tmp_avg += u->act_eq;
      tmp_max = MAX(tmp_max, u->act_eq);
    }
    lay->acts.avg = tmp_avg / (float)lay->units.leaves;
    lay->acts.max = tmp_max;
  }
}
 
void LeabraLayerSpec::Compute_NetScale(LeabraLayer* lay, int) {
  if(lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_NetScale(lay);
}

void LeabraLayerSpec::Send_ClampNet(LeabraLayer* lay, int) {
  if(!lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Send_ClampNet(lay);
}

//////////////////////////////////
//	Stage 1: netinput 	//
//////////////////////////////////

void LeabraLayerSpec::Compute_Net(LeabraLayer* lay, int, int) {
  // hard-clamped input layers are already computed in the clmp_net value
  if(lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_Net(lay);
}

//////////////////////////////////////////////////////////////////
//	Stage 2: netinput averages and clamping (if necc)	//
//////////////////////////////////////////////////////////////////

void LeabraLayerSpec::Compute_Clamp_NetAvg(LeabraLayer* lay, int cycle, int phase) {
  if(lay->hard_clamped) return;

  if(lay->ext_flag & Unit::EXT)
    Compute_SoftClamp(lay, cycle, phase);

  lay->netin.avg = 0.0f;
  lay->netin.max = 0.0f;

  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_NetAvg(lay, rugp, (LeabraInhib*)rugp, cycle, phase);
      // keep maximums for linking purposes
      lay->netin.avg = MAX(lay->netin.avg, rugp->netin.avg);
      lay->netin.max = MAX(lay->netin.max, rugp->netin.max);
    }
  }
  else {
    Compute_NetAvg(lay, &(lay->units), (LeabraInhib*)lay, cycle, phase);
  }
}

void LeabraLayerSpec::Compute_NetAvg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
				    int, int)
{
  thr->netin.avg = 0.0f;
  thr->netin.max = -MAXFLOAT;
  thr->un_g_i.avg = 0.0f;
  thr->un_g_i.max = -MAXFLOAT;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->Compute_NetAvg(lay, thr);
    thr->netin.avg += u->net;
    thr->netin.max = MAX(thr->netin.max, u->net);
    u->Compute_InhibAvg(lay, thr);
    thr->un_g_i.avg += u->gc.i;
    thr->un_g_i.max = MAX(thr->un_g_i.max, u->gc.i);
  }

  if(ug->leaves > 0) {
    thr->netin.avg /= (float)ug->leaves;	// turn it into an average
    thr->un_g_i.avg /= (float)ug->leaves;	// turn it into an average
  }
}

void LeabraLayerSpec::Compute_SoftClamp(LeabraLayer* lay, int cycle, int) {
  LeabraUnit* u;
  taLeafItr i;
  bool inc_gain = false;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    if(!inc_gain)
      inc_gain = u->Compute_SoftClamp(lay);
    else
      u->Compute_SoftClamp(lay);
  }
  if(inc_gain && (cycle > 1)) // only increment after it has a chance to activate
    lay->stm_gain += clamp.d_gain;
}


//////////////////////////////////////////
//	Stage 3: inhibition		//
//////////////////////////////////////////

void LeabraLayerSpec::InitInhib(LeabraLayer* lay) {
  lay->adapt_pt.avg_avg = lay->kwta.pct;
  lay->adapt_pt.i_kwta_pt = i_kwta_pt;
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      rugp->adapt_pt.avg_avg = rugp->kwta.pct;
      rugp->adapt_pt.i_kwta_pt = i_kwta_pt;
    }
  }
}

void LeabraLayerSpec::Compute_Inhib(LeabraLayer* lay, int, int) {
  if(lay->hard_clamped)	return;	// say no more..

  if(inhib_group != UNIT_GROUPS) {
    Compute_Inhib_impl(lay, &(lay->units), (LeabraInhib*)lay);
  }
  if(lay->units.gp.size > 0) {
    int g;
    if(inhib_group != ENTIRE_LAYER) {
      if(inhib_group == UNIT_GROUPS) {
	lay->Inhib_SetVals(0.0f);
	for(g=0; g<lay->units.gp.size; g++) {
	  LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	  Compute_Inhib_impl(lay, rugp, (LeabraInhib*)rugp);
	  // i_val keeps the averages...
	  lay->i_val.g_i += rugp->i_val.g_i;
	  // i_val_orig keeps the maximums for linking purposes..
	  lay->i_val.g_i_orig = MAX(lay->i_val.g_i_orig, rugp->i_val.g_i);
	}
	lay->i_val.g_i /= (float)g;
      }
      else {
	for(g=0; g<lay->units.gp.size; g++) {
	  LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	  Compute_Inhib_impl(lay, rugp, (LeabraInhib*)rugp);
	  // actual inhibition is max of layer and group
	  rugp->i_val.g_i = MAX(rugp->i_val.g_i, lay->i_val.g_i);
	}
      }
    }
    else {
      // propagate g_i to all subgroups even if doing ENTIRE_LAYER
      for(g=0; g<lay->units.gp.size; g++) {
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	rugp->i_val.g_i = lay->i_val.g_i;
      }
    }
  }
}

void LeabraLayerSpec::Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr) {
  // for the time being, always compute kwta just to have it around..
  if(compute_i == KWTA_AVG_INHIB)
    Compute_Inhib_kWTA_Avg(lay, ug, thr);
  else
    Compute_Inhib_kWTA(lay, ug, thr);

  switch(compute_i) {
  case KWTA_INHIB:
    thr->i_val.g_i = thr->i_val.kwta;
    break;
  case KWTA_AVG_INHIB:
    thr->i_val.g_i = thr->i_val.kwta;
    break;
  case UNIT_INHIB:
    thr->i_val.g_i = 0.0f;	// make sure it's zero, cuz this gets added to units.. 
    break;
  }

  thr->i_val.g_i_orig = thr->i_val.g_i;	// retain original values..
}

#ifdef USE_HEAP_KWTA

// this is the heap-based kwta, with the top-k organized into a heap
// turns out to not be any faster than the kwta based.

void LeabraLayerSpec::Compute_Inhib_kWTA(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one
  k_plus_1 = MIN(ug->leaves,k_plus_1);

  LeabraUnit* u;
  taLeafItr lfi;
  int idx;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    for(idx=0, u = (LeabraUnit*)ug->FirstEl(lfi); u && (idx < k_plus_1);
	u = (LeabraUnit*)ug->NextEl(lfi), idx++)
    {
      thr->active_buf.Add(u);		// add unit to the list
    }
    thr->inact_buf.size = 0;
    for(; u; u = (LeabraUnit*)ug->NextEl(lfi)) {
      thr->inact_buf.Add(u);		// add unit to the list
    }
  }

  // first heap the active buf, using upside-down heap so [0] = *smallest*
  int i, j;
  LeabraUnit* rra;
  int l = (k_plus_1 >> 1)+1;
  while(l>1) {
    rra = thr->active_buf.FastEl(--l -1);
    i=l;
    j=l << 1;
    while(j<= k_plus_1) {
      if(j<k_plus_1 && thr->active_buf.FastEl(j-1)->net > thr->active_buf.FastEl(j)->net) {
	j++;
      }
      if(rra->net > thr->active_buf.FastEl(j-1)->net) {
	thr->active_buf.Replace(i-1, thr->active_buf.FastEl(j-1));
	j += (i=j);
      }
      else j=k_plus_1+1;
    }
    thr->active_buf.Replace(i-1, rra);
  }

  // then add in new ones
  for(idx=0; idx < thr->inact_buf.size; idx++) {
    rra = thr->inact_buf.FastEl(idx);
    // skip anything smaller than current smallest!
    if(rra->net < thr->active_buf.FastEl(0)->net) continue; 
    i=1;
    j=2;
    while(j<= k_plus_1) {
      if(j<k_plus_1 && thr->active_buf.FastEl(j-1)->net > thr->active_buf.FastEl(j)->net) {
	j++;
      }
      if(rra->net > thr->active_buf.FastEl(j-1)->net) {
	if(thr->inact_buf.FastEl(idx) == rra) // save replaced one 
	  thr->inact_buf.Replace(idx, thr->active_buf.FastEl(i-1));	// now inactive
	thr->active_buf.Replace(i-1, thr->active_buf.FastEl(j-1));
	j += (i=j);
      }
      else j=k_plus_1+1;
    }
    if(thr->inact_buf.FastEl(idx) == rra) // save replaced one 
      thr->inact_buf.Replace(idx, thr->active_buf.FastEl(i-1));	// now inactive
    thr->active_buf.Replace(i-1, rra);
  }

  // active_buf now has k+1 most active units in a heap, get the next-highest one

  // lowest (k+1) unit is element [0]
  LeabraUnit* k1_u = thr->active_buf.FastEl(0);
  // next lowest is either 1 or 2
  LeabraUnit* k_u;
  if(k_plus_1 == 2)
    k_u = thr->active_buf.FastEl(1);
  else {
    if(thr->active_buf.FastEl(1)->net < thr->active_buf.FastEl(2)->net)
      k_u = thr->active_buf.FastEl(1);
    else
      k_u = thr->active_buf.FastEl(2);
  }

  float k1_i = k1_u->Compute_IThresh();
  float k_i = k_u->Compute_IThresh();

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_i;
  thr->kwta.k1_ithr = k1_i;
  thr->kwta.Compute_IThrR();
}

#else

// this is the original version which does not impose any ordering on the 
// top-k grouping

void LeabraLayerSpec::Compute_Inhib_kWTA(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one
  k_plus_1 = MIN(ug->leaves,k_plus_1);

  float net_k1 = MAXFLOAT;
  int k1_idx = 0;
  LeabraUnit* u;
  taLeafItr i;
  int j;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    j = 0;
    for(u = (LeabraUnit*)ug->FirstEl(i); u && (j < k_plus_1);
	u = (LeabraUnit*)ug->NextEl(i), j++)
    {
      thr->active_buf.Add(u);		// add unit to the list
      if(u->net < net_k1) {
	net_k1 = u->net;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (LeabraUnit*)ug->NextEl(i)) {
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
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(i)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k+1 most active units, get the next-highest one
  int k_idx = -1;
  float net_k = MAXFLOAT;
  for(j=0; j < k_plus_1; j++) {
    float tmp = thr->active_buf.FastEl(j)->net;
    if((tmp < net_k) && (j != k1_idx)) {
      net_k = tmp;		k_idx = j;
    }
  }
  if(k_idx == -1) {		// we didn't find the next one
    k_idx = k1_idx;
    net_k = net_k1;
  }

  LeabraUnit* k1_u = (LeabraUnit*)thr->active_buf.FastEl(k1_idx);
  LeabraUnit* k_u = (LeabraUnit*)thr->active_buf.FastEl(k_idx);

  float k1_i = k1_u->Compute_IThresh();
  float k_i = k_u->Compute_IThresh();

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_i;
  thr->kwta.k1_ithr = k1_i;
  thr->kwta.Compute_IThrR();
}

#endif

void LeabraSort::FastInsertLink(void* it, int where) {
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

int LeabraSort::FindNewNetPos(float nw_net) {
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

#ifdef USE_HEAP_KWTA

// this is the heap-based kwta, with the top-k organized into a heap
// turns out to not be any faster than the kwta based.

void LeabraLayerSpec::Compute_Inhib_kWTA_Avg(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k;	// keep cutoff at k

  LeabraUnit* u;
  taLeafItr lfi;
  int idx;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    for(idx=0, u = (LeabraUnit*)ug->FirstEl(lfi); u && (idx < k_plus_1);
	u = (LeabraUnit*)ug->NextEl(lfi), idx++)
    {
      thr->active_buf.Add(u);		// add unit to the list
    }
    thr->inact_buf.size = 0;
    for(; u; u = (LeabraUnit*)ug->NextEl(lfi)) {
      thr->inact_buf.Add(u);		// add unit to the list
    }
  }

  // first heap the active buf, using upside-down heap so [0] = *smallest*
  int i, j;
  LeabraUnit* rra;
  int l = (k_plus_1 >> 1)+1;
  while(l>1) {
    rra = thr->active_buf.FastEl(--l -1);
    i=l;
    j=l << 1;
    while(j<= k_plus_1) {
      if(j<k_plus_1 && thr->active_buf.FastEl(j-1)->net > thr->active_buf.FastEl(j)->net) {
	j++;
      }
      if(rra->net > thr->active_buf.FastEl(j-1)->net) {
	thr->active_buf.Replace(i-1, thr->active_buf.FastEl(j-1));
	j += (i=j);
      }
      else j=k_plus_1+1;
    }
    thr->active_buf.Replace(i-1, rra);
  }

  // then add in new ones
  for(idx=0; idx < thr->inact_buf.size; idx++) {
    rra = thr->inact_buf.FastEl(idx);
    // skip anything smaller than current smallest!
    if(rra->net < thr->active_buf.FastEl(0)->net) continue; 
    i=1;
    j=2;
    while(j<= k_plus_1) {
      if(j<k_plus_1 && thr->active_buf.FastEl(j-1)->net > thr->active_buf.FastEl(j)->net) {
	j++;
      }
      if(rra->net > thr->active_buf.FastEl(j-1)->net) {
	if(thr->inact_buf.FastEl(idx) == rra) // save replaced one 
	  thr->inact_buf.Replace(idx, thr->active_buf.FastEl(i-1));	// now inactive
	thr->active_buf.Replace(i-1, thr->active_buf.FastEl(j-1));
	j += (i=j);
      }
      else j=k_plus_1+1;
    }
    if(thr->inact_buf.FastEl(idx) == rra) // save replaced one 
      thr->inact_buf.Replace(idx, thr->active_buf.FastEl(i-1));	// now inactive
    thr->active_buf.Replace(i-1, rra);
  }

  // active_buf now has k most active units, get averages of both groups
  float k_avg = 0.0f;
  for(j=0; j < k_plus_1; j++)
    k_avg += thr->active_buf.FastEl(j)->Compute_IThresh();
  k_avg /= (float)k_plus_1;

  float oth_avg = 0.0f;
  for(j=0; j < thr->inact_buf.size; j++)
    oth_avg += thr->inact_buf.FastEl(j)->Compute_IThresh();
  if(thr->inact_buf.size > 0)
    oth_avg /= (float)thr->inact_buf.size;

  // place kwta inhibition between two averages
  // this uses the adapting point!
  float pt = i_kwta_pt;
  if(adapt_pt.pt_dt > 0.0f)
    pt = thr->adapt_pt.i_kwta_pt;
  float nw_gi = oth_avg + pt * (k_avg - oth_avg);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_avg;
  thr->kwta.k1_ithr = oth_avg;
  thr->kwta.Compute_IThrR();
}

#else

// this is the unordered top-k list method

void LeabraLayerSpec::Compute_Inhib_kWTA_Avg(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k;	// keep cutoff at k

  float net_k1 = MAXFLOAT;
  int k1_idx = 0;
  LeabraUnit* u;
  taLeafItr i;
  int j;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    j = 0;
    for(u = (LeabraUnit*)ug->FirstEl(i); u && (j < k_plus_1);
	u = (LeabraUnit*)ug->NextEl(i), j++)
    {
      thr->active_buf.Add(u);		// add unit to the list
      if(u->net < net_k1) {
	net_k1 = u->net;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (LeabraUnit*)ug->NextEl(i)) {
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
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(i)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k most active units, get averages of both groups
  float k_avg = 0.0f;
  for(j=0; j < k_plus_1; j++)
    k_avg += thr->active_buf.FastEl(j)->Compute_IThresh();
  k_avg /= (float)k_plus_1;

  float oth_avg = 0.0f;
  for(j=0; j < thr->inact_buf.size; j++)
    oth_avg += thr->inact_buf.FastEl(j)->Compute_IThresh();
  if(thr->inact_buf.size > 0)
    oth_avg /= (float)thr->inact_buf.size;

  // place kwta inhibition between two averages
  // this uses the adapting point!
  float pt = i_kwta_pt;
  if(adapt_pt.pt_dt > 0.0f)
    pt = thr->adapt_pt.i_kwta_pt;
  float nw_gi = oth_avg + pt * (k_avg - oth_avg);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_avg;
  thr->kwta.k1_ithr = oth_avg;
  thr->kwta.Compute_IThrR();
}

#endif


void LeabraLayerSpec::Compute_LinkInhib(LeabraLayer* lay, Unit_Group*, LeabraInhib* thr,
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
      float i_del = llk->link_wt * layer_link.gain * 
        (llk->layer->i_val.g_i_orig - lay->i_val.g_i_orig);
      max_i = MAX(max_i, i_del);
    }
    lay->i_val.g_i += max_i;
  }
  else if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    // lay->i_val_orig has the maximums
    float max_i = layer_link.gain * (lay->i_val.g_i_orig - thr->i_val.g_i_orig);
    thr->i_val.g_i += max_i;
    // update averages too
    lay->i_val.g_i += max_i / (float)lay->units.gp.size;
  }
}

void LeabraLayerSpec::Compute_Active_K(LeabraLayer* lay) {
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Active_K_impl(lay, rugp, (LeabraInhib*)rugp, gp_kwta);
    }
  }
  Compute_Active_K_impl(lay, &(lay->units), (LeabraInhib*)lay, kwta);
}

void LeabraLayerSpec::Compute_Active_K_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr,
					    KWTASpec& kwtspec)
{
  int new_k = 0;
  if(kwtspec.k_from == KWTASpec::USE_PCT)
    new_k = (int)(kwtspec.pct * (float)ug->leaves);
  else if((kwtspec.k_from == KWTASpec::USE_PAT_K) && 
	  ((lay->ext_flag & (Unit::TARG | Unit::COMP)) || (lay->ext_flag & Unit::EXT)))
    new_k = Compute_Pat_K(lay, ug, thr);
  else
    new_k = kwtspec.k;

  if(compute_i == KWTA_INHIB)
    new_k = MIN(ug->leaves - 1, new_k);
  else
    new_k = MIN(ug->leaves, new_k);
  new_k = MAX(1, new_k);

  if(thr->kwta.k != new_k)
    thr->Inhib_ResetSortBuf();

  thr->kwta.k = new_k;
  thr->kwta.Compute_Pct(ug->leaves);
  thr->kwta.Compute_AdthK(kwtspec.adth_pct);
}

int LeabraLayerSpec::Compute_Pat_K(LeabraLayer*, Unit_Group* ug, LeabraInhib*) {
  int pat_k = 0;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
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
//	Stage 4: the final activation 	//
//////////////////////////////////////////

void LeabraLayerSpec::Compute_Act(LeabraLayer* lay, int cycle, int phase) {
  if((cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing

  if(lay->units.gp.size > 0) {	// even if ENTIRE_LAYER, do it by sub-group to get stats..
    lay->acts.max = 0.0f; 
    float lay_acts_avg = 0.0f;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Act_impl(lay, rugp, (LeabraInhib*)rugp, cycle, phase);
      lay_acts_avg += rugp->acts.avg * (float)rugp->leaves; // weight by no of units
      lay->acts.max = MAX(lay->acts.max, rugp->acts.max);
    }
    lay->acts.avg = lay_acts_avg / (float)lay->units.leaves;
  }
  else {
    Compute_Act_impl(lay, &(lay->units), (LeabraInhib*)lay, cycle, phase);
  }
}

void LeabraLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug,
				      LeabraInhib* thr, int cycle, int)
{
  if(layer_link.link)
    Compute_LinkInhib(lay, ug, thr, cycle);

  float tmp_max = 0.0f;
  float tmp_avg = 0.0f;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->Compute_Act(lay,thr,cycle);
    tmp_max = MAX(tmp_max, u->act_eq);
    tmp_avg += u->act_eq;
  }
  thr->acts.max = tmp_max;
  if(ug->leaves > 0)
    thr->acts.avg = tmp_avg / (float)ug->leaves;
  else
    thr->acts.avg = 0.0f;
}

//////////////////////////////////////////
//	Stage 5: Between Events 	//
//////////////////////////////////////////

void LeabraLayerSpec::PhaseInit(LeabraLayer* lay, int phase) {
  if(!(lay->ext_flag & Unit::TARG))	// only process target layers..
    return;
  if(phase == LeabraTrial::MINUS_PHASE)
    lay->UnSetExtFlag(Unit::EXT);
  else
    lay->SetExtFlag(Unit::EXT);
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->PhaseInit(lay, phase);
  }
}

void LeabraLayerSpec::DecayEvent(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayState(lay, decay.event);
  }
}
  
void LeabraLayerSpec::DecayPhase(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayState(lay, decay.phase);
  }
}

void LeabraLayerSpec::DecayPhase2(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayState(lay, decay.phase2);
  }
}
  
void LeabraLayerSpec::ExtToComp(LeabraLayer* lay) {
  if(!(lay->ext_flag & Unit::EXT))	// only process ext
    return;
  lay->ext_flag = Unit::COMP;	// totally reset to comparison
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->ExtToComp(lay);
}

void LeabraLayerSpec::TargExtToComp(LeabraLayer* lay) {
  if(!(lay->ext_flag & Unit::TARG_EXT))	// only process w/ external input
    return;
  lay->ext_flag = Unit::COMP;	// totally reset to comparison
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->TargExtToComp(lay);
}

void LeabraLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if(set_both) {
    lay->acts_m = lay->acts;
    lay->acts_p = lay->acts;
    lay->acts_dif.avg = 0.0f;
    lay->acts_dif.max = 0.0f;
    lay->phase_dif_ratio = 1.0f;

    if(lay->units.gp.size > 0) {
      int g;
      for(g=0; g<lay->units.gp.size; g++) {
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	rugp->acts_m = rugp->acts;
	rugp->acts_p = rugp->acts;
	rugp->acts_dif.avg = 0.0f;
	rugp->acts_dif.max = 0.0f;
	rugp->phase_dif_ratio = 1.0f;
      }
    }
  }
  else {
    if(trl->phase == LeabraTrial::MINUS_PHASE)
      lay->acts_m = lay->acts;
    else
      lay->acts_p = lay->acts;
    lay->acts_dif.avg = lay->acts_p.avg - lay->acts_m.avg;
    lay->acts_dif.max = lay->acts_p.max - lay->acts_m.max;
    if(lay->acts_p.avg > 0.0f)
      lay->phase_dif_ratio = lay->acts_m.avg / lay->acts_p.avg;
    else
      lay->phase_dif_ratio = 1.0f;

    if(lay->units.gp.size > 0) {
      int g;
      for(g=0; g<lay->units.gp.size; g++) {
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	if(trl->phase == LeabraTrial::MINUS_PHASE)
	  rugp->acts_m = rugp->acts;
	else
	  rugp->acts_p = rugp->acts;
	rugp->acts_dif.avg = rugp->acts_p.avg - rugp->acts_m.avg;
	rugp->acts_dif.max = rugp->acts_p.max - rugp->acts_m.max;
	if(rugp->acts_p.avg > 0.0f)
	  rugp->phase_dif_ratio = rugp->acts_m.avg / rugp->acts_p.avg;
	else
	  rugp->phase_dif_ratio = 1.0f;
      }
    }
  }

  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      LeabraUnit* u;
      taLeafItr i;
      FOR_ITR_EL(LeabraUnit, u, rugp->, i)
	u->PostSettle(lay, (LeabraInhib*)rugp, trl, set_both);
    }
  }
  else {
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i)
      u->PostSettle(lay, (LeabraInhib*)lay, trl, set_both);
  }
  lay->prv_phase = trl->phase;	// record previous phase
}

//////////////////////////////////////////
//	Stage 6: Learning 		//
//////////////////////////////////////////

void LeabraLayerSpec::AdaptKWTAPt(LeabraLayer* lay) {
  if((lay->ext_flag & Unit::EXT) && !(lay->ext_flag & Unit::TARG))
    return;			// don't adapt points for input-only layers
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      // use minus phase activations to adapt
      rugp->adapt_pt.avg_avg += adapt_pt.a_dt * (rugp->acts_m.avg - rugp->adapt_pt.avg_avg);
      float dif = rugp->adapt_pt.avg_avg - rugp->kwta.pct;
      if(dif < -adapt_pt.tol) {	// average is less than target
	// so reduce the point towards lower value
	rugp->adapt_pt.i_kwta_pt += adapt_pt.pt_dt * 
	  ((i_kwta_pt - adapt_pt.mx_d) - rugp->adapt_pt.i_kwta_pt);
      }
      else if(dif > adapt_pt.tol) {	// average is more than target
	// so increase point towards higher value
	rugp->adapt_pt.i_kwta_pt += adapt_pt.pt_dt * 
	  ((i_kwta_pt + adapt_pt.mx_d) - rugp->adapt_pt.i_kwta_pt);
      }
    }
  }
  lay->adapt_pt.avg_avg += adapt_pt.a_dt * (lay->acts_m.avg - lay->adapt_pt.avg_avg);
  float dif = lay->adapt_pt.avg_avg - lay->kwta.pct;
  if(dif < -adapt_pt.tol) {	// average is less than target
    // so reduce the point towards lower value
    lay->adapt_pt.i_kwta_pt += adapt_pt.pt_dt * 
      ((i_kwta_pt - adapt_pt.mx_d) - lay->adapt_pt.i_kwta_pt);
  }
  else if(dif > adapt_pt.tol) {	// average is more than target
    // so increase point towards higher value
    lay->adapt_pt.i_kwta_pt += adapt_pt.pt_dt * 
      ((i_kwta_pt + adapt_pt.mx_d) - lay->adapt_pt.i_kwta_pt);
  }
}

void LeabraLayerSpec::AdaptThreshold(LeabraLayer* lay) {
  if(lay->ext_flag & (Unit::EXT | Unit::TARG))
    return;			// don't adapt thresholds for clamped layers
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      adapt_thr_list.size = 0;

      LeabraUnit* u;
      taLeafItr ui;
      FOR_ITR_EL(LeabraUnit, u, rugp->, ui) {
	if(u->AdaptThreshTest(lay, (LeabraInhib*)rugp))
	  adapt_thr_list.AddEl_((void*)u);
      }
      if(adapt_thr_list.size > rugp->kwta.adth_k)
	adapt_thr_list.Permute();
      int mxi = MIN(adapt_thr_list.size, rugp->kwta.adth_k);
      int i;
      for(i=0;i<mxi;i++)
	adapt_thr_list.FastEl(i)->AdaptThresh(lay, (LeabraInhib*)rugp);
    }
  }
  else {
    adapt_thr_list.size = 0;

    LeabraUnit* u;
    taLeafItr ui;
    FOR_ITR_EL(LeabraUnit, u, lay->units., ui) {
      if(u->AdaptThreshTest(lay, (LeabraInhib*)lay))
	adapt_thr_list.AddEl_((void*)u);
    }
    if(adapt_thr_list.size > lay->kwta.adth_k)
      adapt_thr_list.Permute();
    int mxi = MIN(adapt_thr_list.size, lay->kwta.adth_k);
    int i;
    for(i=0;i<mxi;i++)
      adapt_thr_list.FastEl(i)->AdaptThresh(lay, (LeabraInhib*)lay);
  }
  adapt_thr_list.size = 0;
}
  
void LeabraLayerSpec::Compute_dWt(LeabraLayer* lay) {
  lay->needs_wt_updt = true;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_dWt(lay);
  AdaptThreshold(lay);
  AdaptKWTAPt(lay);
}

void LeabraLayerSpec::Compute_dWt_post(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_dWt_post(lay);
}

void LeabraLayerSpec::Compute_WtFmLin(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_WtFmLin(lay);
}

void LeabraLayer::UpdateWeights() {
  if(!needs_wt_updt) return;
  Layer::UpdateWeights();
  needs_wt_updt = false;
}

//////////////////////////
// 	LeabraLayer	//
//////////////////////////
  
void AvgMaxVals::Initialize() {
  avg = max = 0.0f;
}

void KWTAVals::Initialize() {
  k = 12;
  pct = .25f;
  pct_c = .75f;
  adth_k = 1;
  k_ithr = 0.0f;
  k1_ithr = 0.0f;
  ithr_r = 0.0f;
}

void KWTAVals::Copy_(const KWTAVals& cp) {
  k = cp.k;
  pct = cp.pct;
  pct_c = cp.pct_c;
  adth_k = cp.adth_k;
}

void KWTAVals::Compute_Pct(int n_units) {
  if(n_units > 0)
    pct = (float)k / (float)n_units;
  else
    pct = 0.0f;
  pct_c = 1.0f - pct;
}

void KWTAVals::Compute_AdthK(float adth_pct) {
  adth_k = (int)(adth_pct * (float)k);
  adth_k = MAX(adth_k, 1);
}

void KWTAVals::Compute_IThrR() {
  if(k1_ithr <= 0.0f)
    ithr_r = 0.0f;
  else 
    ithr_r = logf(k_ithr / k1_ithr);
}

void AdaptKWTAVals::Initialize() {
  avg_avg = 0.0f;
  i_kwta_pt = 0.0f;
}

void AdaptKWTAVals::Copy_(const AdaptKWTAVals& cp) {
  avg_avg = cp.avg_avg;
  i_kwta_pt = cp.i_kwta_pt;
}

void InhibVals::Initialize() {
  kwta = 0.0f;
  g_i = 0.0f;
  g_i_orig = 0.0f;
}

void InhibVals::Copy_(const InhibVals& cp) {
  kwta = cp.kwta;
  g_i = cp.g_i;
  g_i_orig = cp.g_i_orig;
}

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

void LeabraInhib::Inhib_Initialize() {
  kwta.k = 1;
  kwta.pct = .25;
  kwta.pct_c = .75;
  i_val.Initialize();
  phase_dif_ratio = 1.0f;
}

void LeabraInhib::Inhib_InitState(LeabraLayerSpec*) {
  i_val.Initialize();
  netin.Initialize();
  acts.Initialize();
  un_g_i.Initialize();
}

void LeabraLayer::Initialize() {
  spec.SetBaseType(&TA_LeabraLayerSpec);
  units.SetBaseType(&TA_LeabraUnit);
  units.gp.SetBaseType(&TA_LeabraUnit_Group);
  unit_spec.SetBaseType(&TA_LeabraUnitSpec);
  layer_links.SetBaseType(&TA_LayerLink);

  Inhib_Initialize();
  stm_gain = .5f;
  hard_clamped = false;
  needs_wt_updt = false;
  prv_phase = LeabraTrial::MINUS_PHASE;
  td = 0.0f;
  td_updt = false;
}  

void LeabraLayer::InitLinks() {
  Layer::InitLinks();
  taBase::Own(netin, this);
  taBase::Own(acts, this);

  taBase::Own(acts_p, this);
  taBase::Own(acts_m, this);
  taBase::Own(acts_dif, this);

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);
  taBase::Own(adapt_pt, this);

  taBase::Own(layer_links, this);
  spec.SetDefaultSpec(this);
  units.gp.SetBaseType(&TA_LeabraUnit_Group);
}

void LeabraLayer::CutLinks() {
  Layer::CutLinks();
  spec.CutLinks();
  layer_links.Reset();		// get rid of the links...
}

void LeabraInhib::Inhib_Copy_(const LeabraInhib& cp) {
  netin = cp.netin;
  acts = cp.acts;
  acts_p = cp.acts_p;
  acts_m = cp.acts_m;
  acts_dif = cp.acts_dif;
  phase_dif_ratio = cp.phase_dif_ratio;
  kwta = cp.kwta;
  i_val = cp.i_val;
  un_g_i = cp.un_g_i;
}

void LeabraLayer::Copy_(const LeabraLayer& cp) {
  Inhib_Copy_(cp);
  spec = cp.spec;
  layer_links = cp.layer_links;
  stm_gain = cp.stm_gain;
  hard_clamped = cp.hard_clamped;
}

void LeabraLayer::ResetSortBuf() {
  Inhib_ResetSortBuf();		// reset sort buf after any edit..
  if(units.gp.size > 0) {
    int g;
    for(g=0; g<units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)units.gp.FastEl(g);
      rugp->Inhib_ResetSortBuf();
    }
  }
}

void LeabraLayer::UpdateAfterEdit() {
  Layer::UpdateAfterEdit();
  ResetSortBuf();
}

bool LeabraLayer::SetLayerSpec(LayerSpec* sp) {
  if(sp == NULL)	return false;
  if(sp->CheckObjectType(this))
    spec.SetSpec((LeabraLayerSpec*)sp);
  else
    return false;
  return true;
} 

void LeabraLayer::Build() {
  ResetSortBuf();
  Layer::Build();
}

void LeabraUnit_Group::Initialize() {
  Inhib_Initialize();
}

void LeabraUnit_Group::InitLinks() {
  Unit_Group::InitLinks();
  taBase::Own(netin, this);
  taBase::Own(acts, this);

  taBase::Own(acts_p, this);
  taBase::Own(acts_m, this);
  taBase::Own(acts_dif, this);

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);
  taBase::Own(adapt_pt, this);
}


//////////////////////////
//  	Procs		//
//////////////////////////

//////////////////////////
// 	CycleProcess	//
//////////////////////////

void LeabraCycle::Initialize() {
  sub_proc_type = NULL;
  leabra_settle = NULL;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  min_con_group = &TA_LeabraCon_Group;
}

void LeabraCycle::CutLinks() {
  CycleProcess::CutLinks();
  leabra_settle = NULL;
}

void LeabraCycle::UpdateAfterEdit() {
  CycleProcess::UpdateAfterEdit();
  leabra_settle = (LeabraSettle*)FindSuperProc(&TA_LeabraSettle);
}

void LeabraCycle::Compute_Net() {
  network->InitDelta();		// this is done first because of sender-based net
  network->Compute_Net();
}

void LeabraCycle::Compute_Clamp_NetAvg() {
  if(leabra_settle == NULL)
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Clamp_NetAvg(leabra_settle->cycle.val,
			      (int)leabra_settle->leabra_trial->phase);
  }
}  

void LeabraCycle::Compute_Inhib() {
  if(leabra_settle == NULL)
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Inhib(leabra_settle->cycle.val,
			     (int)leabra_settle->leabra_trial->phase);
  }
}  

void LeabraCycle::Compute_Act() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Act(leabra_settle->cycle.val,
		     (int)leabra_settle->leabra_trial->phase);
  }
}

void LeabraCycle::Loop() {
  if(leabra_settle == NULL)	return;
  Compute_Net();
  Compute_Clamp_NetAvg();
  Compute_Inhib();
  Compute_Act();
}

//////////////////////////
// 	SettleProcess	//
//////////////////////////

void LeabraSettle::Initialize() {
  sub_proc_type = &TA_LeabraCycle;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  leabra_trial = NULL;
  cycle.max = 60;
  min_cycles = 15;
}

void LeabraSettle::InitLinks() {
  SettleProcess::InitLinks();
  if(!taMisc::is_loading && (loop_stats.size == 0)) {
    LeabraMaxDa* da = (LeabraMaxDa*)loop_stats.NewEl(1, &TA_LeabraMaxDa);
    da->UpdateAfterEdit();	// get it redy to go
  }
}

void LeabraSettle::CutLinks() {
  SettleProcess::CutLinks();
  leabra_trial = NULL;
}

void LeabraSettle::UpdateAfterEdit() {
  SettleProcess::UpdateAfterEdit();
  leabra_trial = (LeabraTrial*)FindSuperProc(&TA_LeabraTrial);
}

void LeabraSettle::Compute_Active_K() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Active_K();	// this gets done at the outset..
  }
}
  
void LeabraSettle::DecayEvent() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayEvent();
  }
}

void LeabraSettle::DecayPhase() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayPhase();
  }
}

void LeabraSettle::DecayPhase2() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayPhase2();
  }
}

void LeabraSettle::PhaseInit() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->PhaseInit(leabra_trial->phase);
  }
}

void LeabraSettle::ExtToComp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->ExtToComp();
  }
}

void LeabraSettle::TargExtToComp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->TargExtToComp();
  }
}

void LeabraSettle::Compute_HardClamp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_HardClamp(leabra_trial);
  }
}

void LeabraSettle::Compute_NetScale() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_NetScale(leabra_trial->phase);
  }
}

void LeabraSettle::Send_ClampNet() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Send_ClampNet(leabra_trial->phase);
  }
}

void LeabraSettle::PostSettle() {
  bool set_both = false;
  if((leabra_trial != NULL) && (leabra_trial->epoch_proc != NULL)) {
    if((leabra_trial->phase_order == LeabraTrial::PLUS_ONLY) ||
       (leabra_trial->epoch_proc->wt_update == EpochProcess::TEST))
      set_both = true;
  }
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->PostSettle(leabra_trial, set_both);
  }
}

void LeabraSettle::Compute_WtFmLin() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_WtFmLin();
  }
}

void LeabraSettle::Compute_dWt() {
  leabra_trial->Compute_dWt();	// fctn is complicated, use trial version..
  Compute_WtFmLin();		// if being computed here, means we need to revert to real wts
}

void LeabraSettle::Init_impl() {
  SettleProcess::Init_impl();
  if((leabra_trial == NULL) || (leabra_trial->cur_event == NULL) || (network == NULL))
    return;

  if(leabra_trial->cur_event->InheritsFrom(TA_DurEvent)) {
    cycle.max = (int)((DurEvent*)leabra_trial->cur_event)->duration;
  }

  if(leabra_trial->phase_no == 3) { // second plus phase
    DecayPhase();
  }
  else if(leabra_trial->phase_no == 2) {
    DecayPhase2();		// decay before 2nd phase set
    if(leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_NOTHING) {
      TargExtToComp();		// all external input is now 'comparison'
    }
  }
  else if(leabra_trial->phase_no == 1) {
    if(leabra_trial->phase_order == LeabraTrial::PLUS_NOTHING) { // actually a nothing phase
      DecayPhase2();
      TargExtToComp();
    }
    else
      DecayPhase();		    // prepare for next phase
  }
  else if(leabra_trial->phase_no == 0) { // initial phase, apply patterns
    network->InitExterns();
    if(leabra_trial->cur_event != NULL)
      leabra_trial->cur_event->ApplyPatterns(network);
    Compute_Active_K();		// compute here because could depend on pat_n
  }
  PhaseInit();
  
  Compute_HardClamp();		// first clamp all hard-clamped input acts
  Compute_NetScale();		// and then compute net scaling
  Send_ClampNet();		// and send net from clamped inputs
}	

void LeabraSettle::Final() {
  PostSettle();
  if(leabra_trial == NULL)	return;
  // compute weight changes at end of second settle..
  if((leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_NOTHING) &&
     (leabra_trial->phase_no.val == 1))
    Compute_dWt();
  if((leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_PLUS) &&
     (leabra_trial->phase_no.val == 1)) {
    if(leabra_trial->first_plus_dwt == LeabraTrial::NO_DWT)
      return;
    if(leabra_trial->first_plus_dwt == LeabraTrial::ALL_DWT)
      Compute_dWt();
    else {			// just AC unit
      LeabraLayer* lay;
      taLeafItr l;
      FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
	if(lay->lesion) continue;
	if(lay->spec.spec->InheritsFrom(TA_LeabraACLayerSpec)) {
	  lay->Compute_dWt();
	  lay->Compute_WtFmLin();
	}
      }
    }
  }
}
  
//////////////////////////
// 	TrialProcess	//
//////////////////////////

void LeabraTrial::Initialize() {
  sub_proc_type = &TA_LeabraSettle;
  phase_order = MINUS_PLUS;
  phase = MINUS_PHASE;
  phase_no.SetMax(2);
  no_plus_stats = true;
  no_plus_test = true;
  trial_init = INIT_STATE;
  first_plus_dwt = ALL_DWT;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  min_con_group =  &TA_LeabraCon_Group;
  min_con =  &TA_LeabraCon;
}

void LeabraTrial::InitLinks() {
  TrialProcess::InitLinks();
  taBase::Own(phase_no, this);
  if(!taMisc::is_loading && (loop_stats.size == 0)) {
    LeabraAeSE_Stat* st = (LeabraAeSE_Stat*)loop_stats.NewEl(1, &TA_LeabraAeSE_Stat);
    st->CreateAggregates(Aggregate::SUM);
    st->UpdateAfterEdit();
    CyclesToSettle* cst = (CyclesToSettle*)loop_stats.NewEl(1, &TA_CyclesToSettle);
    cst->CreateAggregates(Aggregate::AVG);
    cst->UpdateAfterEdit();
  }
}

void LeabraTrial::Copy_(const LeabraTrial& cp) {
  phase_no = cp.phase_no;
  phase = cp.phase;
  no_plus_stats = cp.no_plus_stats;
  no_plus_test = cp.no_plus_test;
  trial_init = cp.trial_init;
}

void LeabraTrial::UpdateAfterEdit() {
  if((epoch_proc != NULL) && 
     (epoch_proc->wt_update != EpochProcess::TEST) &&
     (epoch_proc->wt_update != EpochProcess::ON_LINE)) {
    taMisc::Error("Leabra requires that weight updates be made ON_LINE, so the",
		  "epoch process wt_update parameter is being reset to ON_LINE");
    epoch_proc->wt_update = EpochProcess::ON_LINE;
  }
  TrialProcess::UpdateAfterEdit();
}

void LeabraTrial::C_Code() {
  bool critval;

  if(re_init) {
    Init();
    InitProcs();
  }

  do {
    Loop();			// user defined code goes here
    UpdateCounters();
    LoopProcs();		// check/run loop procs (using mod based on counter)
    if(!no_plus_stats || (phase == MINUS_PHASE)) {
      LoopStats();		// udpate in-loop stats
      if(log_loop)
	UpdateLogs();		// generate log output
    }
    UpdateState();		// update process state vars

    critval = Crit();
    if(!critval) {		// if at critera, going to quit anyway, so don't
      StepCheck();		// check for stepping
      StopCheck();		// check for stopping
    }
  }
  while(!critval);

  Final();
  FinalProcs();
  FinalStats();
  if(!log_loop)
    UpdateLogs();
  UpdateDisplays();		// update displays after the loop
  SetReInit(true);		// made it through the loop
}

void LeabraTrial::InitThresh() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->InitThresh();
  }
}

void LeabraTrial::InitState() {
  network->InitState();
}

void LeabraTrial::SetCurLrate() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->SetCurLrate(network->epoch);
  }
}

void LeabraTrial::DecayState() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayEvent();
  }
}

void LeabraTrial::EncodeState() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)     continue;
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i)
      u->EncodeState(lay);
  }
}

void LeabraTrial::Compute_dWt_post() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_dWt_post();
  }
}

void LeabraTrial::Compute_dWt() {
  if(cur_event == NULL) return;

  if(cur_event->spec->InheritsFrom(TA_EncLrnEventSpec)) {
    EncLrnEventSpec* es = (EncLrnEventSpec*)cur_event->spec.spec;
    if((es->enc_lrn == EncLrnEventSpec::LEARN) ||
       (es->enc_lrn == EncLrnEventSpec::LRN_ENC)) {
      network->Compute_dWt();
      //      Compute_dWt_post();		// post not currently used..
    }
    if((es->enc_lrn == EncLrnEventSpec::ENCODE) ||
       (es->enc_lrn == EncLrnEventSpec::LRN_ENC)) {
      EncodeState();
    }
  }
  else {
    network->Compute_dWt();
    // Compute_dWt_post();		// post not currently used..
    EncodeState();
  }
}

void LeabraTrial::UpdateWeights() {
  network->UpdateWeights();
}

void LeabraTrial::Init_impl() {
  TrialProcess::Init_impl();

  phase = MINUS_PHASE;
  phase_no.SetMax(2);
  bool is_testing = false;

  if(no_plus_test && (epoch_proc != NULL) && 
     (epoch_proc->wt_update == EpochProcess::TEST))
  {
    phase_no.SetMax(1);		// just do one loop (the minus phase)
    is_testing = true;
  }
  if((cur_event != NULL) && 
     (cur_event->spec->InheritsFrom(TA_PhaseOrderEventSpec)))
  {
    PhaseOrderEventSpec* es = (PhaseOrderEventSpec*)cur_event->spec.spec;
    if(es->phase_order == PhaseOrderEventSpec::MINUS_PLUS)
      phase = MINUS_PHASE;
    else if(es->phase_order == PhaseOrderEventSpec::PREV_PLUS_MINUS) {
      phase = MINUS_PHASE;
      phase_no.SetMax(1);
    }
    else if(es->phase_order == PhaseOrderEventSpec::PLUS_MINUS) {
      phase = PLUS_PHASE;
      if(is_testing)
	phase_no.SetMax(2);	// need to present plus phase first, then minus..
    }
    else if(es->phase_order == PhaseOrderEventSpec::PREV_MINUS_PLUS) {
      phase = PLUS_PHASE;
      phase_no.SetMax(1);
    }
  }
  else if(!is_testing) {
    if(phase_order == PLUS_ONLY) {
      phase_no.SetMax(1);
      phase = PLUS_PHASE;
    }
    else if((phase_order == MINUS_PLUS_NOTHING) || (phase_order == MINUS_PLUS_PLUS)) {
      phase_no.SetMax(3);
    }
    else if(phase_order == PLUS_NOTHING) {
      phase_no.SetMax(2);
      phase = PLUS_PHASE;
    }
  }

  if(trial_init == INIT_STATE)
    InitState();
  else if(trial_init == DECAY_STATE)
    DecayState();

  SetCurLrate();
}

void LeabraTrial::UpdateState() {
  TrialProcess::UpdateState();

  if((cur_event != NULL) && (cur_event->spec->InheritsFrom(TA_PhaseOrderEventSpec))) {
    PhaseOrderEventSpec* es = (PhaseOrderEventSpec*)cur_event->spec.spec;
    if(es->phase_order == PhaseOrderEventSpec::MINUS_PLUS)
      phase = PLUS_PHASE;
    else if(es->phase_order == PhaseOrderEventSpec::PLUS_MINUS)
      phase = MINUS_PHASE;
  }
  else {
    if(phase_order == PLUS_NOTHING) {
      phase = MINUS_PHASE;
    }
    else {
      if(phase_no.val == 1)
	phase = PLUS_PHASE;
      else {
	if(phase_order == MINUS_PLUS_NOTHING)
	  phase = MINUS_PHASE;
	else {
	  phase = PLUS_PHASE;
	}
      }
    }
  }
}

void LeabraTrial::Final() {
  TrialProcess::Final();	// do anything else

  if((epoch_proc != NULL) && (epoch_proc->wt_update != EpochProcess::TEST)) {
    Compute_dWt();
    UpdateWeights();		// always update weights after each trial of learning to have
				// weights of correct sign for viewing (checks in place for 
  }
}

void LeabraTrial::GetCntrDataItems() {
  if(cntr_items.size >= 2)
    return;
  TrialProcess::GetCntrDataItems();
  DataItem* it = (DataItem*)cntr_items.New(1,&TA_DataItem);
  it->SetNarrowName("Phase");
  it->is_string = true;
}

void LeabraTrial::GenCntrLog(LogData* ld, bool gen) {
  TrialProcess::GenCntrLog(ld, gen);
  if(gen) {
    if(cntr_items.size < 2)
      GetCntrDataItems();
    if(phase == MINUS_PHASE)
      ld->AddString(cntr_items.FastEl(1), "-");
    else
      ld->AddString(cntr_items.FastEl(1), "+");
  }
}


//////////////////////////
// 	Max Da Stat	//
//////////////////////////

void LeabraMaxDa::Initialize() {
  settle_proc = NULL;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  net_agg.op = Aggregate::MAX;
  da.stopcrit.flag = true;	// defaults
  da.stopcrit.val = .005f;
  da.stopcrit.cnt = 1;
  da.stopcrit.rel = CritParam::LESSTHANOREQUAL;
  has_stop_crit = true;
  da_type = INET_DA;
  inet_scale = 1.0f;
  lay_avg_thr = .01f;
}

void LeabraMaxDa::UpdateAfterEdit() {
  Stat::UpdateAfterEdit();
  if(own_proc == NULL) return;
  settle_proc = (LeabraSettle*)own_proc->FindProcOfType(&TA_LeabraSettle);
}

void LeabraMaxDa::CutLinks() {
  settle_proc = NULL;
  Stat::CutLinks();
}

void LeabraMaxDa::InitStat() {
  da.InitStat(InitStatVal());
  InitStat_impl();
}

void LeabraMaxDa::Init() {
  da.Init();
  Init_impl();
}

bool LeabraMaxDa::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return da.Crit();
}

void LeabraMaxDa::Network_Init() {
  InitStat();
}

void LeabraMaxDa::Unit_Stat(Unit* unit) {
  LeabraUnit* lu = (LeabraUnit*)unit;
  float fda;
  if(da_type == DA_ONLY)
    fda = fabsf(lu->da);
  else if(da_type == INET_ONLY)
    fda = fabsf(inet_scale * lu->I_net);
  else {
    LeabraLayer* blay = (LeabraLayer*)((Unit_Group*)unit->owner)->own_lay;
    if(blay->acts.avg <= lay_avg_thr)
      fda = fabsf(inet_scale * lu->I_net);
    else
      fda = fabsf(lu->da);
  }
  net_agg.ComputeAgg(&da, fda);
}

void LeabraMaxDa::Network_Stat() {
  if(settle_proc == NULL) return;
  if(settle_proc->cycle.val < settle_proc->min_cycles)
    da.val = MAX(da.val, da.stopcrit.val + .01); // keep above thresh!
}


//////////////////////////
//  	Ae Stat		//
//////////////////////////

void LeabraAeSE_Stat::Initialize() {
  targ_or_comp = Unit::COMP_TARG;
  trial_proc = NULL;
  tolerance = .5;
}

void LeabraAeSE_Stat::UpdateAfterEdit() {
  SE_Stat::UpdateAfterEdit();
  if(own_proc == NULL) return;
  trial_proc = (LeabraTrial*)own_proc->FindProcOfType(&TA_LeabraTrial);
  if(time_agg.real_stat == NULL)
    return;
  LeabraAeSE_Stat* rstat = (LeabraAeSE_Stat*)time_agg.real_stat;
  // don't rename unless one or the other
  if((rstat->targ_or_comp != Unit::TARG) && (rstat->targ_or_comp != Unit::COMP))
    return;
  MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
  name += String("_") + md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
}

void LeabraAeSE_Stat::NameStatVals() {
  Stat::NameStatVals();
  if(time_agg.real_stat == NULL)
    return;
  LeabraAeSE_Stat* rstat = (LeabraAeSE_Stat*)time_agg.real_stat;
  // don't rename unless one or the other
  if((rstat->targ_or_comp != Unit::TARG) && (rstat->targ_or_comp != Unit::COMP))
    return;
  MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
  String vlnm = md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
  vlnm.downcase();
  se.name += String("_") + vlnm.at(0,4);
}

void LeabraAeSE_Stat::CutLinks() {
  trial_proc = NULL;
  SE_Stat::CutLinks();
}

void LeabraAeSE_Stat::Init() {
  // don't init if it's not the right time!
  if((time_agg.from == NULL) && (trial_proc != NULL)) {
    // run targ-only comparisons in the first minus phase only
    if((targ_or_comp == Unit::TARG) && (trial_proc->phase_no.val != 1))
      return;
    // run comp-only comparisons in the last minus phase only
    if((targ_or_comp == Unit::COMP) && (trial_proc->phase_no.val != 3))
      return;
  }
  SE_Stat::Init();
}

void LeabraAeSE_Stat::Network_Run() {
  if(trial_proc != NULL) {
    // run targ-only comparisons in the first minus phase only
    if((targ_or_comp == Unit::TARG) && (trial_proc->phase_no.val != 1))
      return;
    // run comp-only comparisons in the last minus phase only
    if((targ_or_comp == Unit::COMP) && (trial_proc->phase_no.val != 3))
      return;
  }
  SE_Stat::Network_Run();
}

void LeabraAeSE_Stat::Unit_Stat(Unit* unit) {
  if(!(unit->ext_flag & targ_or_comp))
    return;
  float tmp = fabsf(((LeabraUnit*)unit)->act_eq - unit->targ);
  if(tmp >= tolerance)
    tmp *= tmp;
  else
    tmp = 0.0f;
  net_agg.ComputeAgg(&se, tmp);
}

////////////////////////////////
// 	LeabraGoodStat        //
////////////////////////////////

void LeabraGoodStat::Initialize() {
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  net_agg.op = Aggregate::AVG;
  subtr_inhib = false;
}

void LeabraGoodStat::Destroy() {
}

void LeabraGoodStat::Copy_(const LeabraGoodStat& cp) {
  subtr_inhib = cp.subtr_inhib;
  hrmny = cp.hrmny;
  strss = cp.strss;
  gdnss = cp.gdnss;
}

void LeabraGoodStat::InitStat() {
  hrmny.InitStat(InitStatVal());
  strss.InitStat(InitStatVal());
  gdnss.InitStat(InitStatVal());
  InitStat_impl();
}

void LeabraGoodStat::Init() {
  hrmny.Init();
  strss.Init();
  gdnss.Init();
  Init_impl();
}

bool LeabraGoodStat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  if(hrmny.Crit())	return true;
  if(strss.Crit())	return true;
  if(gdnss.Crit())	return true;
  return false;
}

void LeabraGoodStat::Network_Init() {
  InitStat();
}

void LeabraGoodStat::Unit_Stat(Unit* un) {
  LeabraUnit* lun = (LeabraUnit*) un;

  float harm = lun->net;
  if(subtr_inhib)
    harm -= lun->gc.i;

  net_agg.ComputeAggNoUpdt(&hrmny, lun->act * harm);

  float act_c = 1.0f - lun->act_eq;
  float stress = 0.0f;
  if(lun->act_eq > 0.0f) stress += lun->act_eq * logf(lun->act_eq);
  if(act_c > 0.0f) stress += act_c * logf(act_c);
  net_agg.ComputeAgg(&strss, -stress);
}

void LeabraGoodStat::Network_Stat() {
  // this is not aggregated, because it is the difference of two sum terms..
  gdnss.val = hrmny.val - strss.val;
}

//////////////////////////
// 	WrongOnStat	//
//////////////////////////

void WrongOnStat::Initialize() {
  threshold = .5f;
  trg_lay = NULL;
}

void WrongOnStat::InitLinks() {
  Stat::InitLinks();
}

void WrongOnStat::CutLinks() {
  Stat::CutLinks();
  taBase::DelPointer((TAPtr*)&trg_lay);
}

void WrongOnStat::NameStatVals() {
  Stat::NameStatVals();
  wrng.AddDispOption("MIN=0, ");
  wrng.AddDispOption("TEXT, ");
}

void WrongOnStat::InitStat() {
  float init_val = InitStatVal();
  wrng.InitStat(init_val);
  InitStat_impl();
}

void WrongOnStat::Init() {
  wrng.Init();
  Init_impl();
}

bool WrongOnStat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return wrng.Crit();
}

bool WrongOnStat::CheckLayerInNet() {
  bool ok = Stat::CheckLayerInNet();
  if(!ok)
    return false;
  if(trg_lay == NULL) return true;
  if(trg_lay->own_net == network)
    return true;

  taMisc::Error("*** WrongOnStat:", name, "layer:", trg_lay->name,
		 "is not in current network, finding one of same name...");
  Layer* nw_lay = (Layer*)network->layers.FindName(trg_lay->name);
  if(nw_lay == NULL) {
    taMisc::Error("==> could not find layer with same name in current network, aborting");
    return false;
  }
  taBase::SetPointer((TAPtr*)&trg_lay, nw_lay);
  return true;
}
 
void WrongOnStat::Network_Init() {
  InitStat();
}

void WrongOnStat::Layer_Run() {
  if(layer != NULL) {
    if(!CheckLayerInNet()) return;
    if(layer->lesion)      return;
    if(trg_lay == NULL) {
      taMisc::Error("*** WrongOnStat:", name, "trg_lay must be set!");
      return;
    }
    Layer_Init(layer);
    Unit_Run(layer);
    Layer_Stat(layer);
    return;
  }
  taMisc::Error("*** WrongOnStat:", name, "layer must be set!");
  return;
}

void WrongOnStat::Unit_Run(Layer* lay) {
  Unit* unit, *tu;
  taLeafItr i, ti;
  bool no_above_thr = true;
  for(unit = (Unit*)lay->units.FirstEl(i), tu = (Unit*)trg_lay->units.FirstEl(ti);
      (unit != NULL) && (tu != NULL);
      unit = (Unit*)lay->units.NextEl(i), tu = (Unit*)trg_lay->units.NextEl(ti)) {
    Unit_Init(unit);
    float err = 0.0f;
    if(unit->act > threshold) {
      no_above_thr = false;
      if(tu->act < threshold)
	err = 1.0f;
    }
    net_agg.ComputeAgg(&wrng, err);
  }
  if(no_above_thr)		// get a point off if nobody above threshold!
    net_agg.ComputeAgg(&wrng, 1.0f);
}

//////////////////////////////////////////
// 	Reinforcement Learning		//
//////////////////////////////////////////

void ACRewSpec::Initialize() {
  discount = 1.0f;
  inv_disc = 1.0f / discount;
  reset = true;
  val = .0f;
  immediate = false;
}

void ACRewSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  inv_disc = 1.0f / discount;
}

void SoftMaxSpec::Initialize() {
  use = false;
  gain = 5.0f;
}

void LeabraACLayerSpec::Initialize() {
  un_gain_spec.SetBaseType(&TA_LeabraUnitSpec);
  mod_type = MOD_NOTHING;
  un_gain_range.min = 400.0f;
  un_gain_range.max = 800.0f;
  un_gain_range.UpdateAfterEdit();
  err_delta = .2f;
}

void LeabraACLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(rew, this);
  taBase::Own(un_gain_range, this);
  un_gain_spec.SetDefaultSpec(this);
  taBase::Own(softmax, this);
}

void LeabraACLayerSpec::CutLinks() {
  LeabraLayerSpec::CutLinks();
  un_gain_spec.CutLinks();
}

void LeabraACLayerSpec::UpdateAfterEdit() {
  LeabraLayerSpec::UpdateAfterEdit();
  rew.UpdateAfterEdit();
}

void LeabraACLayerSpec::UpdateUnitGain(LeabraUnitSpec* us, float new_gain) {
  us->act.gain = new_gain;
  us->CreateNXX1Fun();
  LeabraUnitSpec* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnitSpec, u, us->children., i)
    UpdateUnitGain(u, new_gain);
}

void LeabraACLayerSpec::UpdateUnitErr(LeabraUnitSpec* us, float new_err) {
  us->phase_delta = new_err;
  LeabraUnitSpec* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnitSpec, u, us->children., i)
    UpdateUnitErr(u, new_err);
}

void LeabraACLayerSpec::InitState(LeabraLayer* lay) {
  if((mod_type == MOD_GAIN) && (un_gain_spec.spec != NULL)) {
    LeabraUnitSpec* us = (LeabraUnitSpec*)un_gain_spec.spec;
    UpdateUnitGain(us, un_gain_range.MidPoint());
  }
  LeabraLayerSpec::InitState(lay);
}

void LeabraACLayerSpec::ExtToComp(LeabraLayer* lay) {
  // do nothing!
}

// for 2 opposing units, 1st unit is the "reward" predictor,
// and second is the "no-reward" predictor.

// TD signal could be based on behavior of the 1st unit (act_dif)
// or rew->act_dif - norew->act_dif

void LeabraACLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* acu = (LeabraUnit*)lay->units.Leaf(0);	// get the unit
  if(acu == NULL) return;
  LeabraUnit* acu2 = NULL;
  if(lay->units.leaves > 1)
    acu2 = (LeabraUnit*)lay->units.Leaf(1);	// get 2nd unit

  // modulate activity by prior plus-phase value if applicable
  if((mod_type != MOD_NOTHING) && (un_gain_spec.spec != NULL)) {

    float td = acu->act_dif;
    if(acu2 != NULL)
      td = 0.5f * (acu->act_dif - acu2->act_dif); // combine two by default
    lay->td = td;

    LeabraUnitSpec* us = (LeabraUnitSpec*)un_gain_spec.spec;
    if(mod_type == MOD_GAIN) {
      if((trl->phase == LeabraTrial::PLUS_PHASE) &&
	 (lay->prv_phase == LeabraTrial::PLUS_PHASE)) { // 2nd plus phase
	// map full -1, +1 range of td onto un_gain_range
	float gain = un_gain_range.MidPoint() + (.5f * un_gain_range.Range() * td);
	UpdateUnitGain(us, gain);
      }
      else {
	UpdateUnitGain(us, un_gain_range.MidPoint()); // reset gain!
      }
    }
    else if(mod_type == MOD_ERR) {
      if((trl->phase == LeabraTrial::PLUS_PHASE) &&
	 (lay->prv_phase == LeabraTrial::PLUS_PHASE)) { // 2nd plus phase
	float err = err_delta * td;
	UpdateUnitErr(us, err);
      }
      else {
	UpdateUnitErr(us, 0.0f); // reset err
      }
    }
  }

  if(rew.immediate && (trl->phase == LeabraTrial::MINUS_PHASE)) {
    // in immediate mode, minus phase is free, plus phase is clamped
    lay->hard_clamped = false;
    return;			
  }

  if(trl->phase == LeabraTrial::PLUS_PHASE) {
    bool got_rew = (acu->ext > 0.0f);
    if(acu2 != NULL)
      got_rew = got_rew || (acu2->ext > 0.0f);
    if(!got_rew) { // not receiving some reward
      lay->hard_clamped = false; // don't clamp it
      return;			// let it ride
    }
  }

  lay->hard_clamped = true;	// cache this flag
  lay->SetExtFlag(Unit::EXT);	// make sure this is set, regardless
  lay->Inhib_SetVals(i_kwta_pt); // assume 0 - 1 clamped inputs
  acu->SetExtFlag(Unit::EXT);
  if(acu2 != NULL)
    acu2->SetExtFlag(Unit::EXT);

  if(trl->phase == LeabraTrial::MINUS_PHASE) {
    if(rew.reset && (acu->clmp_net == -10.0f)) {
      // reset after reward (clmp_net marks reward)
      acu->ext = rew.val;
      if(acu2 != NULL)
	acu2->ext = rew.val;
    }
    else {
      // use prior plus phase, which was v(t+1) and is now v(t)
      // but assume that v(t+1) was discounted so, now "undiscount" it
      acu->ext = rew.inv_disc * acu->act_p;
      if(acu2 != NULL)
	acu2->ext = rew.inv_disc * acu2->act_p;
    }
    if(softmax.use) {
      // filter clamped values through the softmax!
      acu->net = acu->ext;
      if(acu2 != NULL)	acu2->net = acu2->ext;
      Compute_Act_impl(lay, &(lay->units), lay, 0, trl->phase);
      acu->ext = acu->act;
      if(acu2 != NULL)	acu2->ext = acu2->act;
    }
    acu->Compute_HardClamp(lay);
    acu->act_eq = acu->act = acu->ext; // avoid clamp_range
    acu->clmp_net = 0.0f;	// reset this flag
    if(acu2 != NULL) {
      acu2->Compute_HardClamp(lay);
      acu2->act_eq = acu2->act = acu2->ext; // avoid clamp_range
      acu2->clmp_net = 0.0f;	// reset this flag
    }
  }
  else {
    if(softmax.use) {
      // filter clamped values through the softmax!
      acu->net = acu->ext;
      if(acu2 != NULL)	acu2->net = acu2->ext;
      Compute_Act_impl(lay, &(lay->units), lay, 0, trl->phase);
      acu->ext = acu->act;
      if(acu2 != NULL)	acu2->ext = acu2->act;
    }
    acu->clmp_net = -10.0f;	// set the flag for having been clamped
    acu->Compute_HardClamp(lay); // clamp plus phase reward, use clamp_range
    if(acu2 != NULL) {
      acu2->clmp_net = -10.0f;	// set the flag for having been clamped
      acu2->Compute_HardClamp(lay); // clamp plus phase reward, use clamp_range
    }
  }
  if(acu2 != NULL) {
    lay->acts.max = MAX(acu->act_eq, acu2->act_eq);
    lay->acts.avg = 0.5f * (acu->act_eq + acu2->act_eq);
  }
  else {
    lay->acts.max = lay->acts.avg = acu->act_eq;
  }
}

void LeabraACLayerSpec::Compute_Inhib(LeabraLayer* lay, int cycle, int phase) {
  if(softmax.use) return;
  LeabraLayerSpec::Compute_Inhib(lay, cycle, phase);
}

void LeabraACLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug,
					 LeabraInhib* thr, int cycle, int phase) { 
  if(!softmax.use) {
    LeabraLayerSpec::Compute_Act_impl(lay, ug, thr, cycle, phase);
    return;
  }
  float sum = 0.0f;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->act = expf(softmax.gain * u->net); // e to the net
    sum += u->act;
  }

  float tmp_max = 0.0f;
  float tmp_avg = 0.0f;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->act /= sum;
    u->act_eq = u->act;
    tmp_max = MAX(tmp_max, u->act_eq);
    tmp_avg += u->act_eq;
  }
  thr->acts.max = tmp_max;
  if(ug->leaves > 0)
    thr->acts.avg = tmp_avg / (float)ug->leaves;
  else
    thr->acts.avg = 0.0f;
}

//////////////////////////////////
// 	Linear Unit		//
//////////////////////////////////

void LeabraLinUnitSpec::Initialize() {
  SetUnique("act_fun", true);
  SetUnique("act_range", true);
  SetUnique("clamp_range", true);
  SetUnique("act", true);
  act_fun = LINEAR;
  act_range.max = 20;
  act_range.min = 0;
  clamp_range.max = 1.0f;
  act.gain = 2;
}

void LeabraLinUnitSpec::Compute_Act(LeabraUnit* u, LeabraLayer*, LeabraInhib* thr,
				  int cycle)
{
  if((noise_type == NETIN_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->net += noise_sched.GetVal(cycle) * noise.Gen();
  }

  // total conductances
  u->gc.l = g_bar.l;
  u->gc.i += g_bar.i * thr->i_val.g_i; // add in inhibition from global inhib fctn
  u->gc.h = g_bar.h * u->vcb.g_h;
  u->gc.a = g_bar.a * u->vcb.g_a;

  u->I_net = 
    (u->net * (e_rev.e - u->v_m)) + (u->gc.l * (e_rev.l - u->v_m)) + 
    (u->gc.i * (e_rev.i - u->v_m)) + (u->gc.h * (e_rev.h - u->v_m)) +
    (u->gc.a * (e_rev.a - u->v_m));

  u->v_m += dt.vm * u->I_net;

  if((noise_type == VM_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    u->v_m += noise_sched.GetVal(cycle) * noise.Gen();
  }

  u->v_m = vm_range.Clip(u->v_m); // just for kicks, compute v_m

  float new_act = u->net * act.gain; // but use linear netin as act

  u->da = new_act - u->act;
  if((noise_type == ACT_NOISE) && (noise.type != Random::NONE) && (cycle >= 0)) {
    new_act += noise_sched.GetVal(cycle) * noise.Gen();
  }
  u->act = u->act_eq = act_range.Clip(new_act);

  // fast-time scale updated during settling
  hyst.UpdateBasis(u->vcb.hyst, u->vcb.hyst_on, u->vcb.g_h, u->act_eq);
  acc.UpdateBasis(u->vcb.acc, u->vcb.acc_on, u->vcb.g_a, u->act_eq);
}

//////////////////////////////////
//  	ContextLayerSpec	//
//////////////////////////////////

void CtxtUpdateSpec::Initialize() {
  fm_hid = 1.0f;
  fm_prv = 0.0f;
  to_out = 1.0f;
}

void LeabraContextLayerSpec::Initialize() {
  updt.fm_prv = 0.0f;
  updt.fm_hid = 1.0f;
  updt.to_out = 1.0f;
}

void LeabraContextLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(updt, this);
}

// void LeabraContextLayerSpec::UpdateAfterEdit() {
//   LeabraLayerSpec::UpdateAfterEdit();
//   hysteresis_c = 1.0f - hysteresis;
// }

void LeabraContextLayerSpec::Compute_Context(LeabraLayer* lay, LeabraUnit* u, int phase) {
  if(phase == LeabraTrial::PLUS_PHASE) {
    u->ext = u->act_m;		// just use previous minus phase value!
  }
  else {
    LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[0];
    if(cg == NULL) {
      taMisc::Error("*** LeabraContextLayerSpec requires one recv projection!");
      return;
    }
    LeabraUnit* su = (LeabraUnit*)cg->Un(0);
    if(su == NULL) {
      taMisc::Error("*** LeabraContextLayerSpec requires one unit in recv projection!");
      return;
    }
    u->ext = updt.fm_prv * u->act_p + updt.fm_hid * su->act_p; // compute new value
  }
  u->SetExtFlag(Unit::EXT);
  u->Compute_HardClamp(lay);
}

void LeabraContextLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  lay->hard_clamped = true;	// cache this flag
  lay->SetExtFlag(Unit::EXT);

  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  if(lay->units.gp.size > 0) {
    float lay_avg = 0.0f;
    float lay_max = 0.0f;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      float tmp_avg = 0.0f;
      float tmp_max = 0.0f;
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      rugp->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs
      int i;
      for(i=0;i<rugp->size;i++) {
	LeabraUnit* u = (LeabraUnit*)rugp->FastEl(i);
	Compute_Context(lay, u, trl->phase);
	tmp_avg += u->act_eq;
	tmp_max = MAX(tmp_max, u->act_eq);
      }
      lay_avg += tmp_avg;
      lay_max = MAX(tmp_max, lay_max);
      if(rugp->size > 0)
	rugp->acts.avg = tmp_avg / (float)rugp->size;
      rugp->acts.max = tmp_max;
    }
    lay->acts.avg = lay_avg / (float)lay->units.leaves;
    lay->acts.max = lay_max;
  }
  else {
    float tmp_avg = 0.0f;
    float tmp_max = 0.0f;
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
      Compute_Context(lay, u, trl->phase);
      tmp_avg += u->act_eq;
      tmp_max = MAX(tmp_max, u->act_eq);
    }
    lay->acts.avg = tmp_avg / (float)lay->units.leaves;
    lay->acts.max = tmp_max;
  }
}

//////////////////////////////////
//  	GatedContextLayerSpec	//
//////////////////////////////////

void GateSpec::Initialize() {
  maint = EXTRACELL;
  out = NO_OUT;
  reset = true;
}

void GateNoiseSpec::Initialize() {
  p_pos = .5f;
  pos_var = .2f;
  neg_var = .2f;
  pos_thr = .5f;
  neg_thr = 1.0f;
}

void LeabraGatedCtxLayerSpec::Initialize() {
  base.fm_hid = 0.0f;
  base.fm_prv = 1.0f;
  base.to_out = 0.2f;
  in_con_spec.SetBaseType(&TA_LeabraConSpec);
  maint_con_spec.SetBaseType(&TA_LeabraConSpec);
  out_con_spec.SetBaseType(&TA_LeabraConSpec);
}

void LeabraGatedCtxLayerSpec::InitLinks() {
  LeabraContextLayerSpec::InitLinks();
  taBase::Own(base, this);
  taBase::Own(gate, this);
  taBase::Own(gate_noise, this);
  in_con_spec.SetDefaultSpec(this);
  maint_con_spec.SetDefaultSpec(this);
  out_con_spec.SetDefaultSpec(this);
}

void LeabraGatedCtxLayerSpec::CutLinks() {
  LeabraContextLayerSpec::CutLinks();
  in_con_spec.CutLinks();
  maint_con_spec.CutLinks();
  out_con_spec.CutLinks();
}

void LeabraGatedCtxLayerSpec::UpdateConSpec(LeabraConSpec* cs, float new_gain) {
  cs->wt_scale.abs = new_gain;
  LeabraConSpec* c;
  taLeafItr i;
  FOR_ITR_EL(LeabraConSpec, c, cs->children., i)
    UpdateConSpec(c, new_gain);
}

void LeabraGatedCtxLayerSpec::InitState(LeabraLayer* lay) {
  updt.fm_hid = base.fm_hid;
  updt.fm_prv = base.fm_prv;
  updt.to_out = base.to_out;
  LeabraContextLayerSpec::InitState(lay);
  if(in_con_spec.spec != NULL)
    UpdateConSpec((LeabraConSpec*)in_con_spec.spec, updt.fm_hid);
  if(maint_con_spec.spec != NULL)
    UpdateConSpec((LeabraConSpec*)maint_con_spec.spec, updt.fm_prv);
  if((gate.out != GateSpec::NO_OUT) && (out_con_spec.spec != NULL))
    UpdateConSpec((LeabraConSpec*)out_con_spec.spec, updt.to_out);
}

void LeabraGatedCtxLayerSpec::Compute_Context(LeabraLayer* lay, LeabraUnit* u, int) {
  // this is only called when it is time to clamp context to previous values
  u->ext = u->act_p;
  u->SetExtFlag(Unit::EXT);
  u->Compute_HardClamp(lay);
}

void LeabraGatedCtxLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase == LeabraTrial::MINUS_PHASE) { // first minus phase: use base setting of to_out
    updt.to_out = base.to_out;
    LeabraConSpec* oc = (LeabraConSpec*)out_con_spec.spec;
    if(oc != NULL)
      UpdateConSpec(oc, updt.to_out);

    if(gate.reset) {		// reset after reward
      bool reset = Get_RewReset(lay);
      if(reset)
	lay->InitState();
    }

    if(gate.maint == GateSpec::EXTRACELL) {
      // minus phase is clamping to prior plus phase values
      LeabraContextLayerSpec::Compute_HardClamp(lay, trl); // then clamp everything!
    }
    else {
      // intracell: minus phase is letting it update on its own!
      // what about the gain modulation?? what to set it at??
      updt.fm_hid = 1.0f;	// input is strong in minus phase so that it is reflected in pfc
      updt.fm_prv = base.fm_prv;
      UpdateConSpec((LeabraConSpec*)in_con_spec.spec, updt.fm_hid);
      UpdateConSpec((LeabraConSpec*)maint_con_spec.spec, updt.fm_prv);
      UndoHardClamp(lay);
    }
  }
  else if((trl->phase == LeabraTrial::PLUS_PHASE) && (lay->prv_phase == LeabraTrial::MINUS_PHASE)) {
    if(gate.maint == GateSpec::EXTRACELL) {
      // just stay clamped for the 1st plus phase
      LeabraContextLayerSpec::Compute_HardClamp(lay, trl);
    }
    else {
      // intracell: continue to just let it update
      UndoHardClamp(lay);
    }	  
  }
  else {  // 2nd plus phase
    if((in_con_spec.spec == NULL) || (maint_con_spec.spec == NULL))
      return;

    float td = Get_TD(lay);
    if(Random::ZeroOne() < gate_noise.p_pos)
      td += fabs(Random::Gauss(gate_noise.pos_var));
    else
      td -= fabs(Random::Gauss(gate_noise.neg_var));
    lay->td = td;		// store this baby for later..

    lay->td_updt = false;	// decide based on td if updating will occur
    // stochastically update current state with probability = abs(td)
    // todo: actually want to pass this through a sigmoid function, but use a threshold of .5 for now:
    if(lay->td >= 0.0f) {
      if((lay->td >= gate_noise.pos_thr) || (Random::ZeroOne() <= lay->td))
	lay->td_updt = true;
    }
    else {
      if((-lay->td >= gate_noise.neg_thr) || (Random::ZeroOne() <= -lay->td))
	lay->td_updt = true;
    }

    if(lay->td_updt) {		// initialize prior maintenance stuff if updating!
      LeabraUnit* u;
      taLeafItr i;
      FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
	u->DecayState(lay, fabs(lay->td)); // clear out in proportion to td
	u->vcb.g_h -= fabs(lay->td) * u->vcb.g_h;
      }
    }

    // update context reps after td has been computed, 
    updt.fm_hid = base.fm_hid + td;
    updt.fm_prv = base.fm_prv + td;	
    updt.fm_hid = MIN(updt.fm_hid, 1.0f);  updt.fm_hid = MAX(updt.fm_hid, 0.0f);
    updt.fm_prv = MIN(updt.fm_prv, 1.0f);  updt.fm_prv = MAX(updt.fm_prv, 0.0f);
    
    UpdateConSpec((LeabraConSpec*)in_con_spec.spec, updt.fm_hid);
    UpdateConSpec((LeabraConSpec*)maint_con_spec.spec, updt.fm_prv);

    if(gate.out == GateSpec::TD_MOD) {
      updt.to_out = base.to_out + td;
      updt.to_out = MIN(updt.to_out, 1.0f);  updt.to_out = MAX(updt.to_out, 0.0f);
      LeabraConSpec* oc = (LeabraConSpec*)out_con_spec.spec;
      if(oc != NULL)
	UpdateConSpec(oc, updt.to_out);
    }
    
    UndoHardClamp(lay);	// just update based on gated values..
  }
}

void LeabraGatedCtxLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if((gate.maint == GateSpec::EXTRACELL) || (trl->phase == LeabraTrial::MINUS_PHASE) ||
     ((trl->phase == LeabraTrial::PLUS_PHASE) && (lay->prv_phase == LeabraTrial::MINUS_PHASE))) {
    // can't do this vvv before because it updates lay->prv_phase!
    LeabraContextLayerSpec::PostSettle(lay, trl, set_both);
    return;
  }
  LeabraContextLayerSpec::PostSettle(lay, trl, set_both);
  // only intracell in 2nd plus phase: activate or deactivate based on td
  // do it for entire layer at the same time

  if((lay->td < 0.0f) || !lay->td_updt) return;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->vcb.g_h = u->act_p;
  }
}

void LeabraGatedCtxLayerSpec::UndoHardClamp(LeabraLayer* lay) {
  lay->hard_clamped = false;	// turn off hard clamping!
  lay->UnSetExtFlag(Unit::EXT);
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->UnSetExtFlag(Unit::EXT);
}

bool LeabraGatedCtxLayerSpec::Get_RewReset(LeabraLayer* lay) {
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[1];
  if(ac_cg == NULL) {
    // allow no AC projections -- constant updating gating
    //    taMisc::Error("*** LeabraGatedCtxLayerSpec requires 2nd AC recv projection!");
    return false;
  }
  // must be an ac layer..
  if(!((LeabraLayer*)ac_cg->prjn->from)->spec.spec->InheritsFrom(TA_LeabraACLayerSpec)) {
    return 1.0f;
  }
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL) {
    taMisc::Error("*** LeabraGatedCtxLayerSpec requires AC unit in 2nd AC recv projection!");
    return 0.0f;
  }
  if(acu->clmp_net == -10.0f)
    return true;
  return false;
}

float LeabraGatedCtxLayerSpec::Get_TD(LeabraLayer* lay) {
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[1];
  if(ac_cg == NULL) {
    // allow no AC projections -- constant updating gating
    //    taMisc::Error("*** LeabraGatedCtxLayerSpec requires 2nd AC recv projection!");
    return 1.0f;
  }
  // must be an ac layer..
  if(!((LeabraLayer*)ac_cg->prjn->from)->spec.spec->InheritsFrom(TA_LeabraACLayerSpec)) {
    return 1.0f;
  }
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL) {
    taMisc::Error("*** LeabraGatedCtxLayerSpec requires AC unit in 2nd AC recv projection!");
    return 0.0f;
  }

  LeabraUnit* acu2 = NULL;
  if(ac_cg->units.size > 1)
    acu2 = (LeabraUnit*)ac_cg->Un(1);	// get 2nd unit

  float td = acu->act_dif;
  if(acu2 != NULL)
    td = 0.5f * (acu->act_dif - acu2->act_dif); // combine two by default
  return td;
}


//////////////////////////////////////////
// 	Encode/Learn Environment	//
//////////////////////////////////////////

void EncLrnEventSpec::Initialize() {
  enc_lrn = LRN_ENC;
}

void EncLrnEventSpec::Copy_(const EncLrnEventSpec& cp) {
  enc_lrn = cp.enc_lrn;
}


//////////////////////////////////////////
// 	Phase-Order Environment		//
//////////////////////////////////////////

void PhaseOrderEventSpec::Initialize() {
  phase_order = MINUS_PLUS;
}

void PhaseOrderEventSpec::Copy_(const PhaseOrderEventSpec& cp) {
  phase_order = cp.phase_order;
}

//////////////////////////////////////////
// 	Duration Environment		//
//////////////////////////////////////////

void DurEvent::Initialize() {
  duration = 50.0f;
}

void DurEvent::Copy_(const DurEvent& cp) {
  duration = cp.duration;
}

//////////////////////////////////////////////////
// 	Different Hebb Learning Variants	//
//////////////////////////////////////////////////


//////////////////////////////////////////
// 	Time-Based Learning		//
//////////////////////////////////////////

void LeabraTimeConSpec::Initialize() {
  min_obj_type = &TA_LeabraTimeCon_Group;
  k_prv = .5f;
  k_cur = 1.0f - k_prv;
}

void LeabraTimeConSpec::UpdateAfterEdit() {
  LeabraConSpec::UpdateAfterEdit();
  k_cur = 1.0f - k_prv;
}

void LeabraTimeConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  children.SetBaseType(&TA_LeabraConSpec); // make this the base type so bias specs
					   // can live under here..
  children.el_typ = &TA_LeabraTimeConSpec; // but this is the default type
}

void LeabraTimeCon_Group::Initialize() {
  p_savg = 0.0f;
  p_max_cor = 0.0f;
}

void LeabraTimeCon_Group::Copy_(const LeabraTimeCon_Group& cp) {
  p_savg = cp.p_savg;
  p_max_cor = cp.p_max_cor;
}

void LeabraTimeUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraTimeUnit;
}

void LeabraTimeUnitSpec::InitState(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  lu->p_act_m = -.1f;
  lu->p_act_p = -.1f;
}

void LeabraTimeUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay) {
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  if((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) {
    if((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn))
      return;
  }
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay);
}

void LeabraTimeUnitSpec::EncodeState(LeabraUnit* u, LeabraLayer*) {
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  // just save phase activation states
  lu->p_act_p = lu->act_p;
  lu->p_act_m = lu->act_m;
  
  LeabraTimeCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraTimeCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
    recv_gp->EncodeState(u);
  }
}

void LeabraTimeUnit::Initialize() {
  p_act_m = -.1f;
  p_act_p = -.1f;
}

//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////


