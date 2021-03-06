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

// bp.cc

#include <bp/bp.h>
#include <pdp/enviro.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#ifdef DMEM_COMPILE    
#include <mpi.h>
#endif

//////////////////////////
//  	Con, Spec	//
//////////////////////////

void BpConSpec::Initialize() {
  min_obj_type = &TA_BpCon_Group;
  min_con_type = &TA_BpCon;
  lrate = .25f;
  cur_lrate = .25f;
  lrate_sched.interpolate = false;
  momentum_type = BEFORE_LRATE;
  momentum = .9f;
  momentum_c = .1f;
  decay = 0.0f;
  decay_fun = NULL;
}

void BpConSpec::InitLinks() {
  ConSpec::InitLinks();
  taBase::Own(lrate_sched, this);
}

void BpConSpec::UpdateAfterEdit() {
  ConSpec::UpdateAfterEdit();
  lrate_sched.UpdateAfterEdit();
  momentum_c = 1.0f - momentum;
}

void BpConSpec::SetCurLrate(int epoch) {
  cur_lrate = lrate * lrate_sched.GetVal(epoch);
}

void BpCon_Group::Initialize() {
  spec.SetBaseType(&TA_BpConSpec);
}

void Bp_Simple_WtDecay(BpConSpec* spec, BpCon* cn, BpUnit*, BpUnit*) {
  cn->dEdW -= spec->decay * cn->wt;
}

void Bp_WtElim_WtDecay(BpConSpec* spec, BpCon* cn, BpUnit*, BpUnit*) {
  float denom = (1.0f + (cn->wt * cn->wt));
  cn->dEdW -= spec->decay * ((2.0f * cn->wt * cn->wt) / (denom * denom));
}


//////////////////////////
//  	Unit, Spec	//
//////////////////////////

void BpUnitSpec::Initialize() {
  min_obj_type = &TA_BpUnit;
  bias_con_type = &TA_BpCon;
  bias_spec.SetBaseType(&TA_BpConSpec);
  err_tol = 0.0f;
  err_fun = Bp_Squared_Error;
}

void BpUnitSpec::InitLinks() {
  UnitSpec::InitLinks();
  taBase::Own(sig, this);
}

void BpUnitSpec::CutLinks() {
  UnitSpec::CutLinks(); 
}

void BpUnitSpec::Copy_(const BpUnitSpec& cp) {
  sig = cp.sig;
  err_tol = cp.err_tol;
  err_fun = cp.err_fun;
}

void BpUnitSpec::SetCurLrate(BpUnit* u, int epoch) {
  ((BpConSpec*)bias_spec.spec)->SetCurLrate(epoch);
  BpCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(BpCon_Group, recv_gp, u->recv., g)
    recv_gp->SetCurLrate(epoch);
}

void BpUnitSpec::InitState(Unit* u) {
  UnitSpec::InitState(u);
  BpUnit* bu = (BpUnit*)u;
  bu->err = bu->dEdA = bu->dEdNet = 0.0f;
}

void BpUnitSpec::Compute_Act(Unit* u) { // this does the sigmoid
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else
    u->act = act_range.Project(sig.Eval(u->net));
}

void BpUnitSpec::Compute_Error(BpUnit* u) {
  if(u->ext_flag & Unit::TARG) (*err_fun)(this, u);
}

void BpUnitSpec::Compute_dEdA(BpUnit* u) {
  u->dEdA = 0.0f;
  u->err = 0.0f;
  BpCon_Group* send_gp;
  int g;
  FOR_ITR_GP(BpCon_Group, send_gp, u->send., g) {
    if(!send_gp->prjn->layer->lesion)
      u->dEdA += send_gp->Compute_dEdA(u);
  }
}

void BpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = u->dEdA * sig.gain * (u->act - act_range.min) *
    (act_range.max - u->act) * act_range.scale;
}

void BpUnitSpec::Compute_dWt(Unit* u) {
  if(u->ext_flag & Unit::EXT)  return; // don't compute dwts for clamped units
  UnitSpec::Compute_dWt(u);
  ((BpConSpec*)bias_spec.spec)->B_Compute_dWt((BpCon*)u->bias, (BpUnit*)u);
}

void BpUnitSpec::UpdateWeights(Unit* u) {
  if(u->ext_flag & Unit::EXT)  return; // don't update for clamped units
  UnitSpec::UpdateWeights(u);
  ((BpConSpec*)bias_spec.spec)->B_UpdateWeights((BpCon*)u->bias, (BpUnit*)u);
}

