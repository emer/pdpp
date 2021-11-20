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

// taiv_dialog.cc


#include <ta/taiv_dialog.h>
#include <ta/taiv_type.h>
#include <ta/ta_base.h>
#include <css/css_iv.h>
#include <css/basic_types.h>
#include <css/ta_css.h>

#include <ta/enter_iv.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <InterViews/display.h>
#include <InterViews/font.h>
#include <InterViews/event.h>
#include <InterViews/hit.h>
#include <InterViews/color.h>
#include <InterViews/label.h>
#include <InterViews/background.h>
#include <InterViews/cursor.h> // for record_cursor
#include <InterViews/patch.h>
#include <InterViews/composition.h>
#include <InterViews/simplecomp.h>
#include <InterViews/texcomp.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <IV-look/menu.h>
#include <InterViews/style.h>
#include <InterViews/handler.h>
#ifndef CYGWIN
#include <IV-X11/xdisplay.h>
#include <X11/keysym.h>
#endif
#include <ta/leave_iv.h>

// pointer alignment for putting up windows based on the mouse
#define POINTER_ALIGN_X		.5
#define POINTER_ALIGN_Y		.5


//////////////////////////////////////////////////
// 		ScriptButton			//
//////////////////////////////////////////////////

ScriptButton::ScriptButton(char* name, ivAction* a, char* srp) 
: ivButton(NULL,NULL,new ivTelltaleState,a) {
  script = srp;
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivButton* temp = wkit->push_button(name,NULL);
  ivResource::ref(temp);
  body(temp->body());
  state(temp->state());
  ivResource::unref(temp);
}

void ScriptButton::release(const ivEvent& e){
  taivMisc::RecordScript(script);
  ivButton::release(e);
}

//////////////////////////////////////////////////
// 		HiLightButton			//
//////////////////////////////////////////////////

HiLightButton::HiLightButton(char* name, ivAction* action,
			     char* srp,bool unhilight_enable)
: ivDeck(2) {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivButton* un_hilight_button;
  ivButton* hilight_button;
  if(srp == NULL)
    un_hilight_button = wkit->push_button(name,action);
  else
    un_hilight_button = new ScriptButton(name,action,srp);
  wkit->push_style(taivM->apply_button_style);
  if(srp == NULL) 
    hilight_button = wkit->push_button(name,action);
  else
    hilight_button = new ScriptButton(name,action,srp);
  wkit->pop_style();
  un_hilight_button->state()->set(ivTelltaleState::is_enabled,unhilight_enable);

  append(un_hilight_button);
  append(hilight_button);
  UnHiLight();
}

// returns previous state
bool  HiLightButton::HiLight(){
  if(card() != 1) { flip_to(1); return 0; }
  else return 1;
}

// returns previous state
bool  HiLightButton::UnHiLight(){
  if(card() != 0) { flip_to(0); return 1; }
  else return 0;
}

bool HiLightButton::Set(bool hilight){
  bool result = (bool) card();
  flip_to(hilight);
  return result;
}

//////////////////////////////////////////////////
// 		NoBlockDialog			//
//////////////////////////////////////////////////

class DialogWMDeleteHandler : public ivHandler {
public:
  NoBlockDialog*	dlg;
  osboolean event(ivEvent& ev);
  DialogWMDeleteHandler(NoBlockDialog* obj);
};

DialogWMDeleteHandler::DialogWMDeleteHandler(NoBlockDialog* obj) : ivHandler() {
  dlg = obj;
}

osboolean DialogWMDeleteHandler::event(ivEvent&) {
  if(dlg->owner->modified) {
    int chs = taMisc::Choice("Changes have not been applied", "Apply", "Abandon Changes", "Cancel Close");
    if(chs == 2)
      return true;
    else if(chs == 0) {
      dlg->owner->GetValue();
      dlg->owner->state = taivDialog::ACCEPTED;
    }
    else {
      dlg->owner->state = taivDialog::CANCELED;
    }
    return true;
  }
  dlg->owner->state = taivDialog::CANCELED;
  return true;
}

NoBlockDialog::NoBlockDialog(taivDialog* dlg, ivGlyph* g, ivStyle* s)
  : ivDialog(g, s) {
  owner = dlg;
  was_accepted = false;
  win = NULL;
  last_focus = 0;
}

NoBlockDialog::~NoBlockDialog() {
  owner = NULL;
  win = NULL;
}

osboolean NoBlockDialog::post_for_aligned(ivWindow* w, float x_align, float y_align) {
  if(w == NULL) {		// safety..
    ivCoord x=0,y=0;
    taivDialog::GetPointerPos(x,y);
    return post_at_aligned(x, y, POINTER_ALIGN_X, POINTER_ALIGN_Y);
  }
  if((owner != NULL) && owner->waiting) {
    win = NULL;
    return ivDialog::post_for_aligned(w, x_align, y_align);
  }
  IconGlyph* ig = new IconGlyph(this); // this handle the wait cursor setting
  win = new ivTopLevelWindow(ig);
  ivHandler* delh = new DialogWMDeleteHandler(this);
  win->wm_delete(delh);
  ig->window = win;
  win->style(new ivStyle(style()));
  win->group_leader(w);
  win->place(w->left() + 0.5 * w->width(), w->bottom() + 0.5 * w->height());
  win->align(x_align, y_align);
  win->map();
  taivMisc::active_wins.Add(win);
  return ostrue;
}

osboolean NoBlockDialog::post_at_aligned(ivCoord x, ivCoord y, float x_align, float y_align) {
  if((owner != NULL) && owner->waiting) {
    win = NULL;
    return ivDialog::post_at_aligned(x, y, x_align, y_align);
  }
  IconGlyph* ig = new IconGlyph(this); // this handle the wait cursor setting
  win = new ivTopLevelWindow(ig);
  ivHandler* delh = new DialogWMDeleteHandler(this);
  win->wm_delete(delh);
  ig->SetWindow(win);
  win->style(new ivStyle(style()));
  win->place(x, y);
  win->align(x_align, y_align);
  win->map();
  taivMisc::active_wins.Add(win);
  return ostrue;
}

