// -*- C++ -*-
// lstm.h 

#ifdef __GNUG__
#pragma implementation
#endif
#include "lstm.h"

void LstmBpUnitSpec::Initialize() {
  min_obj_type = &TA_LstmBpUnit;
}

bool LstmBpUnitSpec::CheckConfig(Unit* u, Layer* lay, TrialProcess* trl) {
  bool rval = BpUnitSpec::CheckConfig(u, lay, trl);
  if(!rval) return rval;

  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(!recv_gp->spec.spec->InheritsFrom(TA_LstmBpConSpec)) {
      taMisc::Error("LstmBpUnitSpec: all recv connections must have at least LstmBpConSpec connection specs, not:",
		    recv_gp->spec.spec->name,"in unit:", u->GetPath());
      return false;
    }
  }
  return true;
}

void LstmBpUnitSpec::InitState(Unit* u) {
  BpUnitSpec::InitState(u);
  LstmBpUnit* mu = (LstmBpUnit*)u;
  mu->act_prv = 0.0f;
}

void LstmBpUnitSpec::Compute_Act(Unit* u) {
  LstmBpUnit* mu = (LstmBpUnit*)u;
  if(mu->ext_flag & Unit::EXT) {
    mu->act = mu->act_prv = mu->ext; // both current and previous set to ext
    return;
  }
  float prv_act = u->act;
  BpUnitSpec::Compute_Act(u);
  mu->act_prv = prv_act;
}

void LstmBpUnit::Initialize() {
  act_prv = 0.0f;
}

void LstmBpUnit::Copy_(const LstmBpUnit& cp) {
  act_prv = cp.act_prv;
}

//////////////////////////////////
//  	ToMemConSpecs		//
//////////////////////////////////

void InGateToMemConSpec::Initialize() {
  SetUnique("rnd", true);
  rnd.mean = 0.0f;
  rnd.var = 0.0f;
  SetUnique("lrate", true);
  lrate = 0.0f;
}

void FrgGateToMemConSpec::Initialize() {
  SetUnique("rnd", true);
  rnd.mean = 0.0f;
  rnd.var = 0.0f;
  SetUnique("lrate", true);
  lrate = 0.0f;
}

void OutGateToMemConSpec::Initialize() {
  SetUnique("rnd", true);
  rnd.mean = 0.0f;
  rnd.var = 0.0f;
  SetUnique("lrate", true);
  lrate = 0.0f;
}

//////////////////////////////////
//  	DmemUnitSpec		//
//////////////////////////////////

void DmemBpCon::Copy_(const DmemBpCon& cp) {
  dMdw_sum = cp.dMdw_sum;
  int i;
  for(i=0;i<lstm_max_mem;i++)
    dMdw[i] = cp.dMdw[i];
}

void DmemBpCon::Initialize() {
  dMdw_sum = 0.0f;
  int i;
  for(i=0;i<lstm_max_mem;i++)
    dMdw[i] = 0.0f;
}

void DmemBpConSpec::Initialize() {
  min_con_type = &TA_DmemBpCon;
}

void DmemBpUnitSpec::Initialize() {
  bias_con_type = &TA_DmemBpCon;
  bias_spec.SetBaseType(&TA_DmemBpConSpec);
}

