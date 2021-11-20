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

#include <colorbar.h>
#include <ta/taiv_data.h>

#include <ta/enter_iv.h>

#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/layout.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/background.h>
#include <InterViews/brush.h>

//for colorpad
#include <InterViews/printer.h>
#include <InterViews/background.h>
#include <InterViews/patch.h>
#include <InterViews/stencil.h>
#include <InterViews/event.h>
#include <InterViews/debug.h>
#include <InterViews/font.h>

// for pbar
#include <InterViews/border.h>
#include <IV-look/choice.h>
#include <IV-look/field.h>
#include <InterViews/polyglyph.h>
#include <InterViews/telltale.h>


// for dragcursor
#include <InterViews/cursor.h>
#include <InterViews/bitmap.h>
#include <ta_misc/painter.bm>
#include <ta_misc/painter_mask.bm>
#include <InterViews/window.h>


//for scalebar
#include <InterViews/style.h>
#include <IV-look/field.h>
#include <InterViews/patch.h>
#include <OS/math.h>
#include <ta/leave_iv.h>

//for scalebar
#include <iv_misc/dastepper.h>

// for colorpadtext
#include <iv_misc/dynalabel.h>

Bar::Bar(ColorScale* c, int, int h, int w){
  scale = c;
  taBase::Ref(scale);
//  blocks = b;
  blocks = scale->chunks;	// use this instead!!
  height = h;
  width = w;
}

Bar::~Bar(){
  taBase::unRefDone(scale);
}

void Bar::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  ext.merge(c, a);
}

void Bar::SetColorScale(ColorScale* c){
  taBase::Ref(c);
  taBase::unRefDone(scale);
  scale = c;
  blocks = scale->chunks;
}

CBar::CBar(const ivBrush* br, ColorScale* c, int b, int h, int w)
  : Bar(c,b,h,w) {
  brush = br;
  ivResource::ref(brush);
}

CBar::~CBar(){
  ivResource::unref(brush);
}


// **********************
// 	HCBar
// **********************

HCBar::HCBar(const ivBrush* br, ColorScale* c, int b, int h, int w)
: CBar(br,c,b,h,w) {};

void HCBar::request(ivRequisition& req) const {
  ivCoord w = 2 * brush->width();
  ivCoord wd = 288 + w;
//  ivCoord wd = (width + w) * (blocks +1);
  ivCoord ht = (height + w);
  
  ivRequirement rx(wd, 1000, blocks, 0.5);
  ivRequirement ry(ht, 0, 0, 0);
  req.require(Dimension_X, rx);
  req.require(Dimension_Y, ry);
}

void HCBar::draw(ivCanvas* c, const ivAllocation& a) const {
  ivCoord l = a.left(), r = a.right(), t = a.top(), b = a.bottom();
  ivCoord ht = (t - b);
  ivCoord w = brush->width();
  ivCoord wd = (r-l)/(float)blocks;
  int idx;
  c->new_path();
  float factor = (float)(scale->chunks-1) / (float)(blocks -1);
  int i;
  for(i=0;i<blocks;i++) {
    idx = (int) ((float) i * factor);
    c->fill_rect(l + (((float) i) * wd),b,
		 l + (((float) i) * wd) + (wd-(2.0*w)),b+ht,
		 scale->GetColor(idx));
  }
}



// **********************
// 	VCBar
// **********************

VCBar::VCBar(const ivBrush* br, ColorScale* c, int b, int h, int w)
: CBar(br,c,b,h,w) {};


void VCBar::request(ivRequisition& req) const {
  ivCoord w = 2 * brush->width();
  ivCoord wd = width + w;
//  ivCoord ht = (height + w) * (blocks + 1);
  ivCoord ht = 288 + w;
  
  ivRequirement rx(wd, 0, 0, 0.5);
  ivRequirement ry(ht, 1000, blocks, 0.5);
  req.require(Dimension_X, rx);
  req.require(Dimension_Y, ry);
}

void VCBar::draw(ivCanvas* c, const ivAllocation& a) const {
  ivCoord l = a.left(), r = a.right(), t = a.top(), b = a.bottom();
  ivCoord ht = (t - b)/(float)blocks;
  ivCoord w = brush->width();
  ivCoord wd = (r-l) - (2 * w);
  int idx;
  c->new_path();
  float factor = ((float) (scale->chunks -1)) / ((float) (blocks -1));
  int i;
  for(i=0;i<blocks;i++) {
    idx = (int) ((float) i * factor);
    c->fill_rect(l,b + (((float) i) * ht),l+wd,
		 b + (((float) i) * ht) + (ht-(2.0*w)),
		 scale->GetColor(idx));
  }
}

PBar::PBar(ColorScale* c, int b, int h, int w) : Bar(c,b,h,w) {
  box = NULL;
  grp = new ivTelltaleGroup();
  ivResource::ref(grp);
  blocks = 17;
};

PBar::~PBar(){
  ivResource::unref(grp);
  ivResource::unref(blockpatch);
}

void PBar::SetColorScale(ColorScale* c){
  Bar::SetColorScale(c);
  blocks = 15;
  while(box->count() > 0){
    box->remove(0);
  }
  InsertBlocks(box);
  blockpatch->reallocate();
  blockpatch->redraw();
}

