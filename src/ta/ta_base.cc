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

// ta_base.cc

#include <ta/ta_base.h>
#include <ta/tdefaults.h>
#include <ta/ta_dump.h>

#ifndef NO_IV
#include <ta/ta_group_iv.h>
#include <ta/enter_iv.h>
#include <IV-look/kit.h>
#include <ta/leave_iv.h>
#endif


//////////////////////////////////////////
// 		tabMisc			//
//////////////////////////////////////////

TAPtr tabMisc::root = NULL;

taBase_List tabMisc::delayed_remove;
taBase_List tabMisc::delayed_updateafteredit;

void tabMisc::Close_Obj(TAPtr obj) {
  delayed_remove.Link(obj);
#ifndef NO_IV
  if(taMisc::iv_active) {
    taivMisc::CloseEdits((void*)obj, obj->GetTypeDef());
    return;
  }
#endif
  //  WaitProc();		// kill me now ?
}

int tabMisc::WaitProc() {
#ifndef NO_IV
  taivMisc::PurgeDialogs();
#endif
  bool did_something = false;
  if(delayed_remove.size > 0) {
    did_something = true;
    int i;
    for(i=0; i<delayed_remove.size; i++) {
      TAPtr it = delayed_remove.FastEl(i);
      TAPtr ownr = it->GetOwner();
      if(ownr != NULL) {
	if(!ownr->Close_Child(it)) { // didn't work
	  if(it->InheritsFrom(TA_taList_impl))
	    ((taList_impl*)it)->RemoveAll(); // remove all might have been called
	}
      }
    }
    delayed_remove.RemoveAll();
  }
  if(delayed_updateafteredit.size > 0) {
    did_something = true;
    int i;
    for(i=0; i<delayed_updateafteredit.size; i++) {
      TAPtr it = delayed_updateafteredit.FastEl(i);
      it->UpdateAfterEdit();
    }
    delayed_updateafteredit.RemoveAll();
  }
  return did_something;
}

bool tabMisc::NotifyEdits(TAPtr obj) {
#ifndef NO_IV
  return taivMisc::NotifyEdits(obj, obj->GetTypeDef());
#endif
}

void tabMisc::DelayedUpdateAfterEdit(TAPtr obj) {
  delayed_updateafteredit.Link(obj);
}


//////////////////////////////////////////////////////////
// 			taBase				//
//////////////////////////////////////////////////////////


String 	taBase::no_name;
int	taBase::no_idx = -1;
MemberDef* taBase::no_mdef = NULL;

void taBase::UpdateAfterEdit() {
  tabMisc::NotifyEdits(this);
}

void taNBase::UpdateAfterEdit(){
  tabMisc::NotifyEdits(this);
}

void taOBase::UpdateAfterEdit(){
  tabMisc::NotifyEdits(this);
}

void taOBase::CutLinks() {
  owner = NULL;
// maybe don't do this for only obases, just for nbases..
// #ifndef NO_IV
//   if(taMisc::iv_active)
//     taivMisc::CloseEdits((void*)this, GetTypeDef());
// #endif
  taBase::CutLinks();
}

void taNBase::CutLinks() {
#ifndef NO_IV
  SelectEdit::BaseClosingAll(this); // close any select edits affecting this guy (before cutting owner!)
#endif
  owner = NULL;
#ifndef NO_IV
  if(taMisc::iv_active)
    taivMisc::CloseEdits((void*)this, GetTypeDef());
#endif
  taBase::CutLinks();
}

void taBase::CutLinks() {
}

TAPtr taBase::New(int, TypeDef*) {
  return NULL;
}

void taBase::Own(TAPtr it, TAPtr onr) {
  if(it != NULL)
    Own(*it, onr);
}

void taBase::Own(taBase& it, TAPtr onr) {
  taBase::Ref(it);
  bool prv_own = false;
  if(it.GetOwner() != NULL)
    prv_own = true;
  it.SetOwner(onr);
  if(!prv_own)
    it.SetTypeDefaults();
  it.InitLinks();
  if(prv_own) {
//     if(it.InheritsFrom(TA_taNBase))
//       taMisc::Error("*** Warning: Object:",it.GetPath(),
// 		    "was transfered to a new owner, some parameters might have been reset");
  }
}

void taBase::SetPointer(TAPtr* ptr, TAPtr new_val) {
  if(*ptr == new_val)
    return;
  if(*ptr != NULL)
    unRefDone(*ptr);
  *ptr = new_val;
  if(*ptr != NULL)
    Ref(*ptr);
}

// void taBase::SetPointer(TAPtr& ptr, TAPtr new_val) {
//   if(ptr == new_val)
//     return;
//   if(ptr != NULL)
//     unRefDone(ptr);
//   ptr = new_val;
//   if(ptr != NULL)
//     Ref(ptr);
// }

void taBase::OwnPointer(TAPtr* ptr, TAPtr new_val, TAPtr onr) {
  if(*ptr == new_val)
    return;
  if(*ptr != NULL)
    unRefDone(*ptr);
  *ptr = new_val;
  if(*ptr != NULL)
    Own(*ptr, onr);
}

// void taBase::OwnPointer(TAPtr& ptr, TAPtr new_val, TAPtr onr) {
//   if(ptr == new_val)
//     return;
//   if(ptr != NULL)
//     unRefDone(ptr);
//   ptr = new_val;
//   if(ptr != NULL)
//     Own(ptr, onr);
// }

