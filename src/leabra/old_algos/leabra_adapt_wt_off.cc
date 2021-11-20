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
#include "bleabra.h"


//////////////////////////
//  	Con, Spec	//
//////////////////////////

void LeabraCon::Copy_(const LeabraCon& cp) {
  dwt = cp.dwt;
  pdw = cp.pdw;
}

void LeabraConSpec::Initialize() {
  min_obj_type = &TA_LeabraCon_Group;
  min_con_type = &TA_LeabraCon;
  rnd.mean = .5f;
  rnd.var = .25f;

  lrate = .01f;
  k_hebb = .01f;
  k_err = 1.0f - k_hebb;
  soft_bound = SB_ERR;
  savg_src = SLAYER_AVG_ACT;
  hebb_type = PSR_MAX_COR;
  fixed_savg = .25f;
  k_savg_cor = 1.0f;
}

void LeabraConSpec::InitLinks() {
  BioBelConSpec::InitLinks();
}

void LeabraConSpec::UpdateAfterEdit() {
  BioBelConSpec::UpdateAfterEdit();
  k_err = 1.0f - k_hebb;
}

void LeabraCon_Group::Initialize() {
  spec.SetBaseType(&TA_LeabraConSpec);
  el_typ = &TA_LeabraCon;
  savg = 0.0f;
  max_cor = inc_cor = dec_cor = 1.0f;
}

void LeabraBiasSpec::Initialize() {
  min_obj_type = &TA_LeabraUnitSpec;
  SetUnique("rnd", true);
  SetUnique("wt_limits", true);
  rnd.mean = 0.0f;
  rnd.var = 0.0f;
  wt_limits.min = -1.0f;
  wt_limits.max = 5.0f;
  wt_limits.sym = false;
  wt_limits.type = WeightLimits::NONE;
  dwt_thresh = .1f;
}


//////////////////////////
//  	Unit, Spec	//
//////////////////////////

void LeabraUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraUnit;
  bias_con_type = &TA_LeabraCon;
  bias_spec.SetBaseType(&TA_LeabraConSpec);
}

void LeabraUnitSpec::InitLinks() {
  BioBelUnitSpec::InitLinks();
  bias_spec.type = &TA_LeabraBiasSpec;
}

void LeabraUnitSpec::InitWtState(Unit* u) {
  BioBelUnitSpec::InitWtState(u); 
  bias_spec->C_InitWtState(u->bias, u, NULL);
  LeabraUnit* lu = (LeabraUnit*)u;
  lu->act_p = lu->act_m = lu->act_dif = 0.0f;
}

