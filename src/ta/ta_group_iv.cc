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

// ta_group_iv.cc

#include <ta/ta_group_iv.h>
#include <css/ta_css.h>

#include <iv_misc/lrScrollBox.h>
#include <iv_misc/tbScrollBox.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/event.h>
#include <InterViews/handler.h>
#include <InterViews/hit.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/window.h>
#include <InterViews/label.h>
#ifndef CYGWIN
#include <X11/keysym.h>
#endif
#include <InterViews/debug.h>
#include <ta/leave_iv.h>


//////////////////////////
// 	Edit Buttons	//
//////////////////////////


gpivListEditButton::gpivListEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: taivEditButton(tp, base, win, NULL, dlg, par)
{
}

void gpivListEditButton::SetLabel() {
  TABLPtr lst = (TABLPtr)cur_base;
  if(lst == NULL) {
    taivEditButton::SetLabel();
    return;
  }
  String nm = " Size: ";
  nm += String(lst->size);
  nm += String(" (") + lst->el_typ->name + ")";
  rep->SetMLabel(nm);
}

gpivGroupEditButton::gpivGroupEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: taivEditButton(tp, base, win, NULL, dlg, par)
{
}

void gpivGroupEditButton::SetLabel() {
  TAGPtr gp = (TAGPtr)cur_base;
  if(gp == NULL) {
    taivEditButton::SetLabel();
    return;
  }
  String nm = " Size: ";
  nm += String(gp->size);
  if(gp->gp.size > 0)
    nm += String(".") + String(gp->gp.size);
  if(gp->leaves != gp->size) 
    nm += String(".") + String((int) gp->leaves);
  nm += String(" (") + gp->el_typ->name + ")";
  rep->SetMLabel(nm);
}


gpivSubEditButton::gpivSubEditButton
(TypeDef* tp, void* base, ivWindow* win, const char* nm, taivDialog* dlg, taivData* par)
: taivEditButton(tp, base, win, NULL, dlg, par)
{
  label = nm;
}

void gpivSubEditButton::SetLabel() {
  TAGPtr gp = (TAGPtr)cur_base;
  if(gp == NULL) {
    rep->SetMLabel(label);
    return;
  }
  String nm = label + ", Size: ";
  nm += String(gp->size);
  if(gp->gp.size > 0)
    nm += String(".") + String(gp->gp.size);
  if(gp->leaves != gp->size) 
    nm += String(".") + String((int) gp->leaves);
  nm += String(" (") + gp->el_typ->name + ")";
  rep->SetMLabel(nm);
}

//////////////////////////////////
//  	LinkEditButton		//
//////////////////////////////////

gpivLinkEditButton::gpivLinkEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: gpivGroupEditButton(tp, base, win, dlg, par)
{
}

void gpivLinkEditButton::GetMethMenus() {
  if(meth_el.size > 0)		// only do this once..
    return;
  String men_nm, lst_men_nm;
  int i;
  for(i=0; i<typ->methods.size; i++) {
    MethodDef* md = typ->methods.FastEl(i);
    if((md->iv == NULL) || (md->name == "Close"))
      continue;
    String cur_nm = md->OptionAfter("MENU_ON_");
    if(cur_nm != "")
      men_nm = cur_nm;
    // has to be on one of these two menus..
    if((men_nm != "Object") && (men_nm != "Edit"))
      continue;
    if((men_nm == "Object") && (md->name != "Edit"))
      continue;
    if((md->name == "DuplicateEl") || (md->name == "Transfer"))
      continue;
    lst_men_nm = men_nm;
    taivMethMenu* mth_rep = md->iv->GetMethodRep(cur_base, dialog);
    if(mth_rep == NULL)
      continue;
    meth_el.Add(mth_rep);
    mth_rep->AddToMenu(rep);
  }
}

gpivListLinkEditButton::gpivListLinkEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: gpivListEditButton(tp, base, win, dlg, par)
{
}

void gpivListLinkEditButton::GetMethMenus() {
  if(meth_el.size > 0)		// only do this once..
    return;
  String men_nm, lst_men_nm;
  int i;
  for(i=0; i<typ->methods.size; i++) {
    MethodDef* md = typ->methods.FastEl(i);
    if((md->iv == NULL) || (md->name == "Close"))
      continue;
    String cur_nm = md->OptionAfter("MENU_ON_");
    if(cur_nm != "")
      men_nm = cur_nm;
    // has to be on one of these two menus..
    if((men_nm != "Object") && (men_nm != "Edit"))
      continue;
    if((men_nm == "Object") && (md->name != "Edit"))
      continue;
    if((md->name == "DuplicateEl") || (md->name == "Transfer"))
      continue;
    lst_men_nm = men_nm;
    taivMethMenu* mth_rep = md->iv->GetMethodRep(cur_base, dialog);
    if(mth_rep == NULL)
      continue;
    meth_el.Add(mth_rep);
    mth_rep->AddToMenu(rep);
  }
}


//////////////////////////////////
//  	ArrayEditButton		//
//////////////////////////////////


gpivArrayEditButton::gpivArrayEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: taivEditButton(tp, base, win, NULL, dlg, par)
{
}

void gpivArrayEditButton::SetLabel() {
  taArray_base* gp = (taArray_base*)cur_base;
  if(gp == NULL) {
    taivEditButton::SetLabel();
    return;
  }
  String nm = " Size: " + String(gp->size);
  TypeDef* td = gp->GetTypeDef()->GetTemplInstType();
  if((td != NULL) && (td->templ_pars.size > 0))
    nm += String(" (") + td->templ_pars.FastEl(0)->name + ")";
  rep->SetMLabel(nm);
}

//////////////////////////
//     Element Menus 	//
//////////////////////////

//////////////////////////
// 	gpivListEls	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(gpivListEls)
implementActionCallback(gpivListEls)
#include <ta/leave_iv.h>

gpivListEls::gpivListEls
(int rt, int ft, TABLPtr lst, taivDialog* dlg, taivData* par, bool nul, bool nlst, bool edt_ok)
: taivData(dlg, par) {
  ths = lst;
  null_ok = nul;
  no_lst = nlst;
  edit_ok = edt_ok;
  ta_menu = new taivHierMenu(rt, taivMenu::radio_update, ft, (void*)ths, dlg, this);
  ownflag = true;
  over_max = false;
  chs_obj = NULL;
}
gpivListEls::gpivListEls
(taivHierMenu* existing_menu, TABLPtr lst, taivDialog* dlg, taivData* par, bool nul, bool nlst,
 bool edt_ok)
: taivData(dlg, par) {
  ths = lst;
  null_ok = nul;
  no_lst = nlst;
  edit_ok = edt_ok;
  ta_menu = existing_menu;
  ownflag = false;
  over_max = false;
  chs_obj = NULL;
}
gpivListEls::~gpivListEls() {
  if(ownflag)
    delete ta_menu;
  ta_menu = NULL;
}

ivGlyph* gpivListEls::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* gpivListEls::GetLook() {
  return ta_menu->GetLook();
}

void gpivListEls::GetImage(TABLPtr base_lst, TAPtr it) {
  if(ths != base_lst) {
    ths = base_lst;
  }
  UpdateMenu();
  ta_menu->GetImage((void*)it);
  chs_obj = it;
}
TAPtr gpivListEls::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(over_max) {
    if((cur == NULL) || (cur->label == "<Over max, select>")
       || (cur->label == "gp.<Over max, select>"))
      return chs_obj;
    else
      return (TAPtr)cur->usr_data;
  }
  if(cur != NULL)
    return (TAPtr)cur->usr_data;
  return NULL;
}

void gpivListEls::Edit() {
  TAPtr cur_base = GetValue();
  if(cur_base != NULL) {
    if(dialog != NULL)
      cur_base->Edit(dialog->waiting);
    else
      cur_base->Edit();
  }
}

void gpivListEls::Choose() {
  TABLPtr chs_root = ths;
  // this is not a good idea: it makes the prior selection the root, so you can't actuall
  // choose anything!
//   if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "<Over max, select>") &&
//      (ta_menu->cur_sel->usr_data != NULL))
//     chs_root = (TABLPtr)ta_menu->cur_sel->usr_data; 
    
  taivObjChooser* chs = new taivObjChooser(chs_root, taivM->wkit, taivM->wkit->style(),
					   "List/Group Objects", true);
  if((chs_obj != NULL) && !chs_obj->GetName().empty())
    chs->sel_str = chs_obj->GetName();
  bool rval = chs->Choose();
  if(rval) {
    chs_obj = chs->sel_obj;
    if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "<Over max, select>") &&
       (ta_menu->cur_sel->men_act != NULL)) {
      ta_menu->cur_sel->usr_data = (void*)chs_obj;
      ta_menu->cur_sel->men_act->Select(ta_menu->cur_sel); // call it!
    }
    else
      ta_menu->SetMLabel(chs->sel_str);
  }
  delete chs;
}

void gpivListEls::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  if(null_ok)
    ta_menu->AddItem("NULL", NULL, taivMenu::use_default, actn);
  if(edit_ok)
    ta_menu->AddItem("Edit", NULL, taivMenu::normal,
		     new ActionCallback(gpivListEls)(this, &gpivListEls::Edit));
  if(!no_lst) {
    String nm;
    if(ths->GetName() == "")
      nm = "All";
    else
      nm = ths->GetName();
    ta_menu->AddItem(nm, (void*)ths, taivMenu::use_default, actn);
  }
  if(null_ok || edit_ok || !no_lst)
    ta_menu->AddSep();
  GetMenu_impl(ths, actn);
  ivResource::unref(actn);
}

void gpivListEls::UpdateMenu(taivMenuAction* actn) {
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void gpivListEls::GetMenu_impl(TABLPtr cur_lst, taivMenuAction* actn) {
  if(cur_lst == NULL)	return;
  if(cur_lst->size >= taMisc::max_menu) {
    taivMenuEl* mnel = ta_menu->AddItem
      ("<Over max, select>", cur_lst, taivMenu::normal,
       new ActionCallback(gpivListEls)(this, &gpivListEls::Choose));
    over_max = true;
    if(actn != NULL) {		// also set callback action!
      mnel->men_act = actn;
      ivResource::ref(mnel->men_act);
    }
    return;
  }
  int i;
  for(i=0; i<cur_lst->size; i++) {
    TAPtr tmp = (TAPtr)cur_lst->FastEl_(i);
    if(tmp == NULL)	continue;
    String nm = tmp->GetName();
    if(nm == "")
      nm = String("[") + String(i) + "]: (" + tmp->GetTypeDef()->name + ")"; 
    ta_menu->AddItem((char*)nm, (void*)tmp, taivMenu::use_default, actn);
  }
}


//////////////////////////
// 	gpivGroupEls	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(gpivGroupEls)
implementActionCallback(gpivGroupEls)
#include <ta/leave_iv.h>

gpivGroupEls::gpivGroupEls
(int rt, int ft, TABLPtr lst, taivDialog* dlg, taivData* par, bool nul, bool nlst, bool ngpm,
 bool edt_ok)
: gpivListEls(rt,ft,lst,dlg,par,nul,nlst, edt_ok)
{
  no_in_gpmenu = ngpm;
}

gpivGroupEls::gpivGroupEls
(taivHierMenu* existing_menu, TABLPtr lst, taivDialog* dlg, taivData* par, bool nul, bool nlst, bool ngpm,
 bool edt_ok)
: gpivListEls(existing_menu,lst,dlg,par,nul,nlst, edt_ok)
{
  no_in_gpmenu = ngpm;
}

void gpivGroupEls::ChooseGp() {
  TABLPtr chs_root = &(((TAGPtr)ths)->gp);
  // this is not a good idea: it makes the prior selection the root, so you can't actuall
  // choose anything!
//   if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "gp.<Over max, select>") &&
//      (ta_menu->cur_sel->usr_data != NULL))
//     chs_root = (TABLPtr)ta_menu->cur_sel->usr_data; 
    
  taivObjChooser* chs = new taivObjChooser(chs_root, taivM->wkit, taivM->wkit->style(),
					   "SubGroups", true);
  bool rval = chs->Choose();
  if(rval) {
    chs_obj = chs->sel_obj;
    if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "gp.<Over max, select>") &&
       (ta_menu->cur_sel->men_act != NULL)) {
      ta_menu->cur_sel->usr_data = (void*)chs_obj;
      ta_menu->cur_sel->men_act->Select(ta_menu->cur_sel); // call it!
    }
    else
      ta_menu->SetMLabel(chs->sel_str);
  }
  delete chs;
}

