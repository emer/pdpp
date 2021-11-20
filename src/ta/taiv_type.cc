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

// taiv_type.cc

#include <ta/taiv_type.h>
#include <ta/tdefaults.h>
#include <css/css_iv.h>
#include <css/basic_types.h>
#include <css/ta_css.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <ta/leave_iv.h>


//////////////////////////
// 	taivType	//
//////////////////////////

taivType::taivType() {
  typ = NULL;
  bid = 0;
  sub_types = NULL;
  no_setpointer = false;
}

taivType::taivType(TypeDef* tp) {
  typ = tp;
  bid = 0;
  sub_types = NULL;
}

taivType::~taivType() {
  if(sub_types != NULL)
    delete sub_types;
  sub_types = NULL;
}

taivData* taivType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  String strval;
  if(typ->GetInstance() != NULL)
    strval = typ->GetValStr(typ->GetInstance());
  taivField* rval = new taivField((char*)strval, dlg, ih, par);
  return rval;
}

void taivType::GetImage(taivData* dat, void* base, ivWindow*) {
  String strval = typ->GetValStr(base);
  taivField* rval = (taivField*)dat;
  rval->GetImage((char*)strval);
}

void taivType::GetValue(taivData* dat, void* base) {
  taivField* rval = (taivField*)dat;
  String strval = String(rval->GetValue());
  typ->SetValStr(strval, base);
}

// add "this" type to the td->iv slot, or link in lower slot as per the bid

void taivType::AddIv(TypeDef* td) {
  taivType* cur_iv = td->iv;		// the current iv
  taivType** ptr_to_iv = &(td->iv);	// were to put one
  while(cur_iv != NULL) {
    if(bid < cur_iv->bid) {		// we are lower than current
      ptr_to_iv = &(cur_iv->sub_types);	// put us on its sub_types
      cur_iv = cur_iv->sub_types;	// and compare to current sub_type
    }
    else {
      sub_types = cur_iv;		// we are higher, current is our sub
      cur_iv = NULL;			// and we are done
    }
  }
  *ptr_to_iv = this;			// put us here
}


////////////////////////
//  taivIntType      //
////////////////////////

int taivIntType::BidForType(TypeDef* td){
  if(td->InheritsFrom("Int") || td->InheritsFrom("int"))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivIntType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  String strval;
  if(typ->GetInstance() != NULL)
    strval = typ->GetValStr(typ->GetInstance());
  taivIncrField* rval = new taivIncrField((char*)strval, dlg, ih, par, false);
  return rval;
}

void taivIntType::GetImage(taivData* dat, void* base, ivWindow*) {
  String strval = typ->GetValStr(base);
  taivIncrField* rval = (taivIncrField*)dat;
  rval->GetImage((char*)strval);
}

void taivIntType::GetValue(taivData* dat, void* base) {
  taivIncrField* rval = (taivIncrField*)dat;
  String strval = String(rval->GetValue());
  typ->SetValStr(strval, base);
}


////////////////////////
//  taivEnumType     //
////////////////////////

int taivEnumType::BidForType(TypeDef* td){
  if(td->InheritsFormal(TA_enum))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivEnumType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivMenu* rval = new taivMenu
    (taivMenu::menubar, taivMenu::radio_update, taivMenu::small, dlg, par);
  int i;
  for(i=0; i<typ->enum_vals.size; i++) {
    rval->AddItem(typ->enum_vals.FastEl(i)->GetLabel());
  }
  return rval;
}

void taivEnumType::GetImage(taivData* dat, void* base, ivWindow*) {
  taivMenu* rval = (taivMenu*)dat;
  EnumDef* td = typ->enum_vals.FindNo(*((int*)base));
  if(td)
    rval->GetImage(td->idx);
}

void taivEnumType::GetValue(taivData* dat, void* base) {
  taivMenu* rval = (taivMenu*)dat;
  taivMenuEl* cur = rval->GetValue();
  EnumDef* td = NULL;
  if(cur && (cur->itm_no < typ->enum_vals.size))
    td = typ->enum_vals.FastEl(cur->itm_no);
  if(td)
    *((int*)base) = td->enum_no;
}


////////////////////////
//  taivBoolType     //
////////////////////////

int taivBoolType::BidForType(TypeDef* td){
  if(td->InheritsFrom(TA_bool))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivBoolType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par){
  taivToggle* rval = new taivToggle(dlg, par);
  return rval;
}

void taivBoolType::GetImage(taivData* dat, void* base, ivWindow*) {
  bool val = *((bool*)base);
  taivToggle* rval = (taivToggle*)dat;
  rval->GetImage((int)val);
}

void taivBoolType::GetValue(taivData* dat, void* base) {
  taivToggle* rval = (taivToggle*)dat;
  *((bool*)base) = (bool)rval->GetValue();
}


////////////////////////
//  taivClassType    //
////////////////////////

int taivClassType::BidForType(TypeDef* td) {
  if(td->InheritsFormal(TA_class))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivClassType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  if(typ->HasOption("INLINE") || typ->HasOption("EDIT_INLINE")) {
    taivPolyData* rval = new taivPolyData(typ, dlg, par);
    rval->GetDataRep(ih, dlg);
    return rval;
  }
  else {
    taivEditButton* rval = new taivEditButton(typ, NULL, NULL, NULL, dlg, par);
    return rval;
  }
}

void taivClassType::GetImage(taivData* dat, void* base, ivWindow* win) {
  if(typ->HasOption("INLINE") || typ->HasOption("EDIT_INLINE")) {
    taivPolyData* rval = (taivPolyData*)dat;
    rval->GetImage(base, win);
  }
  else {
    taivEditButton *rval = (taivEditButton*)dat;
    rval->GetImage(base, win);
  }
}

void taivClassType::GetValue(taivData* dat, void* base) {
  if(typ->HasOption("INLINE") || typ->HasOption("EDIT_INLINE")) {
    taivPolyData* rval = (taivPolyData*)dat;
    rval->GetValue(base);
  }
}


////////////////////////
//  taivStringType   //
////////////////////////

int taivStringType::BidForType(TypeDef* td){
  if(td->InheritsFrom(TA_taString))
    return(taivClassType::BidForType(td) + 1);
  return 0;
}
taivData* taivStringType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  return taivType::GetDataRep(ih, dlg, par);
}

void taivStringType::GetImage(taivData* dat, void* base, ivWindow* win) {
  taivType::GetImage(dat, base, win);
}

void taivStringType::GetValue(taivData* dat, void* base) {
  taivType::GetValue(dat, base);
}

//////////////////////////////////
//  	taivTokenPtrType  	//
//////////////////////////////////

int taivTokenPtrType::BidForType(TypeDef* td) {
  if((td->ptr == 1) && td->DerivesFrom(TA_taBase))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivTokenPtrType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  TypeDef* npt = typ->GetNonPtrType();
  if(!npt->tokens.keep) {
    taivEditButton* ebrval = new taivEditButton(npt, NULL, NULL, NULL, dlg, par);
    return ebrval;
  }
  else {
    taivToken* rval = new taivToken(taivMenu::menubar, taivMenu::small, npt, dlg, par);
    rval->GetMenu();
    return rval;
  }
}

void taivTokenPtrType::GetImage(taivData* dat, void* base, ivWindow* win) {
  TypeDef* npt = typ->GetNonPtrType();
  if(!npt->tokens.keep) {
    taivEditButton *ebrval = (taivEditButton*) dat;
    ebrval->GetImage(*((void**) base), win);
  }
  else {
    taivToken* rval = (taivToken*)dat;
    rval->UpdateMenu();
    rval->GetImage(*((void**)base), NULL, win);
  }
}

