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

// processes.cc

#include <pdp/process.h>
#include <ta/ta_group_iv.h>
#include <css/ta_css.h>
#include <css/css_iv.h>
#include <pdp/pdp_iv.h>
#include <pdp/sched_proc.h>

#include <ta/enter_iv.h>
#include <OS/enter-scope.h>
#include <IV-look/kit.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/display.h>
#include <ta/leave_iv.h>

String DataItem::opt_after;

void DataItem::Initialize() {
  is_string = false;
  vec_n = 0;
  disp_opts = " ";		// always pad with initial blank
}

void DataItem::Copy_(const DataItem& cp) {
  name = cp.name;
  disp_opts = cp.disp_opts;
  is_string = cp.is_string;
  vec_n = cp.vec_n;
}

void DataItem::SetStringName(const char* nm) {
  name = "$";
  name += nm;
  is_string = true;
}

void DataItem::SetNarrowName(const char* nm) {
  name = "|";
  name += nm;
  AddDispOption("NARROW");
}

void DataItem::SetFloatVecNm(const char* nm, int n) {
  if(n > 1) {
    name = String("<") + (String)n + ">" + nm;
    vec_n = n;
  }
  else {
    name = nm;
    vec_n = 0;
  }
}

void DataItem::SetStringVecNm(const char* nm, int n) {
  is_string = true;
  if(n > 1) {
    name = String("$<") + (String)n + ">" + nm;
    vec_n = n;
  }
  else {
    name = String("$") + nm;
    vec_n = 0;
  }
}

void DataItem::AddDispOption(const char* opt) {
  String nm = " ";		// pad with preceding blank to provide start cue
  nm += String(opt) + ",";
  if(HasDispOption(nm))
    return;
  disp_opts += nm;
}
  
void LogData::Initialize() {
  items.SetBaseType(&TA_DataItem);
}

void LogData::InitLinks() {
  taBase::InitLinks();
  taBase::Own(items, this);
  taBase::Own(index, this);
  taBase::Own(r_data, this);
  taBase::Own(s_data, this);
}

void LogData::Copy_(const LogData& cp) {
  items.BorrowUnique(cp.items); // link group, so borrow
  index = cp.index;
  r_data = cp.r_data;
  s_data = cp.s_data;
}

void LogData::Reset() {
  items.Reset();
  index.Reset();
  r_data.Reset();
  s_data.Reset();
}

void LogData::AddFloat(DataItem* head, float val) {
  float tmp = val;		// some kind of weird cfront warning about using val
  items.Link(head);
  r_data.Add(tmp);
  index.Add(r_data.size-1);
}

void LogData::AddString(DataItem* head, char* val) {
  items.Link(head);
  s_data.Add(val);
  index.Add(s_data.size-1);
}

void LogData::InitBlankData() {
  r_data.Reset();
  s_data.Reset();
  index.Reset();
  int i;
  for(i=0; i<items.size; i++) {
    if(IsString(i)) {
      s_data.Add("");
      index.Add(s_data.size-1);
    }
    else {
      r_data.Add(0);
      index.Add(r_data.size-1);
    }
  }
}

bool LogData::CompareItems(const LogData& cmp) {
  // quick and dirty is so much faster, user can do GetHeaders when necessary..
  if(cmp.items.size == items.size)
    return true;
  return false;
}



//////////////////////////
// 	CtrlPanelData	//
//////////////////////////
 
void CtrlPanelData::Initialize() {
  ctrl_panel = NULL;
  active = false;
  lft = 0.0f;
  bot = 0.0f;
}

void CtrlPanelData::Copy_(const CtrlPanelData& cp) {
  lft = cp.lft;
  bot = cp.bot;
}

void CtrlPanelData::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  if(taMisc::iv_active && active && (ctrl_panel == NULL) && (owner != NULL)) {
    ((Process*)owner)->ControlPanel();
  }    
  SetWinPos();
}

void CtrlPanelData::GetPanel() {
  if(!taMisc::iv_active || (owner == NULL))
    return;
  ctrl_panel = (ProcessDialog*)taivMisc::FindEdit((void*)owner, owner->GetTypeDef());
  if((ctrl_panel != NULL) && (ctrl_panel->CtrlPanel()))
    active = true;
  else {
    active = false;
    ctrl_panel = NULL;
  }
}

void CtrlPanelData::GetWinPos() {
  if(!taMisc::iv_active) return;
  GetPanel();
  if(ctrl_panel == NULL)	return;
  ivTopLevelWindow* win = ((NoBlockDialog*)ctrl_panel->dialog)->win;
  active = true;
  lft = win->left();
  bot = win->bottom();
}

