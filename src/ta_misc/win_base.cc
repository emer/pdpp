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

// win_base.cc

#include <ta_misc/win_base.h>
#include <ta/ta_group_iv.h>
#include <ta_misc/script_base.h>
#include <css/css_iv.h>
// following are for mkdir
#include <sys/stat.h>
#include <sys/types.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/place.h>     // for resizelayout
#include <InterViews/superpose.h> // for resizelayout
#include <InterViews/patch.h>
#include <InterViews/printer.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/display.h> // for display size limitations
#include <InterViews/cursor.h> // for record_cursor
#include <InterViews/handler.h> // for wm_delete
#ifndef CYGWIN
#include <IV-X11/xwindow.h>		// this is for the window title hack
#endif
#include <InterViews/session.h>
#include <InterViews/background.h>
#include <ta/leave_iv.h>

//////////////////////////
//	winbMisc	//
//////////////////////////

ivManagedWindow* 	winbMisc::group_leader = NULL;
taBase_List		winbMisc::update_menus;
taBase_List		winbMisc::update_winpos;
taBase_List		winbMisc::unopened_windows;


void winbMisc::DamageCanvas(ivCanvas* can){
#ifndef CYGWIN
  if((can != NULL) && (can->status() == ivCanvas::mapped))
#else
  if((can != NULL))
#endif
    can->damage_all();
}

void winbMisc::DamageWindow(ivWindow* win){
  if(win != NULL) DamageCanvas(win->canvas());
}


void winbMisc::OpenWindows(){
  if(unopened_windows.size == 0)	return;

  taMisc::is_loading = true;
  int i;
  for(i=0;i < unopened_windows.size;i++){
    WinBase* win =   ((WinBase *) unopened_windows.FastEl(i));
    win->OpenNewWindow();
    win->UpdateAfterEdit();
  }
  taivMisc::RunIVPending();
  taivMisc::RunIVPending();
  for(i=0;i < unopened_windows.size;i++){
    WinBase* win =   ((WinBase *) unopened_windows.FastEl(i));
    if(win->iconified) win->Iconify();
  }
  unopened_windows.RemoveAll();
  taMisc::is_loading = false;
  cssivSession::RaiseObjEdits(); // make sure css objects are in front!

  for(i=0;i<10;i++)
    taivMisc::RunIVPending();
  for(i=taivMisc::active_edits.size-1; i>=0; i--) {
    taivEditDialog* dlg = (taivEditDialog*)taivMisc::active_edits.FastEl(i);
    if((dlg->typ != NULL) && dlg->typ->InheritsFrom(TA_SelectEdit) && (dlg->state == taivDialog::ACTIVE))
      dlg->Raise();
  }
  for(i=0;i<10;i++)
    taivMisc::RunIVPending();
}

int winbMisc::WaitProc() {
  if(tabMisc::WaitProc())
    return true;

  if(!taMisc::iv_active)    return false;

  if(update_winpos.size > 0) {
    Wait_UpdateWinPos();
    return true;
  }
  if(update_menus.size > 0) {
    Wait_UpdateMenus();
    return true;
  }
  return Script::Wait_RecompileScripts();
}

void winbMisc::Wait_UpdateMenus() {
  if(!taMisc::iv_active)    return;
  taMisc::Busy();
  int i;
  for(i=0; i<update_menus.size; i++) {
    TAPtr it = update_menus.FastEl(i);
    if(taBase::GetRefn(it) == 0) {
      taMisc::Error("*** Object is not owned:", it->GetName(), "of type:",
		     it->GetTypeDef()->name);
      taBase::Ref(it);
    }
    MenuUpdate(it);
  }
  update_menus.RemoveAll();
  taMisc::DoneBusy();
}

void winbMisc::Wait_UpdateWinPos() {
  if(!taMisc::iv_active)    return;
  taMisc::Busy();
  int i;
  for(i=0; i<update_winpos.size; i++) {
    TAPtr it = update_winpos.FastEl(i);
    if(taBase::GetRefn(it) == 0) {
      taMisc::Error("*** Object is not owned:", it->GetName(), "of type:",
		     it->GetTypeDef()->name);
      taBase::Ref(it);
    }
    if(it->InheritsFrom(&TA_WinBase)) {
      ((WinBase*)it)->SetWinPos();	// extra safe..
    }
  }
  update_winpos.RemoveAll();
  taMisc::DoneBusy();
}

void winbMisc::DelayedMenuUpdate(TAPtr obj) {
  if(!taMisc::iv_active)    return;
  if(taBase::GetRefn(obj) == 0) {
    taMisc::Error("*** Object is not owned:", obj->GetName(), "of type:",
		  obj->GetTypeDef()->name);
    taBase::Ref(obj);
  }
  update_menus.LinkUnique(obj);
}

void winbMisc::MenuUpdate(TAPtr obj) {
  if(!taMisc::iv_active)	return;

  taMisc::Busy();

  // try to update the menu group (less impact)
  MenuGroup_impl* mg;
  if(obj->InheritsFrom(TA_MenuGroup_impl))
    mg = (MenuGroup_impl*)obj;
  else
    mg = GET_OWNER(obj,MenuGroup_impl);

  TAPtr ownr = mg;
  // get the highest menugroup
  if((mg != NULL) && (mg->owner != NULL) &&
     mg->owner->InheritsFrom(TA_taList_impl))
  {
    ownr = mg->owner;
    while((ownr != NULL) && (ownr->GetOwner() != NULL) &&
	  ownr->GetOwner()->InheritsFrom(TA_taList_impl)) {
      if(ownr->InheritsFrom(TA_MenuGroup_impl))
	mg = (MenuGroup_impl*)ownr;
      ownr = ownr->GetOwner();
      if(ownr->InheritsFrom(TA_MenuGroup_impl))	// always try to get a mgroup
	mg = (MenuGroup_impl*)ownr;
    }
  }

  if((mg != NULL) && (mg->ta_menu != NULL)) {
    mg->UpdateMenu();
  }
  else {
    if(ownr != NULL)		// get the owner of the highest group
      ownr = ownr->GetOwner();

    // update it if it is a winbase..
    if((ownr != NULL) && ownr->InheritsFrom(&TA_WinBase)) {
      ((WinBase*)ownr)->UpdateMenus();
      taMisc::DoneBusy();
      return;
    }
    else if((ownr != NULL) && ownr->HasOption("MEMB_IN_GPMENU")) {
      // if it has a IN_GPMENU, it might be that we are in a subgroup of it
      // so go ahead and update that object instead of us
      taMisc::DoneBusy();
      MenuUpdate(ownr);
      return;
    }

    // if all else fails, then update based on the first winbase
    WinBase* we = GET_OWNER(obj,WinBase);
    if(we != NULL)
      we->UpdateMenus();
    else if(obj->InheritsFrom(TA_WinBase)) // if nothing else, do object itself
      ((WinBase*)obj)->UpdateMenus();
  }

  taMisc::DoneBusy();
}

int winbMisc::SetIconify(void* obj, int onoff){
  WinBase* wb = (WinBase *) obj;
  if(onoff != -1) {
    if(!onoff && !wb->IsMapped())
      return wb->iconified;	// attempt to update iconified (non-mapped) window!
    if(!onoff && wb->iconified)	// switching from iconfied to not, update menus
      DelayedMenuUpdate(wb);	// update menus when de-iconifying
    wb->iconified = onoff;
  }
  return (int) wb->iconified;
}

void winbMisc::ScriptIconify(void*, int) {
// do nothing, use script win pos to record final iconify status
//  WinBase* wb = (WinBase *) obj;
//   if(onoff)
//     taivMisc::RecordScript(wb->GetPath() + ".Iconify();\n");
//   else
//     taivMisc::RecordScript(wb->GetPath() + ".DeIconify();\n");
}


//////////////////////////
//	MenuGroup	//
//////////////////////////

void MenuGroup_impl::Initialize() {
  win_owner = NULL;
  ta_menu = NULL;
  ta_file = NULL;
  itm_list = NULL;
  grp_list = NULL;
}

void MenuGroup_impl::Destroy() {
  if(ta_menu != NULL)
    delete ta_menu;
  if(ta_file != NULL)
    taRefN::unRefDone(ta_file);
  ta_menu = NULL;
  ta_file = NULL;
  if(itm_list != NULL) { delete itm_list; itm_list = NULL; }
  if(grp_list != NULL) { delete grp_list; grp_list = NULL; }
}

