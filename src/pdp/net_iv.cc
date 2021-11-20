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

// stuff to implement unit view..

#include <pdp/net_iv.h>
#include <pdp/pdpshell.h>
#include <pdp/stats.h>
#include <pdp/sched_proc.h>
#include <ta/ta_group_iv.h>

#include <ta_misc/picker.bm>
#include <ta_misc/picker_mask.bm>

#include <iv_graphic/graphic_viewer.h>
#include <iv_graphic/graphic_idraw.h>
#include <iv_graphic/graphic_objs.h>
#include <iv_misc/lrScrollBox.h>
#include <iv_misc/tbScrollBox.h>
#include <iv_misc/dastepper.h>

#include <ta/enter_iv.h>
#include <InterViews/brush.h>
#include <InterViews/color.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <InterViews/action.h>
#include <InterViews/box.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <InterViews/canvas.h>
#include <InterViews/session.h>
#include <InterViews/event.h>
#include <InterViews/style.h>
#include <InterViews/telltale.h>
#include <InterViews/geometry.h>
#include <InterViews/patch.h>
#include <InterViews/background.h>
#include <InterViews/cursor.h>
#include <InterViews/bitmap.h>
#include <InterViews/deck.h> // for hilit buttons
#include <InterViews/font.h>
#include <InterViews/polyglyph.h> // for height field neighbors
#include <IV-look/kit.h>
#include <IV-look/stepper.h>
#include <IV-look/bevel.h>
#include <OS/file.h>
#include <OS/math.h>
// for hier menu
#include <IV-look/menu.h>
#ifdef CYGWIN
#include <IV-Win/event.h>	// needed for setting window pointer of event
#endif
#include <ta/leave_iv.h>

#include <limits.h>
#include <float.h>
#include <unistd.h>

// netview stuff
const float NET_VIEW_INITIAL_X = 330.0f;
const float NET_VIEW_INITIAL_Y = 300.0f;
const float NET_VIEW_X_MARG = 8.0f;
const float NET_VIEW_Y_MARG = 16.0f;

// projection_g stuff
const float SELF_CON_DIST = .03f;

#define DIST(x,y) sqrt((double) ((x * x) + (y*y)))

//////////////////////////////////
//	   TDTransform		//
//////////////////////////////////

void TDTransform::Set(TDCoord& max, float skew, float fpct_x, float fpct_y, float fpct_z) {
  float adjx = max.x == 0 ? 1: max.x;
  float adjy = max.y == 0 ? 1: max.y;
  float adjz = max.z == 0 ? 1: max.z;
    
  scx = (1.0 - skew) / adjx;
  scz = 1.0 / adjz;
  dfx = skew / (scx * adjy);
  dfz = fpct_z / adjy;

  pt_x_1 = fpct_x * scx;
  pt_x_2 = fpct_x * scx + (fpct_y*skew/adjy);
  pt_x_3 = fpct_y * skew / adjy;
  pt_y_1 = fpct_y * scz * dfz;
}

void TDTransform::Project(int x, int y, int z, float& ix, float& iy) {
  ix = scx * ((float)x + ((float)y * dfx));
  iy = scz * ((float)z + ((float)y * dfz));
}

// this assumes z is given (fixed)
void TDTransform::InverseProjXY(float ix, float iy, int& x, int& y, int& z) {
  y = (int)(((iy / scz) - (float)z) / dfz);
  x = (int)((ix / scx) - ((float)y * dfx));
}

// this assumes y is given (fixed)
void TDTransform::InverseProjXZ(float ix, float iy, int& x, int& y, int& z) {
  z = (int) ((iy/scz) - (dfz * y));
  x = (int)((ix / scx) - ((float)y * dfx));
}

// this assumes x is given (fixed)
void TDTransform::InverseProjYZ(float ix, float iy, int& x, int& y, int& z) {
  y = (int) ((ix / (dfx * scx)) - (((float) x)/dfx));
  z = (int) ((iy/scz) - (dfz * y));
}


//////////////////////////////////
//	   Network_G		//
//////////////////////////////////

Network_G::Network_G(Network* n) {
  net = n;
  skew  = 0;

  unitbrush = NULL;

  defbrush = new ivBrush(BRUSH_SOLID,DEFAULT_BRUSH_WIDTH);
  ivResource::ref(defbrush);

  border = new ivColor(0.0, 0.0, 0.0, 1.0);
  ivResource::ref(border);

  reinit_display = false;
  cur_mbr = 0;
  update_scale_only = false;
  taBase::Own(selectgroup, net); // these are conceptually owned by the network..
  taBase::Own(pickgroup, net);	  // ditto
}


Network_G::~Network_G() {
//Reset(); // do not call this since there is a flush in Reset();
  selectgroup.RemoveAll(); // just do these two instead
  pickgroup.RemoveAll(); 
  membs.Reset();

  ivResource::unref(unitbrush); unitbrush = NULL;
  ivResource::unref(defbrush); defbrush = NULL;
  ivResource::unref(border);	border = NULL;
}

void Network_G::ScaleCenter(const ivAllocation& a) {
  ivAllocation b(a);
  ivAllotment& ax = _a.x_allotment();
  ivAllotment& ay = _a.y_allotment();
  ivAllotment& bx = b.x_allotment();
  ivAllotment& by = b.y_allotment();
  translate((bx.span()-ax.span())/2.0, (by.span()-ay.span())/2.0);
  translate(bx.begin()-ax.begin(), by.begin()-ay.begin());
}

void Network_G::ReCenter() {
  // clear out old allocation so network is rescaled/translated.
  ivAllocation a = _a; //temp for clearing
  ivAllocation b;
  _a = b; // zero's
  ScaleCenter(a);
  _a = a;
}

void Network_G::allocate (ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  if((owner != NULL) && (owner->owner != NULL) && (owner->owner->frozen_net_form != NULL)) {
    transformer(owner->owner->frozen_net_form->transformer());
    _a = a;
  }
  else {
    if (!_a.equals(a, 0.001)) {
      ScaleCenter(a);
      _a = a;
    }
  }
  if (c != nil) {
    PolyGraphic::allocate(c, a, ext);
  }
}

void Network_G::SetMyTransform(const ivAllocation& a) {
  if((owner != NULL) && (owner->owner != NULL) && (owner->owner->frozen_net_form != NULL)) {
    transformer(owner->owner->frozen_net_form->transformer());
  }
  else {
    transformer(NULL);
    no_text_extent = true;
    owner->viewer->init_graphic();
    no_text_extent = false;
    scale(a.x_allotment().span() - 2.0f * NET_VIEW_X_MARG,
	  a.y_allotment().span() - 2.0f * NET_VIEW_Y_MARG);
    ReCenter();
  }
}

void Network_G::SetMax(TDCoord& max) {
  max_size = max;
  
  tdp.Set(max_size, skew,
	  1.0f - owner->owner->spacing.x,
	  1.0f - owner->owner->spacing.y,
	  1.0f - owner->owner->spacing.z);
}

void Network_G::GetMembs() { // this fills a member group with the valid
			     // memberdefs from the units and connections
  membs.Reset();

  // first do the connections
  Layer* lay;
  taLeafItr l_itr;
  FOR_ITR_EL(Layer, lay, net->layers., l_itr) {
    Projection* prjn;
    taLeafItr p_itr;
    FOR_ITR_EL(Projection, prjn, lay->projections., p_itr) {
      TypeDef* td = prjn->con_type;
      int k;
      for(k=0; k<td->members.size; k++) {
	MemberDef* smd = td->members.FastEl(k);
	if(smd->type->InheritsFrom(&TA_float) || smd->type->InheritsFrom(&TA_double)) {
	  if((smd->HasOption("NO_VIEW") || smd->HasOption("HIDDEN") ||
	      smd->HasOption("READ_ONLY")))
	    continue;
	  String nm = "r." + smd->name;
	  if(membs.FindName(nm)==NULL) {
	    MemberDef* nmd = smd->Clone();
	    nmd->name = nm;
	    membs.Add(nmd);
	    nmd->idx = smd->idx;
	  }
	  nm = "s." + smd->name;
	  if(membs.FindName(nm)==NULL) {
	    MemberDef* nmd = smd->Clone();
	    nmd->name = nm;
	    membs.Add(nmd);
	    nmd->idx = smd->idx;
	  }
	}
      }
    }
  }

  // then do the unit variables
  FOR_ITR_EL(Layer, lay, net->layers., l_itr) {
    TypeDef* prv_td = NULL;
    Unit* u;
    taLeafItr u_itr;
    FOR_ITR_EL(Unit, u, lay->units., u_itr) {
      TypeDef* td = u->GetTypeDef();
      if(td == prv_td) continue; // don't re-scan!
      prv_td = td;
      int m;
      for(m=0; m<td->members.size; m++) {
	MemberDef* md = td->members.FastEl(m);
	if((md->HasOption("NO_VIEW") || md->HasOption("HIDDEN") ||
	    md->HasOption("READ_ONLY")))
	  continue;
	if((md->type->InheritsFrom(&TA_float) || md->type->InheritsFrom(&TA_double))
	   && (membs.FindName(md->name)==NULL))
	{
	  MemberDef* nmd = md->Clone();
	  membs.Add(nmd);	// index now reflects position in list...
	  nmd->idx = md->idx;	// so restore it to the orig one
	}	// check for nongroup owned sub fields (ex. bias)
	else if(md->type->DerivesFrom(&TA_taBase) && !md->type->DerivesFrom(&TA_taGroup)) {
	  if(md->type->ptr > 1) continue; // only one level of pointer tolerated
	  TypeDef* nptd;
	  if(md->type->ptr > 0) {
	    TAPtr* par_ptr = (TAPtr*)md->GetOff((void*)u);
	    if(*par_ptr == NULL) continue; // null pointer
	    nptd = (*par_ptr)->GetTypeDef(); // get actual type of connection
	  }
	  else
	    nptd = md->type;
	  int k;
	  for(k=0; k<nptd->members.size; k++) {
	    MemberDef* smd = nptd->members.FastEl(k);
	    if(smd->type->InheritsFrom(&TA_float) || smd->type->InheritsFrom(&TA_double)) {
	      if((smd->HasOption("NO_VIEW") || smd->HasOption("HIDDEN") ||
		  smd->HasOption("READ_ONLY")))
		continue;
	      String nm = md->name + "." + smd->name;
	      if(membs.FindName(nm)==NULL) {
		MemberDef* nmd = smd->Clone();
		nmd->name = nm;
		membs.Add(nmd);
		nmd->idx = smd->idx;
	      }
	    }
	  }
	}
      }
    }
  }

  cur_mbr = -1;
  if((owner != NULL) && (owner->owner != NULL)) {
    int_Array& oul = owner->owner->ordered_uvg_list;
    int i;
    for(i=oul.size-1; i>=0; i--) { // make sure no oul index is beyond number of membs
      if(oul[i] >= membs.size)
	oul.Remove((uint)i,1);
    }
    if(oul.size > 0)
      cur_mbr = oul.Peek();
  }
}


void Network_G::RemoveGraphics(){
  safe_damage_me(owner->viewer->canvas());
  long i;
  for(i=count_()-1; i>=0; i--) {
    if((component_(i)->GetTypeDef() == &TA_Layer_G) ||
       (component_(i)->GetTypeDef() == &TA_Projection_G) ||
       (component_(i)->GetTypeDef() == &TA_NoScale_Text_G)) {
      continue;
    }
    remove_(i);
  }
}
 
void Network_G::Reset() {
  long i;
  for(i=count_()-1; i>=0; i--) {
    if((component_(i)->GetTypeDef() == &TA_Layer_G) ||
       (component_(i)->GetTypeDef() == &TA_Projection_G) ||
       (component_(i)->GetTypeDef() == &TA_NoScale_Text_G)) {
       // what else could there be? idraw graphics
      remove_(i);
    }
  }
  selectgroup.RemoveAll();
  pickgroup.RemoveAll();
  ivResource::flush(); // clear our all defered unrefs.
}

void NetworkGSelectEffect(void* obj){
  ((Network_G*) obj)->owner->FixEditorButtons();
}

ivColor*  NetworkGGetLabelColor(void* obj){
  return ((Network_G *) obj)->GetLabelColor();
}

void Network_G::Build() {
  // add the general labels here
  if((owner ==NULL) || (owner->owner ==  NULL)) return;
  NetView* nv = (NetView *) owner->owner;

  if(unitbrush != NULL)
    ivResource::unref(unitbrush);

  if(nv->unit_border < 0.0)
    unitbrush = new ivBrush(BRUSH_DOT,0);
  else
    unitbrush = new ivBrush(BRUSH_SOLID,nv->unit_border);
  ivResource::ref(unitbrush);

  Reset();			// clear out exiting layers here
  SetMax(net->max_size);
  GetMembs();

  Layer_G* lg;

  Layer* lay;
  taLeafItr i;
  FOR_ITR_EL(Layer, lay, net->layers., i) {
    if(lay->own_net == NULL) continue; // indication that layer is just about to be removed!
    lg = new Layer_G(lay, this);
    append_(lg);
    lg->Build();
  }

  // projections

  FOR_ITR_EL(Layer, lay, net->layers., i) {
    if(lay->own_net == NULL) continue;
    Projection* pjn;
    taLeafItr j;
    FOR_ITR_EL(Projection, pjn, lay->projections., j) {
      if(pjn->owner == NULL) continue; // not really there!
      // get from_g;
      if(pjn->from == NULL) pjn->SetFrom();
      if(pjn->from == NULL) continue; // if no from layer skip this one
      if(pjn->from->own_net == NULL) continue;
      // okay if its not already there create it
      int exists = false;
      Projection_G* prg;
      int m;
      for(m=0;m<count_();m++){
	prg = (Projection_G *) component_(m);
	if((prg->InheritsFrom(&TA_Projection_G)) && (prg->pjn == pjn)) {
	  exists = true; break;
	}
      }
      if (exists) continue;
      Projection_G * pg = new Projection_G;
      Layer_G* from_layer_g = NULL;
      for(m=0;m<count_();m++){
	from_layer_g = (Layer_G *) component_(m);
	if((from_layer_g->InheritsFrom(&TA_Layer_G)) &&
	   (from_layer_g->lay == pjn->from))  break;
      }
      Layer_G* to_layer_g = NULL;
      for(m=0;m<count_();m++){
	to_layer_g = (Layer_G *) component_(m);
	if((to_layer_g->InheritsFrom(&TA_Layer_G)) &&
	   (to_layer_g->lay == pjn->layer))  break;
      }
      pg->NewProjection
	(pjn,to_layer_g,from_layer_g,this);
      // make sure projection is after idraw graphics and before layers
      int pindex = 0;
      if(owner->owner->has_graphic){
	for(pindex = 0; pindex < count_();pindex++) {
	  if(component_(pindex)->InheritsFrom(TA_Layer_G)){
	    insert_(pindex,pg); break;
	  }
	}
      }
      else prepend_(pg);

      // check to see if there is an recip connection and if so build it
      // now instead of later so that the projections are sidebyside
      if(pjn->from == lay) continue; // skip self prjns
      Projection * rpjn;
      taLeafItr n;
      FOR_ITR_EL(Projection, rpjn, pjn->from->projections., n) {
	if(rpjn->owner == NULL) continue; // not really there!
	if(rpjn->from == NULL) rpjn->SetFrom();
	if(rpjn->from != lay) continue;
	// we have a recip prjn, if it's not already there then create it
	int exists = false;
	int m;
	for(m=0;m<count_();m++){
	  prg = (Projection_G *) component_(m);
	  if((prg->InheritsFrom(&TA_Projection_G)) && (prg->pjn == rpjn)) {
	    exists = true; break; // already here
	  }
	}
	if (exists) continue;
	Projection_G * pg = new Projection_G;
	pg->NewProjection(rpjn,from_layer_g,to_layer_g,this);
	if(owner->owner->has_graphic)
	  insert_(pindex,pg);
	else prepend_(pg);

      }
    }
  }
  int v;
  for(v=0; v < nv->labels.size; v++) {
    NetViewLabel* vl = (NetViewLabel*)nv->labels.FastEl(v);
    vl->select_effect = NetworkGSelectEffect; // reset non saved function pointers
    vl->get_color = NetworkGGetLabelColor; // reset non saved function pointers
    vl->MakeText();
  }
  ivExtension ext;
  if((owner!= NULL) && (owner->viewer != NULL) &&
     (owner->viewer->canvas() != NULL)) {
    allocate(owner->viewer->canvas(),_a,ext);
    safe_damage_me(owner->viewer->canvas());
  }
  owner->FixEditorButtons();
  RePick(); // set UnitValue_G's accordingly
}

ivColor* Network_G::GetLabelColor() {
  return owner->scale->background.contrastcolor;
}

bool Network_G::LoadGraphic(const char* nm){
  // puts the graphic at the begining
  PolyGraphic* g = NULL;
  g = IdrawReader::load(nm);
  if(g == NULL) {
    int i;
    for(i=0; i<taMisc::include_paths.size; i++) {
      String trynm = taMisc::include_paths.FastEl(i) + "/" + nm;
      g = IdrawReader::load(trynm.chars());
      if (g != NULL) break;
    }
  }
  if (g == NULL) {
    taMisc::Error("Unable to find graphic file:", nm);
    return false;
  }
  NetViewGraphic_G* netvg = new NetViewGraphic_G(NULL,NULL);
  netvg->prepend_(g);
  prepend_(netvg);
  float net_scale = 1.0 / 500.0;
  g->scale(net_scale, net_scale, 150, 0);
  safe_damage_me(owner->viewer->canvas()); // redraw
  return true;
}

void Network_G::RePick(){
  int i;
  for(i=0; i<count_(); i++) {
    if(component_(i)->GetTypeDef() == &TA_Layer_G) {
      (( Layer_G*)component_(i))->RePick();
    }
  }
}

void Network_G::FixProjection(Projection* pjn, int index){
  Projection_G* pg = NULL;
  // find the projection_g
  if( index != -1) {
    pg =  (Projection_G *) component_(index);
  }
  if(index == -1 || pg->pjn != pjn){ // find the index
    int i;
    Projection_G * prg;
    for(i=0;i<count_();i++){
      prg = (Projection_G *) component_(i);
      if ((prg->InheritsFrom(&TA_Projection_G)) && (prg->pjn == pjn)) {
	pg = prg; index = i; break;
      }
    }
  }

  // find the from layer_g
  if(pjn->from == NULL) pjn->SetFrom();
  if(pjn->from == NULL) return;
  Layer* from_lay = pjn->from;
  ivGlyphIndex count = count_();
  Layer_G* from_g = NULL;
  Graphic* gr;
  int i;
  for(i=0;i<count;i++){
    if(((gr = component_(i))->InheritsFrom(&TA_Layer_G)) &&
       (((Layer_G *) gr)->lay == from_lay)){
      from_g = (Layer_G *) gr;
      break;
    }
  }
  if(from_g == NULL) return;

  //  find the to layer_g

  Layer_G* to_g = NULL;
  if(!((index ==-1) || (pg == NULL))) {
    to_g= pg->to_g;
  }
  if((pg != NULL) && (pg->from_g == from_g) && (pg->to_g == to_g)) { // no change
    // redraw the projection in case it was just connected
      pg->safe_damage_me(pg->net_g->owner->viewer->canvas());
    return;
  }
  if((pg!= NULL) &&
     ((from_g == to_g) || ( pg->from_g == pg->to_g))){ // self<->noself
    pg->RemoveMe();
    pg = NULL;
  }
  if(pg == NULL) { // create a new proj_g
    if((pg = new Projection_G) == NULL) return;
    Layer* to_layer = GET_OWNER(pjn,Layer);
    count = count_();
    for(i=0;i<count;i++){ 
      if(((gr = component_(i))->InheritsFrom(&TA_Layer_G)) &&
	 (((Layer_G *) gr)->lay == to_layer)){
	to_g = (Layer_G *) gr;
	break;
      }
    }
    if(to_g == NULL) { delete pg; return;}
    pg->NewProjection(pjn, to_g,from_g,this);
    // make sure projection is after idraw graphics and before layers
    if(owner->owner->has_graphic){
      int pindex;
      for(pindex = 0; pindex < count_();pindex++) {
	if(component_(pindex)->InheritsFrom(TA_Layer_G)){
	  insert_(pindex,pg); break;
	}
      }
    }
    else prepend_(pg);
    winbMisc::DamageCanvas(owner->viewer->canvas());
    return;
  }

  if(pg->from_g != from_g) { // update with the new from_g
    pg->undraw();
    pg->from_g->RemoveProj_G(pg);
    pg->from_g = from_g;
    pg->to_g->RemoveProj_G(pg);  // although the to_g hasn't changed, this
    pg->RecomputePoints(); // adds pg to the side so we must remove it 1st
    winbMisc::DamageCanvas(owner->viewer->canvas());
  }
}

void Network_G::RepositionPrjns() {
  int i;
  for(i=0;i<count_();i++) {
    if((component_(i)->InheritsFrom(&TA_Layer_G)))
      ((Layer_G*)component_(i))->RedistributeSide(-1);
  }
}

void Network_G::FixLayer(Layer* lay) {
  Layer_G* layg = NULL;
  int j;
  for(j=0;j<count_();j++){
    layg = (Layer_G *) component_(j);
    if(layg->InheritsFrom(&TA_Layer_G) && (layg->lay == lay)) break;
    layg = NULL;
  }
  if(layg != NULL) {
    layg->FixAll();
    winbMisc::DamageCanvas(owner->viewer->canvas());
  }
  else { // there is no layer glyph for this one so make one!
    layg = new Layer_G(lay,this);
    append_(layg);
    layg->Build();
    winbMisc::DamageCanvas(owner->viewer->canvas());
  }
}

void Network_G::FixUnit(Unit* u, Layer* lay) {
  int i;
  for(i = 0; i < count_(); i++)  {
    Layer_G* layg = (Layer_G *) component_(i);
    if(layg->InheritsFrom(&TA_Layer_G) && (layg->lay == lay)) {
      layg->FixUnit(u);
    }
  }
}

void Network_G::FixUnitGroup(Unit_Group* ug, Layer* lay) {
  int i;
  for(i = 0; i < count_(); i++)  {
    Layer_G* layg = (Layer_G *) component_(i);
    if(layg->InheritsFrom(&TA_Layer_G) && (layg->lay == lay)) {
      layg->FixUnitGroup(ug);
    }
  }
}

bool Network_G::update_from_state(ivCanvas* c) {
  // fix the scale
  if(owner->auto_scale) {
    update_scale_only = true;
    float org_min = owner->cbar->min;
    float org_max = owner->cbar->max;
    owner->cbar->ModRange(0); // set range to zero
    GraphicMaster::update_from_state(c);
    update_scale_only = false;
    // only update if its different..
    if((org_min != owner->cbar->min) || (org_max != owner->cbar->max))
      owner->cbar->UpdateScaleValues();
  }
  else {
    owner->cbar->SetMinMax(owner->owner->scale_min,
			   owner->owner->scale_max);
  }
  bool result = GraphicMaster::update_from_state(c);
  return result;
}

void Network_G::UpdateSelect(){
  // a fake event and tool for selecting
  ivEvent ev;
  ev.window(owner->win);
#ifdef CYGWIN
  // cygwin doesn't implement above call, so need to do this manually
  ev.rep()->windowSet(owner->win);
#endif
  Tool tl;

  // make a copy of the selectgroup;
  taBase_Group grp;
  int i;
  for(i=0;i<selectgroup.size;i++){
    grp.Link(selectgroup[i]);
  }

  // unselect everything
  int cnt = (int)count_();   // cache the count
  for(i=0;i<cnt;i++){
    component_(i)->select(ev,tl,true); // true means unselect
  }
  selectgroup.RemoveAll();

  // copy the group back
  for(i=grp.size-1;i>=0;i--){
    selectgroup.Link(grp[i]);
    grp.Remove(i);
  }
  for(i=0;i<selectgroup.size;i++) {
    TAPtr obj = (TAPtr) selectgroup[i];
    SelectObject_impl(obj);
  }
  owner->UpdateDisplay();
  owner->FixEditorButtons();
}

void Network_G::SelectObject(TAPtr o) {
  if(selectgroup.FindEl(o) >= 0)
    return;			// already selected!
  SelectObject_impl(o);
  if(selectgroup.size <= 2) {
    owner->FixEditorButtons();
  }
  else {
    owner->update_from_state();	// this obeys the display toggle..
  }
}  

