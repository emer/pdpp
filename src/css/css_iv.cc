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


// These are some routines to handle simultaneous use of css and interviews

#include <css/css_iv.h>
#include <css/basic_types.h>
#include <css/c_ptr_types.h>
#include <css/ta_css.h>
#include <ta/taiv_data.h>
#include <stdlib.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/font.h>
#include <InterViews/color.h>
#include <InterViews/background.h>
#include <InterViews/event.h>
#include <InterViews/composition.h>
#include <InterViews/patch.h>
#ifndef CYGWIN
#include <IV-X11/xevent.h>
#endif
#include <ta/leave_iv.h>

extern "C" {
  extern int readline_waitproc(void);
  extern int (*rl_event_hook)(void);	// this points to the waitproc if running IV
  extern int rl_done;			// set this to kill readline..
}

int readline_waitproc() {
  return cssivSession::Run();
}

bool cssivSession::stdin_event = false;
bool cssivSession::block_stdin = false;
bool cssivSession::block_in_event = false;
bool cssivSession::done_busy = false;
bool cssivSession::in_session = false;
cssivSession* cssivSession::instance = NULL;
taivEditDialog_List cssivSession::active_edits;

int (*cssivSession::WaitProc)() = NULL; // user sets this to a work process 

cssivSession::cssivSession() {
  stdin_event = block_stdin = in_session = false;
}

void cssivSession::Init() {
  rl_event_hook = readline_waitproc; // set the hook to our "waitproc"
  rl_done = false;

  if(instance == NULL)
    instance = new cssivSession;

  // the link function first arg spec's the file desc. index
  // and 0 == stdin.  Thus, this is handling stdin.
  Dispatcher::instance().link(0, Dispatcher::ReadMask, instance);
  Dispatcher::instance().link(0, Dispatcher::ExceptMask, instance);
}

void cssivSession::Stop() {
  in_session = false;
  ivSession::instance()->quit();
}

void cssivSession::Quit() {
  in_session = false;
  ivSession::instance()->quit();
}

void cssivSession::ProcEvent(ivSession* ses) {
  if(rl_done) return;
  ivEvent e;
  ses->read(e);		// it waits here for events
#ifndef CYGWIN
  if(block_in_event) {
    ivEventType etyp = e.type();
    if(etyp==ivEvent::other_event) {
//        ivEventRep* er = e.rep();
//        cerr << "Event type: " << er->xevent_.type << endl;
      e.handle();
    }
  }
  else {
    e.handle();
  }
#else
  // blocking input doesn't work for cygwin -- all events must be processed immediately!
  e.handle();
#endif
}
  

int cssivSession::Run() {
  stdin_event = false;
  in_session = true;
  
  int post_wait_proc = false;

  ivSession* ses = ivSession::instance();
  ses->unquit();

  while(!ses->done() && in_session && !rl_done) {
    ProcEvent(ses);

    if(!ses->done() && in_session && !ses->pending() && !rl_done) {
      int rval = false;
      if(WaitProc != NULL) 	// then its safe to do some work
	rval = (*WaitProc)();
      if(!rval && post_wait_proc) {
	// do special things that can only be done post wait proc..
	post_wait_proc = false;
      }
      else
	post_wait_proc = rval;
      if(!rval && done_busy) {
	taivMisc::DoneBusy_impl();
	done_busy = false;
      }
      ivResource::flush();	// flush at this point

      // loops down into a next shell..
      if(cssMisc::next_shell != NULL) {
	cssProgSpace* nxt_shl = cssMisc::next_shell;
	cssMisc::next_shell = NULL;
	nxt_shl->CtrlShell(*(nxt_shl->fin), *(nxt_shl->fout), nxt_shl->name);
      }
    }
  }

  in_session = false; 	// make sure its false
  ses->unquit();	// return session to undone state..

  if(!stdin_event) {
    // this causes infinite loop of non-stdin events..
//    taMisc::Error("non-stdin event triggered exit from session");
  }
  return stdin_event;
}

// returns if it was stopped or not..