void BpUnitSpec::GraphActFun(GraphLog* graph_log, float min, float max) {
  if(graph_log == NULL) {
    graph_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(graph_log == NULL) return;
  }
  graph_log->name = name + ": Act Fun";
  DataTable* dt = &(graph_log->data);
  dt->Reset();
  dt->NewColFloat("netin");
  dt->NewColFloat("act");

  BpUnit un;

  float x;
  for(x = min; x <= max; x += .01f) {
    un.net = x;
    Compute_Act(&un);
    dt->AddBlankRow();
    dt->SetLastFloatVal(x, 0);
    dt->SetLastFloatVal(un.act, 1);
  }
  dt->UpdateAllRanges();
  graph_log->ViewAllData();
}

void BpUnit::Initialize() {
  spec.SetBaseType(&TA_BpUnitSpec);
  err = dEdA = dEdNet = 0.0f;
}

void Bp_Squared_Error(BpUnitSpec* spec, BpUnit* u) {
  float err = u->targ - u->act;
  if(fabs(err) < spec->err_tol) {
    u->err = 0.0f;
  }
  else {
    u->dEdA += err;
    u->err = err * err;
  }
}

void Bp_CrossEnt_Error(BpUnitSpec* spec, BpUnit* u) {
  float err = u->targ - u->act;
  if(fabs(err) < spec->err_tol) {
    u->err = 0.0f;
  }
  else {
    err /= (u->act - spec->act_range.min) * (spec->act_range.max - u->act)
      * spec->act_range.scale;
    float a = spec->sig.Clip(spec->act_range.Normalize(u->act));
    float t = spec->act_range.Normalize(u->targ);
    u->dEdA += err;
    u->err = (t * logf(a) + (1.0f - t) * logf(1.0f - a));
  }
}


void BpUnit::Copy_(const BpUnit& cp) {
  err = cp.err;
  dEdA = cp.dEdA;
  dEdNet = cp.dEdNet;
}

//////////////////////////
//  	Procs		//
//////////////////////////

void BpTrial::Initialize() {
  min_unit = &TA_BpUnit;
  min_con_group = &TA_BpCon_Group;
  min_con = &TA_BpCon;
  bp_to_inputs = false;
}

void BpTrial::InitLinks() {
  TrialProcess::InitLinks();
  if(!taMisc::is_loading && (final_stats.size == 0)) {
    SE_Stat* st = (SE_Stat*)final_stats.NewEl(1, &TA_SE_Stat);
    st->CreateAggregates(Aggregate::SUM);
  }
}

void BpTrial::SetCurLrate() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    BpUnit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(BpUnit, u, layer->units., u_itr)
      u->SetCurLrate(network->epoch);
  }
}

void BpTrial::Compute_Act() {
  // compute activations
  Layer* lay;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, lay, network->layers., l_itr) {
    if(lay->lesion)	continue;
    if(!(lay->ext_flag & Unit::EXT)) {
      lay->Compute_Net();
#ifdef DMEM_COMPILE    
      lay->DMem_SyncNet();
#endif
    }
    lay->Compute_Act();
  }
}

void BpTrial::Compute_Error() {
  // compute errors
  Layer* lay;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, lay, network->layers., l_itr) {
    if(lay->lesion || !(lay->ext_flag & Unit::TARG)) // only compute err on targs
      continue;

    BpUnit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(BpUnit, u, lay->units., u_itr) {
      u->dEdA = 0.0f;		// must reset -- error is incremental!
      u->Compute_Error();
    }
  }
}

void BpTrial::Compute_dEdA_dEdNet() {
  // send the error back
  Layer* lay;
  int i;
  for(i=network->layers.leaves-1; i>= 0; i--) {
    lay = ((Layer*) network->layers.Leaf(i));
    if(lay->lesion || (!bp_to_inputs && (lay->ext_flag & Unit::EXT))) // don't compute err on inputs
      continue;

    BpUnit* u;
    taLeafItr u_itr;
#ifdef DMEM_COMPILE
    // first compute dEdA from connections and share it
    FOR_ITR_EL(BpUnit, u, lay->units., u_itr)
      u->Compute_dEdA();
    lay->dmem_share_units.Aggregate(3, MPI_SUM);

    // then compute error to add to dEdA, and dEdNet
    FOR_ITR_EL(BpUnit, u, lay->units., u_itr) {
      u->Compute_Error();
      u->Compute_dEdNet();
    }
#else
    FOR_ITR_EL(BpUnit, u, lay->units., u_itr)
      u->Compute_dEdA_dEdNet();
#endif
  }
}