bool DmemBpUnitSpec::CheckConfig(Unit* u, Layer* lay, TrialProcess* trl) {
  bool rval = LstmBpUnitSpec::CheckConfig(u, lay, trl);
  if(!rval) return rval;

  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(!recv_gp->spec.spec->InheritsFrom(TA_DmemBpConSpec)) {
      taMisc::Error("DmemBpUnitSpec: all recv connections must have at least DmemBpConSpec connection specs, not:",
		    recv_gp->spec.spec->name,"in unit:", u->GetPath());
      return false;
    }
    if(!recv_gp->prjn->con_type->InheritsFrom(TA_DmemBpCon)) {
      taMisc::Error("DmemBpUnitSpec: all recv connections must have at least DmemBpCon connections, not:",
		    recv_gp->prjn->con_type->name,"in unit:", u->GetPath());
      return false;
    }
  }
  if(!bias_spec.spec->InheritsFrom(TA_DmemBpConSpec)) {
    taMisc::Error("DmemBpUnitSpec: bias spec must be at least a DmemBpConSpec, not:",
		  bias_spec.spec->name,"in unit spec:", GetPath());
    return false;
  }
  if(!u->bias->InheritsFrom(TA_DmemBpCon)) {
    taMisc::Error("DmemBpUnitSpec: bias connections must be at least a DmemBpCon, not:",
		  u->bias->GetTypeDef()->name,"in unit:", u->GetPath());
    return false;
  }

  if(u->send.gp.size != 1) {
    taMisc::Error("DmemBpUnitSpec: Can only have exactly one sending connection group",
		  "in unit:", u->GetPath());
    return false;
  }
  Con_Group* send_gp = (Con_Group*)u->send.gp[0];
  if(send_gp->size == 0) {
    taMisc::Error("DmemBpUnitSpec: Must have connections in sending connection group",
		  "in unit:", u->GetPath());
    return false;
  }
  if(send_gp->size > lstm_max_mem) {
    taMisc::Error("DmemBpUnitSpec: Must at most:",(String)lstm_max_mem,
		  "(lstm_max_mem) connections to memory cells -- hard compiled constraint",
		  "in unit:", u->GetPath());
    return false;
  }
  int i;
  for(i=0;i<send_gp->size;i++) {
    Unit* su = send_gp->Un(i);
    if(!su->InheritsFrom(TA_MemBpUnit)) {
      taMisc::Error("DmemBpUnitSpec: units I send to must be at least MemBpUnits, not:",
		    su->GetTypeDef()->name,"in unit:", u->GetPath());
      return false;
    }
  }

  return true;
}

void DmemBpUnitSpec::InitState(Unit* u) {
  LstmBpUnitSpec::InitState(u);
  // init the mems when initing state
  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(recv_gp->prjn->from->lesion)
      continue;
    ((DmemBpConSpec*)recv_gp->spec.spec)->InitMem(recv_gp);
  }
  ((DmemBpCon*)u->bias)->InitMem();
}

void DmemBpUnitSpec::Compute_Dmem(LstmBpUnit* u) {
  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(recv_gp->prjn->from->lesion)
      continue;

    // iterate over *sending* connections (should only be to memory units!)
    Con_Group* send_gp;
    int sg;
    FOR_ITR_GP(Con_Group, send_gp, u->send., sg) {
      if(send_gp->prjn->layer->lesion)
	continue;
      int i;
      for(i=0;i<send_gp->size;i++) {
	MemBpUnit* mem = (MemBpUnit*)send_gp->Un(i);
	((DmemBpConSpec*)recv_gp->spec.spec)->Compute_Dmem(recv_gp, u, mem, i);
      }
    }
  }

  Con_Group* send_gp;
  int sg;
  FOR_ITR_GP(Con_Group, send_gp, u->send., sg) {
    if(send_gp->prjn->layer->lesion)
      continue;
    int i;
    for(i=0;i<send_gp->size;i++) {
      if(i >= lstm_max_mem) continue;
      MemBpUnit* mem = (MemBpUnit*)send_gp->Un(i);
      ((DmemBpConSpec*)bias_spec.spec)->B_Compute_Dmem((DmemBpCon*)u->bias, u, mem, i);
    }
  }
}

void DmemBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  u->dEdNet = 0.0f;		// not applicable!
  LstmBpUnit* lu = (LstmBpUnit*)u;

  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(recv_gp->prjn->from->lesion)
      continue;

    DmemBpConSpec* cs = (DmemBpConSpec*)recv_gp->spec.spec;
    cs->InitMemSum(recv_gp);

    // iterate over *sending* connections (should only be to memory units!)
    Con_Group* send_gp;
    int sg;
    FOR_ITR_GP(Con_Group, send_gp, u->send., sg) {
      if(send_gp->prjn->layer->lesion)
	continue;
      int i;
      for(i=0;i<send_gp->size;i++) {
	MemBpUnit* mem = (MemBpUnit*)send_gp->Un(i);
	cs->Compute_DmemSum(recv_gp, lu, mem, i);
      }
    }
  }
  DmemBpConSpec* bcs = (DmemBpConSpec*)bias_spec.spec;
  ((DmemBpCon*)u->bias)->dMdw_sum = 0.0f;
  Con_Group* send_gp;
  int sg;
  FOR_ITR_GP(Con_Group, send_gp, u->send., sg) {
    if(send_gp->prjn->layer->lesion)
      continue;
    int i;
    for(i=0;i<send_gp->size;i++) {
      if(i >= lstm_max_mem) continue;
      MemBpUnit* mem = (MemBpUnit*)send_gp->Un(i);
      bcs->C_Compute_DmemSum((DmemBpCon*)u->bias, lu, mem, i);
    }
  }
}