void CtrlPanelData::ScriptWinPos(ostream& strm) {
  if(!taMisc::iv_active || (owner == NULL)) return;
  GetPanel();
  if(ctrl_panel == NULL)	return;
  String temp = owner->GetPath() + ".ctrl_panel.Place(" 
    + String(lft) + ", " +  String(bot) + ");\n";
  if(taivMisc::record_script != NULL)  taivMisc::RecordScript(temp);
  else   strm << temp;
}


void CtrlPanelData::SetWinPos() {
  if(!taMisc::iv_active || !active || (ctrl_panel == NULL) || ((lft == 0) && (bot == 0)))
    return;
  GetPanel();
  if(ctrl_panel == NULL)	return;
  float newleft, newbottom;
  ivDisplay* dsp = ivSession::instance()->default_display();
  ivTopLevelWindow* win = ((NoBlockDialog*)ctrl_panel->dialog)->win;
  newleft =    MIN(lft,dsp->width() - win->width());
  newbottom =  MIN(bot,dsp->height() - win->height() - 20.0f); // allow for window decor
  lft = newleft;
  bot = newbottom;
  win->move(lft + taMisc::window_decor_offset_x, bot + taMisc::window_decor_offset_y);
}

void CtrlPanelData::Place(float left, float bottom) {
  lft = left;
  bot = bottom;
  SetWinPos();
}

void CtrlPanelData::Revert() {
  if(!taMisc::iv_active || !active || (ctrl_panel == NULL))
    return;
  ctrl_panel->GetImage();
}

//////////////////////////
// 	Process		//
//////////////////////////

void Process::Initialize() {
  type=C_CODE;
  project = NULL;
  network = NULL;
  environment = NULL;
  min_network = &TA_Network;
  min_layer = &TA_Layer;
  min_unit = &TA_Unit;
  min_con_group = &TA_Con_Group;
  min_con = &TA_Connection;
}

void Process::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(script_file, this);
  taBase::Own(mod, this);
  taBase::Own(rndm_seed, this);
  taBase::Own(log_data, this);
  taBase::Own(ctrl_panel, this);
  SetDefaultPNEPtrs();
  if(script_file.fname.empty()) { // initialize only on startup up, not transfer
    SetScript("");		// open the file thing
    type = C_CODE;		// reset to C_Code after setting script..
  }
}

void Process::Destroy() {
  CutLinks();
}

// Process::CutLinks in pdpshell.cc

void Process::Copy_(const Process& cp) {
  rndm_seed = cp.rndm_seed;
  type = cp.type;
  mod = cp.mod;
  taBase::SetPointer((TAPtr*)&network, cp.network);
  taBase::SetPointer((TAPtr*)&environment, cp.environment);
  script_file = cp.script_file;
  script_string = cp.script_string;
  ctrl_panel = cp.ctrl_panel;
  min_network = cp.min_network;
  min_layer = cp.min_layer;
  min_unit = cp.min_unit;
  min_con_group = cp.min_con_group;
  min_con = cp.min_con;
}

void Process::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  if((!script_file.fname.empty() && script_file.file_selected) || !script_string.empty())
    type = SCRIPT;
  if(type == SCRIPT) {
    if((script == NULL) || (script->run == cssEl::Waiting)
       || (script->run == cssEl::Stopping))
      UpdateReCompile();
  }
}

void Process::CopyPNEPtrs(Network* net, Environment* env) {
  taBase::SetPointer((TAPtr*)&network, net);
  taBase::SetPointer((TAPtr*)&environment, env);
}

void Process::SetEnv(Environment* env) {
  taBase::SetPointer((TAPtr*)&environment, env);
  UpdateAfterEdit();
}

void Process::SetNet(Network* net) {
  taBase::SetPointer((TAPtr*)&network, net);
  UpdateAfterEdit();
}

void Process::NewInit() {
  NewSeed();
  Init();
}

void Process::ReInit() {
  OldSeed();
  Init();
}

void Process::NewSeed() {
  rndm_seed.NewSeed();
#ifdef DMEM_COMPILE
  DMem_SyncSameNetSeeds();
#endif
}

void Process::OldSeed() {
  rndm_seed.OldSeed();
#ifdef DMEM_COMPILE
  DMem_SyncSameNetSeeds();
#endif
}