void gpivGroupEls::GetMenu_impl(TABLPtr cur_lst, taivMenuAction* actn) {
  if(cur_lst == NULL) return;
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  if(cur_gp->size >= taMisc::max_menu) {
    taivMenuEl* mnel = ta_menu->AddItem
      ("<Over max, select>", cur_gp, taivMenu::normal,
       new ActionCallback(gpivListEls)(this, &gpivListEls::Choose));
    over_max = true;
    if(actn != NULL) {		// also set callback action!
      mnel->men_act = actn;
      ivResource::ref(mnel->men_act);
    }
  }
  else {
    int i;
    for(i=0; i<cur_gp->size; i++) {
      TAPtr tmp = (TAPtr)cur_gp->FastEl_(i);
      if(tmp == NULL)	continue;
      String nm = tmp->GetName();
      if(nm.empty())
	nm = String("[") + String(i) + "]: (" + tmp->GetTypeDef()->name + ")"; 

      bool added_sub = false;	// if a IN_GPMENU sub was added for this item
      int prev_subno = 0;		// the number of that subno, if added
      int cur_no = 0;
      if(!no_in_gpmenu && tmp->HasOption("MEMB_IN_GPMENU") && 
	 (tmp->GetOwner() == cur_gp)) // only get for owned items
      {
	TypeDef* ttd = tmp->GetTypeDef();
	int fm;
	for(fm=0; fm<ttd->members.size; fm++) {
	  MemberDef* md = ttd->members[fm];
	  if(md->HasOption("IN_GPMENU")) {
	    TAGPtr tmp_grp = (TAGPtr)md->GetOff(tmp);
	    if(tmp_grp->leaves == 0)
	      continue;
	    if(!added_sub) {
	      cur_no = ta_menu->cur_sno;
	      ta_menu->AddSubMenu(nm, (void*)tmp_grp);
	      prev_subno = ta_menu->cur_sno;
	      ta_menu->AddItem(nm, (void*)tmp, taivMenu::use_default, actn);
	    }
	    else {
	      ta_menu->SetSub(prev_subno);
	      ta_menu->AddSep();
	    }
	    String subnm = String("::") + md->name;
	    ta_menu->AddItem(subnm, (void*)tmp_grp, taivMenu::use_default, actn);
	    ta_menu->AddSep();
	    GetMenu_impl(tmp_grp, actn);
	    ta_menu->SetSub(cur_no);
	    added_sub = true;
	  }
	}
      }
      if(!added_sub) {
	ta_menu->AddItem((char*)nm, (void*)tmp, taivMenu::use_default, actn);
      }
    }
  }
  if(cur_gp->gp.size >= taMisc::max_menu) {
    taivMenuEl* mnel = ta_menu->AddItem
      ("gp.<Over max, select>", &(cur_gp->gp), taivMenu::normal,
       new ActionCallback(gpivGroupEls)(this, &gpivGroupEls::ChooseGp));
    over_max = true;
    if(actn != NULL) {		// also set callback action!
      mnel->men_act = actn;
      ivResource::ref(mnel->men_act);
    }
  }
  else {
    int i;
    for(i=0; i<cur_gp->gp.size; i++) {
      TAGPtr tmp_grp = (TAGPtr)cur_gp->FastGp_(i);
      String nm = tmp_grp->GetName();
      if(nm == "")
	nm = "Group [" + String(i) + "]";
      int cur_no = ta_menu->cur_sno;
      ta_menu->AddSubMenu(nm, (void*)tmp_grp);
      if(!no_lst) {
	String subnm = nm + ": All";
	ta_menu->AddItem(subnm, (void*)tmp_grp, taivMenu::use_default, actn);
	ta_menu->AddSep();
      }
      GetMenu_impl(tmp_grp, actn);
      ta_menu->SetSub(cur_no);
    }
  }
}


//////////////////////////
// 	gpivSubGroups	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(gpivSubGroups)
implementActionCallback(gpivSubGroups)
#include <ta/leave_iv.h>


gpivSubGroups::gpivSubGroups(int rt, int ft, TAGPtr gp, taivDialog* dlg, taivData* par,
			       bool nul_ok, bool edt_ok)
: taivData(dlg, par) {
  ths = gp;
  ta_menu = new taivHierMenu(rt, taivMenu::radio_update, ft, (void*)ths, dlg, this);
  ownflag = true;
  null_ok = nul_ok;
  edit_ok = edt_ok;
  over_max = false;
  chs_obj = NULL;
}
gpivSubGroups::gpivSubGroups
(taivHierMenu* existing_menu, TAGPtr gp, taivDialog* dlg, taivData* par, bool nul_ok, bool edt_ok)
: taivData(dlg, par) {
  ths = gp;
  ta_menu = existing_menu;
  ownflag = false;
  null_ok = nul_ok;
  edit_ok = edt_ok;
  over_max = false;
  chs_obj = NULL;
}

gpivSubGroups::~gpivSubGroups() {
  if(ownflag) delete ta_menu;
}

ivGlyph* gpivSubGroups::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* gpivSubGroups::GetLook() {
  return ta_menu->GetLook();
}

void gpivSubGroups::GetImage(TAGPtr base_gp, TAGPtr gp) {
  if(ths != base_gp) {
    ths = base_gp;
    GetMenu();
  }
  ta_menu->GetImage((void*)gp);
  chs_obj = gp;
}

TAGPtr gpivSubGroups::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(over_max) {
    if((cur == NULL) || (cur->label == "<Over max, select>"))
      return chs_obj;
    else
      return (TAGPtr)cur->usr_data;
  }
  if(cur != NULL)
    return (TAGPtr)cur->usr_data;
  return NULL;
}
  
void gpivSubGroups::Edit() {
  TAGPtr cur_base = GetValue();
  if(cur_base != NULL) {
    if(dialog != NULL)
      cur_base->Edit(dialog->waiting);
    else
      cur_base->Edit();
  }
}

void gpivSubGroups::Choose() {
  TABLPtr chs_root = ths;
  // this is not a good idea: it makes the prior selection the root, so you can't actuall
  // choose anything!
//   if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "<Over max, select>") &&
//      (ta_menu->cur_sel->usr_data != NULL))
//     chs_root = (TABLPtr)ta_menu->cur_sel->usr_data; 
    
  taivObjChooser* chs = new taivObjChooser(chs_root, taivM->wkit, taivM->wkit->style(),
					   "SubGroups", true);
  bool rval = chs->Choose();
  if(rval) {
    chs_obj = (TAGPtr)chs->sel_obj;
    if((ta_menu->cur_sel != NULL) && (ta_menu->cur_sel->label == "<Over max, select>") &&
       (ta_menu->cur_sel->men_act != NULL)) {
      ta_menu->cur_sel->men_act->Select(ta_menu->cur_sel); // call it!
    }
    else
      ta_menu->SetMLabel(chs->sel_str);
  }
  delete chs;
}

void gpivSubGroups::GetMenu(taivMenuAction* actn) {
  if(null_ok)
    ta_menu->AddItem("NULL", NULL, taivMenu::use_default, actn);
  if(edit_ok)
    ta_menu->AddItem("Edit", NULL, taivMenu::normal,
		     new ActionCallback(gpivSubGroups)(this, &gpivSubGroups::Edit));
  if(ths == NULL)	return;
  String nm;
  if(ths->owner == NULL)
    nm = ths->GetTypeDef()->name;
  else if(ths->owner->GetName() == "")
    nm = ths->owner->GetTypeDef()->name;
  else
    nm = ths->owner->GetName();
  ta_menu->AddItem(nm, (void*)ths, taivMenu::use_default, actn);
  ta_menu->AddSep();
  GetMenu_impl(ths, actn);
}