int cssivSession::RunPending() {
  stdin_event = false;
  in_session = true;

  ivSession* ses = ivSession::instance();
  ses->unquit();

  ivEvent e;
  while(ses->pending() && !ses->done() && cssivSession::in_session) {
    ses->read(e);		// don't do the ProcEvent here
    e.handle();
  }
  return (ses->done() || !(cssivSession::in_session));
}


int cssivSession::inputReady(int) {
  stdin_event = true;	// set the flag
  if(!block_stdin)
    Stop();		// break from whatever loop might be running..
  return 0;
}

int cssivSession::outputReady(int) {
  stdin_event = true;	// set the flag
  if(!block_stdin)
    Stop();		// break from whatever loop might be running..
  return 0;
}

// this is apparently if some problem occurs, not sure how to deal with it
// at this point..
int cssivSession::exceptionRaised(int) {
  stdin_event = true;
  Stop();
  taMisc::Error("exception raised on stdin in cssivSession");
  return 0;
}


void cssivSession::CancelProgEdits(cssProgSpace* prsp) {
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    cssivEditDialog* dlg = (cssivEditDialog*)active_edits.FastEl(i);
    if((dlg->state == taivDialog::ACTIVE) && ((dlg->top == prsp) || (dlg->top == NULL)))
      dlg->Cancel();
  }
  taivMisc::PurgeDialogs();
}

void cssivSession::CancelClassEdits(cssClassType* cltyp) {
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    cssivEditDialog* dlg = (cssivEditDialog*)active_edits.FastEl(i);
    if((dlg->state == taivDialog::ACTIVE) && (dlg->obj->type_def == cltyp))
      dlg->Cancel();
  }
  taivMisc::PurgeDialogs();
}

void cssivSession::CancelObjEdits(cssClassInst* clobj) {
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    cssivEditDialog* dlg = (cssivEditDialog*)active_edits.FastEl(i);
    if((dlg->state == taivDialog::ACTIVE) && (dlg->obj == clobj))
      dlg->Cancel();
  }
  taivMisc::PurgeDialogs();
}

void cssivSession::RaiseObjEdits() {
  int i;
  for(i=0;i<10;i++)
    RunPending();
  for(i=active_edits.size-1; i>=0; i--) {
    cssivEditDialog* dlg = (cssivEditDialog*)active_edits.FastEl(i);
    if(dlg->state == taivDialog::ACTIVE)
      dlg->Raise();
  }
  for(i=0;i<10;i++)
    RunPending();
}

//////////////////////////////////////////////////
// 		cssivPolyData			//
//////////////////////////////////////////////////

cssivPolyData::cssivPolyData(cssClassInst* ob, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  obj = ob;
  show = taMisc::show_iv;
}

cssivPolyData::~cssivPolyData() {
  type_el.Reset();
  data_el.Reset();
  ivResource::unref(rep);  rep = NULL;
}

ivGlyph* cssivPolyData::GetLook() {
  return rep;
}

void cssivPolyData::GetDataRep(ivInputHandler* ih, taivDialog* dlg) {
  rep = taivM->layout->hbox();
  ivResource::ref(rep);
  int i;
  for(i=0; i<obj->members->size; i++) {
    cssEl* md = obj->members->FastEl(i);
    if((obj->type_def != NULL) && !obj->type_def->MbrHasOption(i, "SHOW") &&
       (obj->type_def->MbrHasOption(i, "HIDDEN") ||
	obj->type_def->MbrHasOption(i, "HIDDEN_INLINE") ||
	obj->type_def->MbrHasOption(i, "READ_ONLY")))
      continue;
    bool read_only = false;
    if((obj->type_def != NULL) && obj->type_def->MbrHasOption(i, "READ_ONLY"))
      read_only = true;
    cssivType* tv = cssivEditDialog::GetTypeFromEl(md, read_only); // get the actual object..
    if((tv == NULL) || (tv->cur_base == NULL))
      continue;
    type_el.Add(tv);
    taivData* mb_dat = tv->GetDataRep(ih, dlg, this);
    data_el.Add(mb_dat);
    String nm = md->GetName();
    rep->append(taivM->layout->center(taivM->wkit->label(nm), 0, 0));
    rep->append(taivM->hfsep);
    rep->append(taivM->layout->vcenter(mb_dat->GetLook(),0));
    if(i != (obj->members->size -1))
      rep->append(taivM->hfsep);
  }
}