void taBase::DelPointer(TAPtr* ptr) {
  if(*ptr != NULL)
    unRefDone(*ptr);
  *ptr = NULL;
}

// void taBase::DelPointer(TAPtr& ptr) {
//   if(ptr != NULL)
//     unRefDone(ptr);
//   ptr = NULL;
// }

void taBase::SetDefaultName() {
  if(taMisc::not_constr || taMisc::in_init)
    return;
  TypeDef* td = GetTypeDef();
  int tok;
  if(td->tokens.keep && ((tok = td->tokens.Find((void *)this)) >= 0)) {
    String nm = td->name + "_" + String(tok);
    SetName(nm);
  }
}  

TAPtr taBase::GetOwner(TypeDef* td) const {
  TAPtr own = GetOwner();
  if(own == NULL)
    return NULL;
  if(own->InheritsFrom(td))
    return own;
  
  return own->GetOwner(td);
}

String taBase::GetPath_Long(TAPtr ta, TAPtr par_stop) const {
  if((this == par_stop) && (ta == NULL))
    return ".";
  String rval;

  taBase* par = GetOwner();
  if(par == NULL) {
    if(ta == NULL) rval = "root";
  }
  else if(this != par_stop)
    rval = par->GetPath_Long((TAPtr)this, par_stop);

  if((par != NULL) && (GetName() != ""))
    rval += "(" + GetName() + ")";

  if(ta != NULL) {
    MemberDef* md;
    if((md = FindMember(ta)) != NULL) {
      rval += "." + md->name;
    }
    else if((md = FindMemberPtr(ta)) != NULL) {
      rval = String("*(") + rval + "." + md->name + ")";
    }
    else {
      rval += ".?.";
    }
  }
  return rval;
}

String taBase::GetPath(TAPtr ta, TAPtr par_stop) const {
  if((this == par_stop) && (ta == NULL))
    return ".";

  String rval;
  taBase* par = GetOwner();
  if(par == NULL) {
    if(ta == NULL) rval = "root";
  }
  else if(this != par_stop)
    rval = par->GetPath((TAPtr)this, par_stop);

  if(ta != NULL) {
    MemberDef* md;
    if((md = FindMember(ta)) != NULL) {
      rval += "." + md->name;
    }
    else if((md = FindMemberPtr(ta)) != NULL) {
      rval = String("*(") + rval + "." + md->name + ")";
    }
    else {
      rval += ".?.";
    }
  }
  return rval;
}

int taBase::GetNextPathDelimPos(const String& path, int start) {
  int point_idx = path.index('.', start+1); // skip any possible starting delim
  int brack_idx = path.index('[', start+1);

  // if there is a period but not a bracket, or the period is before the bracket
  if(((brack_idx < start) && (point_idx >= start)) ||
     ((point_idx < brack_idx ) && (point_idx >= start)))
  {
    return point_idx;
  }
  else if(brack_idx >= start) {		// else try the bracket
    return brack_idx;
  }
  return path.length();			// delimiter is end of string (its all element)
}

int taBase::GetLastPathDelimPos(const String& path) {
  int point_idx = path.index('.',-1);
  int brack_idx = path.index('[',-1);

  if(point_idx > brack_idx) {		// point comes after bracket
    return point_idx;
  }
  else if(brack_idx >= 0) {
    return brack_idx;
  }
  return 0;
}


TAPtr taBase::FindFromPath(String& path, MemberDef*& ret_md, int start) const {
  if(((int)path.length() <= start) || (path == ".")) {
    ret_md = NULL;
    return (TAPtr)this;
  }
  if((path == "Null") || (path == "NULL")) {
    ret_md = NULL;
    return NULL;
  }
  
  TAPtr rval = NULL;
  bool ptrflag = false;
  int length = path.length();
  
  while(start < length) {
    if(path[start] == '*') {	// path is a pointer
      start += 2;
      ptrflag = true;
    }
    if(path[start] == '.') { 	// must be root, so search on next stuff
      start++;
      continue;
    }
    
    int delim_pos = taBase::GetNextPathDelimPos(path, start);
    String el_path = path(start,delim_pos-start); // element is between start and delim
    int next_pos = delim_pos+1;
    if((delim_pos < length) && (path[delim_pos] == '['))
      next_pos--;
    
    void* tmp_ptr;
    MemberDef* md;
    if(((md = FindMembeR(el_path, tmp_ptr)) != NULL) && md->type->InheritsFrom(TA_taBase)) {
      TAPtr mbr = (TAPtr)tmp_ptr;
      if(delim_pos < length) {	// there's more to be done..
	rval = mbr->FindFromPath(path, ret_md, next_pos); // start from after delim
      }
      else {
	rval = mbr;		// that's all folks..
	ret_md = md;
      }
    }
    else if((el_path == "root") && (delim_pos < length)) {
      start = next_pos;	// skip this element since it must be us
      continue;
    }
    if((ptrflag) && (rval != NULL))
      return *((TAPtr *)rval);
    return rval;
  }
  return NULL;
}

TypeDef* taBase::GetScopeType() {
  TypeDef* scp_tp = taMisc::default_scope;
  String scp_nm  = GetTypeDef()->OptionAfter("SCOPE_");
  if(scp_nm != "")
    scp_tp = taMisc::types.FindName(scp_nm);
  return scp_tp;
}