void PBar::InsertBlocks(ivPolyGlyph* pg){
  ivWidgetKit* kit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivTelltaleState* firststate = NULL;

  ivColor* selectcolor = new ivColor(1.0,1.0,1.0,1.0);
  int i;
  for(i=0;i<blocks;i++){
    int idx = (int) ( (float) i * ((float) (scale->chunks -1)
			       / (float) (blocks-1)));
    ivGlyph* coloroff =
      layout->margin
      (kit->outset_frame
       (layout->flexible
	(layout->natural
	 (new ivBackground(NULL,scale->GetColor(idx)),
	  width-2,height-2),fil,8)),1);

    ivGlyph* coloron =
      new ivBorder
      (layout->margin
       (kit->outset_frame
	(layout->flexible
	 (layout->natural
	  (new ivBackground(NULL,scale->GetColor(idx)),
	   width-2,height-2),fil,8)),1),selectcolor,1);

    ivTelltaleState* bstate = 
      new ivTelltaleState(ivTelltaleState::is_enabled_visible |
			ivTelltaleState::is_choosable);
    bstate->join(grp);
    if(i==0) {
      firststate = bstate;
    }
    pg->append
       (new ivButton
	(new ivChoiceItem
	 (bstate,coloroff,
	  coloroff,coloroff,coloron,coloron,
	  coloron,coloron,coloron,coloron,
	  coloroff),kit->style(),bstate,NULL));
  }
  if(firststate != NULL)
    firststate->set(ivTelltaleState::is_chosen,true);
}


int PBar::GetSelected(){
  if(box){
    int i;
    for(i=0;i<box->count();i++){
      if(((ivButton *)(box->component(i)))->state()->test
	 (ivTelltaleState::is_chosen)) {
	return i;
      }
    }
  }
  return -1;
}

VPBar::VPBar(ColorScale* c, int b, int h, int w) : PBar(c,b,h,w) {
  ivLayoutKit* layout = ivLayoutKit::instance();
  box = layout->vbox();
  InsertBlocks(box);
  blockpatch  = new ivPatch(box);
  ivResource::ref(blockpatch);
};

ivGlyph* VPBar::GetBar() {
  ivWidgetKit* kit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  return (kit->inset_frame(layout->hflexible(blockpatch)));
}

HPBar::HPBar(ColorScale* c, int b, int h, int w) : PBar(c,b,h,w) {
  ivLayoutKit* layout = ivLayoutKit::instance();
  box = layout->hbox();
  InsertBlocks(box);
  blockpatch  = new ivPatch(box);
  ivResource::ref(blockpatch);
};

ivGlyph* HPBar::GetBar() {
  ivWidgetKit* kit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  return (kit->inset_frame(layout->vflexible(blockpatch)));
}


#include <ta/enter_iv.h>
declareFieldEditorCallback(ScaleBar)
implementFieldEditorCallback(ScaleBar)
#include <ta/leave_iv.h>

ScaleBar::ScaleBar(float mn,float mx,bool adj, bool ed){
  adjustflag = adj;
  editflag = ed;
  min = mn;
  max = mx;
  range = (max - min)/2.0;
  zero = (min + max)/2.0;
  sm = MIN_MAX;
  bar = NULL;
};

ScaleBar::ScaleBar(float r,bool adj,bool ed){
  adjustflag = adj;
  editflag = ed;
  range = r;
  sm = RANGE;
  min = -range;
  max = range;
  zero = 0;
  bar = NULL;
};

void ScaleBar::UpdatePads(){
  int i;
  for(i=0;i<padlist.size;i++) {
    ((ColorPad*) padlist.FastEl(i))->Reset();
  }
}

#include <ta/enter_iv.h>
declareIntActionCallback(ScaleBar)
implementIntActionCallback(ScaleBar)
#include <ta/leave_iv.h>

void ScaleBar::Initialize(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivDialogKit*  dkit = ivDialogKit::instance();

  adjustnotify = NULL;
  String min_str(min, "%5.3f");
  String max_str(max, "%5.3f");

  ihan = NULL;
  if (editflag) {
    ivStyle* style = new ivStyle(wkit->style());
    min_frep = dkit->field_editor
      (min_str,style,
	new FieldEditorCallback(ScaleBar)
       (this,&ScaleBar::editor_accept,&ScaleBar::editor_reject));
    max_frep = dkit->field_editor
      (max_str,style,
	new FieldEditorCallback(ScaleBar)
       (this,&ScaleBar::editor_accept,&ScaleBar::editor_reject));
    min_frep->select(0);
    max_frep->select(0);
    min_label = new ivPatch(layout->hfixed(min_frep,42));
    max_label = new ivPatch(layout->hfixed(max_frep,42));

  }
  else {
    min_label = new ivPatch
      (wkit->inset_frame
       (layout->hflexible
	(layout->hnatural(wkit->fancy_label(min_str),42),fil,0)));
    max_label = new ivPatch
      (wkit->inset_frame
       (layout->hflexible
	(layout->hnatural(wkit->fancy_label(max_str),42),fil,0)));
  }
  if (adjustflag) {

    if(sm == RANGE) {
      enlarger = GenEnlarger();
      shrinker = GenShrinker();
    }
    else {			// sm = min/max
      ivTelltaleState* miniss = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);
      ivTelltaleState* mindss = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);
      ivTelltaleState* maxiss = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);
      ivTelltaleState* maxdss = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);

      min_incr = new ParameterStepper
	(wkit->up_mover_look(miniss),
	 wkit->style(),miniss,
	 new IntActionCallback
	 (ScaleBar)(this,&ScaleBar::Incr_Min));

      min_decr = new ParameterStepper
	(wkit->down_mover_look(mindss),
	 wkit->style(),mindss,
	 new IntActionCallback
	 (ScaleBar)(this,&ScaleBar::Decr_Min));

      max_incr = new ParameterStepper
	(wkit->up_mover_look(maxiss),
	 wkit->style(),maxiss,
	 new IntActionCallback
	 (ScaleBar)(this,&ScaleBar::Incr_Max));

      max_decr = new ParameterStepper
	(wkit->down_mover_look(maxdss),
	 wkit->style(),maxdss,
	 new IntActionCallback
	 (ScaleBar)(this,&ScaleBar::Decr_Max));
    }
  }
}