void cssivPolyData::GetImage(void*, ivWindow* win) {
  int i;
  for(i=0; i<type_el.size; i++) {
    cssivType* tv = (cssivType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    tv->GetImage(mb_dat, win);
  }
}

void cssivPolyData::GetValue(void*) {
  int i;
  for(i=0; i<type_el.size; i++) {
    cssivType* tv = (cssivType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    tv->GetValue(mb_dat);
  }
}


//////////////////////////////////////////////////
// 		cssivType			//
//////////////////////////////////////////////////

cssivType::cssivType(cssEl* orgo, TypeDef* tp, void* bs, bool use_ptr_type) : taivType(tp) {
  orig_obj = orgo;
  cur_base = bs;
  use_iv = NULL;
  if(use_ptr_type) {
    use_iv = new taivTokenPtrType(tp);
    use_iv->no_setpointer = true; // don't set pointers for css pointers!
  }
}
cssivType::~cssivType() {
  if(use_iv != NULL)
    delete use_iv;
}

taivData* cssivType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  if(use_iv != NULL)
    return use_iv->GetDataRep(ih, dlg, par);
  else
    return typ->iv->GetDataRep(ih, dlg, par);
}

void cssivType::GetImage(taivData* dat, void* base, ivWindow* win) {
  if(use_iv != NULL)
    use_iv->GetImage(dat, base, win);
  else
    typ->iv->GetImage(dat, base, win);
}

void cssivType::GetValue(taivData* dat, void* base) {
  if(use_iv != NULL)
    use_iv->GetValue(dat, base);
  else
    typ->iv->GetValue(dat, base);
}


//////////////////////////////////////////////////
// 		cssivROType			//
//////////////////////////////////////////////////

cssivROType::cssivROType(cssEl* orgo, TypeDef* tp, void* bs, bool use_ptr_type)
  : cssivType(orgo, tp, bs, use_ptr_type)
{
}

taivData* cssivROType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivLabel* rval = new taivLabel(dlg, par);
  return rval;
}

void cssivROType::GetImage(taivData* dat, void* base, ivWindow*) {
  taivLabel* rval = (taivLabel*)dat;
  String strval = typ->GetValStr(base);
  rval->GetImage(strval);
}

void cssivROType::GetValue(taivData*, void*) {
}


//////////////////////////////////////////////////
// 		cssivEnumType			//
//////////////////////////////////////////////////

cssivEnumType::cssivEnumType(cssEl* orgo, cssEnumType* enum_typ, void* bs)
: cssivType(orgo, &TA_enum, bs, false) {
  enum_type = enum_typ;
}

taivData* cssivEnumType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivMenu* rval = new taivMenu
    (taivMenu::menubar, taivMenu::radio_update, taivMenu::small, dlg, par);
  int i;
  for(i=0; i<enum_type->enums->size; i++) {
    rval->AddItem(enum_type->enums->FastEl(i)->name);
  }
  return rval;
}

void cssivEnumType::GetImage(taivData* dat, void* base, ivWindow*) {
  taivMenu* rval = (taivMenu*)dat;
  int enm_val = *((int*)base);
  int i;
  for(i=0; i<enum_type->enums->size; i++) {
    if(((cssInt*)enum_type->enums->FastEl(i))->val == enm_val) {
      rval->GetImage(i);
      break;
    }
  }
}

void cssivEnumType::GetValue(taivData* dat, void* base) {
  taivMenu* rval = (taivMenu*)dat;
  taivMenuEl* cur = rval->GetValue();
  if(cur) {
    cssEl* itm = enum_type->enums->El(cur->itm_no);
    if(itm != &cssMisc::Void)
      *((int*)base) = (int)*itm;
  }
}

//////////////////////////////////////////////////
//              cssivClassType                  //
//////////////////////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(cssivClassType)
implementActionCallback(cssivClassType)
#include <ta/leave_iv.h>

cssivClassType::cssivClassType(cssEl* orgo, void* bs)
: cssivType(orgo, &TA_void, bs, false) {
}