void NoBlockDialog::dismiss(osboolean accept) {
  ivDialog::dismiss(accept);
  was_accepted = accept;
  if(win != NULL) {
    win->unmap();
    win->display()->sync();
    taivMisc::active_wins.Remove(win);
    delete win;
    win = NULL;
  }
}  

void NoBlockDialog::raise() {
  if(win != NULL)
    win->raise();
}

void NoBlockDialog::keystroke(const ivEvent& e) {
  if(special_key(e)) return;
  ivDialog::keystroke(e);
}

bool NoBlockDialog::special_key(const ivEvent& e){
  if(owner == NULL) return false;
  if(e.type() != ivEvent::key) return false;
#ifndef CYGWIN
  if(e.len != 0) return false;
#endif
  char c;
  e.mapkey(&c,1);
  if(c == '\033') {		// ESCAPE
    owner->Cancel();
    return true;
  }
  if(c == '\r')	{		// RETURN
    owner->Ok(); 
    return true;
  }
#ifndef CYGWIN
  if((c == '\020') || (e.keysym() == XK_Up)) 	// CTRL-P, up
#else
  if((c == '\020'))
#endif
    {
      prev_focus();
      return true;
    }
#ifndef CYGWIN
  if((c == '\016') || (e.keysym() == XK_Down))	// CTRL-N, down
#else
  if((c == '\016'))
#endif
    {
      next_focus();
      return true;
    }
  if((c == '\t') && e.shift_is_down()) { // SHFT-TAB
    prev_focus();
    return true;
  }
  return false;
}

void NoBlockDialog::prev_focus(){
  last_focus =-1;
  ivDialog::prev_focus();
}
void NoBlockDialog::next_focus(){
  last_focus = 1;
  ivDialog::next_focus();
}

//////////////////////////////////////////////////////////
// 			taivDialog			//
//////////////////////////////////////////////////////////

class taivDButton : public ivButton {
public:
  taivDialog* diag;
  virtual void release(const ivEvent&);
  static taivDButton* GetButton(taivDialog* d, const char* s, ivAction* a);
  taivDButton(taivDialog* d, ivGlyph* g, ivStyle* s, ivTelltaleState* t, ivAction* a);
};

taivDButton::taivDButton(taivDialog* d, ivGlyph* g, ivStyle* s, ivTelltaleState* t, ivAction* a)
: ivButton(g,s,t,a) {
  diag = d;
}
  
taivDButton* taivDButton::GetButton(taivDialog *d, const char* s, ivAction* a){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* t = new ivTelltaleState(ivTelltaleState::is_enabled);
  return new taivDButton(d,wkit->default_button_look(wkit->label(s),t),
		      wkit->style(),t,a);
}
  
void taivDButton::release(const ivEvent& e) {
  diag->mouse_button = taivM->GetButton(e);
  ivButton::release(e);
}

ivWidgetKit* taivDialog::wkit = NULL;
ivDialogKit* taivDialog::dkit = NULL;
ivLayoutKit* taivDialog::layout = NULL;
// **TODO maybe each dialog should have its own style so that
// it could update its name ect...
ivStyle*     taivDialog::style = NULL;


#include <ta/enter_iv.h>
declareActionCallback(taivDialog)
implementActionCallback(taivDialog)
#include <ta/leave_iv.h>

taivDialog::taivDialog() {
  if(wkit == NULL) {
    wkit = ivWidgetKit::instance();
    dkit = ivDialogKit::instance();
    layout = ivLayoutKit::instance();
    // **todo When does this get deleted?
    style = new ivStyle(wkit->style());
    ivResource::ref(style);
  }
  prompt = NULL;
  box = NULL;
  okbut = NULL;
  canbut = NULL;
  apply_but = NULL;
  revert_but = NULL;
  but_box = NULL;
  but_patch = NULL;
  body_box = NULL;
  body = NULL;
  dialog = NULL;
  bg_color = NULL;
  bg_color_dark = NULL;
  cur_win = NULL;
  state = EXISTS;
  mouse_button = 0;
  modified = false;
  prev_modified = false;
  typ = NULL;
  cur_base = NULL;
  no_revert_hilight = false;
  cancel_only = false;
  no_can_but = false;
  no_ok_but = false;
}

taivDialog::~taivDialog() {
  CloseWindow();
}

void taivDialog::CloseWindow() {
  ivResource::unref(body);    body= NULL;
  ivResource::unref(dialog);  dialog = NULL;

  ivResource::unref(bg_color);    bg_color= NULL;
  ivResource::unref(bg_color_dark);  bg_color_dark = NULL;

  prompt = NULL;
  box = NULL;
  okbut = NULL;
  canbut = NULL;
  apply_but = NULL;
  revert_but = NULL;
  but_box = NULL;
  but_patch = NULL;
  body_box = NULL;
  body = NULL;
  dialog = NULL;
  cur_win = NULL;
  state = EXISTS;
  mouse_button = 0;
  modified = false;
  prev_modified = false;
}

void taivDialog::Constr_Dialog() {
  dialog = new NoBlockDialog(this, taivM->hspc, style);
  ivResource::ref(dialog);
}

void taivDialog::Constr_Strings(const char* aprompt, const char* win_title) {
  prompt_str = aprompt;
  win_str = win_title;
}