void taivTokenPtrType::GetValue(taivData* dat, void* base) {
  TypeDef* npt = typ->GetNonPtrType();
  if(!npt->tokens.keep) {
    // do nothing?
    //   taivEditButton *ebrval = (taivEditButton*) dat;
  }
  else {
    taivToken* rval = (taivToken*)dat;
    if(!no_setpointer && typ->DerivesFrom(TA_taBase))
      taBase::SetPointer((TAPtr*)base, (TAPtr)rval->GetValue());
    else
      *((void**)base) = rval->GetValue();
  }
}

//////////////////////////
//    taivTypePtr	//
//////////////////////////

void taivTypePtr::Initialize() {
  orig_typ = NULL;
}

int taivTypePtr::BidForType(TypeDef* td) {
  if(td->DerivesFrom(TA_TypeDef) && (td->ptr == 1))
    return taivType::BidForType(td) + 1;
  return 0;
}

// todo: the problem is that it doesn't know at this point what the base is
// and can't therefore figure out what kind of datarep to use..
// need to have a datarep that is a "string or type menu" kind of thing..

taivData* taivTypePtr::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  if(orig_typ == NULL)
    return taivType::GetDataRep(ih, dlg, par);
  taivTypeHier* rval =
    new taivTypeHier(taivMenu::menubar, taivMenu::small, typ, dlg, par);
  rval->GetMenu();
  return rval;
}

void taivTypePtr::GetImage(taivData* dat, void* base, ivWindow* win) {
  if(orig_typ == NULL) {
    taivType::GetImage(dat, base, win);
    return;
  }
  taivTypeHier* rval = (taivTypeHier*)dat;
  rval->typd = (TypeDef*)*((void**)base); 
  rval->UpdateMenu();
  rval->GetImage((TypeDef*)*((void**)base));
}

void taivTypePtr::GetValue(taivData* dat, void* base) {
  if(orig_typ == NULL) {
    taivType::GetValue(dat, base);
    return;
  }
  taivTypeHier* rval = (taivTypeHier*)dat;
  *((void**)base) = rval->GetValue();
}

////////////////////////
//  taivFilePtrType  //
////////////////////////

int taivFilePtrType::BidForType(TypeDef* td) {
  if(td->DerivesFrom("taivGetFile") && (td->ptr == 1))
    return (taivType::BidForType(td) +1);
  return 0;
}

taivData* taivFilePtrType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  return new taivFileButton(NULL, NULL, dlg, par);
}

void taivFilePtrType::GetImage(taivData* dat, void* base, ivWindow* win){
  taivFileButton* fbut = (taivFileButton*) dat;
  fbut->GetImage(base,win);
}

void taivFilePtrType::GetValue(taivData* dat, void* base) {
  taivFileButton* rval = (taivFileButton*)dat;
  *((void**)base) = rval->GetValue();
}



//////////////////////////
// 	taivEdit	//
//////////////////////////

void taivEdit::AddIve(TypeDef* td) {
  taivEdit* cur_ive = td->ive;
  taivEdit** ptr_to_ive = (taivEdit **) &(td->ive);
  while(cur_ive != NULL) {
    if(bid < cur_ive->bid) {
      ptr_to_ive = (taivEdit**)&(cur_ive->sub_types);
      cur_ive = (taivEdit*)cur_ive->sub_types;
    }
    else {
      sub_types = cur_ive;
      cur_ive = NULL;
    }
  }
  *ptr_to_ive = this;
}

int taivEdit::Edit(void* base, ivWindow* win, bool wait, bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new taivEditDialog(typ, base);
    if(typ->HasOption("NO_OK"))
      dlg->no_ok_but = true;
    if(typ->HasOption("NO_CANCEL"))
      dlg->no_can_but = true;
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


//////////////////////////
// 	taivMember	//
//////////////////////////

taivData* taivMember::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  taivData* dat = mbr->type->iv->GetDataRep(ih,dlg,par);
  if(!(mbr->OptionAfter("DEF_")).empty()) dat->CreateHiBG();
  return dat;
}

void taivMember::GetImage(taivData* dat, void* base, ivWindow* win) {
  mbr->type->iv->GetImage(dat, mbr->GetOff(base), win);
  GetOrigVal(dat, base);
}

void taivMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  mbr->type->iv->GetValue(dat, mbr->GetOff(base));
  CmpOrigVal(dat, base, first_diff);
}

void taivMember::GetOrigVal(taivData* dat, void* base) {
  dat->orig_val = mbr->type->GetValStr(mbr->GetOff(base));
  if(dat->hi_bg != NULL) {
    String defval = mbr->OptionAfter("DEF_");
    if(dat->orig_val != defval)
      dat->hi_bg->highlight = true;
    else
      dat->hi_bg->highlight = false;
  }
}

void taivMember::StartScript(void* base) {
  if((taivMisc::record_script == NULL) || !typ->InheritsFrom(TA_taBase))
    return;

  if((((TAPtr)base)->GetOwner() == NULL) && ((TAPtr)base != tabMisc::root))
    return;	// no record for unowned objs (except root)!
  TAPtr tab = (TAPtr)base;
  *taivMisc::record_script << "{ " << typ->name << "* ths = "
			   << tab->GetPath() << ";" << endl;
}  

void taivMember::EndScript(void* base) {
  if(taivMisc::record_script == NULL)
    return;
  if((((TAPtr)base)->GetOwner() == NULL) && ((TAPtr)base != tabMisc::root))
    return;	// no record for unowned objs (except root)!
  *taivMisc::record_script << "}" << endl;
}

void taivMember::CmpOrigVal(taivData* dat, void* base, bool& first_diff) {
  if((taivMisc::record_script == NULL) || !typ->InheritsFrom(TA_taBase))
    return;
  if((((TAPtr)base)->GetOwner() == NULL) && ((TAPtr)base != tabMisc::root))
    return;	// no record for unowned objs (except root)!
  String new_val = mbr->type->GetValStr(mbr->GetOff(base));
  if(dat->orig_val == new_val)
    return;
  if(first_diff)
    StartScript(base);
  first_diff = false;
  new_val.gsub("\"", "\\\"");	// escape the quotes..
  *taivMisc::record_script << "  ths->" << mbr->name << " = \""
			   << new_val << "\";" << endl;
}

void taivMember::AddIvm(MemberDef* md) {
  taivMember* cur_ivm = md->iv;
  taivMember** ptr_to_ivm = (taivMember **) &(md->iv);
  while(cur_ivm != NULL) {
    if(bid < cur_ivm->bid) {
      ptr_to_ivm = (taivMember**)&(cur_ivm->sub_types);
      cur_ivm = (taivMember*)cur_ivm->sub_types;
    }
    else {
      sub_types = cur_ivm;
      cur_ivm = NULL;
    }
  }
  *ptr_to_ivm = this;
}

////////////////////////
//  taivMembers      //
////////////////////////


////////////////////////
//  taivROMember     //
////////////////////////

int taivROMember::BidForMember(MemberDef* md, TypeDef* td) {
  if(md->HasOption("READ_ONLY") || md->HasOption("IV_READ_ONLY"))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivROMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivLabel* rval = new taivLabel(dlg, par);
  return rval;
}

static String taivROMember_get_str(MemberDef* mbr, void* new_base, void* base) {
  String strval;
  if(mbr->type->InheritsFormal(TA_class)) {
    int i;
    for(i=0; i<mbr->type->members.size; i++) {
      MemberDef* md = mbr->type->members.FastEl(i);
      if(md->HasOption("HIDDEN_INLINE") || !md->ShowMember(taMisc::show_iv))
	continue;
      strval += md->name + "=" + taivROMember_get_str(md, md->GetOff(new_base), new_base) + ": ";
    }
  }
  else if(mbr->type->InheritsFormal(TA_enum)) {
    EnumDef* ed = mbr->type->enum_vals.FindNo(*((int*)new_base));
    if(ed != NULL) {
      strval = ed->GetLabel();
    }
    else 
      strval = String(*((int*)new_base));
  }
  else {
    strval = mbr->type->GetValStr(new_base, base, mbr);
  }
  return strval;
}

