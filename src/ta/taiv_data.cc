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

// taiv_data.cc

#include <ta/taiv_data.h>
#include <ta/taiv_dialog.h>
#include <ta/taiv_type.h>
#include <css/css_iv.h>
#include <css/basic_types.h>
#include <css/ta_css.h>
#include <ta/wait_cursor.h>
#include <ta/wait_mask.h>
#include <ta/record_cursor.h>
#include <ta/record_mask.h>
#include <ta/enter_iv.h>
#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/window.h>
#include <InterViews/font.h>
#include <InterViews/handler.h>
#include <InterViews/color.h>
#include <InterViews/background.h>
#include <InterViews/bitmap.h>
#include <InterViews/cursor.h>
#include <InterViews/deck.h>
#include <InterViews/patch.h>
#include <InterViews/label.h>
#include <InterViews/target.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <IV-look/choice.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/printer.h>	// for highlightbg
#include <IV-look/menu.h>
#include <IV-look/field.h> // for scroll field editor
#include <IV-look/fbrowser.h>
#include <OS/string.h> //  for scroll field editor
#ifndef CYGWIN
#include <IV-X11/xwindow.h>	// this is for window dumps
#include <IV-X11/xdisplay.h>	// this is for window dumps
#endif
#include <ta/leave_iv.h>

#include <iv_misc/dastepper.h> // for incrfield
#include <iv_misc/dynalabel.h> // for load dialog

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

// pointer alignment for putting up windows based on the mouse
#define POINTER_ALIGN_X		.5
#define POINTER_ALIGN_Y		.5

using namespace std;

//////////////////////////////////////////////////////////
// 	taivMisc: miscellaneous useful stuff 		//
//////////////////////////////////////////////////////////

taivMisc* taivM = NULL;

bool 			taivMisc::iv_active = false;
ivWindow_List		taivMisc::active_wins;
ivWindow_List		taivMisc::delete_wins;
taivDialog_List 	taivMisc::active_dialogs;
taivEditDialog_List	taivMisc::active_edits;
NoBlockDialog*       	taivMisc::load_dlg;
TypeSpace		taivMisc::arg_types;
ostream*		taivMisc::record_script = NULL;
void (*taivMisc::Update_Hook)(TAPtr) = NULL;

int taivMisc::busy_count = 0;

int taivMisc::GetButton(const ivEvent& e) {
  if(e.pointer_button() == ivEvent::left) {
    if(e.shift_is_down()) return ivEvent::middle;
    if(e.meta_is_down()) return ivEvent::right;
    return ivEvent::left;
  }
  return(e.pointer_button());
}

void taivMisc::Update(TAPtr obj) {
  if(Update_Hook != NULL)
    (*Update_Hook)(obj);
}  

void taivMisc::Busy() {
  if(!iv_active)    return;
  busy_count++;	// keep track of number of times called
  if(cssivSession::block_in_event == true) // already busy 
    return;
  cssivSession::block_in_event = true;
  int i;
  for(i=active_wins.size-1; i>=0; i--) {
    ivWindow* win = active_wins.FastEl(i);
    if(win->is_mapped())
      SetWinCursor(win);
  }
}

void taivMisc::DoneBusy() {
  if(!iv_active)    return;
  if(--busy_count <= 0)
    cssivSession::done_busy = true;
  busy_count = MAX(0,busy_count); // don't go less than 0
}

void taivMisc::DoneBusy_impl() {
  if(!iv_active)    return;
  cssivSession::block_in_event = false;
  int i;
  for(i=active_wins.size-1; i>=0; i--) {
    ivWindow* win = active_wins.FastEl(i);
    if(win->is_mapped())
      SetWinCursor(win);
  }
}

int taivMisc::RunIV() {
  if(!iv_active)    return false;
  return cssivSession::Run();
}

int taivMisc::RunIVPending() {
  if(!iv_active)    return false;
  return cssivSession::RunPending();
}

int taivMisc::FlushIVPending() {
  if(!iv_active)    return false;
  int i;
  for(i=0;i<100;i++)
    cssivSession::RunPending();
  sleep(1);
  for(i=0;i<100;i++)
    cssivSession::RunPending();
  return cssivSession::RunPending();
}

void taivMisc::StartRecording(ostream* strm){
  record_script = strm;
  if(!iv_active) return;
  int i;
  for(i=active_wins.size-1; i>=0; i--) {
    ivWindow* win = active_wins.FastEl(i);
    if(win->is_mapped())
      SetWinCursor(win);
  }
}

void taivMisc::StopRecording(){
  record_script = NULL;
  if(!iv_active)    return;
  int i;
  for(i=active_wins.size-1; i>=0; i--) {
    ivWindow* win = active_wins.FastEl(i);
    if(win->is_mapped())
      SetWinCursor(win);
  }
}

bool taivMisc::RecordScript(const char* cmd) {
  if(record_script == NULL)
    return false;
  if(record_script->bad() || record_script->eof()) {
    taMisc::Error("*** Error: recording script is bad or eof, no script command recorded!!",
		  cmd);
    return false;
  }
  *record_script << cmd;
  if(cmd[strlen(cmd)-1] != '\n') {
    taMisc::Error("*** Warning: cmd must end in a newline, but doesn't -- should be fixed:",
		  cmd);
    *record_script << '\n';
  }
  record_script->flush();
  return true;
}

void taivMisc::SetWinCursor(ivWindow* win) {
  bool is_busy = false;
  bool is_rec = false;
  if((taivMisc::busy_count > 0) || cssivSession::block_in_event)
    is_busy = true;
  if(taivMisc::record_script !=NULL)
    is_rec = true;

  if(is_busy && is_rec) {	// both busy and recording (busy wins)
    while(win->cursor() == taivM->record_cursor)
      win->pop_cursor();	// get rid of any recording cursors..
    if(win->cursor() != taivM->wait_cursor) {
      win->push_cursor();
      win->cursor(taivM->wait_cursor);
    }
    return;
  }

  if(is_busy) {
    while(win->cursor() == taivM->record_cursor)
      win->pop_cursor();	// get rid of any recording cursors..
    if(win->cursor() != taivM->wait_cursor) {
      win->push_cursor();
      win->cursor(taivM->wait_cursor);
    }
    return;
  }    

  if(is_rec) {
    while(win->cursor() == taivM->wait_cursor)
      win->pop_cursor();	// get rid of any waiting cursors..
    if(win->cursor() != taivM->record_cursor) {
      win->push_cursor();
      win->cursor(taivM->record_cursor);
    }
    return;
  }

  // get rid of any special cursors..
  while((win->cursor() == taivM->record_cursor) ||
	(win->cursor() == taivM->wait_cursor))
      win->pop_cursor();
}


void taivMisc::PurgeDialogs() {
  bool did_purge = false;
  int i;
  for(i=active_dialogs.size-1; i>=0; i--) {
    taivDialog* dlg = active_dialogs.FastEl(i);
    if((dlg->state == taivDialog::ACCEPTED) || (dlg->state == taivDialog::CANCELED)) {
      // this list does not delete after, so its ok...
      dlg->dialog->dismiss(0);
      active_edits.Remove((taivEditDialog*)dlg);	// in case its on this list too
      cssivSession::active_edits.Remove((taivEditDialog*)dlg); // ditto
      // this one does the deleting
      active_dialogs.Remove(i);
      did_purge = true;
    }
    if(dlg->state == taivDialog::SHOW_CHANGED) {
      active_edits.Remove((taivEditDialog*)dlg);	// in case its on this list too
      cssivSession::active_edits.Remove((taivEditDialog*)dlg); // ditto
      // close it and reconstruct it!
      const ivColor* bg_color = dlg->bg_color;
      if(bg_color != NULL)
	ivResource::ref(bg_color);
      dlg->CloseWindow();
      dlg->Constr(dlg->cur_win, dlg->waiting, dlg->prompt_str, dlg->win_str, dlg->no_can_but, bg_color);
      if(bg_color != NULL)
	ivResource::unref(bg_color);
      dlg->Edit();
      did_purge = true;
    }
  }
  if(did_purge)
    ivResource::flush();		// get rid of iv's
}


bool taivMisc::CloseEdits(void* obj, TypeDef* td) {
  if(!iv_active)    return false;
  PurgeDialogs();
  bool got_one = false;
  if(!td->InheritsFrom(TA_taBase)) {
    int i;
    for(i=active_edits.size-1; i>=0; i--) {
      taivEditDialog* dlg = active_edits.FastEl(i);
      if((dlg->cur_base == obj) && (dlg->state == taivDialog::ACTIVE)) {
	dlg->Cancel();
	got_one = true;
      }
    }
    return got_one;
  }

  TAPtr ta_obj = (TAPtr)obj;
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    taivEditDialog* dlg = active_edits.FastEl(i);
    if((dlg->typ == NULL) || !dlg->typ->InheritsFrom(TA_taBase) ||
       (dlg->state != taivDialog::ACTIVE))
      continue;
    TAPtr dbase = (TAPtr)dlg->cur_base;
    if(dbase == ta_obj) {
      dlg->Cancel();
      got_one = true;
      continue;
    }
    TAPtr dbpar = dbase->GetOwner(td); // if its a parent of that type..
    if(dbpar == ta_obj) {
      dlg->Cancel();
      got_one = true;
      continue;
    }
    // also check for groups that might contain the object
    if(!dlg->typ->InheritsFrom(TA_taList_impl))
      continue;
    taList_impl* lst = (taList_impl*)dlg->cur_base;
    if(lst->Find(ta_obj) >= 0)
       dlg->SetRevert();	// it's been changed..
  }
  return got_one;
}

bool taivMisc::NotifyEdits(void* obj, TypeDef*) {
  if(!iv_active)    return false;
  bool got_one = false;
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    taivEditDialog* dlg = active_edits.FastEl(i);
    if((dlg->state != taivDialog::ACTIVE) || (dlg->cur_base == NULL) ||
       (dlg->typ == NULL))
      continue;
    // this object contains the given one
    if((dlg->cur_base <= obj) && ((char*)obj <= ((char*)dlg->cur_base + dlg->typ->size))) {
      dlg->SetRevert();
      got_one = true;
      continue;
    }
    // also check for groups that might contain the object
    if(!dlg->typ->InheritsFrom(TA_taList_impl))
      continue;
    taList_impl* lst = (taList_impl*)dlg->cur_base;
    if(lst->Find((TAPtr)obj) >= 0) {
       dlg->SetRevert();
       got_one = true;
    }
  }
  return got_one;
}

bool taivMisc::RevertEdits(void* obj, TypeDef*) {
  if(!iv_active)    return false;
  bool got_one = false;
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    taivEditDialog* dlg = active_edits.FastEl(i);
    if((dlg->cur_base == obj) && (dlg->state == taivDialog::ACTIVE)) {
      dlg->Revert_force();
      got_one = true;
    }
  }
  return got_one;
}

taivEditDialog* taivMisc::FindEdit(void* obj, TypeDef*) {
  if(!iv_active)    return NULL;
  int i;
  for(i=active_edits.size-1; i>=0; i--) {
    taivEditDialog* dlg = active_edits.FastEl(i);
    if((dlg->cur_base == obj) && (dlg->state == taivDialog::ACTIVE))
      return dlg;
  }
  return NULL;
}

void taivMisc::CreateLoadDialog(){
  if(taMisc::iv_active != true) return;
  if(load_dlg == NULL) {
    DynamicLabel* lab =
      new DynamicLabel
      ("Loading...............",taivM->wkit->font(),taivM->wkit->foreground());
    load_dlg = new NoBlockDialog
      (NULL, new ivBackground
       (taivM->layout->natural
	(taivM->layout->flexible
	 (taivM->wkit->inset_frame(taivM->wrap(lab))),140,100),
	taivM->wkit->background()),
       taivM->style);
    ivResource::ref(load_dlg);
  }
  load_dlg->post_for_aligned(NULL,0.5,1.0);
}

void taivMisc::SetLoadDialog(char* tpname){
  if(taMisc::iv_active != true) return;
  if(load_dlg == NULL) return;
  String loadstring = String("Loading: ") + String(tpname);
  ivBackground* bg = (ivBackground *) load_dlg->body();
  DynamicLabel* dlabel = (DynamicLabel *)
    ((ivPolyGlyph *)		// vbox (wrap)
     ((ivPolyGlyph*)		// hbox (wrap)
      ((ivMonoGlyph *)		// inset_frame
       ((ivMonoGlyph *)		// flexible
	((ivMonoGlyph *)		// natural
	 bg->body())
	->body())
       ->body())
      ->component(1))
     ->component(1));
  dlabel->set(loadstring.chars());
  load_dlg->win->repair();
  RunIVPending();
}

void taivMisc::RemoveLoadDialog(){
  if(taMisc::iv_active != true) return;
  if(load_dlg == NULL) return;
  RunIVPending();
  load_dlg->dismiss(1);
  RunIVPending();
}


void taivMisc::DeleteWindows() {
  int i;
  for(i=delete_wins.size-1; i>=0; i--) {
    delete delete_wins.FastEl(i);
  }
  delete_wins.RemoveAll();
}

void taivMisc::Cleanup(int) {
#ifndef __GNUG__
  String cmd = "/bin/rm ";
  cmd += taMisc::tmp_dir + "/taiv_gf." + String((int)getpid()) + ".* >/dev/null 2>&1";
  system(cmd);			// get rid of any remaining temp files
#endif
}


//////////////////////////////////////////////////////////
// 	taivMisc: helper functions for iv              //
//////////////////////////////////////////////////////////