taivData* cssivClassType::GetDataRep(ivInputHandler* ih, taivDialog* dlg, taivData* par) {
  cssClassInst* obj = (cssClassInst*) cur_base;
  if((obj->type_def != NULL) && (obj->type_def->HasOption("INLINE")
				 || obj->type_def->HasOption("EDIT_INLINE"))) {
    cssivPolyData* rval = new cssivPolyData(obj, dlg, par);
    rval->GetDataRep(ih, dlg);
    return rval;
  }
  else {
    taivMenu* rval = new taivMenu
      (taivMenu::menubar, taivMenu::normal_update, taivMenu::small, dlg, par);
    rval->AddItem("Edit", NULL, taivMenu::use_default, 
		  new ActionCallback(cssivClassType)(this, &cssivClassType::CallEdit));
    String lbl = String(obj->GetTypeName()) + ": Actions";
    rval->SetMLabel(lbl);
    return rval;
  }
}

void cssivClassType::GetImage(taivData* dat, void* base, ivWindow* win) {
  cssClassInst* obj = (cssClassInst*) cur_base;
  if((obj->type_def != NULL) && (obj->type_def->HasOption("INLINE")
				 || obj->type_def->HasOption("EDIT_INLINE"))) {
    cssivPolyData* rval = (cssivPolyData*)dat;
    rval->GetImage(base, win);
  }
}

void cssivClassType::GetValue(taivData* dat, void* base) {
  cssClassInst* obj = (cssClassInst*) cur_base;
  if((obj->type_def != NULL) && (obj->type_def->HasOption("INLINE")
				 || obj->type_def->HasOption("EDIT_INLINE"))) {
    cssivPolyData* rval = (cssivPolyData*)dat;
    rval->GetValue(base);
  }
}

void cssivClassType::CallEdit() {
  cssClassInst* obj = (cssClassInst*) cur_base;
  obj->Edit();
}

//////////////////////////////////////////////////
//              cssivArrayType                  //
//////////////////////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(cssivArrayType)
implementActionCallback(cssivArrayType)
#include <ta/leave_iv.h>

cssivArrayType::cssivArrayType(cssEl* orgo, void* bs)
: cssivType(orgo, &TA_void, bs, false) {
}

taivData* cssivArrayType::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  taivMenu* rval = new taivMenu
    (taivMenu::menubar, taivMenu::normal_update, taivMenu::small, dlg, par);
  rval->AddItem("Edit", NULL, taivMenu::use_default, 
                new ActionCallback(cssivArrayType)(this, &cssivArrayType::CallEdit));
  cssArray* obj = (cssArray*) cur_base;
  String lbl = String(obj->GetTypeName()) + ": Actions";
  rval->SetMLabel(lbl);
  return rval;
}

void cssivArrayType::GetImage(taivData*, void*, ivWindow*) {
}

void cssivArrayType::GetValue(taivData*, void*) {
}

void cssivArrayType::CallEdit() {
  cssArray* obj = (cssArray*) cur_base;

  cssClassInst* arg_obj = new cssClassInst("Select element of array to edit");
  arg_obj->name = "Select element: (0-" + String((int)obj->items->size-1) + ")";
  cssInt* eldx = new cssInt(0, "index");
  arg_obj->members->Push(eldx);
  cssivEditDialog* carg_dlg = new cssivEditDialog(arg_obj);
  carg_dlg->Constr(NULL, true);
  int ok_can = carg_dlg->Edit();	// true = wait for a response
  if(ok_can) {
    cssEl* el = obj->items->El(eldx->val);
    el->Edit();
  }
  delete carg_dlg;
  delete arg_obj;
}

//////////////////////////////////////////////////
// 		cssivMethMenu			//
//////////////////////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(cssivMethMenu)
implementActionCallback(cssivMethMenu)
#include <ta/leave_iv.h>

cssivMethMenu::cssivMethMenu(cssClassInst* ob, cssProgSpace* tp, cssMbrScriptFun* cfn,
			     taivDialog* dlg, taivData* par)
: taivMethMenu(NULL, (MethodDef*)NULL, (TypeDef*)NULL, dlg, par) {
  obj = ob;
  top = tp;
  css_fun = cfn;
  arg_obj = NULL;
  if(css_fun->HasOption("BUTTON")) {
    is_menu_item = false;
    rep = taivM->wkit->push_button
      (css_fun->name,
       new ActionCallback(cssivMethMenu)(this, &cssivMethMenu::CallFun));
    ivResource::ref(rep);
  }
  else {
    rep = NULL;
    is_menu_item = true;
  }
}