void ScaleBar::Destroy(){
  padlist.Reset();
  if (adjustnotify) { ivResource::unref(adjustnotify); adjustnotify = NULL; }
  ivResource::unref(bar);
}

ivGlyph* ScaleBar::GenIH(ivGlyph* g){
  if(editflag) {
    ivWidgetKit* wkit = ivWidgetKit::instance();
    ihan = new ivInputHandler(g,wkit->style());
    ihan->append_input_handler((ivInputHandler *) min_frep);
    ihan->append_input_handler((ivInputHandler *) max_frep);

    ihan->focus((ivInputHandler *) min_frep);
    ihan->focus((ivInputHandler *) max_frep);

    return(ihan);
  }
  else return(g);
}

void ScaleBar::Adjust(){
  if(adjustnotify) {
    adjustnotify->execute();
  }
}

void ScaleBar::editor_accept(ivFieldEditor*) {
  GetScaleValues();
  UpdatePads();
  Adjust();
}

void ScaleBar::editor_reject(ivFieldEditor*) { 
  UpdateScaleValues();
  UpdatePads();
  Adjust();
}

#define scalebar_low_value	1.0e-10

void ScaleBar::Incr_Range(int i){
  if(adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++){
      float incr = osMath::max((float)(fabs((double)range)*.111111)
			       ,(float) scalebar_low_value);
      ModRange(range + incr);
    }
    UpdateScaleValues();
    UpdatePads();
    Adjust();
  }

};

void ScaleBar::Decr_Range(int i){
  if (adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++){
      float decr =  osMath::max((fabs((double) range )*.1),scalebar_low_value);
      ModRange(range - decr);
    }
    UpdateScaleValues();
    UpdatePads();
    Adjust();
  }
};

void ScaleBar::Incr_Min(int i){
  if (adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++) {
      float newmin = min  + osMath::max((float)(fabs((double) min)*.111111)
				    ,(float) scalebar_low_value);
      if(newmin > max) newmin = max;
      min = newmin;
    }
    FixRangeZero();
    UpdateScaleValues();
    UpdatePads();
    Adjust();

  }
}
void ScaleBar::Decr_Min(int i){
  if (adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++) {
      min -= osMath::max((fabs((double) min)*.1),scalebar_low_value);
    }
    FixRangeZero();
    UpdateScaleValues();
    UpdatePads();
    Adjust();
  }
}

void ScaleBar::Incr_Max(int i){
  if (adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++) {
      max += osMath::max((fabs((double)max) * 0.111111),scalebar_low_value);
    }
    FixRangeZero();
    UpdateScaleValues();
    UpdatePads();
    Adjust();
  }
}
void ScaleBar::Decr_Max(int i){
  if (adjustflag) {
    GetScaleValues();
    int j;
    for(j=0;j<i;j++) {
      float newmax = max - osMath::max((fabs((double) max )*.1),scalebar_low_value);
      if(newmax < min) newmax = min;
      max = newmax;
    }
    FixRangeZero();
    UpdateScaleValues();
    UpdatePads();
    Adjust();
  }
}
void ScaleBar::SetRange(float val){
  ModRange(val);
  UpdateScaleValues();
};


void ScaleBar::ModRange(float val){
  range = fabs(val);
  min = zero - range;
  max = zero + range;
}


void ScaleBar::ModRoundRange(float val) {
  val = fabs(val);
  int val_order = (val <= 0) ? 0 : (int)log10(fabs(val)); // order of the value
  double val_units;
  if(val > 1)
    val_units = powf(10.0f, (float)val_order); // units
  else
    val_units = powf(10.0f, (float)(val_order-1));
  float val_rat = (val / val_units);
  int rng_val = 0;
  if(val_rat <= 1.0)
    rng_val = 1;
  if(val_rat <= 2.0)
    rng_val = 2;
  else if(val_rat <= 5.0)
    rng_val = 5;
  else
    rng_val = 10;
  
  range = (double)rng_val * val_units;
  if(range == 0)	range = 1.0; // avoid div by 0
  min = zero - range;
  max = zero + range;
}

// rounds up the range to the nearest 1,2,or 5 value
void ScaleBar::SetRoundRange(float val){
  ModRoundRange(val);
  UpdateScaleValues();
}


void ScaleBar::SetAdjustNotify(ivAction* a){
  if (adjustnotify) { ivResource::unref(adjustnotify); }
  adjustnotify = a;
  ivResource::ref(adjustnotify);
}

void ScaleBar::FixRangeZero() {
  range = (max - min) /2.0;
  zero = max - range;
};

void ScaleBar::UpdateMinMax(float mn, float mx) {
  if(mn < min) min = mn;
  if(mx > max) max = mx;
  FixRangeZero();
}

