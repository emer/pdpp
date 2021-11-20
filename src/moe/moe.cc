/* -*- C++ -*- */
/*=============================================================================
//                                                                            //
// This file is part of the PDP++ software package.                           //
//                                                                            //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson,                 //
//                    James L. McClelland, and Carnegie Mellon University     //
//                                                                            //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted       //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//                                                                            //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions.  HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
//                                                                            //
// Note that the taString class, which is derived from the GNU String class,  //
// is Copyright (C) 1988 Free Software Foundation, written by Doug Lea, and   //
// is covered by the GNU General Public License, see ta_string.h.             //
// The iv_graphic library and some iv_misc classes were derived from the      //
// InterViews morpher example and other InterViews code, which is             //
// Copyright (C) 1987, 1988, 1989, 1990, 1991 Stanford University             //
// Copyright (C) 1991 Silicon Graphics, Inc.                                  //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,         //
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY           //
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.           //
//                                                                            //
// IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY SPECIAL,    //
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES  //
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT     //
// ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY,      //
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        //
// SOFTWARE.                                                                  //
==============================================================================*/

// moe.cc

#ifdef __GNUG__
#pragma implementation
#endif
#include <moe/moe.h>


/////////////////////////////////
//    Output Units/Layers      //
/////////////////////////////////
  
void MoeOutputUnitSpec::Initialize() {
  var_init = 1.0f;
  var_init_wt = 1.0f;
}

void MoeOutputUnitSpec::InitWtState(Unit* u) {
  BpUnitSpec::InitWtState(u);
  MoeOutputUnit* mu = (MoeOutputUnit*)u;
  mu->g_act = mu->g_exp = mu->g_exp_raw = 0.0f;
  mu->var = var_init;
  mu->var_num = var_init_wt * var_init;
  mu->var_den = var_init_wt;	// start off with initial weighting
}

void MoeOutputUnitSpec::Compute_Net(Unit* u) {
  MoeOutputUnit* mu = (MoeOutputUnit*)u;

  BpUnit* expt_u = mu->GetExpertUnit();
  if(expt_u == NULL) return;
  MoeGateUnit* gate_u = mu->GetGatingUnit();
  if(gate_u == NULL) return;
  mu->net = expt_u->act;	// simple copy of activation from expert unit
  mu->g_act = gate_u->act;	// just activity from the gating unit
}

void MoeOutputUnitSpec::Compute_Act(Unit* u) {
  MoeOutputUnit* mu = (MoeOutputUnit*)u;
  mu->act = mu->net * mu->g_act; // net is expert activity, g_act is gating activity
}

float MoeOutputUnitSpec::Compute_GExpRaw(MoeOutputUnit* mu, MoeOutputLayer*) {
  mu->dEdA = mu->targ - mu->net;	// comparison is against expert output, not act
  mu->err = mu->dEdA * mu->dEdA;
  float exp_term = (-0.5f * mu->err) / mu->var;
  exp_term = MAX(exp_term, -50.0f);	// prevent exponent from blowing up..
  exp_term = MIN(exp_term, 50.0f);
  mu->g_exp_raw = mu->g_act * (1.0f / sqrtf(2.0f * PI * mu->var)) *
    expf(exp_term);
  return mu->g_exp_raw;
}

void MoeOutputUnitSpec::Compute_GExp(MoeOutputUnit* mu, MoeOutputLayer* lay) {
  // simple normalization of raw values
  mu->g_exp = mu->g_exp_raw / lay->g_exp_raw_sum;
}

// this must be computed *after* Compute_GExpRaw
void MoeOutputUnitSpec::Compute_VarDeltas(MoeOutputUnit* mu) {
  mu->var_num += mu->g_exp * mu->err;
  mu->var_den += mu->g_exp;
}

void MoeOutputUnitSpec::Compute_dEdA(BpUnit* u) {
  MoeOutputUnit* mu = (MoeOutputUnit*)u;
  // don't do the standard thing.  note that dEdA itself already computed in Estep
  Compute_VarDeltas(mu);	// compute this here for lack of a better idea
  // however, this means that it will get called repeatedly, even though it is static..
}

void MoeOutputUnitSpec::Compute_dEdNet(BpUnit* u) {
  MoeOutputUnit* mu = (MoeOutputUnit*)u;
  mu->dEdNet = mu->dEdA * (mu->g_exp / mu->var);// this will get send back to experts
  // note that the weight back to experts must be 1.0
}

void MoeOutputUnitSpec::UpdateWeights(Unit* u) {
  MoeOutputUnit* mu = (MoeOutputUnit*)u;
  mu->var = mu->var_num / mu->var_den; // update the sigma at this point based on increments
  // don't bother updating any other weights cuz they're all 0 anyway!
}


void MoeOutputUnit::Initialize() {
  spec.SetBaseType(&TA_MoeOutputUnitSpec);
  g_act = g_exp = g_exp_raw = 0.0f;
  var = var_num = var_den = 0.0f;
}