void MenuGroup_impl::InitLinks() {
  MENU_GROUP_PARENT::InitLinks();
  win_owner = GET_MY_OWNER(WinBase);
}

ivMenuItem* MenuGroup_impl::GetMenuItem() {
  if(ta_menu != NULL)
    return ta_menu->GetMenuItem();
  return NULL;
}

bool MenuGroup_impl::Close_Child(TAPtr obj) {
  for(int i=winbMisc::update_menus.size-1;i>=0;i--) {
    TAPtr it = winbMisc::update_menus.FastEl(i);
    char* objmin = (char*)obj;
    char* objmax = (char*)obj + obj->GetTypeDef()->size;
    char* itc = (char*)it;
    if((itc >= objmin) && (itc < objmax))
      winbMisc::update_menus.Remove(i);	// remove this item from the list -- it has just been deleted!!
  }
  bool rval = Remove(obj);
  winbMisc::DelayedMenuUpdate(this);
  return rval;
}

declare_taivMenuCallback(MenuGroup_impl)
implement_taivMenuCallback(MenuGroup_impl)

taivHierMenu* MenuGroup_impl::GetMenu() {
  if(!taMisc::iv_active)  return NULL;
  if(ta_menu != NULL)
    delete ta_menu;
  GetMenu_impl();
  return ta_menu;
}

void MenuGroup_impl::GetMenu_impl() {
  ta_menu = new taivHierMenu(taivMenu::menuitem, taivMenu::normal, taivMenu::big);
  String menuname = el_typ->GetLabel();
  if(owner != NULL) {
    MemberDef* md = owner->FindMember(this);
    if(md != NULL) {
      menuname = ".";		// give it the member name
      menuname += md->name;
    }
  }
  ta_menu->SetMLabel(menuname);

  gpivElTypes* typ_list = new gpivElTypes(ta_menu, el_base, GetTypeDef());
  if(itm_list != NULL)
    delete itm_list;
  itm_list = new gpivGroupEls(ta_menu, this);
  if(grp_list != NULL)
    delete grp_list;
  grp_list = new gpivSubGroups(ta_menu, this);
  int cur_no = ta_menu->cur_sno;

  ta_menu->AddSubMenu("Edit");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Edit_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("New");
  typ_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::New_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Open In");
  grp_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Open_in_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Load Over");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this,&MenuGroup_impl::Load_over_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Save");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Save_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Save As");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::SaveAs_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Remove");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Remove_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Duplicate");
  itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Duplicate_mc));
  ta_menu->SetSub(cur_no);

  ta_menu->AddSubMenu("Move Within");
  grp_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		    (this, &MenuGroup_impl::Move_mc));
  ta_menu->SetSub(cur_no);

  if(el_base->InheritsFrom(TA_WinBase)) {
    ta_menu->AddSubMenu("View Window");
    itm_list->GetMenu(new taivMenuCallback(MenuGroup_impl)
		      (this, &MenuGroup_impl::View_mc));
    ta_menu->SetSub(cur_no);
  }

  delete typ_list;
}

void MenuGroup_impl::UpdateMenu() {
  if(!taMisc::iv_active || (ta_menu == NULL) || (win_owner == NULL)) return;
  if(win_owner->menu == NULL) return;
  ivMenuItem* mymi = GetMenuItem();
  taivHierMenu* old_men = ta_menu;
  int i;
  for(i=0; i<win_owner->menu->item_count(); i++) {
    if(win_owner->menu->item(i) == mymi) {
      ivMenuItem* temp_mi = new ivMenuItem(NULL,new ivTelltaleState());
      win_owner->menu->replace_item(i,temp_mi);
      delete old_men;
      ivResource::flush();
      GetMenu_impl();
      win_owner->menu->replace_item(i, GetMenuItem());
      ivResource::flush();
      break;
    }
  }
  UpdateElMenus();
}

void MenuGroup_impl::UpdateElMenus() {
  if(!taMisc::iv_active) return;
  if(!el_base->InheritsFrom(TA_WinBase))
    return;			// not a win-base group
  taLeafItr li;
  taBase* ta;
  FOR_ITR_EL(taBase, ta, this->, li) {
    if(ta->InheritsFrom(TA_WinBase)) {
      WinBase* wb = (WinBase*)ta;
      if(wb->IsMapped())	// only update mapped windows!
	wb->UpdateMenus_impl();
    }
  }
}

void MenuGroup_impl::GetAllWinPos() {
  if(!taMisc::iv_active) return;
  if(!el_base->InheritsFrom(TA_WinBase))
    return;			// not a win-base group
  taLeafItr li;
  taBase* ta;
  FOR_ITR_EL(taBase, ta, this->, li) {
    if(ta->InheritsFrom(TA_WinBase)) {
      WinBase* wb = (WinBase*)ta;
      wb->GetAllWinPos();
    }
  }
}

void MenuGroup_impl::ScriptAllWinPos() {
  if(!taMisc::iv_active) return;
  if(!el_base->InheritsFrom(TA_WinBase))
    return;			// not a win-base group
  taLeafItr li;
  taBase* ta;
  FOR_ITR_EL(taBase, ta, this->, li) {
    if(ta->InheritsFrom(TA_WinBase)) {
      WinBase* wb = (WinBase*)ta;
      wb->ScriptAllWinPos();
    }
  }
}

void MenuGroup_impl::New_mc(taivMenuEl* sel) {
  if(win_owner == NULL) return;
  taMisc::Busy();
  if(ta_file != NULL)
    ta_file->fname = "";	// reset file name for new object so no auto-save
  TAPtr rval = NULL;
  if((sel != NULL) && (sel->usr_data != NULL))
    rval = gpivGroupNew::New(this, (TypeDef*)sel->usr_data);
  else
    rval = gpivGroupNew::New(this, NULL);
  if(rval != NULL) {
    win_owner->UpdateAfterEdit();
    winbMisc::DelayedMenuUpdate(this);
  }
  taMisc::DoneBusy();
}

void MenuGroup_impl::Duplicate_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;

  taMisc::Busy();
  TAPtr itm = (TAPtr)sel->usr_data;
  TAPtr nw_itm = NULL;
  DMEM_GUI_RUN_IF {
    nw_itm = (TAPtr)itm->MakeToken(); // first just make a token, then add
  }
  if(itm == (TAPtr)this) {		  // when you duplicate this group..
    DMEM_GUI_RUN_IF {
      MenuGroup_impl* nmg = (MenuGroup_impl*)nw_itm;
      nmg->owner = &gp;		// will be owned by my subgroup
      nmg->InitLinks();		// initialize links as-if owned
      nw_itm->UnSafeCopy(itm);	// replicate _before_ adding into group
      gp.Add((taGroup_impl*)nw_itm);	  // add it as a subgroup of self
      winbMisc::DelayedMenuUpdate(nmg);
    }
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << gp.GetPath() << "->DuplicateEl("
			       << itm->GetPath() << ");" << endl;
    }
  }
  else if(itm->InheritsFrom(TA_taGroup_impl)) {
    taSubGroup* sgp = GET_OWNER(itm,taSubGroup);
    if(sgp != NULL) {
      DMEM_GUI_RUN_IF {
	sgp->Add((taGroup_impl*)nw_itm);
      }
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << sgp->GetPath() << "->DuplicateEl("
				 << itm->GetPath() << ");" << endl;
      }
      winbMisc::DelayedMenuUpdate(sgp);
    }
    else {
      DMEM_GUI_RUN_IF {
	gp.Add((taGroup_impl*)nw_itm);
      }
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << gp.GetPath() << "->DuplicateEl("
				 << itm->GetPath() << ");" << endl;
      }
      winbMisc::DelayedMenuUpdate(this);
    }
    DMEM_GUI_RUN_IF {
      nw_itm->UnSafeCopy(itm);	// replicate after adding into group
    }
  }
  else {
    TAGPtr itm_owner = (TAGPtr)itm->GetOwner(&TA_taGroup_impl);
    if(itm_owner != NULL) {
      DMEM_GUI_RUN_IF {
	itm_owner->Add(nw_itm);
      }
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << itm_owner->GetPath() << "->DuplicateEl("
				 << itm->GetPath() << ");" << endl;
      }
      winbMisc::DelayedMenuUpdate(itm_owner);
    }
    else {
      DMEM_GUI_RUN_IF {
	Add(nw_itm);
      }
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << GetPath() << "->DuplicateEl("
				 << itm->GetPath() << ");" << endl;
      }
      winbMisc::DelayedMenuUpdate(this);
    }
    DMEM_GUI_RUN_IF {
      nw_itm->UnSafeCopy(itm);	// replicate after adding into group
    }
  }
  win_owner->UpdateAfterEdit();
  taMisc::DoneBusy();