void ScaleBar::SetMinMax(float mn,float mx){
  if((min == mn) && (max == mx)) return; 
  min = mn; max = mx;  FixRangeZero();
  UpdateScaleValues();
}
void ScaleBar::GetScaleValues(){
  if(editflag){
    min = atof(min_frep->text()->string());
    max = atof(max_frep->text()->string());
  }
  FixRangeZero();
}
void ScaleBar::UpdateScaleValues(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();

  String min_str(min, "%5.3f");
  String max_str(max, "%5.3f");

  if (editflag) {
      min_frep->field(min_str);
      min_frep->select(0);
      max_frep->field(max_str);
      max_frep->select(0);
  }
  else {
    ivGlyph* minl = wkit->inset_frame
      (layout->hflexible
       (layout->hnatural(wkit->fancy_label(min_str),42),fil,0));
    ivGlyph* maxl = wkit->inset_frame
      (layout->hflexible
       (layout->hnatural(wkit->fancy_label(max_str),42),fil,0));
    min_label->body(minl);
    max_label->body(maxl);
  }
    max_label->reallocate();
    min_label->reallocate();
    min_label->redraw();
    max_label->redraw();

  UpdatePads();
}
int ScaleBar::GetIdx(float val) {
  int chnk = bar->scale->chunks-1;
  float rval = ((max - min) == 0.0f) ? 0.0f : (val-min)/(max-min);
  int idx = (int) ((rval * (float)chnk) + .5);
  idx = MAX(0, idx);
  idx = MIN(chnk, idx);
  return idx;
}
float ScaleBar::GetAbsPercent(float val){
  if(val >= zero) return ((max - zero) == 0.0f) ? 0.0f : 
  fabs((val-zero) / (max - zero));
  else return ((zero-min) == 0.0f) ? 0.0f : 
  fabs((zero-val) / (zero-min));
}

float ScaleBar::GetVal(int idx) {
  return (((idx * (max-min))/(bar->blocks-1)) + min);
}

ivColor* ScaleBar::GetColor(float val,ivColor** maincolor, ivColor** contrast) {
  int idx;
  ivColor *m,*c;
  if(range == 0) {
    m = bar->scale->GetColor((int) ((.5 * (float)(bar->scale->chunks-1)) + .5));
    c = bar->scale->GetContrastColor(bar->scale->chunks-1);
  }
  else if(val > max) {
    m = bar->scale->maxout.color;
    c = bar->scale->maxout.contrastcolor; // GetContrastColor(bar->scale->chunks -1);
  }
  else if(val < min) {
    m = bar->scale->minout.color;
    c = bar->scale->minout.contrastcolor; // GetContrastColor(0);
  }
  else {
    idx = GetIdx(val);
    m = bar->scale->GetColor(idx);
    c = bar->scale->GetContrastColor(idx);
  }
  if (maincolor!= NULL) *maincolor = m;
  if (contrast != NULL) *contrast = c;
  return m;
}

ivColor* ScaleBar::GetColor(int idx){
  int i = idx;
  if(i < 0) i = bar->scale->colors.size + i;
  return(bar->scale->GetColor(i));
}

ivColor* ScaleBar::GetContrastColor(int idx){
  int i = idx;
  if(i < 0) i = bar->scale->colors.size + i;
  return(bar->scale->GetContrastColor(i));
}


void ScaleBar::SetColorScale(ColorScale* c){
  if(bar != NULL) bar->SetColorScale(c);
}

HCScaleBar::HCScaleBar(float amin, float amax, bool adj,bool ed,
		       const ivBrush* br, ColorScale* c,
		       int b,int h,int w)
: ScaleBar(amin,amax,adj,ed) {
  bar = new HCBar(br,c,b,h,w);
  ivResource::ref(bar);
  Initialize();
}


HCScaleBar::HCScaleBar(float r,bool adj, bool ed,const ivBrush* br, ColorScale* c,
	   int b,int h,int w)
: ScaleBar(r,adj,ed) {
  bar = new HCBar(br,c,b,h,w);
  ivResource::ref(bar);
  Initialize();
}

DAStepper* HCScaleBar::GenEnlarger() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* ess = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);

  return (new ParameterStepper
	  (wkit->right_mover_look(ess),
	   wkit->style(),ess,
	   new IntActionCallback
	   (ScaleBar)(this,&ScaleBar::Incr_Range)));
}
DAStepper* HCScaleBar::GenShrinker() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* sss = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  return(new ParameterStepper
	 (wkit->left_mover_look(sss),
	  wkit->style(),sss,
	  new IntActionCallback
	  (ScaleBar)(this,&ScaleBar::Decr_Range)));
}
       
ivGlyph* HCScaleBar::GetLook(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  if(adjustflag){
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
			   (layout->vflexible(bar->GetBar())),0),
	   layout->vcenter(max_label,0),
	   layout->vcenter
	   (layout->vflexible(layout->natural(shrinker,24,16)),0),
	   layout->vcenter
	   (layout->vflexible(layout->natural(enlarger,24,16)),0)
	   )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(min_incr,24,16),0),
	     layout->hcenter(layout->natural(min_decr,24,16),0)),0),
	   layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
			   (layout->vflexible(bar->GetBar())),0),
	   layout->vcenter(max_label,0),
	   layout->vcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(max_incr,24,16),0),
	     layout->hcenter(layout->natural(max_decr,24,16),0)),0)
	   )));
    }
  }
  else {
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
                           (layout->vflexible(bar)),0),
	   layout->vcenter(max_label,0)
	  )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
                           (layout->vflexible(bar)),0),
	   layout->vcenter(max_label,0)
	    )));
    }
  }
  return GenIH(scalebar);
}