void Network_G::SelectObject_impl(TAPtr o) {
  // a fake event and tool for selecting
  ivEvent ev;
  ev.window(owner->win);
#ifdef CYGWIN
  // cygwin doesn't implement above call, so need to do this manually
  ev.rep()->windowSet(owner->win);
#endif
  Tool tl;

  int j;
  for(j=0;j<(int)count_();j++) { 
    Graphic* gr = component_(j);
    if((o->InheritsFrom(&TA_ViewLabel)) &&
       (gr->InheritsFrom(&TA_NoScale_Text_G)) &&
       (((NoScale_Text_G *) gr)->obj == (ViewLabel*) o)){
      ((NoScale_Text_G *) gr)->select(ev,tl,false);
      break;
    }
    if((o->InheritsFrom(&TA_Layer)) &&
       (gr->InheritsFrom(&TA_Layer_G)) &&
       (((Layer_G* ) gr)->lay == (Layer *)o)) {
      ((Layer_G *) gr)->select(ev,tl,false);
      break;
    }
    if((o->InheritsFrom(&TA_Projection)) &&
       (gr->InheritsFrom(&TA_Projection_G)) &&
       (((Projection_G* ) gr)->pjn == (Projection *) o)) {
      ((Projection_G*) gr)->select(ev,tl,false);
      break;
    }
    if((o->InheritsFrom(&TA_Unit_Group) || o->InheritsFrom(&TA_Unit)) &&
       (gr->InheritsFrom(&TA_Layer_G) &&
	(((Layer_G *)gr)->lay == (Layer*)o->GetOwner(&TA_Layer))))
    {
      int g;
      for(g=0; g<gr->count_(); g++) {
	Graphic* lggp = gr->component_(g);
	if(!lggp->InheritsFrom(&TA_Unit_Group_G))
	  continue;
	Unit_Group_G* ugg = (Unit_Group_G *) lggp;
	if(o->InheritsFrom(&TA_Unit)) {
	  Unit* un= (Unit*)o;
	  if((Unit_Group*)un->GetOwner(&TA_Unit_Group) != ugg->group)
	    continue;
	  int unitcount = (int) ugg->count_();
	  int k;
	  for(k=0;k<unitcount;k++) {
	    Graphic* lgr = ugg->component_(k);
	    if((lgr->InheritsFrom(&TA_Unit_G)) &&
	       (((Unit_G *) lgr)->unit == (Unit *) o)){
	      ((Unit_G *) lgr)->select(ev,tl,false);
	      break;
	    }
	  }
	}
	else {
	  if((Unit_Group*)o != ugg->group) continue;
	  ugg->select(ev,tl,false);
	}
	break;
      }
      break;
    }
  }
}

void Network_G::UpdatePick(){
  // make a copy of the pickgroup;
  taBase_Group grp;
  int i;
  for(i=0;i<pickgroup.size;i++){
    grp.Link(pickgroup[i]);
  }

  ivEvent ev;
  ev.window(owner->win);
#ifdef CYGWIN
  // cygwin doesn't implement above call, so need to do this manually
  ev.rep()->windowSet(owner->win);
#endif
  Tool tl;
  // unpick everything
  int cnt = (int)count_();   // cache the count
  for(i=0;i<cnt;i++){
    component_(i)->pick_me(ev,tl,true); // true means unpick
  }
  pickgroup.RemoveAll();

  // copy the group back
  for(i=grp.size-1;i>=0;i--){
    pickgroup.Link(grp[i]);
    grp.Remove(i);
  }
  TAPtr o;
  int n;
  for(n=0; n<pickgroup.size; n++) {
    o = pickgroup.FastEl(n);
    int j;
    for(j=0;j<cnt;j++){
      Graphic* gr;
      gr = component_(j);
      if((o->InheritsFrom(&TA_Unit)) &&
	 (gr->InheritsFrom(&TA_Layer_G) &&
	  (((Layer_G *) gr)->lay == (Layer*)
	   (((Unit*) o)->GetOwner(&TA_Layer))))){
	Unit_Group_G* ugg = (Unit_Group_G *) gr->component_(0);
	int unitcount = (int) ugg->count_();
	int k;
	for(k=0;k<unitcount;k++) {
	  Graphic* lgr = ugg->component_(k);
	  if((lgr->InheritsFrom(&TA_Unit_G)) &&
	     (((Unit_G *) lgr)->unit == (Unit *) o)){
	    ((Unit_G *) lgr)->spread_pick(ev,tl,true);
	    break;
	  }
	}
	continue;
      }
      gr->pick_me(ev,tl,true); // turn off if not picked
    }
  }
  RePick();
  owner->UpdateDisplay();
  owner->FixEditorButtons();
}

bool Network_G::effect_select(bool set_select) {
  bool result = GraphicMaster::effect_select(set_select);
  if(reinit_display) {
    reinit_display = false;
    owner->InitDisplay();
  }
  return result;
}

bool Network_G::select(const ivEvent& e, Tool& tool, bool b) {
  bool result = GraphicMaster::select(e,tool,b);
  if(reinit_display) {
    reinit_display = false;
    owner->InitDisplay();
  }
  else {
    owner->FixEditorButtons();
  }
  return result;
}

bool Network_G::pick_me(const ivEvent& e, Tool& tool, bool b) {
  changed_pickgroup = false;
  bool result = GraphicMaster::pick_me(e,tool,b);
  // check to see if the the background was picked
  ivCoord tol = 2.0;
  BoxObj box(e.pointer_x()-tol, e.pointer_y()-tol, 
	     e.pointer_x()+tol, e.pointer_y()+tol);
  if(last_intersecting(box) == -1) { // no objects
    while(owner->owner->ordered_uvg_list.Remove(-1)); // remove all -1's
    owner->NumberVarButtons();
  }
  if(changed_pickgroup) { // pick group has changed
    RePick(); // update all the send/recv UnitBlock_G\'s
    safe_damage_me(owner->viewer->canvas()); // redraw all units
    owner->NumberVarButtons();
  }
  owner->UpdateDisplay();
  owner->FixEditorButtons();
  return result;
}

//////////////////////
// NetViewGraphic_G //
//////////////////////

NetViewGraphic_G::NetViewGraphic_G(NetViewGraphic* n, Graphic*g)
: PolyGraphic(g) {
  netvg = n;
}

NetViewGraphic_G::~NetViewGraphic_G(){
}

void NetViewGraphic_G::flush(){
  // don't flush!
}

bool NetViewGraphic_G::select(const ivEvent& e, Tool& tool, bool unselect){
  long i;
  for(i=0; i < count_(); i++)
    component_(i)->select(e, tool, unselect);

  if(unselect) {
    _selected = false;
    netvg->master->selectgroup.Remove(netvg);
  }
  else {
    _selected = !_selected;
    if(_selected)
      netvg->master->selectgroup.LinkUnique(netvg);
  }
  return true;
}

bool NetViewGraphic_G::effect_stretch(const ivEvent& e, Tool& t, ivCoord ix,ivCoord iy,
				      ivCoord fx, ivCoord fy) {
  bool result = PolyGraphic::effect_stretch(e,t,ix,iy,fx,fy);
  return result;
}

bool NetViewGraphic_G::effect_move(const ivEvent& e, Tool& t, ivCoord ix,ivCoord iy,
				   ivCoord fx, ivCoord fy) {
  bool result = PolyGraphic::effect_move(e,t,ix,iy,fx,fy);
  netvg->SetGraphicXform(transformer());
  return result;
}

bool NetViewGraphic_G::effect_scale(const ivEvent& e, Tool& t, ivCoord ix,ivCoord iy,
				  ivCoord fx, ivCoord fy, ivCoord ctrx, ivCoord ctry) {
  bool result = PolyGraphic::effect_scale(e,t,ix,iy,fx,fy,ctrx,ctry);
  netvg->SetGraphicXform(transformer());
  return result;
}

bool NetViewGraphic_G::effect_rotate(const ivEvent& e, Tool& t, ivCoord ideg, ivCoord fdeg,
				     ivCoord ctrx, ivCoord ctry) {
  bool result = PolyGraphic::effect_rotate(e,t,ideg,fdeg,ctrx,ctry);
  netvg->SetGraphicXform(transformer());
  return result;
}


//////////////////////////////////

NetViewLabelGroup_G::NetViewLabelGroup_G(Graphic*g)
: PolyGraphic(g) {
}

NetViewLabelGroup_G::~NetViewLabelGroup_G(){
}

bool NetViewLabelGroup_G::effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
				      ivCoord iy, ivCoord fx, ivCoord fy) {
  translate(ix-fx, iy-fy);	// undo our translation
  int i;
  int cnt = (int)count_();   // cache the count
  for(i=0;i<cnt;i++) {
    Graphic* vgr = component_(i);
    vgr->manip_move(ev, tl, ix, iy, ix, iy, fx, fy); // apply the translation to subguys
    vgr->effect_move(ev, tl, ix, iy, fx, fy);
  }
  return true;
}


//////////////////////////////////
//	   Layer_G		//
//////////////////////////////////

Layer_G::Layer_G(Layer* lyr, Network_G* ng){
  lay = lyr;
  netg = ng;
  Position();
  proj_gs = NULL;
}

Layer_G::~Layer_G() {
  if (proj_gs) {
    while(proj_gs->count() > 0){
      proj_gs->remove(0);
    }
    ivResource::unref(proj_gs);
  }
}

float Layer_G::GetCurrentXformScale(){
  if (transformer() == NULL) return 1;
  float a00,a01,a10,a11,a20,a21;
  transformer()->matrix(a00,a01,a10,a11,a20,a21);
  return a00; // x scale
}

float Layer_G::GetCurrentYformScale(){
  if (transformer() == NULL) return 1;
  float a00,a01,a10,a11,a20,a21;
  transformer()->matrix(a00,a01,a10,a11,a20,a21);
  return a11; // y scale
}

void Layer_G::RePick(){
  long i;
  for(i=0; i<count_(); i++) {
    if(component_(i)->GetTypeDef() == &TA_Unit_Group_G)
      ((Unit_Group_G*)component_(i))->RePick();
   }
}
  
ivGlyph* Layer_G::clone() const {
  return new Layer_G(lay, netg); 
}

void Layer_G::Position() {
  float pt_x_0, pt_y_0;
  netg->tdp.Project(lay->pos, pt_x_0, pt_y_0);
  translate(pt_x_0, pt_y_0);
}

LayerBox_G* Layer_G::GetActualLayerBox() {
  if(lay->units.gp.size > 0)
    return (LayerBox_G*)component_(count_()-3);
  else
    return (LayerBox_G*)component_(count_()-1);
}

void Layer_G::Build() {
  Xform* lxf = netg->owner->owner->GetLayerXform(lay);
  if(lxf != NULL) {
    transformer(lxf->transformer());
  }
  
  // first the units
  if(lay->units.gp.size > 0) {
    int g;
    Unit_Group* ug;
    for(g=lay->units.gp.size-1; g>=0; g--) { // go in reverse for 3d stuff!
      ug = (Unit_Group*)lay->units.gp.FastEl(g);
      Unit_Group_G* ng = new Unit_Group_G(ug,this);
      append_(ng);
      ng->Build();
    }
    // add a box representing the actual geometry (count_()-3 if here)
    ivCoord px[4],py[4];
    LayerBox_G* lb = new LayerBox_G
      (lay, &(lay->act_geom), netg, netg->defbrush,netg->border,NULL,px,py,4,NULL);
    lb->SetPointsFmGeom();
    lb->recompute_shape();
    append_(lb);
  }
  else {
    Unit_Group_G* ng;
    ng = new Unit_Group_G(&lay->units,this);
    append_(ng);
    ng->Build();
  }

  // then the layer text (count_()-2)
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivFont* fnt = ((NetView*)netg->owner->owner)->layer_font.fnt;
  LayerText_G* ltg =
    new LayerText_G(this, fnt, wkit->foreground(), (char*)lay->name, NULL);
  append_(ltg);

  // then the layer box (count_()-1)
  ivCoord px[4],py[4];
  LayerBox_G* lb = new LayerBox_G
    (lay, &(lay->geom), netg, netg->defbrush,netg->border,NULL,px,py,4,NULL);
  lb->SetPointsFmGeom();
  lb->recompute_shape();
  append_(lb);

  if(proj_gs != NULL) {
    while(proj_gs->count() > 0){
      proj_gs->remove(0);
    }
    ivResource::unref(proj_gs);
  }
  proj_gs = new ivPolyGlyph(); // holder for each side
  ivResource::ref(proj_gs);
  int j;
  for(j=0;j<5;j++) {     // one pg for each side + independent
    proj_gs->append(new ivPolyGlyph());
  }
  FixText();
}

void Layer_G::FixText() {
  // fix the text
  LayerText_G* ltg = (LayerText_G *) component_(count_()-2); // change the name
  ltg->text(lay->name);
  Xform* lxf = netg->owner->owner->GetLayerLabelXform(lay);
  if(lxf != NULL) {
    ltg->transformer(lxf->transformer());
  }
  else {
    ivFont* fnt = ((NetView*)netg->owner->owner)->layer_font.fnt;
    ivFontBoundingBox bbox;
    fnt->font_bbox(bbox);
    float fy = bbox.ascent() + bbox.descent();
    ltg->translate(5, -fy + 2);
  }
}

void Layer_G::FixBox() {
  lay->LayoutUnits();
  transformer(NULL);
  Position();
  LayerBox_G* lbg = (LayerBox_G *) component_(count_()-1);
  lbg->SetPointsFmGeom();
  LayerBox_G* alb = GetActualLayerBox();
  if(alb != lbg)
    alb->SetPointsFmGeom();
}

void Layer_G::FixUnitGroup(Unit_Group* ug) {
  int j;
  for(j=0; j<count_(); j++) {
    Unit_Group_G* ugg= (Unit_Group_G*) component_(j);
    if(ugg->InheritsFrom(TA_Unit_Group_G) && (ugg->group == ug))
      ugg->FixUnits();
  }
}

void Layer_G::FixUnits() {
  // fix the positions of the units
  lay->LayoutUnits();
  int j;
  for(j=0; j<count_()-2; j++) {
    Graphic* ugg= (Graphic*) component_(j);
    if(ugg->InheritsFrom(TA_Unit_Group_G))
      ((Unit_Group_G*)ugg)->FixUnits();
  }
}

void Layer_G::FixUnit(Unit* u) {
  Unit_Group* ug = (Unit_Group*)u->owner;
  int j;
  for(j=0; j<count_(); j++) {
    Unit_Group_G* ugg= (Unit_Group_G*) component_(j);
    if(ugg->InheritsFrom(TA_Unit_Group_G) && (ugg->group == ug))
      ugg->FixUnit(u);
  }
}

void Layer_G::draw_gs (ivCanvas* c, Graphic* gs) {
  PolyGraphic::draw_gs(c,gs);
}

bool Layer_G::select(const ivEvent& e, Tool& tool, bool unselect){
  long i;
  if(unselect)
    _selected = false;
  else
    _selected = !_selected;

  if(count_() > 0) { // there are subobjs
    LayerBox_G* lbg = (LayerBox_G*) component_(count_()-1);
    if(unselect == false) {
      if(lbg->is_selected()){
	lbg->select(e,tool,false);
      }
      else {
	lbg->select(e,tool,false);
	for(i=0; i < count_()-1; i++) {
	  component_(i)->select(e, tool, true);
	}
      }
    }
    else {
      if(lbg->is_selected()){
	lbg->select(e,tool,true);
      }
      else {
	for(i=0; i < count_()-1; i++) {
	  component_(i)->select(e, tool, unselect);
	}
      }
    }
  }
  return true;
}

bool Layer_G::grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord) {
  move_pos.Initialize();
  netg->owner->UpdatePosivPatch(lay->pos);
  netg->owner->FixEditorButtons();
  return true;
}

bool Layer_G::manip_move(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0))
    return true;

  dx = cx - ix;
  dy = cy - iy;
  TDCoord ng,dg;
  // layer movement is in y only with right mouse button for three_d!
  switch(netg->owner->viewer->last_button) {
  case(ivEvent::right):
    if((lay->own_net != NULL) && (lay->own_net->lay_layout == Network::TWO_D))
      netg->tdp.InverseProjXZ(dx, dy, ng);
    else
      netg->tdp.InverseProjXY(dx, dy, ng);
    break;
  default:
    if((lay->own_net != NULL) && (lay->own_net->lay_layout == Network::TWO_D))
      netg->tdp.InverseProjXY(dx, dy, ng);
    else
      netg->tdp.InverseProjXZ(dx, dy, ng);
    break;
  }
  dg = ng;
  dg -= move_pos;
  if((dg.x != 0) || (dg.y != 0) || (dg.z != 0)) {
    TDCoord updtpos = lay->pos + dg;
    if((updtpos.x <0) || (updtpos.y <0) || (updtpos.z <0)) {
      return true;
    }
    move_pos = ng;
    lay->pos.Invert();
    Position();
    lay->pos.Invert();
    PosTDCoord og = lay->pos;
    lay->pos += dg;
    Position();			// put into the new one
    netg->owner->UpdatePosivPatch(lay->pos);
  }
  return true;
}

bool Layer_G::effect_move(const ivEvent&,Tool&,ivCoord,
			     ivCoord,ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0) && (move_pos.z == 0)) return true;
  move_pos.Initialize();
  DMEM_GUI_RUN_IF {
    lay->LayoutUnits();
  }
  tabMisc::NotifyEdits(lay);
  netg->owner->ClearPosivPatch();
  netg->reinit_display = true;
  taivMisc::SRIAssignment(lay,lay->FindMember("pos"));
  return true;
}

void Layer_G::damage_me(ivCanvas *c){
  PolyGraphic::damage_me(c);
  int i;
  for(i=0;i<proj_gs->count();i++){
    ivPolyGlyph* pg = (ivPolyGlyph *) proj_gs->component(i);
    int j;
    for(j = 0; j<pg->count();j++){
      ((Graphic *) pg->component(j))->damage_me(c);
    }
  }
}

bool Layer_G::grasp_stretch(const ivEvent& , Tool& ,ivCoord, ivCoord) {
  move_pos.Initialize();
  netg->owner->UpdatePosivPatch(lay->geom);
  netg->owner->FixEditorButtons();
  return true;
}

bool Layer_G::manip_stretch(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0))
    return true;

  dx = cx - ix;
  dy = cy - iy;
  TDCoord ng,dg;
  switch(netg->owner->viewer->last_button) {
  case(ivEvent::right):
    netg->tdp.InverseProjXZ(dx, dy, ng); break;
  default:
    netg->tdp.InverseProjXY(dx, dy, ng); break;
  }
  dg = ng;
  dg -= move_pos;
  if((dg.x != 0) || (dg.y != 0) || (dg.z != 0)) {
    TDCoord updtgeom = lay->geom + dg;
    if(((updtgeom.x <0) || (updtgeom.y <1) || (updtgeom.z < 1))
       || ((updtgeom.x * updtgeom.y) < (int) lay->units.size)) {
      return true;
    }
    move_pos = ng;
    PosTDCoord og = lay->geom;
    lay->geom += dg;
    LayerBox_G* lbg = (LayerBox_G *) component_(count_()-1);
    lbg->SetPointsFmGeom();
    LayerBox_G* alb = GetActualLayerBox();
    if(alb != lbg)
      alb->SetPointsFmGeom();
    netg->owner->UpdatePosivPatch(lay->geom);
  }
  return true;
}

bool Layer_G::effect_stretch(const ivEvent&, Tool&,ivCoord,
				  ivCoord, ivCoord, ivCoord) {
  // always update n_units on resize, even if no change
  if((lay->n_units == 0) || (lay->units.size == 0))
    lay->n_units = lay->geom.x * lay->geom.y;
  move_pos.Initialize();
  netg->reinit_display = true;
  DMEM_GUI_RUN_IF {
    lay->LayoutUnits();
  }
  tabMisc::NotifyEdits(lay);
  FixUnits();
  RedistributeSide(-1);
  netg->owner->ClearPosivPatch();
  taivMisc::SRIAssignment(lay,lay->FindMember("geom"));
  return true;
}

bool Layer_G::effect_scale(const ivEvent& e, Tool& t,
				ivCoord ix,ivCoord iy,
				ivCoord fx, ivCoord fy,
				ivCoord ctrx, ivCoord ctry) {
  Graphic::effect_scale(e,t,ix,iy,fx,fy,ctrx,ctry);
  DMEM_GUI_RUN_IF {
    netg->owner->SetLayerXform(lay,transformer());
  }
  float a00,a01,a10,a11,a20,a21;
  transformer()->matrix(a00,a01,a10,a11,a20,a21);
  taivMisc::RecordScript(netg->owner->owner->GetPath()+
			 ".SetLayerXform(" + lay->GetPath() + ", " +
			  String(a00) + ", " + String(a01) + ", " +
			  String(a10) + ", " + String(a11) + ", " +
			  String(a20) + ", " + String(a21) + ");\n");
  return true;
}

void Layer_G::RemoveProj_G(Projection_G* pjn_g){
  int i;
  int j;
  ivPolyGlyph* pg;
  int size;
  int remove = false;

  size = (int) proj_gs->count();
  for(i=0;i<size;i++){
    pg = (ivPolyGlyph *) proj_gs->component(i);
    for(j = 0;j<pg->count();j++){
      if(pg->component(j) == pjn_g){
	pg->remove(j); j--; remove = true;
      }					  
    }
    if (remove == true) {RedistributeSide(i); remove= false; break;}
  }
}

static void SortPrjns(int_Array* ary, ivPolyGlyph* lspg, bool topright) {
  if(ary->size <= 1) return;
  int n = ary->size;
  int l,j,ir,i;
  int tmp;

  l = (n >> 1) + 1;
  ir = n;
  for(;;){
    if(l>1)
      tmp = ary->FastEl(--l -1); // tmp = ra[--l]
    else {
      tmp = ary->FastEl(ir-1); // tmp = ra[ir]
      ary->FastEl(ir-1) = ary->FastEl(0); // ra[ir] = ra[1]
      if(--ir == 1) {
	ary->FastEl(0) = tmp; // ra[1]=tmp
	return;
      }
    }
    i=l;
    j=l << 1;
    while(j<= ir) {
      if(j<ir) {
	float jm1d = ((Projection_G*)lspg->component(ary->FastEl(j-1)))->mid_dist;
	float jd = ((Projection_G*)lspg->component(ary->FastEl(j)))->mid_dist;
	float dif = jm1d - jd;
	if(topright && fabs(dif) < 1.0f) dif = -dif;	// flip tie-breaking values
	if(dif < 0.0f) j++;	// jm1d < jd
      }

      float tmpd = ((Projection_G*)lspg->component(tmp))->mid_dist;
      float jm1d = ((Projection_G*)lspg->component(ary->FastEl(j-1)))->mid_dist;
      float dif = tmpd - jm1d;
      if(topright && fabs(dif) < 1.0f) dif = -dif;	// flip tie-breaking values
      if(dif < 0.0f) { // tmp < ra[j]
	ary->FastEl(i-1) = ary->FastEl(j-1); // ra[i]=ra[j];
	j += (i=j);
      }
      else j = ir+1;
    }
    ary->FastEl(i-1) = tmp; // ra[i] = tmp;
  }
}

void Layer_G::RedistributeSide(int side_index) { // 0<side_index<4, -1= all
  if(side_index == INDEPENDENT) return;
  if(side_index == -1) {
    // first redistribute all the self prjns into least-loaded sides
    ivPolyGlyph* selfs = new ivPolyGlyph; // tmp hold all self prjns
    int s;
    for(s=0;s<4;s++) {    // first collect them all and remove from prior
      ivPolyGlyph* lspg = (ivPolyGlyph*) proj_gs->component(s);
      int i;
      for(i=lspg->count()-1;i>=0;i--) {
	Projection_G* pjn_g = (Projection_G*) lspg->component(i);
	if(pjn_g->from_g == pjn_g->to_g) {
	  selfs->append(pjn_g);
	  lspg->remove(i--); lspg->remove(i); // get rid of second version!
	}
      }
    }
    int i;
    for(i=0; i<selfs->count(); i++) {
      Projection_G* pjn_g = (Projection_G*) selfs->component(i);
      int fidx, tidx;
      pjn_g->GetSelfProjSides(fidx, tidx);
      ((SelfCon_PLine_G*) pjn_g->component_(0))->side_index = fidx;
      ivPolyGlyph* lspg = (ivPolyGlyph*) proj_gs->component(fidx);
      lspg->append(pjn_g);
      lspg->append(pjn_g);	// two copies!
    }
    delete selfs;
    for(s=0;s<4;s++) RedistributeSide(s);
    return;
  }
  ivCoord x0, x1, x2, x3;
  ivCoord y0, y1, y2, y3;
  ivPolyGlyph* lspg = (ivPolyGlyph*) proj_gs->component(side_index);
  if(lspg->count() == 0) return;

  bool topright = false;
  if((side_index == Layer_G::TOP) || (side_index == Layer_G::RIGHT))
    topright = true;

  // now sort according to mid_dist
  int_Array* ary = new int_Array;
  ary->EnforceSize(lspg->count());  ary->FillSeq();
  SortPrjns(ary, lspg, topright);

  // now check for any bidir cons and make sure they don't cross
  float incr = .01f;
  Projection_G* pjn_g;
  int i;
  Projection_G* prv_pjn_g = (Projection_G*) lspg->component(ary->FastEl(0));
  for(i=1; i<lspg->count(); i++) {
    pjn_g = (Projection_G*) lspg->component(ary->FastEl(i));
    //    swtch = false;
    if((prv_pjn_g->from_g == pjn_g->to_g) && (prv_pjn_g->to_g == pjn_g->from_g)) {
      if(prv_pjn_g->mid_dist == pjn_g->mid_dist) {
	pjn_g->mid_dist += incr; // closer
	incr += .01f;
      }
    }
    else if((prv_pjn_g->from_g == pjn_g->from_g) && (prv_pjn_g->to_g == pjn_g->to_g)) {
      if(prv_pjn_g->mid_dist == pjn_g->mid_dist) {
	pjn_g->mid_dist += incr;
	incr += .01f;
      }
    }
    else {
      incr = .01f; // reset
    }
    prv_pjn_g = pjn_g;
  }

  LayerBox_G* box = GetActualLayerBox();
  box->GetPoints(&x0,&y0,&x1,&y1,&x2,&y2,&x3,&y3);
  
  float x_start = 0; float x_end = 0; float y_start = 0; float y_end = 0;
  switch(side_index) {
  case Layer_G::TOP:
    x_start = x3; x_end = x2; y_start = y3; y_end = y2; break;
  case Layer_G::BOTTOM:
    x_start = x1; x_end = x0; y_start = y0; y_end = y1; break;
  case Layer_G::LEFT:
    x_start = x3; x_end = x0; y_start = y3; y_end = y0; break;
  case Layer_G::RIGHT:
    x_start = x1; x_end = x2; y_start = y1; y_end = y2; break;
  }

  //  cerr << "\nLayer: " << lay->name << " side: " << side_index << "\n";

  float x_inc = (x_end - x_start) / (lspg->count() + 1);
  float y_inc = (y_end - y_start) / (lspg->count() + 1);
  float fx,fy;
  PLine_G* plg;
  for(i=0,fx = x_start + x_inc, fy = y_start + y_inc; i<lspg->count();
      i++, fx += x_inc, fy += y_inc) {
    pjn_g = (Projection_G*) lspg->component(ary->FastEl(i));
//      cerr << i << ": fm " << pjn_g->from_g->lay->name << " to " << pjn_g->to_g->lay->name 
//  	 << ", dst " << pjn_g->mid_dist << "\n";
    plg = (PLine_G*) pjn_g->component_(0);
    if((pjn_g->from_g == pjn_g->to_g) && (pjn_g->from_g == this)) {
      // selfcon, do both send and receive
      plg->SetStartX(fx); plg->SetStartY(fy);
      i++; fx += x_inc; fy += y_inc;
      plg->SetEndX(fx); plg->SetEndY(fy);	
    }
    else { // normal projection, do corect one
      if(pjn_g->from_g == this) {
	plg->SetStartX(fx); plg->SetStartY(fy);
      }
      else {			// the prjn is receiving on this side
	plg->SetEndX(fx); plg->SetEndY(fy);	
      }
    }
  }
  delete ary;
}