void gpivSubGroups::UpdateMenu(taivMenuAction* actn) {
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void gpivSubGroups::GetMenu_impl(TAGPtr gp, taivMenuAction* actn) {
  if(gp == NULL) return;
  if(gp->gp.size >= taMisc::max_menu) {
    taivMenuEl* mnel = ta_menu->AddItem
      ("<Over max, select>", gp, taivMenu::normal,
       new ActionCallback(gpivSubGroups)(this, &gpivSubGroups::Choose));
    over_max = true;
    if(actn != NULL) {		// also set callback action!
      mnel->men_act = actn;
      ivResource::ref(mnel->men_act);
    }
    return;
  }
  int i;
  for(i=0; i<gp->size; i++) {
    TAPtr tmp = (TAPtr)gp->FastEl_(i);
    if(tmp == NULL)	continue;
    String nm = tmp->GetName();
    if(nm.empty())
      nm = String("[") + String(i) + "]: (" + tmp->GetTypeDef()->name + ")"; 
    bool added_sub = false;	// if a IN_GPMENU sub was added for this item
    int prev_subno = 0;		// the number of that subno, if added
    int cur_no = 0;
    if(tmp->HasOption("MEMB_IN_GPMENU") && (tmp->GetOwner() == gp))  {
      TypeDef* ttd = tmp->GetTypeDef();
      int fm;
      for(fm=0; fm<ttd->members.size; fm++) {
	MemberDef* md = ttd->members[fm];
	if(md->HasOption("IN_GPMENU")) {
	  TAGPtr tmp_grp = (TAGPtr)md->GetOff(tmp);
	  if(tmp_grp->leaves == 0)
	    continue;
	  if(!added_sub) {
	    cur_no = ta_menu->cur_sno;
	    ta_menu->AddSubMenu(nm, (void*)tmp_grp);
	    prev_subno = ta_menu->cur_sno;
	    ta_menu->AddItem(nm, (void*)tmp, taivMenu::use_default, actn);
	  }
	  else {
	    ta_menu->SetSub(prev_subno);
	  }
	  String subnm = String("::") + md->name;
	  ta_menu->AddItem(subnm, (void*)tmp_grp, taivMenu::use_default, actn);
	  GetMenu_impl(tmp_grp, actn);
	  ta_menu->SetSub(cur_no);
	  added_sub = true;
	}
      }
    }
  }
  for(i=0; i<gp->gp.size; i++) {
    TAGPtr tmp_grp = (TAGPtr)gp->FastGp_(i);
    String nm;
    if(tmp_grp->GetName() == "")
      nm = "Group [" + String(i) + "]";
    else
      nm = tmp_grp->GetName();
    bool has_sub_stuff = false;
    if(tmp_grp->gp.size > 0)
      has_sub_stuff = true;
    if(!has_sub_stuff) {
      int j;
      for(j=0; j<tmp_grp->size; j++) {
	TAPtr tmp = (TAPtr)tmp_grp->FastEl_(j);
	if(tmp == NULL)	continue;
	if(tmp->HasOption("MEMB_IN_GPMENU") && (tmp->GetOwner() == tmp_grp))  {
	  TypeDef* ttd = tmp->GetTypeDef();
	  int fm;
	  for(fm=0; fm<ttd->members.size; fm++) {
	    MemberDef* md = ttd->members[fm];
	    if(md->HasOption("IN_GPMENU")) {
	      TAGPtr sbgrp = (TAGPtr)md->GetOff(tmp);
	      if(sbgrp->leaves == 0)
		continue;
	      has_sub_stuff = true;
	      break;
	    }
	  }
	  if(has_sub_stuff)
	    break;
	}
      }
    }
    if(has_sub_stuff) {
      int cur_no = ta_menu->cur_sno;
      ta_menu->AddSubMenu(nm, (void*)tmp_grp);
      String subnm = nm + ": Group";
      ta_menu->AddItem(subnm, (void*)tmp_grp, taivMenu::use_default, actn);
      ta_menu->AddSep();
      GetMenu_impl(tmp_grp, actn);
      ta_menu->SetSub(cur_no);
    }
    else
      ta_menu->AddItem((char*)nm, (void*)tmp_grp, taivMenu::use_default,actn);
  }
}


//////////////////////////
// 	gpivElTypes	//
//////////////////////////

gpivElTypes::gpivElTypes
(int rt, int ft, TypeDef* td, TypeDef* lstd, taivDialog* dlg, taivData* par)
: taivTypeHier(rt, ft, td, dlg, par)
{
  lst_typd = lstd;
}

gpivElTypes::gpivElTypes
(taivHierMenu* existing_menu, TypeDef* td, TypeDef* gtd, taivDialog* dlg, taivData* par)
: taivTypeHier(existing_menu, td, dlg, par) 
{
  lst_typd = gtd;
}
void gpivElTypes::GetMenu(taivMenuAction* nact) {
  ivResource::ref(nact);
  int cur_no = ta_menu->cur_sno;
  GetMenu_impl(typd, nact);
  ta_menu->SetSub(cur_no);
  if(ta_menu->GetSubEl(ta_menu->cur_sno, ta_menu->cur_item)->label != "-")
    ta_menu->AddSep();
  GetMenu_impl(lst_typd, nact);	// get group types for this type
  ivResource::unref(nact);
}


//////////////////////////
//    gpivNewFuns	//
//////////////////////////

gpivNewFuns::gpivNewFuns(TypeDef* td, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  typ = td;
  ivLayoutKit* layout = ivLayoutKit::instance();
  rep = layout->vbox();
  ivResource::ref(rep);

  int i;
  for(i=0;i<typ->methods.size;i++){
    MethodDef* md = typ->methods.FastEl(i);
    if(md->HasOption("NEW_FUN")) {
      taivMethToggle* mt = new taivMethToggle(NULL, md, typ);
      funs.Add(mt);
      rep->append(mt->GetLook());
    }
  }
}

gpivNewFuns::~gpivNewFuns() {
  ivResource::unref(rep);
}

ivGlyph* gpivNewFuns::GetLook() {
  return rep;
}

void gpivNewFuns::CallFuns(void* base) {
  int i;
  for(i=0;i<funs.size;i++) {
    taivMethToggle* mt = (taivMethToggle*)funs.FastEl(i);
    mt->base = base;
    mt->CallFun();
  }
}


//////////////////////////////////
// 	gpivListNew		//
//////////////////////////////////

gpivListNew::gpivListNew(TABLPtr lst, TypeDef* td, int n_els) {
  ths = lst;
  num = n_els;
  if(td == NULL)
    typ = ths->el_typ;
  else
    typ = td;
  num_rep = NULL;
  typ_rep = NULL;
  fun_list  = NULL;
}
gpivListNew::~gpivListNew() {
  delete num_rep;
  delete typ_rep;
  delete fun_list;
}
void gpivListNew::Constr_Box() {
  num_rep = new taivIncrField(String(num), this, dialog, NULL, true);
  typ_rep = new gpivElTypes(taivMenu::menubar, taivMenu::small,
			       ths->el_base, ths->GetTypeDef(), this);
  typ_rep->GetMenu();
  typ_rep->GetImage(typ);

  fun_list = new gpivNewFuns(ths->el_base, this);

  sub_box = layout->hbox
    (taivM->top_spc
     (layout->vbox(wkit->label("Number:"),
		   taivM->vspc,
		   num_rep->GetLook())),
     taivM->hspc,
     taivM->top_spc(layout->vbox(wkit->label("Of Type:"),
				  taivM->vspc, typ_rep->GetLook())));

  Constr_SubGpList();
  box = layout->vbox(sub_box, taivM->vspc, fun_list->GetRep());
}

void gpivListNew::Constr_Strings(const char*, const char* win_title) {
  taivDialog::Constr_Strings("New: Create new object(s)", win_title);
}

void gpivListNew::GetValue() {
  num = atoi(num_rep->GetValue());
  typ = typ_rep->GetValue();
}

TAPtr gpivListNew::New(TABLPtr the_lst, TypeDef* td, ivWindow* win,
			int n_els)
{
  gpivListNew* create = new gpivListNew(the_lst, td, n_els);
  create->Constr(win, true);
  bool ok_can = create->Edit();
  if(!ok_can || (create->num <= 0)) {
    delete create;
    return NULL;
  }

  TAPtr rval = NULL;
  DMEM_GUI_RUN_IF {
    rval = the_lst->New(create->num, create->typ);
  }
  if(taivMisc::record_script != NULL) {
    *taivMisc::record_script << the_lst->GetPath() << "->New(" << create->num
			     << ", " << create->typ->name << ");" << endl;
  }
  DMEM_GUI_RUN_IF {
    int i;
    for(i = (int) (the_lst->size - create->num); i < the_lst->size; i++) {
      create->fun_list->CallFuns(the_lst->FastEl_(i));
    }
    bool auto_edit = taMisc::auto_edit;
    if(create->mouse_button == ivEvent::middle) // reverse of default
      auto_edit = !auto_edit;
    if(create->mouse_button==ivEvent::right) // always edit on rt
      auto_edit = true;
    if(auto_edit) {
      if(create->num == 1) {	// edit-after-create
	rval->Edit(false);
      }
      else {			// edit the group
	the_lst->Edit(false);
      }
    }
  }
  delete create;
  return rval;
}


//////////////////////////////////
// 	taivGroupNew		//
//////////////////////////////////

gpivGroupNew::gpivGroupNew(TAGPtr gp, TAGPtr init_gp, TypeDef* td, int n_els)
: gpivListNew(gp, td, n_els)
{
  in_gp = gp;
  if(init_gp != NULL)
    in_gp = init_gp;
  subgp_list = NULL;
}

gpivGroupNew::~gpivGroupNew() {
  if(subgp_list != NULL)
    delete subgp_list;
}

void gpivGroupNew::Constr_SubGpList() {
  // could have some members with groups to create things in..
  if((in_gp->gp.size == 0) && !(in_gp->el_typ->HasOption("MEMB_IN_GPMENU") &&
				(in_gp->leaves > 0)))
    return;

  subgp_list = new gpivSubGroups(taivMenu::menubar, taivMenu::small, (TAGPtr)ths,
				  this);
  subgp_list->GetMenu();
  subgp_list->GetImage((TAGPtr)ths, in_gp);
  
  sub_box->append(taivM->hspc);
  sub_box->append
    (taivM->top_spc(layout->vbox(wkit->label("In:"),
				  taivM->vspc, subgp_list->GetLook())));
}

void gpivGroupNew::GetValue() {
  if(subgp_list != NULL)
    in_gp = subgp_list->GetValue();
  gpivListNew::GetValue();
}

TAPtr gpivGroupNew::New(TAGPtr the_gp, TypeDef* td, TAGPtr init_gp, ivWindow* win,
			 int n_els)
{
  gpivGroupNew* create = new gpivGroupNew(the_gp, init_gp, td, n_els);
  create->Constr(win, true);
  bool ok_can = create->Edit();
  if(!ok_can || (create->num <= 0)) {
    delete create;
    return NULL;
  }

  TAPtr rval;
  DMEM_GUI_RUN_IF {
    rval = create->in_gp->New(create->num, create->typ);
  }
  if(taivMisc::record_script != NULL) {
    *taivMisc::record_script << create->in_gp->GetPath() << "->New(" << create->num
			     << ", " << create->typ->name << ");" << endl;
  }
  DMEM_GUI_RUN_IF {
    if(!create->typ->InheritsFrom(TA_taGroup_impl)) {
      int i;
      for(i = (int) (create->in_gp->size - create->num); i < create->in_gp->size; i++){
	create->fun_list->CallFuns(create->in_gp->FastEl_(i));
      }
    }
    bool auto_edit = taMisc::auto_edit;
    if(create->mouse_button==ivEvent::middle) // reverse of default
      auto_edit = !auto_edit;
    if(create->mouse_button==ivEvent::right) // always edit
      auto_edit = true;
    if(auto_edit) {
      if(create->num == 1) {	// edit-after-create
	if(!create->typ->InheritsFrom(TA_taGroup_impl))
	  rval->Edit(false);
      }
      else {			// edit the group
	create->in_gp->Edit(false);
      }
    }
  }
  delete create;
  return rval;
}


//////////////////////////////////
// 	gpivListEdit		//
//////////////////////////////////

class ListEditNBDialog : public EditNBDialog {
public:
  ListEditNBDialog(taivEditDialog* ted, ivGlyph* g, ivStyle* s)
  : EditNBDialog(ted,g,s) {};
  bool special_key(const ivEvent& e);
};

bool ListEditNBDialog::special_key(const ivEvent& e){
  if(owner == NULL) return false;
  if(e.type() != ivEvent::key) return false;
#ifndef CYGWIN
  if(e.len != 0) return false;
#endif
  char c;
  e.mapkey(&c,1);
  // META-B, META-P, META-V, Page_Up (prior)
#ifndef CYGWIN
  if((c == '\342') || (c == '\360') || (c == '\366') || (e.keysym() == XK_Prior))
#else
  if((c == '\342') || (c == '\360') || (c == '\366'))
#endif
    { 
      int j = (int) ((input_handler_count()-((gpivListDialog*)owner)->num_lst_fields)/
		     ((gpivListDialog*)owner)->cur_lst->size);
      int i;
      for(i=0;i<j;i++) prev_focus();
      return true;
    }
  // META-F, META-N, CTRL-V, Page_Down (next)
#ifndef CYGWIN
  if((c == '\346') || (c == '\356') || (c == '\026') || (e.keysym() == XK_Next))
#else
  if((c == '\346') || (c == '\356') || (c == '\026'))
#endif
    {
      int j = (int) ( (input_handler_count()-((gpivListDialog*)owner)->num_lst_fields)
		      /  ((gpivListDialog*)owner)->cur_lst->size);
      int i;
      for(i=0;i<j;i++) next_focus();
      return true;
    }

  return EditNBDialog::special_key(e);
}

gpivList_ElData::gpivList_ElData(TypeDef* tp, TAPtr base) {
  typ = tp; cur_base = base;
}

gpivList_ElData::~gpivList_ElData() {
  data_el.Reset();
}


gpivListDialog::gpivListDialog(TypeDef* tp, void* base) : taivEditDialog(tp,base) {
  cur_lst = (TAGPtr)cur_base;
  num_lst_fields = 0;
  lstel_rep = NULL;
  lst_data_g = NULL;
  lst_data_patch = NULL;
}

gpivListDialog::~gpivListDialog() {
  CloseWindow();
}

void gpivListDialog::CloseWindow() {
  lst_data_el.Reset();
  lst_membs.Reset();
  num_lst_fields = 0;
  lstel_rep = NULL;
  lst_data_g = NULL;
  lst_data_patch = NULL;
  taivEditDialog::CloseWindow();
  ivResource::flush();
}  

void gpivListDialog::Constr_Dialog() {
  dialog = new ListEditNBDialog(this, taivM->hspc, style);
  ivResource::ref(dialog);
}

void gpivListDialog::Constr_Strings(const char*, const char* win_title) {
  prompt_str = cur_lst->GetTypeDef()->name + ": ";
  if(cur_lst->GetTypeDef()->desc == TA_taBase_List.desc) {
    prompt_str += cur_lst->el_typ->name + "s: " + cur_lst->el_typ->desc;
  }
  else {
    prompt_str += cur_lst->GetTypeDef()->desc;
  }
  win_str = String(ivSession::instance()->classname()) + ": " +
    win_title + " " + cur_lst->GetPath();
}

// don't check for null iv ptr here
bool gpivListDialog::ShowMember(MemberDef* md) {
  return md->ShowMember(show);
}

void gpivListDialog::Constr_ListMembs() {
  bool has_labels = false;
  if(lst_membs.size > 0)
    has_labels = true;
  int lf;
  for(lf = 0; lf < cur_lst->size; lf++) {
    TAPtr tmp_lf = (TAPtr)cur_lst->FastEl_(lf);
    if(tmp_lf == NULL)	continue;
    TypeDef* tmp_td = tmp_lf->GetTypeDef();
    lst_data_el.Add(new gpivList_ElData(tmp_td, tmp_lf));
    if(has_labels)
      continue;
    int i;
    for(i=0; i<tmp_td->members.size; i++) {
      MemberDef* md = tmp_td->members.FastEl(i);
      if(ShowMember(md) && !(lst_membs.FindName(md->name))) {
	MemberDef* nmd = md->Clone();
	lst_membs.Add(nmd);	// index now reflects position in list...
      }
    }
  }
}

void gpivListDialog::Constr_Labels() {
  Constr_Labels_style();
  Constr_ListMembs();
  GetVFix(taivM->name_font);
  GetHFix(taivM->name_font, lst_membs);
  hfix = MAX(hfix, 15);
  labels = layout->vbox(layout->vfixed(taivM->hfsep, vfix));
  Constr_Labels_impl(lst_membs);
  wkit->pop_style();
}

void gpivListDialog::Scroll(){
  if(dialog->last_focus == -1)
    lst_data_g->scroll_backward(Dimension_X);
  else if(dialog->last_focus == 1)
    lst_data_g->scroll_forward(Dimension_X);
}

void gpivListDialog::Constr_Data() {
  lst_data_g = new lrScrollBox();
  lst_data_g->naturalnum = 3;
  lst_data_patch = new ivPatch(lst_data_g);
  lstel_rep = layout->vbox();
// todo: figure out how to get this to resize properly..
//       layout->hcenter(layout->vbox
// 		      (layout->hbox(layout->vcenter(labels,0), taivM->hfspc,
// 				    taivM->hfspc, layout->vcenter(lst_data_patch,0)),
// 		       wkit->hscroll_bar(lst_data_g))
// 		      ,0)), style);
  Constr_ListData();
  Constr_ElData();
  GetImage();
}

void gpivListDialog::Constr_ListData() {
  ivPolyGlyph* lstel_hb=NULL;
  int cnt = 0;
  int lst_count = (int)lst_data_el.size;
  lst_count = MAX(lst_count, 1); // no less than 1
  lst_count = MIN(lst_count, 3); // no more than 3
  Constr_Labels_style();
  int i;
  for(i=0; i<typ->members.size; i++) {
    MemberDef* md = typ->members.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat=md->iv->GetDataRep(dialog, this);
    data_el.Add(mb_dat);
    if((cnt % lst_count) == 0) {
      if(cnt != 0) {
	lstel_rep->append(layout->hcenter(lstel_hb,0));
      }
      lstel_hb = layout->hbox();
    }
    ivGlyph* lbl_rep = layout->fixed(taivM->top_sep(GetNameRep(md)), hfix+32, vfix);
    lstel_hb->append
      (layout->vcenter
       (layout->hbox
	(layout->vcenter(lbl_rep,0),
	 taivM->hfsep,
	 layout->vcenter(mb_dat->GetLook(),0),
	 taivM->hfsep),0));
    cnt++;
  }
  wkit->pop_style();
  lstel_rep->append(layout->hcenter(lstel_hb,0));
  num_lst_fields = (int)dialog->input_handler_count();
}

void gpivListDialog::Constr_ElData() {  
  int lf;
  for(lf=0; lf<lst_data_el.size; lf++) {
    gpivList_ElData* lf_el = lst_data_el.FastEl(lf);
    String nm = String("[") + String(lf) + "]: (" + lf_el->typ->name + ")";
    ivGlyph* el_data_g = layout->vbox(layout->vfixed(taivM->top_sep
						   (wkit->fancy_label(nm)),vfix));
    int cnt = 0;
    int i;
    for(i=0; i<lf_el->typ->members.size; i++) {
      MemberDef* md = lf_el->typ->members.FastEl(i);
      if(!ShowMember(md))
	continue;
      MemberDef* lst_md = lst_membs.FindName(md->name);
      if(lst_md == NULL)
	continue;
      int idx;
      for(idx=cnt; idx < lst_md->idx; idx++) {	// align with other elements in List
	el_data_g->append(layout->vfixed(taivM->hfsep, vfix));
	cnt++;
      }
      taivData* mb_dat = md->iv->GetDataRep(dialog, this);
      lf_el->data_el.Add(mb_dat);
      el_data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()),vfix));
      cnt++;
    }
    ivGlyph* el_data_look = wkit->inset_frame
      (layout->vflexible
       (new ivBackground
	(el_data_g, bg_color_dark)));
    // add to the List
    lst_data_g->append(layout->vcenter(el_data_look));
  }
}

