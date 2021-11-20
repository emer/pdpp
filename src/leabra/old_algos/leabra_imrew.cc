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
#include <pdp/pdpshell.h>
#include <pdp/procs_extra.h>
#include <pdp/enviro_extra.h>
#include <pdp/netstru_extra.h>
#include <css/css_iv.h>
#include <limits.h>
#include <float.h>

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
  hebb = .001f;
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
  cor = .4f;
  src = SLAYER_AVG_ACT;
  thresh = .001f;
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
  wt_sig_fun.res = 1.0e-5f;	// 1e-6 = 1.9Mb & 33% slower!, but 4x more accurate; 1e-5 = .19Mb
  wt_sig_fun.x_range.UpdateAfterEdit();

  wt_sig_fun_inv.x_range.min = 0.0f;
  wt_sig_fun_inv.x_range.max = 1.0f;
  wt_sig_fun_inv.res = 1.0e-5f;	// 1e-6 = 1.9Mb & 33% slower!, but 4x more accurate; 1e-5 = .19Mb
  wt_sig_fun_inv.x_range.UpdateAfterEdit();

  wt_sig_fun_lst.off = -1;   wt_sig_fun_lst.gain = -1; // trigger an update
  wt_sig_fun_res = -1.0;

  rnd.mean = .5f;
  rnd.var = .25f;
  lrate = .01f;
  cur_lrate = .01f;
  lrate_sched.interpolate = false;
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
  taBase::Own(wt_sig_fun_lst, this);
  CreateWtSigFun();
}

void LeabraConSpec::UpdateAfterEdit() {
  ConSpec::UpdateAfterEdit();
  lrate_sched.UpdateAfterEdit();
  CreateWtSigFun();
  lmix.UpdateAfterEdit();
}

void LeabraConSpec::Defaults() {
  wt_scale.Initialize();
  wt_sig.Initialize();
  lmix.Initialize();
  fix_savg.Initialize();
  savg_cor.Initialize();
  Initialize();
}

void LeabraConSpec::SetCurLrate(int epoch) {
  cur_lrate = lrate * lrate_sched.GetVal(epoch);
}

void LeabraConSpec::CreateWtSigFun() {
  if((wt_sig_fun_lst.gain == wt_sig.gain) && (wt_sig_fun_lst.off == wt_sig.off)
     && (wt_sig_fun_res == wt_sig_fun.res))
    return;
  wt_sig_fun.AllocForRange();
  int i;
  for(i=0; i<wt_sig_fun.size; i++) {
    float w = wt_sig_fun.Xval(i);
    wt_sig_fun.FastEl(i) = WtSigSpec::SigFun(w, wt_sig.gain, wt_sig.off);
  }
  wt_sig_fun_inv.AllocForRange();
  for(i=0; i<wt_sig_fun_inv.size; i++) {
    float w = wt_sig_fun_inv.Xval(i);
    wt_sig_fun_inv.FastEl(i) = WtSigSpec::SigFunInv(w, wt_sig.gain, wt_sig.off);
  }
  // prevent needless recomputation of this lookup table..
  wt_sig_fun_lst.gain = wt_sig.gain; wt_sig_fun_lst.off = wt_sig.off;
  wt_sig_fun_res = wt_sig_fun.res;
}

void LeabraConSpec::GraphWtSigFun(GraphLog* graph_log) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  graph_log->name = name + ": Wt Sig Fun";
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("lin wt");
  dt->NewColFloat("eff wt");
  dt->AddColDispOpt("MIN=0", 1);
  dt->AddColDispOpt("MAX=1", 1);

  float x;
  for(x = 0.0; x <= 1.0; x += .01) {
    float y = WtSigSpec::SigFun(x, wt_sig.gain, wt_sig.off);
    dt->AddBlankRow();
    dt->SetLastFloatVal(x, 0);
    dt->SetLastFloatVal(y, 1);
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
}

void LeabraCon_Group::Initialize() {
  spec.SetBaseType(&TA_LeabraConSpec);
  el_typ = &TA_LeabraCon;
  scale_eff = 0.0f;
  savg = 0.0f;
  max_cor = 1.0f;
  net = 0.0f;
  p_savg = 0.0f;
  p_max_cor = 0.0f;
}

void LeabraCon_Group::Copy_(const LeabraCon_Group& cp) {
  scale_eff = cp.scale_eff;
  savg = cp.savg;
  max_cor = cp.max_cor;
  net = cp.net;
  p_savg = cp.p_savg;
  p_max_cor = cp.p_max_cor;
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

void LeabraBiasSpec::Defaults() {
  LeabraConSpec::Defaults();
  Initialize();
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
  avg_dt = .005f;
  send_delta = false;
}

void SpikeFunSpec::Initialize() {
  decay = 0.05f;
  v_m_r = 0.0f;
  eq_gain = 10.0f;
  eq_dt = 0.02f;
  hard_gain = .4f;
  // vm_dt of .1 should also be used; vm_noise var .002???
}

void DepressSpec::Initialize() {
  p_spike = P_NXX1;
  rec = .2f;
  asymp_act = .5f;
  depl = rec * (1.0f - asymp_act) / (asymp_act * .95f);
  max_amp = (.95f * depl + rec) / rec;
}

void DepressSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  if(rec < .00001f) rec = .00001f;
  if(asymp_act < .00001f) asymp_act = .00001f;
  if(asymp_act > 1.0f) asymp_act = 1.0f;
  depl = rec * (1.0f - asymp_act) / (asymp_act * .95f);
  depl = MAX(depl, 0.0f);
  max_amp = (.95f * depl + rec) / rec;
}

void OptThreshSpec::Initialize() {
  send = .1f;
  delta = 0.005f;
  learn = 0.01f;
  updt_wts = true;
  phase_dif = 0.0f;		// .8 also useful
}

void DtSpec::Initialize() {
  vm = 0.2f;
  net = 0.7f;
}

void PhaseSharpSpec::Initialize() {
  type = NONE;
  gain = 1.01f;
  off = 1.0f;
  delta = .1f;
  i_gain = 1.6f;
  act_gain = .1f;
}

void ActRegSpec::Initialize() {
  on = false;
  min = .001f;
  max = .35f;
  wt_dt = 0.2f;
}

void LeabraUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraUnit;
  bias_con_type = &TA_LeabraCon;
  bias_spec.SetBaseType(&TA_LeabraConSpec);

  act_fun = NOISY_XX1;

  clamp_range.min = .0f;
  clamp_range.max = .95f;
  clamp_range.UpdateAfterEdit();

  vm_range.max = 1.0f;
  vm_range.min = 0.0f;
  vm_range.UpdateAfterEdit();

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

  CreateNXX1Fun();
  phase_delta = 0.0f;
}

void LeabraUnitSpec::Defaults() {
  act.Initialize();
  spike.Initialize();
  depress.Initialize();
  opt_thresh.Initialize();
  dt.Initialize();
  phase_sharp.Initialize();
  act_reg.Initialize();
  Initialize();
  bias_spec.SetSpec(bias_spec.spec);
}

void LeabraUnitSpec::InitLinks() {
  bias_spec.type = &TA_LeabraBiasSpec;
  UnitSpec::InitLinks();
  taBase::Own(act, this);
  taBase::Own(spike, this);
  taBase::Own(depress, this);
  taBase::Own(opt_thresh, this);
  taBase::Own(clamp_range, this);
  taBase::Own(vm_range, this);
  taBase::Own(v_m_init, this);
  taBase::Own(dt, this);
  taBase::Own(g_bar, this);
  taBase::Own(e_rev, this);
  taBase::Own(hyst, this);
  taBase::Own(acc, this);
  taBase::Own(phase_sharp, this);
  taBase::Own(act_reg, this);
  taBase::Own(noise, this);
  taBase::Own(noise_sched, this);
  taBase::Own(nxx1_fun, this);
  taBase::Own(noise_conv, this);
}

void LeabraUnitSpec::UpdateAfterEdit() {
  UnitSpec::UpdateAfterEdit();
  clamp_range.UpdateAfterEdit();
  vm_range.UpdateAfterEdit();
  depress.UpdateAfterEdit();
  noise_sched.UpdateAfterEdit();
  CreateNXX1Fun();
  if(act_fun == DEPRESS)
    act_range.max = depress.max_amp;
}

bool LeabraUnitSpec::CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, un->recv., g) {
    if(!recv_gp->CheckConfig(lay, un, tp, quiet)) return false;
  }

  if(opt_thresh.updt_wts &&
     ((tp->epoch_proc != NULL) &&
      (tp->epoch_proc->wt_update != EpochProcess::ON_LINE) &&
      (tp->epoch_proc->wt_update != EpochProcess::TEST))) {
    taMisc::Error("LeabraUnitSpec Warning: cannot use opt_thresh.updt_wts when wt_update is not ON_LINE",
		  "I turned this flag off for you in LeabraUnitSpec:", name);
    SetUnique("opt_thresh", true);
    opt_thresh.updt_wts = false;
  }
  return true;
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
  lu->act_avg = 0.0f;
  lu->misc_1 = 0.0f;
  if(act_fun != DEPRESS)
    lu->spk_amp = 0.0f;
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

void LeabraUnitSpec::SetCurLrate(LeabraUnit* u, LeabraTrial*, int epoch) {
  ((LeabraConSpec*)bias_spec.spec)->SetCurLrate(epoch);
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g)
    recv_gp->SetCurLrate(epoch);
}

////////////////////////////////////////////
//	Stage 0: at start of settling	  // 
////////////////////////////////////////////

void LeabraUnitSpec::InitDelta(LeabraUnit* u) {
  if(act.send_delta) {
    u->net_delta = 0.0f;
    u->g_i_delta = 0.0f;
    u->net = 0.0f;		// important for soft-clamped layers
    u->gc.i = 0.0f;
  }
  else {
    u->gc.i = 0.0f;
    u->net = u->clmp_net;
  }

  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    recv_gp->net = 0.0f;
  } 
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

  ru->act_sent = 0.0f;
  ru->act_delta = 0.0f;
  ru->net_raw = 0.0f;
  ru->net_delta = 0.0f;
  ru->g_i_delta = 0.0f;
  ru->g_i_raw = 0.0f;

  ru->i_thr = 0.0f;
  if(act_fun == DEPRESS)
    ru->spk_amp = act_range.max;
}

void LeabraUnitSpec::Compute_NetScale(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  // this is all receiver-based and done only at beginning of settling
  u->clmp_net = 0.0f;
  u->net_scale = 0.0f;	// total of scale values for this unit's inputs

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
  }
  // add the bias weight into the netinput, scaled by 1/n
  if(u->bias != NULL) {
    LeabraConSpec* bspec = (LeabraConSpec*)bias_spec.spec;
    u->bias_scale = bspec->wt_scale.abs;  // still have absolute scaling if wanted..
    if(u->n_recv_cons > 0)
      u->bias_scale /= (float)u->n_recv_cons; // one over n scaling for bias!
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

void LeabraUnitSpec::Send_ClampNet(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  if(u->act > opt_thresh.send) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      if(send_gp->prjn->layer->lesion) continue;
      if(((LeabraConSpec*)send_gp->spec.spec)->inhib) {
	taMisc::Error("*** Error: cannot send inhibition from a hard-clamped layer!  Set layerspec clamp.hard off!");
	continue;
      }
      send_gp->Send_ClampNet(u);
    }
  }
}

////////////////////////////////////
//	Stage 1: netinput 	  //
////////////////////////////////////

void LeabraUnitSpec::Send_Net(LeabraUnit* u, LeabraLayer*) {
  // sender-based (and this fun not called on hard_clamped EXT layers)
  if(u->act > opt_thresh.send) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion || tol->hard_clamped)	continue;
      send_gp->Send_Net(u);
    }
  }
}

void LeabraUnitSpec::Send_NetDelta(LeabraUnit* u, LeabraLayer*) {
  if(u->act > opt_thresh.send) {
    if(fabs(u->act_delta) > opt_thresh.delta) {
      LeabraCon_Group* send_gp;
      int g;
      FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
	LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
	if(tol->lesion || tol->hard_clamped)	continue;
	send_gp->Send_NetDelta(u);
      }
      u->act_sent = u->act;	// cache the last sent value
    }
  }
  else if(u->act_sent > opt_thresh.send) {
    u->act_delta = - u->act_sent; // un-send the last above-threshold activation to get back to 0
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion || tol->hard_clamped)	continue;
      send_gp->Send_NetDelta(u);
    }
    u->act_sent = 0.0f;		// now it effectively sent a 0..
  }
}

//////////////////////////////////////////////////////////////////
//	Stage 2: netinput averages and clamping (if necc)	//
//////////////////////////////////////////////////////////////////

void LeabraUnitSpec::Compute_HardClamp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial*) {
  u->net = u->prv_net = u->ext * lay->stm_gain;
  u->act_eq = clamp_range.Clip(u->ext);
  if(act_fun == SPIKE)
    u->act = spike.hard_gain * u->act_eq;
  else
    u->act = u->act_eq;
  if(u->act_eq == 0.0f)
    u->v_m = e_rev.l;
  else
    u->v_m = act.thr + u->act_eq / act.gain;
  u->da = u->I_net = 0.0f;
}

bool LeabraUnitSpec::Compute_SoftClamp(LeabraUnit* u, LeabraLayer* lay, LeabraTrial*) {
  bool inc_gain = false;
  if(u->ext_flag & Unit::EXT) {
    if((u->ext > .5f) && (u->act < .5f))
      inc_gain = true;	// must increase gain because targ unit is < .5..

    u->net += u->ext * lay->stm_gain;
  }
  return inc_gain;
}

//////////////////////////////////////////
//	Stage 3: inhibition		//
//////////////////////////////////////////

float LeabraUnitSpec::Compute_IThresh(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  float non_bias_net = u->net;
  if(u->bias != NULL)		// subtract out bias weights so they can change k
    non_bias_net -= u->bias_scale * u->bias->wt;

  // including the ga and gh terms
  return ((non_bias_net * (e_rev.e - act.thr) + u->gc.l * (e_rev.l - act.thr)
	   + u->gc.a * (e_rev.a - act.thr) + u->gc.h * (e_rev.h - act.thr)) /
	  (act.thr - e_rev.i));
} 

//////////////////////////////////////////
//	Stage 4: the final activation 	//
//////////////////////////////////////////

void LeabraUnitSpec::Compute_Act(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl) {
  Compute_Conduct(u, lay, thr, trl);
  Compute_Vm(u, lay, thr, trl);
  Compute_ActFmVm(u, lay, thr, trl);
  Compute_SelfReg(u, lay, thr, trl);
  u->act_delta = u->act - u->act_sent;
}

void LeabraUnitSpec::Compute_Conduct(LeabraUnit* u, LeabraLayer* lay, LeabraInhib*, LeabraTrial* trl) {
  // total conductances
  float g_bar_i_val = g_bar.i;
  float	g_bar_e_val = g_bar.e;
  float	g_bar_l_val = g_bar.l;
  if((phase_sharp.type == PhaseSharpSpec::NETIN) && (trl->phase == LeabraTrial::PLUS_PHASE)) {
    g_bar_i_val += phase_sharp.i_gain * phase_sharp.delta;
    g_bar_e_val += phase_sharp.delta + u->act_eq * phase_sharp.act_gain;
  }
  LeabraLayerSpec* ls = (LeabraLayerSpec*)lay->spec.spec;
  if((ls->adapt_i.type == AdaptISpec::G_BAR_I) || (ls->adapt_i.type == AdaptISpec::G_BAR_IL))
    g_bar_i_val = lay->adapt_i.g_bar_i; // adapting value..
  if(ls->adapt_i.type == AdaptISpec::G_BAR_IL)
    g_bar_l_val = lay->adapt_i.g_bar_l; // adapting value..
  if((ls->compute_i == LeabraLayerSpec::UNIT_M_KWTA_P) ||
     (ls->compute_i == LeabraLayerSpec::UNIT_M_KWTA_AVG_P)) {
    if(trl->phase == LeabraTrial::PLUS_PHASE)
      g_bar_i_val = 1.0;		// don't use g_bar_i for plus phase: kwta!
  }
  u->gc.i *= g_bar_i_val;
  u->net *= g_bar_e_val;
  u->gc.l = g_bar_l_val;
  u->gc.h = g_bar.h * u->vcb.g_h;
  u->gc.a = g_bar.a * u->vcb.g_a;
}

void LeabraUnitSpec::Compute_Vm(LeabraUnit* u, LeabraLayer* lay, LeabraInhib*, LeabraTrial* trl) {
  float dt_vm_val = dt.vm;
  LeabraLayerSpec* ls = (LeabraLayerSpec*)lay->spec.spec;
  if((ls->compute_i == LeabraLayerSpec::UNIT_M_KWTA_P) ||
     (ls->compute_i == LeabraLayerSpec::UNIT_M_KWTA_AVG_P)) {
    if(trl->phase == LeabraTrial::PLUS_PHASE)
      dt_vm_val = .2;		// normal fast settling in plus phase!
  }

  u->I_net = 
    (u->net * (e_rev.e - u->v_m)) + (u->gc.l * (e_rev.l - u->v_m)) + 
    (u->gc.i * (e_rev.i - u->v_m)) + (u->gc.h * (e_rev.h - u->v_m)) +
    (u->gc.a * (e_rev.a - u->v_m));

  u->v_m += dt_vm_val * u->I_net;

  if((noise_type == VM_NOISE) && (noise.type != Random::NONE) && (trl->cycle >= 0)) {
    u->v_m += noise_sched.GetVal(trl->cycle) * noise.Gen();
  }

  u->v_m = vm_range.Clip(u->v_m);
}

void LeabraUnitSpec::Compute_ActFmVm(LeabraUnit* u, LeabraLayer*, LeabraInhib*, LeabraTrial* trl) {
  float new_act = u->v_m - act.thr;
  switch(act_fun) {
  case NOISY_XX1: {
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
  break;
  case XX1: {
    if(new_act < 0.0f)
      new_act = 0.0f;
    else {
      new_act *= act.gain;
      new_act = new_act / (new_act + 1.0f);
    }
  }
  break;
  case LINEAR: {
    if(new_act < 0.0f)
      new_act = 0.0f;
    else
      new_act *= act.gain;
  }
  break;
  case DEPRESS: {
    // new_act = spike probability
    if(depress.p_spike == DepressSpec::P_NXX1) {
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
    else {
      if(new_act < 0.0f)
	new_act = 0.0f;
      else {
	new_act *= act.gain;
	new_act = MIN(new_act, 1.0f);
      }
    }
    new_act *= u->spk_amp; // actual output = probability * amplitude
    u->spk_amp += -new_act * depress.depl + (act_range.max - u->spk_amp) * depress.rec;
    u->spk_amp = act_range.Clip(u->spk_amp);
  }
  break;
  case SPIKE: {
    float spike_act = 0.0f;
    if(u->v_m > act.thr) {
      u->act = 1.0f;
      u->v_m = spike.v_m_r;
      spike_act = 1.0f;
    }
    else {
      u->act *= (1.0f - spike.decay);
    }
    float new_eq = u->act_eq / spike.eq_gain;
    if(spike.eq_dt > 0.0f) {
      new_eq = act_range.Clip(spike.eq_gain * ((1.0f - spike.eq_dt) * new_eq + spike.eq_dt * spike_act));
    }
    else {
      if(trl->cycle > 0)
	new_eq *= (float)trl->cycle;
      new_eq = act_range.Clip(spike.eq_gain * (new_eq + spike_act) / (float)(trl->cycle+1));
    }
    if((phase_sharp.type == PhaseSharpSpec::SIG) && (trl->phase == LeabraTrial::PLUS_PHASE)) {
      new_eq = WtSigSpec::SigFun(new_eq, phase_sharp.gain, phase_sharp.off);
    }
    u->da = new_eq - u->act_eq;	// da is on equilibrium activation
    u->act_eq = new_eq;
  }
  break;
  }

  if(act_fun != SPIKE) {
    if((phase_sharp.type == PhaseSharpSpec::SIG) && (trl->phase == LeabraTrial::PLUS_PHASE)) {
      new_act = act_range.max * WtSigSpec::SigFun(new_act / act_range.max, phase_sharp.gain, phase_sharp.off);
    }
    u->da = new_act - u->act;
    if((noise_type == ACT_NOISE) && (noise.type != Random::NONE) && (trl->cycle >= 0)) {
      new_act += noise_sched.GetVal(trl->cycle) * noise.Gen();
    }
    u->act = u->act_eq = act_range.Clip(new_act);
  }
}

void LeabraUnitSpec::Compute_SelfReg(LeabraUnit* u, LeabraLayer*, LeabraInhib*, LeabraTrial*) {
  // fast-time scale updated during settling
  hyst.UpdateBasis(u->vcb.hyst, u->vcb.hyst_on, u->vcb.g_h, u->act_eq);
  acc.UpdateBasis(u->vcb.acc, u->vcb.acc_on, u->vcb.g_a, u->act_eq);
}

//////////////////////////////////////////
//	Stage 5: Between Events 	//
//////////////////////////////////////////

void LeabraUnitSpec::PhaseInit(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(!(u->ext_flag & Unit::TARG))
    return;

  if(trl->phase == LeabraTrial::MINUS_PHASE) {
    if(!(lay->ext_flag & Unit::TARG)) {	// layer isn't a target but unit is..
      u->targ = u->ext;
    }
    u->ext = 0.0f;
    u->UnSetExtFlag(Unit::EXT);
  }
  else {
    u->ext = u->targ;
    u->SetExtFlag(Unit::EXT);
  }
}

void LeabraUnitSpec::DecayPhase(LeabraUnit* u, LeabraLayer*, LeabraTrial*, float decay) {
  u->v_m -= decay * (u->v_m - v_m_init.mean);
  u->act -= decay * u->act;
  u->act_eq -= decay * u->act_eq;
  u->prv_net -= decay * u->prv_net;
  u->prv_g_i -= decay * u->prv_g_i;
  u->vcb.hyst -= decay * u->vcb.hyst;
  u->vcb.acc -= decay * u->vcb.acc;
  if(act_fun == DEPRESS)
    u->spk_amp += (act_range.max - u->spk_amp) * decay;

  // reset the rest of this stuff just for clarity
  u->act_sent = 0.0f;
  u->act_delta = u->act;
  u->net_raw = 0.0f;
  u->net_delta = 0.0f;
  u->g_i_raw = 0.0f;
  u->g_i_delta = 0.0f;
  
  u->net = 0.0f;
  u->net_scale = u->bias_scale = 0.0f;
  u->da = u->I_net = 0.0f;
}

void LeabraUnitSpec::DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay) {
  LeabraUnitSpec::DecayPhase(u, lay, trl, decay);
}

void LeabraUnitSpec::ExtToComp(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  if(!(u->ext_flag & Unit::EXT))
    return;
  u->ext_flag = Unit::COMP;
  u->targ = u->ext;
  u->ext = 0.0f;
}

void LeabraUnitSpec::TargExtToComp(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  if(!(u->ext_flag & Unit::TARG_EXT))
    return;
  u->ext_flag = Unit::COMP;
  u->targ = u->ext;
  u->ext = 0.0f;
}

void LeabraUnitSpec::PostSettle(LeabraUnit* u, LeabraLayer*, LeabraInhib*,
				LeabraTrial* trl, bool set_both)
{
  if((phase_sharp.type == PhaseSharpSpec::SIG_END) && (trl->phase == LeabraTrial::PLUS_PHASE)) {
    u->act_eq = act_range.max * WtSigSpec::SigFun(u->act_eq / act_range.max, phase_sharp.gain, phase_sharp.off);
  }
  if(set_both) {
    u->act_m = u->act_p = u->act_eq;
    u->act_dif = 0.0f;
  }
  else {
    if(trl->phase == LeabraTrial::MINUS_PHASE)
      u->act_m = u->act_eq;
    else if((trl->phase == LeabraTrial::PLUS_PHASE) && (trl->phase_no < 2)) {
      // act_p is only for first plus phase: others require something else
      u->act_p = u->act_eq;
      if(phase_delta != 0.0f)
	u->act_p = act_range.Clip(u->act_p * (1.0f + phase_delta));
    }
    u->act_dif = u->act_p - u->act_m;
  }
}

//////////////////////////////////////////
//	Stage 6: Learning 		//
//////////////////////////////////////////

void LeabraUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(act.avg_dt > 0.0f)
    u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
  if((u->act_p <= opt_thresh.learn) && (u->act_m <= opt_thresh.learn))
    return;
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay, trl);
}

void LeabraUnitSpec::Compute_dWt_impl(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  // don't adapt bias weights on clamped inputs..: why?  what possible consequence could it have!?
  //  if(!((u->ext_flag & Unit::EXT) && !(u->ext_flag & Unit::TARG))) {
  ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
    //  }
  UnitSpec::Compute_dWt(u);
}

void LeabraUnitSpec::Compute_WtFmLin(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
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
  //  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
  ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
    //  }
  if(opt_thresh.updt_wts && 
     ((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)))
    return;
  UnitSpec::UpdateWeights(lu);
}

void LeabraUnitSpec::GraphVmFun(GraphLog* graph_log, float g_i, float min, float max, float incr) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  graph_log->name = name + ": Vm Fun";
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("net");
  dt->NewColFloat("v_m");

  float x;
  for(x = min; x <= max; x += incr) {
    float y = ((g_bar.e * x * e_rev.e) + (g_bar.i * g_i * e_rev.i) + (g_bar.l * e_rev.l)) /
      ((g_bar.e * x) + (g_bar.i * g_i) + g_bar.l);
    dt->AddBlankRow();
    dt->SetLastFloatVal(x, 0);
    dt->SetLastFloatVal(y, 1);
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
}