//////////////////////////////////
//	   LayerNameEdit	//
//////////////////////////////////

void LayerNameEdit::Initialize(){
  layer = NULL;
}

void LayerNameEdit::SetLayer(Layer* l){
  if(l != NULL){   taBase::SetPointer((TAPtr*)&(layer), l);  name = layer->name;}
}

void LayerNameEdit::Destroy(){
  if(layer != NULL){
    taBase::DelPointer((TAPtr*)&layer);   
  }
}

void LayerNameEdit::UpdateAfterEdit(){
  if(layer != NULL){
    layer->name = name;
    layer->UpdateAfterEdit();	
  }
}

//////////////////////////////////
//	   LayerText_G		//
//////////////////////////////////

LayerText_G::LayerText_G(Layer_G* l,const ivFont* font, const ivColor* stroke,
			 const char* c, ivTransformer* t) 
:NoScale_Text_G(l->lay,l->netg,font,stroke,c,t) {
  lay_g = l;
  get_color = NetworkGGetLabelColor; // use background color of network_g
  editable = true;
  lne = new LayerNameEdit();
  taBase::Ref(lne);
  lne->SetLayer(lay_g->lay);
}

LayerText_G::~LayerText_G() {
  taBase::DelPointer((TAPtr*)&lne);
}

void LayerText_G::transformer(ivTransformer * t){
  NoScale_Text_G::transformer(t);
}

ivTransformer* LayerText_G::transformer(){
  float sx = 1.0; float sy = 1.0;
  float nx = lay_g->GetCurrentXformScale();
  float ny = lay_g->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;
  nx = lay_g->netg->GetCurrentXformScale();
  ny = lay_g->netg->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;
  scaled_tform = *(Graphic::transformer());
  scaled_tform.scale(sx,sy);
  return &scaled_tform;
}

bool LayerText_G::effect_move(const ivEvent& , Tool& , ivCoord ,
				  ivCoord , ivCoord , ivCoord ) {
  ivTransformer* tr = _t; // get the real thing
  if(tr == NULL) return false;
  DMEM_GUI_RUN_IF {
    lay_g->netg->owner->SetLayerLabelXform(lay_g->lay,tr);
  }
  float a00,a01,a10,a11,a20,a21;
  tr->matrix(a00,a01,a10,a11,a20,a21);
  taivMisc::RecordScript(lay_g->netg->owner->owner->GetPath()+
			 ".SetLayerLabelXform(" + lay_g->lay->GetPath() + ", " + 
			 String(a00) + ", " + String(a01) + ", " +
			 String(a10) + ", " + String(a11) + ", " +
			 String(a20) + ", " + String(a21) + ");\n");
//   taivMisc::RecordScript(lay_g->netg->net->GetPath() + ".Fix_Layer_Views(" +
// 			 lay_g->lay->GetPath() + ");");
  return true;
}

bool LayerText_G::effect_select(bool set_select) {
  bool temp = Graphic::effect_select(set_select);
  Network_G* netg = (Network_G*)master;
  if(set_select && editb_used && (!netg->reinit_display) && (netg->selectgroup.size == 1)) {
    lne->Edit(true);		// wait
    editb_used = false;		// don't do it again!
    netg->reinit_display = true;
  }
  netg->selectgroup.Remove(obj);
  if(set_select)
    netg->selectgroup.LinkUnique(lne);
  else
    netg->selectgroup.Remove(lne);
  return temp;
}

//////////////////////////////////
//	   LayerBox_G		//
//////////////////////////////////

LayerBox_G::LayerBox_G(TAPtr obj, PosTDCoord* gm, Network_G* ng,
			 const ivBrush* brush, const ivColor* stroke,
			 const ivColor* fill, ivCoord* x, ivCoord* y,
			 int ctrlpts, ivTransformer* t)
: Polygon(brush,stroke,fill,x,y,ctrlpts, t)
{
  object = obj;
  geom = gm;
  netg = ng;
}

bool LayerBox_G::effect_select(bool set_select) {
  bool temp = Polygon::effect_select(set_select);
  if(set_select)
    netg->selectgroup.LinkUnique(object);
  else
    netg->selectgroup.Remove(object);
  return temp;
}

void LayerBox_G::SetPointsFmGeom() {
  ivCoord px[4], py[4];
  px[0] = py[0] = py[1] = 0;
  netg->tdp.Project(geom->x, 0, 0, px[1], py[1]);
  netg->tdp.Project(geom->x, geom->y, 0, px[2],py[2]);
  netg->tdp.Project(0, geom->y, 0, px[3],py[3]);
  SetPoints(px[0],py[0],px[1],py[1],px[2],py[2],px[3],py[3]);
}

void LayerBox_G::SetPoints(ivCoord x0,ivCoord y0, ivCoord x1, ivCoord y1,
			ivCoord x2,ivCoord y2, ivCoord x3, ivCoord y3) {
  _x[0] = x0; _x[1] = x1; _x[2] = x2; _x[3] = x3;
  _y[0] = y0; _y[1] = y1; _y[2] = y2; _y[3] = y3;
  recompute_shape();
}

void LayerBox_G::GetPoints(ivCoord* x0,ivCoord* y0, ivCoord* x1, ivCoord* y1,
			ivCoord* x2,ivCoord* y2, ivCoord* x3, ivCoord* y3) {
   *x0 = _x[0];  *x1 = _x[1];  *x2 = _x[2];  *x3 = _x[3];
   *y0 = _y[0];  *y1 = _y[1];  *y2 = _y[2];  *y3 = _y[3];
  recompute_shape();
}

void LayerBox_G::draw_gs (ivCanvas* c, Graphic* gs) {
  if(_ctrlpts <= 0)
    return;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }
  c->new_path();
  c->move_to(_x[0], _y[0]);
  for (int i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  if (_closed) {
    c->close_path();
  }
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
    // now if it is selected draw grid
    if(_selected){
      float pt_x_0,pt_y_0;
      TDCoord temp_pos;
      for(temp_pos.x=1; temp_pos.x < geom->x; temp_pos.x++){
	temp_pos.y = 0;
	netg->tdp.Project(temp_pos,pt_x_0,pt_y_0);
	c->new_path();
	c->move_to(pt_x_0,pt_y_0);
	temp_pos.y = geom->y;
	netg->tdp.Project(temp_pos,pt_x_0,pt_y_0);
	c->line_to(pt_x_0,pt_y_0);
	c->stroke(stroke, brush);
      }
      for(temp_pos.y=1; temp_pos.y < geom->y; temp_pos.y++){
	temp_pos.x = 0;
	netg->tdp.Project(temp_pos,pt_x_0,pt_y_0);
	c->new_path();
	c->move_to(pt_x_0,pt_y_0);
	temp_pos.x = geom->x;
	netg->tdp.Project(temp_pos,pt_x_0,pt_y_0);
	c->line_to(pt_x_0,pt_y_0);
	c->stroke(stroke, brush);
      }
    }
  }
  if (tx != nil) {
    c->pop_transform();
  }
}

//////////////////////////////////
//	   Unit_Group_G		//
//////////////////////////////////

Unit_Group_G::Unit_Group_G(Unit_Group* gp, Layer_G* lg) {
  group = gp;
  layg = lg;
  Position();
}

void Unit_Group_G::Position() {
  float pt_x_0, pt_y_0;
  layg->netg->tdp.Project(group->pos, pt_x_0, pt_y_0);
  translate(pt_x_0, pt_y_0);
}

void Unit_Group_G::RePick(){
  long i;
  for(i=0; i<count_()-1; i++) {
    ((Unit_G*)component_(i))->Fix_UVGS();
  }
}
  
ivGlyph* Unit_Group_G::clone() const {
  return new Unit_Group_G(group, layg); 
}

bool Unit_Group_G::grasp_move(const ivEvent&, Tool&,ivCoord, ivCoord) {
  move_pos.Initialize();
  layg->netg->owner->UpdatePosivPatch(group->pos);
  layg->netg->owner->FixEditorButtons();
  return true;
}

bool Unit_Group_G::manip_move(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy,
			      ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy)
{
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0))
    return true;

  dx = cx - ix;
  dy = cy - iy;
  TDCoord ng,dg;
  switch(layg->netg->owner->viewer->last_button) {
  case(ivEvent::right):
    layg->netg->tdp.InverseProjXZ(dx, dy, ng); break;
  default:
    layg->netg->tdp.InverseProjXY(dx, dy, ng); break;
  }
  dg = ng;
  dg -= move_pos;
  if((dg.x != 0) || (dg.y != 0) || (dg.z != 0)) {
    TDCoord updtpos = group->pos + dg;
    if((updtpos.x <0) || (updtpos.y <0) || (updtpos.z <0)) {
      return true;
    }
    move_pos = ng;
    group->pos.Invert();
    Position();
    group->pos.Invert();
    PosTDCoord og = group->pos;
    group->pos += dg;
    Position();			// put into the new one
    layg->netg->owner->UpdatePosivPatch(group->pos);
  }
  return true;
}

bool Unit_Group_G::effect_move(const ivEvent&, Tool&,ivCoord , ivCoord , ivCoord , ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0) && (move_pos.z == 0)) return true;
  move_pos.Initialize();
  tabMisc::NotifyEdits(group);
  layg->netg->owner->ClearPosivPatch();
  taivMisc::SRIAssignment(group,group->FindMember("pos"));
  return true;
}

bool Unit_Group_G::grasp_stretch(const ivEvent& , Tool& ,ivCoord, ivCoord) {
  move_pos.Initialize();
  layg->netg->owner->UpdatePosivPatch(group->geom);
  layg->netg->owner->FixEditorButtons();
  return true;
}

bool Unit_Group_G::manip_stretch(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0))
    return true;

  dx = cx - ix;
  dy = cy - iy;
  TDCoord ng,dg;
  switch(layg->netg->owner->viewer->last_button) {
  case(ivEvent::right):
    layg->netg->tdp.InverseProjXZ(dx, dy, ng); break;
  default:
    layg->netg->tdp.InverseProjXY(dx, dy, ng); break;
  }
  dg = ng;
  dg -= move_pos;
  if((dg.x != 0) || (dg.y != 0) || (dg.z != 0)) {
    TDCoord updtgeom = group->geom + dg;
    if(((updtgeom.x <0) || (updtgeom.y <1) || (updtgeom.z < 1))
       || ((updtgeom.x * updtgeom.y) < (int) group->size)) {
      return true;
    }
    move_pos = ng;
    PosTDCoord og = group->geom;
    group->geom += dg;
    LayerBox_G* lbg = (LayerBox_G *) component_(count_()-1);
    lbg->SetPointsFmGeom();
    layg->netg->owner->UpdatePosivPatch(group->geom);
  }
  return true;
}

bool Unit_Group_G::effect_stretch(const ivEvent&, Tool&, ivCoord,
				  ivCoord, ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0))
    return true;
  move_pos.Initialize();
  if((group->n_units == 0) || (group->size == 0))
    group->n_units = group->geom.x * group->geom.y;
  DMEM_GUI_RUN_IF {
    group->own_lay->LayoutUnits();
  }
  tabMisc::NotifyEdits(group);
  FixUnits();
  layg->netg->owner->ClearPosivPatch();
  taivMisc::SRIAssignment(group,group->FindMember("geom"));
  return true;
}

void Unit_Group_G::FixUnits(){
  ivCanvas* c = layg->netg->owner->viewer->canvas();
  if(c == NULL) return;

  safe_damage_me(c);
  transformer(NULL);
  Position();
  LayerBox_G* lbg = (LayerBox_G *) component_(count_()-1);
  lbg->SetPointsFmGeom();
  safe_damage_me(c);

  if((count_()-1) != group->size) {
    int i;
    for(i=0; i<group->size; i++) {
      FixUnit((Unit*)group->FastEl(i));
    }
  }
  else {
    int j;
    for(j=0;j<count_()-1;j++){
      Unit_G* ug = (Unit_G *) component_(j);
      ug->Position();
    }
  }
}

void Unit_Group_G::FixUnit(Unit* u){
  ivCanvas* c = layg->netg->owner->viewer->canvas();
  if(c == NULL) return;
  int unit_index = -1;
  Unit_G* ug = NULL;
  int j;
  for(j=0;j<count_()-1;j++){
    ug = (Unit_G*) component_(j);
    if (ug->unit == u) { unit_index = j ; break; }
  }
  bool new_unit = false;
  if(unit_index == -1) {
    unit_index = group->FindEl(u);
    if(unit_index == -1) return; // wrong layer_g!!
    ug = new Unit_G(layg->netg,u);
    new_unit = true;
    insert_(count_()-unit_index -2,ug);
  }
  ug->safe_damage_me(c);
  ug->Position();
  if(new_unit) ug->Fix_UVGS();
  ug->safe_damage_me(c);
}

void Unit_Group_G::Build() {
  int i;
  // layout unit_g's like so 3-d unit overlaps correctly
  // 123
  // 456
  // 789
  Unit_G* lug = NULL;		// last unit_g
  Unit_G* nug;			// new unit_g;
  ivGlyphIndex last_index = 0; 	// last_unit_g's index
  for(i=0; i<group->size; i++) {
    Unit* nu = group->FastEl(i);
    if(nu->owner == NULL) continue; // signal to cut unit!!
    nug = new Unit_G(layg->netg,nu);
    if(count_() == 0){
      insert_(0,nug); lug = nug; last_index = 0;
      continue;
    }
    if(nug->unit->pos.x > lug->unit->pos.x) 
      insert_(++last_index,nug);
    else {
      insert_(0,nug);
      last_index = 0;
    }
    lug = nug;
  }

  // finally a layer box (count_()-1)
  ivCoord px[4],py[4];
  LayerBox_G* lb = new LayerBox_G
    (group, &(group->geom), layg->netg, layg->netg->defbrush,
     layg->netg->border,NULL,px,py,4,NULL);
  lb->SetPointsFmGeom();
  lb->recompute_shape();
  append_(lb);
}

bool Unit_Group_G::select(const ivEvent& e, Tool& tool, bool unselect){
  long i;
  if(unselect)
    _selected = false;
  else
    _selected = !_selected;

  if(count_() > 0) { // there are subobjs
    LayerBox_G* lbg = (LayerBox_G*) component_(count_()-1);
    if(unselect == false) {
      if(lbg->is_selected()){
	lbg->select(e,tool,false);
      }
      else {
	lbg->select(e,tool,false);
	for(i=0; i < count_()-1; i++) {
	  component_(i)->select(e, tool, true);
	}
      }
    }
    else {
      if(lbg->is_selected()){
	lbg->select(e,tool,true);
      }
      else {
	for(i=0; i < count_()-1; i++) {
	  component_(i)->select(e, tool, unselect);
	}
      }
    }
  }
  return true;
}
  
//////////////////////////////////
//	   Projection_G		//
//////////////////////////////////

Projection_G::Projection_G() {
  pjn = NULL;
  from_g = NULL;
  to_g = NULL;
  net_g = NULL;
  mid_dist = 0;
  l = r = t = b = 0.0f;
  move_end = 0;
}

Projection_G::~Projection_G() {
}

void Projection_G::RemoveMe(){
  // remove from layers
  to_g->RemoveProj_G(this);
  from_g->RemoveProj_G(this);

  // remove from the network
  int i;
  int size = (int) net_g->count_();
  for(i=0;i<size;i++){
    if(net_g->component_(i) == this){
      net_g->remove_(i);
      break;
    }
  }
}

void Projection_G::GetClosestSides(int& from_side_index,int& to_side_index) {
  from_side_index = 0;
  to_side_index = 0;

  Layer* frml = from_g->lay;
  Layer* tol = to_g->lay;

  int frml_mid_x = frml->pos.x + (frml->act_geom.x / 2);
  int frml_mid_y = frml->pos.y + (frml->act_geom.y / 2);

  int tol_mid_x = tol->pos.x + (tol->act_geom.x / 2);
  int tol_mid_y = tol->pos.y + (tol->act_geom.y / 2);

  int mid_dist_x = frml_mid_x - tol_mid_x;
  int mid_dist_y = frml_mid_y - tol_mid_y;

  if(frml->pos.z < tol->pos.z) {
    from_side_index = Layer_G::TOP;
    to_side_index = Layer_G::BOTTOM;
    mid_dist = -mid_dist_x;
  }
  else if(frml->pos.z > tol->pos.z) {
    to_side_index = Layer_G::TOP;
    from_side_index = Layer_G::BOTTOM;
    mid_dist = mid_dist_x;
  }
  else if((frml->pos.y + frml->act_geom.y) < tol->pos.y) {
    from_side_index = Layer_G::TOP;
    to_side_index = Layer_G::BOTTOM;
    mid_dist = -mid_dist_x;
  }
  else if((tol->pos.y + tol->act_geom.y) < frml->pos.y) {
    to_side_index = Layer_G::TOP;
    from_side_index = Layer_G::BOTTOM;
    mid_dist = mid_dist_x;
  }
  else if(frml->pos.x < tol->pos.x) {
    from_side_index = Layer_G::RIGHT;
    to_side_index = Layer_G::LEFT;
    mid_dist = -mid_dist_y;
  }
  else {
    to_side_index = Layer_G::RIGHT;
    from_side_index = Layer_G::LEFT;
    mid_dist = mid_dist_y;
  }
}

void Projection_G::GetSelfProjSides(int& from_side_index,int& to_side_index) {
  mid_dist = 0;			// default is center

  // never put a self prjn on the bottom!
  int rcnt = ((ivPolyGlyph*)from_g->proj_gs->component(Layer_G::RIGHT))->count();
  int lcnt = ((ivPolyGlyph*)from_g->proj_gs->component(Layer_G::LEFT))->count();
  int tcnt = ((ivPolyGlyph*)from_g->proj_gs->component(Layer_G::TOP))->count();

  from_side_index = Layer_G::LEFT;
  if((lcnt < rcnt) && (lcnt < tcnt))
    from_side_index = Layer_G::LEFT;
  else if((rcnt < tcnt) && (rcnt < lcnt))
    from_side_index = Layer_G::RIGHT;
  else if((tcnt < rcnt) && (tcnt < lcnt))
    from_side_index = Layer_G::TOP;
  else if((lcnt <= rcnt) && (lcnt <= tcnt))
    from_side_index = Layer_G::LEFT;
  else if((rcnt <= tcnt) && (rcnt <= lcnt))
    from_side_index = Layer_G::RIGHT;
  else if((tcnt <= rcnt) && (tcnt <= lcnt))
    from_side_index = Layer_G::TOP;

  to_side_index = from_side_index;
  if(from_side_index == Layer_G::TOP) {
    if(tcnt > 0) {
      if(rcnt < lcnt) mid_dist = -20; // put self cons at the right
      else mid_dist = 20; // put self cons at the left
    }
  }
  else if(from_side_index == Layer_G::LEFT) {
    if(lcnt > 0) mid_dist = -20;		// put self cons at the back
  }
  else {
    if(rcnt > 0) mid_dist = 20;		// put self cons at the back
  }
}

void Projection_G::RecomputePoints() {
  // this adds the projection_g to the correct layer_g's side polyglyphs,
  // and recomputes the line end points for all points on the each layer's
  // side which is added to, except the points for the plines which have
  // been set in the projection_g
  
  // warning this function expects net_g,pjn,to_g,pjn->from,ect to be set
  // on the proj_g;
  // it also expects that you removed the projection_G from its
  // previous lay_g sides before calling this

  int from_side_index, to_side_index;
  if(from_g != to_g)
    GetClosestSides(from_side_index,to_side_index);
  else {
    GetSelfProjSides(from_side_index,to_side_index);
    ((SelfCon_PLine_G *) component_(0))->side_index = from_side_index;
  }
 
  ivPolyGlyph* lspg = (ivPolyGlyph *) from_g->proj_gs->component(from_side_index);
  lspg->append(this);
  from_g->RedistributeSide(from_side_index);
  
  lspg = (ivPolyGlyph *) to_g->proj_gs->component(to_side_index);
  lspg->append(this);
  to_g->RedistributeSide(to_side_index);
}

void Projection_G::NewProjection(Projection* p, Layer_G *tg, Layer_G *fg,Network_G* n) {
  net_g = n;
  pjn = p;
  to_g = tg;
  from_g = fg;

  PLine_G* pl;
  if (to_g != from_g) { // non self Connection
    ivCoord plx[4],ply[4];
    pl = new PLine_G(this,net_g->defbrush,net_g->border,NULL,
			      (ivCoord *) plx, (ivCoord *) ply, 4,
			      (ivTransformer *) NULL);
  }
  else { // change this to self_con_pline with 6 pts
    ivCoord scplx[6],scply[6];
    pl = new SelfCon_PLine_G(this,net_g->defbrush,net_g->border,NULL,
			    (ivCoord *) scplx, (ivCoord *) scply, 6,
			    (ivTransformer *) NULL);
  }
  append_(pl);
  if(pjn->proj_points == NULL) {
    RecomputePoints();
  }
  else {
    pl->Set_Start_End(p->proj_points->a00,p->proj_points->a01,
		      p->proj_points->a10,p->proj_points->a11);
    ivPolyGlyph* lspg = (ivPolyGlyph *)
      from_g->proj_gs->component(Layer_G::INDEPENDENT);
    lspg->append(this);
    lspg = (ivPolyGlyph *)
      to_g->proj_gs->component(Layer_G::INDEPENDENT);
    lspg->append(this);
  }
}

bool Projection_G::grasp_move(const ivEvent& e, Tool&,ivCoord, ivCoord) {
  // do not move self_connections
  if(from_g == to_g) return false;
  // find the closest end point
  ivCoord epx = e.pointer_x();
  ivCoord epy = e.pointer_y();
  PLine_G* pl = (PLine_G *) component_(0);
  ivCoord sx,sy,ex,ey;
  pl->Xform_Point(pl->GetStartX(),pl->GetStartY(),&sx,&sy,0);
  pl->Xform_Point(pl->GetEndX(),pl->GetEndY(),&ex,&ey,1);
  if(DIST((epx-sx),(epy-sy)) < DIST((epx-ex),(epy-ey))){
    move_end = pl->StartIndex(); // sending side
  }
  else {
    move_end = pl->EndIndex();
  }
  Graphic* box;
  if(move_end == pl->StartIndex())
    box = from_g->GetActualLayerBox();
  else
    box = to_g->GetActualLayerBox();
  Graphic* par = box->parent(); ivResource::ref(par);box->parent(NULL);
  const ivBrush* bru = box->brush(); ivResource::ref(bru); box->brush(NULL);
  box->getbounds(l,b,r,t);
  box->parent(par);ivResource::unref(par);
  box->brush(bru);ivResource::unref(bru);
  l += 1; b+=1; t-=1; r-=1; // compensate for fixed tolerance
  net_g->owner->FixEditorButtons();
  return true;
}

bool Projection_G::manip_move(const ivEvent&, Tool&,ivCoord, ivCoord, ivCoord lx, ivCoord ly,
			    ivCoord cx, ivCoord cy) {
  // do not move self_connections
  if(from_g == to_g) return false;
  PLine_G* pl = (PLine_G *) component_(0);
  pl->AddX(move_end,(cx-lx));
  pl->AddY(move_end,(cy-ly));
  pl->MaxX(move_end,r);
  pl->MinX(move_end,l);
  pl->MaxY(move_end,t);
  pl->MinY(move_end,b);
  return true;
}