void gpivListDialog::Constr_Box() {
  Constr_Labels();
  Constr_Data();
  Constr_Methods();
  box = layout->flexible
    (layout->vbox
     (layout->hcenter(lstel_rep,0),
      taivM->vfsep,
      layout->hcenter(wkit->menu_item_separator_look(),0),
      taivM->vfsep,
      layout->hcenter(layout->hbox(labels, taivM->hfsep, taivM->hfsep,
				   layout->vbox
				   (lst_data_patch,wkit->hscroll_bar(lst_data_g)))
		      ,0)));
  // under some circumstances these are not used, so we ref them
  ivResource::ref(labels);
}

void gpivListDialog::Constr_BodyBox() {
  body_box = layout->vbox(layout->hcenter(prompt,0),
			  taivM->vfsep,
			  layout->hcenter(box,0),
			  taivM->vfspc);
}

void gpivListDialog::GetValue() {
  bool rebuild = false;
  if(lst_data_el.size != cur_lst->size) rebuild = true;
  if(!rebuild) {		// check that same elements are present!
    int lf;
    for(lf=0; lf<lst_data_el.size; lf++) {
      if(lst_data_el.FastEl(lf)->cur_base != (TAPtr)cur_lst->FastEl_(lf)) {
	rebuild = true;
	break;
      }
    }
  }
  if(rebuild) {
    taMisc::Error("Cannot apply changes: List size or elements have changed");
    return;
  }

  // first for the List-structure members
  GetValue_impl(typ->members, data_el, cur_base);
  // then the List elements
  int lf;
  for(lf=0; lf<lst_data_el.size; lf++) {
    gpivList_ElData* lf_el = lst_data_el.FastEl(lf);
    GetValue_impl(lf_el->typ->members, lf_el->data_el, lf_el->cur_base);
    ((TAPtr)lf_el->cur_base)->UpdateAfterEdit();
  }
  cur_lst->UpdateAfterEdit();	// call here too!
  taivMisc::Update((TAPtr)cur_lst);
  GetButtonImage();
  Unchanged();
}

void gpivListDialog::GetImage() {
  bool rebuild = false;
  if(lst_data_el.size != cur_lst->size) rebuild = true;
  if(!rebuild) {		// check that same elements are present!
    int lf;
    for(lf=0; lf<lst_data_el.size; lf++) {
      if(lst_data_el.FastEl(lf)->cur_base != (TAPtr)cur_lst->FastEl_(lf)) {
	rebuild = true;
	break;
      }
    }
  }

  if(rebuild) {
    if(cur_lst->size == 0) {
      taMisc::Error("List has zero elements: canceling edit");
      Cancel();
      return;
    }
    lst_data_el.Reset();
    ivGlyphIndex i;
    for(i=lst_data_g->count()-1; i >= 0; i--)
      lst_data_g->remove(i);
    for(i=labels->count()-1; i >= 1; i--)
      labels->remove(i);
    lst_membs.Reset();
    Constr_ListMembs();
    Constr_Labels_impl(lst_membs);
    Constr_ElData();
  }

  // first for the List-structure members
  GetImage_impl(typ->members, data_el, cur_base, cur_win);

  // then the elements
  int lf;
  for(lf=0; lf<lst_data_el.size; lf++) {
    gpivList_ElData* lf_el = lst_data_el.FastEl(lf);
    GetImage_impl(lf_el->typ->members, lf_el->data_el, lf_el->cur_base, cur_win);
  }
  lst_data_patch->reallocate();
  GetButtonImage();
  Unchanged();
  FocusOnFirst();
}

int gpivListDialog::Edit() {
  if((cur_lst != NULL) && (cur_lst->size > 100)) {
    int rval = taMisc::Choice("List contains more than 100 items (size = " +
			      String(cur_lst->size) + "), continue with Edit?",
			      "Ok", "Cancel");
    if(rval == 1) return 0;
  }
  return taivEditDialog::Edit();
}


//////////////////////////////////
// 	taivGroupDialog		//
//////////////////////////////////

gpivGroupDialog::gpivGroupDialog(TypeDef* tp, void* base) : gpivListDialog(tp, base) {
  sub_data_g = NULL;
}

gpivGroupDialog::~gpivGroupDialog() {
  CloseWindow();
}

void gpivGroupDialog::CloseWindow() {
  sub_data_el.Reset();
  sub_data_g = NULL;
  gpivListDialog::CloseWindow();
}