void LeabraUnitSpec::PhaseInit(BioBelUnit* u, BioBelLayer*, int phase) {
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

void LeabraUnitSpec::PostSettle(BioBelUnit* u, BioBelLayer* lay, BioBelInhib* thr,
				int phase)
{
  // don't call parent, and do basis updating in plus phase only..
  //  BioBelUnitSpec::PostSettle(u, lay, thr, phase);
  
  LeabraUnit* lu = (LeabraUnit*)u;
  if(phase == LeabraTrial::MINUS_PHASE) {
    lu->act_m = lu->act;
  }
  else {
    lu->act_p = lu->act;
    if(!(lu->ext_flag & (Unit::EXT | Unit::TARG))) {
      lu->act_avg += adapt_thr.dt * (lu->act_p - lu->act_avg);
      AdaptThreshold(u, lay, thr, phase);
      Compute_AvgAct_dWt(lu);
    }
  }
  lu->act_dif = lu->act_p - lu->act_m;
}

void LeabraUnitSpec::Compute_AvgAct_dWt(LeabraUnit* u) {
  if(u->act_avg > adapt_thr.max) {
    u->act_p -= adapt_thr.err; 	// subtract off error amount
    u->act_p = act_range.Clip(u->act_p);
  }
  else if(u->act_avg < adapt_thr.min) {
    u->act_p += adapt_thr.err; 	// add in error amount
    u->act_p = act_range.Clip(u->act_p);
  }
}

void LeabraUnitSpec::Compute_dWt(Unit* u) {
  LeabraUnit* lu = (LeabraUnit*)u;
  if((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn))
    return;
  Compute_dWt_impl(lu);
}

void LeabraUnitSpec::Compute_dWt_impl(LeabraUnit* u) {
  // don't adapt bias weights on clamped inputs..
  if(!((u->ext_flag & Unit::EXT) && !(u->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_Compute_dWt((LeabraCon*)u->bias, u);
  }
  BioBelUnitSpec::Compute_dWt(u);
}

void LeabraUnitSpec::Compute_dWt_post(LeabraUnit*, LeabraLayer*) {
}

void LeabraUnitSpec::UpdateWeights(Unit* u) {
  LeabraUnit* lu = (LeabraUnit*)u;
  if(!((lu->ext_flag & Unit::EXT) && !(lu->ext_flag & Unit::TARG))) {
    ((LeabraConSpec*)bias_spec.spec)->B_UpdateWeights((LeabraCon*)u->bias, lu);
  }
  if(((lu->act_p > opt_thresh.learn) || (lu->act_m > opt_thresh.learn)))
    BioBelUnitSpec::UpdateWeights(lu);
}

void LeabraUnit::Initialize() {
  act_p = act_m = act_dif = 0.0f;
  spec.SetBaseType(&TA_LeabraUnitSpec);
}

void LeabraUnit::InitLinks() {
  BioBelUnit::InitLinks();
}


//////////////////////////
//  	LayerSpec	//
//////////////////////////

void LeabraLayerSpec::Initialize() {
  min_obj_type = &TA_LeabraLayer;
}

void LeabraLayerSpec::PhaseInit(BioBelLayer* lay, int phase) {
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

void LeabraLayerSpec::PostSettle(BioBelLayer* lay, int phase) {
  BioBelLayerSpec::PostSettle(lay, phase);
  LeabraLayer* llay = (LeabraLayer*)lay;
  if(phase == LeabraTrial::MINUS_PHASE) {
    llay->acts_m = llay->acts;
  }
  else {
    llay->acts_p = llay->acts;
  }
}

void LeabraLayerSpec::Compute_dWt_post(LeabraLayer* lay) {
  LeabraUnit* u;
  taLeafItr i;
  FOR_ITR_EL(LeabraUnit, u, lay->units., i)
    u->Compute_dWt_post(lay);
}

void LeabraLayer::Initialize() {
  spec.SetBaseType(&TA_LeabraLayerSpec);
  units.SetBaseType(&TA_LeabraUnit);
}

void LeabraLayer::InitLinks() {
  BioBelLayer::InitLinks();
  taBase::Own(acts_p, this);
  taBase::Own(acts_m, this);
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

void CascadeParams::Initialize() {
  cycles = 3;
  depth = 0;
  max = 1;
}

void LeabraSettle::Initialize() {
  sub_proc_type = &TA_LeabraCycle;
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  leabra_trial = NULL;
  cycle.max = 60;
  min_cycles = 15;
  use_cascade = false;
}

void LeabraSettle::InitLinks() {
  SettleProcess::InitLinks();
  taBase::Own(cascade, this);
  if(!taMisc::is_loading && (loop_stats.size == 0)) {
    loop_stats.NewEl(1, &TA_BioBelMaxDa);
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
  BioBelLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(BioBelLayer, lay, network->layers., l) {
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

void LeabraSettle::DecayAE() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayAE();
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

void LeabraSettle::CompToTarg() {
  BioBelLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(BioBelLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->CompToTarg();
  }
}

void LeabraSettle::ExtToComp() {
  BioBelLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(BioBelLayer, lay, network->layers., l) {
    if(lay->lesion || !(lay->ext_flag & Unit::TARG_EXT))
      continue;
    lay->ext_flag = Unit::COMP;
    
    BioBelUnit* u;
    taLeafItr i;
    FOR_ITR_EL(BioBelUnit, u, lay->units., i) {
      if(!(u->ext_flag & Unit::TARG_EXT))
	continue;
      u->ext_flag = Unit::COMP;
      u->targ = u->ext;
      u->ext = 0.0f;
    }
  }
}

void LeabraSettle::Compute_HardClamp() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_HardClamp(leabra_trial->phase);
  }
}

void LeabraSettle::Compute_InputDist() {
  int max_depth;
  int i;
  for(i=0; i<3; i++) {		// do it at most 3 times to make sure everybody's cool
    max_depth = 0;
    int rval = 0;
    LeabraLayer* lay;
    taLeafItr l;
    FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
      if(lay->lesion) continue;
      int tmp = lay->Compute_InputDist(leabra_trial->phase);
      rval = MIN(tmp, rval);	// check for negative error conditions
      max_depth = MAX(max_depth, lay->input_dist);
    }
    if(rval == 0)		// no negative err conditions
      break;
  }
  cascade.max = max_depth;
  cascade.depth = 1;		// start at 1
}

void LeabraSettle::Compute_Cascade() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_Cascade(leabra_trial->phase, cascade.depth);
  }
}

void LeabraSettle::Compute_ClampNet() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_ClampNet(leabra_trial->phase);
  }
}

void LeabraSettle::PostSettle() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->PostSettle(leabra_trial->phase);
  }
}