void MoeOutputUnit::Copy_(const MoeOutputUnit& cp) {
  g_act = cp.g_act;
  g_exp = cp.g_exp;
  g_exp_raw = cp.g_exp_raw;
  var = cp.var;
  var_num = cp.var_num;
  var_den = cp.var_den;
}

BpUnit* MoeOutputUnit::GetExpertUnit() {
  Con_Group* fm_expt = (Con_Group*)recv.Gp(0); // first group is experts
  if(fm_expt == NULL) {
    taMisc::Error("*** MoeOutputUnit: expecting recv.gp[0] to be one-to-one prjn",
		  "from EXPERT layer, did not find con group");
    return NULL;
  }
  BpUnit* expt_u = (BpUnit*)fm_expt->Un(0);
  if(expt_u == NULL) {
    taMisc::Error("*** MoeOutputUnit: expecting recv.gp[0] to be one-to-one prjn",
		  "from EXPERT layer, did not find unit [0]");
    return NULL;
  }
  return expt_u;
}

MoeGateUnit* MoeOutputUnit::GetGatingUnit() {
  Con_Group* fm_gate = (Con_Group*)recv.Gp(1); // second group is gate
  if(fm_gate == NULL) {
    taMisc::Error("*** MoeOutputUnit: expecting recv.gp[1] to be one-to-one prjn",
		  "from GATING layer, did not find con group");
    return NULL;
  }
  MoeGateUnit* gate_u = (MoeGateUnit*)fm_gate->Un(0);
  if(gate_u == NULL) {
    taMisc::Error("*** MoeOutputUnit: expecting recv.gp[1] to be one-to-one prjn",
		  "from GATING layer, did not find unit [0]");
    return NULL;
  }
  return gate_u;
}

void MoeOutputUnit::ReScaleVariance(float new_scale) {
  var_den = new_scale;
  var_num = new_scale * var;
}

void MoeOutputLayer::Initialize() {
  units.SetBaseType(&TA_MoeOutputUnit);
  unit_spec.SetBaseType(&TA_MoeOutputUnitSpec);
  g_exp_raw_sum = 0.0f;
}

void MoeOutputLayer::Compute_GExp() {
  g_exp_raw_sum = 0.0f;
  MoeOutputUnit* u;
  taLeafItr u_itr;
  // first pass get the raw values
  FOR_ITR_EL(MoeOutputUnit, u, units., u_itr)
    g_exp_raw_sum += u->Compute_GExpRaw(this);
  // second pass do the normalization
  FOR_ITR_EL(MoeOutputUnit, u, units., u_itr)
    u->Compute_GExp(this);
}

void MoeOutputLayer::Compute_Act() {
  Layer::Compute_Act();
  Compute_GExp();		// after computing activations, compute expected gates (once)
  // then, multiple Compute_dEdA_dEdNet iterations.
}

void MoeOutputLayer::ReScaleVariance(float new_scale) {
  MoeOutputUnit* u;
  taLeafItr u_itr;
  FOR_ITR_EL(MoeOutputUnit, u, units., u_itr)
    u->ReScaleVariance(new_scale);
}

/////////////////////////////////
//     Gate Units/Layers       //
/////////////////////////////////

void MoeGateUnitSpec::Initialize() {
  act_delta_cost = 0.0f;
}

void MoeGateUnitSpec::Compute_Net(Unit* u) {
  BpUnitSpec::Compute_Net(u);
  u->net = MIN(u->net, 100.0f);	// prevent blowup
  u->act = expf(u->net);
}

void MoeGateUnitSpec::Compute_Act(MoeGateUnit* u, MoeGateLayer* lay) {
  u->prv_act = u->act;
  u->act /= lay->act_sum;
}

void MoeGateUnitSpec::Compute_dEdA(BpUnit* u) {
  // we need to get the h value off of the output units!
  MoeGateUnit* mu = (MoeGateUnit*)u;
  MoeOutputUnit* out_u = mu->GetOutputUnit();
  if(out_u == NULL) return;
  mu->targ = out_u->g_exp;	// keep this around for display
  mu->dEdA = out_u->g_exp - mu->act;	// diff between expected and actual
  mu->dEdA += act_delta_cost * (mu->prv_act - mu->act);
}

void MoeGateUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = u->dEdA;
}

void MoeGateUnit::Initialize() {
  spec.SetBaseType(&TA_MoeGateUnitSpec);
  prv_act = 0.0f;
}

MoeOutputUnit* MoeGateUnit::GetOutputUnit() {
  Con_Group* to_out = (Con_Group*)send.Gp(0); // first group is outputs
  if(to_out == NULL) {
    taMisc::Error("*** MoeGateUnit: expecting send.gp[0] to be one-to-one prjn",
		  "to output layer, did not find con group");
    return NULL;
  }
  MoeOutputUnit* out_u = (MoeOutputUnit*)to_out->Un(0);
  if(out_u == NULL) {
    taMisc::Error("*** MoeGateUnit: expecting send.gp[0] to be one-to-one prjn",
		  "to output layer, did not find unit [0]");
    return NULL;
  }
  return out_u;
}