void LeabraUnitSpec::GraphActFmVmFun(GraphLog* graph_log, float min, float max, float incr) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  graph_log->name = name + ": Act Fm Vm Fun";
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("v_m");
  dt->NewColFloat("act");

  LeabraUnit un;
  LeabraTrial trl;

  float x;
  for(x = min; x <= max; x += incr) {
    un.v_m = x;
    Compute_ActFmVm(&un, NULL, NULL, &trl);
    dt->AddBlankRow();
    dt->SetLastFloatVal(x, 0);
    dt->SetLastFloatVal(un.act, 1);
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
}

void LeabraUnitSpec::GraphActFmNetFun(GraphLog* graph_log, float g_i, float min, float max, float incr) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  graph_log->name = name + ": Act Fm Net Fun";
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("v_m");
  dt->NewColFloat("act");

  LeabraUnit un;
  LeabraTrial trl;

  float x;
  for(x = min; x <= max; x += incr) {
    un.v_m = ((g_bar.e * x * e_rev.e) + (g_bar.i * g_i * e_rev.i) + (g_bar.l * e_rev.l)) /
      ((g_bar.e * x) + (g_bar.i * g_i) + g_bar.l);
    Compute_ActFmVm(&un, NULL, NULL, &trl);
    dt->AddBlankRow();
    dt->SetLastFloatVal(x, 0);
    dt->SetLastFloatVal(un.act, 1);
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
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

  in_subgp = false;
  clmp_net = 0.0f;
  net_scale = 0.0f;
  bias_scale = 0.0f;
  prv_net = 0.0f;
  prv_g_i = 0.0f;

  act_sent = 0.0f;
  act_delta = 0.0f;
  net_raw = 0.0f;
  net_delta = 0.0f;
  g_i_delta = 0.0f;
  g_i_raw = 0.0f;

  i_thr = 0.0f;
  spk_amp = 2.0f;
  misc_1 = 0.0f;
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
}

void AdaptISpec::Initialize() {
  type = NONE;
  tol = .02f;			// allow to be off by this amount before taking action
  p_dt = .1f;			// take reasonably quick action..
  mx_d = .9f;			// move this far in either direction
  l = .2f;			// proportion to assign to leak..
  a_dt = .005f;			// time averaging
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
  clamp_phase2 = false;
}

void LayerLinkSpec::Initialize() {
  link = false;
  gain = 0.5f;
  gp_uses_lay_avg = false;
}
  
void LeabraLayerSpec::Initialize() {
  min_obj_type = &TA_LeabraLayer;
  inhib_group = ENTIRE_LAYER;
  compute_i = KWTA_INHIB;
  i_kwta_pt = .25f;
}

void LeabraLayerSpec::Defaults() {
  adapt_i.Initialize();
  clamp.Initialize();
  decay.Initialize();
  layer_link.Initialize();
  Initialize();
}

void LeabraLayerSpec::UpdateAfterEdit() {
  LayerSpec::UpdateAfterEdit();
}

void LeabraLayerSpec::InitLinks() {
  LayerSpec::InitLinks();
  taBase::Own(kwta, this);
  taBase::Own(gp_kwta, this);
  taBase::Own(adapt_i, this);
  taBase::Own(clamp, this);
  taBase::Own(decay, this);
  taBase::Own(layer_link, this);
}

void LeabraLayerSpec::CutLinks() {
  LayerSpec::CutLinks();
}

bool LeabraLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* tp, bool quiet) {
  LeabraUnit* u;
  taLeafItr ui;
  FOR_ITR_EL(LeabraUnit, u, lay->units., ui) {
    if(!u->CheckConfig(lay, tp, quiet)) return false;
  }
  return true;
}

void LeabraLayerSpec::HelpConfig() {
  String help = "LeabraLayerSpec Configuration:\n\
The layer spec sets the target level of activity, k, for each layer.  \
Therefore, you must have a different layer spec with an appropriate activity \
level for layers that have different activity levels.  Note that if you set \
the activity level by percent this will work for different sized layers that \
have the same percent activity level.";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

void LeabraLayerSpec::InitWtState(LeabraLayer* lay) {
  Compute_Active_K(lay);	// need kwta.pct for init
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->InitWtState();
  InitInhib(lay);		// initialize inhibition at start..
}

void LeabraLayerSpec::SetCurLrate(LeabraLayer* lay, LeabraTrial* trl, int epoch) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->SetCurLrate(trl, epoch);
}


//////////////////////////////////////////
//	Stage 0: at start of settling	// 
//////////////////////////////////////////

void LeabraLayerSpec::Compute_Active_K(LeabraLayer* lay) {
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int totk = 0;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Active_K_impl(lay, rugp, (LeabraInhib*)rugp, gp_kwta);
      totk += rugp->kwta.k;
    }
    if(inhib_group == UNIT_GROUPS) {
      if(layer_link.gp_uses_lay_avg) {
	Compute_Active_K_impl(lay, &(lay->units), (LeabraInhib*)lay, kwta);
      }
      else {
	if(lay->kwta.k != totk)
	  lay->Inhib_ResetSortBuf();
	lay->kwta.k = totk;
	lay->kwta.Compute_Pct(lay->units.leaves);
      }
    }
  }
  if(inhib_group == GPS_THEN_UNITS) {
    // layer level k is now number of unit groups!
    int new_k = 0;
    if(kwta.k_from == KWTASpec::USE_PCT)
      new_k = (int)(kwta.pct * (float)lay->units.gp.size);
    else
      new_k = kwta.k;
    new_k = MIN(lay->units.gp.size - 1, new_k);
    new_k = MAX(1, new_k);
    if(lay->kwta.k != new_k)
      lay->Inhib_ResetSortBuf();
    lay->kwta.k = new_k;
    // but pct should be computed as fctn of pct of groups * pct in groups..
    // take first gp as representative.. 
    LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(0);
    lay->kwta.pct = rugp->kwta.pct * ((float)lay->kwta.k / (float)lay->units.gp.size);
    lay->kwta.pct_c = 1.0f - lay->kwta.pct;
  }
  else if(inhib_group != UNIT_GROUPS) {
    Compute_Active_K_impl(lay, &(lay->units), (LeabraInhib*)lay, kwta);
  }
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
}

int LeabraLayerSpec::Compute_Pat_K(LeabraLayer* lay, Unit_Group* ug, LeabraInhib*) {
  bool use_comp = false;
  if(lay->ext_flag & Unit::COMP) // only use comparison vals if entire lay is COMP!
    use_comp = true;
  int pat_k = 0;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    // use either EXT or TARG information...
    if(u->ext_flag & Unit::EXT) {
      if(u->ext >= kwta.pat_q)
	pat_k++;
    }
    else if(u->ext_flag & Unit::TARG) {
      if(u->targ >= kwta.pat_q)
	pat_k++;
    }
    else if(use_comp && (u->ext_flag | Unit::COMP)) {
      if(u->targ >= kwta.pat_q)
	pat_k++;
    }	      
  }
  return pat_k;
}

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

void LeabraLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(decay.clamp_phase2 && (trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no == 2)) {
    // special case to speed up processing by clamping 2nd plus phase to prior + phase
    lay->SetExtFlag(Unit::EXT);
    lay->hard_clamped = true;
    lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
      u->SetExtFlag(Unit::EXT);
      u->ext = u->act_p;
      u->Compute_HardClamp(lay, trl);
    }
    Compute_ActAvg(lay, trl);
    return;
  }
  if(!(clamp.hard && (lay->ext_flag & Unit::EXT))) {
    lay->hard_clamped = false;
    return;
  }
  lay->hard_clamped = true;	// cache this flag
  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->Compute_HardClamp(lay, trl);
  }
  Compute_ActAvg(lay, trl);
}

void LeabraLayerSpec::Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->Compute_NetScale(lay, trl);
  }
}

void LeabraLayerSpec::Send_ClampNet(LeabraLayer* lay, LeabraTrial* trl) {
  if(!lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Send_ClampNet(lay, trl);
}

//////////////////////////////////
//	Stage 1: netinput 	//
//////////////////////////////////

void LeabraLayerSpec::Send_Net(LeabraLayer* lay) {
  // hard-clamped input layers are already computed in the clmp_net value
  if(lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Send_Net(lay);
}

void LeabraLayerSpec::Send_NetDelta(LeabraLayer* lay) {
  // hard-clamped input layers are already computed in the clmp_net value
  if(lay->hard_clamped) return;

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Send_NetDelta(lay);
}

//////////////////////////////////////////////////////////////////
//	Stage 2: netinput averages and clamping (if necc)	//
//////////////////////////////////////////////////////////////////

void LeabraLayerSpec::Compute_Clamp_NetAvg(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->hard_clamped) return;

  if(lay->ext_flag & Unit::EXT)
    Compute_SoftClamp(lay, trl);

  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    lay->netin.avg = 0.0f;
    lay->netin.max = -FLT_MAX;
    lay->netin.max_i = -1;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_NetAvg(lay, rugp, (LeabraInhib*)rugp, trl);
      // keep maximums for linking purposes
      lay->netin.avg = MAX(lay->netin.avg, rugp->netin.avg);
      if(rugp->netin.max > lay->netin.max) {
	lay->netin.max = rugp->netin.max;
	lay->netin.max_i = g;
      }
    }
  }
  else {
    Compute_NetAvg(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
}

void LeabraLayerSpec::Compute_NetAvg(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl)
{
  thr->netin.avg = 0.0f;
  thr->netin.max = -FLT_MAX;
  thr->netin.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->Compute_NetAvg(lay, thr, trl);
    thr->netin.avg += u->net;
    if(u->net > thr->netin.max) {
      thr->netin.max = u->net;
      thr->netin.max_i = lf;
    }
    lf++;
  }

  if(ug->leaves > 0)
    thr->netin.avg /= (float)ug->leaves;	// turn it into an average
}

void LeabraLayerSpec::Compute_SoftClamp(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  bool inc_gain = false;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    if(!inc_gain)
      inc_gain = u->Compute_SoftClamp(lay, trl);
    else
      u->Compute_SoftClamp(lay, trl);
  }
  if(inc_gain && (trl->cycle > 1)) // only increment after it has a chance to activate
    lay->stm_gain += clamp.d_gain;
}


//////////////////////////////////////////
//	Stage 3: inhibition		//
//////////////////////////////////////////

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
      if(FastEl(st)->i_thr < nw_net)
	return st;
      else if((st+1 < size) && (FastEl(st+1)->i_thr < nw_net))
	return st + 1;
      else if((st+2 < size) && (FastEl(st+2)->i_thr < nw_net))
	return st + 2;
      return st;
    }
    int n2 = n / 2;
    if(FastEl(st + n2)->i_thr < nw_net)
      n = n2;			// search in upper 1/2 of list
    else {
      st += n2; n -= n2;	// search in lower 1/2 of list
    }
  }
}

void LeabraLayerSpec::InitInhib(LeabraLayer* lay) {
  lay->adapt_i.avg_avg = lay->kwta.pct;
  lay->adapt_i.i_kwta_pt = i_kwta_pt;
  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  lay->adapt_i.g_bar_i = us->g_bar.i;
  lay->adapt_i.g_bar_l = us->g_bar.l;
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      rugp->adapt_i.avg_avg = rugp->kwta.pct;
      rugp->adapt_i.i_kwta_pt = i_kwta_pt;
      rugp->adapt_i.g_bar_i = us->g_bar.i;
      rugp->adapt_i.g_bar_l = us->g_bar.l;
    }
  }
}

void LeabraLayerSpec::Compute_Inhib(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->hard_clamped)	return;	// say no more..

  if((inhib_group != UNIT_GROUPS) && (inhib_group != GPS_THEN_UNITS)) {
    Compute_Inhib_impl(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
  if(lay->units.gp.size > 0) {
    int g;
    if(inhib_group != ENTIRE_LAYER) {
      if((inhib_group == UNIT_GROUPS) || (inhib_group == GPS_THEN_UNITS)) {
	lay->Inhib_SetVals(0.0f);
	for(g=0; g<lay->units.gp.size; g++) {
	  LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	  Compute_Inhib_impl(lay, rugp, (LeabraInhib*)rugp, trl);
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
	  Compute_Inhib_impl(lay, rugp, (LeabraInhib*)rugp, trl);
	  // actual inhibition is max of layer and group
	  rugp->i_val.g_i = MAX(rugp->i_val.g_i, lay->i_val.g_i);
	}
      }

      if(inhib_group == GPS_THEN_UNITS) {
	Compute_Inhib_kWTA_Gps(lay, trl);
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

void LeabraLayerSpec::Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
  // for the time being, always compute kwta just to have it around..
  if((compute_i == UNIT_M_KWTA_P) || (compute_i == UNIT_M_KWTA_AVG_P)) {
    if(trl->phase == LeabraTrial::MINUS_PHASE) {
      thr->i_val.g_i = 0.0f;
    }
    else {
      if(compute_i == UNIT_M_KWTA_AVG_P)
	Compute_Inhib_kWTA_Avg(lay, ug, thr, trl);
      else
	Compute_Inhib_kWTA(lay, ug, thr, trl);
      thr->i_val.g_i = thr->i_val.kwta;
    }
  }
  else {
    if(compute_i == UNIT_INHIB) {
      thr->i_val.g_i = 0.0f;	// make sure it's zero, cuz this gets added to units.. 
    }
    else {
      if(compute_i == KWTA_AVG_INHIB)
	Compute_Inhib_kWTA_Avg(lay, ug, thr, trl);
      else 
	Compute_Inhib_kWTA(lay, ug, thr, trl);
      thr->i_val.g_i = thr->i_val.kwta;
    }
  }

  thr->i_val.g_i_orig = thr->i_val.g_i;	// retain original values..
}

void LeabraLayerSpec::Compute_Inhib_kWTA(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one
  k_plus_1 = MIN(ug->leaves,k_plus_1);

  float net_k1 = FLT_MAX;
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
      if(u->i_thr < net_k1) {
	net_k1 = u->i_thr;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (LeabraUnit*)ug->NextEl(i)) {
      if(u->i_thr <=  net_k1) {	// not bigger than smallest one in sort buffer
	thr->inact_buf.Add(u);
	continue;
      }
      thr->inact_buf.Add(thr->active_buf.FastEl(k1_idx)); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf.FastEl(j)->i_thr;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf.FastEl(j);
      if(u->i_thr <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf.FastEl(k1_idx));	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(i)->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k+1 most active units, get the next-highest one
  int k_idx = -1;
  float net_k = FLT_MAX;
  for(j=0; j < k_plus_1; j++) {
    float tmp = thr->active_buf.FastEl(j)->i_thr;
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

  float k1_i = k1_u->i_thr;
  float k_i = k_u->i_thr;

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_i;
  thr->kwta.k1_ithr = k1_i;
  thr->kwta.Compute_IThrR();
}

void LeabraLayerSpec::Compute_Inhib_kWTA_Avg(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }

  int k_plus_1 = thr->kwta.k;	// keep cutoff at k

  float net_k1 = FLT_MAX;
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
      if(u->i_thr < net_k1) {
	net_k1 = u->i_thr;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (LeabraUnit*)ug->NextEl(i)) {
      if(u->i_thr <=  net_k1) {	// not bigger than smallest one in sort buffer
	thr->inact_buf.Add(u);
	continue;
      }
      thr->inact_buf.Add(thr->active_buf.FastEl(k1_idx)); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf.FastEl(j)->i_thr;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf.FastEl(j);
      if(u->i_thr <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf.FastEl(k1_idx));	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(i)->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k most active units, get averages of both groups
  float k_avg = 0.0f;
  for(j=0; j < k_plus_1; j++)
    k_avg += thr->active_buf.FastEl(j)->i_thr;
  k_avg /= (float)k_plus_1;

  float oth_avg = 0.0f;
  for(j=0; j < thr->inact_buf.size; j++)
    oth_avg += thr->inact_buf.FastEl(j)->i_thr;
  if(thr->inact_buf.size > 0)
    oth_avg /= (float)thr->inact_buf.size;

  // place kwta inhibition between two averages
  // this uses the adapting point!
  float pt = i_kwta_pt;
  if(adapt_i.type == AdaptISpec::KWTA_PT)
    pt = thr->adapt_i.i_kwta_pt;
  float nw_gi = oth_avg + pt * (k_avg - oth_avg);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;
  thr->kwta.k_ithr = k_avg;
  thr->kwta.k1_ithr = oth_avg;
  thr->kwta.Compute_IThrR();
}

void LeabraLayerSpec::Compute_Inhib_kWTA_Gps(LeabraLayer* lay, LeabraTrial* trl) {
  // computing the top *groups*, not units here!
  int k_plus_1 = lay->kwta.k;	// only get top k

  float net_k1 = FLT_MAX;
  int k1_idx = 0;
  LeabraUnit_Group* u;
  int i;
  int j;
  if(lay->active_buf.size != k_plus_1) { // need to fill the sort buf..
    lay->active_buf.size = 0;
    for(i = 0; i < k_plus_1; i++) {
      u = (LeabraUnit_Group*)lay->units.gp.FastEl(i), 
      lay->active_buf.Add((LeabraUnit*)u);		// add unit to the list
      if(u->i_val.g_i < net_k1) {
	net_k1 = u->i_val.g_i;	k1_idx = i;
      }
    }
    lay->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; i<lay->units.gp.size; i++) {
      u = (LeabraUnit_Group*)lay->units.gp.FastEl(i);
      if(u->i_val.g_i <=  net_k1) {	// not bigger than smallest one in sort buffer
	lay->inact_buf.Add((LeabraUnit*)u);
	continue;
      }
      lay->inact_buf.Add(lay->active_buf.FastEl(k1_idx)); // now inactive
      lay->active_buf.Replace(k1_idx, (LeabraUnit*)u);// replace the smallest with it
      net_k1 = u->i_val.g_i;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = ((LeabraUnit_Group*)lay->active_buf.FastEl(j))->i_val.g_i;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = ((LeabraUnit_Group*)lay->active_buf.FastEl(j))->i_val.g_i;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < lay->inact_buf.size; j++) {
      u = (LeabraUnit_Group*)lay->inact_buf.FastEl(j);
      if(u->i_val.g_i <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      lay->inact_buf.Replace(j, lay->active_buf.FastEl(k1_idx));	// now inactive
      lay->active_buf.Replace(k1_idx, (LeabraUnit*)u);// replace the smallest with it
      net_k1 = u->i_val.g_i;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = ((LeabraUnit_Group*)lay->active_buf.FastEl(i))->i_val.g_i;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k most active groups - go through groups and set large inhib in 
  // inactive ones!
  for(j=0; j < lay->inact_buf.size; j++) {
    u = (LeabraUnit_Group*)lay->inact_buf.FastEl(j);
    // set inhib to more than what is needed to keep *max* unit below threshold..
    LeabraUnit* un = (LeabraUnit*)u->Leaf(0);// get first unit 
    float tnet = un->net;
    un->net = u->netin.max;	// set to max
    u->i_val.g_i = 1.2f * un->Compute_IThresh(lay, trl);
    un->net = tnet;
  }
}

//////////////////////////////////////////
//	Stage 3.5: Inhib Avg	 	//
//////////////////////////////////////////

void LeabraLayerSpec::Compute_LinkInhib(LeabraLayer* lay, Unit_Group*, LeabraInhib* thr, LeabraTrial*)
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

void LeabraLayerSpec::Compute_InhibAvg(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing

  if(lay->units.gp.size > 0) {	// even if ENTIRE_LAYER, do it by sub-group to get stats..
    lay->un_g_i.avg = 0.0f;
    lay->un_g_i.max = -FLT_MAX;
    lay->un_g_i.max_i = -1;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_InhibAvg_impl(lay, rugp, (LeabraInhib*)rugp, trl);
      lay->un_g_i.avg += rugp->un_g_i.avg * (float)rugp->leaves; // weight by no of units
      if(rugp->un_g_i.max > lay->un_g_i.max) {
	lay->un_g_i.max = rugp->un_g_i.max;
	lay->un_g_i.max_i = g;
      }
    }
  }
  else {
    Compute_InhibAvg_impl(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
}

void LeabraLayerSpec::Compute_InhibAvg_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl)
{
  if(layer_link.link)
    Compute_LinkInhib(lay, ug, thr, trl);

  // now actually go through and set the gc.i for each unit
  thr->un_g_i.avg = 0.0f;
  thr->un_g_i.max = -FLT_MAX;
  thr->un_g_i.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->Compute_InhibAvg(lay, thr, trl);
    thr->un_g_i.avg += u->gc.i;
    if(u->gc.i > thr->un_g_i.max) {
      thr->un_g_i.max = u->gc.i;
      thr->un_g_i.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 0)
    thr->un_g_i.avg /= (float)ug->leaves;	// turn it into an average
}

//////////////////////////////////////////
//	Stage 4: the final activation 	//
//////////////////////////////////////////

void LeabraLayerSpec::Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts.avg = 0.0f;
  thr->acts.max = -FLT_MAX;
  thr->acts.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    thr->acts.avg += u->act_eq;
    if(u->act_eq > thr->acts.max) {
      thr->acts.max = u->act_eq;  thr->acts.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 0) thr->acts.avg /= (float)ug->leaves;
}

void LeabraLayerSpec::Compute_ActAvg(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->units.gp.size > 0) {
    lay->acts.avg = 0.0f;
    lay->acts.max = -FLT_MAX;
    lay->acts.max_i = -1;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_ActAvg_ugp(lay, rugp, (LeabraInhib*)rugp, trl);
      lay->acts.avg += rugp->acts.avg * (float)rugp->leaves; // weight by no of units
      if(rugp->acts.max > lay->acts.max) {
	lay->acts.max = rugp->acts.max; lay->acts.max_i = g;
      }
    }
    if(lay->units.leaves > 0) lay->acts.avg /= (float)lay->units.leaves;
  }
  else {
    Compute_ActAvg_ugp(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
}

void LeabraLayerSpec::Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts_m.avg = 0.0f;
  thr->acts_m.max = -FLT_MAX;
  thr->acts_m.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    thr->acts_m.avg += u->act_m;
    if(u->act_m > thr->acts_m.max) {
      thr->acts_m.max = u->act_m;  thr->acts_m.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 0) thr->acts_m.avg /= (float)ug->leaves;
}

void LeabraLayerSpec::Compute_ActMAvg(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->units.gp.size > 0) {
    lay->acts_m.avg = 0.0f;
    lay->acts_m.max = -FLT_MAX;
    lay->acts_m.max_i = -1;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_ActMAvg_ugp(lay, rugp, (LeabraInhib*)rugp, trl);
      lay->acts_m.avg += rugp->acts_m.avg * (float)rugp->leaves; // weight by no of units
      if(rugp->acts_m.max > lay->acts_m.max) {
	lay->acts_m.max = rugp->acts_m.max; lay->acts_m.max_i = g;
      }
    }
    if(lay->units.leaves > 0) lay->acts_m.avg /= (float)lay->units.leaves;
  }
  else {
    Compute_ActMAvg_ugp(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
}

void LeabraLayerSpec::Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts_p.avg = 0.0f;
  thr->acts_p.max = -FLT_MAX;
  thr->acts_p.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    thr->acts_p.avg += u->act_p;
    if(u->act_p > thr->acts_p.max) {
      thr->acts_p.max = u->act_p;  thr->acts_p.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 0) thr->acts_p.avg /= (float)ug->leaves;
}

void LeabraLayerSpec::Compute_ActPAvg(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->units.gp.size > 0) {
    lay->acts_p.avg = 0.0f;
    lay->acts_p.max = -FLT_MAX;
    lay->acts_p.max_i = -1;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_ActPAvg_ugp(lay, rugp, (LeabraInhib*)rugp, trl);
      lay->acts_p.avg += rugp->acts_p.avg * (float)rugp->leaves; // weight by no of units
      if(rugp->acts_p.max > lay->acts_p.max) {
	lay->acts_p.max = rugp->acts_p.max; lay->acts_p.max_i = g;
      }
    }
    if(lay->units.leaves > 0) lay->acts_p.avg /= (float)lay->units.leaves;
  }
  else {
    Compute_ActPAvg_ugp(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }
}

void LeabraLayerSpec::Compute_Act(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing

  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_Act_impl(lay, rugp, (LeabraInhib*)rugp, trl);
    }
  }
  else {
    Compute_Act_impl(lay, &(lay->units), (LeabraInhib*)lay, trl);
  }

  Compute_ActAvg(lay, trl);
}

void LeabraLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl)
{
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    u->Compute_Act(lay,thr,trl);
  }
}


//////////////////////////////////////////
//	Stage 5: Between Events 	//
//////////////////////////////////////////

void LeabraLayerSpec::PhaseInit(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->ext_flag & Unit::TARG) {	// only process target layers..
    if(trl->phase == LeabraTrial::PLUS_PHASE)
      lay->SetExtFlag(Unit::EXT);
  }
  else {
    if(clamp.hard) return;	// not target and hard-clamped -- bail
  }
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->PhaseInit(lay, trl);
  }
}

void LeabraLayerSpec::DecayEvent(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayEvent(lay, trl, decay.event);
  }
}
  
void LeabraLayerSpec::DecayPhase(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayPhase(lay, trl, decay.phase);
  }
}

void LeabraLayerSpec::DecayPhase2(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->DecayPhase(lay, trl, decay.phase2);
  }
}
  
void LeabraLayerSpec::ExtToComp(LeabraLayer* lay, LeabraTrial* trl) {
  if(!(lay->ext_flag & Unit::EXT))	// only process ext
    return;
  lay->ext_flag = Unit::COMP;	// totally reset to comparison
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->ExtToComp(lay, trl);
}