void BpTrial::Compute_dWt() {
  if(network == NULL)	return;
  network->Compute_dWt();
}

void BpTrial::Loop() {
  if(network == NULL) return;
  
  SetCurLrate();

  network->InitExterns();
  if(cur_event != NULL)
    cur_event->ApplyPatterns(network);

  Compute_Act();
  Compute_dEdA_dEdNet();
  
  // compute the weight err derivatives (only if not testing...)
  if((epoch_proc != NULL) && (epoch_proc->wt_update != EpochProcess::TEST)) {
    Compute_dWt();
  }
//   else {
//     Compute_Error();		// for display purposes only..
//   }

  // weight update taken care of by the epoch process
}

bool BpTrial::CheckNetwork() {
  if(network && (network->dmem_sync_level != Network::DMEM_SYNC_LAYER)) {
    network->dmem_sync_level = Network::DMEM_SYNC_LAYER;
  }
  return TrialProcess::CheckNetwork();
}


//////////////////////////
// 	CE_Stat		//
//////////////////////////

void CE_Stat::Initialize() {
  tolerance = 0.0f;
}

void CE_Stat::NameStatVals() {
  Stat::NameStatVals();
  ce.AddDispOption("MIN=0");
}

void CE_Stat::InitStat() {
  float init_val = InitStatVal();
  ce.InitStat(init_val);
  InitStat_impl();
}

void CE_Stat::Init() {
  ce.Init();
  Init_impl();
}

bool CE_Stat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return ce.Crit();
}

void CE_Stat::Network_Init() {
  InitStat();
}

void CE_Stat::Layer_Run() {
  if(layer != NULL) {
    if(layer->lesion)      return;
    Layer_Init(layer);
    Unit_Run(layer);
    Layer_Stat(layer);
    return;
  }
  Layer *lay;
  taLeafItr i;
  FOR_ITR_EL(Layer, lay, network->layers., i) {
    // only target layers need to be checked for error
    if(lay->lesion || !((lay->ext_flag & Unit::TARG) || (lay->ext_flag & Unit::COMP)))
      continue;
    Layer_Init(lay);
    Unit_Run(lay);
    Layer_Stat(lay);
  }
}

void CE_Stat::Unit_Stat(Unit* unit) {
  if(!((unit->ext_flag & Unit::TARG) || (unit->ext_flag & Unit::COMP)))
    return;

  float tmp = fabs(unit->targ - unit->act);
  if(tmp < tolerance) {
    net_agg.ComputeAgg(&ce, 0.0f);
    return;
  }

  UnitSpec* us = unit->spec.spec;
  if(us == NULL)	return;

  float a = SigmoidSpec::Clip(us->act_range.Normalize(unit->act));
  float t = us->act_range.Normalize(unit->targ);
  
  if(t == 1.0f)
    tmp = -logf(a);
  else if(t == 0.0f)
    tmp = -logf(1.0f - a);
  else
    tmp = t * logf(t/a) + (1.0f - t) * logf((1.0f - t) / (1.0f - a));
  net_agg.ComputeAgg(&ce, tmp);
}

/////////////////////////////////
//        NormDotProd_Stat     //
/////////////////////////////////

void NormDotProd_Stat::Initialize(){
  ndp = 0.0f;
  net_agg.op = Aggregate::AVG;
}


void NormDotProd_Stat::NameStatVals() {
  Stat::NameStatVals();
  ndp.AddDispOption("MIN=0");
}

void NormDotProd_Stat::InitStat() {
  float init_val = InitStatVal();
  ndp.InitStat(init_val);
  InitStat_impl();
}

void NormDotProd_Stat::Init() {
  ndp.Init();
  Init_impl();
}

bool NormDotProd_Stat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return ndp.Crit();
}

void NormDotProd_Stat::Network_Init() {
  InitStat();
}

void NormDotProd_Stat::Layer_Run() {
  if(layer != NULL) {
    if(layer->lesion)      return;
    Layer_Init(layer);
    Unit_Run(layer);
    Layer_Stat(layer);
    return;
  }
  Layer *lay;
  taLeafItr i;
  FOR_ITR_EL(Layer, lay, network->layers., i) {
    // only target layers need to be checked for error
    if(lay->lesion || !((lay->ext_flag & Unit::TARG) || (lay->ext_flag & Unit::COMP)))
      continue;
    Layer_Init(lay);
    Unit_Run(lay);
    Layer_Stat(lay);
  }
}