void taivROMember::GetImage(taivData* dat, void* base, ivWindow*){
  void* new_base = mbr->GetOff(base);
  taivLabel* rval = (taivLabel*)dat;
  String strval = taivROMember_get_str(mbr, new_base, base);
  rval->GetImage(strval);
}

void taivROMember::GetMbrValue(taivData*, void*, bool&) {
}



//////////////////////////////////
//  	taivTokenPtrMember  	//
//////////////////////////////////

int taivTokenPtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if(td->InheritsFrom(TA_taBase) && 
     (md->type->ptr == 1) && md->type->DerivesFrom(TA_taBase))
    return taivMember::BidForMember(md,td)+1;
  return 0;
}

taivData* taivTokenPtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  TypeDef* npt = mbr->type->GetNonPtrType();
  if(!npt->tokens.keep) {
    taivEditButton* ebrval = new taivEditButton(npt, NULL, NULL, NULL, dlg, par);
    return ebrval;
  }
  bool nul_not = false;
  if(mbr->HasOption("NO_NULL"))
    nul_not = true;
  bool edit_not = false;
  if(mbr->HasOption("NO_EDIT"))
    edit_not = true;

  taivToken* rval = new taivToken(taivMenu::menubar, taivMenu::small, npt, dlg, par,
				    nul_not, edit_not);
  if(mbr->HasOption("NO_SCOPE"))
    rval->scope_ref = NULL;
  else if((dlg != NULL) && ((taivEditDialog*)dlg)->typ->InheritsFrom(TA_taBase))
    rval->scope_ref = (TAPtr)((taivEditDialog*)dlg)->cur_base;
  rval->GetMenu();
  return rval;
}

void taivTokenPtrMember::GetImage(taivData* dat, void* base, ivWindow* win) {
  TypeDef* npt = mbr->type->GetNonPtrType();
  if(!npt->tokens.keep) {
    taivEditButton *ebrval = (taivEditButton*) dat;
    ebrval->GetImage(*((void**) mbr->GetOff(base)), win);
    return;
  }
  
  String mb_nm = mbr->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    MemberDef* md = typ->members.FindName(mb_nm);
    if(md != NULL)
      npt = (TypeDef*)*((void**)md->GetOff(base)); // set according to value of this member
  }

  taivToken* rval = (taivToken*)dat;
  if(mbr->HasOption("NO_SCOPE"))
    rval->scope_ref = NULL;
  else if((rval->dialog != NULL) && ((taivEditDialog*)rval->dialog)->typ->InheritsFrom(TA_taBase))
      rval->scope_ref = (TAPtr)((taivEditDialog*)rval->dialog)->cur_base;
  else
    rval->scope_ref = (TAPtr)base;
  rval->UpdateMenu();
  rval->GetImage(*((void**)mbr->GetOff(base)), rval->scope_ref, win);
  GetOrigVal(dat, base);
}

void taivTokenPtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  TypeDef* npt = mbr->type->GetNonPtrType();
  if(!npt->tokens.keep) {
    // do nothing
    return;
  }
  String mb_nm = mbr->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    MemberDef* md = typ->members.FindName(mb_nm);
    if(md != NULL)
      npt = (TypeDef*)*((void**)md->GetOff(base)); // set according to value of this member
  }
  taivToken* rval = (taivToken*)dat;
  if(!no_setpointer && npt->DerivesFrom(TA_taBase))
    taBase::SetPointer((TAPtr*)mbr->GetOff(base), (TAPtr)rval->GetValue());
  else
    *((void**)mbr->GetOff(base)) = rval->GetValue();
  CmpOrigVal(dat, base, first_diff);
}

//////////////////////////////////
//	taivTypePtrMember	//
//////////////////////////////////

int taivTypePtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->type->ptr == 1) &&
     ((md->OptionAfter("TYPE_") != "") || (md->OptionAfter("TYPE_ON_") != "")
      || (md->HasOption("NULL_OK")))
     && md->type->DerivesFrom(TA_TypeDef))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivTypePtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  bool nul_ok = false;
  if(mbr->HasOption("NULL_OK"))
    nul_ok = true;
  taivTypeHier* rval =
    new taivTypeHier(taivMenu::menubar, taivMenu::small, mbr->type, dlg, par, nul_ok);
  rval->GetMenu();
  return rval;
}

void taivTypePtrMember::GetImage(taivData* dat, void* base, ivWindow*){
  void* new_base = mbr->GetOff(base);
  taivTypeHier* rval = (taivTypeHier*)dat;
  TypeDef* td = NULL;
  String mb_nm = mbr->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    MemberDef* md = typ->members.FindName(mb_nm);
    if(md != NULL)
      td = (TypeDef*)*((void**)md->GetOff(base)); // set according to value of this member
  }
  else {
    mb_nm = mbr->OptionAfter("TYPE_");
    if(mb_nm == "this")
      td = typ;
    else if(mb_nm != "")
      td = taMisc::types.FindName((char*)mb_nm);
  }
  if(td != NULL)
    rval->typd = td;
  else
    rval->typd = mbr->type;
  rval->UpdateMenu();
  rval->GetImage((TypeDef*)*((void**)new_base));
  GetOrigVal(dat, base);
}

void taivTypePtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  void* new_base = mbr->GetOff(base);
  taivTypeHier* rval = (taivTypeHier*)dat;
  TypeDef* nw_typ = (TypeDef*)rval->GetValue();
  if(mbr->HasOption("NULL_OK"))
    *((void**)new_base) = nw_typ;
  else {
    if(nw_typ != NULL) 
      *((void**)new_base) = nw_typ;
  }
  CmpOrigVal(dat, base, first_diff);
}


//////////////////////////////////
//	taivSubTokenPtrMember	//
//////////////////////////////////

int taivSubTokenPtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->type->ptr == 1) && (md->OptionAfter("SUBTYPE_") != ""))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivSubTokenPtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  TypeDef* td = NULL;
  String typ_nm = mbr->OptionAfter("SUBTYPE_");
  if(typ_nm != "")
    td = taMisc::types.FindName((char*)typ_nm);
  if(td == NULL)
    td = mbr->type;
  bool nul_ok = false;
  if(mbr->HasOption("NULL_OK"))
    nul_ok = true;
  bool edit_not = false;
  if(mbr->HasOption("NO_EDIT"))
    edit_not = true;
  taivSubToken* rval =
    new taivSubToken( taivMenu::menubar, taivMenu::small, td, dlg, par, nul_ok,
		       edit_not);
  return rval;
}


void taivSubTokenPtrMember::GetImage(taivData* dat, void* base, ivWindow* win){
  void* new_base = mbr->GetOff(base);
  taivSubToken* rval = (taivSubToken*)dat;
  rval->UpdateMenu();
  rval->GetImage(base,win,*((void **)new_base));
  GetOrigVal(dat, base);
}

void taivSubTokenPtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  void* new_base = mbr->GetOff(base);
  taivSubToken* rval = (taivSubToken*)dat;
  if(!no_setpointer && mbr->type->DerivesFrom(TA_taBase))
    taBase::SetPointer((TAPtr*)new_base, (TAPtr)rval->GetValue());
  else
    *((void**)new_base) = rval->GetValue();
  CmpOrigVal(dat, base, first_diff);
}


//////////////////////////////////////////
//	taivMemberDefPtrMember		//
//////////////////////////////////////////

int taivMemberDefPtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if((md->type->ptr == 1) && (md->type->DerivesFrom(TA_MemberDef)))
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivMemberDefPtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivMemberDefMenu* rval =
    new taivMemberDefMenu( taivMenu::menubar, taivMenu::small, mbr, dlg, par);
  return rval;
}