#ifdef DMEM_COMPILE
void Process::DMem_SyncSameNetSeeds() {
  if((taMisc::dmem_nprocs <= 1) || (network == NULL) || (network->dmem_nprocs_actual <= 1))
    return;

  // just blast the first guy to all members of the same communicator
  DMEM_MPICALL(MPI_Bcast(rndm_seed.seed.el, MTRnd::N, MPI_LONG, 0, network->dmem_share_units.comm),
	       "Process::SyncSameNetSeeds", "Bcast");
  rndm_seed.OldSeed();		// then get my seed!
}

void Process::DMem_SyncAllSeeds() {
  if(taMisc::dmem_nprocs <= 1)
    return;

  // just blast the first guy to all members of the same communicator
  DMEM_MPICALL(MPI_Bcast(rndm_seed.seed.el, MTRnd::N, MPI_LONG, 0, MPI_COMM_WORLD),
	       "Process::SyncAllSeeds", "Bcast");
  rndm_seed.OldSeed();		// then get my seed!
}
#endif

void Process::Init() {
  rndm_seed.GetCurrent();
}
  
void Process::Run() {
  TimeUsed start;  start.rec = time_used.rec;
  start.GetTimes();
  if(!RunScript())
    C_Code();			// else run the C_Code
  time_used.GetUsed(start);
}

void Process::Run_gui() {
  Run();
}

LogData& Process::GenLogData(LogData* ld) {
  if(ld == NULL)
    return log_data;
  return *ld;
}

void Process::LoadScript(const char* nm) {
  ScriptBase::LoadScript(nm);
  type = SCRIPT;
}

bool Process::RunScript() {
  if(type == C_CODE) return false;
  return ScriptBase::RunScript();
}

void Process::ControlPanel(float left, float bottom) {
  if(!taMisc::iv_active) return;
  taivProcess* ivp = (taivProcess*)GetTypeDef()->ive;
  if(ivp->run_iv == NULL) {
    ivp->run_iv = new taivProcessRunBox(GetTypeDef());
  }
  const ivColor* bgclr = GetEditColorInherit();
  ivp->run_iv->Edit((void*)this, NULL, false, false, bgclr);
  ctrl_panel.GetPanel();
  if((left != 0) && (bottom != 0)) {
    ctrl_panel.Place(left, bottom);
  }
}

void Process::CheckError(TAPtr ck, TypeDef* td) {
  if(ck == NULL) {
    taMisc::Error("NULL object found in check of type:", td->name);
  }
  else {
    taMisc::Error("In process:", name, ", incorrect type of obj:", ck->GetPath(),
		   "of type:", ck->GetTypeDef()->name,
		   "should be at least:", td->name);
  }
}

// cache the types already checked to avoid redundant checking!
static TypeDef* proc_checked_network;
static TypeDef* proc_checked_layer;
static TypeDef* proc_checked_unit;
static TypeDef* proc_checked_con_group;
static TypeDef* proc_checked_con;

void Process::CheckResetCache() {
  proc_checked_network = NULL;
  proc_checked_layer = NULL;
  proc_checked_unit = NULL;
  proc_checked_con_group = NULL;
  proc_checked_con = NULL;
}

bool Process::CheckNetwork() {
  int n_checked = 0;
  if((proc_checked_network != NULL) &&
     proc_checked_network->InheritsFrom(min_network)) n_checked++;
  else proc_checked_network = min_network;
  if((proc_checked_layer != NULL) &&
     proc_checked_layer->InheritsFrom(min_layer)) n_checked++;
  else proc_checked_layer = min_layer;
  if((proc_checked_unit != NULL) &&
     proc_checked_unit->InheritsFrom(min_unit)) n_checked++;
  else proc_checked_unit = min_unit;
  if((proc_checked_con_group != NULL) &&
     proc_checked_con_group->InheritsFrom(min_con_group)) n_checked++;
  else proc_checked_con_group = min_con_group;
  if((proc_checked_con != NULL) &&
     proc_checked_con->InheritsFrom(min_con)) n_checked++;
  else proc_checked_con = min_con;
  if(n_checked == 5)
    return true;		// allready been checked!

  if((network==NULL) || !network->InheritsFrom(min_network)) {
    CheckError(network, min_network);
    return false;
  }
  if(!network->CheckTypes()) return false;
  if(network->CheckBuild()) {
    taMisc::Error("In process:", name, ", Network:",network->name,
		  "Needs the 'Build' command to be run");
    return false;
  }
  if(network->CheckConnect()) {
    taMisc::Error("In process:", name, ", Network:",network->name,
		  "Needs the 'Connect' command to be run");
    return false;
  }

  Layer *layer;
  taLeafItr i;
  FOR_ITR_EL(Layer, layer, network->layers., i) {
    if(!CheckLayer(layer))
      return false;
  }

  // make sure network is fully updated..
  network->UpdtAfterNetMod();
  return true;
}

