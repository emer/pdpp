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

#include "oldlba.h"
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
  err_sb = true;
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
  wt_sig_fun.UpdateAfterEdit();

  wt_sig_fun_inv.x_range.min = 0.0f;
  wt_sig_fun_inv.x_range.max = 1.0f;
  wt_sig_fun_inv.res = 1.0e-5f;	// 1e-6 = 1.9Mb & 33% slower!, but 4x more accurate; 1e-5 = .19Mb
  wt_sig_fun_inv.UpdateAfterEdit();

  wt_sig_fun_lst.off = -1;   wt_sig_fun_lst.gain = -1; // trigger an update
  wt_sig_fun_res = -1.0;

  rnd.mean = .5f;
  rnd.var = .25f;
  lrate = .01f;
  cur_lrate = .01f;
  lrs_value = EPOCH;
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

void LeabraConSpec::SetCurLrate(int epoch, LeabraTrial* trl) {
  cur_lrate = lrate;		// as a backup..
  if(lrs_value == NO_LRS) return;

  if(lrs_value == EXT_REW_AVG) {
    LeabraLayer* er_lay = LeabraLayerSpec::FindLayerFmSpecNet(trl->network, &TA_ExtRewLayerSpec);
    if(er_lay != NULL) {
      LeabraUnit* un = (LeabraUnit*)er_lay->units.Leaf(0);
      float avg_rew = un->act_avg;
      int ar_pct = (int)(100.0f * avg_rew);
      cur_lrate = lrate * lrate_sched.GetVal(ar_pct);
      return;
    }
    else {
      taMisc::Error("*** Warning: LeabraConSpec::SetCurLrate(): appropriate ExtRew layer not found for EXT_REW_AVG, reverting to EPOCH! for:",
		    name);
      SetUnique("lrs_value", true);
      lrs_value = EPOCH;
      UpdateAfterEdit();
    }
  }

  if(lrs_value == EXT_REW_STAT) {
    int arval = 0;
    ExtRew_Stat* ars = NULL;
    if(epoch >= 1) {
      EpochProcess* epc = trl->GetMyEpochProc();
      if((epc != NULL) && (epc->super_proc != NULL)) {
	SchedProcess* sp = epc->super_proc;
	ars = (ExtRew_Stat*)sp->loop_stats.FindType(&TA_ExtRew_Stat);
	if(ars != NULL) {
	  if(ars->time_agg.op != Aggregate::LAST) {
	    ars->time_agg.op = Aggregate::LAST;
	  }
	  arval = (int)(100.0f * ars->rew.val);
	}
      }
      if((epc != NULL) && (ars == NULL) && (epc->wt_update == EpochProcess::TEST)) {
	// do nothing; it is a test process and its ok.
	arval = lrate_sched.last_ctr;
      }
      else {
	if(ars == NULL) {
	  if(epc == NULL) {
	    taMisc::Error("*** Warning: LeabraConSpec::SetCurLrate(): appropriate EpochProc for ExtRew_Stat not found for EXT_REW_STAT, reverting to EPOCH!, for:",
			  name);
	  }
	  else {
	    taMisc::Error("*** Warning: LeabraConSpec::SetCurLrate(): appropriate ExtRew_Stat not found for EXT_REW_STAT, reverting to EPOCH!, for:",
			  name);
	  }
	  SetUnique("lrs_value", true);
	  lrs_value = EPOCH;
	  UpdateAfterEdit();
	}
      }
    }
    cur_lrate = lrate * lrate_sched.GetVal(arval);
  }

  if(lrs_value == SE_STAT) {
    int seval = 0;
    LeabraSE_Stat* ses = NULL;
    if(lrate_sched.size > 0) {
      seval = ((SchedItem*)lrate_sched.Peek())->start_ctr + 1; // get the last one
    }
    if(epoch >= 1) {
      EpochProcess* epc = trl->GetMyEpochProc();
      if((epc != NULL) && (epc->super_proc != NULL)) {
	SchedProcess* sp = epc->super_proc;
	LeabraSE_Stat* ses = (LeabraSE_Stat*)sp->loop_stats.FindType(&TA_LeabraSE_Stat);
	if(ses != NULL) {
	  if(ses->time_agg.op != Aggregate::LAST) {
	    ses->time_agg.op = Aggregate::LAST;
	  }
	  seval = (int)ses->se.val;
	}
      }
      if((epc != NULL) && (ses == NULL) && (epc->wt_update == EpochProcess::TEST)) {
	// do nothing; it is a test process and its ok.
	seval = lrate_sched.last_ctr;
      }
      else {
	if(ses == NULL) {
	  taMisc::Error("*** Warning: LeabraConSpec::SetCurLrate(): appropriate LeabraSE_Stat not found for SE_STAT, reverting to EPOCH!, for:",
			name);
	  SetUnique("lrs_value", true);
	  lrs_value = EPOCH;
	  UpdateAfterEdit();
	}
      }
    }
    cur_lrate = lrate * lrate_sched.GetVal(seval);
  }

  if(lrs_value == EPOCH) {
    cur_lrate = lrate * lrate_sched.GetVal(epoch);
  }
}

void LeabraConSpec::CreateWtSigFun() {
  if((wt_sig_fun_lst.gain == wt_sig.gain) && (wt_sig_fun_lst.off == wt_sig.off)
     && (wt_sig_fun_res == wt_sig_fun.res))
    return;
  wt_sig_fun.AllocForRange();
  int i;
  for(i=0; i<wt_sig_fun.size; i++) {
    float w = wt_sig_fun.Xval(i);
    wt_sig_fun[i] = WtSigSpec::SigFun(w, wt_sig.gain, wt_sig.off);
  }
  wt_sig_fun_inv.AllocForRange();
  for(i=0; i<wt_sig_fun_inv.size; i++) {
    float w = wt_sig_fun_inv.Xval(i);
    wt_sig_fun_inv[i] = WtSigSpec::SigFunInv(w, wt_sig.gain, wt_sig.off);
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
  init = false;
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
  min = 0.0f;
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
  g_bar.h = 0.01f;
  g_bar.a = 0.03f;
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
  noise_conv.UpdateAfterEdit();

  nxx1_fun.x_range.min = -.03f;
  nxx1_fun.x_range.max = .20f;
  nxx1_fun.res = .001f;
  nxx1_fun.UpdateAfterEdit();

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
    if(!quiet) taMisc::Error("LeabraUnitSpec Warning: cannot use opt_thresh.updt_wts when wt_update is not ON_LINE",
			     "I turned this flag off for you in LeabraUnitSpec:", name);
    SetUnique("opt_thresh", true);
    opt_thresh.updt_wts = false;
  }

  if(opt_thresh.updt_wts && act_reg.on && (act_reg.min > 0.0f)) {
    if(!quiet) taMisc::Error("LeabraUnitSpec Warning: cannot use opt_thresh.updt_wts when act_reg is on and min > 0",
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
    noise_conv[i] = expf(-((x * x) / var));
  }

  // normalize it
  float sum = noise_conv.Sum();
  for(i=0; i < noise_conv.size; i++)
    noise_conv[i] /= sum;

  // then create the initial function
  FunLookup fun;
  fun.x_range.min = nxx1_fun.x_range.min + noise_conv.x_range.min;
  fun.x_range.max = nxx1_fun.x_range.max + noise_conv.x_range.max;
  fun.res = nxx1_fun.res;
  fun.UpdateAfterEdit();
  fun.AllocForRange();

  for(i=0; i<fun.size; i++) {
    float x = fun.Xval(i);
    float val = 0.0f;
    if(x > 0.0f)
      val = (act.gain * x) / ((act.gain * x) + 1.0f);
    fun[i] = val;
  }

  nxx1_fun.Convolve(fun, noise_conv);
}

void LeabraUnitSpec::InitWtState(Unit* u) {
  UnitSpec::InitWtState(u);
  LeabraUnit* lu = (LeabraUnit*)u;
  lu->act_avg = .5 * (act_reg.max + MAX(act_reg.min, 0.0f));
  lu->misc_1 = 0.0f;
  if(act_fun != DEPRESS)
    lu->spk_amp = 0.0f;
  lu->vcb.hyst = lu->vcb.g_h = 0.0f;
  lu->vcb.hyst_on = false;
  lu->vcb.acc = lu->vcb.g_a = 0.0f;
  lu->vcb.acc_on = false;
  phase_delta = 0.0f;
}

void LeabraUnitSpec::SetCurLrate(LeabraUnit* u, LeabraTrial* trl, int epoch) {
  ((LeabraConSpec*)bias_spec.spec)->SetCurLrate(epoch, trl);
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g)
    recv_gp->SetCurLrate(epoch, trl);
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

// NOTE: these two functions should always be the same modulo the clamp_range.Clip

void LeabraUnitSpec::Compute_HardClampNoClip(LeabraUnit* u, LeabraLayer* lay, LeabraTrial*) {
  u->net = u->prv_net = u->ext * lay->stm_gain;
  //  u->act_eq = clamp_range.Clip(u->ext);
  u->act_eq = u->ext;
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

float LeabraUnitSpec::Compute_IThreshNoAH(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  float non_bias_net = u->net;
  if(u->bias != NULL)		// subtract out bias weights so they can change k
    non_bias_net -= u->bias_scale * u->bias->wt;

  // NOT including the ga and gh terms
  return ((non_bias_net * (e_rev.e - act.thr) + u->gc.l * (e_rev.l - act.thr)) /
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
    if(act.avg_dt > 0.0f)
      u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
  }
  else {
    if(trl->phase == LeabraTrial::MINUS_PHASE)
      u->act_m = u->act_eq;
    else if((trl->phase == LeabraTrial::PLUS_PHASE) && (trl->phase_no < 2)) {
      // act_p is only for first plus phase: others require something else
      u->act_p = u->act_eq;
      if(phase_delta != 0.0f)
	u->act_p = act_range.Clip(u->act_p * (1.0f + phase_delta));
      if(act.avg_dt > 0.0f)
	u->act_avg += act.avg_dt * (u->act_eq - u->act_avg);
      u->act_dif = u->act_p - u->act_m;
    }
  }
}

//////////////////////////////////////////
//	Stage 6: Learning 		//
//////////////////////////////////////////

void LeabraUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if((u->act_p <= opt_thresh.learn) && (u->act_m <= opt_thresh.learn))
    return;
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay, trl);
}

void LeabraUnitSpec::Compute_dWt_impl(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  // don't adapt bias weights on clamped inputs..: why?  what possible consequence could it have!?
  // furthermore: it is not right for units that are clamped in 2nd plus phase!
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
  // see above commment
  //  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
  ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu, this);
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
  diff_act_pct = false;
  act_pct = .1;
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
  if(lay->units.gp.size > 0) {
    int gi;
    for(gi=0;gi<lay->units.gp.size;gi++) {
      LeabraUnit_Group* gp = (LeabraUnit_Group*)lay->units.gp[gi];
      gp->misc_state = gp->misc_state1 = gp->misc_state2 = 0;
    }
  }
  InitInhib(lay);		// initialize inhibition at start..
}

void LeabraLayerSpec::SetCurLrate(LeabraLayer* lay, LeabraTrial* trl, int epoch) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->SetCurLrate(trl, epoch);
}

LeabraLayer* LeabraLayerSpec::FindLayerFmSpec(LeabraLayer* lay, int& prjn_idx, TypeDef* layer_spec) {
  LeabraLayer* rval = NULL;
  prjn_idx = -1;
  Projection* p;
  taLeafItr pi;
  FOR_ITR_EL(Projection, p, lay->projections., pi) {
    LeabraLayer* fmlay = (LeabraLayer*)p->from;
    if(fmlay->spec.spec->InheritsFrom(layer_spec)) {	// inherits - not excact match!
      prjn_idx = p->recv_idx;
      rval = fmlay;
      break;
    }
  }
  return rval;
}

LeabraLayer* LeabraLayerSpec::FindLayerFmSpecExact(LeabraLayer* lay, int& prjn_idx, TypeDef* layer_spec) {
  LeabraLayer* rval = NULL;
  prjn_idx = -1;
  Projection* p;
  taLeafItr pi;
  FOR_ITR_EL(Projection, p, lay->projections., pi) {
    LeabraLayer* fmlay = (LeabraLayer*)p->from;
    if(fmlay->spec.spec->GetTypeDef() == layer_spec) {	// not inherits - excact match!
      prjn_idx = p->recv_idx;
      rval = fmlay;
      break;
    }
  }
  return rval;
}

LeabraLayer* LeabraLayerSpec::FindLayerFmSpecNet(Network* net, TypeDef* layer_spec) {
  LeabraLayer* lay;
  taLeafItr li;
  FOR_ITR_EL(LeabraLayer, lay, net->layers., li) {
    if(lay->spec.spec->InheritsFrom(layer_spec)) {	// inherits - not excact match!
      return lay;
    }
  }
  return NULL;
}

//////////////////////////////////////////
//	Stage 0: at start of settling	// 
//////////////////////////////////////////

void LeabraLayerSpec::Compute_Active_K(LeabraLayer* lay) {
  if((inhib_group != ENTIRE_LAYER) && (lay->units.gp.size > 0)) {
    int totk = 0;
    int g;
    for(g=0; g<lay->units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	if(gp_kwta.diff_act_pct)
	  lay->kwta.pct = gp_kwta.act_pct;	// override!!
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
    LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[0];
    lay->kwta.pct = rugp->kwta.pct * ((float)lay->kwta.k / (float)lay->units.gp.size);
    if(kwta.diff_act_pct)
      lay->kwta.pct = kwta.act_pct;	// override!!
    lay->kwta.pct_c = 1.0f - lay->kwta.pct;
  }
  else if(inhib_group != UNIT_GROUPS) {
    Compute_Active_K_impl(lay, &(lay->units), (LeabraInhib*)lay, kwta);
    if(kwta.diff_act_pct)
      lay->kwta.pct = kwta.act_pct;	// override!!
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
      rugp->Inhib_InitState(this);
    }
  }
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->InitState(lay);
}

void LeabraLayerSpec::Compute_HardClampPhase2(LeabraLayer* lay, LeabraTrial* trl) {
  // special case to speed up processing by clamping 2nd plus phase to prior + phase
  lay->SetExtFlag(Unit::EXT);
  lay->hard_clamped = true;
  lay->Inhib_SetVals(i_kwta_pt);		// assume 0 - 1 clamped inputs

  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->SetExtFlag(Unit::EXT);
    u->ext = u->act_p;
    u->Compute_HardClampNoClip(lay, trl); // important: uses no clip here: not really external values!
  }
  Compute_ActAvg(lay, trl);
}

void LeabraLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(decay.clamp_phase2 && (trl->phase != LeabraTrial::MINUS_PHASE) && (trl->phase_no >= 2)) {
    Compute_HardClampPhase2(lay, trl);
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	  LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	  LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      thr->inact_buf.Add(thr->active_buf[k1_idx]); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf[j]->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf[j]->i_thr;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf[j];
      if(u->i_thr <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf[k1_idx]);	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf[i]->i_thr;
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
    float tmp = thr->active_buf[j]->i_thr;
    if((tmp < net_k) && (j != k1_idx)) {
      net_k = tmp;		k_idx = j;
    }
  }
  if(k_idx == -1) {		// we didn't find the next one
    k_idx = k1_idx;
    net_k = net_k1;
  }

  LeabraUnit* k1_u = (LeabraUnit*)thr->active_buf[k1_idx];
  LeabraUnit* k_u = (LeabraUnit*)thr->active_buf[k_idx];

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
      thr->inact_buf.Add(thr->active_buf[k1_idx]); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf[j]->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf[j]->i_thr;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf[j];
      if(u->i_thr <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf[k1_idx]);	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->i_thr;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf[i]->i_thr;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k most active units, get averages of both groups
  float k_avg = 0.0f;
  for(j=0; j < k_plus_1; j++)
    k_avg += thr->active_buf[j]->i_thr;
  k_avg /= (float)k_plus_1;

  float oth_avg = 0.0f;
  for(j=0; j < thr->inact_buf.size; j++)
    oth_avg += thr->inact_buf[j]->i_thr;
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
      u = (LeabraUnit_Group*)lay->units.gp[i], 
      lay->active_buf.Add((LeabraUnit*)u);		// add unit to the list
      if(u->i_val.g_i < net_k1) {
	net_k1 = u->i_val.g_i;	k1_idx = i;
      }
    }
    lay->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; i<lay->units.gp.size; i++) {
      u = (LeabraUnit_Group*)lay->units.gp[i];
      if(u->i_val.g_i <=  net_k1) {	// not bigger than smallest one in sort buffer
	lay->inact_buf.Add((LeabraUnit*)u);
	continue;
      }
      lay->inact_buf.Add(lay->active_buf[k1_idx]); // now inactive
      lay->active_buf.Replace(k1_idx, (LeabraUnit*)u);// replace the smallest with it
      net_k1 = u->i_val.g_i;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = ((LeabraUnit_Group*)lay->active_buf[j])->i_val.g_i;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = ((LeabraUnit_Group*)lay->active_buf[j])->i_val.g_i;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < lay->inact_buf.size; j++) {
      u = (LeabraUnit_Group*)lay->inact_buf[j];
      if(u->i_val.g_i <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      lay->inact_buf.Replace(j, lay->active_buf[k1_idx]);	// now inactive
      lay->active_buf.Replace(k1_idx, (LeabraUnit*)u);// replace the smallest with it
      net_k1 = u->i_val.g_i;		// assume its the smallest
      int i;
      for(i=0; i < k_plus_1; i++) { 	// and recompute the actual smallest
	float tmp = ((LeabraUnit_Group*)lay->active_buf[i])->i_val.g_i;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = i;
	}
      }
    }
  }

  // active_buf now has k most active groups - go through groups and set large inhib in 
  // inactive ones!
  for(j=0; j < lay->inact_buf.size; j++) {
    u = (LeabraUnit_Group*)lay->inact_buf[j];
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
      llk = (LayerLink*)lay->layer_links[i];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
	LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[g];
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
  prv_phase = LeabraTrial::MINUS_PHASE;
  dav = 0.0f;
  da_updt = false;
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
  taBase::Own(misc_iar, this);
  spec.SetDefaultSpec(this);
  units.gp.SetBaseType(&TA_LeabraUnit_Group);
}

void LeabraLayer::CutLinks() {
  Layer::CutLinks();
  spec.CutLinks();
  layer_links.Reset();		// get rid of the links...
  misc_iar.CutLinks();
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
  misc_iar = cp.misc_iar;
}