void LeabraLayerSpec::TargExtToComp(LeabraLayer* lay, LeabraTrial* trl) {
  if(!(lay->ext_flag & Unit::TARG_EXT))	// only process w/ external input
    return;
  lay->ext_flag = Unit::COMP;	// totally reset to comparison
    
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->TargExtToComp(lay, trl);
}

void LeabraLayerSpec::AdaptGBarI(LeabraLayer* lay, LeabraTrial* trl) {
  if(((compute_i == LeabraLayerSpec::UNIT_M_KWTA_P) ||
      (compute_i == LeabraLayerSpec::UNIT_M_KWTA_AVG_P)) && 
     (trl->phase == LeabraTrial::PLUS_PHASE))
    return;			// don't addapt on plus phase if g_bar.i has no effect!

  float diff = lay->kwta.pct - lay->acts.avg;
  if(fabs(diff) > adapt_i.tol) {
    float p_i = 1.0f;
    if(adapt_i.type == AdaptISpec::G_BAR_IL) {
      p_i = 1.0f - adapt_i.l;
    }
    lay->adapt_i.g_bar_i -= p_i * adapt_i.p_dt * diff;
    LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
    float minv = us->g_bar.i * (1.0 - adapt_i.mx_d);
    float maxv = us->g_bar.i * (1.0 + adapt_i.mx_d);
    if(lay->adapt_i.g_bar_i < minv) lay->adapt_i.g_bar_i = minv;
    if(lay->adapt_i.g_bar_i > maxv) lay->adapt_i.g_bar_i = maxv;
    if(adapt_i.type == AdaptISpec::G_BAR_IL) {
      lay->adapt_i.g_bar_l -= adapt_i.l * adapt_i.p_dt * diff;
      LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
      float minv = us->g_bar.l * (1.0 - adapt_i.mx_d);
      float maxv = us->g_bar.l * (1.0 + adapt_i.mx_d);
      if(lay->adapt_i.g_bar_l < minv) lay->adapt_i.g_bar_l = minv;
      if(lay->adapt_i.g_bar_l > maxv) lay->adapt_i.g_bar_l = maxv;
    }
  }
}

void LeabraLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if(set_both) {
    lay->acts_m = lay->acts;
    lay->acts_p = lay->acts;
    lay->phase_dif_ratio = 1.0f;

    if(lay->units.gp.size > 0) {
      int g;
      for(g=0; g<lay->units.gp.size; g++) {
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
	rugp->acts_m = rugp->acts;
	rugp->acts_p = rugp->acts;
	rugp->phase_dif_ratio = 1.0f;
      }
    }
  }
  else {
    if(trl->phase == LeabraTrial::MINUS_PHASE)
      lay->acts_m = lay->acts;
    else
      lay->acts_p = lay->acts;
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

  if((adapt_i.type == AdaptISpec::G_BAR_I) || (adapt_i.type == AdaptISpec::G_BAR_IL)) {
    AdaptGBarI(lay, trl);
  }

  lay->prv_phase = trl->phase;	// record previous phase
}

//////////////////////////////////////////
//	Stage 6: Learning 		//
//////////////////////////////////////////

void LeabraLayerSpec::AdaptKWTAPt(LeabraLayer* lay, LeabraTrial*) {
  if((lay->ext_flag & Unit::EXT) && !(lay->ext_flag & Unit::TARG))
    return;			// don't adapt points for input-only layers
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      // use minus phase activations to adapt
      rugp->adapt_i.avg_avg += adapt_i.a_dt * (rugp->acts_m.avg - rugp->adapt_i.avg_avg);
      float dif = rugp->adapt_i.avg_avg - rugp->kwta.pct;
      if(dif < -adapt_i.tol) {	// average is less than target
	// so reduce the point towards lower value
	rugp->adapt_i.i_kwta_pt += adapt_i.p_dt * 
	  ((i_kwta_pt - adapt_i.mx_d) - rugp->adapt_i.i_kwta_pt);
      }
      else if(dif > adapt_i.tol) {	// average is more than target
	// so increase point towards higher value
	rugp->adapt_i.i_kwta_pt += adapt_i.p_dt * 
	  ((i_kwta_pt + adapt_i.mx_d) - rugp->adapt_i.i_kwta_pt);
      }
    }
  }
  lay->adapt_i.avg_avg += adapt_i.a_dt * (lay->acts_m.avg - lay->adapt_i.avg_avg);
  float dif = lay->adapt_i.avg_avg - lay->kwta.pct;
  if(dif < -adapt_i.tol) {	// average is less than target
    // so reduce the point towards lower value
    lay->adapt_i.i_kwta_pt += adapt_i.p_dt * 
      ((i_kwta_pt - adapt_i.mx_d) - lay->adapt_i.i_kwta_pt);
  }
  else if(dif > adapt_i.tol) {	// average is more than target
    // so increase point towards higher value
    lay->adapt_i.i_kwta_pt += adapt_i.p_dt * 
      ((i_kwta_pt + adapt_i.mx_d) - lay->adapt_i.i_kwta_pt);
  }
}

void LeabraLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_dWt(lay, trl);
  AdaptKWTAPt(lay, trl);
}

void LeabraLayerSpec::Compute_WtFmLin(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_WtFmLin(lay, trl);
}

void LeabraLayer::UpdateWeights() {
  Layer::UpdateWeights();
}

//////////////////////////
// 	LeabraLayer	//
//////////////////////////
  
void AvgMaxVals::Initialize() {
  avg = max = 0.0f; max_i = -1;
}

void AvgMaxVals::Copy_(const AvgMaxVals& cp) {
  avg = cp.avg; max = cp.max; max_i = cp.max_i;
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

void KWTAVals::Compute_IThrR() {
  if(k1_ithr <= 0.0f)
    ithr_r = 0.0f;
  else 
    ithr_r = logf(k_ithr / k1_ithr);
}

void AdaptIVals::Initialize() {
  avg_avg = 0.0f;
  i_kwta_pt = 0.0f;
  g_bar_i = 1.0f;
  g_bar_l = .1f;
}

void AdaptIVals::Copy_(const AdaptIVals& cp) {
  avg_avg = cp.avg_avg;
  i_kwta_pt = cp.i_kwta_pt;
  g_bar_i = cp.g_bar_i;
  g_bar_l = cp.g_bar_l;
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
  taBase::DelPointer((TAPtr)layer);
}

void LayerLink::Copy_(const LayerLink& cp) {
  taBase::SetPointer((TAPtr)layer, cp.layer);
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

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);
  taBase::Own(adapt_i, this);

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
  spec.CheckSpec();
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

bool LeabraLayer::CheckTypes() {
  if(!spec.CheckSpec()) return false;
  return Layer::CheckTypes();
}

void LeabraLayer::Build() {
  ResetSortBuf();
  Layer::Build();
}

void LeabraUnit_Group::Initialize() {
  Inhib_Initialize();
  misc_state = 0;
}

void LeabraUnit_Group::InitLinks() {
  Unit_Group::InitLinks();
  taBase::Own(netin, this);
  taBase::Own(acts, this);

  taBase::Own(acts_p, this);
  taBase::Own(acts_m, this);

  taBase::Own(kwta, this);
  taBase::Own(i_val, this);
  taBase::Own(adapt_i, this);
}

void LeabraUnit_Group::Copy_(const LeabraUnit_Group& cp) {
  Inhib_Copy_(cp);
  misc_state = cp.misc_state;
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
  if(leabra_settle == NULL) return;
  network->InitDelta();		// this is done first because of sender-based net
  if(leabra_settle->send_delta) {
    LeabraLayer* l;
    taLeafItr i;
    FOR_ITR_EL(LeabraLayer, l, network->layers., i) {
      if(!l->lesion)
	l->Send_NetDelta();
    }
#ifdef DMEM_COMPILE
    network->dmem_share_units.Sync(3);
#endif
  }
  else {
    network->Send_Net();
  }
}

void LeabraCycle::Compute_Clamp_NetAvg() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Clamp_NetAvg(leabra_settle->leabra_trial);
  }
}

void LeabraCycle::Compute_Inhib() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Inhib(leabra_settle->leabra_trial);
  }
}

void LeabraCycle::Compute_InhibAvg() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_InhibAvg(leabra_settle->leabra_trial);
  }
}

void LeabraCycle::Compute_Act() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_Act(leabra_settle->leabra_trial);
  }
}

void LeabraCycle::Loop() {
  if((leabra_settle == NULL) || (leabra_settle->leabra_trial == NULL))
    return;
  leabra_settle->leabra_trial->cycle = leabra_settle->cycle.val;
  if((leabra_settle->cycle.val % leabra_settle->netin_mod) == 0) {
    Compute_Net();
    Compute_Clamp_NetAvg();
    Compute_Inhib();
    Compute_InhibAvg();
  }
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
  min_cycles_phase2 = 15;
  netin_mod = 1;
  send_delta = false;
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
      lay->DecayEvent(leabra_trial);
  }
}

void LeabraSettle::DecayPhase() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayPhase(leabra_trial);
  }
}

void LeabraSettle::DecayPhase2() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayPhase2(leabra_trial);
  }
}

void LeabraSettle::PhaseInit() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->PhaseInit(leabra_trial);
  }
}

void LeabraSettle::ExtToComp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->ExtToComp(leabra_trial);
  }
}

void LeabraSettle::TargExtToComp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->TargExtToComp(leabra_trial);
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
      lay->Compute_NetScale(leabra_trial);
  }
}

void LeabraSettle::Send_ClampNet() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Send_ClampNet(leabra_trial);
  }
#ifdef DMEM_COMPILE
  if(send_delta)
    network->dmem_share_units.Sync(4);
#endif
}

void LeabraSettle::PostSettle() {
  bool set_both = false;
  if((leabra_trial != NULL) && (leabra_trial->epoch_proc != NULL)) {
    if((leabra_trial->phase_order == LeabraTrial::PLUS_ONLY) ||
       (leabra_trial->no_plus_test && (leabra_trial->epoch_proc->wt_update == EpochProcess::TEST)))
      set_both = true;
  }
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->PostSettle(leabra_trial, set_both);
  }
}

void LeabraSettle::PostSettle_NStdLay() {
  bool set_both = false;
  if((leabra_trial != NULL) && (leabra_trial->epoch_proc != NULL)) {
    if((leabra_trial->phase_order == LeabraTrial::PLUS_ONLY) ||
       (leabra_trial->no_plus_test && (leabra_trial->epoch_proc->wt_update == EpochProcess::TEST)))
      set_both = true;
  }
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion && (lay->spec.spec->GetTypeDef() != &TA_LeabraLayerSpec))
      lay->PostSettle(leabra_trial, set_both);
  }
}

void LeabraSettle::Compute_WtFmLin() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_WtFmLin(leabra_trial);
  }
}

void LeabraSettle::Compute_dWt_NStdLay() {
  leabra_trial->Compute_dWt_NStdLay();	// fctn is complicated, use trial version..
}

void LeabraSettle::Compute_dWt() {
  leabra_trial->Compute_dWt();	// fctn is complicated, use trial version..
  Compute_WtFmLin();		// if being computed here, means we need to revert to real wts
}

void LeabraSettle::UpdateWeights() {
  network->UpdateWeights();
}

void LeabraSettle::Init_impl() {
  SettleProcess::Init_impl();
  if((leabra_trial == NULL) || (leabra_trial->cur_event == NULL) || (network == NULL))
    return;

  leabra_trial->cycle = -2;
  if(leabra_trial->cur_event->InheritsFrom(TA_DurEvent)) {
    cycle.SetMax((int)((DurEvent*)leabra_trial->cur_event)->duration);
  }

  network->InitExterns();
  if(leabra_trial->cur_event != NULL)
    leabra_trial->cur_event->ApplyPatterns(network);

  if(leabra_trial->phase_no >= 3) { // second plus phase or more
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

  Compute_Active_K();		// compute here because could depend on pat_n
  PhaseInit();
  
  Compute_HardClamp();		// first clamp all hard-clamped input acts
  Compute_NetScale();		// and then compute net scaling
  Send_ClampNet();		// and send net from clamped inputs
}	

void LeabraSettle::Final() {
  if((leabra_trial == NULL) || (leabra_trial->epoch_proc == NULL)) return;
  // compute weight changes at end of second settle..
  PostSettle();
  if((leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_NOTHING) &&
     (leabra_trial->phase_no == 1)) {
    if(leabra_trial->epoch_proc->wt_update != EpochProcess::TEST)
      Compute_dWt();
  }
  else if((leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_PLUS) &&
	  (leabra_trial->phase_no == 1)) {
    if(leabra_trial->epoch_proc->wt_update != EpochProcess::TEST) {
      if(leabra_trial->first_plus_dwt == LeabraTrial::NO_FIRST_DWT)
	Compute_dWt_NStdLay();	// only do non-standard ones
      else if(leabra_trial->first_plus_dwt == LeabraTrial::ALL_DWT)
	Compute_dWt();
      else if(leabra_trial->first_plus_dwt == LeabraTrial::ONLY_FIRST_DWT) {
	leabra_trial->Compute_dWt();
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
  trial_init = DECAY_STATE;
  first_plus_dwt = ONLY_FIRST_DWT;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  min_con_group =  &TA_LeabraCon_Group;
  min_con =  &TA_LeabraCon;
}

void LeabraTrial::InitLinks() {
  TrialProcess::InitLinks();
  taBase::Own(phase_no, this);
  if(!taMisc::is_loading && (loop_stats.size == 0)) {
    LeabraSE_Stat* st = (LeabraSE_Stat*)loop_stats.NewEl(1, &TA_LeabraSE_Stat);
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
  if((phase_order == PLUS_ONLY) && no_plus_stats) {
    taMisc::Error("LeabraTrial: With phase_order == PLUS_ONLY, need to un-set no_plus_stats.  I just did that for you.");
    no_plus_stats = false;
  }
  TrialProcess::UpdateAfterEdit();
}

void LeabraTrial::C_Code() {
  bool stop_crit = false;	// a stopping criterion was reached
  bool stop_force = false;	// either the Stop button was pressed or our Step level was reached

  if(re_init) {
    Init();
    InitProcs();
  }

  if((cur_event != NULL) && (cur_event->InheritsFrom(TA_DurEvent)) &&
     (((DurEvent*)cur_event)->duration <= 0)) {
    SetReInit(true);
    return;
  }

  do {
    Loop();			// user defined code goes here
    if(!bailing) {	
      UpdateCounters();
      LoopProcs();		// check/run loop procs (using mod based on counter)
      if(!no_plus_stats || (phase == MINUS_PHASE)) {
	LoopStats();		// udpate in-loop stats
	if(log_loop)
	  UpdateLogs();		// generate log output
      }
      UpdateState();		// update process state vars

      stop_crit = Crit();   	// check if stopping criterion was reached
      if(!stop_crit) {		// if at critera, going to quit anyway, so don't
	stop_force = StopCheck(); // check for stopping (either by Stop button or Step)
      }
    }
  }
  while(!bailing && !stop_crit && !stop_force);
  // loop until we reach criterion (e.g. ctr > max) or are forcibly stopped

  if(stop_crit) {
    Final();
    FinalProcs();
    FinalStats();
    if(!log_loop)
      UpdateLogs();
    UpdateDisplays();		// update displays after the loop
    SetReInit(true);		// made it through the loop
    FinalStepCheck();		// always stop at end if i'm the step process
  }
  else {
    bailing = true;
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
      lay->SetCurLrate(this, network->epoch);
  }
}

void LeabraTrial::DecayState() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayEvent(this);
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
      u->EncodeState(lay, this);
  }
}

void LeabraTrial::Compute_dWt() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_dWt(this);
  }
}

void LeabraTrial::Compute_dWt_NStdLay() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    if(lay->spec.spec->GetTypeDef() != &TA_LeabraLayerSpec)
      lay->Compute_dWt(this);
  }
}

void LeabraTrial::UpdateWeights() {
  network->UpdateWeights();
}

void LeabraTrial::Compute_WtFmLin() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(lay->lesion)	continue;
    lay->Compute_WtFmLin(this);
  }
}

void LeabraTrial::Init_impl() {
  TrialProcess::Init_impl();

  cycle = -1;
  phase = MINUS_PHASE;
  phase_no.SetMax(2);
  bool is_testing = false;

  if(no_plus_test && (epoch_proc != NULL) && 
     (epoch_proc->wt_update == EpochProcess::TEST))
  {
    phase_no.SetMax(1);		// just do one loop (the minus phase)
    is_testing = true;
  }
  if(cur_event != NULL) {
    if(cur_event->spec->InheritsFrom(TA_PhaseOrderEventSpec)) {
      PhaseOrderEventSpec* es = (PhaseOrderEventSpec*)cur_event->spec.spec;
      if(es->phase_order == PhaseOrderEventSpec::MINUS_PLUS)
	phase = MINUS_PHASE;
      else if(es->phase_order == PhaseOrderEventSpec::MINUS_ONLY) {
	phase = MINUS_PHASE;
	phase_no.SetMax(1);
      }
      else if(es->phase_order == PhaseOrderEventSpec::PLUS_MINUS) {
	phase = PLUS_PHASE;
	if(is_testing)
	  phase_no.SetMax(2);	// need to present plus phase first, then minus..
      }
      else if(es->phase_order == PhaseOrderEventSpec::PLUS_ONLY) {
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
  }

  if(trial_init == INIT_STATE)
    InitState();
  else if(trial_init == DECAY_STATE)
    DecayState();
}

void LeabraTrial::Loop() {
  if(phase_no.val == 0) {	// do this here so it can pick up any changes from InitProcs()!
    SetCurLrate();
  }
  if(phase_no.val < phase_no.max)
    if(sub_proc != NULL) sub_proc->Run();
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
      if(phase_no == 1)
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
    if((phase_order == MINUS_PLUS_PLUS) && (first_plus_dwt == ONLY_FIRST_DWT))
      Compute_dWt_NStdLay();	// only update the non-standard layers, which will have checks
    else
      Compute_dWt();		// get them all
  }
  EncodeState();
}

void LeabraTrial::GetCntrDataItems() {
  if(cntr_items.size < 2)
    cntr_items.EnforceSize(2);
  TrialProcess::GetCntrDataItems();
  DataItem* it = (DataItem*)cntr_items.FastEl(1);
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

bool LeabraTrial::CheckNetwork() {
  if(network && (network->dmem_sync_level != Network::DMEM_SYNC_NETWORK)) {
    network->dmem_sync_level = Network::DMEM_SYNC_NETWORK;
  }
  return TrialProcess::CheckNetwork();
}

bool LeabraTrial::CheckUnit(Unit* ck) {
  bool rval = TrialProcess::CheckUnit(ck);
  if(!rval) return rval;
  LeabraSettle* setl = (LeabraSettle*)FindSubProc(&TA_LeabraSettle);
  if(setl == NULL) return rval;
  LeabraUnitSpec* us = (LeabraUnitSpec*)ck->spec.spec;
  us->act.send_delta = setl->send_delta;
  return rval;
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
  loop_init = INIT_START_ONLY;
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
  net_agg.ComputeAgg(da.val, fda);
}

void LeabraMaxDa::Network_Stat() {
  if(settle_proc == NULL) return;
  int mincyc = settle_proc->min_cycles;
  if(settle_proc->leabra_trial != NULL) {
    if(settle_proc->leabra_trial->phase_no > 1)
      mincyc = settle_proc->min_cycles_phase2;
  }
  if(settle_proc->cycle.val < mincyc) {
    da.val = MAX(da.val, da.stopcrit.val + .01); // keep above thresh!
  }
}


//////////////////////////
//  	Ae Stat		//
//////////////////////////

void LeabraSE_Stat::Initialize() {
  targ_or_comp = Unit::COMP_TARG;
  trial_proc = NULL;
  tolerance = .5;
  no_off_err = false;
}

void LeabraSE_Stat::UpdateAfterEdit() {
  SE_Stat::UpdateAfterEdit();
  if(own_proc == NULL) return;
  trial_proc = (LeabraTrial*)own_proc->FindProcOfType(&TA_LeabraTrial);
  if(time_agg.real_stat == NULL)
    return;
  LeabraSE_Stat* rstat = (LeabraSE_Stat*)time_agg.real_stat;
  // don't rename unless one or the other
  if((rstat->targ_or_comp != Unit::TARG) && (rstat->targ_or_comp != Unit::COMP))
    return;
  MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
  name += String("_") + md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
}

void LeabraSE_Stat::NameStatVals() {
  Stat::NameStatVals();
  se.AddDispOption("MIN=0");
  se.AddDispOption("TEXT");
  if(time_agg.real_stat == NULL)
    return;
  LeabraSE_Stat* rstat = (LeabraSE_Stat*)time_agg.real_stat;
  // don't rename unless one or the other
  if((rstat->targ_or_comp != Unit::TARG) && (rstat->targ_or_comp != Unit::COMP))
    return;
  MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
  String vlnm = md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
  vlnm.downcase();
  se.name += String("_") + vlnm.at(0,4);
}

void LeabraSE_Stat::CutLinks() {
  trial_proc = NULL;
  SE_Stat::CutLinks();
}

void LeabraSE_Stat::Init() {
  // don't init if it's not the right time!
  if((time_agg.from == NULL) && (trial_proc != NULL)) {
    // run targ-only comparisons in the first minus phase only
    if((targ_or_comp == Unit::TARG) && (trial_proc->phase_no != 1))
      return;
    // run comp-only comparisons in the last minus phase only
    if((targ_or_comp == Unit::COMP) && (trial_proc->phase_no != 3))
      return;
  }
  SE_Stat::Init();
}

void LeabraSE_Stat::Network_Run() {
  if(trial_proc != NULL) {
    // run targ-only comparisons in the first minus phase only
    if((targ_or_comp == Unit::TARG) && (trial_proc->phase_no != 1))
      return;
    // run comp-only comparisons in the last minus phase only
    if((targ_or_comp == Unit::COMP) && (trial_proc->phase_no != 3))
      return;
  }
  SE_Stat::Network_Run();
}

void LeabraSE_Stat::Unit_Stat(Unit* unit) {
  if(!(unit->ext_flag & targ_or_comp))
    return;
  float act = ((LeabraUnit*)unit)->act_eq;
  float tmp = 0.0f;
  if(no_off_err) {
    if(act > tolerance) {	// was active
      if(unit->targ < tolerance) // shouldn't have been
	tmp = 1.0f;
    }
  }
  else {
    tmp = fabsf(act - unit->targ);
    if(tmp >= tolerance)
      tmp *= tmp;
    else
      tmp = 0.0f;
  }
  net_agg.ComputeAgg(se.val, tmp);
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

  net_agg.ComputeAggNoUpdt(hrmny.val, lun->act * harm);

  float act_c = 1.0f - lun->act_eq;
  float stress = 0.0f;
  if(lun->act_eq > 0.0f) stress += lun->act_eq * logf(lun->act_eq);
  if(act_c > 0.0f) stress += act_c * logf(act_c);
  net_agg.ComputeAgg(strss.val, -stress);
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
  taBase::DelPointer((TAPtr)trg_lay);
}

void WrongOnStat::NameStatVals() {
  Stat::NameStatVals();
  wrng.AddDispOption("MIN=0");
  wrng.AddDispOption("TEXT");
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
  taBase::SetPointer((TAPtr)trg_lay, nw_lay);
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
    net_agg.ComputeAgg(wrng.val, err);
  }
  if(no_above_thr)		// get a point off if nobody above threshold!
    net_agg.ComputeAgg(wrng.val, 1.0f);
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

void LeabraACLayerSpec::Initialize() {
  un_gain_spec.SetBaseType(&TA_LeabraUnitSpec);
  mod_type = MOD_NOTHING;
  un_gain_range.min = 400.0f;
  un_gain_range.max = 800.0f;
  un_gain_range.UpdateAfterEdit();
  err_delta = .2f;
  SetUnique("decay", true);
  decay.event = 0.0f;
  decay.phase = 0.0f;
}

void LeabraACLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(rew, this);
  taBase::Own(un_gain_range, this);
  un_gain_spec.SetDefaultSpec(this);
}

void LeabraACLayerSpec::CutLinks() {
  LeabraLayerSpec::CutLinks();
  un_gain_spec.CutLinks();
}

void LeabraACLayerSpec::UpdateAfterEdit() {
  LeabraLayerSpec::UpdateAfterEdit();
  un_gain_range.UpdateAfterEdit();
  rew.UpdateAfterEdit();
}

void LeabraACLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  rew.Initialize();
  Initialize();
}

bool LeabraACLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;
  // todo: do this!
  return true;
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

void LeabraACLayerSpec::ExtToComp(LeabraLayer*, LeabraTrial*) {
  // do nothing!
}

// TD signal is act_dif

void LeabraACLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    taMisc::Error("LeabraTrial: LeabraACLayerSpec requires trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  LeabraUnit* acu = (LeabraUnit*)lay->units.Leaf(0);	// get the unit
  if(acu == NULL) return;

  // modulate activity by prior plus-phase value if applicable
  if((mod_type != MOD_NOTHING) && (un_gain_spec.spec != NULL)) {

    float td = acu->act_dif;
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
    lay->InitExterns();
    return;			
  }

  if(trl->phase == LeabraTrial::PLUS_PHASE) {
    bool got_rew = (acu->ext > 0.0f);
    if(!got_rew) { // not receiving some reward
      lay->hard_clamped = false; // don't clamp it
      lay->InitExterns();
      return;			// let it ride
    }
  }

  lay->hard_clamped = true;	// cache this flag
  lay->SetExtFlag(Unit::EXT);	// make sure this is set, regardless
  lay->Inhib_SetVals(i_kwta_pt); // assume 0 - 1 clamped inputs
  acu->SetExtFlag(Unit::EXT);

  if(trl->phase == LeabraTrial::MINUS_PHASE) {
    if(rew.reset && (acu->clmp_net == -10.0f)) {
      // reset after reward (clmp_net marks reward)
      acu->ext = rew.val;
    }
    else {
      // use prior plus phase, which was v(t+1) and is now v(t)
      // but assume that v(t+1) was discounted so, now "undiscount" it
      acu->ext = rew.inv_disc * acu->act_p;
    }
    acu->Compute_HardClamp(lay, trl);
    acu->act_eq = acu->act = acu->ext; // avoid clamp_range
    acu->clmp_net = 0.0f;	// reset this flag
  }
  else {
    acu->clmp_net = -10.0f;	// set the flag for having been clamped
    acu->Compute_HardClamp(lay, trl); // clamp plus phase reward, use clamp_range
  }
  lay->acts.max = lay->acts.avg = acu->act_eq;
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
  act_range.UpdateAfterEdit();
  clamp_range.max = 1.0f;
  clamp_range.UpdateAfterEdit();
  act.gain = 2;
}

void LeabraLinUnitSpec::Defaults() {
  LeabraUnitSpec::Defaults();
  Initialize();
}

void LeabraLinUnitSpec::Compute_ActFmVm(LeabraUnit* u, LeabraLayer*, LeabraInhib*, LeabraTrial* trl) {
  float new_act = u->net * act.gain; // use linear netin as act

  u->da = new_act - u->act;
  if((noise_type == ACT_NOISE) && (noise.type != Random::NONE) && (trl->cycle >= 0)) {
    new_act += noise_sched.GetVal(trl->cycle) * noise.Gen();
  }
  u->act = u->act_eq = act_range.Clip(new_act);
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
  SetUnique("decay", true);
  decay.event = 0.0f;
  decay.phase = 0.0f;
}

void LeabraContextLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(updt, this);
}

// void LeabraContextLayerSpec::UpdateAfterEdit() {
//   LeabraLayerSpec::UpdateAfterEdit();
//   hysteresis_c = 1.0f - hysteresis;
// }

bool LeabraContextLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;
  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("LeabraTrial: LeabraContextLayerSpec requires trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  return true;
}

void LeabraContextLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  Initialize();
}

void LeabraContextLayerSpec::Compute_Context(LeabraLayer* lay, LeabraUnit* u, LeabraTrial* trl) {
  if(trl->phase == LeabraTrial::PLUS_PHASE) {
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
  u->Compute_HardClamp(lay, trl);
}

void LeabraContextLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  lay->hard_clamped = true;	// cache this flag
  lay->SetExtFlag(Unit::EXT);
  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    Compute_Context(lay, u, trl);
  }
  Compute_ActAvg(lay, trl);
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

void LeabraGatedCtxLayerSpec::Defaults() {
  LeabraContextLayerSpec::Defaults();
  gate.Initialize();
  gate_noise.Initialize();
  Initialize();
}

bool LeabraGatedCtxLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;
  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("LeabraTrial: LeabraContextLayerSpec requires trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(trl->phase_order != LeabraTrial::MINUS_PLUS_PLUS) {
    if(!quiet) taMisc::Error("LeabraTrial: LeabraContextLayerSpec requires phase_oder = MINUX_PLUS_PLUS, I just set it for you");
    trl->phase_order = LeabraTrial::MINUS_PLUS_PLUS;
  }
  // todo: do this!
  return true;
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

void LeabraGatedCtxLayerSpec::Compute_Context(LeabraLayer* lay, LeabraUnit* u, LeabraTrial* trl) {
  // this is only called when it is time to clamp context to previous values
  u->ext = u->act_p;
  u->SetExtFlag(Unit::EXT);
  u->Compute_HardClamp(lay, trl);
}

void LeabraGatedCtxLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  int ac_prjn_idx = FindACLayer(lay);
  if(trl->phase == LeabraTrial::MINUS_PHASE) { // first minus phase: use base setting of to_out
    updt.to_out = base.to_out;
    LeabraConSpec* oc = (LeabraConSpec*)out_con_spec.spec;
    if(oc != NULL)
      UpdateConSpec(oc, updt.to_out);

    if(gate.reset) {		// reset after reward
      bool reset = Get_RewReset(lay, ac_prjn_idx);
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

    float td = Get_TD(lay, ac_prjn_idx);
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
	u->DecayPhase(lay, trl, fabs(lay->td)); // clear out in proportion to td
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
  LeabraContextLayerSpec::PostSettle(lay, trl, set_both);
  if((trl->phase == LeabraTrial::MINUS_PHASE) || (trl->phase_no == 1)) {
    return;
  }
  // only intracell in 2nd plus phase: activate or deactivate based on td
  // do it for entire layer at the same time
  if((gate.maint == GateSpec::INTRACELL) && (lay->td >= 0.0f) && lay->td_updt) {
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
      u->vcb.g_h = u->act_p;
    }
  }
  // now grab the act_p value from 2nd plus phase -- standard PostSettle doesn't do this!
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->act_p = u->act_eq;
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

bool LeabraGatedCtxLayerSpec::Get_RewReset(LeabraLayer* lay, int ac_prjn_idx) {
  if(ac_prjn_idx < 0)
    return false;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[ac_prjn_idx];
  if(ac_cg == NULL)
    return false;
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL)
    return false;
  if(acu->clmp_net == -10.0f)
    return true;
  return false;
}

float LeabraGatedCtxLayerSpec::Get_TD(LeabraLayer* lay, int ac_prjn_idx) {
  if(ac_prjn_idx < 0)
    return 1.0f;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[ac_prjn_idx];
  if(ac_cg == NULL)
    return 1.0f;
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL)
    return 1.0f;
  LeabraUnit* acu2 = NULL;
  if(ac_cg->units.size > 1)
    acu2 = (LeabraUnit*)ac_cg->Un(1);	// get 2nd unit
  float td = acu->act_dif;
  if(acu2 != NULL)
    td = 0.5f * (acu->act_dif - acu2->act_dif); // combine two by default
  return td;
}

int LeabraGatedCtxLayerSpec::FindACLayer(LeabraLayer* lay) {
  int ac_prjn_idx = -1;		// -1 = not found
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  int i;
  for(i=0;i<u->recv.gp.size;i++) {
    LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[i];
    LeabraLayer* fmlay = (LeabraLayer*)ac_cg->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("*** LeabraGatedCtxLayerSpec: null from layer in recv projection:", (String)i);
      return -1;
    }
    if(fmlay->spec.spec->InheritsFrom(TA_LeabraACLayerSpec)) {
      LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
      if(acu == NULL) {
	taMisc::Error("*** LeabraGatedCtxLayerSpec: AC Layer projection doesn't have AC unit!");
	return -1;
      }
      ac_prjn_idx = i;
      break;
    }
  }
  if(ac_prjn_idx < 0) {
    taMisc::Error("*** LeabraGatedCtxLayerSpec: Warning: no projection from AC Layer found -- gating will be set to 1 always!");
  }
  return ac_prjn_idx;
}

////////////////////////////////////////////////////////////////////
//  	AC-based gating of intrisic maintenance (gc.h, gc.a	  //
////////////////////////////////////////////////////////////////////

void LeabraACMaintLayerSpec::Initialize() {
  td_clear_thresh = .5f;
  ignore_td = false;
  min_ac_pred = .1f;
  clear_bwt_pos_td = false;
  ac_prjn_idx = -1;
}

void LeabraACMaintLayerSpec::HelpConfig() {
  String help = "LeabraACMaintLayerSpec Computation:\n\
This layer maintains activation over time (activation-based working memory) via \
excitatory intracelluar ionic mechanisms (implemented via the hysteresis channels, gc.h), \
and excitatory self-connections. These ion channels are turned on and off via TD \
signals received from the AC Layer.  \
Updating occurs in the plus phase --- if a strong TD gating signal is received (either \
positive or negative), any previous ion current is turned off.  Positive TD signals \
turn on gc.h values that support maintenance, while negative TD signals turn on gc.a values \
that tend to deactivate the units.\n\
\nPFCLayerSpec Configuration:\n\
- Units must recv from LeabraACLayerSpec layer for TD gating signal\n\
- This connection must be fixed and have 0 weight (it is essentially a marker)\n\
- A MarkerConSpec MUST be used for this connection if using under DMEM!!!";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

// TODO: make MarkerConSpec mandatory..

bool LeabraACMaintLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  if(decay.clamp_phase2) {
    SetUnique("decay", true);
    decay.clamp_phase2 = false;
  }

  if(trl->phase_order != LeabraTrial::MINUS_PLUS) {
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires LeabraTrial phase_oder = MINUS_PLUS, I just set it for you");
    trl->phase_order = LeabraTrial::MINUS_PLUS;
  }
  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;

  if(us->g_bar.h < .3f) {
    us->SetUnique("g_bar", true);
    us->g_bar.h = .5f;
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires UnitSpec g_bar.h around .5, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->g_bar.a < 1.0f) {
    us->SetUnique("g_bar", true);
    us->g_bar.a = 2.0f;
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires UnitSpec g_bar.a around 2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }

  FindACLayer(lay);
  if(ac_prjn_idx < 0) return false;

  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[ac_prjn_idx];
  if(ac_cg == NULL)
    return false;
  LeabraConSpec* accs = (LeabraConSpec*)ac_cg->spec.spec;
  if(accs->wt_scale.rel != 0.0f) {
    accs->SetUnique("wt_scale", true);
    accs->wt_scale.rel = 0.0f;
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires connection fm AC to have wt_scale.rel = 0, I just set it for you in spec:",
		  accs->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(accs->lrate != 0.0f) {
    accs->SetUnique("lrate", true);
    accs->lrate = 0.0f;
    if(!quiet) taMisc::Error("LeabraACMaintLayerSpec: requires connection fm AC to have lrate = 0, I just set it for you in spec:",
		  accs->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  return true;
}

void LeabraACMaintLayerSpec::FindACLayer(LeabraLayer* lay) {
  ac_prjn_idx = -1;		// -1 = not found
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  int i;
  for(i=0;i<u->recv.gp.size;i++) {
    LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[i];
    LeabraLayer* fmlay = (LeabraLayer*)ac_cg->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("*** LeabraGatedCtxLayerSpec: null from layer in recv projection:", (String)i);
      return;
    }
    if(fmlay->spec.spec->InheritsFrom(TA_LeabraACLayerSpec)) {
      LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
      if(acu == NULL) {
	taMisc::Error("*** LeabraGatedCtxLayerSpec: AC Layer projection doesn't have AC unit!");
	return;
      }
      ac_prjn_idx = i;
      break;
    }
  }
  if(ac_prjn_idx < 0) {
    taMisc::Error("*** LeabraGatedCtxLayerSpec: Warning: no projection from AC Layer found -- gating will be set to 1 always!");
  }
}

bool LeabraACMaintLayerSpec::Get_RewReset(LeabraLayer* lay) {
  if(ac_prjn_idx < 0)
    return false;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[ac_prjn_idx];
  if(ac_cg == NULL)
    return false;
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL)
    return false;
  if(acu->clmp_net == -10.0f)
    return true;
  return false;
}

float LeabraACMaintLayerSpec::Get_TD(LeabraLayer* lay) {
  if(ac_prjn_idx < 0)
    return 1.0f;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* ac_cg = (LeabraCon_Group*)u->recv.gp[ac_prjn_idx];
  if(ac_cg == NULL)
    return 1.0f;
  LeabraUnit* acu = (LeabraUnit*)ac_cg->Un(0);
  if(acu == NULL)
    return 1.0f;
  float pred = MAX(acu->act_m, min_ac_pred);
  float td = acu->targ - pred;
  return td;
}

void LeabraACMaintLayerSpec::PhaseInit(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraLayerSpec::PhaseInit(lay, trl);
  if(ignore_td) return;

  if(trl->phase != LeabraTrial::PLUS_PHASE) return;
  FindACLayer(lay);

  float td = Get_TD(lay);

  if(fabsf(td) > td_clear_thresh) {
    // reset all vcb.g_h values..
    LeabraUnit* u;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
      u->vcb.g_h = 0.0f;
      u->vcb.g_a = 0.0f;
      if(clear_bwt_pos_td && (td > td_clear_thresh)) {
        u->bias->wt = 0.0f;
      }
    }
  }
  
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    if(td < 0.0f) {
      if(u->vcb.g_h > 0.0f)
	u->vcb.g_h += td * u->act_m; // decrease h current (td is neg..)
      else
	u->vcb.g_a += -td * u->act_m; // increase a current
    }
    else {
      if(u->vcb.g_a > 0.0f)
	u->vcb.g_a -= td * u->act_m; // decrease a current
      else
	u->vcb.g_h += td * u->act_m; // increase h current
    }
    if(u->vcb.g_a > 1.0f) u->vcb.g_a = 1.0f;
    else if(u->vcb.g_a < 0.0f) u->vcb.g_a = 0.0f;
    if(u->vcb.g_h > 1.0f) u->vcb.g_h = 1.0f;
    else if(u->vcb.g_h < 0.0f) u->vcb.g_h = 0.0f;
  }
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
// 	Time-Based Learning		//
//////////////////////////////////////////

void TimeMixSpec::Initialize() {
  prv = .5f;
  cur = 1.0f - prv;
}
void TimeMixSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  cur = 1.0f - prv;
}

void LeabraTimeConSpec::Initialize() {
}

void LeabraTimeConSpec::UpdateAfterEdit() {
  LeabraConSpec::UpdateAfterEdit();
  time_mix.UpdateAfterEdit();
}

void LeabraTimeConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(time_mix, this);
}

void LeabraTimeUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraTimeUnit;
  SetUnique("opt_thresh", true);
  opt_thresh.learn = -1.0f;
}

void LeabraTimeUnitSpec::UpdateAfterEdit() {
  LeabraUnitSpec::UpdateAfterEdit();
  if(opt_thresh.learn >= 0.0f) {
    taMisc::Error("LeabraTimeUnitSpec: opt_thresh.learn must be -1 to allow proper recording of p_savg and p_max_cor values in connections",
		  "even when this unit is not active on a prior time step!  I just set it for you.");
    SetUnique("opt_thresh", true);
    opt_thresh.learn = -1.0f;
  }
}

void LeabraTimeUnitSpec::InitState(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  lu->p_act_m = -.1f;
  lu->p_act_p = -.1f;
}

void LeabraTimeUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(act.avg_dt > 0.0f)
    u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  if(((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) &&
     ((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn)))
    return;
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay, trl);
}

void LeabraTimeUnitSpec::UpdateWeights(Unit* u) {
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
  }
  if(opt_thresh.updt_wts && 
     ((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) &&
      ((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn)))
    return;
  UnitSpec::UpdateWeights(lu);
}

void LeabraTimeUnitSpec::EncodeState(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  // just save phase activation states
  lu->p_act_p = lu->act_p;
  lu->p_act_m = lu->act_m;
  
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
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

void LeabraNegBiasSpec::Initialize() {
  decay = 0.0f;
  updt_immed = false;
}

void LeabraMaintConSpec::Initialize() {
  maint_thresh = .9f;
}

void PhaseDWtConSpec::Initialize() {
  dwt_phase = ALL_DWT;
}

void PhaseDWtUnitSpec::Initialize() {
  dwt_phase = ALL_DWT;
}

void PhaseDWtUnitSpec::Compute_dWt_impl(LeabraUnit* u, LeabraLayer*, LeabraTrial* trl) {
  // don't adapt bias weights on clamped inputs..
  if(!((u->ext_flag & Unit::EXT) && !(u->ext_flag & Unit::TARG))) {
    if(dwt_phase == NO_FIRST_DWT) {
      if(trl->phase_no.val != 1) 
	((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
    }
    else if(dwt_phase == ONLY_FIRST_DWT) {
      if(trl->phase_no.val == 1) 
      ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
    }
    else {
      ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
    }
  }
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from->lesion) continue;
    if(recv_gp->spec.spec->InheritsFrom(TA_PhaseDWtConSpec)) {
      PhaseDWtConSpec* sdcg = (PhaseDWtConSpec*)recv_gp->spec.spec;
      if(sdcg->dwt_phase == PhaseDWtConSpec::NO_FIRST_DWT) {
	if(trl->phase_no.val != 1)
	  recv_gp->Compute_dWt(u);
      }
      else if(sdcg->dwt_phase == PhaseDWtConSpec::ONLY_FIRST_DWT) {
	if(trl->phase_no.val == 1) {
	  recv_gp->Compute_dWt(u);
	  recv_gp->UpdateWeights(u); // not going to happen otherwise!
	}
      }
      else {
	recv_gp->Compute_dWt(u);	
      }
    }
    else {
      recv_gp->Compute_dWt(u);
    }
  }
}

//////////////////////////////////
// 	TabledConSpec		//
//////////////////////////////////

void LeabraTabledConSpec::Initialize() {
  table_type = ONE_PHASE;
  squashing = SQUASH_WTS;
  table_mix = 1.0f;
}

void LeabraTabledConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(dwt_table, this);
}

void LeabraTabledConSpec::Defaults() {
  LeabraConSpec::Defaults();
  Initialize();
}

void LeabraTabledConSpec::LoadDWtTable(istream& is) {
  cur_table = taivGetFile::last_fname;
  dwt_table.LoadTable(is);
  int expected_dim;
  switch (table_type) {
  case ONE_PHASE: expected_dim = 2; break;
  case ONE_PHASE_WITH_WT: expected_dim = 3; break;
  case TWO_PHASE: expected_dim = 4; break;
  case TWO_PHASE_WITH_WT: expected_dim = 5; break;
  default: expected_dim = -1;
  }
  if (expected_dim != dwt_table.n_dims) {
    taMisc::Error("Warning: Lookup table dimensions do not match expected number of dimensions for given table type.",
		  "Expected:", String(expected_dim), "Actual:", String(dwt_table.n_dims));
  }
}

float LeabraTabledConSpec::LookupDWt(float su_act_m, float ru_act_m, 
				     float su_act_p, float ru_act_p, 
				     float wt)
{
  if((table_type == ONE_PHASE) || (table_type == ONE_PHASE_WITH_WT)) {
    return dwt_table.EvalArgs(su_act_p, ru_act_p, wt);
  }
  else if ((table_type == TWO_PHASE) || (table_type == TWO_PHASE_WITH_WT)) {
    return dwt_table.EvalArgs(su_act_m, ru_act_m, su_act_p, ru_act_p, wt);
  }
  else {
    taMisc::Error("*** LeabraTableConSpec::LookupDWt unrecognized mesh type", String(table_type));
    return 0.0f;
  }
}

void LeabraTabledConSpec::ListTable(ostream& strm) {
  dwt_table.ListTable(strm);
}

void LeabraTabledConSpec::ShiftNorm(float desired_mean) {
  dwt_table.ShiftNorm(desired_mean);
}

void LeabraTabledConSpec::MulNorm(float desired_mean) {
  dwt_table.MulNorm(desired_mean);
}

void LeabraTabledConSpec::Graph2DTable(GraphLog* graph_log) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  else {
    graph_log->Clear();
    LogView* lv = (LogView*)graph_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }
  graph_log->name = "Graph dWt Lookup Table: " + name;
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("post");
  dt->NewColFloat("dwt");
  dt->AddColDispOpt("MIN=-1", 1);
  dt->AddColDispOpt("MAX=1", 1);

  float pre, post;
  for(pre = 0.0f; pre < 1.0f; pre += .01f) {
    for(post=0.0f; post < 1.0f; post += .01f) {
      float dwt = LookupDWt(.5f, .5f, pre, post, .5f);
      dt->AddBlankRow();
      dt->SetLastFloatVal(post, 0);
      dt->SetLastFloatVal(dwt, 1);
    }
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
  GraphLogView* glv = (GraphLogView*)graph_log->views[0];
  glv->TraceIncrement(-2, 2);
  glv->SetLineType(DA_GraphViewSpec::VALUE_COLORS);
  graph_log->ViewWindow();
}

//////////////////////////////////
// 	Scalar Value Layer	//
//////////////////////////////////

void ScalarValSpec::Initialize() {
  min = -0.25f;
  max = 1.25f;
  range = 1.5f;
  scale = 1.0f / range;
  un_width = .4f;
  clamp_full_pat = false;
}

void ScalarValLayerSpec::UpdateAfterEdit() {
  LeabraLayerSpec::UpdateAfterEdit();
  scalar.UpdateAfterEdit();
  val_range.min = scalar.min + (.5f * scalar.un_width);
  val_range.max = scalar.max - (.5f * scalar.un_width);
  val_range.UpdateAfterEdit();
}

void ScalarValLayerSpec::Initialize() {
  val_range.min = scalar.min + (.5f * scalar.un_width);
  val_range.max = scalar.max - (.5f * scalar.un_width);
  val_range.UpdateAfterEdit();
}

void ScalarValLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(scalar, this);
  taBase::Own(val_range, this);
}

void ScalarValLayerSpec::HelpConfig() {
  String help = "ScalarValLayerSpec Computation:\n\
Uses distributed coarse-coding units to represent a single scalar value.  Each unit \
has a preferred value arranged evenly between the min-max range, and decoding \
simply computes an activation-weighted average based on these preferred values.  The \
current scalar value is displayed in the first unit in the layer, which can be clamped \
and compared, etc (i.e., set the environment patterns to have just one unit and provide \
the actual scalar value and it will automatically establish the appropriate distributed \
representation in the rest of the units).  This first unit is only viewable as act_eq, \
not act, because it must not send activation to other units.\n\
\nScalarValLayerSpec Configuration:\n\
- A self connection using the ScalarValSelfPrjnSpec can be made, which provides a bias \
for neighboring units to have similar values.  It should usually have a fairly small wt_scale.rel \
parameter (e.g., .1)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool ScalarValLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  if(lay->n_units < 3) {
    if(!quiet) taMisc::Error("ScalarValLayerSpec: coarse-coded scalar representation requires at least 3 units, I just set n_units to 8");
    lay->n_units = 8;
  }

  // check for conspecs with correct params
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if((recv_gp->prjn == NULL) || (recv_gp->prjn->spec.spec == NULL)) continue;
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(recv_gp->prjn->spec.spec->InheritsFrom(TA_ScalarValSelfPrjnSpec)) {
      if(cs->wt_scale.rel > 0.5f) {
	cs->SetUnique("wt_scale", true);
	cs->wt_scale.rel = 0.1f;
	if(!quiet) taMisc::Error("ScalarValLayerSpec: scalar val self connections should have wt_scale < .5, I just set it to .1 for you in spec:",
		      cs->name,"(make sure this is appropriate for all connections that use this spec!)");
      }
      if(cs->lrate > 0.0f) {
	cs->SetUnique("lrate", true);
	cs->lrate = 0.0f;
	if(!quiet) taMisc::Error("ScalarValLayerSpec: scalar val self connections should have lrate = 0, I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
  }
  return true;
}

void ScalarValLayerSpec::Compute_ActAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts.avg = 0.0f;
  thr->acts.max = -FLT_MAX;
  thr->acts.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    if(lf == 0) { lf++; continue; }
    thr->acts.avg += u->act_eq;
    if(u->act_eq > thr->acts.max) {
      thr->acts.max = u->act_eq;  thr->acts.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 1) thr->acts.avg /= (float)(ug->leaves - 1);
}

void ScalarValLayerSpec::Compute_ActMAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts_m.avg = 0.0f;
  thr->acts_m.max = -FLT_MAX;
  thr->acts_m.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    if(lf == 0) { lf++; continue; }
    thr->acts_m.avg += u->act_m;
    if(u->act_m > thr->acts_m.max) {
      thr->acts_m.max = u->act_m;  thr->acts_m.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 1) thr->acts_m.avg /= (float)(ug->leaves - 1);
}

void ScalarValLayerSpec::Compute_ActPAvg_ugp(LeabraLayer*, Unit_Group* ug, LeabraInhib* thr, LeabraTrial*) {
  thr->acts_p.avg = 0.0f;
  thr->acts_p.max = -FLT_MAX;
  thr->acts_p.max_i = -1;
  LeabraUnit* u;
  taLeafItr i;
  int lf = 0;
  FOR_ITR_EL(LeabraUnit, u, ug->, i) {
    if(lf == 0) { lf++; continue; }
    thr->acts_p.avg += u->act_p;
    if(u->act_p > thr->acts_p.max) {
      thr->acts_p.max = u->act_p;  thr->acts_p.max_i = lf;
    }
    lf++;
  }
  if(ug->leaves > 1) thr->acts_p.avg /= (float)(ug->leaves - 1);
}

void ScalarValLayerSpec::ClampValue(Unit_Group* ugp, LeabraTrial*, float rescale) {
  if(ugp->size < 3) return;	// must be at least a few units..
  LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
  LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
  u->SetExtFlag(Unit::EXT);
  float val = val_range.Clip(u->ext);		// first unit has the value to clamp
  float cur = scalar.min;
  float incr = scalar.range / (float)(ugp->size - 2); // skip 1st unit
  int i;
  for(i=1;i<ugp->size;i++, cur += incr) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    float dist = (val - cur) / scalar.un_width;
    float act = rescale * expf(-(dist * dist));
    if(act < us->opt_thresh.send)
      act = 0.0f;
    u->SetExtFlag(Unit::EXT);
    u->ext = act;
  }
}