//  taivEdit* gc = (taivEdit*)nw_itm->GetTypeDef()->ive; // also edit it..
//  gc->Edit((void*)nw_itm, win_owner->window, false);
}

void MenuGroup_impl::Open_in_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;

  taivGetFile* taf;
  TAPtr itm = (TAPtr)sel->usr_data;
  if(itm->InheritsFrom(&TA_WinBase)) {
    ((WinBase*)itm)->GetFileDlg();
    taf = ((WinBase*)itm)->ta_file;
  }
  else {
    win_owner->GetFileDlg();
    taf = ta_file;
  }
  String defpth = GetTypeDef()->OptionAfter("DEF_PATH_");
  if(!defpth.empty()) {
    if(defpth.contains('$')) {
      String envar = defpth.after('$');
      envar = envar.before('$');
      char* envval = getenv(envar);
      if(envval == NULL) 
	envval = "/usr/local/pdp++"; // todo: lame to have hard-coded default, but..
      envar = String("$") + envar + "$";
      defpth.gsub((const char*)envar, envval);
    }
    taf->dir = defpth;
    if(taf->fname.empty()) taf->fname = defpth;
  }
  istream* strm = taf->Open();
  if((strm != NULL) && strm->good()) {
    DMEM_GUI_RUN_IF {
      taMisc::Busy();
      taivMisc::CreateLoadDialog();
      itm->Load(*strm);
      winbMisc::DelayedMenuUpdate(itm);
    }
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->Load(\""
			       << taf->fname << "\");" << endl;
    }
    DMEM_GUI_RUN_IF {
      taivMisc::RemoveLoadDialog();
      winbMisc::OpenWindows();
      taMisc::DoneBusy();
      if(itm->InheritsFrom(&TA_WinBase))
	((WinBase*)itm)->SetFileName(taf->fname);
    }
  }
  taf->Close();
  win_owner->UpdateAfterEdit();
}


void MenuGroup_impl::Load_over_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;

  taivGetFile* taf;
  TAPtr itm = (TAPtr)sel->usr_data;
  if(itm->InheritsFrom(&TA_WinBase)) {
    ((WinBase*)itm)->GetFileDlg();
    taf = ((WinBase*)itm)->ta_file;
  }
  else {
    win_owner->GetFileDlg();
    taf = ta_file;
  }
  String defpth = GetTypeDef()->OptionAfter("DEF_PATH_");
  if(!defpth.empty()) {
    if(defpth.contains('$')) {
      String envar = defpth.after('$');
      envar = envar.before('$');
      char* envval = getenv(envar);
      if(envval == NULL) 
	envval = "/usr/local/pdp++"; // todo: lame to have hard-coded default, but..
      envar = String("$") + envar + "$";
      defpth.gsub((const char*)envar, envval);
    }
    taf->dir = defpth;
    if(taf->fname.empty()) taf->fname = defpth;
  }
  DMEM_GUI_RUN_IF {
    taMisc::Busy();
  }
  istream* strm = taf->Open();
  if((strm != NULL) && strm->good()) {
    DMEM_GUI_RUN_IF {
      itm->Load(*strm);
      winbMisc::DelayedMenuUpdate(itm);
    }
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->Load(\""
			       << taf->fname << "\");" << endl;
    }
    if(itm->InheritsFrom(&TA_WinBase))
      ((WinBase*)itm)->SetFileName(taf->fname);
  }
  taf->Close();
  DMEM_GUI_RUN_IF {
    taMisc::DoneBusy();
  }
  win_owner->UpdateAfterEdit();
}

void MenuGroup_impl::Save_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;

  DMEM_GUI_RUN_IF {
    taMisc::Busy();
  }
  taivGetFile* taf;
  TAPtr itm = (TAPtr)sel->usr_data;
  if(itm->InheritsFrom(&TA_WinBase)) {
    ((WinBase*)itm)->GetFileDlg();
    taf = ((WinBase*)itm)->ta_file;
  }
  else {
    win_owner->GetFileDlg();
    taf = ta_file;
  }
  ostream* strm = taf->Save();
  if((strm != NULL) && strm->good()) {
    if(itm->InheritsFrom(&TA_WinBase))
      ((WinBase*)itm)->SetFileName(taf->fname);

    DMEM_GUI_RUN_IF {
      itm->Save(*strm);
    }

    if(itm->InheritsFrom(&TA_WinBase))
      ((WinBase*)itm)->SetWinName();
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->Save(\"" << taf->fname << "\");" << endl;
    }
  }
  taf->Close();
  DMEM_GUI_RUN_IF {
    taMisc::DoneBusy();
  }
}

void MenuGroup_impl::SaveAs_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;

  DMEM_GUI_RUN_IF {
    taMisc::Busy();
  }
  taivGetFile* taf;
  TAPtr itm = (TAPtr)sel->usr_data;
  if(itm->InheritsFrom(&TA_WinBase)) {
    ((WinBase*)itm)->GetFileDlg();
    taf = ((WinBase*)itm)->ta_file;
  }
  else {
    win_owner->GetFileDlg();
    taf = ta_file;
  }
  ostream* strm = taf->SaveAs();
  if((strm != NULL) && strm->good()) {
    if(itm->InheritsFrom(&TA_WinBase))
      ((WinBase*)itm)->SetFileName(taf->fname);

    DMEM_GUI_RUN_IF {
      itm->SaveAs(*strm);
    }

    if(itm->InheritsFrom(&TA_WinBase))
      ((WinBase*)itm)->SetWinName();
    if(taivMisc::record_script != NULL) {
      *taivMisc::record_script << itm->GetPath() << "->Save(\"" << taf->fname << "\");" << endl;
    }
  }
  taf->Close();
  DMEM_GUI_RUN_IF {
    taMisc::DoneBusy();
  }
}

void MenuGroup_impl::Remove_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;
  if(taMisc::Choice("Ok to Close (PERMANENTLY Destroy this object)?", "Ok", "Cancel") == 1) 	return;

  winbMisc::DelayedMenuUpdate(this);

  TAPtr it = (TAPtr)sel->usr_data;

  if(it->InheritsFrom(TA_taGroup_impl)) {
    TAGPtr tg = (TAGPtr)it;
    taSubGroup* own_g = NULL;
    if((tg->owner != NULL) && tg->owner->InheritsFrom(TA_taSubGroup))
      own_g = (taSubGroup*)tg->owner;
    if(tg->el_typ->InheritsFrom(TA_WinView)) {
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << tg->GetPath() << "->Close();" << endl;
      }
      DMEM_GUI_RUN_IF {
	tabMisc::Close_Obj(tg); // might cause current window to die..
      }
    }
    else {
      if((tg->leaves == 0) && (own_g != NULL)) {
	if(taivMisc::record_script != NULL) {
	  *taivMisc::record_script << own_g->GetPath() << "->RemoveEl("
				   << tg->GetPath() << ");" << endl;
	}
	taivMisc::CloseEdits((void*)tg, tg->GetTypeDef());
	DMEM_GUI_RUN_IF {
	  own_g->Remove(tg);	// get rid of sub group
	}
      }
      else {
	if(taivMisc::record_script != NULL) {
	  *taivMisc::record_script << tg->GetPath() << "->RemoveAll();" << endl;
	}
	taivMisc::CloseEdits((void*)tg, tg->GetTypeDef());
	DMEM_GUI_RUN_IF {
	  tg->RemoveAll();
	}
      }
    }
  }
  else {
    if(it->InheritsFrom(TA_WinView)) {
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << it->GetPath() << "->Close();" << endl;
      }
      DMEM_GUI_RUN_IF {
	tabMisc::Close_Obj(it);
      }
    }
    else {
      TAGPtr tg = (TAGPtr)it->GetOwner(&TA_taGroup_impl);
      if(tg != NULL) {
	if(taivMisc::record_script != NULL) {
	  *taivMisc::record_script << tg->GetPath() << "->RemoveEl("
				   << it->GetPath() << ");" << endl;
	}
	taivMisc::CloseEdits((void*)it, it->GetTypeDef());
	DMEM_GUI_RUN_IF {
	  tg->Remove(it);
	}
      }
    }
  }
  win_owner->UpdateAfterEdit();
}

void MenuGroup_impl::Edit_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;
  TAPtr itm = (TAPtr)sel->usr_data;
  itm->Edit(false);
}