void taivMemberDefPtrMember::GetImage(taivData* dat, void* base, ivWindow* win){
  void* new_base = mbr->GetOff(base);
  taivMemberDefMenu* rval = (taivMemberDefMenu*)dat;
  rval->GetImage(base,win,*((void **)new_base));
  taivEditDialog* dlg = taivM->FindEdit(base, typ);
  if(dlg != NULL) {
    rval->ta_menu->ResetMenu();
    MemberSpace* mbs = &(typ->members);
    int i;
    for(i=0;i<mbs->size;i++){
      MemberDef* mbd = mbs->FastEl(i);
      if(!mbd->ShowMember(dlg->show)) continue;
      rval->ta_menu->AddItem(mbd->name,mbd,taivMenu::use_default,(taivMenuAction*)NULL);
    }
  }
  GetOrigVal(dat, base);
}

void taivMemberDefPtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  void* new_base = mbr->GetOff(base);
  taivMemberDefMenu* rval = (taivMemberDefMenu*)dat;
  *((void**)new_base) = rval->GetValue();
  CmpOrigVal(dat, base, first_diff);
}

//////////////////////////////
//	taivFunPtrMember   //
//////////////////////////////

int taivFunPtrMember::BidForMember(MemberDef* md, TypeDef* td) {
  if(md->fun_ptr != 0)
    return (taivMember::BidForMember(md,td) + 1);
  return 0;
}

taivData* taivFunPtrMember::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivMenu* rval = new taivMenu
    (taivMenu::menubar, taivMenu::radio_update, taivMenu::small, dlg, par);
  rval->AddItem("NULL", NULL);
  rval->AddSep();
  MethodDef* fun;
  int i;
  for(i=0; i<TA_taRegFun.methods.size; i++) {
    fun = TA_taRegFun.methods.FastEl(i);
    if(mbr->CheckList(fun->lists))
      rval->AddItem((char*)fun->name, (void*)fun->addr);
  }
  return rval;
}

void taivFunPtrMember::GetImage(taivData* dat, void* base, ivWindow*){
  void* new_base = mbr->GetOff(base);
  taivMenu* rval = (taivMenu*)dat;
  if(*((void**)new_base) == NULL) {
    rval->GetImage(0);
    return;
  }
  int cnt;
  MethodDef* fun = TA_taRegFun.methods.FindOnListAddr(*((ta_void_fun*)new_base),
							mbr->lists, cnt);
  if(fun)
    rval->GetImage(cnt+2);
  GetOrigVal(dat, base);
}

void taivFunPtrMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  void* new_base = mbr->GetOff(base);
  taivMenu* rval = (taivMenu*)dat;
  taivMenuEl* cur = rval->GetValue();
  if(cur != NULL)
    *((void**)new_base) = cur->usr_data;
  CmpOrigVal(dat, base, first_diff);
}

//////////////////////////////
//	taivCondEditMember  //
//////////////////////////////

void taivCondEditMember::Initialize() {
  ro_iv = NULL;
}

void taivCondEditMember::Destroy() {
  if(ro_iv != NULL)
    delete ro_iv;
  ro_iv = NULL;
}

taivCondEditMember::taivCondEditMember(MemberDef* md, TypeDef* td) : taivMember(md, td) {
  ro_iv = new taivROMember(md, td);
}

int taivCondEditMember::BidForMember(MemberDef* md, TypeDef* td) {
  if(!td->InheritsFrom(TA_taBase))
    return 0;
  String optedit = md->OptionAfter("CONDEDIT_");
  if(!optedit.empty()) {
    return (taivMember::BidForMember(md,td) + 100); // 100 = definitely do this over other!
  }
  return 0;
}

taivData* taivCondEditMember::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  taivDataDeck* rval = new taivDataDeck(dlg, par);
  if(sub_types)
    rval->data_el.Add(sub_types->GetDataRep(ih, dlg, rval));
  else
    rval->data_el.Add(taivMember::GetDataRep(ih, dlg, rval));
  if(!dlg->waiting)		// only get read-only for waiting dialogs (otherwise, no chance to update!)
    rval->data_el.Add(ro_iv->GetDataRep(ih, dlg, rval));
  rval->GetDataRep(ih, dlg);
  return rval;
}

void taivCondEditMember::GetImage(taivData* dat, void* base, ivWindow* win){
  taivDataDeck* rval = (taivDataDeck*)dat;
  if(sub_types) {
    sub_types->GetImage(rval->data_el.FastEl(0), base, win);
  }
  else {
    taivMember::GetImage(rval->data_el.FastEl(0), base, win);
  }
  if(rval->data_el.size == 2) {	// only if two options (i.e., !dlg->waiting)
    ro_iv->GetImage(rval->data_el.FastEl(1), base, win);

    // format: CONDEDIT_ON_member:value
    String optedit = mbr->OptionAfter("CONDEDIT_");
    if(optedit.empty()) return;
    String onoff = optedit.before('_');
    optedit = optedit.after('_');
    String mbr = optedit.before(':');
    String val = optedit.after(':');
  
    TAPtr tab = (TAPtr)base;
    MemberDef* md = NULL;
    void* mbr_base = NULL;	// base for member itself
    void* mbr_par_base = base;	// base for parent of member
    if(mbr.contains('.')) {
      String par_path = mbr.before('.', -1);
      MemberDef* par_md = NULL;
      TAPtr par_par = (TAPtr)tab->FindFromPath(par_path, par_md);
      if((par_par == NULL) || !(par_md->type->InheritsFrom(&TA_taBase))) {
	taMisc::Error("CONDEDIT: can't find parent of member:", par_path);
	return;
      }
      String subpth = mbr.after('.', -1);
      md = par_par->FindMembeR(subpth, mbr_base);
      mbr_par_base = (void*)par_par;
    }
    else {
      md = tab->FindMembeR(mbr, mbr_base);
    }
    if((md == NULL) || (mbr_base == NULL)) {
      taMisc::Error("*** CONDEDIT: conditionalizing member",mbr,"not found!");
      return;
    }
    String mbr_val = md->type->GetValStr(mbr_base, mbr_par_base, md);
    bool val_is_eq = false;
    while(true) {
      String nxtval;
      if(val.contains(',')) {
	nxtval = val.after(',');
	val = val.before(',');
      }
      if(val == mbr_val) {
	val_is_eq = true;
	break;
      }
      if(!nxtval.empty())
	val = nxtval;
      else
	break;
    }
    if(onoff == "ON") {
      if(val_is_eq)
	rval->GetImage(0);	// editable
      else
	rval->GetImage(1);	// not editable
    }
    else {
      if(val_is_eq)
	rval->GetImage(1);	// not editable
      else
	rval->GetImage(0);	// editable
    }
  }
  else {
    rval->GetImage(0);		// always editable
  }
  GetOrigVal(dat, base);
}

void taivCondEditMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  taivDataDeck* rval = (taivDataDeck*)dat;
  if(sub_types) {
    ((taivMember*)sub_types)->GetMbrValue(rval->data_el.FastEl(0), base, first_diff);
  }
  else {
    taivMember::GetMbrValue(rval->data_el.FastEl(0), base, first_diff);
  }
  if(rval->data_el.size == 2) {	// only if two options (i.e., !dlg->waiting)
    ro_iv->GetMbrValue(rval->data_el.FastEl(1), base, first_diff);
  }
}


/////////////////////////////
//    taivTDefaultMember  //
/////////////////////////////

int taivTDefaultMember::BidForMember(MemberDef*, TypeDef*) {
// TD_Default member does not bid, it is only applied in special cases.
  return (0);
}

taivData* taivTDefaultMember::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  taivData* rdat;
  if(sub_types)
    rdat = sub_types->GetDataRep(ih, dlg, par);
  else
    rdat = taivMember::GetDataRep(ih, dlg, par);
  taivPlusToggle* rval = new taivPlusToggle(rdat, dlg, par);
  return rval;
}