taivMisc::taivMisc() {
  vsep_c = 2;
  hsep_c = 2;
  vspc_c = 4;
  hspc_c = 4;
  edit_darkbg_brightness = -0.15;
  edit_lightbg_brightness = 0.50;

  if(taMisc::not_constr || taMisc::in_init) {
    wkit = NULL;
    dkit = NULL;
    layout = NULL;
    style = NULL;
    vsep = NULL;
    hsep = NULL;
    vfsep = NULL;
    hfsep = NULL;
    vspc = NULL;
    hspc = NULL;
    vfspc = NULL;
    hfspc = NULL;
    name_font = NULL;
    small_menu_font = NULL;
    small_submenu_font = NULL;
    big_menu_font = NULL;
    big_submenu_font = NULL;
    big_menubar_font = NULL;
    big_italic_menubar_font = NULL;
    small_button_width=46.0;
    medium_button_width=72.0;
    big_button_width=115.0;
    title_style = NULL;
    apply_button_style = NULL;
    name_style = NULL;
    wait_cursor = NULL;
    record_cursor = NULL;
    return;
  }
    
  wkit = ivWidgetKit::instance();
  dkit = ivDialogKit::instance();
  layout = ivLayoutKit::instance();
  style = new ivStyle(wkit->style());
  ivResource::ref(style);

  vsep = layout->vglue(vsep_c);
  ivResource::ref(vsep);
  hsep = layout->hglue(hsep_c);
  ivResource::ref(hsep);
  vfsep = layout->vspace(vsep_c);
  ivResource::ref(vfsep);
  hfsep = layout->hspace(hsep_c);
  ivResource::ref(hfsep);

  vspc = layout->vglue(vspc_c);
  ivResource::ref(vspc);
  hspc = layout->hglue(hspc_c);
  ivResource::ref(hspc);
  vfspc = layout->vspace(vspc_c);
  ivResource::ref(vfspc);
  hfspc = layout->hspace(hspc_c);
  ivResource::ref(hfspc);

  font_foreground = wkit->foreground();
  ivResource::ref(font_foreground);

  edit_darkbg = wkit->background()->brightness(edit_darkbg_brightness);
  ivResource::ref(edit_darkbg); 

  edit_lightbg = new ivColor(1.0f, 1.0f, 0.0f, 1.0f); // wkit->background()->brightness(edit_lightbg_brightness);
  ivResource::ref(edit_lightbg); 

  // instead of pushing and popping styles, just get the damn fonts!
  ivStyle* sty;

  // name_style is special because it also specifies a color for the buttons
  name_style = new ivStyle(style);  name_style->alias("name");
  ivResource::ref(name_style);
  wkit->push_style(name_style);
  name_font = (ivFont*)wkit->font();  ivResource::ref(name_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("small_menu");
  wkit->push_style(sty);
  small_menu_font = (ivFont*)wkit->font();  ivResource::ref(small_menu_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("small_submenu");
  wkit->push_style(sty);
  small_submenu_font = (ivFont*)wkit->font();  ivResource::ref(small_submenu_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("big_menu");
  wkit->push_style(sty);
  big_menu_font = (ivFont*)wkit->font();  ivResource::ref(big_menu_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("big_submenu");
  wkit->push_style(sty);
  big_submenu_font = (ivFont*)wkit->font();  ivResource::ref(big_submenu_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("big_menubar");
  wkit->push_style(sty);
  big_menubar_font = (ivFont*)wkit->font();  ivResource::ref(big_menubar_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("big_italic_menubar");
  wkit->push_style(sty);
  big_italic_menubar_font = (ivFont*)wkit->font();  ivResource::ref(big_italic_menubar_font);
  wkit->pop_style();

  sty = new ivStyle(style);  sty->alias("TaIVButton");
  wkit->push_style(sty);
  small_button_width=46.0;
  wkit->style()->find_attribute("SmallWidth", small_button_width);
  medium_button_width=72.0;
  wkit->style()->find_attribute("MediumWidth", medium_button_width);
  big_button_width=100.0;
  wkit->style()->find_attribute("BigWidth", big_button_width);
  wkit->pop_style();

  title_style = new ivStyle(style);
  ivResource::ref(title_style);
  title_style->alias("title");

  apply_button_style = new ivStyle(style);
  ivResource::ref(apply_button_style);
  apply_button_style->alias("apply_button");

  ivBitmap* waiter = new ivBitmap(wait_cursor_bits, wait_cursor_width, wait_cursor_height,
				  wait_cursor_x_hot, wait_cursor_y_hot);

  ivBitmap* waiter_m = new ivBitmap(wait_mask_bits, wait_mask_width, wait_mask_height,
				    wait_mask_x_hot, wait_mask_y_hot);
			      
  wait_cursor = new ivCursor(waiter, waiter_m);
  ivBitmap* recorder = new ivBitmap(record_cursor_bits, record_cursor_width, record_cursor_height,
				  record_cursor_x_hot, record_cursor_y_hot);

  ivBitmap* recorder_m = new ivBitmap(record_mask_bits, record_mask_width, record_mask_height,
				    record_mask_x_hot, record_mask_y_hot);
			      
  record_cursor = new ivCursor(recorder, recorder_m);

  icon_bitmap = NULL;
}

taivMisc::~taivMisc() {
  if(wkit == NULL)
    return;
  ivResource::unref(style);
  ivResource::unref(vsep);
  ivResource::unref(hsep);
  ivResource::unref(vspc);
  ivResource::unref(hspc);
  ivResource::unref(vfsep);
  ivResource::unref(hfsep);
  ivResource::unref(vfspc);
  ivResource::unref(hfspc);
  ivResource::unref(font_foreground);
  ivResource::unref(name_font);
  ivResource::unref(small_menu_font);
  ivResource::unref(small_submenu_font);
  ivResource::unref(big_menu_font);
  ivResource::unref(big_submenu_font);
  ivResource::unref(big_menubar_font);
  ivResource::unref(big_italic_menubar_font);
  ivResource::unref(name_style);
  ivResource::unref(title_style);
  ivResource::unref(apply_button_style);
  ivResource::unref(icon_bitmap);
  delete wait_cursor;
  delete record_cursor;
}


ivGlyph* taivMisc::small_button(ivGlyph* b){
  return(layout->hfixed(b, small_button_width));
}

ivGlyph* taivMisc::medium_button(ivGlyph* b){
  return(layout->hfixed(b, medium_button_width));
}

ivGlyph* taivMisc::big_button(ivGlyph* b){
  return(layout->hfixed(b, big_button_width));
}


ivGlyph* taivMisc::small_flex_button(ivGlyph* b){
  return(layout->hflexible(layout->hnatural(b,small_button_width)));
}

ivGlyph* taivMisc::medium_flex_button(ivGlyph* b){
  return(layout->hflexible(layout->hnatural(b,medium_button_width)));
}

ivGlyph* taivMisc::big_flex_button(ivGlyph* b){
  return(layout->hflexible(layout->hnatural(b,big_button_width)));
}

String taivMisc::color_to_string(const ivColor* clr) {
  ivColorIntensity r, g, b;
  clr->intensities(r, g, b);
  int ri, gi, bi;
  ri = (int)(r * 255.0); gi = (int)(g * 255.0); bi = (int)(b * 255.0); 
  String clstr = "#" + String(ri, "%02x") + String(gi, "%02x") + String(bi, "%02x");
  return clstr;
}

ivPolyGlyph* taivMisc::top_sep(ivGlyph* g)	{ return layout->vbox(g, vsep); }
ivPolyGlyph* taivMisc::bot_sep(ivGlyph* g)	{ return layout->vbox(vsep, g); }
ivPolyGlyph* taivMisc::mid_sep(ivGlyph* g)	{ return layout->vbox(vsep, g, vsep); }
ivPolyGlyph* taivMisc::lft_sep(ivGlyph* g)	{ return layout->hbox(g, hsep); }
ivPolyGlyph* taivMisc::rgt_sep(ivGlyph* g)	{ return layout->hbox(hsep, g); }
ivPolyGlyph* taivMisc::cen_sep(ivGlyph* g)	{ return layout->hbox(hsep, g, hsep); }

ivPolyGlyph* taivMisc::top_spc(ivGlyph* g)	{ return layout->vbox(g, vspc); }
ivPolyGlyph* taivMisc::bot_spc(ivGlyph* g)	{ return layout->vbox(vspc, g); }
ivPolyGlyph* taivMisc::mid_spc(ivGlyph* g)	{ return layout->vbox(vspc, g, vspc); }
ivPolyGlyph* taivMisc::lft_spc(ivGlyph* g)	{ return layout->hbox(g, hspc); }
ivPolyGlyph* taivMisc::rgt_spc(ivGlyph* g)	{ return layout->hbox(hspc, g); }
ivPolyGlyph* taivMisc::cen_spc(ivGlyph* g)	{ return layout->hbox(hspc, g, hspc); }

ivPolyGlyph* taivMisc::wrap(ivGlyph* g)	{ return layout->vbox
					  (layout->vglue(),
					   layout->hbox
					   (layout->hglue(),g,layout->hglue()),
					   layout->vglue()); }

////////////////////////////////////////////////////////////////////////////
// Dump JPEG, TIFF
////////////////////////////////////////////////////////////////////////////

#ifndef CYGWIN

extern "C" {
#include <jpeglib.h>
// using the IV version of lib tiff!
#include <TIFF/tiffio.h>
}

#define lowbit(x) ((x) & (~(x) + 1))

void taivMisc::DumpJpeg(XDisplay* dpy, XWindow win, const char* fnm, int quality, int xstart, int ystart, int width, int height) {
  XWindowAttributes win_info;
  if(!XGetWindowAttributes(dpy, win, &win_info)) {
    taMisc::Error("DumpJPEG: XGetWindowAttributes failed!");
    return;
  }

  if(!((win_info.visual->c_class == TrueColor) || (win_info.visual->c_class == DirectColor))) {
    taMisc::Error("DumpJPEG: only works for TrueColor or DirectColor displays!");
    return;
  }

  if(width < 0) width = win_info.width - xstart;
  if(height < 0) height = win_info.height - ystart;

  XImage* image = XGetImage(dpy, win, xstart, ystart, width, height, AllPlanes, ZPixmap);
  if(image == NULL) {
    taMisc::Error("*** DumpJPEG: XGetImage failed!");
    return;
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  FILE* outfile;
  if ((outfile = fopen(fnm, "wb")) == NULL) {
    taMisc::Error("DumpJPEG: Can't open file:", fnm);
    return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = image->width; 	/* image width and height, in pixels */
  cinfo.image_height = image->height;
  cinfo.input_components = 3;	/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, false);

  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer[1];	/* pointer to a single row */
  int row_stride = image->width * 3;	/* JSAMPLEs per row in image_buffer */;

  JSAMPLE* scanline = new JSAMPLE[row_stride];
  row_pointer[0] = scanline;

  ulong maxval = 1 << image->depth;

  ulong red1 = lowbit(image->red_mask);
  ulong green1 = lowbit(image->green_mask);
  ulong blue1 = lowbit(image->blue_mask);

  // r = high, g = med, blue = low bits; compute # of bytes as follows:
  ulong bluesz = green1;
  ulong greensz = red1 / green1;
  ulong redsz = maxval / red1;

  ulong bluemult = 256 / bluesz; // 256 is scale expected by JPEG
  ulong greenmult = 256 / greensz;
  ulong redmult = 256 / redsz;

  while (cinfo.next_scanline < cinfo.image_height) {
    int scpos = 0;
    for(int x=0; x<image->width;x++) {
      ulong pixel = XGetPixel(image, x, cinfo.next_scanline);
      ulong rm = pixel & image->red_mask;
      ulong gm = pixel & image->green_mask;
      ulong bm = pixel & image->blue_mask;

      rm /= red1; gm /= green1; bm /= blue1; // get rid of mask offsets
      rm *= redmult; gm *= greenmult; bm *= bluemult;// upscale from 5-6-5 bits to 8 bits

      scanline[scpos] = (JSAMPLE)rm;
      scanline[scpos+1] = (JSAMPLE)gm;
      scanline[scpos+2] = (JSAMPLE)bm;
      scpos+=3;
    }
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  delete [] scanline;

  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);

  XDestroyImage(image);
}

void taivMisc::DumpJpegIv(ivWindow* win, const char* fnm, int quality, int xstart, int ystart, int width, int height) {
  ivDisplay* dsp = ivSession::instance()->default_display();
  ivDisplayRep* d = dsp->rep();
  ivWindowRep* rep = win->rep();
  DumpJpeg(d->display_, rep->xwindow_, fnm, quality, xstart, ystart, width, height);
}

void taivMisc::DumpTiff(XDisplay* dpy, XWindow win, const char* fnm, int xstart, int ystart, int width, int height) {
  XWindowAttributes win_info;
  if(!XGetWindowAttributes(dpy, win, &win_info)) {
    taMisc::Error("DumpTIFF: XGetWindowAttributes failed!");
    return;
  }

  if(!((win_info.visual->c_class == TrueColor) || (win_info.visual->c_class == DirectColor))) {
    taMisc::Error("DumpTIFF: only works for TrueColor or DirectColor displays!");
    return;
  }

  if(width < 0) width = win_info.width - xstart;
  if(height < 0) height = win_info.height - ystart;

  XImage* image = XGetImage(dpy, win, xstart, ystart, width, height, AllPlanes, ZPixmap);
  if(image == NULL) {
    taMisc::Error("*** DumpTIFF: XGetImage failed!");
    return;
  }

  TIFF* tiff;
  if((tiff = TIFFOpen(fnm, "w")) == NULL) {
    taMisc::Error("DumpTIFF: Can't open file:", fnm);
    return;
  }

  TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, image->width);
  TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, image->height);
  TIFFSetField(tiff, TIFFTAG_XRESOLUTION, 100.0);
  TIFFSetField(tiff, TIFFTAG_YRESOLUTION, 100.0);
  TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
  TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, image->height);
  TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8,8,8);
  TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(tiff, TIFFTAG_PLANARCONFIG,  PLANARCONFIG_CONTIG);
  TIFFSetField(tiff, TIFFTAG_ARTIST, "TypeAccess/PDP++ Window Dump");
//   TIFFSetField(tiff, TIFFTAG_ ,  );
//   TIFFSetField(tiff, TIFFTAG_ ,  );
//   TIFFSetField(tiff, TIFFTAG_ ,  );

  int row_stride = image->width * 3;	/* JSAMPLEs per row in image_buffer */;
  JSAMPLE* scanline = new JSAMPLE[row_stride];

  ulong maxval = 1 << image->depth;
  ulong red1 = lowbit(image->red_mask);
  ulong green1 = lowbit(image->green_mask);
  ulong blue1 = lowbit(image->blue_mask);

  // r = high, g = med, blue = low bits; compute # of bytes as follows:
  ulong bluesz = green1;
  ulong greensz = red1 / green1;
  ulong redsz = maxval / red1;

  ulong bluemult = 256 / bluesz; // 256 is scale expected by JPEG
  ulong greenmult = 256 / greensz;
  ulong redmult = 256 / redsz;

  for(int y=0;y<image->height;y++) {
    int scpos = 0;
    for(int x=0; x<image->width;x++) {
      ulong pixel = XGetPixel(image, x, y);
      ulong rm = pixel & image->red_mask;
      ulong gm = pixel & image->green_mask;
      ulong bm = pixel & image->blue_mask;

      rm /= red1; gm /= green1; bm /= blue1; // get rid of mask offsets
      rm *= redmult; gm *= greenmult; bm *= bluemult;// upscale from 5-6-5 bits to 8 bits

      scanline[scpos] = (JSAMPLE)rm;
      scanline[scpos+1] = (JSAMPLE)gm;
      scanline[scpos+2] = (JSAMPLE)bm;
      scpos+=3;
    }

    TIFFWriteScanline(tiff, scanline, y, 0);
  }

  delete [] scanline;

  TIFFClose(tiff);

  XDestroyImage(image);
}

void taivMisc::DumpTiffIv(ivWindow* win, const char* fnm, int xstart, int ystart, int width, int height) {
  ivDisplay* dsp = ivSession::instance()->default_display();
  ivDisplayRep* d = dsp->rep();
  ivWindowRep* rep = win->rep();
  DumpTiff(d->display_, rep->xwindow_, fnm, xstart, ystart, width, height);
}

#endif // CYGWIN


//////////////////////////
// 	IconGlyph 	//
//////////////////////////

IconGlyph::IconGlyph(ivGlyph* g,ivManagedWindow* w,void* o)
: ivMonoGlyph(g) {
  window = w;
  obj = o;
  last_draw_allocation.x_allotment().span(-1); // bogus initial
  first_draw = true;
  ScriptIconify = NULL;
  SetIconify = NULL;
} 

void IconGlyph::SetWindow(ivManagedWindow* w){
  window = w;
  if(taivM->icon_bitmap != NULL)  window->icon_bitmap(taivM->icon_bitmap);
}  


// for some reason, when you load an object that is iconified),
// undraw gets called twice, and then draw gets called once.
// first_draw ignores this first drawing.

// CYGWIN: these are not supported because they wreak havoc with
// everything -- the user needs to use the Iconify and DeIconify calls
// explicitly instead of relying on the window manager

void IconGlyph::draw(ivCanvas* c, const ivAllocation& a) const {
  ivMonoGlyph::draw(c,a);
#ifndef CYGWIN
  if(window == NULL) return;
  taivMisc::SetWinCursor(window);

  if(taMisc::is_loading) return;
  if(first_draw == true) { 
    ((IconGlyph*)this)->first_draw = false;
    return;
  }
  if(SetIconify == NULL) return;
  if((*SetIconify)(obj,-1) != false) {
    (*SetIconify)(obj,0);
    if(!(last_draw_allocation.equals(a,0.01))) {
      if(ScriptIconify != NULL) (*ScriptIconify)(obj,0);
      ((IconGlyph *) this)->last_draw_allocation = a;
    }
  }
#endif
}

void IconGlyph::undraw(){
  ivMonoGlyph::undraw();
#ifndef CYGWIN
  if(taMisc::is_loading) return;
  if(SetIconify == NULL) return;
  if((*SetIconify)(obj,-1) != true) {
    (*SetIconify)(obj,1);
    ivAllocation bogus_allocation;
    if(!(last_draw_allocation.equals(bogus_allocation,0.01))) {
      if(ScriptIconify != NULL) (*ScriptIconify)(obj,1);
      last_draw_allocation = bogus_allocation;
    }
  }
#endif
}

//////////////////////////
// 	HighlightBG	//
//////////////////////////

HighlightBG::HighlightBG(ivGlyph* body, const ivColor* bg, const ivColor* hi)
  : ivMonoGlyph(body) {
  bg_color = bg;
  hi_color = hi;
  ivResource::ref(bg_color);
  ivResource::ref(hi_color);
}

HighlightBG::~HighlightBG() {
  ivResource::unref(bg_color);
  ivResource::unref(hi_color);
}

void HighlightBG::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  ivMonoGlyph::allocate(c, a, ext);
  ext.merge(c, a);
}

void HighlightBG::draw(ivCanvas* c, const ivAllocation& a) const {
  ivExtension ext;
  ext.set(c, a);
  if (c->damaged(ext)) {
    if(highlight)
      c->fill_rect(a.left(), a.bottom(), a.right(), a.top(), hi_color);
    else 
      c->fill_rect(a.left(), a.bottom(), a.right(), a.top(), bg_color);
  }
  ivMonoGlyph::draw(c, a);
}

void HighlightBG::print(ivPrinter* p, const ivAllocation& a) const {
  if(highlight)
    p->fill_rect(a.left(), a.bottom(), a.right(), a.top(), hi_color);
  else
    p->fill_rect(a.left(), a.bottom(), a.right(), a.top(), bg_color);
  ivMonoGlyph::print(p, a);
}


//////////////////////////////////////////////////////////
// 	taivData: glyphs to represent kinds of data	//
//////////////////////////////////////////////////////////

taivData::taivData(taivDialog* dlg, taivData* par) {
  dialog = dlg;
  parent = par;
  hi_bg = NULL;
}

taivData::~taivData() {
  parent = NULL; dialog = NULL;
  if(hi_bg != NULL)
    ivResource::unref(hi_bg);
  hi_bg = NULL;
}

void taivData::CreateHiBG() {
  if(hi_bg != NULL)
    ivResource::unref(hi_bg);
  if(dialog != NULL) {
    hi_bg = new HighlightBG(NULL, dialog->bg_color->brightness(taivM->edit_darkbg_brightness),
			    taivM->edit_lightbg);
  }
  else
    hi_bg = new HighlightBG(NULL, taivM->edit_darkbg, taivM->edit_lightbg);
  ivResource::ref(hi_bg);
}

ivGlyph* taivData::HiBGLook(ivGlyph* body) {
  if(hi_bg == NULL) return body;
  hi_bg->body(taivM->layout->hbox(taivM->hfspc, body, taivM->hfspc));
  return hi_bg;
}

void taivData::DataChanged(taivData*) {
  // don't do anything ourselves, but notify dialog and our parent..
  if(dialog != NULL)
    dialog->Changed();
  if(parent != NULL)
    parent->DataChanged(this);
}

class ScrollFieldEditor : public ivFieldEditor {
public:
  taivData* data;
  int	drawn;
  ScrollFieldEditor(taivData* d,const char* strval,ivWidgetKit* w, ivStyle* st);
  void undraw();
  void allocate(ivCanvas* c, const ivAllocation& a,ivExtension& ext);
  void keystroke(const ivEvent& e);
  ivInputHandler* focus_in();
};

ScrollFieldEditor::ScrollFieldEditor(taivData* d,const char* strval,ivWidgetKit* w, ivStyle* st)
: ivFieldEditor(strval,w,st,NULL){
  data = d;
  drawn = false;
}

void ScrollFieldEditor::allocate(ivCanvas* c, const ivAllocation& a,
				 ivExtension& ext) {
  drawn = true;
  ivFieldEditor::allocate(c,a,ext);
}

void ScrollFieldEditor::undraw(){
  drawn = false;
  ivFieldEditor::undraw();
}

ivInputHandler* ScrollFieldEditor::focus_in() {
  ivInputHandler* result = ivFieldEditor::focus_in();
  if(data != NULL) {
    if(!drawn && (data->dialog != NULL) && (data->dialog->state == taivDialog::ACTIVE)) {
      data->dialog->Scroll();
    }
  }
  //  cerr << "focused" << endl << flush;
  return result;
}

void ScrollFieldEditor::keystroke(const ivEvent& e) {
  if((data == NULL) || (data->dialog == NULL)) {
    ivFieldEditor::keystroke(e);
    return;
  }
  if(data->dialog->dialog->special_key(e))
    return;
  String curval = text()->string();
  ivFieldEditor::keystroke(e);
  if((data != NULL) && (curval != text()->string())) {
    data->DataChanged();
  }
}

taivField::taivField(const char* strval, taivDialog* dlg, ivInputHandler* ih, taivData* par)
 : taivData(dlg, par) {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  rep = new ScrollFieldEditor(this,strval,wkit,taivM->style);
  ivResource::ref(rep);
  rep->select(0);
  input_handler = NULL;
  SetHandler(ih);
}
taivField::~taivField() {
  RemoveFromHandler();
  ivResource::unref(rep);
}

void taivField::SetHandler(ivInputHandler* ih){
  RemoveFromHandler();
  input_handler = ih;
  ivResource::ref(input_handler);
  if(input_handler != NULL) {
    input_handler->append_input_handler(rep);
    input_handler->focus(rep);
  }
}

void taivField::RemoveFromHandler(){
  if(input_handler == NULL) return;
  int i;
  for(i=(int)input_handler->input_handler_count() -1 ;i >= 0;i--){
    if(input_handler->input_handler(i) == rep)
      input_handler->remove_input_handler(i);
  }
  ivResource::unref(input_handler);
}
  
ivGlyph* taivField::GetRep() {
  return rep;
}
ivGlyph* taivField::GetLook() {
  return HiBGLook(taivM->layout->hflexible(taivM->layout->hnatural(rep, 40), fil, 0));
}

const char* taivField::GetValue() {
  return rep->text()->string();
}
void taivField::GetImage(const char* val) {
  rep->field(val); rep->select(0);
}

//////////////////////////////////
//	taivIncrField		//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivIncrField)
implementActionCallback(taivIncrField)
#include <ta/leave_iv.h>

taivIncrField::taivIncrField(const char* strval, taivDialog* dlg, ivInputHandler* ih,
			     taivData* par, int positive)
: taivField(strval,dlg,ih,par) {
  pos = positive;

  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivTelltaleState* upstate = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			ivTelltaleState::is_choosable);
  ivTelltaleState* downstate = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);

  upbutton= new DAStepper
    (wkit->up_mover_look(upstate),
     wkit->style(),upstate,
     new ActionCallback
     (taivIncrField)(this,&taivIncrField::Incr));

  downbutton = new DAStepper
    (wkit->down_mover_look(downstate),
     wkit->style(),downstate,
     new ActionCallback
     (taivIncrField)(this,&taivIncrField::Decr));

  ivResource::ref(upbutton);
  ivResource::ref(downbutton);

  incr_rep = (layout->hbox
	 (layout->vcenter(layout->hflexible(layout->hnatural(rep,30),fil,0),0),
	  layout->vcenter(layout->fixed(upbutton,18,16),0),
	  layout->vcenter(layout->fixed(downbutton,18,16),0)));
  ivResource::ref(incr_rep);
};