TAPtr taBase::GetScopeObj(TypeDef* scp_tp) {
  if(scp_tp == NULL)
    scp_tp = GetScopeType();
  if(scp_tp == NULL)
    return tabMisc::root;
  return GetOwner(scp_tp);
}

bool taBase::SameScope(TAPtr ref_obj, TypeDef* scp_tp) {
  if(scp_tp == NULL)
    scp_tp = GetScopeType();
  if((scp_tp == NULL) || (ref_obj == NULL))
    return true;
  
  TAPtr my_scp = GetOwner(scp_tp);
  if((my_scp == NULL) || (my_scp == ref_obj) || (my_scp == ref_obj->GetOwner(scp_tp)))
    return true;
  return false;
}

int taBase::NTokensInScope(TypeDef* td, TAPtr ref_obj, TypeDef* scp_tp) {
  if(ref_obj == NULL)
    return td->tokens.size;
  int cnt = 0;
  int i;
  for(i=0; i<td->tokens.size; i++) {
    TAPtr tmp = (TAPtr)td->tokens.FastEl(i);
    if(tmp->SameScope(ref_obj, scp_tp))
      cnt++;
  }
  return cnt;
}

TAPtr taBase::MakeToken(TypeDef* td) {
  if(td->GetInstance() != NULL) {
    return ((TAPtr)td->GetInstance())->MakeToken();
  }
  else
    return NULL;
}

TAPtr taBase::MakeTokenAry(TypeDef* td, int no) {
  if(td->GetInstance() != NULL) {
    return ((TAPtr)td->GetInstance())->MakeTokenAry(no);
  }
  else
    return NULL;
}

void taBase::SetTypeDefaults_impl(TypeDef* ttd, TAPtr scope) {
  if(ttd->defaults == NULL) return;
  int i;
  for(i=0; i<ttd->defaults->size; i++) {
    TypeDefault* td = (TypeDefault*)ttd->defaults->FastEl(i);
    TAPtr tdscope = td->GetScopeObj(taMisc::default_scope);
    if(tdscope == scope) {
      td->SetTypeDefaults(this);
      break;
    }
  }
}  

void taBase::SetTypeDefaults_parents(TypeDef* ttd, TAPtr scope) {
  int i;
  for(i=0; i<ttd->parents.size; i++) {
    TypeDef* par = ttd->parents.FastEl(i);
    if(!par->InheritsFrom(TA_taBase)) continue;	// only ta-bases
    SetTypeDefaults_parents(par, scope); // first do parents of parent
    if(par->defaults != NULL)
      SetTypeDefaults_impl(par, scope);    // then actually do parent
  }
  if(ttd->defaults != NULL)
    SetTypeDefaults_impl(ttd, scope);    // then actually do it
}

void taBase::SetTypeDefaults() {
  TypeDef* ttd = GetTypeDef();	// this typedef = ttd
  TAPtr scope = GetScopeObj(taMisc::default_scope); // scope for default vals
  SetTypeDefaults_parents(ttd, scope);
}

int taBase::Edit(bool wait) {
  taivEdit* ive;
#ifndef NO_IV
  if((ive = GetTypeDef()->ive) != NULL) {
    const ivColor* bgclr = GetEditColorInherit();
    return ive->Edit((void*)this, NULL, wait, false, bgclr);
  }
#endif
  return false;
}

bool taBase::CloseEdit() {
#ifndef NO_IV
  return taivMisc::CloseEdits((void*)this, GetTypeDef());
#endif
  return false;
}

#ifndef NO_IV
const ivColor* taBase::GetEditColorInherit() {
  const ivColor* bgclr = GetEditColor();
  if(bgclr == NULL) {
    TAPtr ownr = GetOwner();
    while((ownr != NULL) && (bgclr == NULL)) {
      bgclr = ownr->GetEditColor();
      ownr = ownr->GetOwner();
    }
  }
  return bgclr;
}
#endif

void taBase::Close() {
  if(GetOwner() == NULL)
    return;
  tabMisc::Close_Obj(this);
}

bool taBase::Close_Child(TAPtr) {
  return false;
}

bool taBase::CopyFrom(TAPtr cpy_from) {
  if(cpy_from == NULL) return false;
  if(!cpy_from->InheritsFrom(GetTypeDef()) && !InheritsFrom(cpy_from->GetTypeDef())) {
    taMisc::Error("Cannot copy from given object of type:",cpy_from->GetTypeDef()->name,
		  "which does not inherit from:", GetTypeDef()->name,
		  "(or I don't inherit from it)", GetPath());
    return false;
  }
  UnSafeCopy(cpy_from);
  return true;
}

bool taBase::CopyTo(TAPtr cpy_to) {
  if(cpy_to == NULL) return false;
  if(!cpy_to->InheritsFrom(GetTypeDef()) && !InheritsFrom(cpy_to->GetTypeDef())) {
    taMisc::Error("Cannot copy to given object of type:",cpy_to->GetTypeDef()->name,
		  "which does not inherit from:", GetTypeDef()->name,
		  "(or I don't inherit from it)",GetPath());
    return false;
  }
  cpy_to->UnSafeCopy(this);
  return true;
}