void LeabraLayer::ResetSortBuf() {
  Inhib_ResetSortBuf();		// reset sort buf after any edit..
  if(units.gp.size > 0) {
    int g;
    for(g=0; g<units.gp.size; g++) {
      LeabraUnit_Group* rugp = (LeabraUnit_Group*)units.gp[g];
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
  misc_state1 = 0;
  misc_state2 = 0;
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
  misc_state1 = cp.misc_state1;
  misc_state2 = cp.misc_state2;
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

void LeabraSettle::Compute_dWt_NStdLay() {
  leabra_trial->Compute_dWt_NStdLay();	// fctn is complicated, use trial version..
}

void LeabraSettle::Compute_dWt() {
  leabra_trial->Compute_dWt();	// fctn is complicated, use trial version..
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

  if(leabra_trial->phase_no >= 3) { // second plus phase or more: use phase2..
    DecayPhase2();
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
  else if(leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_2) {
    if(leabra_trial->phase_no == 1)
      leabra_trial->Compute_dWt();
    else if(leabra_trial->phase_no == 2)
      Compute_dWt_NStdLay();	// only do non-standard ones
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
      else if(phase_order == MINUS_PLUS_2) {
	phase_no.SetMax(4);
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
	else if(phase_order == MINUS_PLUS_PLUS)
	  phase = PLUS_PHASE;
	else if(phase_order == MINUS_PLUS_2) {
	  if(phase_no == 2) phase = MINUS_2;
	  else phase = PLUS_2;
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
    else if(phase_order == MINUS_PLUS_2)
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
  DataItem* it = (DataItem*)cntr_items[1];
  it->SetNarrowName("Phase");
  it->is_string = true;
}

void LeabraTrial::GenCntrLog(LogData* ld, bool gen) {
  TrialProcess::GenCntrLog(ld, gen);
  if(gen) {
    if(cntr_items.size < 2)
      GetCntrDataItems();
    if(phase == MINUS_PHASE)
      ld->AddString(cntr_items[1], "-");
    else if(phase == PLUS_PHASE)
      ld->AddString(cntr_items[1], "+");
    else if(phase == MINUS_2)
      ld->AddString(cntr_items[1], "2-");
    else if(phase == PLUS_2)
      ld->AddString(cntr_items[1], "2+");
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
  net_agg.ComputeAgg(&da, fda);
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

////////////////////////////////
// 	LeabraSharpStat        //
////////////////////////////////

void LeabraSharpStat::Initialize() {
  min_layer = &TA_LeabraLayer;
  net_agg.op = Aggregate::AVG;
}

void LeabraSharpStat::Destroy() {
}

void LeabraSharpStat::Copy_(const LeabraSharpStat& cp) {
  sharp = cp.sharp;
}

void LeabraSharpStat::InitStat() {
  sharp.InitStat(InitStatVal());
  InitStat_impl();
}

void LeabraSharpStat::Init() {
  sharp.Init();
  Init_impl();
}

bool LeabraSharpStat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  if(sharp.Crit())	return true;
  return false;
}

void LeabraSharpStat::Network_Init() {
  InitStat();
}

void LeabraSharpStat::Layer_Stat(Layer* lay) {
  LeabraLayer* llay = (LeabraLayer*)lay;

  float val = 0.0f;
  if(llay->acts.avg > 0.0f)
    val = llay->acts.max / llay->acts.avg;

  net_agg.ComputeAgg(sharp.val, val);
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
    lay->dav = td;

    LeabraUnitSpec* us = (LeabraUnitSpec*)un_gain_spec.spec;
    if(mod_type == MOD_GAIN) {
      if(trl->phase_no == 2) { // 2nd plus phase
	// map full -1, +1 range of td onto un_gain_range
	float gain = un_gain_range.MidPoint() + (.5f * un_gain_range.Range() * td);
	UpdateUnitGain(us, gain);
      }
      else {
	UpdateUnitGain(us, un_gain_range.MidPoint()); // reset gain!
      }
    }
    else if(mod_type == MOD_ERR) {
      if(trl->phase_no == 2) { // 2nd plus phase
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
  else if(trl->phase_no == 1) {
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
    lay->dav = td;		// store this baby for later..

    lay->da_updt = false;	// decide based on td if updating will occur
    // stochastically update current state with probability = abs(td)
    // todo: actually want to pass this through a sigmoid function, but use a threshold of .5 for now:
    if(lay->dav >= 0.0f) {
      if((lay->dav >= gate_noise.pos_thr) || (Random::ZeroOne() <= lay->dav))
	lay->da_updt = true;
    }
    else {
      if((-lay->dav >= gate_noise.neg_thr) || (Random::ZeroOne() <= -lay->dav))
	lay->da_updt = true;
    }

    if(lay->da_updt) {		// initialize prior maintenance stuff if updating!
      LeabraUnit* u;
      taLeafItr i;
      FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
	u->DecayPhase(lay, trl, fabs(lay->dav)); // clear out in proportion to da
	u->vcb.g_h -= fabs(lay->dav) * u->vcb.g_h;
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
  if((gate.maint == GateSpec::INTRACELL) && (lay->dav >= 0.0f) && lay->da_updt) {
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
  FindLayerFmSpec(lay, ac_prjn_idx, &TA_LeabraACLayerSpec);
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
  FindLayerFmSpec(lay, ac_prjn_idx, &TA_LeabraACLayerSpec);
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
  opt_thresh.updt_wts = false;
}

bool LeabraTimeUnitSpec::CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet) {
  LeabraUnitSpec::CheckConfig(un, lay, tp, quiet);

  if((opt_thresh.learn >= 0.0f) || opt_thresh.updt_wts) {
    if(!quiet)
      taMisc::Error("LeabraTimeUnitSpec: opt_thresh.learn must be -1 and updt_wts = false to allow proper recording of p_savg and p_max_cor values in connections",
		    "even when this unit is not active on a prior time step!  I just set it for you.");
    SetUnique("opt_thresh", true);
    opt_thresh.learn = -1.0f;
    opt_thresh.updt_wts = false;
  }
  return true;
}

void LeabraTimeUnitSpec::InitState(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  LeabraTimeUnit* lu = (LeabraTimeUnit*)u;
  lu->p_act_m = -.01f;
  lu->p_act_p = -.01f;
}

void LeabraTimeUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
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
    ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu, this);
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
  p_act_m = -.01f;
  p_act_p = -.01f;
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
// 	TrialSynDepConSpec	//
//////////////////////////////////

void SynDepSpec::Initialize() {
  rec = 1.0f;
  depl = 1.1f;
}

void TrialSynDepConSpec::Initialize() {
  min_con_type = &TA_TrialSynDepCon;
}

void TrialSynDepConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(syn_dep, this);
}

void TrialSynDepConSpec::UpdateAfterEdit() {
  LeabraConSpec::UpdateAfterEdit();
  if(syn_dep.rec <= 0.0f)	// can't go to zero!
    syn_dep.rec = 1.0f;
}

//////////////////////////////////
// 	FastWtConSpec		//
//////////////////////////////////

void FastWtSpec::Initialize() {
  lrate = .05f;
  use_lrs = false;
  cur_lrate = .05f;
  decay = 1.0f;
  slw_sat = true;
  dk_mode = SU_THR;
}

void FastWtConSpec::Initialize() {
  min_con_type = &TA_FastWtCon;
}

void FastWtConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(fast_wt, this);
}

void FastWtConSpec::SetCurLrate(int epoch, LeabraTrial* trl) {
  LeabraConSpec::SetCurLrate(epoch, trl);
  if(fast_wt.use_lrs)
    fast_wt.cur_lrate = fast_wt.lrate * lrate_sched.GetVal(epoch);
  else
    fast_wt.cur_lrate = fast_wt.lrate;
}

//////////////////////////////////
// 	NewHebbConSpec		//
//////////////////////////////////

void LeabraNewHebbConSpec::Initialize() {
  hebb_fun = CPCA_ORIG;
  fixed_wt = .5f;
  sinxy_freq = 3.5f;
  subsu_thr = .5f;
  subsu_gain = 1.0f;
}

void LeabraNewHebbConSpec::Defaults() {
  LeabraConSpec::Defaults();
  Initialize();
}

void LeabraNewHebbConSpec::GraphHebb(GraphLog* graph_log, float wt_val, float savg_act) {
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
  graph_log->name = "Graph Hebbian dWt: " + name;
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("post");
  dt->NewColFloat("dwt");
  dt->AddColDispOpt("MIN=-1", 1);
  dt->AddColDispOpt("MAX=1", 1);

  LeabraCon cn; cn.wt = -wt_val; // neg!
  LeabraCon_Group cg; cg.savg = savg_act;
  float savg = .5f + savg_cor.cor * (cg.savg - .5f);
  savg = MAX(savg_cor.thresh, savg); // keep this computed value within bounds
  cg.max_cor = .5f / savg;

  float pre, post;
  for(pre = 0.0f; pre < 1.0f; pre += .01f) {
    for(post=0.0f; post < 1.0f; post += .01f) {
      float dwt = C_Compute_Hebb(&cn, &cg, post, pre);
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
// 	SigHebbConSpec		//
//////////////////////////////////

void LeabraSigHebbConSpec::Initialize() {
}

void LeabraSigHebbConSpec::Defaults() {
  LeabraConSpec::Defaults();
  Initialize();
}

void LeabraSigHebbConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  taBase::Own(pre0,this);	
  taBase::Own(pre1,this);	
  taBase::Own(pre2,this);	
  taBase::Own(pre3,this);	
  taBase::Own(pst0,this);	
  taBase::Own(pst1,this);	
  taBase::Own(pst2,this);	
  taBase::Own(pst3,this);	
  taBase::Own(prepst0,this);
  taBase::Own(prepst1,this);
  taBase::Own(prepst2,this);
  taBase::Own(prepst3,this);
  taBase::Own(ppdist0,this);
  taBase::Own(ppdist1,this);
  taBase::Own(ppdist2,this);
  taBase::Own(ppdist3,this);
}

void LeabraSigHebbConSpec::WriteToFile(ostream& strm) {
  pre0.WriteToFile(strm); strm << endl;
  pre1.WriteToFile(strm); strm << endl;
  pre2.WriteToFile(strm); strm << endl;
  pre3.WriteToFile(strm); strm << endl;
  pst0.WriteToFile(strm); strm << endl;
  pst1.WriteToFile(strm); strm << endl;
  pst2.WriteToFile(strm); strm << endl;
  pst3.WriteToFile(strm); strm << endl;
  prepst0.WriteToFile(strm); strm << endl;
  prepst1.WriteToFile(strm); strm << endl;
  prepst2.WriteToFile(strm); strm << endl;
  prepst3.WriteToFile(strm); strm << endl;
  ppdist0.WriteToFile(strm); strm << endl;
  ppdist1.WriteToFile(strm); strm << endl;
  ppdist2.WriteToFile(strm); strm << endl;
  ppdist3.WriteToFile(strm); strm << endl;
}

void LeabraSigHebbConSpec::ReadFromFile(istream& strm) {
  pre0.ReadFromFile(strm);
  pre1.ReadFromFile(strm);
  pre2.ReadFromFile(strm);
  pre3.ReadFromFile(strm);
  pst0.ReadFromFile(strm);
  pst1.ReadFromFile(strm);
  pst2.ReadFromFile(strm);
  pst3.ReadFromFile(strm);
  prepst0.ReadFromFile(strm);
  prepst1.ReadFromFile(strm);
  prepst2.ReadFromFile(strm);
  prepst3.ReadFromFile(strm);
  ppdist0.ReadFromFile(strm);
  ppdist1.ReadFromFile(strm);
  ppdist2.ReadFromFile(strm);
  ppdist3.ReadFromFile(strm);
}

void LeabraSigHebbConSpec::GraphHebb(GraphLog* graph_log, float wt_val, float savg_act) {
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
  graph_log->name = "Graph Hebbian dWt: " + name;
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("post");
  dt->NewColFloat("dwt");
  dt->AddColDispOpt("MIN=-1", 1);
  dt->AddColDispOpt("MAX=1", 1);

  LeabraCon cn; cn.wt = -wt_val; // neg!
  LeabraCon_Group cg; cg.savg = savg_act;
  float savg = .5f + savg_cor.cor * (cg.savg - .5f);
  savg = MAX(savg_cor.thresh, savg); // keep this computed value within bounds
  cg.max_cor = .5f / savg;

  float pre, post;
  for(pre = 0.0f; pre < 1.0f; pre += .01f) {
    for(post=0.0f; post < 1.0f; post += .01f) {
      float dwt = C_Compute_Hebb(&cn, &cg, post, pre);
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
  rep = GAUSSIAN;
  un_width = .3f;
  min_net = .1f;
  clamp_pat = false;
  min = val = sb_ev = 0.0f;
  range = incr = 1.0f;
  sb_lt = 0;
}

void ScalarValSpec::InitVal(float sval, int ugp_size, float umin, float urng) {
  min = umin; range = urng;
  val = sval;
  if(rep == GAUSSIAN)
    incr = range / (float)(ugp_size - 2); // skip 1st unit, and count end..
  else
    incr = range / (float)(ugp_size - 1); // skip 1st unit
  //  incr -= .000001f;		// round-off tolerance..
  sb_lt = (int)floor((val - min) / incr);
  sb_ev = (val - (min + ((float)sb_lt * incr))) / incr;
}

// rep 1.5.  ugp_size = 4, incr = 1.5 / 3 = .5
// 0  .5   1
// oooo111122222 = val / incr


float ScalarValSpec::GetUnitAct(int unit_idx) {
  int eff_idx = unit_idx - 1;
  if(rep == GAUSSIAN) {
    float cur = min + incr * (float)eff_idx;
    float dist = (cur - val) / un_width;
    return expf(-(dist * dist));
  }
  else {
    float rval;
    if(eff_idx < sb_lt) rval = 1.0f;
    else if(eff_idx > sb_lt) rval = 0.0f;
    else rval = sb_ev;
    return rval;
  }
}

float ScalarValSpec::GetUnitVal(int unit_idx) {
  int eff_idx = unit_idx - 1;
  float cur = min + incr * (float)eff_idx;
  return cur;
}

void ScalarValBias::Initialize() {
  un = NO_UN;
  un_shp = VAL;
  un_gain = 1.0f;
  wt = NO_WT;
  val = 0.0f;
  wt_gain = 1.0f;
}

void ScalarValLayerSpec::Initialize() {
  SetUnique("kwta", true);
  kwta.k_from = KWTASpec::USE_K;
  kwta.k = 8;
  SetUnique("inhib_group", true);
  inhib_group = ENTIRE_LAYER;
  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25;

  if(scalar.rep == ScalarValSpec::GAUSSIAN) {
    unit_range.min = -0.5f;   unit_range.max = 1.5f;
    unit_range.UpdateAfterEdit();
    val_range.min = unit_range.min + (.5f * scalar.un_width);
    val_range.max = unit_range.max - (.5f * scalar.un_width);
  }
  else {
    unit_range.min = 0.0f;   unit_range.max = 1.0f;
    unit_range.UpdateAfterEdit();
    val_range.min = unit_range.min;
    val_range.max = unit_range.max;
  }
  val_range.UpdateAfterEdit();
}

void ScalarValLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(scalar, this);
  taBase::Own(unit_range, this);
  taBase::Own(val_range, this);
  taBase::Own(bias_val, this);
}

void ScalarValLayerSpec::UpdateAfterEdit() {
  LeabraLayerSpec::UpdateAfterEdit();
  unit_range.UpdateAfterEdit();
  scalar.UpdateAfterEdit();
  if(scalar.rep == ScalarValSpec::GAUSSIAN) {
    val_range.min = unit_range.min + (.5f * scalar.un_width);
    val_range.max = unit_range.max - (.5f * scalar.un_width);
  }
  else {
    val_range.min = unit_range.min;
    val_range.max = unit_range.max;
  }
  val_range.UpdateAfterEdit();
  if(scalar.rep == ScalarValSpec::SUM_BAR) {
    compute_i = UNIT_INHIB;
  }
}

void ScalarValLayerSpec::HelpConfig() {
  String help = "ScalarValLayerSpec Computation:\n\
 Uses distributed coarse-coding units to represent a single scalar value.  Each unit\
 has a preferred value arranged evenly between the min-max range, and decoding\
 simply computes an activation-weighted average based on these preferred values.  The\
 current scalar value is displayed in the first unit in the layer, which can be clamped\
 and compared, etc (i.e., set the environment patterns to have just one unit and provide\
 the actual scalar value and it will automatically establish the appropriate distributed\
 representation in the rest of the units).  This first unit is only viewable as act_eq,\
 not act, because it must not send activation to other units.\n\
 \nScalarValLayerSpec Configuration:\n\
 - The bias_val settings allow you to specify a default initial and ongoing bias value\
 through a constant excitatory current (GC) or bias weights (BWT) to the unit, and initial\
 weight values.  These establish a distributed representation that represents the given .val\n\
 - A self connection using the ScalarValSelfPrjnSpec can be made, which provides a bias\
 for neighboring units to have similar values.  It should usually have a fairly small wt_scale.rel\
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

  if((scalar.rep == ScalarValSpec::SUM_BAR) && (compute_i != UNIT_INHIB)) {
    compute_i = UNIT_INHIB;
    if(!quiet) taMisc::Error("ScalarValLayerSpec: SUM_BAR rep type requires compute_i = UNIT_INHIB, because it sets gc.i individually");
  }

  if(bias_val.un == ScalarValBias::GC) {
    LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
    if(us->hyst.init) {
      us->SetUnique("hyst", true);
      us->hyst.init = false;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: bias_val.un = GCH requires UnitSpec hyst.init = false, I just set it for you in spec:",
			       us->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
    if(us->acc.init) {
      us->SetUnique("acc", true);
      us->acc.init = false;
      if(!quiet) taMisc::Error("ScalarValLayerSpec: bias_val.un = GC requires UnitSpec acc.init = false, I just set it for you in spec:",
			       us->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
  }

  // check for conspecs with correct params
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  if(u == NULL) {
    taMisc::Error("Error: ScalarValLayerSpec: scalar val layer doesn't have any units:", lay->name);
    return false;
  }
    
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
    else if(cs->InheritsFrom(TA_MarkerConSpec)) {
      continue;
    }
    else {
      if((scalar.rep == ScalarValSpec::SUM_BAR) && cs->lmix.err_sb) {
	cs->SetUnique("lmix", true);
	cs->lmix.err_sb = false;
	if(!quiet) taMisc::Error("ScalarValLayerSpec: scalar val cons for SUM_BAR should have lmix.err_sb = false (are otherwise biased!), I just set it for you in spec:",
		      cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
  }
  return true;
}

void ScalarValLayerSpec::ReConfig(Network* net, int n_units) {
  LeabraLayer* lay;
  taLeafItr li;
  FOR_ITR_EL(LeabraLayer, lay, net->layers., li) {
    if(lay->spec.spec != this) continue;
    
    if(n_units > 0) {
      lay->n_units = n_units;
      lay->geom.x = n_units;
    }

    LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
    LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
    
    if(scalar.rep == ScalarValSpec::SUM_BAR) {
      compute_i = UNIT_INHIB;
      bias_val.un = ScalarValBias::BWT;
      bias_val.un_shp = ScalarValBias::NEG_SLP;
      bias_val.wt = ScalarValBias::NO_WT;
      us->g_bar.h = .1f; us->g_bar.a = .1f;
      unit_range.min = 0.0; unit_range.max = 1.0f;

      LeabraCon_Group* recv_gp;
      int g;
      FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
	if((recv_gp->prjn == NULL) || (recv_gp->prjn->spec.spec == NULL)) continue;
	LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
	if(recv_gp->prjn->spec.spec->InheritsFrom(TA_ScalarValSelfPrjnSpec) ||
	   cs->InheritsFrom(TA_MarkerConSpec)) {
	  continue;
	}
	cs->lmix.err_sb = false;
	cs->rnd.mean = 0.1f;
      }
    }
    else {			// GAUSSIAN
      compute_i = KWTA_INHIB;
      us->g_bar.h = .015f; us->g_bar.a = .045f;
      bias_val.un = ScalarValBias::GC;
      bias_val.wt = ScalarValBias::WT;
      unit_range.min = -.5f; unit_range.max = 1.5f;

      LeabraCon_Group* recv_gp;
      int g;
      FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
	if((recv_gp->prjn == NULL) || (recv_gp->prjn->spec.spec == NULL)) continue;
	LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
	if(recv_gp->prjn->spec.spec->InheritsFrom(TA_ScalarValSelfPrjnSpec) ||
	   cs->InheritsFrom(TA_MarkerConSpec)) {
	  continue;
	}
	cs->lmix.err_sb = true;
	cs->rnd.mean = 0.1f;
      }
    }
    us->UpdateAfterEdit();
  }
  UpdateAfterEdit();
}

// todo: deal with lesion flag in lots of special purpose code like this!!!

void ScalarValLayerSpec::Compute_WtBias_Val(Unit_Group* ugp, float val) {
  if(ugp->size < 3) return;	// must be at least a few units..
  scalar.InitVal(val, ugp->size, unit_range.min, unit_range.range);
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    float act = .03f * bias_val.wt_gain * scalar.GetUnitAct(i);
    LeabraCon_Group* recv_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
      LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
      if(recv_gp->prjn->spec.spec->InheritsFrom(TA_ScalarValSelfPrjnSpec) ||
	 cs->InheritsFrom(TA_MarkerConSpec)) continue;
      int ci;
      for(ci=0;ci<recv_gp->size;ci++) {
	LeabraCon* cn = (LeabraCon*)recv_gp->Cn(ci);
	cn->wt += act;
	if(cn->wt < cs->wt_limits.min) cn->wt = cs->wt_limits.min;
	if(cn->wt > cs->wt_limits.max) cn->wt = cs->wt_limits.max;
	recv_gp->C_InitWtState_Post(cn, u, recv_gp->Un(ci));
      }
    }
  }
}

void ScalarValLayerSpec::Compute_UnBias_Val(Unit_Group* ugp, float val) {
  if(ugp->size < 3) return;	// must be at least a few units..
  scalar.InitVal(val, ugp->size, unit_range.min, unit_range.range);
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    float act = bias_val.un_gain * scalar.GetUnitAct(i);
    if(bias_val.un == ScalarValBias::GC)
      u->vcb.g_h = act;
    else if(bias_val.un == ScalarValBias::BWT)
      u->bias->wt = act;
  }
}

void ScalarValLayerSpec::Compute_UnBias_NegSlp(Unit_Group* ugp) {
  if(ugp->size < 3) return;	// must be at least a few units..
  float val = 0.0f;
  float incr = bias_val.un_gain / (float)(ugp->size - 2);
  int i;
  for(i=1;i<ugp->size;i++, val += incr) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    if(bias_val.un == ScalarValBias::GC)
      u->vcb.g_a = val;
    else if(bias_val.un == ScalarValBias::BWT)
      u->bias->wt = -val;
  }
}

void ScalarValLayerSpec::Compute_UnBias_PosSlp(Unit_Group* ugp) {
  if(ugp->size < 3) return;	// must be at least a few units..
  float val = bias_val.un_gain;
  float incr = bias_val.un_gain / (float)(ugp->size - 2);
  int i;
  for(i=1;i<ugp->size;i++, val -= incr) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    if(bias_val.un == ScalarValBias::GC)
      u->vcb.g_h = val;
    else if(bias_val.un == ScalarValBias::BWT)
      u->bias->wt = val;
  }
}

void ScalarValLayerSpec::Compute_BiasVal(LeabraLayer* lay) {
  if(bias_val.un != ScalarValBias::NO_UN) {
    if(bias_val.un_shp == ScalarValBias::VAL) {
      UNIT_GP_ITR(lay, Compute_UnBias_Val(ugp, bias_val.val););
    }
    else if(bias_val.un_shp == ScalarValBias::NEG_SLP) {
      UNIT_GP_ITR(lay, Compute_UnBias_NegSlp(ugp););
    }
    else if(bias_val.un_shp == ScalarValBias::POS_SLP) {
      UNIT_GP_ITR(lay, Compute_UnBias_PosSlp(ugp););
    }
  }
  if(bias_val.wt == ScalarValBias::WT) {
    UNIT_GP_ITR(lay, Compute_WtBias_Val(ugp, bias_val.val););
  }
}

void ScalarValLayerSpec::InitWtState(LeabraLayer* lay) {
  LeabraLayerSpec::InitWtState(lay);
  Compute_BiasVal(lay);
}

void ScalarValLayerSpec::Compute_NetScale(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraLayerSpec::Compute_NetScale(lay, trl);
  if(lay->hard_clamped) return;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    LeabraConSpec* bspec = (LeabraConSpec*)u->spec.spec->bias_spec.spec;
    u->clmp_net -= u->bias_scale * u->bias->wt;

    u->bias_scale = bspec->wt_scale.abs;  // still have absolute scaling if wanted..
    u->bias_scale /= 100.0f; 		  // keep a constant scaling so it doesn't depend on network size!
    u->clmp_net += u->bias_scale * u->bias->wt;
  }
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
  scalar.InitVal(val, ugp->size, unit_range.min, unit_range.range);
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    float act = rescale * scalar.GetUnitAct(i);
    if(act < us->opt_thresh.send)
      act = 0.0f;
    u->SetExtFlag(Unit::EXT);
    u->ext = act;
  }
}

float ScalarValLayerSpec::ClampAvgAct(int ugp_size) {
  if(ugp_size < 3) return 0.0f;
  float val = val_range.min + .5f * val_range.Range(); // half way
  scalar.InitVal(val, ugp_size, unit_range.min, unit_range.range);
  float sum = 0.0f;
  int i;
  for(i=1;i<ugp_size;i++) {
    float act = scalar.GetUnitAct(i);
    sum += act;
  }
  sum /= (float)(ugp_size - 1);
  return sum;
}

float ScalarValLayerSpec::ReadValue(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return 0.0f;	// must be at least a few units..

  scalar.InitVal(0.0f, ugp->size, unit_range.min, unit_range.range);
  float avg = 0.0f;
  float sum_act = 0.0f;
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
    float cur = scalar.GetUnitVal(i);
    float act_val = us->clamp_range.Clip(u->act_eq) / us->clamp_range.max; // clipped & normalized!
    avg += cur * act_val;
    sum_act += act_val;
  }
  if(sum_act > 0.0f)
    avg /= sum_act;
  // set the first unit in the group to represent the value
  LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
  if(scalar.rep == ScalarValSpec::GAUSSIAN)
    u->act_eq = avg;
  else
    u->act_eq = scalar.min + scalar.incr * sum_act;
  u->act = 0.0f;		// very important to clamp act to 0: don't send!
  u->da = 0.0f;			// don't contribute to change in act
  return u->act_eq;
}

void ScalarValLayerSpec::LabelUnits_impl(Unit_Group* ugp) {
  if(ugp->size < 3) return;	// must be at least a few units..
  scalar.InitVal(0.0f, ugp->size, unit_range.min, unit_range.range);
  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    float cur = scalar.GetUnitVal(i);
    u->name = (String)cur;
  }
}

void ScalarValLayerSpec::LabelUnits(LeabraLayer* lay) {
  UNIT_GP_ITR(lay, LabelUnits_impl(ugp); );
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
  UNIT_GP_ITR(lay, 
	      if(ugp->size > 2) {
		LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
		u->act = 0.0f;		// must reset so it doesn't contribute!
		u->act_eq = u->ext;	// avoid clamp_range!
	      }
	      );
}

void ScalarValLayerSpec::HardClampExt(LeabraLayer* lay, LeabraTrial* trl) {
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
  ResetAfterClamp(lay, trl);
}

void ScalarValLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(scalar.clamp_pat) {
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
    return;
  }
  if(!(clamp.hard && (lay->ext_flag & Unit::EXT))) {
    lay->hard_clamped = false;
    return;
  }

  UNIT_GP_ITR(lay, if(ugp->size > 2) { ClampValue(ugp, trl); } );
  HardClampExt(lay, trl);
}

void ScalarValLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
  LeabraLayerSpec::Compute_Act_impl(lay, ug, thr, trl);
  ReadValue(ug, trl);		// always read out the value
}

void ScalarValLayerSpec::Compute_Inhib_impl(LeabraLayer* lay, Unit_Group* ugp, LeabraInhib* thr, LeabraTrial* trl) {
  if(scalar.rep == ScalarValSpec::GAUSSIAN) {
    LeabraLayerSpec::Compute_Inhib_impl(lay, ugp, thr, trl);
    return;
  }
  thr->i_val.g_i = 0.0f;	// make sure it's zero, cuz this gets added to units.. 
  thr->i_val.g_i_orig = thr->i_val.g_i;	// retain original values..
  if(ugp->size < 3) return;	// must be at least a few units..

  int i;
  for(i=1;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    //    LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
    float old_net = u->net;
    u->net = MIN(scalar.min_net, u->net);
    float ithr = u->Compute_IThreshNoAH(lay, trl); // exclude AH here so they can be used as a modulator!
    if(ithr < 0.0f) ithr = 0.0f;
    u->net = old_net;
    u->gc.i = u->g_i_raw = ithr;
  }    
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
  UNIT_GP_ITR(lay, Connect_UnitGroup(ugp, prjn); );
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


/////////////////////////////////////////////////////////////////////////////////////
// 			PFC / BG code

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
// 	DaMod Units and Cons		//
//////////////////////////////////////////

void DaModUnit::Initialize() {
  act_m2 = 0.0f;
  act_p2 = 0.0f;
  p_act_m = -.01f;
  p_act_p = -.01f;
  dav = 0.0f;
}

void DaModUnit::Copy_(const DaModUnit& cp) {
  act_m2 = cp.act_m2;
  act_p2 = cp.act_p2;
  p_act_p = cp.p_act_p;
  p_act_m = cp.p_act_m;
  dav = cp.dav;
}

void DaModSpec::Initialize() {
  on = false;
  post = false;
  gain = 1.0f;
  p_dwt = false;
}

void DaModUnitSpec::Initialize() {
  min_obj_type = &TA_DaModUnit;
}

void DaModUnitSpec::InitLinks() {
  LeabraUnitSpec::InitLinks();
  taBase::Own(da_mod, this);
}

void DaModUnitSpec::Defaults() {
  LeabraUnitSpec::Defaults();
  Initialize();
}

bool DaModUnitSpec::CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet) {
  LeabraUnitSpec::CheckConfig(un, lay, tp, quiet);

  if(da_mod.p_dwt) {
    if((opt_thresh.learn >= 0.0f) || opt_thresh.updt_wts) {
      if(!quiet)
	taMisc::Error("DaModUnitSpec: for da_mod.p_dwt, opt_thresh.learn must be -1 and updt_wts = false to allow proper recording of p_savg and p_max_cor values in connections",
		      "even when this unit is not active on a prior time step!  I just set it for you.");
      SetUnique("opt_thresh", true);
      opt_thresh.learn = -1.0f;
      opt_thresh.updt_wts = false;
    }
  }
//   else {			// not p_dwt
//     if(opt_thresh.learn != 0.01f) {
//       if(!quiet)
// 	taMisc::Error("DaModUnitSpec: opt_thresh.learn should be .01f when da_mod.p_dwt is off",
// 		      "I just set it for you.");
//       SetUnique("opt_thresh", true);
//       opt_thresh.learn = 0.01f;
//     }
//   }
  return true;
}

void DaModUnitSpec::InitState(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  DaModUnit* lu = (DaModUnit*)u;
  lu->act_m2 = 0.0f;
  lu->act_p2 = 0.0f;
  lu->p_act_m = -.01f;
  lu->p_act_p = -.01f;
  lu->dav = 0.0f;
}

void DaModUnitSpec::Compute_Conduct(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr, LeabraTrial* trl) {
  if(da_mod.on && !da_mod.post) {
    // note: u->misc_1 contains maintenance currents in PFC units: g_h should always be set to this!
    DaModUnit* lu = (DaModUnit*)u;
    if(trl->phase == LeabraTrial::PLUS_PHASE) {
      if(lu->dav > 0.0f) {
	lu->vcb.g_a = 0.0f;
	lu->vcb.g_h = u->misc_1 + da_mod.gain * lu->dav * lu->act_m; // increase in proportion to participation in minus phase
      }
      else {
	lu->vcb.g_h = u->misc_1;
	lu->vcb.g_a = -da_mod.gain * lu->dav * lu->act_m; // decrease in proportion to participation in minus phase
      }
    }
    else {
      lu->vcb.g_h = u->misc_1;
      lu->vcb.g_a = 0.0f;	// clear in minus phase!
    }
  }

  LeabraUnitSpec::Compute_Conduct(u, lay, thr, trl);
}

void DaModUnitSpec::Compute_dWt(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl) {
  if(!da_mod.p_dwt) {
    LeabraUnitSpec::Compute_dWt(u, lay, trl);
    return;
  }
  DaModUnit* lu = (DaModUnit*)u;
  if((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) {
    if((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn))
      return;
  }
  if(lay->phase_dif_ratio < opt_thresh.phase_dif)
    return;
  Compute_dWt_impl(u, lay, trl);
}

void DaModUnitSpec::UpdateWeights(Unit* u) {
  if(!da_mod.p_dwt) {
    LeabraUnitSpec::UpdateWeights(u);
    return;
  }
  DaModUnit* lu = (DaModUnit*)u;
  ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu, this);
  if(opt_thresh.updt_wts && 
     ((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) &&
      ((lu->p_act_p <= opt_thresh.learn) && (lu->p_act_m <= opt_thresh.learn)))
    return;
  UnitSpec::UpdateWeights(lu);
}

void DaModUnitSpec::EncodeState(LeabraUnit* u, LeabraLayer*, LeabraTrial* trl) {
  DaModUnit* lu = (DaModUnit*)u;
  // just save phase activation states
  if(trl->phase_no.max >= 3)
    lu->p_act_p = lu->act_p2;
  else
    lu->p_act_p = lu->act_p;
  if(trl->phase_no.max >= 4)
    lu->p_act_m = lu->act_m2;
  else
    lu->p_act_m = lu->act_m;

  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* lay = (LeabraLayer*) recv_gp->prjn->from;
    if(lay->lesion)		continue;
    recv_gp->EncodeState(u);
  }
}

void DaModUnitSpec::DecayEvent(LeabraUnit* u, LeabraLayer* lay, LeabraTrial* trl, float decay) {
  LeabraUnitSpec::DecayEvent(u, lay, trl, decay);
  DaModUnit* lu = (DaModUnit*)u;
  lu->dav = 0.0f;
}

void DaModUnitSpec::PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			       LeabraTrial* trl, bool set_both)
{
  LeabraUnitSpec::PostSettle(u, lay, thr, trl, set_both);
  DaModUnit* lu = (DaModUnit*)u;

  if((trl->phase == LeabraTrial::MINUS_PHASE) && (trl->phase_no < 2)) {
    lu->act_m2 = lu->act_m;	// set this just in case..
  }
  if((trl->phase == LeabraTrial::PLUS_PHASE) && (trl->phase_no < 2)) {
    if(da_mod.on && da_mod.post) {
      float dact = da_mod.gain * lu->dav * lu->act_m; // delta activation
      if(dact > 0.0f) {
	dact *= 1.0f - lu->act_p;
      }
      else {
	dact *= lu->act_p;
      }
      lu->act_p = act_range.Clip(lu->act_p + dact);
      u->act_dif = u->act_p - u->act_m;
    }
    lu->act_p2 = lu->act_p;	// always set this just in case..
  }

  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no == 2))
    lu->act_p2 = lu->act_eq;
  else if(trl->phase_order == LeabraTrial::MINUS_PLUS_2) {
    if(trl->phase_no == 2)
      lu->act_m2 = lu->act_eq;
    else
      lu->act_p2 = lu->act_eq;
  }
}

//////////////////////////////////////////
//	Ext Rew Layer Spec		//
//////////////////////////////////////////

void AvgExtRewSpec::Initialize() {
  sub_avg = false;
  avg_dt = .005f;
}

void OutErrSpec::Initialize() {
  graded = false;
  no_off_err = false;
  seq_all_cor = false;
}

void ExtRewSpec::Initialize() {
  err_val = 0.0f;
  norew_val = 0.5f;
  rew_val = 1.0f;
}

void ExtRewLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = true;

  rew_type = OUT_ERR_REW;
}

void ExtRewLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  rew.Initialize();
  avg_rew.Initialize();
  out_err.Initialize();
  Initialize();
}

void ExtRewLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(rew, this);
  taBase::Own(avg_rew, this);
  taBase::Own(out_err, this);
}

void ExtRewLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  rew.UpdateAfterEdit();
  avg_rew.UpdateAfterEdit();
  out_err.UpdateAfterEdit();
}

void ExtRewLayerSpec::HelpConfig() {
  String help = "ExtRewLayerSpec Computation:\n\
 Computes external rewards based on network performance on an output layer or directly provided rewards.\n\
 - Minus phase = zero reward represented\n\
 - Plus phase = external reward value (computed at start of 1+) is clamped as distributed scalar-val representation.\n\
 - misc_1 on units stores the average reward value, computed using rew.avg_dt.\n\
 \nExtRewLayerSpec Configuration:\n\
 - OUT_ERR_REW: A recv connection from the output layer(s) where error is computed (marked with MarkerConSpec)\n\
 AND a MarkerConSpec from a layer called RewTarg that signals (>.5 act) when output errors count\n\
 - EXT_REW: external TARGET inputs to targ values deliver the reward value (e.g., input pattern or script)\n\
 - DA_REW: A recv connection or other means of setting da values = reward values.\n\
 - This layer must be before layers that depend on it in list of layers\n\
 \n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec\
 which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool ExtRewLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("ExtRewLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("ExtRewLayerSpec: must have DaModUnits!");
    return false;
  }

  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = true;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("ExtRewLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }
  if(us->act.avg_dt != 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.0f;
    if(!quiet) taMisc::Error("ExtRewLayerSpec: requires UnitSpec act.avg_dt = 0, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
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
  }
  if(!got_marker) {
    if(rew_type == DA_REW) {
      taMisc::Error("ExtRewLayerSpec: requires at least one recv MarkerConSpec connection from DA layer",
		    "to get reward based on performance.  This was not found -- please fix!");
      return false;
    }
    else if(rew_type == OUT_ERR_REW) {
      taMisc::Error("ExtRewLayerSpec: requires at least one recv MarkerConSpec connection from output/response layer(s) to compute",
		    "reward based on performance.  This was not found -- please fix!");
      return false;
    }
  }
  if(rew_type == OUT_ERR_REW) {
    if(rew_targ_lay == NULL) {
      taMisc::Error("ExtRewLayerSpec: requires a recv MarkerConSpec connection from layer called RewTarg",
		    "that signals (act > .5) when output error should be used for computing rewards.  This was not found -- please fix!");
      return false;
    }
    if(rew_targ_lay->units.size == 0) {
      taMisc::Error("ExtRewLayerSpec: RewTarg layer must have one unit (has zero) -- please fix!");
      return false;
    }
    int myidx = lay->own_net->layers.FindLeaf(lay);
    int rtidx = lay->own_net->layers.FindLeaf(rew_targ_lay);
    if(rtidx > myidx) {
      taMisc::Error("ExtRewLayerSpec: reward target (RewTarg) layer must be *before* this layer in list of layers -- it is now after, won't work");
      return false;
    }
  }
  return true;
}

void ExtRewLayerSpec::Compute_UnitDa(float er, DaModUnit* u, Unit_Group* ugp, LeabraLayer*, LeabraTrial* trl) {
  u->dav = er;
  if(avg_rew.sub_avg) u->dav -= u->misc_1;
  u->ext = u->dav;
  u->act_avg += avg_rew.avg_dt * (er - u->act_avg);
  ClampValue(ugp, trl);
}

bool ExtRewLayerSpec::OutErrRewAvail(LeabraLayer* lay, LeabraTrial*) {
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

float ExtRewLayerSpec::GetOutErrRew(LeabraLayer* lay, LeabraTrial*) {
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
    if(out_err.no_off_err) {
      totposs += rew_lay->kwta.k; // only on units can make errors
    }
    else {
      totposs += 2 * rew_lay->kwta.k; // both on and off units count
    }
    LeabraUnit* eu;
    taLeafItr i;
    FOR_ITR_EL(LeabraUnit, eu, rew_lay->units., i) {
      if(out_err.no_off_err) {
	if(!(eu->ext_flag & (Unit::TARG | Unit::COMP))) continue;
	if(eu->act_m > 0.5f) {	// was active
	  if(eu->targ < 0.5f)	// shouldn't have been
	    toterr += 1.0f;
	}
      }
      else {
	if(!(eu->ext_flag & (Unit::TARG | Unit::COMP))) continue;
	float tmp = fabsf(eu->act_m - eu->targ);
	float err = 0.0f;
	if(tmp >= 0.5f) err = 1.0f;
	toterr += err;
      }
    }
  }
  if(totposs == 0.0f)
    return -1.0f;		// -1 = no reward signal at all
  if(out_err.graded) {
    float nrmerr = toterr / totposs;
    if(nrmerr > 1.0f) nrmerr = 1.0f;
    return 1.0f - nrmerr;
  }
  if(toterr > 0.0f) return 0.0f; // 0 = wrong, 1 = correct
  return 1.0f;
}

void ExtRewLayerSpec::Compute_OutErrRew(LeabraLayer* lay, LeabraTrial* trl) {
  if(!OutErrRewAvail(lay, trl)) {
    Compute_NoRewAct(lay, trl);	
    return;
  }

  float er = 0.0f;
  if(out_err.seq_all_cor) {
    bool old_graded = out_err.graded;
    out_err.graded = false;		// prevent graded sig here!
    float itm_er = GetOutErrRew(lay, trl);
    out_err.graded = old_graded;

    lay->misc_iar.EnforceSize(3); // 0 = addr of eg; 1 = # tot; 2 = # cor
    Event_MGroup* eg = trl->GetMyCurEventGp();
    int eg_addr = (int)eg;
    if(lay->misc_iar[0] != eg_addr) { // new seq
      lay->misc_iar[0] = eg_addr;
      lay->misc_iar[1] = 1;
      lay->misc_iar[2] = (int)itm_er;
    }
    else {
      lay->misc_iar[1]++;
      lay->misc_iar[2] += (int)itm_er;
    }
    Event* ev = trl->GetMyCurEvent();
    int idx = eg->Find(ev);
    if(idx < eg->size-1) {	// not last event: no reward!
      Compute_NoRewAct(lay, trl);
      return;
    }
    er = (float)lay->misc_iar[2] / (float)lay->misc_iar[1];
    if(!out_err.graded && (er < 1.0f)) er = 0.0f; // didn't make it!
  }
  else {
    er = GetOutErrRew(lay, trl);
  }

  // starts out 0-1, transform into correct range
  er = (rew.rew_val - rew.err_val) * er + rew.err_val;

  UNIT_GP_ITR
    (lay,
     DaModUnit* u = (DaModUnit*)ugp->Leaf(0);
     u->misc_1 = 1.0f;		// indication of reward!
     Compute_UnitDa(er, u, ugp, lay, trl);
     );
}

void ExtRewLayerSpec::Compute_ExtRew(LeabraLayer* lay, LeabraTrial* trl) {
  if(!(lay->ext_flag & Unit::TARG)) {
    Compute_NoRewAct(lay, trl);	
    return;
  }    
  UNIT_GP_ITR
    (lay, 
     DaModUnit* u = (DaModUnit*)ugp->Leaf(0);
     u->misc_1 = 1.0f;		// indication of reward!
     float er = u->ext;
     Compute_UnitDa(er, u, ugp, lay, trl);
     );
}

void ExtRewLayerSpec::Compute_DaRew(LeabraLayer* lay, LeabraTrial* trl) {
  UNIT_GP_ITR
    (lay, 
     DaModUnit* u = (DaModUnit*)ugp->Leaf(0);
     float er = u->dav;
     Compute_UnitDa(er, u, ugp, lay, trl);
     );
}

void ExtRewLayerSpec::Compute_ZeroAct(LeabraLayer* lay, LeabraTrial* trl) {
  UNIT_GP_ITR
    (lay,
     LeabraUnit* u = (LeabraUnit*)ugp->Leaf(0);
     u->misc_1 = 0.0f;		// indication of no reward!
     u->ext = rew.norew_val;	// this is appropriate to set here..
     ClampValue(ugp, trl);
     );
}

void ExtRewLayerSpec::Compute_NoRewAct(LeabraLayer* lay, LeabraTrial* trl) {
  UNIT_GP_ITR
    (lay,
     LeabraUnit* u = (LeabraUnit*)ugp->Leaf(0);
     u->misc_1 = 0.0f;		// indication of no reward!
     u->ext = rew.norew_val;
     ClampValue(ugp, trl);
     );
}

void ExtRewLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no == 0) {
    lay->SetExtFlag(Unit::EXT);
    Compute_ZeroAct(lay, trl);	// zero reward in minus
    HardClampExt(lay, trl);
  }
  else if(trl->phase_no == 1) {
    lay->SetExtFlag(Unit::EXT);
    if(rew_type == OUT_ERR_REW)
      Compute_OutErrRew(lay, trl);
    else if(rew_type == EXT_REW)
      Compute_ExtRew(lay, trl);
    else if(rew_type == DA_REW)
      Compute_DaRew(lay, trl);
    HardClampExt(lay, trl);
  }
  else {
    // clamp to prior act_p value: will happen automatically
    LeabraLayerSpec::Compute_HardClamp(lay, trl);
  }
}

void ExtRewLayerSpec::Compute_dWt(LeabraLayer*, LeabraTrial*) {
  return;			// never compute dwts!
}


//////////////////////////////////
// 	ExtRew_Stat		//
//////////////////////////////////

void ExtRew_Stat::Initialize() {
}

void ExtRew_Stat::NameStatVals() {
  Stat::NameStatVals();
  rew.AddDispOption("MIN=0");
  rew.AddDispOption("TEXT");
}

void ExtRew_Stat::InitStat() {
  float init_val = InitStatVal();
  rew.InitStat(init_val);
  InitStat_impl();
}

void ExtRew_Stat::Init() {
  if(loop_init == NO_INIT) return;
  if(time_agg.from != NULL) {	// no init for real stats!!
    rew.Init();
    Init_impl();
  }
}

bool ExtRew_Stat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return rew.Crit();
}

void ExtRew_Stat::Network_Run() {
  if(network == NULL) return;
  LeabraLayer* er_lay = NULL;
  LeabraLayer* lay;
  taLeafItr i;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., i) {
    if(lay->lesion || !lay->spec.spec->InheritsFrom(&TA_ExtRewLayerSpec))
      continue;
    er_lay = lay;
    break;
  }
  if(er_lay == NULL) return;
  LeabraUnit* eru = (LeabraUnit*)er_lay->units.Leaf(0);
  if(eru->misc_1 == 0.0f) { // indication of no reward available
    rew.val = -1.1f;
  }
  else {
    rew.val = eru->act_eq;	// just set it!
  }
}

void ExtRew_Stat::ComputeAggregates() {
  if(time_agg.from == NULL)
    return;

  ExtRew_Stat* fms = (ExtRew_Stat*)time_agg.from;
  if(fms->rew.val == -1.1f) return; // don't agg no reward cases!!
  time_agg.ComputeAggNoUpdt(&rew, &(fms->rew));
  time_agg.IncUpdt();
}

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// 	Standard TD Reinforcement Learning 		//
//////////////////////////////////////////////////////////

void TDRewPredConSpec::Initialize() {
  SetUnique("lmix", true);
  lmix.hebb = 0.0f;
  lmix.err = 1.0f;

  SetUnique("rnd", true);
  rnd.mean = 0.1f;
  rnd.var = 0.0f;

  SetUnique("wt_limits", true);
  wt_limits.sym = false;
}

//////////////////////////////////////////
//	TD Rew Pred Layer Spec		//
//////////////////////////////////////////

void TDRewPredLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;
  unit_range.min = 0.0f;
  unit_range.max = 3.0f;
  unit_range.UpdateAfterEdit();
  val_range.UpdateAfterEdit();
}

void TDRewPredLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  Initialize();
}

void TDRewPredLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
}

void TDRewPredLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
}

void TDRewPredLayerSpec::HelpConfig() {
  String help = "TDRewPredLayerSpec Computation:\n\
 Computes expected rewards according to the TD algorithm: predicts V(t+1) at time t. \n\
 - Minus phase = previous expected reward V^(t) clamped\
 - Plus phase = free-running expected reward computed (over settlng, fm recv wts)\n\
 - Learning is (act_p - act_m) * p_act_p: delta on recv units times sender activations at (t-1).\n\
 \nTDRewPredLayerSpec Configuration:\n\
 - All units I recv from must be DaModUnit/Spec units (to hold t-1 act vals)\n\
 - Sending connection to a TDRewIntegLayerSpec to integrate predictions with external rewards";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool TDRewPredLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("TDRewPredLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(trl->no_plus_test) {
    if(!quiet) taMisc::Error("TDRewPredLayerSpec: requires LeabraTrial no_plus_test = false, I just set it for you");
    trl->no_plus_test = false;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("TDRewPredLayerSpec: must have DaModUnits!");
    return false;
  }

  SetUnique("decay", true);
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("TDRewPredLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }
  ((DaModUnitSpec*)us)->da_mod.p_dwt = true; // do need prior state dwt
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      continue;
    }
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(!cs->InheritsFrom(TA_TDRewPredConSpec)) {
      taMisc::Error("TDRewPredLayerSpec: requires recv connections to be of type TDRewPredConSpec");
      return false;
    }
    if(cs->wt_limits.sym != false) {
      cs->SetUnique("wt_limits", true);
      cs->wt_limits.sym = false;
      if(!quiet) taMisc::Error("TDRewPredLayerSpec: requires recv connections to have wt_limits.sym=false, I just set it for you in spec:",
		    cs->name,"(make sure this is appropriate for all layers that use this spec!)");
    }
  }
  return true;
}

void TDRewPredLayerSpec::InitState(LeabraLayer* lay) {
  ScalarValLayerSpec::InitState(lay);
  // initialize the misc_1 variable to 0.0 -- no prior predictions!
  UNIT_GP_ITR(lay, 
      LeabraUnit* u = (LeabraUnit*)ugp->Leaf(0);
      u->misc_1 = 0.0f;
	      );
}  

void TDRewPredLayerSpec::Compute_SavePred(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->misc_1 = u->act_eq;
  }
}

void TDRewPredLayerSpec::Compute_ClampPred(Unit_Group* ugp, LeabraTrial*) {
  if(ugp->size < 3) return;
  int i;
  for(i=0;i<ugp->size;i++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(i);
    u->ext = u->misc_1;
    u->SetExtFlag(Unit::EXT);
  }
}

void TDRewPredLayerSpec::Compute_ClampPrev(LeabraLayer* lay, LeabraTrial* trl) {
  UNIT_GP_ITR(lay, Compute_ClampPred(ugp, trl); );
}

void TDRewPredLayerSpec::Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial*) {
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

void TDRewPredLayerSpec::Compute_TdPlusPhase_impl(Unit_Group* ugp, LeabraTrial* trl) {
  Compute_SavePred(ugp, trl);	// first, always save current predictions!

  DaModUnit* u = (DaModUnit*)ugp->FastEl(0);
  u->ext = u->act_m + u->dav;
  ClampValue(ugp, trl);		// apply new value
  Compute_ExtToPlus(ugp, trl);	// copy ext values to act_p
}

void TDRewPredLayerSpec::Compute_TdPlusPhase(LeabraLayer* lay, LeabraTrial* trl) {
  UNIT_GP_ITR(lay, Compute_TdPlusPhase_impl(ugp, trl); );
}

void TDRewPredLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  ScalarValLayerSpec::PostSettle(lay, trl, set_both); 
  if(trl->phase_no.val < trl->phase_no.max-1)
    return; // only at very last phase, do this!  see note on Compute_dWt as to why..
  Compute_TdPlusPhase(lay, trl);
}

void TDRewPredLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no == 0) {
    lay->SetExtFlag(Unit::EXT);
    Compute_ClampPrev(lay, trl);
    HardClampExt(lay, trl);
  }
  else {
    lay->hard_clamped = false;	// run free: generate prediction of future reward
    lay->InitExterns();
  }
}

void TDRewPredLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  // doing second because act_p is computed only at end of settling!
  // this is better than clamping the value in the middle of everything
  // and then continuing with settling..
  if(trl->phase_no.val < trl->phase_no.max-1)
    return; // only do FINAL dwt!
  ScalarValLayerSpec::Compute_dWt(lay, trl);
}

//////////////////////////////////////////
//	TD Rew Integ Layer Spec		//
//////////////////////////////////////////

void TDRewIntegSpec::Initialize() {
  discount = .8f;
}

void TDRewIntegLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;
  unit_range.min = 0.0f;
  unit_range.max = 3.0f;
  unit_range.UpdateAfterEdit();
  val_range.UpdateAfterEdit();
}

void TDRewIntegLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  Initialize();
}

void TDRewIntegLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(rew_integ, this);
}

void TDRewIntegLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  rew_integ.UpdateAfterEdit();
}

void TDRewIntegLayerSpec::HelpConfig() {
  String help = "TDRewIntegLayerSpec Computation:\n\
 Integrates reward predictions from TDRewPred layer, and external actual rewards from\
 ExtRew layer.  Plus-minus phase difference is td value.\n\
 - Minus phase = previous expected reward V^(t) copied directly from TDRewPred\n\
 - Plus phase = integration of ExtRew r(t) and new TDRewPred computing V^(t+1)).\n\
 - No learning.\n\
 \nTDRewIntegLayerSpec Configuration:\n\
 - Requires 2 input projections, from TDRewPred, ExtRew layers.\n\
 - Sending connection to TdLayerSpec(s) (marked with MarkerConSpec)\n\
 (to compute the td change in expected rewards as computed by this layer)\n\
 - This layer must be before  TdLayerSpec layer in list of layers\n\
 \n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec\
 which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool TDRewIntegLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("TDRewIntegLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("TDRewIntegLayerSpec: must have DaModUnits!");
    return false;
  }

  SetUnique("decay", true);
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("TDRewIntegLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  LeabraLayer* rew_pred_lay = NULL;
  LeabraLayer* ext_rew_lay = NULL;

  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    LeabraLayer* flay = (LeabraLayer*)recv_gp->prjn->from;
    LeabraLayerSpec* fls = (LeabraLayerSpec*)flay->spec.spec;
    if(fls->InheritsFrom(&TA_TDRewPredLayerSpec)) {
      rew_pred_lay = flay;
    }
    else if(fls->InheritsFrom(&TA_ExtRewLayerSpec)) {
      ext_rew_lay = flay;
    }
  }

  if(rew_pred_lay == NULL) {
    taMisc::Error("TDRewIntegLayerSpec: requires recv projection from TDRewPredLayerSpec!");
    return false;
  }
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int rpidx = lay->own_net->layers.FindLeaf(rew_pred_lay);
  if(rpidx > myidx) {
    taMisc::Error("TDRewIntegLayerSpec: reward prediction layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  if(ext_rew_lay == NULL) {
    taMisc::Error("TDRewIntegLayerSpec: TD requires recv projection from ExtRewLayerSpec!");
    return false;
  }
  int eridx = lay->own_net->layers.FindLeaf(ext_rew_lay);
  if(eridx > myidx) {
    taMisc::Error("TDRewIntegLayerSpec: external reward layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }
  return true;
}

void TDRewIntegLayerSpec::Compute_Act(LeabraLayer* lay, LeabraTrial* trl) {
  lay->SetExtFlag(Unit::EXT);

  float rew_pred_val = 0.0f;
  float ext_rew_val = 0.0f;
  bool ext_rew_avail = true;

  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      continue;
    }
    LeabraLayer* flay = (LeabraLayer*)recv_gp->prjn->from;
    LeabraLayerSpec* fls = (LeabraLayerSpec*)flay->spec.spec;
    if(fls->InheritsFrom(&TA_TDRewPredLayerSpec)) {
      LeabraUnit* rpu = (LeabraUnit*)flay->units.Leaf(0); // todo; base on connections..
      rew_pred_val = rpu->act_eq; // use current input 
    }
    else if(fls->InheritsFrom(&TA_ExtRewLayerSpec)) {
      LeabraUnit* eru = (LeabraUnit*)flay->units.Leaf(0);
      ext_rew_val = eru->act_eq;
      if(flay->acts.max < .1f)	// indication of no reward available!
	ext_rew_avail = false;
    }
  }

  float new_val;
  if(trl->phase_no == 0) {
    new_val = rew_pred_val; // no discount in minus phase!!!  should only reflect previous V^(t)
  }
  else {
    new_val = rew_integ.discount * rew_pred_val + ext_rew_val; // now discount new rewpred!
  }
    
  UNIT_GP_ITR(lay, 
      DaModUnit* u = (DaModUnit*)ugp->FastEl(0);
      u->ext = new_val;
      ClampValue(ugp, trl);
	      );
  HardClampExt(lay, trl);
}

void TDRewIntegLayerSpec::Compute_dWt(LeabraLayer*, LeabraTrial*) {
  return;
}

//////////////////////////////////
//	Td Layer Spec		//
//////////////////////////////////

void TdLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.clamp_phase2 = false;
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
}

void TdLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  Initialize();
}

void TdLayerSpec::HelpConfig() {
  String help = "TdLayerSpec Computation:\n\
 - act of unit(s) = act_dif of unit(s) in reward integration layer we recv from\n\
 - td is dynamically computed in plus phaes and sent all layers that recv from us\n\
 - No Learning\n\
 \nTdLayerSpec Configuration:\n\
 - Single recv connection marked with a MarkerConSpec from reward integration layer\
     (computes expectations and actual reward signals)\n\
 - This layer must be after corresp. reward integration layer in list of layers\n\
 - Sending connections must connect to units of type DaModUnit/Spec \
     (td signal from this layer put directly into td var on units)\n\
 - UnitSpec for this layer must have act_range and clamp_range set to -1 and 1 \
     (because negative td = negative activation signal here";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool TdLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  SetUnique("decay", true);
  decay.clamp_phase2 = false;

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
  LeabraLayer* rewinteg_lay = NULL;
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
    if(fmlay == NULL) {
      taMisc::Error("TdLayerSpec: null from layer in recv projection:", (String)g);
      return false;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)
	&& fmlay->spec.spec->InheritsFrom(TA_TDRewIntegLayerSpec)) {
      rewinteg_lay = fmlay;
      if(recv_gp->size <= 0) {
	taMisc::Error("TdLayerSpec: requires one recv projection with at least one unit!");
	return false;
      }
      if(!recv_gp->Un(0)->InheritsFrom(TA_DaModUnit)) {
	taMisc::Error("TdLayerSpec: I need to receive from a DaModUnit!");
	return false;
      }
    }
  }

  if(rewinteg_lay == NULL) {
    taMisc::Error("TdLayerSpec: did not find TDRewInteg layer to get Td from!");
    return false;
  }

  int myidx = lay->own_net->layers.FindLeaf(lay);
  int rpidx = lay->own_net->layers.FindLeaf(rewinteg_lay);
  if(rpidx > myidx) {
    taMisc::Error("TdLayerSpec: reward integration layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  // check sending layer projections for appropriate unit types
  int si;
  for(si=0;si<lay->send_prjns.size;si++) {
    Projection* prjn = (Projection*)lay->send_prjns[si];
    if(!prjn->from->units.el_typ->InheritsFrom(TA_DaModUnit)) {
      taMisc::Error("TdLayerSpec: all layers I send to must have DaModUnits!, layer:",
		    prjn->from->GetPath(),"doesn't");
      return false;
    }
  }
  return true;
}

void TdLayerSpec::Compute_ZeroAct(LeabraLayer* lay, LeabraTrial*) {
  lay->dav = 0.0f;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->ext = 0.0f;
    u->SetExtFlag(Unit::EXT);
  }      
}

void TdLayerSpec::Compute_Td(LeabraLayer* lay, LeabraTrial*) {
  int ri_prjn_idx;
  FindLayerFmSpec(lay, ri_prjn_idx, &TA_TDRewIntegLayerSpec);

  lay->dav = 0.0f;
  DaModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(DaModUnit, u, lay->units., i) {
    LeabraCon_Group* cg = (LeabraCon_Group*)u->recv.gp[ri_prjn_idx];
    // just taking the first unit = scalar val
    DaModUnit* su = (DaModUnit*)cg->Un(0);
    u->dav = su->act_eq - su->act_m; // subtract current minus previous!
    u->ext = u->dav;
    u->act_eq = u->act = u->net = u->ext;
    lay->dav += u->dav;
  }
  if(lay->units.leaves > 0) lay->dav /= (float)lay->units.leaves;
}

void TdLayerSpec::Send_Td(LeabraLayer* lay, LeabraTrial*) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion)	continue;
      int j;
      for(j=0;j<send_gp->size; j++) {
	((DaModUnit*)send_gp->Un(j))->dav = u->act;
      }
    }
  }
}

void TdLayerSpec::Compute_Act(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing
  Compute_Td(lay, trl);	// now get the td and clamp it to layer
  Send_Td(lay, trl);
  Compute_ActAvg(lay, trl);
}

void TdLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no == 0) {
    lay->hard_clamped = true;
    lay->SetExtFlag(Unit::EXT);
    lay->Inhib_SetVals(i_kwta_pt); // assume 0 - 1 clamped inputs
    Compute_ZeroAct(lay, trl);	// can't do anything during settle anyway -- just zero it
  }
  else {
    // run "free" in plus phase: compute act = td
    lay->hard_clamped = false;
    lay->UnSetExtFlag(Unit::EXT);
  }
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
}

////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// 	LV Con Spec			//
//////////////////////////////////////////

