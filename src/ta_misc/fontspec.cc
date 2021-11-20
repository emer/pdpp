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

#include <ta_misc/fontspec.h>

#include <iv_graphic/graphic_text.h>
#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_viewer.h>

#include <ta/taiv_data.h> // taivMisc stuff
#include <css/ta_css.h> // for label viewer and master lookup

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <ta/leave_iv.h>

void FontSpec::Initialize() {
  text_g = NULL;
  fnt = NULL;
}

void FontSpec::InitLinks() {
  taBase::InitLinks();
  if(!taivMisc::iv_active) return;
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivResource::unref(fnt);
  fnt = (ivFont *) wkit->font();
  ivResource::ref(fnt);
#ifndef CYGWIN
  pattern = "*" + (String) fnt->name() + "-*";
  if (fnt != (ivFont*)ivFont::lookup(pattern)) {
    pattern = "*-" + (String) fnt->name() + "-medium-r*";
  }
#else
  pattern = "*" + (String) fnt->name() + "*medium*--10*";
#endif
  prv_pat = pattern;
}

void FontSpec::CutLinks() {
  ivResource::unref(fnt);
  fnt = NULL;
  if(text_g != NULL) {
    ivResource::unref(text_g);
    text_g = NULL;
  }
  taBase::CutLinks();
}

void FontSpec::SetFont(char* fn) {
  pattern = fn;
  UpdateAfterEdit();
}

void FontSpec::SetFontSize(int sz) {
  String szstr = String(sz);
  int len = pattern.length();
  int szpos;
  for(szpos=0;szpos<len;szpos++) {
    if(isdigit(pattern[szpos])) break;
  }
  if(szpos < len) {
    pattern = pattern.before(szpos) + szstr + "*";
  }
  else {
    pattern += szstr + "*";
  }
  UpdateAfterEdit();
}

void FontSpec::SetTextG(NoScale_Text_G* txg) {
  if(text_g != NULL)
    ivResource::unref(text_g);
  text_g = txg;
  ivResource::ref(text_g);
}

void FontSpec::UpdateAfterEdit(){
  taBase::UpdateAfterEdit();
  if(!taivMisc::iv_active) return;
  if(pattern == prv_pat) return;
#ifdef CYGWIN
  if(pattern.contains("-Helvetica-medium-r"))
    pattern.gsub("Helvetica-medium-r", "Arial*medium*");
  if(pattern.contains("-helvetica-medium-r"))
    pattern.gsub("helvetica-medium-r", "Arial*medium*");
  if(pattern.contains("Arial*medium*")) {
    String szs = pattern.after("Arial*medium*");
    String nwsz;
    int i;
    for(i=0;i<(int)szs.length();i++) {
      if(!isdigit(szs[i])) continue;
      nwsz += szs[i];
    }
    if(nwsz.empty()) nwsz = "12";
    else if((int)nwsz < 9) nwsz = "9";
    pattern = "*Arial*medium*--" + nwsz + "*";
  }
  ivResource::unref(fnt);
  fnt = new ivFont(pattern);	// always get something with the CYGWIN impl
  ivResource::ref(fnt);
  prv_pat = pattern;
#else
  const ivFont* f = ivFont::lookup(pattern);
  if((f==NULL) && pattern.contains("Arial*medium*")) {
    String szs = pattern.after("Arial*medium*");
    String nwsz;
    int i;
    for(i=0;i<(int)szs.length();i++) {
      if(!isdigit(szs[i])) continue;
      nwsz += szs[i];
    }
    if(nwsz.empty()) nwsz = "10";
    else if((int)nwsz <= 9) nwsz = "8";
    pattern = "*-Helvetica-medium-r*" + nwsz + "*";
    f = ivFont::lookup(pattern);
  }
  if(f==NULL){
    f = ivFont::lookup(String( "*-" + pattern + "-medium-r*"));
  }
  if(f==NULL){
    f = ivFont::lookup(String( "*-" + pattern + "-r*"));
  }
  if(f==NULL){
    f = ivFont::lookup(String( "*-" + pattern + "-*"));
  }
  if(f==NULL){
    taMisc::Error("Cannot find font: ", pattern);
    return;
  }
  prv_pat = pattern;
  if(fnt == f) return;
  ivResource::unref(fnt);
  fnt = (ivFont*)f;
  ivResource::ref(fnt);
#endif
  if(text_g != NULL) {
    text_g->font(fnt);
  }
}