bool taBase::DuplicateMe() {
  TAPtr ownr = GetOwner();
  if(ownr == NULL) return false;
  if(!ownr->InheritsFrom(TA_taList_impl)) {
    taMisc::Error("Cannot duplicate me because owner is not a list/group!",GetPath());
    return false;
  }
  taList_impl* own = (taList_impl*)ownr;
  own->DuplicateEl(this);
  return true;
}

#ifndef NO_IV
static void tabase_base_closing_all_gp(TAPtr obj) {
  SelectEdit::BaseClosingAll(obj); // get it before it is moved around and stuff
  // also check for groups and objects in them that might die
  TypeDef* td = obj->GetTypeDef();
  int i;
  for(i=0;i<td->members.size;i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(TA_taList_impl)) continue;
    if(md->type->InheritsFrom(TA_taGroup_impl)) {
      taGroup_impl* gp = (taGroup_impl*)md->GetOff((void*)obj);
      int lf;
      for(lf=0;lf<gp->leaves;lf++) {
	TAPtr chld = (TAPtr)gp->Leaf_(lf);
	if((chld != NULL) && (chld->GetOwner() == gp))
	  tabase_base_closing_all_gp(chld); // get it before it is moved around and stuff
      }
      continue;
    }
    taList_impl* gp = (taList_impl*)md->GetOff((void*)obj);
    int gi;
    for(gi=0;gi<gp->size;gi++) {
      TAPtr chld = (TAPtr)gp->FastEl_(gi);
      if((chld != NULL) && (chld->GetOwner() == gp))
	tabase_base_closing_all_gp(chld); // get it before it is moved around and stuff
    }
  }
}
#endif

bool taBase::ChangeMyType(TypeDef* new_type) {
  TAPtr ownr = GetOwner();
  if((new_type == NULL) || (ownr == NULL)) return false;
  if(!ownr->InheritsFrom(TA_taList_impl)) {
    taMisc::Error("ChangeMyType: Cannot change my type because owner is not a list/group!",GetPath());
    return false;
  }
  if(new_type == GetTypeDef()) {
    taMisc::Error("ChangeMyType: Object is already of selected type!",GetPath());
    return false;
  }
#ifndef NO_IV  
  tabase_base_closing_all_gp(this);
#endif
  taList_impl* own = (taList_impl*)ownr;
  return own->ChangeType(this, new_type);
  // owner will used delayed remove to make this safe!
}

void taBase::ReplacePointersHook(TAPtr old_ptr) {
  taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)old_ptr, (void*)this);
}

int taBase::ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr) {
  return GetTypeDef()->ReplaceAllPtrsThis((void*)this, obj_typ, old_ptr, new_ptr);
}

bool taBase::SelectForEdit(MemberDef* member, SelectEdit* editor, const char* extra_label) {
  if((editor == NULL) || (member == NULL)) return false;
#ifndef NO_IV  
  return editor->SelectMember(this, member, extra_label);
#else
  return false;
#endif
}

bool taBase::SelectForEditNm(const char* member, SelectEdit* editor, const char* extra_label) {
  if((editor == NULL) || (member == NULL)) return false;
#ifndef NO_IV  
  return editor->SelectMemberNm(this, member, extra_label);
#else
  return false;
#endif
}

bool taBase::SelectFunForEdit(MethodDef* function, SelectEdit* editor, const char* extra_label) {
  if((editor == NULL) || (function == NULL)) return false;
#ifndef NO_IV  
  return editor->SelectMethod(this, function, extra_label);
#else
  return false;
#endif
}

bool taBase::SelectFunForEditNm(const char* function, SelectEdit* editor, const char* extra_label) {
  if((editor == NULL) || (function == NULL)) return false;
#ifndef NO_IV  
  return editor->SelectMethodNm(this, function, extra_label);
#else
  return false;
#endif
}

taivGetFile* taBase::GetFileDlg() {
  if(!taMisc::iv_active) 
    return NULL;

  TypeDef* td = GetTypeDef();
  String fltr;
  int opt;
  if((opt = td->opts.FindContains("EXT_")) >= 0) {
    fltr = td->opts.FastEl(opt).after("EXT_");
    fltr = "*." + fltr + "*";
  }
  else
    fltr = "*";
  bool cmprs = false;
  if(td->HasOption("COMPRESS"))
    cmprs = true;
#ifndef NO_IV
  return new taivGetFile(".", fltr, NULL, taivM->wkit->style(), cmprs);
#else
  return NULL;
#endif
}

void taBase::Help() {
  TypeDef* mytd = GetTypeDef();
  String full_file;
  while((mytd != NULL) && full_file.empty()) {
    String help_file = taMisc::help_file_tmplt;
    help_file.gsub("%t", mytd->name);
    full_file = taMisc::FindFileInclude(help_file);
    mytd = mytd->parents.SafeEl(0);	// go with the parent
  }
  if(full_file.empty()) {
    taMisc::Error("Sorry, no help available for type:", GetTypeDef()->name);
    return;
  }

  String help_cmd = taMisc::help_cmd;
  help_cmd.gsub("%s", full_file);
  system(help_cmd);
}

void taBase::CallFun(const char* fun_name) {
#ifndef NO_IV
  if(!taMisc::iv_active) return;
  MethodDef* md = GetTypeDef()->methods.FindName(fun_name);
  if(md != NULL)
    md->CallFun((void*)this);
  else
    taMisc::Error("*** CallFun Error: function:", fun_name, "not found on object:", this->GetPath());
#endif
}