void LVConSpec::Initialize() {
  SetUnique("lmix", true);
  lmix.hebb = 0.0f;
  lmix.err = 1.0f;

  SetUnique("rnd", true);
  rnd.mean = 0.1f;
  rnd.var = 0.0f;

  SetUnique("wt_limits", true);
  wt_limits.sym = false;

  SetUnique("wt_sig", true);
  wt_sig.gain = 1.0f;  wt_sig.off = 1.0f;

  SetUnique("lrate", true);
  lrate = .01f;
  cur_lrate = .01f;

  SetUnique("lrate_sched", true); // not to have any lrate schedule!!
  SetUnique("lrs_value", true); // not to have any lrate schedule!!
  lrs_value = NO_LRS;
}

//////////////////////////////////
//	LV (NAc) Layer Spec	//
//////////////////////////////////

void LVSpec::Initialize() {
  avg_dt = .005f;
  avg_init = .5f;
  use_pt = false;
  thr_avg_pt = .6f;
  thr_min = .25f;
  thr_max = .75f;
}

void LVLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  bias_val.val = .5f;		// default is no-information case; extrew = .5
}

void LVLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  Initialize();
}

void LVLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(lv, this);
}

void LVLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  lv.UpdateAfterEdit();
}

void LVLayerSpec::HelpConfig() {
  String help = "LVLayerSpec Learned Value (NAC_lv) Computation:\n\
 Learns when external rewards occur, and cancels these reward values in DA computation.\
 It is always trained on the current external reward value from the ExtRew layer\
 (0 = no reward or none avail).\n\
 - Activation is always expectation of reward\n\
 - At very end of trial, training value is clamped onto unit act_p values to provide training signal:\n\
 - Learning is (ru->act_p - ru->act_m) * su->act_p: delta on recv units times sender activations.\n\
 \nLVLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
 - All units I recv from must be DaModUnit/Spec units\n\
 - Recv cons from relvant network state layers\n\
 - Marker recv con from ExtRew layer to get external rewards\n\
 - Sending cons to Da/SNc layers\
 \n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec\
 which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool LVLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("LVLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(trl->no_plus_test) {
    if(!quiet) taMisc::Error("LVLayerSpec: requires LeabraTrial no_plus_test = false, I just set it for you");
    trl->no_plus_test = false;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("LVLayerSpec: must have DaModUnits!");
    return false;
  }

  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("LVLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }
  if((us->opt_thresh.learn >= 0.0f) || us->opt_thresh.updt_wts) {
    if(!quiet) taMisc::Error("LVLayerSpec: UnitSpec opt_thresh.learn must be -1 to allow proper learning of all units",
			     "I just set it for you in spec:", us->name,
			     "(make sure this is appropriate for all layers that use this spec!)");
    us->SetUnique("opt_thresh", true);
    us->opt_thresh.learn = -1.0f;
    us->opt_thresh.updt_wts = false;
  }
  ((DaModUnitSpec*)us)->da_mod.p_dwt = false; // don't need prior state dwt
  if(us->act.avg_dt != 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.0f;
    if(!quiet) taMisc::Error("LVLayerSpec: requires UnitSpec act.avg_dt = 0, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  LeabraLayer* ext_rew_lay = NULL;
  LeabraLayer* da_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* flay = (LeabraLayer*)recv_gp->prjn->from;
      LeabraLayerSpec* fls = (LeabraLayerSpec*)flay->spec.spec;
      if(fls->InheritsFrom(TA_ExtRewLayerSpec)) ext_rew_lay = flay;
      if(fls->InheritsFrom(TA_PVLVDaLayerSpec)) da_lay = flay;
      continue;
    }
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(!cs->InheritsFrom(TA_LVConSpec)) {
      taMisc::Error("LVLayerSpec: requires recv connections to be of type LVConSpec");
      return false;
    }
  }
  
  if(ext_rew_lay == NULL) {
    taMisc::Error("LVLayerSpec: requires MarkerConSpec connection from ExtRewLayerSpec layer to get external rewards!");
    return false;
  }
  if(da_lay == NULL) {
    taMisc::Error("LVLayerSpec: requires MarkerConSpec connection from PVLVDaLayerSpec layer to get DA values!");
    return false;
  }
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int eridx = lay->own_net->layers.FindLeaf(ext_rew_lay);
  if(eridx > myidx) {
    taMisc::Error("LVLayerSpec: ExtRew layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  return true;
}

void LVLayerSpec::InitWtState(LeabraLayer* lay) {
  ScalarValLayerSpec::InitWtState(lay);
  UNIT_GP_ITR(lay, 
	      DaModUnit* u = (DaModUnit*)ugp->FastEl(0);
	      u->act_avg = lv.avg_init;
	      u->misc_1 = lv.avg_init; // misc_1 = avg val of LV when rewarded
	      );
}

float LVLayerSpec::Compute_ExtRew(LeabraLayer* lay, LeabraTrial*, bool& actual_er_avail, bool& lv_est_er_avail) {
  float ext_rew_val = 0.0f;
  actual_er_avail = true;

  int prjn_idx = 0;
  LeabraLayer* erlay = FindLayerFmSpec(lay, prjn_idx, &TA_ExtRewLayerSpec);
  if(erlay != NULL) {
    LeabraUnit* eru = (LeabraUnit*)erlay->units.Leaf(0);
    ext_rew_val = eru->act_eq;
    if(eru->misc_1 == 0.0f) { // indication of no reward available!
      actual_er_avail = false;
    }
  }

  lv_est_er_avail = false;
     
  LeabraUnit* lvu = (LeabraUnit*)lay->units.Leaf(0);
  if(lv.use_pt) {
    float lv_oth_avg = lvu->act_avg;
    float lv_hit_avg = lvu->misc_1;
    float lv_thr =  lv_oth_avg + lv.thr_avg_pt * (lv_hit_avg - lv_oth_avg);
    if(lv_thr > lv.thr_max) lv_thr = lv.thr_max;
    if(lv_thr < lv.thr_min) lv_thr = lv.thr_min;
    if(MAX(lvu->act_m, ext_rew_val) > lv_thr)
      lv_est_er_avail = true; // lv says reward!
  }
  else {
    if((MAX(lvu->act_m, ext_rew_val) > lv.thr_max) || (MIN(lvu->act_m, ext_rew_val) < lv.thr_min))
      lv_est_er_avail = true;
  }

  return ext_rew_val;
}

void LVLayerSpec::Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial*) {
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

void LVLayerSpec::Compute_dWtUgp(Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl) {
  int ui;
  for(ui=1;ui<ugp->size;ui++) {	// don't bother with first unit!
    DaModUnit* u = (DaModUnit*)ugp->FastEl(ui);
    u->Compute_dWt(lay, trl);
  }
  //  AdaptKWTAPt(lay, trl);
}

void LVLayerSpec::Compute_LVPlusPhaseDwt(LeabraLayer* lay, LeabraTrial* trl) {
  bool actual_er_avail = false;
  bool lv_est_er_avail = false;
  float ext_rew_val = Compute_ExtRew(lay, trl, actual_er_avail, lv_est_er_avail);

  int da_prjn_idx = 0;
  FindLayerFmSpec(lay, da_prjn_idx, &TA_PVLVDaLayerSpec);

  UNIT_GP_ITR
    (lay, 
     LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
     // update average: keep "hits" and "others" separately, put between two
     if(actual_er_avail) 
       u->misc_1 += lv.avg_dt * (u->act_m - u->misc_1);	// hits
     else
       u->act_avg += lv.avg_dt * (u->act_m - u->act_avg); // others

     u->ext = ext_rew_val;
     ClampValue(ugp, trl); // apply new value
     Compute_ExtToPlus(ugp, trl); // copy ext values to act_p
     Compute_dWtUgp(ugp, lay, trl);
     );
}

void LVLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if(trl->phase_no.val < trl->phase_no.max-1)
    return; // only do FINAL dwt!
  Compute_LVPlusPhaseDwt(lay, trl);
}

//////////////////////////////////////////
// 	PV Con Spec			//
//////////////////////////////////////////

void PVConSpec::Initialize() {
//   min_con_type = &TA_PVLVCon;

  SetUnique("lmix", true);
  lmix.hebb = 0.0f;
  lmix.err = 1.0f;

  SetUnique("rnd", true);
  rnd.mean = 0.1f;
  rnd.var = 0.0f;

  SetUnique("wt_limits", true);
  wt_limits.sym = false;

  SetUnique("wt_sig", true);
  wt_sig.gain = 1.0f;  wt_sig.off = 1.0f;

  SetUnique("lrate", true);
  lrate = .05f;
  cur_lrate = .05f;

//   decay = 0.0f;

  syn_dep.depl = 1.1f;
  SetUnique("lrate_sched", true); // not to have any lrate schedule!!
  SetUnique("lrs_value", true); // not to have any lrate schedule!!
  lrs_value = NO_LRS;
}

//////////////////////////////////////////
//	PV Layer Spec: Perceived Value	//
//////////////////////////////////////////

void PVSpec::Initialize() {
  lrn_mode = ER;
//   gc_mod = false;
  da_gain = 1.5f;
  use_er = false;
}

void PVLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  bias_val.val = .05f;
}

void PVLayerSpec::Defaults() {
  ScalarValLayerSpec::Defaults();
  Initialize();
}

void PVLayerSpec::InitLinks() {
  ScalarValLayerSpec::InitLinks();
  taBase::Own(pv, this);
}

void PVLayerSpec::UpdateAfterEdit() {
  ScalarValLayerSpec::UpdateAfterEdit();
  pv.UpdateAfterEdit();
}

void PVLayerSpec::HelpConfig() {
  String help = "PVLayerSpec Perceived Value (PV_fast = ABL, PV_slow = NAC_pv) Computation:\n\
 Learns perceived values (PV) according to the PVLV algorithm: looks at current network state\
 and computes how much it resembles states that have been associated with learned value (LV) in the past\n\
 - Activation is always perceived values\n\
 - At very end of trial, training value is clamped onto unit act_p values to provide training signal:\n\
 - (training only occurs when external reward was delivered (or expected to be delivered)\n\
 - Learning is (ru->act_p - ru->act_m) * su->act_p: delta on recv units times sender activations.\n\
 \nPVLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
 - All units I recv from must be DaModUnit/Spec units\n\
 - Recv cons from relvant network state layers\n\
 - Marker recv con from LV layer to get training signal\n\
 - Sending cons to Da/SNc layers\
 \n(After pressing OK here, you will see information for configuring the ScalarValLayerSpec\
 which this layer is based on)";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  ScalarValLayerSpec::HelpConfig();
}

bool PVLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!ScalarValLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("PVLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(trl->no_plus_test) {
    if(!quiet) taMisc::Error("PVLayerSpec: requires LeabraTrial no_plus_test = false, I just set it for you");
    trl->no_plus_test = false;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("PVLayerSpec: must have DaModUnits!");
    return false;
  }

  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("PVLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }
  if((us->opt_thresh.learn >= 0.0f) || us->opt_thresh.updt_wts) {
    if(!quiet) taMisc::Error("PVLayerSpec: UnitSpec opt_thresh.learn must be -1 to allow proper learning of all units",
			     "I just set it for you in spec:", us->name,
			     "(make sure this is appropriate for all layers that use this spec!)");
    us->SetUnique("opt_thresh", true);
    us->opt_thresh.learn = -1.0f;
    us->opt_thresh.updt_wts = false;
  }
  ((DaModUnitSpec*)us)->da_mod.p_dwt = false; // don't need prior state dwt
  us->UpdateAfterEdit();

  // check for conspecs with correct params
  LeabraLayer* da_lay = NULL;
  LeabraLayer* lv_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from == recv_gp->prjn->layer) { // self projection, skip it
      continue;
    }
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* flay = (LeabraLayer*)recv_gp->prjn->from;
      LeabraLayerSpec* fls = (LeabraLayerSpec*)flay->spec.spec;
      if(fls->InheritsFrom(TA_PVLVDaLayerSpec)) da_lay = flay;
      if(fls->InheritsFrom(TA_LVLayerSpec)) lv_lay = flay;
      continue;
    }
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    if(!cs->InheritsFrom(TA_PVConSpec)) {
      taMisc::Error("PVLayerSpec: requires recv connections to be of type PVConSpec");
      return false;
    }
  }
  
  if(da_lay == NULL) {
    taMisc::Error("PVLayerSpec: requires MarkerConSpec connection from PVLVDaLayerSpec layer to get DA values!");
    return false;
  }
  if(lv_lay == NULL) {
    taMisc::Error("PVLayerSpec: requires MarkerConSpec connection from LVLayerSpec layer to get DA values!");
    return false;
  }

  return true;
}

void PVLayerSpec::Compute_ExtToPlus(Unit_Group* ugp, LeabraTrial*) {
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

// void PVLayerSpec::Compute_DaLearnMod(LeabraLayer* lay, Unit_Group* ugp, LeabraInhib*, LeabraTrial* trl) {
//   int da_prjn_idx = 0;
//   FindLayerFmSpec(lay, da_prjn_idx, &TA_PVLVDaLayerSpec);
//   int lv_prjn_idx = 0;
//   LeabraLayer* lv_lay = FindLayerFmSpec(lay, lv_prjn_idx, &TA_LVLayerSpec);
//   LVLayerSpec* lvls = (LVLayerSpec*)lv_lay->spec.spec;
//   bool ext_rew_avail = false;
//   float ext_rew_val = lvls->Compute_ExtRew(lv_lay, trl, ext_rew_avail);

//   float dav = 0.0f;
//   DaModUnit* u = (DaModUnit*)ugp->FastEl(0);
//   if(pv.lrn_mode == PVSpec::ER) {
//     if(!ext_rew_avail) {
//       return;			// no modulation!
//     }
//     dav = 2.0f * (ext_rew_val - .5f); // + 1 for er, -1 for no er..
//   }
//   else if(pv.lrn_mode == PVSpec::ER_LV) {
//     dav = 2.0f * (ext_rew_val - .5f); // + 1 for er, -1 for no er..
//   }
//   else if(pv.lrn_mode == PVSpec::DA_LV) {
//     LeabraCon_Group* lvcg = (LeabraCon_Group*)u->recv.gp[lv_prjn_idx];
//     DaModUnit* lvsu = (DaModUnit*)lvcg->Un(0);
//     dav = ext_rew_val - lvsu->act_m;
//   }
//   else if(pv.lrn_mode == PVSpec::DA) {
//     if(trl->phase_no == 1)
//       dav = u->dav;
//     else {
//       LeabraCon_Group* dacg = (LeabraCon_Group*)u->recv.gp[da_prjn_idx];
//       DaModUnit* dasu = (DaModUnit*)dacg->Un(0);
//       dav = dasu->act_p;	// er - lv DA signal
//     }
//   }
  
//   taLeafItr i;
//   FOR_ITR_EL(DaModUnit, u, ugp->, i) {
//     if(dav > 0.0f)  { 
//       u->vcb.g_h = pv.da_gain * dav;
//       u->vcb.g_a = 0.0f;
//     }
//     else {
//       u->vcb.g_h = 0.0f;
//       u->vcb.g_a = -pv.da_gain * dav;
//     }
//   }
// }

// void PVLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
//   if(pv.gc_mod && (trl->phase_no >= 1)) {
//     Compute_DaLearnMod(lay, ug, thr, trl);
//   }
//   ScalarValLayerSpec::Compute_Act_impl(lay, ug, thr, trl);
// }

void PVLayerSpec::Compute_dWtUgp(Unit_Group* ugp, LeabraLayer* lay, LeabraTrial* trl) {
  int ui;
  for(ui=1;ui<ugp->size;ui++) {	// don't bother with first unit!
    DaModUnit* u = (DaModUnit*)ugp->FastEl(ui);
    u->Compute_dWt(lay, trl);
  }
  //  AdaptKWTAPt(lay, trl);
}

void PVLayerSpec::Compute_DepressWt(Unit_Group* ugp, LeabraLayer*, LeabraTrial*) {
  int ui;
  for(ui=1;ui<ugp->size;ui++) {	// don't bother with first unit!
    DaModUnit* u = (DaModUnit*)ugp->FastEl(ui);
    LeabraCon_Group* recv_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
      if(!recv_gp->spec.spec->InheritsFrom(TA_PVConSpec)) continue;
      PVConSpec* cs = (PVConSpec*)recv_gp->spec.spec;
      cs->Depress_Wt(recv_gp, u);
    }
  }
}

void PVLayerSpec::Compute_PVPlusPhaseDwt(LeabraLayer* lay, LeabraTrial* trl) {
  int da_prjn_idx = 0;
  FindLayerFmSpec(lay, da_prjn_idx, &TA_PVLVDaLayerSpec);
  int lv_prjn_idx = 0;
  LeabraLayer* lv_lay = FindLayerFmSpec(lay, lv_prjn_idx, &TA_LVLayerSpec);
  LVLayerSpec* lvls = (LVLayerSpec*)lv_lay->spec.spec;
  bool actual_er_avail = false;
  bool lv_est_er_avail = false;
  float ext_rew_val = lvls->Compute_ExtRew(lv_lay, trl, actual_er_avail, lv_est_er_avail);

  bool er_avail = lv_est_er_avail;
  if(pv.use_er) er_avail = actual_er_avail; // cheat..

  UNIT_GP_ITR
    (lay, 
     LeabraUnit* u = (LeabraUnit*)ugp->FastEl(0);
     LeabraCon_Group* lvcg = (LeabraCon_Group*)u->recv.gp[lv_prjn_idx];
     DaModUnit* lvsu = (DaModUnit*)lvcg->Un(0);

     if(pv.lrn_mode == PVSpec::ER) {
       if(er_avail) {
	 u->ext = ext_rew_val;
	 ClampValue(ugp, trl); // apply new value
	 Compute_ExtToPlus(ugp, trl); // copy ext values to act_p
	 Compute_dWtUgp(ugp, lay, trl);
       }
       else {
	 Compute_DepressWt(ugp, lay, trl); // always depress!!
       }
     }
     else if(pv.lrn_mode == PVSpec::DA_LV) {
       float dav = ext_rew_val - lvsu->act_m; // LV da value
       if(er_avail) {
	 float curval = u->act_p; if(curval > 1.0f) curval = 1.0f; if(curval < 0.0f) curval = 0.0f;
	 float nval;
	 if(dav > 0.0f) {
	   nval = curval + pv.da_gain * (1.0f - curval) * dav;
	   if(nval > 1.0f) nval = 1.0f;
	 }
	 else {
	   nval = curval + pv.da_gain * curval * dav;
	   if(nval < 0.0f) nval = 0.0f;
	 }
	 u->ext = nval;
	 ClampValue(ugp, trl); // apply new value
	 Compute_ExtToPlus(ugp, trl); // copy ext values to act_p
	 Compute_dWtUgp(ugp, lay, trl);
       }
       else {
	 Compute_DepressWt(ugp, lay, trl); // always depress!!
       }
     }
     else if(pv.lrn_mode == PVSpec::DA) {
       LeabraCon_Group* dacg = (LeabraCon_Group*)u->recv.gp[da_prjn_idx];
       DaModUnit* dasu = (DaModUnit*)dacg->Un(0);
       float dav = dasu->act_p;	// global DA signal
       if(er_avail) {
	 float curval = u->act_p; if(curval > 1.0f) curval = 1.0f; if(curval < 0.0f) curval = 0.0f;
	 float nval;
	 if(dav > 0.0f) {
	   nval = curval + pv.da_gain * (1.0f - curval) * dav;
	   if(nval > 1.0f) nval = 1.0f;
	 }
	 else {
	   nval = curval + pv.da_gain * curval * dav;
	   if(nval < 0.0f) nval = 0.0f;
	 }
	 u->ext = nval;
	 ClampValue(ugp, trl); // apply new value
	 Compute_ExtToPlus(ugp, trl); // copy ext values to act_p
	 Compute_dWtUgp(ugp, lay, trl);
       }
       else {
	 Compute_DepressWt(ugp, lay, trl); // always depress!!
       }
     }
     );
}

void PVLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  // doing second because act_p is computed only at end of settling!
  // this is better than clamping the value in the middle of everything
  // and then continuing with settling..
  if(trl->phase_no.val < trl->phase_no.max-1)
    return;
  Compute_PVPlusPhaseDwt(lay, trl);
}

//////////////////////////////////
//	PVLVDa Layer Spec	//
//////////////////////////////////

void PVLVDaSpec::Initialize() {
  mode = IF_ER_LV;
  tonic_da = 0.0f;
  min_pv_s = 0.1f;
  pp_lv = false;
  use_er = false;
}

void PVLVDaLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.clamp_phase2 = false;
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

void PVLVDaLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(da, this);
}

void PVLVDaLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  da.Initialize();
  Initialize();
}