taivIncrField::~taivIncrField() {
  ivResource::unref(upbutton);
  ivResource::unref(downbutton);
  ivResource::unref(incr_rep);
}

ivGlyph* taivIncrField::GetRep() {
  return rep;
}

ivGlyph* taivIncrField::GetLook() {
  return HiBGLook(incr_rep);
}

void taivIncrField::Incr(){
  int val = atoi(GetValue());
  val++;
  String  temp(val);
  GetImage(temp);
  DataChanged();
}

void taivIncrField::Decr(){
  int val = atoi(GetValue());
  val--;
  if((pos == true) && (val < 0) ) val = 0;
  String  temp(val);
  GetImage(temp);
  DataChanged();
}

//////////////////////////////////
//	taivToggle		//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivToggle)
implementActionCallback(taivToggle)
#include <ta/leave_iv.h>

taivToggle::taivToggle(taivDialog* dlg, taivData* par) : taivData(dlg,par) {
  rep = taivM->wkit->check_box((ivGlyph*)NULL, 
				new ActionCallback
				(taivToggle)(this,&taivToggle::Toggle_Callback));
  ivResource::ref(rep);
}

taivToggle::~taivToggle() {
  ivResource::unref(rep);
}

ivGlyph* taivToggle::GetRep() {
  return rep;
}
ivGlyph* taivToggle::GetLook() {
  return HiBGLook(taivM->lft_sep(rep));
}
int taivToggle::GetValue() {
  return rep->state()->test(ivTelltaleState::is_chosen);
}
void taivToggle::GetImage(int stat) {
  rep->state()->set(ivTelltaleState::is_chosen, stat);
}
void taivToggle::Toggle_Callback() {
  DataChanged();
}

//////////////////////////////////
//	taivLabel		//
//////////////////////////////////

taivLabel::taivLabel(taivDialog* dlg, taivData* par) : taivData(dlg, par) {
  rep = new ivPatch(taivM->wkit->label(" "));
  ivResource::ref(rep);
}
taivLabel::~taivLabel() {
  ivResource::unref(rep);
}
ivGlyph* taivLabel::GetLook() {
  return HiBGLook(taivM->layout->hbox(taivM->hfsep, rep, taivM->hsep));
}
void taivLabel::GetImage(const char* val) {
  ((ivPatch*)rep)->body(taivM->wkit->label(val));
  ((ivPatch*)rep)->redraw();
}

//////////////////////////////////
//	taivPlusToggle		//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivPlusToggle)
implementActionCallback(taivPlusToggle)
#include <ta/leave_iv.h>

taivPlusToggle::taivPlusToggle(taivData* dat, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  data = dat;
  data->parent = this;		// we are the parent!
  but_rep = taivM->wkit->check_box
    ((ivGlyph*)NULL, new ActionCallback
     (taivPlusToggle)(this,&taivPlusToggle::Toggle_Callback));
  but_rep->state()->set(ivTelltaleState::is_chosen, false);
  rep = taivM->layout->hbox
    (taivM->layout->vcenter(but_rep), taivM->hfsep,
     taivM->layout->vcenter(data->GetLook()));
  ivResource::ref(rep);
}

taivPlusToggle::~taivPlusToggle() {
  ivResource::unref(rep);   rep = NULL;
  if(data != NULL)
    delete data;
  data = NULL;
}

ivGlyph* taivPlusToggle::GetRep() {
  if(data->NeedsFocus())
    return data->GetRep();		// get rep called for focusing, do it.
  return rep;
}
ivGlyph* taivPlusToggle::GetLook() {
  return HiBGLook(rep);
}
int taivPlusToggle::GetValue() {
  return but_rep->state()->test(ivTelltaleState::is_chosen);
}
void taivPlusToggle::GetImage(int stat) {
  but_rep->state()->set(ivTelltaleState::is_chosen, stat);
}
void taivPlusToggle::Toggle_Callback() {
  if(dialog != NULL) dialog->Changed();
  // DataChanged();
}

void taivPlusToggle::DataChanged(taivData* chld) {
  but_rep->state()->set(ivTelltaleState::is_chosen, true);  
  taivData::DataChanged(chld);
}


//////////////////////////////////
// 	taivPolyData		//
//////////////////////////////////

taivPolyData::taivPolyData(TypeDef* tp, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  typ = tp;
  show = taMisc::show_iv;
}

taivPolyData::~taivPolyData() {
  data_el.Reset();
  ivResource::unref(rep);  rep = NULL;
}

ivGlyph* taivPolyData::GetLook() {
  return rep;			// no hibg look here!
}

bool taivPolyData::ShowMember(MemberDef* md) {
  if(md->HasOption("HIDDEN_INLINE"))
    return false;
  return md->ShowMember(taMisc::show_iv);
}

void taivPolyData::GetDataRep(ivInputHandler* ih, taivDialog* dlg) {
  rep = taivM->layout->hbox();
  ivResource::ref(rep);
  int i;
  for(i=0; i<typ->members.size; i++) {
    MemberDef* md = typ->members.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = md->iv->GetDataRep(ih, dlg, this);
    data_el.Add(mb_dat);
    String nm = md->GetLabel();
    rep->append(taivM->layout->center(taivM->wkit->label(nm), 0, 0));
    rep->append(taivM->hfsep);
    rep->append(taivM->layout->vcenter(mb_dat->GetLook(),0));
    if(i != (typ->members.size -1))
      rep->append(taivM->hfsep);
  }
//  rep = layout->hflexible(rep);
}

void taivPolyData::GetImage(void* base, ivWindow* win) {
  cur_base = base;
  int i, cnt = 0;
  for(i=0; i<typ->members.size; i++) {
    MemberDef* md = typ->members.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = data_el.FastEl(cnt++);
    md->iv->GetImage(mb_dat, base, win);
  }
}

void taivPolyData::GetValue(void* base) {
  ostream* rec_scrpt = taivMisc::record_script; // don't record script stuff now
  taivMisc::record_script = NULL;
  bool first_diff = true;
  int i, cnt = 0;
  for(i=0; i<typ->members.size; i++) {
    MemberDef* md = typ->members.FastEl(i);
    if(!ShowMember(md))
      continue;
    taivData* mb_dat = data_el.FastEl(cnt++);
    md->iv->GetMbrValue(mb_dat, base, first_diff);
  }
  if(typ->InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)base;
    rbase->UpdateAfterEdit();	// hook to update the contents after an edit..
  }
  taivMisc::record_script = rec_scrpt;
}

//////////////////////////////////
// 	taivDataDeck		//
//////////////////////////////////

taivDataDeck::taivDataDeck(taivDialog* dlg, taivData* par)
: taivData(dlg,par) {
  rep = NULL;
  deck = NULL;
  cur_data = 0;
}

taivDataDeck::~taivDataDeck() {
  data_el.Reset();
  ivResource::unref(rep);  rep = NULL;
}

ivGlyph* taivDataDeck::GetLook() {
  return rep;
}

void taivDataDeck::GetDataRep(ivInputHandler*, taivDialog*) {
  deck = new ivDeck(data_el.size);
  rep = new ivPatch(deck);
  ivResource::ref(rep);
  int i;
  for(i=0; i<data_el.size; i++) {
    taivData* dat = data_el.FastEl(i);
    deck->append(taivM->layout->vcenter(dat->GetLook()));
  }
}

void taivDataDeck::GetImage(int i) {
  cur_data = i;
  deck->flip_to(i);
  ((ivPatch*)rep)->redraw();
}

//////////////////////////
// 	taivMenu	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivMenu)
implementActionCallback(taivMenu)

declareActionCallback(taivMenuEl)
implementActionCallback(taivMenuEl)
#include <ta/leave_iv.h>

taivMenuAction::taivMenuAction() { };
taivMenuAction::~taivMenuAction() { };
void taivMenuAction::Select(taivMenuEl*) { };


taivMenuEl::taivMenuEl(taivMenu* own, ivMenuItem* rp, int it, const char* lbl,
			 void* usr, ivAction* act)	
{
  owner = own; rep = rp; itm_no = it; label = lbl; usr_data = usr;
  if(rep != NULL)
    ivResource::ref(rep);
  action = act;
  if(action != NULL)
    ivResource::ref(action);
  men_act = NULL;
}
taivMenuEl::taivMenuEl(taivMenu* own, ivMenuItem* rp, int it, const char* lbl,
			 void* usr, taivMenuAction* act)	
{
  owner = own; rep = rp; itm_no = it; label = lbl; usr_data = usr;
  if(rep != NULL)
    ivResource::ref(rep);
  action = NULL;
  men_act = act;
  if(men_act != NULL)
    ivResource::ref(men_act);
}

taivMenuEl::~taivMenuEl() {
  if(rep != NULL)
    ivResource::unref(rep);
  rep = NULL;
  if(action != NULL)
    ivResource::unref(action);
  action = NULL;
  if(men_act != NULL)
    ivResource::unref(men_act);
  men_act = NULL;
}

void taivMenuEl::Select() {
  owner->DataChanged();		// something was selected..
  if(action != NULL) {
    // if men_act or usr_data are set, set cur_sel cuz they are presumably needed!
    if((men_act != NULL) || (usr_data != NULL)) 
      owner->cur_sel = this;
    action->execute();		// don't set the cur_sel if executing
  }
  else if(men_act != NULL) {
    owner->cur_sel = this;
    owner->Update();		// this must go first?
    men_act->Select(this);	// but do it for selecting
  }
  else {
    owner->cur_sel = this;	// and otherwise
    owner->Update();
  }
}

void taivMenu::Constr() {
  if(rep_type == menubar) {
    bar = taivM->wkit->menubar();
    ivResource::ref(bar);
  }
  else {
    bar = NULL;
  }
  mnu = taivM->wkit->pulldown();
  ivResource::ref(mnu);
  mlab = NULL;
  grp = NULL;
  cur_item = 0;
  cur_sel = NULL;

  if((sel_type == radio_update) || (sel_type == radio)) {
    grp = new ivTelltaleGroup();
    ivResource::ref(grp);
  }
  ivGlyph* big_spc;
  switch(font_type) {
  case big:
    mlab_g = new ivPatch(new ivLabel("?", taivM->big_menubar_font, taivM->font_foreground));
    mlab = taivM->wkit->menubar_item(mlab_g);
    break;
  case big_italic:
    mlab_g = new ivPatch(new ivLabel("?", taivM->big_italic_menubar_font, taivM->font_foreground));
    mlab = taivM->wkit->menubar_item(mlab_g);
    break;
  case small:
    // make it a big space surrounding an item 
    // when its in small format (eg. dialog)
    big_spc = taivM->layout->hglue(24);
    mlab_g = new ivPatch(new ivLabel("?", taivM->small_menu_font, taivM->font_foreground));
    mlab = taivM->wkit->menubar_item(taivM->layout->hbox(mlab_g,big_spc));
    break;
  case skinny_small:
    mlab_g = new ivPatch(new ivLabel("?", taivM->small_menu_font, taivM->font_foreground));
    mlab = taivM->wkit->menubar_item(mlab_g);
    break;
  }
  ivResource::ref(mlab_g);
  ivResource::ref(mlab);
  mlab->menu(mnu);
  if(rep_type == menubar) {
    bar->append_item(mlab);
  }
}

taivMenu::taivMenu(int rt, int st, int ft, taivDialog* dlg, taivData* par) : taivData(dlg,par) {
  rep_type = (RepType)rt;
  sel_type = (SelType)st;
  font_type = (FonType)ft;
  Constr();
}
taivMenu::taivMenu(int rt, int st, int ft, const char* lbl, taivDialog* dlg, taivData* par)
: taivData(dlg,par) {
  rep_type = (RepType)rt;
  sel_type = (SelType)st;
  font_type = (FonType)ft;
  Constr();
  SetMLabel(lbl);
}

taivMenu::~taivMenu() {
  ResetMenu();
  ivResource::unref(grp);		grp = NULL;
  ivResource::unref(mlab_g);		mlab_g = NULL;
  ivResource::unref(mlab);              mlab = NULL;
  ivResource::unref(mnu);		mnu = NULL;
  if(rep_type == menubar)
    ivResource::unref(bar);
}
ivGlyph* taivMenu::GetRep() {
  ivGlyph* rval= mlab->body();
  if(rep_type == menubar)
    rval = bar;
  return rval;
}
ivGlyph* taivMenu::GetLook() {
  return HiBGLook(taivM->cen_sep(GetRep()));
}

void taivMenu::ResetMenu() {
  long i;
  for(i=items.size-1; i>=0; i--) {
    mnu->remove_item(i);
  }
  items.Reset();
  cur_sel = NULL;
}

void taivMenu::AddSep() {
  cur_item = (int)mnu->item_count();
  ivMenuItem* new_men = taivM->wkit->menu_item_separator();
  mnu->append_item(new_men);
  items.Add(new taivMenuEl(this,new_men,cur_item, "-", NULL,(ivAction*)NULL));
}

ivMenuItem* taivMenu::NewItem(const char* val, int st) {
  ivMenuItem* new_men;
  if(st == use_default)   st = sel_type;
  ivGlyph* lbl = NULL;
  if((font_type == big) || (font_type == big_italic)) {
    lbl = new ivLabel(val, taivM->big_menu_font, taivM->font_foreground);
  }
  else {
    lbl = new ivLabel(val, taivM->small_menu_font, taivM->font_foreground);
  }
  if((st == radio_update) || (st == radio))
    new_men = taivM->wkit->radio_menu_item(grp, taivM->lft_sep(lbl));
  else
    new_men = taivM->wkit->menu_item(taivM->lft_sep(lbl));
  return new_men;
}

taivMenuEl* taivMenu::AddItem(const char* val, void* usr, int st, ivAction* act) {
  // do not add items of same name
  int i;
  for(i=0; i<items.size; i++) {
    if(items.FastEl(i)->label == val) {
      return CurEl();
    }
  }
  cur_item = (int)mnu->item_count();
  ivMenuItem* new_men = NewItem(val, st);
  items.Add(new taivMenuEl(this, new_men, cur_item, val, usr, act));
  new_men->action(new ActionCallback(taivMenuEl)(CurEl(), &taivMenuEl::Select));
  mnu->append_item(new_men);
  return CurEl();
}

taivMenuEl* taivMenu::AddItem(const char* val, void* usr, int st, taivMenuAction* act) {
  // do not add items of same name
  int i;
  for(i=0; i<items.size; i++) {
    if(items.FastEl(i)->label == val) {
      return CurEl();
    }
  }
  cur_item = (int)mnu->item_count();
  ivMenuItem* new_men = NewItem(val, st);
  items.Add(new taivMenuEl(this, new_men, cur_item, val, usr, act));
  new_men->action(new ActionCallback(taivMenuEl)(CurEl(), &taivMenuEl::Select));
  mnu->append_item(new_men);
  return CurEl();
}
  