//////////////////////////
//	MemBpUnitSpec	//
//////////////////////////

void MemBpUnitSpec::Initialize() {
  min_obj_type = &TA_MemBpUnit;
  bias_spec.SetBaseType(&TA_MemGateInConSpec);
  SetUnique("act_range", true);
  act_range.min = -1;
  act_range.max = 1;
  inx_range.min = -2;
  inx_range.max = 2;
  skip_act_oux_deriv = false;
}

void MemBpUnitSpec::InitLinks() {
  LstmBpUnitSpec::InitLinks();
  taBase::Own(inx_range, this);
}

bool MemBpUnitSpec::CheckConfig(Unit* u, Layer* lay, TrialProcess* trl) {
  bool rval = LstmBpUnitSpec::CheckConfig(u, lay, trl);	// skip Dmem..
  if(!rval) return rval;

  bool got_in = false;
  bool got_frg = false;
  bool got_out = false;
  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    LstmBpConSpec* cs = (LstmBpConSpec*)recv_gp->spec.spec;
    if(cs->GetGateType() == LstmBpConSpec::IN_GATE)
      got_in = true;
    else if(cs->GetGateType() == LstmBpConSpec::FRG_GATE)
      got_frg = true;
    else if(cs->GetGateType() == LstmBpConSpec::OUT_GATE)
      got_out = true;
    else {
      if(!recv_gp->spec.spec->InheritsFrom(TA_DmemBpConSpec)) {
	taMisc::Error("MemBpUnitSpec: all recv connections must have at least DmemBpConSpec connection specs, not:",
		      recv_gp->spec.spec->name,"in unit:", u->GetPath());
	return false;
      }
      if(!recv_gp->prjn->con_type->InheritsFrom(TA_DmemBpCon)) {
	taMisc::Error("MemBpUnitSpec: all recv connections must have at least DmemBpCon connections, not:",
		      recv_gp->prjn->con_type->name,"in unit:", u->GetPath());
	return false;
      }
    }
  }
  if(!got_in) {
    taMisc::Error("MemBpUnitSpec: Did not find an InGateToMemConSpec sending me input gating info!",
		  "in unit:", u->GetPath());
    return false;
  }
  if(!got_frg) {
    taMisc::Error("MemBpUnitSpec: Did not find a FrgGateToMemConSpec sending me forget gating info!",
		  "in unit:", u->GetPath());
    return false;
  }
  if(!got_out) {
    taMisc::Error("MemBpUnitSpec: Did not find an OutGateToMemConSpec sending me output gating info!",
		  "in unit:", u->GetPath());
    return false;
  }
  return true;
}

void MemBpUnitSpec::InitState(Unit* u) {
  LstmBpUnitSpec::InitState(u);
  MemBpUnit* mu = (MemBpUnit*)u;
  mu->in_gt = mu->frg_gt = mu->out_gt = 0.0f;
  mu->act_inx = mu->act_mem = mu->mem_prv = mu->act_oux = mu->act_prv = 0.0f;
}

void MemBpUnitSpec::Compute_Net(Unit* u) {
  MemBpUnit* mu = (MemBpUnit*)u;
  mu->net = 0.0f;
  int g;
  Con_Group* recv_gp;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from->lesion)
      continue;
    LstmBpConSpec* cs = (LstmBpConSpec*)recv_gp->spec.spec;
    if(cs->GetGateType() == LstmBpConSpec::NOT_GATE)
      u->net += cs->Compute_Net(recv_gp, u);
  }
  // no bias weights for memory units!!
//    if(u->bias != NULL)
//      u->net += u->bias->wt;
}

void MemBpUnitSpec::Compute_Act(Unit* u) {
  MemBpUnit* mu = (MemBpUnit*)u;

  // first grab gate activations (based on current time, not previous time!)
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(recv_gp->prjn->from->lesion)
      continue;
    LstmBpConSpec* cs = (LstmBpConSpec*)recv_gp->spec.spec;
    if(cs->GetGateType() == LstmBpConSpec::IN_GATE)
      mu->in_gt = recv_gp->Un(0)->act;
    else if(cs->GetGateType() == LstmBpConSpec::FRG_GATE)
      mu->frg_gt = recv_gp->Un(0)->act;
    else if(cs->GetGateType() == LstmBpConSpec::OUT_GATE)
      mu->out_gt = recv_gp->Un(0)->act;
  }

  mu->act_prv = mu->act;
  mu->mem_prv = mu->act_mem;	// previous values

  mu->act_inx = inx_range.Project(sig.Eval(u->net)); // input xform (g)
  mu->act_mem = mu->frg_gt * mu->act_mem + mu->in_gt * mu->act_inx; // mem act (s)
  mu->act_oux = act_range.Project(sig.Eval(mu->act_mem)); // output xform (h)
  mu->act = mu->out_gt * mu->act_oux; // output gated value (finally)
}