void gpivGroupDialog::Constr_Strings(const char*, const char* win_title) {
  prompt_str = cur_lst->GetTypeDef()->name + ": ";
  if(cur_lst->GetTypeDef()->desc == TA_taBase_Group.desc) {
    prompt_str += cur_lst->el_typ->name + "s: " + cur_lst->el_typ->desc;
  }
  else {
    prompt_str += cur_lst->GetTypeDef()->desc;
  }
  win_str = String(ivSession::instance()->classname()) + ": " +
    win_title + " " + cur_lst->GetPath();
}


void gpivGroupDialog::Constr_Data() {
  lst_data_g = new lrScrollBox(); lst_data_g->naturalnum = 3;
  lst_data_patch = new ivPatch(lst_data_g);
  lstel_rep = layout->vbox();
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  if(cur_gp->gp.size > 0) {
    sub_data_g = new tbScrollBox();
    sub_data_g->naturalnum = 5;
    lst_data_g->naturalnum = 2; // if there are subgroups, only show 2 of maingroup
  }
  Constr_ListData();
  Constr_ElData();
  Constr_SubGpData();
  FocusOnFirst();
  GetImage();
}

void gpivGroupDialog::Constr_Box() {
  Constr_Labels();
  Constr_Data();
  Constr_Methods();
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  if(cur_gp->gp.size > 0) {
    box = layout->flexible
      (layout->vbox
       (layout->hcenter(lstel_rep,0),
	taivM->vfsep,
	layout->hcenter(wkit->menu_item_separator_look(),0),
	taivM->vfsep,
	layout->hcenter
	(layout->hbox
	 (labels, taivM->hfspc,
	  layout->hbox
	  (layout->vbox
	   (lst_data_patch,wkit->hscroll_bar(lst_data_g)),
	   layout->hflexible(layout->hbox(wkit->vscroll_bar(sub_data_g), sub_data_g), 0,0)))
	 ,0)));
  }
  else {
    box = layout->flexible
      (layout->vbox
       (layout->hcenter(lstel_rep,0),
	taivM->vfsep,
	layout->hcenter(wkit->menu_item_separator_look(),0),
	taivM->vfsep,
	layout->hcenter
	(layout->hbox(labels, taivM->hfspc,
		      layout->vbox
		      (lst_data_patch,wkit->hscroll_bar(lst_data_g)))
	 ,0)));
  }
  // under some circumstances these are not used, so we ref them
  ivResource::ref(labels);
}

void gpivGroupDialog::GetImage() {
  gpivListDialog::GetImage();
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  if(sub_data_el.size != cur_gp->gp.size) {
    if(sub_data_g == NULL)
      return;
    sub_data_el.Reset();
    ivGlyphIndex i;
    for(i=sub_data_g->count()-1; i >= 0; i--)
      sub_data_g->remove(i);
    Constr_SubGpData();
  }

  // and the sub-Lists
  int lf;
  for(lf=0; lf<cur_gp->gp.size; lf++) {
    TAGPtr sub = cur_gp->gp.FastEl(lf);
    gpivSubEditButton* sub_dat = (gpivSubEditButton*)sub_data_el.FastEl(lf);
    String nm = sub->name;
    if(nm == "")
      nm = String("[") + String(lf) + "]";
    sub_dat->label = nm;
    sub_dat->GetImage((void*)sub, cur_win);
  }
}

void gpivGroupDialog::Constr_SubGpData() {
  if(sub_data_g == NULL)
    return;
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  int lf;
  for(lf=0; lf<cur_gp->gp.size; lf++) {
    TAGPtr sub = cur_gp->gp.FastEl(lf);
    String nm = sub->name;
    if(nm == "")
      nm = String("[") + String(lf) + "]";
    taivData* mb_dat =
      new gpivSubEditButton(sub->GetTypeDef(), (void*)sub, NULL, nm, this);
    sub_data_el.Add(mb_dat);
    sub_data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()), vfix));
  }
}

int gpivGroupDialog::Edit() {
  TAGPtr cur_gp = (TAGPtr)cur_lst;
  if((cur_gp != NULL) && ((cur_gp->size > 100) || (cur_gp->gp.size > 100))) {
    int rval = taMisc::Choice("Group contains more than 100 items or sub-groups (size = " +
			      String(cur_gp->size) + ", gp.size = "
			      + String(cur_gp->gp.size) + "), continue with Edit?",
			      "Ok", "Cancel");
    if(rval == 1) return 0;
  }
  return taivEditDialog::Edit();
}

//////////////////////////////////
//	gpivArrayEditDialog	//
//////////////////////////////////

gpivArrayEditDialog::gpivArrayEditDialog(TypeDef *tp, void* base)
: taivEditDialog(tp, base) {
  n_ary_membs = 0;
  ary_data_g = NULL;
}

bool gpivArrayEditDialog::ShowMember(MemberDef* md) {
  if(md->name == "size")
    return true;
  else
    return taivEditDialog::ShowMember(md);
}

void gpivArrayEditDialog::GetImage() {
  taivEditDialog::GetImage();
  taArray_base* cur_ary = (taArray_base*)cur_base;
  if(data_el.size != cur_ary->size + n_ary_membs) {
    ivGlyphIndex i;
    for(i=ary_data_g->count()-1; i >= 0; i--)
      ary_data_g->remove(i);
    int j;
    for(j=data_el.size-1; j >= n_ary_membs; j--)
      data_el.Remove(j);
    Constr_AryData();
  }
  MemberDef* eldm = typ->members.FindName("el");
  taivType* tiv = eldm->type->GetNonPtrType()->iv;
  int i;
  for(i=0;i<cur_ary->size;i++) {
    taivData* mb_dat = data_el.FastEl(i+n_ary_membs);
    tiv->GetImage(mb_dat, cur_ary->FastEl_(i), cur_win);
  }
}

void gpivArrayEditDialog::GetValue() {
  taivEditDialog::GetValue();
  taArray_base* cur_ary = (taArray_base*)cur_base;
  if(data_el.size != cur_ary->size + n_ary_membs) {
    taMisc::Error("Cannot apply changes: Array size has changed");
    return;
  }
  MemberDef* eldm = typ->members.FindName("el");
  taivType* tiv = eldm->type->GetNonPtrType()->iv;
  int i;
  for(i=0;i<cur_ary->size;i++){
    taivData* mb_dat = data_el.FastEl(i+n_ary_membs);
    tiv->GetValue(mb_dat, cur_ary->FastEl_(i));
  }
}

void gpivArrayEditDialog::Constr_AryData() {
  taArray_base* cur_ary = (taArray_base*)cur_base;
  MemberDef* eldm = typ->members.FindName("el");
  taivType* tiv = eldm->type->GetNonPtrType()->iv;
  int i;
  for(i=0;i<cur_ary->size;i++) {
    taivData* mb_dat = tiv->GetDataRep(dialog, this);
    data_el.Add(mb_dat);
    String nm = String("[") + String(i) + "]";
    ary_data_g->append
       (layout->vcenter
	(layout->vbox
	 (layout->hcenter(wkit->fancy_label(nm),0),
	  layout->hcenter(mb_dat->GetLook(),0)),0));
  }
}

void gpivArrayEditDialog::Constr_Data() {
  data_g = layout->vbox();
  ary_data_g = new lrScrollBox;
  ary_data_g->naturalnum = 5;
  Constr_Data_impl(typ->members, data_el);
  n_ary_membs = data_el.size;
  FocusOnFirst();
  GetImage();
}

void gpivArrayEditDialog::Constr_Box() {
  Constr_Labels();
  Constr_Data();
  Constr_Methods();
  box = layout->hflexible
    (layout->hbox
     (labels, taivM->hfspc,
      (wkit->inset_frame
       (new ivBackground
	(layout->hflexible(layout->vbox(data_g,ary_data_g,wkit->hscroll_bar(ary_data_g))),
	 bg_color_dark)))));
  // under some circumstances these are not used, so we ref them
  ivResource::ref(labels);
}

void gpivArrayEditDialog::Scroll(){
  if(dialog->last_focus == -1)
    ary_data_g->scroll_backward(Dimension_X);
  else if(dialog->last_focus == 1) 
    ary_data_g->scroll_forward(Dimension_X);
}

int gpivArrayEditDialog::Edit() {
  taArray_base* cur_ary = (taArray_base*)cur_base;
  if((cur_ary != NULL) && (cur_ary->size > 100)) {
    int rval = taMisc::Choice("Array contains more than 100 items (size = " +
			      String(cur_ary->size) + "), continue with Edit?",
			      "Ok", "Cancel");
    if(rval == 1) return 0;
  }
  return taivEditDialog::Edit();
}
    
//////////////////////////////////
// 	gpivListEdit		//
//////////////////////////////////

int gpivListEdit::BidForEdit(TypeDef* td) {
  if(td->InheritsFrom(TA_taList_impl))
    return taivEdit::BidForEdit(td)+1;
  return 0;
}

int gpivListEdit::Edit(void* base, ivWindow* win, bool wait, bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new gpivListDialog(typ, base);
    dlg->Constr(win, wait, "", "", false, bgclr);
    dlg->cancel_only = readonly;
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

int gpivGroupEdit::BidForEdit(TypeDef* td) {
  if(td->InheritsFrom(TA_taGroup_impl))
    return gpivListEdit::BidForEdit(td)+1;
  return 0;
}

int gpivGroupEdit::Edit(void* base, ivWindow* win, bool wait, bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new gpivGroupDialog(typ, base);
    dlg->Constr(win, wait, "", "", false, bgclr);
    dlg->cancel_only = readonly;
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

int gpivArrayEdit::BidForEdit(TypeDef* td){
  if(td->InheritsFrom(TA_taArray))
    return (taivEdit::BidForType(td) +1);
  return 0;
}

int gpivArrayEdit::Edit(void* base, ivWindow* win, bool wait,bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new gpivArrayEditDialog(typ, base);
    dlg->Constr(win, wait, "", "", false, bgclr);
    dlg->cancel_only = readonly;
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

//////////////////////////////////
// 	gpivListType		//
//////////////////////////////////

int gpivListType::BidForType(TypeDef* td) {
  if(td->InheritsFrom(TA_taList_impl))
    return (taivClassType::BidForType(td) +1);
  return 0;
}
taivData* gpivListType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivListEditButton *rval = new gpivListEditButton(typ, NULL, NULL, dlg, par);
  return rval;
}

void gpivListType::GetImage(taivData* dat, void* base, ivWindow* win) {  
  gpivListEditButton *rval = (gpivListEditButton*)dat;
  rval->GetImage(base, win);
}
void gpivListType::GetValue(taivData*, void*) {
}

//////////////////////////////////
// 	gpivGroupType		//
//////////////////////////////////

int gpivGroupType::BidForType(TypeDef* td) {
  if(td->InheritsFrom(TA_taGroup_impl))
    return (gpivListType::BidForType(td) +1);
  return 0;
}
taivData* gpivGroupType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivGroupEditButton *rval = new gpivGroupEditButton(typ, NULL, NULL, dlg, par);
  return rval;
}

void gpivGroupType::GetImage(taivData* dat, void* base, ivWindow* win) {  
  gpivGroupEditButton *rval = (gpivGroupEditButton*)dat;
  rval->GetImage(base, win);
}
void gpivGroupType::GetValue(taivData*, void*) {
}


//////////////////////////////////
// 	gpivArrayType		//
//////////////////////////////////

int gpivArray_Type::BidForType(TypeDef* td) {
  if (td->InheritsFrom(TA_taArray)) { // bid higher than the class  type
    return (taivClassType::BidForType(td) +1);
  }
  else {
    return 0;
  } 
}

taivData* gpivArray_Type::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivArrayEditButton *rval =
    new gpivArrayEditButton(typ, NULL,	NULL, dlg, par);
  return rval;
}