void NormDotProd_Stat::Unit_Stat(Unit* unit) {
  if(!((unit->ext_flag & Unit::TARG) || (unit->ext_flag & Unit::COMP)))
    return;

  float tmp = unit->targ * unit->act;
  net_agg.ComputeAgg(&ndp, tmp);
  return;
}


////////////////////////////
//        VecCor_Stat     //
////////////////////////////

void VecCor_Stat::Initialize(){
  vcor = 0.0f;
}


void VecCor_Stat::NameStatVals() {
  Stat::NameStatVals();
  vcor.AddDispOption("MIN=0");
}

void VecCor_Stat::InitStat() {
  float init_val = InitStatVal();
  vcor.InitStat(init_val);
  dp = l1 = l2 =  0.0f;
  InitStat_impl();
}

void VecCor_Stat::Init() {
  vcor.Init();
  Init_impl();
}

bool VecCor_Stat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return vcor.Crit();
}

void VecCor_Stat::Network_Init() {
  InitStat();
}

void VecCor_Stat::Layer_Run() {
  if(layer != NULL) {
    if(layer->lesion)      return;
    Layer_Init(layer);
    Unit_Run(layer);
    Layer_Stat(layer);
    return;
  }
  Layer *lay;
  taLeafItr i;
  FOR_ITR_EL(Layer, lay, network->layers., i) {
    // only target layers need to be checked for error
    if(lay->lesion || !((lay->ext_flag & Unit::TARG) || (lay->ext_flag & Unit::COMP)))
      continue;
    Layer_Init(lay);
    Unit_Run(lay);
    Layer_Stat(lay);
  }
}

void VecCor_Stat::Unit_Stat(Unit* unit) {
  if(!((unit->ext_flag & Unit::TARG) || (unit->ext_flag & Unit::COMP)))
    return;
  dp += unit->targ * unit->act ;
  l1 += unit->targ * unit->targ;
  l2 += unit->act * unit->act;
  return;
}

void VecCor_Stat::Network_Stat(){
  if (l1 == 0.0 || l2 == 0.0)
    vcor = (float) 0.0f;
  else vcor = (float) (dp/sqrt(l1*l2));
}


////////////////////////////////
//        NormVecLen_Stat     //
////////////////////////////////

void NormVecLen_Stat::Initialize(){
  nvl = 0.0f;
  net_agg.op = Aggregate::AVG;
}


void NormVecLen_Stat::NameStatVals() {
  Stat::NameStatVals();
  nvl.AddDispOption("MIN=0");
}

void NormVecLen_Stat::InitStat() {
  float init_val = InitStatVal();
  nvl.InitStat(init_val);
  InitStat_impl();
}

void NormVecLen_Stat::Init() {
  nvl.Init();
  Init_impl();
}

bool NormVecLen_Stat::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return nvl.Crit();
}

void NormVecLen_Stat::Network_Init() {
  InitStat();
}

void NormVecLen_Stat::Layer_Run() {
  if(layer != NULL) {
    if(layer->lesion)      return;
    Layer_Init(layer);
    Unit_Run(layer);
    Layer_Stat(layer);
    return;
  }
  Layer *lay;
  taLeafItr i;
  FOR_ITR_EL(Layer, lay, network->layers., i) {
    // only target layers need to be checked for error
    if(lay->lesion || !((lay->ext_flag & Unit::TARG) || (lay->ext_flag & Unit::COMP)))
      continue;
    Layer_Init(lay);
    Unit_Run(lay);
    Layer_Stat(lay);
  }
}

void NormVecLen_Stat::Unit_Stat(Unit* unit) {
  if(!((unit->ext_flag & Unit::TARG) || (unit->ext_flag & Unit::COMP)))
    return;

  float tmp = unit->act * unit->act;
  net_agg.ComputeAgg(&nvl, tmp);
  return;
}

void NormVecLen_Stat::Network_Stat() {
  nvl = (float) sqrtf(nvl);
}

//////////////////////////////////////////
//	Additional Con Types		//
//////////////////////////////////////////

void DeltaBarDeltaBpConSpec::Initialize() {
  min_con_type = &TA_DeltaBarDeltaBpCon;
  lrate_incr = .1;
  lrate_decr = .9;
  act_lrate_incr = lrate * lrate_incr;
}

void DeltaBarDeltaBpConSpec::UpdateAfterEdit() {
  BpConSpec::UpdateAfterEdit();
  act_lrate_incr = lrate * lrate_incr;
}