void MenuGroup_impl::Move_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;
  TAGPtr itm = (TAGPtr)sel->usr_data;

  MethodDef* md = itm->GetTypeDef()->methods.FindName("MoveEl");
  if((md == NULL) || (md->iv == NULL)) {
    taMisc::Error("MoveEl method not found in group");
    return;
  }
  taivMethMenu* mth_rep = md->iv->GetMethodRep((void*)itm, NULL);
  if(mth_rep != NULL) {
    mth_rep->CallFun();
    delete mth_rep;
  }
}

void MenuGroup_impl::View_mc(taivMenuEl* sel) {
  if((sel == NULL) || (sel->usr_data == NULL))
    return;
  if(win_owner == NULL) return;
  TAPtr itm = (TAPtr)sel->usr_data;
  if(itm->InheritsFrom(TA_WinBase)) {
    WinBase* wb = (WinBase*)itm;
    wb->ViewWindow();
  }
  else if(itm->InheritsFrom(TA_MenuGroup_impl)) {
    MenuGroup_impl* mg = (MenuGroup_impl*)itm;
    if(!mg->el_base->InheritsFrom(TA_WinBase)) {
      itm->Edit(false);
      return;
    }
    taLeafItr li;
    WinBase* wb;
    FOR_ITR_EL(WinBase, wb, mg->, li) {
      wb->ViewWindow();
    }
  }
  else
    itm->Edit(false);
}


//////////////////////////
//	WinGeometry	//
//////////////////////////

void WinGeometry::Initialize() {
  owner = NULL;
  lft = bot = wd = ht = 0.0f;
}

void WinGeometry::Copy_(const WinGeometry& cp) {
  lft = cp.lft;
  bot = cp.bot;
  wd = cp.wd;
  ht = cp.ht;
}

void WinGeometry::GetWinPos() {
  if((owner == NULL) || (owner->window == NULL))
    return;

  if(owner->window->is_mapped()) {
    lft = owner->window->left();
    bot = owner->window->bottom();
    wd = owner->window->width();
    ht = owner->window->height();
  }
}

void WinGeometry::SetWinPos() {
  if((owner == NULL) || (owner->window == NULL))
    return;

  if((wd != 0) && (ht != 0))	// resize before moving..
    owner->ReSize(wd,ht);

  if((lft != 0) && (bot != 0)) {
    owner->Place(lft, bot);
  }
}

void WinGeometry::ScriptWinPos(ostream& strm) {
  if(owner == NULL)
    return;
  GetWinPos();
  String temp = owner->GetPath();
  if(owner->iconified) {
    temp += ".Iconify();\n";
  }
  else {
    temp += String(".SetWinPos(") + String(lft) + ", " +
      String(bot) + ", " + String(wd) + ", " + String(ht) + ");\n";
  }
  if(taivMisc::record_script != NULL)  taivMisc::RecordScript(temp);
  else   strm << temp;
}

void WinGeometry::UpdateAfterEdit() {
  if((owner != NULL) && (taMisc::iv_active))
    winbMisc::update_winpos.LinkUnique(owner);
}


//////////////////////////
//	WinBase		//
//////////////////////////

void WinBase::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  if(!taMisc::iv_active || (window == NULL)) return;
// this is a bad idea -- iconifies to quickly upon loading!
// need to let window open then iconify it -- otherwise get zombies
// and it breaks CYGWIN
//  if(iconified) Iconify();
  SetWinName();
}

int WinBase::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = taNBase::Dump_Load_Value(strm, par);
  taivMisc::RunIVPending();
  return rval;
}
  
void WinBase::Initialize() {
  win_owner = NULL;
  wkit = NULL;
  dkit = NULL;
  layout = NULL;
  window = NULL;
  win_box = NULL;
  main_box = NULL;
  this_menus = new taivMenu_List;
  this_meths = new taivDataList;
  menu = NULL;
  body = NULL;
  ta_file= NULL;
  print_file = NULL;
  iconified = false;
}

void WinBase::InitLinks() {
  taNBase::InitLinks();
  WinBase* tmp = GET_MY_OWNER(WinBase);
  if(tmp == this)
    tmp = NULL;
  win_owner = tmp;
  taBase::Own(win_pos, this);
  WinInit();			// initialize after linking, so that path is available
  if(owner && (!owner->InheritsFrom(&TA_TypeDefault))) { // no windows for defaults
    if(taMisc::is_loading)
      winbMisc::unopened_windows.LinkUnique(this);
    else
      OpenNewWindow();		// and open a window
  }
  GetFileDlg();
}


void WinBase::Copy_(const WinBase& cp) {
  wkit = cp.wkit;
  dkit = cp.dkit;
  layout = cp.layout;
  win_pos = cp.win_pos;
  win_pos.lft += 20; 
  win_pos.bot -= 20;
  winbMisc::update_winpos.LinkUnique(this);
  if(!name.empty())
    name += "_copy";		// only label winbases as copies
}

void WinBase::WinInit() {
  if(!taMisc::iv_active) return;
  wkit = ivWidgetKit::instance();
  dkit = ivDialogKit::instance();
  layout = ivLayoutKit::instance();
}

void WinBase::Destroy() {
  if(ta_file != NULL)    taRefN::unRefDone(ta_file);  ta_file = NULL;
  if(this_menus != NULL)    delete this_menus;  this_menus = NULL;
  if(this_meths != NULL)    delete this_meths;  this_meths = NULL;
  CutLinks();
}

void WinBase::CutLinks() {
  taivMisc::CloseEdits((void*)this, GetTypeDef());
  CloseWindow();
  win_owner = NULL;
  taNBase::CutLinks();
}

void WinBase::Iconify() {
  if(!taMisc::iv_active || (window == NULL)) return;
  window->iconify();
  iconified = true;
}

void WinBase::DeIconify() {
  if(!taMisc::iv_active || (window == NULL)) return;
  iconified = false;
  window->deiconify();
#ifdef CYGWIN
  Place(win_pos.lft, win_pos.bot); 
#endif
}

bool WinBase::IsMapped() {
  if(!taMisc::iv_active || (window == NULL)) return false;
  return window->is_mapped();
}

void WinBase::ViewWindow(float left, float bottom, float width, float height) {
  if(!taMisc::iv_active) return;
  if(!window) {
    OpenNewWindow();
    if(((left != 0) && (bottom != 0)) || ((width != 0) && (height != 0)))
      SetWinPos(left, bottom, width, height);
  }
  else {
    DeIconify();
    Raise();
  }
}

void WinBase::SetWinPos(float left, float bottom, float width, float height) {
  bool was_set = false;
  if((left != 0) && (bottom != 0)) {
    win_pos.lft = left;
    win_pos.bot = bottom;
    was_set = true;
  }
  if((width != 0) && (height != 0)) {
    win_pos.wd = width;
    win_pos.ht = height;
    was_set = true;
  }
  if(was_set) {
    win_pos.UpdateAfterEdit();	// puts it on the update_winpos list
  }
  else {
    win_pos.SetWinPos();	// does it now
  }
}

class ResizeLayout : public ivLayout {
public:
  ResizeLayout(const DimensionName, ivCoord natural);
  void request(ivGlyphIndex count, const ivRequisition*, ivRequisition& result);
protected:
  DimensionName dimension_;
  ivCoord natural_;
};

ResizeLayout::ResizeLayout(const DimensionName d, ivCoord natural) {
    dimension_ = d;
    natural_ = natural;
}

void ResizeLayout::request(ivGlyphIndex,
			   const ivRequisition*, ivRequisition& result){
   ivRequirement& r = result.requirement(dimension_);
   ivCoord nat = r.natural();
   ivCoord shr = r.shrink();
   ivCoord str = r.stretch();
   if(nat < natural_){
     r.shrink(shr + (natural_ - nat));
   }
   if(nat > natural_){
     r.stretch(str + (nat - natural_));
   }
   r.natural(natural_);
}

class GetWinPosPatch : public ivPatch {
public:
  WinBase*	obj;

  GetWinPosPatch(WinBase* wb, ivGlyph* g) : ivPatch(g) { obj = wb; }

  void 	allocate(ivCanvas* c, const ivAllocation& a, ivExtension& e) {
    ivPatch::allocate(c, a, e);
     if(obj->IsMapped()) {
      obj->win_pos.wd = obj->window->width();
#ifndef CYGWIN
      obj->win_pos.ht = obj->window->height();
#else
      obj->win_pos.ht = obj->window->height() - 20.0f; // subtract out window bar
#endif
    }
  }
};