void gpivArray_Type::GetImage(taivData* dat, void* base, ivWindow* win) {  
  gpivArrayEditButton *rval = (gpivArrayEditButton*)dat;
  rval->GetImage(base, win);
}

void gpivArray_Type::GetValue(taivData*, void*) {
}

//////////////////////
// taivROListMember //
/////////////////////


int taivROListMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->HasOption("READ_ONLY") || md->HasOption("IV_READ_ONLY")) &&
     (td->InheritsFrom(TA_taList_impl)))
    return (taivROMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivROListMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivListEditButton *rval = new gpivListEditButton(typ, NULL, NULL, dlg, par);
  rval->read_only = true;
  return rval;
}

void taivROListMember::GetImage(taivData* dat, void* base, ivWindow* win){
  gpivListEditButton *rval = (gpivListEditButton*)dat;
  rval->GetImage(base, win);
}

void taivROListMember::GetMbrValue(taivData*, void*, bool&) {
}


//////////////////////
// taivROGroupMember //
/////////////////////


int taivROGroupMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->HasOption("READ_ONLY") || md->HasOption("IV_READ_ONLY")) &&
     (td->InheritsFrom(TA_taGroup_impl)))
    return (taivROMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivROGroupMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivGroupEditButton *rval = new gpivGroupEditButton(typ, NULL, NULL, dlg, par);
  rval->read_only = true;
  return rval;
}

void taivROGroupMember::GetImage(taivData* dat, void* base, ivWindow* win){
  gpivGroupEditButton *rval = (gpivGroupEditButton*)dat;
  rval->GetImage(base, win);
}

void taivROGroupMember::GetMbrValue(taivData*, void*, bool&) {
}



//////////////////////
// taivROArrayMember //
/////////////////////


int taivROArrayMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->HasOption("READ_ONLY") || md->HasOption("IV_READ_ONLY")) &&
     (td->InheritsFrom(TA_taArray)))
    return (taivROMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivROArrayMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivArrayEditButton *rval = new gpivArrayEditButton(typ, NULL, NULL, dlg, par);
  rval->read_only = true;
  return rval;
}

void taivROArrayMember::GetImage(taivData* dat, void* base, ivWindow* win){
  gpivArrayEditButton *rval = (gpivArrayEditButton*)dat;
  rval->GetImage(base, win);
}

void taivROArrayMember::GetMbrValue(taivData*, void*, bool&) {
}


//////////////////////////////////
// 	gpivDefaultEl		//
//////////////////////////////////

int gpivDefaultEl::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->name == "el_def") && (td->InheritsFrom(TA_taList_impl)))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* gpivDefaultEl::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivListEls *rval = new gpivListEls(taivMenu::menubar, taivMenu::small,
					NULL, dlg, par, true, true);
  return rval;
}

void gpivDefaultEl::GetImage(taivData* dat, void* base, ivWindow*) {
  taList_impl* tl = (taList_impl*)base;
  TAPtr tmp_ptr = tl->DefaultEl_();
  gpivListEls* rval = (gpivListEls*)dat;
  rval->GetImage((TAGPtr)base, tmp_ptr);
  GetOrigVal(dat, base);
}

void gpivDefaultEl::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  taList_impl* tl = (taList_impl*)base;
  gpivListEls* rval = (gpivListEls*)dat;
  TAPtr tmp_ptr = rval->GetValue();
  tl->SetDefaultEl(tmp_ptr);
  CmpOrigVal(dat, base, first_diff);
}


//////////////////////////////////
// 	gpivLinkGp		//
//////////////////////////////////

int gpivLinkGP::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->type->InheritsFrom(TA_taGroup_impl)) &&
     (md->HasOption("LINK_GROUP")))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* gpivLinkGP::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivLinkEditButton* rval = new gpivLinkEditButton(mbr->type, NULL, NULL, dlg, par);
  return rval;
}

void gpivLinkGP::GetImage(taivData* dat, void* base, ivWindow* win) {
  gpivLinkEditButton* rval = (gpivLinkEditButton*)dat;
  rval->GetImage(mbr->GetOff(base), win);
}

void gpivLinkGP::GetMbrValue(taivData*, void*, bool&) {
}

//////////////////////////////////
// 	gpivLinkList		//
//////////////////////////////////

int gpivLinkList::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->type->InheritsFrom(TA_taList_impl)) &&
     (md->HasOption("LINK_GROUP")))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* gpivLinkList::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivListLinkEditButton* rval = new gpivListLinkEditButton(mbr->type, NULL, NULL, dlg, par);
  return rval;
}

void gpivLinkList::GetImage(taivData* dat, void* base, ivWindow* win) {
  gpivListLinkEditButton* rval = (gpivListLinkEditButton*)dat;
  rval->GetImage(mbr->GetOff(base), win);
}

void gpivLinkList::GetMbrValue(taivData*, void*, bool&) {
}


//////////////////////////////////
// 	gpivFromGpTokenPtr	//
//////////////////////////////////

int gpivFromGpTokenPtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if(td->InheritsFrom(TA_taBase) && (md->type->ptr == 1)
     && md->type->DerivesFrom(TA_taBase) && (md->OptionAfter("FROM_GROUP_") != ""))
    return taivTokenPtrMember::BidForMember(md,td)+1;
  return 0;
}

taivData* gpivFromGpTokenPtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  MemberDef* from_md = GetFromMd();
  if(from_md == NULL)	return NULL;
  bool null_ok = true;
  if(mbr->HasOption("NO_NULL"))
    null_ok = false;
  bool edit_ok = true;
  if(mbr->HasOption("NO_EDIT"))
    edit_ok = false;

  bool no_gp = true;
  if(mbr->HasOption("GROUP_OPT_OK"))
    no_gp = false;

  if(mbr->type->DerivesFrom(&TA_taGroup_impl))
    return new gpivSubGroups(taivMenu::menubar, taivMenu::small, NULL, dlg, par, null_ok,
			      edit_ok);
  else if(from_md->type->DerivesFrom(TA_taGroup_impl))
    return new gpivGroupEls(taivMenu::menubar, taivMenu::small, NULL,
			     dlg, par, null_ok, no_gp, true, edit_ok);
  else
    return new gpivListEls(taivMenu::menubar, taivMenu::small, NULL,
			    dlg, par, null_ok, no_gp, edit_ok);
}

void gpivFromGpTokenPtrMember::GetImage(taivData* dat, void* base, ivWindow*) {
  MemberDef* from_md = GetFromMd();
  if(from_md == NULL)	return;
  if(mbr->type->DerivesFrom(TA_taGroup_impl)) {
    gpivSubGroups* rval = (gpivSubGroups*)dat;
    TAGPtr lst = (TAGPtr)GetList(from_md, base);
    rval->GetImage(lst, *((TAGPtr*)mbr->GetOff(base)));
  }
  else {
    gpivListEls* rval = (gpivListEls*)dat;
    TABLPtr lst = GetList(from_md, base);
    rval->GetImage(lst, *((TAPtr*)mbr->GetOff(base)));
  }
  GetOrigVal(dat, base);
}

void gpivFromGpTokenPtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  if(mbr->type->DerivesFrom(&TA_taGroup_impl)) {
    gpivSubGroups* rval = (gpivSubGroups*)dat;
    taBase::SetPointer((TAPtr*)mbr->GetOff(base), (TAPtr)rval->GetValue());
  }
  else {
    gpivListEls* rval = (gpivListEls*)dat;
    taBase::SetPointer((TAPtr*)mbr->GetOff(base), (TAPtr)rval->GetValue());
  }
  CmpOrigVal(dat, base, first_diff);
}

MemberDef* gpivFromGpTokenPtrMember::GetFromMd() {
  String mb_nm = mbr->OptionAfter("FROM_GROUP_");
  MemberDef* from_md = NULL;
  if(mb_nm != "")
    from_md = typ->members.FindName(mb_nm);
  return from_md;
}

TABLPtr	gpivFromGpTokenPtrMember::GetList(MemberDef* from_md, void* base) {
  if(from_md == NULL)
    return NULL;
  if(from_md->type->ptr == 1)
    return *((TABLPtr*)from_md->GetOff(base));
  else
    return (TABLPtr)from_md->GetOff(base);
}


//////////////////////////////////
//        gpivTAPtrArgType     //
//////////////////////////////////

int gpivTAPtrArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(td->InheritsFrom(TA_taList_impl) &&
     (argt->ptr == 1) && argt->DerivesFrom(TA_taBase))
    return taivTokenPtrArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* gpivTAPtrArgType::GetElFromArg(const char* nm, void* base) {
  TABLPtr lst = (TABLPtr)base;
  if((lst != NULL) && 
     (arg_typ->DerivesFrom(lst->el_base) || lst->el_base->DerivesFrom(arg_typ->GetNonPtrType()))) {
    String ptrnm = lst->el_base->name + "_ptr";
    TypeDef* ntd = taMisc::types.FindName(ptrnm);
    if(ntd != NULL)
      arg_typ = ntd;	// search in el_base (if args are compatible)
  }
  return taivTokenPtrArgType::GetElFromArg(nm,base);
}


//////////////////////////////////
//        gpivInObjArgType     //
//////////////////////////////////

int gpivInObjArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(td->InheritsFrom(TA_taList_impl) &&
     (argt->ptr == 1) && argt->DerivesFrom(TA_taBase) && (md->HasOption("ARG_ON_OBJ")))
    return gpivTAPtrArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* gpivInObjArgType::GetElFromArg(const char* nm, void* base) {
  TABLPtr lst = (TABLPtr)base;
  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  if(lst != NULL)
    arg_val = new cssTA_Base(lst->DefaultEl_(), 1, npt, nm);
  else 
    arg_val = new cssTA_Base(NULL, 1, npt, nm);

  arg_base = (void*)&(((cssTA_Base*)arg_val)->ptr);
  return arg_val;
}

taivData* gpivInObjArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  bool null_ok = false;
  if(meth->HasOption("NULL_OK"))
    null_ok = true;
  bool edit_ok = false;
  if(meth->HasOption("EDIT_OK"))
    edit_ok = true;
  if(typ->InheritsFrom(TA_taGroup_impl))
    return new gpivGroupEls(taivMenu::menubar, taivMenu::small, NULL,
			     dlg, par, null_ok, false, true, edit_ok);
  else
    return new gpivListEls(taivMenu::menubar, taivMenu::small, NULL,
			    dlg, par, null_ok, false, edit_ok);
}

void gpivInObjArgType::GetImage(taivData* dat, void* base, ivWindow*) {
  if(arg_base == NULL)
    return;
  gpivListEls* els;
  if(typ->InheritsFrom(TA_taGroup_impl))
    els = (gpivGroupEls*)dat;
  else
    els = (gpivListEls*)dat;
  els->GetImage((TABLPtr)base, *((TAPtr*)arg_base));
}

void gpivInObjArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  gpivListEls* els;
  if(typ->InheritsFrom(TA_taGroup_impl))
    els = (gpivGroupEls*)dat;
  else
    els = (gpivListEls*)dat;

  *((TAPtr*)arg_base) = els->GetValue();
}

//////////////////////////////////
//        gpivFromGpArgType    //
//////////////////////////////////

int gpivFromGpArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if((argt->ptr != 1) || !argt->DerivesFrom(TA_taBase))
    return 0;
  String fmgp = md->OptionAfter("FROM_GROUP_");
  if(fmgp.empty()) return 0;
  if(isdigit(fmgp.firstchar()) && (fmgp.at(1,1) == "_")) {
    int idx = (int)(String)fmgp.before('_');
    if(aidx > idx) return 0;	// index means anything at this index or less
  }
  return taivTokenPtrArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* gpivFromGpArgType::GetElFromArg(const char* nm, void* base) {
  MemberDef* from_md = GetFromMd();
  if(from_md == NULL)	return NULL;
  TABLPtr lst = GetList(from_md, base);
  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  if(lst != NULL)
    arg_val = new cssTA_Base(lst->DefaultEl_(), 1, npt, nm);
  else 
    arg_val = new cssTA_Base(NULL, 1, npt, nm);

  arg_base = (void*)&(((cssTA_Base*)arg_val)->ptr);
  return arg_val;
}

taivData* gpivFromGpArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  MemberDef* from_md = GetFromMd();
  if(from_md == NULL)	return NULL;
  bool null_ok = false;
  if(meth->HasOption("NULL_OK"))
    null_ok = true;
  bool edit_ok = false;
  if(meth->HasOption("EDIT_OK"))
    edit_ok = true;

  bool no_gp = false;
  if(meth->HasOption("NO_GROUP_OPT"))
    no_gp = true;

  if(from_md->type->DerivesFrom(TA_taGroup_impl))
    return new gpivGroupEls(taivMenu::menubar, taivMenu::small, NULL,
			     dlg, par, null_ok, no_gp, true, edit_ok);
  else
    return new gpivListEls(taivMenu::menubar, taivMenu::small, NULL,
			    dlg, par, null_ok, no_gp, edit_ok);
}

void gpivFromGpArgType::GetImage(taivData* dat, void* base, ivWindow*) {
  if(arg_base == NULL)  return;
  MemberDef* from_md = GetFromMd();
  if(from_md == NULL)	return;
  gpivListEls* els = (gpivListEls*)dat;
  TABLPtr lst = GetList(from_md, base);
  els->GetImage(lst, *((TAPtr*)arg_base));
}

void gpivFromGpArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  gpivListEls* els = (gpivListEls*)dat;
  *((TAPtr*)arg_base) = els->GetValue();
}

MemberDef* gpivFromGpArgType::GetFromMd() {
  MemberDef* from_md = NULL;
  String mb_nm = meth->OptionAfter("FROM_GROUP_");
  if(!mb_nm.empty()) {
    if(isdigit(mb_nm.firstchar()) && (mb_nm.at(1,1) == "_"))
      mb_nm = mb_nm.after('_');
    from_md = typ->members.FindName(mb_nm);
  }
  return from_md;
}

TABLPtr	gpivFromGpArgType::GetList(MemberDef* from_md, void* base) {
  if(from_md == NULL)
    return NULL;
  if(from_md->type->ptr == 1)
    return *((TABLPtr*)from_md->GetOff(base));
  else
    return (TABLPtr)from_md->GetOff(base);
}


//////////////////////////////////
//	select edit dialog	//
//////////////////////////////////

void SelectEditConfig::Initialize() {
  auto_edit = true;
}

void SelectEditConfig::Destroy() {
}

void SelectEditConfig::InitLinks() {
  taBase::InitLinks();
  taBase::Own(mbr_labels, this);
  taBase::Own(meth_labels, this);
}

void SelectEditConfig::Copy_(const SelectEditConfig& cp) {
  auto_edit = cp.auto_edit;
  mbr_labels = cp.mbr_labels;
  meth_labels = cp.meth_labels;
}

void SelectEdit::Initialize() {
  edit_on_reopen = false;
}

void SelectEdit::InitLinks() {
  taNBase::InitLinks();

  taBase::Own(config, this);

  taBase::Own(mbr_bases, this);
  taBase::Own(mbr_strs, this);
  taBase::Own(mbr_base_paths, this);

  taBase::Own(meth_bases, this);
  taBase::Own(meth_strs, this);
  taBase::Own(meth_base_paths, this);
}

void SelectEdit::Copy_(const SelectEdit& cp) {
  config = cp.config;

  mbr_bases.Borrow(cp.mbr_bases);
  members.Borrow(cp.members);
  mbr_strs.Copy(cp.mbr_strs);

  meth_bases.Borrow(cp.meth_bases);
  methods.Borrow(cp.methods);
  meth_strs.Copy(cp.meth_strs);
}

void SelectEdit::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  if((mbr_base_paths.size > 0) || (meth_base_paths.size > 0)) {
    BaseChangeReOpen();		// must have been saved, so reopen it!
  }
  config.mbr_labels.EnforceSize(mbr_bases.size);
  config.meth_labels.EnforceSize(meth_bases.size);
  mbr_strs.EnforceSize(mbr_bases.size);
  meth_strs.EnforceSize(meth_bases.size);
  if(taMisc::is_loading) {
    GetMembsFmStrs();
    GetMethsFmStrs();
  }
  if(edit_on_reopen) {
    Edit();
    edit_on_reopen = false;
  }
}

int SelectEdit::Dump_Load_Value(istream& strm, TAPtr par) {
  edit_on_reopen = CloseEdit();
  members.Reset();
  mbr_bases.Reset();
  mbr_strs.Reset();
  config.mbr_labels.Reset();

  methods.Reset();
  meth_bases.Reset();
  meth_strs.Reset();
  config.meth_labels.Reset();

  return taNBase::Dump_Load_Value(strm, par);
}

void SelectEdit::GetAllPaths() {
  if(mbr_bases.size > 0) {
    mbr_base_paths.Reset();
    for(int i=0;i<mbr_bases.size;i++) {
      mbr_base_paths.Add(mbr_bases.FastEl(i)->GetPath());
    }
  }
  if(meth_bases.size > 0) {
    meth_base_paths.Reset();
    int i;
    for(i=0;i<meth_bases.size;i++) {
      meth_base_paths.Add(meth_bases.FastEl(i)->GetPath());
    }
  }
}

int SelectEdit::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  return taNBase::Dump_Save_Value(strm, par, indent);
}

void SelectEdit::GetMembsFmStrs() {
  int i;
  for(i=0;i<mbr_bases.size;i++) {
    TAPtr bs = mbr_bases.FastEl(i);
    if(bs == NULL) { // didn't get loaded, bail..
      taMisc::Error("*** SelectEdit: couldn't find object:", config.mbr_labels[i], mbr_strs[i], "in object to edit");
      mbr_bases.Remove(i);      mbr_strs.Remove(i);      config.mbr_labels.Remove(i);
      i--;
      continue;
    }
    String nm = mbr_strs.FastEl(i);
    MemberDef* md = bs->FindMember((const char*)nm);
    if(md == NULL) {
      taMisc::Error("*** SelectEdit: couldn't find member:", nm, "in object to edit:",bs->GetPath());
      mbr_bases.Remove(i);      mbr_strs.Remove(i);      config.mbr_labels.Remove(i);
      i--;
      continue;
    }
    members.Link(md);
  }
}

void SelectEdit::GetMethsFmStrs() {
  int i;
  for(i=0;i<meth_bases.size;i++) {
    TAPtr bs = meth_bases.FastEl(i);
    if(bs == NULL) { // didn't get loaded, bail..
      taMisc::Error("*** SelectEdit: couldn't find object:", config.meth_labels[i], meth_strs[i], "in object to edit");
      meth_bases.Remove(i);      meth_strs.Remove(i);      config.meth_labels.Remove(i);
      i--;
      continue;
    }
    String nm = meth_strs.FastEl(i);
    MethodDef* md = bs->GetTypeDef()->methods.FindName((const char*)nm);
    if(md == NULL) {
      taMisc::Error("*** SelectEdit: couldn't find method:", nm, "in object to edit:",bs->GetPath());
      meth_bases.Remove(i);      meth_strs.Remove(i);      config.meth_labels.Remove(i);
      i--;
      continue;
    }
    methods.Link(md);
  }
}

int SelectEdit::FindMbrBase(TAPtr base, MemberDef* md) {
  int i;
  for(i=0;i<mbr_bases.size;i++) {
    if((mbr_bases.FastEl(i) == base) && (members.FastEl(i) == md))
      return i;
  }
  return -1;
}

bool SelectEdit::SelectMember(TAPtr base, MemberDef* md, const char* lbl) {
  bool rval = false;
  bool reopen = CloseEdit();
  int bidx = FindMbrBase(base, md);
  if(bidx >= 0) {
    config.mbr_labels.SafeEl(bidx) = lbl;
  }
  else {
    mbr_bases.Link(base);  members.Link(md);  mbr_strs.Add(md->name);  config.mbr_labels.Add(lbl);
    rval = true;
  }
  if(reopen) Edit();
  return rval;
}

bool SelectEdit::SelectMemberNm(TAPtr base, const char* md, const char* lbl) {
  if(base == NULL) return false;
  MemberDef* mda = (MemberDef*)base->FindMember(md);
  if(mda == NULL) return false;
  return SelectMember(base, mda, lbl);
}

int SelectEdit::FindMethBase(TAPtr base, MethodDef* md) {
  int i;
  for(i=0;i<meth_bases.size;i++) {
    if((meth_bases.FastEl(i) == base) && (methods.FastEl(i) == md))
      return i;
  }
  return -1;
}

bool SelectEdit::SelectMethod(TAPtr base, MethodDef* md, const char* lbl) {
  bool rval = false;
  bool reopen = CloseEdit();
  int bidx = FindMethBase(base, md);
  if(bidx >= 0) {
    config.meth_labels.SafeEl(bidx) = lbl;
  }
  else {
    meth_bases.Link(base);  methods.Link(md);  meth_strs.Add(md->name);  config.meth_labels.Add(lbl);
    rval = true;
  }
  if(reopen) Edit();
  return rval;
}

bool SelectEdit::SelectMethodNm(TAPtr base, const char* md, const char* lbl) {
  if(base == NULL) return false;
  MethodDef* mda = (MethodDef*)base->GetTypeDef()->methods.FindName(md);
  if(mda == NULL) return false;
  return SelectMethod(base, mda, lbl);
}

void SelectEdit::UpdateAllBases() {
  int i;
  for(i=0;i<mbr_bases.size;i++) {
    TAPtr bs = mbr_bases.FastEl(i);
    if(bs == NULL) continue;
    bs->UpdateAfterEdit();
    taivMisc::Update(bs);
  }
  for(i=0;i<meth_bases.size;i++) {
    TAPtr bs = meth_bases.FastEl(i);
    if(bs == NULL) continue;
    bs->UpdateAfterEdit();
  }
}