void taivMenu::SetMLabel(const char* val) {
  mlab_str = val;
  if(font_type == big) {
    mlab_g->body(new ivLabel(val, taivM->big_menubar_font, taivM->font_foreground));
  }
  else if(font_type == big_italic) {
    mlab_g->body(new ivLabel(val, taivM->big_italic_menubar_font, taivM->font_foreground));
  }
  else {
    mlab_g->body(new ivLabel(val, taivM->small_menu_font, taivM->font_foreground));
  }
  
  ((ivPatch*)mlab_g)->reallocate();
  ((ivPatch*)mlab_g)->redraw();
}

bool taivMenu::GetImage(int itm) {
  if(itm >= items.size)
    return false;
  mnu->item(itm)->state()->set(ivTelltaleState::is_chosen, 1);
  cur_sel = items.FastEl(itm);
  Update();
  return true;
}

bool taivMenu::GetImage(void* usr) {
  int first_usr_data = -1;
  int first_usr_data_cnt = 0;
  int i;
  for(i=0; i<items.size; i++) {
    taivMenuEl* itm = items.FastEl(i);
    if(itm->usr_data == usr) {
      if((usr == NULL) && (itm->label != "NULL"))
	continue;
      GetImage(i);
      return true;
    }
    // get first item with data and without any menu callbacks, as default if nothing else works
    else if((itm->usr_data != NULL) && (itm->action == NULL)) {
      first_usr_data_cnt++;
      if(first_usr_data == -1)
	first_usr_data = i;
    }
  }
  if((first_usr_data >= 0) && (first_usr_data_cnt == 1))
    GetImage(first_usr_data);
  return false;
}

void taivMenu::Update() {
  if((cur_sel != NULL) && ((sel_type == radio_update) || (sel_type == normal_update)))
    SetMLabel(cur_sel->label);
  else if(cur_sel == NULL) 
    SetMLabel("NULL");
  return;
}


//////////////////////////////////
// 	taivHierMenu		//
//////////////////////////////////

taivHierEl::taivHierEl(taivHierMenu* own, ivMenuItem* rp, int sb, int it,
			 const char* lbl, void* usr, ivAction* act)
: taivMenuEl((taivMenu*)own, rp, it, lbl, usr, act)
{ sub_no = sb; }

taivHierEl::taivHierEl(taivHierMenu* own, ivMenuItem* rp, int sb, int it,
			 const char* lbl, void* usr, taivMenuAction* act)
: taivMenuEl((taivMenu*)own, rp, it, lbl, usr, act)
{ sub_no = sb; }

taivHierSub::taivHierSub(ivMenu* sm) {
  sub_mnu = sm;
  ivResource::ref(sub_mnu);
}
taivHierSub::taivHierSub(ivMenu* sm, void* usr) {
  sub_mnu = sm;
  usr_data= usr;
  ivResource::ref(sub_mnu);
}
taivHierSub::~taivHierSub() {
  ivResource::unref(sub_mnu);	  sub_mnu = NULL;
}


taivHierMenu::taivHierMenu(int rt, int st, int ft, taivDialog* dlg, taivData* par)
: taivMenu(rt,st,ft,dlg,par)
{
  subs.Add(new taivHierSub(mnu)); cur_sub = mnu; cur_sno = 0;
}
taivHierMenu::taivHierMenu(int rt, int st, int ft, void* usr, taivDialog* dlg, taivData* par)
: taivMenu(rt,st,ft,dlg,par)
{
  subs.Add(new taivHierSub(mnu, usr)); cur_sub = mnu; cur_sno = 0;
}
taivHierMenu::~taivHierMenu() {
  ResetMenu();
  taivHierSub* sb = subs.SafeEl(0);
  if(sb != NULL) {
    sb->sub_items.Reset();
    subs.Remove(0);
  }
}

void taivHierMenu::SetSub(int ns) {
  cur_sno = ns;
  cur_sub = subs.FastEl(ns)->sub_mnu;
  cur_item = (int)cur_sub->count()-1;
}

ivMenuItem* taivHierMenu::GetSubMenu(int sub, int itm) {
  return subs.FastEl(sub)->sub_mnu->item(itm);
}

void taivHierMenu::ResetMenu() {
  int i, j;
  for(i=subs.size-1; i>=0; i--) {
    taivHierSub* sb = subs.FastEl(i);
    if(i==0) {                  // get rid items in 0'th menu (not removed!)
      for(j=sb->sub_items.size-1; j>=0; j--) {
	sb->sub_mnu->remove_item(j);
      }
    }
    sb->sub_items.Reset();
    if(i > 0)
      subs.Remove(i);
  }
  cur_sel = NULL;
}

int taivHierMenu::AddSubMenu(const char* val, void* usr) {
  cur_item = (int)cur_sub->item_count();
  ivMenuItem* new_men;
  ivGlyph* lbl;
  String subname = String(val) + "...";
  if((font_type == big) || (font_type == big_italic)) {
    lbl = new ivLabel(subname.chars(), taivM->big_submenu_font, taivM->font_foreground);
  }
  else {
    lbl = new ivLabel(subname.chars(), taivM->small_submenu_font, taivM->font_foreground);
  }
  new_men = taivM->wkit->menu_item
    (taivM->layout->hbox(taivM->hsep, lbl, taivM->hfsep));
  subs.FastEl(cur_sno)->sub_items.Add
    (new taivHierEl(this, new_men, cur_sno, cur_item, "->", NULL, (ivAction*)NULL));
  ivMenu* new_sub = taivM->wkit->pullright();
  subs.Add(new taivHierSub(new_sub, usr));
  new_men->menu(new_sub);
  cur_sub->append_item(new_men);

  cur_sub = new_sub;
  cur_sno = (int)subs.size - 1;
  return cur_sno;
}
void taivHierMenu::AddSep() {
  cur_item = (int)cur_sub->item_count();
  ivMenuItem* new_men = taivM->wkit->menu_item_separator();
  cur_sub->append_item(new_men);
  subs.FastEl(cur_sno)->sub_items.Add
    (new taivHierEl(this, new_men, cur_sno, cur_item, "-", NULL, (ivAction*)NULL));
}

taivMenuEl* taivHierMenu::AddItem(const char* val, void* usr, int st, ivAction* act) {
  cur_item = (int)cur_sub->item_count();
  ivMenuItem* new_men = NewItem(val, st);
  subs.FastEl(cur_sno)->sub_items.Add
    (new taivHierEl(this, new_men, cur_sno, cur_item, val, usr, act));
  new_men->action(new ActionCallback(taivMenuEl)(CurEl(), &taivMenuEl::Select));
  cur_sub->append_item(new_men);
  return CurEl();
}
taivMenuEl* taivHierMenu::AddItem(const char* val, void* usr, int st,
				    taivMenuAction* act)
{
  cur_item = (int)cur_sub->item_count();
  ivMenuItem* new_men = NewItem(val, st);
  subs.FastEl(cur_sno)->sub_items.Add
    (new taivHierEl(this, new_men, cur_sno, cur_item, val, usr, act));
  new_men->action(new ActionCallback(taivMenuEl)(CurEl(), &taivMenuEl::Select));
  cur_sub->append_item(new_men);
  return CurEl();
}
bool taivHierMenu::GetImage(int sub, int itm) {
  GetSubMenu(sub, itm)->state()->set(ivTelltaleState::is_chosen, 1);
  cur_sel = GetSubEl(sub,itm);
  Update();
  return true;
}

bool taivHierMenu::GetImage(void* usr) {
  int first_usr_data_s = -1;
  int first_usr_data_i = -1;
  int first_usr_data_cnt = 0;
  int i,j;
  for(j=0; j<subs.size; j++) {
    for(i=0; i<subs.FastEl(j)->sub_items.size; i++) {
      taivHierEl* itm = GetSubEl(j,i);
      if(itm->usr_data == usr) {
	if((usr == NULL) && (itm->label != "NULL"))
	  continue;
	GetImage(j,i);
	return true;
      }
      // get first item with data and without any menu callbacks, as default if nothing else works
      else if((itm->usr_data != NULL) && (itm->action == NULL)) {
	first_usr_data_cnt++;
	if(first_usr_data_s == -1) { 
	  first_usr_data_s = j;	
	  first_usr_data_i = i;
	}
      }
    }
  }
  if((first_usr_data_s >= 0) && (first_usr_data_cnt == 1)) { // only set if there is just one -- otherwise choose!
    GetImage(first_usr_data_s, first_usr_data_i);
  }
  return false;
}


//////////////////////////////////
// 	taivEditButton	 	//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivEditButton)
implementActionCallback(taivEditButton)
#include <ta/leave_iv.h>

taivEditButton::taivEditButton(TypeDef* tp, void* base, ivWindow* win,
				 taivEdit *taive, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  typ = tp;
  cur_base = base;
  cur_win = win;
  read_only = false;
  if(taive != NULL)
    ive = taive;
  else
    ive = NULL;			// if null, it uses type's ive
  rep = new taivMenu(taivMenu::menubar, taivMenu::normal_update, taivMenu::small, dlg, this);
  if(!typ->InheritsFrom(TA_taBase) || typ->HasOption("EDIT_ONLY"))
    rep->AddItem("Edit", NULL, taivMenu::use_default,
		 new ActionCallback(taivEditButton)(this, &taivEditButton::Edit));
  SetLabel();
}

void taivEditButton::SetLabel() {
  String lbl = typ->name;
  if(typ->HasOption("EDIT_ONLY"))
    lbl += ": Edit";
  else
    lbl += ": Actions";
  rep->SetMLabel(lbl);
}

taivEditButton::~taivEditButton(){
  if(ive != NULL)    delete ive;
  ive = NULL;
  delete rep;
  meth_el.Reset();
}
ivGlyph* taivEditButton::GetRep() {
  return rep->GetRep();
}
ivGlyph* taivEditButton::GetLook() {
  return rep->GetLook();
}

void taivEditButton::GetMethMenus() {
  if(meth_el.size > 0)		// only do this once..
    return;
  if(typ->HasOption("EDIT_ONLY")) return;
  String men_nm, lst_men_nm;
  int i;
  for(i=0; i<typ->methods.size; i++) {
    MethodDef* md = typ->methods.FastEl(i);
    if((md->iv == NULL) || (md->name == "Close"))
      continue;
    // don't do following unless 
    if((md->name == "CopyFrom") || (md->name == "CopyTo") || (md->name == "DuplicateMe")
       || (md->name == "ChangeMyType") || (md->name == "Help"))
      continue;
    if((read_only == true) && !(md->HasOption("EDIT_READ_ONLY"))) continue;
    String cur_nm = md->OptionAfter("MENU_ON_");
    if(cur_nm != "")
      men_nm = cur_nm;
    // has to be on one of these two menus..
    if((men_nm != "Object") && (men_nm != "Edit"))
      continue;
    if((men_nm != lst_men_nm) && (lst_men_nm != ""))
      rep->AddSep();
    lst_men_nm = men_nm;
    taivMethMenu* mth_rep = md->iv->GetMethodRep(cur_base, dialog);
    if(mth_rep == NULL)
      continue;
    meth_el.Add(mth_rep);
    if((ive != NULL) && (md->name == "Edit"))
      rep->AddItem("Edit", NULL, taivMenu::use_default,
		   new ActionCallback(taivEditButton)
		   (this, &taivEditButton::Edit));
    else
      mth_rep->AddToMenu(rep);
  }
}

void taivEditButton::GetImage(void* base, ivWindow* win) {
  cur_base = base;
  cur_win = win;
  SetLabel();
  GetMethMenus();
}

void taivEditButton::Edit() {
  if(cur_base == NULL)
    return;
  const ivColor* bgclr = NULL;
  if(typ->InheritsFrom(TA_taBase)) {
    bgclr = ((TAPtr)cur_base)->GetEditColorInherit();
  }
  if((bgclr == NULL) && (dialog != NULL)) bgclr = dialog->bg_color;
  bool wait = false;
  if(dialog != NULL) wait = dialog->waiting;
  if(ive == NULL) {
    typ->ive->Edit(cur_base, cur_win, wait, read_only, bgclr);
  }
  else {
    ive->typ = typ;
    ive->Edit(cur_base, cur_win, wait, read_only, bgclr);
  }
  GetImage(cur_base, cur_win);
}

#ifndef DONT_USE_ZLIB
// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : gzstream.h
// Revision      : $Revision: 1.5 $
// Revision_date : $Date: 2002/04/26 23:30:15 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#ifndef GZSTREAM_H
#define GZSTREAM_H 1

// standard C++ with new header file names and std:: namespace
#include <iostream>
#include <fstream>
#include <zlib.h>

#ifdef GZSTREAM_NAMESPACE
namespace GZSTREAM_NAMESPACE {
#endif

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See below for user classes.
// ----------------------------------------------------------------------------

class gzstreambuf : public std::streambuf {
private:
    static const int bufferSize = 47+256;    // size of data buff
    // totals 512 bytes under g++ for igzstream at the end.

    gzFile           file;               // file handle for compressed file
    char             buffer[bufferSize]; // data buffer
    char             opened;             // open/close state of stream
    int              mode;               // I/O mode

    int flush_buffer();
public:
    gzstreambuf() : opened(0) {
        setp( buffer, buffer + (bufferSize-1));
        setg( buffer + 4,     // beginning of putback area
              buffer + 4,     // read position
              buffer + 4);    // end position      
        // ASSERT: both input & output capabilities will not be used together
    }
    int is_open() { return opened; }
    gzstreambuf* open( const char* name, int open_mode);
    gzstreambuf* close();
    ~gzstreambuf() { close(); }
    
    virtual int     overflow( int c = EOF);
    virtual int     underflow();
    virtual int     sync();
};

class gzstreambase : virtual public std::ios {
protected:
    gzstreambuf buf;
public:
    gzstreambase() { init(&buf); }
    gzstreambase( const char* name, int open_mode);
    ~gzstreambase();
    void open( const char* name, int open_mode);
    void close();
    gzstreambuf* rdbuf() { return &buf; }
};

// ----------------------------------------------------------------------------
// User classes. Use igzstream and ogzstream analogously to ifstream and
// ofstream respectively. They read and write files based on the gz* 
// function interface of the zlib. Files are compatible with gzip compression.
// ----------------------------------------------------------------------------

class igzstream : public gzstreambase, public std::istream {
public:
    igzstream() : std::istream( &buf) {} 
    igzstream( const char* name, int open_mode = std::ios::in)
        : gzstreambase( name, open_mode), std::istream( &buf) {}  
    gzstreambuf* rdbuf() { return gzstreambase::rdbuf(); }
    void open( const char* name, int open_mode = std::ios::in) {
        gzstreambase::open( name, open_mode);
    }
};

class ogzstream : public gzstreambase, public std::ostream {
public:
    ogzstream() : std::ostream( &buf) {}
    ogzstream( const char* name, int mode = std::ios::out)
        : gzstreambase( name, mode), std::ostream( &buf) {}  
    gzstreambuf* rdbuf() { return gzstreambase::rdbuf(); }
    void open( const char* name, int open_mode = std::ios::out) {
        gzstreambase::open( name, open_mode);
    }
};

#ifdef GZSTREAM_NAMESPACE
} // namespace GZSTREAM_NAMESPACE
#endif

#endif // GZSTREAM_H
// ============================================================================
// EOF //

#include <string.h>  // for memcpy

#ifdef GZSTREAM_NAMESPACE
namespace GZSTREAM_NAMESPACE {
#endif

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See header file for user classes.
// ----------------------------------------------------------------------------

// --------------------------------------
// class gzstreambuf:
// --------------------------------------

gzstreambuf* gzstreambuf::open( const char* name, int open_mode) {
    if ( is_open())
        return (gzstreambuf*)0;
    mode = open_mode;
    // no append nor read/write mode
    if ((mode & std::ios::ate) || (mode & std::ios::app)
        || ((mode & std::ios::in) && (mode & std::ios::out)))
        return (gzstreambuf*)0;
    char  fmode[10];
    char* fmodeptr = fmode;
    if ( mode & std::ios::in)
        *fmodeptr++ = 'r';
    else if ( mode & std::ios::out)
        *fmodeptr++ = 'w';
    *fmodeptr++ = 'b';
    *fmodeptr = '\0';
    file = gzopen( name, fmode);
    if (file == 0)
        return (gzstreambuf*)0;
    opened = 1;
    return this;
}

gzstreambuf * gzstreambuf::close() {
    if ( is_open()) {
        sync();
        opened = 0;
        if ( gzclose( file) == Z_OK)
            return this;
    }
    return (gzstreambuf*)0;
}

int gzstreambuf::underflow() { // used for input buffer only
    if ( gptr() && ( gptr() < egptr()))
        return * reinterpret_cast<unsigned char *>( gptr());

    if ( ! (mode & std::ios::in) || ! opened)
        return EOF;
    // Josuttis' implementation of inbuf
    int n_putback = gptr() - eback();
    if ( n_putback > 4)
        n_putback = 4;
    memcpy( buffer + (4 - n_putback), gptr() - n_putback, n_putback);

    int num = gzread( file, buffer+4, bufferSize-4);
    if (num <= 0) // ERROR or EOF
        return EOF;

    // reset buffer pointers
    setg( buffer + (4 - n_putback),   // beginning of putback area
          buffer + 4,                 // read position
          buffer + 4 + num);          // end of buffer

    // return next character
    return * reinterpret_cast<unsigned char *>( gptr());    
}

int gzstreambuf::flush_buffer() {
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    int w = pptr() - pbase();
    if ( gzwrite( file, pbase(), w) != w)
        return EOF;
    pbump( -w);
    return w;
}

int gzstreambuf::overflow( int c) { // used for output buffer only
    if ( ! ( mode & std::ios::out) || ! opened)
        return EOF;
    if (c != EOF) {
        *pptr() = c;
        pbump(1);
    }
    if ( flush_buffer() == EOF)
        return EOF;
    return c;
}

int gzstreambuf::sync() {
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ( pptr() && pptr() > pbase()) {
        if ( flush_buffer() == EOF)
            return -1;
    }
    return 0;
}

// --------------------------------------
// class gzstreambase:
// --------------------------------------

gzstreambase::gzstreambase( const char* name, int mode) {
    init( &buf);
    open( name, mode);
}

gzstreambase::~gzstreambase() {
    buf.close();
}

void gzstreambase::open( const char* name, int open_mode) {
    if ( ! buf.open( name, open_mode))
        clear( rdstate() | std::ios::badbit);
}

void gzstreambase::close() {
    if ( buf.is_open())
        if ( ! buf.close())
            clear( rdstate() | std::ios::badbit);
}

#ifdef GZSTREAM_NAMESPACE
} // namespace GZSTREAM_NAMESPACE
#endif