void taivTDefaultMember::GetImage(taivData* dat, void* base, ivWindow* win) {
  taivPlusToggle* rval = (taivPlusToggle*)dat;
  if(sub_types){
    sub_types->GetImage(rval->data,base,win);
  }
  else {
    taivMember::GetImage(rval->data, base, win);
  }
  taBase_List* gp = typ->defaults;
  tpdflt = NULL;
  if(gp != NULL) {
    int i;
    for(i=0; i<gp->size; i++) {
      TypeDefault* td = (TypeDefault*)gp->FastEl(i);
      if(td->token == (TAPtr)base) {
	tpdflt = td;
	break;
      }
    }
  }
  if(tpdflt != NULL)
    rval->GetImage(tpdflt->GetActive(mbr->idx));
  GetOrigVal(dat, base);
}

void taivTDefaultMember::GetMbrValue(taivData* dat, void* base, bool& first_diff) {
  taivPlusToggle* rval = (taivPlusToggle*)dat;
  if(sub_types) {
    ((taivMember*)sub_types)->GetMbrValue(rval->data,base,first_diff);
  }
  else {
    taivMember::GetMbrValue(rval->data, base,first_diff);
  }

  if(tpdflt != NULL)		// gotten by prev GetImage
    tpdflt->SetActive(mbr->idx, rval->GetValue());
  CmpOrigVal(dat, base, first_diff);
}


/////////////////////////////
//    taivDefaultToken    //
/////////////////////////////

int taivDefaultToken::BidForMember(MemberDef* md, TypeDef* td) {
  TypeDef* mtd = md->type;
  if(((mtd->ptr == 1) && mtd->DerivesFrom(TA_taBase)) &&
     md->HasOption("DEFAULT_EDIT"))
    return taivTokenPtrMember::BidForMember(md,td)+10;
  return 0;
}

taivData* taivDefaultToken::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivEditButton *rval =
	new taivEditButton(mbr->type->GetNonPtrType(), NULL, NULL,
			    new taivDefaultEdit(mbr->type->GetNonPtrType()), dlg, par);
  return rval;
}

void taivDefaultToken::GetImage(taivData* dat, void* base, ivWindow* win) {
  void* new_base = mbr->GetOff(base);

  taivEditButton* rval = (taivEditButton*)dat;
  rval->GetImage(*((void**)new_base), win);

  taBase* new_token = *((taBase**)new_base);
  if(new_token != NULL) {
    rval->typ = new_token->GetTypeDef();
  }
}

void taivDefaultToken::GetMbrValue(taivData*, void*, bool&) {
  return;
}


//////////////////////////
// 	taivMethod	//
//////////////////////////

void taivMethod::AddIvm(MethodDef* md) {
  taivMethod* cur_ivm = md->iv;
  taivMethod** ptr_to_ivm = (taivMethod **) &(md->iv);
  while(cur_ivm != NULL) {
    if(bid < cur_ivm->bid) {
      ptr_to_ivm = (taivMethod**)&(cur_ivm->sub_types);
      cur_ivm = (taivMethod*)cur_ivm->sub_types;
    }
    else {
      sub_types = cur_ivm;
      cur_ivm = NULL;
    }
  }
  *ptr_to_ivm = this;
}

/////////////////////////////
//       taivMethods      //
/////////////////////////////

/////////////////////////////
//   taivButtonMethod     //
/////////////////////////////

int taivButtonMethod::BidForMethod(MethodDef* md, TypeDef* td) {
  if(md->HasOption("BUTTON"))
    return (taivMethod::BidForMethod(md,td) + 1);
  return 0;
}

taivMethMenu* taivButtonMethod::GetMethodRep(void* base, taivDialog* dlg, taivData* par) {
  taivMethButton* rval = new taivMethButton(base, meth, typ, dlg, par);
  return rval;
}


/////////////////////////////
//     taivMenuMethod     //
/////////////////////////////

int taivMenuMethod::BidForMethod(MethodDef* md, TypeDef* td) {
  if(md->HasOption("MENU"))
    return (taivMethod::BidForMethod(md,td) + 1);
  return 0;
}

taivMethMenu* taivMenuMethod::GetMethodRep(void* base, taivDialog* dlg, taivData* par) {
  taivMethMenu* rval = new taivMethMenu(base, meth, typ, dlg, par);
  return rval;
}

////////////////////////////////
//   taivMenuButtonMethod     //
////////////////////////////////

int taivMenuButtonMethod::BidForMethod(MethodDef* md, TypeDef* td) {
  if(md->HasOption("MENU_BUTTON"))
    return (taivMethod::BidForMethod(md,td) + 1);
  return 0;
}

taivMethMenu* taivMenuButtonMethod::GetMethodRep(void* base, taivDialog* dlg, taivData* par) {
  taivMethMenu* rval = new taivMethMenu(base, meth, typ, dlg, par);
  return rval;
}

//////////////////////////
// 	taivArgType	//
//////////////////////////

taivArgType::taivArgType(TypeDef* argt, MethodDef* mb, TypeDef* td)
: taivType(td) {
  meth = mb;
  arg_typ = argt;
  err_flag = false;

  arg_base = NULL;
  arg_val = NULL;
  use_iv = NULL;
  obj_inst = NULL;
}

taivArgType::taivArgType() {
  meth = NULL;
  arg_typ = NULL;
  err_flag = false;

  arg_base = NULL;
  arg_val = NULL;
  use_iv = NULL;
  obj_inst = NULL;
}

taivArgType::~taivArgType() {
  if(use_iv != NULL)
    delete use_iv;
  use_iv = NULL;
  if(obj_inst != NULL)
    delete obj_inst;
  obj_inst = NULL;
}

void taivArgType::GetImage(taivData* dat, void*, ivWindow* win) {
  if(arg_base == NULL)
    return;
  if(use_iv != NULL)
    use_iv->GetImage(dat, arg_base, win);
  else
    arg_typ->iv->GetImage(dat, arg_base, win);
}

void taivArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  if(use_iv != NULL)
    use_iv->GetValue(dat, arg_base);
  else
    arg_typ->iv->GetValue(dat, arg_base);
}

taivData* taivArgType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  if(arg_base == NULL)
    return NULL;
  if(use_iv != NULL)
    return use_iv->GetDataRep(ih, dlg, par);

  return arg_typ->iv->GetDataRep(ih, dlg, par);
}