void taivDialog::Constr_Buttons() {
  if(!waiting && no_ok_but) {
    okbut = NULL;
  }
  else {
    okbut = taivDButton::GetButton
      (this, "Ok", new ActionCallback(taivDialog)(this, &taivDialog::Ok));
  }
  if(no_can_but) {
    canbut = NULL;
  }
  else {
    canbut = wkit->push_button
      ("Cancel", new ActionCallback(taivDialog)(this, &taivDialog::Cancel));
  }
  if(waiting) {
    apply_but = NULL;
    revert_but = NULL;
    if(no_can_but)      but_box = layout->hbox(okbut, taivM->hspc);
    else                but_box = layout->hbox(okbut, taivM->hspc, canbut);
  }
  else {
    apply_but = new HiLightButton
      ("Apply", new ActionCallback(taivDialog)(this, &taivDialog::Apply),
       NULL,false);
    revert_but = new HiLightButton
      ("Revert", new ActionCallback(taivDialog)(this, &taivDialog::Revert));

    if(no_ok_but) {
      but_box = layout->hbox(taivM->medium_button(apply_but), taivM->hspc,
			     taivM->medium_button(revert_but));
    }
    else {
      but_box = layout->hbox(taivM->medium_button(okbut), taivM->hspc,
			     taivM->medium_button(apply_but), taivM->hspc,
			     taivM->medium_button(revert_but));
    }

    if(!no_can_but) {
      but_box->append(taivM->hspc);
      but_box->append(taivM->medium_button(canbut));
    }
    Unchanged();
  }
  but_patch = new ivPatch(layout->center(but_box,0,0));
}

void taivDialog::Constr_WinName() {
//    if(win_str.length() > 50)
//      win_str = win_str.from((int)(win_str.length() - 50));
  style->attribute("name", win_str);
  style->attribute("title", win_str);
  String inm = win_str;
  if(inm.contains(':'))
    inm = inm.after(':');
  if(inm.length() > 10)
    inm = inm.from((int)(inm.length() - 10));
  style->attribute("iconName", inm);
}

void taivDialog::Constr_Prompt() {
//   wkit->push_style(taivM->title_style);
//   ivCoord wdth = 300.0;
//   if(typ != NULL) {
//     String ewd = typ->OptionAfter("EDIT_WIDTH_");
//     if(!ewd.empty()) {
//       ivCoord wd = (float)ewd;
//       if(wd > 100.0) {
// 	wdth = wd;
// // 	prompt = layout->hfixed(prompt, wd);
//       }
//     }
//   }
//   prompt = new ivLRComposition(layout->vbox(), new ivSimpleCompositor(),
// 			       taivM->hsep, wdth, fil, 0.0);
//   int len = prompt_str.length();
//   int pos = 0;
//   char c = prompt_str[pos++];
//   while(pos < len) {
//     String wrd;
//     while((pos < len) && (c != ' ') && (c != '\t') && (c != '\n') && (c != '\r')) {
//       wrd += c;
//       c = prompt_str[pos++];
//     }
//     wrd += c;
//     prompt->append(wkit->fancy_label(wrd));
//     while((pos < len) && ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))) {
//       c = prompt_str[pos++];
//     }
//   }
//   ((ivLRComposition*)prompt)->repair();

//   wkit->pop_style();

  int max_wd = taMisc::display_width;

  if(typ != NULL) {
    String ewd = typ->OptionAfter("EDIT_WIDTH_");
    if(!ewd.empty()) {
      int wd = (int)ewd;
      if(wd > 10) {
	max_wd = wd;
      }
    }
  }
  // cut lines at \n's and after each 80 chars
  wkit->push_style(taivM->title_style);
  if(((int)prompt_str.length() <= max_wd) && !(prompt_str.contains('\n')))  {
    prompt = taivM->cen_sep(wkit->fancy_label(prompt_str));
    wkit->pop_style();
    return;
  }
  prompt = layout->vbox();
  String str = prompt_str;
  String one_line;
  int wdth = 0;
  while(!str.empty()) {
    String wrd;
    if(str.contains(' ')) {
      wrd = str.before(' ');
      str = str.after(' ');
    }
    else {
      wrd = str;
      str = "";
    }
    if(wdth + (int)wrd.length() > max_wd) {
      if(one_line.contains('\n')) {
	while(one_line.contains('\n')) {
	  String tmp = one_line.before('\n');
	  one_line = one_line.after('\n');
	  prompt->append(taivM->cen_sep(wkit->fancy_label(tmp)));
	}
	wdth = one_line.length();
      }
      else {
	prompt->append(taivM->cen_sep(wkit->fancy_label(one_line)));
	one_line = "";
	wdth = 0;
      }
    }
    one_line += wrd + " ";
    wdth += wrd.length() + 1;
  }
  one_line += str;
  while(one_line.contains('\n')) {
    String tmp = one_line.before('\n');
    one_line = one_line.after('\n');
    prompt->append(taivM->cen_sep(wkit->fancy_label(tmp)));
  }
  if(!one_line.empty())
    prompt->append(taivM->cen_sep(wkit->fancy_label(one_line)));

  wkit->pop_style();
}

void taivDialog::Constr_Box() {
  box = taivM->hspc;
}

void taivDialog::Constr_BodyBox() {
  body_box = layout->vbox(layout->hcenter(prompt,0),
			  taivM->vfsep,
			  layout->hcenter(wkit->menu_item_separator_look(),0),
			  taivM->vfspc,
			  layout->hcenter(box,0),
			  taivM->vfspc);
}

void taivDialog::Constr_Body() {
  body_box->append(but_patch);
  body = new ivPatch(layout->margin(body_box, taivM->hsep_c));
  ivResource::ref(body);
}

void taivDialog::Constr_Final() {
  dialog->body(new ivBackground(layout->center(body), bg_color));
}