void MoeGateLayer::Initialize() {
  units.SetBaseType(&TA_MoeGateUnit);
  unit_spec.SetBaseType(&TA_MoeGateUnitSpec);
  act_sum = 0.0f;
}

void MoeGateLayer::Compute_Net() {
  act_sum = 0.0f;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i) {
    u->Compute_Net();
    act_sum += u->act;
  }
}
    
void MoeGateLayer::Compute_Act() {
  MoeGateUnit* u;
  taLeafItr i;
  FOR_ITR_EL(MoeGateUnit, u, units., i)
    u->Compute_Act(this);	// pass layer for normalization
}

/////////////////////////////////
//     Processes               //
/////////////////////////////////

void MoeTrial::Initialize() {
  sub_proc_type = &TA_MoeMStep;
  m_step.SetMax(1);
}

void MoeTrial::InitLinks() {
  BpTrial::InitLinks();
  taBase::Own(m_step, this);
}

bool MoeTrial::CheckUnit(Unit* ck) {
  bool rval = BpTrial::CheckUnit(ck);
  if(!rval) return rval;
  if(ck->InheritsFrom(&TA_MoeOutputUnit)) {
    MoeOutputUnit* mu = (MoeOutputUnit*)ck;
    BpUnit* bu = mu->GetExpertUnit();
    if(bu == NULL) return false;
    MoeGateUnit* mg = mu->GetGatingUnit();
    if(mg == NULL) return false;
    if(!mg->InheritsFrom(TA_MoeGateUnit)) {
      taMisc::Error("MoeOuputUnit did not find MoeGateUnit in recv.gp[1]",
		    "which is required by algorithm.");
      return false;
    }
  }
  if(ck->InheritsFrom(&TA_MoeGateUnit)) {
    MoeGateUnit* mu = (MoeGateUnit*)ck;
    MoeOutputUnit* mo = mu->GetOutputUnit();
    if(mo == NULL) return false;
    if(!mo->InheritsFrom(TA_MoeOutputUnit)) {
      taMisc::Error("MoeGateUnit did not find MoeOutputUnit in send.gp[0]",
		    "which is required by algorithm.");
      return false;
    }
  }
  return rval;
}


void MoeTrial::Loop() {
  // on first pass through, present event, do acts, etc
  if(m_step.val == 0) {
    if(network == NULL) return;
    network->InitExterns();
    if(cur_event != NULL)
      cur_event->ApplyPatterns(network);

    Compute_Act();
  }
  // testing does nothing..
  if((epoch_proc == NULL) || (epoch_proc->wt_update == EpochProcess::TEST)) {
    Compute_Error();		// for display purposes only..
  }
  else {
    // this case relies on standard epoch-based calling of UpdateWeights
    if(m_step.max == 1) {
      Compute_dEdA_dEdNet();
      Compute_dWt();
    }
    // otherwise, we need to do it by iterating over m-steps
    else {
      if(sub_proc != NULL) sub_proc->Run();
    }
  }
}

void MoeMStep::Initialize() {
  sub_proc_type = NULL;
}

void MoeMStep::Compute_dEdA_dEdNet() {
  // send the error back
  Layer* layer;
  int i;
  for(i=network->layers.leaves-1; i>= 0; i--) {
    layer = ((Layer*) network->layers.Leaf(i));
    if(layer->lesion || (layer->ext_flag & Unit::EXT)) // don't compute err on inputs
      continue;

    BpUnit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(BpUnit, u, layer->units., u_itr)
      u->Compute_dEdA_dEdNet();
  }
}

void MoeMStep::Compute_dWt() {
  if(network == NULL)   return;
  network->Compute_dWt();
}

void MoeMStep::UpdateWeights() {
  if(network == NULL)   return;
  network->UpdateWeights();
}

void MoeMStep::Loop() {
  // it is assumed that this is only called if m_step > 1 and not testing
  Compute_dEdA_dEdNet();
  Compute_dWt();
  UpdateWeights();	// weights need updated, else nothing happens over iters
}


/////////////////////////////////
//     Projection Spec         //
/////////////////////////////////

void ManyToOnePrjnSpec::Initialize() {
  send_n = -1;
}

void ManyToOnePrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  
  int i;
  int max_recv = n_conns;
  if(n_conns < 0)
    max_recv = prjn->layer->units.leaves - recv_start;
  max_recv = MIN(prjn->layer->units.leaves - recv_start, max_recv);
  int max_send = send_n;
  max_send = MIN(prjn->from->units.leaves - send_start, max_send);
  for(i=0; i<max_recv; i++) {
    Unit* ru = (Unit*)prjn->layer->units.Leaf(recv_start + i);
    int j;
    for(j=0; j<max_send; j++) {
      Unit* su = (Unit*)prjn->from->units.Leaf(send_start + j);
      if(self_con || (ru != su))
	ru->ConnectFrom(su, prjn);
    }
  }
}