bool Process::CheckLayer(Layer* ck) {
  if(ck->lesion)	return true; // don't care about lesioned layers
  if((ck == NULL) || !ck->InheritsFrom(min_layer)) {
    CheckError(ck, min_layer);
    return false;
  }
  Unit* unit;
  taLeafItr i;
  FOR_ITR_EL(Unit, unit, ck->units., i) {
    if(!CheckUnit(unit))
      return false;
  }
  return true;
}

bool Process::CheckUnit(Unit* ck) {
  if((ck == NULL) || !ck->InheritsFrom(min_unit)) {
    CheckError(ck, min_unit);
    return false;
  }
  Con_Group* cg;
  int i;
  FOR_ITR_GP(Con_Group, cg, ck->recv., i) {
    if(!CheckConGroup(cg))      	return false;
    if(!cg->CheckOtherIdx_Recv()) 	return false;
  }
  FOR_ITR_GP(Con_Group, cg, ck->send., i) {
    if(!CheckConGroup(cg))      	return false;
    if(!cg->CheckOtherIdx_Send()) 	return false;
  }
  return true;
}
bool Process::CheckConGroup(Con_Group* ck) {
  if((ck==NULL) || !ck->InheritsFrom(min_con_group)) {
    CheckError(ck, min_con_group);
    return false;
  }
  if(!ck->el_typ->InheritsFrom(min_con)) {
    CheckError(ck, min_con);
    return false;
  }
  if(ck->spec.spec == NULL) {
    CheckError(ck->spec.spec, NULL);
  }
  return true;
}

SchedProcess* Process::GetMySchedProc() {
  if(InheritsFrom(TA_SchedProcess))
    return (SchedProcess*)this;
  return GET_MY_OWNER(SchedProcess);
}

SchedProcess* Process::GetMySProcOfType(TypeDef* proc_type) {
  SchedProcess* sp = GetMySchedProc();
  if(sp == NULL) return sp;
  return sp->FindProcOfType(proc_type);
}

TrialProcess* Process::GetMyTrialProc() {
  return (TrialProcess*)GetMySProcOfType(&TA_TrialProcess);
}

EpochProcess* Process::GetMyEpochProc() {
  return (EpochProcess*)GetMySProcOfType(&TA_EpochProcess);
}

Event* Process::GetMyCurEvent() {
  TrialProcess* tp = GetMyTrialProc();
  if(tp == NULL) return NULL;
  return tp->cur_event;
}

Event* Process::GetMyNextEvent() {
  EpochProcess* ep = GetMyEpochProc();
  if(ep == NULL) return NULL;
  return ep->GetMyNextEvent();
}

Event_MGroup* Process::GetMyCurEventGp() {
  SequenceProcess* sp = (SequenceProcess*)GetMySProcOfType(&TA_SequenceProcess);
  if(sp == NULL) {
    EpochProcess* ep = GetMyEpochProc();
    if(ep == NULL) return NULL;
    return ep->enviro_group;
  }
  return sp->cur_event_gp;
}

//////////////////////////
//    Process_MGroup	//
//////////////////////////

bool Process_Group::Close_Child(TAPtr obj) {
  SchedProcess* sp = GET_MY_OWNER(SchedProcess);
  if(sp != NULL) {
    winbMisc::DelayedMenuUpdate(sp);
  }
  return Remove(obj);		// otherwise just nuke it
}

void Process_MGroup::GetAllWinPos() {
  if(!taMisc::iv_active) return;
  taLeafItr li;
  Process* ta;
  FOR_ITR_EL(Process, ta, this->, li) {
    ta->ctrl_panel.GetWinPos();
  }
}

void Process_MGroup::ScriptAllWinPos() {
  if(!taMisc::iv_active) return;
  taLeafItr li;
  Process* ta;
  FOR_ITR_EL(Process, ta, this->, li) {
    ta->ctrl_panel.ScriptWinPos();
  }
}

bool Process_MGroup::Close_Child(TAPtr obj) {
  if(!obj->InheritsFrom(&TA_SchedProcess)) {
    return PDPMGroup::Close_Child(obj);
  }
  SchedProcess* sp = (SchedProcess*)obj;
  bool rval = true;
  // do a clean removal of object from hierarchy instead of nixing entire set
  if(sp->super_proc != NULL) {
    sp->super_proc->RemoveSubProc(); 
  }
  else if(sp->sub_proc != NULL) {
    sp->sub_proc->RemoveSuperProc();
  }
  else
    rval = Remove(obj);		// otherwise just nuke it
  winbMisc::DelayedMenuUpdate(this);
  return rval;
}