VCScaleBar::VCScaleBar(float amin, float amax, bool adj,bool ed,
		       const ivBrush* br, ColorScale* c,
		       int b,int h,int w)
  : ScaleBar(amin,amax,adj,ed) {
  bar = new VCBar(br,c,b,h,w);
  ivResource::ref(bar);
  Initialize();
}


VCScaleBar::VCScaleBar(float r,bool adj,bool ed,const ivBrush* br, ColorScale* c,
		       int b,int h, int w)
: ScaleBar(r,adj,ed) {
  bar = new VCBar(br,c,b,h,w);
  ivResource::ref(bar);
  Initialize();
}


DAStepper* VCScaleBar::GenEnlarger() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
      ivTelltaleState* ess = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);
  return (new ParameterStepper
	  (wkit->up_mover_look(ess),
	   wkit->style(),ess,
	   new IntActionCallback
	   (ScaleBar)(this,&ScaleBar::Incr_Range)));
}

DAStepper* VCScaleBar::GenShrinker() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* sss = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  return(new ParameterStepper
	 (wkit->down_mover_look(sss),
	  wkit->style(),sss,
	  new IntActionCallback
	  (ScaleBar)(this,&ScaleBar::Decr_Range)));
}

ivGlyph* VCScaleBar::GetLook(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  if(adjustflag) {
    if(sm == RANGE) {
      scalebar =
	(layout->vbox
	 (layout->hcenter
	  (layout->vflexible
	   (layout->hflexible(layout->natural(enlarger,32,20),fil,0),0,0),0),
	  layout->hcenter
	  (layout->vflexible
	   (layout->hflexible(layout->natural(shrinker,32,20),fil,0),0,0),0),
	  layout->hcenter(max_label,0),
	  layout->hcenter
	  (wkit->outset_frame
	   (layout->hflexible(bar->GetBar())),0),
	  layout->hcenter(min_label,0)));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(min_incr,32,20),0),
	     layout->hcenter(layout->natural(min_decr,32,20),0)),0),
	   layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
			   (layout->hflexible(bar->GetBar())),0),
	   layout->hcenter(max_label,0),
	   layout->hcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(max_incr,32,20),0),
	     layout->hcenter(layout->natural(max_decr,32,20),0)),0)
	   )));
    }
  }
  else {
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
                           (layout->hflexible(bar)),0),
	   layout->hcenter(max_label,0)
	  )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
                           (layout->hflexible(bar)),0),
	   layout->hcenter(max_label,0)
	    )));
    }
  }
  return GenIH(scalebar);
}

#include <ta/enter_iv.h>
declareFieldEditorCallback(PScaleBar)
implementFieldEditorCallback(PScaleBar)
#include <ta/leave_iv.h>


PScaleBar::PScaleBar(float mn, float mx,bool adj,bool ed)
: ScaleBar(mn,mx,adj,ed) {
  tagpads = NULL;
};

PScaleBar::PScaleBar(float r,bool adj,bool ed)
: ScaleBar(r,adj,ed) {
  tagpads=NULL;
};

void PScaleBar::Destroy()
{
   if (tagpads) { ivResource::unref(tagpads); tagpads = NULL; }
   if (fbox) { ivResource::unref(fbox); fbox = NULL; }
}

ivGlyph* PScaleBar::GenIH(ivGlyph* g){
  ivGlyph* rg = ScaleBar::GenIH(g);
  if(ihan){
    int i;
    for(i=0;i<4;i++){
      ihan->append_input_handler(((ivInputHandler *) fbox->component(i)));
      ihan->focus(((ivInputHandler *) fbox->component(i)));
    }
  }
  return rg;
}

void PScaleBar::MakeTags(){
  ivLayoutKit* layout = ivLayoutKit::instance();  
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivDialogKit*  dkit = ivDialogKit::instance();

  int i;
  tag[0] = -1; tag[1] = 0; tag[2] = .5; tag[3]= 1;
  fbox = layout->hbox();
  ivResource::ref(fbox);
  for(i=0;i<4;i++){
    stag[i] = "";
    stag[i] += String(tag[i]);
    fbox->append(dkit->field_editor
		 ((char *) stag[i],wkit->style(),
		  new FieldEditorCallback(PScaleBar)
		  (this,&PScaleBar::Get_Tags,&PScaleBar::Set_Tags)
		  ));
  }
}  

void PScaleBar::MakePads(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();

  if(!tagpads) {
    tagpads=layout->hbox();
    ivResource::ref(tagpads);
  }
  int i;
  for(i=0;i<4;i++){
    ivTelltaleState* bstate = 
      new ivTelltaleState(ivTelltaleState::is_enabled_visible |
			ivTelltaleState::is_choosable);
    bstate->join(((PBar *)bar)->grp);
    ivColor* selectcolor = new ivColor(1.0,1.0,1.0,1.0);
    pb[i] = new PadButton(wkit->style(),this);
    pb[i]->Set(tag[i]);
    ivGlyph* coloroff = layout->margin (wkit->outset_frame(pb[i]),1);
    ivGlyph* coloron = new ivBorder(layout->margin(wkit->outset_frame(pb[i]),1),selectcolor,1);
    ivButton* tagbut =  new ivButton
      (new ivChoiceItem
       (bstate,coloroff,
	coloroff,coloroff,coloron,coloron,
	coloron,coloron,coloron,coloron,
	coloroff),wkit->style(),bstate,NULL);
    tagpads->append(tagbut);
    if(i==3) bstate->set(ivTelltaleState::is_chosen,true);
  }
}