cssEl* taivArgType::GetElFromArg(const char* nm, void*) {
  if(arg_typ->ptr == 0) {
    if(arg_typ->DerivesFrom(TA_int) || arg_typ->DerivesFrom(TA_short) ||
       arg_typ->DerivesFrom(TA_long) || arg_typ->DerivesFrom(TA_char) ||
       arg_typ->DerivesFrom(TA_unsigned) || arg_typ->DerivesFrom(TA_signed) ||
       arg_typ->DerivesFrom(TA_bool))
    {
      arg_typ = &TA_int;
      arg_val = new cssInt(0, nm);
      arg_base = (void*)&(((cssInt*)arg_val)->val);
      return arg_val;
    }
    else if(arg_typ->DerivesFrom(TA_float) || arg_typ->DerivesFrom(TA_double)) {
      arg_typ = &TA_double;
      arg_val = new cssReal(0, nm);
      arg_base = (void*)&(((cssReal*)arg_val)->val);
      return arg_val;
    }      
    else if(arg_typ->DerivesFrom(TA_taString) || arg_typ->DerivesFrom(TA_taSubString)) {
      arg_typ = &TA_taString;
      arg_val = new cssString("", nm);
      arg_base = (void*)&(((cssString*)arg_val)->val);
      return arg_val;
    }
    else if(arg_typ->DerivesFrom(TA_taBase)) {
      arg_typ = arg_typ->GetNonRefType()->GetNonConstType();
      if(arg_typ->GetInstance() == NULL) return NULL;
      obj_inst = ((TAPtr)arg_typ->GetInstance())->MakeToken();
      arg_val = new cssTA_Base(obj_inst, 1, arg_typ, nm);
      arg_base = obj_inst;
      return arg_val;
    }
    else if(arg_typ->DerivesFormal(TA_enum)) {
      arg_val = new cssEnum(0, nm);
      arg_base = (void*)&(((cssEnum*)arg_val)->val);
      return arg_val;
    }
    return NULL;
  }
  // ptr > 0 (probably 1)

  if(arg_typ->DerivesFrom(TA_char)) {
    arg_typ = &TA_taString;
    arg_val = new cssString("", nm);
    arg_base = (void*)&(((cssString*)arg_val)->val);
    return arg_val;
  }
  else if(arg_typ->DerivesFrom(TA_taBase)) {
    TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
    arg_val = new cssTA_Base(NULL, 1, npt, nm);
    arg_base = (void*)&(((cssTA*)arg_val)->ptr);
    return arg_val;
  }

  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  arg_val = new cssTA(NULL, 1, npt, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}
  

/////////////////////////////
//       taivArgTypes     //
/////////////////////////////

//////////////////////////////////
//       taivStreamArgType     //
//////////////////////////////////

void taivStreamArgType::Initialize() {
  gf = NULL;
  filter = "";
  old_filter = "";
  old_compress = false;
}

void taivStreamArgType::Destroy() {
  if(gf != NULL) {
    gf->Close();
    gf->filter = old_filter;
    gf->compress = old_compress;
    taRefN::unRefDone(gf);
  }
  gf = NULL;
}

int taivStreamArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(argt->InheritsFrom(TA_ios))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* taivStreamArgType::GetElFromArg(const char* nm, void*) {
  // arg_val is for the function
  arg_val = new cssTA(NULL, 1, arg_typ, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}

taivData* taivStreamArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  String varext = meth->OptionAfter("EXT_");
  if(varext != "") {
    String myname = meth->arg_names[meth->arg_types.Find(arg_typ)];
    if(varext.before("_",-1) == myname) {
      if(meth->HasOption("COMPRESS"))
	filter = "*." + varext.after("_",-1) + taMisc::compress_sfx;
      else
	filter = "*." + varext.after("_",-1) + "*";
    }
  }
  if(arg_typ->InheritsFrom(TA_istream))
    return new taivFileButton((void*)&gf, NULL, dlg, par, true);
  else if(arg_typ->InheritsFrom(TA_ostream))
    return new taivFileButton((void*)&gf, NULL, dlg, par, false, true);
  return new taivFileButton((void*)&gf, NULL, dlg, par);
}

void taivStreamArgType::GetImage(taivData* dat, void* base, ivWindow* win){
  if(arg_base == NULL)
    return;
  taivFileButton* fbut = (taivFileButton*) dat;
  if(typ->InheritsFrom(TA_taBase)) {
    TAPtr it = (TAPtr)base;
    if(gf != NULL)
      taRefN::unRefDone(gf);
    gf = it->GetFileDlg();
    if(gf != NULL)
      taRefN::Ref(gf);
    old_filter = gf->filter;
    old_compress = gf->compress;
    if(filter != "") {
      if(filter.contains(taMisc::compress_sfx)) {
	gf->filter = filter.before(taMisc::compress_sfx) + "*";
	gf->compress = true;
      }
      else {
	gf->filter = filter;
	gf->compress = false;	// don't compress if we got a new filter..
      }
    }
  }
  fbut->GetImage((void*)&gf,win);
}

void taivStreamArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  taivFileButton* rval = (taivFileButton*)dat;
  if(gf == NULL) {
    gf = (taivGetFile*)rval->GetValue();
    taRefN::Ref(gf);
  }
  if(gf == NULL) {
    *((void**)arg_base) = NULL;
    err_flag = true;		// error-value not set..
    return;
  }
  GetValueFromGF();
}

void taivStreamArgType::GetValueFromGF() {
  if(arg_typ->InheritsFrom(TA_fstream)) {
    if(gf->fstrm == NULL) {
      *((void**)arg_base) = NULL;
      err_flag = true;		// error-value not set..
      return;
    }
    *((fstream**)arg_base) = gf->fstrm;
    return;
  }
  if(arg_typ->InheritsFrom(TA_istream)) {
    if(gf->istrm == NULL) {
      *((void**)arg_base) = NULL;
      err_flag = true;		// error-value not set..
      return;
    }
    *((istream**)arg_base) = gf->istrm;
    return;
  }
  if(arg_typ->InheritsFrom(TA_ostream)) {
    if(gf->ostrm == NULL) {
      *((void**)arg_base) = NULL;
      err_flag = true;		// error-value not set..
      return;
    }
    *((ostream**)arg_base) = gf->ostrm;
    return;
  }
}

//////////////////////////////////////////
//       taivBoolArgType		//
//////////////////////////////////////////

int taivBoolArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(argt->InheritsFrom(TA_bool))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* taivBoolArgType::GetElFromArg(const char* nm, void*) {
  arg_val = new cssBool(false, nm);
  arg_base = (void*)&(((cssBool*)arg_val)->val);
  use_iv = new taivBoolType(arg_typ); // make an iv for it...
  return arg_val;
}

//////////////////////////////////////////
//       taivTokenPtrArgType		//
//////////////////////////////////////////

int taivTokenPtrArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(td->InheritsFrom(TA_taBase) && 
     (argt->ptr == 1) && argt->DerivesFrom(TA_taBase))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}

cssEl* taivTokenPtrArgType::GetElFromArg(const char* nm, void*) {
  // arg_val is for the function
  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  arg_val = new cssTA_Base(NULL, 1, npt, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}

taivData* taivTokenPtrArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  bool null_not = true;
  if(meth->HasOption("NULL_OK"))
    null_not = false;
  bool edit_not = true;
  if(meth->HasOption("EDIT_OK"))
    edit_not = false;
  taivToken* rval = new taivToken(taivMenu::menubar, taivMenu::small, npt, dlg, par,
				    null_not, edit_not);
  rval->GetMenu();
  return rval;
}

void taivTokenPtrArgType::GetImage(taivData* dat, void* base, ivWindow* win){
  if(arg_base == NULL)
    return;
  TypeDef* npt = arg_typ->GetNonRefType()->GetNonConstType()->GetNonPtrType();
  String mb_nm = meth->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    if(mb_nm == "this") {
      npt = ((TAPtr)base)->GetTypeDef(); // use object type
    }
    else {
      MemberDef* md = typ->members.FindName(mb_nm);
      if(md != NULL) {
	TypeDef* mbr_typ = (TypeDef*)*((void**)md->GetOff(base)); // set according to value of this member
	if(mbr_typ->InheritsFrom(npt) || npt->InheritsFrom(mbr_typ))
	  npt = mbr_typ;		// make sure this applies to this argument..
      }
    }
  }
  taivToken* rval = (taivToken*)dat;
  rval->typd = npt;
  if(meth->HasOption("NO_SCOPE"))
    rval->scope_ref = NULL;
  else if((rval->dialog != NULL) && (rval->dialog->typ != NULL) &&
	  ((taivEditDialog*)rval->dialog)->typ->InheritsFrom(TA_taBase))
    rval->scope_ref = (TAPtr)((taivEditDialog*)rval->dialog)->cur_base;
  else
    rval->scope_ref = (TAPtr)base;
  rval->UpdateMenu();
  rval->GetImage(*((void**)arg_base), rval->scope_ref, win);
}

void taivTokenPtrArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  taivToken* rval = (taivToken*)dat;
// since it is an arg type, its a ptr to a css ptr, don't set it..
//   if(npt->DerivesFrom(TA_taBase))
//     taBase::SetPointer((TAPtr*)arg_base, (TAPtr)rval->GetValue());
//   else
  *((void**)arg_base) = rval->GetValue();
}

///////////////////////////////////
//       taivTypePtrArgType     //
///////////////////////////////////

int taivTypePtrArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(argt->DerivesFrom(TA_TypeDef) && (argt->ptr == 1))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}


cssEl* taivTypePtrArgType::GetElFromArg(const char* nm, void* base) {
  String mb_nm = meth->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    MemberDef* md = typ->members.FindName(mb_nm);
    if((md != NULL) && (md->type == &TA_TypeDef_ptr)) {
      TypeDef* tpdf = *(TypeDef**)(md->GetOff(base));
      arg_val = new cssTA(tpdf, 1, &TA_TypeDef, nm);
      arg_base = (void*)&(((cssTA*)arg_val)->ptr);
      return arg_val;
    }
  }
  else {
    mb_nm = meth->OptionAfter("TYPE_");
    if(mb_nm != "") {
      TypeDef* tpdf;
      if(mb_nm == "this") {
	tpdf = typ;
	if(typ->InheritsFrom(&TA_taBase) && (base != NULL))
	  tpdf = ((TAPtr)base)->GetTypeDef();
      }
      else
	tpdf = taMisc::types.FindName(mb_nm);
      if(tpdf == NULL)
	tpdf = &TA_taBase;
      arg_val = new cssTA(tpdf, 1, &TA_TypeDef, nm);
      arg_base = (void*)&(((cssTA*)arg_val)->ptr);
      return arg_val;
    }
  }
  arg_val = new cssTA(&TA_taBase, 1, &TA_TypeDef, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}

taivData* taivTypePtrArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  bool nul_ok = false;
  if(meth->HasOption("NULL_OK"))
    nul_ok = true;
  TypeDef* init_typ = &TA_taBase;
  if(*((TypeDef**)arg_base) != NULL)
    init_typ = *((TypeDef**)arg_base);
  taivTypeHier* rval = new taivTypeHier(taivMenu::menubar, taivMenu::small,
					  init_typ, dlg, par, nul_ok);
  rval->GetMenu();
  return rval;
}

void taivTypePtrArgType::GetImage(taivData* dat, void*, ivWindow*) {
  if(arg_base == NULL)
    return;
  taivTypeHier* rval = (taivTypeHier*)dat;
  rval->typd = (TypeDef*)*((void**)arg_base); 
  rval->UpdateMenu();
  rval->GetImage((TypeDef*)*((void**)arg_base));
}

void taivTypePtrArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  taivTypeHier* rval = (taivTypeHier*)dat;
  *((void**)arg_base) = rval->GetValue();
}


///////////////////////////////////
//       taivMemberPtrArgType     //
///////////////////////////////////

int taivMemberPtrArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(argt->DerivesFrom(TA_MemberDef) && (argt->ptr == 1))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}


cssEl* taivMemberPtrArgType::GetElFromArg(const char* nm, void*) {
  arg_val = new cssTA(NULL, 1, &TA_MemberDef, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}

taivData* taivMemberPtrArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  MemberDef* init_md = typ->members.SafeEl(0);
  if(*((MemberDef**)arg_base) != NULL)
    init_md = *((MemberDef**)arg_base);
  taivMemberDefMenu* rval = new taivMemberDefMenu(taivMenu::menubar, taivMenu::small,
						  init_md, dlg, par);
  rval->typd = typ;
  rval->GetMenu();
  return rval;
}

void taivMemberPtrArgType::GetImage(taivData* dat, void* base, ivWindow*) {
  if(arg_base == NULL)
    return;
  taivMemberDefMenu* rval = (taivMemberDefMenu*)dat;
  rval->md = (MemberDef*)*((void**)arg_base); 
  taivEditDialog* dlg = taivM->FindEdit(base, typ);
  if(dlg != NULL) {
    rval->menubase = typ;
    rval->ta_menu->ResetMenu();
    ivResource::flush();
    MemberSpace* mbs = &(typ->members);
    int i;
    for(i=0;i<mbs->size;i++){
      MemberDef* mbd = mbs->FastEl(i);
      if(!mbd->ShowMember(dlg->show)) continue;
      rval->ta_menu->AddItem(mbd->GetLabel(),mbd,taivMenu::use_default,(taivMenuAction*)NULL);
    }
  }
  MemberDef* initmd = (MemberDef*)*((void**)arg_base);
  if((initmd != NULL) && typ->InheritsFrom(initmd->GetOwnerType()))
    rval->ta_menu->GetImage(initmd);
  else
    rval->ta_menu->GetImage(0);	// just get first on list
}

void taivMemberPtrArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  taivMemberDefMenu* rval = (taivMemberDefMenu*)dat;
  *((void**)arg_base) = rval->GetValue();
}


///////////////////////////////////
//       taivMethodPtrArgType     //
///////////////////////////////////

int taivMethodPtrArgType::BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  if(argt->DerivesFrom(TA_MethodDef) && (argt->ptr == 1))
    return taivArgType::BidForArgType(aidx,argt,md,td)+1;
  return 0;
}


cssEl* taivMethodPtrArgType::GetElFromArg(const char* nm, void*) {
  arg_val = new cssTA(NULL, 1, &TA_MethodDef, nm);
  arg_base = (void*)&(((cssTA*)arg_val)->ptr);
  return arg_val;
}

taivData* taivMethodPtrArgType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  MethodDef* init_md = typ->methods.FindName("Load");
  if(*((MethodDef**)arg_base) != NULL)
    init_md = *((MethodDef**)arg_base);
  taivMethodDefMenu* rval = new taivMethodDefMenu(taivMenu::menubar, taivMenu::small,
						  init_md, dlg, par);
  rval->typd = typ;
  rval->GetMenu();
  return rval;
}

void taivMethodPtrArgType::GetImage(taivData* dat, void*, ivWindow*) {
  if(arg_base == NULL)
    return;
  taivMethodDefMenu* rval = (taivMethodDefMenu*)dat;
  rval->md = (MethodDef*)*((void**)arg_base); 
  rval->menubase = typ;
  rval->ta_menu->ResetMenu();
  ivResource::flush();
  MethodSpace* mbs = &(typ->methods);
  int i;
  for(i=0;i<mbs->size;i++){
    MethodDef* mbd = mbs->FastEl(i);
    if(mbd->iv == NULL) continue;
    if((mbd->name == "Close") || (mbd->name == "DuplicateMe") || (mbd->name == "ChangeMyType")	
       || (mbd->name == "SelectForEdit") || (mbd->name == "SelectFunForEdit")
       || (mbd->name == "Help"))
      continue;
    rval->ta_menu->AddItem(mbd->GetLabel(),mbd,taivMenu::use_default,(taivMenuAction*)NULL);
  }
  MethodDef* initmd = (MethodDef*)*((void**)arg_base);
  if((initmd != NULL) && typ->InheritsFrom(initmd->GetOwnerType()))
    rval->ta_menu->GetImage(initmd);
  else
    rval->ta_menu->GetImage(0);	// just get first on list
}

void taivMethodPtrArgType::GetValue(taivData* dat, void*) {
  if(arg_base == NULL)
    return;
  taivMethodDefMenu* rval = (taivMethodDefMenu*)dat;
  *((void**)arg_base) = rval->GetValue();
}




//////////////////////////////
//    taivDefaultEditDialog  //
//////////////////////////////

// this special edit is for defualt instances
class taivDefaultEditDialog : public taivEditDialog {
public:
  void	Constr_Labels();	
  void	Constr_Data();
  void	GetImage();
  void	GetValue();

  MemberSpace   mspace;		// special copy of the mspace (added toggles)

  taivDefaultEditDialog(TypeDef* tp, void * base);
  taivDefaultEditDialog(taivDefaultEditDialog&)	{ }; // avoid copy constr bug
  ~taivDefaultEditDialog();
};
  