void taNBase::Copy_(const taNBase& cp) {
  if(!cp.name.empty())
    name = cp.name;
}



//////////////////////////////////////////////////////////
// 			taList_impl			//
//////////////////////////////////////////////////////////

MemberDef* taList_impl::find_md = NULL;

void taList_impl::Initialize() {
  owner = NULL;
  SetBaseType(&TA_taBase);
  el_def = 0;
}

void taList_impl::Destroy() {
  CutLinks();
}

void taList_impl::CutLinks() {
  RemoveAll();
  owner = NULL;
  taBase::CutLinks();
}

void taList_impl::Copy(const taList_impl& cp) {
  if(!cp.name.empty())
    name = cp.name;
  el_base = cp.el_base;
  el_typ = cp.el_typ;
  el_def = cp.el_def;
  taPtrList_impl::Copy_Duplicate(cp);
}

void taList_impl::SetBaseType(TypeDef* it) {
  el_base = it;
  el_typ = it;
}

TAPtr taList_impl::New(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivListNew::New(this,typ);
#endif
    return NULL;
  }
  if(typ == NULL)
    typ = el_typ;
  if(!typ->InheritsFrom(el_base)) {
    taMisc::Error("*** Attempt to create type:", typ->name,
		   "in list with base type:", el_base->name);
    return NULL;
  }
  TAPtr rval = NULL;
  Alloc(size + no);		// pre-allocate!
  if((size == 0) || (no > 1))
    el_typ = typ;	// first item or multiple items set el_typ
  int i;
  for(i=0; i < no; i++) {
    rval = taBase::MakeToken(typ);
    if(rval != NULL)
      Add_(rval);
  }
  return rval;
}

void taList_impl::EnforceSize(int sz) {
  if(size < sz)
    New(sz - size);
  else {
    int i;
    for(i=size-1; i>=sz; i--)
      Remove(i);
  }
}

void taList_impl::EnforceType() {
  int i;
  for(i=0; i<size; i++) {
    TAPtr itm = (TAPtr)el[i];
    if((itm == NULL) || (itm->GetTypeDef() == el_typ))
      continue;

    TAPtr rval = taBase::MakeToken(el_typ);
    if(rval != NULL)
      Replace(i, rval);
  }
}

void taList_impl::EnforceSameStru(const taList_impl& cp) {
  int i;
  for(i=0; i<cp.size; i++) {
    TAPtr citm = (TAPtr)cp.el[i];
    if(citm == NULL) continue;
    if(size <= i) {
      TAPtr itm = taBase::MakeToken(citm->GetTypeDef());
      if(itm != NULL)
	Add_(itm);
    }
    else {
      TAPtr itm = (TAPtr)el[i];
      if(citm->GetTypeDef() == itm->GetTypeDef())
	continue;
      TAPtr rval = taBase::MakeToken(citm->GetTypeDef());
      if(rval != NULL)
	Replace(i, rval);
    }
  }
  if(size > cp.size)
    for(i=size-1; i>=cp.size; i--)
      Remove(i);
}

void taList_impl::Close() {
  if(size > 0) {
    RemoveAll();
    return;
  }
  if(GetOwner() == NULL)
    return;
  tabMisc::Close_Obj(this);
}

int taList_impl::Find(TypeDef* it) const {
  int i;
  for(i=0; i < size; i++) {
    if(((TAPtr)el[i])->InheritsFrom(it))
      return i;
  }
  return -1;
}

TAPtr taList_impl::FindType_(TypeDef* it, int& idx) const {
  idx = Find(it);
  if(idx >= 0) return (TAPtr)el[idx];
  return NULL;
}

int taList_impl::SetDefaultEl(TAPtr it) {
  int idx = Find(it);
  if(idx >= 0)    el_def = idx;
  return idx;
}

int taList_impl::SetDefaultEl(const char* nm) {
  int idx = Find(nm);
  if(idx >= 0)    el_def = idx;
  return idx;
}

int taList_impl::SetDefaultEl(TypeDef* it) {
  int idx = Find(it);
  if(idx >= 0)    el_def = idx;
  return idx;
}

bool taList_impl::Remove(int i) {
  // default could be out of range..
  if(el_def >= size-1)
    el_def = 0;
  return taPtrList_ta_base::Remove(i);
}

bool taList_impl::Close_Child(TAPtr obj) {
  return Remove(obj);
}