cssivMethMenu::~cssivMethMenu() {
  if(rep != NULL)
    ivResource::unref(rep);
  rep = NULL;
}

void cssivMethMenu::AddToMenu(taivMenu* mnu) {
  if(css_fun->HasOption("MENU_SEP_BEFORE"))
    mnu->AddSep();
  mnu->AddItem(css_fun->name, NULL, taivMenu::use_default, 
	       new ActionCallback(cssivMethMenu)(this, &cssivMethMenu::CallFun));
  if(css_fun->HasOption("MENU_SEP_AFTER"))
    mnu->AddSep();
}

ivGlyph* cssivMethMenu::GetLook() {
  return taivM->layout->margin
    (taivM->small_flex_button(((ivButton *) rep)), taivM->hsep_c);
}

void cssivMethMenu::CallFun() {
  ApplyBefore();
  use_argc = css_fun->argc;
  String argc_str = css_fun->OptionAfter("ARGC_");
  if(argc_str != "")
    use_argc = (int)argc_str + 1;

  // get appropriate context to run function in
  cssProg* prg = cssMisc::CallFunProg;
  cssProgSpace* old_top = NULL;
  if(top != NULL)
    old_top = cssMisc::SetCurTop(top);
  else
    old_top = cssMisc::SetCurTop(cssMisc::Top);
  cssProgSpace* old_prg_top = prg->SetTop(cssMisc::cur_top);

  if((use_argc <= 1) && !css_fun->HasOption("CONFIRM")) {
    prg->Stack()->Push(&cssMisc::Void);	// argstop
    prg->Stack()->Push(obj);
    cssEl::Ref(obj);		// make sure it doesn't get to 0 refcount in arg del..
    css_fun->Do(prg);
    cssEl::unRef(obj);		// undo that
    cssEl* rval = prg->Stack()->Pop();
    cssEl::Ref(rval);
    rval->prog = prg;
    GenerateScript();
    UpdateAfter();
    if(rval != &cssMisc::Void)
      ShowReturnVal(rval);
    cssEl::unRefDone(rval);
    prg->PopTop(old_prg_top);
    cssMisc::PopCurTop(old_top);
    return;
  }
  arg_obj = new cssClassInst("args for: ");
  arg_obj->name = css_fun->name + ": " + css_fun->desc;
  int i;
  for(i=2; i<=use_argc; i++) {
    cssEl* av = css_fun->argv[i].El()->Clone();
    if(av->GetType() == cssEl::T_Class) {
      ((cssClassInst*)av)->ConstructToken();
      arg_obj->members->Push(av); // currently not saving to string..
    }
    else {
      arg_obj->members->Push(av);
      if(!css_fun->arg_vals[i].empty())	// set to saved values
	*av = css_fun->arg_vals[i];
    }
  }
  
  cssivEditDialog* carg_dlg = new cssivEditDialog(arg_obj);
  carg_dlg->Constr(NULL, true);
  int ok_can = carg_dlg->Edit();	// true = wait for a response
  if(ok_can) {
    prg->Stack()->Push(&cssMisc::Void);	// argstop
    prg->Stack()->Push(obj);	// always the first arg
    int i;
    for(i=0; i<arg_obj->members->size; i++) {
      cssEl* av = arg_obj->members->FastEl(i);
      prg->Stack()->Push(av);
      css_fun->arg_vals[2 + i] = av->GetStr(); // set cur val based on arg val
      if(av->GetType() == cssEl::T_Class)
	((cssClassInst*)av)->DestructToken();
    }
    cssEl::Ref(obj);		// make sure it doesn't get to 0 refcount in arg del..
    css_fun->Do(prg);
    cssEl::unRef(obj);		// undo that
    cssEl* rval = prg->Stack()->Pop();
    cssEl::Ref(rval);
    rval->prog = prg;
    GenerateScript();
    UpdateAfter();
    if(rval != &cssMisc::Void)
      ShowReturnVal(rval);
    cssEl::unRefDone(rval);
  }
  delete carg_dlg;
  delete arg_obj;
  prg->PopTop(old_prg_top);
  cssMisc::PopCurTop(old_top);
}

