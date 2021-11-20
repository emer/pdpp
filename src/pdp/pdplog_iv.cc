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

// stuff to implement pdplog_view graphics

#include <pdp/pdplog.h>
#include <pdp/sched_proc.h>
#include <ta_misc/datatable_iv.h>
#include <ta_misc/datagraph.h>
#include <iv_misc/scrollable.h>
#include <iv_misc/hungrybox.h>
#include <iv_misc/vcrbitmaps.h>
#include <iv_misc/dastepper.h>
#include <time.h>		// for animate

#include <ta/enter_iv.h>
#include <OS/math.h>
#include <IV-look/button.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/patch.h>
#include <InterViews/font.h>
#include <InterViews/bitmap.h>
#include <InterViews/stencil.h>
#include <InterViews/background.h>
#include <InterViews/action.h>
#include <InterViews/session.h>
#ifndef CYGWIN
#include <IV-X11/xwindow.h>		// this is for the window title hack
#endif
#include <ta/leave_iv.h>

//////////////////////////////////
//	LogView IV Stuff	//
//////////////////////////////////

void LogView::Initialize() {
  view_bufsz = 100;
  view_shift = .2f;
  view_range.min = 0;
  view_range.max = -1;
  viewspec = NULL;
  log_mgr = NULL;

  disp_tog_but = NULL;
}

void LogView::CutLinks() {
  log_mgr = NULL;		// prevent from cutting updater links when killed
  taBase::DelPointer((TAPtr*)&viewspec);
  PDPView::CutLinks();
}

void LogView::InitLinks() {
  log_mgr = GET_MY_OWNER(PDPLog);
  taBase::Own(view_range, this);
  taBase::Own(viewspecs,this);

  if(log_mgr == NULL) {
    body = wkit->label("This view does not work unless it is a view of a Log");
    return;
  }

  log_mgr->SyncLogViewUpdaters(); // make sure we're all together here..
  CreateViewSpec();
  viewspec->BuildFromDataTable(&(log_mgr->data));
  PDPView::InitLinks();
}

void LogView::Copy_(const LogView& cp) {
  view_bufsz = cp.view_bufsz;
  view_shift = cp.view_shift;
  view_range = cp.view_range;
  viewspecs = cp.viewspecs;
}

void LogView::UpdateAfterEdit() {
  PDPView::UpdateAfterEdit();
  if(!taMisc::iv_active) return;
  SetToggle(display_toggle);
  InitDisplay();
}

