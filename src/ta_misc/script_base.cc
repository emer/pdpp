/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the TypeAccess/C-Super-Script software package.	      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, modify, and distribute this software and its      //
// documentation for any purpose is hereby granted without fee, provided that //
// the above copyright notice and this permission notice appear in all copies //
// of the software and related documentation.                                 //
// 									      //
// Note that the PDP++ software package, which contains this package, has a   //
// more restrictive copyright, which applies only to the PDP++-specific       //
// portions of the software, which are labeled as such.			      //
//									      //
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

// script_base.cc

#include <ta_misc/script_base.h>
#include <css/ta_css.h>
#include <css/css_iv.h>
#include <ta/ta_group_iv.h>
#include <iv_misc/lrScrollBox.h>
#include <iv_misc/tbScrollBox.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <InterViews/style.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <ta/leave_iv.h>


ScriptBase_List Script::recompile_scripts;

bool Script::Wait_RecompileScripts() {
  if(recompile_scripts.size == 0)
    return false;
  int i;
  for(i=0; i<recompile_scripts.size; i++)
    recompile_scripts[i]->LoadScript();
  recompile_scripts.Reset();
  return true;
}

//////////////////////////
// 	ScriptBase	//
//////////////////////////

ScriptBase::ScriptBase() {
  script=NULL;
  script_file.cmp = false;	// don't compress
  script_file.mode = taFile::NO_AUTO;
}

ScriptBase::~ScriptBase() {
  if(script != NULL) {
    if(script->DeleteOk())
      delete script;
    else
      script->DeferredDelete();
  }
  script = NULL;
}

bool ScriptBase::HasScript() {
  if(script_file.fname.empty() && script_string.empty())
    return false;
  return true;
}

void ScriptBase::SetScript(const char* nm) {
  if(script_file.gf == NULL) {
    if(!taMisc::iv_active)
      script_file.GetGetFile();
    else {
      ivWidgetKit* wkit = ivWidgetKit::instance();
      script_file.gf = new taivGetFile(".","*.css*",NULL,wkit->style(),false);
      taRefN::Ref(script_file.gf);
    }
  }
  script_file.gf->select_only = true;	// just selecting a file name here
  script_file.fname = nm;
  script_file.UpdateGF();
}

void ScriptBase::LoadScript(const char* nm) {
  if(nm != NULL)
    SetScript(nm);
  
  if(!HasScript()) {
    taMisc::Error("Cannot Load Script: No script file or string specified");
    return;
  }

  if(script == NULL) {
    script = new cssProgSpace();
    InstallThis();
  }

  if(script->in_readline) {
    Script::recompile_scripts.Add(this);
    script->ExitShell();
    return;
  }
  script->ClearAll();
  if(script_file.fname.empty())
    script->CompileCode(script_string);	// compile the string itself
  else
    script->Compile(script_file.fname);
}

void ScriptBase::LoadScriptString(const char* string) {
  if(string != NULL)
    script_string = string;
  script_file.fname = "";
  LoadScript();
}

void ScriptBase::InstallThis() {
  cssTA_Base* ths = new cssTA_Base(GetThisPtr(), 1, GetThisTypeDef(), "this");
  ths->InstallThis(script);
}

bool ScriptBase::RunScript() {
  if(script == NULL)   return false;
  script->Run();		// just run the script
  return true;
}

void ScriptBase::InteractScript() {
  if(script == NULL)   return;
  cssMisc::next_shell = script;
}

// todo: replace this with a time-stamp for recompiling
// maybe this should be done at a lower level within css itself

void ScriptBase::UpdateReCompile() {
  if(!HasScript())
    return;

  if((script == NULL) || script_file.file_selected || !script_string.empty()) {
    script_file.file_selected = false;
    LoadScript();
  }
}

//////////////////////////
// 	SArg_Array	//
//////////////////////////

void SArg_Array::Initialize() {
}

void SArg_Array::InitLinks() {
  String_Array::InitLinks();
  taBase::Own(labels, this);
}

void SArg_Array::UpdateAfterEdit() {
  labels.EnforceSize(size);
}

void SArg_Array::Copy_(const SArg_Array& cp) {
  labels = cp.labels;
}

int SArg_Array::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  int rval = String_Array::Dump_Save_Value(strm, par, indent);
  strm << "};" << endl;
  int rv2 = labels.Dump_Save_Value(strm, this, indent+1);
  return (rval && rv2);
}

int SArg_Array::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = String_Array::Dump_Load_Value(strm, par);
  int c = taMisc::skip_white(strm, true); // peek
  if(c == '{') {
    labels.Dump_Load_Value(strm, this);
  }
  return rval;
}

//////////////////////////////////
//	SArgEditDialog		//
//////////////////////////////////

SArgEditDialog::SArgEditDialog(TypeDef *tp, void* base)
: gpivArrayEditDialog(tp, base) {
  n_ary_membs = 0;
  ary_data_g = NULL;
}

bool SArgEditDialog::ShowMember(MemberDef* md) {
  if(md->name == "size")
    return true;
  else
    return taivEditDialog::ShowMember(md);
}

void SArgEditDialog::Constr_AryData() {
  SArg_Array* cur_ary = (SArg_Array*)cur_base;
  cur_ary->UpdateAfterEdit();
  MemberDef* eldm = typ->members.FindName("el");
  taivType* tiv = eldm->type->GetNonPtrType()->iv;
  int i;
  for(i=0;i<cur_ary->size;i++) {
    taivData* mb_dat = tiv->GetDataRep(dialog, this);
    data_el.Add(mb_dat);
    String nm = String("[") + String(i) + "]";
    String lbl = cur_ary->labels[i];
    if(!lbl.empty())
      nm = lbl + nm;
    ary_data_g->append
       (layout->vcenter
	(layout->vbox
	 (layout->hcenter(wkit->fancy_label(nm),0),
	  layout->hcenter(mb_dat->GetLook(),0)),0));
  }
}