// ============================================================================
// EOF //
#endif

//////////////////////////////////
// 	taivGetfile		//
//////////////////////////////////

#include <sys/stat.h>
#include <sys/wait.h>

int taivGetFile::buf_size = 1016;
String taivGetFile::last_fname;
String taivGetFile::last_dir;

taivGetFile::taivGetFile() {
  win = NULL;
  dlg = NULL;
  chooser = NULL;
  ifstrm = NULL;
  iofbuf = NULL;
  compress_pid = 0;
  ofstrm = NULL;
  fstrm = NULL;
  istrm = NULL;
  ostrm = NULL;
  compress = false;
  open_file = false;
  select_only = false;
  file_selected = false;
}

taivGetFile::taivGetFile(const char* dr, const char* fltr, ivWindow* wn, ivStyle*, bool comp) {
  chooser = NULL;
  ifstrm = NULL;
  iofbuf = NULL;
  compress_pid = 0;
  ofstrm = NULL;
  fstrm = NULL;
  istrm = NULL;
  ostrm = NULL;
  open_file = false;
  compress = comp;
  select_only = false;
  file_selected = false;
  dir = dr;
  if(dir == "")
    dir = ".";
  filter = fltr;
  win = wn;
  dlg = NULL;
}

taivGetFile::~taivGetFile() {
  Close();
  if(chooser != NULL) {
    ivResource::unref(chooser);
    ivResource::flush();
    chooser = NULL;
  }
}

void taivGetFile::FixFileName() {
  String ext;
  if(filter != "") {
    ext = filter.from('.');
    if(ext.contains('*'))
      ext = ext.before('*');
  }
  if(!compress) {
    bool has_ext = (fname.from((int)(fname.length() - ext.length())) == ext);
    if(!has_ext)
      fname += ext;
  }
  else {
    String sfx = taMisc::compress_sfx;
    bool has_sfx = (fname.from((int)(fname.length() - sfx.length())) == sfx);
    if(!has_sfx) {
      bool has_ext = (fname.from((int)(fname.length() - ext.length())) == ext);
      if(!has_ext)
	fname += ext;
      fname += sfx;
    }
    else {
      String raw_fn = fname.before(sfx,-1);
      bool has_ext = (raw_fn.from((int)(raw_fn.length() - ext.length())) == ext);
      if(!has_ext)
	fname = raw_fn + ext + sfx;
    }
  }
}

void taivGetFile::GetDir() {
  dir = fname.before('/', -1);
  if(!dir.empty() && (dir != "."))
    taMisc::include_paths.AddUnique(dir); // make sure its on the include path..
}

// this is for doing our own procbuf stuff!
// code suggested by scp@predict.com (Steve Pope)
// note that this has now been replaced by gzstream & zlib unless DONT_USE_ZLIB is defined!
#ifdef DONT_USE_ZLIB
istream* taivGetFile::UnCompressInput(const char* filename) {
  int p[2];
  if (pipe(p) < 0) {
    taMisc::Error("Could not open pipe for UnCompressInput of:",filename);
    return NULL;
  }

  cout << flush;	// make sure that we don't copy buffered data

  compress_pid = fork();	// spawn a child
  if(compress_pid > 0) {	// we're the parent
    close(p[1]);	// close the side for writing, will become stdout of the child
    iofbuf = new char[buf_size];
    ifstrm = new ifstream(p[0], iofbuf, buf_size-8); // input from uncompression 
    return ifstrm;
  }
  else if (compress_pid == 0) {
    close(p[0]);	// we're the child, so close the read end of the pipe

    dup2(p[1], 1);	// get rid of our stdout and replace with the pipe

    int f = open(filename, O_RDONLY); // now open the file we want, and make it stdin
    if (f >= 0) {
      if (dup2(f,0) < 0) {
	cerr << "could not dup2(f,0)\n";
	exit(1);		    // error
      }
     String execmd = taMisc::uncompress_cmd.before(' ');
     String exeargs = taMisc::uncompress_cmd.after(' '); // note only does 1 set of args
     execlp((const char*)execmd, (const char*)execmd, (const char*)exeargs, 0); // now invoke gunzip reading from stdin
//     String execmd = taMisc::uncompress_cmd + " " + filename;
//     execl("/bin/sh", "sh", "-c", (const char*)execmd, NULL);
    }
    cerr << "UnCompressInput: could not open file: " << filename << "\n";
    exit(1); // if we're here, something went wrong
  }

  taMisc::Error("Could not fork for UnCompressInput of:",filename);
  return NULL;
}
#else
istream* taivGetFile::UnCompressInput(const char*) {
  return NULL;
}
#endif

// this is for doing our own procbuf stuff!
// code suggested by scp@predict.com (Steve Pope)
// note that this has now been replaced by gzstream & zlib unless DONT_USE_ZLIB is defined!
#ifdef DONT_USE_ZLIB
ostream* taivGetFile::CompressOutput(const char* filename, bool append) {
  int p[2];
  if (pipe(p) < 0) {
    taMisc::Error("Could not open pipe for CompressOutput of:",filename,
		  "saving uncompressed");
    fstrm = new fstream(filename, ios::out);
    ostrm = (ostream*)fstrm;
    compress = false;		// no longer compressed
    return ostrm;
  }

  cout << flush;	// make sure that we don't copy buffered data

  compress_pid = fork();	// spawn a child
  if(compress_pid > 0) {	// we're the parent
    close(p[0]);	// close the side for reading, will become stdin of the child
    iofbuf = new char[buf_size];
    ofstrm = new ofstream(p[1], iofbuf, buf_size-8); // output to compression
    return ofstrm;
  }
  else if(compress_pid == 0) {
    close(p[1]);	// we're the child, so close the write end of the pipe

    dup2(p[0], 0);	// get rid of our stdin and replace with the pipe

    mode_t mymode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    int f;
    if(!append) {
      String rmcmd = String("/bin/rm -f ") + filename;
      system((const char*)rmcmd);
      f = open(filename, O_WRONLY|O_CREAT, mymode); // now open the file we want, and make it stdout
    }
    else
      f = open(filename, O_WRONLY|O_CREAT|O_APPEND, mymode);
    if (f >= 0) {
      if (dup2(f,1) < 0) {
	cerr << "could not dup2(f,1)\n";
	exit(1);		    // error
      }
      String execmd = taMisc::compress_cmd.before(' ');
      String exeargs = taMisc::compress_cmd.after(' '); // note only does 1 set of args
      execlp((const char*)execmd, (const char*)execmd, (const char*)exeargs, 0); // now invoke gunzip reading from stdin
    }

    cerr << "CompressOutput: could not open file: " << filename << "\n";
    exit(1); // if we're here, something went wrong
  }

  taMisc::Error("Could not fork for CompressOutput of:",filename,
		"saving uncompressed");
  fstrm = new fstream(filename, ios::out);
  ostrm = (ostream*)fstrm;
  compress = false;		// no longer compressed
  return ostrm;
}
#else
ostream* taivGetFile::CompressOutput(const char*, bool) {
  return NULL;
}
#endif

istream* taivGetFile::open_read() {
  Close();
  GetDir();
  int acc = access(fname, R_OK);
  if((acc != 0) && !select_only) {
    taMisc::Error("File:", fname, "could not be opened for reading");
    return NULL;
  }
  if((acc != 0) && select_only)		// fix the file name for new files..
    FixFileName();
  open_file = true;
  last_fname = fname;
  last_dir = dir;
  String sfx = taMisc::compress_sfx;
  if(fname.from((int)(fname.length() - sfx.length())) == sfx) {
    compress = true;		// set for compressed files..
#ifdef DONT_USE_ZLIB
    istrm = UnCompressInput(fname);
#else
    istrm = new igzstream(fname, ios::in);
#endif
    return istrm;
  }
  fstrm = new fstream(fname, ios::in);
  istrm = (istream*)fstrm;
  return istrm;
}

bool taivGetFile::open_write_exist_check() {
  Close();
  GetDir();
  int acc = access(fname, F_OK);
  if(acc == 0)
    return true;
  return false;
}

ostream* taivGetFile::open_write() {
  Close();
  GetDir();
  int acc = access(fname, W_OK);
  if((acc != 0) && (errno != ENOENT)) {
    perror("open_write: ");
    taMisc::Error("File:", fname, "could not be opened for writing");
    return NULL;
  }
  open_file = true;
  last_dir = dir;
  String sfx = taMisc::compress_sfx;
  bool hasfx = (fname.from((int)(fname.length() - sfx.length())) == sfx);
  if(compress || hasfx) {
    compress = true;		// set for compressed files..
    FixFileName();
    last_fname = fname;
#ifdef DONT_USE_ZLIB
    ostrm = CompressOutput(fname, false);
#else
    ostrm = new ogzstream(fname, ios::out);
#endif
    return ostrm;
  }
  else {
    FixFileName();
  }
  last_fname = fname;
  fstrm = new fstream(fname, ios::out);
  ostrm = (ostream*)fstrm;
  return ostrm;
}

ostream* taivGetFile::open_append() {
  Close();
  GetDir();
  int acc = access(fname, W_OK);
  if((acc != 0) && (errno != ENOENT)) {
    perror("open_append: ");
    taMisc::Error("File:", fname, "could not be opened for appending");
    return NULL;
  }
  open_file = true;
  last_dir = dir;
  String sfx = taMisc::compress_sfx;
  bool hasfx = (fname.from((int)(fname.length() - sfx.length())) == sfx);
  if(compress || hasfx) {
    compress = true;		// set for compressed files..
    if(!hasfx)
      fname += sfx;
    last_fname = fname;
#ifdef DONT_USE_ZLIB
    ostrm = CompressOutput(fname, true);
#else
    ostrm = new ogzstream(fname, ios::out | ios::app);
#endif
    return ostrm;
  }    
  last_fname = fname;
  fstrm = new fstream(fname, ios::out | ios::app);
  ostrm = (ostream*)fstrm;
  return ostrm;
}


istream* taivGetFile::Open(const char* nm, bool no_dlg) {
  file_selected = false;
  if(nm != NULL) {
    fname = nm;
    file_selected = true;
  }
  if(no_dlg || !cssivSession::in_session)
    return open_read();
  if(chooser == NULL) {
    chooser = taivM->dkit->file_chooser(dir, taivM->wkit->style());
    ivResource::ref(chooser);		
  }

  chooser->style()->attribute("filter", "on");
  chooser->style()->attribute("defaultSelection", fname);
  chooser->style()->attribute("filterPattern", filter);
  chooser->style()->attribute("open", "Open");
  chooser->style()->attribute("caption", "Select File to Open for Reading");
  String win_title = String("Open: ") + filter;
  chooser->style()->attribute("name", win_title);
  chooser->style()->attribute("title", win_title);
  chooser->style()->attribute("iconName", win_title);
  int rval;
  istream* rstrm = NULL;
  ivCoord x=0,y=0;
  taivDialog::GetPointerPos(x,y);
  rval = chooser->post_at_aligned(x, y, POINTER_ALIGN_X, POINTER_ALIGN_Y);
  if(rval) {
    file_selected = true;
    fname = chooser->selected()->string();
    if(open_read() != NULL) {
      if(!select_only && (istrm->bad() || (istrm->peek() == EOF)))
	taMisc::Error("File:",fname,"could not be opened for reading or is empty");
      else if(select_only)
	Close();
      else
	rstrm = istrm;
    }
  }
  if(chooser != NULL) {
    ivResource::unref(chooser);
    ivResource::flush();
    chooser = NULL;
  }
  return rstrm;
}

ostream* taivGetFile::Save(const char* nm, bool no_dlg) {
  if(nm != NULL) {
    fname = nm;
    file_selected = true;
  }
  if(no_dlg || !cssivSession::in_session)
    return open_write();

  if(fname == "")
    return SaveAs();
    
  if(open_write() == NULL) {
    return SaveAs(); 
  }
  return ostrm;
}

ostream* taivGetFile::SaveAs(const char* nm, bool no_dlg) {
  file_selected = false;
  if(nm != NULL) {
    fname = nm;
    file_selected = true;
  }
  if(no_dlg || !cssivSession::in_session)
    return open_write();

  if(chooser == NULL) {
    chooser = taivM->dkit->file_chooser(dir, taivM->wkit->style());
    ivResource::ref(chooser);		
  }

  chooser->style()->attribute("filter", "on");
  chooser->style()->attribute("defaultSelection", fname);
  chooser->style()->attribute("filterPattern", filter);
  chooser->style()->attribute("open", "Save");
  chooser->style()->attribute("caption", "Select File to Save for Writing");
  String win_title = String("Save: ") + filter;
  chooser->style()->attribute("name", win_title);
  chooser->style()->attribute("title", win_title);
  chooser->style()->attribute("iconName", win_title);
  int rval;
  ostream* rstrm = NULL;
  ivCoord x=0,y=0;
  taivDialog::GetPointerPos(x,y);
  rval = chooser->post_at_aligned(x, y, POINTER_ALIGN_X, POINTER_ALIGN_Y);
  if(rval) {
    file_selected = true;
    fname = chooser->selected()->string();
    bool no_go = false;
    if(!select_only && open_write_exist_check()) {
      int chs = taMisc::Choice("File: "+fname+" exists, overwrite?", "Ok", "Cancel");
      if(chs == 1) no_go = true;
    }
    if(!no_go && (open_write() != NULL)) {
      if(ostrm->bad())
	taMisc::Error("File:",fname,"could not be opened for SaveAs");
      else if(select_only)
	Close();
      else {
	rstrm = ostrm;
      }
    }
  }

  if(chooser != NULL) {
    ivResource::unref(chooser);
    ivResource::flush();
    chooser = NULL;
  }
  return rstrm;
}

ostream* taivGetFile::Append(const char* nm, bool no_dlg) {
  file_selected = false;
  if(nm != NULL) {
    fname = nm;
    file_selected = true;
  }
  if(no_dlg || !cssivSession::in_session)
    return open_append();

  if(chooser == NULL) {
    chooser = taivM->dkit->file_chooser(dir, taivM->wkit->style());
    ivResource::ref(chooser);		
  }

  chooser->style()->attribute("filter", "on");
  chooser->style()->attribute("defaultSelection", fname);
  chooser->style()->attribute("filterPattern", filter);
  chooser->style()->attribute("open", "Append");
  chooser->style()->attribute("caption", "Select File to Append for Writing");
  String win_title = String("Append: ") + filter;
  chooser->style()->attribute("name", win_title);
  chooser->style()->attribute("title", win_title);
  chooser->style()->attribute("iconName", win_title);
  int rval;
  ostream* rstrm = NULL;
  ivCoord x=0,y=0;
  taivDialog::GetPointerPos(x,y);
  rval = chooser->post_at_aligned(x, y, POINTER_ALIGN_X, POINTER_ALIGN_Y);
  if(rval) {
    file_selected = true;
    fname = chooser->selected()->string();
    if(open_append() != NULL) {
      if(ostrm->bad())
	taMisc::Error("File",fname,"could not be opened for append");
      else if(select_only)
	Close();
      else
	rstrm = ostrm;
    }
  }
  if(chooser != NULL) {
    ivResource::unref(chooser);
    ivResource::flush();
    chooser = NULL;
  }
  return NULL;
}

void taivGetFile::Close() {
  if(fstrm != NULL) {
    fstrm->close();
    delete fstrm;
    fstrm = NULL;
    istrm = NULL;		// these are ptrs to fstrm
    ostrm = NULL;
  }
  if(ifstrm != NULL) {
    ifstrm->close();
    delete ifstrm;
    ifstrm = NULL;
    istrm = NULL;
  }
  if(ofstrm != NULL) {
#ifdef SOLARIS
    int fd = ofstrm->rdbuf()->fd();
#endif
    ofstrm->close();
#ifdef SOLARIS
    close(fd);			// just in case -- doesn't get closed on solaris.
#endif
    delete ofstrm;
    ofstrm = NULL;
    ostrm = NULL;
  }
  if(istrm != NULL) {
    delete istrm;
    istrm = NULL;
  }
  if(ostrm != NULL) {
    delete ostrm;
    ostrm = NULL;
  }
  if(iofbuf != NULL) {
    delete [] iofbuf;
    iofbuf = NULL;
  }
  if(compress_pid > 0) {	// must wait around for child to die..
    pid_t wait_pid;
    int wstatus;
    do {
      wait_pid = waitpid(compress_pid, &wstatus, 0);
    } while((wait_pid == -1) && (errno == EINTR));
    compress_pid = 0;
  }

  open_file = false;
  file_selected = true;		// closing a file is as good as opening one
}

//////////////////////////////////////////
// 		taivObjChooser		//
//////////////////////////////////////////

#include <ta/enter_iv.h>
declareFieldEditorCallback(taivObjChooser)
implementFieldEditorCallback(taivObjChooser)

declareActionCallback(taivObjChooser)
implementActionCallback(taivObjChooser)
#include <ta/leave_iv.h>

