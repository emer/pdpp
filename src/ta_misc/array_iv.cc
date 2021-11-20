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


#include <array_iv.h>
#include <ta_misc/colorscale.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <ta/leave_iv.h>


////////////////////////
///  gpivarrayc_type ///
////////////////////////
  
int gpivArrayC_Type::BidForType(TypeDef* td) {
  if (td->InheritsFrom(&TA_float_Array))
    return (gpivArray_Type::BidForType(td) +1);
  return 0;
}

taivData* gpivArrayC_Type::GetDataRep(ivInputHandler*, taivDialog* dlg, taivData* par) {
  gpivArrayCEditButton *rval =
    new gpivArrayCEditButton(typ, NULL,NULL,dlg,par);
  return rval;
}

void gpivArrayC_Type::GetImage(taivData* dat, void* base, ivWindow* win) {  
  gpivArrayCEditButton *rval = (gpivArrayCEditButton*)dat;
  rval->GetImage(base, win);
}

void gpivArrayC_Type::GetValue(taivData*, void*) {
}


/////////////////////////////
/// gpivArrayCEditButton ///
////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(gpivArrayCEditButton)
implementActionCallback(gpivArrayCEditButton)
#include <ta/leave_iv.h>

gpivArrayCEditButton::gpivArrayCEditButton
(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg, taivData* par)
: gpivArrayEditButton(tp, base, win, dlg, par) {
  cive = NULL;
}

gpivArrayCEditButton::~gpivArrayCEditButton() {};

void gpivArrayCEditButton::GetMethMenus() {
  gpivArrayEditButton::GetMethMenus();
  rep->AddItem
    ("Color Edit", NULL, taivMenu::use_default,
     new ActionCallback(gpivArrayCEditButton)
     (this, &gpivArrayCEditButton::ColorEdit));
}

void gpivArrayCEditButton::ColorEdit() {
  if(!cive) {
    cive = (taivEdit *) (TA_gpivArrayCEdit.GetInstance());
  }
  cive->typ = typ;
  cive->Edit(cur_base, cur_win, false);
}


/////////////////////////
// gpivArrayCEditDialog //
/////////////////////////

// gpivArrayCEditDialog a dialog for an array type

class gpivArrayCEditDialog : public taivEditDialog {
public:
  // regular array edit stuff
  bool ShowMember(MemberDef* md);
  void GetValue();
  void GetImage();
  void Constr_Box();

  // color array edit stuff
  ColorScale*	cs;
  ivPolyGlyph* data_g;
  ivPolyGlyph* block_g;
  PScaleBar* hpb;

  gpivArrayCEditDialog(TypeDef *tp, void* base);
  ~gpivArrayCEditDialog();
};

gpivArrayCEditDialog::gpivArrayCEditDialog(TypeDef *tp, void* base)
: taivEditDialog(tp,base) 
{
  cs = NULL;
  taBase::SetPointer((TAPtr*)&cs, new ColorScale);
  cs->NewDefaults();
}

gpivArrayCEditDialog::~gpivArrayCEditDialog() {
  if (hpb) { ivResource::unref(hpb); hpb = NULL; }
  taBase::DelPointer((TAPtr*)&cs);
}

///////////////////////
/// gpivArrayCEdit ///
///////////////////////

int gpivArrayCEdit::Edit(void* base, ivWindow* win, bool wait, bool, const ivColor* bgclr) {
  taivEditDialog* dlg = taivMisc::FindEdit(base, typ);
  if(wait || (dlg == NULL)) {
    dlg = new gpivArrayCEditDialog(typ, base);
    dlg->Constr(win, wait, "", "", false, bgclr);
    return dlg->Edit();
  }
  if(!dlg->waiting) {
    ((NoBlockDialog*)dlg->dialog)->win->deiconify();
    ((NoBlockDialog*)dlg->dialog)->win->raise();
  }
  return 2;
}

int gpivArrayCEdit::BidForEdit(TypeDef*){
  return 0;
  // this is a special case edit.
  // do not bid for regular editing
}

//////////////////////////////
// gpivArrayCEditDialog funx //
/////////////////////////////

bool gpivArrayCEditDialog::ShowMember(MemberDef* md) {
  if(md->name == "size")
    return true;
  else
    return false;
}

void gpivArrayCEditDialog::GetImage() {
  taivEditDialog::GetImage();
  taArray_base* cur_ary = (taArray_base*)cur_base;
  int i;
  for(i=0;i<cur_ary->size;i++) {
    ((PadButton *) ((ivMonoGlyph*) block_g->component(i))
     ->body())->Set(*((float *)cur_ary->FastEl_(i)));
  }
}

void gpivArrayCEditDialog::GetValue() {
  taivEditDialog::GetValue();
  if(hpb) hpb->GetScaleValues();
  taArray_base* cur_ary = (taArray_base*)cur_base;
  int i;
  for(i=0;i<cur_ary->size;i++){
    *((float *) cur_ary->FastEl_(i)) =
      ((ColorPad *)
       ((PadButton *) ((ivMonoGlyph*) block_g->component(i))->body())->body())
      ->GetVal();
  }
}

void gpivArrayCEditDialog::Constr_Box() {
  taivMember* ivm;
  taivData* mb_dat;
  data_g = layout->hbox(); // this is the typedef members
  block_g = layout->hbox(); // these are the blocks
  hpb = new HPScaleBar(0,1,true,true,cs,16,8,18);
  ivResource::ref(hpb);
  // add the size thing as a read-only field..
  MemberDef* md = typ->members.FindName("size");
  ivm = md->iv;
  mb_dat = ivm->GetDataRep(dialog);
  data_el.Add(mb_dat);
  data_g->append(layout->vcenter(wkit->label("size"),-0.5));
  data_g->append(layout->vcenter
		  (layout->hnatural
		   (taivM->top_sep(mb_dat->GetLook()),35),0));

  taArray_base* cur_ary = (taArray_base*)cur_base;
  int i;
  for(i=0;i<cur_ary->size;i++) {
    PadButton* pb = new PadButton(style,hpb,ColorPad::COLOR,16,16);
    hpb->UpdateMinMax(*((float *) cur_ary->FastEl_(i)),
		      *((float *) cur_ary->FastEl_(i)));
    block_g->append(wkit->outset_frame(pb));
  }
  hpb->UpdateScaleValues();
  GetImage();
  dialog->append_input_handler(hpb->ihan);
  dialog->focus(hpb->ihan);
  box = wkit->inset_frame
    (new ivBackground
     (layout->vbox
      (layout->hcenter(data_g,0),
       layout->hcenter
       (layout->hbox(layout->hglue(),
		     layout->vcenter(block_g,0),
		     layout->hglue()),0),
       layout->hcenter(hpb->GetLook(),0)),wkit->background()->brightness(-0.15)));
  // under some circumstances these are not used, so we ref them
  ivResource::ref(labels);
}