void PVLVDaLayerSpec::HelpConfig() {
  String help = "PVLVDaLayerSpec (DA value) Computation:\n\
 - Computes DA value based on inputs from PVLV layers.\n\
 - No Learning\n\
 \nPVLVDaLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
 - Recv cons marked with a MarkerConSpec from PVLV\n\
 - Sending cons to units of type DaModUnit/Spec; puts into their da value\n\
 - This layer must be after recv layers in list of layers\n\
 - UnitSpec for this layer must have act_range and clamp_range set to -1 and 1 \
     (because negative da = negative activation signal here";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool PVLVDaLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  SetUnique("decay", true);
  decay.clamp_phase2 = false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("PVLVDaLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  // must have the appropriate ranges for unit specs..
  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if((us->act_range.max != 2.0f) || (us->act_range.min != -2.0f)) {
    us->SetUnique("act_range", true);
    us->act_range.max = 2.0f;
    us->act_range.min = -2.0f;
    us->act_range.UpdateAfterEdit();
    if(!quiet) taMisc::Error("PVLVDaLayerSpec: requires UnitSpec act_range.max = 2, min = -2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if((us->clamp_range.max != 2.0f) || (us->clamp_range.min != -2.0f)) {
    us->SetUnique("clamp_range", true);
    us->clamp_range.max = 2.0f;
    us->clamp_range.min = -2.0f;
    us->clamp_range.UpdateAfterEdit();
    if(!quiet) taMisc::Error("PVLVDaLayerSpec: requires UnitSpec clamp_range.max = 2, min = -2, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->act.avg_dt != 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.0f;
    if(!quiet) taMisc::Error("PVLVDaLayerSpec: requires UnitSpec act.avg_dt = 0, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }

  // check recv connection
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraLayer* pv_lay = NULL;
  LeabraLayer* pvs_lay = NULL;
  LeabraLayer* lv_lay = NULL;
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
    LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
    LeabraLayerSpec* fls = (LeabraLayerSpec*)fmlay->spec.spec;
    if(cs->InheritsFrom(TA_MarkerConSpec)) {
      if(recv_gp->size <= 0) {
	taMisc::Error("PVLVDaLayerSpec: requires one recv projection with at least one unit!");
	return false;
      }
      if(!recv_gp->Un(0)->InheritsFrom(TA_DaModUnit)) {
	taMisc::Error("PVLVDaLayerSpec: I need to receive from a DaModUnit!");
	return false;
      }
      if(fls->InheritsFrom(TA_PVLayerSpec))  pv_lay = fmlay;
      if(fls->InheritsFrom(TA_PVSLayerSpec))  pvs_lay = fmlay;
      if(fls->InheritsFrom(TA_LVLayerSpec)) lv_lay = fmlay;
    }
  }

  if(pv_lay == NULL) {
    taMisc::Error("PVLVDaLayerSpec: did not find PV layer to get Da from!");
    return false;
  }
  if(pvs_lay == NULL) {
    taMisc::Error("PVLVDaLayerSpec: did not find PVS layer to get Da from!");
    return false;
  }
  if(lv_lay == NULL) {
    taMisc::Error("PVLVDaLayerSpec: did not find LV layer to get Da from!");
    return false;
  }

  int myidx = lay->own_net->layers.FindLeaf(lay);
  int pvidx = lay->own_net->layers.FindLeaf(pv_lay);
  if(pvidx > myidx) {
    taMisc::Error("PVLVDaLayerSpec: PV layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }
  pvidx = lay->own_net->layers.FindLeaf(pvs_lay);
  if(pvidx > myidx) {
    taMisc::Error("PVLVDaLayerSpec: PVS layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }
  pvidx = lay->own_net->layers.FindLeaf(lv_lay);
  if(pvidx > myidx) {
    taMisc::Error("PVLVDaLayerSpec: LV layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  // check sending layer projections for appropriate unit types
  int si;
  for(si=0;si<lay->send_prjns.size;si++) {
    Projection* prjn = (Projection*)lay->send_prjns[si];
    if(!prjn->from->units.el_typ->InheritsFrom(TA_DaModUnit)) {
      taMisc::Error("PVLVDaLayerSpec: all layers I send to must have DaModUnits!, layer:",
		    prjn->from->GetPath(),"doesn't");
      return false;
    }
  }
  return true;
}

void PVLVDaLayerSpec::Compute_ZeroAct(LeabraLayer* lay, LeabraTrial*) {
  lay->dav = 0.0f;
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    u->ext = da.tonic_da;
    u->SetExtFlag(Unit::EXT);
  }      
}

void PVLVDaLayerSpec::Compute_Da(LeabraLayer* lay, LeabraTrial* trl) {
  int pv_prjn_idx;
  FindLayerFmSpec(lay, pv_prjn_idx, &TA_PVLayerSpec); // abl
  int pvs_prjn_idx;
  FindLayerFmSpec(lay, pvs_prjn_idx, &TA_PVSLayerSpec);
  int lv_prjn_idx;
  LeabraLayer* lv_lay = FindLayerFmSpec(lay, lv_prjn_idx, &TA_LVLayerSpec);
  LVLayerSpec* lvls = (LVLayerSpec*)lv_lay->spec.spec;
  bool actual_er_avail = false;
  bool lv_est_er_avail = false;
  float ext_rew_val = lvls->Compute_ExtRew(lv_lay, trl, actual_er_avail, lv_est_er_avail);

  bool er_avail = lv_est_er_avail;
  if(da.use_er) er_avail = actual_er_avail; // cheat..

  lay->dav = 0.0f;
  DaModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(DaModUnit, u, lay->units., i) {
    LeabraCon_Group* pvcg = (LeabraCon_Group*)u->recv.gp[pv_prjn_idx];
    DaModUnit* pvsu = (DaModUnit*)pvcg->Un(0);
    LeabraCon_Group* pvscg = (LeabraCon_Group*)u->recv.gp[pvs_prjn_idx];
    DaModUnit* pvssu = (DaModUnit*)pvscg->Un(0);
    LeabraCon_Group* lvcg = (LeabraCon_Group*)u->recv.gp[lv_prjn_idx];
    DaModUnit* lvsu = (DaModUnit*)lvcg->Un(0);
    float eff_pvs = MAX(pvssu->act_eq, da.min_pv_s); // effective pvs value
    float pv_da = pvsu->act_eq - eff_pvs; 
    float lv_da = ext_rew_val - lvsu->act_m; 

    if(trl->phase_no == 0) {	// not used at this point..
      u->dav = pv_da; 		// pvsu->act_eq - avgbl;
    }
    else if(trl->phase_no == 1) {
      if(da.mode == PVLVDaSpec::IF_ER_LV) {
	if(er_avail)
	  u->dav = lv_da;
	else
	  u->dav = pv_da;
      }
      else if(da.mode == PVLVDaSpec::LV_PLUS_PV) {
	u->dav = lv_da + pv_da;
      }
    }
    else {			// phase p2
      u->dav = pv_da;
      if(da.pp_lv) {
	if(da.mode == PVLVDaSpec::IF_ER_LV) {
	  if(er_avail)
	    u->dav += lv_da;
	}
	else if(da.mode == PVLVDaSpec::LV_PLUS_PV) {
	  u->dav += lv_da;	// always add regardless of er..
	}
      }
    }
    u->ext = da.tonic_da + u->dav;
    u->act_eq = u->act = u->net = u->ext;
    lay->dav += u->dav;
  }
  if(lay->units.leaves > 0) lay->dav /= (float)lay->units.leaves;
}

void PVLVDaLayerSpec::Send_Da(LeabraLayer* lay, LeabraTrial*) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i) {
    LeabraCon_Group* send_gp;
    int g;
    FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
      LeabraLayer* tol = (LeabraLayer*) send_gp->prjn->layer;
      if(tol->lesion)	continue;
      int j;
      for(j=0;j<send_gp->size; j++) {
	((DaModUnit*)send_gp->Un(j))->dav = u->act;
      }
    }
  }
}

void PVLVDaLayerSpec::Compute_Act(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->cycle >= 0) && lay->hard_clamped)
    return;			// don't do this during normal processing
  Compute_Da(lay, trl);	// now get the da and clamp it to layer
  Send_Da(lay, trl);
  Compute_ActAvg(lay, trl);
}

void PVLVDaLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  lay->hard_clamped = false;
  lay->UnSetExtFlag(Unit::EXT);
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
}

void PVLVDaLayerSpec::Compute_dWt(LeabraLayer*, LeabraTrial*) {
  return;
}

//////////////////////////////////
//	Patch Layer Spec	//
//////////////////////////////////

void PatchLayerSpec::Initialize() {
  SetUnique("gp_kwta", true);
  gp_kwta.k_from = KWTASpec::USE_K;
  gp_kwta.k = 8;
  inhib_group = UNIT_GROUPS;
}

//////////////////////////////////
//	SNc Layer Spec		//
//////////////////////////////////

void SNcMiscSpec::Initialize() {
  patch_mode = NO_PATCH;
  patch_gain = .5f;
}

void SNcLayerSpec::Initialize() {
}

void SNcLayerSpec::Defaults() {
  PVLVDaLayerSpec::Defaults();
  snc.Initialize();
  Initialize();
}

void SNcLayerSpec::InitLinks() {
  PVLVDaLayerSpec::InitLinks();
  taBase::Own(snc, this);
}

void SNcLayerSpec::HelpConfig() {
  String help = "SNcLayerSpec Computation:\n\
 Provides a stripe-specifc DA signal to Matrix Layer units, based on patch input.\n\
 This is currently not supported.  Also, stripe-specific DA signals are computed\
 directly in the Matrix based on SNrThal multiplication of the signal, even though\
 biologically this signal is likely reflected here in the SNc activations\
 (this is computationally easier and creates fewer interdependencies.\n\
 After pressing OK here, you will see configuration info for the PVLVDaLayerSpec\
 which this layer is based on";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
  PVLVDaLayerSpec::HelpConfig();
}

bool SNcLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!PVLVDaLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  int myidx = lay->own_net->layers.FindLeaf(lay);

  int pc_prjn_idx;
  LeabraLayer* pclay = FindLayerFmSpec(lay, pc_prjn_idx, &TA_PatchLayerSpec);
  if(pclay != NULL) {
    int patchidx = lay->own_net->layers.FindLeaf(pclay);
    if(patchidx > myidx) {
      taMisc::Error("SNcLayerSpec: Patch layer must be *before* this layer in list of layers -- it is now after, won't work");
      return false;
    }
  }
  else {
    snc.patch_mode = SNcMiscSpec::NO_PATCH;
  }

  return true;
}

void SNcLayerSpec::Compute_Da(LeabraLayer* lay, LeabraTrial* trl) {
  // todo: patch not supported right now!
  PVLVDaLayerSpec::Compute_Da(lay, trl);
}

//////////////////////////////////
//	MatrixConSpec		//
//////////////////////////////////

void MatrixConSpec::Initialize() {
  SetUnique("lmix", true);
//   lmix.hebb = .001f;
//   lmix.err = .999f;
  lmix.hebb = 0.0f;
  lmix.err = 1.0f;

  learn_rule = PFC;
  pfc_hebb_nego = true;
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

void MatrixBiasSpec::Initialize() {
  learn_rule = PFC;
}

void MatrixUnitSpec::Initialize() {
  SetUnique("bias_spec", true);
  bias_spec.type = &TA_MatrixBiasSpec;
  SetUnique("g_bar", true);
  g_bar.a = .03f;
  g_bar.h = .01f;

  freeze_net = true;
}

void MatrixUnitSpec::Defaults() {
  DaModUnitSpec::Defaults();
  Initialize();
}

void MatrixUnitSpec::InitLinks() {
  DaModUnitSpec::InitLinks();
  bias_spec.type = &TA_MatrixBiasSpec;
}

void MatrixUnitSpec::Compute_NetAvg(LeabraUnit* u, LeabraLayer* lay, LeabraInhib*, LeabraTrial* trl) {
  if(act.send_delta) {
    u->net_raw += u->net_delta;
    u->net += u->clmp_net + u->net_raw;
  }
  MatrixLayerSpec* mls = (MatrixLayerSpec*)lay->spec.spec;
  float eff_dt = dt.net;
  if(freeze_net) {
    if(mls->bg_type == MatrixLayerSpec::PFC) {
      if(trl->phase_no == 2) eff_dt = 0.0f;
    }
    else {
      if(trl->phase_no >= 1) eff_dt = 0.0f;
    }
  }

  u->net = u->prv_net + eff_dt * (u->net - u->prv_net);
  u->prv_net = u->net;
  if((noise_type == NETIN_NOISE) && (noise.type != Random::NONE) && (trl->cycle >= 0)) {
    u->net += noise_sched.GetVal(trl->cycle) * noise.Gen();
  }
  u->i_thr = Compute_IThresh(u, lay, trl);
}

void MatrixUnitSpec::PostSettle(LeabraUnit* u, LeabraLayer* lay, LeabraInhib* thr,
			       LeabraTrial* trl, bool set_both)
{
  DaModUnitSpec::PostSettle(u, lay, thr, trl, set_both);
  DaModUnit* lu = (DaModUnit*)u;
  if((trl->phase_order == LeabraTrial::MINUS_PLUS_PLUS) && (trl->phase_no == 2)) {
    lu->act_dif = lu->act_p2 - lu->act_p;
  }
}

//////////////////////////////////
//	Matrix Layer Spec	//
//////////////////////////////////

void ContrastSpec::Initialize() {
  gain = 1.0f;
  go_p = .5f;
  go_n = .5f;
  nogo_p = .5f;
  nogo_n = .5f;
}

void MatrixRndGoSpec::Initialize() {
  on = true;
  go_p = .1f;
  prf_go_p = .0001f;
  rgo_da = 20.0f;
  prf_rgo_da = 1.0f;
}

void MatrixRndGoThrSpec::Initialize() {
  abs_thr = 0.0f;
  abs_max = 0.1f;
  rel = 0.05f;
  mnt = 10;
  avgrew = 0.9f;
}

void MatrixMiscSpec::Initialize() {
  neg_da_bl = 0.0002f;
  neg_gain = 1.5f;
  perf_gain = 0.0f;
  max_mnt = 50;
  avg_go_dt = 0.005f;
}

/////////////////////////////////////////////////////

void MatrixLayerSpec::Initialize() {
  //  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  //  SetUnique("gp_kwta", true);
  gp_kwta.k_from = KWTASpec::USE_PCT;
  gp_kwta.pct = .25f;
  //  SetUnique("inhib_group", true);
  inhib_group = UNIT_GROUPS;
  //  SetUnique("compute_i", true);
  compute_i = KWTA_INHIB;
  //  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .25f;

  bg_type = PFC;
}

void MatrixLayerSpec::UpdateAfterEdit() {
  LeabraLayerSpec::UpdateAfterEdit();
}

void MatrixLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  matrix.Initialize();
  contrast.Initialize();
  rnd_go.Initialize();
  rnd_go_thr.Initialize();
  Initialize();
}

void MatrixLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(matrix, this);
  taBase::Own(contrast, this);
  taBase::Own(rnd_go, this);
  taBase::Own(rnd_go_thr, this);
}

void MatrixLayerSpec::HelpConfig() {
  String help = "MatrixLayerSpec Computation:\n\
 There are 2 types of units arranged sequentially in the following order within each\
 stripe whose firing affects the gating status of the corresponding stripe in PFC:\n\
 - GO unit = toggle maintenance of units in PFC: this is the direct pathway\n\
 - NOGO unit = maintain existing state in PFC (i.e. do nothing): this is the indirect pathway\n\
 \nMatrixLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
 - Units must be DaModUnits w/ MatrixUnitSpec and must recv from PVLVDaLayerSpec layer\
 (either VTA or SNc) to get da modulation for learning signal\n\
 - Recv connections need to be MatrixConSpec as learning occurs based on the da-signal\
 on the matrix units.\n\
 - This layer must be after DaLayers in list of layers\n\
 - Units must be organized into groups (stipes) of same number as PFC";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool MatrixLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet))
    return false;

  SetUnique("decay", true);
  decay.phase = 0.0f;
  decay.phase2 = 0.0f;
  decay.clamp_phase2 = false;

  if(lay->units.gp.size == 1) {
    taMisc::Error("MatrixLayerSpec: layer must contain multiple unit groups (= stripes) for indepent searching of gating space!");
    return false;
  }

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }
  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("MatrixLayerSpec: must have DaModUnits!");
    return false;
  }

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_MatrixUnitSpec)) {
    taMisc::Error("MatrixLayerSpec: UnitSpec must be MatrixUnitSpec!");
    return false;
  }
  ((DaModUnitSpec*)us)->da_mod.p_dwt = false; // don't need prior state dwt
  if((us->opt_thresh.learn >= 0.0f) || us->opt_thresh.updt_wts) {
    if(!quiet) taMisc::Error("MatrixLayerSpec: UnitSpec opt_thresh.learn must be -1 to allow proper learning of all units",
			     "I just set it for you in spec:", us->name,
			     "(make sure this is appropriate for all layers that use this spec!)");
    us->SetUnique("opt_thresh", true);
    us->opt_thresh.learn = -1.0f;
    us->opt_thresh.updt_wts = false;
  }
  if(us->act.avg_dt <= 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.005f;
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires UnitSpec act.avg_dt > 0, I just set it to .005 for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  us->SetUnique("g_bar", true);
  // must have these not initialized every trial!
  if(us->hyst.init) {
    us->SetUnique("hyst", true);
    us->hyst.init = false;
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires UnitSpec hyst.init = false, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->acc.init) {
    us->SetUnique("acc", true);
    us->acc.init = false;
    if(!quiet) taMisc::Error("MatrixLayerSpec: requires UnitSpec acc.init = false, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  us->UpdateAfterEdit();

  LeabraBiasSpec* bs = (LeabraBiasSpec*)us->bias_spec.spec;
  if(bs == NULL) {
    taMisc::Error("MatrixLayerSpec: Error: null bias spec in unit spec", us->name);
    return false;
  }

  LeabraLayer* da_lay = NULL;
  LeabraLayer* snr_lay = NULL;
  LeabraUnit* u = (LeabraUnit*)lay->units.Leaf(0);	// taking 1st unit as representative
  LeabraCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from == recv_gp->prjn->layer) // self projection, skip it
      continue;
    if(recv_gp->spec.spec->InheritsFrom(TA_MarkerConSpec)) {
      LeabraLayer* fmlay = (LeabraLayer*)recv_gp->prjn->from;
      if(fmlay->spec.spec->InheritsFrom(TA_SNcLayerSpec)) da_lay = fmlay;
      if(fmlay->spec.spec->InheritsFrom(TA_SNrThalLayerSpec)) snr_lay = fmlay;
      continue;
    }
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
    if(bg_type == MatrixLayerSpec::MOTOR) {
      if((cs->learn_rule != MatrixConSpec::MOTOR_DELTA) && (cs->learn_rule != MatrixConSpec::MOTOR_CHL)) {
	cs->SetUnique("learn_rule", true);
	cs->learn_rule = MatrixConSpec::MOTOR_DELTA;
	if(!quiet) taMisc::Error("MatrixLayerSpec: MOTOR BG requires MatrixConSpec learn_rule of MOTOR type, I just set it for you in spec:",
				 cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
    else {			// pfc
      if((cs->learn_rule == MatrixConSpec::MOTOR_DELTA) || (cs->learn_rule == MatrixConSpec::MOTOR_CHL)) {
	cs->SetUnique("learn_rule", true);
	cs->learn_rule = MatrixConSpec::PFC;
	if(!quiet) taMisc::Error("MatrixLayerSpec: BG_pfc requires MatrixConSpec learn_rule of PFC type, I just set it for you in spec:",
				 cs->name,"(make sure this is appropriate for all layers that use this spec!)");
      }
    }
  }
  if(da_lay == NULL) {
    taMisc::Error("MatrixLayerSpec: Could not find DA layer (PVLVDaLayerSpec, VTA or SNc) -- must receive MarkerConSpec projection from one!");
    return false;
  }
  if(snr_lay == NULL) {
    taMisc::Error("MatrixLayerSpec: Could not find SNrThal layer -- must receive MarkerConSpec projection from one!");
    return false;
  }
  // vta/snc must be before matrix!  good.
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int daidx = lay->own_net->layers.FindLeaf(da_lay);
  if(daidx > myidx) {
    taMisc::Error("MatrixLayerSpec: DA layer (PVLVDaLayerSpec, VTA or SNc) must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }
  return true;
}

void MatrixLayerSpec::InitWtState(LeabraLayer* lay) {
  LeabraLayerSpec::InitWtState(lay);
  UNIT_GP_ITR(lay, 
	      DaModUnit* u = (DaModUnit*)ugp->FastEl(0);
	      u->misc_1 = rnd_go_thr.abs_max;	// initialize to above rnd go val..
	      );
}

void MatrixLayerSpec::Compute_RndGo(LeabraLayer* lay, LeabraTrial* trl) {
  float avg_rew = -1.0f;

  // first try to use the stat value
  EpochProcess* epc = trl->GetMyEpochProc();
  if((epc != NULL) && (epc->super_proc != NULL)) {
    SchedProcess* sp = epc->super_proc;
    ExtRew_Stat* ars = (ExtRew_Stat*)sp->loop_stats.FindType(&TA_ExtRew_Stat);
    if(ars != NULL) {
      if(ars->time_agg.op != Aggregate::LAST) {
	ars->time_agg.op = Aggregate::LAST;
      }
      avg_rew = ars->rew.val;
    }
  }
  // if in a test process, don't do random go's!
  if((epc != NULL) && (avg_rew == -1.0f) && (epc->wt_update == EpochProcess::TEST)) {
    avg_rew = 1.0f;
  }

  if(avg_rew == -1.0f) {	// didn't get from stat, use value on layer
    LeabraLayer* er_lay = FindLayerFmSpecNet(lay->own_net, &TA_ExtRewLayerSpec);
    if(er_lay != NULL) {
      LeabraUnit* un = (LeabraUnit*)er_lay->units.Leaf(0);
      avg_rew = un->act_avg;
    }
  }

  // never do anything if already doing well!!!
  if(avg_rew >= rnd_go_thr.avgrew) return;

  int gi;
  for(gi=0; gi<lay->units.gp.size; gi++) {
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp[gi];

    // compute random performance go's: do this first so it can be overwritten..
    if(rnd_go.on && (Random::ZeroOne() < rnd_go.prf_go_p)) {
      mugp->misc_state1 = PFCGateSpec::PERF_RND_GO;
    }

    // check for over max..
    if((int)fabs((float)mugp->misc_state) > matrix.max_mnt) {
      if(Random::ZeroOne() < rnd_go.go_p) {
	mugp->misc_state1 = PFCGateSpec::RANDOM_GO;
      }
    }
  }

  if(!rnd_go.on) return;

  float avg_agd = 0.0f;
  for(gi=0; gi<lay->units.gp.size; gi++) {
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp[gi];
    LeabraUnit* u = (LeabraUnit*)mugp->FastEl(0);
    float agd = u->misc_1;
    avg_agd += agd;
  }
  avg_agd /= (float)lay->units.gp.size;

  for(gi=0; gi<lay->units.gp.size; gi++) {
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp[gi];
    LeabraUnit* u = (LeabraUnit*)mugp->FastEl(0);

    if((int)fabs((float)mugp->misc_state) < rnd_go_thr.mnt) continue; // must be this duration

    float agd = u->misc_1;
    if(agd > rnd_go_thr.abs_max) continue;

    if((agd < rnd_go_thr.abs_thr) || ((agd - avg_agd) < -rnd_go_thr.rel)) {
      // now, we meet the critera: fire a random go!
      if(Random::ZeroOne() < rnd_go.go_p) {
	mugp->misc_state1 = PFCGateSpec::RANDOM_GO;
	u->misc_1 = MIN(rnd_go_thr.abs_max, avg_agd);	// reset to above-impunity
      }
    }
  }
}

void MatrixLayerSpec::Compute_ClearRndGo(LeabraLayer* lay, LeabraTrial*) {
  int gi;
  for(gi=0; gi<lay->units.gp.size; gi++) {
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp[gi];
    if((mugp->misc_state1 == PFCGateSpec::RANDOM_GO) ||
       (mugp->misc_state1 == PFCGateSpec::PERF_RND_GO))
      mugp->misc_state1 = PFCGateSpec::EMPTY_GO;
  }
}

void MatrixLayerSpec::Compute_DaModUnit_NoContrast(DaModUnit* u, float dav, int go_no) {
  if(go_no == (int)PFCGateSpec::GATE_GO) {	// we are a GO gate unit
    if(dav >= 0.0f)  { 
      u->vcb.g_h = dav;
      u->vcb.g_a = 0.0f;
    }
    else {
      u->vcb.g_h = 0.0f;
      u->vcb.g_a = -dav;
    }
  }
  else {			// we are a NOGO gate unit
    if(dav >= 0.0f) {
      u->vcb.g_h = 0.0f;
      u->vcb.g_a = dav;
    }
    else {
      u->vcb.g_h = -dav;
      u->vcb.g_a = 0.0f;
    }
  }
}


void MatrixLayerSpec::Compute_DaModUnit_Contrast(DaModUnit* u, float dav, float act_val, int go_no) {
  if(go_no == (int)PFCGateSpec::GATE_GO) {	// we are a GO gate unit
    if(dav >= 0.0f)  { 
      u->vcb.g_h = contrast.gain * dav * ((1.0f - contrast.go_p) + (contrast.go_p * act_val));
      u->vcb.g_a = 0.0f;
    }
    else {
      u->vcb.g_h = 0.0f;
      u->vcb.g_a = -matrix.neg_gain * contrast.gain * dav * ((1.0f - contrast.go_n) + (contrast.go_n * act_val));
    }
  }
  else {			// we are a NOGO gate unit
    if(dav >= 0.0f) {
      u->vcb.g_h = 0.0f;
      u->vcb.g_a = contrast.gain * dav * ((1.0f - contrast.nogo_p) + (contrast.nogo_p * act_val));
    }
    else {
      u->vcb.g_h = -matrix.neg_gain * contrast.gain * dav * ((1.0f - contrast.nogo_n) + (contrast.nogo_n * act_val));
      u->vcb.g_a = 0.0f;
    }
  }
}

void MatrixLayerSpec::Compute_DaTonicMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib*, LeabraTrial*) {
  int da_prjn_idx;
  LeabraLayer* da_lay = FindLayerFmSpec(lay, da_prjn_idx, &TA_SNcLayerSpec);
  PVLVDaLayerSpec* dals = (PVLVDaLayerSpec*)da_lay->spec.spec;
  float dav = contrast.gain * dals->da.tonic_da;
  int idx = 0;
  DaModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(DaModUnit, u, mugp->, i) {
    PFCGateSpec::GateSignal go_no = (PFCGateSpec::GateSignal)(idx % 2); // GO = 0, NOGO = 1
    Compute_DaModUnit_NoContrast(u, dav, go_no);
    idx++;
  }
}

void MatrixLayerSpec::Compute_DaPerfMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib*, LeabraTrial*) {
  int da_prjn_idx;
  LeabraLayer* da_lay = FindLayerFmSpec(lay, da_prjn_idx, &TA_SNcLayerSpec);
  PVLVDaLayerSpec* dals = (PVLVDaLayerSpec*)da_lay->spec.spec;
  float tonic_da = dals->da.tonic_da;

  int idx = 0;
  DaModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(DaModUnit, u, mugp->, i) {
    PFCGateSpec::GateSignal go_no = (PFCGateSpec::GateSignal)(idx % 2); // GO = 0, NOGO = 1

    // need to separate out the tonic and non-tonic because tonic contributes with contrast.gain
    // but perf is down-modulated by matrix.perf_gain..
    float non_tonic = u->dav - tonic_da;
    float dav = contrast.gain * (tonic_da + matrix.perf_gain * non_tonic);
//     if(mugp->misc_state1 == PFCGateSpec::RANDOM_GO) {
//       dav += matrix.rgo_da_prf;
//     }
    Compute_DaModUnit_NoContrast(u, dav, go_no);
    idx++;
  }
}

void MatrixLayerSpec::Compute_DaLearnMod(LeabraLayer* lay, LeabraUnit_Group* mugp, LeabraInhib*, LeabraTrial* trl) {
  int snr_prjn_idx = 0;
  FindLayerFmSpec(lay, snr_prjn_idx, &TA_SNrThalLayerSpec);

  PFCGateSpec::GateSignal gate_sig = (PFCGateSpec::GateSignal)mugp->misc_state2;
    
  int idx = 0;
  DaModUnit* u;
  taLeafItr i;
  FOR_ITR_EL(DaModUnit, u, mugp->, i) {
    PFCGateSpec::GateSignal go_no = (PFCGateSpec::GateSignal)(idx % 2); // GO = 0, NOGO = 1
    LeabraCon_Group* snrcg = (LeabraCon_Group*)u->recv.gp[snr_prjn_idx];
    DaModUnit* snrsu = (DaModUnit*)snrcg->Un(0);

    float gating_act = 0.0f;	// activity of the unit during the gating action firing
    float snrthal_act = 0.0f;	// activity of the snrthal during gating action firing
    if(trl->phase_no == 3) 	{ gating_act = u->act_m2; snrthal_act = snrsu->act_m2; } // TRANS
    else if(trl->phase_no == 2) { gating_act = u->act_p;  snrthal_act = snrsu->act_p; }	// GOGO
    else if(trl->phase_no == 1)	{ gating_act = u->act_m;  snrthal_act = snrsu->act_m; }	// MOTOR

    if(gate_sig == PFCGateSpec::GATE_NOGO)	// if didn't actually GO (act > thresh), then no learning!
      snrthal_act = 0.0f;

    float dav = snrthal_act * u->dav - matrix.neg_da_bl; // da is modulated by snrthal; sub baseline
    if(mugp->misc_state1 == PFCGateSpec::RANDOM_GO) {
      dav += rnd_go.rgo_da; 
    }
    if(mugp->misc_state1 == PFCGateSpec::PERF_RND_GO) {
      dav += rnd_go.prf_rgo_da; 
    }
    u->dav = dav;		// make it show up in display
    Compute_DaModUnit_Contrast(u, dav, gating_act, go_no);
    idx++;
  }
}

void MatrixLayerSpec::Compute_AvgGODA(LeabraLayer* lay, LeabraTrial*) {
  int snc_prjn_idx = 0;
  FindLayerFmSpec(lay, snc_prjn_idx, &TA_SNcLayerSpec);

  int gi;
  for(gi=0; gi<lay->units.gp.size; gi++) {
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)lay->units.gp[gi];
    PFCGateSpec::GateSignal gate_sig = (PFCGateSpec::GateSignal)mugp->misc_state2;
    if(gate_sig == PFCGateSpec::GATE_NOGO) continue; // no action

    DaModUnit* u = (DaModUnit*)mugp->FastEl(0);
    LeabraCon_Group* snccg = (LeabraCon_Group*)u->recv.gp[snc_prjn_idx];
    DaModUnit* sncsu = (DaModUnit*)snccg->Un(0);
    float raw_da = sncsu->dav;	// need to use raw da here because otherwise negatives don't show up!!

    u->misc_1 += matrix.avg_go_dt * (raw_da - u->misc_1);
    // copy value to other units, to make it easier to monitor value using unit group monitor stat!
    for(int i=1;i<mugp->size;i++) {
      DaModUnit* nu = (DaModUnit*)mugp->FastEl(i);
      nu->misc_1 = u->misc_1;
    }
  }
}

void MatrixLayerSpec::Compute_Act_impl(LeabraLayer* lay, Unit_Group* ug, LeabraInhib* thr, LeabraTrial* trl) {
  LeabraUnit_Group* mugp = (LeabraUnit_Group*)ug;
  if(bg_type == MatrixLayerSpec::PFC) {
    if(trl->phase_no == 0)
      Compute_DaTonicMod(lay, mugp, thr, trl);
    else if(trl->phase_no == 1)
      Compute_DaPerfMod(lay, mugp, thr, trl);
    else if(trl->phase_no == 2)
      Compute_DaLearnMod(lay, mugp, thr, trl);
  }
  else {			// MOTOR
    if(trl->phase_no == 0)	// todo: could mod by da even here!
      Compute_DaTonicMod(lay, mugp, thr, trl);
    else
      Compute_DaLearnMod(lay, mugp, thr, trl);
  }
  LeabraLayerSpec::Compute_Act_impl(lay, ug, thr, trl);
}

void MatrixLayerSpec::Compute_HardClamp(LeabraLayer* lay, LeabraTrial* trl) {
  if(bg_type == MatrixLayerSpec::PFC) {
    if(trl->phase_no == 0)
      Compute_ClearRndGo(lay, trl);
    else if(trl->phase_no == 1)
      Compute_RndGo(lay, trl);
  }
  LeabraLayerSpec::Compute_HardClamp(lay, trl);
}

void MatrixLayerSpec::PostSettle(LeabraLayer* lay, LeabraTrial* trl, bool set_both) {
  LeabraLayerSpec::PostSettle(lay, trl, set_both);

  if(trl->phase_no == 2) {
    Compute_AvgGODA(lay, trl);
  }
}

void MatrixLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if(bg_type == MatrixLayerSpec::MOTOR) {
    if((trl->phase_no.max > 2) && (trl->phase_no != 1))
      return;
  }
  else {
    if(trl->phase_no.val < trl->phase_no.max-1)	// only final dwt!
      return;
  }
  LeabraLayerSpec::Compute_dWt(lay, trl);
}

//////////////////////////////////
//	SNrThal Layer Spec	//
//////////////////////////////////

void SNrThalLayerSpec::Initialize() {
  SetUnique("decay", true);
  decay.clamp_phase2 = false;
  SetUnique("kwta", true);
  kwta.k_from = KWTASpec::USE_PCT;
  kwta.pct = .75f;
  SetUnique("inhib_group", true);
  inhib_group = ENTIRE_LAYER;
  SetUnique("compute_i", true);
  compute_i = KWTA_AVG_INHIB;
  SetUnique("i_kwta_pt", true);
  i_kwta_pt = .6f;

  avg_net_dt = .005f;
}

void SNrThalLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
}