taivObjChooser::taivObjChooser(TAPtr parob, ivWidgetKit* kt, ivStyle* s, const char* captn, bool selonly) {
  kit = kt;
  lst_par_obj = NULL;
  reg_par_obj = NULL;
  typ_par_obj = NULL;
  scope_ref = NULL;
  caption = captn;
  select_only = selonly;
  sel_obj = NULL;
  dialog = NULL;
  fbrowser = NULL;
  editor = NULL;
  style = new ivStyle(s);
  ivResource::ref(style);
  style->alias("taivObjChooser");
  style->alias("Dialog");

  if(parob->InheritsFrom(&TA_taList_impl))
    lst_par_obj = (TABLPtr)parob;
  else
    reg_par_obj = parob;

  Build();
}

taivObjChooser::taivObjChooser(TypeDef* td, ivWidgetKit* kt, ivStyle* s, const char* captn) {
  kit = kt;
  lst_par_obj = NULL;
  reg_par_obj = NULL;
  if(!td->InheritsFrom(TA_taBase)) {
    taMisc::Error("*** warning, will not be able to select non-taBase tokens in chooser");
  }
  typ_par_obj = td;
  scope_ref = NULL;
  caption = captn;
  select_only = true;		// always true for typedef!
  sel_obj = NULL;
  dialog = NULL;
  fbrowser = NULL;
  editor = NULL;
  style = new ivStyle(s);
  ivResource::ref(style);
  style->alias("taivObjChooser");
  style->alias("Dialog");
  Build();
}

taivObjChooser::~taivObjChooser() {
  ivResource::unref(body); 	body = NULL;
  ivResource::unref(dialog);	dialog = NULL;
  ivResource::unref(style);
}

int taivObjChooser::Choose() {
  ivCoord x=0,y=0;
  taivDialog::GetPointerPos(x,y);
  int rval = dialog->post_at_aligned(x, y, .5, .5);
  return rval;
}

void taivObjChooser::GetPathStr() {
  if(lst_par_obj != NULL)
    path_str = lst_par_obj->GetPath();
  else if(reg_par_obj != NULL)
    path_str = reg_par_obj->GetPath();
  else if(typ_par_obj != NULL)
    path_str = typ_par_obj->name;
}

void taivObjChooser::ReRead() {
  GetPathStr();
  editor->field((const char*)path_str);
  Clear();
  Load();
}

void taivObjChooser::Build() {
  const ivLayoutKit& layout = *ivLayoutKit::instance();
  kit->push_style(style);
  osString subcaption("Select an Object:");
  osString open("Select");
  style->find_attribute("select", open);
  osString descend("Descend");
  style->find_attribute("descend", descend);
  osString close("Cancel");
  style->find_attribute("cancel", close);
  long rows = 10;
  style->find_attribute("rows", rows);
  const ivFont* f = kit->font();
  ivFontBoundingBox bbox;
  f->font_bbox(bbox);
  ivCoord height = rows * (bbox.ascent() + bbox.descent()) + 1.0;
  ivCoord width;
  if(!style->find_attribute("width", width)) {
    width = 16 * f->width('m') + 3.0;
  }

  ivAction* accept = new ActionCallback(taivObjChooser)
    (this, &taivObjChooser::AcceptBrowser);
  ivAction* descend_act = NULL;
  if(!select_only)
    descend_act = new ActionCallback(taivObjChooser)
      (this, &taivObjChooser::DescendBrowser);
  ivAction* cancel = new ActionCallback(taivObjChooser)
    (this, &taivObjChooser::CancelBrowser);

  GetPathStr();
  editor = ivDialogKit::instance()->field_editor
    (path_str, style, new FieldEditorCallback(taivObjChooser)
     (this, &taivObjChooser::AcceptEditor, &taivObjChooser::CancelEditor));

  if(!select_only)
    fbrowser = new ivFileBrowser(kit, descend_act, cancel);
  else
    fbrowser = new ivFileBrowser(kit, accept, cancel);

  ivGlyph* g = layout.vbox();
  if(caption.length() > 0) {
    g->append(layout.rmargin(kit->fancy_label(caption), 5.0, fil, 0.0));
  }
  if(subcaption.length() > 0) {
    g->append(layout.rmargin(kit->fancy_label(subcaption), 5.0, fil, 0.0));
  }
  g->append(layout.vglue(5.0, 0.0, 2.0));
  g->append(editor);
  g->append(layout.vglue(15.0, 0.0, 12.0));
  g->append
    (layout.hbox
     (layout.vcenter
      (kit->inset_frame
       (layout.margin(layout.natural_span(fbrowser, width, height), 1.0)),
       1.0),
      layout.hspace(4.0),
      kit->vscroll_bar(fbrowser->adjustable())
      )
     );
  g->append(layout.vspace(15.0));
  if(!select_only) {
    g->append(layout.hbox
	      (layout.hglue(10.0),
	       layout.vcenter(kit->default_button(descend, descend_act)),
	       layout.hglue(10.0, 0.0, 5.0),
	       layout.vcenter(kit->push_button(open, accept)),
	       layout.hglue(10.0, 0.0, 5.0),
	       layout.vcenter(kit->push_button(close, cancel)),
	       layout.hglue(10.0)
	       )
	      );
  }
  else {
    g->append(layout.hbox
	      (layout.hglue(10.0),
	       layout.vcenter(kit->default_button(open, accept)),
	       layout.hglue(10.0, 0.0, 5.0),
	       layout.vcenter(kit->push_button(close, cancel)),
	       layout.hglue(10.0)
	       )
	      );
  }

  body = new ivPatch(layout.back(layout.vcenter(kit->outset_frame(layout.margin(g, 5.0)), 1.0),
		   new ivTarget(nil, TargetPrimitiveHit)));
  ivResource::ref(body);

  dialog = new ivDialog(body, style);
  ivResource::ref(dialog);

  dialog->remove_all_input_handlers();
  dialog->append_input_handler(editor);
  dialog->append_input_handler(fbrowser);
  dialog->focus(editor);

  kit->pop_style();
  Load();
}

void taivObjChooser::Clear() {
  ivBrowser& b = *fbrowser;
  b.select(-1);
  ivGlyphIndex n = b.count();
  for (ivGlyphIndex i = 0; i < n; i++) {
    b.remove_selectable(0);
    b.remove(0);
  }
  items.Reset();
}

void taivObjChooser::Load() {
  kit->push_style(style);

  if(lst_par_obj != NULL) {
    if(!select_only) {
      AddItem("..");
      TypeDef* td = lst_par_obj->GetTypeDef();
      int i;
      for (i=0; i<td->members.size; i++) {
	MemberDef* md = td->members.FastEl(i);
	if(!md->type->InheritsFrom(&TA_taBase)) continue;
	TAPtr mb = (TAPtr)md->GetOff((void*)lst_par_obj);
	if(mb->GetOwner() == NULL) continue; // not going to be a good selection item
	AddItem((const char*)md->name);
      }
    }
    int i;
    for (i=0; i<lst_par_obj->size; i++) {
      TAPtr ob = (TAPtr)lst_par_obj->FastEl_(i);
      String lbl = ob->GetName();
      if(lbl.length() == 0)
	lbl = String("[") + String(i) + "]";
      AddItem((const char*)lbl);
    }
  }
  else if(reg_par_obj != NULL) {
    if(!select_only) {
      AddItem("..");
    }
    TypeDef* td = reg_par_obj->GetTypeDef();
    int i;
    for (i=0; i<td->members.size; i++) {
      MemberDef* md = td->members.FastEl(i);
      if(!md->type->InheritsFrom(&TA_taBase)) continue;
      TAPtr mb = (TAPtr)md->GetOff((void*)reg_par_obj);
      if(mb->GetOwner() == NULL) continue; // not going to be a good selection item
      AddItem((const char*)md->name);
    }
  }
  else if(typ_par_obj != NULL) {
    AddTokens(typ_par_obj);
  }

  fbrowser->refresh();
  kit->pop_style();
}

void taivObjChooser::AddTokens(TypeDef* td) {
  int i;
  for(i=0; i<td->tokens.size; i++) {
    void* tmp = td->tokens.FastEl(i);
    String adrnm = String((long)tmp);
    if(td->InheritsFrom(TA_taBase)) {
      TAPtr btmp = (TAPtr)tmp;
      if((scope_ref != NULL) && !btmp->SameScope(scope_ref))
	continue;
      if(!btmp->GetName().empty()) {
	AddItem(btmp->GetName());
	items.Peek() = adrnm;	// always store the actual address in the string!
      }
      else {
	AddItem(adrnm);
      }
    }
    else {
      AddItem(adrnm);
    }
  }

  for(i=0; i<td->children.size; i++) {
    TypeDef* chld = td->children[i];
    if(chld->ptr != 0)
      continue;
    if((chld->tokens.size == 0) && (chld->tokens.sub_tokens == 0))
      continue;
    if((chld->tokens.size > 0) || (chld->children.size > 0))
      AddTokens(chld);
  }
}

void taivObjChooser::AddItem(const char* itm) {
  items.Add(itm);
  const ivLayoutKit& layout = *ivLayoutKit::instance();
  ivGlyph* name = kit->label(itm);
  ivGlyph* label = new ivTarget
    (layout.h_margin(name, 3.0, 0.0, 0.0, 15.0, fil, 0.0),
     TargetPrimitiveHit);
  ivTelltaleState* t = new ivTelltaleState(ivTelltaleState::is_enabled);
  if(sel_str == itm)
    t->set(ivTelltaleState::is_chosen, 1);
  fbrowser->append_selectable(t);
  fbrowser->append(new ivChoiceItem(t, label, kit->bright_inset_frame(label)));
}

void taivObjChooser::UpdateFmSelStr() {
  sel_obj = NULL;
  if(lst_par_obj != NULL) {
    if(sel_str == "root")
      sel_obj = tabMisc::root;
    else if(sel_str == "..")
      sel_obj = lst_par_obj->GetOwner();
    else
      sel_obj = lst_par_obj->FindFromPath(sel_str);
  }
  else if(reg_par_obj != NULL) {
    if(sel_str == "root")
      sel_obj = tabMisc::root;
    else if(sel_str == "..")
      sel_obj = reg_par_obj->GetOwner();
    else
      sel_obj = reg_par_obj->FindFromPath(sel_str);
  }
  else if(typ_par_obj != NULL) {
    if(!typ_par_obj->InheritsFrom(TA_taBase))
      return;
    long adr = (long)sel_str;
    sel_obj = (TAPtr)adr;
    if((sel_obj != NULL) && !sel_obj->GetName().empty())
      sel_str = sel_obj->GetName();
  }
  if(sel_obj == NULL) {
    taMisc::Error("Could not find object:", sel_str, "in path:", path_str,"try again");
    return;
  }
}

void taivObjChooser::AcceptBrowser() {
  int i = int(fbrowser->selected());
  if (i == -1) {
    if(!select_only)
      AcceptEditor(NULL);	// null is clue to not fork to descend!
    else
      AcceptEditor(editor);
    return;
  }
  sel_str = items.FastEl(i);

  UpdateFmSelStr();		// get the new selection based on that!
  if(sel_obj != NULL) 
    dialog->dismiss(true);
}

void taivObjChooser::DescendBrowser() {
  int i = int(fbrowser->selected());
  if (i == -1) {
    sel_str = editor->text()->string();
    if(select_only) {
      sel_str = sel_str.after(path_str);
    }
    else {
      reg_par_obj = tabMisc::root;
      lst_par_obj = NULL;
    }
  }
  else {
    sel_str = items.FastEl(i);
  }

  UpdateFmSelStr();		// get the new selection based on that!
  if(sel_obj == NULL) return;

  String nw_txt = sel_obj->GetPath();
  editor->field((const char*)nw_txt);

  if(sel_obj->InheritsFrom(&TA_taList_impl)) {
    lst_par_obj = (TABLPtr)sel_obj;
    reg_par_obj = NULL;
  }
  else {
    reg_par_obj = sel_obj;
    lst_par_obj = NULL;
  }
  ReRead();
}  

void taivObjChooser::CancelBrowser() {
  sel_obj = NULL;
  dialog->dismiss(false);
}

void taivObjChooser::AcceptEditor(ivFieldEditor* e) {
  if(!select_only && (e != NULL)) {
    DescendBrowser();
    return;
  }
  if(e == NULL) e = editor;
  sel_str = e->text()->string();
  if(select_only) {
    sel_str = sel_str.after(path_str);
    UpdateFmSelStr();		// get the new selection based on that!
    if(sel_obj != NULL) 
      dialog->dismiss(true);
  }
  else {
    reg_par_obj = tabMisc::root;
    lst_par_obj = NULL;
    UpdateFmSelStr();
    if(sel_obj != NULL) 
      dialog->dismiss(true);
  }
}

void taivObjChooser::CancelEditor(ivFieldEditor*) {
  dialog->dismiss(false);
}

//////////////////////////////////////////
// 		taivFileButton		//
//////////////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivFileButton)
implementActionCallback(taivFileButton)
#include <ta/leave_iv.h>

taivFileButton::taivFileButton(void* base, ivWindow* win, taivDialog* dlg, taivData* par,
			       bool rd_only, bool wrt_only)
: taivData(dlg, par) {
  if(base != NULL) {
    gf = *((taivGetFile**) base);
    if(gf != NULL)
      taRefN::Ref(gf);
  }
  else
    gf = NULL;
  cur_win = win;
  read_only = rd_only;
  write_only = wrt_only;
  filemenu = new taivMenu(taivMenu::menubar, taivMenu::normal_update, taivMenu::small, dlg, this);
  filemenu->SetMLabel("------No File-----");
}

taivFileButton::~taivFileButton() {
  delete filemenu;
  if(gf != NULL)
    taRefN::unRefDone(gf);
}
ivGlyph* taivFileButton::GetRep() {
  return filemenu->GetRep();
}
ivGlyph* taivFileButton::GetLook() {
  return filemenu->GetLook();
}

void* taivFileButton::GetValue(){
  return (void*) gf;
}

void taivFileButton::GetImage(void* base, ivWindow* win) {
  if(gf != NULL)
    taRefN::unRefDone(gf);
  gf = *((taivGetFile**) base);
  if(gf != NULL)
    taRefN::Ref(gf);
  cur_win = win;
  GetImage();
}

void taivFileButton::GetImage() {
  if(filemenu->items.size == 0) {
    if(!write_only)
      filemenu->AddItem("Open", NULL, taivMenu::use_default,
			new ActionCallback(taivFileButton)(this,&taivFileButton::Open));
    if(!read_only && ((gf == NULL) || !gf->select_only)) {
      filemenu->AddItem("Save", NULL, taivMenu::use_default,
			new ActionCallback(taivFileButton)(this,&taivFileButton::Save));
      filemenu->AddItem("SaveAs", NULL, taivMenu::use_default,
			new ActionCallback(taivFileButton)(this,&taivFileButton::SaveAs));
      filemenu->AddItem("Append", NULL, taivMenu::use_default,
			new ActionCallback(taivFileButton)(this,&taivFileButton::Append));
    }
    filemenu->AddItem("Close", NULL, taivMenu::use_default,
		      new ActionCallback(taivFileButton)(this,&taivFileButton::Close));
    filemenu->AddItem("Edit", NULL, taivMenu::use_default,
		      new ActionCallback(taivFileButton)(this,&taivFileButton::Edit));
  }

  if((gf == NULL) || (!gf->select_only && !gf->open_file) || (gf->fname == ""))
    filemenu->SetMLabel("------No File-----");
  else
    filemenu->SetMLabel(gf->fname);
}

void taivFileButton::GetGetFile() {
  if (gf ==  NULL) {
    ivWidgetKit* wkit = ivWidgetKit::instance();
    gf = new taivGetFile(".","",cur_win,wkit->style(),false);
    taRefN::Ref(gf);
  }
}

void taivFileButton::Open() {
  GetGetFile();
  if((gf->Open() != NULL) || (gf->select_only)) {
    GetImage();
  }
}

void taivFileButton::Append() {
  GetGetFile();
  if(gf->Append() != NULL) {
    GetImage();
  }
}

void taivFileButton::Save() {
  GetGetFile();
  if(gf->Save() != NULL) {
    GetImage();
  }
}
void taivFileButton::SaveAs() {
  GetGetFile();
  if(gf->SaveAs() != NULL) {
    GetImage();
  }
}
void taivFileButton::Close() {
  GetGetFile();
  gf->Close();
  if(gf->select_only)
    gf->fname = "";		// reset file name on close
  GetImage();
}

void taivFileButton::Edit() {
  GetGetFile();
  char* edtr_c = getenv("EDITOR");
  String edtr = "emacs";
  if(edtr_c != NULL)
    edtr = edtr_c;
  edtr += String(" ") + gf->fname + " &";
  system(edtr);
}



//////////////////////////
// 	taivToken	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivToken)
implementActionCallback(taivToken)
#include <ta/leave_iv.h>

taivToken::taivToken(int rt, int ft, TypeDef* td, taivDialog* dlg, taivData* par, bool nul_not,
		       bool edt_not)
: taivData(dlg, par) {
  typd = td;
  ta_menu = new taivHierMenu(rt, taivMenu::radio_update, ft, (void*)typd, dlg, this);
  ownflag = true;
  scope_ref = NULL;
  null_not = nul_not;
  edit_not = edt_not;
  over_max = false;
  chs_obj = NULL;
}
taivToken::taivToken(taivHierMenu* existing_menu, TypeDef* td, taivDialog* dlg, taivData* par,
		       bool nul_not, bool edt_not)
: taivData(dlg, par) {
  typd = td;
  ta_menu = existing_menu;
  ownflag = false;
  scope_ref = NULL;
  null_not = nul_not;
  edit_not = edt_not;
  over_max = false;
  chs_obj = NULL;
}