void cssivMethMenu::ShowReturnVal(cssEl* rval) {
  if(!css_fun->HasOption("USE_RVAL"))
    return;

  if((rval->GetType() == cssEl::T_TA) || (rval->GetType() == cssEl::T_Class)) {
    if(dialog != NULL)
      rval->Edit(dialog->waiting);
    else
      rval->Edit(false);
    return;
  }
  String val = css_fun->name + " Return Value: ";
  val += rval->PrintStr();
  taivDialog::WaitDialog(NULL, val, "Return Value", true);
}

void cssivMethMenu::ApplyBefore() {
  if((dialog == NULL) || (dialog->state != taivDialog::ACTIVE))
    return;
  if(css_fun->HasOption("NO_APPLY_BEFORE") || !dialog->prev_modified)
    return;
  if(taMisc::auto_revert == taMisc::CONFIRM_REVERT) {
    int chs = taivChoiceDialog::WaitDialog
      (NULL, "Auto Apply/Revert: You have edited the dialog--apply or revert and lose changes?,Apply,Revert");
    if(chs == 0)
      dialog->GetValue();
  }
  else {
    dialog->GetValue();
  }
}
    
void cssivMethMenu::UpdateAfter() {
  if(css_fun->HasOption("NO_REVERT_AFTER"))
     return;
  if((dialog == NULL) || (dialog->state != taivDialog::ACTIVE))
    return;
  dialog->Revert();		// apply stuff dealt with already
}

void cssivMethMenu::GenerateScript() {
}


//////////////////////////////////////////////////
// 		cssivEditDialog			//
//////////////////////////////////////////////////

cssivEditDialog::cssivEditDialog(cssClassInst* ob, cssProgSpace* tp)
: taivEditDialog(NULL, NULL) {
  obj = ob;
  top = tp;
  if(ob->type_def->HasOption("NO_OK"))
    no_ok_but = true;
  if(ob->type_def->HasOption("NO_CANCEL"))
    no_can_but = true;
}

cssivEditDialog::~cssivEditDialog() {
  type_el.Reset();
}

ivGlyph* cssivEditDialog::GetNameRep(int idx, cssEl* md) {
//   String nm = md->GetName();
//   ivGlyph* rval = layout->hfixed(taivM->lft_sep(wkit->label(nm)), hfix);
//   return rval;
  String nm = md->GetName();
  String desc;
  if((obj->type_def != NULL) && (obj->type_def->member_desc.size > idx)) 
     desc = obj->type_def->member_desc[idx];
  ivMenu* rval = wkit->menubar();
  ivMenu* dscm = wkit->pulldown();
  ivMenuItem* newlab = wkit->menubar_item
    (layout->hfixed(taivM->lft_sep(wkit->label(nm)), hfix));
  newlab->menu(dscm);
  rval->append_item(newlab);
  dscm->append_item(wkit->menu_item(wkit->label((char*)desc)));
  return rval;
}

void cssivEditDialog::Constr_Labels() {
  Constr_Labels_style();
  GetVFix(taivM->name_font);
  // GetHFix();
  ivFontBoundingBox fbb;
  int i;
  hfix = 0;
  for(i=0; i< obj->members->size; i++) {
    cssEl* md = obj->members->FastEl(i);
    String nm = md->GetName();
    taivM->name_font->string_bbox(nm, nm.length(), fbb);
    hfix = MAX(fbb.width(), hfix);
  }

  labels = layout->vbox();
  for(i=0; i< obj->members->size; i++) {
    cssEl* md = obj->members->FastEl(i);
    if((obj->type_def != NULL) && !obj->type_def->MbrHasOption(i, "SHOW") &&
       (obj->type_def->MbrHasOption(i, "HIDDEN") || obj->type_def->MbrHasOption(i, "READ_ONLY")))
      continue;
    bool read_only = false;
    if((obj->type_def != NULL) && obj->type_def->MbrHasOption(i, "READ_ONLY"))
      read_only = true;
    cssivType* tv = GetTypeFromEl(md, read_only); // get the actual object..
    if((tv == NULL) || (tv->cur_base == NULL))
      continue;
    type_el.Add(tv);
    // 32 is for the openlook extra v symbol...
    labels->append(layout->fixed(taivM->top_sep(GetNameRep(i, md)), hfix + 32, vfix));
  }
  wkit->pop_style();
}

