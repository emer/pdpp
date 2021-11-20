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

// colorscale.cc

#include <ta_misc/colorscale.h>

#include <ta/enter_iv.h>
#include <InterViews/color.h>
#include <InterViews/brush.h>
#include <InterViews/canvas.h>
#include <InterViews/bitmap.h>
#include <InterViews/session.h>	// for RGBA name lookup
#include <IV-look/kit.h>
#include <OS/math.h>
#include <ta/leave_iv.h>


//////////////////////////
//	RGBA		//
//////////////////////////

RGBA::RGBA(float rd, float gr, float bl, float al) {
  Register(); Initialize(); SetDefaultName();
  r = rd; g = gr; b = bl; a = al;
}

void RGBA::Initialize() {
  r = g = b = 0.0f;
  a = 1.0f;
  name = "";
}

void RGBA::UpdateAfterEdit(){
  taNBase::UpdateAfterEdit();
  if(!taMisc::iv_active)    return;
  if(!name.empty()){
    if(!(ivColor::find
	 (ivSession::instance()->default_display(),(char *)name,r,g,b))){
      taMisc::Error("Color: " , name , " not found for this display");
    }

  }
}

void RGBA::Copy_(const RGBA& cp) {
  name = cp.name;		// copy name because it goes with r,g,b
  r = cp.r;
  g = cp.g;
  b = cp.b;
  a = cp.a;
}


// set the color to a new color based on the values given
bool TAColor::SetColor(RGBA* c, RGBA* background,RGBA* cc){
  if(cc != NULL)
    return SetColor(c->r, c->g, c->b, c->a, background,
		    cc->r, cc->g, cc->b, cc->a);
  else 
    return SetColor(c->r, c->g, c->b, c->a, background);
}

// set the color to a new color based on the values given
bool TAColor::SetColor(float r, float g, float b, float a, RGBA* background,
		       float cr, float cg, float cb, float ca){
  float 	dr=1.0,dg=1.0,db=1.0;
  float		nr=1.0,ng=1.0,nb=1.0,na=1.0;
  int changes = false;
  if(color != NULL) { // there was an old color so let's compare
    color->intensities(dr,dg,db);
    if((r != dr)||(g != dg)||(b != db)||(a != color->alpha())){
      changes = true;
    }
  }
  else changes = true;
  if(changes == true) {
    ivResource::unref(color);
    color = new ivColor(r,g,b,a);
    ivResource::ref(color);
  }

  // compute contrast color
  
  if(cr != -1) { // contrast color specified
    ivResource::unref(contrastcolor);
    contrastcolor = new ivColor(cr,cg,cb,ca);
    ivResource::ref(contrastcolor);
  }
  else {
    // first get background color from specified background and its
    // relationship to the wkit's background

    float bgc = 0.0;
    if(background !=  NULL) 
      bgc = ((background->r + background->g + background->b)/3.0f) *  background->a;
    ivWidgetKit::instance()->background()->intensities(dr,dg,db);
    bgc += ((dr+dg+db)/3.0f) * ((background == NULL) ? 1.0 :
				(1.0 - background->a));

    // now get the intensity of the foreground color

    color->intensities(nr,ng,nb);
    na = color->alpha();
    // compute the ratio of the background color to the foreground color
    float bw = ((nr+ng+nb)/3.0f) * na + (bgc * (1.0f-na));
    
    if(bw >= 0.5f)        // color is mostly white
      nr = ng = nb = 0.0; // make contrast black
    else                  // color is mostly black
      nr = ng = nb = 1.0; // make contrast white
    if(contrastcolor != NULL) { // we already have a color
      contrastcolor->intensities(dr,dg,db);
    }
    if((contrastcolor == NULL) ||
       (dr != nr) || (dg != ng) || (db != nb)) { // no color or different color
      ivResource::unref(contrastcolor);
      contrastcolor =  new ivColor(nr,ng,nb,1.0);
      ivResource::ref(contrastcolor);
    }
  }
  return changes;
}

// set the color to the color given