void WinBase::ReSize(float width, float height) {
  if(!taMisc::iv_active || (window == NULL)) return;
  if(!window->is_mapped())
    return;
  if((width != 0) && (height != 0)) {
//    win_box->body(layout->flexible(layout->natural(main_box, width, height)));
//    win_box->body(layout->natural(main_box, width, height));
    win_box->body(new ivPlacement
		  (main_box,
		   new ivSuperpose
		   (new ResizeLayout(Dimension_X,width),
		    new ResizeLayout(Dimension_Y,height))));
    win_box->reallocate();
    win_pos.wd = width;
    win_pos.ht = height;
  }
  else {
    GetWinPos();
  }
  window->resize();
}

void WinBase::Move(float left, float bottom) {
  // move = place with delta
  Place(win_pos.lft + left, win_pos.bot + bottom); 
}

void WinBase::Place(float left, float bottom){
  if(!taMisc::iv_active || (window == NULL)) return;
  // check to make sure that its not off the screen
  float newleft, newbottom;
  ivDisplay* dsp = ivSession::instance()->default_display();
  newleft =    MIN(left,dsp->width()-win_pos.wd);
  newbottom =  MIN(bottom,dsp->height()-win_pos.ht-20.0f);
  newleft = MAX(newleft, -win_pos.wd + 20.0f);
#ifndef CYGWIN
  newbottom = MAX(newbottom, -win_pos.ht + 20.0f);
#else
  newbottom = MAX(newbottom, -win_pos.ht + 50.0f); // lv room for taskbar!
#endif
  window->move(newleft + taMisc::window_decor_offset_x, newbottom + taMisc::window_decor_offset_y);
  win_pos.lft = newleft;
  win_pos.bot = newbottom;
}

void WinBase::Raise() {
  if(!taMisc::iv_active || (window == NULL)) return;
  window->raise();
}

void WinBase::Lower() {
  if(!taMisc::iv_active || (window == NULL)) return;
  window->lower();
}

void WinBase::GetWinBoxes() {
  main_box = layout->vbox();
  ivResource::ref(main_box);
  
  if((win_pos.ht != 0) && (win_pos.wd != 0)) {
//    win_box = new ivPatch(layout->natural(main_box, win_pos.wd, win_pos.ht));
    win_box = new GetWinPosPatch(this, new ivPlacement
				 (main_box,
				  new ivSuperpose
				  (new ResizeLayout(Dimension_X,win_pos.wd),
				   new ResizeLayout(Dimension_Y,win_pos.ht))));
  }
  else {
    win_box = new GetWinPosPatch(this, main_box);
  }
  ivResource::ref(win_box);

  menu = wkit->menubar();
  ivResource::ref(menu);
}

class WinBaseWMDeleteHandler : public ivHandler {
public:
  WinBase*	win_obj;
  osboolean event(ivEvent& ev);
  WinBaseWMDeleteHandler(WinBase* obj);
};

WinBaseWMDeleteHandler::WinBaseWMDeleteHandler(WinBase* obj) : ivHandler() {
  win_obj = obj;
}

osboolean WinBaseWMDeleteHandler::event(ivEvent&) {
  if(win_obj->InheritsFrom(TA_WinView) && (((WinView*)win_obj)->mgr != NULL)) {
    WinMgr* mgr = ((WinView*)win_obj)->mgr;
    int chs = taMisc::Choice("Close (PERMANENTLY Destroy) this VIEW of the object, or destroy the ENTIRE object including all views, losing all unsaved changes?", "Close View", "Close Obj", "Save Then Close Obj", "Cancel");
    if(chs == 3)
      return true;
    else if(chs == 2) {
      taivGetFile* taf = mgr->GetFileDlg();
      ostream* strm = taf->Save();
      if((strm != NULL) && strm->good()) {
	taivMisc::RecordScript(mgr->GetPath() + ".Save(" + taf->fname + ");\n");
	mgr->SetFileName(taf->fname);
	DMEM_GUI_RUN_IF {
	  mgr->Save(*strm);
	}
      }
    }
    else if(chs == 0) {
      taivMisc::RecordScript(win_obj->GetPath() + ".Close();\n");
      DMEM_GUI_RUN_IF {
	tabMisc::Close_Obj(win_obj);
      }
      return true;
    }
    taivMisc::RecordScript(mgr->GetPath() + ".Close();\n");
    DMEM_GUI_RUN_IF {
      tabMisc::Close_Obj(mgr);
    }
  }
  else {
    int chs = taMisc::Choice("Ok to Close (PERMANENTLY Destroy) this object, losing all unsaved changes?", "Close", "Save Then Close", "Cancel");
    if(chs == 2)
      return true;
    else if(chs == 1) {
      taivGetFile* taf = win_obj->GetFileDlg();
      ostream* strm = taf->Save();
      if((strm != NULL) && strm->good()) {
	taivMisc::RecordScript(win_obj->GetPath() + ".Save(" + taf->fname + ");\n");
	win_obj->SetFileName(taf->fname);
	DMEM_GUI_RUN_IF {
	  win_obj->Save(*strm);
	}
      }
    }
    taivMisc::RecordScript(win_obj->GetPath() + ".Close();\n");
    DMEM_GUI_RUN_IF {
      tabMisc::Close_Obj(win_obj);
    }
  }
  return true;
}

void WinBase::GetWindow() {
  IconGlyph* ig = 
    new IconGlyph(new ivBackground(win_box, wkit->background()),NULL,this);
  ig->SetIconify = winbMisc::SetIconify;
  ig->ScriptIconify= winbMisc::ScriptIconify;
  window = new ivTopLevelWindow(ig);
  // group leader seems to screw up KDE -- and nothing else seems to use it anyway
//    if(winbMisc::group_leader != NULL) 
//      ((ivTopLevelWindow*)window)->group_leader(winbMisc::group_leader);
  ivHandler* delh = new WinBaseWMDeleteHandler(this);
  window->wm_delete(delh);
  ig->SetWindow(window);
}

void WinBase::OpenNewWindow() {
  if(!taMisc::iv_active) return;

  if(window != NULL) {
    SetWinName();
    return;
  }
  GetWinBoxes();
  GetWindow();
  GetThisMenus();
  GetMenu();
  GetBodyRep();

  main_box->append(layout->hflexible(menu,fil,0));
  main_box->append(layout->hcenter(body, 0));

  SetWinName();
  if((win_pos.lft != 0) && (win_pos.bot != 0)) {
    window->place(win_pos.lft, win_pos.bot);
  }
  window->map();
  taivMisc::active_wins.Add(window);
}

bool WinBase::ThisMenuFilter(MethodDef* md) {
  if((md->name == "GetAllWinPos") || (md->name == "ScriptAllWinPos") ||
     (md->name == "SetWinPos") || (md->name == "SelectForEdit") ||
     (md->name == "SelectFunForEdit")) return false;
  return true;
}

void WinBase::GetThisMenus_impl(taivMenu_List& ths_men, taivDataList& ths_meth,
			       String prfx)
{
  TypeDef* td = GetTypeDef();
  String men_nm = prfx + "Object";
  taivMenu* cur_menu = NULL;
  bool first_prfx = true;
  int i;
  for(i=0; i<td->methods.size; i++) {
    MethodDef* md = td->methods.FastEl(i);
    if(md->iv == NULL)
      continue;
    if(!ThisMenuFilter(md))
      continue;
    String opt_nm;
    int opt;
    if((opt = md->opts.FindContains("MENU_ON_")) >= 0) {
      opt_nm = md->opts.FastEl(opt).after("MENU_ON_");
    }
    if(!opt_nm.empty()) {
      if(first_prfx) {
	men_nm = prfx + opt_nm;
	first_prfx = false;
      }
      else
	men_nm = opt_nm;
      cur_menu = ths_men.FindName(men_nm);
    }
    if(cur_menu == NULL) {
      cur_menu = new 
	taivMenu(taivMenu::menuitem, taivMenu::normal, taivMenu::big_italic,
		  men_nm);
      ths_men.Add(cur_menu);
    }
    taivMethMenu* mth_rep = md->iv->GetMethodRep((void*)this, NULL);
    ths_meth.Add(mth_rep);
    mth_rep->AddToMenu(cur_menu);
  }
}

// call this only once at the open new window stage.
void WinBase::GetThisMenus() {
  this_menus->Reset();
  this_meths->Reset();

  String prfx = "";
  GetThisMenus_impl(*this_menus, *this_meths, prfx);

  int i;
  for(i=0; i<this_menus->size; i++) {
    taivMenu* mnu = this_menus->FastEl(i);
    menu->append_item(mnu->GetMenuItem());
  }
}