void FontSpec::XFontSel() {
  String command = String("xfontsel -pattern \"")  + pattern + String("\" &");
  system((char *) command);
}

void FontSpec::Copy_(const FontSpec& cp) {
  pattern = cp.pattern;
  prv_pat = cp.prv_pat;
  ivResource::unref(fnt);
  fnt = cp.fnt;
  ivResource::ref(fnt);
}

void ViewLabel::Initialize() {
  label_xform = NULL;
  master = NULL;
  viewer = NULL;
  select_effect = NULL;
  get_color = NULL;
}

void ViewLabel::Destroy() {
  CutLinks();
  master = NULL;
  viewer = NULL;
}

void ViewLabel::Copy_(const ViewLabel& cp) {
  spec = cp.spec;
  if(cp.label_xform != NULL) {
    SetLabelXform((Xform*)cp.label_xform->Clone());
  }
}

void ViewLabel::UpdateAfterEdit(){
  taNBase::UpdateAfterEdit();
  if(!taMisc::iv_active) return;
  // we need to find something with a viewer and a GraphicMaster
  GetMasterViewer();
  UpdateView();
}

void ViewLabel::InitLinks(){
  taNBase::InitLinks();
  taBase::Own(spec, this);
  if(!taMisc::iv_active) return;
}
  
void ViewLabel::CutLinks() {
  spec.CutLinks();
  RemoveFromView();
  taBase::DelPointer((TAPtr*)&label_xform);
  taNBase::CutLinks();
}

void ViewLabel::SetLabelXform(Xform* xf){
  if(label_xform != NULL)
    taBase::DelPointer((TAPtr*)&label_xform);
  label_xform = xf;
  taBase::Own(label_xform,this);
}

void ViewLabel::XFontSel(){
  spec.XFontSel();
}

void ViewLabel::MakeText() {
  GetMasterViewer();
  if((master == NULL) || (viewer == NULL)) return;
  if(spec.text_g == NULL) {
    ivWidgetKit* wkit = ivWidgetKit::instance();
    spec.SetTextG(new NoScale_Text_G(this,master,spec.fnt,wkit->foreground(),
				     name,NULL));
    spec.text_g->editable = true;
    if(select_effect != NULL) spec.text_g->select_effect = select_effect;
    if(get_color != NULL) spec.text_g->get_color = get_color;
    if(label_xform != NULL) {
      ivTransformer* tx = label_xform->transformer();
      spec.text_g->transformer(tx);
      ivResource::unref(tx);	// unref the self-ref from transform
    }
  }
  AddToView();
}

void ViewLabel::AddToView() {
  if(UpdateView()) return;
  master->append_(spec.text_g);
}

bool ViewLabel::UpdateView() {
  if((viewer == NULL) || (master == NULL) || (spec.text_g == NULL))  return false;
  spec.text_g->select_effect = select_effect;
  spec.text_g->text(name);
  if(label_xform != NULL) {
    ivTransformer* tx = label_xform->transformer();
    spec.text_g->transformer(tx);
    ivResource::unref(tx);	// unref the self-ref from transform
  }
  Graphic* vlg;
  ivGlyphIndex j;
  for(j=master->count_()-1; j>=0; j--) {
    vlg = master->component_(j);
    if((vlg->InheritsFrom(&TA_NoScale_Text_G)) && (((NoScale_Text_G*)vlg)->obj == this)) {
      //	  master->modified_(j);
      if(viewer->canvas() != NULL) 
	vlg->damage_me(viewer->canvas());
      return true;
    }
  }
  return false;
}

void ViewLabel::RemoveFromView() {
  if((viewer == NULL) || (master == NULL))  return;
  Graphic* vlg;
  ivGlyphIndex j;
  for(j=master->count_()-1; j>=0; j--) {
    vlg = master->component_(j);
    if((vlg->InheritsFrom(&TA_NoScale_Text_G)) && (((NoScale_Text_G*)vlg)->obj == this)) {
      if(viewer->canvas() != NULL) 
	vlg->damage_me(viewer->canvas());
      master->remove_(j);
      return; // we've found and removed the label no need to keep searching
    }
  }
} 