void LogView::SetWinName() {
  if(log_mgr == NULL)    return;
  if(!taMisc::iv_active) return;
  if(window == NULL)    return;
  if(name.contains(GetTypeDef()->name) || name.contains(mgr->name)) {
    int idx = mgr->views.FindLeaf(this);
    name = mgr->name + "_" + String(idx);
  }
  String nw_name = String(ivSession::instance()->classname()) + ": " + mgr->GetPath() +
    "(" + mgr->name + ")" + " Vw: " + name.after("_");
  if(log_mgr->log_file.IsOpen() && !log_mgr->log_file.fname.empty()) {
    nw_name += String(" Fn:") + log_mgr->log_file.fname;
    file_name = log_mgr->log_file.fname;
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

TypeDef* LogView::DT_ViewSpecType() {
  return &TA_DT_ViewSpec;
}

TypeDef* LogView::DA_ViewSpecType() {
  return &TA_DA_ViewSpec;
}

void LogView::CreateViewSpec() {
  if(viewspecs.gp.size == 0) {
    viewspecs.NewGp(1, DT_ViewSpecType());
    taBase::DelPointer((TAPtr*)&viewspec); // this must be old if it exists
  }
  if(viewspec == NULL) {
    taBase::SetPointer((TAPtr*)&viewspec, viewspecs.gp.DefaultEl());
    viewspec->SetBaseType(DA_ViewSpecType());
  }
}

void LogView::UpdateDispLabels() {
  if((log_mgr == NULL) || (viewspec == NULL)) return;
  // restore the display names
  int mxsz = MIN(viewspec->leaves, log_mgr->display_labels.size);
  int i;
  for(i=0;i<mxsz;i++) {
    DA_ViewSpec* dvs = (DA_ViewSpec*)viewspec->Leaf(i);
    String dnm = log_mgr->display_labels[i];
    if(!dnm.empty())
      dvs->display_name = dnm;
  }
}

void LogView::NewHead() {
  if(log_mgr && (viewspec != NULL)) {
    viewspec->BuildFromDataTable();
    viewspec->ReBuildFromDataTable();	// make sure it rebuilds for sure!!
  }
  //  UpdateDispLabels();
}

void LogView::Clear() {
  if(log_mgr == NULL)	return;
  log_mgr->Clear();
  Clear_impl();
}

void LogView::Clear_impl(){};

void LogView::AddNotify(TAPtr ud) {
  if(ud->InheritsFrom(TA_SchedProcess) && (log_mgr != NULL)) {
    log_mgr->AddUpdater((SchedProcess*)ud);
  }
}

void LogView::RemoveNotify(TAPtr ud) {
  if(ud->InheritsFrom(TA_SchedProcess) && (log_mgr != NULL)) {
    log_mgr->RemoveUpdater((SchedProcess*)ud);
  }
}

void LogView::SetToggle(bool value){
  display_toggle = value;
  if(disp_tog_but) {
    disp_tog_but->state()->set(ivTelltaleState::is_chosen,value);
    if(window != NULL) window->repair();
  }
}

void LogView::ToggleDisplay() {
  display_toggle = !display_toggle;
  if(disp_tog_but != NULL) {
    disp_tog_but->state()->set(ivTelltaleState::is_chosen,display_toggle);
    if(window != NULL) window->repair();
  }
}

void LogView::View_F() {
  if(!taMisc::iv_active) return;
  int shft = (int)((float)view_bufsz * view_shift);
  if(shft < 1) shft = 1;
  view_range.max += shft;
  int log_max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.MaxLT(log_max); // keep it less than max
  view_range.min = view_range.max - view_bufsz +1; // always keep min and max valid
  view_range.MinGT(0);
  if(view_range.max > log_mgr->data_range.max) {
    log_mgr->Buffer_F();
  }
  view_range.MaxLT(log_mgr->data_range.max); // redo now that data_range.max is final
  view_range.min = view_range.max - view_bufsz +1;
  view_range.MinGT(log_mgr->data_range.min);
  UpdateFromBuffer();
}

void LogView::View_FSF() {
  if(!taMisc::iv_active) return;
  view_range.max += view_bufsz;
  int log_max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.MaxLT(log_max); // keep it less than max
  view_range.min = view_range.max - view_bufsz +1; // always keep min and max valid
  view_range.MinGT(0);
  if(view_range.max > log_mgr->data_range.max) {
    log_mgr->Buffer_F();
  }
  view_range.MaxLT(log_mgr->data_range.max); // redo now that data_range.max is final
  view_range.min = view_range.max - view_bufsz +1;
  view_range.MinGT(log_mgr->data_range.min);
  UpdateFromBuffer();
}

void LogView::View_FF() {
  if(!taMisc::iv_active) return;
  view_range.max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.min = view_range.max - view_bufsz +1; // always keep min and max valid
  view_range.MinGT(0);
  if(view_range.max > log_mgr->data_range.max) {
    log_mgr->Buffer_FF();
  }
  view_range.MaxLT(log_mgr->data_range.max); // redo now that data_range.max is final
  view_range.min = view_range.max - view_bufsz +1;
  view_range.MinGT(log_mgr->data_range.min);
  UpdateFromBuffer();
}

void LogView::View_R() {
  if(!taMisc::iv_active) return;
  int shft = (int)((float)view_bufsz * view_shift);
  if(shft < 1) shft = 1;
  view_range.min -= shft;
  view_range.MinGT(0);
  view_range.max = view_range.min + view_bufsz -1; // always keep min and max valid
  int log_max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.MaxLT(log_max); // keep it less than max
  if(view_range.min < log_mgr->data_range.min) {
    log_mgr->Buffer_R();
  }
  view_range.MinGT(log_mgr->data_range.min); // redo now that data_range.min is final
  view_range.max = view_range.min + view_bufsz -1;
  view_range.MaxLT(log_mgr->data_range.max);
  UpdateFromBuffer();
}

void LogView::View_FSR() {
  if(!taMisc::iv_active) return;
  view_range.min -= view_bufsz;
  view_range.MinGT(0);
  view_range.max = view_range.min + view_bufsz -1; // always keep min and max valid
  int log_max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.MaxLT(log_max); // keep it less than max
  if(view_range.min < log_mgr->data_range.min) {
    log_mgr->Buffer_R();
  }
  view_range.MinGT(log_mgr->data_range.min); // redo now that data_range.min is final
  view_range.max = view_range.min + view_bufsz -1;
  view_range.MaxLT(log_mgr->data_range.max);
  UpdateFromBuffer();
}

void LogView::View_FR() {
  if(!taMisc::iv_active) return;
  view_range.min = 0;
  view_range.max = view_range.min + view_bufsz -1; // always keep min and max valid
  int log_max = MAX(log_mgr->log_lines-1, log_mgr->data_range.max);
  view_range.MaxLT(log_max); // keep it less than max
  if(view_range.min < log_mgr->data_range.min) {
    log_mgr->Buffer_FR();
  }
  view_range.MinGT(log_mgr->data_range.min); // redo now that data_range.min is final
  view_range.max = view_range.min + view_bufsz -1;
  view_range.MaxLT(log_mgr->data_range.max);
  UpdateFromBuffer();
}

void LogView::CloseWindow() {  
  if(!taMisc::iv_active || (window == NULL)) return;
  PDPView::CloseWindow();
}

void LogView::InitDisplay(){
  // save the display names
  int mxsz = MIN(viewspec->leaves, log_mgr->display_labels.size);
  int i;
  for(i=0;i<mxsz;i++) {
    DA_ViewSpec* dvs = (DA_ViewSpec*)viewspec->Leaf(i);
    if(dvs->display_name != DA_ViewSpec::CleanName(dvs->name)) {
      log_mgr->display_labels[i] = dvs->display_name;
    }
    else {
      log_mgr->display_labels[i] = "";
    }
  }
}

void LogView::UpdateDisplay(TAPtr){
  PDPView::UpdateDisplay();
}

void LogView::EditViewSpec(taBase* spec){
  if(spec==NULL) return;
  spec->Edit();
}

void LogView::SetVisibility(taBase* spec, bool vis) {
  if(spec==NULL) return;
  if(spec->InheritsFrom(&TA_DA_ViewSpec)) {
    DA_ViewSpec* davs = (DA_ViewSpec*)spec;
    davs->visible = vis;
    davs->UpdateAfterEdit();
    davs->UpdateView();
  }
  else if(spec->InheritsFrom(&TA_DT_ViewSpec)) {
    DT_ViewSpec* dtvs = (DT_ViewSpec*)spec;
    dtvs->SetVisibility(vis);
  }
}

void LogView::SetLogging(taBase* spec, bool log_data, bool also_chg_vis) {
  if(spec==NULL) return;
  if(spec->InheritsFrom(&TA_DA_ViewSpec)) {
    DA_ViewSpec* davs = (DA_ViewSpec*)spec;
    if(davs->data_array == NULL) return;
    davs->data_array->save_to_file = log_data;
    if(also_chg_vis) {
      davs->visible = log_data;
      davs->UpdateAfterEdit();
      davs->UpdateView();
    }
  }
  else if(spec->InheritsFrom(&TA_DT_ViewSpec)) {
    DT_ViewSpec* dtvs = (DT_ViewSpec*)spec;
    if(dtvs->data_table == NULL) return;
    dtvs->data_table->SetSaveToFile(log_data);
    if(also_chg_vis)
      dtvs->SetVisibility(log_data);
  }
}

ivGlyph* LogView::GetPrintData() {
  return win_box;
}

#include <ta/enter_iv.h>
declareActionCallback(LogView)
implementActionCallback(LogView)
#include <ta/leave_iv.h>

void LogView::CreateLogButtons() {
  ivBitmap* fr_bm = new ivBitmap((void *) fr_bits,fr_width,fr_height);
  ivBitmap* fsr_bm = new ivBitmap((void *) fsr_bits,fsr_width,fsr_height);
  ivBitmap* r_bm = new ivBitmap((void *) r_bits,r_width,r_height);
  ivBitmap* f_bm = new ivBitmap((void *) f_bits,f_width,f_height);
  ivBitmap* fsf_bm = new ivBitmap((void *) fsf_bits,fsf_width,fsf_height);
  ivBitmap* ff_bm = new ivBitmap((void *) ff_bits,ff_width,ff_height);

  frev_but = wkit->push_button(new ivStencil(fr_bm,wkit->foreground()),
				 new ActionCallback(LogView)
				 (this,&LogView::View_FR));
  ivTelltaleState* fsrev_state = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  fsrev_but = new DAStepper
    (wkit->push_button_look
     (new ivStencil(fsr_bm,wkit->foreground()),fsrev_state),
     wkit->style(),fsrev_state,
     new ActionCallback(LogView)
     (this,&LogView::View_FSR));

  ivTelltaleState* rev_state = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  rev_but = new DAStepper
    (wkit->push_button_look
     (new ivStencil(r_bm,wkit->foreground()),rev_state),
     wkit->style(),rev_state,
     new ActionCallback(LogView)
     (this,&LogView::View_R));

  ivTelltaleState* fwd_state = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  fwd_but = new DAStepper
    (wkit->push_button_look
     (new ivStencil(f_bm,wkit->foreground()),fwd_state),
     wkit->style(),fwd_state,
     new ActionCallback(LogView)
     (this,&LogView::View_F));

  ivTelltaleState* fsfwd_state = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  fsfwd_but = new DAStepper
    (wkit->push_button_look
     (new ivStencil(fsf_bm,wkit->foreground()),fsfwd_state),
     wkit->style(),fsfwd_state,
     new ActionCallback(LogView)
     (this,&LogView::View_FSF));

  ffwd_but = wkit->push_button(new ivStencil(ff_bm,wkit->foreground()),
				 new ActionCallback(LogView)
				 (this,&LogView::View_FF));

  disp_tog_but = wkit->check_box("Display",
			  new ActionCallback(LogView)
			  (this,&LogView::ToggleDisplay));

  disp_tog_but->state()->set(ivTelltaleState::is_chosen,display_toggle);

  clear_but = 
    wkit->push_button("Clear",
		      new ActionCallback(LogView)
		      (this,&LogView::Clear));


  init_but = 
    wkit->push_button("Init",
		      new ActionCallback(LogView)
		      (this,&LogView::InitDisplay));

  update_but = 
    wkit->push_button("Update",
		      new ActionCallback(LogView)
		      (this,&LogView::UpdateCallback));
}

ivGlyph* LogView::GetLogButtons() {
  CreateLogButtons();
  ivGlyph* rval = layout->hbox(layout->vcenter(disp_tog_but),
			       layout->hglue(),
			       layout->vcenter
			       (layout->hbox
				(layout->hfixed(frev_but,VCR_BUTTON_WIDTH),
				 layout->hfixed(fsrev_but,VCR_BUTTON_WIDTH),
				 layout->hfixed(rev_but,VCR_BUTTON_WIDTH),
				 layout->hfixed(fwd_but,VCR_BUTTON_WIDTH),
				 layout->hfixed(fsfwd_but,VCR_BUTTON_WIDTH),
				 layout->hfixed(ffwd_but,VCR_BUTTON_WIDTH))),
			       layout->hglue(),
			       layout->vcenter(taivM->small_button(update_but)),
			       layout->vcenter(taivM->small_button(init_but)),
			       layout->vcenter(taivM->small_button(clear_but)));
  return rval;
}

//////////////////////////
// 	TextLogView	//
//////////////////////////
 
void TextLogView::Initialize() {
  view_bufsz = 10;
  view_shift = .2f;

  sb = NULL;
  h_line= NULL;
  patch = NULL;
  scrb = NULL;
  vbox = NULL;
  ovbox = NULL;
  cur_line = NULL;
}

void TextLogView::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  ivResource::unref(body);	body = NULL;
  ivResource::unref(sb);	sb = NULL;
  ivResource::unref(h_line);	h_line = NULL;
  ivResource::unref(patch);	patch = NULL;
  ivResource::unref(scrb);	scrb = NULL;
  ivResource::unref(vbox);	vbox = NULL;
  ivResource::unref(ovbox);	ovbox = NULL;
  cur_line = NULL;
  LogView::CloseWindow();
}

void TextLogView::UpdateAfterEdit() {
  LogView::UpdateAfterEdit();
  UpdateDisplay();
}

TypeDef* TextLogView::DT_ViewSpecType() {
  return &TA_DT_ViewSpec;
}

TypeDef* TextLogView::DA_ViewSpecType() {
  return &TA_DA_TextViewSpec;
}

int TLVMore(int* tlv, int newsize, HungryBox*) {
 return ((TextLogView*) tlv)->ShowMore(newsize);
}


int TLVLess(int* tlv, int newsize, HungryBox*) {
  return ((TextLogView*) tlv)->ShowLess(newsize);
}

int TextLogView::ShowLess(int newsize) {
  if((newsize <= 0) || (newsize == view_bufsz)) return 1;
  view_bufsz = MAX(newsize, 1);	// never show zero!
  view_range.max = view_range.min + view_bufsz -1;
  view_range.MaxLT(log_mgr->data_range.max); // double check
  view_range.min = view_range.max - view_bufsz +1;
  view_range.MinGT(log_mgr->data_range.min);
  // this crashes because its called within a draw..
  //  FixSize();
  return 0;
}

int TextLogView::ShowMore(int newsize) {
  if((newsize <= 0) || (newsize == view_bufsz)) return 1;
  int oldsize = (view_range.max - view_range.min +1);
  view_bufsz = MAX(newsize, 1);
  view_range.max = view_range.min + view_bufsz -1;
  view_range.MaxLT(log_mgr->data_range.max); // double check
  view_range.min = view_range.max - view_bufsz +1;
  view_range.MinGT(log_mgr->data_range.min);
  if(oldsize != (view_range.max - view_range.min + 1)) {
    // this crashes because its called within a draw..
    //    FixSize();
    //    return 0;
    return 0;
  }
  else {
    return 1;
  }
}

void TextLogView::GetBodyRep() {
  if(!taMisc::iv_active) return;
  if(body != NULL) return;

  ivFontBoundingBox fbb;
  wkit->font()->font_bbox(fbb);
  font_width = fbb.width();
#ifdef CYGWIN
  font_width *= .4f;		// too wide!
#endif
  float font_height = fbb.font_ascent() + fbb.font_descent();

  sb = new TBHungryBox((int*) this,&TLVMore,&TLVLess,(ivGlyphIndex) view_bufsz);
  ivResource::ref(sb);

  wkit->push_style(taivM->title_style);
  h_line = wkit->label("-----------------------No-Current-Header------------------------");
  ivResource::ref(h_line);
  wkit->pop_style();

  vbox = layout->vbox
    (layout->hflexible
     (layout->hbox(layout->hglue(),h_line,layout->hglue())),
     layout->hflexible(wkit->menu_item_separator_look()),
     layout->vnatural(sb,font_height * 4.0));
  ivResource::ref(vbox);

  scrb = (ivPatch *) new HScrollable
    (layout->vcenter
     (vbox,0),0,0,0,0);
  ivResource::ref(scrb);

  patch = new ivPatch(wkit->inset_frame
		      (layout->hmargin(scrb,5)));
  ivResource::ref(patch);

  ivGlyph* log_buttons = GetLogButtons();

  ovbox = layout->vbox
    (layout->hcenter(layout->hflexible(log_buttons,fil,0),0),
     layout->hcenter(layout->margin(patch,5),0),
     layout->hcenter(wkit->hscroll_bar((Scrollable *) scrb),0));
  ivResource::ref(ovbox);

  const ivColor* bg_color = log_mgr->GetEditColor();
  if(bg_color == NULL) bg_color = wkit->background();

  body = new ivBackground(ovbox, bg_color);
  ivResource::ref(body);
}

ivGlyph* TextLogView::GetPrintData(){
  if(vbox == NULL) return LogView::GetPrintData();
  return patch;
}

ivCoord TextLogView::GetFieldWidth(DA_TextViewSpec* vs){
  return(vs->width * 4.0f);
}  

void TextLogView::StartHeaderLine(){
  ivResource::unref(h_line);
  h_line = layout->hbox();
  ivResource::ref(h_line);
  wkit->push_style(taivM->title_style);
}  

void TextLogView::AddToHeaderLine(const char* nm, ivCoord width){
  if(h_line == NULL)   StartHeaderLine();
  h_line->append(layout->hnatural(wkit->fancy_label(nm),font_width * width));
}

void TextLogView::EndHeaderLine(){
  wkit->pop_style();
  vbox->replace
    (0,layout->hflexible
     (layout->hbox(h_line,layout->hglue())));
  ivResource::flush();

  // notify the boxes
  vbox->modified(0);
}

void TextLogView::AddToLine(const char* nm, ivCoord width){
  if(cur_line == NULL)   cur_line = layout->hbox();
  cur_line->append(layout->hnatural(wkit->label(nm), font_width * width));
}

void TextLogView::AddLine(){
  if(cur_line != NULL)  sb->append(cur_line);
  cur_line = NULL;
  vbox->modified(2);
}

void TextLogView::RemoveLine(int index){
  if(sb == NULL) return;
  if(index == -1) {
    while(sb->count()>0)  sb->remove(sb->count()-1);
  }
  else if(sb->count() > index) sb->remove(index);
  ivResource::flush();
  vbox->modified(2);
}

void TextLogView::MakeLine(int from_end){
  taLeafItr i;
  DataArray_impl* da;
  DA_TextViewSpec* vs;
  FOR_ITR_EL(DA_TextViewSpec, vs, viewspec->, i) {
    if((vs->visible == false) || (vs->data_array == NULL)) continue;
    da = vs->data_array;
    taArray_base* ar = da->AR();
    String el;
    if((ar != NULL) && (ar->size > from_end) && (from_end >= 0)) {
      el = ar->El_GetStr_(ar->FastEl_(ar->size-1-from_end));
    }
    else {
      el = "n/a";
    }
    AddToLine(el.chars(), GetFieldWidth(vs));
  }
}

void TextLogView::InitDisplay() {
  if(!taMisc::iv_active) return;
  if(body == NULL) return;
  LogView::InitDisplay();
  StartHeaderLine();
  taLeafItr i;
  DA_TextViewSpec* vs;
  FOR_ITR_EL(DA_TextViewSpec, vs, viewspec->, i) {
    if((vs->visible == false) || (vs->data_array == NULL)) continue;
    AddToHeaderLine(vs->display_name.chars(),GetFieldWidth(vs));
  }
  EndHeaderLine();
  UpdateFromBuffer();
  UpdateDisplay();
}

void TextLogView::UpdateDisplay(TAPtr) {
  if(!taMisc::iv_active) return;
  if(patch == NULL) return;
  patch->reallocate();
  patch->redraw();
  if(anim.capture) CaptureAnimImg();
}

void TextLogView::UpdateFromBuffer() {
  FixSize();
  if(patch == NULL) return;
  vbox->modified(0);
  ovbox->modified(1);
  main_box->modified(1);
  scrb->reallocate();
  patch->reallocate();
  scrb->redraw();
  patch->redraw();
}

void TextLogView::Reset() {
  if(!taMisc::iv_active) return;
  RemoveLine(-1);
}

void TextLogView::Clear_impl() {
  if(!taMisc::iv_active ) return;
  Reset();
  view_range.max = -1;
  view_range.min = 0; 
  InitDisplay();
}


void TextLogView::NewHead() {
  LogView::NewHead();
  if(!IsMapped() || !display_toggle) return;
  InitDisplay();
}

void TextLogView::NewData() {
  if(!IsMapped() || !display_toggle) return;

  if((view_range.max < log_mgr->data_range.max-1) && (view_range.max > 0))
    return;			// i'm not viewing the full file here..
  MakeLine();
  AddLine();
  view_range.max++;
  while((view_range.max - view_range.min + 1) > view_bufsz) {
    view_range.min++;
    RemoveLine(0);
  }
  UpdateDisplay();
}

void TextLogView::FixSize(){
  // this updates the data area
  if(!taMisc::iv_active || !display_toggle) return;

  view_range.WithinRange(log_mgr->data_range);
  Reset();

  // this is the index (in view_range units) of the last line of data in the buffer
  int data_offset = log_mgr->data.MaxLength() + log_mgr->data_range.min - 1;

  int ln;
  for(ln = view_range.min; ln <= view_range.max; ln++) {
    int from_end = data_offset - ln ;
    MakeLine(from_end);
    AddLine();
  }
}

//////////////////////////
//  NetLogView 		//
//////////////////////////

void NetLogView::Initialize(){
  view_bufsz = 1;
  view_shift = 1;
  network = NULL;
}

void NetLogView::InitLinks() {
  LogView::InitLinks();
  if(updaters.size == 0) return;
  SchedProcess* proc = (SchedProcess*)updaters.FastEl(0);
  if(proc->network != NULL)
    SetNetwork(proc->network);	// use process network by default
}

// NetLogView::CutLinks() is in pdpshell.cc

TypeDef* NetLogView::DA_ViewSpecType() {
  return &TA_DA_NetViewSpec;
}

void NetLogView::UpdateAfterEdit(){
  LogView::UpdateAfterEdit();
  UpdateDisplay();
}

void NetLogView::SetNetwork(Network* net) {
  taBase::SetPointer((TAPtr*)&network, net);
  ArrangeLabels();
}

void NetLogView::NewHead(){
  LogView::NewHead();
  if(!display_toggle) return;
  InitDisplay();
}

void NetLogView::NewData(){
  if(!taMisc::iv_active || !display_toggle) return;
  UpdateDisplay();
}

void NetLogView::InitDisplay() {
  LogView::InitDisplay();
  if(network == NULL) {
    if(updaters.size == 0) return;
    SchedProcess* proc = (SchedProcess*)updaters.FastEl(0);
    if(proc->network != NULL)
      SetNetwork(proc->network);	// use process network by default
  }
  else
    UpdateDisplay();
}

void NetLogView::ArrangeLabels(int cols, int rows, int width, float left, float top) {
  if(!taMisc::iv_active || (network == NULL)) return;
  UpdateDisplay();

  top -= .07f;			// mystery offset...

  int sz = viewspec->leaves;
  if(cols < 0 && rows > 0) {
    cols = sz / rows;
    while((cols * rows) < sz) cols++;
  }
  else if(cols > 0 && rows < 0) {
    rows = sz / cols;
    while((cols * rows) < sz) rows++;
  }
  else if(cols < 0 && rows < 0) {
    cols = (int)sqrtf((float)sz);
    rows = sz / cols;
    while((cols * rows) < sz) cols++;
  }

  NetView* netview;
  taLeafItr j;
  FOR_ITR_EL(NetView, netview, network->views., j) {
    if(netview->display_toggle == false) continue;
    if(netview->labels.size < 1) continue;

    NetViewLabel* vl = (NetViewLabel*) netview->labels.FastEl(0); // take first one as representative
    ivFontBoundingBox bbox;
    vl->spec.fnt->font_bbox(bbox);
    float fy = bbox.ascent() + bbox.descent();
    float fx = vl->spec.fnt->width('m');

    float xscale = (float)width * fx;
    float yscale = 1.25f * fy;
    float xoff = left * vl->viewer->viewallocation().x_allotment().span();
    float yoff = top * vl->viewer->viewallocation().y_allotment().span();

    int curx = 0;
    int cury = 0;

    taLeafItr i;
    DA_NetViewSpec* vs;
    FOR_ITR_EL(DA_NetViewSpec, vs, viewspec->, i) {
      if((vs->visible == false) || (vs->data_array == NULL)) continue;
      NetViewLabel* vl = (NetViewLabel*) netview->labels.FastEl(vs->label_index);
      Xform* xf = new Xform;
      float xc = xoff + ((float)curx * xscale);
      float yc = yoff - ((float)cury * yscale);
      xf->Set(1.0f, 0.0f, 0.0f, 1.0f, xc, yc);
      vl->SetLabelXform(xf);
      curx++;
      if(curx >= cols) {
	curx = 0;
	cury++;
      }
    }
    netview->InitDisplay();
  }  
}

void NetLogView::UpdateDisplay(TAPtr) {
  if(!taMisc::iv_active || (network == NULL)) return;
  NetView* netview;
  taLeafItr j;
  FOR_ITR_EL(NetView, netview, network->views., j) {
    if(netview->display_toggle == false) continue;
    bool newlabel = false;
    taLeafItr i;
    DA_NetViewSpec* vs;
    FOR_ITR_EL(DA_NetViewSpec, vs, viewspec->, i) {
      if((vs->visible == false) || (vs->data_array == NULL)) continue;
      DataArray_impl* da = vs->data_array;
      NetViewLabel* vl = NULL;
      if(vs->label_index < 0) {	// not there, try to find existing first..
	String srch = vs->display_name + ":";
	int k;
	for(k=0;k<netview->labels.size;k++) {
	  NetViewLabel* tvl = (NetViewLabel*)netview->labels.FastEl(k);
	  if(tvl->name.contains(srch)) {
	    // now need to make sure its unique..
	    bool unique = true;
	    DA_NetViewSpec* tvs;
	    taLeafItr ti;
	    FOR_ITR_EL(DA_NetViewSpec, tvs, viewspec->, ti) {
	      if(tvs->label_index == k) {
		unique = false;
		break;
	      }
	    }
	    if(unique) {
	      vl = tvl;
	      vs->label_index = k;
	      break;
	    }
	  }
	}
	if(vl == NULL) { // didn't find it
	  vl = (NetViewLabel*)netview->labels.New(1,&TA_NetViewLabel);
	  vs->label_index = netview->labels.size -1;
	  newlabel = true;
	}
      }
      else if(vs->label_index >= netview->labels.size) {
	vl = (NetViewLabel*)netview->labels.New(1,&TA_NetViewLabel);
	vs->label_index = netview->labels.size -1;
	newlabel = true;
      }
      else {
	vl = (NetViewLabel*) netview->labels.FastEl(vs->label_index);
      }
      
      vl->name = vs->display_name + ":";
      taArray_base* ar = da->AR();
      String el;
      if((ar != NULL) && (ar->size > 0)){
	el = ar->El_GetStr_(ar->FastEl_(ar->size-1));
      }
      else {
	el = "n/a";
      }
      vl->name += el;
      vl->label_gp = 1;
      vl->UpdateAfterEdit(); //    vl->MakeText();
    }
    if(newlabel)
      netview->InitDisplay();
    else
      netview->UpdateDisplay();
  }
}

void NetLogView::RemoveLabels() {
  if(!taMisc::iv_active || (network == NULL)) return;
  NetView* netview;
  taLeafItr j;
  FOR_ITR_EL(NetView, netview, network->views., j) {
    if(netview->labels.size < 1) continue;
    int i;
    for(i=viewspec->leaves-1;i>=0;i--) {
      DA_NetViewSpec* vs = (DA_NetViewSpec*)viewspec->Leaf(i);
      netview->labels.Remove(vs->label_index);
    }
    netview->InitDisplay();
  }  
}

void NetLogView::UpdateFromBuffer(){
  UpdateAfterEdit();
}


//////////////////////////
//    GridLogView	//
//////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(GridLogView)
implementActionCallback(GridLogView)
#include <ta/leave_iv.h>


// The GLV_Action class is given to the color bar in the
// grid view so that when the user changes the values in
// the colorbar by typing a number or twiddling the steppers,
// the grid view's scale range is set to fixed.

class GLV_Action : public ivAction {
public:
  GridLogView* glv;
  void execute();
  GLV_Action(GridLogView* g) {glv = g;}
  ~GLV_Action() {};
};

void GLV_Action::execute(){
  glv->auto_scale = false;
  glv->scale_range.min = glv->editor->cbar->min;
  glv->scale_range.max = glv->editor->cbar->max;
  glv->editor->auto_scale = glv->auto_scale;
  glv->editor->scale_range = glv->scale_range;
  if(glv->auto_sc_but != NULL) {
    glv->auto_sc_but->state()->set(ivTelltaleState::is_chosen,glv->auto_scale);
    if(glv->window != NULL) glv->window->repair();
  }
  glv->InitDisplay();
}

void GridLogView::Initialize() {
  fill_type = DT_GridViewSpec::COLOR;
  block_size = 8;
  block_border_size = 1;
  header_on = true;
  auto_scale = false;
  scale_range.min = -1.0f;
  scale_range.max = 1.0f;
  view_bufsz = 3;
  view_shift = .2f;
  colorspec = NULL;

  editor = NULL;
  head_tog_but = NULL;
  auto_sc_but = NULL;
  adjuster = NULL;
}

void GridLogView::InitLinks(){
  log_mgr = GET_MY_OWNER(PDPLog);
  if(log_mgr == NULL) {
    LogView::InitLinks();
    return;
  }

  CreateViewSpec();
  // set top level view spec to not use gp_name
  ((DT_GridViewSpec *) viewspec)->use_gp_name = false;

  taBase::Own(view_font,this);
  taBase::Own(scale_range,this);
  taBase::Own(actual_range,this);
  LogView::InitLinks(); 
}

void GridLogView::CutLinks() {
  view_font.CutLinks();
  taBase::DelPointer((TAPtr*)&colorspec);
  LogView::CutLinks();
  if(editor != NULL) { delete editor; editor = NULL; }
  if(adjuster != NULL) { ivResource::unref(adjuster); adjuster = NULL; }
}

void GridLogView::Destroy() {
  CutLinks();
}

void GridLogView::Copy_(const GridLogView& cp) {
  fill_type = cp.fill_type;
  block_size = cp.block_size;
  block_border_size = cp.block_border_size;
  header_on = cp.header_on;
  auto_scale = cp.auto_scale;
  scale_range = cp.scale_range;
  taBase::SetPointer((TAPtr*)&colorspec, cp.colorspec);
  view_font = cp.view_font;
  actual_range = cp.actual_range;
}

void GridLogView::UpdateAfterEdit(){
  LogView::UpdateAfterEdit();
  if(!taMisc::iv_active) return;
  if(body == NULL) return; // loading

  bool init = false;
  if((colorspec != NULL) && (editor->scale->spec != colorspec)) {
    taBase::SetPointer((TAPtr*)&(editor->scale->spec), colorspec);
    editor->scale->MapColors();
    editor->dtg->background(editor->scale->Get_Background());
    init = true;
  }
  if(editor->fill_type != fill_type) {
    editor->fill_type = fill_type;    init = true;
  }
  if(editor->block_size != block_size) {
    editor->block_size = block_size;    init = true;
  }
  if(editor->block_border_size != block_border_size) {
    editor->block_border_size = block_border_size;    init = true;
  }
  if(editor->header_on != header_on) {
    editor->header_on = header_on;    init = true;
  }
  if(editor->auto_scale != auto_scale) {
    editor->auto_scale = auto_scale;    init = true;
  }
  if(editor->view_font.pattern != view_font.pattern) {
    editor->view_font = view_font;    init = true;
  }
  if(editor->scale_range != scale_range) {
    editor->scale_range = scale_range;    init = true;
  }

  if(init) InitDisplay();
  else UpdateDisplay();
}

void GridLogView::CloseWindow() {
  ivResource::unref(body);	body = NULL;
  LogView::CloseWindow();
}

TypeDef* GridLogView::DT_ViewSpecType() {
  return &TA_DT_GridViewSpec;
}

TypeDef* GridLogView::DA_ViewSpecType() {
  return &TA_DA_GridViewSpec;
}

void GridLogView::NewHead() {
  LogView::NewHead();
  if(!IsMapped() || !display_toggle) return;
//    if((editor != NULL) && (editor->dtvsp != NULL))
//      editor->dtvsp->UpdateLayout();
  InitDisplay();
}

void GridLogView::NewData() {
  if(!IsMapped() || !display_toggle ) return;
  if((view_range.max < log_mgr->data_range.max-1) && (view_range.max > 0))
    return;			// not viewing the whole range, don't update
  view_range.max++;
  view_range.MaxLT(log_mgr->data_range.max); // double check
  view_range.MinGT(view_range.max - view_bufsz + 1); 
  view_range.WithinRange(log_mgr->data_range); // triple check

  actual_range.min = view_range.min - log_mgr->data_range.min;
  actual_range.max = view_range.max - log_mgr->data_range.min;
  editor->view_range = actual_range;
  editor->AddOneLine();
  view_bufsz = editor->disp_lines;
  actual_range = editor->view_range;
  view_range.min = actual_range.min + log_mgr->data_range.min;
  view_range.max = actual_range.max + log_mgr->data_range.min;
}

void GridLogView::InitDisplay(){
  if(!taMisc::iv_active) return;
  if(body == NULL) return; // loading
  LogView::InitDisplay();
  actual_range.min = view_range.min - log_mgr->data_range.min;
  actual_range.max = view_range.max - log_mgr->data_range.min;
  editor->view_range = actual_range;
  editor->InitDisplay();
  view_bufsz = editor->disp_lines;
  actual_range = editor->view_range;
  view_range.min = actual_range.min + log_mgr->data_range.min;
  view_range.max = actual_range.max + log_mgr->data_range.min;
  auto_scale = editor->auto_scale;
  scale_range = editor->scale_range;
  if(auto_sc_but != NULL) {
    auto_sc_but->state()->set(ivTelltaleState::is_chosen,auto_scale);
    if(window != NULL) window->repair();
  }
}

void GridLogView::UpdateDisplay(TAPtr){
  if(!taMisc::iv_active) return;
  editor->UpdateDisplay();
  auto_scale = editor->auto_scale;
  scale_range = editor->scale_range;
  if(auto_sc_but != NULL) {
    auto_sc_but->state()->set(ivTelltaleState::is_chosen,auto_scale);
    if(window != NULL) window->repair();
  }
  if(anim.capture) CaptureAnimImg();
}

void GridLogView::UpdateFromBuffer(){
  if(!display_toggle)
    return;
  InitDisplay();
}

void GridLogView::Clear_impl(){
  view_range.max = -1;
  view_range.min = 0; 
  InitDisplay();
}

void GridLogView::CreateLogButtons() {
  LogView::CreateLogButtons();
  delete disp_tog_but;
  disp_tog_but = wkit->check_box("Dsp",
			  new ActionCallback(GridLogView)
			  (this,&GridLogView::ToggleDisplay));
  disp_tog_but->state()->set(ivTelltaleState::is_chosen,display_toggle);

  auto_sc_but = wkit->check_box("Auto",
				 new ActionCallback(GridLogView)
				 (this,&GridLogView::ToggleAutoScale));
  auto_sc_but->state()->set(ivTelltaleState::is_chosen,auto_scale);

  head_tog_but = wkit->check_box("Hdr",
				 new ActionCallback(GridLogView)
				 (this,&GridLogView::ToggleHeader));
  head_tog_but->state()->set(ivTelltaleState::is_chosen,header_on);
}

ivGlyph* GridLogView::GetLogButtons() {
  CreateLogButtons();
  return layout->hbox(layout->vcenter(disp_tog_but),
		      layout->vcenter(auto_sc_but),
		      layout->vcenter(head_tog_but),
		      layout->hglue(),
		      layout->vcenter
		      (layout->hbox
		       (layout->hfixed(frev_but,VCR_BUTTON_WIDTH),
			layout->hfixed(fsrev_but,VCR_BUTTON_WIDTH),
			layout->hfixed(rev_but,VCR_BUTTON_WIDTH),
			layout->hfixed(fwd_but,VCR_BUTTON_WIDTH),
			layout->hfixed(fsfwd_but,VCR_BUTTON_WIDTH),
			layout->hfixed(ffwd_but,VCR_BUTTON_WIDTH))),
		      layout->hglue(),
		      layout->vcenter(taivM->small_button(update_but)),
		      layout->vcenter(taivM->small_button(init_but)),
		      layout->vcenter(taivM->small_button(clear_but)));
}

void GridLogView::GetBodyRep(){
  if(!taMisc::iv_active) return;
  if(body != NULL) return;

  editor = new DTEditor(&(log_mgr->data), (DT_GridViewSpec*)viewspec, (ivWindow*) window);
  actual_range.min = view_range.min - log_mgr->data_range.min;
  actual_range.max = view_range.max - log_mgr->data_range.min;

  editor->fill_type = fill_type;
  editor->block_size = block_size;
  editor->block_border_size = block_border_size;
  editor->header_on = header_on;
  editor->auto_scale = auto_scale;
  editor->view_font = view_font;
  editor->scale_range = scale_range;
  editor->view_range = actual_range;

  editor->Init();

  adjuster = (ivAction *) new GLV_Action(this);
  ivResource::ref(adjuster);

  editor->cbar->SetAdjustNotify(adjuster);

  const ivColor* bgclr = GetEditColorInherit();
  if(bgclr == NULL) bgclr = wkit->background();

  ivGlyph* log_buttons = new ivBackground(GetLogButtons(), bgclr);

  body = layout->vbox
	  (log_buttons,
	   layout->flexible
	   (layout->natural
	    (wkit->inset_frame(editor->GetLook()),200,150),fil,150));
  ivResource::ref(body);
  if(bgclr != NULL) {
    wkit->pop_style();
  }
}

void GridLogView::ToggleHeader() {
  header_on = !header_on;
  if(editor != NULL) editor->header_on = header_on;
  if(head_tog_but != NULL) {
    head_tog_but->state()->set(ivTelltaleState::is_chosen,header_on);
    if(window != NULL) window->repair();
    InitDisplay();
  }
}

void GridLogView::ToggleAutoScale() {
  auto_scale = !auto_scale;
  if(editor != NULL) editor->auto_scale = auto_scale;
  if(auto_sc_but != NULL) {
    auto_sc_but->state()->set(ivTelltaleState::is_chosen,auto_scale);
    if(window != NULL) window->repair();
    InitDisplay();
  }
}

ivGlyph* GridLogView::GetPrintData() {
  if((editor != NULL) && (editor->print_patch != NULL))
    return editor->print_patch;
  return LogView::GetPrintData();
}

void GridLogView::SetColorSpec(ColorScaleSpec* colors) {
  taBase::SetPointer((TAPtr*)&colorspec, colors);
  UpdateAfterEdit();
}
  
void GridLogView::SetBlockFill(DT_GridViewSpec::BlockFill b){
  fill_type = b;
  editor->fill_type = b;
  InitDisplay();
}

void GridLogView::SetBlockSizes(int block_sz, int border_sz) {
  block_size = block_sz;
  block_border_size = border_sz;
  UpdateAfterEdit();
}

void GridLogView::UpdateGridLayout(DT_GridViewSpec::MatrixLayout ml){
   editor->dtvsp->UpdateLayout(ml);
   InitDisplay(); // gets new headerbox and grids
}

void GridLogView::SetViewFontSize(int point_size) {
  view_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void GridLogView::AllBlockTextOn() {
  DT_GridViewSpec* vs = (DT_GridViewSpec*)viewspec;
  int i;
  for(i=0;i<vs->size;i++) {
    DA_GridViewSpec* da = (DA_GridViewSpec*)vs->FastEl(i);
    if(da->display_style == DA_GridViewSpec::BLOCK)
      da->display_style = DA_GridViewSpec::TEXT_AND_BLOCK;
  }
  int j;
  FOR_ITR_GP(DT_GridViewSpec, vs, viewspec->, j) {
    if(vs->use_gp_name) {
      if(vs->display_style == DA_GridViewSpec::BLOCK)
	vs->display_style = DA_GridViewSpec::TEXT_AND_BLOCK;
    }
    else {
      for(i=0;i<vs->size;i++) {
	DA_GridViewSpec* da = (DA_GridViewSpec*)vs->FastEl(i);
	if(da->display_style == DA_GridViewSpec::BLOCK)
	  da->display_style = DA_GridViewSpec::TEXT_AND_BLOCK;
      }
    }
  }
  InitDisplay();
}

void GridLogView::AllBlockTextOff() {
  DT_GridViewSpec* vs = (DT_GridViewSpec*)viewspec;
  int i;
  for(i=0;i<vs->size;i++) {
    DA_GridViewSpec* da = (DA_GridViewSpec*)vs->FastEl(i);
    if(da->display_style == DA_GridViewSpec::TEXT_AND_BLOCK)
      da->display_style = DA_GridViewSpec::BLOCK;
  }
  int j;
  FOR_ITR_GP(DT_GridViewSpec, vs, viewspec->, j) {
    if(vs->use_gp_name) {
      if(vs->display_style == DA_GridViewSpec::TEXT_AND_BLOCK)
	vs->display_style = DA_GridViewSpec::BLOCK;
    }
    else {
      for(i=0;i<vs->size;i++) {
	DA_GridViewSpec* da = (DA_GridViewSpec*)vs->FastEl(i);
	if(da->display_style == DA_GridViewSpec::TEXT_AND_BLOCK)
	  da->display_style = DA_GridViewSpec::BLOCK;
      }
    }
  }
  InitDisplay();
}

//////////////////////////
//    GraphLogView	//
//////////////////////////

void GraphLogViewLabel::GetMasterViewer() {
  GraphLogView* glv = GET_MY_OWNER(GraphLogView);
  if((glv != NULL) && (glv->editor != NULL)) {
    GraphEditor* ge = glv->editor;
    if(ge->first_graph != NULL) {
      viewer = ge->first_graph->viewer;
      master = ge->first_graph->graphg;
    }
  }
}

void GraphLogView::Initialize() {
  x_axis_index = 0;
  editor = NULL;
  labels.SetBaseType(&TA_GraphLogViewLabel); // make sure to use this type..
  view_range.max = 10000;
  view_bufsz = 10000;
  colorspec = NULL;
  separate_graphs = false;
  graph_layout.x = 1; graph_layout.y = 10;
  anim_stop = false;
}

TypeDef* GraphLogView::DT_ViewSpecType() {
  return &TA_DT_GraphViewSpec;
}

TypeDef* GraphLogView::DA_ViewSpecType() {
  return &TA_DA_GraphViewSpec;
}

void GraphLogView::InitLinks() {
  taBase::Own(labels,this);
  taBase::Own(actual_range,this);
  LogView::InitLinks();
  UpdateViewRange();
}

void GraphLogView::CutLinks() {
  labels.CutLinks();
  taBase::DelPointer((TAPtr*)&colorspec);
  LogView::CutLinks();
  if (editor) { delete editor; editor = NULL; }
}

void GraphLogView::Destroy() {
  CutLinks();
}

void GraphLogView::Copy_(const GraphLogView& cp) {
  x_axis_index = cp.x_axis_index;
  labels = cp.labels;
  actual_range = cp.actual_range;
  taBase::SetPointer((TAPtr*)&colorspec, cp.colorspec);
  separate_graphs = cp.separate_graphs;
  graph_layout = cp.graph_layout;
}

void GraphLogView::UpdateAfterEdit() {
  LogView::UpdateAfterEdit();
  labels.SetBaseType(&TA_GraphLogViewLabel); // make sure to use this type..
  if((editor == NULL) || (log_mgr == NULL)) return;
  bool init = false;
  if((colorspec != NULL) && (editor->scale->spec != colorspec)) {
    taBase::SetPointer((TAPtr*)&(editor->scale->spec), colorspec);
    editor->scale->MapColors();
    //    editor->dtg->background(editor->scale->Get_Background());
    init = true;
  }
  if(editor->separate_graphs != separate_graphs) {
    editor->separate_graphs = separate_graphs;    init = true;
  }
  if(editor->graph_layout != graph_layout) {
    editor->graph_layout = graph_layout;    init = true;
  }
  x_axis_index = MIN(log_mgr->data.size-1, x_axis_index);
  x_axis_index = MAX(0, x_axis_index);
  UpdateViewRange();
  if(editor->x_axis_index != x_axis_index) {
    editor->SetXAxis(x_axis_index);
  }
  if(init)
    InitDisplay();
  else
    UpdateDisplay();
}

void GraphLogView::UpdateViewRange() {
  actual_range.min = view_range.min - log_mgr->data_range.min;
  actual_range.max = view_range.max - log_mgr->data_range.min;
  if(editor != NULL) {
    editor->view_range = actual_range;
    editor->view_bufsz = view_bufsz;
  }
}
  
void GraphLogView::NewData() {
  if(!IsMapped() || !display_toggle || (editor == NULL)) return;
  bool not_at_last = false;
  if((view_range.max < log_mgr->data_range.max-1) && (view_range.max > 0)) {
    // not viewing most recent data point..
    not_at_last = true;
    if((log_mgr->data_range.max - view_range.min) > view_bufsz) // beyond current display -- don't view
      return;			
  }
  view_range.max++;
  view_range.MaxLT(log_mgr->data_range.max); // double check
  view_range.MinGT(view_range.max - view_bufsz + 1); 
  view_range.WithinRange(log_mgr->data_range); // triple check

  UpdateViewRange();

  actual_range.max++;
  editor->view_range = actual_range;

  if(not_at_last) {
    UpdateDisplay();
  }
  else if(actual_range.Range() >= view_bufsz) {
    int shft = (int)((float)view_bufsz * view_shift);
    if(shft < 1) shft = 1;
    view_range.min += shft;
    actual_range.min += shft;
    UpdateDisplay();
  }
  else {
    editor->AddLastPoint();
  }
}

void GraphLogView::NewHead() {
  LogView::NewHead();
  if(!IsMapped() || !display_toggle) return;
  if(editor != NULL) {
    editor->InitDisplay();
  }
}

int GraphLogView::SetXAxis(char* nm) {
  int dx = log_mgr->data.FindLeaf(nm);
  if(dx >= 0) {
    x_axis_index = dx;
    UpdateAfterEdit();
  }
  return x_axis_index;
}

void GraphLogView::Clear_impl() {
  view_range.max = -1;
  view_range.min = 0; // collapse the view range..
  InitDisplay();
}

void GraphLogView::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  ivResource::unref(body);	body = NULL;
  LogView::CloseWindow();
}

void GraphLogView::InitColors(){
  if(editor!=NULL) editor->InitColors();
}

void GLVSetXAxis(TAPtr tap, int idx){
  GraphLogView* glv = (GraphLogView *) tap;
  if(glv->x_axis_index != idx){
    glv->x_axis_index = idx;
    tabMisc::NotifyEdits(glv);
  }
}

void GraphLogView::GetBodyRep() {
  if(!taMisc::iv_active) return;
  if(body != NULL) return;

  editor = new GraphEditor((TAPtr) this, &(log_mgr->data),
			   (DT_GraphViewSpec*) viewspec, (ivWindow*) window);
  editor->Init();
  editor->SetOwnerXAxis = GLVSetXAxis;

  const ivColor* bgclr = GetEditColorInherit();
  if(bgclr == NULL) bgclr = wkit->background();

  ivGlyph* ed_bod = editor->GetLook();
  ivGlyph* log_buttons = GetLogButtons();
  body = layout->vbox(new ivBackground(log_buttons, bgclr), ed_bod);
  ivResource::ref(body);
}

ivGlyph* GraphLogView::GetPrintData(){
  if(editor == NULL) return LogView::GetPrintData();
  for(int i=0;i<editor->graphs.size;i++) {
    GraphGraph* gg = (GraphGraph*)editor->graphs[i];
    gg->viewer->ClearCachedAllocation(); // recompute size for printing
  }
  return editor->boxpatch;
}

void GraphLogView::UpdateFromBuffer() {
  if(!display_toggle)
    return;
  UpdateDisplay();
}

void GraphLogView::UpdateDisplay(TAPtr) {
  UpdateViewRange();
  if(editor != NULL) editor->UpdateDisplay();
  if(anim.capture) CaptureAnimImg();
}

void GraphLogView::InitDisplay() {
  LogView::InitDisplay();
  UpdateViewRange();
  if(editor == NULL) return;
  editor->last_pt_offset = 0;
  editor->InitDisplay();
}

void GraphLogView::Animate(int msec_per_point) {
  if(editor == NULL) return;
  anim_stop = false;
  int old_bufsz = editor->view_bufsz;
  editor->view_bufsz = 0;
  editor->InitDisplay();	// init with nothing shown!
  taivMisc::RunIVPending();
  editor->view_bufsz = old_bufsz;
  struct timespec ts;
  ts.tv_sec = msec_per_point / 1000;
  ts.tv_nsec = (msec_per_point % 1000) * 1000000;
  int mx_cnt = MIN(old_bufsz, log_mgr->data.MaxLength());
  cerr << "running: " << mx_cnt << " cycles: " << endl;
  for(int i=mx_cnt-1; i>=0; i--) {
    cerr << "plotting point: " << mx_cnt - i -1<< endl;
    editor->view_bufsz = mx_cnt - i -1;
    editor->last_pt_offset = i;
    editor->last_row_cnt = editor->x_axis_ar->ar->size - 1; // fake last point
    editor->AddLastPoint();
    taivMisc::RunIVPending();
    if(anim.capture) CaptureAnimImg();
    if(anim_stop) break;
    nanosleep(&ts, NULL);
  }
  anim_stop = false;
  editor->last_pt_offset = 0;	// just make sure
}

void GraphLogView::StopAnimate() {
  anim_stop = true;
}

void GraphLogView::SetColorSpec(ColorScaleSpec* colors) {
  taBase::SetPointer((TAPtr*)&colorspec, colors);
  UpdateAfterEdit();
}

void GraphLogView::SetBackground(RGBA background){
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->background = background;
  dtvsp->UpdateAfterEdit();
}

void GraphLogView::UpdateLineFeatures() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->UpdateLineFeatures();
  InitDisplay();
}