bool Projection_G::effect_move(const ivEvent& e , Tool& tl,ivCoord ix, ivCoord iy,
					  ivCoord cx, ivCoord cy) {
  // do not move self_connections
  if(from_g == to_g) return false;
  PLine_G* pl = (PLine_G *) component_(0);
  // set both pairs of points so both get saved
  pjn->SetFromPoints(pl->GetStartX(),pl->GetStartY());
  pjn->SetToPoints(pl->GetEndX(),pl->GetEndY());

  // also save the points of all the projections on the same side

  int size,i,j;
  ivPolyGlyph* pg;
  int to_g_side=0;
  size= (int) to_g->proj_gs->count();
  for(i=0;i<size;i++){
    pg = (ivPolyGlyph *) to_g->proj_gs->component(i);
    for(j=0;j<pg->count();j++){
      if(pg->component(j) == this){
	pg->remove(j);
	to_g_side = i; i=size; break;
      }
    }
  }
  int from_g_side=0;
  size= (int) from_g->proj_gs->count();
  for(i=0;i<size;i++){
    pg = (ivPolyGlyph *) from_g->proj_gs->component(i);
    for(j = 0;j<pg->count();j++){
      if(pg->component(j) == this){
	pg->remove(j);
	from_g_side = i; i=size; break;
      }
    }
  }
  ivPolyGlyph* lspg = (ivPolyGlyph *)
    from_g->proj_gs->component(Layer_G::INDEPENDENT);
  lspg->append(this);
  lspg = (ivPolyGlyph *)
    to_g->proj_gs->component(Layer_G::INDEPENDENT);
  lspg->append(this);

  if(to_g_side != Layer_G::INDEPENDENT) {
    pg = (ivPolyGlyph *) to_g->proj_gs->component(to_g_side);
    for(j=0;j<pg->count();j++){
      Projection_G* t_prjn = (Projection_G *) pg->component(j);
      if(t_prjn == this) continue;
      if(t_prjn->from_g == t_prjn->to_g) continue;
      t_prjn->effect_move(e,tl,ix,iy,cx,cy);
    }
  }
  if(from_g_side != Layer_G::INDEPENDENT) {
    pg = (ivPolyGlyph *) from_g->proj_gs->component(from_g_side);
    for(j=0;j<pg->count();j++){
      Projection_G* f_prjn = (Projection_G *) pg->component(j);
      if(f_prjn == this) continue;
      if(f_prjn->from_g == f_prjn->to_g) continue;
      f_prjn->effect_move(e,tl,ix,iy,cx,cy);
    }
  }
  return true;
}

//////////////////////////////////
//	   PLine_G		//
//////////////////////////////////

PLine_G::PLine_G( Projection_G* pg, const ivBrush* brush, const ivColor* stroke,
  const ivColor* fill,
  ivCoord* x, ivCoord* y, int size, ivTransformer* tx)
: Polyline(brush,stroke,fill,x,y,size,tx){
  proj_g = pg;
  int i;
  for(i=0;i<size;i++){
    _x[i] = _y[i] = 0;
  }
  editb_used = false;
}
				       

void PLine_G::Xform_Point(ivCoord ix, ivCoord iy, ivCoord* tx, ivCoord* ty,int side) {
  ivTransformer* t = new ivTransformer(*(proj_g->net_g->transformer()));
  ivTransformer* tl;
  ivTransformer* tb;
  if(side == 0) { // from_side
    tl = proj_g->from_g->transformer();
    tb = proj_g->from_g->GetActualLayerBox()->transformer();
    if(tl) t->premultiply(*tl);
    if(tb) t->premultiply(*tb);
    t->transform(ix,iy,tx[0],ty[0]);
  }
  else { // to side
    tl = proj_g->to_g->transformer();
    tb = proj_g->to_g->GetActualLayerBox()->transformer();
    if(tl) t->premultiply(*tl);
    if(tb) t->premultiply(*tb);
    t->transform(ix,iy,tx[0],ty[0]);
  }
  delete t;
}

void PLine_G::Xform_gs(ivCoord* tx, ivCoord* ty, Graphic* gs) {
  ivTransformer t = *gs->transformer();
  ivTransformer* tl = proj_g->from_g->transformer();
  ivTransformer* tb = proj_g->from_g->GetActualLayerBox()->transformer();
  if(tl) t.premultiply(*tl);
  if(tb)t.premultiply(*tb);
  t.transform(_x[0],_y[0],tx[0],ty[0]);

  t = *gs->transformer();                                                       
  tl = proj_g->to_g->transformer();                                      
  tb = proj_g->to_g->GetActualLayerBox()->transformer();
  if(tl) t.premultiply(*tl);
  if(tb) t.premultiply(*tb);
  t.transform(_x[1],_y[1],tx[1],ty[1]);
  t.transform(_x[2],_y[2],tx[2],ty[2]);
  t.transform(_x[3],_y[3],tx[3],ty[3]);
}  
  
void PLine_G::getextent_gs
(ivCoord& l, ivCoord& b, ivCoord& cx, ivCoord& cy, ivCoord& tol, Graphic* gs){
  tol = 1.0;
  const ivBrush* br = gs->brush();
  if (br != nil) {
    ivCoord width = ivCoord(br->width());
    tol = (width > 1) ? width : tol;
  }
  ivCoord* tx = new ivCoord[_ctrlpts+1];
  ivCoord* ty = new ivCoord[_ctrlpts+1];
  Xform_gs(tx,ty,gs);
  ivCoord left, bottom, right, top;
  left = right = tx[0]; bottom = top = ty[0];
  int i;
  for(i=0;i<_ctrlpts;i++){
    left = MIN(left,tx[i]);
    bottom = MIN(bottom,ty[i]);
    right = MAX(right,tx[i]);
    top = MAX(top,ty[i]);
  }
  delete tx; delete ty;
  
  l = left;
  b = bottom;
  cx = (left + right)/2.0;
  cy = (top + bottom)/2.0;
}
  
    

void PLine_G::draw_gs(ivCanvas* c, Graphic* gs) {
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();

  // transform for the network
  ivTransformer* tn = gs->transformer();
  if (tn != nil) {
    c->push_transform();
    c->transform(*tn);
  }

  // move_to the from_layer and start the line
  // if the layer's transformer has been flushed then
  // get the transform off the layer_box:
  ivTransformer* tf = proj_g->from_g->transformer();
  ivTransformer* tfb = proj_g->from_g->GetActualLayerBox()->transformer();

  ivCoord layeronly_x0 = _x[0];
  ivCoord layeronly_y0 = _y[0];

  if (tf != nil) {
    c->push_transform();
    c->transform(*tf);
    tf->transform(_x[0], _y[0], layeronly_x0,layeronly_y0);
  }
  if(tfb != nil) {
    c->push_transform();
    c->transform(*tfb);
    tfb->transform(_x[0], _y[0], layeronly_x0,layeronly_y0);
  }
  ivCoord actual_x0,actual_y0;
  c->transformer().transform(_x[0], _y[0], actual_x0,actual_y0);
  if (tfb != nil) {
    c->pop_transform();
  }
  if (tf != nil) {
    c->pop_transform();
  }

  //  this is moved out of the above transformation space
  //  because interview's pop_transform() function generates
  //  postscript (grestore) which clears the current path
  //  so if we did a c->pop_transform() after the move_to
  //  expression below, the move_to point would be lost!

  c->new_path();
  c->move_to(layeronly_x0,layeronly_y0);

  // move to the to layer_and end the line
  // if the layer's transformer has been flushed then
  // get the transform off the layer_box:
  ivTransformer* tt = proj_g->to_g->transformer();
  ivTransformer* ttb = proj_g->to_g->GetActualLayerBox()->transformer();
  if (tt != nil) {
    c->push_transform();
    c->transform(*tt);
  }
  if (ttb != nil) {
    c->push_transform();
    c->transform(*ttb);
  }
  c->line_to(_x[1],_y[1]);

  c->close_path();
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }
  
  // recompute the arrow points
  ivCoord actual_x1,actual_y1;
  c->transformer().transform(_x[1], _y[1], actual_x1,actual_y1);

  double angle;
  double dy = actual_y0 - actual_y1;
  double dx = actual_x0 - actual_x1;
  if(dx == 0){
    // divide by 0 case = +-90 deg
    angle = 1.5707963 * (dy/fabs(dy));
  }
  else {
    angle = atan2(dy,dx);
  }


  double aa = ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_angle;
  double a1= angle - aa;
  double a2= angle + aa;
  double ca1,sa1,ca2,sa2;
  sa1 = sin(a1);	ca1 = cos(a1);
  sa2 = sin(a2);	ca2 = cos(a2);

  // get the scaling factors of the receiving layer and the netview
  float sx = 1.0; float sy = 1.0;
  float nx = proj_g->to_g->GetCurrentXformScale();
  float ny = proj_g->to_g->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;
  nx = proj_g->net_g->GetCurrentXformScale();
  ny = proj_g->net_g->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;

  // compute the arrow head points
  _x[2] = _x[1] + (ca1*sx) * ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _y[2] = _y[1] + (sa1*sy) * ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _x[3] = _x[1] + (ca2*sx) * ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _y[3] = _y[1] + (sa2*sy) * ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;

  //  draw arrow head;
  c->new_path();
  c->move_to(_x[1],_y[1]);
  c->line_to(_x[2],_y[2]);
  c->line_to(_x[3], _y[3]);
  c->close_path();

  if(proj_g->pjn->projected == true) { // if projected then draw solid
    if(stroke != nil) c->fill(stroke);
  }
  else {			// otherwise draw open
    if (brush != nil && stroke != nil) {
      c->stroke(stroke, brush);
    }
  }
  
  if (ttb != nil) {
    c->pop_transform();
  }
  if (tt != nil) {
    c->pop_transform();
  }
  if (tn != nil) {
    c->pop_transform();
  }
}

bool PLine_G::intersects_gs (BoxObj& userb, Graphic* gs) { 
  ivCoord* convx, *convy;
  ivCoord ll, bb, rr, tt;
  getbounds_gs(ll, bb, rr, tt, gs);
  BoxObj b(ll, bb, rr, tt);;
  bool result = false;

  if (!_curved && !_fill) {
    if (b.Intersects(userb)) {
      convx = new ivCoord[_ctrlpts+1];
      convy = new ivCoord[_ctrlpts+1];
      this->Xform_gs(convx, convy, gs);
      if (_closed) {
	convx[_ctrlpts] = *convx;
	convy[_ctrlpts] = *convy;
	MultiLineObj ml(convx, convy, _ctrlpts+1);
	result = ml.Intersects(userb);
      } else {
	MultiLineObj ml(convx, convy, _ctrlpts);
	result = ml.Intersects(userb);
      }
      delete convx;
      delete convy;
    }
    return result;

  } else if (!_curved && _fill) {
    if (b.Intersects(userb)) {
      convx = new ivCoord[_ctrlpts];
      convy = new ivCoord[_ctrlpts];
      this->Xform_gs(convx, convy, gs);
      FillPolygonObj fp (convx, convy, _ctrlpts);
      result = fp.Intersects(userb);
      delete convx;
      delete convy;
    }
    return result;    

  } else if (_curved && !_fill) {
    if (b.Intersects(userb)) {
      convx = new ivCoord[_ctrlpts];
      convy = new ivCoord[_ctrlpts];
      this->Xform_gs(convx, convy, gs);
      MultiLineObj ml;
      if (_closed) {
	ml.ClosedSplineToPolygon(convx, convy, _ctrlpts);
      } else {
	ml.SplineToMultiLine(convx, convy, _ctrlpts);
      }
      result = ml.Intersects(userb);
      delete convx;
      delete convy;
    }
    return result;

  } else { // this is our case, not curved, not filled
    if (b.Intersects(userb)) {
      convx = new ivCoord[_ctrlpts];
      convy = new ivCoord[_ctrlpts];
      this->Xform_gs(convx, convy, gs);
      FillPolygonObj fp;
      fp.ClosedSplineToPolygon(convx, convy, _ctrlpts);
      result = fp.Intersects(userb);
      delete convx;
      delete convy;
    }
    return result;
  }
}

bool PLine_G::select(const ivEvent& e, Tool& tool, bool unselect) {
  int but = Graphic::GetButton(e);
  if(but == ivEvent::right)
    editb_used = true;
  else
    editb_used = false;
  return Polyline::select(e, tool, unselect);
}

bool PLine_G::effect_select(bool set_select) {
  bool tmp = Polyline::effect_select(set_select);
  if(set_select && editb_used) {
    proj_g->pjn->Edit();
  }
  editb_used = false;
  if(set_select)
    proj_g->net_g->selectgroup.LinkUnique(proj_g->pjn);
  else
    proj_g->net_g->selectgroup.Remove(proj_g->pjn);
  return tmp;
}


//////////////////////////////////
//	   SelfCon_Pline_G	//
//////////////////////////////////

SelfCon_PLine_G::SelfCon_PLine_G( Projection_G* pg, const ivBrush* brush, const ivColor* stroke,
  const ivColor* fill,
  ivCoord* x, ivCoord* y, int size, ivTransformer* tx)
: PLine_G(pg,brush,stroke,fill,x,y,size,tx){ // size better be 6
  side_index = 0;
}

void SelfCon_PLine_G::Xform_gs(ivCoord* tx, ivCoord* ty, Graphic* gs) {
  ivTransformer t = *gs->transformer();
  ivTransformer* tl = proj_g->from_g->transformer();
  ivTransformer* tb = proj_g->from_g->GetActualLayerBox()->transformer();
  if(tl) t.premultiply(*tl);
  if(tb)t.premultiply(*tb);

  // all points on from_g

  t.transform(_x[0],_y[0],tx[0],ty[0]);
  t.transform(_x[1],_y[1],tx[1],ty[1]);
  t.transform(_x[2],_y[2],tx[2],ty[2]);
  t.transform(_x[3],_y[3],tx[3],ty[3]);
  t.transform(_x[4],_y[4],tx[4],ty[4]);
  t.transform(_x[5],_y[5],tx[5],ty[5]);
}  