void WinBase::GetMenu() {
  if(!taMisc::iv_active) return;

  TypeDef* td = GetTypeDef();
  int i, cnt = this_menus->size;	// start after the "this" menus
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
      continue;
    MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
    if(mg->ta_menu == NULL) {
      mg->GetMenu();
      if(menu->item_count() > cnt)	// offset to account for "this" menu
	menu->replace_item(cnt, mg->GetMenuItem());
      else
	menu->append_item(mg->GetMenuItem());
    }
    else {
      mg->UpdateMenu();
    }
    cnt++;
  }
}

void WinBase::GetAllWinPos() {
  if(!taMisc::iv_active) return;
  GetWinPos();			// get mine
  TypeDef* td = GetTypeDef();
  int i;
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
      continue;
    MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
    mg->GetAllWinPos();
  }
}

void WinBase::ScriptAllWinPos() {
  if(!taMisc::iv_active) return;
  ScriptWinPos();		// get mine
  TypeDef* td = GetTypeDef();
  int i;
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
      continue;
    MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
    mg->ScriptAllWinPos();
  }
}

// called once upon initialization of the wineve
// allocates a getfile structure for the wineve + all of its menugroups..

void WinBase::GetFileProps(TypeDef* td, String& fltr, bool& cmprs) {
  int opt;
  if((opt = td->opts.FindContains("EXT_")) >= 0) {
    fltr = td->opts.FastEl(opt).after("EXT_");
    fltr = "*." + fltr + "*";
  }
  else
    fltr = "*";
  cmprs = false;
  if(td->HasOption("COMPRESS"))
    cmprs = true;
}

taivGetFile* WinBase::GetFileDlg() {
  if(!taMisc::iv_active)
    return NULL;
  if(ta_file != NULL)
    return ta_file;

  ta_file = taNBase::GetFileDlg();
  taRefN::Ref(ta_file);
  MenuGroup_impl* mg_own = GET_MY_OWNER(MenuGroup_impl);
  if((mg_own != NULL) && (mg_own->ta_file != NULL)) {
    ta_file->fname = mg_own->ta_file->fname; // get file name from menugroup
  }

  TypeDef* td = GetTypeDef();
  String fltr;
  bool cmprs;
  int i;
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(md->type->InheritsFrom(&TA_MenuGroup_impl)) {
      MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
      GetFileProps(mg->el_typ, fltr, cmprs);
      if(mg->ta_file != NULL)
	taRefN::unRefDone(mg->ta_file);
      mg->ta_file = new taivGetFile(".", fltr, window, wkit->style(), cmprs);
      taRefN::Ref(mg->ta_file);
    }
  }
  return ta_file;
}

void WinBase::SetFileName(const char* fname) {
  if(fname == NULL) return;
  file_name = fname;
}

void WinBase::SetWinName() {
  if(!taMisc::iv_active) return;
  if(window == NULL)
    return;
  String prog_nm = ivSession::instance()->classname();
  String nw_name = prog_nm + ": " + GetPath() + "(" + name + ")";
  if((ta_file != NULL) && (!ta_file->fname.empty())) {
    nw_name += String(" Fn: ") + ta_file->fname;
    SetFileName(ta_file->fname);
  }
  if(nw_name == win_name) return; // no need to set, same name!
  win_name = nw_name;
  if(window->style() == NULL)
    window->style(new ivStyle(wkit->style()));
  window->style()->attribute("title", prog_nm); // win_name);
  window->style()->attribute("name", win_name);
  window->style()->attribute("iconName", name);
#ifndef CYGWIN
  if(window->is_mapped()) {
    ivManagedWindowRep* mwRep = window->rep();
    mwRep->do_set(window, &ivManagedWindowRep::set_name);
    mwRep->do_set(window, &ivManagedWindowRep::set_icon_name);
  }
#endif
}

void WinBase::UpdateMenus() {
  if(!taMisc::iv_active) return;
  taMisc::Busy();
  UpdateMenus_impl();
  taMisc::DoneBusy();
}

void WinBase::UpdateMenus_impl() {
  if(!IsMapped()) return;	// only update mapped windows!
  SetWinName();
  GetMenu();		// get yours
  GetWinPos();
}

void WinBase::GetBodyRep() {
  if(!taMisc::iv_active) return;
  // make it stretchable
  body = taivM->hspc;
  ivResource::ref(body);
}


void WinBase::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  taivMisc::active_wins.Remove(window);
  window->unmap();
  ivResource::unref(menu);	menu = NULL;
  ivResource::unref(body);	body = NULL;
  ivResource::unref(main_box);	main_box = NULL;
  ivResource::unref(win_box);	win_box = NULL;
  if(print_file != NULL) {
    taRefN::unRefDone(print_file); print_file = NULL;
  }
  this_menus->Reset();
  this_meths->Reset();
  delete window;  		window = NULL;
}

String WinBase::GetPrintFileExt(PrintFmt fmt) {
  String ext;
  if(fmt == POSTSCRIPT)
    ext = ".ps";
  else if(fmt == JPEG)
    ext = ".jpg";
  else if(fmt == TIFF)
    ext = ".tiff";
  return ext;
}

void WinBase::GetPrintFileDlg(PrintFmt fmt) {
  static PrintFmt last_fmt = POSTSCRIPT;
  if(!taMisc::iv_active) return;
  if((print_file != NULL) && (last_fmt == fmt))  return;
  if(print_file != NULL) {
    taRefN::unRefDone(print_file); print_file = NULL;
  }

  String ext = GetPrintFileExt(fmt);
  String filter = "*" + ext + "*";

  print_file = new taivGetFile(".", filter, window, wkit->style(), false);
  taRefN::Ref(print_file);
  last_fmt = fmt;
}

void WinBase::Print(PrintFmt format, const char* fname) {
  if(win_box != NULL) Print_impl(format, win_box, fname);
}

// *** warning GetPrintData() must return a Patch;
ivGlyph* WinBase::GetPrintData(){
  return win_box;
}

void WinBase::Print_Data(PrintFmt format, const char* fname) {
  ivGlyph* obj = this->GetPrintData();
  if (obj == NULL) return;
  Print_impl(format, obj, fname);
}

void WinBase::Print_impl(PrintFmt format, ivGlyph* obj, const char* fname) {
  // obj is really a patch
  if(taMisc::iv_active)  taMisc::Busy();
  else return;
  GetWinPos();
  GetPrintFileDlg(format);
  if(print_file == NULL) {
    if(taMisc::iv_active)  taMisc::DoneBusy();
    return;
  }
  ostream* strm = NULL;
  if(fname == NULL)
    strm = print_file->SaveAs(fname);
  else {
    print_file->fname = fname;
    strm = print_file->open_write();
  }
  if(strm == NULL) {
    if(taMisc::iv_active)  taMisc::DoneBusy();
    return;
  }
  if(format != POSTSCRIPT) {
#ifdef CYGWIN
    taMisc::Error("MS Windows version does not support JPEG or TIFF window saving formats -- POSTSCRIPT only!");
#else    
    const ivAllocation& alloc = ((ivPatch*)obj)->allocation();
    ivCanvas* c = window->canvas();
    int xstart = c->to_pixels(alloc.left(), Dimension_X);
    int xend = c->to_pixels(alloc.right(), Dimension_X);
    int ystart = c->pheight() - c->to_pixels(alloc.top(), Dimension_Y);
    int yend = c->pheight() - c->to_pixels(alloc.bottom(), Dimension_Y);
    if(format == JPEG) {
      taivMisc::DumpJpegIv(window, print_file->fname, taMisc::jpeg_quality, xstart, ystart, xend-xstart, yend-ystart);
    }
    else if(format == TIFF) {
      taivMisc::DumpTiffIv(window, print_file->fname, xstart, ystart, xend-xstart, yend-ystart);
    }
#endif
  }
  else {			// POSTSCRIPT
    ivPrinter* p = new ivPrinter(strm);
    String myname = String(ivSession::instance()->classname()) +
      ": " + GetPath();
    p->prolog(myname);
    const ivAllocation& a = ((ivPatch*) obj)->allocation();
    p->resize(a.left(), a.bottom(), a.right(), a.top());
    p->page("1");
    obj->print(p, a);
    p->epilog();
    obj->undraw();
    print_file->Close();
    delete p;

    window->repair();
    ReSize(win_pos.wd, win_pos.ht); // updates everything
    //    if(InheritsFrom(TA_WinView)) {
    //      ((WinView*)this)->InitDisplay();
    //    }
  }
  if(taMisc::iv_active)  taMisc::DoneBusy();
}