float ScalarValLayerSpec::ClampAvgAct(int ugp_size) {
  if(ugp_size < 3) return 0.0f;
  float val = val_range.min + .5f * val_range.Range(); // half way
  float cur = scalar.min;
  float incr = scalar.range / (float)(ugp_size - 2); // skip 1st unit
  float sum = 0.0f;
  int i;
  for(i=1;i<ugp_size;i++, cur += incr) {
    float dist = (val - cur) / scalar.un_width;
    float act = expf(-(dist * dist));
    sum += act;
  }
  sum /= (float)(ugp_size - 1);
  return sum;
}

float ScalarValLayerSpec::ReadValue(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return 0.0f;	// must be at least a few units..
  float cur = scalar.min;
  float incr = scalar.range / (float)(ugp->size - 2); // skip 1st unit
  float avg = 0.0f;
  float sum_act = 0.0f;
  int i;
  for(i=1;i<ugp->size;i++, cur += incr) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    avg += cur * u->act_eq;
    sum_act += u->act_eq;
  }
  if(sum_act > 0.0f)
    avg /= sum_act;
  // set the first unit in the group to represent the value
  LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
  u->act_eq = avg;
  u->act = 0.0f;		// very important to clamp act to 0: don't send!
  u->da = 0.0f;			// don't contribute to change in act
  return avg;
}

void ScalarValLayerSpec::LabelUnits_impl(Unit_Group* ugp) {
  if(ugp->size < 3) return;	// must be at least a few units..
  float cur = scalar.min;
  float incr = scalar.range / (float)(ugp->size - 2); // skip 1st unit
  int i;
  for(i=1;i<ugp->size;i++, cur += incr) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->name = (String)cur;
  }
}

void ScalarValLayerSpec::LabelUnits(LeabraLayer* lay) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      LabelUnits_impl(rugp);
    }
  }
  else {
    LabelUnits_impl(&(lay->units));
  }
}

void ScalarValLayerSpec::LabelUnitsNet(Network* net) {
  LeabraLayer* l;
  taLeafItr li;
  FOR_ITR_EL(LeabraLayer, l, net->layers., li) {
    if(l->spec.spec == this)
      LabelUnits(l);
  }
}

void ScalarValLayerSpec::ResetAfterClamp(LeabraLayer* lay, LeabraTrial*) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      if(rugp->size > 2) {
	LeabraUnit* u = (LeabraUnit*)rugp->FastEl(0);
	u->act = 0.0f;		// must reset so it doesn't contribute!
	u->act_eq = u->ext;	// avoid clamp_range!
      }	
    }
  }
  else {
    if(lay->units.size > 2) {
      LeabraUnit* u = (LeabraUnit*)lay->units.FastEl(0);
      u->act = 0.0f;		// must reset so it doesn't contribute!
      u->act_eq = u->ext;	// avoid clamp_range!
    }
  }
}

void ScalarValLayerSpec::HardClampExt(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
  ResetAfterClamp(lay, trl);
}

void ScalarValLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(scalar.clamp_full_pat) {
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
    return;
  }
  if(!(clamp.hard && (lay->ext_flag & Unit::EXT))) {
    lay->hard_clamped = false;
    return;
  }

  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      if(rugp->size < 3) continue;
      ClampValue(rugp, trl);
    }
  }
  else {
    if(lay->units.size > 2) {
      ClampValue(&(lay->units), trl);
    }
  }
  HardClampExt(lay, trl);
}

void ScalarValLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
  LeabraLayerSpec::Compute_Act_impl(lay, ug, thr, trl);
  ReadValue(ug, trl);		// always read out the value
}


//////////////////////////////////
// 	Scalar Value Self Prjn	//
//////////////////////////////////

void ScalarValSelfPrjnSpec::Initialize() {
  init_wts = true;
  width = 3;
  wt_width = 2.0f;
  wt_max = 1.0f;
}

void ScalarValSelfPrjnSpec::Connect_UnitGroup(Unit_Group* gp, Projection* prjn) {
  float neigh1 = 1.0f / wt_width;
  float val1 = expf(-(neigh1 * neigh1));
  float scale_val = wt_max / val1;

  int i;
  for(i=0;i<gp->size;i++) {
    Unit* ru = (Unit*)gp->FastEl(i);
    int j;
    for(j=-width;j<=width;j++) {
      int sidx = i+j;
      if((sidx < 0) || (sidx >= gp->size)) continue;
      Unit* su = (Unit*)gp->FastEl(sidx);
      if(!self_con && (ru == su)) continue;
      Connection* cn = ru->ConnectFromCk(su, prjn);
      if(cn != NULL) {
	float dist = (float)j / wt_width;
	float wtval = scale_val * expf(-(dist * dist));
	cn->wt = wtval;
      }
    }
  }
}

void ScalarValSelfPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(prjn->from != prjn->layer) {
    taMisc::Error("Error: ScalarValSelfPrjnSpec must be used as a self-projection!");
    return;
  }

  Layer* lay = prjn->layer;

  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Connect_UnitGroup(rugp, prjn);
    }
  }
  else {
    Connect_UnitGroup(&(lay->units), prjn);
  }
}

void ScalarValSelfPrjnSpec::C_InitWtState(Projection*, Con_Group* cg, Unit* ru) {
  float neigh1 = 1.0f / wt_width;
  float val1 = expf(-(neigh1 * neigh1));
  float scale_val = wt_max / val1;

  int ru_idx = ((Unit_Group*)ru->owner)->Find(ru);

  int i;
  for(i=0; i<cg->size; i++) {
    Unit* su = cg->Un(i);
    int su_idx = ((Unit_Group*)su->owner)->Find(su);
    float dist = (float)(ru_idx - su_idx) / wt_width;
    float wtval = scale_val * expf(-(dist * dist));
    cg->Cn(i)->wt = wtval;
  }
}


////////////////////////////////////////////////

void MarkerConSpec::Initialize() {
  SetUnique("rnd", true);
  rnd.mean = 0.0f; rnd.var = 0.0f;
  SetUnique("wt_limits", true);
  wt_limits.sym = false;
  SetUnique("wt_scale", true);
  wt_scale.rel = 0.0f;
  SetUnique("lrate", true);
  lrate = 0.0f;
  cur_lrate = 0.0f;
}

//////////////////////////////////////////
// 	TdMod Units and Cons		//
//////////////////////////////////////////

void TdModUnit::Initialize() {
  td = 0.0f;
  act_pp = 0.0f;
  p_act_m = -.1f;
  p_act_p = -.1f;
}

void TdModUnit::Copy_(const TdModUnit& cp) {
  td = cp.td;
  act_pp = cp.act_pp;
  p_act_p = cp.p_act_p;
  p_act_m = cp.p_act_m;
}

void TdModUnitSpec::Initialize() {
  min_obj_type = &TA_TdModUnit;
}

void TdModUnitSpec::Defaults() {
  LeabraUnitSpec::Defaults();
  Initialize();
}

void TdModUnitSpec::InitState(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  TdModUnit* lu = (TdModUnit*)u;
  lu->td = 0.0f;
  lu->act_pp = 0.0f;
  lu->p_act_m = -.1f;
  lu->p_act_p = -.1f;
}

void TdModUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(act.avg_dt > 0.0f)
    u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
  TdModUnit* lu = (TdModUnit*)u;
  if((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) {
    if((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn))
      return;
  }
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay, trl);
}

void TdModUnitSpec::UpdateWeights(Unit* u) {
  TdModUnit* lu = (TdModUnit*)u;
  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
  }
  if(opt_thresh.updt_wts && 
     ((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) &&
      ((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn)))
    return;
  UnitSpec::UpdateWeights(lu);
}

void TdModUnitSpec::EncodeState(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  TdModUnit* lu = (TdModUnit*)u;
  // just save phase activation states
  lu->p_act_p = lu->act_p;
  lu->p_act_m = lu->act_m;

  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
    recv_gp->EncodeState(u);
  }
}

void TdModUnitSpec::DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay) {
  LeabraUnitSpec::DecayEvent(u, lay, trl, decay);
  TdModUnit* lu = (TdModUnit*)u;
  lu->td = 0.0f;
}

void TdModUnitSpec::PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			       LeabraTrial* trl, bool set_both)
{
  LeabraUnitSpec::PostSettle(u, lay, thr, trl, set_both);
  TdModUnit* lu = (TdModUnit*)u;
  if(trl->phase_no == 2)
    lu->act_pp = lu->act_eq;
}

void TdLearnSpec::Initialize() {
  gain = 1.0f;
  lmix = .2f;
  lmix_c = .8f;
  min_lrate = .2f;
}

void TdLearnSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  lmix_c = 1.0f - lmix;
}

void TdModConSpec::Initialize() {
}

void TdModConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(td, this);
}

void TdModConSpec::UpdateAfterEdit() {
  LeabraConSpec::UpdateAfterEdit();
  td.UpdateAfterEdit();
}

//////////////////////////////////
//	ImRewPred Layer Spec	//
//////////////////////////////////

void ImRewPredSpec::Initialize() {
  graded = false;
  no_off_err = false;
}

void ImRewPredLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.clamp_phase2 = true;
  SetUnique("kwta", true);
  kwta.k_from = KWTASpec::USE_K;
  kwta.k = 3;
  SetUnique("inhib_group", true);
  inhib_group = ENTIRE_LAYER;
  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25f;

  scalar.min = -.5f;
  scalar.max = 1.5f;
  scalar.un_width = .4f;
  scalar.UpdateAfterEdit();

  pred_range.min = .1f;
  pred_range.max = 1.0f;
  pred_range.UpdateAfterEdit();

}

void ImRewPredLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  Initialize();
  rew.Initialize();
}

void ImRewPredLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(pred_range, this);
  taBase::Own(rew, this);
}

void ImRewPredLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  pred_range.UpdateAfterEdit();
}

void ImRewPredLayerSpec::HelpConfig() {
  String help = "ImRewPredLayerSpec Computation:\n\
Computes predictions of immediate reward information based on network performance\n\
- Minus phase = free-running expected reward computed (over settlng, fm recv wts)\n\
- Plus phase = external reward actually received\n\
- Learning is std phase-based (only for current time step t, unlike RewPredLayerSpec)\n\
\nImRewPredLayerSpec Configuration:\n\
- I must have at least one Recv connection marked with MarkerConSpec from output layer\n\
    (where network performance is measured to compute error)\n\
- Normal learning connections from other layers to compute expected rewards\n\
    (rnd.mean = .5, var = 0.05, hebb=0) -- low variance to minimize random predictions\n\
- Sending connection to a TdLayerSpec (marked with MarkerConSpec)\n\
    (to compute the td change in expected rewards as computed by this layer)\n\
- This layer must be before TdLayerSpec layer in list of layers\n\
\n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec \
which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool ImRewPredLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("ImRewPredLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_TdModUnitSpec)) {
    taMisc::Error("ImRewPredLayerSpec: UnitSpec must be TdModUnitSpec!");
    return false;
  }
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  bool got_marker = false;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      got_marker = true;
      continue;
    }
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    if(cs->lmix.hebb != 0.0f) {
      cs->SetUnique("lmix", true);
      cs->lmix.hebb = 0.0f;   cs->UpdateAfterEdit();
      if(!quiet) taMisc::Error("ImRewLayerSpec: requires recv connections to have lmix.hebb=0, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
    if((cs->wt_sig.gain != 1.0f) || (cs->wt_sig.off != 1.0f)) {
      cs->SetUnique("wt_sig", true);
      cs->wt_sig.gain = 1.0f;  cs->wt_sig.off = 1.0f;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: scalar val connections should have wt_sig.gain=1, off=1, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
    if((cs->rnd.mean == 0.5f) && (cs->rnd.var == 0.25f)) {
      cs->SetUnique("rnd", true);
      cs->rnd.mean = 0.5f; cs->rnd.var = 0.05f;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: requires recv connections to have weights initialized to 0.5 mean, 0.05 var, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all connections that use this spec!)");
    }
    if(cs->wt_limits.sym != false) {
      cs->SetUnique("wt_limits", true);
      cs->wt_limits.sym = false;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: requires recv connections to have wt_limits.sym=false, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
  }
  if(!got_marker) {
    taMisc::Error("ImRewPredLayerSpec: requires at least one recv MarkerConSpec connection from output/response layer(s) to compute",
		  "reward based on performance.  This was not found -- please fix!");
    return false;
  }
  return true;
}

bool ImRewPredLayerSpec::RewardAvail(LeabraLayer* lay, LeabraTrial*) {
  bool got_some = false;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* rew_lay = (LeabraLayer*)recv_gp->prjn->from;
      if(rew_lay->ext_flag & Unit::TARG) {
	got_some = true;
	break;
      }
    }
  }
  return got_some;
}

float ImRewPredLayerSpec::GetReward(LeabraLayer* lay, LeabraTrial*) {
  float totposs = 0.0f;		// total possible error
  float toterr = 0.0f;		// total error
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec))
      continue;
    LeabraLayer* rew_lay = (LeabraLayer*)recv_gp->prjn->from;
    if(rew.no_off_err) {
      totposs += rew_lay->kwta.k; // only on units can make errors
    }
    else {
      totposs += 2 * rew_lay->kwta.k; // both on and off units count
    }
    LeabraUnit* eu;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, eu, rew_lay->units., i) {
      if(rew.no_off_err) {
	if(!(eu->ext_flag & (Unit::TARG) | Unit::COMP)) continue;
	if(eu->act_m > 0.5f) {	// was active
	  if(eu->targ < 0.5f)	// shouldn't have been
	    toterr += 1.0f;
	}
      }
      else {
	if(!(eu->ext_flag & Unit::TARG)) continue;
	float tmp = fabsf(eu->act_m - eu->targ);
	float err = 0.0f;
	if(tmp >= 0.5f) err = 1.0f;
	toterr += err;
      }
    }
  }
  if(totposs == 0.0f)
    return -1.0f;		// -1 = no reward signal at all
  if(rew.graded) {
    float nrmerr = toterr / totposs;
    if(nrmerr > 1.0f) nrmerr = 1.0f;
    return 1.0f - nrmerr;
  }
  if(toterr > 0.0f) return 0.0f; // 0 = wrong, 1 = correct
  return 1.0f;
}

void ImRewPredLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no == 0) {	// predict in minus phase
    lay->hard_clamped = false;
    lay->InitExterns();
    return;
  }
  else if(trl->phase_no > 1) {
    // clamp to prior act_p value: will happen automatically
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
    return;
  }

  bool rew_avail = RewardAvail(lay, trl);
  if(!rew_avail) {
    // if no reward available, just let it settle on its own
    lay->hard_clamped = false;
    lay->InitExterns();
    return;
  }

  float er = GetReward(lay, trl);
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);
  u->ext = er;
  u->SetExtFlag(Unit::EXT); u->SetExtFlag(Unit::TARG);
  lay->SetExtFlag(Unit::EXT); lay->SetExtFlag(Unit::TARG);
  ScalarValLayerSpec::Compute_HardClamp(lay, trl);
}

void ImRewPredLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  ScalarValLayerSpec::PostSettle(lay, trl, set_both);
  if(trl->phase_no == 0) { // reset act_dif to 0 for minus phase
    LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);
    u->act_m = pred_range.Clip(u->act_m); // ensure min/max predictions
    u->act_dif = 0.0f;
    Compute_ActMAvg(lay, trl);
  }
}

void ImRewPredLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no != 1))
    return; // only do first dwt!
  ScalarValLayerSpec::Compute_dWt(lay, trl);
}

//////////////////////////////////
//	Td Layer Spec		//
//////////////////////////////////

void ErrRateSpec::Initialize() {
  td_err_thr = .2;
  rate_dt = .05;
}

void TdLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.clamp_phase2 = true;
  SetUnique("kwta", true);
  kwta.k_from = KWTASpec::USE_K;
  kwta.k = 1;
  SetUnique("inhib_group", true);
  inhib_group = ENTIRE_LAYER;
  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25;
}

void TdLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(err_rate, this);
}

void TdLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  Initialize();
}

void TdLayerSpec::HelpConfig() {
  String help = "TdLayerSpec Computation:\n\
- act of unit(s) = act_dif of unit(s) in reward prediction layer we recv from\n\
- td is sent at end of first plus phase to all layers that recv from us\n\
- No Learning\n\
\nTdLayerSpec Configuration:\n\
- Single recv connection marked with a MarkerConSpec from reward prediction layer \
    (computes expectations and actual reward signals)\n\
- This layer must be after corresp. reward prediction layer in list of layers\n\
- Sending connections must connect to units of type TdModUnit/Spec \
     (td signal from this layer put directly into td var on units at end of plus)\n\
- UnitSpec for this layer must have act_range and clamp_range set to -1 and 1 \
     (because negative td = negative activation signal here";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool TdLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  if(!decay.clamp_phase2) {
    SetUnique("decay", true);
    decay.clamp_phase2 = true;
  }

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("TdLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  // must have the appropriate ranges for unit specs..
  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if((us->act_range.max != 2.0f) || (us->act_range.min != -2.0f)) {
    us->SetUnique("act_range", true);
    us->act_range.max = 2.0f;
    us->act_range.min = -2.0f;
    us->act_range.UpdateAfterEdit();
    if(!quiet) taMisc::Error("TdLayerSpec: requires UnitSpec act_range.max = 2, min = -2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if((us->clamp_range.max != 2.0f) || (us->clamp_range.min != -2.0f)) {
    us->SetUnique("clamp_range", true);
    us->clamp_range.max = 2.0f;
    us->clamp_range.min = -2.0f;
    us->clamp_range.UpdateAfterEdit();
    if(!quiet) taMisc::Error("TdLayerSpec: requires UnitSpec clamp_range.max = 2, min = -2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }

  // check recv connection
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[0];
  if((cg == NULL) || (cg->size <= 0)) {
    taMisc::Error("*** TdLayerSpec: requires one recv projection with at least one unit!");
    return false;
  }
  if(!cg->Un(0)->InheritsFrom(TA_TdModUnit)) {
    taMisc::Error("*** TdLayerSpec: I need to receive from a TdModUnit!");
    return false;
  }

  int myidx = lay->own_net->layers.FindLeaf(lay);
  int rpidx = lay->own_net->layers.FindLeaf(cg->prjn->from);
  if(rpidx > myidx) {
    taMisc::Error("TdLayerSpec: reward prediction layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  // check sending layer projections for appropriate unit types
  int si;
  for(si=0;si<lay->send_prjns.size;si++) {
    Projection* prjn = (Projection*)lay->send_prjns.FastEl(si);
    if(!prjn->from->units.el_typ->InheritsFrom(TA_TdModUnit)) {
      taMisc::Error("TdLayerSpec: all layers I send to must have TdModUnits!, layer:",
		    prjn->from->GetPath(),"doesn't");
      return false;
    }
  }
  return true;
}

void TdLayerSpec::ExtToComp(LeabraLayer*, LeabraTrial*) {
  // do nothing!
}

void TdLayerSpec::InitWtState(LeabraLayer* lay) {
  LeabraLayerSpec::InitWtState(lay);
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->misc_1 = 0.0f;
  }      
}

void TdLayerSpec::Compute_ZeroAct(LeabraLayer* lay, LeabraTrial*) {
  lay->td = 0.0f;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->ext = 0.0f;
    u->SetExtFlag(Unit::EXT);
  }      
}

void TdLayerSpec::Compute_Td(LeabraLayer* lay, LeabraTrial*) {
  lay->td = 0.0f;
  TdModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(TdModUnit, u, lay->units., i) {
    LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[0];
    // just taking the first unit = scalar val
    TdModUnit* su = (TdModUnit*)cg->Un(0);
    u->ext = u->td = su->act_dif;
    lay->td += su->act_dif;
    u->SetExtFlag(Unit::EXT);
    float p_act_dif = su->p_act_p - su->p_act_m; // use prior td value: one step behind!
    if(p_act_dif < -err_rate.td_err_thr)
      u->misc_1 += err_rate.rate_dt * (1.0 - u->misc_1);
    else
      u->misc_1 -= err_rate.rate_dt * u->misc_1;
  }
  if(lay->units.leaves > 0) lay->td /= (float)lay->units.leaves;
}

void TdLayerSpec::Send_Td(LeabraLayer* lay, LeabraTrial*) {
  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    // this is ok here because Compute_dWt is a NOP
    if(us->act.avg_dt > 0.0f)
      u->act_avg += us->act.avg_dt * (u->ext - u->act_avg); // compute on ext: raw td values

    // then, send to all sending projections
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion)	continue;
      int j;
      for(j=0;j<send_gp->size; j++) {
	((TdModUnit*)send_gp->Un(j))->td = u->act;
      }
    }
  }
}

void TdLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  lay->hard_clamped = true;	// cache this flag
  lay->SetExtFlag(Unit::EXT);
  lay->Inhib_SetVals(i_kwta_pt); // assume 0 - 1 clamped inputs

  Compute_ZeroAct(lay, trl);	// can't do anything during settle anyway -- just zero it
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
}

void TdLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if(trl->phase_no == 1) {
    Compute_Td(lay, trl);	// now get the td and clamp it to layer
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
    Send_Td(lay, trl);
  }
  LeabraLayerSpec::PostSettle(lay, trl, set_both);
}

//////////////////////////////////
//	NAc Layer Spec		//
//////////////////////////////////

void RewPredConSpec::Initialize() {
  SetUnique("lmix", true);
  lmix.hebb = 0.0f;
  lmix.err = 1.0f;

  SetUnique("rnd", true);
  rnd.mean = 0.5f;
  rnd.var = 0.05f;

  SetUnique("wt_limits", true);
  wt_limits.sym = false;
}

void RewPredSpec::Initialize() {
  discount = .8f;
  gain = .5f;
  graded = false;
  no_off_err = false;
  sub_avg = false;
  avg_dt = .005f;
}

void RewPredLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase2 = 1.0f;
  decay.clamp_phase2 = true;
  SetUnique("kwta", true);
  kwta.k_from = KWTASpec::USE_K;
  kwta.k = 3;
  SetUnique("inhib_group", true);
  inhib_group = ENTIRE_LAYER;
  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25;

  scalar.min = -1.25f;
  scalar.max = 1.25f;
  scalar.un_width = .4f;
  scalar.UpdateAfterEdit();
  val_range.UpdateAfterEdit();

  pred_range.min = -1.0f;
  pred_range.max = 1.0f;
  pred_range.UpdateAfterEdit();

  rew_type = OUT_ERR_REW;
}

void RewPredLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  rew.Initialize();
  Initialize();
}

void RewPredLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(pred_range, this);
  taBase::Own(rew, this);
}

void RewPredLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  pred_range.UpdateAfterEdit();
  rew.UpdateAfterEdit();
}

void RewPredLayerSpec::HelpConfig() {
  String help = "RewPredLayerSpec Computation:\n\
Computes expected rewards according to the TD algorithm: predicts V(t+1) at time t. \n\
- Minus phase = previous expected reward computed in plus phase of t-1 (but act_p(t-1) is \
  discounted and has external reward added)\n\
- Plus phase = free-running expected reward computed (over settlng, fm recv wts) \
  and at end of plus, external reward is added and the prediction is discounted.\n\
- Learning is std phase-based but learns based on senders at t-1, to improve prediction.\n\
\nRewPredLayerSpec Configuration:\n\
- All units I recv from must be TdModUnit/Spec units (to hold t-1 act vals)\n\
- For TD_VAL: A recv connection from the TDLayer associated with the ImRewPredLayer which discounts \
immediate reward(s) (marked with MarkerConSpec)\n\
- For OUT_ERR_REW: A recv connection from the output layers where error is computed (marked with MarkerConSpec)\n\
    NOTE: Also requires a MarkerConSpec from a layer called RewTarg that signals (>.5 act) when output errors count\n\
- Normal learning connections from other layers to compute expected rewards\n\
    (rnd.mean = .5, var = 0.05, hebb=0) -- low variance to minimize random predictions\n\
- Sending connection to a VtaLayerSpec (marked with MarkerConSpec)\n\
    (to compute the td change in expected rewards as computed by this layer)\n\
- This layer must be before  VtaLayerSpec layer in list of layers\n\
\n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec \
which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool RewPredLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("RewPredLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_TdModUnit)) {
    taMisc::Error("RewPredLayerSpec: must have TdModUnits!");
    return false;
  }

  SetUnique("decay", true);
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = true;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_TdModUnitSpec)) {
    taMisc::Error("RewPredLayerSpec: UnitSpec must be TdModUnitSpec!");
    return false;
  }
  if(rew.sub_avg && (us->act.avg_dt > 0.0f)) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.0f;
    if(!quiet) taMisc::Error("RewPredLayerSpec: requires UnitSpec act.avg_dt = 0 for sub_avg, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if((us->opt_thresh.learn >= 0.0f) || us->opt_thresh.updt_wts) {
    if(!quiet) taMisc::Error("RewPredLayerSpec: UnitSpec opt_thresh.learn must be -1 to allow proper recording of p_savg and p_max_cor values in connections",
		  "even when this unit is not active on a prior time step!  I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
    us->SetUnique("opt_thresh", true);
    us->opt_thresh.learn = -1.0f;
    us->opt_thresh.updt_wts = false;
  }
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  bool got_marker = false;
  LeabraLayer* rew_targ_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      if(recv_gp->prjn->from->name == "RewTarg")
	rew_targ_lay = (LeabraLayer*)recv_gp->prjn->from;
      else
	got_marker = true;
      continue;
    }
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(!cs->InheritsFrom(TA_RewPredConSpec)) {
      taMisc::Error("RewPredLayerSpec: requires recv connections to be of type RewPredConSpec to learn from one time step back!");
      return false;
    }
    if((cs->wt_sig.gain != 1.0f) || (cs->wt_sig.off != 1.0f)) {
      cs->SetUnique("wt_sig", true);
      cs->wt_sig.gain = 1.0f;  cs->wt_sig.off = 1.0f;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: scalar val connections should have wt_sig.gain=1, off=1, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
    if((cs->rnd.mean == 0.5f) && (cs->rnd.var == 0.25f)) {
      cs->SetUnique("rnd", true);
      cs->rnd.mean = 0.5f; cs->rnd.var = 0.05f;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: requires recv connections to have weights initialized to 0.5 mean, 0.05 var, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all connections that use this spec!)");
    }
    if(cs->wt_limits.sym != false) {
      cs->SetUnique("wt_limits", true);
      cs->wt_limits.sym = false;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: requires recv connections to have wt_limits.sym=false, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
    if(cs->lmix.hebb != 0.0f) {
      cs->SetUnique("lmix", true);
      cs->lmix.hebb = 0.0f;   cs->UpdateAfterEdit();
      if(!quiet) taMisc::Error("RewPredLayerSpec: requires recv connections to have lmix.hebb=0, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
  }
  if(!got_marker) {
    if(rew_type == TD_VAL) {
      taMisc::Error("RewPredLayerSpec: requires at least one recv MarkerConSpec connection from TD layer",
		    "to get reward based on performance.  This was not found -- please fix!");
      return false;
    }
    else if(rew_type == OUT_ERR_REW) {
      taMisc::Error("RewPredLayerSpec: requires at least one recv MarkerConSpec connection from output/response layer(s) to compute",
		    "reward based on performance.  This was not found -- please fix!");
      return false;
    }
  }
  if((rew_type == OUT_ERR_REW) && (rew_targ_lay == NULL)) {
    taMisc::Error("RewPredLayerSpec: requires a recv MarkerConSpec connection from layer called RewTarg",
		  "that signals (act > .5) when output error should be used for computing rewards.  This was not found -- please fix!");
    return false;
  }
  if(rew_type == OUT_ERR_REW) {
    if(rew_targ_lay->units.size == 0) {
      taMisc::Error("RewPredLayerSpec: RewTarg layer must have one unit (has zero) -- please fix!");
      return false;
    }
  }
  return true;
}

void RewPredLayerSpec::ExtToComp(LeabraLayer*, LeabraTrial*) {
  // do nothing!
}

void RewPredLayerSpec::InitState(LeabraLayer* lay) {
  ScalarValLayerSpec::InitState(lay);
  // initialize the misc_1 variable to 0.0 -- no prior predictions!
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      LeabraUnit* u = (LeabraUnit*)rugp->Leaf(0);
      u->misc_1 = 0.0f;
    }
  }
  else {
    LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);
    u->misc_1 = 0.0f;
  }
}  