void SelfCon_PLine_G::ComputeArrowPoints(){
  double angle = 0;
  if(side_index == Layer_G::TOP){
    angle = 1.5707963;
  }
  if(side_index == Layer_G::LEFT){
    angle = 3.1415927;
  }
  if(side_index == Layer_G::RIGHT){
    angle = 0.0;
  }

  double aa = ((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_angle;
  double a1= angle - aa;
  double a2= angle + aa;
  double ca1,sa1,ca2,sa2;
  sa1 = sin(a1);	ca1 = cos(a1);
  sa2 = sin(a2);	ca2 = cos(a2);


  // get the scaling factors of the receiving layer and the netview
  float sx = 1.0; float sy = 1.0;
  float nx = proj_g->to_g->GetCurrentXformScale();
  float ny = proj_g->to_g->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;
  nx = proj_g->net_g->GetCurrentXformScale();
  ny = proj_g->net_g->GetCurrentYformScale();
  if(nx != 0) sx /= nx;
  if(ny != 0) sy /= ny;

  // compute the arrow head points
  _x[4] = _x[3] + ca1*sx*((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _y[4] = _y[3] + sa1*sy*((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _x[5] = _x[3] + ca2*sx*((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
  _y[5] = _y[3] + sa2*sy*((NetView *)proj_g->net_g->owner->owner)->prjn_arrow_size;
}

void SelfCon_PLine_G::draw_gs(ivCanvas* c, Graphic* gs) {
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();

  // transform for the network
  ivTransformer* tn = gs->transformer();
  if (tn != nil) {
    c->push_transform();
    c->transform(*tn);
  }

  // move_to the layer and start the line
  // if the layer's transformer has been flushed then
  // get the transform off the layer_box:
  ivTransformer* tl = proj_g->from_g->transformer();
  ivTransformer* tb = proj_g->from_g->GetActualLayerBox()->transformer();
  if (tl != nil) {
    c->push_transform();
    c->transform(*tl);
  }
  if(tb != nil) {
    c->push_transform();
    c->transform(*tb);
  }
  c->new_path();
  c->move_to(_x[0], _y[0]);
  c->line_to(_x[1],_y[1]);
  c->line_to(_x[2],_y[2]);
  c->line_to(_x[3],_y[3]);
  
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }
  
  // need to recompute the arrow points
  ComputeArrowPoints();

  //  draw arrow head;
  c->new_path();
  c->move_to(_x[3],_y[3]);
  c->line_to(_x[4],_y[4]);
  c->line_to(_x[5], _y[5]);
  c->line_to(_x[3],_y[3]);
  c->close_path();

  if(proj_g->pjn->projected == true) { // if projected then draw solid
    if(stroke != nil) c->fill(stroke);
  }
  else {			// otherwise draw open
    if (brush != nil && stroke != nil) {
      c->stroke(stroke, brush);
    }
  }
  
  if (tb != nil) {
    c->pop_transform();
  }
  if (tl != nil) {
    c->pop_transform();
  }
  if (tn != nil) {
    c->pop_transform();
  }
}

// the SelfCon_Pline_G draws an arch.
// it insures that the points line up.
//
//	1-------2
//	|	|
//	|     4 | 5
//	0      \3/

void SelfCon_PLine_G::SetX(int i, ivCoord val){
  // if you wanted to scale by the size of a unit, which you don't
//    float unit_xsz = proj_g->netg->tdp->scx;
//    float	unit_ysz = proj_g->netg->tdp->dfz;
  _x[i] = val;
  if(side_index == Layer_G::TOP){
    if(i == 0)_x[1] = val;
    if(i == 3)_x[2] = val;
  }
  if(side_index == Layer_G::RIGHT){
    if(i == 0)  _x[1] = val + SELF_CON_DIST;
    if(i == 3)  _x[2] = val + SELF_CON_DIST;
  }
  if(side_index == Layer_G::LEFT){
    if(i == 0)  _x[1] = val - SELF_CON_DIST;
    if(i == 3)  _x[2] = val - SELF_CON_DIST;
  }
  ComputeArrowPoints();
  recompute_shape();
}

void SelfCon_PLine_G::SetY(int i, ivCoord val){
  _y[i] = val;
  if(side_index == Layer_G::TOP){
    if(i == 0) _y[1] = val + SELF_CON_DIST;
    if(i == 3) _y[2] = val + SELF_CON_DIST;
  }
  if((side_index == Layer_G::RIGHT) || (side_index == Layer_G::LEFT)) {
    if(i == 0)_y[1] = val;
    if(i == 3)_y[2] = val;
  }
  ComputeArrowPoints();
  recompute_shape();
}

//////////////////////////////////
//	     Unit_G		//
//////////////////////////////////

Unit_G::Unit_G(Network_G* ng,Unit* u) 
: PolyGraphic(NULL){
  netg = ng;
  unit = u;
  cur_uvg = NULL;
  lastcanvas = NULL;
  Position();
  AddMD(NULL);
}

void Unit_G::Position() { // find the coords for the corners of the unitspace
  float pt_x_0, pt_y_0;
  netg->tdp.Project(unit->pos, pt_x_0, pt_y_0);
  
  x[0] = pt_x_0; 			y[0] = pt_y_0;		
  x[1] = pt_x_0 + netg->tdp.pt_x_1;	y[1] = pt_y_0;
  x[2] = pt_x_0 + netg->tdp.pt_x_2; 	y[2] = pt_y_0 + netg->tdp.pt_y_1;
  x[3] = pt_x_0 + netg->tdp.pt_x_3;	y[3] = pt_y_0 + netg->tdp.pt_y_1;

  DistributeUVGS();
}

UnitValue_G* Unit_G::NewUnitValue(MemberDef* md) {
  UnitValue_G* uvg = NULL;
  switch(netg->owner->shape) {
  case NetView::AREA:
    uvg = new AreaUnit_G(this, md);
    break;
  case NetView::LINEAR:
    uvg = new LinearUnit_G(this, md);
    break;
  case NetView::FILL:
    uvg = new FillUnit_G(this, md);
    break;
  case NetView::DIR_FILL:
    uvg = new DirFillUnit_G(this, md);
    break;
  case NetView::THREE_D:
    uvg = new ThreeDUnit_G(this, md);
    break;
  case NetView::ROUND:
    uvg =  new RoundUnit_G(this, md);
    break;
  case NetView::HGT_FIELD: 
    uvg = new HgtFieldUnit_G(this, md);
    break;
  case NetView::HGT_PEAKS: 
    uvg = new HgtPeakUnit_G(this, md);
    break;
  case NetView::COLOR:
  default:
    uvg = new SquareUnit_G(this, md);
    break;
  }
  return uvg;
}

void Unit_G::DistributeUVGS(int index){
  ivGlyphIndex c = count_();
  if(c == 0) return;
  if(index == -1) {
    int i;
    for(i=0;i<c;i++) DistributeUVGS(i);
    return;
  }
  UnitValue_G* uvg = (UnitValue_G*) component_(index);
  uvg->displabel = false;
  float mod1,mod2,offset1, offset2,offset3;
  switch(netg->owner->owner->unit_layout) {
  case NetView::LEFT_RIGHT:
    if(index == 0) uvg->displabel = true;
    mod1 = (x[1]-x[0])/c;
    offset1 = x[0] + mod1* index;
    offset2 = x[3] + mod1* index;
    uvg->SetPos(offset1,y[0],offset1+mod1,y[1],
		offset2+mod1,y[2],offset2,y[3]);
    break;
  case NetView::RIGHT_LEFT:
    if(index == count_()-1) uvg->displabel = true;
    mod1 = (x[1]-x[0])/c;
    offset1 = x[1] - mod1*index;
    offset2 = x[2] - mod1*index;
    uvg->SetPos(offset1-mod1,y[0],offset1,y[1],
		offset2,y[2],offset2-mod1,y[3]);
    break;
  case NetView::BOTTOM_TOP:
    if(index == count_()-1) uvg->displabel = true;
    mod1 = (y[3]-y[0])/c;
    mod2 = (x[3]-x[0])/c;
    offset1 = y[0] + mod1* index;
    offset2 = x[0] + mod2* index;
    offset3 = x[1] + mod2* index;
    uvg->SetPos(offset2,offset1,offset3,offset1,
		offset3+mod2,offset1+mod1,offset2+mod2,offset1+mod1);
    break;
  case NetView::TOP_BOTTOM:
    if(index == 0) uvg->displabel = true;
    mod1 = (y[3]-y[0])/c;
    mod2 = (x[3]-x[0])/c;
    offset1 = y[3] - mod1* index;
    offset2 = x[3] - mod2* index;
    offset3 = x[2] - mod2* index;
    uvg->SetPos(offset2-mod2,offset1-mod1,offset3-mod2,offset1-mod1,
		offset3,offset1,offset2,offset1);
    break;
  }
}

int Unit_G::FindValue_G(UnitValue_G* uvg){
  ivGlyphIndex i;
  ivGlyphIndex c = count_();
  for(i=0;i<c;i++){
    if(((UnitValue_G *)component_(i)) == uvg){
      return i;
    }
  }
  return -1;
}

void Unit_G::AddMD(MemberDef* md, bool chosen){
  if(!chosen) {
    RemoveMD(md); return;
  }
  ivGlyphIndex i;
  ivGlyphIndex c = count_();
  for(i=0;i<c;i++){
    // take out bogus non memberdef objects since we have a real one now
    UnitValue_G* uvg = (UnitValue_G *) component_(i);
    if(uvg->act_md == NULL) {
      remove_(i); i--; c--; continue;
    }
    // if this md is already there then don't add another one
    if (((uvg->spec_md == md) || (uvg->act_md == md)) &&
	uvg->pickindex == -1) return;
  }
  UnitValue_G* unvg = NewUnitValue(md);
  append_(unvg);
  DistributeUVGS(); // reposition all the uvg's
}
  
void Unit_G::ChangeMD(MemberDef* md) {
  ivGlyphIndex c = count_();
  if(c == 0) {
    AddMD(md, true); // if there are no current md's add one
    return;
  } 
  int i;
  for(i=(int) c-1; i>=0 ; i--){
    UnitValue_G* uvg = (UnitValue_G *) component_(i);    
    uvg->SetCurMD(md); // change the md
    return; // just do one
  }
}

void Unit_G::RemoveMD(MemberDef* md) {
  ivGlyphIndex i;
  ivGlyphIndex c = count_();
  for(i=0;i<c;i++){
    UnitValue_G* uvg = (UnitValue_G *) component_(i);
    if((uvg->spec_md == md) || (uvg->act_md == md)) {
      remove_(i);
      DistributeUVGS();
      return;
    }
  }
}

bool Unit_G::grasp_move(const ivEvent&,Tool& ,ivCoord, ivCoord){
  netg->owner->UpdatePosivPatch(unit->pos); return true;
}  

bool Unit_G::manip_move(const ivEvent& , Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0))
    return true;

  dx = cx - ix;
  dy = cy - iy;
  TDCoord ng;
  ng.z = 0;
  switch(netg->owner->viewer->last_button) {
  case(ivEvent::right):
    netg->tdp.InverseProjXZ(dx, dy, ng); break;
  default:
    netg->tdp.InverseProjXY(dx, dy, ng); break;
  }
  TDCoord og = unit->pos;
  unit->pos += ng;
  Layer* lay = GET_OWNER(unit,Layer);
  if((unit->pos.x >=0) && (unit->pos.y >= 0) &&
     ((lay == NULL) || ((unit->pos.x < lay->geom.x) && (unit->pos.y < lay->geom.y)))) {
    netg->owner->UpdatePosivPatch(unit->pos);
    safe_damage_me(netg->owner->viewer->canvas());
    Position();			// don't actually move that much 
  }
  unit->pos = og;
  return true;
}

bool Unit_G::effect_move(const ivEvent& , Tool&,ivCoord ix, ivCoord iy, ivCoord fx, ivCoord fy) {
  ivCoord dx = fx - ix;
  ivCoord dy = fy - iy;

  TDCoord ng;
  ng.z = 0;
  switch(netg->owner->viewer->last_button) {
  case(ivEvent::right):
    netg->tdp.InverseProjXZ(dx, dy, ng); break;
  default:
    netg->tdp.InverseProjXY(dx, dy, ng); break;
  }
  if((ng.x != 0) || (ng.y != 0) || (ng.z != 0)) {
    TDCoord og = unit->pos;
    unit->pos += ng;
    Layer* lay = GET_OWNER(unit,Layer);
    if((unit->pos.x >=0) && (unit->pos.y >= 0) &&
       ((lay == NULL) || ((unit->pos.x < lay->geom.x) && (unit->pos.y < lay->geom.y)))) {
      tabMisc::NotifyEdits(unit);
      Position();
    }
    else unit->pos = og;
  }
  netg->owner->ClearPosivPatch();
  return true;
}

bool Unit_G::grasp_alter(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,ivCoord,ivCoord,ivCoord){
  if(count_() == 0 ) return true;
  ivGlyphIndex i = 0;
  ivGlyphIndex c = count_();
  while(i<c) {
    cur_uvg = (UnitValue_G *) component_(i);
    if(cur_uvg->act_md != NULL)
      break;
    i++;
  }
  if(i==c) cur_uvg = NULL;
  else {
    float val = cur_uvg->GetDisplayVal();
    netg->owner->UpdatePosivPatch(val);
  }
  return true;
}

bool Unit_G::manip_alter(const ivEvent&, Tool&,ivCoord, ivCoord, ivCoord, ivCoord,
			 ivCoord, ivCoord cy, ivCoord, ivCoord)
{
  if (cur_uvg == NULL) return true;
  float maxy = netg->owner->viewer->viewallocation().top();
  float miny = netg->owner->viewer->viewallocation().bottom();
  float ceny = (maxy + miny)/2.0;
  float val;
  ScaleBar* cbar = netg->owner->cbar;
  if(cy > ceny) val = (((cy-ceny)/(maxy-ceny)) * (cbar->max - cbar->zero)) + cbar->zero;
  else if (cy < ceny)  {
    val = (((ceny-cy)/(ceny-miny)) * (cbar->min - cbar->zero)) + cbar->zero;
  }
  else val = cbar->zero;
  switch(cur_uvg->md_type) {
  case UnitValue_G::FLOAT:
    *((float*)(cur_uvg->base)) = (float) val; break;
  case UnitValue_G::DOUBLE:
    *((double*)(cur_uvg->base)) = (double) val; break;
  case UnitValue_G::INT:
    *((int*)(cur_uvg->base)) = (int) floor((double)val); break;
  default: break;
  }
  cur_uvg->update_from_state(lastcanvas);
  if(netg->owner->shape >= NetView::HGT_FIELD)
    ((HgtFieldUnit_G *) cur_uvg->component_(0))->UpdateNeighbors(lastcanvas);
  netg->owner->UpdatePosivPatch(val);
  return true;
}

bool Unit_G::manipulating(const ivEvent& e, Tool& tool) {
  ivWindow* w = e.window();
  lastcanvas = w->canvas();
  return PolyGraphic::manipulating(e,tool);
}

bool Unit_G::effect_alter(const ivEvent& , Tool& ,ivCoord, ivCoord, ivCoord, ivCoord, ivCoord, ivCoord) {
  netg->owner->ClearPosivPatch();
  cur_uvg = NULL;
  return true;
}

bool Unit_G::select(const ivEvent& e, Tool& tool, bool unselect){
  if(unselect){
    if(_selected)     netg->selectgroup.Remove(unit);
    _selected = false;
  }
  else {
    _selected = !_selected;
    if(_selected) netg->selectgroup.LinkUnique(unit);
    else netg->selectgroup.Remove(unit);
  }
  long i;
  for(i=0; i < count_(); i++)
    component_(i)->select(e, tool, !_selected);
  return true;
}

void Unit_G::spread_pick(const ivEvent& e, Tool&, bool pick_state) {
  if(_picked == pick_state) return;
  _picked = pick_state;
  if(pick_state)
    netg->changed_pickgroup = netg->pickgroup.LinkUnique(unit) ||
      netg->changed_pickgroup;
  else
    netg->changed_pickgroup = netg->pickgroup.RemoveEl(unit) ||
      netg->changed_pickgroup;
  long i;
  for(i=0; i < count_(); i++) {
    UnitValue_G* uvg = ((UnitValue_G *) component_(i));
    if(uvg->effect_pick(_picked))
      uvg->damage_me(e.window()->canvas());
  }
}

void Unit_G::Fix_UVGS() {
  // iterate though the memberdefs on the ordered_uvg_list, and
  // ensure that there are the right # of uvgs, and the uvgs match up

  ivGlyphIndex old_size = count_();
  int_Array& oul = netg->owner->owner->ordered_uvg_list;
  int pos = 0;
  //  pick state temporaries
  ivEvent ev;
  ev.window(netg->owner->win);
#ifdef CYGWIN
  // cygwin doesn't implement above call, so need to do this manually
  ev.rep()->windowSet(netg->owner->win);
#endif
  Tool tl;
  int i;
  for(i=0;i<oul.size;i++){
    int md_idx = oul[i];
    MemberDef* md = netg->membs[md_idx];
    UnitValue_G* unvg = NULL;
    if(count_() <= pos) { // there is not a unvg there
      unvg = NewUnitValue(md);
      insert_(pos,unvg); 
    }
    else { // the unvg for this pos already exists
      unvg = (UnitValue_G*)component_(pos); 
      unvg->SetCurMD(md); // ensure mds matchup
    }
    if(unvg->pickindex != -2) { // this is a send/recv uvg;
      if(unvg->pickindex != 0) { // the pickindex is not correct
	unvg->pickindex = 0;
	unvg->RePick();
      }
      // iterate though the pickgroup and set the uvgs correctly
      int j;
      pos++; // move to the next position
      for(j=1;j<netg->pickgroup.size;j++){
	if(count_() <= pos) { // there is not a unvg there
	  unvg = NewUnitValue(md);
	  insert_(pos,unvg);
	}
	else { // the unvg for this pos already exists
	  unvg = (UnitValue_G *)component_(pos); 
	  unvg->SetCurMD(md); // ensure mds matchup
	}
	if(unvg->pickindex != j) { // pickindex is off
	  unvg->pickindex = j;
	  unvg->RePick();
	}
	pos++;
      }
      continue;
    }
    pos++;
  }
  while(count_() > pos) remove_(count_()-1);
  if(count_() != old_size) DistributeUVGS();
}


//////////////////////////////////
//	   UnitValue_G		//
//////////////////////////////////

UnitValue_G::UnitValue_G(Unit_G* u, MemberDef* md, int num, bool curved) 
: Graphic(u->netg->unitbrush,u->netg->border,
	  u->netg->owner->cbar->bar->scale->nocolor.color,
	  nil,true,curved,num,nil) {
  ug = u;
  spec_md = NULL;
  act_md = NULL;
  base = NULL;
  pickindex = -2;
  md_type = UNKNOWN;
  prv_val = -2222.222;
  displabel = false;
  SetCurMD(md);
}

void UnitValue_G::FixType(){
  md_type = UNKNOWN;
  if((act_md != NULL) && (act_md->type != NULL)) {
    if(act_md->type->InheritsFrom(&TA_float))
      md_type = FLOAT;
    else if(act_md->type->InheritsFrom(&TA_double))
      md_type = DOUBLE;
    else if(act_md->type->InheritsFrom(&TA_int))
      md_type = INT;
  }
}

// this gets the base pointer for values in connections
void UnitValue_G::UpdateConBase() {
  prv_val = -2222.222;
  if(spec_md == NULL) return;
  String nm = spec_md->name.before(".");
  if(nm.empty() || !((nm=="s") || (nm == "r"))) return; // only for cons
  Con_Group* cong;
  if(nm=="s")
    cong = &(ug->unit->recv);
  else
    cong = &(ug->unit->send);
  base = NULL;
  act_md = NULL;
  md_type = UNKNOWN;
  nm = spec_md->name.after(".");
  if(pickindex == -2) pickindex = -1; // a connection uvg without a connection
  if((cong->leaves == 0) || (pickindex < 0) || (pickindex >= ug->netg->pickgroup.size))
    return;
  Unit* src_u = (Unit*)(ug->netg->pickgroup[pickindex]);
  Con_Group* tcong;
  int g;
  FOR_ITR_GP(Con_Group, tcong, cong->, g) {
    act_md = tcong->prjn->con_type->members.FindName(nm);
    if(act_md == NULL)	continue;
    Connection* con = tcong->FindConFrom(src_u);
    if(con == NULL) continue;
    base = act_md->GetOff(con);
    FixType(); return;
  }
}

void UnitValue_G::SetCurMD(MemberDef* md) {
  prv_val = -2222.222;
  if((md == NULL) || (ug->unit == NULL)) {
    base = NULL; spec_md = act_md = NULL;
    FixType(); return;
  }

  String nm = md->name.before(".");
  if(nm.empty()) { // member is a unit member
    pickindex = -2; // clear pick index;
    if(spec_md == md) return; // already set!
    spec_md = md;
    if((act_md = ug->unit->FindMember((const char*)md->name)) == NULL){
      FixType(); return;
    }
    base = act_md->GetOff(ug->unit);
  }
  else if((nm=="s") || (nm == "r")) { // sending or receiving connection
    spec_md = md;
    UpdateConBase();
  }
  else { // member is a suboject
    pickindex = -2; // clear pick index
    if(spec_md == md) return; // already set!
    spec_md = md;
    MemberDef* par_md = ug->unit->FindMember((const char*)nm);
    if(par_md == NULL) {
      act_md = NULL; FixType(); return;
    }
    nm = md->name.after(".");
    TypeDef* nptd = par_md->type;
    void* par_base = par_md->GetOff(ug->unit);
    if(nptd->ptr == 1) {
      par_base = *((void**)par_base); // its a pointer, so de-reference it
      if(par_base == NULL) {
	act_md = NULL; FixType(); return;
      }
      nptd = ((TAPtr)par_base)->GetTypeDef(); // get actual typedef
    }
    MemberDef* smd = nptd->members.FindName(nm);
    if(smd == NULL) {
      act_md = NULL; FixType(); return;
    }
    base = smd->GetOff(par_base);
    act_md = smd;
  }
  FixType();
}

float UnitValue_G::GetDisplayVal(){
  if((act_md == NULL) || (base == NULL) || (md_type < 0)) {
    return 0.0f;
  }
  else {
    switch(md_type) {
    case FLOAT:
      return *((float*)base);
    case DOUBLE:
      return *((double*)base);
    case INT:
      return  *((int*)base);
    case UNKNOWN:
      return 0.0f;
    }
  }
  return 0.0f;
}

void UnitValue_G::SetPos(ivCoord, ivCoord, ivCoord,ivCoord,
			  ivCoord, ivCoord, ivCoord,ivCoord) {
}

bool UnitValue_G::select(const ivEvent& e, Tool&, bool) {
  if(effect_select(ug->is_selected())) {
    damage_me(e.window()->canvas());
    return true;
  }
  else {
    _selected = ug->is_selected();
    return false;
  }
}

void UnitValue_G::RePick() {
  UpdateConBase();
}

bool UnitValue_G::pick_me(const ivEvent& e, Tool& tool, bool unpick){
  if(unpick && !_picked)
    return false;
  bool rval = Graphic::pick_me(e, tool, unpick);
  ug->spread_pick(e,tool,_picked);
  return rval;
}

bool UnitValue_G::update_from_state(ivCanvas* c) {
  float val = GetDisplayVal();
  ScaleBar* cbar = ug->netg->owner->cbar;
  if(ug->netg->update_scale_only && ug->netg->owner->auto_scale){ 
    if(fabs(val) > cbar->range){
      cbar->ModRoundRange(val);
      return true;
    }
    return false;
  }
  ivColor* fl;  ivColor* tx;
  if((act_md == NULL) || (base == NULL) || (md_type < 0)) {
    fl = cbar->bar->scale->nocolor.color;
    tx = cbar->bar->scale->nocolor.contrastcolor;
    if(fl != fill()) {
      fill(fl);
      damage_me(c);
      return true;
    }
    return false;
  } 

  cbar->GetColor(val,&fl,&tx);
  bool diff_color = false;
  if(fill() != fl) {
    diff_color = true;
    fill(fl);
  }
  if(!diff_color && (val == prv_val)) return false;
  prv_val = val;
  damage_me(c);
  return false;
}

void UnitValue_G::render_text(ivCanvas* c, ScaleBar* cbar, float val, String& str,
			      bool from_top)
{
  ivColor* fl;  ivColor* tx;
  cbar->GetColor(val,&fl,&tx);

  ivFontBoundingBox bbox;
  ug->netg->owner->owner->unit_font.fnt->font_bbox(bbox);
  float fy=bbox.ascent() + bbox.descent();

  float nx,ny;
  float ex,ey;
  if(from_top) {
    c->transformer().transform(_x[3],_y[3],nx,ny);
    c->transformer().transform(_x[2],_y[2],ex,ey);
    ny -= fy;
  }
  else {
    c->transformer().transform(_x[0],_y[0],nx,ny);
    c->transformer().transform(_x[1],_y[1],ex,ey);
    ny += 1.0f;
    nx += 3.0f;
  }
  c->push_transform(); // store the line*net transform;
  ivTransformer nt = c->transformer();
  nt.invert();			// invert current transformer
  c->transform(nt);		// put inverse in there -- effectively null xform
  int len = str.length();
  float cx = nx;
  int j;
  for(j=0;j<len;j++){	// draw each character
    ug->netg->owner->owner->unit_font.fnt->char_bbox(str[j], bbox);
    float nxtx = cx + bbox.width();
    if(nxtx > ex) break;
    c->character((const ivFont*)(ug->netg->owner->owner->unit_font.fnt),str[j],8,tx,cx,ny);
    cx = nxtx;
  }
  c->pop_transform(); // remove null transform
}

void UnitValue_G::draw_text(ivCanvas* c, float val) {
  ScaleBar* cbar = ug->netg->owner->cbar;
  if((ug->netg->owner->unit_text == NetView::VALUES) ||
     (ug->netg->owner->unit_text == NetView::BOTH)) {
    String val_str = (String)val;
    if(val_str.contains("e-")) val_str = "0.0";
    render_text(c, cbar, val, val_str);
  }
  if(displabel && ((ug->netg->owner->unit_text == NetView::NAMES) ||
     (ug->netg->owner->unit_text == NetView::BOTH))) {
    render_text(c, cbar, val, ug->unit->name, true);
  }
}

//////////////////////////////////
//	SquareUnit_G		//
//////////////////////////////////

SquareUnit_G::SquareUnit_G(Unit_G* u, MemberDef* md) : UnitValue_G(u,md,4,false) {
}

void SquareUnit_G::draw_gs(ivCanvas* c, Graphic* gs) {
  if(ug->netg->owner->owner->unit_border < 0)
    gs->stroke(nil);

  Graphic::draw_gs(c, gs);

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  float val = GetDisplayVal();
  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

void SquareUnit_G::SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
			   ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3) {
  _x[0] = x0; _x[1] = x1; _x[2] = x2; _x[3] = x3;
  _y[0] = y0; _y[1] = y1; _y[2] = y2; _y[3] = y3;
  _ctrlpts = 4;
  recompute_shape();
}

//////////////////////////////////
//	AreaUnit_G		//
//////////////////////////////////

AreaUnit_G::AreaUnit_G(Unit_G* u, MemberDef* md) : SquareUnit_G(u,md) {
  bg_color = NULL;
  fg_color = NULL;
}

AreaUnit_G::~AreaUnit_G(){
  if(fg_color) ivResource::unref(fg_color); fg_color = NULL;
  if(bg_color) ivResource::unref(bg_color); bg_color = NULL;
}

void AreaUnit_G::GetColors(ivCanvas*, Graphic*) {
  // first fix the colors
  float val = GetDisplayVal();
  ScaleBar* cbar = ug->netg->owner->cbar;

  if(bg_color == NULL) {
    bg_color = cbar->bar->scale->
      GetColor(((int) (.5f * (float) (cbar->bar->scale->chunks-1))));
    ivResource::ref(bg_color);
  }
  if(fg_color) ivResource::unref(fg_color);
  if(val > cbar->max)
    fg_color = cbar->bar->scale->maxout.color;
  else if(val < cbar->min)
    fg_color = cbar->bar->scale->minout.color;
  else {
    if(val >= cbar->zero)
      fg_color = cbar->bar->scale->GetColor(cbar->bar->scale->colors.size-1);
    else 
      fg_color = cbar->bar->scale->GetColor(0);

    if(fg_color->alpha() < 0.5f){
      ivColorIntensity r,g,b;
      cbar->bar->scale->background.color->intensities(r,g,b);
      fg_color = new ivColor(r,g,b,1.0-fg_color->alpha());
    }
  }
  ivResource::ref(fg_color);
}

void AreaUnit_G::draw_text(ivCanvas* c, float val) {
  ScaleBar* cbar = ug->netg->owner->cbar;
  if((ug->netg->owner->unit_text == NetView::VALUES) ||
     (ug->netg->owner->unit_text == NetView::BOTH)) {
    String val_str = (String)val;
    if(val_str.contains("e-")) val_str = "0.0";
    render_text(c, cbar, cbar->max, val_str); // max = better contrast color
  }
  if(displabel && ((ug->netg->owner->unit_text == NetView::NAMES) ||
     (ug->netg->owner->unit_text == NetView::BOTH))) {
    render_text(c, cbar, cbar->max, ug->unit->name, true);
  }
}

void AreaUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  GetColors(c, gs);
  ScaleBar* cbar = ug->netg->owner->cbar;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  float val = GetDisplayVal();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  //first draw the background
  c->new_path();
  c->move_to(_x[0], _y[0]);
  for (int i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  c->close_path();
  if(bg_color != NULL) c->fill(bg_color);
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop,temp;
  c->transformer().transform(_x[0],_y[0],cleft,cbottom);
  c->transformer().transform(_x[1],_y[1],cright,temp);
  c->transformer().transform(_x[2],_y[2],temp,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = top-bottom;
  int max_area = width * height;
  float area = ((float) max_area) * cbar->GetAbsPercent(val);
  if(area > max_area) area = max_area;

  ivPixelCoord max_size = (ivPixelCoord) sqrt(fabs((double) max_area));
  ivPixelCoord box_size = (ivPixelCoord) sqrt(fabs((double) area));

  int x_size = (int) ((((float) width)/(float) max_size) * (float) box_size);
  int y_size = (int) (((float) (height)/(float) max_size) * (float) box_size);

  ivPixelCoord xboxoff = (width - x_size) / 2;
  ivPixelCoord yboxoff = (height - y_size) / 2;

  ivCoord x0,y0,x3,y3;
  c->transformer().transform(_x[0],_y[0],x0,y0);
  c->transformer().transform(_x[3],_y[3],x3,y3);

  c->push_transform(); // save old xform

  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  c->new_path();
  float posmod =  ((float) yboxoff/(float) height) * (float) (x3-x0);
  x0 += posmod;
  x3 -= posmod;
  c->move_to(x0 + c->to_coord(xboxoff), y0 + c->to_coord(yboxoff));
  c->line_to(x0 + c->to_coord(xboxoff+x_size), y0 + c->to_coord(yboxoff));
  c->line_to(x3 + c->to_coord(xboxoff+x_size), y0 + c->to_coord(yboxoff+y_size));
  c->line_to(x3 + c->to_coord(xboxoff), y0 + c->to_coord(yboxoff+y_size));

  c->close_path();
  if(fg_color != NULL) c->fill(fg_color);

  c->pop_transform(); // remove null xform

  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}


//////////////////////////////////
//	LinearUnit_G		//
//////////////////////////////////

LinearUnit_G::LinearUnit_G(Unit_G* g, MemberDef* md) : AreaUnit_G(g,md) {
}

void LinearUnit_G::draw_gs(ivCanvas* c, Graphic*gs){
  GetColors(c, gs);
  ScaleBar* cbar = ug->netg->owner->cbar;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  float val = GetDisplayVal();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  //first draw the background
  c->new_path();
  c->move_to(_x[0], _y[0]);
  for (int i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  c->close_path();
  if(bg_color != NULL) c->fill(bg_color);
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop,temp;
  c->transformer().transform(_x[0],_y[0],cleft,cbottom);
  c->transformer().transform(_x[1],_y[1],cright,temp);
  c->transformer().transform(_x[2],_y[2],temp,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = top-bottom;

  int x_size = (int) (((float) width) * cbar->GetAbsPercent(val));
  int y_size = (int) (((float) height) * cbar->GetAbsPercent(val));

  if(y_size > height) y_size = height;
  if(x_size > width) x_size = width;

  ivPixelCoord xboxoff = (width - x_size) / 2;
  ivPixelCoord yboxoff = (height - y_size) / 2;

  ivCoord x0,y0,x3,y3;
  c->transformer().transform(_x[0],_y[0],x0,y0);
  c->transformer().transform(_x[3],_y[3],x3,y3);

  c->push_transform(); // save old xform

  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  c->new_path();
  float posmod =  ((float) yboxoff/(float) height) * (float) (x3-x0);
  x0 += posmod;
  x3 -= posmod;
  c->move_to(x0 + c->to_coord(xboxoff), y0 + c->to_coord(yboxoff));
  c->line_to(x0 + c->to_coord(xboxoff+x_size), y0 + c->to_coord(yboxoff));
  c->line_to(x3 + c->to_coord(xboxoff+x_size), y0 + c->to_coord(yboxoff+y_size));
  c->line_to(x3 + c->to_coord(xboxoff), y0 + c->to_coord(yboxoff+y_size));

  c->close_path();
  if(fg_color != NULL) c->fill(fg_color);

  c->pop_transform(); // remove null xform

  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

//////////////////////////////////
//	FillUnit_G		//
//////////////////////////////////

FillUnit_G::FillUnit_G(Unit_G* g, MemberDef* md) : AreaUnit_G(g,md) {
}

void FillUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  GetColors(c, gs);
  ScaleBar* cbar = ug->netg->owner->cbar;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  float val = GetDisplayVal();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  //first draw the background
  c->new_path();
  c->move_to(_x[0], _y[0]);
  for (int i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  c->close_path();
  if(bg_color != NULL) c->fill(bg_color);
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop,temp;
  c->transformer().transform(_x[0],_y[0],cleft,cbottom);
  c->transformer().transform(_x[1],_y[1],cright,temp);
  c->transformer().transform(_x[2],_y[2],temp,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = top-bottom;
  int max_area = width * height;
  float area = ((float) max_area) * cbar->GetAbsPercent(val);
  if(area > max_area) area = max_area;
  if(area == 0) {
    if (tx != nil)       c->pop_transform();  // remove unit xform
    return;
  }

  int y_size = (int) ((float) (area)/ (float) width);
  int d = y_size * width; // actual number of pixels covered by box

  ivCoord x0,y0,x3,y3;
  c->transformer().transform(_x[0],_y[0],x0,y0);
  c->transformer().transform(_x[3],_y[3],x3,y3);
  c->push_transform(); // save old xform

  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  float posmod =  ((float) y_size/(float) height) * (float) (x3-x0);
  x3 = x0 + posmod;
  c->new_path();
  c->move_to(x0,y0);
  c->line_to(x0 + c->to_coord(width),y0);
  c->line_to(x3 + c->to_coord(width), y0 + c->to_coord(+y_size));
  c->line_to(x3, y0 + c->to_coord(+y_size));
  c->close_path();
  if(fg_color != NULL) c->fill(fg_color);
  if(area-d > 0) {
    c->new_path();
    c->move_to(x3,y0 + c->to_coord(y_size +1));
    c->line_to(x3 + c->to_coord((int) area-d),
	       y0 + c->to_coord(y_size +1));
    c->close_path();
    if(fg_color != NULL) c->stroke(fg_color,brush);
  }
  c->pop_transform(); // remove null xform

  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

//////////////////////////////////
//   DirFillUnit_G		//
//////////////////////////////////

DirFillUnit_G::DirFillUnit_G(Unit_G* g, MemberDef* md) : AreaUnit_G(g,md) {
}

void DirFillUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  GetColors(c, gs);
  ScaleBar* cbar = ug->netg->owner->cbar;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  float val = GetDisplayVal();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  //first draw the background
  c->new_path();
  c->move_to(_x[0], _y[0]);
  for (int i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  c->close_path();
  if(bg_color != NULL) c->fill(bg_color);
  if (brush != nil && stroke != nil) {
    c->stroke(stroke, brush);
  }

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop,temp;
  c->transformer().transform(_x[0],_y[0],cleft,cbottom);
  c->transformer().transform(_x[1],_y[1],cright,temp);
  c->transformer().transform(_x[2],_y[2],temp,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = (top-bottom)/2;
  int max_area = width * height;
  float area = ((float) max_area) * cbar->GetAbsPercent(val);
  if(area > max_area) area = max_area;
  if(area == 0) {
    if (tx != nil)       c->pop_transform();  // remove unit xform
    return;
  }

  int y_size = (int) ((float) (area)/ (float) width);

  int d = y_size * width; // actual number of pixels covered by box

  ivCoord x0,y0,x3,y3;
  c->transformer().transform(_x[0],_y[0],x0,y0);
  c->transformer().transform(_x[3],_y[3],x3,y3);
  c->push_transform(); // save old xform

  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  float midposmod = (.5f * (float) (x3-x0));
  c->new_path();
  c->move_to(x0+midposmod,y0+height); // midline
  c->line_to(x0+midposmod + c->to_coord(width),y0+height);
  if(val >= cbar->zero) { // positive
    float posmod =  ((float) (y_size + height))/((float) (height*2))
      * (float) (x3-x0);
    x3 = x0 + posmod;
    c->line_to(x3 + c->to_coord(width), y0 + c->to_coord(y_size+height));
    c->line_to(x3, y0 + c->to_coord(height+y_size));
  }
  else {
    float posmod =  ((float) (height - y_size))/((float) (height*2))
      * (float) (x3-x0);    
    x3 = x0 + posmod;
    c->line_to(x3 + c->to_coord(width), y0 + c->to_coord(height-y_size));
    c->line_to(x3, y0 + c->to_coord(height-y_size));
  }
  c->close_path();
  if(fg_color != NULL) c->fill(fg_color);
  if(area-d > 0) {
    c->new_path();
    if(val >= cbar->zero) { // positive
      c->move_to(x3,y0 + c->to_coord(height + y_size +1));
      c->line_to(x3 + c->to_coord((int) area-d),
		 y0 + c->to_coord(height+y_size +1));
    }
    else {
      c->move_to(x3,y0 + c->to_coord(height - y_size));
      c->line_to(x3 + c->to_coord((int) area-d),
		 y0 + c->to_coord(height- y_size));
    }
    c->close_path();
    if(fg_color != NULL) c->stroke(fg_color,brush);
  }
  c->pop_transform(); // remove null xform

  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

//////////////////////////////////
//	ThreeDUnit_G		//
//////////////////////////////////

ThreeDUnit_G::ThreeDUnit_G(Unit_G* u, MemberDef* md) : SquareUnit_G(u, md) {
  last_height = 0.0f;
}

void ThreeDUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  float val = GetDisplayVal();
  ScaleBar* cbar = ug->netg->owner->cbar;
  float height = _y[3]-_y[0];
  if(val > cbar->max) height = height;
  else if(val >= cbar->zero) {
    if(cbar->max == cbar->zero) height = 0.0f;
    else height *= (val - cbar->zero)/(cbar->max -cbar->zero);
  }
  else if(val >= cbar->min) {
    if(cbar->max == cbar->zero) height = 0.0f;
    else height *= (val - cbar->zero)/(cbar->zero -cbar->min);
  }
  else height = - height;
  float zfactor = ug->netg->owner->owner->spacing.z;
  height /= (1.0f - zfactor);

  last_height = height;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  const ivColor* fill = gs->fill();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  if(height != 0.0f){
    c->new_path();		// back panel
    c->move_to(_x[3],_y[3]+height);    c->line_to(_x[2],_y[2]+height);
    c->line_to(_x[2],_y[2]);    c->line_to(_x[3],_y[3]);
    c->close_path();
    c->fill(fill);
    c->stroke(stroke, brush);
    c->new_path();		// left side panel
    c->move_to(_x[0],_y[0]+height);    c->line_to(_x[3],_y[3]+height);
    c->line_to(_x[3],_y[3]);	    c->line_to(_x[0],_y[0]);
    c->close_path();
    c->fill(fill);
    c->stroke(stroke, brush);
  }
  if(height != 0.0f){
    c->new_path();		// right side panel
    c->move_to(_x[1],_y[1]);    c->line_to(_x[2],_y[2]);
    c->line_to(_x[2],_y[2]+height);    c->line_to(_x[1],_y[1]+height);
    c->close_path();
    c->fill(fill);
    c->stroke(stroke, brush);

    c->new_path();		// front panel
    c->move_to(_x[0],_y[0]);    c->line_to(_x[1],_y[1]);
    c->line_to(_x[1],_y[1]+height);    c->line_to(_x[0],_y[0]+height);
    c->close_path();
    c->fill(fill);
    c->stroke(stroke, brush);
  }
  if(height < 0.0f) height = 0.0f; // draw top part of negative blocks
  c->new_path();		// top/bottom  panel
  c->move_to(_x[0],_y[0] + height);  c->line_to(_x[1],_y[1] + height);
  c->line_to(_x[2],_y[2] + height);  c->line_to(_x[3],_y[3] + height);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

void ThreeDUnit_G::getextent_gs(ivCoord& l, ivCoord& b, ivCoord& cx, ivCoord& cy,
				ivCoord& tol, Graphic* gs) {
  tol = 1.0;
  const ivBrush* br = gs->brush();
  if (br != nil) {
    ivCoord width = ivCoord(br->width());
    tol = (width > 1) ? width : tol;
  }
  if (_ctrlpts > 0) {
    float mxht = (_y[3]-_y[0]);
    float zfactor = ug->netg->owner->owner->spacing.z;
    mxht /= (1.0f - zfactor);

    ivCoord left = _xmin, bottom = _ymin - mxht;
    ivCoord right = _xmax, top = _ymax + mxht;
    ivTransformer* t = gs->transformer();

    if (t != nil) {
      corners(left, bottom, right, top, *t);
    } 
    l = left;
    b = bottom;
    cx = (left + right)/2.0;
    cy = (top + bottom)/2.0;
  }
}

//////////////////////////////////
//	RoundUnit_G		//
//////////////////////////////////

RoundUnit_G::RoundUnit_G(Unit_G* u, MemberDef* md) : UnitValue_G(u,md,7,true) {
}

void RoundUnit_G::draw_gs(ivCanvas* c, Graphic* gs) {
  Graphic::draw_gs(c, gs);
  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  float val = GetDisplayVal();
  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

void RoundUnit_G::SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
			 ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3) {
  _x[1] = x0; _y[1] = y0;		
  _x[2] = x1; _y[2] = y1;
  _x[4] = x2; _y[4] = y2;
  _x[5] = x3; _y[5] = y3;
  
  _x[0] = (_x[1]+_x[5]) / 2.0;    _y[0] = (_y[1]+_y[5]) / 2.0;
  _x[3] = (_x[2]+_x[4]) / 2.0;    _y[3] = (_y[2]+_y[4]) / 2.0;      
  _x[6] = _x[0]; _y[6]=_y[0];

  _ctrlpts =  7;
  recompute_shape();
}


//////////////////////////////////
//	HgtFieldUnit_G		//
//////////////////////////////////

HgtFieldUnit_G::HgtFieldUnit_G(Unit_G* u, MemberDef* md) : SquareUnit_G(u,md) {
  height = 0.0f;
  oldval = -FLT_MAX;
  corner_heights[0]= corner_heights[1]= corner_heights[2]= corner_heights[3]=0.0f;
  height = 0.0f;
}

void HgtFieldUnit_G::SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
			    ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3) {
  SquareUnit_G::SetPos(x0,y0,x1,y1,x2,y2,x3,y3);
  // reset everything so that it will rebuild neighbors next time an update from state occurs!
  int i;
  for(i=neighbors.count()-1;i>=0;i--)
    neighbors.remove(i);
  corner_heights[0]= corner_heights[1]= corner_heights[2]= corner_heights[3]=0.0f;
  height = 0.0f;
}

bool HgtFieldUnit_G::update_from_state(ivCanvas* c) {
  GetNeighbors();
  return SquareUnit_G::update_from_state(c);
}

void HgtFieldUnit_G::GetNeighbors() {
  if(neighbors.count() != 0) return;
  Layer* lay = GET_OWNER(ug->unit,Layer);
  // get our layer_g
  long i;
  Layer_G* layg = NULL;
  for(i=0; i<ug->netg->count_(); i++) {
    Graphic* g = ug->netg->component_(i);
    if((g->GetTypeDef() == &TA_Layer_G) &&
       (((Layer_G *) g)->lay == lay)){
      layg = (Layer_G *) g;
      break;
    }
  }
  if(layg == NULL) return;
  TwoDCoord pos = ug->unit->pos;
  TwoDCoord newpos;
  int x, y;
  for(y=-1;y<2;y++){
    for(x=-1;x<2;x++) {
      if(x==0 && y==0) { neighbors.append(this); continue; }
      newpos.x = pos.x + x;
      newpos.y = pos.y + y;
      Unit* u = lay->FindUnitFmCoord(newpos);
      // find unit_g with unit u
      if(u==NULL) { neighbors.append(NULL); continue; }
      Unit_G* nug = NULL; 
      Unit_Group* oug = (Unit_Group*)u->owner;
      bool found = false;
      for(int j=0; (!found && (j<layg->count_())); j++) { // scan unit groups
	Unit_Group_G* ugg= (Unit_Group_G*) layg->component_(j);
	if(ugg->InheritsFrom(TA_Unit_Group_G) && (ugg->group == oug)) {
	  int k;
	  for(k=0; k < ugg->count_(); k++) { // scan unit_g's
	    nug = (Unit_G *)ugg->component_(k);
	    if(nug->unit == u) {
	      found = true;
	      break;
	    }
	  }
	}
      }
      if(!found) {
	neighbors.append(NULL);
	continue;
      }
      found = false;
      // now find a UnitValue_G 
      int m;
      for(m=0;m<nug->count_();m++) { 
	UnitValue_G* nuvg = (UnitValue_G *) nug->component_(m);
	if(nuvg->InheritsFrom(&TA_HgtFieldUnit_G)) {
	  found = true; neighbors.append(nuvg);
	  break;
	}
      }
      if(!found) neighbors.append(NULL);
    }
  }
}

void HgtFieldUnit_G::ComputeHeight() {
  float val = GetDisplayVal();
  if(val == oldval) return;
  oldval = val;
  height = _y[3]-_y[0];
  ScaleBar* cbar = ug->netg->owner->cbar;
  if(val > cbar->max) height = height;
  else if(val >= cbar->zero) {
    if(cbar->max == cbar->zero) height = 0.0f;
    else height *= (val - cbar->zero)/(cbar->max -cbar->zero);
  }
  else if(val >= cbar->min) {
    if(cbar->max == cbar->zero) height = 0.0f;
    else height *= (val - cbar->zero)/(cbar->zero -cbar->min);
  }
  else height = - height;
  float zfactor = ug->netg->owner->owner->spacing.z;
  height /= (1.0f - zfactor);
}  

void HgtFieldUnit_G::ComputeCorners() {
  if(neighbors.count() == 0) return;
  int i;
  for(i=0;i<9;i++) {
    HgtFieldUnit_G* hfu = (HgtFieldUnit_G*)neighbors.component(i);
    if(hfu == NULL) continue;
    hfu->ComputeHeight();	// inefficient, but what the heck..
  }

  for(int l=0;l<4;l++) {
    int c1,c2,c3; // neighbor idicies for this corner

    if(l==0)      { c1=3; c2=0; c3=1;}
    else if(l==1) { c1=1; c2=2; c3=5;}
    else if(l==2) { c1=7; c2=5; c3=8;}
    else          { c1=6; c2=3; c3=7;}
    float divby = 1.0f;
    corner_heights[l] = height;
    if(neighbors.component(c1) != NULL) {
      corner_heights[l] += ((HgtFieldUnit_G*)neighbors.component(c1))->height;
      divby += 1.0f;
    }
    if(neighbors.component(c2) != NULL) {
      corner_heights[l] += ((HgtFieldUnit_G*)neighbors.component(c2))->height;
      divby += 1.0f;
    }
    if(neighbors.component(c3) != NULL) {
      corner_heights[l] += ((HgtFieldUnit_G*)neighbors.component(c3))->height;
      divby += 1.0f;
    }
    corner_heights[l] /= divby;
  }
}

void HgtFieldUnit_G::UpdateNeighbors(ivCanvas* c){
  int i;     
  for(i=0;i<9;i++) {
    HgtFieldUnit_G* hfg = (HgtFieldUnit_G *) neighbors.component(i);
    if(hfg) hfg->update_from_state(c);
  }
}
 
void HgtFieldUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  ComputeCorners();
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  const ivColor* fill = gs->fill();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }
  c->new_path();		// top/bottom  panel
  c->move_to(_x[0],_y[0] + corner_heights[0]);  c->line_to(_x[1],_y[1] + corner_heights[1]);
  c->line_to(_x[2],_y[2] + corner_heights[2]);  c->line_to(_x[3],_y[3] + corner_heights[3]);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  float val = GetDisplayVal();
  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}


void HgtFieldUnit_G::getextent_gs(ivCoord& l, ivCoord& b, ivCoord& cx, ivCoord& cy,
				  ivCoord& tol, Graphic* gs)
{
  tol = 1.0;
  const ivBrush* br = gs->brush();
  if (br != nil) {
    ivCoord width = ivCoord(br->width());
    tol = (width > 1) ? width : tol;
  }
  if (_ctrlpts > 0) {
    float mxht = (_y[3]-_y[0]);
    float zfactor = ug->netg->owner->owner->spacing.z;
    mxht /= (1.0f - zfactor);

    ivCoord left = _xmin, bottom = _ymin - mxht;
    ivCoord right = _xmax, top = _ymax + mxht;
    ivTransformer* t = gs->transformer();

    if (t != nil) {
      corners(left, bottom, right, top, *t);
    } 
    l = left;
    b = bottom;
    cx = (left + right)/2.0;
    cy = (top + bottom)/2.0;
  }
}

//////////////////////////////////
//	HgtPeakUnit_G		//
//////////////////////////////////

HgtPeakUnit_G::HgtPeakUnit_G(Unit_G* u, MemberDef*) : HgtFieldUnit_G(u) {
}

void HgtPeakUnit_G::draw_gs(ivCanvas* c, Graphic*gs) {
  ComputeCorners();
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  const ivColor* fill = gs->fill();

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }
  float xcenter = (_x[0] + _x[1] + _x[2] + _x[3] )/4.0;
  float ycenter = (_y[0] + _y[1] + _y[2] + _y[3] )/4.0;
  ycenter += height;
  c->new_path();		// back face
  c->move_to(xcenter,ycenter);
  c->line_to(_x[2],_y[2] + corner_heights[2]);  c->line_to(_x[3],_y[3] + corner_heights[3]);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  c->new_path();		// left face
  c->move_to(xcenter,ycenter);
  c->line_to(_x[3],_y[3] + corner_heights[3]);  c->line_to(_x[0],_y[0] + corner_heights[0]);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  c->new_path();		// right face
  c->move_to(xcenter,ycenter);
  c->line_to(_x[1],_y[1] + corner_heights[1]);  c->line_to(_x[2],_y[2] + corner_heights[2]);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  c->new_path();		// front
  c->move_to(xcenter,ycenter);
  c->line_to(_x[0],_y[0] + corner_heights[0]);  c->line_to(_x[1],_y[1] + corner_heights[1]);
  c->close_path();
  c->fill(fill);
  c->stroke(stroke, brush);

  float val = GetDisplayVal();
  draw_text(c, val);

  if (tx != nil) {
    c->pop_transform();  // remove unit xform
  }
}

//////////////////////////////////
//	GraphicCallBack		//
//////////////////////////////////

typedef void (GlyphViewer::*ToolSetter) (unsigned int);

class GraphicCallback : public ivAction {
public:
  GraphicCallback(GlyphViewer*, ToolSetter, unsigned int);
  virtual void execute();
private:
  GlyphViewer* _gv;
  ToolSetter _tool;
  unsigned _type;
};

GraphicCallback::GraphicCallback (GlyphViewer* gv, ToolSetter tool, unsigned int t) {
  _gv = gv;
  _tool = tool;
  _type = t;
}

void GraphicCallback::execute () {
  /* workaround for cfront bug */
  GlyphViewer* gv = _gv;
  ToolSetter tool = _tool;
  unsigned int type = _type;
  (gv->*tool)(type);
}

//////////////////////////////////
//	  NetViewer		//
//////////////////////////////////

static ivCursor* picker_cursor = nil;
static ivCursor* mywindow_cursor = nil;

NetViewer::NetViewer(NetView* n, float w, float h, const ivColor* bg)
: GlyphViewer(w,h,bg) {
  netv = n;
};

void NetViewer::release(const ivEvent& ev) {
  GlyphViewer::release(ev);
}

void NetViewer::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  GlyphViewer::allocate(c,a,ext);
  update_from_state();
  Network_G* netg = (Network_G*)_root;
  netg->SetMyTransform(viewallocation());
}

//////////////////////////////////
//	StateSelButton		//
//////////////////////////////////

class StateSelButton : public ivButton {
  // this is a button for selecting state variables
public:
  NetEditor* 	ned;
  String	varname;
  int 		md_idx;		// index into the membs structure for this one
  int		auto_tool;
  int		press_button;	// button actually pressed

  void		press(const ivEvent& e); // don't indent on right button
  void		release(const ivEvent& e); // process mouse release..
  void          FixDisplay();
  bool		IsChosen();	// is this button selected

  static StateSelButton* GetButton(char* txt,NetEditor* n,int mdx, int at=-1);
  StateSelButton(char* txt,ivGlyph* g,ivStyle* s,  ivTelltaleState* t, NetEditor* n,
		 int mdx, int at= -1);
};  

StateSelButton::StateSelButton(char* txt,ivGlyph* g, ivStyle* s,ivTelltaleState* t, 
			       NetEditor* n, int mdx, int at)
: ivButton(new ivPatch(g),s,t,NULL){
  ned = n;
  varname = txt;
  md_idx = mdx;
  auto_tool = at;
  state()->set(ivTelltaleState::is_choosable | 
	       ivTelltaleState::is_toggle |
	       ivTelltaleState::is_enabled, true);
  state()->set(ivTelltaleState::is_chosen, false);
}


StateSelButton* StateSelButton::GetButton(char* txt,NetEditor* n,
					  int mdx, int at){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* ts = new ivTelltaleState;
  ivGlyph* look = wkit->palette_button_look
    (wkit->label(txt),ts);
  return new StateSelButton(txt,look,new ivStyle(wkit->style()),ts,n,mdx,at);
}
  
bool StateSelButton::IsChosen() {
  return state()->test(ivTelltaleState::is_chosen);
}
  
void StateSelButton::press(const ivEvent& e) {
  press_button = taivM->GetButton(e);
  if(press_button != ivEvent::right) {
    ivButton::press(e);
  }
}

void StateSelButton::release(const ivEvent& e) {
  if (!inside(e))    return;
  ivTelltaleState* s = state();
  int_Array& oul = ned->owner->ordered_uvg_list;
  bool prev_chosen = s->test(ivTelltaleState::is_chosen); // previously chosen
  if(press_button == ivEvent::right) {
    MemberDef* md = ned->netg->membs[md_idx];
    String message = md->name + " : " + md->desc;
    taMisc::Choice(message.chars(),"Ok");
    return;
  }
  else  if(press_button == ivEvent::middle) {
    s->set(ivTelltaleState::is_chosen |
	   ivTelltaleState::is_active, !prev_chosen);
    if(prev_chosen) {
      // clean out all instances of this mdef
      int remove_pos;
      while((remove_pos = oul.Find(md_idx)) != -1) oul.Remove(remove_pos,1);
      varname += '~'; // mark for redrawing
      DMEM_GUI_RUN_IF {
	if(taivMisc::record_script != NULL) {
	  *taivMisc::record_script << ned->owner->GetPath() << ".SelectVar(\"\");" << endl;
	}
      }
    }
    else {
      DMEM_GUI_RUN_IF {
	if(taivMisc::record_script != NULL) {
	  *taivMisc::record_script << ned->owner->GetPath() << ".SelectVar(\"" <<
	    varname << "\",true);" << endl;
	}
      }
      oul.Add(md_idx);
    }
  }
  else if  (press_button == ivEvent::left) {
    // remove all the other ones but the last one, and then change it
    int i = ned->owner->ordered_uvg_list.size;
    DMEM_GUI_RUN_IF {
      if(taivMisc::record_script != NULL) {
	*taivMisc::record_script << ned->owner->GetPath() << ".SelectVar(\"" <<
	  varname << "\");" << endl;
      }
    }
    if(i==0) {// none selected
      s->set(ivTelltaleState::is_chosen |  ivTelltaleState::is_active, true);
      oul.Add(md_idx);
    }
    else { // at least one selected
      StateSelButton* ssb;
      for(i=i-1;i>=0;i--){ // remove all
	if(ned->var_gp.count() <= oul[i]) {
	  oul.Remove((uint) i, 1);
	  continue;
	}
	ssb = (StateSelButton *)  ned->var_gp.component(oul[i]);
	oul.Remove((uint) i,1);
	if(ssb->varname.lastchar() != '~') {
	  ssb->varname += '~'; // mark for redrawing
	  ssb->state()->set(ivTelltaleState::is_chosen |  ivTelltaleState::is_active, false);
	}
      }
      s->set(ivTelltaleState::is_chosen |  ivTelltaleState::is_active, true);
      oul.Add(md_idx); // replace
    }
  }
  bool chosen = s->test(ivTelltaleState::is_chosen); // chosen now
  if((!prev_chosen) && (chosen)){ // it was chosen
    if(auto_tool != -1){
      ned->SelectActButton(auto_tool);
    }
  }
  FixDisplay();
}

void StateSelButton::FixDisplay(){
  int_Array& oul = ned->owner->ordered_uvg_list;
  if(oul.size > 0)  {
    ned->netg->cur_mbr = oul.Peek();
    ned->netg->safe_damage_me(ned->viewer->canvas());
    if((oul.size == 1) && (oul.Peek() == md_idx)) { // we are the only thing on the list
      NetView* ntv = ned->owner;
      MemberDef* md = (MemberDef *) ned->netg->membs[oul[0]];
      NetViewScaleRange* nvsr =
	(NetViewScaleRange *) ntv->scale_ranges.FindName(md->name);
      if (nvsr == NULL) {
	nvsr = (NetViewScaleRange *)
	  ntv->scale_ranges.New(1,&TA_NetViewScaleRange);
	nvsr->name = md->name;
	nvsr->auto_scale = ned->auto_scale;
	nvsr->minmax.min = ned->cbar->min;
	nvsr->minmax.max = ned->cbar->max;
      }
      else {
	ntv->auto_scale = ned->auto_scale = nvsr->auto_scale;
	ntv->scale_min = nvsr->minmax.min;
	ntv->scale_max = nvsr->minmax.max;
	ned->cbar->SetMinMax(nvsr->minmax.min, nvsr->minmax.max);
	ned->as_ckbox->state()->set(ivTelltaleState::is_chosen,nvsr->auto_scale);
      }
      tabMisc::NotifyEdits(ntv);
    }
  }    
  else ned->netg->cur_mbr = -1;
  ned->NumberVarButtons();
  ned->RePick();
  ned->UpdateDisplay(); // do not check for display_toggle
}

//////////////////////////////////
//	   NetEditor		//
//////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(NetEditor)
implementActionCallback(NetEditor)
#include <ta/leave_iv.h>

declare_taivMenuCallback(NetEditor)
implement_taivMenuCallback(NetEditor)

NetEditor::NetEditor(Network* nt, NetView* nv, ivWindow* w) {
  net = nt;
  owner = nv;
  viewer = NULL;
  win = w;
  ckbox = NULL;
  pospatch =NULL;
  var_rep = NULL; 
  var_patch = NULL;
  scale = NULL;
  cbar = NULL;
  as_ckbox = NULL;
  ut_ckbox = NULL;
  tool_gp = NULL;
  statbutton=NULL;
  mstatmenu=NULL;
  utextmenu=NULL;
  dispmdmenu=NULL;
  buttonpatch = NULL;
  new_but = NULL;
  connect_but = NULL;
  remove_but = NULL;
  edit_but = NULL;
  build_but = NULL;
  con_but = NULL;
  
  unit_text = NetView::NONE;
  shape = NetView::THREE_D;
  display_toggle = true;
  
  if (picker_cursor == nil) {
    ivBitmap* picker = new ivBitmap(picker_bits, picker_width, picker_height,
				picker_x_hot, picker_y_hot);
    ivBitmap* picker_mask = new ivBitmap(pickerMask_bits, pickerMask_width, pickerMask_height,
				     pickerMask_x_hot, pickerMask_y_hot);
    picker_cursor = new ivCursor(picker, picker_mask);
  }
  
  if (mywindow_cursor == nil) {
    mywindow_cursor = win->cursor();
  }
  win->cursor(picker_cursor);
}

NetEditor::~NetEditor() {
  if (netg != NULL) { ivResource::unref(netg); netg = NULL; }
  if (viewer != NULL) { ivResource::unref(viewer); viewer = NULL; }
  if (ckbox != NULL) { ivResource::unref(ckbox); ckbox = NULL;  }
  if (pospatch != NULL) { ivResource::unref(pospatch); pospatch = NULL; }
  if (var_rep != NULL) { ivResource::unref(var_rep); var_rep = NULL; }
  if (cbar != NULL) { ivResource::unref(cbar); cbar = NULL; }
  if (scale != NULL) { taBase::unRefDone(scale); scale = NULL; }
  if (as_ckbox != NULL) { ivResource::unref(as_ckbox); as_ckbox = NULL; }
  if (ut_ckbox != NULL) { ivResource::unref(ut_ckbox); ut_ckbox = NULL; }
  if (tool_gp != NULL) { ivResource::unref(tool_gp); tool_gp = NULL; }
  statbutton = NULL;
  if (mstatmenu != NULL) { delete mstatmenu; mstatmenu = NULL; }
  if (utextmenu != NULL) { delete utextmenu; utextmenu = NULL; }
  if (dispmdmenu != NULL) { delete dispmdmenu; dispmdmenu = NULL; }
  if (buttonpatch != NULL) { ivResource::unref(buttonpatch); buttonpatch = NULL; }
}

void NetEditor::Init() {
  scale = new ColorScale();
  taBase::Own(scale,net);

  var_rep = new tbScrollBox();
  ((tbScrollBox *) var_rep)->naturalnum = 3;
  ivResource::ref(var_rep);

  netg = new Network_G(net); // in this case mgr is not a unit_mgr
  netg->owner = this;
  netg->skew  = owner->skew;
  ivResource::ref(netg);
  net->UpdateMax();

  cbar = new VCScaleBar(0.0,true,true,netg->defbrush,scale);
  ivResource::ref(cbar);

  viewer = new NetViewer((NetView *) owner,NET_VIEW_INITIAL_X,
			 NET_VIEW_INITIAL_Y, scale->Get_Background());
  ivResource::ref(viewer);
  viewer->root(netg);
  netg->SetMax(net->max_size);

  netg->Build();

  if(owner->frozen_net_form != NULL) {
    netg->transformer(owner->frozen_net_form->transformer());
  }
  else {
    if(netg->transformer()) {
      ivTransformer* t = netg->transformer();
      ivTransformer idnty;
      *t = idnty;			// return to identity
      netg->scale(NET_VIEW_INITIAL_X - 2.0f * NET_VIEW_X_MARG,
		  NET_VIEW_INITIAL_Y - 2.0f * NET_VIEW_Y_MARG);
      netg->translate(-(.5f * (float)NET_VIEW_INITIAL_X),
		      -(.5f * (float)NET_VIEW_INITIAL_Y));
    }
    else {
      netg->scale(NET_VIEW_INITIAL_X - 2.0f * NET_VIEW_X_MARG,
		  NET_VIEW_INITIAL_Y - 2.0f * NET_VIEW_Y_MARG,0.5,0.5);
    }
  }
  auto_scale = true;
}

//////////////////////////////////////////
// 	NetEditor:: display construct	//
//////////////////////////////////////////

ivGlyph* NetEditor::GetLook() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();

  ivRequisition r;
  ivGlyph* dbox = layout->hbox
    (taivM->medium_button(wkit->palette_button("",NULL)),
     taivM->hfsep,
     taivM->medium_button(wkit->palette_button("",NULL)));
  dbox->request(r);
  ivCoord w = r.x_requirement().natural();
  delete dbox;

  GetVars();
  statbutton = wkit->menubar();
  BuildMStatMenu();

  var_patch = new ivPatch(var_rep);
  
  ivPolyGlyph* ctrl_box = layout->vbox
    (GetTools(), 
     taivM->vfsep,
     wkit->menu_item_separator_look(),
     statbutton,
     wkit->menu_item_separator_look(),
     taivM->vfsep,
     layout->hbox
     (wkit->vscroll_bar(var_rep),
      layout->vbox
      (layout->hspace(w), // this is a bogus width holder
       var_patch)
      )
     );
    
  cbar->SetAdjustNotify(new ActionCallback(NetEditor)(this,&NetEditor::CBarAdjust));
  as_ckbox = wkit->check_box("",new ActionCallback
			     (NetEditor)(this,&NetEditor::MenuToggleAutoScale));
  ivResource::ref(as_ckbox);
  as_ckbox->state()->set(ivTelltaleState::is_chosen,auto_scale);

  print_patch = new ivPatch(viewer);
  BuildUTextMenu();
  BuildDispMdMenu();

  const ivColor* bg_color = net->GetEditColor();
  if(bg_color == NULL) bg_color = wkit->background();
  
  return taivM->layout->vbox
    (wkit->inset_frame
	 (layout->hbox
	  (layout->vcenter
	   (wkit->outset_frame
	    (new ivBackground(ctrl_box, bg_color)),0),
	   taivM->hfsep,
	   layout->vcenter
	   ((wkit->inset_frame(print_patch)),0),
	   layout->vcenter
	   (layout->vbox
	    (layout->hcenter(wkit->fancy_label("Unit Text"),0),
	     layout->hcenter(layout->hfixed(utextmenu->GetRep(),50),0),
	     layout->hcenter(wkit->fancy_label("Disp Mode"),0),
	     layout->hcenter(layout->hfixed(dispmdmenu->GetRep(),50),0),
	     layout->hcenter(wkit->fancy_label("Auto Scale"),0),
	     layout->hcenter(as_ckbox,0),
	     layout->hcenter(wkit->inset_frame
			     (layout->vflexible
			      (layout->hflexible
			       (cbar->GetLook(),fil,0))),0)),0))));

//  always let the cbar be adjustable, adjusting the cbar turns off auto
//  otherwise it auto is on it will just override the adjustments
//  cbar->adjustflag = false; // turnoff adjusting  
}

ivGlyph* NetEditor::GetTools() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();

  tool_gp = new ivTelltaleGroup;
  ivResource::ref(tool_gp);
  ckbox = wkit->check_box("Display",
			  new ActionCallback(NetEditor)
			  (this, &NetEditor::MenuToggleDisplay));
  ivResource::ref(ckbox);

  ClearPosivPatch();
  ckbox->state()->set(ivTelltaleState::is_chosen,display_toggle);
  
  int i;
  for(i=0;i<9;i++){
    actbutton[i]=NULL;
  }
  actbutton[Tool::select] = MEButton
    (tool_gp,"Select",
     new ActionCallback(NetEditor)(this,&NetEditor::SetSelect));
  ((NoUnSelectButton *) actbutton[Tool::select])->chooseme();

  actbutton[Tool::pick]= MEButton
    (tool_gp,"View",
     new ActionCallback(NetEditor)(this,&NetEditor::SetPick));

  actbutton[Tool::move] =  MEButton
    (tool_gp,"Move",
     new ActionCallback(NetEditor)(this,&NetEditor::SetMove));

  actbutton[Tool::scale] =  MEButton
    (tool_gp,"Zoom",
     new ActionCallback(NetEditor)(this,&NetEditor::SetReScale));

  actbutton[Tool::stretch] =  MEButton
    (tool_gp,"ReShape",
     new ActionCallback(NetEditor)(this,&NetEditor::SetReShape));

  actbutton[Tool::rotate] =  MEButton
    (tool_gp,"Rotate",
     new ActionCallback(NetEditor)(this,&NetEditor::SetRotate));

  actbutton[Tool::alter] =  MEButton
    (tool_gp,"Alter Value",
     new ActionCallback(NetEditor)(this,&NetEditor::SetAlter));

  
  connect_but = new ivDeck(CON_COUNT);
  connect_but->append(wkit->push_button("Connect",new ActionCallback(NetEditor)
					(this, &NetEditor::ConnectObjects)));
  ((ivButton*) connect_but->component(0))->state()->set(ivTelltaleState::is_enabled,false);
  connect_but->append(wkit->push_button("New Label",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("New Prjn(s)",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("New Self Prjn",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("Fill Prjn(s)",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("Connect Units",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("SelfCon Unit",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("Edit Con",new ActionCallback(NetEditor)
		      (this, &NetEditor::EditConnection)));
  wkit->push_style(taivM->apply_button_style);
  connect_but->append(wkit->push_button("New Prjn(s)",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  connect_but->append(wkit->push_button("Fill Prjn(s)",new ActionCallback(NetEditor)
		      (this, &NetEditor::ConnectObjects)));
  wkit->pop_style();

  new_but = new ivDeck(NEW_COUNT);
  new_but->append(wkit->push_button("New", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  ((ivButton*) new_but->component(0))->state()->set(ivTelltaleState::is_enabled,false);
  new_but->append(wkit->push_button("New Layer(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  new_but->append(wkit->push_button("New BiPrjns", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  new_but->append(wkit->push_button("BiCon Units", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  new_but->append(wkit->push_button("Rmv Con", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveConnection)));
  new_but->append(wkit->push_button("New Unit(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  wkit->push_style(taivM->apply_button_style);
  new_but->append(wkit->push_button("New Layer(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  new_but->append(wkit->push_button("New Unit(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::NewObjects)));
  wkit->pop_style();

  remove_but = new ivDeck(REMOVE_COUNT);
  remove_but->append(wkit->push_button("Remove", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  ((ivButton*) remove_but->component(0))->
    state()->set(ivTelltaleState::is_enabled,false);
  remove_but->append(wkit->push_button("Rmv Layer(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv UnitGp(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv Prjn(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv Unit(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv Label(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv Grphic(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));
  remove_but->append(wkit->push_button("Rmv All", new ActionCallback(NetEditor)
		       (this, &NetEditor::RemoveSelections)));

  edit_but = new ivDeck(EDIT_COUNT);
  edit_but->append(wkit->push_button("Edit", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  ((ivButton*) edit_but->component(0))->state()->set(ivTelltaleState::is_enabled,false);
  edit_but->append(wkit->push_button("Edit Name(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit Layer(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit UnitGp(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit Prjn(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit Unit(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit Label(s)", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));
  edit_but->append(wkit->push_button("Edit All", new ActionCallback(NetEditor)
		       (this, &NetEditor::EditSelections)));

  connect_action = NEW_LABEL;
  new_action = HOT_NEW_LAYERS;
  remove_action = NO_REMOVE;
  edit_action = NO_EDIT;

  connect_but->flip_to(connect_action);
  new_but->flip_to(new_action);
  remove_but->flip_to(remove_action);
  edit_but->flip_to(edit_action);

  ivPolyGlyph* uibox = 
    layout->hbox
    (taivM->medium_button
     (new ScriptButton("Update", new ActionCallback(NetEditor)
		      (this, &NetEditor::UpdateDisplay),
		      owner->GetPath() + ".UpdateDisplay();")),
     layout->hglue(),
     taivM->medium_button
     (new ScriptButton("Init", new ActionCallback(NetEditor)
		      (this, &NetEditor::InitDisplay),
		      owner->GetPath() + ".InitDisplay();\n")));

  build_but = new HiLightButton("Build All", new ActionCallback(NetEditor)
				(this, &NetEditor::BuildAll), net->GetPath() + ".Build();\n");
  con_but = new HiLightButton("Connect All",  new ActionCallback(NetEditor)
			      (this, &NetEditor::ConnectAll), net->GetPath() + ".Connect();\n");

  buttonpatch = 
    new ivPatch(layout->vbox
		(wkit->inset_frame(layout->vbox
				   (layout->hbox(taivM->medium_button(build_but),
						 layout->hglue(),
						 taivM->medium_button(con_but)),
				    uibox
				    )),
		 wkit->inset_frame(layout->vbox
				   (layout->hbox(taivM->medium_button(new_but),
						 layout->hglue(),
						 taivM->medium_button(connect_but)),
				    layout->hbox(taivM->medium_button(remove_but),
						 layout->hglue(),
						 taivM->medium_button(edit_but))
				    ))));
  ivResource::ref(buttonpatch);

  return (layout->hflexible
	  (layout->vbox
	   (layout->hbox(layout->vcenter(ckbox,0),layout->hglue(5),
			 layout->vcenter
			 (wkit->inset_frame(pospatch),0),
			 layout->hglue()),
	    taivM->vfsep,
	    wkit->inset_frame
	    (layout->vbox
	     (layout->hbox
	      (taivM->medium_button((ivButton *) actbutton[Tool::select]),
	       layout->hglue(),
	       taivM->medium_button((ivButton *) actbutton[Tool::pick])),
	      layout->hbox
	      (taivM->medium_button((ivButton *) actbutton[Tool::move]),
	       layout->hglue(),
	       taivM->medium_button((ivButton *) actbutton[Tool::stretch])),
	      layout->hbox
	      (taivM->medium_button((ivButton *) actbutton[Tool::scale]),
	       layout->hglue(),
	       taivM->medium_button((ivButton *) actbutton[Tool::rotate])))),
	      //	      layout->hflexible(taivM->medium_button((ivButton *) actbutton[Tool::alter])))),
	    taivM->vfsep,buttonpatch)));
}

ivGlyph* NetEditor::GetVars() {
  ivLayoutKit* layout = ivLayoutKit::instance();

  ivGlyphIndex c;
  for(c=var_gp.count()-1; c>= 0; c--) // clear out any existing
    var_gp.remove(c);

  for(c=var_rep->count()-1; c>= 0; c--) // clear out any existing
    var_rep->remove(c);
  
  if(netg->membs.size == 0)
    return var_rep;
  
  int i;
  MemberDef* md;
  ivGlyph* hbox = NULL;
  ivButton* but;
  int at;			// auto_tool

  for(i=0; i < netg->membs.size; i++) {
    md = netg->membs[i];
    if((i % 2) == 0) {
      if(i != 0)
	var_rep->append(hbox);
      hbox = layout->hbox();
    }
    if( (md->name.before(2) == "r.") || (md->name.before(2) == "s.")){
      at = Tool::pick;
    }
    else {
      at = Tool::select;
    }
    but = VarButton(md->name, i,at);
    if((owner->ordered_uvg_list.Find(i)) != -1){
      but->state()->set(ivTelltaleState::is_chosen | ivTelltaleState::is_active,true);
      // if list is size 1 make sure that there is a scale_range entry for this one
      if (owner->ordered_uvg_list.size == 1) {
	NetViewScaleRange* nvsr = (NetViewScaleRange *)
	  owner->scale_ranges.FindName(md->name);
	if(nvsr == NULL) {
	  nvsr = (NetViewScaleRange *) owner->scale_ranges.New(1,&TA_NetViewScaleRange);
	  nvsr->name = md->name;
	  nvsr->auto_scale = auto_scale;
	  nvsr->minmax.min = cbar->min;
	  nvsr->minmax.max = cbar->max;
	}
	else {
	  owner->auto_scale = auto_scale = nvsr->auto_scale;
	  owner->scale_min = nvsr->minmax.min;
	  owner->scale_max = nvsr->minmax.max;
	  cbar->SetMinMax(nvsr->minmax.min, nvsr->minmax.max);
	}
      }
    }
    hbox->append(taivM->medium_button(but));
    if((i % 2) == 0)
      hbox->append(taivM->hfsep);
  }
  
  if((i % 2) == 0)
    hbox->append(taivM->hsep);
  var_rep->append(hbox);
  if(var_patch != NULL) {
    var_patch->reallocate();
    var_patch->redraw();
    if((win->is_mapped()) && (win->bound())) win->repair();
  }
  return var_rep;
}

ivButton* NetEditor::MEButton(ivTelltaleGroup* gp, char* txt, ivAction* a) {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* t = new Teller((TelltaleFlags)0, (ivAction*)NULL);
  ivButton* rval = new NoUnSelectButton
    (wkit->palette_button_look(wkit->label(txt), t), wkit->style(),t,a);
  rval->state()->set(ivTelltaleState::is_choosable | ivTelltaleState::is_toggle |
	ivTelltaleState::is_enabled, true);
  rval->state()->join(gp);
  return rval;
}

ivButton* NetEditor::VarButton(char* txt, int mdx, int at) {
  ivButton* rval = StateSelButton::GetButton(txt,this,mdx,at);
  var_gp.append(rval);
  return rval;
}

//////////////////////////////////////////
// 	NetEditor:: updating/init	//
//////////////////////////////////////////

void NetEditor::UpdateDisplay(){ // ignores display_toggle
  viewer->update_from_state();
  if((win->is_mapped()) && (win->bound())) win->repair();
}

void NetEditor::update_from_state() {
  if(display_toggle) UpdateDisplay();
}

void NetEditor::InitDisplay() {
  net->UpdateMax();
  netg->Build();
  GetVars();
  NumberVarButtons();
  netg->SetMyTransform(viewer->viewallocation());
  netg->RepositionPrjns();
  UpdateDisplay();
  int i;
  for(i=0;i<owner->idraw_graphics.size;i++){
    NetViewGraphic* netvg = (NetViewGraphic *) owner->idraw_graphics.FastEl(i);
    netvg->UpdateAfterEdit();
  }
  FixEditorButtons();
}

//////////////////////////////////////////
// 	NetEditor:: unit text menu	//
//////////////////////////////////////////

void NetEditor::BuildUTextMenu(){
  utextmenu = new taivMenu(taivMenu::menubar,taivMenu::normal_update,
			    taivMenu::skinny_small);
  TypeDef* flagenm = TA_NetView.sub_types.FindName("UnitTextDisplay");
  int i;
  for(i=0;i<flagenm->enum_vals.size;i++) {
    utextmenu->AddItem(flagenm->enum_vals[i]->name,NULL,taivMenu::use_default,
		       new taivMenuCallback(NetEditor)(this,&NetEditor::MenuSetUText));
  }
  unit_text = owner->unit_text;
  utextmenu->GetImage(unit_text);
  if(utextmenu->items.FastEl(unit_text)->rep){
    utextmenu->items.FastEl(unit_text)
      ->rep->state()->set(ivTelltaleState::is_chosen,false);
  }
}

void NetEditor::SetUText(taivMenuEl* sel) {
  unit_text = owner->unit_text = ( NetView::UnitTextDisplay) sel->itm_no;
  sel->rep->state()->set(ivTelltaleState::is_chosen,false);
}

void NetEditor::MenuSetUText(taivMenuEl* sel){ // sets unit_text from menu
  SetUText(sel);
  owner->InitDisplay();
  tabMisc::NotifyEdits(owner);
}

void NetEditor::UpdateUTextMenu(NetView::UnitTextDisplay utd){
  if(utextmenu){
    utextmenu->GetImage(owner->unit_text);
    SetUText(utextmenu->items.FastEl(utd));
  }
}

//////////////////////////////////////////
// 	NetEditor:: display mode menu	//
//////////////////////////////////////////

void NetEditor::BuildDispMdMenu(){
  dispmdmenu = new taivMenu(taivMenu::menubar,taivMenu::normal_update,
			    taivMenu::skinny_small);
  TypeDef* flagenm = TA_NetView.sub_types.FindName("UnitShape");
  int i;
  for(i=0;i<flagenm->enum_vals.size;i++) {
    dispmdmenu->AddItem(flagenm->enum_vals[i]->name,NULL,taivMenu::use_default,
		       new taivMenuCallback(NetEditor)
		       (this,&NetEditor::MenuSetDispMd));
  }
  dispmdmenu->GetImage(shape);
  if(dispmdmenu->items.FastEl(shape)->rep) {
    dispmdmenu->items.FastEl(shape)
      ->rep->state()->set(ivTelltaleState::is_chosen,false);
  }
}

void NetEditor::SetDispMd(taivMenuEl* sel) {
  shape = owner->shape = (NetView::UnitShape) sel->itm_no;
  sel->rep->state()->set(ivTelltaleState::is_chosen,false);
}

void NetEditor::MenuSetDispMd(taivMenuEl* sel){ // sets unit_text from menu
  SetDispMd(sel);
  owner->InitDisplay();
  tabMisc::NotifyEdits(owner);
}

void NetEditor::UpdateDispMdMenu(NetView::UnitShape utd){
  if(dispmdmenu){
    dispmdmenu->GetImage(owner->shape);
    SetDispMd(dispmdmenu->items.FastEl(utd));
  }
}

//////////////////////////////////////////
// 	NetEditor:: misc callbacks	//
//////////////////////////////////////////

void NetEditor::CBarAdjust(){ // someone hit the cbar;
  owner->scale_min = cbar->min;
  owner->scale_max = cbar->max;
  if(auto_scale) ToggleAutoScale(false);
  else if(owner->ordered_uvg_list.size == 1) { 
    MemberDef* md = (MemberDef *) netg->membs[owner->ordered_uvg_list[0]];
    NetViewScaleRange* nvsr =
      (NetViewScaleRange *) owner->scale_ranges.FindName(md->name);
    if (nvsr != NULL) {
      nvsr->auto_scale = auto_scale;
      nvsr->minmax.min = cbar->min;
      nvsr->minmax.max = cbar->max;
      tabMisc::NotifyEdits(nvsr);
    }
  }

  tabMisc::NotifyEdits(owner);
  viewer->update_from_state();
  viewer->Update_All();
}

void NetEditor::ToggleDisplay(bool update){
  display_toggle = !display_toggle;
  ckbox->state()->set(ivTelltaleState::is_chosen,display_toggle);
  if(update == true) {
    UpdateDisplay();
    owner->display_toggle = display_toggle;
    tabMisc::NotifyEdits(owner);
  }
}

void NetEditor::ToggleAutoScale(bool update){
  auto_scale = !auto_scale;
  as_ckbox->state()->set(ivTelltaleState::is_chosen,auto_scale);
  if(owner->auto_scale != auto_scale) {
    owner->auto_scale = auto_scale;
    if(!auto_scale) {
      owner->scale_min = cbar->min;
      owner->scale_max = cbar->max;
    }
    if(owner->ordered_uvg_list.size == 1) { 
      MemberDef* md = (MemberDef *) netg->membs[owner->ordered_uvg_list[0]];
      NetViewScaleRange* nvsr =
	(NetViewScaleRange *) owner->scale_ranges.FindName(md->name);
      if (nvsr != NULL) {
	nvsr->auto_scale = auto_scale;
	nvsr->minmax.min = cbar->min;
	nvsr->minmax.max = cbar->max;
	if(update == true) tabMisc::NotifyEdits(nvsr);
      }
    }
    if(update == true ) {
      UpdateDisplay();
      tabMisc::NotifyEdits(owner);
    }
  }
}

//////////////////////////////////////////
// 	NetEditor:: tools/buttons	//
//////////////////////////////////////////

void NetEditor::NumberVarButtons(){
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivGlyphIndex c;
  int total_count=0;
  for(c=0;c<var_gp.count(); c++){
    StateSelButton* ssb = (StateSelButton *) var_gp.component(c);
    if (ssb->state()->test(ivTelltaleState::is_chosen)) {
      total_count++;
    }
  }
  for(c=0;c<var_gp.count(); c++){
    ivResource::flush();
    StateSelButton* ssb = (StateSelButton *) var_gp.component(c);
    ivPatch* pat = (ivPatch*) ssb->body();
    if (ssb->state()->test(ivTelltaleState::is_chosen) &&
	(total_count > 1)) { // if just one then no number
      int display_number = owner->ordered_uvg_list.Find(ssb->md_idx);
      String newname = ssb->varname + '~' + String(display_number);
      pat->body	(wkit->palette_button_look
		 (wkit->label(newname.chars()),ssb->state()));
      pat->reallocate();
      pat->redraw();
    }
    else  if(ssb->varname.contains('~')){ // remove ~X\'s on non selected ones
      ssb->varname = ssb->varname.before('~');
      pat->body(wkit->palette_button_look
		(wkit->label(ssb->varname.chars()),ssb->state()));
      pat->reallocate();
      pat->redraw();
    }
  }
}

void NetEditor::SelectActButton(int toolnumber) {
  if(viewer == NULL)
    return;
  if(actbutton[toolnumber] != NULL) {
    NoUnSelectButton* but = (NoUnSelectButton*) actbutton[toolnumber];
    but->chooseme();
    but->action()->execute();
  }
}

void NetEditor::SetSelect(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::select);
}
void NetEditor::SetPick(){
  // Set pick cursor
  if(win) win->cursor(picker_cursor);
  if(viewer)viewer->cur_tool(Tool::pick);
}
  
void NetEditor::SetMove(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::move);
}
  
void NetEditor::SetReScale(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::scale);
}

void NetEditor::SetReShape(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::stretch);
}

void NetEditor::SetRotate(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::rotate);
}

void NetEditor::SetAlter(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::alter);
}

void NetEditor::RePick(){
  netg->RePick();
}

void NetEditor::FixEditorButtons(){
  if(buttonpatch == NULL) return;
  connect_action = NEW_LABEL;
  new_action = NEW_LAYERS;
  remove_action = NO_REMOVE;
  edit_action = NO_EDIT;

  int layercount =0;  int unitcount=0;  int prjncount=0;  int vlcount=0;
  int lncount =0; // layer_name
  int unitgpcount = 0;
  int grphcount = 0;
  TAPtr o;  int i;
  for(i=0; i<netg->selectgroup.size; i++) {
    o = netg->selectgroup.FastEl(i);
    if(o->InheritsFrom(&TA_Unit)) { unitcount++; continue; }
    if(o->InheritsFrom(&TA_Layer)) { layercount++; continue; }
    if(o->InheritsFrom(&TA_Projection)) { prjncount++;continue; }
    if(o->InheritsFrom(&TA_LayerNameEdit)) { lncount++;continue; }
    if(o->InheritsFrom(&TA_ViewLabel)) { vlcount++;continue; }
    if(o->InheritsFrom(&TA_Unit_Group)) { unitgpcount++;continue; }
    if(o->InheritsFrom(&TA_NetViewGraphic)) { grphcount++;continue; }
  }

  if((layercount == 0) && (prjncount == 0) && (unitcount == 0) &&
     (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    if(net->layers.leaves == 0) new_action = HOT_NEW_LAYERS;
  }
  else if((layercount > 1) && (prjncount == 0) && (unitcount == 0) &&
	  (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NEW_BIPRJNS;  connect_action = HOT_NEW_PRJNS;
    remove_action = REMOVE_LAYERS; edit_action = EDIT_LAYERS;
  }
  else if((layercount == 1) && (prjncount == 0) && (unitcount == 0) &&
	  (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NEW_LAYERS;  connect_action = NEW_SELF_PRJN;
    remove_action = REMOVE_LAYERS; edit_action = EDIT_LAYERS;
    Layer* nlayer = (Layer *) netg->selectgroup.FastEl(0);
    if((nlayer->geom.z == 1) && (nlayer->n_units != nlayer->units.size))
      new_action = HOT_NEW_UNITS;
  }
  else if((layercount == 0) && (prjncount == 0) && (unitcount == 0) &&
     (vlcount == 0) && (lncount == 0) && (unitgpcount >= 1) && (grphcount == 0)) {
    new_action = NO_NEW;  connect_action = NO_CONNECT;
    remove_action = REMOVE_UNITGPS; edit_action = EDIT_UNITGPS;
  }
  else if((layercount == 0) && (prjncount >= 1) && (unitcount == 0) &&
	   (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NO_NEW;  connect_action = FILL_PRJNS;
    remove_action = REMOVE_PRJNS; edit_action = EDIT_PRJNS;
    for(i=0;i<netg->selectgroup.size;i++){
      Projection* prjn  = (Projection *) netg->selectgroup.FastEl(i);
      if(prjn->projected == false) connect_action = HOT_FILL_PRJNS;
    }
  }
  else if ((layercount == 0) && (prjncount == 0) && (unitcount > 1) &&
	   (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    remove_action = REMOVE_UNITS; edit_action = EDIT_UNITS;
    if(unitcount == 2) { // if 2 units selected then allow editing of conection
      Unit* recv_unit = (Unit *) netg->selectgroup.FastEl(0);
      Unit* send_unit = (Unit *) netg->selectgroup.FastEl(1);
      Connection *c1,*c2;
      c1 = c2 = NULL;
      Con_Group* cg;
      int g;
      FOR_ITR_GP(Con_Group, cg, recv_unit->recv., g) {
	c1 = cg->FindConFrom(send_unit);
	if (c1 != NULL) break;
      }
      if (c1 != NULL) { connect_action = EDIT_CON; new_action = RMV_CON;}
      else   {connect_action = CONNECT_UNITS; new_action = BICON_UNITS;}
      g = 0;
      FOR_ITR_GP(Con_Group, cg, send_unit->recv., g) {
	c2 = cg->FindConFrom(recv_unit);
	if (c2 != NULL) break;
      }
      if (c2 != NULL) { connect_action = EDIT_CON; new_action = RMV_CON;}
      else if(c1 == NULL){
	connect_action = CONNECT_UNITS; new_action = BICON_UNITS;
      }
    }
    else {connect_action = CONNECT_UNITS;   new_action = BICON_UNITS;}
  }
  else if ((layercount == 0) && (prjncount == 0) && (unitcount == 1) &&
	   (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NO_NEW;  connect_action = SELFCON_UNIT;
    remove_action = REMOVE_UNITS; edit_action = EDIT_UNITS;
  }
  else if ((layercount == 0) && (prjncount == 0) && (unitcount == 0) &&
	   (vlcount >= 1) && (lncount == 0) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NO_NEW;  connect_action = NO_CONNECT;
    remove_action = REMOVE_LABELS; edit_action = EDIT_LABELS;
  }
  else if ((layercount == 0) && (prjncount == 0) && (unitcount == 0) &&
	   (vlcount == 0) && (lncount == 0) && (unitgpcount == 0) && (grphcount >= 1)) {
    new_action = NO_NEW;  connect_action = NO_CONNECT;
    remove_action = REMOVE_GRAPHICS; edit_action = NO_EDIT;
  }
  else if((layercount == 0) && (prjncount == 0) && (unitcount == 0) &&
     (vlcount == 0) && (lncount >= 1) && (unitgpcount == 0) && (grphcount == 0)) {
    new_action = NO_NEW;  connect_action = NO_CONNECT;
    remove_action = NO_REMOVE; edit_action = EDIT_NAMES;
  }
  else {
    new_action = NO_NEW;  connect_action = NO_CONNECT;
    remove_action = REMOVE_ALL; edit_action = EDIT_ALL;
  }

  new_but->flip_to(new_action);
  connect_but->flip_to(connect_action);
  remove_but->flip_to(remove_action);
  edit_but->flip_to(edit_action);

  // turn on BUILD_ALL if any layer's n_units don't match up with actual number of units
  // turn on CONNECT_ALL if there are some projections which haven't been built
  bool nbuild_hilit = net->CheckBuild();
  bool nconnect_hilit = net->CheckConnect();
  build_but->flip_to(nbuild_hilit);
  con_but->flip_to(nconnect_hilit);

  buttonpatch->redraw();
  if((win->is_mapped()) && (win->bound())) win->repair();
  // update project if selection changes actions avail in project view
  Project* prj = (Project*)owner->GetOwner(&TA_Project);
  if(prj != NULL)
    prj->InitDisplay();
}

void NetEditor::BuildAll() {
  // script code automatically recorded by button!
  DMEM_GUI_RUN_IF {
    net->Build();
  }
}

void NetEditor::ConnectAll(){
  // script code automatically recorded by button!
  DMEM_GUI_RUN_IF {
    String prv_view;
    if(owner->ordered_uvg_list.size == 1) {
      prv_view = netg->membs.FastEl(owner->ordered_uvg_list.FastEl(0))->name;
      owner->ordered_uvg_list.Reset(); // get rid of it!
    }
    net->Connect();
    if(!prv_view.empty()) {	// need to compensate for things moving around w/ new vars
      int actidx;
      netg->membs.FindName(prv_view, actidx);
      if(actidx >= 0) {
	owner->ordered_uvg_list.Add(actidx); // add it back
	StateSelButton* ssb = (StateSelButton *)  var_gp.component(actidx);
	ssb->state()->set(ivTelltaleState::is_chosen |  ivTelltaleState::is_active, true);
	ssb->FixDisplay();
      }
    }
  }
}

void NetEditor::EditSelections() {
  taBase_List& selectgroup = netg->selectgroup;
  if(selectgroup.size > 1){
    selectgroup.Edit();
  }
  else if (selectgroup.size == 1) {
    selectgroup[0]->Edit();
  }
}   

void NetEditor::RemoveSelections() {
  int okdel = taMisc::Choice("Are you sure you want to remove these objects?","Yes - Delete","No - Cancel");
  if(okdel == 1) return;
  RemoveMonitors();		// get rid of monitors before deleting anything
  netg->safe_damage_me(viewer->canvas());
  taBase_List& selectgroup = netg->selectgroup;
  int i;
  for(i=selectgroup.size-1;i>=0;i--){
    TAPtr o = selectgroup[i];
    selectgroup.Remove(i);
    if(o == NULL) continue;
    taList_impl* og = (taList_impl*)(o)->GetOwner(&TA_taList_impl);
    if(o->InheritsFrom(&TA_Unit_Group)) {
      Unit_Group* ug = (Unit_Group*)o;
      if(ug->size > 0) {
	taivMisc::RecordScript(o->GetPath() + ".RemoveAll();\n");
	DMEM_GUI_RUN_IF {
	  ug->RemoveAll();
	}
	continue;		// first just remove the units
      }
    }
    else if(o->InheritsFrom(&TA_Unit)) {
      DMEM_GUI_RUN_IF {
	((Unit *) o)->DisConnectAll();	// disconnect unit before removing it..
      }
      taivMisc::RecordScript(o->GetPath() + ".DisConnectAll();\n");
    }
    else if(o->InheritsFrom(&TA_Layer)) {
      winbMisc::DelayedMenuUpdate(og);
    }
    if(og != NULL) {
      String recordstring = og->GetPath() + ".RemoveEl(" + o->GetPath() + ");\n";
      DMEM_GUI_RUN_IF {
	og->Remove(o);
      }
      taivMisc::RecordScript(recordstring);
    }
  }
  netg->safe_damage_me(viewer->canvas());
  InitDisplay();
  UpdateMonitors();		// then update them afterwards
}

void NetEditor::NewObjects() {
  taBase_List& selectgroup = netg->selectgroup;
  Layer_MGroup lg;
  Unit_Group ug;
  TAPtr o;
  int i;
  for(i=0; i<selectgroup.size; i++) {
    o = selectgroup.FastEl(i);
    if(o->InheritsFrom(&TA_Layer)) lg.Link((Layer *) o);
    if(o->InheritsFrom(&TA_Unit)) ug.Link((Unit *) o);
  }
  int oldsize;
  int unitcount = 0;
  int j;
  if(ug.size > 1) { // bi-con units
    Unit* u_to = (Unit *) ug.FastEl(0);
    for(j=1;j<ug.size;j++) { // recv based units 1-n connect to unit 0
      Unit* u_from = (Unit *) ug[j];
      net->ConnectUnits(u_to,u_from);
      net->ConnectUnits(u_from,u_to);
    }
    return;
  }
  else if(lg.size == 0) {
    oldsize = net->layers.leaves;
    gpivGroupNew::New(&(net->layers));
    if(net->layers.leaves > oldsize) {
      int cnt = 0;
      taLeafItr j;
      Layer* lay;
      FOR_ITR_EL(Layer, lay, net->layers., j) {
	if(cnt<oldsize) // old layer
	  unitcount += MAX(lay->n_units,lay->units.size);
	else  // new layer
	  lay->UpdateAfterEdit();
	cnt++;
      }
      winbMisc::DelayedMenuUpdate(&(net->layers));
      SelectActButton(Tool::stretch);
      InitDisplay(); // recenter if previous units
    }
    return;
  }
  else if(lg.size > 1){ // bi directional projections
    Layer* lay = (Layer *) lg.FastEl(0); // receiver
    for(j=1;j<lg.size;j++) { // recv based. 1-n send to 0
      Layer* from_l = (Layer*) lg.FastEl(j);
      DMEM_GUI_RUN_IF {
	Projection* pjn = (Projection*)lay->projections.New(1); // new projection
	pjn->SetCustomFrom(from_l);
	selectgroup.LinkUnique(pjn);
      }
      // record to script
      taivMisc::RecordScript(lay->projections.GetPath() + ".New(1)->SetCustomFrom("
			     + from_l->GetPath() + ");\n");

      // and the reverse
      DMEM_GUI_RUN_IF {
	Projection* pjnr = (Projection*)from_l->projections.New(1); // new projection
	pjnr->SetCustomFrom(lay);
	selectgroup.LinkUnique(pjnr);
      }
      taivMisc::RecordScript(from_l->projections.GetPath() + ".New(1)->SetCustomFrom("
			     + lay->GetPath() + ");\n");
    }
    netg->UpdateSelect();
  }
  else { // new units
    Layer* lay = (Layer *)lg[0];
    oldsize = lay->units.leaves;
    gpivGroupNew::New(&(lay->units),NULL,NULL,NULL,
		       lay->n_units - lay->units.size);
    if(lay->units.leaves > oldsize){
      lay->n_units = lay->units.leaves;
      lay->UpdateAfterEdit();
    }
    InitDisplay();
  }
}

void NetEditor::ConnectObjects() {
  taBase_List& selectgroup = netg->selectgroup;
  Unit_Group ug;
  Layer_MGroup lg;
  Projection_Group pg;
  TAPtr o;
  int i;
  for(i=0; i<selectgroup.size; i++) {
    o = selectgroup.FastEl(i);
    if(o->InheritsFrom(&TA_Unit)) { ug.Link((Unit *) o); continue; }
    if(o->InheritsFrom(&TA_Layer)) { lg.Link((Layer *) o); continue; }
    if(o->InheritsFrom(&TA_Projection)) { pg.Link((Projection*)o);continue; }
  }
  if((lg.size == 0) && (ug.size == 0) && (pg.size == 0)) {
    String namestring = String("NetView Label:") + String(owner->labels.size);
    DMEM_GUI_RUN_IF {
      owner->NewLabel(namestring);
    }
    taivMisc::RecordScript(owner->GetPath() +
			   ".NewLabel(\""+namestring+"\");\n");
    return;
  }
  if(((lg.size > 0) || (ug.size > 0)) && (pg.size > 0)){
    taMisc::Error
      ("Please select (units & layers) or projections, but not both");
    return;
  }
  Layer* lay;
  Projection* pjn;
  int j,p;
  if((lg.size == 1) && (ug.size == 0)) { // one layer: Self Projection
    selectgroup.RemoveAll();
    lay = (Layer*)lg[0];
    DMEM_GUI_RUN_IF {
      Projection* pjn = (Projection*)lay->projections.New(1);
      pjn->from_type = Projection::SELF;
      pjn->UpdateAfterEdit();
      netg->FixProjection(pjn, -1); // make sure it gets fixed, even if display is off
      selectgroup.LinkUnique(pjn);
    }
    taivMisc::RecordScript(lay->projections.GetPath() + ".New(1)->SetCustomFrom("
			   + lay->GetPath() + ");\n");
    netg->UpdateSelect();
    return;
  }
  if(lg.size > 1) {
    selectgroup.RemoveAll();
    lay = (Layer *) lg.FastEl(0); // receiver
    for(j=1;j<lg.size;j++) { // recv based. 1-n send to 0
      DMEM_GUI_RUN_IF {
	pjn = (Projection*)lay->projections.New(1); // new projection
	pjn->SetCustomFrom((Layer*)lg[j]);
	netg->FixProjection(pjn, -1); // make sure it gets fixed, even if display is off
	selectgroup.LinkUnique(pjn);
      }
      taivMisc::RecordScript(lay->projections.GetPath() + ".New(1)->SetCustomFrom("
			     + lg[j]->GetPath() + ");\n");
    }
    netg->UpdateSelect();
    return;
  }
  Unit* u_from;
  Unit* u_to;
  if(ug.size == 1){ // self con
    u_to = u_from = ug[0];
    net->ConnectUnits(u_to);
    return;
  }
  else if(ug.size > 1) {
    u_to = (Unit *) ug.FastEl(0);
    for(j=1;j<ug.size;j++) { // recv based units 1-n connect to unit 0
      u_from = (Unit *) ug[j];
      net->ConnectUnits(u_to,u_from);
    }
    return;
  }
  if(pg.size >= 1) {
    for(p=0;p<pg.size;p++){
      pjn = (Projection *) pg[p];
      DMEM_GUI_RUN_IF {
	pjn->Connect();
	pjn->UpdateAfterEdit();
      }
      taivMisc::RecordScript(pjn->GetPath() + ".Connect();\n");
    }
  }
}

void NetEditor::EditConnection(){
  Unit* recv_unit = (Unit *) netg->selectgroup.FastEl(0);
  Unit* send_unit = (Unit *) netg->selectgroup.FastEl(1);
  String choice_string = String("Edit connection from unit: ");
  if(send_unit->name.empty() == true)
    choice_string +=  send_unit->GetPath_Long(NULL,GET_OWNER(send_unit,Layer));
  else
    choice_string += send_unit->name + " in layer " +
      (GET_OWNER(send_unit,Layer))->name;
  choice_string += " to unit: ";
  if(recv_unit->name.empty() == true)
    choice_string +=  recv_unit->GetPath_Long(NULL,GET_OWNER(recv_unit,Layer));
  else
    choice_string += recv_unit->name + " in layer " +
      (GET_OWNER(recv_unit,Layer))->name;
    
  if(taMisc::Choice(choice_string,"Yes","Other_Direction") == 1) {
    send_unit = recv_unit;
    recv_unit = (Unit *) netg->selectgroup.FastEl(1);
  }
  Connection* cn = NULL;
  Con_Group* cg;
  int g;
  FOR_ITR_GP(Con_Group, cg, recv_unit->recv., g) {
    cn = cg->FindConFrom(send_unit);
    if (cn != NULL) break;
  }
  if (cn != NULL) cn->Edit();
  else {
    taMisc::Error("Those units are not connected in that direction!!");
  }
}

void NetEditor::RemoveConnection(){
  Unit* recv_unit = (Unit *) netg->selectgroup.FastEl(0);
  Unit* send_unit = (Unit *) netg->selectgroup.FastEl(1);
  String choice_string = String("Remove connection from unit: ");
  if(send_unit->name.empty() == true)
    choice_string +=  send_unit->GetPath_Long(NULL,GET_OWNER(send_unit,Layer));
  else
    choice_string += send_unit->name + " in layer " +
      (GET_OWNER(send_unit,Layer))->name;
  choice_string += " to unit: ";
  if(recv_unit->name.empty() == true)
    choice_string +=  recv_unit->GetPath_Long(NULL,GET_OWNER(recv_unit,Layer));
  else
    choice_string += recv_unit->name + " in layer " +
      (GET_OWNER(recv_unit,Layer))->name;
   
  bool both_directions = false;
  switch(taMisc::Choice(choice_string,"Yes","Other_Direction","Cancel")) {
  case 0: break;
  case 1:
    send_unit = recv_unit;
    recv_unit = (Unit *) netg->selectgroup.FastEl(1);
    break;
  case 2:
    both_directions = true;
  case 3: return;
  }
  Connection* cn = NULL;
  Con_Group* cg;
  int g;
  FOR_ITR_GP(Con_Group, cg, recv_unit->recv., g) {
    cn = cg->FindConFrom(send_unit);
    if (cn != NULL) break;
  }
  if (cn != NULL) {
    recv_unit->DisConnectFrom(send_unit);
    if(both_directions == true) send_unit->DisConnectFrom(recv_unit);
  }
  else {
    taMisc::Error("Those units are not connected in that direction!!");
  }
  RePick();
  UpdateDisplay();
  viewer->Update_All();
}

//////////////////////////////////////////
// 	NetEditor:: moving/xforms	//
//////////////////////////////////////////

void NetEditor::SetLayerLabelXform(Layer* l, ivTransformer *t) {
  float a00,a01,a10,a11,a20,a21;
  t->matrix(a00,a01,a10,a11,a20,a21);
  owner->SetLayerLabelXform(l,a00,a01,a10,a11,a20,a21);
}

void NetEditor::SetLayerXform(Layer* l, ivTransformer *t) {
  float a00,a01,a10,a11,a20,a21;
  t->matrix(a00,a01,a10,a11,a20,a21);
  owner->SetLayerXform(l,a00,a01,a10,a11,a20,a21);
}

void NetEditor::UpdatePosivPatch(float val) {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  if(pospatch == NULL) {
     pospatch = new ivPatch(NULL);
     ivResource::ref(pospatch);
  }
  String valstr(val,"%5.3g");
  pospatch->body(layout->hbox(wkit->fancy_label("Value: "),wkit->raised_label(valstr)));
  pospatch->reallocate();
  pospatch->redraw();
  win->repair();
}


void NetEditor::UpdatePosivPatch(int x, int y, int z){
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  if(pospatch == NULL) {
     pospatch = new ivPatch(NULL);
     ivResource::ref(pospatch);
  }

  String xstr(x,"%3d ");
  String ystr(y,"%3d ");
  String zstr(z,"%3d ");

  ivGlyph* xrep = layout->hbox
    (wkit->fancy_label("X: "),wkit->raised_label(xstr));
  ivGlyph* yrep = layout->hbox
    (wkit->fancy_label("Y: "),wkit->raised_label(ystr));
  ivGlyph* zrep = layout->hbox
    (wkit->fancy_label("Z: "),wkit->raised_label(zstr));

  pospatch->body
    (layout->hbox(xrep,yrep,zrep));
  pospatch->reallocate();
  pospatch->redraw();
  win->repair();

}

void NetEditor::ClearPosivPatch() {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  if(pospatch == NULL) {
     pospatch = new ivPatch(NULL);
     ivResource::ref(pospatch);
  }
  pospatch->body(wkit->fancy_label("X: --- Y: --- Z: ---"));
  pospatch->reallocate();
  pospatch->redraw();
  win->repair();
}

//////////////////////////////////////////
// 	NetEditor:: MStat Fctns		//
//////////////////////////////////////////

static void neteditor_add_mstats(taivHierMenu* mstatmenu, taBase* scope_ref, taivMenuAction* actn) {
  TypeDef* td = &TA_MonitorStat;
  int i;
  for(i=0; i<td->tokens.size; i++) {
    MonitorStat* ms = (MonitorStat*)td->tokens.FastEl(i);
    if(!ms->SameScope(scope_ref))
      continue;
    if(ms->time_agg.from != NULL) continue; // not a real stat!
    mstatmenu->AddItem((char*)ms->name, (void*)ms, taivMenu::use_default, actn);
  }
}

void NetEditor::BuildMStatMenu(){
  if(statbutton->count() > 0) {	// replace with temp so old mstatmenu can be deleted
    ivMenuItem* temp_mi = new ivMenuItem(NULL,new ivTelltaleState());
    statbutton->replace_item(0, temp_mi);
    if (mstatmenu != NULL) delete mstatmenu; mstatmenu = NULL;
  }
  ivResource::flush();
#ifdef CYGWIN
  // small somehow is redefined in IV-Win/event.h as a (char) or something, so it can't
  // be used here..
  mstatmenu = new taivHierMenu(taivMenu::menuitem,taivMenu::normal,2);
#else
  mstatmenu = new taivHierMenu(taivMenu::menuitem,taivMenu::normal,taivMenu::small);
#endif

  mstatmenu->SetMLabel("Monitor Values");

  mstatmenu->AddItem("New",NULL,taivMenu::use_default,
		     new ActionCallback(NetEditor)
		     (this,&NetEditor::NewMStat));

  int cur_no = mstatmenu->cur_sno;

  mstatmenu->AddSubMenu("Set Objects");
  neteditor_add_mstats(mstatmenu, owner, new taivMenuCallback(NetEditor)(this,&NetEditor::SetMStatObjects));
  mstatmenu->SetSub(cur_no);

  mstatmenu->AddSubMenu("Set Variable");
  neteditor_add_mstats(mstatmenu, owner, new taivMenuCallback(NetEditor)(this,&NetEditor::SetMStatMember));
  mstatmenu->SetSub(cur_no);

  mstatmenu->AddSubMenu("Edit");
  neteditor_add_mstats(mstatmenu, owner, new taivMenuCallback(NetEditor)(this,&NetEditor::EditMStat));
  mstatmenu->SetSub(cur_no);

  mstatmenu->AddSubMenu("Remove");
  neteditor_add_mstats(mstatmenu, owner, new taivMenuCallback(NetEditor)(this,&NetEditor::RemoveMStat));
  mstatmenu->SetSub(cur_no);

  if(statbutton->count() > 0)
    statbutton->replace_item(0,mstatmenu->GetMenuItem());
  else
    statbutton->append_item(mstatmenu->GetMenuItem());
}

void NetEditor::UpdateMStatMenu(){
  if(statbutton != NULL)
    BuildMStatMenu();
}

void NetEditor::NewMStat(){
  if(netg->selectgroup.size == 0) {
    taMisc::Error("No objects select objects to monitor");
    return;
  }
  owner->CallFun("NewMonitor");
  winbMisc::DelayedMenuUpdate(owner);
  Project* prj = (Project*)owner->GetOwner(&TA_Project);
  if(prj != NULL)
    winbMisc::DelayedMenuUpdate((taBase*)prj); // for process stat menu
}

void NetEditor::SetMStatObjects(taivMenuEl* sel) {
  if(netg->selectgroup.size == 0) {
    taMisc::Error("No objects selected to monitor!");
    return;
  }
  MonitorStat* stat = (MonitorStat *) sel->usr_data;
  stat->objects.Reset();
  int i;
  for(i=0;i<netg->selectgroup.size;i++){
    stat->objects.Link(netg->selectgroup.FastEl(i));
  }
  stat->UpdateAfterEdit();
  winbMisc::DelayedMenuUpdate(owner);
  Project* prj = (Project*)owner->GetOwner(&TA_Project);
  if(prj != NULL)
    winbMisc::DelayedMenuUpdate((taBase*)prj); // for process stat menu
}

void NetEditor::SetMStatMember(taivMenuEl* sel) {
  String variable = "";
  if(netg->cur_mbr >= 0) {
    variable = netg->membs[netg->cur_mbr]->name;
  }
  if(variable == "")  {
    taMisc::Error("Must select a valid variable to monitor!");
    return;
  }
  MonitorStat* stat = (MonitorStat *) sel->usr_data;
  stat->variable = variable;
  stat->UpdateAfterEdit();
  winbMisc::DelayedMenuUpdate(owner);
  Project* prj = (Project*)owner->GetOwner(&TA_Project);
  if(prj != NULL)
    winbMisc::DelayedMenuUpdate((taBase*)prj); // for process stat menu
}

void NetEditor::EditMStat(taivMenuEl* sel) {
  MonitorStat* stat = (MonitorStat *) sel->usr_data;
  TA_MonitorStat.ive->Edit(stat,win);
}

void NetEditor::RemoveMStat(taivMenuEl* sel) {
  if(taMisc::iv_active)
    if(taMisc::Choice("Ok to Remove MonitorStat?", "Ok", "Cancel") == 1) return;
  taMisc::DoneBusy();
  MonitorStat* stat = (MonitorStat *) sel->usr_data;
  ((Stat_Group *) (stat->owner))->Remove(stat);
  winbMisc::DelayedMenuUpdate(owner);
  Project* prj = (Project*)owner->GetOwner(&TA_Project);
  if(prj != NULL)
    winbMisc::DelayedMenuUpdate((taBase*)prj); // for process stat menu
}

void NetEditor::RemoveMonitors() {
  if((owner != NULL) && (owner->mgr != NULL))
    ((Network*)owner->mgr)->RemoveMonitors();
}

void NetEditor::UpdateMonitors() {
  if((owner != NULL) && (owner->mgr != NULL))
    ((Network*)owner->mgr)->UpdateMonitors();
}