void taivDialog::Constr(ivWindow* win, bool wait, const char* aprompt,
			 const char* win_title, bool no_cancel, const ivColor* bgclr)
{
  waiting = wait;
  if(no_cancel) no_can_but = no_cancel;
  cur_win = win;
  if(bg_color != NULL)
    ivResource::unref(bg_color);
  if(bg_color_dark != NULL)
    ivResource::unref(bg_color_dark);
  if(bgclr != NULL)
    bg_color = bgclr;
  else
    bg_color = wkit->background();
  ivResource::ref(bg_color);
  bg_color_dark = bg_color->brightness(taivM->edit_darkbg_brightness);
  ivResource::ref(bg_color_dark);
  Constr_Dialog();
  Constr_Strings(aprompt, win_title);
  Constr_Buttons();
  Constr_WinName();
  Constr_Prompt();
  Constr_Box();
  Constr_BodyBox();
  Constr_Body();
  Constr_Final();
  state = CONSTRUCTED;
}

int taivDialog::Edit() {
  if(state != CONSTRUCTED)
    return false;
  if(!waiting) {
    taivMisc::active_dialogs.AddUnique(this); // add to the list of active dialogs
  }
  state = ACTIVE;
  int rval;
  ivCoord x=0,y=0;
  GetPointerPos(x,y);
  rval = dialog->post_at_aligned(x, y, POINTER_ALIGN_X, POINTER_ALIGN_Y);
  return rval;
}

void taivDialog::Ok() {
  if(cancel_only == true){
    taMisc::Error("This is a read-only dialog, you must press cancel");
    return;
  }
  if(HasChanged())
    GetValue();
  state = ACCEPTED;
  dialog->dismiss(1);
}

void taivDialog::Cancel() {
  state = CANCELED;
  dialog->dismiss(0);
}

void taivDialog::Apply() {
  if(cancel_only == true){
    taMisc::Error("This is a read-only dialog, Apply is ignored");
    return;
  }
  no_revert_hilight = true;
  GetValue();
  GetImage();
  UnSetRevert();
  if(state == ACTIVE)		// could be canceled by get image..
    ReDraw();
  no_revert_hilight = false;
}

void taivDialog::Revert_force() {
  if(prev_modified && (taMisc::auto_revert == taMisc::CONFIRM_REVERT)) {
    int chs = taivChoiceDialog::WaitDialog
      (cur_win, "Revert: You have edited the dialog--apply or revert and lose changes?!Apply!Revert!Cancel!");
    if(chs == 2)
      return;
    if(chs == 0) {
      Apply();
      return;
    }
  }
  Unchanged();
  Revert();			// use real revert to be sure..
}

void taivDialog::Revert() {
  GetImage();
  UnSetRevert();
  if(state == ACTIVE)		// could be canceled by get image..
    ReDraw();
}

bool taivDialog::HasChanged() {
  return modified;
}

void taivDialog::SetRevert(){
  if((no_revert_hilight == true) || (taMisc::is_loading)) return;
  if(revert_but == NULL) return;
  revert_but->HiLight();
  if(but_patch != NULL) but_patch->redraw();
}

void taivDialog::UnSetRevert(){
  if(revert_but == NULL) return;
  revert_but->UnHiLight();
  if(but_patch != NULL) but_patch->redraw();
}

void taivDialog::Changed() {
  prev_modified = modified;
  modified = true;
  if(apply_but == NULL)
    return;
  apply_but->HiLight();
  if(but_patch != NULL) but_patch->redraw();
}

void taivDialog::Unchanged() {
  modified = false;
  prev_modified = false;
  if(apply_but == NULL)
    return;
  apply_but->UnHiLight();
  if(but_patch != NULL) but_patch->redraw();
}

void taivDialog::ReDraw() {
  if(!waiting) {
    dialog->win->canvas()->damage_all();
    dialog->win->repair();
    //    dialog->win->resize();
  }
  else {
    body->redraw();
  }
}

void taivDialog::Raise() {
  if(!waiting)
    ((NoBlockDialog*)dialog)->raise();
}


// allow for pixel 1/2 width of window when getting pointer pos for
// purpose of displaying window
#define taiv_dialog_half_win	250

void taivDialog::GetPointerPos(ivCoord& x, ivCoord& y) {
  ivDisplay* dsp = ivSession::instance()->default_display();
  int rootx, rooty;
#ifndef CYGWIN
  ivDisplayRep* d = dsp->rep();
  int winx, winy;
  unsigned int butts;
  XWindow child;		// this is the window, we don't care..
  XWindow root;			// this is the root window, 
  XQueryPointer(d->display_, d->root_, &root, &child, &rootx, &rooty,
		&winx, &winy, &butts);
#else
  rootx = dsp->pwidth() / 2;
  rooty = dsp->pheight() / 2;
#endif
  int max_x = dsp->pwidth() - taiv_dialog_half_win;
  int max_y = dsp->pheight() - taiv_dialog_half_win;
  rooty = dsp->pheight() - rooty;
  rootx = MAX(taiv_dialog_half_win, rootx);
  rootx = MIN(max_x, rootx);
  rooty = MAX(taiv_dialog_half_win, rooty);
  rooty = MIN(max_y, rooty);
  x = dsp->to_coord(rootx);
  y = dsp->to_coord(rooty);
}

int taivDialog::WaitDialog(ivWindow* win, const char* prompt, const char* win_title,
			    bool no_cancel, const ivColor* bgclr)
{
  taivDialog* dlg = new taivDialog;
  dlg->Constr(win, true, prompt, win_title, no_cancel, bgclr);
  int rval = dlg->Edit();
  delete dlg;
  return rval;
}


//////////////////////////////////////////////////////////
// 		taivChoiceDialog			//
//////////////////////////////////////////////////////////

class ChoiceNBDialog : public NoBlockDialog {
public:
  ChoiceNBDialog(taivChoiceDialog* cdg, ivGlyph* g, ivStyle* s)
  : NoBlockDialog(cdg,g,s) { };
  bool special_key(const ivEvent& e);
};