bool RewPredLayerSpec::OutErrRewAvail(LeabraLayer* lay, LeabraTrial*) {
  bool got_some = false;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* rew_lay = (LeabraLayer*)recv_gp->prjn->from;
      if(rew_lay->name != "RewTarg") continue;
      LeabraUnit* rtu = (LeabraUnit*)rew_lay->units[0];
      if(rtu->act_eq > .5) {
	got_some = true;
	break;
      }
    }
  }
  return got_some;
}

float RewPredLayerSpec::GetOutErrRew(LeabraLayer* lay, LeabraTrial*) {
  float totposs = 0.0f;		// total possible error
  float toterr = 0.0f;		// total error
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec))
      continue;
    LeabraLayer* rew_lay = (LeabraLayer*)recv_gp->prjn->from;
    if(rew_lay->name == "RewTarg") continue;
    if(rew.no_off_err) {
      totposs += rew_lay->kwta.k; // only on units can make errors
    }
    else {
      totposs += 2 * rew_lay->kwta.k; // both on and off units count
    }
    LeabraUnit* eu;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, eu, rew_lay->units., i) {
      if(rew.no_off_err) {
	if(!(eu->ext_flag & (Unit::TARG) | Unit::COMP)) continue;
	if(eu->act_m > 0.5f) {	// was active
	  if(eu->targ < 0.5f)	// shouldn't have been
	    toterr += 1.0f;
	}
      }
      else {
	if(!(eu->ext_flag & Unit::TARG)) continue;
	float tmp = fabsf(eu->act_m - eu->targ);
	float err = 0.0f;
	if(tmp >= 0.5f) err = 1.0f;
	toterr += err;
      }
    }
  }
  if(totposs == 0.0f)
    return -1.0f;		// -1 = no reward signal at all
  if(rew.graded) {
    float nrmerr = toterr / totposs;
    if(nrmerr > 1.0f) nrmerr = 1.0f;
    return 1.0f - nrmerr;
  }
  if(toterr > 0.0f) return 0.0f; // 0 = wrong, 1 = correct
  return 1.0f;
}

void RewPredLayerSpec::Compute_SavePred(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->misc_1 = u->act_eq;
  }
}

void RewPredLayerSpec::Compute_ClampPred(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->ext = u->misc_1;
    u->SetExtFlag(Unit::EXT);
  }
}

void RewPredLayerSpec::Compute_ClampPrev(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_ClampPred(rugp, trl);
    }
  }
  else {
    Compute_ClampPred(&(lay->units), trl);
  }
}

void RewPredLayerSpec::Compute_ExtRew(LeabraLayer* lay, LeabraTrial*) {
  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      TdModUnit* u = (TdModUnit*)rugp->Leaf(0);
      if(u->ext_flag & Unit::TARG)
	u->td = u->ext;		// save the reward as a td value!
      else
	u->td = 0.0f;		// no reward = 0
    }
  }
  else {
    TdModUnit* u = (TdModUnit*)lay->units.Leaf(0);
    if(u->ext_flag & Unit::TARG)
      u->td = u->ext;		// save the reward as a td value!
    else
      u->td = 0.0f;		// no reward = 0
  }
}

void RewPredLayerSpec::Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
    if(i > 0) u->act_p = us->clamp_range.Clip(u->ext);
    else u->act_p = u->ext;
    u->act_dif = u->act_p - u->act_m;
  }
}

void RewPredLayerSpec::Compute_TdPlusPhase(Unit_Group* ugp, LeabraTrial* trl) {
  TdModUnit* u = (TdModUnit*)ugp->FastEl(0);

  u->act_eq = pred_range.Clip(u->act_eq); // ensure it is within range
  Compute_SavePred(ugp, trl);
  float cur_pred = u->act_eq;	// current prediction value

  // then, clamp plus phase to current prediction
  float rval = u->td;		// reward value
  if(rew.sub_avg) {
    rval -= u->act_avg;
    u->act_avg += rew.avg_dt * (u->td - u->act_avg); // update the average
  }
  u->ext = (rew.discount * cur_pred) + rew.gain * rval;
  ClampValue(ugp, trl);		// apply new value
  Compute_ExtToPlus(ugp, trl);	// copy ext values to act_p
}

void RewPredLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  // if we're clamping external reward, it will be lost after this point so we
  // need to save the value in the td units.
  if((rew_type == EXT_REW) && (trl->phase_no == 1)) {
    Compute_ExtRew(lay, trl);
  }

  if(trl->phase_no == 0) {
    lay->SetExtFlag(Unit::EXT);
    Compute_ClampPrev(lay, trl);
    HardClampExt(lay, trl);
  }
  else if(trl->phase_no == 1) {
    lay->hard_clamped = false;	// run free in plus phase
    lay->InitExterns();
  }
  else {
    // clamp to prior act_p value: will happen automatically
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
  }
}

void RewPredLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if(trl->phase_no != 1) {
    ScalarValLayerSpec::PostSettle(lay, trl, set_both);
    return; // minus phase does nothing
  }

  if((trl->phase_no == 1) && (rew_type == OUT_ERR_REW)) { // get the reward from performance
    float er = 0.0f;
    if(OutErrRewAvail(lay, trl))
      er = GetOutErrRew(lay, trl);
    TdModUnit* u;
    taLeafItr ui;
    FOR_ITR_EL(TdModUnit, u, lay->units., ui) {
      u->td = er;		// set the td to er
    }
  }

  if(lay->units.gp.size > 0) {
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp.FastEl(g);
      Compute_TdPlusPhase(rugp, trl);
    }
  }
  else {
    Compute_TdPlusPhase(&(lay->units), trl);
  }
}

void RewPredLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no != 1))
    return; // only do first dwt!
  ScalarValLayerSpec::Compute_dWt(lay, trl);
}

//////////////////////////////////
//	Patch Layer Spec	//
//////////////////////////////////

void PatchLearnSpec::Initialize() {
  lrn_mod = PROT_NEG;
  td_gain = 1.0f;
}

void PatchLayerSpec::Initialize() {
  SetUnique("gp_kwta", true);
  gp_kwta.k_from = KWTASpec::USE_K;
  gp_kwta.k = 3;
  SetUnique("inhib_group", true);
  inhib_group = UNIT_GROUPS;
}

void PatchLayerSpec::Defaults() {
  RewPredLayerSpec::Defaults();
  learn.Initialize();
  Initialize();
}

void PatchLayerSpec::InitLinks() {
  RewPredLayerSpec::InitLinks();
  taBase::Own(learn, this);
}

void PatchLayerSpec::HelpConfig() {
  String help = "PatchLayerSpec Computation:\n\
The patch units work just like RewPredLayerSpec units, except that they are \
gated by activation in the corresponding matrix stripe -- only if the matrix \
stripe units have fired a GO-gating signal will the patch units be allowed to activate.  \
Activation is gated on until the corresponding matrix fires an OFF signal.\n\
\nPatchLayerSpec Configuration:\n\
- See RewPredLayerSpec HelpConfig for basic config info (this will come up after you press OK here)\n\
- Must have same number of unit groups (with one unit per group) as matrix layer\n\
    (one-to-one correspondence between patch unit and matrix layer group (stripe))\n\
- Units must be TdModUnits w/ TdModUnitSpec and must recv from TdLayerSpec layer \
associated with the NAC RewPredLayerSpec (gets td signal at end of 1st plus phase into u->td)\n\
- Must recv marked connection (w/ MarkerConSpec) from MatrixLayerSpec layer\n\
    (learning of patch units driven by activation of matrix units)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  RewPredLayerSpec::HelpConfig();
}

bool PatchLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!RewPredLayerSpec::CheckConfig(lay, trl, quiet))
    return false;
  
  int matrix_prjn_idx;
  LeabraLayer* matrix_lay = FindMatrixLayer(lay, matrix_prjn_idx);
  if((matrix_prjn_idx < 0) || (matrix_lay == NULL))
    return false;

  if(!matrix_lay->spec.spec->InheritsFrom(TA_MatrixLayerSpec)) {
    taMisc::Error("PatchLayerSpec: matrix layer does not have MatrixLayerSpec layer spec!");
    return false;
  }

  if(matrix_lay->units.gp.size != lay->units.gp.size) {
    taMisc::Error("PatchLayerSpec: matrix layer does not have same number of groups (",
		  (String)matrix_lay->units.gp.size,") as I have groups (",
		  (String)lay->units.gp.size,")");
    return false;
  }

  // check for ordering of layers!
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int mtidx = lay->own_net->layers.FindLeaf(matrix_lay);
  if(mtidx < myidx) {
    taMisc::Error("PatchLayerSpec: MatrixLayer must be *after* this layer in list of layers -- it is now before, won't work");
    return false;
  }
  return true;
}

LeabraLayer* PatchLayerSpec::FindMatrixLayer(LeabraLayer* lay, int& matrix_prjn_idx) {
  matrix_prjn_idx = -1;
  LeabraLayer* matrix_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  for(g=0;g<u->recv.gp.size; g++) {
    recv_gp = (LeabraCon_Group*)u->recv.gp[g];
    LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("*** PatchLayerSpec: null from layer in recv projection:", (String)g);
      return NULL;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)
	&& fmlay->spec.spec->InheritsFrom(TA_MatrixLayerSpec)) {
      matrix_prjn_idx = g;
      matrix_lay = fmlay;
    }
  }
  if((matrix_prjn_idx < 0) || (matrix_lay == NULL)) {
    taMisc::Error("*** PatchLayerSpec: Warning: no projection from Matrix Layer found (must be marked with a MarkerConSpec)!");
  }
  return matrix_lay;
}


void PatchLayerSpec::InitWtState(LeabraLayer* lay) {
  RewPredLayerSpec::InitWtState(lay);
  int mg;
  for(mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* gp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
    gp->misc_state = 0;	// reset!
  }
}

void PatchLayerSpec::Compute_ZeroAct(Unit_Group* ugp, LeabraTrial* trl) {
  if(ugp->size < 3) return;
  LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
  u->ext = 0.0f;
  ClampValue(ugp, trl);		// set zero act distrib
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
    u->act = u->act_eq = us->clamp_range.Clip(u->ext);
    u->da = 0.0f;
  }
}

void PatchLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
  if((trl->phase_no < 2) && ((LeabraUnit_Group*)ug)->misc_state <= 0) {	// gated off, turn off!
    Compute_ZeroAct(ug, trl);
  }
  else {			// settle in second plus phase always
    RewPredLayerSpec::Compute_Act_impl(lay, ug, thr, trl);
  }
}

void PatchLayerSpec::Compute_SavePredFmActP(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->misc_1 = u->act_p;
  }
}

// this is called by snclayer
void PatchLayerSpec::Compute_TdPlusPhase(Unit_Group* ugp, LeabraTrial* trl, float snc) {
  TdModUnit* u = (TdModUnit*)ugp->FastEl(0);

  u->act_eq = pred_range.Clip(u->act_eq); // ensure it is within range
  float cur_pred = u->act_eq;	// current prediction value

  if(((LeabraUnit_Group*)ugp)->misc_state <= 0) {
    cur_pred = 0.0f;
    u->ext = 0.0f; 		// reset
    u->td = 0.0f;
    ClampValue(ugp, trl);	// clamp to the 0 value
    Compute_ExtToPlus(ugp, trl);	// copy ext values to act_p
    Compute_SavePredFmActP(ugp, trl);	// save this as the prediction
  }
  else {
    Compute_SavePred(ugp, trl);

    if(learn.lrn_mod == PatchLearnSpec::PROT_NEG) {
      if((snc > 0.0f) && (u->td < 0.0f)) {	// only if negative-going
	u->td *= (1.0f - snc);
      }
    }

    // then, clamp plus phase to current prediction
    float rval = u->td * learn.td_gain;		// reward value
    u->ext = (rew.discount * cur_pred) + rew.gain * rval;
    ClampValue(ugp, trl); // apply new value
    Compute_ExtToPlus(ugp, trl);	// copy ext values to act_p
  }
}

void PatchLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  if(trl->phase_no == 0) {
    ScalarValLayerSpec::PostSettle(lay, trl, set_both);
    return; // minus phase does nothing
  }

  if(trl->phase_no > 1) {
    ScalarValLayerSpec::PostSettle(lay, trl, set_both);
  }

  int matrix_prjn_idx;
  LeabraLayer* matrix_lay = FindMatrixLayer(lay, matrix_prjn_idx);
  if((matrix_prjn_idx < 0) || (matrix_lay == NULL))
    return;
  MatrixLayerSpec* mls = (MatrixLayerSpec*)matrix_lay->spec.spec;

  // first, get current information out of the matrix layer!
  mls->UpdateGates(matrix_lay, trl);
  
  // only get final gating info -- TdPlusPhase is called by the SNc in phase 1 and needs old state info!
  if(trl->phase_no > 1) {
    int mg = 0;
    for(mg=0;mg<lay->units.gp.size;mg++) {
      LeabraUnit_Group* ug = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      LeabraUnit_Group* mugp = (LeabraUnit_Group*)matrix_lay->units.gp.FastEl(mg);

      ug->misc_state = mugp->misc_state; // get our misc state from matrix
    }
  }
}

//////////////////////////////////
//	SNc Layer Spec		//
//////////////////////////////////

void SNcLayerSpec::Initialize() {
}

void SNcLayerSpec::Compute_Td(LeabraLayer* lay, LeabraTrial* trl) {
  if(lay->units.leaves <= 0) return;
  lay->td = 0.0f;
  TdModUnit* u;
  taLeafItr ui;
  FOR_ITR_EL(TdModUnit, u, lay->units., ui) {
    LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[0];
    // just taking the first unit = scalar val
    TdModUnit* su = (TdModUnit*)cg->Un(0);
    float pat_val = su->act_m;
    if(pat_val > 1.0f) pat_val = 1.0f;
    u->misc_1 = u->net = u->ext = u->td = pat_val;
    lay->td += u->ext;
    u->SetExtFlag(Unit::EXT);
  }
  lay->td /= (float)lay->units.leaves;

  u = (TdModUnit*)lay->units.Leaf(0);
  LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[0];
  LeabraLayer* pat_lay = (LeabraLayer*)cg->prjn->from;
  PatchLayerSpec* pat_ls = (PatchLayerSpec*)pat_lay->spec.spec;
  int j;
  for(j=0;j<lay->units.gp.size;j++) {
    LeabraUnit_Group* snc_gp = (LeabraUnit_Group*)lay->units.gp.FastEl(j);
    LeabraUnit_Group* pat_gp = (LeabraUnit_Group*)pat_lay->units.gp.FastEl(j);
    u = (TdModUnit*)snc_gp->FastEl(0);
    pat_ls->Compute_TdPlusPhase(pat_gp, trl, u->ext);
  }
}

void SNcLayerSpec::Send_Td(LeabraLayer* lay, LeabraTrial*) {
  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    // this is ok here because Compute_dWt is a NOP
    if(us->act.avg_dt > 0.0f)
      u->act_avg += us->act.avg_dt * (u->ext - u->act_avg); // compute on ext: raw td values

    // then, send to all sending projections
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion)	continue;
      int j;
      for(j=0;j<send_gp->size; j++) {
	((TdModUnit*)send_gp->Un(j))->misc_1 = u->act;	// send to misc_1: matrix decides what to do with it
      }
    }
  }
}

//////////////////////////////////
//	MatrixConSpec		//
//////////////////////////////////

void MatrixConSpec::Initialize() {
  SetUnique("lmix", true);
  lmix.hebb = .01f;
  lmix.err = .99f;
}

void MatrixConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  children.SetBaseType(&TA_LeabraConSpec); // make this the base type so bias specs
					   // can live under here..
  children.el_typ = &TA_MatrixConSpec; // but this is the default type
}

//////////////////////////////////////////
// 	Matrix Unit Spec		//
//////////////////////////////////////////

void NoiseBiasCon::Copy_(const NoiseBiasCon& cp) {
  zwt = cp.zwt;
}

void NoiseBiasSpec::Initialize() {
  min_con_type = &TA_NoiseBiasCon;
}

void MatrixUnitSpec::Initialize() {
  SetUnique("bias_spec", true);
  bias_spec.type = &TA_NoiseBiasSpec;
  SetUnique("bias_con_type", true);
  bias_con_type = &TA_NoiseBiasCon;
  SetUnique("g_bar", true);
  g_bar.a = .5f;
  g_bar.h = .5f;
  SetUnique("opt_thresh", true);
  opt_thresh.learn = -1.0f;
  opt_thresh.updt_wts = false;
}

void MatrixUnitSpec::Defaults() {
  TdModUnitSpec::Defaults();
  Initialize();
}

void MatrixUnitSpec::InitLinks() {
  TdModUnitSpec::InitLinks();
  bias_spec.type = &TA_NoiseBiasSpec;
  bias_con_type = &TA_NoiseBiasCon;
}

void MatrixUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(act.avg_dt > 0.0f)
    u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
  // always learn regardless of act_p and act_m
  Compute_dWt_impl(u, lay, trl);
}

void MatrixUnitSpec::Compute_dWt_impl(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  // do adapt bias weights on clamped inputs..
  ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
  UnitSpec::Compute_dWt(u);
}

void MatrixUnitSpec::UpdateWeights(Unit* u) {
  LeabraUnit* lu = (LeabraUnit*)u;
  ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
  UnitSpec::UpdateWeights(lu);
}


//////////////////////////////////
//	Matrix Layer Spec	//
//////////////////////////////////

void UpDnGradSpec::Initialize() {
  start = 0.0f;
  gps_per = 8;
  incr = 0.0f;
}

void MaintDurSpec::Initialize() {
  start = 100;
  gps_per = 8;
  incr = 0;
}

void MatrixGateSpec::Initialize() {
  down_g_a = 1.0f;
  no_patch = false;
  go_toggle = true;
  snc_gain = 1.0f;
  snc_err_thr = 0.1f;
}

void MatrixLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase2 = 1.0f;
  decay.clamp_phase2 = false;

  SetUnique("gp_kwta", true);
  gp_kwta.k_from = KWTASpec::USE_K;
  gp_kwta.k = 1;
  SetUnique("inhib_group", true);
  inhib_group = UNIT_GROUPS;
  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25;

  bias_noise.type = Random::UNIFORM;
  bias_noise.mean = 0.0f;
  bias_noise.var = 0.0f;
}

void MatrixLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  gate.Initialize();
  nop_up_g_h.Initialize();
  max_maint_dur.Initialize();
  Initialize();
}

void MatrixLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(bias_noise, this);
  taBase::Own(nop_up_g_h, this);
  taBase::Own(max_maint_dur, this);
  taBase::Own(gate, this);
}

void MatrixLayerSpec::HelpConfig() {
  String help = "MatrixLayerSpec Computation:\n\
There are 3 types of units arranged sequentially in the following order within each \
stripe whose firing affects the gating status of the corresponding stripe in PFC:\n\
- ON unit = update corresponding units in PFC (gate on, 'go'): this is the direct pathway\n\
- NOP unit = do nothing (no operation); this is the indirect pathway\n\
- OFF unit = turn off maintenance of corresponding units in PFC: could also be weaker \
direct path Learning is based on both td signal (from SNc) and any error signal that \
might be derived from other network projections\n\
\nMatrixLayerSpec Configuration:\n\
- Units must be TdModUnits w/ TdModUnitSpec and must recv from TDLayerSpec layer \
associated with SNc (gets td signal at end of 1st plus phase into u->td)\n\
- Connections need to be MatrixConSpec as learning occurs based on the td-signal \
on the matrix units.\n\
- This layer must be after SNcLayer layer (TdLayerSpec) in list of layers\n\
- Units must be organized into groups (stipes) of same number as PFC, Patch";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool MatrixLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  if(decay.clamp_phase2) {
    SetUnique("decay", true);
    decay.phase2 = 1.0f;
    decay.clamp_phase2 = false;
  }

  if(lay->units.gp.size == 1) {
    taMisc::Error("MatrixLayerSpec: layer must contain multiple unit groups (= stripes) for indepent searching of gating space!");
    return false;
  }

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_TdModUnit)) {
    taMisc::Error("MatrixLayerSpec: must have TdModUnits!");
    return false;
  }

  MatrixUnitSpec* us = (MatrixUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_MatrixUnitSpec)) {
    taMisc::Error("MatrixLayerSpec: UnitSpec must be MatrixUnitSpec!");
    return false;
  }
  // must have these not initialized every trial!
  us->hyst.init = false;
  us->acc.init = false;
  if(us->act.avg_dt <= 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.005f;
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires UnitSpec act.avg_dt > 0, I just set it to .005 for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if((us->opt_thresh.learn >= 0.0f) || us->opt_thresh.updt_wts) {
    if(!quiet) taMisc::Error("MatrixLayerSpec: UnitSpec opt_thresh.learn must be -1 to allow proper recording of p_savg and p_max_cor values in connections",
		  "even when this unit is not active on a prior time step!  I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
    us->SetUnique("opt_thresh", true);
    us->opt_thresh.learn = -1.0f;
    us->opt_thresh.updt_wts = false;
  }
  us->UpdateAfterEdit();

  LeabraBiasSpec* bs = (LeabraBiasSpec*)us->bias_spec.spec;
  if(bs == NULL) {
    taMisc::Error("MatrixLayerSpec: Error: null bias spec in unit spec", us->name);
    return false;
  }

  // find snc layer (TdLayerSpec w/ marker cons)
  LeabraLayer* vta_lay = NULL;
  LeabraLayer* snc_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  for(g=0;g<u->recv.gp.size; g++) {
    recv_gp = (LeabraCon_Group*)u->recv.gp[g];
    LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("*** MatrixLayerSpec: null from layer in recv projection:", (String)g);
      return false;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)
       && fmlay->spec.spec->InheritsFrom(TA_TdLayerSpec)) {
      vta_lay = fmlay;
    }
    else if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)
	    && fmlay->spec.spec->InheritsFrom(TA_SNcLayerSpec)) {
      snc_lay = fmlay;
    }
    else {
      MatrixConSpec* cs = (MatrixConSpec*)recv_gp->spec.spec;
      if(!cs->InheritsFrom(TA_MatrixConSpec)) {
	taMisc::Error("MatrixLayerSpec:  Receiving connections must be of type MatrixConSpec!");
	return false;
      }
      if(cs->wt_limits.sym != false) {
	cs->SetUnique("wt_limits", true);
	cs->wt_limits.sym = false;
	if(!quiet) taMisc::Error("MatrixLayerSpec: requires recv connections to have wt_limits.sym=false, I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
  }
  int myidx = lay->own_net->layers.FindLeaf(lay);
  if(snc_lay != NULL) {
    int sncidx = lay->own_net->layers.FindLeaf(snc_lay);
    if(sncidx > myidx) {
      taMisc::Error("MatrixLayerSpec: SNc layer (SNcLayerSpec) must be *before* this layer in list of layers -- it is now after, won't work");
      return false;
    }
  }
  if(vta_lay != NULL) {
    int vtaidx = lay->own_net->layers.FindLeaf(vta_lay);
    if(vtaidx > myidx) {
      taMisc::Error("MatrixLayerSpec: VTA layer (TdLayerSpec) must be *before* this layer in list of layers -- it is now after, won't work");
      return false;
    }
  }
  LeabraLayer* vta_lay_p2 = (LeabraLayer*)lay->own_net->layers.FindName("VTA");
  if(vta_lay_p2 == NULL) {
    taMisc::Error("*** MatrixLayerSpec: Error, cannot find layer named VTA for computing err rate!");
    return false;
  }
  return true;
}

void MatrixLayerSpec::InitWtState(LeabraLayer* lay) {
  LeabraLayerSpec::InitWtState(lay);
  int mg;
  for(mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* gp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
    gp->misc_state = 0;	// reset!
    int i;
    for(i=0;i<gp->size;i++) {
      LeabraUnit* u = (LeabraUnit*)gp->FastEl(i);
      u->vcb.g_a = 0.0f; u->vcb.g_h = 0.0f;
      u->misc_1 = 0.0f;
    }
  }
}

MatrixLayerSpec::GateSignal MatrixLayerSpec::GetGateSignal(LeabraLayer* lay, int strp) {
  LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp.FastEl(strp);
  if(mugp->size <= 0) return GATE_NOGO;
  MatrixUnitSpec* us = (MatrixUnitSpec*)lay->unit_spec.spec;
  GateSignal rval = GATE_NOGO;
  if(mugp->acts.max >= us->opt_thresh.send) {
    rval = (GateSignal) (mugp->acts.max_i % 2); // GO = 0, NOGO = 1
  }
  return rval;
}

void MatrixLayerSpec::Compute_ClampMinus(LeabraUnit_Group* ugp, LeabraTrial*) {
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->SetExtFlag(Unit::EXT);
    u->ext = u->act_p;
  }
}