void LeabraSettle::Compute_dWt() {
  // first do basic weight update
  network->Compute_dWt();

  // then post-weight update processing..
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_dWt_post();
  }
}

void LeabraSettle::Init_impl() {
  SettleProcess::Init_impl();
  if((leabra_trial == NULL) || (leabra_trial->cur_event == NULL) || (network == NULL))
    return;

  if(leabra_trial->phase_no == 2) {
    DecayAE();			    // prepare for auto-encoder nothing phase
    ExtToComp();		    // all external input is now 'comparison'
  }
  else if(leabra_trial->phase_no == 1) {
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
  if(use_cascade) {
    Compute_InputDist();		// then get all the layer input distances
    Compute_Cascade();		// then set cascading parameters
  }
  Compute_ClampNet();		// and then compute net input from clamped *once*
}	

void LeabraSettle::Loop() {
  if(use_cascade && (cascade.depth < cascade.max) &&
     (((cycle.val + 1) % cascade.cycles) == 0))
  {
    cascade.depth++;
    Compute_Cascade();
    Compute_ClampNet();		// recompute clamped net based on new cascading
  }
  SettleProcess::Loop();
}

void LeabraSettle::Final() {
  PostSettle();
  if(leabra_trial == NULL)	return;
  // compute weight changes at end of second settle..
  if((leabra_trial->phase_order == LeabraTrial::MINUS_PLUS_NOTHING) &&
     (leabra_trial->phase_no.val == 1))
    Compute_dWt();
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
  min_layer = &TA_LeabraLayer;
  min_unit = &TA_LeabraUnit;
  min_con_group =  &TA_LeabraCon_Group;
  min_con =  &TA_LeabraCon;
}

void LeabraTrial::InitLinks() {
  TrialProcess::InitLinks();
  taBase::Own(phase_no, this);
  if(!taMisc::is_loading && (loop_stats.size == 0)) {
    SE_Stat* st = (SE_Stat*)loop_stats.NewEl(1, &TA_SE_Stat);
    st->CreateAggregates(Aggregate::SUM);
  }
}

void LeabraTrial::Copy_(const LeabraTrial& cp) {
  phase_no = cp.phase_no;
  phase = cp.phase;
  no_plus_stats = cp.no_plus_stats;
  no_plus_test = cp.no_plus_test;
  trial_init = cp.trial_init;
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
  FinalStats();
  if(!log_loop)
    UpdateLogs();
  UpdateDisplays();		// update displays after the loop
  SetReInit(true);		// made it through the loop
}

void LeabraTrial::InitInhib() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->InitInhib();
  }
}

void LeabraTrial::InitState() {
  network->InitState();
}

void LeabraTrial::DecayState() {
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->DecayEvent();
  }
}

void LeabraTrial::Compute_dWt() {
  // first do basic weight update
  network->Compute_dWt();

  // then post-weight update processing..
  LeabraLayer* lay;
  taLeafItr l;
  FOR_ITR_EL(LeabraLayer, lay, network->layers., l) {
    if(!lay->lesion)
      lay->Compute_dWt_post();
  }
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
    else if(phase_order == MINUS_PLUS_NOTHING) {
      phase_no.SetMax(3);
    }
  }

  if(trial_init == INIT_STATE)
    InitState();
  else if(trial_init == DECAY_STATE)
    DecayState();
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
    if(phase_no.val == 1)
      phase = PLUS_PHASE;
    else
      phase = MINUS_PHASE;	// only occurs when val gets to 2!
  }
}