taivDefaultEditDialog::taivDefaultEditDialog(TypeDef* tp, void* base)
: taivEditDialog(tp, base)
{
  typ = tp;
  cur_base = base;
  int i;
  for(i=0;i<tp->members.size;i++){
    MemberDef* md = new MemberDef(*(tp->members.FastEl(i)));
    mspace.Add(md);
    md->iv = tp->members.FastEl(i)->iv;	// set this here
    if(md->iv != NULL) {
      taivTDefaultMember* tdm = new taivTDefaultMember(md,tp);
      tdm->bid = md->iv->bid +1;
      tdm->AddIvm(md);
    }
  }
}

taivDefaultEditDialog::~taivDefaultEditDialog(){
  data_el.Reset();
  int i;
  for(i=0; i < mspace.size; i++) {
    MemberDef* md = mspace.FastEl(i);
    if(md->iv != NULL)
      delete md->iv;
    md->iv = NULL;
  }
  mspace.Reset();
}

void taivDefaultEditDialog::Constr_Labels() {
  Constr_Labels_style();
  GetVFix(taivM->name_font);
  GetHFix(taivM->name_font, mspace);
  labels = layout->vbox();
  Constr_Labels_impl(mspace);
  wkit->pop_style();
}

void taivDefaultEditDialog::Constr_Data() {
  data_g = layout->vbox();
  Constr_Data_impl(mspace,data_el);
  FocusOnFirst();
  GetImage();
}

void taivDefaultEditDialog::GetValue() {
  GetValue_impl(mspace, data_el,cur_base);
  if(typ->InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)cur_base;
    rbase->UpdateAfterEdit();	// hook to update the contents after an edit..
    taivMisc::Update(rbase);

    taBase_List* gp = typ->defaults;
    TypeDefault* tpdflt = NULL;
    if(gp != NULL) {
      int i;
      for(i=0; i<gp->size; i++) {
	TypeDefault* td = (TypeDefault*)gp->FastEl(i);
	if(td->token == rbase) {
	  tpdflt = td;
	  break;
	}
      }
    }
    if(tpdflt != NULL) 
      tpdflt->UpdateToNameValue();
  }
  Unchanged();
}

void taivDefaultEditDialog::GetImage() {
  GetImage_impl(mspace, data_el,cur_base, cur_win);
  Unchanged();
}

int taivDefaultEdit::Edit(void* base, ivWindow* win, bool wait, bool readonly, const ivColor* bgclr) {
  taivEditDialog* dlg = new taivDefaultEditDialog(typ, base);
  dlg->cancel_only = readonly;
  dlg->Constr(win, wait, "", "", false, bgclr);
  return dlg->Edit();
}


////////////////////////////////
//  taivMisc:Initialize()    //
////////////////////////////////

void taivMisc::Initialize() {
  int i,j,k;

  // todo: make this ifdef'd
  cssivSession::Init();		// setup the interaction between iv & css

  // allocate iv structures in typedefs
  taivM = new taivMisc();

  taivMisc::iv_active = true;		// now it is activated
  taMisc::iv_active = true;		// now it is activated

  TypeDef* td;
  // generate a list of all the iv types  

  TypeSpace iv_type_space;
  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFrom(TA_taivType) && (td->instance != NULL)
       && !(td->InheritsFrom(TA_taivMember) || td->InheritsFrom(TA_taivMethod) ||
	    td->InheritsFrom(TA_taivArgType) || td->InheritsFrom(TA_taivEdit)))
      iv_type_space.Link(td);
  }

  // generate a list of all the member_iv types
  TypeSpace iv_memb_space;
  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFrom(TA_taivMember) && (td->instance != NULL))
      iv_memb_space.Link(td);
  }

  // generate a list of all the method_iv types
  TypeSpace iv_meth_space;
  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFrom(TA_taivMethod) && (td->instance != NULL))
      iv_meth_space.Link(td);
  }

  // generate a list of all the method arg types to be used later
  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFrom(TA_taivArgType) && (td->instance != NULL))
      taivMisc::arg_types.Link(td);
  }

  // generate a list of all the ive types (edit dialogs)
  TypeSpace iv_edit_space;
  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFrom(TA_taivEdit) && (td->instance != NULL))
      iv_edit_space.Link(td);
  }

  if(iv_type_space.size == 0)
    taMisc::Error("taivInit: warning: no taivType's found with instance != NULL");
  if(iv_memb_space.size == 0)
    taMisc::Error("taivInit: warning: no taivMembers's found with instance != NULL");
  if(iv_edit_space.size == 0)
    taMisc::Error("taivInit: warning: no taivEdit's found with instance != NULL");

  // go through all the types and assign the highest bid for
  //   the iv and ive
  int bid;

  for(i=0;i<taMisc::types.size;i++){
    td = taMisc::types.FastEl(i);
    for(j=0;j<iv_type_space.size;j++) {
      taivType* tiv_i = (taivType*) iv_type_space.FastEl(j)->GetInstance();
      if((bid = tiv_i->BidForType(td)) > 0) {
	taivType* tiv = tiv_i->TypInst(td); // make one
	tiv->bid = bid;
	tiv->AddIv(td);		// add it
      }
    }
  }

  for(i=0;i<taMisc::types.size;i++){
    td = taMisc::types.FastEl(i);
    for(j=0;j<iv_edit_space.size;j++) {
      taivEdit* tive_i = (taivEdit*) iv_edit_space.FastEl(j)->GetInstance();
      if((bid = tive_i->BidForEdit(td)) > 0) {
	taivEdit* tive = (taivEdit*)tive_i->TypInst(td);
	tive->bid = bid;
	tive->AddIve(td);
      }
    }
  }

  // go though all the types and find the ones that are classes
  // for each class type go through the members and assign
  // the highest bid for the member's iv (may be based on opts field)
  // and do the enum types since they are not global and only on members

  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFormal(TA_class)) {
      for(j=0; j<td->members.size; j++) {
	MemberDef* md = td->members.FastEl(j);
	if(md->owner->owner != td) continue; // if we do not own this mdef, skip
	for(k=0;k<iv_memb_space.size;k++){
	  taivMember* tivm_i = (taivMember*) iv_memb_space.FastEl(k)->GetInstance();
	  if ((bid = tivm_i->BidForMember(md,td)) > 0 ) {
	    taivMember* tivm = tivm_i->MembInst(md,td);
	    tivm->bid = bid;
	    tivm->AddIvm(md);
	  }
	}
      }
      for(j=0;j<td->sub_types.size;j++){
	TypeDef* subt = td->sub_types.FastEl(j);
	for(k=0;k<iv_type_space.size;k++) {
	  taivType* tiv_i = (taivType*) iv_type_space.FastEl(k)->GetInstance();
	  if ((bid = tiv_i->BidForType(subt)) > 0) {
	    taivType* tiv = tiv_i->TypInst(subt); // make one
	    tiv->bid = bid;
	    tiv->AddIv(subt);		// add it
	  }
	}
      }
    }
  }

  // only assign method iv's to those methods that do better than the default
  // (which has a value of 0).  Thus, most methods don't generate a new object here

  for(i=0; i<taMisc::types.size; i++) {
    td = taMisc::types.FastEl(i);
    if(td->InheritsFormal(TA_class)) {
      for(j=0; j<td->methods.size; j++) {
	MethodDef* md = td->methods.FastEl(j);
	if(md->owner->owner != td) continue; // if we do not own this mdef, skip
	for(k=0;k<iv_meth_space.size;k++){
	  taivMethod* tivm_i = (taivMethod*) iv_meth_space.FastEl(k)->GetInstance();
	  if ((bid = tivm_i->BidForMethod(md,td)) > 0 ) {
	    taivMethod* tivm = tivm_i->MethInst(md,td);
	    tivm->bid = bid;
	    tivm->AddIvm(md);
	  }
	}
      }
    }
  }
#ifdef CYGWIN
  extern void iv_display_scale(float scale);
  iv_display_scale(taMisc::mswin_scale);
#endif
}