void MatrixLayerSpec::UpdateGates(LeabraLayer* lay, LeabraTrial* trl) {
  // this function is called by the Patch Layer, so it can get up-to-date information!

  if(trl->phase_no == 1) {	// phase 1 can only de-activate an active representation
    int mg;
    for(mg=0;mg<lay->units.gp.size;mg++) {
      LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      GateSignal gate_sig = GetGateSignal(lay, mg);
      if(gate_sig == GATE_GO) {
	mugp->misc_state = 0;	// it is off!
      }
    }
  }
  else {			// phase 2
    int mg;
    for(mg=0;mg<lay->units.gp.size;mg++) {
      LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      GateSignal gate_sig = GetGateSignal(lay, mg);
      if(gate_sig == GATE_NOGO) {
	if(mugp->misc_state > 0) // if on, update time on
	  mugp->misc_state += 1;
	else
	  mugp->misc_state -= 1; // else, update time off
      }
      else {			// GATE_GO
	if(gate.go_toggle) {
	  if(mugp->misc_state > 0) { // already maintaining: GO here means OFF!!
	    mugp->misc_state = 0; // OFF!
	  }
	  else {		// was off, so now its ok to turn it on!
	    mugp->misc_state = 1;	// it is on!
	  }
	}
	else {			// GO in second plus means GO, regardless!
	  mugp->misc_state = 1;	// it is on!
	}
      }

      int max_dur = max_maint_dur.GetDur(mg);
      float nopup = nop_up_g_h.GetG(mg);
      int j;
      for(j=0;j<mugp->size;j++) {
	LeabraUnit* u = (LeabraUnit*)mugp->FastEl(j);
	u->vcb.g_a = 0.0f; u->vcb.g_h = 0.0f;
	if((j % 2) == GATE_NOGO) {
	  if((max_dur > 0) && (mugp->misc_state >= max_dur))
	    u->vcb.g_a = gate.down_g_a; // no more nops!
	  else if(mugp->misc_state > 0) // upstate
	    u->vcb.g_h = nopup;
	}
      }
    }
  }
}

void MatrixLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no == 0) {
    // set noise, only on the minus phase!
    int g;
    for(g=0;g<lay->units.gp.size;g++) {
      Unit_Group* gp = (Unit_Group*)lay->units.gp.FastEl(g);
      int j;
      for(j=0;j<gp->size;j++) {
	LeabraUnit* u = (LeabraUnit*)gp->FastEl(j);
	NoiseBiasCon* nbc = (NoiseBiasCon*)u->bias;
	nbc->wt = nbc->zwt + bias_noise.Gen();
      }
    }
    // clamp to previous for minus phase
    lay->SetExtFlag(Unit::EXT);
    int mg;
    for(mg=0;mg<lay->units.gp.size;mg++) {
      LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      Compute_ClampMinus(mugp, trl);
    }
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
  }
  else {			// run free in plus phases to compute gating
    lay->hard_clamped = false;
    lay->InitExterns();
  }
}

void MatrixLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  LeabraLayerSpec::PostSettle(lay, trl, set_both);
  if(trl->phase_no == 0) return;
  if(trl->phase_no == 1) {
    TdModUnit* u;
    taLeafItr ui;
    FOR_ITR_EL(TdModUnit, u, lay->units., ui) {
      u->act_m = u->act_p;	// save the plus phase, goes into p_act_m
    }
    Compute_ActMAvg(lay, trl);
  }
  else {			// phase_no == 2
    TdModUnit* u;
    taLeafItr ui;
    FOR_ITR_EL(TdModUnit, u, lay->units., ui) {
      u->act_p = u->act_pp;	// save the second plus phase, goes into p_act_p
    }
    Compute_ActPAvg(lay, trl);
  }

  if(gate.no_patch)
    UpdateGates(lay, trl);
}

void MatrixLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no != 1))
    return;
  float err_rate = 0.0f;
  LeabraLayer* vta_lay = (LeabraLayer*)lay->own_net->layers.FindName("VTA");
  if(vta_lay != NULL) {
    LeabraUnit* u = (LeabraUnit*)vta_lay->units.FastEl(0);
    if(u != NULL) {
      err_rate = u->misc_1;
    }
  }

  MatrixUnitSpec* us = (MatrixUnitSpec*)lay->unit_spec.spec;
  TdModUnit* u;
  taLeafItr ui;
  FOR_ITR_EL(TdModUnit, u, lay->units., ui) {
    u->p_act_m = MAX(u->p_act_m, u->p_act_p); // use max val as "or" for firing in either phase 1 or 2

     // only if negative-going, modulate by gain: PROTECT_NEG
    if((u->misc_1 > 0.0f) && (u->td < 0.0f) && (err_rate < gate.snc_err_thr)) {
      float snc_eff = gate.snc_gain * u->misc_1;
      snc_eff = MIN(1.0, snc_eff);
      u->td *= (1.0f - snc_eff);
    }

    u->p_act_p = u->p_act_m + u->td * u->p_act_m; // td at time t modulates activations from t-1, p = plus phase
    u->p_act_p = us->act_range.Clip(u->p_act_p); // keep it clean
  }
  LeabraLayerSpec::Compute_dWt(lay, trl);
}


//////////////////////////////////
//	PFC Layer Spec		//
//////////////////////////////////

void PFCMaintSpec::Initialize() {
  off_accom = false;
  td_mod_gc = NO_TD_MOD;
  td_gain = .1f;
}

void PFCLayerSpec::Initialize() {
  SetUnique("gp_kwta", true);
  gp_kwta.k_from = KWTASpec::USE_PCT;
  gp_kwta.pct = .15f;
  SetUnique("inhib_group", true);
  inhib_group = UNIT_GROUPS;
  SetUnique("compute_i", true);
  compute_i = KWTA_AVG_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .6f;
  SetUnique("decay", true);
  decay.event = 0.0f;
  decay.phase = 0.0f;
  decay.phase2 = 0.1f;
  decay.clamp_phase2 = false;	// this is the one exception!
}

void PFCLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  Initialize();
}

void PFCLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(maint, this);
}