bool ChoiceNBDialog::special_key(const ivEvent& e) {
  taivChoiceDialog* cdg = (taivChoiceDialog*)owner;
  if(cdg == NULL) return false;
  if(e.type() != ivEvent::key) return false;
#ifndef CYGWIN
  if(e.len != 0) return false;
#endif
  char c;
  if(e.mapkey(&c,1)) {
    switch (c) {
    case '0':		
    case '\r': 		cdg->But0Press(); return true;	// RETURN
    case '1':
    case '\033':	cdg->But1Press(); return true; 	// ESCAPE
    case '2':		cdg->But2Press(); return true;
    case '3':		cdg->But3Press(); return true;
    case '4':		cdg->But4Press(); return true;
    case '5':		cdg->But5Press(); return true;
    case '6':		cdg->But6Press(); return true;
    case '7':		cdg->But7Press(); return true;
    case '8':		cdg->But8Press(); return true;
    case '9':		cdg->But8Press(); return true;
    default:
      owner->Changed();
    }
  }
  return false;
}

#include <ta/enter_iv.h>
declareActionCallback(taivChoiceDialog)
implementActionCallback(taivChoiceDialog)
#include <ta/leave_iv.h>

void taivChoiceDialog::Constr_Dialog() {
  dialog = new ChoiceNBDialog(this, taivM->hspc, style);
  ivResource::ref(dialog);
}

void taivChoiceDialog::Constr_Strings(const char* aprompt, const char* win_title) {
  prompt_str = aprompt;
  but_labels = prompt_str.after('!');
  prompt_str = prompt_str.before('!');
  win_str = win_title;
}

bool taivChoiceDialog::Constr_OneBut(String& lbl, void (taivChoiceDialog::*fun)()) {
  if(lbl.length() == 0)
    return false;
  String blab = lbl.before('!');
  if(blab == "") {
    blab = lbl;
    lbl = "";			// done next time, anyway
  }
  else 
    lbl = lbl.after('!');
  if(blab == "")
    return false;

  if(blab[0] == ' ')		// allow for one space..
    blab = blab.after(' ');

  ivButton* but;
  if(but_box->count() > 0) {
    but = wkit->push_button(blab, new ActionCallback(taivChoiceDialog)(this, fun));
    but_box->append(taivM->hspc);
  }
  else
    but = wkit->default_button(blab, new ActionCallback(taivChoiceDialog)(this, fun));
  but_box->append(but);
  return true;
}
  

void taivChoiceDialog::Constr_Buttons() {
  but_box = layout->hbox();
  String blabs = but_labels;
  Constr_OneBut(blabs, &taivChoiceDialog::But0Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But1Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But2Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But3Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But4Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But5Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But6Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But7Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But8Press);
  Constr_OneBut(blabs, &taivChoiceDialog::But9Press);
  but_patch = new ivPatch(layout->center(but_box,0,0));
}

int taivChoiceDialog::Edit() {
  taivDialog::Edit();
  if(!((state == ACCEPTED) || (state == CANCELED)))
    state = ACCEPTED;
  return result;
}

int taivChoiceDialog::WaitDialog(ivWindow* win, const char* prompt, const char* win_title,
				  bool no_cancel, const ivColor* bgclr)
{
  taivDialog* dlg = new taivChoiceDialog;
  dlg->Constr(win, true, prompt, win_title, no_cancel, bgclr);
  int rval = dlg->Edit();
  delete dlg;
  return rval;
}

//////////////////////////////////////////////////////////
// 			taivEditDialog			//
//////////////////////////////////////////////////////////

bool EditNBDialog::special_key(const ivEvent& e){
  if(owner == NULL) return false;
  if(e.type() != ivEvent::key) return false;
#ifndef CYGWIN
  if(e.len != 0) return false;
#endif
  char c = 0;
  e.mapkey(&c,1);
  if(c == '\033') {
    owner->Revert();		// revert on ESCAPE instead of Cancel
    return true;
  }
  if(c == '\r') {
    next_focus();		// focus on next for RETURN, not accept..
    return true;
  }
  return NoBlockDialog::special_key(e);
}

ivCoord taivEditDialog::vfix_font_mult = 1.60;

taivEditDialog::taivEditDialog(TypeDef* tp, void* base) {
  typ = tp;
  cur_base = base;
  show = taMisc::show_iv;
  labels = NULL;
  data_g = NULL;
  meth_buts = NULL;
  meth_butg = NULL;
  menu = NULL;
  cur_menu = NULL;
}

taivEditDialog::~taivEditDialog() {
  CloseWindow();
}

void taivEditDialog::CloseWindow() {
  data_el.Reset();
  meth_el.Reset();
  ivResource::unref(labels);    labels = NULL;
  ta_menus.Reset();
  labels = NULL;
  data_g = NULL;
  meth_buts = NULL;
  meth_butg = NULL;
  menu = NULL;
  cur_menu = NULL;
  taivDialog::CloseWindow();
}

void taivEditDialog::Constr_Dialog() {
  dialog = new EditNBDialog(this, taivM->hspc, style);
  ivResource::ref(dialog);
}

bool taivEditDialog::ShowMember(MemberDef* md) {
  return (md->ShowMember(show) && (md->iv != NULL));
}

void taivEditDialog::GetMembDescRep(MemberDef* md, ivMenu* dscm, String indent) {
  String desc = md->desc;
  String defval = md->OptionAfter("DEF_");
  if(!defval.empty())
    desc = String("[Default: ") + defval + "] " + desc;
  else
    desc = desc;
  if(!indent.empty())
    desc = indent + md->GetLabel() + String(": ") + desc;
  dscm->append_item
    (taivM->wkit->menu_item
     (new ivLabel((char*)desc, taivM->name_font, taivM->font_foreground)));
  if(md->type->InheritsFormal(TA_class) && 
     (md->type->HasOption("INLINE") || md->type->HasOption("EDIT_INLINE"))) {
    indent += "  ";
    int i;
    for(i=0; i<md->type->members.size; i++) {
      MemberDef* smd = md->type->members.FastEl(i);
      if(!smd->ShowMember(taMisc::show_iv) || smd->HasOption("HIDDEN_INLINE"))
	continue;
      GetMembDescRep(smd, dscm, indent);
    }
  }
  else if(md->type->InheritsFormal(TA_enum)) {
    int i;
    for(i=0; i<md->type->enum_vals.size; i++) {
      EnumDef* ed = md->type->enum_vals.FastEl(i);
      if(ed->desc.empty() || (ed->desc == " ") || (ed->desc == "  ")) continue;
      desc = indent + "  " + ed->GetLabel() + String(": ") + ed->desc;
      dscm->append_item
	(taivM->wkit->menu_item
	 (new ivLabel((char*)desc, taivM->name_font, taivM->font_foreground)));
    }
  }
}