bool TAColor::SetColor(ivColor* c, RGBA* background, ivColor* cc){
  float 	dr=-1,dg=-1,db=-1;
  float		nr=-1,ng=-1,nb=-1,na=1.0;
  int changes = false;
  if(c == NULL) return changes;
  if(color != NULL) { // there was an old color so let's compare
    color->intensities(dr,dg,db);
    c->intensities(nr,ng,nb);
    if((dr != nr)||(dg != ng)||(db != nb)||(c->alpha() != color->alpha())){
      changes = true;
    }
  }
  else changes = true;
  ivResource::unref(color);
  color = c;
  ivResource::ref(color);
  if(cc != NULL) { // contrast color specified
    ivResource::unref(contrastcolor);
    contrastcolor = cc;
    ivResource::ref(contrastcolor);
  }
  else {
    // first get background color from specified background and its
    // relationship to the wkit's background

    float bgc = 0.0;
    if(background !=  NULL) 
      bgc = ((background->r + background->g + background->b)/3.0f) *  background->a;
    ivWidgetKit::instance()->background()->intensities(dr,dg,db);
    bgc += ((dr+dg+db)/3.0f) * ((background == NULL) ? 1.0 :
				(1.0 - background->a));

    // now get the intensity of the foreground color

    color->intensities(nr,ng,nb);
    na = color->alpha();

    // compute the ratio of the background color to the foreground color
    float bw = ((nr+ng+nb)/3.0f) * na + (bgc * (1.0f-na));

      if(bw > 0.5f)        // color is mostly white
	nr = ng = nb = 0.0; // make contrast black
      else                  // color is mostly black
	nr = ng = nb = 1.0; // make contrast white
    if(contrastcolor != NULL) { // we already have a color
      contrastcolor->intensities(dr,dg,db);
    }
    if((contrastcolor == NULL) ||
       (dr != nr) || (dg != ng) || (db != nb)) { // no color or different color
      ivResource::unref(contrastcolor);
      contrastcolor =  new ivColor(nr,ng,nb,1.0);
      ivResource::ref(contrastcolor);
    }
  }
  return changes;
}

void TAColor::Destroy()	{
  if(color != NULL) ivResource::unref(color);
  if(contrastcolor != NULL) ivResource::unref(contrastcolor);
  color = NULL;
  contrastcolor=NULL;
}

//////////////////////////
//   ColorScaleSpec	//
//////////////////////////

void ColorScaleSpec::Initialize() {
  clr.SetBaseType(&TA_RGBA);
}

void ColorScaleSpec::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(background, this);
  taBase::Own(clr, this);
}

void ColorScaleSpec::Copy_(const ColorScaleSpec& cp) {
  background = cp.background;
  clr = cp.clr;
}

void ColorScaleSpec::GenRanges(TAColor_List* cl, int chunks) {
  // for odd colors, assuming 5 clrs and 16 chunks:
  // chunk: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
  // color: 0        1           2           3           4
  // range: <-   0  -><-    1   -><-    2   -><-    3   ->

  // for even colors, assuming 4 clrs and 14 chunks:
  // chunk: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
  // color: 0           1              2              3
  // range: <-    0    -><-     1    -><-     2      ->

  int n_range = clr.size-1;		// number of ranges
  int n_per = (chunks-1) / n_range; 	// number of chunks per range
  if(n_per <= 1) {
    n_per = 1;
  }

  cl->RemoveAll();
  int i;
  for(i=0; i< n_range; i++) {
    RGBA* low_color = clr[i];
    RGBA* hi_color = clr[i+1];
    float dr = (hi_color->r - low_color->r) / ((float) n_per);
    float dg = (hi_color->g - low_color->g) / ((float) n_per);
    float db = (hi_color->b - low_color->b) / ((float) n_per);
    float da = (hi_color->a - low_color->a) / ((float) n_per);
      
    int j;
    for(j=0; j<n_per; j++) {
      TAColor* pc = (TAColor *) cl->New(1,&TA_TAColor);
      float pcr = low_color->r + (dr * (float)j);
      float pcg = low_color->g + (dg * (float)j);
      float pcb = low_color->b + (db * (float)j);
      float pca = low_color->a + (da * (float)j);
      pc->SetColor(pcr,pcg,pcb,pca,&background);
    }
  }
  TAColor* pc = (TAColor*)cl->New(1,&TA_TAColor);
  RGBA* hi_color = clr.Peek();
  pc->SetColor(hi_color, &background);
}


//////////////////////////
//	ColorScale	//
//////////////////////////

void ColorScale::Initialize() {
  chunks = 0;
  spec = NULL;
  owner = NULL;
  background.color = NULL;
  maxout.color = NULL;
  minout.color = NULL;
  nocolor.color = NULL;
  colors.SetBaseType(&TA_TAColor);
}

void ColorScale::Destroy() {
  CutLinks();
  background.Destroy();
  maxout.Destroy();
  minout.Destroy();
  nocolor.Destroy();
}

void ColorScale::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(colors, this);
  NewDefaults();
}

void ColorScale::CutLinks() {
  // don't want to close edits and purge dialogs, cause flush is generated at bad times..
//  taNBase::CutLinks();
  taBase::DelPointer((TAPtr*)&spec);
}

ColorScale::ColorScale(int chunk) {
  Register();
  chunks = chunk;
  spec = NULL;
  SetDefaultName();
}

void ColorScale::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  MapColors();
  if(owner != NULL)
    owner->UpdateAfterEdit();
}