void PFCLayerSpec::HelpConfig() {
  String help = "PFCLayerSpec Computation:\n\
The PFC maintains activation over time (activation-based working memory) via \
excitatory intracelluar ionic mechanisms (implemented via the hysteresis channels, gc.h), \
and excitatory self-connections. These ion channels are turned on and off via units in the \
MatrixLayerSpec layer, which are themselves trained up by other basal ganglia (BG) layers.  \
Updating occurs in the 2nd plus phase --- if a gating signal was activated, any previous ion \
current is turned off, and the units are allowed to settle into a new state in the 2nd plus -- \
then the ion channels are activated in proportion to activations at the end of this 2nd phase.\n\
\nPFCLayerSpec Configuration:\n\
- Use the ConfigureBG button to automatically configure BG layers.\n\
- Units must recv MarkerConSpec from MatrixLayerSpec layer for gating\n\
- This layer must be after MatrixLayerSpec layer in list of layers\n\
- Units must be organized into groups corresponding to the matrix groups (stripes).";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool PFCLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  if(decay.clamp_phase2) {
    SetUnique("decay", true);
    decay.clamp_phase2 = false;
  }

  if(lay->units.gp.size == 1) {
    taMisc::Error("PFCLayerSpec: layer must contain multiple unit groups (= stripes) for indepent searching of gating space!");
    return false;
  }

  if(trl->phase_order != LeabraTrial::MINUS_PLUS_PLUS) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraTrial phase_oder = MINUS_PLUS_PLUS, I just set it for you");
    trl->phase_order = LeabraTrial::MINUS_PLUS_PLUS;
  }
  if(trl->first_plus_dwt != LeabraTrial::ONLY_FIRST_DWT) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraTrial first_plus_dwt = ONLY_FIRST_DWT, I just set it for you");
    trl->first_plus_dwt = LeabraTrial::ONLY_FIRST_DWT;
  }
  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  LeabraSettle* set = (LeabraSettle*)trl->FindSubProc(&TA_LeabraSettle);
  if(set->min_cycles_phase2 < 35) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraSettle min_cycles_phase2 >= 35, I just set it for you");
    set->min_cycles_phase2 = 35;
  }

  if(!lay->units.el_typ->InheritsFrom(TA_TdModUnit)) {
    taMisc::Error("PFCLayerSpec: must have TdModUnits!");
    return false;
  }

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_TdModUnitSpec)) {
    taMisc::Error("PFCLayerSpec: UnitSpec must be TdModUnitSpec!");
    return false;
  }

  if(us->act.avg_dt <= 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.005f;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec act.avg_dt > 0, I just set it to .005 for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->g_bar.h < .3f) {
    us->SetUnique("g_bar", true);
    us->g_bar.h = .5f;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec g_bar.h around .5, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->g_bar.a < 1.0f) {
    us->SetUnique("g_bar", true);
    us->g_bar.a = 2.0f;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec g_bar.a around 2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->hyst.init) {
    us->SetUnique("hyst", true);
    us->hyst.init = false;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec hyst.init = false, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }

  int matrix_prjn_idx;
  LeabraLayer* matrix_lay = FindMatrixLayer(lay, matrix_prjn_idx);
  if((matrix_prjn_idx < 0) || (matrix_lay == NULL)) return false;

  if(!matrix_lay->spec.spec->InheritsFrom(TA_MatrixLayerSpec)) {
    taMisc::Error("PFCLayerSpec: matrix layer does not have MatrixLayerSpec layer spec!");
    return false;
  }

  if(matrix_lay->units.gp.size != lay->units.gp.size) {
    taMisc::Error("PFCLayerSpec: MatrixLayer unit groups must = PFCLayer unit groups!");
    matrix_lay->geom.z = lay->units.gp.size;
    return false;
  }

  // check for ordering of layers!
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int mtidx = lay->own_net->layers.FindLeaf(matrix_lay);
  if(mtidx > myidx) {
    taMisc::Error("PFCLayerSpec: MatrixLayerSpec must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  for(g=0;g<u->recv.gp.size; g++) {
    recv_gp = (LeabraCon_Group*)u->recv.gp[g];
    LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("*** PFCLayerSpec: null from layer in recv projection:", (String)g);
      return false;
    }
    LeabraConSpec* cs = (MatrixConSpec*)recv_gp->spec.spec;
    if(cs->InheritsFrom(TA_MarkerConSpec)) continue;
    if((fmlay == lay) && (recv_gp->prjn->spec.type->InheritsFrom(TA_OneToOnePrjnSpec))) {
      if((cs->rnd.mean != .9f) || (cs->rnd.var != 0.0f)) {
	cs->SetUnique("rnd", true);
	cs->rnd.mean = .9f; cs->rnd.var = 0.0f;
	if(!quiet) taMisc::Error("PFCLayerSpec: requires self connections to have rnd.mean=.9, .var=0, I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
      if(cs->wt_scale.rel > 0.5f) {
	cs->SetUnique("wt_scale", true);
	cs->wt_scale.rel = 0.1f;
	if(!quiet) taMisc::Error("PFCLayerSpec: requires self connections to have wt_scale.rel small (e.g., .1), I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
      if(cs->lrate != 0.0f) {
	cs->SetUnique("lrate", true);
	cs->lrate = 0.0f;
	if(!quiet) taMisc::Error("PFCLayerSpec: requires self connections to have lrate=0, I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
    else if(fmlay == lay) {
      if(cs->wt_scale.rel > 0.5f) {
	cs->SetUnique("wt_scale", true);
	cs->wt_scale.rel = 0.1f;
	if(!quiet) taMisc::Error("PFCLayerSpec: requires self connections to have wt_scale.rel small (e.g., .1), I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
  }
  return true;
}

LeabraLayer* PFCLayerSpec::FindMatrixLayer(LeabraLayer* lay, int& matrix_prjn_idx) {
  matrix_prjn_idx = -1;		// -1 = not found
  LeabraLayer* rval = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
      if(fmlay == NULL) {
	taMisc::Error("*** PFCLayerSpec: Warning: NULL prjn layer from pointer!");
	continue;
      }
      if(!fmlay->spec.spec->InheritsFrom(TA_MatrixLayerSpec))
	continue;
      rval = fmlay;
      matrix_prjn_idx = g;
      break;
    }
  }
  if(matrix_prjn_idx < 0) {
    taMisc::Error("*** PFCLayerSpec: Warning: no projection from Matrix Layer found: must have MarkerConSpec!");
  }
  return rval;
}

void PFCLayerSpec::Compute_GatePPhase12(LeabraLayer* lay, LeabraTrial* trl, LeabraLayer* matrix_lay, MatrixLayerSpec* mls) {
  if(trl->phase_no == 1) {	// clear out g_h activations based on gating signal..
    int mg;
    for(mg=0;mg<matrix_lay->units.gp.size;mg++) {
      LeabraUnit_Group* gategp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      MatrixLayerSpec::GateSignal gate_sig = mls->GetGateSignal(matrix_lay, mg);
      if(gate_sig == MatrixLayerSpec::GATE_GO) { // clears out any existing representations (toggle)
	int j;
	for(j=0;j<gategp->size;j++) {
	  LeabraUnit* u = (LeabraUnit*)gategp->FastEl(j);
	  u->vcb.g_h = u->vcb.g_a = u->misc_1 = 0.0f;
	}
      }
      if(maint.td_mod_gc == PFCMaintSpec::NO_TD_MOD) continue;
      int j;
      for(j=0;j<gategp->size;j++) {
	TdModUnit* u = (TdModUnit*)gategp->FastEl(j);
	if((maint.td_mod_gc == PFCMaintSpec::MAINT_ONLY) && (u->misc_1 == 0.0f)) continue;
	if(u->td == 0.0f) continue;
	float td = u->td * maint.td_gain;
	if(td < 0.0f) {
	  if(u->vcb.g_h > 0.0f)
	    u->vcb.g_h += td * u->act_m; // decrease h current (td is neg..)
	  else
	    u->vcb.g_a += -td * u->act_m; // increase a current
	}
	else {
	  if(u->vcb.g_a > 0.0f)
	    u->vcb.g_a -= td * u->act_m; // decrease a current
	  else
	    u->vcb.g_h += td * u->act_m; // increase h current
	}
	if(u->vcb.g_a > 1.0f) u->vcb.g_a = 1.0f;
	else if(u->vcb.g_a < 0.0f) u->vcb.g_a = 0.0f;
	if(u->vcb.g_h > 1.0f) u->vcb.g_h = 1.0f;
	else if(u->vcb.g_h < 0.0f) u->vcb.g_h = 0.0f;
      }
    }
  }
  else {			// gating occurs on phase 2
    int mg;
    for(mg=0;mg<matrix_lay->units.gp.size;mg++) {
      LeabraUnit_Group* gategp = (LeabraUnit_Group*)lay->units.gp.FastEl(mg);
      LeabraUnit_Group* mugp = (LeabraUnit_Group*)matrix_lay->units.gp.FastEl(mg);
      int j;
      for(j=0;j<gategp->size;j++) {
	LeabraUnit* u = (LeabraUnit*)gategp->FastEl(j);
	if(mugp->misc_state == 0) { 	// clear: OFF!
	  if(maint.off_accom)
	    u->vcb.g_a = u->vcb.g_h;
	  u->vcb.g_h = u->misc_1 = 0.0f;
	}
	else if(mugp->misc_state == 1) { // GATE_GO!
	  u->vcb.g_h = u->misc_1 = u->act_eq; // set marker for active maintenance
	  u->vcb.g_a = 0.0f;
	}
      }
    }
  }
}

void PFCLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial*) {
  // not to hard clamp: needs to update in 2nd plus phase!
  lay->hard_clamped = false;
  lay->InitExterns();
  return;
}

void PFCLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  LeabraLayerSpec::PostSettle(lay, trl, set_both);

  if(trl->phase_no == 0) return; // not for minus phase

  int matrix_prjn_idx;
  LeabraLayer* matrix_lay = FindMatrixLayer(lay, matrix_prjn_idx);
  if((matrix_prjn_idx < 0) || (matrix_lay == NULL)) return;
  MatrixLayerSpec* mls = (MatrixLayerSpec*)matrix_lay->spec.spec;

  Compute_GatePPhase12(lay, trl, matrix_lay, mls);
}

void PFCLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no != 1))
    return; // only do first dwt!
  LeabraLayerSpec::Compute_dWt(lay, trl);
}


///////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////
//		Wizard		//
//////////////////////////////////

void LeabraWiz::Initialize() {
  connectivity = BIDIRECTIONAL;
}

void LeabraWiz::StdNetwork(Network* net) {
  if(net == NULL)
    net = pdpMisc::GetNewNetwork(GET_MY_OWNER(Project));
  if(net == NULL) return;
  Wizard::StdNetwork(net);
  StdLayerSpecs(net);
}

void LeabraWiz::StdLayerSpecs(Network* net) {
  if(net == NULL) return;
  Project* proj = GET_MY_OWNER(Project);
  LeabraLayerSpec* hid = (LeabraLayerSpec*)pdpMisc::FindMakeSpec(proj, NULL, &TA_LeabraLayerSpec);
  hid->name = "HiddenLayer";
  LeabraLayerSpec* inout = (LeabraLayerSpec*)hid->children.FindMakeSpec("Input_Output", &TA_LeabraLayerSpec);
  hid->compute_i = LeabraLayerSpec::KWTA_AVG_INHIB;
  hid->i_kwta_pt = .6f;
  inout->SetUnique("compute_i", true);
  inout->SetUnique("i_kwta_pt", true);
  inout->SetUnique("kwta", true);
  inout->compute_i = LeabraLayerSpec::KWTA_INHIB;
  inout->i_kwta_pt = .25f;
  inout->kwta.k_from = KWTASpec::USE_PAT_K;
  winbMisc::DelayedMenuUpdate(hid);

  int i;
  if(net->layers.size == layer_cfg.size) {	// likely to be using specs
    for(i=0;i<layer_cfg.size;i++) {
      LayerWizEl* el = (LayerWizEl*)layer_cfg[i];
      Layer* lay = (Layer*)net->layers.FindName(el->name);
      if(lay != NULL) {
	if(el->io_type == LayerWizEl::HIDDEN)
	  lay->SetLayerSpec(hid);
	else
	  lay->SetLayerSpec(inout);
      }
    }
  }
  else {
    for(i=0;i<net->layers.size;i++) {
      Layer* lay = (Layer*)net->layers[i];
      String nm = lay->name;
      nm.downcase();
      if(nm.contains("hid"))
	lay->SetLayerSpec(hid);
      else if(nm.contains("in") || nm.contains("out") || nm.contains("resp"))
	lay->SetLayerSpec(inout);
      else
	lay->SetLayerSpec(hid);
    }
  }

  // move the bias spec under the con spec
  LeabraBiasSpec* bs = (LeabraBiasSpec*)proj->specs.FindType(&TA_LeabraBiasSpec);
  if(bs != NULL) {
    LeabraConSpec* ps = (LeabraConSpec*)bs->FindParent();
    if(ps != NULL) return;
    ps = (LeabraConSpec*)proj->specs.FindSpecTypeNotMe(&TA_LeabraConSpec, bs);
    if(ps != NULL) {
      ps->children.Transfer(bs);
      winbMisc::DelayedMenuUpdate(ps);
    }
  }
}

///////////////////////////////////////////////////////////////
//			Unit Inhib
///////////////////////////////////////////////////////////////

void LeabraWiz::SRNContext(Network* net) {
  if(net == NULL) {
    taMisc::Error("SRNContext: must have basic constructed network first");
    return;
  }
  Project* proj = GET_MY_OWNER(Project);
  OneToOnePrjnSpec* otop = (OneToOnePrjnSpec*)pdpMisc::FindMakeSpec(proj, "CtxtPrjn", &TA_OneToOnePrjnSpec);
  LeabraContextLayerSpec* ctxts = (LeabraContextLayerSpec*)pdpMisc::FindMakeSpec(proj, "CtxtLayerSpec", &TA_LeabraContextLayerSpec);

  if((otop == NULL) || (ctxts == NULL)) {
    return;
  }

  LeabraLayer* hidden = (LeabraLayer*)net->FindLayer("Hidden");
  LeabraLayer* ctxt = (LeabraLayer*)net->FindMakeLayer("Context");
  
  if((hidden == NULL) || (ctxt == NULL)) return;

  ctxt->SetLayerSpec(ctxts);
  ctxt->n_units = hidden->n_units;
  ctxt->geom = hidden->geom;

  net->layers.MoveAfter(hidden, ctxt);
  net->FindMakePrjn(ctxt, hidden, otop); // one-to-one into the ctxt layer
  net->FindMakePrjn(hidden, ctxt); 	 // std prjn back into the hidden from context
  net->Build();
  net->Connect();
}

///////////////////////////////////////////////////////////////
//			Unit Inhib
///////////////////////////////////////////////////////////////

void LeabraWiz::UnitInhib(Network* net, int n_inhib_units) {
  Project* proj = GET_MY_OWNER(Project);
  net->RemoveUnits();
  
  LeabraUnitSpec* basic_us = (LeabraUnitSpec*)pdpMisc::FindSpecType(proj, &TA_LeabraUnitSpec);
  if(basic_us == NULL) {
    taMisc::Error("ConfigUnitInhib: basic LeabraUnitSpec not found, bailing!");
    return;
  }
  LeabraUnitSpec* inhib_us = (LeabraUnitSpec*)basic_us->children.FindMakeSpec("InhibUnits", &TA_LeabraUnitSpec);
  if(inhib_us == NULL) return;

  LeabraConSpec* basic_cs = (LeabraConSpec*)pdpMisc::FindSpecType(proj, &TA_LeabraConSpec);
  if(basic_cs == NULL) {
    taMisc::Error("ConfigUnitInhib: basic LeabraConSpec not found, bailing!");
    return;
  }
  LeabraConSpec* inhib_cs = (LeabraConSpec*)basic_cs->children.FindMakeSpec("InhibCons", &TA_LeabraConSpec);
  if(inhib_cs == NULL) return;

  LeabraConSpec* fb_inhib_cs = (LeabraConSpec*)basic_cs->children.FindMakeSpec("FBtoInhib", &TA_LeabraConSpec);
  if(fb_inhib_cs == NULL) return;
  LeabraConSpec* ff_inhib_cs = (LeabraConSpec*)fb_inhib_cs->children.FindMakeSpec("FFtoInhib", &TA_LeabraConSpec);
  if(ff_inhib_cs == NULL) return;

  LeabraLayerSpec* basic_ls = (LeabraLayerSpec*)pdpMisc::FindSpecType(proj, &TA_LeabraLayerSpec);
  if(basic_ls == NULL) {
    taMisc::Error("ConfigUnitInhib: basic LeabraLayerSpec not found, bailing!");
    return;
  }
  LeabraLayerSpec* inhib_ls = (LeabraLayerSpec*)basic_ls->children.FindMakeSpec("InhibLayers", &TA_LeabraLayerSpec);
  if(inhib_ls == NULL) return;

  FullPrjnSpec* fullprjn = (FullPrjnSpec*)pdpMisc::FindSpecType(proj, &TA_FullPrjnSpec);
  if(fullprjn == NULL) {
    taMisc::Error("ConfigUnitInhib: basic FullPrjnSpec not found, bailing!");
    return;
  }

  // todo: optimize these params..
  basic_us->dt.vm = .04f;
  basic_us->g_bar.i = 10.0f;
  inhib_us->SetUnique("dt", true);
  inhib_us->dt.vm = .07f;

  inhib_cs->SetUnique("rnd", true);
  inhib_cs->rnd.mean = 1.0f;  inhib_cs->rnd.var = 0.0f;
  inhib_cs->SetUnique("wt_limits", true);
  inhib_cs->wt_limits.sym = false;
  inhib_cs->SetUnique("inhib", true);
  inhib_cs->inhib = true;

  fb_inhib_cs->SetUnique("wt_limits", true);
  fb_inhib_cs->wt_limits.sym = false;
  fb_inhib_cs->SetUnique("rnd", true);
  fb_inhib_cs->rnd.mean = .5f;  fb_inhib_cs->rnd.var = 0.05f;
  fb_inhib_cs->SetUnique("lrate", true);
  fb_inhib_cs->lrate = 0.0f;

  // todo: optimize
  ff_inhib_cs->SetUnique("wt_scale", true);
  ff_inhib_cs->wt_scale.abs = .4f;

  basic_ls->compute_i = LeabraLayerSpec::UNIT_INHIB;
  basic_ls->adapt_i.type = AdaptISpec::G_BAR_I;
  basic_ls->adapt_i.tol = .02f;	// these params sometimes get off..
  basic_ls->adapt_i.p_dt = .1f;

  inhib_ls->SetUnique("kwta", true);
  inhib_ls->kwta.k_from = KWTASpec::USE_PCT;
  inhib_ls->kwta.pct = .34f;

  basic_us->UpdateAfterEdit();
  basic_cs->UpdateAfterEdit();
  basic_ls->UpdateAfterEdit();

  int i;
  for(i=0;i<net->layers.size;i++) {
    LeabraLayer* lay = (LeabraLayer*)net->layers[i];
    String nm = lay->name;
    nm.downcase();
    if(nm.contains("input") || nm.contains("stimulus")) continue;
    if(nm.contains("_inhib")) continue;

    String inm = lay->name + "_Inhib";
    LeabraLayer* ilay = (LeabraLayer*)net->layers.FindName(inm);
    if(ilay == NULL) {
      ilay = (LeabraLayer*)net->layers.NewEl(1);
      ilay->name = inm;
      ilay->pos.z = lay->pos.z;
      ilay->pos.x = lay->pos.x + lay->act_geom.x + 1;
      ilay->n_units = n_inhib_units;
      if(n_inhib_units <= 20) {
	ilay->geom.x = 2; ilay->geom.y = n_inhib_units / 2;
	while(ilay->geom.x * ilay->geom.y < n_inhib_units) ilay->geom.y++;
      }
      else if(n_inhib_units <= 40) {
	ilay->geom.x = 4; ilay->geom.y = n_inhib_units / 4;
	while(ilay->geom.x * ilay->geom.y < n_inhib_units) ilay->geom.y++;
      }
    }
    ilay->SetLayerSpec(inhib_ls);
    ilay->SetUnitSpec(inhib_us);

    int j;
    for(j=0;j<lay->projections.size;j++) {
      LeabraLayer* fmlay = (LeabraLayer*)((Projection*)lay->projections[j])->from;
      if(fmlay->name.contains("_Inhib")) continue;
      if(fmlay == lay) continue;
      net->FindMakePrjn(ilay, fmlay, fullprjn, ff_inhib_cs);
    }
    net->FindMakePrjn(ilay, lay, fullprjn, fb_inhib_cs);
    net->FindMakePrjn(lay, ilay, fullprjn, inhib_cs);
    net->FindMakePrjn(ilay, ilay, fullprjn, inhib_cs);
  }

  net->UpdateAfterEdit();
  net->InitAllViews();

  winbMisc::DelayedMenuUpdate(net);
  winbMisc::DelayedMenuUpdate(proj);

  // set settle cycles to 300
  taLeafItr pi;
  LeabraSettle* sp;
  FOR_ITR_EL(LeabraSettle, sp, proj->processes.,pi) {
    if(!sp->InheritsFrom(&TA_LeabraSettle)) continue;
    sp->cycle.max = 300;
    sp->min_cycles = 150;
  }

  SelectEdit* edit = pdpMisc::FindSelectEdit(proj);
  if(edit != NULL) {
    basic_us->SelectForEditNm("dt", edit, "excite");
    inhib_us->SelectForEditNm("dt", edit, "inhib");
    basic_us->SelectForEditNm("g_bar", edit, "excite");
    ff_inhib_cs->SelectForEditNm("wt_scale", edit, "ff_inhib");
    fb_inhib_cs->SelectForEditNm("rnd", edit, "to_inhib");
    inhib_cs->SelectForEditNm("rnd", edit, "fm_inhib");
    basic_ls->SelectForEditNm("adapt_i", edit, "layers");

    FOR_ITR_EL(LeabraSettle, sp, proj->processes.,pi) {
      if(!sp->InheritsFrom(&TA_LeabraSettle)) continue;
      sp->SelectForEditNm("cycle", edit, sp->name);
      sp->SelectForEditNm("min_cycles", edit, sp->name);
    }
  }
}

static void set_n_stripes(Network* net, char* nm, int n_stripes, int n_units, bool sp) {
  LeabraLayer* lay = (LeabraLayer*)net->FindLayer(nm);
  if(lay == NULL) return;
  lay->geom.z = n_stripes;
  if(n_units > 0) lay->n_units = n_units;
  if(sp) {
    lay->gp_spc.x = 1;
    lay->gp_spc.y = 1;
  }
  lay->UpdateAfterEdit();
  if(n_stripes <= 4) {
    lay->gp_geom.x = n_stripes;
    lay->gp_geom.y = 1;
  }
}

void LeabraWiz::SetPFCStripes(Network* net, int n_stripes, int n_units) {
  set_n_stripes(net, "PFC", n_stripes, n_units, true);
  set_n_stripes(net, "Matrix", n_stripes, -1, true);
  set_n_stripes(net, "Patch", n_stripes, 12, true);
  set_n_stripes(net, "SNc", n_stripes, -1, false);
  net->RemoveUnitGroups();
  net->Build();
  net->Connect();
}

static void lay_set_geom(LeabraLayer* lay, int half_stripes) {
  if(lay->n_units == 0) {
    lay->geom.x = 1; lay->geom.y = 1; lay->n_units = 1;
  }
  lay->geom.z = half_stripes * 2;
  lay->gp_geom.y = 2; lay->gp_geom.x = half_stripes;
  lay->UpdateAfterEdit();
}

void LeabraWiz::BgPFC(Network* net, bool make_patch_snc, bool out_to_pfc_cons, int n_stripes) {
  String msg = "There are two things you will need to check manually after this automatic configuration \
process completes (this note will be repeated when things complete --- there may be some \
messages in the interim):\n\n";

  String man_msg = "1. Check that connection(s) were made from all appropriate output layers \
to the NAc layer, using the MarkerConSpec (MarkerCons) Con spec.  \
This will provide the error signal to the system based on output error performance.\n\n";

  man_msg += "2. Check that bidirectional connections between the pfc and all appropriate \
hidden layers, using the ToPFC projection and connection specs for the connection into the PFC \
(which should be uniform random) and a regular full projection from the PFC.  \
The con specs INTO the pfc should be ToPFC conspecs; the ones out should be regular learning conspecs.";

  msg += man_msg + "\n\nThe configuration will now be checked and a number of default parameters \
will be set.  If there are any actual errors which must be corrected before \
the network will run, you will see a message to that effect --- you will then need to \
re-run this configuration process to make sure everything is OK.  When you press \
Re/New/Init on the control process these same checks will be performed, so you \
can be sure everything is ok.";
  taMisc::Choice(msg,"Ok");	// "

  int half_stripes = n_stripes /2;
  half_stripes = MAX(1, half_stripes);
  n_stripes = half_stripes * 2;	// make it even

  Project* proj = GET_MY_OWNER(Project);
  net->RemoveUnits();

  LeabraLayer* patch = NULL;
  LeabraLayer* snc = NULL;

  // if not new layers, don't make prjns into them!
  bool	nac_new = false;
  bool	patch_new = false;
  bool	matrix_new = false;
  bool	pfc_new = false;

  net->layers.Remove("ImRew");
  net->layers.Remove("dRew");

  LeabraLayer* rew_targ_lay = (LeabraLayer*)net->FindMakeLayer("RewTarg");

  LeabraLayer* nac = (LeabraLayer*)net->FindMakeLayer("NAc", NULL, nac_new);
  LeabraLayer* vta = (LeabraLayer*)net->FindMakeLayer("VTA");
  if(nac == NULL || vta == NULL) return;
  if(nac_new) { 
    nac->pos.z = 0; nac->pos.y = 3; nac->pos.x = 0;
    vta->pos.z = 0; vta->pos.y = 0; vta->pos.x = 0;
  }

  if(make_patch_snc) {
    patch = (LeabraLayer*)net->FindMakeLayer("Patch", NULL, patch_new);
    snc = (LeabraLayer*)net->FindMakeLayer("SNc");

    if(patch_new) { 
      patch->pos.z = 0; patch->pos.y = 0; patch->pos.x = nac->pos.x + 6; 
      snc->pos.z = 0; snc->pos.x = patch->pos.x + 6; 
      if(n_stripes > 4) snc->pos.y = 9; 
      else snc->pos.y = 4;
    }
  }
  else {
    net->layers.Remove("Patch");
    net->layers.Remove("SNc");
  }
  LeabraLayer* matrix = (LeabraLayer*)net->FindMakeLayer("Matrix", NULL, matrix_new);
  LeabraLayer* pfc = (LeabraLayer*)net->FindMakeLayer("PFC", NULL, pfc_new);

  if(matrix == NULL || pfc == NULL) return;

  if(matrix_new) { 
    matrix->pos.z = 1; matrix->pos.y = 0; matrix->pos.x = nac->pos.x + 6; 
  }
  if(pfc_new) {
    pfc->pos.z = 2; pfc->pos.y = 0; pfc->pos.x = matrix->pos.x;
  }

  // sort the layers to get the order right..
  nac->name = "ZZZ3";
  vta->name = "ZZZ4";
  if(make_patch_snc) {
    patch->name = "ZZZ5";
    snc->name = "ZZZ6";
  }
  matrix->name = "ZZZ7";
  pfc->name = "ZZZ8";

  net->layers.Sort();

  nac->name = "NAc";
  vta->name = "VTA";
  if(make_patch_snc) {
    patch->name = "Patch";
    snc->name = "SNc";
  }
  matrix->name = "Matrix";
  pfc->name = "PFC";

  // set all layers in network to TdModUnits
  // also collect list of non-pfc/bg layers..
  Layer_MGroup other_lays;
  Layer_MGroup hidden_lays;
  Layer_MGroup output_lays;
  Layer_MGroup input_lays;
  int i;
  for(i=0;i<net->layers.size;i++) {
    LeabraLayer* lay = (LeabraLayer*)net->layers[i];
    lay->SetUnitType(&TA_TdModUnit);
    if(lay != nac && lay != vta && lay != patch && lay != snc && lay != matrix && lay != pfc) {
      other_lays.Link(lay);
      String nm = lay->name;
      nm.downcase();
      if(nm.contains("hidden"))
	hidden_lays.Link(lay);
      if(nm.contains("output") || nm.contains("response") || nm.contains("_out") || nm.contains("_resp"))
	output_lays.Link(lay);
      if(nm.contains("input") || nm.contains("stimulus") || nm.contains("_in"))
	input_lays.Link(lay);
      LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
      if(us == NULL || !us->InheritsFrom(TA_TdModUnitSpec)) {
// 	taMisc::Error("Warning: all unit specs for all layers must be of type TdModUnitSpec!  I am changing the type of:",
// 		      us->name);
	us->ChangeMyType(&TA_TdModUnitSpec);
      }
    }
  }

  BaseSpec_MGroup* units = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Units");
  BaseSpec_MGroup* cons = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Cons");
  BaseSpec_MGroup* layers = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Layers");
  BaseSpec_MGroup* prjns = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Prjns");

  if(units == NULL || cons == NULL || layers == NULL || prjns == NULL) return;

  LeabraUnitSpec* pfc_units = (LeabraUnitSpec*)units->FindMakeSpec("PFCUnits", &TA_TdModUnitSpec);
  LeabraUnitSpec* rewpred_units = (LeabraUnitSpec*)units->FindMakeSpec("RewPredUnits", &TA_TdModUnitSpec);
  LeabraUnitSpec* td_units = (LeabraUnitSpec*)units->FindMakeSpec("TdUnits", &TA_TdModUnitSpec);
  LeabraUnitSpec* matrix_units = (LeabraUnitSpec*)units->FindMakeSpec("MatrixUnits", &TA_MatrixUnitSpec);

  if(pfc_units == NULL || rewpred_units == NULL || td_units == NULL || matrix_units == NULL) return;

  LeabraConSpec* pfc_self = (LeabraConSpec*)cons->FindMakeSpec("PFCSelfCon", &TA_LeabraConSpec);
  LeabraConSpec* learn_cons = (LeabraConSpec*)cons->FindMakeSpec("LearnCons", &TA_LeabraConSpec);
  if(learn_cons == NULL) return;

  LeabraConSpec* topfc_cons = (LeabraConSpec*)learn_cons->FindMakeChild("ToPFC", &TA_LeabraConSpec);
  if(topfc_cons == NULL) return;
  LeabraConSpec* intra_pfc = (LeabraConSpec*)topfc_cons->FindMakeChild("IntraPFC", &TA_LeabraConSpec);
  LeabraConSpec* pfc_bias = (LeabraConSpec*)topfc_cons->FindMakeChild("PFCBias", &TA_LeabraBiasSpec);

  LeabraConSpec* rewpred_cons = (LeabraConSpec*)learn_cons->FindMakeChild("RewPredCons", &TA_RewPredConSpec);
  LeabraConSpec* matrix_cons = (LeabraConSpec*)learn_cons->FindMakeChild("MatrixCons", &TA_MatrixConSpec);
  LeabraConSpec* bg_bias = (LeabraConSpec*)learn_cons->FindMakeChild("BgBias", &TA_LeabraBiasSpec);
  if(bg_bias == NULL) return;
  LeabraConSpec* matrix_bias = (LeabraConSpec*)bg_bias->FindMakeChild("MatrixBias", &TA_NoiseBiasSpec);
  LeabraConSpec* fixed_bias = (LeabraConSpec*)bg_bias->FindMakeChild("FixedBias", &TA_LeabraBiasSpec);

  LeabraConSpec* marker_cons = (LeabraConSpec*)cons->FindMakeSpec("MarkerCons", &TA_MarkerConSpec);

  if(pfc_self == NULL || intra_pfc == NULL || rewpred_cons == NULL || matrix_cons == NULL ||
     marker_cons == NULL || matrix_bias == NULL || fixed_bias == NULL)
    return;

  // .01 hebb on learn cons
  learn_cons->lmix.hebb = .01f;
  learn_cons->not_used_ok = true;
  learn_cons->UpdateAfterEdit();
  // slow learning rate on to pfc cons!
  topfc_cons->SetUnique("lrate", true);
  topfc_cons->lrate = .001f;
  topfc_cons->SetUnique("lmix", true);
  topfc_cons->lmix.hebb = .001f;
  intra_pfc->SetUnique("wt_scale", true);
  intra_pfc->wt_scale.rel = .1f;
  bg_bias->SetUnique("lrate", true);
  bg_bias->lrate = 0.0f;
  fixed_bias->SetUnique("lrate", true);
  fixed_bias->lrate = 0.0f;

  // optimization to speed up settling in phase 2: only the basic layers here
  int j;
  for(j=0;j<proj->specs.size;j++) {
    if(proj->specs[j]->InheritsFrom(TA_LeabraLayerSpec)) {
      LeabraLayerSpec* sp = (LeabraLayerSpec*)proj->specs[j];
      sp->decay.clamp_phase2 = true;
      sp->UpdateAfterEdit();
    }
  }

  LeabraLayerSpec* pfcsp = (LeabraLayerSpec*)layers->FindMakeSpec("PFCLayer", &TA_PFCLayerSpec);
  RewPredLayerSpec* nacsp = (RewPredLayerSpec*)layers->FindMakeSpec("NAcLayer", &TA_RewPredLayerSpec);
  if(nacsp == NULL) return;
  LeabraLayerSpec* patchsp = (LeabraLayerSpec*)nacsp->FindMakeChild("PatchLayer", &TA_PatchLayerSpec);
  LeabraLayerSpec* tdsp = (LeabraLayerSpec*)layers->FindMakeSpec("TdLayer", &TA_TdLayerSpec);
  if(tdsp == NULL) return;
  LeabraLayerSpec* sncsp = (LeabraLayerSpec*)tdsp->children.FindMakeSpec("SNcLayer", &TA_SNcLayerSpec);
  MatrixLayerSpec* matrixsp = (MatrixLayerSpec*)layers->FindMakeSpec("MatrixLayer", &TA_MatrixLayerSpec);

  if(pfcsp == NULL || patchsp == NULL || tdsp == NULL || sncsp == NULL || matrixsp == NULL) return;

  UniformRndPrjnSpec* topfc = (UniformRndPrjnSpec*)prjns->FindMakeSpec("ToPFC", &TA_UniformRndPrjnSpec);
  ProjectionSpec* pfc_selfps = (ProjectionSpec*)prjns->FindMakeSpec("PFCSelf", &TA_OneToOnePrjnSpec);
  GpRndTesselPrjnSpec* intra_pfcps = (GpRndTesselPrjnSpec*)prjns->FindMakeSpec("IntraPFC", &TA_GpRndTesselPrjnSpec);
  ProjectionSpec* fullprjn = (ProjectionSpec*)prjns->FindMakeSpec("FullPrjn", &TA_FullPrjnSpec);
  ProjectionSpec* gponetoone = (ProjectionSpec*)prjns->FindMakeSpec("GpOneToOne", &TA_GpOneToOnePrjnSpec);
  ProjectionSpec* onetoone = (ProjectionSpec*)prjns->FindMakeSpec("OneToOne", &TA_OneToOnePrjnSpec);

  if(topfc == NULL || pfc_selfps == NULL || intra_pfcps == NULL || fullprjn == NULL || gponetoone == NULL
     || onetoone == NULL) return;

  // now have all the stuff, configure it!
  // set layer geometry:
  lay_set_geom(pfc, half_stripes);
  if(pfc->n_units == 1) { pfc->n_units = 30; pfc->geom.x = 5; pfc->geom.y = 6; }
  if(nac->n_units < 12) { nac->n_units = 12; nac->geom.x = 4; nac->geom.y = 3; }
  vta->n_units = 1;
  if(make_patch_snc) {
    lay_set_geom(patch, half_stripes);
    if(patch->n_units < 12) { patch->n_units = 12; patch->geom.x = 4; patch->geom.y = 3; }
    lay_set_geom(snc, half_stripes);
    snc->n_units = 1;
  }
  lay_set_geom(matrix, half_stripes);
  if(matrix->n_units == 1) { matrix->n_units = 10; matrix->geom.x = 2; matrix->geom.y = 5; }
  rew_targ_lay->n_units = 1;

  // set layer specs:
  pfc->SetLayerSpec(pfcsp);
  nac->SetLayerSpec(nacsp);
  vta->SetLayerSpec(tdsp);
  if(make_patch_snc) {
    patch->SetLayerSpec(patchsp);
    snc->SetLayerSpec(sncsp);
  }
  matrix->SetLayerSpec(matrixsp);

  // set bias specs for unit specs
  pfc_units->bias_spec.SetSpec(pfc_bias);
  pfc_units->SetUnique("act_reg", true); // turn on activity regulation in pfc by default!
  pfc_units->act_reg.on = true;

  rewpred_units->bias_spec.SetSpec(bg_bias);
  td_units->bias_spec.SetSpec(fixed_bias);
  matrix_units->bias_spec.SetSpec(matrix_bias);

  // set unit specs for layers
  pfc->SetUnitSpec(pfc_units);
  nac->SetUnitSpec(rewpred_units);
  vta->SetUnitSpec(td_units);
  if(make_patch_snc) {
    patch->SetUnitSpec(rewpred_units);
    snc->SetUnitSpec(td_units);
  }
  matrix->SetUnitSpec(matrix_units);
  
  // set projection parameters
  topfc->p_con = .4;
  
  pfc_selfps->self_con = true;

  intra_pfcps->def_p_con = .4;
  intra_pfcps->recv_gp_n.y = 1;
  intra_pfcps->recv_gp_group.x = half_stripes;
  intra_pfcps->MakeRectangle(half_stripes, 1, 0, 1);
  intra_pfcps->wrap = false;
  
  // now need to make projections
  net->FindMakePrjn(vta, nac, onetoone, marker_cons);
  if(make_patch_snc) {
    net->FindMakePrjn(snc, patch, gponetoone, marker_cons);
    net->FindMakePrjn(matrix, snc, gponetoone, marker_cons);
    net->FindMakePrjn(patch, matrix, gponetoone, marker_cons);
  }

  net->FindMakePrjn(matrix, vta, fullprjn, marker_cons);
  net->FindMakePrjn(pfc, matrix, gponetoone, marker_cons);

  net->FindMakePrjn(matrix, pfc, gponetoone, matrix_cons);

  net->FindMakeSelfPrjn(pfc, pfc_selfps, pfc_self);
  //  net->FindMakeSelfPrjn(pfc, intra_pfcps, intra_pfc);

  net->FindMakePrjn(nac, pfc, fullprjn, rewpred_cons);
  if(make_patch_snc) {
    net->FindMakePrjn(patch, pfc, gponetoone, rewpred_cons);
  }
  for(i=0;i<other_lays.size;i++) {
    Layer* ol = (Layer*)other_lays[i];
    if(nac_new)
      net->FindMakePrjn(nac, ol, fullprjn, rewpred_cons);
    if(make_patch_snc && patch_new) {
      net->FindMakePrjn(patch, ol, fullprjn, rewpred_cons);
    }
    if(matrix_new)
      net->FindMakePrjn(matrix, ol, fullprjn, matrix_cons);
  }

  if(pfc_new) {
    for(i=0;i<hidden_lays.size;i++) {
      Layer* hl = (Layer*)hidden_lays[i];
      //      net->FindMakePrjn(pfc, hl, topfc, topfc_cons);
      net->FindMakePrjn(pfc, hl, fullprjn, topfc_cons);
      net->FindMakePrjn(hl, pfc, fullprjn, learn_cons);
    }
  }

  net->FindMakePrjn(nac, rew_targ_lay, onetoone, marker_cons);
  if(make_patch_snc)
    net->FindMakePrjn(patch, rew_targ_lay, onetoone, marker_cons);
  for(i=0;i<output_lays.size;i++) {
    Layer* ol = (Layer*)output_lays[i];
    net->FindMakePrjn(nac, ol, onetoone, marker_cons);
  }

  if(pfc_new) {
    for(i=0;i<input_lays.size;i++) {
      Layer* ol = (Layer*)input_lays[i];
      net->FindMakePrjn(pfc, ol, fullprjn, topfc_cons);
    }
    if(out_to_pfc_cons) {
      for(i=0;i<output_lays.size;i++) {
	Layer* ol = (Layer*)output_lays[i];
	net->FindMakePrjn(pfc, ol, fullprjn, topfc_cons);
      }
    }
  }

  SetPFCStripes(net, n_stripes);

  LeabraTrial* trl = (LeabraTrial*)pdpMisc::FindProcType(proj, &TA_LeabraTrial);
  if(trl == NULL) {
    taMisc::Error("Warning: a LeabraTrial TrialProcess was not found -- the trial \
process must be configured properly for the network to run, and this cannot be done \
at this time.  This also prevents other checks from being made.  All of this can be \
done later when you press Re/New/Init on a schedule process when you run the network.");
    return;
  }

  bool ok = pfcsp->CheckConfig(pfc, trl, true) && nacsp->CheckConfig(nac, trl, true) &&
    matrixsp->CheckConfig(matrix, trl, true);

  if(ok && make_patch_snc) ok = patchsp->CheckConfig(patch, trl, true) &&
			     sncsp->CheckConfig(snc, trl, true);

  if(!ok) {
    msg =
      "An error in the configuration has occurred (it should be the last message \
you received prior to this one).  The network will not run until this is fixed. \
In addition, the configuration process may not be complete, so you should run this \
function again after you have corrected the source of the error.";
  }
  else {
    msg = 
    "Configuration is now complete.  Do not forget the two remaining things \
you need to do manually:\n\n" + man_msg;
  }
  taMisc::Choice(msg,"Ok");

  nacsp->scalar.min = -0.25;
  nacsp->scalar.max = 2.25;
  nacsp->rew_type = RewPredLayerSpec::OUT_ERR_REW;
  nacsp->pred_range.min = .1;
  nacsp->pred_range.max = 2;
  nacsp->rew.gain = .5;
  nacsp->UpdateAfterEdit();
  
  if(make_patch_snc) {
    matrixsp->gate.no_patch = false;
    matrixsp->gate.snc_gain = 1.0;
  }
  else {
    matrixsp->gate.no_patch = true;
  }
  matrixsp->UpdateAfterEdit();

  winbMisc::DelayedMenuUpdate(net);
  winbMisc::DelayedMenuUpdate(proj);


  SelectEdit* edit = pdpMisc::FindSelectEdit(proj);
  if(edit != NULL) {
    pfcsp->SelectForEditNm("maint", edit, "pfc");
    pfcsp->SelectForEditNm("act_reg", edit, "pfc");
    matrixsp->SelectForEditNm("max_maint_dur", edit, "matrix");
    matrixsp->SelectForEditNm("gate", edit, "matrix");
    nacsp->SelectForEditNm("rew", edit, "nac");
    rewpred_cons->SelectForEditNm("lrate", edit, "rewpred");
    matrix_cons->SelectForEditNm("lrate", edit, "matrix");
  }
}