ivGlyph* taivEditDialog::GetNameRep(MemberDef* md) {
  ivMenu* rval = wkit->menubar();
  ivMenu* dscm = wkit->pulldown();
  String nm = md->GetLabel();
  ivMenuItem* newlab = wkit->menubar_item
    (layout->hfixed(taivM->lft_sep
		    (new ivLabel(nm, taivM->name_font, taivM->font_foreground)),
		    hfix));
  newlab->menu(dscm);
  rval->append_item(newlab);
  GetMembDescRep(md, dscm, "");
  return rval;
}

void taivEditDialog::GetVFix(ivFont* fnt) {
  ivFontBoundingBox fbb;
  fnt->font_bbox(fbb);
  vfix = vfix_font_mult * (fbb.font_ascent() + fbb.font_descent()) + taivM->vsep_c;
}

void taivEditDialog::GetHFix(ivFont* fnt, const MemberSpace& ms) {
  ivFontBoundingBox fbb;
  hfix = 0;
  int i;
  for(i=0; i< ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md))
      continue;
    String nm = md->GetLabel();
    fnt->string_bbox(nm, nm.length(), fbb);
    hfix = MAX(fbb.width(), hfix);
  }
}

void taivEditDialog::Constr_Labels_impl(const MemberSpace& ms) {
  int i;
  for(i=0; i< ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md))
      continue;
    // 32 is for the openlook extra v symbol...
    labels->append(layout->fixed(taivM->top_sep(GetNameRep(md)), hfix + 32, vfix));
  }
}

void taivEditDialog::Constr_Labels_style() {
  if(bg_color != wkit->background()) {
    ivStyle* clrsty = new ivStyle(taivM->name_style);
    clrsty->attribute("flat", taivM->color_to_string(bg_color));
    wkit->push_style(clrsty);
  }
  else {
    wkit->push_style(taivM->name_style);
  }
}

void taivEditDialog::Constr_Labels() {
  Constr_Labels_style();
  GetVFix(taivM->name_font);
  GetHFix(taivM->name_font, typ->members);
  labels = layout->vbox();
  Constr_Labels_impl(typ->members);
  wkit->pop_style();
}

void taivEditDialog::Constr_Data_impl(const MemberSpace& ms, taivDataList& dl) {
  int i;
  for(i=0; i<ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = md->iv->GetDataRep(dialog, this);
    dl.Add(mb_dat);
    data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()),vfix));
  }
}

void taivEditDialog::Constr_Data() {
  data_g = layout->vbox();
  Constr_Data_impl(typ->members, data_el);
  GetImage();
}

void taivEditDialog::FocusOnFirst() {
  int i;
  for(i=0; i<data_el.size; i++) {
    taivData* mb_dat = data_el.FastEl(i);
    if(mb_dat->NeedsFocus()) {
      dialog->focus((ivInputHandler*)mb_dat->GetRep());
      break;
    }
  }
}

void taivEditDialog::Constr_Methods() {
  if(typ == NULL)
    return;
  int i;
  for(i=0; i<typ->methods.size; i++) {
    MethodDef* md = typ->methods.FastEl(i);
    if((md->iv == NULL) || (md->name == "Edit")) // don't put edit on edit dialogs..
      continue;
    taivMethMenu* mth_rep = md->iv->GetMethodRep(cur_base, this);
    if(mth_rep == NULL)
      continue;
    meth_el.Add(mth_rep);
    if(mth_rep->is_menu_item) {
      if(md->HasOption("MENU_BUTTON")) {
	if(meth_buts == NULL) GetButtonRep();
	GetMenuButtonRep(md);
      }
      else {
	GetMenuRep(md);
      }
      mth_rep->AddToMenu(cur_menu);
    }
    else {
      if(meth_buts == NULL) GetButtonRep();
//        if(meth_buts->count() > 0) meth_buts->append(taivM->hsep);
      meth_buts->append(mth_rep->GetLook());
    }
  }
  GetButtonImage();
  GetShowMenu();
}

void taivEditDialog::GetButtonRep() {
  meth_buts = new ivLRComposition(layout->vbox(), new ivSimpleCompositor(),
				  taivM->hsep, 100, fil, taivM->small_button_width);
}