void PScaleBar::SetColorScale(ColorScale* c) {
  ScaleBar::SetColorScale(c);
  // also all the tag's colorpads
  int i;
  for(i=0;i<4;i++){
    pb[i]->cp->Reset(); // force a change
  }
}

int PScaleBar::GetSelected() {
  int result =((PBar *) bar)->GetSelected();
  if(result) {
    return result;
  }
  else {
    int i;
    for(i=0;i<4;i++){
      if(((ivButton *)(tagpads->component(i)))->state()->test
	 (ivTelltaleState::is_chosen)) {
	float pushval = atof(min_frep->text()->string());
	return GetIdx(pushval);
      }
    }
  }
  return -1;
}


void PScaleBar::GetScaleValues(){
  ScaleBar::GetScaleValues();
  Get_Tags(NULL);
}

void PScaleBar::Get_Tags(ivFieldEditor*){
  int i;
  for(i=0;i<4;i++){
    float pushval =
      atof(((ivFieldEditor*)fbox->component(i))->text()->string());
    pb[i]->Set(pushval);
    tag[i] = pushval;
    stag[i] = "";
    stag[i] += String(tag[i]);
  }
}

void PScaleBar::Set_Tags(ivFieldEditor*){
  int i;
  for(i=0;i<4;i++){
    pb[i]->Set(tag[i]);
    ((ivFieldEditor *) fbox->component(i))->field((char *) stag[i]);
    ((ivFieldEditor *) fbox->component(i))->select(0);
  }
}

float PScaleBar::GetSelectedVal() {
  int sval = ((PBar *) bar)->GetSelected();
  if(sval != -1) {
    return (GetVal(sval));
  }
  else {
    int i;
    for(i=0;i<4;i++){
      if(((ivButton *)(tagpads->component(i)))->state()->test
	 (ivTelltaleState::is_chosen)) {
	float pushval =
	  atof(((ivFieldEditor*)fbox->component(i))->text()->string());
	return pushval;
      }
    }
  }
  return 0; // ??
}



HPScaleBar::HPScaleBar(float amin, float amax, bool adj, bool ed,ColorScale* c,
		       int b,int h,int w)
: PScaleBar(amin,amax,adj,ed) {
  bar = new HPBar(c,b,h,w);
  ivResource::ref(bar);
  Initialize();
  MakeTags();
  MakePads();
};

HPScaleBar::HPScaleBar(float r,bool adj,bool ed,ColorScale* c,
	   int b,int h, int w)
: PScaleBar(r,adj,ed) {
  bar = new HPBar(c,b,h,w);
  ivResource::ref(bar);
  Initialize();
  MakeTags();
  MakePads();
}

DAStepper* HPScaleBar::GenEnlarger() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* ess = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);

  return (new ParameterStepper
	  (wkit->right_mover_look(ess),
	   wkit->style(),ess,
	   new IntActionCallback
	   (ScaleBar)(this,&ScaleBar::Incr_Range)));
}

DAStepper* HPScaleBar::GenShrinker() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* sss = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  return(new ParameterStepper
	 (wkit->left_mover_look(sss),
	  wkit->style(),sss,
	  new IntActionCallback
	  (ScaleBar)(this,&ScaleBar::Decr_Range)));
}


ivGlyph* HPScaleBar::GetLook(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivGlyph* tagbox = layout->hbox();
  int i;
  for(i=0;i<4;i++){
    tagbox->append
      (layout->vcenter
       (wkit->outset_frame
	(layout->hbox(layout->vcenter(tagpads->component(i),0),
		      layout->vcenter(fbox->component(i),0))),0));
  }
  if(adjustflag){
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter
	   (layout->vflexible(layout->natural(shrinker,24,16)),0),
	   layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
			   (layout->vflexible(bar->GetBar())),0),
	   layout->vcenter(max_label,0),
	   layout->vcenter
	   (layout->vflexible(layout->natural(enlarger,24,16)),0)
	   )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter(tagbox,0),
	   layout->hbox
	   (layout->vcenter
	    (layout->vbox
	     (layout->hcenter(layout->natural(min_incr,24,16),0),
	      layout->hcenter(layout->natural(min_decr,24,16),0)),0),
	    layout->vcenter(min_label,0),
	    layout->vcenter(wkit->outset_frame
			    (layout->vflexible(bar->GetBar())),0),
	    layout->vcenter(max_label,0),
	    layout->vcenter
	    (layout->vbox
	     (layout->hcenter(layout->natural(max_incr,24,16),0),
	      layout->hcenter(layout->natural(max_decr,24,16),0)),0)
	    ))));
    }
  }
  else {
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
                           (layout->vflexible(bar->GetBar())),0),
	   layout->vcenter(max_label,0)
	  )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter(min_label,0),
	   layout->vcenter(wkit->outset_frame
                           (layout->vflexible(bar->GetBar())),0),
	   layout->vcenter(max_label,0)
	    )));
    }
  }
  return GenIH(scalebar);
}