///////////////

int SArgEdit::BidForEdit(TypeDef* td){
  if(td->InheritsFrom(TA_SArg_Array))
    return (gpivArrayEdit::BidForType(td) +1);
  return 0;
}

int SArgEdit::Edit(void* base, ivWindow* win, bool wait,bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new SArgEditDialog(typ, base);
    dlg->cancel_only = readonly;
    dlg->Constr(win, wait, "", "", false, bgclr);
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

//////////////////////////
// 	Script		//
//////////////////////////

void Script::Initialize() {
  recording = false;
  auto_run = false;
}

void Script::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(script_file, this);
  taBase::Own(s_args, this);
  if(script_file.fname.empty())	// initialize only on startup up, not transfer
    SetScript("");
}

void Script::CutLinks() {
  StopRecording();
  taNBase::CutLinks();
}

void Script::Copy_(const Script& cp) {
  auto_run = cp.auto_run;
  s_args = cp.s_args;
  script_file = cp.script_file;
  script_string = cp.script_string;
}

void Script::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  UpdateReCompile();
  if(!script_file.fname.empty()) {
    name = script_file.fname;
    if(name.contains(".css"))
      name = name.before(".css");
    if(name.contains('/'))
      name = name.after('/', -1);
  }
}

void Script::InstallThis() {
  ScriptBase::InstallThis();
}

bool Script::Run() {
  return ScriptBase::RunScript();
}

void Script::Record(const char* file_nm) {
#ifdef DMEM_COMPILE
  if((taMisc::dmem_nprocs > 1) && (taivMisc::record_script != NULL)) {
    taMisc::Error("Record: Cannot record a script under DMEM with the gui -- record script is used to communicate between processes");
    return;
  }
#endif
  Script_MGroup* mg = GET_MY_OWNER(Script_MGroup);
  if(mg != NULL)
    mg->StopRecording();
  if(file_nm != NULL)
    SetScript(file_nm);
  if((script_file.fname.empty()) || (script_file.gf == NULL)) {
    taMisc::Error("Record: No script file selected.\n Open a Script file and press Apply");
    return;
  }

  ostream* strm = script_file.gf->open_append();
  if((strm == NULL) || strm->bad()) {
    taMisc::Error("Record: Script file could not be opened:", script_file.fname);
    script_file.gf->Close();
    return;
  }

  taivMisc::StartRecording(strm);
  recording = true;
}

void Script::StopRecording() {
  if(!recording)
    return;
  taivMisc::StopRecording();
  script_file.gf->Close();
  recording = false;
}

void Script::Interact() {
  if(script == NULL)   return;
  cssMisc::next_shell = script;
}
  
void Script::Clear() {
  if(recording) {
    script_file.gf->Close();
    taivMisc::record_script = script_file.gf->open_write();
    return;
  }
  if((script_file.fname.empty()) || (script_file.gf == NULL)) {
    taMisc::Error("Clear: No Script File Selected\n Open a Script file and press Apply");
    return;
  }

  ostream* strm = script_file.gf->open_write();
  if((strm == NULL) || strm->bad()) {
    taMisc::Error("Clear: Script file could not be opened:", script_file.fname);
  }
  script_file.gf->Close();
}

void Script::Compile() {
  LoadScript();
}

void Script::AutoRun() {
  if(!auto_run)
    return;
  Run();
}

void Script::ScriptAllWinPos() {
  TAPtr scp = GetScopeObj();
  if((scp == NULL) || !scp->InheritsFrom(TA_WinBase)) {
    taMisc::Error("ScriptAllWinPos:: Could not locate appropriate scoping object");
    return;
  }
  ((WinBase*)scp)->ScriptAllWinPos();
}

//////////////////////////

void Script_MGroup::Initialize() {
  SetBaseType(&TA_Script);
}

void Script_MGroup::StopRecording() {
  taLeafItr i;
  Script* sb;
  FOR_ITR_EL(Script, sb, this->, i)
    sb->StopRecording();
}

void Script_MGroup::AutoRun() {
  taLeafItr i;
  Script* sb;
  FOR_ITR_EL(Script, sb, this->, i)
    sb->AutoRun();
}
  
void Script_MGroup::Run_mc(taivMenuEl* sel) {
  if(win_owner == NULL) return;
  if((sel != NULL) && (sel->usr_data != NULL)) {
    Script* itm = (Script*)sel->usr_data;
    itm->Run();
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->Run();" << endl;
    }
  }
}

// setup callbacks
declare_taivMenuCallback(Script_MGroup)
implement_taivMenuCallback(Script_MGroup)

void Script_MGroup::GetMenu_impl() {
  MenuGroup_impl::GetMenu_impl();
  
  // don't put in the groups or the stats processes within menu
  itm_list->no_in_gpmenu = true;
  itm_list->no_lst = true;

  int cur_no = ta_menu->cur_sno;
  ta_menu->AddSep();

  ta_menu->AddSubMenu("Run");
  itm_list->GetMenu(new taivMenuCallback(Script_MGroup)
		    (this, &Script_MGroup::Run_mc));
  ta_menu->SetSub(cur_no);
}