int WinBase::Save(ostream& strm, TAPtr par, int indent) {
  if(taMisc::iv_active) {
    taMisc::Busy();
    GetAllWinPos();		// get window position for all *before* saving!
  }
  SetWinName();			// this also gets file_name
  int rval = taNBase::Save(strm, par, indent);
  if(taMisc::iv_active)  taMisc::DoneBusy();
  return rval;
}

int WinBase::Load(istream& strm, TAPtr par) {
  if(taMisc::iv_active)  taMisc::Busy();
  int rval = taNBase::Load(strm, par);
  // updating of menus done by calling function..
  if(taMisc::iv_active)  taMisc::DoneBusy();
  return rval;
}

void WinBase::Close() {
  if(GetOwner() == NULL)
    return;
  MenuGroup_impl* mg_own =  GET_MY_OWNER(MenuGroup_impl);
  winbMisc::DelayedMenuUpdate(mg_own);	// put on the update list
  tabMisc::Close_Obj(this);
}

int WinBase::Edit(bool wait) {
  GetWinPos();
  taivEdit* ive;
  if((ive = GetTypeDef()->ive) != NULL) {
    const ivColor* bgclr = GetEditColorInherit();
    return ive->Edit((void*)this, window, wait, false, bgclr);
  }
  return false;
}


//////////////////////////
//	WinView		//
//////////////////////////

void AnimImgCapture::Initialize() {
  capture = false;
  img_fmt = WinBase::JPEG;
  ctr = 0;
  subdir = "anim";
}

void WinView::Initialize() {
  gp_ta_menu = NULL;
  n_mgr_menus = 0;
  view_menu = NULL;
  view_menus = new taivMenu_List;
  mgr = NULL;
  updater_type = &TA_taBase;
  display_toggle = true;
}
     
void WinView::InitLinks() {
  // don't call the parent..
  taNBase::InitLinks();
  mgr = GET_MY_OWNER(WinMgr);
  taBase::Own(win_pos, this);
  taBase::Own(updaters, this);
  updater_type = UpdaterType();
  updaters.SetBaseType(updater_type);
  if(mgr != NULL) {
    WinInit();	// initialize after linking, so that path is  available
    if(taMisc::is_loading)
      winbMisc::unopened_windows.LinkUnique(this);
    else
      OpenNewWindow();		// and open a window
  }
}

void WinView::Destroy() {
  if(gp_ta_menu != NULL)    delete gp_ta_menu;  gp_ta_menu = NULL;
  if(view_menus != NULL)    delete view_menus;  view_menus = NULL;
  CutLinks();
}

void WinView::CutLinks() {
  RemoveAllUpdaters();
  WinBase::CutLinks();		// this closes the window already..
  mgr = NULL;
}

void WinView::Copy_(const WinView& cp) {
  updaters.BorrowUnique(cp.updaters);	// link group so just borrow
  mgr = GET_MY_OWNER(WinMgr);
  NotifyAllUpdaters();
}

int WinView::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = WinBase::Dump_Load_Value(strm, par);
  // don't do this by default, but some views might want to..
//   if(rval) {
//     NotifyAllUpdaters();
//   }
  return rval;
}

void WinView::AddUpdater(TAPtr ud) {
  if((ud == NULL) || (ud == &updaters))
    return;
  updaters.LinkUnique(ud);
  AddNotify(ud);
}

void WinView::RemoveUpdater(TAPtr ud) {
  if(ud == NULL)
    return;
  if(ud == &updaters) {
    RemoveAllUpdaters();
    return;
  }
  RemoveNotify(ud);
  updaters.Remove(ud);
}

void WinView::RemoveAllUpdaters() {
  int i;
  for(i=updaters.size-1; i>=0; i--)
    RemoveUpdater(updaters.FastEl(i));
}

void WinView::NotifyAllUpdaters() {
  int i;
  for(i=updaters.size-1; i>=0; i--)
    AddNotify(updaters.FastEl(i));
}

void WinView::GetWinBoxes() {
  WinBase::GetWinBoxes();
  view_menu = wkit->menubar();
  ivResource::ref(view_menu);
}

void WinView::OpenNewWindow() {
  if(!taMisc::iv_active) return;

  if(window != NULL) {
    SetWinName();
    return;
  }

  GetWinBoxes();
  GetWindow();
  GetThisMenus();
  GetMenu();
  GetBodyRep();

  main_box->append
    (wkit->inset_frame
     (layout->hbox(menu, taivM->hspc, view_menu)));
	

  main_box->append(layout->hcenter(body, 0));

  SetWinName();
  if((win_pos.lft != 0) && (win_pos.bot != 0)) {
    window->place(win_pos.lft, win_pos.bot);
  }

  window->map();
  taivMisc::active_wins.Add(window);
  if((mgr != NULL) && (mgr->window == NULL))
    mgr->GetPtrsFromView();
}

void WinView::SetWinName(){
  if(mgr == NULL)    return;
  if(!taMisc::iv_active) return;
  if(window == NULL) return;
  int idx = mgr->views.FindLeaf(this);
  String idxnm = String("_") + String(idx);
  if(name.contains(GetTypeDef()->name) || name.contains(idxnm) || name.empty()) {
    name = mgr->name + idxnm;
  }
  String prog_nm = ivSession::instance()->classname();
  String nw_name = prog_nm + ": " + mgr->GetPath() +
    "(" + mgr->name + ")" + " Vw: " + name.after("_");
  if((mgr->ta_file != NULL) && (!mgr->ta_file->fname.empty())) {
    nw_name += String(" Fn: ") + mgr->ta_file->fname;
    SetFileName(mgr->ta_file->fname);
    mgr->file_name = file_name;
  }
  if(nw_name == win_name) return; // no need to set, same name!
  win_name = nw_name;
  mgr->win_name = nw_name;
  if(window->style() == NULL)
    window->style(new ivStyle(wkit->style()));
  window->style()->attribute("title", win_name);
  window->style()->attribute("name", win_name);
  window->style()->attribute("iconName", name);
#ifndef CYGWIN
  if(window->is_mapped()) {
    ivManagedWindowRep* mwRep = window->rep();
    mwRep->do_set(window, &ivManagedWindowRep::set_name);
    mwRep->do_set(window, &ivManagedWindowRep::set_icon_name);
  }
#endif
}

bool WinView::ThisMenuFilter(MethodDef* md) {
  if((md->name == "Load") || (md->name == "Save") || (md->name == "SaveAs") ||
     (md->name == "UpdateMenus") || (md->name == "InitDisplay") ||
     (md->name == "UpdateDisplay") || (md->name == "GetAllWinPos") ||
     (md->name == "ScriptAllWinPos") || (md->name == "SetWinPos") ||
     (md->name == "SelectForEdit") || (md->name == "SelectFunForEdit"))
    return false;
  return true;
}

void WinView::GetThisMenus() {
  if(mgr == NULL)    return;

  this_menus->Reset();
  view_menus->Reset();
  this_meths->Reset();

  String prfx = "";
  mgr->GetThisMenus_impl(*this_menus, *this_meths, prfx);
  n_mgr_menus = this_menus->size; // this is the number from the manager
  prfx = "View: ";
  GetThisMenus_impl(*view_menus, *this_meths, prfx);
  int i;
  for(i=0; i<n_mgr_menus; i++) {
    taivMenu* mnu = this_menus->FastEl(i);
    menu->append_item(mnu->GetMenuItem());
  }
  for(i=0; i<view_menus->size; i++) { // add these to the view menu
    taivMenu* mnu = view_menus->FastEl(i);
    view_menu->append_item(mnu->GetMenuItem());
  }
}

// this gets the menu from the groups (which the manager has)