taivToken::~taivToken() {
  if(ownflag)
    delete ta_menu;
}
ivGlyph* taivToken::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* taivToken::GetLook() {
  return ta_menu->GetLook();
}
void taivToken::GetImage(void* ths, TAPtr scp_obj, ivWindow* win) {
  scope_ref = scp_obj;
  ta_menu->GetImage(ths);
  cur_win = win;
  chs_obj = (TAPtr)ths;
}
void* taivToken::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(over_max) {
    if((cur == NULL) || (cur->label == "<Over max, select>")
       || (cur->label == "gp.<Over max, select>"))
      return chs_obj;
    else
      return (TAPtr)cur->usr_data;
  }
  if(cur != NULL)
    return cur->usr_data;
  return NULL;
}
void taivToken::Edit() {
  void* cur_base = GetValue();
  if(cur_base == NULL) return;

  taivEdit* gc;
  const ivColor* bgclr = NULL;
  if(typd->InheritsFrom(TA_taBase)) {
    bgclr = ((TAPtr)cur_base)->GetEditColorInherit();
    gc = (taivEdit*) ((taBase*)cur_base)->GetTypeDef()->ive;
  }
  else {
    gc = (taivEdit*)typd->ive;
  }
  if((bgclr == NULL) && (dialog != NULL)) bgclr = dialog->bg_color;
  bool wait = false;
  if(dialog != NULL) wait = dialog->waiting;

  gc->Edit(cur_base, cur_win, wait, false, bgclr);
}

void taivToken::Chooser() {
  taivObjChooser* chs = new taivObjChooser(typd, taivM->wkit, taivM->wkit->style(),
					   "Tokens of Given Type");
  chs->scope_ref = scope_ref;
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

void taivToken::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  if(ownflag) {
    if(!null_not)
      ta_menu->AddItem("NULL", NULL, taivMenu::use_default, actn);
    if(!edit_not)
      ta_menu->AddItem("Edit", NULL, taivMenu::normal,
		       new ActionCallback(taivToken)(this, &taivToken::Edit));
    if(!null_not || !edit_not)
      ta_menu->AddSep();
  }
  GetMenu_impl(typd, actn);
  ivResource::unref(actn);
}
void taivToken::UpdateMenu(taivMenuAction* actn) {
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void taivToken::GetMenu_impl(TypeDef* td, taivMenuAction* actn) {
  String	nm;
  int i;
  bool too_many = false;
  if(td->InheritsFrom(TA_taBase) && (scope_ref != NULL)) {
    int cnt = taBase::NTokensInScope(td, scope_ref);
    if(cnt > taMisc::max_menu)
      too_many = true;
  }
  else if(td->tokens.size > taMisc::max_menu) {
    too_many = true;
  }
  if(too_many) {
    taivMenuEl* mnel = ta_menu->AddItem
      ("<Over max, select>", (void*)NULL, taivMenu::normal,
       new ActionCallback(taivToken)(this, &taivToken::Chooser));
    over_max = true;
    if(actn != NULL) {		// also set callback action!
      mnel->men_act = actn;
      ivResource::ref(mnel->men_act);
    }
  }
  else {
    
    if(!td->tokens.keep) {
      ta_menu->AddItem("<Sorry, not keeping tokens>", (void*)NULL, taivMenu::normal);
    }
    else {
      for(i=0; i<td->tokens.size; i++) {
	void* tmp = td->tokens.FastEl(i);
	nm = String((long)tmp);
	if(td->InheritsFrom(TA_taBase)) {
	  TAPtr btmp = (TAPtr)tmp;
	  if((scope_ref != NULL) && !btmp->SameScope(scope_ref))
	    continue;
	  if(!btmp->GetName().empty())
	    nm = btmp->GetName();
	}
	ta_menu->AddItem((char*)nm, tmp, taivMenu::use_default, actn);
      }
    }
  }

  if(td->children.size == 0)
    return;

  if(td->tokens.size > 0)
    ta_menu->AddSep();

  for(i=0; i<td->children.size; i++) {
    TypeDef* chld = td->children[i];
    if(chld->ptr != 0)
      continue;
    if((chld->tokens.size == 0) && (chld->tokens.sub_tokens == 0))
      continue;
    if(chld->tokens.size != 0) {
      int cur_no = ta_menu->cur_sno;
      ta_menu->AddSubMenu(chld->name, (void*)chld);
      ta_menu->AddSep();
      GetMenu_impl(chld, actn);
      ta_menu->SetSub(cur_no);
    }
    else {
      GetMenu_impl(chld, actn);	// if no tokens, don't add a submenu..
    }
  }
}

//////////////////////////////////
// 	taivSubToken		//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivSubToken)
implementActionCallback(taivSubToken)
#include <ta/leave_iv.h>

taivSubToken::taivSubToken(int rt, int ft, TypeDef* td, taivDialog* dlg, taivData* par,
			   bool nul_ok, bool edt_not)
: taivData(dlg, par) {
  menubase = NULL;
  cur_win = NULL;
  typd = td;
  ta_menu = new taivMenu(rt,taivMenu::radio_update,ft,td->name,dlg,this);
  ownflag = true;
  null_ok = nul_ok;
  edit_not = edt_not;
}

taivSubToken::taivSubToken(taivHierMenu* existing_menu, TypeDef* td,
			     taivDialog* dlg, taivData* par, bool nul_ok, bool edt_not)
: taivData(dlg, par) {
  menubase = NULL;
  cur_win = NULL;
  typd = td;
  ta_menu = existing_menu;
  ownflag = false;
  null_ok = nul_ok;
  edit_not = edt_not;
}

taivSubToken::~taivSubToken() {
  if(ownflag)
    delete ta_menu;
}
ivGlyph* taivSubToken::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* taivSubToken::GetLook() {
  return ta_menu->GetLook();
}

void* taivSubToken::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(cur) return cur->usr_data;
  return NULL;
}

void taivSubToken::Edit() {
  void* cur_base = GetValue();
  if(cur_base == NULL) return;

  taivEdit* gc;
  const ivColor* bgclr = NULL;
  if(typd->InheritsFrom(TA_taBase)) {
    bgclr = ((TAPtr)cur_base)->GetEditColorInherit();
    gc = (taivEdit*) ((taBase*)cur_base)->GetTypeDef()->ive;
  }
  else {
    gc = (taivEdit*)typd->ive;
  }

  if((bgclr == NULL) && (dialog != NULL)) bgclr = dialog->bg_color;
  bool wait = false;
  if(dialog != NULL) wait = dialog->waiting;

  gc->Edit(cur_base, cur_win, wait, false, bgclr);
}

void taivSubToken::GetImage(void* ths, ivWindow* win,void* sel) {
  if(menubase != ths) {
    menubase = ths;
    UpdateMenu();
  }
  if(sel==NULL) sel = ths;
  if(!(ta_menu->GetImage(sel))) ta_menu->GetImage(ths);

  cur_win = win;
}

void taivSubToken::UpdateMenu(taivMenuAction* actn){
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void taivSubToken::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  if(null_ok)
    ta_menu->AddItem("NULL", NULL, taivMenu::use_default, actn);
  if(!edit_not)
    ta_menu->AddItem("Edit", NULL, taivMenu::normal,
		     new ActionCallback(taivSubToken)(this, &taivSubToken::Edit));
  if(null_ok || !edit_not)
    ta_menu->AddSep();

  GetMenuImpl(menubase,actn);
  ivResource::unref(actn);
}

void taivSubToken::GetMenuImpl(void* base,taivMenuAction* actn){
  if (base== NULL) return;
  taBase* rbase = (taBase*) base;
  taBase** memb;
  TypeDef* basetd = rbase->GetTypeDef();

  // if you're the right type, put yourself on the list
  if(basetd->DerivesFrom(typd)) {
    String nm = rbase->GetName();
    ta_menu->AddItem(nm, rbase, taivMenu::use_default, actn);
  }

  // put your typed members on the list
  MemberDef* md;
  int i;
  for(i=0;i<basetd->members.size;i++){
    md = basetd->members.FastEl(i);
    if(md->type->DerivesFrom(typd) && !(md->HasOption("NO_SUBTYPE"))) {
      memb = (taBase **) md->GetOff((void*)rbase);
      if ((*memb != NULL) && ((void *) *memb != (void *) rbase)) {
	GetMenuImpl((void *) *memb,actn);
      }
    }
  }
}


//////////////////////////////////
// 	taivMemberDefMenu	//
//////////////////////////////////

taivMemberDefMenu::taivMemberDefMenu(int rt, int ft,MemberDef* m, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  md = m;
  menubase = NULL;
  sp =  NULL;
  typd = NULL;
  cur_win = NULL;
  String nm;
  if(md != NULL) nm = md->name;
  ta_menu = new taivMenu(rt,taivMenu::radio_update,ft,nm, dlg, this);
}

taivMemberDefMenu::~taivMemberDefMenu() {
  if(ta_menu != NULL)
    delete ta_menu;
}
ivGlyph* taivMemberDefMenu::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* taivMemberDefMenu::GetLook() {
  return ta_menu->GetLook();
}

void* taivMemberDefMenu::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(cur) return cur->usr_data;
  return NULL;
}

void taivMemberDefMenu::GetImage(MemberSpace* space, MemberDef* memb) {
  sp = space;
  md = memb;
  typd = NULL;
  UpdateMenu();
  ta_menu->GetImage(md);
}

void taivMemberDefMenu::GetImage(TypeDef* type, MemberDef* memb){
  md = memb;
  typd = type;
  sp = NULL;
  UpdateMenu();
  ta_menu->GetImage(md);
}

void taivMemberDefMenu::GetImage(void* ths, ivWindow* win,void* sel) {
  if(menubase != ths) {
    menubase = ths;
  }

  // a memberdef* can have one of three options to specify the
  // type for its memberdefmenu.
  // 1) a TYPE_xxxx in its comment directives
  // 2) a TYPE_ON_xxx in is comment directives, specifying the name
  //    of the member in the same object which is a TypeDef*
  // 3) Nothing, which defaults to the type of the object the memberdef
  //      is in.

  String mb_nm = md->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    void* adr;
    MemberDef* tdmd = ((taBase*) menubase)->FindMembeR(mb_nm, adr);
    if(tdmd == NULL) return;
    typd = *((TypeDef **) tdmd->GetOff(ths));
  }
  else {
    mb_nm = md->OptionAfter("TYPE_");
    if(mb_nm != "")
      typd = taMisc::types.FindName((char *) mb_nm);
    else
      typd = ((taBase *)ths)->GetTypeDef();
  }
  sp =  NULL;
  UpdateMenu();
  if(!(ta_menu->GetImage(sel))) ta_menu->GetImage(ta_menu->items.FastEl(0));
  cur_win = win;
}

void taivMemberDefMenu::UpdateMenu(taivMenuAction* actn){
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void taivMemberDefMenu::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  GetMenuImpl(menubase,actn);
  ivResource::unref(actn);
}

void taivMemberDefMenu::GetMenuImpl(void*,taivMenuAction* actn){
  if((typd == NULL) && (sp == NULL)){
    ta_menu->AddItem("TypeSpace Error",NULL,taivMenu::use_default,actn);
    return;
  }
  MemberDef* mbd;
  int i;
  MemberSpace* mbs = sp;
  if(mbs == NULL) mbs = &typd->members;
  for(i=0;i<mbs->size;i++){
    mbd = mbs->FastEl(i);
    ta_menu->AddItem(mbd->GetLabel(),mbd,taivMenu::use_default,actn);
  }
}

//////////////////////////////////
// 	taivMethodDefMenu	//
//////////////////////////////////

taivMethodDefMenu::taivMethodDefMenu(int rt, int ft,MethodDef* m, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  md = m;
  menubase = NULL;
  sp =  NULL;
  typd = NULL;
  cur_win = NULL;
  String nm;
  if(md != NULL) nm = md->name;
  ta_menu = new taivMenu(rt,taivMenu::radio_update,ft,nm, dlg, this);
}

taivMethodDefMenu::~taivMethodDefMenu() {
  if(ta_menu != NULL)
    delete ta_menu;
}
ivGlyph* taivMethodDefMenu::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* taivMethodDefMenu::GetLook() {
  return ta_menu->GetLook();
}

void* taivMethodDefMenu::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(cur) return cur->usr_data;
  return NULL;
}

void taivMethodDefMenu::GetImage(MethodSpace* space, MethodDef* memb) {
  sp = space;
  md = memb;
  typd = NULL;
  UpdateMenu();
  ta_menu->GetImage(md);
}

void taivMethodDefMenu::GetImage(TypeDef* type, MethodDef* memb){
  md = memb;
  typd = type;
  sp = NULL;
  UpdateMenu();
  ta_menu->GetImage(md);
}

void taivMethodDefMenu::GetImage(void* ths, ivWindow* win,void* sel) {
  if(menubase != ths) {
    menubase = ths;
  }

  String mb_nm = md->OptionAfter("TYPE_ON_");
  if(mb_nm != "") {
    void* adr;
    MemberDef* tdmd = ((taBase*) menubase)->FindMembeR(mb_nm, adr);
    if(tdmd == NULL) return;
    typd = *((TypeDef **) tdmd->GetOff(ths));
  }
  else {
    mb_nm = md->OptionAfter("TYPE_");
    if(mb_nm != "")
      typd = taMisc::types.FindName((char *) mb_nm);
    else
      typd = ((taBase *)ths)->GetTypeDef();
  }
  sp =  NULL;
  UpdateMenu();
  if(!(ta_menu->GetImage(sel))) ta_menu->GetImage(ta_menu->items.FastEl(0));
  cur_win = win;
}

void taivMethodDefMenu::UpdateMenu(taivMenuAction* actn){
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}

void taivMethodDefMenu::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  GetMenuImpl(menubase,actn);
  ivResource::unref(actn);
}

void taivMethodDefMenu::GetMenuImpl(void*,taivMenuAction* actn) {
  if((typd == NULL) && (sp == NULL)) {
    ta_menu->AddItem("TypeSpace Error",NULL,taivMenu::use_default,actn);
    return;
  }
  MethodDef* mbd;
  int i;
  MethodSpace* mbs = sp;
  if(mbs == NULL) mbs = &typd->methods;
  for(i=0;i<mbs->size;i++){
    mbd = mbs->FastEl(i);
    ta_menu->AddItem(mbd->GetLabel(),mbd,taivMenu::use_default,actn);
  }
}

//////////////////////////////////
// 	taivTypeHier		//
//////////////////////////////////

taivTypeHier::taivTypeHier(int  rt, int ft, TypeDef* td, taivDialog* dlg, taivData* par, bool nul_ok)
: taivData(dlg, par) {
  typd = td;
  ta_menu = new taivHierMenu(rt, taivMenu::radio_update, ft, (void*)typd, dlg, this);
  ownflag = true;
  null_ok = nul_ok;
}
taivTypeHier::taivTypeHier(taivHierMenu* existing_menu, TypeDef* td,
			     taivDialog* dlg, taivData* par, bool nul_ok)
: taivData(dlg, par) {
  typd = td;
  ta_menu = existing_menu;
  ownflag = false;
  null_ok = nul_ok;
}
taivTypeHier::~taivTypeHier() {
  if(ownflag && (ta_menu != NULL)) delete ta_menu;
  ta_menu = NULL;
}
ivGlyph* taivTypeHier::GetRep() {
  return ta_menu->GetRep();
}
ivGlyph* taivTypeHier::GetLook() {
  return HiBGLook(ta_menu->GetLook());
}
void taivTypeHier::GetImage(TypeDef* ths) {
  ta_menu->GetImage((void*)ths);
}
void taivTypeHier::GetMenu(taivMenuAction* actn) {
  ivResource::ref(actn);
  if(null_ok)
    ta_menu->AddItem("NULL", NULL, taivMenu::use_default, actn);
  GetMenu_impl(typd, actn);
  ivResource::unref(actn);
}
void taivTypeHier::UpdateMenu(taivMenuAction* actn) {
  ta_menu->ResetMenu();
  ivResource::flush();
  GetMenu(actn);
}
int  taivTypeHier::CountChildren(TypeDef* td) {
  int i, rval = 0;
  TypeDef*	chld;
  for(i=0; i<td->children.size; i++) {
    chld = td->children[i];
    if(chld->ptr != 0)
      continue;
    rval++;
  }
  return rval;
}

bool taivTypeHier::AddType(TypeDef* td) {
  if(!td->InheritsFormal(TA_class)) // only type classes please..
    return false;
  // don't add any template instances that have a single further subclass
  // (use the subclass instead)
  if(td->InheritsFormal(TA_templ_inst)) {
    if((td->children.size != 1) || (td->children.FastEl(0)->parents.size > 1))
      return true;
    return false;
  }
  if((td->members.size==0) && (td->methods.size==0) && !(td == &TA_taBase))
    return false;		// don't clutter list with these..
  return true;
}

void taivTypeHier::GetMenu_impl(TypeDef* td, taivMenuAction* actn) {
  ta_menu->AddItem((char*)td->name, (void*)td, taivMenu::use_default, actn);
  ta_menu->AddSep();
  int i;
  for(i=0; i<td->children.size; i++) {
    TypeDef*	chld = td->children.FastEl(i);
    if(chld->ptr != 0)
      continue;
    if(!AddType(chld)) {
      if(chld->InheritsFormal(TA_templ_inst) && (chld->children.size == 1)) {
	GetMenu_impl(chld->children.FastEl(0), actn);
      }
      continue;
    }
    if(CountChildren(chld) > 0) {
      int cur_no = ta_menu->cur_sno;
      ta_menu->AddSubMenu((char*)chld->name, (void*)chld);
      GetMenu_impl(chld, actn);
      ta_menu->SetSub(cur_no);
    }
    else
      ta_menu->AddItem((char*)chld->name, (void*)chld, taivMenu::use_default, actn);
  }
}

TypeDef* taivTypeHier::GetValue() {
  taivMenuEl* cur = ta_menu->GetValue();
  if(cur) return (TypeDef*)cur->usr_data; return NULL;
}


///////////////////////////
// 	taivArgDlg       //
///////////////////////////

class taivArgDialog : public cssivEditDialog {
  // edit dialog for editing function arguments
public:
  bool		err_flag; 	// true if an arg was improperly set
  taivMethMenu*	meth;