void cssivEditDialog::Constr_Data() {
  data_g = layout->vbox();
  int i;
  for(i=0; i<type_el.size; i++) {
    cssivType* tv = (cssivType*)type_el.FastEl(i);
    taivData* mb_dat = tv->GetDataRep(dialog, this);
    data_el.Add(mb_dat);
    data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()),vfix));
  }
  FocusOnFirst();
  GetImage();
}

void cssivEditDialog::Constr_Body() {
  if(menu != NULL) {
    body_box->prepend(taivM->vfsep);
    body_box->prepend(layout->hcenter(menu, 0));
  }
  if(meth_buts != NULL) {
    float ht = 1.4f * vfix;
    String rows;
    if(obj != NULL)
      rows = obj->type_def->OptionAfter("BUTROWS_");
    if(!rows.empty()) {
      ht *= (float)rows;
    }
    else {
      ivLRComposition* cm = (ivLRComposition*)meth_buts;
      if(cm->count() > 12)
	ht *= 3;
      else if(cm->count() > 6)
	ht *= 2;
    }
    meth_butg = layout->vflexible(layout->vnatural(layout->center(meth_buts, 0, 0),ht),fil,1.4f * vfix);
    body_box->append(meth_butg);
    body_box->append(taivM->vfspc);
  }
  body_box->append(but_patch);

  body = new ivPatch(layout->margin(body_box, taivM->hsep_c));
  ivResource::ref(body);
}