bool taList_impl::ChangeType(int idx, TypeDef* new_type) {
  if(new_type == NULL) return false;
  TAPtr itm = (TAPtr)el[idx];
  if(itm == NULL) return false;
  TypeDef* itd = itm->GetTypeDef();
  if(!new_type->InheritsFrom(itd) && !itm->InheritsFrom(new_type)) {
    // do they have a common parent? if so, convert to that first, then back to new_type
    if(itd->parents.size >= 1) {
      if(new_type->InheritsFrom(itd->GetParent())) {
	ChangeType(idx, itd->GetParent());
	itm = (TAPtr)el[idx];
	Remove(size-1);			// remove the last guy!
      }
      else if((itd->GetParent()->parents.size >= 1) && 
	      new_type->InheritsFrom(itd->GetParent()->GetParent())) {
	ChangeType(idx, itd->GetParent()->GetParent());
	itm = (TAPtr)el[idx];
	Remove(size-1);			// remove the last guy!
      }
      else {
	taMisc::Error("Cannot change to new type:",new_type->name,
		      "which does not inherit from:", itd->name,
		      "(or vice-versa)",itm->GetPath());
	return false;
      }
    }
    else {
      taMisc::Error("Cannot change to new type:",new_type->name,
		    "which does not inherit from:", itd->name,
		    "(or vice-versa)",itm->GetPath());
      return false;
    }
  }
  TAPtr rval = taBase::MakeToken(new_type);
  if(rval == NULL) return false;
  Add(rval);		// add to end of list
  String orgnm = rval->GetName();
  rval->UnSafeCopy(itm);	// do the copy!
  // if new name is based on type def, give it the appropriate new type def name
  String nwnm = rval->GetName();
  if(nwnm.contains(itm->GetTypeDef()->name)) 
    rval->SetName(orgnm);
  Swap(idx, size-1);		// switch positions, so old guy is now at end!
  rval->ReplacePointersHook(itm); // allow us to update all things that might point to us
  tabMisc::Close_Obj(itm);
  // then do a delayed remove of this object (in case called by itself!)
  return true;
}

bool taList_impl::ChangeType(TAPtr itm, TypeDef* new_type) {
  int idx = Find(itm);
  if(idx >= 0)
    return ChangeType(idx, new_type);
  return false;
}

int taList_impl::ReplaceType(TypeDef* old_type, TypeDef* new_type) {
  int nchanged = 0;
  int sz = size;		// only go to current size
  int i;
  for(i=0;i<sz;i++) {
    if(((TAPtr)el[i])->GetTypeDef() != old_type) continue;
    if(ChangeType(i, new_type)) nchanged++;
  }
  return nchanged;
}

int taList_impl::ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr) {
  if(!(el_base->InheritsFrom(obj_typ) || obj_typ->InheritsFrom(el_base))) return 0; 
  // could not fit in this list!
  TAPtr taold = (TAPtr)old_ptr;
  TAPtr tanew = (TAPtr)new_ptr;
  // don't replace if no owner or if this is the owner -- could be replacing
  // something that is already in process of being replaced by a list -- dangerous!
  if((taold->GetOwner() == NULL) || (taold->GetOwner() == this)) return 0;
  if(tanew == NULL) {
    if(Remove(taold)) {
      taMisc::Error("*** Note: removed object:", taold->GetName(),"in List:", GetPath());
      return 1;
    }
  }
  else {
    if(ReplaceLink(taold, tanew)) {
      taMisc::Error("*** Note: replaced object:", taold->GetName(),"with:",tanew->GetName(),
		    "in List:", GetPath());
      return 1;
    }
  }
  return 0;
}

String taList_impl::GetPath_Long(TAPtr ta, TAPtr par_stop) const {
  if((((TAPtr) this) == par_stop) && (ta == NULL))
    return ".";
  String rval;

  taBase* par = GetOwner();
  if(par == NULL) {
    if(ta == NULL) rval = "root";
  }
  else if(((TAPtr) this) != par_stop)
    rval = par->GetPath_Long((TAPtr)this, par_stop);

  if(GetName() != "")
    rval += "(" + GetName() + ")";
  
  if (ta != NULL) {
    MemberDef* md;
    if((md = FindMember(ta)) != NULL) {
      rval += "." + md->name;
    }
    else if((md = FindMemberPtr(ta)) != NULL) {
      rval = String("*(") + rval + "." + md->name + ")";
    }
    else {
      int gidx = Find_(ta);
      if(gidx >= 0)
	rval += "[" + String(gidx) + "]";
      else
	rval += "[?]";
    }
  }   
  return rval;
}

String taList_impl::GetPath(TAPtr ta, TAPtr par_stop) const {
  if((((TAPtr) this) == par_stop) && (ta == NULL))
    return ".";
  String rval;

  taBase* par = GetOwner();
  if(par == NULL) {
    if(ta == NULL) rval = "root";
  }
  else if(((TAPtr) this) != par_stop)
    rval = par->GetPath((TAPtr)this, par_stop);

  if (ta != NULL) {
    MemberDef* md;
    if((md = FindMember(ta)) != NULL) {
      rval += "." + md->name;
    }
    else if((md = FindMemberPtr(ta)) != NULL) {
      rval = String("*(") + rval + "." + md->name + ")";
    }
    else {
      int gidx = Find_(ta);
      if(gidx >= 0)
	rval += "[" + String(gidx) + "]";
      else
	rval += "[?]";
    }
  }   
  return rval;
}

MemberDef* taList_impl::ReturnFindMd() const {
  if(find_md != NULL) return find_md;
  find_md = new MemberDef(&TA_taBase, "find_md", "return value", "", "", NULL);
  return find_md;
}

MemberDef* taList_impl::FindMembeR(const char* nm, void*& ptr) const {
  String idx_str = nm;
  idx_str = idx_str.before(']');
  if(idx_str != "") {
    idx_str = idx_str.after('[');
    int idx = atoi(idx_str);
    if((size == 0) || (idx >= size)) {
      ptr = NULL;
      return NULL;
    }
    ptr = el[idx];
    return ReturnFindMd();
  }

  int i;
  if((i = Find(nm)) >= 0) {
    ptr = el[i];
    return ReturnFindMd();
  }

  MemberDef* rval;
  if((rval = GetTypeDef()->members.FindNameAddrR(nm, (void*)this, ptr)) != NULL)
    return rval;
  int max_srch = MIN(taMisc::search_depth, size);
  for(i=0; i<max_srch; i++) {
    TAPtr first_el = (TAPtr)FastEl_(i);
    if((first_el != NULL) && // only search owned objects
       ((first_el->GetOwner()==NULL) || (first_el->GetOwner() == ((TAPtr) this)))) {
      first_el->FindMembeR(nm, ptr);
    }
  }
  ptr = NULL;
  return NULL;
}