void DeltaBarDeltaBpConSpec::Copy_(const DeltaBarDeltaBpConSpec& cp) {
  lrate_incr = cp.lrate_incr;
  lrate_decr = cp.lrate_decr;
}


//////////////////////////////////////////
//	Additional Unit Types		//
//////////////////////////////////////////

//////////////////////////
//  	Context		//
//////////////////////////

void BpContextSpec::Initialize() {
  hysteresis = .3;
  hysteresis_c = .7;
  initial_act.var = 0;
  initial_act.mean = .5;
  variable = "act";
  unit_flags = Unit::NO_EXTERNAL;
}

void BpContextSpec::InitLinks() {
  BpUnitSpec::InitLinks();
  taBase::Own(initial_act, this);
}

void BpContextSpec::Copy_(const BpContextSpec& cp) {
  hysteresis = cp.hysteresis;
  hysteresis_c = cp.hysteresis_c;
  initial_act = cp.initial_act;
  variable = cp.variable;
  unit_flags = cp.unit_flags;
}

void BpContextSpec::UpdateAfterEdit() {
  BpUnitSpec::UpdateAfterEdit();
  hysteresis_c = 1.0f - hysteresis;
  var_md = TA_BpUnit.members.FindName(variable);
  if(var_md == NULL)
    taMisc::Error("BpContextSpec: could not find variable:",variable,"in BpUnit type");
}

bool BpContextSpec::CheckConfig(Unit* un, Layer* lay, TrialProcess* tp) {
  if(!BpUnitSpec::CheckConfig(un, lay, tp)) return false;
  if(var_md == NULL) {
    taMisc::Error("BpContextSpec: could not find variable:",variable,"in BpUnit type");
    return false;
  }
  Con_Group* recv_gp = (Con_Group*)un->recv.SafeGp(0); // first group
  if(recv_gp == NULL) {
    taMisc::Error("BpContextSpec: expecting one one-to-one projection from layer",
		   "did not find con group");
    return false;
  }
  Unit* hu = (Unit*)recv_gp->Un(0);
  if(hu == NULL) {
    taMisc::Error("BpContextSpec: expecting one one-to-one projection from layer",
		   "did not find unit");
    return false;
  }
  int myidx = lay->own_net->layers.FindLeaf(lay);
  int fmidx = lay->own_net->layers.FindLeaf(recv_gp->prjn->from);
  if(myidx < fmidx) {
    taMisc::Error("BpContextSpec: context layer:", lay->name, "must be AFTER layer it copies from:",
		  recv_gp->prjn->from->name, "in list of .layers");
    return false;
  }
  return true;
}

void BpContextSpec::InitState(Unit* u) {
  BpUnitSpec::InitState(u);
  u->act = initial_act.Gen();
}

void BpContextSpec::Compute_Act(Unit* u) {
  Con_Group* recv_gp = (Con_Group*)u->recv.SafeGp(0); // first group
  Unit* hu = (Unit*)recv_gp->Un(0);
  float* varptr = (float*)var_md->GetOff((void*)u);
  *varptr = hysteresis_c * hu->act + hysteresis * (*varptr);
  u->SetExtFlag(unit_flags);
}


//////////////////////////
//  	Linear		//
//////////////////////////

void LinearBpUnitSpec::Initialize() {
  SetUnique("act_range", true);
  act_range.min = -1e20;
  act_range.max = 1e20;
}

void LinearBpUnitSpec::UpdateAfterEdit() {
  BpUnitSpec::UpdateAfterEdit();
  if(err_fun == Bp_CrossEnt_Error) {
    taMisc::Error("LinearBpUnitSpec: Cross entropy error is incompatible with Linear Units!  I switched to Squared_Error for you.");
    SetUnique("err_fun", true);
    err_fun = Bp_Squared_Error;
  }
}

void LinearBpUnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = act_range.Clip(u->ext);
  else 
    u->act = act_range.Clip(u->net);
}

void LinearBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = u->dEdA;		// that's pretty easy!
}

//////////////////////////
//  	ThreshLinear	//
//////////////////////////

void ThreshLinBpUnitSpec::Initialize() {
  SetUnique("act_range", true);
  act_range.min = -1e20;
  act_range.max = 1e20;
  threshold = 0.0f;
}

void ThreshLinBpUnitSpec::UpdateAfterEdit() {
  BpUnitSpec::UpdateAfterEdit();
  if(err_fun == Bp_CrossEnt_Error) {
    taMisc::Error("ThreshLinBpUnitSpec: Cross entropy error is incompatible with Linear Units!  I switched to Squared_Error for you.");
    SetUnique("err_fun", true);
    err_fun = Bp_Squared_Error;
  }
}

void ThreshLinBpUnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = act_range.Clip(u->ext);
  else 
    u->act = act_range.Clip((u->net > threshold) ? (u->net - threshold) : 0.0f);
}

void ThreshLinBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  // derivative is 1 in linear part, 0 elsewhere
  u->dEdNet = (u->net > threshold) ? u->dEdA : 0.0f;
}



//////////////////
//    Noisy	//
//////////////////

void NoisyBpUnitSpec::Initialize() {
  noise.type = Random::GAUSSIAN;
  noise.var = .1;
}

void NoisyBpUnitSpec::InitLinks() {
  BpUnitSpec::InitLinks();
  taBase::Own(noise, this);
}

void NoisyBpUnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = act_range.Clip(u->ext + noise.Gen());
  else   // need to keep in SigmoidSpec clipped range!
    u->act = act_range.min + act_range.range * 
      SigmoidSpec::Clip(sig.Eval(u->net) + noise.Gen());
}


//////////////////////////
// Stochastic Unit Spec //
//////////////////////////


void StochasticBpUnitSpec::Compute_Act(Unit* u) { // this does the probabiltiy on sigmoid
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else {
    float prob =  sig.Eval(u->net);
    float nw_act = (Random::ZeroOne() < prob) ? 1.0f : 0.0f;
    u->act = act_range.Project(nw_act);
  }
}


//////////////////////////
//         RBF          //
//////////////////////////

void RBFBpUnitSpec::Initialize() {
  var = 1.0f;
  norm_const = 1.0f / sqrtf(2.0f * 3.14159265358979323846 * var);
  denom_const = 0.5f / var;
}

void RBFBpUnitSpec::UpdateAfterEdit() {
  BpUnitSpec::UpdateAfterEdit();
  norm_const = 1.0f / sqrtf(2.0f * 3.14159265358979323846 * var);
  denom_const = 0.5f / var;
}

void RBFBpUnitSpec::Compute_Net(Unit* u) {
  // do distance instead of net input
  u->net = 0.0f;
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g)
    u->net += recv_gp->Compute_Dist(u);
}

void RBFBpUnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else
    u->act = norm_const * expf(-denom_const * u->net);
}

void RBFBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = - u->dEdA * u->act * denom_const;
}


//////////////////////////
//       Bump           //
//////////////////////////

void BumpBpUnitSpec::Initialize() {
  mean = 0.0f;
  std_dev = 1.0f;
  std_dev_r = 1.0f;
}

void BumpBpUnitSpec::UpdateAfterEdit() {
  BpUnitSpec::UpdateAfterEdit();
  std_dev_r = 1.0f / std_dev;
}


void BumpBpUnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else {
    float val = std_dev_r * (u->net - mean);
    u->act = expf(- (val * val));
  }
}

void BumpBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  // dadnet = a * 2 * (net - mean) / std_dev
  u->dEdNet = - u->dEdA * u->act * 2.0f * (u->net - mean) * std_dev_r;
}

//////////////////////////
//   Exp, SoftMax       //
//////////////////////////

void ExpBpUnitSpec::Compute_Act(Unit* u) {
  float net = sig.gain * u->net;
  net = MAX(net, -50.0f);
  net = MIN(net, 50.0f);
  u->act = expf(net);
}

void ExpBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = u->dEdA * sig.gain * u->act;
}

void SoftMaxBpUnitSpec::Compute_Act(Unit* u) {
  if((u->recv.gp.size < 2) || (((Con_Group*)u->recv.gp[0])->size == 0)
     || (((Con_Group*)u->recv.gp[1])->size == 0)) {
    taMisc::Error("*** SoftMaxBpUnitSpec: expecting one one-to-one projection from",
		  "exponential units (in first projection) and from linear sum unit (in second), did not find these.");
    return;
  }
  BpUnit* exp_unit = (BpUnit*)((Con_Group*)u->recv.gp[0])->Un(0);
  BpUnit* sum_unit = (BpUnit*)((Con_Group*)u->recv.gp[1])->Un(0);

  float sum_act = sum_unit->act;
  if(sum_act < FLT_MIN)
    sum_act = FLT_MIN;
  u->act = exp_unit->act / sum_unit->act;
  if(u->act < FLT_MIN)
    u->act = FLT_MIN;
}

void SoftMaxBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  // effectively linear
  u->dEdNet = u->dEdA;
}