void GraphLogView::SetLineFeatures(DT_GraphViewSpec::ColorType	color_type,
				   DT_GraphViewSpec::SequenceType sequence1,
				   DT_GraphViewSpec::SequenceType sequence2,
				   DT_GraphViewSpec::SequenceType sequence3,
				   bool visible_only) {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;

  if(sequence3 == sequence1) sequence3 = DT_GraphViewSpec::NONE;
  if(sequence2 == sequence1) sequence2 = DT_GraphViewSpec::NONE;
  else if(sequence3 == sequence2) sequence3 = DT_GraphViewSpec::NONE;

  dtvsp->color_type = color_type;
  dtvsp->sequence_1 = sequence1;
  dtvsp->sequence_2 = sequence2;
  dtvsp->sequence_3 = sequence3;
  dtvsp->UpdateLineFeatures(visible_only);
  InitDisplay();
}

void GraphLogView::SetLineWidths(float line_width) {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->SetLineWidths(line_width);
}

void GraphLogView::SetLineType(DA_GraphViewSpec::LineType line_type) {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->SetLineType(line_type);
}

void GraphLogView::ShareAxisAfter(taBase* spec) {
  if(spec==NULL) return;
  if(!spec->InheritsFrom(&TA_DA_GraphViewSpec)) {
    taMisc::Error("Must specify a valid GraphViewSpec variable as the axis variable");
    return;
  }
  DA_GraphViewSpec* var = (DA_GraphViewSpec*) spec;
  DT_GraphViewSpec* own_gp = (DT_GraphViewSpec*)var->owner;
  bool not_yet = true;
  int i;
  for(i=0;i<own_gp->size;i++) {
    DA_GraphViewSpec* davs = (DA_GraphViewSpec*)own_gp->FastEl(i);
    if(davs == var) {
      not_yet = false;
      continue;
    }
    if(!davs->visible || not_yet) continue;
    taBase::SetPointer((TAPtr*)&(davs->axis_spec), var);
  }
  InitDisplay();
}