void WinView::GetMenu() {
  if(mgr == NULL)    return;
  if(!taMisc::iv_active) return;

  taivHierMenu_List* old_gp_ta_menu = gp_ta_menu;
  gp_ta_menu = new taivHierMenu_List;

  TypeDef* td = mgr->GetTypeDef();
  // dealing with an existing menu here..
  if((old_gp_ta_menu != NULL) && 
     (menu->item_count() >= (n_mgr_menus + old_gp_ta_menu->size)))
  {
    int i;
    int cnt = n_mgr_menus;	// start after the "this" menus
    for(i=TA_WinBase.members.size; i<td->members.size; i++) {
      MemberDef* md = td->members.FastEl(i);
      if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
	continue;
      ivMenuItem* temp_mi = new ivMenuItem(NULL, new ivTelltaleState());
      menu->replace_item(cnt, temp_mi); // replace with temp item
      old_gp_ta_menu->Remove(0);		// safe to get rid of first menu now..
      ivResource::flush();

      MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)mgr);
      mg->ta_menu = NULL;
      mg->GetMenu();
      gp_ta_menu->Add(mg->ta_menu); 	// get our own copy of it
      menu->replace_item(cnt, mg->GetMenuItem()); // replace with mg item
      mg->ta_menu = NULL;		// reset the one in the MG to null
      ivResource::flush();
      if(!mg->el_base->InheritsFrom(TA_WinView)) // update sub-menus only if not views
	mg->UpdateElMenus();
      cnt++;
    }
  }
  else {			// create the menu fresh..
    int i;
    for(i=TA_WinBase.members.size; i<td->members.size; i++) {
      MemberDef* md = td->members.FastEl(i);
      if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
	continue;
      MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)mgr);
      mg->ta_menu = NULL;
      mg->GetMenu();
      gp_ta_menu->Add(mg->ta_menu); // get our own copy of it
      menu->append_item(mg->GetMenuItem());
      mg->ta_menu = NULL;	// reset the one in the MG to null
      if(!mg->el_base->InheritsFrom(TA_WinView))	// update sub-menus only if not views
	mg->UpdateElMenus();
    }
  }
  if(old_gp_ta_menu != NULL) {
    delete old_gp_ta_menu;
  }
}

void WinView::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  taivMisc::active_wins.Remove(window);
  window->unmap();
  // the individual viewer is responsible for unrefing the body
  ivResource::unref(menu);	menu = NULL;
  ivResource::unref(view_menu);	view_menu = NULL;
  ivResource::unref(main_box);	main_box = NULL;
  ivResource::unref(win_box);	win_box = NULL;
  if(gp_ta_menu != NULL) {
    delete gp_ta_menu;		gp_ta_menu = NULL;
  }
  this_menus->Reset();
  view_menus->Reset();
  this_meths->Reset();
  delete window;		window = NULL;
}

void WinView::CaptureAnimImg() {
  if(!taMisc::iv_active) return;

  int acc = access(anim.subdir, F_OK);
  if(acc != 0) {
    int rval = mkdir(anim.subdir, 0xFFF);
    if(rval == -1) {
      taMisc::Error("Error: could not make animation subdirectory:", anim.subdir);
    }
  }

  String ext = GetPrintFileExt(anim.img_fmt);
  String fnm = anim.subdir + "/anim_img." + taMisc::LeadingZeros(anim.ctr, 5) + ext;
  taMisc::Error("...Capturing anim image to:",fnm);
  Print_Data(anim.img_fmt, fnm);
  anim.ctr++;
}

void WinView::StartAnimCapture(PrintFmt image_fmt, int image_ctr, const char* sub_dir) {
  anim.img_fmt = image_fmt;
  anim.ctr = image_ctr;
  anim.subdir = sub_dir;
  anim.capture = true;
}

void WinView::StopAnimCapture() {
  anim.capture = false;
}

//////////////////////////
//	WinMgr		//
//////////////////////////

void WinMgr::Initialize() {
  views.SetBaseType(&TA_WinView);
}

void WinMgr::Destroy() {
  CutLinks();
}

void WinMgr::CutLinks() {
  CloseAllWindows();
  taNBase::CutLinks();
  win_owner = NULL;
}  

void WinMgr::InitLinks() {
  taBase::Own(views, this);
  // don't call winbase initlinks, because we don't want to put in an
  // open new window call for us!
  taNBase::InitLinks();
  WinBase* tmp = GET_MY_OWNER(WinBase);
  if(tmp == this)
    tmp = NULL;
  win_owner = tmp;
  taBase::Own(win_pos, this);
  WinInit();			// initialize after linking, so that path is available
  if(owner && (!owner->InheritsFrom(&TA_TypeDefault))) { // no windows for defaults
    if(!taMisc::is_loading)
      OpenNewWindow();		// only open if not loading!
  }
  GetFileDlg();
}

void WinMgr::Copy(const WinMgr& cp) {
  WinBase::Copy(cp);
  views = cp.views;
}

void WinMgr::UpdateAfterEdit(){
  WinBase::UpdateAfterEdit();
  if(taMisc::is_loading || !taMisc::iv_active) return;
  GetPtrsFromView();
}

void WinMgr::GetPtrsFromView() {
  WinView* wv = (WinView*)views.DefaultEl();
  if(wv == NULL) {
    window = NULL;
    win_box = NULL;
    main_box = NULL;
    return;
  }
  window = wv->window;
  win_box = wv->win_box;
  main_box = wv->main_box;
}

void WinMgr::OpenNewWindow() {
//   if(!taMisc::iv_active) return;

  if(views.leaves == 0) views.New(1, GetDefaultView()); // create a default view
  GetPtrsFromView();
}

void WinMgr::OpenNewWindow(TypeDef* td) {
  if(!taMisc::iv_active) return;

  if(views.leaves == 0) views.New(1, td); // create a default view
  GetPtrsFromView();
}

bool WinMgr::ThisMenuFilter(MethodDef* md) {
  if((md->name == "Print") || (md->name == "Print_Data") ||
     (md->name == "ScriptAllWinPos") || (md->name == "GetAllWinPos") ||
     (md->name == "SetWinPos") || (md->name == "SelectForEdit") ||
     (md->name == "SelectFunForEdit"))
    return false;
  return true;
}

// while the manager, when menu's need updated, just updates all of its views
void WinMgr::GetMenu() {
  if(!taMisc::iv_active) return;
  GetPtrsFromView();		// keep this current
  WinView* vw;
  taLeafItr i;
  FOR_ITR_EL(WinView, vw, views., i) 
    vw->GetMenu();
}

void WinMgr::UpdateMenus() {
  if(!taMisc::iv_active) return;
  taMisc::Busy();
  GetPtrsFromView();		// keep this current
  WinView* vw;
  taLeafItr i;
  FOR_ITR_EL(WinView, vw, views., i) 
    vw->UpdateMenus_impl();
  taMisc::DoneBusy();
}

void WinMgr::CloseWindow() {
  if(!taMisc::iv_active) return;
  if(views.leaves == 0)
    return;

  WinView* vw = (WinView*) views.Leaf(views.leaves-1);
  CloseWindow(vw);
}

void WinMgr::CloseWindow(WinView* vw) {
  views.RemoveLeaf(vw);
  GetPtrsFromView();
}

void WinMgr::CloseAllWindows() {
  views.RemoveAll();
  GetPtrsFromView();
}

void WinMgr::SetWinName() {
  WinView* vw;
  taLeafItr i;
  FOR_ITR_EL(WinView, vw, views., i) 
    vw->SetWinName();
}

void WinMgr::InitAllViews() {
  WinView* vw;
  taLeafItr i;
  FOR_ITR_EL(WinView, vw, views., i) 
    vw->InitDisplay();
}
void WinMgr::UpdateAllViews() {
  WinView* vw;
  taLeafItr i; 
  FOR_ITR_EL(WinView, vw, views., i) {
    vw->UpdateDisplay();
    vw->SetWinName();
  }
}

void WinMgr::GetAllWinPos() {
  if(!taMisc::iv_active) return;
//  GetWinPos();			don't get mine
  TypeDef* td = GetTypeDef();
  int i;
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
      continue;
    MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
    mg->GetAllWinPos();
  }
}

void WinMgr::ScriptAllWinPos() {
  if(!taMisc::iv_active) return;
//  ScriptWinPos();		don't get mine
  TypeDef* td = GetTypeDef();
  int i;
  for(i=TA_WinBase.members.size; i<td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(!md->type->InheritsFrom(&TA_MenuGroup_impl))
      continue;
    MenuGroup_impl* mg = (MenuGroup_impl*)md->GetOff((void*)this);
    mg->ScriptAllWinPos();
  }
}

void WinMgr::GetWinPos() {
  WinView* vw;
  taLeafItr i; 
  FOR_ITR_EL(WinView, vw, views., i)
    vw->GetWinPos();
}

void WinMgr::ScriptWinPos() {
  WinView* vw;
  taLeafItr i; 
  FOR_ITR_EL(WinView, vw, views., i)
    vw->ScriptWinPos();
}