void cssivEditDialog::GetValue() {
  int i;
  for(i=0; i<type_el.size; i++) {
    cssivType* tv = (cssivType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    tv->GetValue(mb_dat);
    tv->orig_obj->UpdateAfterEdit();
  }
  obj->UpdateAfterEdit();
  Unchanged();
}

void cssivEditDialog::GetImage() {
  int i;
  for(i=0; i<type_el.size; i++) {
    cssivType* tv = (cssivType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    tv->GetImage(mb_dat, cur_win);
  }
  Unchanged();
}

void cssivEditDialog::Constr_Strings(const char*, const char* win_title) {
  prompt_str = obj->GetTypeName();
  prompt_str += String(" ") + obj->name + ": ";
  if(obj->type_def != NULL)
    prompt_str += obj->type_def->desc;
  win_str = String(ivSession::instance()->classname()) + ": " + win_title;
  win_str += String(" ") + obj->name;
}

cssivType* cssivEditDialog::GetTypeFromEl(cssEl* el, bool read_only) {
  cssEl* orig_obj = el;
  if(el->IsRef() || (el->GetType() == cssEl::T_Ptr))
    el = el->GetActualObj();

  switch(el->GetType()) {
  case cssEl::T_Bool:
    if(read_only)
      return new cssivROType(orig_obj, &TA_bool, (void*)&(((cssBool*)el)->val));
    return new cssivType(orig_obj, &TA_bool, (void*)&(((cssBool*)el)->val));
  case cssEl::T_Int:
    if(read_only)
      return new cssivROType(orig_obj, &TA_int, (void*)&(((cssInt*)el)->val));
    return new cssivType(orig_obj, &TA_int, (void*)&(((cssInt*)el)->val));
  case cssEl::T_Real:
    if(read_only)
      return new cssivROType(orig_obj, &TA_double, (void*)&(((cssReal*)el)->val));
    return new cssivType(orig_obj, &TA_double, (void*)&(((cssReal*)el)->val));
  case cssEl::T_Array:
    return new cssivArrayType(orig_obj, (void*)el);
  case cssEl::T_Class:
    return new cssivClassType(orig_obj, (void*)el);
  case cssEl::T_String:
    return new cssivType(orig_obj, &TA_taString, (void*)&(((cssString*)el)->val));
  case cssEl::T_Enum:
    return new cssivEnumType(orig_obj, ((cssEnum*)el)->type_def,
			     (void*)&(((cssEnum*)el)->val));
  case cssEl::T_C_Ptr: {
    String sb_typ = el->GetTypeName();
    if(sb_typ == "(c_int)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_int, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_int, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_bool)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_bool, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_bool, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_short)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_short, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_short, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_long)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_long, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_long, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_char)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_char, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_char, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_double)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_double, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_double, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_float)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_float, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_float, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_String)") {
      if(read_only)
	return new cssivROType(orig_obj, &TA_taString, (void*)(((cssCPtr*)el)->ptr));
      return new cssivType(orig_obj, &TA_taString, (void*)(((cssCPtr*)el)->ptr));
    }
    if(sb_typ == "(c_enum)") {
      cssCPtr_enum* enm = (cssCPtr_enum*)el;
      MemberDef* md = enm->GetEnumType();
      if(md != NULL)
	return new cssivType(orig_obj, md->type, (void*)(((cssCPtr*)el)->ptr));
      else
	return new cssivType(orig_obj, &TA_int, (void*)(((cssCPtr*)el)->ptr));
    }
    break;
  }
  case cssEl::T_TA: {
    TypeDef* td = ((cssTA*)el)->type_def;
    if(td == NULL) return NULL;
    // todo: put in support for fstreams, enums, etc.
    if(td->DerivesFrom(TA_taBase)) {
      if(td->ptr == 0)
	return new cssivType(orig_obj, td, (void*)&(((cssTA*)el)->ptr), true); // needs ptr type
      else
	return new cssivType(orig_obj, td, (void*)&(((cssTA*)el)->ptr)); // already has ptr type
    }
    if(td->DerivesFrom(TA_TypeDef) && (td->ptr == 1))
      return new cssivType(orig_obj, &TA_TypeDef_ptr, (void*)&(((cssTA*)el)->ptr));
    else if(td->DerivesFrom(TA_MemberDef) && (td->ptr == 1))
      return new cssivType(orig_obj, &TA_MemberDef_ptr, (void*)&(((cssTA*)el)->ptr));
    else if(td->DerivesFrom(TA_MethodDef) && (td->ptr == 1))
      return new cssivType(orig_obj, &TA_MethodDef_ptr, (void*)&(((cssTA*)el)->ptr));
      
    return NULL;
  }
  default:
    return NULL;
  }
  return NULL;
}


void cssivEditDialog::Constr_Methods() {
  if((obj == NULL) || (obj->type_def == NULL))
    return;
  cssClassType* td = obj->type_def;
  int i;
  for(i=0; i<td->methods->size; i++) {
    cssMbrScriptFun* md = (cssMbrScriptFun*)td->methods->FastEl(i);
    if((md->GetType() == cssEl::T_MbrCFun) ||
       md->is_tor || md->HasOption("HIDDEN"))
      continue;
    cssivMethMenu* mth_rep = new cssivMethMenu(obj, top, md, this);
    meth_el.Add(mth_rep);
    if(mth_rep->is_menu_item) {
      GetMenuRep(md);
      mth_rep->AddToMenu(cur_menu);
    }
    else {
      if(meth_buts == NULL)
	GetButtonRep();
//        if(meth_buts->count() > 0)
//  	meth_buts->append(taivM->hsep);
      meth_buts->append(mth_rep->GetLook());
    }
  }
}

void cssivEditDialog::GetMenuRep(cssMbrScriptFun* md) {
  if(menu == NULL)
    menu = wkit->menubar();
  String men_nm = md->OptionAfter("MENU_ON_");
  if(men_nm != "") {
    cur_menu = ta_menus.FindName(men_nm);
    if(cur_menu != NULL)
      return;
  }
  if(cur_menu != NULL)
    return;
    
  if(men_nm == "")
    men_nm = "Actions";
  cur_menu = new 
    taivMenu(taivMenu::menuitem, taivMenu::normal, taivMenu::small,
	      men_nm);
  menu->append_item(cur_menu->GetMenuItem());
  ta_menus.Add(cur_menu);
}

int cssivEditDialog::Edit() {
  if(!waiting)
    cssivSession::active_edits.Add(this);
  return taivEditDialog::Edit();
}