MemberDef* taList_impl::FindMembeR(TypeDef* it, void*& ptr) const {
  int i;
  if((i = Find(it)) >= 0) {
    ptr = el[i];
    return ReturnFindMd();
  }
  
  MemberDef* rval;
  if((rval = GetTypeDef()->members.FindTypeAddrR(it, (void*)this, ptr)) != NULL)
    return rval;
  int max_srch = MIN(taMisc::search_depth, size);
  for(i=0; i<max_srch; i++) {
    TAPtr first_el = (TAPtr)FastEl_(i);
    if((first_el != NULL) && // only search owned objects
       ((first_el->GetOwner()==NULL) || (first_el->GetOwner() == ((TAPtr) this)))) {
      first_el->FindMembeR(it, ptr);
    }
  }
  ptr = NULL;
  return NULL;
}

ostream& taList_impl::OutputR(ostream& strm, int indent) const {
  taMisc::indent(strm, indent) << name << "[" << size << "] = {\n";
  TypeDef* td = GetTypeDef();
  int i;
  for(i=0; i < td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(md->HasOption("EDIT_IN_GROUP"))
      md->Output(strm, (void*)this, indent+1);
  }

  for(i=0; i<size; i++) {
    if(el[i] == NULL) continue;
    ((TAPtr)el[i])->OutputR(strm, indent+1);
  }

  taMisc::indent(strm, indent) << "}\n";
  return strm;
}

int taList_impl::Dump_Save_PathR(ostream& strm, TAPtr par, int indent) {
  // first save any sub-members (this really doesn't ever happen, cuz there isn't any!)
  int rval = GetTypeDef()->Dump_Save_PathR(strm, (void*)this, (void*)par, indent);

  if(IsEmpty())  return rval;

  strm << "\n";			// actually saving a path: put a newline

  if(par != this) {		// par == this when saved as member of a group
    taMisc::indent(strm, indent, 1);
    Dump_Save_Path(strm, par, indent); // save my path!
    if(HasOption("ARRAY_ALLOC")) { // allocate thing as an array
      TAPtr sb = (TAPtr)el[0];	// check 1st guy to see if onwed or linked!
      if((sb->GetOwner() == NULL) || (sb->GetOwner() == (TAPtr)this))
	strm << " = [" << size << "] " << el_typ->name << " {\n";
      else
	strm << " = [" << size << "] {\n"; // not owned, don't save type!
    }
    else		// always pre-allocate ptrs for the size to make memory cleaner
      strm << " = [" << size << "] {\n";
  }

  Dump_Save_PathR_impl(strm, this, indent+1);

  if(par != this) {
    taMisc::indent(strm, indent, 1);
    strm << "};\n";
  }
  return true;
}

int taList_impl::Dump_Save_PathR_impl(ostream& strm, TAPtr par, int indent) {
  int cnt = 0;
  int i;
  for(i=0; i<size; i++) {
    TAPtr itm = (TAPtr)el[i];
    if(itm == NULL)
      continue;

    if(El_GetOwner_(itm) != this) { // a link, create a dummy placeholder
      taMisc::indent(strm, indent, 1);
      strm << itm->GetTypeDef()->name << " @[" << i << "] { };\n";
      continue;
    }
    taMisc::indent(strm, indent, 1);
    itm->Dump_Save_Path(strm, par, indent);
    // can't put this in dump_save_path cuz don't want it during non PathR times..
    if(itm->InheritsFrom(TA_taList_impl)) {
      taList_impl* litm = (taList_impl*)itm;
      if(!litm->IsEmpty()) {
	if(litm->HasOption("ARRAY_ALLOC")) { // actually save and allocate as one "array"
	  TAPtr sb = (TAPtr)litm->el[0];	// check 1st guy to see if onwed or linked!
	  if((sb->GetOwner() == NULL) || (sb->GetOwner() == (TAPtr)litm))
	    strm << " = [" << litm->size << "] " << litm->el_typ->name;
	  else			// always pre-allocate ptrs for the size to make memory cleaner
	    strm << " = [" << litm->size << "]";
	}
	else			// always pre-allocate ptrs for the size to make memory cleaner
	  strm << " = [" << litm->size << "]";
      }
    }
    strm << " { ";
    if(itm->Dump_Save_PathR(strm, itm, indent+1))
      taMisc::indent(strm, indent, 1);
    strm << "};\n";
    cnt++;
  }
  return cnt;
}