void LeabraTrial::Final() {
  TrialProcess::Final();	// do anything else

  if((epoch_proc != NULL) && (epoch_proc->wt_update != EpochProcess::TEST))
    Compute_dWt();
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

void BioBelMaxDa::Initialize() {
  settle_proc = NULL;
  min_layer = &TA_BioBelLayer;
  min_unit = &TA_BioBelUnit;
  net_agg.op = Aggregate::MAX;
  da.stopcrit.flag = true;	// defaults
  da.stopcrit.val = .02f;
  da.stopcrit.cnt = 1;
  da.stopcrit.rel = CritParam::LESSTHANOREQUAL;
  has_stop_crit = true;
  net_agg.op = Aggregate::MAX;
}

void BioBelMaxDa::UpdateAfterEdit() {
  Stat::UpdateAfterEdit();
  if(own_proc == NULL) return;
  settle_proc = (LeabraSettle*)own_proc->FindProcOfType(&TA_LeabraSettle);
}

void BioBelMaxDa::CutLinks() {
  settle_proc = NULL;
  Stat::CutLinks();
}

void BioBelMaxDa::InitStat() {
  da.InitStat(InitStatVal());
  InitStat_impl();
}

void BioBelMaxDa::Init() {
  da.Init();
  Init_impl();
}

bool BioBelMaxDa::Crit() {
  if(!has_stop_crit)    return false;
  if(n_copy_vals > 0)   return copy_vals.Crit();
  return da.Crit();
}

void BioBelMaxDa::Network_Init() {
  InitStat();
}

void BioBelMaxDa::Unit_Stat(Unit* unit) {
  float fda = fabsf(((BioBelUnit*)unit)->da);
  net_agg.ComputeAgg(&da, fda);
}

void BioBelMaxDa::Network_Stat() {
  if(settle_proc == NULL) return;
  if((settle_proc->cycle.val < settle_proc->min_cycles) ||
     (settle_proc->use_cascade &&
      (settle_proc->cascade.depth < settle_proc->cascade.max)))
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
  if(time_agg.real_stat != NULL) {
    LeabraAeSE_Stat* rstat = (LeabraAeSE_Stat*)time_agg.real_stat;
    MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
    name += String("_") + md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
  }
}

void LeabraAeSE_Stat::NameStatVals() {
  Stat::NameStatVals();
  if(time_agg.real_stat != NULL) {
    LeabraAeSE_Stat* rstat = (LeabraAeSE_Stat*)time_agg.real_stat;
    MemberDef* md = GetTypeDef()->members.FindName("targ_or_comp");
    String vlnm = md->type->GetValStr(md->GetOff((void*)rstat), (void*)rstat, md);
    vlnm.downcase();
    se.name += String("_") + vlnm.at(0,4);
  }
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
  float tmp = fabsf(unit->act - unit->targ);
  if(tmp >= tolerance)
    tmp *= tmp;
  else
    tmp = 0.0f;
  net_agg.ComputeAgg(&se, tmp);
}


//////////////////////////////////////////
// 	Phase-Order Environment		//
//////////////////////////////////////////

void PhaseOrderEventSpec::Initialize() {
  phase_order = MINUS_PLUS;
}

void PhaseOrderEventSpec::Destroy() {
}

void PhaseOrderEventSpec::Copy_(const PhaseOrderEventSpec& cp) {
  phase_order = cp.phase_order;
}

//////////////////////////////////////////////////
// 	Different Hebb Learning Variants	//
//////////////////////////////////////////////////


//////////////////////////////////////////
// 	History-Based Learning		//
//////////////////////////////////////////

void LeabraHistConSpec::Initialize() {
  hist_gain = 1.0;
}

void LeabraHistConSpec::InitLinks() {
  LeabraConSpec::InitLinks();
  children.SetBaseType(&TA_LeabraConSpec); // make this the base type so bias specs
					   // can live under here..
  children.el_typ = &TA_LeabraHistConSpec; // but this is the default type
}

void LeabraHistUnitSpec::Initialize() {
  min_obj_type = &TA_LeabraHistUnit;
}

void LeabraHistUnitSpec::InitState(BioBelUnit* u, BioBelLayer* lay) {
  LeabraUnitSpec::InitState(u, lay);
  LeabraHistUnit* lu = (LeabraHistUnit*)u;
  lu->act_m_h = 0.0f;
  lu->act_p_h = 0.0f;
  lu->hist_cnt = 0;
}

void LeabraHistUnitSpec::Compute_dWt(Unit* u) {
  LeabraHistUnit* lu = (LeabraHistUnit*)u;
  if((lu->act_p <= opt_thresh.learn) && (lu->act_m <= opt_thresh.learn)) {
    if(lu->hist_cnt >= 1) {
      if((lu->act_p_h <= opt_thresh.learn) && (lu->act_m_h <= opt_thresh.learn))
	return;
    }
    else
      return;
  }
  Compute_dWt_impl(lu);
}

void LeabraHistUnitSpec::Compute_dWt_post(LeabraUnit* u, LeabraLayer* lay) {
  LeabraUnitSpec::Compute_dWt_post(u, lay);
  // get history information as function of last time weight were changed..
  LeabraHistUnit* lu = (LeabraHistUnit*)u;
  lu->act_p_h = lu->act_p;
  lu->act_m_h = lu->act_m;
  lu->hist_cnt++;
}

void LeabraHistUnit::Initialize() {
  act_m_h = 0.0f;
  act_p_h = 0.0f;
  hist_cnt = 0;
}

//////////////////////////////////////////
// 	Misc Special Objects		//
//////////////////////////////////////////