bool Process_MGroup::DuplicateEl(TAPtr obj) {
  if(!obj->InheritsFrom(&TA_SchedProcess)) {
    return PDPMGroup::DuplicateEl(obj);
  }
  SchedProcess* sp = (SchedProcess*)obj;
  bool rval = PDPMGroup::DuplicateEl(obj); // first get the new guy
  if(!rval) return rval;
  SchedProcess* np = (SchedProcess*)Peek();
  np->DuplicateElHook(sp);
  winbMisc::DelayedMenuUpdate(this);
  return rval;
}

int Process_MGroup::ReplaceEnvPtrs(Environment* old_ev, Environment* new_ev) {
  int nchg = 0;
  taLeafItr li;
  Process* ta;
  FOR_ITR_EL(Process, ta, this->, li) {
    if(ta->environment == old_ev) {
      taBase::SetPointer((TAPtr*)&ta->environment, new_ev);
      if(new_ev == NULL)
	taMisc::Error("*** Note: set environment pointer to NULL in process:",ta->GetPath());
      else
	taMisc::Error("*** Note: replaced environment pointer to:", old_ev->name,
		      "in process:", ta->GetPath(), "with pointer to:", new_ev->name);
      nchg++;
    }
    ta->UpdateAfterEdit();	// other things might have changed
  }
  return nchg;
}

int Process_MGroup::ReplaceNetPtrs(Network* old_net, Network* new_net) {
  int nchg = 0;
  taLeafItr li;
  Process* ta;
  FOR_ITR_EL(Process, ta, this->, li) {
    if(ta->network == old_net) {
      taBase::SetPointer((TAPtr*)&(ta->network), new_net);
      if(new_net == NULL)
	taMisc::Error("*** Note: set network pointer to NULL in process:",ta->GetPath());
      else
	taMisc::Error("*** Note: replaced network pointer to:", old_net->name,
		      "in process:", ta->GetPath(), "with pointer to:", new_net->name);
      nchg++;
    }
    ta->UpdateAfterEdit();	// other things might have changed
  }
  return nchg;
}

void Process_MGroup::ControlPanel_mc(taivMenuEl* sel) {
  if(win_owner == NULL) return;
  if((sel != NULL) && (sel->usr_data != NULL)) {
    Process* itm = (Process*)sel->usr_data;
    itm->ControlPanel();
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->ControlPanel();" << endl;
    }
  }
}

// setup callbacks
declare_taivMenuCallback(Process_MGroup)
implement_taivMenuCallback(Process_MGroup)

void Process_MGroup::GetMenu_impl() {
  PDPMGroup::GetMenu_impl();
  
  // don't put in the groups or the stats processes within menu
  itm_list->no_in_gpmenu = true;
  itm_list->no_lst = true;

  taivTypeHier* s_typ_list = new taivTypeHier(ta_menu, &TA_Stat);
  taivTypeHier* p_typ_list = new taivTypeHier(ta_menu, &TA_Process);
  int cur_no = ta_menu->cur_sno;

  ta_menu->AddSep();

  // todo: warning, this menu won't work for over-max-menu!
  ta_menu->AddSubMenu("Control Panel");
  itm_list->GetMenu(new taivMenuCallback(Process_MGroup)
		    (this, &Process_MGroup::ControlPanel_mc));
  ta_menu->SetSub(cur_no);

  delete s_typ_list;
  delete p_typ_list;
}

bool Process_Group::nw_itm_def_arg = false;

Process* Process_Group::FindMakeProc(const char* nm, TypeDef* td, bool& nw_itm) {
  Process* gp = NULL;
  if(nm != NULL)
    gp = (Process*)FindLeafName(nm);
  else if(td != NULL)
    gp = (Process*)FindLeafType(td);
  nw_itm = false;
  if(gp == NULL) {
    gp = (Process*)NewEl(1, td);
    if(nm != NULL)
      gp->name = nm;
    nw_itm = true;
  }
  return gp;
}

bool Process_MGroup::nw_itm_def_arg = false;

Process* Process_MGroup::FindMakeProc(const char* nm, TypeDef* td, bool& nw_itm) {
  Process* gp = NULL;
  if(nm != NULL)
    gp = (Process*)FindLeafName(nm);
  else if(td != NULL)
    gp = (Process*)FindLeafType(td);
  nw_itm = false;
  if(gp == NULL) {
    gp = (Process*)NewEl(1, td);
    if(nm != NULL)
      gp->name = nm;
    nw_itm = true;
  }
  return gp;
}