void taivEditDialog::GetMenuRep(MethodDef* md) {
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

void taivEditDialog::GetMenuButtonRep(MethodDef* md) {
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
    taivMenu(taivMenu::menubar, taivMenu::normal, taivMenu::small,
	      men_nm);
  meth_buts->append
    (layout->margin(layout->hnatural(cur_menu->GetRep(), 100.0), taivM->hsep_c));
  ta_menus.Add(cur_menu);
}

declare_taivMenuCallback(taivEditDialog)
implement_taivMenuCallback(taivEditDialog)

void taivEditDialog::GetShowMenu() {
  if(menu == NULL) return;	// if don't even have a menu, bail
  cur_menu = new 
    taivMenu(taivMenu::menuitem, taivMenu::normal, taivMenu::small, "Show");
  menu->append_item(new ivMenuItem(layout->hglue(), new ivTelltaleState)); // move it over to the right
  menu->append_item(cur_menu->GetMenuItem());

  cur_menu->AddItem("None", NULL, taivMenu::normal,
		    new taivMenuCallback(taivEditDialog)
		    (this, &taivEditDialog::ShowChange_mc));
  cur_menu->AddItem("All", NULL, taivMenu::radio,
		    new taivMenuCallback(taivEditDialog)
		    (this, &taivEditDialog::ShowChange_mc));
  cur_menu->AddItem("Hidden", NULL, taivMenu::radio,
		    new taivMenuCallback(taivEditDialog)
		    (this, &taivEditDialog::ShowChange_mc));
  cur_menu->AddItem("Read Only", NULL, taivMenu::radio,
		    new taivMenuCallback(taivEditDialog)
		    (this, &taivEditDialog::ShowChange_mc));
  cur_menu->AddItem("Detail", NULL, taivMenu::radio,
		    new taivMenuCallback(taivEditDialog)
		    (this, &taivEditDialog::ShowChange_mc));

  cur_menu->items.FastEl(1)->rep->state()->set(ivTelltaleState::is_chosen, 
					   show == 0);
  cur_menu->items.FastEl(2)->rep->state()->set(ivTelltaleState::is_chosen, 
					   !(show & taMisc::NO_HIDDEN));
  cur_menu->items.FastEl(3)->rep->state()->set(ivTelltaleState::is_chosen, 
					   !(show & taMisc::NO_READ_ONLY));
  cur_menu->items.FastEl(4)->rep->state()->set(ivTelltaleState::is_chosen, 
					   !(show & taMisc::NO_DETAIL));
}

void taivEditDialog::ShowChange_mc(taivMenuEl* sel) {
  if(waiting) {
    taMisc::Error("Cannot change what to show with a waiting dialog");
    return;
  }
  Ok();				// do an ok first!! to get data before changing show!
  if(sel->itm_no == 0)
    show = show = taMisc::NO_HID_RO_DET;
  else if(sel->itm_no == 1)
    show = 0;
  else {
    int mask = 1 << (sel->itm_no-2);
    show = show & ~mask;
    sel->rep->state()->set(ivTelltaleState::is_chosen, !(show & mask));
  }
  state = SHOW_CHANGED;
}

void taivEditDialog::GetButtonImage() {
  if(typ == NULL)
    return;
  int i, cnt = 0;
  for(i=0; i<typ->methods.size; i++) {
    MethodDef* md = typ->methods.FastEl(i);
    if((md->iv == NULL) || (md->name == "Edit")) // don't put edit on edit dialogs..
      continue;
    taivMethMenu* mth_rep = (taivMethMenu*)meth_el.SafeEl(cnt++);
    if((mth_rep == NULL) || (mth_rep->is_menu_item)
       || (md->OptionAfter("GHOST_") == ""))
      continue;
    ivButton* but = (ivButton*)mth_rep->GetRep();
    String mbnm = md->OptionAfter("GHOST_");
    String on_off = mbnm.before("_");
    mbnm = mbnm.after("_");
    MemberDef* mbrd = typ->members.FindName(mbnm);
    if((but == NULL) || (mbrd == NULL) || !(mbrd->type->InheritsFrom(TA_bool)
					  || mbrd->type->InheritsFrom(TA_int)))
      continue;
    bool but_state;
    if(mbrd->type->InheritsFrom(TA_bool))
      but_state = *((bool*)mbrd->GetOff(cur_base));
    else
      but_state = *((int*)mbrd->GetOff(cur_base));
    if(on_off == "ON") 
      but->state()->set(ivTelltaleState::is_enabled, but_state);
    else
      but->state()->set(ivTelltaleState::is_enabled, !but_state);
  }
}

void taivEditDialog::GetValue_impl(const MemberSpace& ms, const taivDataList& dl,
				  void* base)
{
  bool first_diff = true;
  int i, cnt = 0;
  for(i=0; i<ms.size; i++) {
    MemberDef* md = ms.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = dl.FastEl(cnt++);
    md->iv->GetMbrValue(mb_dat, base, first_diff);
  }
  if(!first_diff)
    taivMember::EndScript(base);
}

void taivEditDialog::GetValue() {
  GetValue_impl(typ->members, data_el, cur_base);
  if(typ->InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)cur_base;
    rbase->UpdateAfterEdit();	// hook to update the contents after an edit..
    taivMisc::Update(rbase);
  }
  GetButtonImage();
  Unchanged();
}

void taivEditDialog::GetImage_impl(const MemberSpace& ms, const taivDataList& dl, 
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
}

void taivEditDialog::GetImage() {
  GetImage_impl(typ->members, data_el, cur_base, cur_win);
  GetButtonImage();
  Unchanged();
  FocusOnFirst();
}

void taivEditDialog::Constr_Strings(const char* aprompt, const char* win_title) {
  prompt_str = typ->name;
  win_str = String(ivSession::instance()->classname()) + ": " + win_title;
  if(typ->InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)cur_base;
    if(rbase->GetOwner() != NULL)
      win_str += String(" ") + rbase->GetPath();
    if(rbase->GetName() != "") {
      win_str += String(" (") + rbase->GetName() + ")";
      prompt_str = rbase->GetName() + " (" + typ->name + ")";
    }
    else
      win_str += String(" (") + typ->name + ")";
  }
  String sapr;
  if(aprompt != NULL) sapr = aprompt;
  if(!sapr.empty())
    prompt_str += ": " + sapr;
  else
    prompt_str +=  ": " + typ->desc;
}

void taivEditDialog::Constr_Box() {
  Constr_Labels();
  Constr_Data();
  Constr_Methods();

  if(data_g->count() > 0) {
    box = layout->hflexible
      (layout->hbox
       (labels, taivM->hfspc,
	(wkit->inset_frame
	 (new ivBackground
	  (layout->hflexible(data_g),bg_color_dark)))));
  }
  else {
    box = taivM->hspc;
  }
  // under some circumstances these are not used, so we ref them
  ivResource::ref(labels);
}