void ColorScale::MapColors() {
  if(spec == NULL)
    return;
  background.SetColor(spec->background.r, spec->background.g,
		      spec->background.b, spec->background.a);
  // ensure that chunks evenly fit with number of colors
  int n_range = spec->clr.size-1;
  if(chunks % n_range != 1) {
    chunks += (n_range - (chunks % n_range)) + 1;
  }
  spec->GenRanges(&colors, chunks);

  maxout.SetColor(new ivColor(*GetColor(colors.size-1),.25), &spec->background);
  minout.SetColor(new ivColor(*GetColor(0),.25), &spec->background);
#ifdef CYGWIN
  // do not set alpha level on colors because it behaves differently than solid
  // colors -- does not appear to be reset or something, causing wrong units to 
  // be highlighted in network viewer.
  nocolor.SetColor(new ivColor(*GetColor((colors.size+1)/2)),&spec->background);
#else
  nocolor.SetColor(new ivColor(*GetColor((colors.size+1)/2),.25),&spec->background);
#endif
}

void ColorScale::DefaultChunks(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  String guiname = wkit->gui();
  if(guiname == "monochrome")
    chunks = taMisc::mono_scale_size;
  else
    chunks = taMisc::color_scale_size;
}

void ColorScale::NewDefaults() {
  MemberDef* md;
  String tmpath = ".ColorScaleSpec_MGroup";
  ColorScaleSpec_MGroup* gp = 
    (ColorScaleSpec_MGroup*)tabMisc::root->FindFromPath(tmpath, md);
  if((gp == NULL) || (md == NULL)) {
    taMisc::Error("ColorScaleSpec_MGroup not found from root in NewDefaults");
    return;
  }
  gp->NewDefaults();
  taBase::SetPointer((TAPtr*)&spec, (ColorScaleSpec*)gp->DefaultEl());
  DefaultChunks();
  MapColors();
}

const ivColor* ColorScale::Get_Background(){
  return background.color;
}

ivColor* ColorScale::GetColor(int idx) {
  if((idx >= 0) && (idx < colors.size)) {
    return (ivColor *) ((TAColor *) colors[idx])->color;
  }
  else {
    return NULL;
  }
}

ivColor* ColorScale::GetContrastColor(int idx) {
  if((idx >= 0) && (idx < colors.size)) {
    return (ivColor *) ((TAColor *) colors[idx])->contrastcolor;
  }
  else {
    return NULL;
  }
}


//////////////////////////////////
// 	ColorScaleSpec_MGroup	//
//////////////////////////////////

void ColorScaleSpec_MGroup::NewDefaults() {
  if(size != 0)
    return;

  ColorScaleSpec* cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_ColdHot";
  cs->clr.Add(new RGBA(0.0, 1.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.5, 0.5, 0.5));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_ColdHotPurple";
  cs->clr.Add(new RGBA(1.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.5, 0.5, 0.5));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_BlueBlackRed";
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.3, 0.3, 0.3));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_BlueGreyRed";
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.5, 0.5, 0.5));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_BlueWhiteRed";
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.9, 0.9, 0.9));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_BlueGreenRed";
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.9, 0.0));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_Rainbow";
  cs->clr.Add(new RGBA(1.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 1.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_ROYGBIV";
  cs->clr.Add(new RGBA(1.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.5));
  cs->clr.Add(new RGBA(0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 1.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_DarkLight";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "C_LightDark";
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->background.name = "grey64";
  cs->background.UpdateAfterEdit();
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "M_DarkLight";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 0.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;
  cs->background.a = 1.0f;
  
  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "M_LightDark";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 1.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;
  cs->background.a = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "M_LightDarkLight";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0, 0.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;
  cs->background.a = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "P_DarkLight";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "P_DarkLight_brite";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(.3, .3, .3));
  cs->clr.Add(new RGBA(.6, .6, .6));
  cs->clr.Add(new RGBA(.8, .8, .8));
  cs->clr.Add(new RGBA(1, 1, 1));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "P_LightDark";
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "P_DarkLightDark";
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;

  cs = (ColorScaleSpec*)NewEl(1, &TA_ColorScaleSpec);
  cs->name = "P_LightDarkLight";
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->clr.Add(new RGBA(0.0, 0.0, 0.0));
  cs->clr.Add(new RGBA(1.0, 1.0, 1.0));
  cs->background.r = 1.0f;
  cs->background.g = 1.0f;
  cs->background.b = 1.0f;

}

void ColorScaleSpec_MGroup::SetDefaultColor() {
  NewDefaults();
  if(!taMisc::iv_active) return;
  ivWidgetKit* wkit = ivWidgetKit::instance();
  String guiname = wkit->gui();
  if(guiname == "monochrome")
    SetDefaultEl("M_DarkLight");
  else
    SetDefaultEl("C_ColdHot");
}