void MemBpUnitSpec::Compute_Dmem(LstmBpUnit* u) {
  Con_Group* recv_gp;
  int rg;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., rg) {
    if(recv_gp->prjn->from->lesion)
      continue;
    LstmBpConSpec* cs = (LstmBpConSpec*)recv_gp->spec.spec;
    if(cs->GetGateType() == LstmBpConSpec::NOT_GATE)
      ((DmemBpConSpec*)cs)->Compute_Dmem(recv_gp, u, (MemBpUnit*)u, 0);
  }
  // no bias for mem units!
  // ((DmemBpConSpec*)bias_spec.spec)->B_Compute_Dmem((DmemBpCon*)u->bias, u, (MemBpUnit*)u, 0);
}

void MemBpUnitSpec::Compute_dEdNet(BpUnit* u) {
  MemBpUnit* mu = (MemBpUnit*)u;
  // basic delta, e_s_c (eq 12): but the peephole paper doesn't have sig_deriv act_oux!
  mu->dEdNet = mu->dEdA * mu->out_gt;
  if(!skip_act_oux_deriv)
    mu->dEdNet *= sig_deriv(act_range, mu->act_oux);
}

void MemBpUnit::Initialize() {
  spec.SetBaseType(&TA_MemBpUnitSpec);
  in_gt = frg_gt = out_gt = 0.0f;
  act_inx = act_mem = mem_prv = act_oux = 0.0f;
}

void MemBpUnit::Copy_(const MemBpUnit& cp) {
  in_gt = cp.in_gt;
  frg_gt = cp.frg_gt;
  out_gt = cp.out_gt;
  act_inx = cp.act_inx;
  act_mem = cp.act_mem;
  mem_prv = cp.mem_prv;
  act_oux = cp.act_oux;
}

//////////////////////////
//  	Procs		//
//////////////////////////

void LstmBpTrial::Initialize() {
  min_unit = &TA_LstmBpUnit;
}

void LstmBpTrial::Compute_ClampExt() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    if(!(layer->ext_flag & Unit::EXT)) continue;
    LstmBpUnit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(LstmBpUnit, u, layer->units., u_itr) {
      if(u->ext_flag & Unit::EXT) {
	u->act = u->act_prv = u->ext; // both current and previous set to ext
      }
    }
  }
}

void LstmBpTrial::Compute_NetHid() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    if((layer->ext_flag & Unit::EXT) || (layer->ext_flag & Unit::TARG)) continue;
    layer->Compute_Net();
  }
}

void LstmBpTrial::Compute_NetOut() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    if((layer->ext_flag & Unit::EXT) || !(layer->ext_flag & Unit::TARG)) continue;
    layer->Compute_Net();
  }
}

void LstmBpTrial::Compute_ActHid() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    if((layer->ext_flag & Unit::EXT) || (layer->ext_flag & Unit::TARG)) continue;
    layer->Compute_Act();
  }
}

void LstmBpTrial::Compute_ActOut() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    if((layer->ext_flag & Unit::EXT) || !(layer->ext_flag & Unit::TARG)) continue;
    layer->Compute_Act();
  }
}

void LstmBpTrial::Compute_Dmem() {
  Layer* layer;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, layer, network->layers., l_itr) {
    if(layer->lesion)	continue;
    BpUnit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(BpUnit, u, layer->units., u_itr) {
      if(u->spec.spec->InheritsFrom(TA_DmemBpUnitSpec)) {
	((DmemBpUnitSpec*)u->spec.spec)->Compute_Dmem((LstmBpUnit*)u);
      }
    }
  }
}

void LstmBpTrial::Loop() {
  if(network == NULL) return;

  SetCurLrate();

  network->InitExterns();
  if(cur_event != NULL)
    cur_event->ApplyPatterns(network);
  
  Compute_ClampExt();
  Compute_NetHid();
  Compute_ActHid();
  Compute_NetOut();
  Compute_ActOut();
  Compute_Dmem();
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