void SNrThalLayerSpec::Defaults() {
  LeabraLayerSpec::Defaults();
  Initialize();
}

void SNrThalLayerSpec::HelpConfig() {
  String help = "SNrThalLayerSpec Computation:\n\
 - act of unit(s) = act_dif of unit(s) in reward integration layer we recv from\n\
 - da is dynamically computed in plus phaes and sent all layers that recv from us\n\
 - No Learning\n\
 \nSNrThalLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
 - Single recv connection marked with a MarkerConSpec from reward integration layer\
     (computes expectations and actual reward signals)\n\
 - This layer must be after corresp. reward integration layer in list of layers\n\
 - Sending connections must connect to units of type DaModUnit/Spec \
     (da signal from this layer put directly into da var on units)\n\
 - UnitSpec for this layer must have act_range and clamp_range set to -1 and 1 \
     (because negative da = negative activation signal here";
  cerr << help << endl << flush;
  taMisc::Choice(help, "Ok");
}

bool SNrThalLayerSpec::CheckConfig(LeabraLayer* lay, LeabraTrial* trl, bool quiet) {
  if(!LeabraLayerSpec::CheckConfig(lay, trl, quiet)) return false;

  SetUnique("decay", true);
  decay.clamp_phase2 = false;

  if(trl->trial_init != LeabraTrial::DECAY_STATE) {
    if(!quiet) taMisc::Error("SNrThalLayerSpec: requires LeabraTrial trial_init = DECAY_STATE, I just set it for you");
    trl->trial_init = LeabraTrial::DECAY_STATE;
  }

  // must have the appropriate ranges for unit specs..
  //  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;

  // check recv connection
  int mtx_prjn_idx = 0;
  LeabraLayer* matrix_lay = FindLayerFmSpec(lay, mtx_prjn_idx, &TA_MatrixLayerSpec);

  if(matrix_lay == NULL) {
    taMisc::Error("SNrThalLayerSpec: did not find Matrix layer to recv from!");
    return false;
  }

  if(matrix_lay->units.gp.size != lay->units.gp.size) {
    taMisc::Error("SNrThalLayerSpec: MatrixLayer unit groups must = SNrThalLayer unit groups!");
    lay->geom.z = matrix_lay->units.gp.size;
    return false;
  }

  int myidx = lay->own_net->layers.FindLeaf(lay);
  int matidx = lay->own_net->layers.FindLeaf(matrix_lay);
  if(matidx > myidx) {
    taMisc::Error("SNrThalLayerSpec: Matrix layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }
  return true;
}

void SNrThalLayerSpec::Compute_GoNogoNet(LeabraLayer* lay, LeabraTrial*) {
  int mtx_prjn_idx = 0;
  LeabraLayer* matrix_lay = FindLayerFmSpec(lay, mtx_prjn_idx, &TA_MatrixLayerSpec);

  int mg;
  for(mg=0; mg<lay->units.gp.size; mg++) {
    LeabraUnit_Group* rugp = (LeabraUnit_Group*)lay->units.gp[mg];
    float gonogo = 0.0f;
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)matrix_lay->units.gp[mg];
    MatrixUnitSpec* us = (MatrixUnitSpec*)matrix_lay->unit_spec.spec;
    if((mugp->size > 0) && (mugp->acts.max >= us->opt_thresh.send)) {
      float sum_go = 0.0f;
      float sum_nogo = 0.0f;
      for(int i=0;i<mugp->size;i++) {
	DaModUnit* u = (DaModUnit*)mugp->FastEl(i);
	PFCGateSpec::GateSignal go_no = (PFCGateSpec::GateSignal)(i % 2); // GO = 0, NOGO = 1
	if(go_no == PFCGateSpec::GATE_GO)
	  sum_go += u->act_eq;
	else
	  sum_nogo += u->act_eq;
      }
      if(sum_go + sum_nogo > 0.0f) {
	gonogo = (sum_go - sum_nogo) / (sum_go + sum_nogo);
      }
    }
    for(int i=0;i<rugp->size;i++) {
      DaModUnit* ru = (DaModUnit*)rugp->FastEl(i);
      ru->net = gonogo;
    }
  }
}

void SNrThalLayerSpec::Compute_Clamp_NetAvg(LeabraLayer* lay, LeabraTrial* trl) {
  Compute_GoNogoNet(lay, trl);
  LeabraLayerSpec::Compute_Clamp_NetAvg(lay, trl);
}

void SNrThalLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial*) {
  DaModUnit* u = (DaModUnit*)lay->units.Leaf(0);
  float cur_avg = u->misc_1;
  float new_avg = cur_avg + avg_net_dt * (lay->netin.avg - cur_avg);
  taLeafItr ui;
  FOR_ITR_EL(DaModUnit, u, lay->units., ui) {
    u->misc_1 = new_avg;
  }
}


//////////////////////////////////
//	PFC Layer Spec		//
//////////////////////////////////

void PFCGateSpec::Initialize() {
  go_thr = 0.1f;
  off_accom = 0.0f;
  updt_reset_sd = true;
  snr_tie_thr = 0.2f;
  snr_tie_max = false;
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
  gate.Initialize();
  Initialize();
}

void PFCLayerSpec::InitLinks() {
  LeabraLayerSpec::InitLinks();
  taBase::Own(gate, this);
}

void PFCLayerSpec::HelpConfig() {
  String help = "PFCLayerSpec Computation:\n\
 The PFC maintains activation over time (activation-based working memory) via\
 excitatory intracelluar ionic mechanisms (implemented via the hysteresis channels, gc.h),\
 and excitatory self-connections. These ion channels are toggled on and off via units in the\
 MatrixLayerSpec layer, which are themselves trained up by other basal ganglia (BG) layers.\
 Updating occurs at the end of the 1st plus phase --- if a gating signal was activated, any previous ion\
 current is turned off, and the units are allowed to settle into a new state in the 2nd plus --\
 then the ion channels are activated in proportion to activations at the end of this 2nd phase.\n\
 \nPFCLayerSpec Configuration:\n\
 - Use the Wizard BG_PFC button to automatically configure BG_PFC layers.\n\
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
    decay.event = 0.0f;
    decay.phase = 0.0f;
    decay.phase2 = 0.1f;
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
  if(trl->no_plus_test) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraTrial no_plus_test = false, I just set it for you");
    trl->no_plus_test = false;
  }

  ExtRew_Stat* ers = (ExtRew_Stat*)trl->final_stats.FindType(&TA_ExtRew_Stat);
  if(ers == NULL) {
    if(!quiet) taMisc::Error("PFCLayerSpec: ExtRew_Stat not found, I just made one for you in TrialProcess:", trl->name);
    ers = (ExtRew_Stat*)trl->final_stats.NewEl(1, &TA_ExtRew_Stat);
    ers->CreateAggregates(Aggregate::AVG);
    ers->UpdateAfterEdit();
    EpochProcess* epc = trl->GetMyEpochProc();
    ExtRew_Stat* eers = (ExtRew_Stat*)epc->loop_stats.FindType(&TA_ExtRew_Stat);
    if(eers != NULL) eers->time_agg.op = Aggregate::AVG;
  }

  LeabraSettle* set = (LeabraSettle*)trl->FindSubProc(&TA_LeabraSettle);
  if(set->min_cycles_phase2 < 35) {
    if(!quiet) taMisc::Error("PFCLayerSpec: requires LeabraSettle min_cycles_phase2 >= 35, I just set it for you");
    set->min_cycles_phase2 = 35;
  }

  SequenceProcess* seq = (SequenceProcess*)trl->FindSuperProc(&TA_SequenceProcess);
  if(seq != NULL) {
    if(seq->sequence_init != SequenceProcess::DO_NOTHING) {
      if(!quiet) taMisc::Error("PFCLayerSpec: requires SequenceProcess sequence_init = DO_NOTHING, I just set it for you");
      seq->sequence_init = SequenceProcess::DO_NOTHING;
    }
  }

  if(!lay->units.el_typ->InheritsFrom(TA_DaModUnit)) {
    taMisc::Error("PFCLayerSpec: must have DaModUnits!");
    return false;
  }

  LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
  if(!us->InheritsFrom(TA_DaModUnitSpec)) {
    taMisc::Error("PFCLayerSpec: UnitSpec must be DaModUnitSpec!");
    return false;
  }

  if(us->act.avg_dt <= 0.0f) {
    us->SetUnique("act", true);
    us->act.avg_dt = 0.005f;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec act.avg_dt > 0, I just set it to .005 for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  us->SetUnique("g_bar", true);
  if(us->hyst.init) {
    us->SetUnique("hyst", true);
    us->hyst.init = false;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec hyst.init = false, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
  }
  if(us->acc.init) {
    us->SetUnique("acc", true);
    us->acc.init = false;
    if(!quiet) taMisc::Error("PFCLayerSpec: requires UnitSpec acc.init = false, I just set it for you in spec:",
		  us->name,"(make sure this is appropriate for all layers that use this spec!)");
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
    LeabraConSpec* cs = (LeabraConSpec*)recv_gp->spec.spec;
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

  int snrthal_prjn_idx;
  LeabraLayer* snrthal_lay = FindLayerFmSpec(lay, snrthal_prjn_idx, &TA_SNrThalLayerSpec);
  if(snrthal_lay == NULL) {
    taMisc::Error("*** PFCLayerSpec: Warning: no projection from SNrThal Layer found: must have MarkerConSpec!");
    return false;
  }
  if(snrthal_lay->units.gp.size != lay->units.gp.size) {
    taMisc::Error("PFCLayerSpec: Gating Layer unit groups must = PFCLayer unit groups!");
    snrthal_lay->geom.z = lay->units.gp.size;
    return false;
  }

  // check for ordering of layers!
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int gateidx = lay->own_net->layers.FindLeaf(snrthal_lay);
  if(gateidx > myidx) {
    taMisc::Error("PFCLayerSpec: SNrThal Layer must be *before* this layer in list of layers -- it is now after, won't work");
    return false;
  }

  return true;
}

void PFCLayerSpec::ResetSynDep(LeabraUnit* u, LeabraLayer*, LeabraTrial*) {
  LeabraCon_Group* send_gp;
  int g;
  FOR_ITR_GP(LeabraCon_Group, send_gp, u->send., g) {
    if(!send_gp->spec.spec->InheritsFrom(TA_TrialSynDepConSpec)) continue;
    TrialSynDepConSpec* cs = (TrialSynDepConSpec*)send_gp->spec.spec;
    cs->Reset_EffWt(send_gp);
  }
}


void PFCLayerSpec::Compute_MaintUpdt(LeabraUnit_Group* ugp, MaintUpdtAct updt_act, LeabraLayer* lay, LeabraTrial* trl) {
  if(updt_act == NO_UPDT) return;
  for(int j=0;j<ugp->size;j++) {
    LeabraUnit* u = (LeabraUnit*)ugp->FastEl(j);
    LeabraUnitSpec* us = (LeabraUnitSpec*)u->spec.spec;
    if(updt_act == STORE) {
      u->vcb.g_h = u->misc_1 = u->act_eq;
      u->vcb.g_a = 0.0f;
      if(gate.updt_reset_sd)
	ResetSynDep(u, lay, trl);
    }
    else if(updt_act == CLEAR) {
      u->vcb.g_a = gate.off_accom * u->vcb.g_h;
      u->vcb.g_h = u->misc_1 = 0.0f;
      if(gate.updt_reset_sd)
	ResetSynDep(u, lay, trl);
    }
    else if(updt_act == RESTORE) {
      u->vcb.g_h = u->act_eq = u->misc_1;
      u->vcb.g_a = 0.0f;
    }
    else if(updt_act == TMP_STORE) {
      u->vcb.g_h = u->act_eq;
      u->vcb.g_a = 0.0f;
    }
    else if(updt_act == TMP_CLEAR) {
      u->vcb.g_a = u->vcb.g_h = 0.0f;
    }
    us->Compute_Conduct(u, lay, (LeabraInhib*)ugp, trl); // update displayed conductances!
  }
  if(updt_act == STORE) ugp->misc_state = 1;
  else if(updt_act == CLEAR) ugp->misc_state = 0;
}

void PFCLayerSpec::Compute_TmpClear(LeabraLayer* lay, LeabraTrial* trl) {
  for(int mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* ugp = (LeabraUnit_Group*)lay->units.gp[mg];
    Compute_MaintUpdt(ugp, TMP_CLEAR, lay, trl); // temporary clear for trans input!
  }
}

void PFCLayerSpec::Compute_GatingTrans(LeabraLayer* lay, LeabraTrial* trl) {
  int snrthal_prjn_idx;
  LeabraLayer* snrthal_lay = FindLayerFmSpec(lay, snrthal_prjn_idx, &TA_SNrThalLayerSpec);

  // todo: this needs to be updated from GOGO if ever used!

  for(int mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* ugp = (LeabraUnit_Group*)lay->units.gp[mg];
    LeabraUnit_Group* snrgp = (LeabraUnit_Group*)snrthal_lay->units.gp[mg];
    DaModUnit* snru = (DaModUnit*)snrgp->Leaf(0);

    PFCGateSpec::GateSignal gate_sig = PFCGateSpec::GATE_NOGO;
    if(snru->act_eq > gate.go_thr)
      gate_sig = PFCGateSpec::GATE_GO;

    ugp->misc_state2 = gate_sig; // store the raw gating signal itself

    if(ugp->misc_state <= 0) { // empty stripe
      if(gate_sig == PFCGateSpec::GATE_GO) {
	ugp->misc_state1 = PFCGateSpec::EMPTY_GO;
	Compute_MaintUpdt(ugp, STORE, lay, trl);
      }
      else {
	ugp->misc_state1 = PFCGateSpec::EMPTY_NOGO;
	ugp->misc_state--;	// more time off
      }
    }
    else {			// latched stripe
      if(gate_sig == PFCGateSpec::GATE_GO) {
	ugp->misc_state1 = PFCGateSpec::LATCH_GO;
	Compute_MaintUpdt(ugp, STORE, lay, trl); // GO is always store for trans: love it!
      }
      else {
	Compute_MaintUpdt(ugp, RESTORE, lay, trl); // restore stored values after trans!
	ugp->misc_state1 = PFCGateSpec::LATCH_NOGO;
	ugp->misc_state++;  // keep on mainting
      }
    }
  }
  SendGateStates(lay, trl);
}

void PFCLayerSpec::Compute_GatingGOGO(LeabraLayer* lay, LeabraTrial* trl) {
  static int_Array tie_sort_list;

  int snrthal_prjn_idx;
  LeabraLayer* snrthal_lay = FindLayerFmSpec(lay, snrthal_prjn_idx, &TA_SNrThalLayerSpec);
  int mtx_prjn_idx;
  LeabraLayer* matrix_lay = FindLayerFmSpec(snrthal_lay, mtx_prjn_idx, &TA_MatrixLayerSpec);

  if((snrthal_lay->acts.max < gate.go_thr) && (snrthal_lay->netin.avg > gate.snr_tie_thr)) {
    // nobody is above threshold, but there is a relatively high average netinput,
    // suggesting tie-based deadlock..
    if(trl->phase_no == 1) {
      if(gate.snr_tie_max) {
	// use the existing active_buf list on snrthal layer to pick top k guys..
	int max_idx = MIN(snrthal_lay->kwta.k, snrthal_lay->active_buf.size);
	for(int ui = 0; ui < max_idx; ui++) {
	  DaModUnit* snru = (DaModUnit*)snrthal_lay->active_buf[ui];
	  snru->act_eq = snru->act_p = .95f; // make them very active: encourage learning!
	}
      }
      else {
	// select k of the top guys at random..
	tie_sort_list.Reset();
	for(int ui = 0; ui < snrthal_lay->units.leaves; ui++) {
	  DaModUnit* snru = (DaModUnit*)snrthal_lay->units.Leaf(ui);
	  if(snru->net > gate.snr_tie_thr) 
	    tie_sort_list.Add(ui);
	}
	int max_idx = MIN(snrthal_lay->kwta.k, tie_sort_list.size);
	tie_sort_list.Permute();
	for(int ui = 0; ui < max_idx; ui++) {
	  DaModUnit* snru = (DaModUnit*)snrthal_lay->units.Leaf(tie_sort_list[ui]);
	  snru->act_eq = snru->act_p = .95f; // make them very active: encourage learning!
	}
      }
    }
    else {			//  phase == 2
      // find the guys from previous phase that had act_p > go_thr
      for(int ui = 0; ui < snrthal_lay->units.leaves; ui++) {
	DaModUnit* snru = (DaModUnit*)snrthal_lay->units.Leaf(ui);
	if(snru->act_p > gate.go_thr) 
	  snru->act_eq = snru->act_p2 = snru->act_p;
      }
    }
  }

  for(int mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* ugp = (LeabraUnit_Group*)lay->units.gp[mg];
    LeabraUnit_Group* snrgp = (LeabraUnit_Group*)snrthal_lay->units.gp[mg];
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)matrix_lay->units.gp[mg];
    DaModUnit* snru = (DaModUnit*)snrgp->Leaf(0);

    // random go must always actually go!  makes it contingent..
    PFCGateSpec::GateSignal gate_sig = PFCGateSpec::GATE_NOGO;
    if((snru->act_eq > gate.go_thr) || (mugp->misc_state1 == PFCGateSpec::RANDOM_GO)
       || (mugp->misc_state1 == PFCGateSpec::PERF_RND_GO))
      gate_sig = PFCGateSpec::GATE_GO;

    ugp->misc_state2 = gate_sig; // store the raw gating signal itself

    if(trl->phase_no == 1) {
      if(ugp->misc_state <= 0) { // empty stripe
	if(gate_sig == PFCGateSpec::GATE_GO) {
	  ugp->misc_state1 = PFCGateSpec::EMPTY_GO;
	  Compute_MaintUpdt(ugp, STORE, lay, trl);
	}
	else {
	  ugp->misc_state1 = PFCGateSpec::EMPTY_NOGO;
	  ugp->misc_state--;	// more time off
	}
      }
      else {			// latched stripe
	if(gate_sig == PFCGateSpec::GATE_GO) {
	  ugp->misc_state1 = PFCGateSpec::LATCH_GO;
	  Compute_MaintUpdt(ugp, CLEAR, lay, trl); // clear in first phase
	}
	else {
	  ugp->misc_state1 = PFCGateSpec::LATCH_NOGO;
	  ugp->misc_state++;  // keep on mainting
	}
      }
    }
    else {			// second plus (2m)
      if(ugp->misc_state <= 0) {
	if(gate_sig == PFCGateSpec::GATE_GO) {
	  if(ugp->misc_state1 == PFCGateSpec::LATCH_GO)
	    ugp->misc_state1 = PFCGateSpec::LATCH_GOGO;
	  else
	    ugp->misc_state1 = PFCGateSpec::EMPTY_GO;
	  Compute_MaintUpdt(ugp, STORE, lay, trl);
	}
      }
    }
  }
  SendGateStates(lay, trl);
}

void PFCLayerSpec::SendGateStates(LeabraLayer* lay, LeabraTrial*) {
  int snrthal_prjn_idx;
  LeabraLayer* snrthal_lay = FindLayerFmSpec(lay, snrthal_prjn_idx, &TA_SNrThalLayerSpec);
  int mtx_prjn_idx = 0;
  LeabraLayer* matrix_lay = FindLayerFmSpec(snrthal_lay, mtx_prjn_idx, &TA_MatrixLayerSpec);
  int mg;
  for(mg=0;mg<lay->units.gp.size;mg++) {
    LeabraUnit_Group* ugp = (LeabraUnit_Group*)lay->units.gp[mg];
    LeabraUnit_Group* snrgp = (LeabraUnit_Group*)snrthal_lay->units.gp[mg];
    LeabraUnit_Group* mugp = (LeabraUnit_Group*)matrix_lay->units.gp[mg];
    // everybody gets gate state info from PFC!
    snrgp->misc_state = mugp->misc_state = ugp->misc_state;
    snrgp->misc_state1 = ugp->misc_state1; 
    if((mugp->misc_state1 != PFCGateSpec::RANDOM_GO) && (mugp->misc_state1 != PFCGateSpec::PERF_RND_GO)) { // don't override this signal!
      mugp->misc_state1 = ugp->misc_state1;
    }
    snrgp->misc_state2 = mugp->misc_state2 = ugp->misc_state2;
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

  if(trl->phase_no >= 1) {
    Compute_GatingGOGO(lay, trl);	// do gating
  }
}

void PFCLayerSpec::Compute_dWt(LeabraLayer* lay, LeabraTrial* trl) {
  if((trl->phase_no.max > 2) && (trl->phase_no != 1))
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
//			SRN Context
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

///////////////////////////////////////////////////////////////
//			TD
///////////////////////////////////////////////////////////////

// todo: set td_mod.on = true for td_mod_all; need to get UnitSpec..

void LeabraWiz::TD(Network* net, bool bio_labels, bool td_mod_all) {
  String msg = "Configuring TD Temporal Differences Layers:\n\n\
 There is one thing you will need to check manually after this automatic configuration\
 process completes (this note will be repeated when things complete --- there may be some\
 messages in the interim):\n\n";

  String man_msg = "1. Check that connection(s) were made from all appropriate output layers\
 to the ExtRew layer, using the MarkerConSpec (MarkerCons) Con spec.\
 This will provide the error signal to the system based on output error performance.\n\n";

  msg += man_msg + "\n\nThe configuration will now be checked and a number of default parameters\
 will be set.  If there are any actual errors which must be corrected before\
 the network will run, you will see a message to that effect --- you will then need to\
 re-run this configuration process to make sure everything is OK.  When you press\
 Re/New/Init on the control process these same checks will be performed, so you\
 can be sure everything is ok.";
  taMisc::Choice(msg,"Ok");

  Project* proj = GET_MY_OWNER(Project);
  net->RemoveUnits();

  //////////////////////////////////////////////////////////////////////////////////
  // make layers

  bool	abl_new = false;
  String ablnm = "ABL";  String nacnm = "NAc";  String vtanm = "VTA";
  if(!bio_labels) {
    ablnm = "TDRewPred";    nacnm = "TDRewInteg";    vtanm = "TD";
  }

  LeabraLayer* rew_targ_lay = (LeabraLayer*)net->FindMakeLayer("RewTarg");
  LeabraLayer* extrew = (LeabraLayer*)net->FindMakeLayer("ExtRew");
  LeabraLayer* abl = (LeabraLayer*)net->FindMakeLayer(ablnm, NULL, abl_new);
  LeabraLayer* nac = (LeabraLayer*)net->FindMakeLayer(nacnm);
  LeabraLayer* vta = (LeabraLayer*)net->FindMakeLayer(vtanm);
  if(rew_targ_lay == NULL || abl == NULL || extrew == NULL || nac == NULL || vta == NULL) return;
  if(abl_new) {
    extrew->pos.z = 0; extrew->pos.y = 4; extrew->pos.x = 0;
    abl->pos.z = 0; abl->pos.y = 2; abl->pos.x = 0;
    nac->pos.z = 0; nac->pos.y = 0; nac->pos.x = 0;
    vta->pos.z = 0; vta->pos.y = 4; vta->pos.x = 10;
  }

  //////////////////////////////////////////////////////////////////////////////////
  // sort layers

  rew_targ_lay->name = "0000";  extrew->name = "0001"; abl->name = "0002";  
  nac->name = "0003";  vta->name = "0004";

  net->layers.Sort();

  rew_targ_lay->name = "RewTarg";  extrew->name = "ExtRew"; abl->name = ablnm; 
  nac->name = nacnm;  vta->name = vtanm;

  //////////////////////////////////////////////////////////////////////////////////
  // collect layer groups

  Layer_MGroup other_lays;
  Layer_MGroup hidden_lays;
  Layer_MGroup output_lays;
  Layer_MGroup input_lays;
  int i;
  for(i=0;i<net->layers.size;i++) {
    LeabraLayer* lay = (LeabraLayer*)net->layers[i];
    LeabraLayerSpec* laysp = (LeabraLayerSpec*)lay->spec.spec;
    lay->SetUnitType(&TA_DaModUnit);
    // todo: add any new bg layer exclusions here!
    if(lay != rew_targ_lay && lay != abl && lay != extrew && lay != nac && lay != vta
       && !laysp->InheritsFrom(&TA_PFCLayerSpec) && !laysp->InheritsFrom(&TA_MatrixLayerSpec)
       && !laysp->InheritsFrom(&TA_PatchLayerSpec) 
       && !laysp->InheritsFrom(&TA_SNcLayerSpec) && !laysp->InheritsFrom(&TA_SNrThalLayerSpec)) {
      other_lays.Link(lay);
      String nm = lay->name;
      nm.downcase();
      if(nm.contains("hidden") || nm.contains("_hid"))
	hidden_lays.Link(lay);
      if(nm.contains("output") || nm.contains("response") || nm.contains("_out") || nm.contains("_resp"))
	output_lays.Link(lay);
      if(nm.contains("input") || nm.contains("stimulus") || nm.contains("_in"))
	input_lays.Link(lay);
      LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
      if(us == NULL || !us->InheritsFrom(TA_DaModUnitSpec)) {
	us->ChangeMyType(&TA_DaModUnitSpec);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // make specs

  String gpprfx = "PFC_BG_";
  if(!bio_labels)
    gpprfx = "TD_";

  BaseSpec_MGroup* units = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Units");
  BaseSpec_MGroup* cons = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Cons");
  BaseSpec_MGroup* layers = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Layers");
  BaseSpec_MGroup* prjns = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Prjns");
  if(units == NULL || cons == NULL || layers == NULL || prjns == NULL) return;

  LeabraUnitSpec* rewpred_units = (LeabraUnitSpec*)units->FindMakeSpec("TDRewPredUnits", &TA_DaModUnitSpec);
  LeabraUnitSpec* td_units = (LeabraUnitSpec*)units->FindMakeSpec("TdUnits", &TA_DaModUnitSpec);
  if(rewpred_units == NULL || td_units == NULL) return;

  LeabraConSpec* learn_cons = (LeabraConSpec*)cons->FindMakeSpec("LearnCons", &TA_LeabraConSpec);
  if(learn_cons == NULL) return;

  TDRewPredConSpec* rewpred_cons = (TDRewPredConSpec*)learn_cons->FindMakeChild("TDRewPredCons", &TA_TDRewPredConSpec);
  LeabraConSpec* bg_bias = (LeabraConSpec*)learn_cons->FindMakeChild("BgBias", &TA_LeabraBiasSpec);
  if(bg_bias == NULL) return;
  LeabraConSpec* fixed_bias = (LeabraConSpec*)bg_bias->FindMakeChild("FixedBias", &TA_LeabraBiasSpec);

//   String abltonacnm = "ABLtoNAc";
//   String ertonacnm = "ExtRewtoNAc";
//   if(!bio_labels) {
//     abltonacnm = "TDRewPredToTDRewInteg";
//     ertonacnm = "ExtRewToTDRewInteg";
//   }
//   LeabraConSpec* abltonac_cons = (LeabraConSpec*)cons->FindMakeSpec(abltonacnm, &TA_LeabraConSpec);
//   if(!abltonac_cons) return;
//   LeabraConSpec* ertonac_cons = (LeabraConSpec*)abltonac_cons->FindMakeChild(ertonacnm, &TA_LeabraConSpec);
  LeabraConSpec* marker_cons = (LeabraConSpec*)cons->FindMakeSpec("MarkerCons", &TA_MarkerConSpec);
  if(rewpred_cons == NULL || marker_cons == NULL || fixed_bias == NULL)
    return;

  ExtRewLayerSpec* ersp = (ExtRewLayerSpec*)layers->FindMakeSpec("ExtRewLayer", &TA_ExtRewLayerSpec);
  TDRewPredLayerSpec* ablsp = (TDRewPredLayerSpec*)layers->FindMakeSpec(ablnm + "Layer", &TA_TDRewPredLayerSpec);
  TDRewIntegLayerSpec* nacsp = (TDRewIntegLayerSpec*)layers->FindMakeSpec(nacnm + "Layer", &TA_TDRewIntegLayerSpec);
  TdLayerSpec* tdsp = (TdLayerSpec*)layers->FindMakeSpec(vtanm + "Layer", &TA_TdLayerSpec);
  if(ablsp == NULL || ersp == NULL || nacsp == NULL || tdsp == NULL) return;

  ProjectionSpec* fullprjn = (ProjectionSpec*)prjns->FindMakeSpec("FullPrjn", &TA_FullPrjnSpec);
  ProjectionSpec* onetoone = (ProjectionSpec*)prjns->FindMakeSpec("OneToOne", &TA_OneToOnePrjnSpec);
  if(fullprjn == NULL || onetoone == NULL) return;

  //////////////////////////////////////////////////////////////////////////////////
  // set default spec parameters

  learn_cons->lmix.hebb = .01f; // .01 hebb on learn cons
  learn_cons->not_used_ok = true;
  learn_cons->UpdateAfterEdit();
  bg_bias->SetUnique("lrate", true);
  bg_bias->lrate = 0.0f;
  fixed_bias->SetUnique("lrate", true);
  fixed_bias->lrate = 0.0f;
  rewpred_cons->SetUnique("rnd", true);
  rewpred_cons->rnd.mean = 0.1f; rewpred_cons->rnd.var = 0.0f;
  rewpred_cons->SetUnique("wt_sig", true);
  rewpred_cons->wt_sig.gain = 1.0f;  rewpred_cons->wt_sig.off = 1.0f;
  rewpred_cons->SetUnique("lmix", true);
  rewpred_cons->lmix.hebb = 0.0f;

  rewpred_units->SetUnique("g_bar", true);
  rewpred_units->g_bar.h = .015f;
  rewpred_units->g_bar.a = .045f;

  if(output_lays.size > 0)
    ersp->rew_type = ExtRewLayerSpec::OUT_ERR_REW;
  else
    ersp->rew_type = ExtRewLayerSpec::EXT_REW;

  int n_rp_u = 19;		// number of rewpred-type units
  ablsp->unit_range.min = 0.0f;  ablsp->unit_range.max = 3.0f;
  nacsp->unit_range.min = 0.0f;  nacsp->unit_range.max = 3.0f;

  // optimization to speed up settling in phase 2: only the basic layers here
  int j;
  for(j=0;j<proj->specs.size;j++) {
    if(proj->specs[j]->InheritsFrom(TA_LeabraLayerSpec)) {
      LeabraLayerSpec* sp = (LeabraLayerSpec*)proj->specs[j];
      sp->decay.clamp_phase2 = true;
      sp->UpdateAfterEdit();
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // set geometries

  if(abl->n_units != n_rp_u) { abl->n_units = n_rp_u; abl->geom.x = n_rp_u; abl->geom.y = 1; }
  if(extrew->n_units != 8) { extrew->n_units = 8; extrew->geom.x = 8; extrew->geom.y = 1; }
  if(nac->n_units != n_rp_u) { nac->n_units = n_rp_u; nac->geom.x = n_rp_u; nac->geom.y = 1; }
  vta->n_units = 1;
  rew_targ_lay->n_units = 1;

  //////////////////////////////////////////////////////////////////////////////////
  // apply specs to objects

  abl->SetLayerSpec(ablsp);	abl->SetUnitSpec(rewpred_units);
  extrew->SetLayerSpec(ersp);	extrew->SetUnitSpec(rewpred_units);
  nac->SetLayerSpec(nacsp);	nac->SetUnitSpec(rewpred_units);
  vta->SetLayerSpec(tdsp);	vta->SetUnitSpec(td_units);

  rewpred_units->bias_spec.SetSpec(bg_bias);
  td_units->bias_spec.SetSpec(fixed_bias);
  
  //////////////////////////////////////////////////////////////////////////////////
  // make projections

  // FindMakePrjn(Layer* recv, Layer* send,
  net->FindMakePrjn(extrew, rew_targ_lay, onetoone, marker_cons);
  net->FindMakePrjn(nac, abl, onetoone, marker_cons);
  net->FindMakePrjn(vta, nac, onetoone, marker_cons);
  net->FindMakePrjn(nac, extrew, onetoone, marker_cons);
  net->FindMakePrjn(abl, vta, onetoone, marker_cons);

  for(i=0;i<other_lays.size;i++) {
    Layer* ol = (Layer*)other_lays[i];
    if(abl_new)
      net->FindMakePrjn(abl, ol, fullprjn, rewpred_cons);
    if(td_mod_all)
      net->FindMakePrjn(ol, vta, fullprjn, marker_cons);
  }

  for(i=0;i<output_lays.size;i++) {
    Layer* ol = (Layer*)output_lays[i];
    net->FindMakePrjn(extrew, ol, onetoone, marker_cons);
  }

  //////////////////////////////////////////////////////////////////////////////////
  // build and check

  net->Build();
  net->Connect();

  LeabraTrial* trl = (LeabraTrial*)pdpMisc::FindProcType(proj, &TA_LeabraTrial);
  if(trl == NULL) {
    taMisc::Error("Warning: a LeabraTrial TrialProcess was not found -- the trial\
 process must be configured properly for the network to run, and this cannot be done\
 at this time.  This also prevents other checks from being made.  All of this can be\
 done later when you press Re/New/Init on a schedule process when you run the network.");
    return;
  }

  bool ok = ablsp->CheckConfig(abl, trl, true) && nacsp->CheckConfig(nac, trl, true)
    && tdsp->CheckConfig(vta, trl, true) && ersp->CheckConfig(extrew, trl, true);

  if(!ok) {
    msg =
      "TD: An error in the configuration has occurred (it should be the last message\
 you received prior to this one).  The network will not run until this is fixed.\
 In addition, the configuration process may not be complete, so you should run this\
 function again after you have corrected the source of the error.";
  }
  else {
    msg = 
    "TD configuration is now complete.  Do not forget the one remaining thing\
 you need to do manually:\n\n" + man_msg;
  }
  taMisc::Choice(msg,"Ok");

  ablsp->UpdateAfterEdit();
  ersp->UpdateAfterEdit();
  nacsp->UpdateAfterEdit();
  
  for(j=0;j<proj->specs.leaves;j++) {
    BaseSpec* sp = (BaseSpec*)proj->specs.Leaf(j);
    sp->UpdateAfterEdit();
  }

  winbMisc::DelayedMenuUpdate(net);
  winbMisc::DelayedMenuUpdate(proj);

  //////////////////////////////////////////////////////////////////////////////////
  // select edit

  SelectEdit* edit = pdpMisc::FindSelectEdit(proj);
  if(edit != NULL) {
    rewpred_cons->SelectForEditNm("lrate", edit, "rewpred");
    ersp->SelectForEditNm("rew", edit, "extrew");
    ablsp->SelectForEditNm("rew_pred", edit, "abl");
    nacsp->SelectForEditNm("rew_integ", edit, "nac");
    //    ertonac_cons->SelectForEditNm("wt_scale", edit, "extrew to nac");
  }
}

///////////////////////////////////////////////////////////////
//			PVLV
///////////////////////////////////////////////////////////////

// todo: set td_mod.on = true for td_mod_all; need to get UnitSpec..

void LeabraWiz::PVLV(Network* net, bool bio_labels, bool fm_hid_cons, bool fm_out_cons, bool da_mod_all) {
  String msg = "Configuring Pavlov (PVLV) Layers:\n\n\
 There is one thing you will need to check manually after this automatic configuration\
 process completes (this note will be repeated when things complete --- there may be some\
 messages in the interim):\n\n";

  String man_msg = "1. Check that connection(s) were made from all appropriate output layers\
 to the ExtRew layer, using the MarkerConSpec (MarkerCons) Con spec.\
 This will provide the error signal to the system based on output error performance.\n\n";

  msg += man_msg + "\n\nThe configuration will now be checked and a number of default parameters\
 will be set.  If there are any actual errors which must be corrected before\
 the network will run, you will see a message to that effect --- you will then need to\
 re-run this configuration process to make sure everything is OK.  When you press\
 Re/New/Init on the control process these same checks will be performed, so you\
 can be sure everything is ok.";
  taMisc::Choice(msg,"Ok");

  Project* proj = GET_MY_OWNER(Project);
  net->RemoveUnits();

  //////////////////////////////////////////////////////////////////////////////////
  // make layers

  bool	abl_new = false;
  String ablnm = "ABL";  String nacnm = "NAcLV";  String pvsnm = "NAcPV";  String vtanm = "VTA";
  if(!bio_labels) {
    ablnm = "PV";    nacnm = "LV";    pvsnm = "PVS";    vtanm = "DA";
  }

  LeabraLayer* rew_targ_lay = (LeabraLayer*)net->FindMakeLayer("RewTarg");
  LeabraLayer* extrew = (LeabraLayer*)net->FindMakeLayer("ExtRew");
  LeabraLayer* nac = (LeabraLayer*)net->FindMakeLayer(nacnm);
  LeabraLayer* abl = (LeabraLayer*)net->FindMakeLayer(ablnm, NULL, abl_new);
  LeabraLayer* pvs = (LeabraLayer*)net->FindMakeLayer(pvsnm);
  LeabraLayer* vta = (LeabraLayer*)net->FindMakeLayer(vtanm);
  if(rew_targ_lay == NULL || abl == NULL || extrew == NULL || nac == NULL || vta == NULL) return;

  //////////////////////////////////////////////////////////////////////////////////
  // sort layers

  rew_targ_lay->name = "0000";  extrew->name = "0001"; nac->name = "0002";  
  abl->name = "0003";  pvs->name = "0004";    vta->name = "0005";

  net->layers.Sort();

  rew_targ_lay->name = "RewTarg";  extrew->name = "ExtRew"; nac->name = nacnm;
  abl->name = ablnm; pvs->name = pvsnm; vta->name = vtanm;

  //////////////////////////////////////////////////////////////////////////////////
  // collect layer groups

  Layer_MGroup other_lays;
  Layer_MGroup hidden_lays;
  Layer_MGroup output_lays;
  Layer_MGroup input_lays;
  int i;
  for(i=0;i<net->layers.size;i++) {
    LeabraLayer* lay = (LeabraLayer*)net->layers[i];
    LeabraLayerSpec* laysp = (LeabraLayerSpec*)lay->spec.spec;
    lay->SetUnitType(&TA_DaModUnit);
    // todo: add any new bg layer exclusions here!
    if(lay != rew_targ_lay && lay != abl && lay != extrew && lay != nac && lay != pvs && lay != vta
       && !laysp->InheritsFrom(&TA_PFCLayerSpec) && !laysp->InheritsFrom(&TA_MatrixLayerSpec)
       && !laysp->InheritsFrom(&TA_PatchLayerSpec) 
       && !laysp->InheritsFrom(&TA_SNcLayerSpec) && !laysp->InheritsFrom(&TA_SNrThalLayerSpec)) {
      other_lays.Link(lay);
      String nm = lay->name;
      nm.downcase();
      if(nm.contains("hidden") || nm.contains("_hid"))
	hidden_lays.Link(lay);
      if(nm.contains("output") || nm.contains("response") || nm.contains("_out") || nm.contains("_resp"))
	output_lays.Link(lay);
      if(nm.contains("input") || nm.contains("stimulus") || nm.contains("_in"))
	input_lays.Link(lay);
      LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
      if(us == NULL || !us->InheritsFrom(TA_DaModUnitSpec)) {
	us->ChangeMyType(&TA_DaModUnitSpec);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // make specs

  String gpprfx = "PFC_BG_";
  if(!bio_labels)
    gpprfx = "DA_";

  BaseSpec_MGroup* units = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Units");
  BaseSpec_MGroup* cons = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Cons");
  BaseSpec_MGroup* layers = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Layers");
  BaseSpec_MGroup* prjns = pdpMisc::FindMakeSpecGp(proj, gpprfx + "Prjns");
  if(units == NULL || cons == NULL || layers == NULL || prjns == NULL) return;

  LeabraUnitSpec* lv_units = (LeabraUnitSpec*)units->FindMakeSpec("LVUnits", &TA_DaModUnitSpec);
  LeabraUnitSpec* pv_units = (LeabraUnitSpec*)units->FindMakeSpec("PVUnits", &TA_DaModUnitSpec);
  LeabraUnitSpec* da_units = (LeabraUnitSpec*)units->FindMakeSpec("DaUnits", &TA_DaModUnitSpec);
  if(pv_units == NULL || lv_units == NULL || da_units == NULL) return;

  LeabraConSpec* learn_cons = (LeabraConSpec*)cons->FindMakeSpec("LearnCons", &TA_LeabraConSpec);
  if(learn_cons == NULL) return;

  PVConSpec* lv_cons = (PVConSpec*)learn_cons->FindMakeChild("NAc_LV", &TA_LVConSpec);
  PVConSpec* abl_cons = (PVConSpec*)learn_cons->FindMakeChild("ABL_PV", &TA_PVConSpec);
  PVConSpec* pvs_cons = (PVConSpec*)abl_cons->FindMakeChild("NAc_PVS", &TA_PVConSpec);
  LeabraConSpec* bg_bias = (LeabraConSpec*)learn_cons->FindMakeChild("BgBias", &TA_LeabraBiasSpec);
  if(bg_bias == NULL) return;
  LeabraConSpec* fixed_bias = (LeabraConSpec*)bg_bias->FindMakeChild("FixedBias", &TA_LeabraBiasSpec);

  LeabraConSpec* marker_cons = (LeabraConSpec*)cons->FindMakeSpec("MarkerCons", &TA_MarkerConSpec);
  if(abl_cons == NULL || marker_cons == NULL || fixed_bias == NULL)
    return;

  ExtRewLayerSpec* ersp = (ExtRewLayerSpec*)layers->FindMakeSpec("ExtRewLayer", &TA_ExtRewLayerSpec);
  LVLayerSpec* nacsp = (LVLayerSpec*)layers->FindMakeSpec(nacnm + "Layer", &TA_LVLayerSpec);
  PVLayerSpec* ablsp = (PVLayerSpec*)layers->FindMakeSpec(ablnm + "Layer", &TA_PVLayerSpec);
  PVLayerSpec* pvssp = (PVLayerSpec*)ablsp->FindMakeChild(pvsnm + "Layer", &TA_PVSLayerSpec);
  PVLVDaLayerSpec* dasp = (PVLVDaLayerSpec*)layers->FindMakeSpec(vtanm + "Layer", &TA_PVLVDaLayerSpec);
  if(ablsp == NULL || ersp == NULL || nacsp == NULL || dasp == NULL) return;

  ProjectionSpec* fullprjn = (ProjectionSpec*)prjns->FindMakeSpec("FullPrjn", &TA_FullPrjnSpec);
  ProjectionSpec* onetoone = (ProjectionSpec*)prjns->FindMakeSpec("OneToOne", &TA_OneToOnePrjnSpec);
  if(fullprjn == NULL || onetoone == NULL) return;

  //////////////////////////////////////////////////////////////////////////////////
  // set default spec parameters

  //  learn_cons->lmix.hebb = .01f; // .01 hebb on learn cons
  learn_cons->not_used_ok = true;
  learn_cons->UpdateAfterEdit();
  bg_bias->SetUnique("lrate", true);
  bg_bias->lrate = 0.0f;
  fixed_bias->SetUnique("lrate", true);
  fixed_bias->lrate = 0.0f;

  abl_cons->SetUnique("lrate", true);
  abl_cons->lrate = .05f;

  pvs_cons->SetUnique("lrate", true);
  pvs_cons->lrate = .001f;
  // NOT unique: inherit from abl:
  pvs_cons->SetUnique("rnd", false);
  pvs_cons->SetUnique("wt_limits", false);
  pvs_cons->SetUnique("wt_sig", false);
  pvs_cons->SetUnique("lmix", false);
  pvs_cons->SetUnique("lrate_sched", false);
  pvs_cons->SetUnique("lrs_value", false);

  // NOT unique: inherit from abl
  pvssp->SetUnique("decay", false);
  pvssp->SetUnique("kwta", false);
  pvssp->SetUnique("inhib_group", false);
  pvssp->SetUnique("compute_i", false);
  pvssp->SetUnique("i_kwta_pt", false);

  lv_units->SetUnique("g_bar", true);
  pv_units->SetUnique("g_bar", true);
  if(nacsp->scalar.rep == ScalarValSpec::GAUSSIAN) {
    ablsp->bias_val.un = ScalarValBias::GC;
    ablsp->bias_val.wt = ScalarValBias::WT;
    nacsp->bias_val.un = ScalarValBias::GC;
    nacsp->bias_val.wt = ScalarValBias::WT;
    lv_units->g_bar.h = .015f;  lv_units->g_bar.a = .045f;
    pv_units->g_bar.h = .015f;  pv_units->g_bar.a = .045f;
  }
  else {
    ablsp->bias_val.un = ScalarValBias::NO_UN;
    ablsp->bias_val.wt = ScalarValBias::NO_WT;
    nacsp->bias_val.un = ScalarValBias::NO_UN;
    nacsp->bias_val.wt = ScalarValBias::NO_WT;
    // todo: fix these:
    lv_units->g_bar.h = .5f;  lv_units->g_bar.a = .5f;
    lv_units->g_bar.h = .5f;  lv_units->g_bar.a = .5f;
  }

  if(output_lays.size > 0)
    ersp->rew_type = ExtRewLayerSpec::OUT_ERR_REW;
  else
    ersp->rew_type = ExtRewLayerSpec::EXT_REW;

  int n_pv_u;		// number of pvlv-type units
  if(nacsp->scalar.rep == ScalarValSpec::GAUSSIAN)
    n_pv_u = 22;
  else
    n_pv_u = 21;

  // optimization to speed up settling in phase 2: only the basic layers here
  int j;
  for(j=0;j<proj->specs.size;j++) {
    if(proj->specs[j]->InheritsFrom(TA_LeabraLayerSpec)) {
      LeabraLayerSpec* sp = (LeabraLayerSpec*)proj->specs[j];
      sp->decay.clamp_phase2 = true;
      sp->UpdateAfterEdit();
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // set positions & geometries

  if(abl_new) {
    extrew->pos.z = 0; extrew->pos.y = 6; extrew->pos.x = 0;
    vta->pos.z = 0; vta->pos.y = 1; vta->pos.x = 23;
    nac->pos.z = 0; nac->pos.y = 4; nac->pos.x = 0;
    pvs->pos.z = 0; pvs->pos.y = 2; pvs->pos.x = 0;
    abl->pos.z = 0; abl->pos.y = 0; abl->pos.x = 0;
  }

  if(nac->n_units != n_pv_u) { nac->n_units = n_pv_u; nac->geom.x = n_pv_u; nac->geom.y = 1; }
  if(abl->n_units != n_pv_u) { abl->n_units = n_pv_u; abl->geom.x = n_pv_u; abl->geom.y = 1; }
  if(pvs->n_units != n_pv_u) { pvs->n_units = n_pv_u; pvs->geom.x = n_pv_u; pvs->geom.y = 1; }
  if(extrew->n_units != n_pv_u) { extrew->n_units = n_pv_u; extrew->geom.x = n_pv_u; extrew->geom.y = 1; }
  vta->n_units = 1;
  rew_targ_lay->n_units = 1;

  //////////////////////////////////////////////////////////////////////////////////
  // apply specs to objects

  nac->SetLayerSpec(nacsp);	nac->SetUnitSpec(lv_units);
  abl->SetLayerSpec(ablsp);	abl->SetUnitSpec(pv_units);
  pvs->SetLayerSpec(pvssp);	pvs->SetUnitSpec(pv_units);
  extrew->SetLayerSpec(ersp);	extrew->SetUnitSpec(lv_units);
  vta->SetLayerSpec(dasp);	vta->SetUnitSpec(da_units);

  lv_units->bias_spec.SetSpec(bg_bias);
  pv_units->bias_spec.SetSpec(bg_bias);
  da_units->bias_spec.SetSpec(fixed_bias);
  
  //////////////////////////////////////////////////////////////////////////////////
  // make projections

  // FindMakePrjn(Layer* recv, Layer* send,
  net->FindMakePrjn(extrew, rew_targ_lay, onetoone, marker_cons);
  net->FindMakePrjn(nac, extrew, onetoone, marker_cons);
  net->FindMakePrjn(abl, nac, onetoone, marker_cons);
  net->FindMakePrjn(pvs, nac, onetoone, marker_cons);

  net->FindMakePrjn(vta, nac, onetoone, marker_cons);
  net->FindMakePrjn(vta, abl, onetoone, marker_cons);
  net->FindMakePrjn(vta, pvs, onetoone, marker_cons);

  net->FindMakePrjn(nac,  vta, onetoone, marker_cons);
  net->FindMakePrjn(abl,  vta, onetoone, marker_cons);
  net->FindMakePrjn(pvs,  vta, onetoone, marker_cons);

  if(abl_new || fm_hid_cons || fm_out_cons) {
    for(i=0;i<input_lays.size;i++) {
      Layer* il = (Layer*)input_lays[i];
      net->FindMakePrjn(nac, il, fullprjn, lv_cons);
      net->FindMakePrjn(abl, il, fullprjn, abl_cons);
      net->FindMakePrjn(pvs, il, fullprjn, pvs_cons);
    }
    if(fm_hid_cons) {
      for(i=0;i<hidden_lays.size;i++) {
	Layer* hl = (Layer*)hidden_lays[i];
	net->FindMakePrjn(nac, hl, fullprjn, lv_cons);
	net->FindMakePrjn(abl, hl, fullprjn, abl_cons);
	net->FindMakePrjn(pvs, hl, fullprjn, pvs_cons);
      }
    }
    if(fm_out_cons) {
      for(i=0;i<output_lays.size;i++) {
	Layer* ol = (Layer*)output_lays[i];
	net->FindMakePrjn(nac, ol, fullprjn, lv_cons);
	net->FindMakePrjn(abl, ol, fullprjn, abl_cons);
	net->FindMakePrjn(pvs, ol, fullprjn, pvs_cons);
      }
    }
  }
  if(da_mod_all) {
    for(i=0;i<other_lays.size;i++) {
      Layer* ol = (Layer*)other_lays[i];
      net->FindMakePrjn(ol, vta, fullprjn, marker_cons);
    }
  }

  for(i=0;i<output_lays.size;i++) {
    Layer* ol = (Layer*)output_lays[i];
    net->FindMakePrjn(extrew, ol, onetoone, marker_cons);
  }

  //////////////////////////////////////////////////////////////////////////////////
  // build and check

  net->Build();
  net->Connect();

  LeabraTrial* trl = (LeabraTrial*)pdpMisc::FindProcType(proj, &TA_LeabraTrial);
  if(trl == NULL) {
    taMisc::Error("Warning: a LeabraTrial TrialProcess was not found -- the trial\
 process must be configured properly for the network to run, and this cannot be done\
 at this time.  This also prevents other checks from being made.  All of this can be\
 done later when you press Re/New/Init on a schedule process when you run the network.");
    return;
  }

  ExtRew_Stat* ers = (ExtRew_Stat*)trl->final_stats.FindType(&TA_ExtRew_Stat);
  if(ers == NULL) {
    ers = (ExtRew_Stat*)trl->final_stats.NewEl(1, &TA_ExtRew_Stat);
    ers->CreateAggregates(Aggregate::AVG);
    ers->UpdateAfterEdit();
    EpochProcess* epc = trl->GetMyEpochProc();
    ExtRew_Stat* eers = (ExtRew_Stat*)epc->loop_stats.FindType(&TA_ExtRew_Stat);
    if(eers != NULL) eers->time_agg.op = Aggregate::AVG;
  }

  bool ok = nacsp->CheckConfig(nac, trl, true) && ablsp->CheckConfig(abl, trl, true)
    && pvssp->CheckConfig(abl, trl, true)
    && dasp->CheckConfig(vta, trl, true) && ersp->CheckConfig(extrew, trl, true);

  if(!ok) {
    msg =
      "PVLV: An error in the configuration has occurred (it should be the last message\
 you received prior to this one).  The network will not run until this is fixed.\
 In addition, the configuration process may not be complete, so you should run this\
 function again after you have corrected the source of the error.";
  }
  else {
    msg = 
    "PVLV configuration is now complete.  Do not forget the one remaining thing\
 you need to do manually:\n\n" + man_msg;
  }
  taMisc::Choice(msg,"Ok");

  nacsp->UpdateAfterEdit();
  ablsp->UpdateAfterEdit();
  pvssp->UpdateAfterEdit();
  ersp->UpdateAfterEdit();
  
  for(j=0;j<proj->specs.leaves;j++) {
    BaseSpec* sp = (BaseSpec*)proj->specs.Leaf(j);
    sp->UpdateAfterEdit();
  }

  winbMisc::DelayedMenuUpdate(net);
  winbMisc::DelayedMenuUpdate(proj);

  //////////////////////////////////////////////////////////////////////////////////
  // select edit

  SelectEdit* edit = pdpMisc::FindSelectEdit(proj);
  if(edit != NULL) {
    lv_cons->SelectForEditNm("lrate", edit, "nac_lv");
    abl_cons->SelectForEditNm("lrate", edit, "abl_pv");
    pvs_cons->SelectForEditNm("lrate", edit, "nac_pvs");
    abl_cons->SelectForEditNm("syn_dep", edit, "abl");
    ersp->SelectForEditNm("rew", edit, "extrew");
    ablsp->SelectForEditNm("pv", edit, "abl");
    nacsp->SelectForEditNm("lv", edit, "nac");
//     nacsp->SelectForEditNm("scalar", edit, "nac_lv");
//     ablsp->SelectForEditNm("scalar", edit, "abl");
    nacsp->SelectForEditNm("bias_val", edit, "nac_lv");
    ablsp->SelectForEditNm("bias_val", edit, "pv");
//    dasp->SelectForEditNm("avg_da", edit, "vta");
    dasp->SelectForEditNm("da", edit, "vta");
  }
}

///////////////////////////////////////////////////////////////
//			BgPFC
///////////////////////////////////////////////////////////////

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
    if(lay->name.contains("Patch")) {
      lay->gp_geom.x = 1;
      lay->gp_geom.y = n_stripes;
    }
    else {
      lay->gp_geom.x = n_stripes;
      lay->gp_geom.y = 1;
    }
  }
}

static void lay_set_geom(LeabraLayer* lay, int half_stripes) {
  if(lay->n_units == 0) {
    lay->geom.x = 1; lay->geom.y = 1; lay->n_units = 1;
  }
  lay->geom.z = half_stripes * 2;
  lay->gp_geom.y = 2; lay->gp_geom.x = half_stripes;
  lay->UpdateAfterEdit();
}

void LeabraWiz::SetPFCStripes(Network* net, int n_stripes, int n_units) {
  set_n_stripes(net, "PFC", n_stripes, n_units, true);
  set_n_stripes(net, "Matrix", n_stripes, -1, true);
  set_n_stripes(net, "Patch", n_stripes, -1, true);
  set_n_stripes(net, "SNc", n_stripes, -1, false);
  set_n_stripes(net, "SNrThal", n_stripes, -1, false);
  net->LayoutUnitGroups();
  net->Build();
  net->Connect();
}

void LeabraWiz::BgPFC(Network* net, int n_stripes, bool fm_hid_cons, bool fm_out_cons, bool mat_fm_pfc_full, bool make_patch, bool nolrn_pfc, bool da_mod_all, bool lr_sched) {
  bool make_snc = true;
  bool make_snr_thal = true;
//   if(make_snr_thal || make_patch) make_snc = true;

  PVLV(net, true, fm_hid_cons, fm_out_cons, da_mod_all); // first configure PVLV system..

  String msg = "Configuring BG PFC (Basal Ganglia Prefrontal Cortex) Layers:\n\n\
 There is one thing you will need to check manually after this automatic configuration\
 process completes (this note will be repeated when things complete --- there may be some\
 messages in the interim):\n\n";

  String man_msg = "1. Check the bidirectional connections between the PFC and all appropriate hidden layers\
 The con specs INTO the PFC should be ToPFC conspecs; the ones out should be regular learning conspecs.";

  msg += man_msg + "\n\nThe configuration will now be checked and a number of default parameters\
 will be set.  If there are any actual errors which must be corrected before\
 the network will run, you will see a message to that effect --- you will then need to\
 re-run this configuration process to make sure everything is OK.  When you press\
 Re/New/Init on the control process these same checks will be performed, so you\
 can be sure everything is ok.";
  taMisc::Choice(msg,"Ok");

  int half_stripes = n_stripes /2;
  half_stripes = MAX(1, half_stripes);
  n_stripes = half_stripes * 2;	// make it even

  Project* proj = GET_MY_OWNER(Project);
  net->RemoveUnits();

  //////////////////////////////////////////////////////////////////////////////////
  // make layers

  // get these from the DA function..
  LeabraLayer* rew_targ_lay = (LeabraLayer*)net->FindLayer("RewTarg");
  LeabraLayer* extrew = (LeabraLayer*)net->FindLayer("ExtRew");
  LeabraLayer* nac = (LeabraLayer*)net->FindLayer("NAcLV");
  LeabraLayer* abl = (LeabraLayer*)net->FindLayer("ABL");
  LeabraLayer* pvs = (LeabraLayer*)net->FindLayer("NAcPV");
  LeabraLayer* vta = (LeabraLayer*)net->FindLayer("VTA");
  if(rew_targ_lay == NULL || abl == NULL || extrew == NULL || nac == NULL || vta == NULL) return;

  LeabraLayer* patch = NULL;
  LeabraLayer* snc = NULL;
  LeabraLayer* snrthal = NULL;

  // if not new layers, don't make prjns into them!
  bool	snc_new = false; bool	patch_new = false;  bool snrthal_new = false;
  bool matrix_new = false;  bool	pfc_new = false;

  if(make_patch)  { patch = (LeabraLayer*)net->FindMakeLayer("Patch", NULL, patch_new); }
  else 		  { net->layers.Remove("Patch"); }
  if(make_snc)	  snc = (LeabraLayer*)net->FindMakeLayer("SNc", NULL, snc_new);
  else		  net->layers.Remove("SNc");

  LeabraLayer* matrix = (LeabraLayer*)net->FindMakeLayer("Matrix", NULL, matrix_new);

  if(make_snr_thal) snrthal = (LeabraLayer*)net->FindMakeLayer("SNrThal", NULL, snrthal_new);
  else		    net->layers.Remove("SNrThal");

  LeabraLayer* pfc = (LeabraLayer*)net->FindMakeLayer("PFC", NULL, pfc_new);

  if(matrix == NULL || pfc == NULL) return;

  //////////////////////////////////////////////////////////////////////////////////
  // sort layers

  rew_targ_lay->name = "0000";  extrew->name = "0001"; nac->name = "0002";  
  abl->name = "0003";  pvs->name = "0004";    vta->name = "0005";

  if(make_patch) 	{ patch->name = "ZZZ1"; }
  if(make_snc)   	snc->name = "ZZZ3";
  matrix->name = "ZZZ4";  
  if(make_snr_thal)    	snrthal->name = "ZZZ5";
  pfc->name = "ZZZ6";

  net->layers.Sort();

  rew_targ_lay->name = "RewTarg";  extrew->name = "ExtRew";  nac->name = "NAcLV";
  abl->name = "ABL";  pvs->name = "NAcPV";	vta->name = "VTA";
  if(make_snc) 		snc->name = "SNc";
  if(make_patch)	{ patch->name = "Patch"; }
  if(make_snr_thal)	snrthal->name = "SNrThal";
  matrix->name = "Matrix";  pfc->name = "PFC";

  //////////////////////////////////////////////////////////////////////////////////
  // collect layer groups

  int mx_z1 = 0;		// max x coordinate on layer z=1
  int mx_z2 = 0;		// z=2
  Layer_MGroup other_lays;  Layer_MGroup hidden_lays;
  Layer_MGroup output_lays;  Layer_MGroup input_lays;
  int i;
  for(i=0;i<net->layers.size;i++) {
    LeabraLayer* lay = (LeabraLayer*)net->layers[i];
    lay->SetUnitType(&TA_DaModUnit);
    if(lay != rew_targ_lay && lay != extrew && lay != nac && lay != abl && lay != pvs && lay != vta
       && lay != patch && lay != snc && lay != snrthal && lay != matrix && lay != pfc) {
      other_lays.Link(lay);
      int xm = lay->pos.x + lay->act_geom.x + 1;
      if(lay->pos.z == 1) mx_z1 = MAX(mx_z1, xm);
      if(lay->pos.z == 2) mx_z2 = MAX(mx_z2, xm);
      String nm = lay->name;
      nm.downcase();
      if(nm.contains("hidden") || nm.contains("_hid"))
	hidden_lays.Link(lay);
      if(nm.contains("output") || nm.contains("response") || nm.contains("_out") || nm.contains("_resp"))
	output_lays.Link(lay);
      if(nm.contains("input") || nm.contains("stimulus") || nm.contains("_in"))
	input_lays.Link(lay);
      LeabraUnitSpec* us = (LeabraUnitSpec*)lay->unit_spec.spec;
      if(us == NULL || !us->InheritsFrom(TA_DaModUnitSpec)) {
	us->ChangeMyType(&TA_DaModUnitSpec);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // set positions & geometries

  if(pfc_new) {
    pfc->pos.z = 2; pfc->pos.y = 0; pfc->pos.x = mx_z2 + 1;
    if(nolrn_pfc && (input_lays.size > 0)) {
      Layer* il = (Layer*)input_lays[0];
      pfc->n_units = il->n_units; pfc->geom.x = il->geom.x; pfc->geom.y = il->geom.y;
    }
    else {
      pfc->n_units = 30; pfc->geom.x = 5; pfc->geom.y = 6;
    }
  }
  lay_set_geom(pfc, half_stripes);

  if(matrix_new) { 
    matrix->pos.z = 1; matrix->pos.y = 0; matrix->pos.x = mx_z1 + 1; 
    matrix->n_units = 28; matrix->geom.x = 4; matrix->geom.y = 7;
  }
  lay_set_geom(matrix, half_stripes);

  if(make_patch) {
    if(patch_new) {
      matrix->UpdateAfterEdit();
      patch->pos.z = 1; patch->pos.y = 0; patch->pos.x = matrix->pos.x + matrix->act_geom.x + 2; 
      patch->n_units = 20; patch->geom.x = 20; patch->geom.y = 1;
      patch->gp_geom.x = 1; patch->gp_geom.y = n_stripes;
      patch->UpdateAfterEdit();
    }
    lay_set_geom(patch, half_stripes);
    patch->gp_geom.x = 1; patch->gp_geom.y = n_stripes;
  }

  if(make_snc) {
    snc->n_units = 1;
    if(snc_new) { 
      snc->pos.z = 0; snc->pos.y = 3; snc->pos.x = vta->pos.x; 
    }
    lay_set_geom(snc, half_stripes);
  }
  if(make_snr_thal) {
    snrthal->n_units = 1;
    if(snrthal_new) {
      snrthal->pos.z = 0; snrthal->pos.y = 6; snrthal->pos.x = snc->pos.x;
    }
    lay_set_geom(snrthal, half_stripes);
  }

  //////////////////////////////////////////////////////////////////////////////////
  // make specs

  BaseSpec_MGroup* units = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Units");
  BaseSpec_MGroup* cons = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Cons");
  BaseSpec_MGroup* layers = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Layers");
  BaseSpec_MGroup* prjns = pdpMisc::FindMakeSpecGp(proj, "PFC_BG_Prjns");
  if(units == NULL || cons == NULL || layers == NULL || prjns == NULL) return;

  LeabraUnitSpec* pv_units = (LeabraUnitSpec*)units->FindMakeSpec("PVUnits", &TA_DaModUnitSpec);
  LeabraUnitSpec* da_units = (LeabraUnitSpec*)units->FindMakeSpec("DaUnits", &TA_DaModUnitSpec);

  LeabraUnitSpec* pfc_units = (LeabraUnitSpec*)units->FindMakeSpec("PFCUnits", &TA_DaModUnitSpec);
  LeabraUnitSpec* matrix_units = (LeabraUnitSpec*)units->FindMakeSpec("MatrixUnits", &TA_MatrixUnitSpec);
  LeabraUnitSpec* snrthal_units = (LeabraUnitSpec*)units->FindMakeSpec("SNrThalUnits", &TA_DaModUnitSpec);
  if(pfc_units == NULL || matrix_units == NULL) return;

  LeabraConSpec* learn_cons = (LeabraConSpec*)cons->FindMakeSpec("LearnCons", &TA_LeabraConSpec);
  if(learn_cons == NULL) return;

  LeabraConSpec* lv_cons = (LeabraConSpec*)learn_cons->FindMakeChild("NAc_LV", &TA_LVConSpec);
  LeabraConSpec* abl_cons = (LeabraConSpec*)learn_cons->FindMakeChild("ABL_PV", &TA_PVConSpec);
  LeabraConSpec* pvs_cons = (LeabraConSpec*)abl_cons->FindMakeChild("NAc_PVS", &TA_PVConSpec);

  LeabraConSpec* topfc_cons = (LeabraConSpec*)learn_cons->FindMakeChild("ToPFC", &TA_LeabraConSpec);
  if(topfc_cons == NULL) return;
  LeabraConSpec* intra_pfc = (LeabraConSpec*)topfc_cons->FindMakeChild("IntraPFC", &TA_LeabraConSpec);
  LeabraConSpec* pfc_bias = (LeabraConSpec*)topfc_cons->FindMakeChild("PFCBias", &TA_LeabraBiasSpec);
  LeabraConSpec* matrix_cons = (LeabraConSpec*)learn_cons->FindMakeChild("MatrixCons", &TA_MatrixConSpec);
  LeabraConSpec* mfmpfc_cons = (LeabraConSpec*)matrix_cons->FindMakeChild("MatrixFmPFC", &TA_MatrixConSpec);
  LeabraConSpec* marker_cons = (LeabraConSpec*)cons->FindMakeSpec("MarkerCons", &TA_MarkerConSpec);
  LeabraConSpec* pfc_self = (LeabraConSpec*)cons->FindMakeSpec("PFCSelfCon", &TA_LeabraConSpec);

  LeabraConSpec* bg_bias = (LeabraConSpec*)learn_cons->FindMakeChild("BgBias", &TA_LeabraBiasSpec);
  if(bg_bias == NULL) return;
  LeabraConSpec* matrix_bias = (LeabraConSpec*)bg_bias->FindMakeChild("MatrixBias", &TA_MatrixBiasSpec);
  if(pfc_self == NULL || intra_pfc == NULL || matrix_cons == NULL || marker_cons == NULL 
     || matrix_bias == NULL)
    return;

  PVLayerSpec* ablsp = (PVLayerSpec*)layers->FindMakeSpec("ABLLayer", &TA_PVLayerSpec);
  PVLVDaLayerSpec* dasp = (PVLVDaLayerSpec*)layers->FindMakeSpec("VTALayer", &TA_PVLVDaLayerSpec);
  LeabraLayerSpec* pfcsp = (LeabraLayerSpec*)layers->FindMakeSpec("PFCLayer", &TA_PFCLayerSpec);
  LeabraLayerSpec* sncsp = (LeabraLayerSpec*)dasp->FindMakeChild("SNcLayer", &TA_SNcLayerSpec);
  MatrixLayerSpec* matrixsp = (MatrixLayerSpec*)layers->FindMakeSpec("MatrixLayer", &TA_MatrixLayerSpec);
  if(pfcsp == NULL || matrixsp == NULL) return;
  LeabraLayerSpec* patchsp = (LeabraLayerSpec*)ablsp->FindMakeChild("PatchLayer", &TA_PatchLayerSpec);
  LeabraLayerSpec* snrthalsp = (LeabraLayerSpec*)layers->FindMakeSpec("SNrThalLayer", &TA_SNrThalLayerSpec);

  ProjectionSpec* fullprjn = (ProjectionSpec*)prjns->FindMakeSpec("FullPrjn", &TA_FullPrjnSpec);
  ProjectionSpec* gponetoone = (ProjectionSpec*)prjns->FindMakeSpec("GpOneToOne", &TA_GpOneToOnePrjnSpec);
  //  ProjectionSpec* onetoone = (ProjectionSpec*)prjns->FindMakeSpec("OneToOne", &TA_OneToOnePrjnSpec);
  UniformRndPrjnSpec* topfc = (UniformRndPrjnSpec*)prjns->FindMakeSpec("ToPFC", &TA_UniformRndPrjnSpec);
  ProjectionSpec* pfc_selfps = (ProjectionSpec*)prjns->FindMakeSpec("PFCSelf", &TA_OneToOnePrjnSpec);
  GpRndTesselPrjnSpec* intra_pfcps = (GpRndTesselPrjnSpec*)prjns->FindMakeSpec("IntraPFC", &TA_GpRndTesselPrjnSpec);
  TesselPrjnSpec* input_pfc = (TesselPrjnSpec*)prjns->FindMakeSpec("Input_PFC", &TA_TesselPrjnSpec);
  if(topfc == NULL || pfc_selfps == NULL || intra_pfcps == NULL || gponetoone == NULL || input_pfc == NULL) return;

  input_pfc->send_offs.New(1); // this is all it takes!

  //////////////////////////////////////////////////////////////////////////////////
  // apply specs to objects

  if(make_patch) 	{ patch->SetLayerSpec(patchsp); patch->SetUnitSpec(pv_units); }
  if(make_snc)	 	{ snc->SetLayerSpec(sncsp); snc->SetUnitSpec(da_units); }
  if(make_snr_thal) 	{ snrthal->SetLayerSpec(snrthalsp); snrthal->SetUnitSpec(snrthal_units); }
  matrix->SetLayerSpec(matrixsp);   matrix->SetUnitSpec(matrix_units);
  pfc->SetLayerSpec(pfcsp);	pfc->SetUnitSpec(pfc_units);

  // set bias specs for unit specs
  pfc_units->bias_spec.SetSpec(pfc_bias);
  matrix_units->bias_spec.SetSpec(matrix_bias);
  
  //////////////////////////////////////////////////////////////////////////////////
  // make projections

  // FindMakePrjn(Layer* recv, Layer* send,
  if(make_snc) {
    net->FindMakePrjn(snc, nac, fullprjn, marker_cons);
    net->FindMakePrjn(snc, abl, fullprjn, marker_cons);
    net->FindMakePrjn(snc, pvs, fullprjn, marker_cons);
    net->FindMakePrjn(matrix, snc, gponetoone, marker_cons);
  }
  else {
    net->FindMakePrjn(matrix, vta, fullprjn, marker_cons);
  }

  if(make_patch) {
    // todo: not right..
    net->FindMakePrjn(patch, extrew, fullprjn, marker_cons);
    net->FindMakePrjn(patch, snc, gponetoone, marker_cons);
    net->FindMakePrjn(snc, patch, gponetoone, marker_cons);
  }

  if(make_snr_thal) {
    net->FindMakePrjn(snrthal, matrix, gponetoone, marker_cons);
    net->FindMakePrjn(pfc, snrthal, gponetoone, marker_cons);
    net->FindMakePrjn(matrix, snrthal, gponetoone, marker_cons);
  }
  else {
    net->FindMakePrjn(pfc, matrix, gponetoone, marker_cons);
  }

  if(mat_fm_pfc_full)
    net->FindMakePrjn(matrix, pfc, fullprjn, mfmpfc_cons);
  else
    net->FindMakePrjn(matrix, pfc, gponetoone, mfmpfc_cons);

  net->FindMakeSelfPrjn(pfc, pfc_selfps, pfc_self);
  //  net->FindMakeSelfPrjn(pfc, intra_pfcps, intra_pfc);

  net->FindMakePrjn(nac, pfc, fullprjn, lv_cons);
  net->FindMakePrjn(abl, pfc, fullprjn, abl_cons);
  net->FindMakePrjn(pvs, pfc, fullprjn, pvs_cons);

  if(make_patch) {
    net->FindMakePrjn(patch, pfc, gponetoone, abl_cons);
  }

  for(i=0;i<input_lays.size;i++) {
    Layer* il = (Layer*)input_lays[i];
    if(pfc_new) {
      if(nolrn_pfc)
	net->FindMakePrjn(pfc, il, input_pfc, topfc_cons);
      else
	net->FindMakePrjn(pfc, il, fullprjn, topfc_cons);
    }
    if(matrix_new)
      net->FindMakePrjn(matrix, il, fullprjn, matrix_cons);
    if(make_patch && patch_new) {
      net->FindMakePrjn(patch, il, fullprjn, abl_cons);
    }
  }
  for(i=0;i<hidden_lays.size;i++) {
    Layer* hl = (Layer*)hidden_lays[i];
    net->FindMakePrjn(hl, pfc, fullprjn, learn_cons);
    if(fm_hid_cons) {
      if(pfc_new && !nolrn_pfc)
	net->FindMakePrjn(pfc, hl, fullprjn, topfc_cons);
      if(matrix_new)
	net->FindMakePrjn(matrix, hl, fullprjn, matrix_cons);
      if(make_patch && patch_new) {
	net->FindMakePrjn(patch, hl, fullprjn, abl_cons);
      }
    }
  }
  if(fm_out_cons) {
    for(i=0;i<output_lays.size;i++) {
      Layer* ol = (Layer*)output_lays[i];
      if(pfc_new && !nolrn_pfc)
	net->FindMakePrjn(pfc, ol, fullprjn, topfc_cons);
      if(matrix_new)
	net->FindMakePrjn(matrix, ol, fullprjn, matrix_cons);
      if(make_patch && patch_new) {
	net->FindMakePrjn(patch, ol, fullprjn, abl_cons);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // set default spec parameters

  // todo: update these values!

  if(lr_sched) {
    learn_cons->lrs_value = LeabraConSpec::EXT_REW_STAT;
    learn_cons->lrate_sched.EnforceSize(2);
    SchedItem* si = (SchedItem*)learn_cons->lrate_sched.FastEl(0);
    si->start_val = 1.0f;
    si = (SchedItem*)learn_cons->lrate_sched.FastEl(1);
    si->start_ctr = 90;
    si->start_val = .1f;
  }

  // slow learning rate on to pfc cons!
  topfc_cons->SetUnique("lrate", true);
  if(nolrn_pfc) {
    topfc_cons->lrate = 0.0f;
    topfc_cons->SetUnique("rnd", true);
    topfc_cons->rnd.var = 0.0f;
  }
  else {
    topfc_cons->lrate = .001f;
    topfc_cons->SetUnique("rnd", false);
    topfc_cons->rnd.var = 0.25f;
  }
  topfc_cons->SetUnique("lmix", true);
  topfc_cons->lmix.hebb = .001f;
  intra_pfc->SetUnique("wt_scale", true);
  intra_pfc->wt_scale.rel = .1f;

  matrix_cons->SetUnique("lrate", true);
  matrix_cons->lrate = .01f;

  mfmpfc_cons->SetUnique("wt_scale", true);
  mfmpfc_cons->wt_scale.rel = .2f;
  mfmpfc_cons->SetUnique("lmix", false);

  matrix_units->g_bar.h = .01f;
  matrix_units->g_bar.a = .03f;
  pfc_units->SetUnique("g_bar", true);
  if(nolrn_pfc)
    pfc_units->g_bar.h = 1.0f;
  else
    pfc_units->g_bar.h = .5f;
  pfc_units->g_bar.a = 2.0f;
  pfc_units->SetUnique("act_reg", true);
  if(nolrn_pfc)
    pfc_units->act_reg.on = false;
  else
    pfc_units->act_reg.on = true;

  snrthal_units->dt.vm = .1f;
  snrthal_units->act.gain = 20.0f;

  // set projection parameters
  topfc->p_con = .4;
  pfc_selfps->self_con = true;

  // todo: out of date!
  intra_pfcps->def_p_con = .4;
  intra_pfcps->recv_gp_n.y = 1;
  intra_pfcps->recv_gp_group.x = half_stripes;
  intra_pfcps->MakeRectangle(half_stripes, 1, 0, 1);
  intra_pfcps->wrap = false;
  
  matrixsp->bg_type = MatrixLayerSpec::PFC;
  // set these to fix old projects..
  matrixsp->gp_kwta.k_from = KWTASpec::USE_PCT;
  matrixsp->gp_kwta.pct = .25f;
  matrixsp->compute_i = LeabraLayerSpec::KWTA_INHIB;
  matrixsp->i_kwta_pt = .25f;
  matrixsp->UpdateAfterEdit();

  if(make_patch) {
    // NOT unique: inherit from abl:
    patchsp->SetUnique("decay", false);
    patchsp->SetUnique("kwta", false);
    patchsp->SetUnique("inhib_group", false);
    patchsp->SetUnique("compute_i", false);
    patchsp->SetUnique("i_kwta_pt", false);
  }


  //////////////////////////////////////////////////////////////////////////////////
  // build and check

  SetPFCStripes(net, n_stripes);

  LeabraTrial* trl = (LeabraTrial*)pdpMisc::FindProcType(proj, &TA_LeabraTrial);
  if(trl == NULL) {
    taMisc::Error("Warning: a LeabraTrial TrialProcess was not found -- the trial\
 process must be configured properly for the network to run, and this cannot be done\
 at this time.  This also prevents other checks from being made.  All of this can be\
 done later when you press Re/New/Init on a schedule process when you run the network.");
    return;
  }

  bool ok = pfcsp->CheckConfig(pfc, trl, true) && matrixsp->CheckConfig(matrix, trl, true);

  if(ok && make_snc) ok = sncsp->CheckConfig(snc, trl, true);
  if(ok && make_patch) ok = patchsp->CheckConfig(patch, trl, true);
  if(ok && make_snr_thal) ok = snrthalsp->CheckConfig(snrthal, trl, true);

  if(!ok) {
    msg =
      "BG/PFC: An error in the configuration has occurred (it should be the last message\
 you received prior to this one).  The network will not run until this is fixed.\
 In addition, the configuration process may not be complete, so you should run this\
 function again after you have corrected the source of the error.";
  }
  else {
    msg = 
    "BG/PFC configuration is now complete.  Do not forget the one remaining thing\
 you need to do manually:\n\n" + man_msg;
  }
  taMisc::Choice(msg,"Ok");

  for(int j=0;j<proj->specs.leaves;j++) {
    BaseSpec* sp = (BaseSpec*)proj->specs.Leaf(j);
    sp->UpdateAfterEdit();
  }

  winbMisc::DelayedMenuUpdate(net);
  winbMisc::DelayedMenuUpdate(proj);

  //////////////////////////////////////////////////////////////////////////////////
  // select edit

  SelectEdit* edit = pdpMisc::FindSelectEdit(proj);
  if(edit != NULL) {
    pfc_units->SelectForEditNm("g_bar", edit, "pfc");
    pfcsp->SelectForEditNm("gate", edit, "pfc");
    pfcsp->SelectForEditNm("act_reg", edit, "pfc");
    matrixsp->SelectForEditNm("matrix", edit, "matrix");
    matrixsp->SelectForEditNm("contrast", edit, "matrix");
    matrixsp->SelectForEditNm("rnd_go", edit, "matrix");
    matrixsp->SelectForEditNm("rnd_go_thr", edit, "matrix");
    matrix_units->SelectForEditNm("g_bar", edit, "matrix");
//     matrix_cons->SelectForEditNm("lrate", edit, "matrix");
    matrix_cons->SelectForEditNm("lmix", edit, "matrix");
    mfmpfc_cons->SelectForEditNm("wt_scale", edit, "mtx_fm_pfc");
    if(make_snc) {
      sncsp->SelectForEditNm("snc", edit, "snc");
    }
    if(make_snr_thal) {
      snrthalsp->SelectForEditNm("kwta", edit, "snr_thal");
//       snrthal_units->SelectForEditNm("g_bar", edit, "snr_thal");
//       snrthal_units->SelectForEditNm("dt", edit, "snr_thal");
    }
  }
}