void taivEditDialog::Constr_BodyBox() {
  body_box = layout->vbox(layout->hcenter(prompt,0),
			  taivM->vfsep,
			  layout->hcenter(wkit->menu_item_separator_look(),0),
			  taivM->vfspc,
			  layout->hcenter(box,0),
			  taivM->vfspc);
}

void taivEditDialog::Constr_Body() {
  if(menu != NULL) {
    body_box->prepend(taivM->vfsep);
    body_box->prepend(layout->hcenter(menu, 0));
  }
  if(meth_buts != NULL) {
    float ht = 1.4f * vfix;
    String rows;
    if(typ != NULL)
      rows = typ->OptionAfter("BUTROWS_");
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

int taivEditDialog::Edit() {
  if(!waiting)
    taivMisc::active_edits.Add(this); // add to the list of active dialogs
  return taivDialog::Edit();
}


//////////////////////////////////////////////////
// 		taivTokenDialog			//
//////////////////////////////////////////////////

taivTokenDialog::taivTokenDialog(TypeDef* td, TAPtr scp_ref, TAPtr in_itm) {
  ths = td;
  scope_ref = scp_ref;
  itm = in_itm;
}
taivTokenDialog::~taivTokenDialog() {
  if(itm_rep != NULL)
    delete itm_rep;
  itm_rep = NULL;
}
void taivTokenDialog::Constr_Box() {
  itm_rep = new taivToken(taivMenu::menubar, taivMenu::small, ths, this);
  itm_rep->scope_ref = scope_ref;
  itm_rep->GetMenu();

  box = layout->hbox
    (layout->hglue(), taivM->top_spc(layout->hbox(wkit->label("Token:"),
				  taivM->vspc, itm_rep->GetLook())), layout->hglue()); 
  itm_rep->GetImage(itm, scope_ref, cur_win);
}

void taivTokenDialog::GetValue() {
  itm = (TAPtr)itm_rep->GetValue();
}

TAPtr taivTokenDialog::GetToken(TypeDef* td, const char* prompt,
				TAPtr scp_ref, TAPtr init_itm, ivWindow* win)
{
  taivTokenDialog* tok = new taivTokenDialog(td, scp_ref, init_itm);
  tok->Constr(win, true, prompt);
  TAPtr rval;
  if(tok->Edit())
    rval = tok->itm;
  else
    rval = NULL;

  delete tok;
  return rval;
}

//////////////////////////////////////////////////
// 		taivTypeDialog			//
//////////////////////////////////////////////////

taivTypeDialog::taivTypeDialog(TypeDef* td, TypeDef* init_tp) {
  base_typ = td;
  sel_typ = init_tp;
}

taivTypeDialog::~taivTypeDialog() {
  if(typ_rep != NULL) delete typ_rep;
  typ_rep = NULL;
}

void taivTypeDialog::Constr_Box() {
  typ_rep = new taivTypeHier(taivMenu::menubar, taivMenu::small, base_typ, this);
  typ_rep->GetMenu();

  box = layout->hbox
    (layout->hglue(), taivM->top_spc(layout->hbox(wkit->label("Type:"),
				 taivM->vspc, typ_rep->GetLook())), layout->hglue()); 
  if(sel_typ != NULL)
    typ_rep->GetImage(sel_typ);
  else
    typ_rep->GetImage(base_typ);
}

void taivTypeDialog::GetValue() {
  sel_typ = typ_rep->GetValue();
}

TypeDef* taivTypeDialog::GetType(TypeDef* td, const char* prompt, TypeDef* init_tp,
				 ivWindow* win)
{
  taivTypeDialog* tok = new taivTypeDialog(td, init_tp);
  tok->Constr(win, true, prompt);
  TypeDef* rval;
  if(tok->Edit())
    rval = tok->sel_typ;
  else
    rval = NULL;

  delete tok;
  return rval;
}

//////////////////////////////////////////////////
// 		taivEnumDialog			//
//////////////////////////////////////////////////

taivEnumDialog::taivEnumDialog(TypeDef* td, int init_vl) {
  enum_typ = td;
  sel_val = init_vl;
}

taivEnumDialog::~taivEnumDialog() {
  if(enm_rep != NULL) delete enm_rep;
  enm_rep = NULL;
}

void taivEnumDialog::Constr_Box() {
  enm_rep = new taivMenu
    (taivMenu::menubar, taivMenu::radio_update, taivMenu::small, this);
  int i;
  for(i=0; i<enum_typ->enum_vals.size; i++) {
    enm_rep->AddItem(enum_typ->enum_vals.FastEl(i)->GetLabel());
  }

  box = layout->hbox
    (layout->hglue(), taivM->top_spc(layout->hbox(wkit->label("Value:"),
				 taivM->vspc, enm_rep->GetLook())), layout->hglue()); 
  EnumDef* td = enum_typ->enum_vals.FindNo(sel_val);
  if(td != NULL)
    enm_rep->GetImage(td->idx);
}

void taivEnumDialog::GetValue() {
  taivMenuEl* cur = enm_rep->GetValue();
  EnumDef* td = NULL;
  if(cur && (cur->itm_no < enum_typ->enum_vals.size))
    td = enum_typ->enum_vals.FastEl(cur->itm_no);
  if(td)
    sel_val = td->enum_no;
}

int taivEnumDialog::GetEnum(TypeDef* td, const char* prompt, int init_vl,
			    ivWindow* win)
{
  taivEnumDialog* tok = new taivEnumDialog(td, init_vl);
  tok->Constr(win, true, prompt);
  int rval;
  if(tok->Edit())
    rval = tok->sel_val;
  else
    rval = -1;

  delete tok;
  return rval;
}