// actually save all the elements in the group
int taList_impl::Dump_SaveR(ostream& strm, TAPtr par, int indent) {
  String mypath = GetPath(NULL, par);
  int i;
  for(i=0; i<size; i++) {
    if(el[i] == NULL) continue;
    TAPtr itm = (TAPtr)el[i];
    if(El_GetOwner_((TAPtr)el[i]) == this) {
      itm->Dump_Save_impl(strm, par, indent);
    }
    else if(El_GetOwner_(itm) != NULL) {	// a link
      taMisc::indent(strm, indent, 1) << GetTypeDef()->name << " ";
      if(par != NULL) strm << "@";
      strm << mypath << " = ";
      strm << "[" << i << "] = ";
      if(itm->Dump_Save_Path(strm, NULL, indent))
	strm << ";\n";
      if(itm->HasOption("LINK_SAVE"))
	itm->Dump_Save_impl(strm, NULL, indent); // save even though its a link!
    }
  }
  return true;
}

int taList_impl::Dump_Load_Value(istream& strm, TAPtr par) {
  int c = taMisc::skip_white(strm);
  if(c == EOF)	return EOF;
  if(c == ';')	return 2;	// signal that its a path
  if(c == '}') {
    if(strm.peek() == ';') strm.get();
    return 2;
  }
  
  if(c == '{') {
    return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par);
  }
  else if(c == '=') {		// a link or the number of items to create
    c = taMisc::skip_white(strm);
    if(c == '[') {
      c = taMisc::read_word(strm);
      if(c == ']') {
	int idx = atoi(taMisc::LexBuf);
	c = taMisc::skip_white(strm, true); // peek
	if(c == '=') {		// this means its a link
	  strm.get();
	  c = taMisc::read_word(strm); // get type
	  String typ_nm = taMisc::LexBuf;
	  TypeDef* eltd = taMisc::types.FindName(typ_nm);
	  if((eltd == NULL) || !(eltd->InheritsFrom(el_base))) {
	    taMisc::Error("*** Null or invalid type:",typ_nm,"to link into group:",
			    GetPath(),"of types:",el_base->name);
	    return false;
	  }
	  c = taMisc::read_till_rb_or_semi(strm);
	  String lnk_path = taMisc::LexBuf;
	  dumpMisc::path_subs.FixPath(eltd, tabMisc::root, lnk_path);
	  MemberDef* md;
	  TAPtr tp = tabMisc::root->FindFromPath(lnk_path, md);
	  if(idx < size)
	    ReplaceLink(idx, tp); // if already room, replace it..
	  else {
	    Link(tp);		// otherwise, add it..
	    idx = size-1;
	  }
	  if(tp == NULL) {
	    dumpMisc::vpus.Add((TAPtr*)&(el[idx]), (TAPtr)NULL, lnk_path);
	  }
	  return true;
	}
	else if(c == '{') {	// no type information -- just the count
	  strm.get();		// get the bracket
	  Alloc(idx);		// just make sure we have the ptrs allocated to this size
	  return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par);
	}
	else {			// type information -- create objects too!
	  taMisc::read_word(strm, true); // get type
	  String typ_nm = taMisc::LexBuf;
	  TypeDef* eltd = taMisc::types.FindName(typ_nm);
	  if((eltd == NULL) || !(eltd->InheritsFrom(el_base))) {
	    taMisc::Error("*** Null or invalid type:",typ_nm,"to create into group:",
			  GetPath(),"of types:",el_base->name);
	    return false;
	  }
	  el_typ = eltd;
	  // ensure that enough items are present (don't do full enforce size)
	  if(size < idx)
	    New(idx - size, el_typ);
	  c = taMisc::skip_white(strm);
	  if(c == '{') {
	    return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par);
	  }
	}
      }
    }
    taMisc::Error("*** Bad formatting for link in group",GetPath(),
		    "of type:",GetTypeDef()->name);
    return false;
  }
  
  taMisc::Error("*** Missing '{', '=', or '[' in dump file for group:",
		GetPath(),"of type:",GetTypeDef()->name);
  return false;
}
    

//////////////////////////////////////////////////////////
// 			taArray_base			//
//////////////////////////////////////////////////////////

void taArray_base::CutLinks() {
  Reset();
  owner = NULL;
  taBase::CutLinks();
}

ostream& taArray_base::Output(ostream& strm, int indent) const {
  taMisc::indent(strm, indent);
  List(strm);
  strm << ";\n";
  return strm;
}

int taArray_base::Dump_Save_Value(ostream& strm, TAPtr, int) {
  strm << "{ ";
  int i;
  for(i=0;i<size;i++) {
    strm << El_GetStr_(FastEl_(i)) << ";";
  }
  return true;
}

int taArray_base::Dump_Load_Value(istream& strm, TAPtr) {
  int c = taMisc::skip_white(strm);
  if(c == EOF)    return EOF;
  if(c == ';') // just a path
    return 2;  // signal that just a path was loaded..
  
  if(c != '{') {
    taMisc::Error("Missing '{' in dump file for type:",GetTypeDef()->name,"\n");
    return false;
  }
  c = taMisc::read_till_rb_or_semi(strm);
  int cnt = 0;
  while((c == ';') && (c != EOF)) {
    if(cnt > size-1)
      Add_(El_GetTmp_());
    El_SetFmStr_(SafeEl_(cnt++), taMisc::LexBuf);
    c = taMisc::read_till_rb_or_semi(strm);
  }
  if(c==EOF)	return EOF;
  return true;
}

void int_Array::FillSeq(int start, int inc) {
  int i, v;
  for(i=0,v=start; i<size; i++, v += inc)
    FastEl(i) = v;
}

void long_Array::FillSeq(long start, long inc) {
  long i, v;
  for(i=0,v=start; i<size; i++, v += inc)
    FastEl(i) = v;
}