VPScaleBar::VPScaleBar(float amin, float amax, bool adj, bool ed,ColorScale* c,
		       int b,int h,int w)
: PScaleBar(amin,amax,adj,ed) {
  bar = new VPBar(c,b,h,w);
  ivResource::ref(bar);
  Initialize();
  MakeTags();
  MakePads();
};


VPScaleBar::VPScaleBar(float r,bool adj,bool ed,ColorScale* c,
	   int b,int h, int w)
: PScaleBar(r,adj,ed) {
  bar = new VPBar(c,b,h,w);
  ivResource::ref(bar);
  Initialize();
  MakeTags();
  MakePads();
}

DAStepper* VPScaleBar::GenEnlarger() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
      ivTelltaleState* ess = 
	new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
			  ivTelltaleState::is_choosable);
  return (new ParameterStepper
	  (wkit->up_mover_look(ess),
	   wkit->style(),ess,
	   new IntActionCallback
	   (ScaleBar)(this,&ScaleBar::Incr_Range)));
}

DAStepper* VPScaleBar::GenShrinker() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* sss = 
    new ivTelltaleState(ivTelltaleState::is_enabled_visible ||
		      ivTelltaleState::is_choosable);
  return(new ParameterStepper
	 (wkit->down_mover_look(sss),
	  wkit->style(),sss,
	  new IntActionCallback
	  (ScaleBar)(this,&ScaleBar::Decr_Range)));
}

ivGlyph* VPScaleBar::GetLook(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivLayoutKit* layout = ivLayoutKit::instance();
  if(adjustflag) {
    if(sm == RANGE) {
      scalebar =
	(layout->vbox
	 (layout->hcenter
	  (layout->hflexible(layout->natural(enlarger,32,20)),0),
	  layout->hcenter(min_label,0),
	  layout->hcenter
	  (wkit->outset_frame
	   (layout->hflexible(bar->GetBar())),0),
	  layout->hcenter(max_label,0),
	  layout->hcenter
	  (layout->hflexible(layout->natural(shrinker,32,20)),0)
	  ));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(min_incr,32,20),0),
	     layout->hcenter(layout->natural(min_decr,32,20),0)),0),
	   layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
			   (layout->hflexible(bar->GetBar())),0),
	   layout->hcenter(max_label,0),
	   layout->hcenter
	   (layout->vbox
	    (layout->hcenter(layout->natural(max_incr,32,20),0),
	     layout->hcenter(layout->natural(max_decr,32,20),0)),0)
	   )));
    }
  }
  else {
    if(sm == RANGE) {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
                           (layout->hflexible(bar)),0),
	   layout->hcenter(max_label,0)
	  )));
    }
    else {
      scalebar =
	(wkit->inset_frame
	 (layout->vbox
	  (layout->hcenter(min_label,0),
	   layout->hcenter(wkit->outset_frame
                           (layout->hflexible(bar)),0),
	   layout->hcenter(max_label,0)
	    )));
    }
  }
  return GenIH(scalebar);
}

DynamicBackground::DynamicBackground(ivGlyph* body, const ivColor* c) : ivMonoGlyph(body) {
    color_ = c;
    ivResource::ref(color_);
}

DynamicBackground::~DynamicBackground() {
    ivResource::unref(color_);
}

void DynamicBackground::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
    ivMonoGlyph::allocate(c, a, ext);
    ext.merge(c, a);
}

void DynamicBackground::draw(ivCanvas* c, const ivAllocation& a) const {
    ivExtension ext;
    ext.set(c, a);
    if (c->damaged(ext)) {
	c->fill_rect(a.left(), a.bottom(), a.right(), a.top(), color_);
    }
    ivMonoGlyph::draw(c, a);
}

void DynamicBackground::print(ivPrinter* p, const ivAllocation& a) const {
    p->fill_rect(a.left(), a.bottom(), a.right(), a.top(), color_);
    ivMonoGlyph::print(p, a);
}

void DynamicBackground::SetColor(ivColor* c){
  ivResource::ref(c);
  ivResource::unref(color_);
  color_ = c;
}



ColorPad::ColorPad(ScaleBar* tsb, BlockFill s,int w, int h,char *nm) 
: ivMonoGlyph(NULL) {
  sb = tsb;
  sb->padlist.Add(this);
  height = h;
  width = w;
  padval = oldpadval = 0;
  fill_type = s;
  thepatch = new ivPatch(nil);
  body(thepatch);
  if(nm !=  NULL) name = nm;
  ReFill();
};

ColorPad::~ColorPad(){
}

void ColorPad::GetColors(){
  if (fill_type == COLOR){
    sb->GetColor(padval,&fg,&tc);
    bg = tc;
  }
  else {
    if(padval > sb->max) {
      fg = sb->bar->scale->maxout.color;
      tc = sb->bar->scale->maxout.contrastcolor;
    }
    else if (padval < sb->min) {
      fg = sb->bar->scale->minout.color;
      tc = sb->bar->scale->minout.contrastcolor;
    }
    else if(padval >= sb->zero) {
      fg = sb->GetColor(-1);
      tc = sb->GetContrastColor(-1);
    }
    else {
      fg = sb->GetColor(0);
      tc = sb->GetContrastColor(0);
    }
    bg = sb->GetColor(sb->zero);
  }
}