  taivArgDialog(taivMethMenu* mm);
  taivArgDialog()		{ };
  ~taivArgDialog();

  void		Constr_Strings(const char* prompt="", const char* win_title="");
  void		Constr(ivWindow* win=NULL, bool wait=false, const char* prompt="",
		       const char* win_title="", bool no_cancel=false, const ivColor* bgclr = NULL);
  void		Constr_ArgTypes();
  void		GetImage(); 
  void		GetValue();
  int		Edit();

  void		Ok();

  void		Constr_Labels(); // construct the labels list
  void		Constr_Data(); // construct the static elements of the dialog
  
  taivArgType*	GetBestArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
};

taivArgDialog::taivArgDialog(taivMethMenu* mm)
: cssivEditDialog(new cssClassInst(mm->meth->name))
{
  meth = mm;
  err_flag = false;
}

taivArgDialog::~taivArgDialog() {
  delete obj;
}

void taivArgDialog::Ok() {
  GetValue();			// always get the value of an arg dialog..
  state = ACCEPTED;
  dialog->dismiss(1);
}

void taivArgDialog::Constr_Strings(const char*, const char* win_title) {
  if(meth->meth->HasOption("CONFIRM")) {
    prompt_str = "Ok To: ";
    prompt_str += meth->meth->name + ": " + meth->meth->desc;
    win_str = String(ivSession::instance()->classname()) + ": " + win_title;
    win_str += String(" ") + meth->meth->name + " () Confirm";
  }
  else {
    prompt_str = meth->meth->name + ": " + meth->meth->desc;
    win_str = String(ivSession::instance()->classname()) + ": " + win_title;
    win_str += String(" ") + meth->meth->name + " (Args)";
  }
}

void taivArgDialog::Constr(ivWindow* win, bool wait, const char* aprompt,
			  const char* win_title, bool no_cancel, const ivColor* bgclr)
{
  Constr_ArgTypes();
  cssivEditDialog::Constr(win, wait, aprompt, win_title, no_cancel, bgclr);
}

void taivArgDialog::Constr_ArgTypes() {
  obj->name = String("(") + meth->meth->type->name + ") " + meth->meth->name +
    ": " + meth->meth->desc;
  obj->members->Push(&cssMisc::Void); // this is just a place-holder for arg[0]
  int i;
  for(i=0; i<meth->use_argc; i++) {
    taivArgType* art = GetBestArgType(i, meth->meth->arg_types.FastEl(i),
				       meth->meth, meth->typ);
    if(art == NULL)
      break;			// don't add new args after bad one..
    cssEl* el = art->GetElFromArg(meth->meth->arg_names.FastEl(i), meth->base);
    if(el == NULL) {
      delete art;
      break;
    }
    // set to default value if not empty
    if(!meth->meth->arg_vals.FastEl(i).empty()) {
      // not for type def pointers (cuz you lose ability to select other types)
      if(!art->arg_typ->DerivesFrom(&TA_TypeDef) && !art->arg_typ->DerivesFrom(&TA_ios)) {
	String val = meth->meth->arg_vals.FastEl(i);
	while(val.firstchar() == ' ') val = val.after(' ');
	if(art->arg_typ->DerivesFormal(TA_enum)) {
	  art->arg_typ->SetValStr(val, art->arg_base);
	}
	else {
	  *el = val;
	}
      }
    }
    obj->members->Push(el);
    type_el.Add(art);
  }
}

taivArgType* taivArgDialog::GetBestArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td) {
  taivArgType* hi_arg = NULL;
  int hi_bid = 0;
  int i;
  for(i=0; i<taivMisc::arg_types.size; i++) {
    taivArgType* art = (taivArgType*)taivMisc::arg_types.FastEl(i)->GetInstance();
    int bid = art->BidForArgType(aidx, argt, md, td);
    if(bid >= hi_bid) {		// preference for the last one..
      hi_bid = bid;
      hi_arg = art;
    }
  }
  if(hi_arg == NULL)
    return NULL;
  return hi_arg->ArgTypeInst(argt, md, td);
}

void taivArgDialog::Constr_Labels() {
  Constr_Labels_style();
  GetVFix(taivM->name_font);
  // GetHFix();
  ivFontBoundingBox fbb;
  int i;
  hfix = 0;
  for(i=1; i< obj->members->size; i++) { // skip over the 1st member
    cssEl* md = obj->members->FastEl(i);
    String nm = md->GetName();
    taivM->name_font->string_bbox(nm, nm.length(), fbb);
    hfix = MAX(fbb.width(), hfix);
  }

  labels = layout->vbox();
  for(i=1; i< obj->members->size; i++) {
    cssEl* md = obj->members->FastEl(i);
    // 32 is for the openlook extra v symbol...
    labels->append(layout->fixed(taivM->top_sep(GetNameRep(i, md)), hfix + 32, vfix));
  }
  wkit->pop_style();
}

void taivArgDialog::Constr_Data() {
  data_g = layout->vbox();
  int i;
  for(i=0; i<type_el.size; i++) {
    taivArgType* art = (taivArgType*)type_el.FastEl(i);
    taivData* mb_dat = art->GetDataRep(dialog,this);

    data_el.Add(mb_dat);
    data_g->append(layout->vfixed(taivM->top_sep(mb_dat->GetLook()),vfix));
  }
  GetImage();
}

void taivArgDialog::GetValue() {
  err_flag = false;
  int i;
  for(i=0; i<type_el.size; i++) {
    taivArgType* art = (taivArgType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    art->GetValue(mb_dat, meth->base);
    if(art->err_flag)
      err_flag = true;
    else {
      if(art->arg_typ->DerivesFormal(TA_enum)) {
	art->meth->arg_vals.FastEl(i) = art->arg_typ->GetValStr(art->arg_base);
      }
      else if(!art->arg_typ->DerivesFrom(TA_ios)) {
	art->meth->arg_vals.FastEl(i) = art->arg_val->GetStr();
      }
    }
  }
  Unchanged();
}

void taivArgDialog::GetImage() {
  int i;
  for(i=0; i<type_el.size; i++) {
    taivArgType* art = (taivArgType*)type_el.FastEl(i);
    taivData* mb_dat = data_el.FastEl(i);
    art->GetImage(mb_dat, meth->base, cur_win);
  }
  Unchanged();
  FocusOnFirst();
}

int taivArgDialog::Edit() {
  // special case for a single stream arg
  if((meth->use_argc == 1) && meth->meth->arg_types[0]->InheritsFrom(TA_ios)
     && !meth->meth->HasOption("FILE_ARG_EDIT"))
  {
    taivStreamArgType* sa = (taivStreamArgType*)type_el.FastEl(0);
    if(sa->gf == NULL)
      return cssivEditDialog::Edit();
    if(sa->arg_typ->InheritsFrom(TA_ostream)) {
      if(meth->meth->HasOption("QUICK_SAVE"))
	sa->gf->Save();
      else if(meth->meth->HasOption("APPEND_FILE"))
	sa->gf->Append();
      else
	sa->gf->SaveAs();
    }
    else {
      sa->gf->Open();
    }
    state = ACCEPTED;
    sa->GetValueFromGF();
    if(sa->err_flag)
      return false;
    return true;
  }
  return cssivEditDialog::Edit();
}

///////////////////////////
// 	taivMethMenu     //
///////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(taivMethMenu)
implementActionCallback(taivMethMenu)
#include <ta/leave_iv.h>

taivMethMenu::taivMethMenu(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg, taivData* par)
: taivData(dlg, par) {
  base = bs;
  meth = md;
  typ = td;
  if((base != NULL) && typ->InheritsFrom(TA_taBase)) {
    typ = ((TAPtr)base)->GetTypeDef(); // get the *actual* type def of this object!
  }
  is_menu_item = true;
  args = NULL;
  arg_dlg = NULL;
  use_argc = 0;
}

void taivMethMenu::AddToMenu(taivMenu* mnu) {
  if(meth->HasOption("MENU_SEP_BEFORE"))
    mnu->AddSep();
  mnu->AddItem(meth->GetLabel(), NULL, taivMenu::use_default, 
	       new ActionCallback(taivMethMenu)(this, &taivMethMenu::CallFun));
  if(meth->HasOption("MENU_SEP_AFTER"))
    mnu->AddSep();
}

void taivMethMenu::CallFun() {
  if((meth->stubp == NULL) || (base == NULL))
    return;

  ApplyBefore();
  use_argc = meth->fun_argc;
  String argc_str = meth->OptionAfter("ARGC_");
  if(argc_str != "")
    use_argc = (int)argc_str;
  use_argc = MIN(use_argc, meth->arg_types.size);
  use_argc = MIN(use_argc, meth->arg_names.size);
  if((use_argc == 0) && !meth->HasOption("CONFIRM")) {
    GenerateScript();
#ifdef DMEM_COMPILE
    // don't actually run the command when using gui in dmem mode: everything happens via the script!
    if(taMisc::dmem_nprocs == 1) {
#endif
      cssEl* rval = (*(meth->stubp))(base, 0, (cssEl**)NULL);
      UpdateAfter();
      if(rval != &cssMisc::Void)
	ShowReturnVal(rval);
#ifdef DMEM_COMPILE
    }
#endif
    return;
  }
  arg_dlg = new taivArgDialog(this);
  const ivColor* bgclr = NULL;
  if(typ->InheritsFrom(TA_taBase)) {
    bgclr = ((TAPtr)base)->GetEditColorInherit();
  }
  if((bgclr == NULL) && (dialog != NULL)) bgclr = dialog->bg_color;
  arg_dlg->Constr(NULL, true, "", "", false, bgclr);
  int ok_can = arg_dlg->Edit();	// true = wait for a response
  if(ok_can && !arg_dlg->err_flag) {
    GenerateScript();
#ifdef DMEM_COMPILE
    // don't actually run the command when using gui in dmem mode: everything happens via the script!
    if(taMisc::dmem_nprocs == 1) {
#endif
      cssEl* rval = (*(meth->stubp))(base, arg_dlg->obj->members->size-1,
				     arg_dlg->obj->members->els);
      UpdateAfter();
      if(rval != &cssMisc::Void)
	ShowReturnVal(rval);
#ifdef DMEM_COMPILE
    }
#endif
  }
  delete arg_dlg;
}

void taivMethMenu::ShowReturnVal(cssEl* rval) {
  if(!(meth->HasOption("USE_RVAL") ||
       (meth->HasOption("USE_RVAL_RMB") && (arg_dlg->mouse_button == ivEvent::right))))
    return;

  if((rval->GetType() == cssEl::T_TA) || (rval->GetType() == cssEl::T_Class)) {
    if(dialog != NULL)
      rval->Edit(dialog->waiting);
    else
      rval->Edit(false);
    return;
  }
  String val = meth->name + " Return Value: ";
  val += rval->PrintStr();
  taivDialog::WaitDialog(NULL, val, "Return Value", true);
}

void taivMethMenu::ApplyBefore() {
  if((dialog == NULL) || (dialog->state != taivDialog::ACTIVE))
    return;
  if(meth->HasOption("NO_APPLY_BEFORE") || !dialog->modified)
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
    
void taivMethMenu::UpdateAfter() {
  if(meth->HasOption("NO_REVERT_AFTER"))
    return;
  // this is for stuff just called from menus, not dialog
  if((dialog == NULL) || (dialog->state != taivDialog::ACTIVE)) {
    if(base == NULL) return;
    TAPtr tap = (TAPtr)base;
    if(meth->HasOption("UPDATE_MENUS"))
      taivMisc::Update(tap);
    return;
  }
  // this is inside the dialog itself
  if((dialog->typ != NULL) && dialog->typ->InheritsFrom(TA_taBase)) {
    TAPtr tap = (TAPtr)dialog->cur_base;
    if(meth->HasOption("UPDATE_MENUS"))
      taivMisc::Update(tap);	// update menus and stuff
  }
  // almost always revert dialog..
  dialog->Revert();		// apply stuff dealt with already
}

void taivMethMenu::GenerateScript() {
  if((taivMisc::record_script == NULL) || !typ->InheritsFrom(TA_taBase))
    return;

#ifndef DMEM_COMPILE
  // dmem requires everything to be scripted to share commands!
  if(meth->HasOption("NO_SCRIPT")) {
    return;
  }
#endif

  TAPtr tab = (TAPtr)base;

  if((use_argc == 0) || (arg_dlg == NULL)) {
    String rscr = tab->GetPath() + "." + meth->name + "();";
#ifdef DMEM_COMPILE
    if(taMisc::dmem_debug)
      cerr << "proc: " << taMisc::dmem_proc << " recording fun call: " << rscr << endl;
#endif
    *(taivMisc::record_script) << rscr << endl;
    return;
  }
  
  int_Array tmp_objs;		// indicies of the temp objects
  int i;
  for(i=0; i<arg_dlg->type_el.size; i++) {
    taivArgType* art = (taivArgType*)arg_dlg->type_el.FastEl(i);
    if((art->arg_typ->ptr == 0) && art->arg_typ->DerivesFrom(TA_taBase))
      tmp_objs.Add(i+1);
  }
  if(tmp_objs.size > 0) {
    *taivMisc::record_script << "{ "; // put in context
    for(i=0; i<tmp_objs.size; i++) {
      taivArgType* art = (taivArgType*)arg_dlg->type_el.FastEl(tmp_objs.FastEl(i)-1);
      cssEl* argv = arg_dlg->obj->members->FastEl(tmp_objs.FastEl(i));
      *taivMisc::record_script << art->arg_typ->name << " tmp_" << i
			       << " = new " << art->arg_typ->name << "; \t tmp_" << i
			       << " = \"" << argv->GetStr() << "\";\n  ";
    }
  }

  String arg_str;
  for(i=1; i<arg_dlg->obj->members->size; i++) {
    if(i > 1)
      arg_str += ", ";
    int idx;
    if((idx = tmp_objs.Find(i)) >= 0)
      arg_str += "tmp_" + String(idx);
    else {
      cssEl* argv = arg_dlg->obj->members->FastEl(i);
      if(argv->GetType() == cssEl::T_String) {
	arg_str += "\"";
	arg_str += argv->PrintFStr();
	arg_str += "\"";
      }
      else if(argv->GetType() == cssEl::T_TA) {
	cssTA* cssta = (cssTA*)argv;
	TypeDef* csstd = cssta->type_def;
	if(csstd->InheritsFrom(&TA_istream) || csstd->InheritsFrom(&TA_fstream)
	   || csstd->InheritsFrom(&TA_ostream))
	  arg_str += "\"" + taivGetFile::last_fname + "\"";
	else {
	  arg_str += argv->PrintFStr();
	}
      }
      else {
	arg_str += argv->PrintFStr();
      }
    }
  }

  String rscr = tab->GetPath() + "." + meth->name + "(" + arg_str + ");\n";
  *taivMisc::record_script << rscr;
#ifdef DMEM_COMPILE
    if(taMisc::dmem_debug)
      cerr << "proc: " << taMisc::dmem_proc << " recording fun call: " << rscr << endl;
#endif
  if(tmp_objs.size > 0) {
    for(i=0; i<tmp_objs.size; i++) {
      *taivMisc::record_script << "  delete tmp_" << i << ";\n";
    }
    *taivMisc::record_script << "}\n"; // put in context
  }
  taivMisc::record_script->flush();
}



// normal non quoted members
void taivMisc::ScriptRecordAssignment(taBase* tab,MemberDef* md){
  if(taivMisc::record_script != NULL)  {
    *taivMisc::record_script << tab->GetPath() << "." << md->name << " = " <<
      md->type->GetValStr(md->GetOff(tab)) << ";" << endl;
  }
}
// Script Record Inline Assignment 
void taivMisc::SRIAssignment(taBase* tab,MemberDef* md){
  if(taivMisc::record_script != NULL)  {
    *taivMisc::record_script << tab->GetPath() << "." << md->name << " = \"" <<
      md->type->GetValStr(md->GetOff(tab)) << "\";\n";
    *taivMisc::record_script << tab->GetPath() << "." << "UpdateAfterEdit();" << endl;
  }
}

// Script Record Enum Assignment
void taivMisc::SREAssignment(taBase* tab,MemberDef* md){
  if(taivMisc::record_script != NULL)  {
    *taivMisc::record_script << tab->GetPath() << "." << md->name << " = " <<
      tab->GetTypeDef()->name << "::" <<
      md->type->GetValStr(md->GetOff(tab)) << ";" << endl;
  }
}


/////////////////////////////
// 	taivMethButton    //
/////////////////////////////

taivMethButton::taivMethButton(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg, taivData* par)
: taivMethMenu(bs,md,td,dlg,par)
{
  is_menu_item = false;
  rep =taivM->wkit->push_button
    (meth->GetLabel(),
     new ActionCallback(taivMethMenu)(this, &taivMethMenu::CallFun));
  ivResource::ref(rep);
}

taivMethButton::~taivMethButton() {
  ivResource::unref(rep);
}
ivGlyph* taivMethButton::GetLook() {
  return taivM->layout->margin
    (taivM->small_flex_button(((ivButton *) rep)), taivM->hsep_c);
}	


//////////////////////////////
// 	taivMethToggle     //
//////////////////////////////

taivMethToggle::taivMethToggle(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg, taivData* par)
: taivMethMenu(bs, md, td, dlg, par)
{
  tog = new taivToggle;
  tog->GetImage(true);
  rep = taivM->layout->hbox (taivM->layout->vcenter(tog->GetRep()),
			      taivM->layout->hglue(5,5,5),
			      taivM->layout->vcenter(taivM->wkit->fancy_label(md->name)));
  ivResource::ref(rep);
}

taivMethToggle::~taivMethToggle() {
  delete tog;
  ivResource::unref(rep);
}

ivGlyph* taivMethToggle::GetLook() {
  return rep;
}	

void taivMethToggle::CallFun() {
  if(tog->GetValue())
    taivMethMenu::CallFun();
}
  