void GraphLogView::ShareAxes() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->ShareAxes();
}

void GraphLogView::SeparateAxes() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->SeparateAxes();
}

void GraphLogView::PlotRows() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->PlotRows();
}

void GraphLogView::PlotCols() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->PlotCols();
}

void GraphLogView::SeparateGraphs(int x_layout, int y_layout) {
  separate_graphs = true;
  graph_layout.x = x_layout;
  graph_layout.y = y_layout;
  UpdateAfterEdit();
}

void GraphLogView::OneGraph() {
  separate_graphs = false;
  UpdateAfterEdit();
}

void GraphLogView::TraceIncrement(float x_inc, float y_inc) {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->TraceIncrement(x_inc, y_inc);
}

void GraphLogView::StackTraces() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->StackTraces();
}

void GraphLogView::UnStackTraces() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->UnStackTraces();
}

void GraphLogView::StackSharedAxes() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->StackSharedAxes();
}

void GraphLogView::UnStackSharedAxes() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  dtvsp->UnStackSharedAxes();
}

void GraphLogView::SpikeRaster(float thresh) {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  taLeafItr i;
  DA_GraphViewSpec* davs;
  FOR_ITR_EL(DA_GraphViewSpec, davs, dtvsp->, i) {
    davs->line_type = DA_GraphViewSpec::THRESH_POINTS;
    davs->negative_draw = false;
    davs->vertical = DA_GraphViewSpec::NO_VERTICAL;
    davs->shared_y = DA_GraphViewSpec::STACK_LINES;
    davs->thresh = thresh;
    davs->line_width = 2.0;
    davs->trace_incr.y = 1.0;
    davs->trace_incr.x = 0.0;
  }
  InitDisplay();
}

void GraphLogView::ColorRaster() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  taLeafItr i;
  DA_GraphViewSpec* davs;
  FOR_ITR_EL(DA_GraphViewSpec, davs, dtvsp->, i) {
    davs->line_type = DA_GraphViewSpec::VALUE_COLORS;
    davs->negative_draw = false;
    davs->vertical = DA_GraphViewSpec::NO_VERTICAL;
    davs->shared_y = DA_GraphViewSpec::STACK_LINES;
    davs->line_width = 2.0;
    davs->trace_incr.y = 1.0;
    davs->trace_incr.x = 0.0;
  }
  InitDisplay();
}

void GraphLogView::StandardLines() {
  DT_GraphViewSpec* dtvsp = (DT_GraphViewSpec*)viewspec;
  taLeafItr i;
  DA_GraphViewSpec* davs;
  FOR_ITR_EL(DA_GraphViewSpec, davs, dtvsp->, i) {
    davs->vertical = DA_GraphViewSpec::FULL_VERTICAL;
    davs->trace_incr.y = 0.0;
    davs->trace_incr.x = 0.0;
  }
  UpdateLineFeatures();
}