void ColorPad::ReFill(){
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  thename = NULL;
  GetColors();
  float percent = 0.0f;
  if(fill_type != COLOR) percent = sb->GetAbsPercent(padval);
  switch (fill_type) {
  case COLOR:   theback = new DynamicBackground(NULL,fg); break;
  case AREA:    theback = new Spiral(percent,fg,bg,width,height);
  case LINEAR:  theback = new Spiral(percent*percent,fg,bg,width,height);
  }
  if(name.empty())  // no text
    thepatch->body(layout->fixed(theback,width,height));
  else {
    const ivFont* fnt = wkit->font();
    ivFontBoundingBox b;
    String newname = name;
    int len = newname.length();
    fnt->string_bbox(newname.chars(), len, b);
    while((len >1) && (b.width() > width)){
      len--;
      fnt->string_bbox(newname.chars(), len, b);
    }
    newname = newname.before(len);
    thename = new DynamicLabel((char *) newname,fnt,tc);
    thepatch->body(layout->fixed
		   (layout->overlay
		    (theback,
		     layout->vcenter(taivM->wrap(thename),0)),
		    width,height));
  }
}
void ColorPad::Redraw(){
  thepatch->reallocate();
  thepatch->redraw();
}
void ColorPad::SetFillType(BlockFill b){
  fill_type = b;
  ReFill();
  Redraw();
}

void ColorPad::Reset(){
  GetColors();
  float percent = 0.0f;
  if(fill_type != COLOR) percent = sb->GetAbsPercent(padval);
  switch(fill_type) {
  case COLOR:  ((DynamicBackground *) theback)->SetColor(fg); break;
  case AREA:   ((Spiral *) theback)->Set(percent,fg,bg); break;
  case LINEAR: ((Spiral *) theback)->Set(percent * percent,fg,bg);
  }
  if(thename != NULL){
    if(thename != NULL) thename->SetColor(tc);
  }
  thepatch->redraw();
  ivResource::flush();
}

void ColorPad::Set(float val) {
  if(padval == val) return; else oldpadval = padval;
  padval = val;
  Reset();
}

float ColorPad::GetVal(){
  return padval;
}

void ColorPad::Toggle() {
  if(padval == ((PScaleBar *) sb)->GetSelectedVal()){
    Set(oldpadval);
  }
  else {
    Set(((PScaleBar *) sb)->GetSelectedVal());
  }
}


PadButton::PadButton(ivStyle* s,PScaleBar* sb,
		     ColorPad::BlockFill sh,int w, int h)
: ivActiveHandler(cp = new ColorPad(sb,sh,w,h),s) {
}

void PadButton::press(const ivEvent&) {
  cp->Toggle();
}

ivCursor* PaintPad::painter_cursor;

PaintPad::PaintPad(ColorPad* c,void* o, void (*cn)(void*))
: ivInputHandler(new ivPatch(c),  ivWidgetKit::instance()->style()) {
  pb = c;
  obj = o;
  change_notify = cn;
  if (painter_cursor == NULL) {
    ivBitmap* painter = new ivBitmap(painter_bits, painter_width, painter_height,
				painter_x_hot, painter_y_hot);
    ivBitmap* painter_mask = new ivBitmap(painterMask_bits, painterMask_width, painterMask_height,
				     painterMask_x_hot, painterMask_y_hot);
    painter_cursor = new ivCursor(painter, painter_mask);
  }    
}


void PaintPad::release(const ivEvent& e){
  ivWindow* win = e.window();
  if (win->cursor() == painter_cursor) {
    win->pop_cursor();
  }
}


void PaintPad::press(const ivEvent& e) {
  ivInputHandler::press(e);
  pb->Toggle();
  if(change_notify != NULL) change_notify(obj);
  ivWindow* win = e.window();
  if (win->cursor() != painter_cursor) {
    win->push_cursor();
    win->cursor(painter_cursor);
  } 
};

void PaintPad::drag(const ivEvent& e) {
  ivInputHandler::drag(e);
  ivHandler* h = handler();
  e.ungrab(h);
};

void PaintPad::move(const ivEvent& e) {
  if(e.left_is_down()) {
    pb->Set(((PScaleBar *) pb->sb)->GetSelectedVal());
    if(change_notify != NULL) change_notify(obj);
  }
  else  if(e.middle_is_down()) {
    pb->Set(pb->oldpadval);
    if(change_notify != NULL) change_notify(obj);
  }
  else {
    // turn off painter cursor if mouse is not down
    ivWindow* win = e.window();
    if (win->cursor() == painter_cursor) {
      win->pop_cursor();
    }
  }
}


//////////////////////////
// 	CachePatch	//
//////////////////////////
 
// the cache patch always requests its first request.
// it is used for the linespace/axes box which changes size
// see docs/graphlogviewnotes for details

CachePatch::CachePatch(ivGlyph* body) : ivPatch(body) {
  impl = new CachePatch_Impl;
  impl->fakeflag = CachePatch_Impl::FIRST;
}

void CachePatch::request(ivRequisition& r) const {
  switch(impl->fakeflag) {
  case CachePatch_Impl::FIRST:
    ivPatch::request(r); // requset and store
    impl->oldreq = r;
    impl->fakeflag = CachePatch_Impl::ALWAYS;
    break;
  case CachePatch_Impl::ONCE:
    ivPatch::request(r); // request but don't store
    impl->fakeflag = CachePatch_Impl::ALWAYS;
    break;
  case CachePatch_Impl::ALWAYS:
    r = impl->oldreq; // use initial request
    break;
  }
  return;
}