void SelectEdit::RemoveField(int idx) {
  bool reopen = CloseEdit();
  mbr_bases.Remove(idx);  members.Remove(idx);  mbr_strs.Remove(idx);  config.mbr_labels.Remove(idx);
  if(reopen) Edit();
}

void SelectEdit::MoveField(int from, int to) {
  bool reopen = CloseEdit();
  mbr_bases.Move(from, to);  members.Move(from, to);  mbr_strs.Move(from, to);  config.mbr_labels.Move(from, to);
  if(reopen) Edit();
}

void SelectEdit::RemoveFun(int idx) {
  bool reopen = CloseEdit();
  meth_bases.Remove(idx);  methods.Remove(idx);  meth_strs.Remove(idx);  config.meth_labels.Remove(idx);
  if(reopen) Edit();
}

void SelectEdit::MoveFun(int from, int to) {
  bool reopen = CloseEdit();
  meth_bases.Move(from, to);  methods.Move(from, to);  meth_strs.Move(from, to);  config.meth_labels.Move(from, to);
  if(reopen) Edit();
}

void SelectEdit::NewEdit() {
  CloseEdit();
  Edit();
}

bool SelectEdit::BaseClosing(TAPtr base) {
  bool reopen = false; 
  bool gotone = false;
  int i;
  for(i=mbr_bases.size-1;i>=0;i--) {
    TAPtr bs = mbr_bases.FastEl(i);
    char* staddr = (char*)bs;
    char* endaddr=staddr+bs->GetSize();
    char* vbase = (char*)base;
    if((vbase >= staddr) && (vbase <= endaddr)) {
      if(!gotone)
	reopen = CloseEdit();
      mbr_bases.Remove(i);  members.Remove(i);  mbr_strs.Remove(i);  config.mbr_labels.Remove(i);
      gotone = true;
    }
  }

  for(i=meth_bases.size-1;i>=0;i--) {
    TAPtr bs = meth_bases.FastEl(i);
    if(bs == base) {
      if(!gotone)
	reopen = CloseEdit();
      meth_bases.Remove(i);  methods.Remove(i);  meth_strs.Remove(i);  config.meth_labels.Remove(i);
      gotone = true;
    }
  }

  if(gotone) {
    if(!edit_on_reopen)
      edit_on_reopen = reopen;
    tabMisc::DelayedUpdateAfterEdit(this); // get this to open up later (after object is killed)
  }
  return gotone;
}

bool SelectEdit::BaseClosingAll(TAPtr base) {
  bool got_one = false;
  int i;
  for(i=0;i<TA_SelectEdit.tokens.size;i++) {
    SelectEdit* se = (SelectEdit*)TA_SelectEdit.tokens.FastEl(i);
    if(se->BaseClosing(base))
      got_one = true;
  }
  return got_one;
}

void SelectEdit::BaseChangeSave() {
  if((mbr_bases.size > 0) || (meth_bases.size > 0))
    edit_on_reopen = CloseEdit();

  GetAllPaths();
  if(mbr_bases.size > 0)
    mbr_bases.Reset();
  if(meth_bases.size > 0)
    meth_bases.Reset();
}

void SelectEdit::BaseChangeReOpen() {
  if((mbr_base_paths.size == 0) && (meth_base_paths.size == 0)) return;

  if(mbr_base_paths.size > 0) {
    mbr_bases.Reset();		// get rid of the mbr_bases!
    int i;
    for(i=0;i<mbr_base_paths.size;i++) {
      String path = mbr_base_paths.FastEl(i);
      TAPtr bs = tabMisc::root->FindFromPath(path);
      if(bs == NULL) {
	taMisc::Error("SelectEdit::BaseChangeReOpen: could not find object from path:",path);
	members.Remove(i);  mbr_strs.Remove(i);  config.mbr_labels.Remove(i);  mbr_base_paths.Remove(i);
	i--;
	continue;
      }
      mbr_bases.Link(bs);
    }
    mbr_base_paths.Reset();
  }

  if(meth_base_paths.size > 0) {
    meth_bases.Reset();		// get rid of the meth_bases!
    int i;
    for(i=0;i<meth_base_paths.size;i++) {
      String path = meth_base_paths.FastEl(i);
      TAPtr bs = tabMisc::root->FindFromPath(path);
      if(bs == NULL) {
	taMisc::Error("SelectEdit::BaseChangeReOpen: could not find object from path:",path);
	methods.Remove(i);  meth_strs.Remove(i);  config.meth_labels.Remove(i);  meth_base_paths.Remove(i);
	i--;
	continue;
      }
      meth_bases.Link(bs);
    }
    meth_base_paths.Reset();
  }

  if(edit_on_reopen)
    Edit();
  edit_on_reopen = false;
}

String SelectEdit::GetMbrLabel(int i) {
  String lbl;
  if(config.mbr_labels.size > i)
    lbl = config.mbr_labels.FastEl(i);
  String nm = String(i) + ": " + lbl;
  if(!lbl.empty()) nm += " ";
  nm += members.FastEl(i)->GetLabel();
  return nm;
}

String SelectEdit::GetMethLabel(int i) {
  String lbl;
  if(config.meth_labels.size > i)
    lbl = config.meth_labels.FastEl(i);
  String nm = String(i) + ": " + lbl;
  if(!lbl.empty()) nm += " ";
  nm += methods.FastEl(i)->GetLabel();
  return nm;
}

//////////////////////////////////
//	gpivSelectEdit		//
//////////////////////////////////

int gpivSelectEdit::BidForEdit(TypeDef* td) {
  if(td->InheritsFrom(TA_SelectEdit))
    return taivEdit::BidForEdit(td)+1;
  return 0;
}

int gpivSelectEdit::Edit(void* base, ivWindow* win, bool wait,bool readonly, const ivColor* bgclr) {
  gpivSelectEditDialog* dlg = (gpivSelectEditDialog*)taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new gpivSelectEditDialog(typ, base);
    dlg->Constr(win, wait, "", "", false, bgclr);
    dlg->cancel_only = readonly;
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

gpivSelectEditDialog::gpivSelectEditDialog(TypeDef* td, void* base) : taivEditDialog(td, base) {
  sele = (SelectEdit*)base;
}

gpivSelectEditDialog::~gpivSelectEditDialog() {
}

bool gpivSelectEditDialog::ShowMember(MemberDef* md) {
  if(md->iv == NULL) return false;
  if((md->name == "config")) return true;
  return false;
}

void gpivSelectEditDialog::Constr_Labels_impl(const MemberSpace& ms) {
  taivEditDialog::Constr_Labels_impl(ms);
  int i;
  for(i=0; i< sele->members.size; i++) {
    MemberDef* md = sele->members.FastEl(i);
    ivMenu* rval = wkit->menubar();
    ivMenu* dscm = wkit->pulldown();
    String nm = sele->GetMbrLabel(i);
    ivMenuItem* newlab = wkit->menubar_item
      (layout->hfixed(taivM->lft_sep
		      (new ivLabel(nm, taivM->name_font, taivM->font_foreground)),
		      2.5f*hfix));	// allow for extra wide!
    newlab->menu(dscm);
    rval->append_item(newlab);
    GetMembDescRep(md, dscm, "");
    labels->append(layout->fixed(taivM->top_sep(rval), 2.5f*hfix + 32, vfix));
  }
}

void gpivSelectEditDialog::Constr_Data_impl(const MemberSpace& ms, taivDataList& dl) {
  taivEditDialog::Constr_Data_impl(ms, dl);

  int i;
  for(i=0; i<sele->members.size; i++) {
    MemberDef* md = sele->members.FastEl(i);
    taivData* mb_dat = md->iv->GetDataRep(dialog, this);
    dl.Add(mb_dat);    data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()),vfix));
  }
}

void gpivSelectEditDialog::GetValue() {
  GetValue_impl(typ->members, data_el, cur_base);
  sele->UpdateAllBases();
  GetButtonImage();
  Unchanged();
}

void gpivSelectEditDialog::GetValue_impl(const MemberSpace& ms, const taivDataList& dl,
					 void* base)
{
  int i, cnt = 0;
  bool first_diff = true;
  for(i=0; i<ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md)) continue;
    taivData* mb_dat = dl.FastEl(cnt++);
    md->iv->GetMbrValue(mb_dat, base, first_diff);
  }
  if(!first_diff) {		// end the basic guy
    taivMember::EndScript(base);
    first_diff = true;
  }

  for(i=0; i<sele->members.size; i++) {
    MemberDef* md = sele->members.FastEl(i);
    TAPtr bs = sele->mbr_bases.FastEl(i);
    if(bs == NULL) continue;
    taivData* mb_dat = dl.FastEl(cnt++);
    md->iv->GetMbrValue(mb_dat, (void*)bs, first_diff);
    if(!first_diff) {		// always reset!
      taivMember::EndScript((void*)bs);
      first_diff = true;
    }
  }
}

void gpivSelectEditDialog::GetImage_impl(const MemberSpace& ms, const taivDataList& dl, 
					 void* base, ivWindow* win)
{
  int i, cnt = 0;
  for(i=0; i<ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = dl.FastEl(cnt++);
    md->iv->GetImage(mb_dat, base, win);
  }

  for(i=0; i<sele->members.size; i++) {
    MemberDef* md = sele->members.FastEl(i);
    TAPtr bs = sele->mbr_bases.FastEl(i);
    if(bs == NULL) continue;
    taivData* mb_dat = dl.FastEl(cnt++);
    md->iv->GetImage(mb_dat, (void*)bs, win);
  }
}

#include <ta/enter_iv.h>
declareActionCallback(taivMethMenu)
#include <ta/leave_iv.h>

void gpivSelectEditDialog::Constr_Methods() {
  int i;
  for(i=0; i<sele->methods.size; i++) {
    MethodDef* md = sele->methods.FastEl(i);
    if(md->iv == NULL) continue;
    taivMethMenu* mth_rep = md->iv->GetMethodRep(sele->meth_bases.FastEl(i), this);
    if(mth_rep == NULL) continue;
    meth_el.Add(mth_rep);
    String nm = sele->GetMethLabel(i);
    if(mth_rep->is_menu_item) {
      GetMenuRep(md);

      // mth_rep->AddToMenu(cur_menu);
      if(md->HasOption("MENU_SEP_BEFORE"))
	cur_menu->AddSep();
      cur_menu->AddItem(nm, NULL, taivMenu::use_default, 
			new ActionCallback(taivMethMenu)(mth_rep, &taivMethMenu::CallFun));
      if(md->HasOption("MENU_SEP_AFTER"))
	cur_menu->AddSep();
    }
    else {
      taivMethButton* but_rep = (taivMethButton*)mth_rep;
      if(meth_buts == NULL)
	GetButtonRep();

      ivResource::unref(but_rep->rep);
      but_rep->rep =taivM->wkit->push_button
	(nm, new ActionCallback(taivMethMenu)(but_rep, &taivMethMenu::CallFun));
      ivResource::ref(but_rep->rep);
      ivGlyph* look = taivM->layout->margin
	(taivM->small_flex_button(((ivButton *) but_rep->rep)), taivM->hsep_c);
      meth_buts->append(look);
    }
  }
  taivEditDialog::Constr_Methods();
}
