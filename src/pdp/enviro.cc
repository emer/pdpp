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

// enviro.cc


#include <pdp/enviro.h>
#include <pdp/enviro_iv.h>
#include <pdp/netstru.h>
#include <pdp/sched_proc.h> // for trialproc check in enview::update_display
#include <ta/ta_group_iv.h>
#include <ta_misc/colorbar.h>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include <ta/enter_iv.h>
#include <InterViews/window.h>
#include <ta/leave_iv.h>

//////////////////////
//   PatternSpec    //
//////////////////////

void PatternSpec::Initialize() {
  type = INACTIVE;
  to_layer = LAY_NAME;
  layer_num = 0;
  layer = NULL;
  pattern_type = &TA_Pattern;
  layer_flags = DEFAULT;
  use_flags = USE_NO_FLAGS;
  n_vals = 0;
  geom.y = 1;
  initial_val = 0.0f;
  noise.type = Random::NONE;
  noise.mean = 0.0f;
  noise.var = 0.5f;
}

void PatternSpec::InitLinks(){
  BaseSubSpec::InitLinks();
  taBase::Own(pos, this);
  taBase::Own(geom, this);  
  taBase::Own(noise,this);
  taBase::Own(value_names, this);
  taBase::Own(global_flags, this);
  EventSpec* evs = GET_MY_OWNER(EventSpec);
  if(evs == NULL)
    return;
  int index = evs->patterns.FindLeaf(this);
  if(index == 0){
    to_layer = FIRST;
    type = INPUT;
    pos.y = 0;
    UpdateAfterEdit();
  }
  else if((index == (evs->patterns.size -1)) && (index == 1)) {	// only first one!
    to_layer= LAST;
    type = TARGET;
    int ypos = evs->MaxY();
    pos.y = ypos + 1;		// by default start higher..
    UpdateAfterEdit();
  }
  else {
    int ypos = evs->MaxY();
    pos.y = ypos + 1;		// by default start higher..
  }
}

void PatternSpec::Destroy() {
  CutLinks();
}

void PatternSpec::CutLinks() {
  taBase::DelPointer((TAPtr*)&layer);
  BaseSubSpec::CutLinks();
}
  
void PatternSpec::Copy_(const PatternSpec& cp) {
  type=cp.type;
  to_layer=cp.to_layer; 
  layer_name=cp.layer_name;
  layer_num = cp.layer_num;
  taBase::SetPointer((TAPtr*)&layer,cp.layer);
  pattern_type = cp.pattern_type;
  layer_flags = cp.layer_flags;
  use_flags = cp.use_flags;
  n_vals = cp.n_vals;
  geom = cp.geom;
  pos = cp.pos;
  initial_val = cp.initial_val;
  noise = cp.noise;
  value_names = cp.value_names;
  global_flags = cp.global_flags;
}

void PatternSpec::SetToLayName(const char* lay_nm) {
  to_layer = LAY_NAME;
  name = lay_nm;
  layer_name = lay_nm;
}

Network* PatternSpec::GetDefaultNetwork() {
  return pdpMisc::GetDefNetwork(GET_MY_OWNER(Project));
}

void PatternSpec::SetToLayer(Layer* lay) {
  if(lay == NULL) {
    Network* net = GetDefaultNetwork();
    if((net == NULL) || !SetLayer(net))
      return;
    lay = layer;
    UnSetLayer();
  }
  to_layer = LAY_NAME;
  layer_name = lay->name;
  name = layer_name;
  lay->GetActGeomNoSpc(geom);
  geom.z = 1;
  n_vals = MAX(lay->units.leaves,lay->n_units);
  pos.z=0;
  pos.x = lay->pos.x;
  pos.y = lay->pos.y;
  if(lay->pos.z > 0) {
    Network* net = lay->own_net;
    if(net != NULL) {
      int index = net->layers.FindLeaf(lay);
      pos.y = 0;
      int n;
      for(n=0;n<index;n++) {
	Layer* nlay = (Layer*)net->layers.Leaf(n);
	int y_val = nlay->pos.y + nlay->act_geom.y + 1;
	pos.y =  MAX(pos.y, y_val);
      }
    }
  }
  UpdateAfterEdit();
  UpdateAllEvents();
}

void PatternSpec::UpdateAfterEdit() {
  BaseSubSpec::UpdateAfterEdit();
  Network* net = GetDefaultNetwork();
  if((net != NULL) && SetLayer(net)) {
    if((geom.x * geom.y == 0) || (n_vals == 0))  {
      layer->GetActGeomNoSpc(geom);
      geom.z = 1;
      n_vals = MAX(layer->units.leaves,layer->n_units);
      if(net->lay_layout == Network::TWO_D){
	pos = layer->pos;
      }
      else{
	pos.z=0;
	pos.x = layer->pos.x;
	pos.y = layer->pos.y;
	if(layer->pos.z > 0) {
	  int index = net->layers.FindLeaf(layer);
	  pos.y = 0;
	  int n;
	  for(n=0;n<index;n++) {
	    Layer* lay = (Layer*)net->layers.Leaf(n);
	    int y_val = lay->pos.y + lay->act_geom.y + 1;
	    pos.y =  MAX(pos.y, y_val);
	  }
	}
      }
    }
    UnSetLayer();
  }

  if(n_vals == 0)
    n_vals = geom.x * geom.y;
  geom.FitNinXY(n_vals);

  if((name.empty() || name.contains(GetTypeDef()->name)) && !layer_name.empty())
    name = layer_name;

  value_names.EnforceSize(n_vals);
  global_flags.EnforceSize(n_vals);

  if(!taMisc::is_loading) {
    EventSpec* es = GET_MY_OWNER(EventSpec);
    if(es != NULL) {
      es->UnSetLayers();
      es->UpdateChildren();
    }
  }
}

void PatternSpec::UpdateAllEvents() {
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if(es == NULL)
    return;
  es->UpdateAllEvents();
}

bool PatternSpec::SetLayer(Network* net) {
  switch(to_layer) {
  case FIRST:
    taBase::SetPointer((TAPtr*)&(layer), net->layers.Leaf(0));
    if(layer ==  NULL) {
      taMisc::Error("*** Cannot apply pattern:", name,", no layers in network:", net->name);
      return false;
    }
    layer_num = 0;
    layer_name = layer->name;
    break;  
  case LAST:
    if (net->layers.leaves > 0 ) {
      taBase::SetPointer((TAPtr*)&(layer), net->layers.Leaf(net->layers.leaves-1));
      layer_num = net->layers.leaves-1;
      layer_name = layer->name;
    }
    else {
      taMisc::Error("*** Cannot apply pattern:", name,",no layers in network:", net->name);
      return false;
    }
    break;
  case LAY_NAME:
    taBase::SetPointer((TAPtr*)&(layer), net->layers.FindLeafName(layer_name,layer_num));
    if (layer == NULL) {
      taMisc::Error("*** Cannot apply pattern:", name,
		     "no layer with name:", layer_name, "in network:", net->name);
      return false;
    }
    break;
  case LAY_NUM:
    if (layer_num >= net->layers.leaves) {
      taMisc::Error("*** Cannot apply pattern:", name,
		     "to layer number:", String(layer_num),
		     ", only:", String((int)net->layers.leaves),
		     "layers in network:", net->name);
      return false;
    }
    taBase::SetPointer((TAPtr*)&(layer), net->layers.Leaf(layer_num));
    layer_name = layer->name;
    break;
  }
  return true;
}

void PatternSpec::UnSetLayer() {
  taBase::DelPointer((TAPtr*)&layer);
}  

void PatternSpec::FlagLayer() {
  if((layer == NULL) || (layer_flags == NO_LAYER_FLAGS))
    return;

  if(layer_flags == DEFAULT) {
    switch(type) {
    case INACTIVE:
      break;
    case INPUT:
      layer->SetExtFlag(Unit::EXT);
      break;
    case TARGET:
      layer->SetExtFlag(Unit::TARG);
      break;
    case COMPARE:
      layer->SetExtFlag(Unit::COMP);
      break;
    }
    return;
  }
  layer->SetExtFlag(layer_flags); // the bits are the same..
}

float& PatternSpec::Value(Pattern* pat, int index) {
  return pat->value.SafeEl(index);
}

int& PatternSpec::Flag(PatUseFlags flag_type, Pattern* pat, int index) {
  static int noflags = NO_FLAGS;
  switch(flag_type) {
  case USE_NO_FLAGS:
    return noflags;
  case USE_PATTERN_FLAGS:
    return pat->flag.SafeEl(index);
  case USE_GLOBAL_FLAGS:
    return global_flags.SafeEl(index);
  case USE_PAT_THEN_GLOBAL_FLAGS: {
    int& flags = pat->flag.SafeEl(index);
    if(flags == NO_FLAGS)
      return global_flags.SafeEl(index);
    return flags;
  }
  }
  return noflags;
}

void PatternSpec::ApplyValue(Pattern* pat, Unit* uni, int index) {
  float val = Value(pat, index) + noise.Gen();
  int flags = Flag(use_flags, pat, index);
  ApplyValue_impl(uni, val, flags);
}

void PatternSpec::ApplyValue_impl(Unit* uni, float val, int flags) {
  if(flags & NO_APPLY)
    return;
  if(flags != NO_FLAGS) {
    ApplyValueWithFlags(uni, val, flags);
    return;
  }
  switch(type) {
  case INACTIVE:
    break;
  case INPUT:
    uni->ext = val;
    uni->SetExtFlag(Unit::EXT);
    break;
  case TARGET:
    uni->targ = val;
    uni->SetExtFlag(Unit::TARG);
    break;
  case COMPARE:
    uni->targ = val;
    uni->SetExtFlag(Unit::COMP);
    break;
  }
}

void PatternSpec::ApplyValueWithFlags(Unit* uni, float val, int flags) {
  if(!(flags & NO_UNIT_FLAG)) {
    if(flags & COMP_TARG_EXT_FLAG) { // one of these guys was set
      uni->SetExtFlag((Unit::ExtType)(flags & COMP_TARG_EXT_FLAG));
    }
    else {
      switch(type) {
      case INACTIVE:
	break;
      case INPUT:
	uni->SetExtFlag(Unit::EXT);
	break;
      case TARGET:
	uni->SetExtFlag(Unit::TARG);
	break;
      case COMPARE:
	uni->SetExtFlag(Unit::COMP);
	break;
      }
    }
  }
  if(!(flags & NO_UNIT_VALUE)) {
    if(flags & TARG_EXT_VALUE) {
      if(flags & TARG_VALUE)
	uni->targ = val;
      if(flags & EXT_VALUE)
	uni->ext = val;
    }
    else {
      switch(type) {
      case INACTIVE:
	break;
      case INPUT:
	uni->ext = val;
	break;
      case TARGET:
	uni->targ = val;
	break;
      case COMPARE:
	uni->targ = val;
	break;
      }
    }
  }
}

void PatternSpec::ApplyPattern(Pattern* pat) {
  if(layer == NULL)
    return;
  FlagLayer();
  int v;     	// current value index;
  Unit* u; 	// current unit
  taLeafItr j;
  Unit_Group* ug = &(layer->units);
  int val_sz = pat->value.size;
  for(u = (Unit*)ug->FirstEl(j), v = 0; (u && (v < val_sz));
      u = (Unit*)ug->NextEl(j), v++)
  {
    ApplyValue(pat,u,v);
  }
}

void PatternSpec::ApplyNames() {
  if(layer == NULL)
    return;
  int v;     	// current value index;
  Unit* u; 	// current unit
  taLeafItr j;
  Unit_Group* ug = &(layer->units);
  int val_sz = value_names.size;
  for(u = (Unit*)ug->FirstEl(j), v = 0; (u && (v < val_sz));
      u = (Unit*)ug->NextEl(j), v++)
  {
    u->name = value_names[v];
  }
}

Pattern* PatternSpec::NewPattern(Event*, Pattern_Group* par) {
  Pattern* rval = (Pattern*)par->NewEl(1, pattern_type);
  rval->value.Insert(initial_val, 0, n_vals);
  if((use_flags == USE_PATTERN_FLAGS) || (use_flags == USE_PAT_THEN_GLOBAL_FLAGS))
    rval->flag.Insert(0, 0, n_vals);
  return rval;
}

void PatternSpec::UpdatePattern(Event*, Pattern* pat) {
  if(pat->value.size < n_vals)
    pat->value.Insert(initial_val, pat->value.size, n_vals - pat->value.size);
  if(pat->value.size > n_vals)
    pat->value.size = n_vals;

  if((use_flags == USE_PATTERN_FLAGS) || (use_flags == USE_PAT_THEN_GLOBAL_FLAGS)) {
    if(pat->flag.size < n_vals)
      pat->flag.Insert(0, pat->flag.size, n_vals - pat->flag.size);
    if(pat->flag.size > n_vals)
      pat->flag.size = n_vals;
  }
  else
    pat->flag.size = 0;
}


////////////////////////////
//   PatternSpec_Group    //
////////////////////////////

void PatternSpec_Group::Initialize() {
  pat_gp_type = &TA_Pattern_Group;
  SetBaseType(&TA_PatternSpec);
}

void PatternSpec_Group::Destroy() {
  CutLinks();
}

void PatternSpec_Group::Copy_(const PatternSpec_Group& cp) {
  pat_gp_type = cp.pat_gp_type;
}

void PatternSpec_Group::UpdateAfterEdit() {
  taBase_Group::UpdateAfterEdit();
  if(!taMisc::is_loading) {
    EventSpec* es = GET_MY_OWNER(EventSpec);
    if(es == NULL)
      return;
    es->UnSetLayers();
    if(es->DetectOverlap()) {
      int choice = taMisc::Choice("Overlap of PatternSpecs",
				  "Enforce Linear Layout", "Ignore");
      if(choice == 0) es->LinearLayout();
    }
  }
}

void PatternSpec_Group::LinearLayout() {
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if(es == NULL) return;
  es->LinearLayout();
}

void PatternSpec_Group::CutLinks() {
  if((pat_gp_type == NULL) || (owner == NULL)) {
    taBase_Group::CutLinks();
    return;
  }
  pat_gp_type = NULL;		// don't do it more than once
  
  if(!owner->InheritsFrom(TA_taSubGroup)) { // only for created groups
    taBase_Group::CutLinks();
    return;
  }
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if(es == NULL) {
    taBase_Group::CutLinks();
    return;
  }
  Environment* env = GET_OWNER(es,Environment);
  if(env == NULL) {
    taBase_Group::CutLinks();
    return;
  }
  String my_path = GetPath(NULL, es); // my path is same as corresp pat group
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    if(ev->spec.spec != es)
      continue;			// only for events that use me!
    MemberDef* md;
    Pattern_Group* pg = (Pattern_Group*)ev->FindFromPath(my_path, md);
    if((pg == NULL) || !pg->InheritsFrom(TA_Pattern_Group) ||
       !pg->owner->InheritsFrom(TA_taSubGroup))
      continue;
    taSubGroup* og = (taSubGroup*)pg->owner;
    og->Remove(pg);
  }
  taBase_Group::CutLinks();
}

TAPtr PatternSpec_Group::NewEl(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if((es == NULL) || taMisc::is_loading) // don't do fancy stuff when loading
    return taBase_Group::NewEl(no,typ);
  Environment* env = GET_OWNER(es,Environment);
  if(env == NULL)
    return taBase_Group::NewEl(no,typ);
  String my_path = GetPath(NULL, es); // my path is same as corresp pat group
  int st_idx = size;
  TAPtr rval = taBase_Group::NewEl(no,typ);
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    if(ev->spec.spec != es)
      continue;			// only for events that use me!
    MemberDef* md;
    Pattern_Group* pg = (Pattern_Group*)ev->FindFromPath(my_path, md);
    if((pg == NULL) || !pg->InheritsFrom(&TA_Pattern_Group))
      continue;
    int i;
    for(i=st_idx; i<size; i++)
      ((PatternSpec*)FastEl(i))->NewPattern(ev, pg);
  }
  es->UnSetLayers();
  es->UpdateChildren();
  return rval;
}

taGroup<taBase>* PatternSpec_Group::NewGp(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  if((typ == NULL) || !(typ->InheritsFrom(GetTypeDef())))
    typ = GetTypeDef();
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if((es == NULL) || taMisc::is_loading) // don't do fancy stuff when loading..
    return taBase_Group::NewGp(no,typ);
  Environment* env = GET_OWNER(es,Environment);
  if(env == NULL)
    return taBase_Group::NewGp(no,typ);
  String my_path = GetPath(NULL, es); // my path is same as corresp pat group
  int st_idx = gp.size;
  taGroup<taBase>* rval = taBase_Group::NewGp(no,typ);
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    if(ev->spec.spec != es)
      continue;			// only for events that use me!
    MemberDef* md;
    Pattern_Group* pg = (Pattern_Group*)ev->FindFromPath(my_path, md);
    if((pg == NULL) || !pg->InheritsFrom(&TA_Pattern_Group))
      continue;
    int i;
    for(i=st_idx; i<gp.size; i++)
      ((PatternSpec_Group*)(gp.FastEl(i)))->NewPatternGroup(ev, pg);
  }
  es->UnSetLayers();
  es->UpdateChildren();
  return rval;
}


TAPtr PatternSpec_Group::New(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  if(typ == NULL) typ = el_typ;
  if(typ->InheritsFrom(TA_taGroup_impl))
    return (TAPtr)NewGp(no,typ);
  return NewEl(no,typ);
}

bool PatternSpec_Group::Remove(int i) {
  EventSpec* es = GET_MY_OWNER(EventSpec);
  if(es == NULL) return false;
  Environment* env = GET_OWNER(es,Environment);
  if(env == NULL) return false;
  String my_path = GetPath(NULL, es); // my path is same as corresp pat group
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    if(ev->spec.spec != es)
      continue;			// only for events that use me!
    MemberDef* md;
    Pattern_Group* pg = (Pattern_Group*)ev->FindFromPath(my_path, md);
    if((pg == NULL) || !pg->InheritsFrom(&TA_Pattern_Group))
      continue;
    pg->Remove(i);
  }
  bool rval = taBase_Group::Remove(i);
  es->UnSetLayers();
  es->UpdateChildren();
  return rval;
}
  
Pattern_Group* PatternSpec_Group::NewPatternGroup(Event* ev, Pattern_Group* par) {
  Pattern_Group* rval = (Pattern_Group*)par->NewGp(1, pat_gp_type);

  PatternSpec* ps;
  int i;
  for(i=0; i<size; i++) {
    ps = (PatternSpec*)FastEl(i);
    ps->NewPattern(ev,rval);
  }

  PatternSpec_Group* pg;
  for(i=0; i<gp.size; i++) {
    pg = (PatternSpec_Group*)gp.FastEl(i);
    pg->NewPatternGroup(ev,rval);
  }
  return rval;
}

void PatternSpec_Group::UpdatePatternGroup(Event* ev, Pattern_Group* pg) {
  // enforce sizes to be the same..
  while(pg->size > size) pg->Remove(pg->size-1);
  PatternSpec* ps;
  Pattern* pat;
  int i;
  for(i=0; i<size; i++) {
    ps = (PatternSpec*)FastEl(i);
    if(pg->size <= i) {
      pat = (Pattern*)pg->NewEl(1,ps->pattern_type);
    }
    else {
      pat = (Pattern*)pg->FastEl(i);
      if(pat->GetTypeDef() != ps->pattern_type) {
	pat = (Pattern*)taBase::MakeToken(ps->pattern_type);
	if(pat != NULL)
	  pg->Replace(i, pat);
      }
    }
    ps->UpdatePattern(ev,pat);
  }

  while(pg->gp.size > gp.size) pg->gp.Remove(pg->gp.size-1);
  PatternSpec_Group* pgs;
  Pattern_Group* sgp;
  for(i=0; i<gp.size; i++) {
    pgs = (PatternSpec_Group*)gp.FastEl(i);
    if(pg->gp.size <= i) {
      sgp = (Pattern_Group*)pg->NewGp(1, pgs->pat_gp_type);
    }
    else {
      sgp = (Pattern_Group*)pg->gp.FastEl(i);
      if(sgp->GetTypeDef() != pgs->pat_gp_type) {
	sgp = (Pattern_Group*)taBase::MakeToken(pgs->pat_gp_type);
	if(sgp != NULL)
	  pg->gp.Replace(i, sgp);
      }
    }
    pgs->UpdatePatternGroup(ev,sgp);
  }
}


////////////////////////////
//        EventSpec       //
////////////////////////////

void EventSpec::Initialize() {
  last_net = NULL;
  pattern_layout = HORIZONTAL;
}

void EventSpec::InitLinks() {
  BaseSpec::InitLinks();
  taBase::Own(patterns, this);
  if(!taMisc::is_loading)
    UpdateAfterEdit();
  if(taMisc::iv_active)
    AddToView();
}

void EventSpec::CutLinks() {
  if(taMisc::iv_active)
    RemoveFromView();
  patterns.CutLinks();
  taBase::DelPointer((TAPtr*)&last_net);
  BaseSpec::CutLinks();
}

void EventSpec::Copy(const EventSpec& cp) {
  BaseSpec::Copy(cp);
  patterns = cp.patterns;
  pattern_layout = cp.pattern_layout;
}
  
void EventSpec::UpdateSubSpecs() {
  EventSpec* parent = (EventSpec*)FindParent();
  if(parent != NULL) {
    // match our sub-patterns with the parents
    patterns.EnforceSameStru(parent->patterns);
  }
  PatternSpec* ps;
  taLeafItr psi;
  FOR_ITR_EL(PatternSpec, ps, patterns., psi)
    ps->UpdateSpec();
}

void EventSpec::UpdateSpec() {
  BaseSpec::UpdateSpec();
  UpdateAllEvents();
}

void EventSpec::SetLayers(Network* net) {
  PatternSpec* ps;
  taLeafItr psi;
  FOR_ITR_EL(PatternSpec, ps, patterns., psi)
    ps->SetLayer(net);
  taBase::SetPointer((TAPtr*)&last_net, net);
}

void EventSpec::UnSetLayers() {
  PatternSpec* ps;
  taLeafItr psi;
  FOR_ITR_EL(PatternSpec, ps, patterns., psi)
    ps->UnSetLayer();
  taBase::DelPointer((TAPtr*)&last_net);
}

void EventSpec::ApplyPatterns(Event* ev, Network* net) {
  if(net == NULL) {
    taMisc::Error("Event:",ev->GetPath(), " cannot apply patterns since network is NULL.",
		   "Try reseting the network pointer in your processes");
    return;
  }
  
  if(net != last_net)
    SetLayers(net);

  Pattern* pat;
  PatternSpec* ps;
  taLeafItr pi, psi;
  FOR_ITR_PAT_SPEC(Pattern, pat, ev->patterns., pi, PatternSpec, ps, patterns., psi) {
    ps->ApplyPattern(pat);
  }
}

void EventSpec::ApplyNames(Network* net) {
  if(net == NULL)
    return;
  
  SetLayers(net);

  PatternSpec* ps;
  taLeafItr psi;
  FOR_ITR_EL(PatternSpec, ps, patterns., psi) {
    ps->ApplyNames();
  }

  UnSetLayers();
}

void EventSpec::NewEvent(Event* ev) {
  ev->spec.SetSpec(this);

  ev->patterns.Reset();
  PatternSpec* ps;
  int i;
  for(i=0; i<patterns.size; i++) {
    ps = (PatternSpec*)patterns.FastEl(i);
    ps->NewPattern(ev,&(ev->patterns));
  }

  PatternSpec_Group* pg;
  for(i=0; i<patterns.gp.size; i++) {
    pg = (PatternSpec_Group*)patterns.gp.FastEl(i);
    pg->NewPatternGroup(ev,&(ev->patterns));
  }
}

void EventSpec::UpdateEvent(Event* ev) {
  patterns.UpdatePatternGroup(ev, &(ev->patterns));
}

void EventSpec::UpdateAfterEdit() {
  UnSetLayers();
  if(!taMisc::is_loading && !taMisc::is_duplicating) {
    if((patterns.leaves == 0) && (patterns.gp.size == 0)) { // new eventspec..
      patterns.NewEl(2);
      if(taMisc::iv_active)
	winbMisc::DelayedMenuUpdate(this);
    }
    if(DetectOverlap()) {
      int choice = taMisc::Choice("Overlap of PatternSpecs",
				  "Enforce Linear Layout", "Ignore");
      if(choice == 0) LinearLayout();
    }
    PatternSpec* ps;
    taLeafItr psi;
    FOR_ITR_EL(PatternSpec, ps, patterns., psi)
      ps->UpdateAfterEdit();
  }
  BaseSpec::UpdateAfterEdit();	// this calls UpdateSpec which calls UpdateAllEvents..
}

void EventSpec::UpdateAllEvents() {
  UnSetLayers();
  UpdateChildren();
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    ev->GetLocalSpec();
    if(ev->spec.spec != this)
      continue;			// only for events that use me!
    UpdateEvent(ev);
  }
  if(!taMisc::is_loading)
    env->InitAllViews();
}  

void EventSpec::UpdateFromLayers() {
  PatternSpec* ps;
  taLeafItr psi;
  FOR_ITR_EL(PatternSpec, ps, patterns., psi)
    ps->SetToLayer();
}

bool EventSpec::DetectOverlap() {
  // check for overlap
  int maxx = MaxX()+1; int maxy = MaxY()+1;
  bool* grid = new bool[maxx * maxy];
  int x;
  for(x=0;x<maxx;x++) {
    int y;
    for(y=0;y<maxy;y++)
      grid[(y*maxx) + x] = false;
  }
  PatternSpec* pat;
  taLeafItr i;
  FOR_ITR_EL(PatternSpec, pat, patterns., i) {
    int px;
    for(px=0; px<pat->geom.x; px++) {
      int py;
      for(py=0;py<pat->geom.y;py++) {
	int acty = MIN(maxy, (py + pat->pos.y));
	int actx = MIN(maxx, (px + pat->pos.x));
	int index = (acty * maxx) + actx;
	if(grid[index] != false) { // overlap
	  delete [] grid;
	  return true;
	}
	grid[index] = true;
      }
    }
  }
  delete [] grid;
  return false;
}

void EventSpec::LinearLayout(PatternLayout pat_layout) {
  if(pat_layout == DEFAULT)
    pat_layout = pattern_layout;
  else
    pattern_layout = pat_layout;
  int del_x = 1;  int del_y = 0;
  if(pat_layout == VERTICAL) {
    del_x = 1;	del_y = 0;
  }

  int xp = 0;
  int yp = 0;
  PatternSpec* pat;
  taLeafItr i;
  FOR_ITR_EL(PatternSpec, pat, patterns., i) {
    pat->pos.x = xp;
    pat->pos.y = yp;
    xp += del_x * (pat->geom.x + 1);
    yp += del_y * (pat->geom.y + 1);
    tabMisc::NotifyEdits(pat);
  }
  UpdateChildren();		// apply changes to sub specs
  UpdateAllEvents();
}
  
void EventSpec::AutoNameEvent(Event* ev, float act_thresh, int max_pat_nm, int max_val_nm) {
  String nm;
  Pattern* pat;
  PatternSpec* ps;
  taLeafItr pi, psi;
  FOR_ITR_PAT_SPEC(Pattern, pat, ev->patterns., pi, PatternSpec, ps, patterns., psi) {
    if(!nm.empty()) nm += ",";
    nm += taMisc::StringMaxLen(ps->name, max_pat_nm) + ":";
    for(int i=0;i<pat->value.size;i++) {
      if(pat->value[i] > act_thresh) {
	if(nm.lastchar() != ':') nm += "_";
	nm += taMisc::StringMaxLen(ps->value_names.SafeEl(i), max_val_nm);
      }
    }
  }
  ev->name = nm;
}

void EventSpec::AutoNameAllEvents(float act_thresh, int max_pat_nm, int max_val_nm) {
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, env->events., e) {
    if(ev->spec.spec != this)
      continue;			// only for events that use me!
    ev->AutoNameEvent(act_thresh, max_pat_nm, max_val_nm);
  }
  env->InitAllViews();
}

int EventSpec::MaxX() {
  int mx = 0;
  PatternSpec* pat;
  taLeafItr i;
  FOR_ITR_EL(PatternSpec, pat, patterns., i){
    int patx = pat->pos.x + pat->geom.x;
    mx = MAX(mx,patx);
  }
  return mx;
}

int EventSpec::MaxY() {
  int mx = 0;
  PatternSpec* pat;
  taLeafItr i;
  FOR_ITR_EL(PatternSpec, pat, patterns., i){
    int paty = pat->pos.y + pat->geom.y;
    mx = MAX(mx,paty);
  }
  return mx;
}

void EventSpec::AddToView() {
  if(!taMisc::iv_active || taMisc::is_loading) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  int leaf_idx = -2;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->AddSpec(this, leaf_idx);	// -2 is signal to update after, and find leaf idx
  }
}

void EventSpec::RemoveFromView() {
  if(!taMisc::iv_active) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->RemoveSpec(this);
  }
}

BaseSpec_MGroup* EventSpec_SPtr::GetSpecGroup() {
  Environment* env = GET_OWNER(owner,Environment);
  if(env == NULL)
    return NULL;
  return &(env->event_specs);
}


////////////////////////////
//        Pattern         //
////////////////////////////

void Pattern::Initialize() {
}

void Pattern::InitLinks() {
  taBase::Own(value, this);
  taBase::Own(flag, this);
  taOBase::InitLinks();
}

void Pattern::Copy_(const Pattern& cp) {
  value = cp.value;
  flag = cp.flag;
}

void Pattern::CutLinks() {
  value.Reset();
  flag.Reset();
  taOBase::CutLinks();
}

//////////////////////
//      Event       //
//////////////////////


void Event::Initialize() {
  index = -1;
  patterns.SetBaseType(&TA_Pattern);
}

void Event::Destroy() {
  CutLinks();
}

void Event::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(patterns, this);
  spec.SetDefaultSpec(this);
  if(taMisc::iv_active)
    AddToView();
}

void Event::CutLinks() {
  if(taMisc::iv_active)
    RemoveFromView();
  index = -1;
  patterns.Reset();
  spec.CutLinks();
  taNBase::CutLinks();
}

void Event::Copy(const Event& cp) {
  taNBase::Copy(cp);
  spec = cp.spec;
  patterns = cp.patterns;
  GetLocalSpec();
}

void Event::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  GetLocalSpec();
  if(spec.spec == NULL) return ;// no spec found (this Event might be a default object)
  if(patterns.leaves != spec->patterns.leaves) {
    int choice = taMisc::Choice("Warning:: The Event does not have the same number of patterns as its EventSpec",
				"Enforce EventSpec","Create New EventSpec");
    if(choice == 0) { // enforce
      patterns.EnforceLeaves(spec->patterns.leaves);
    }
    else { // new eventspec
      Environment* env = GET_OWNER(owner,Environment);      
      env->event_specs.DuplicateEl(spec.spec);
      EventSpec* sp = (EventSpec *) env->event_specs.Peek();
      sp->name +="(copy)";
      sp->patterns.EnforceLeaves(patterns.leaves);
      spec.SetSpec(sp);
      winbMisc::DelayedMenuUpdate(sp);
    }
  }
  spec->UpdateEvent(this);
  if(taMisc::iv_active)
    AddToView();
}

void Event::SetSpec(EventSpec* es) {
  if(es == NULL) return;
  spec.SetSpec(es);
  UpdateFmSpec();
}

void Event::UpdateFmSpec() {
  GetLocalSpec();
  patterns.EnforceLeaves(spec->patterns.leaves);
  spec->UpdateEvent(this);
}
  
void Event::GetLocalSpec() {
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL)
    return;
  if(spec.spec == NULL) {
    spec.SetSpec((EventSpec*)env->event_specs.DefaultEl());
    return;
  }
  if(GET_OWNER(spec.spec,Environment) != env) {
    EventSpec* es = (EventSpec*)env->event_specs.FindName(spec->name);
    if(es != NULL)
      spec.SetSpec(es);
    else if(spec.spec != NULL) {
      es = (EventSpec*)spec.spec->Clone(); // make one of these..
      es->name = spec.spec->name;
      env->event_specs.Add(es);
      spec.SetSpec(es);
      winbMisc::DelayedMenuUpdate(es);
    }
  }
}

String Event::GetDisplayName() {
  Event_MGroup* ownr = (Event_MGroup*)owner;
  if(ownr == NULL) return name;
  String rval = String("[") + String(index) + "] " + name;
  if(ownr->name.empty())
    return rval;
  Environment* env = GET_MY_OWNER(Environment);
  if((env == NULL) || (owner == &(env->events)))
    return rval;
  int min_len = MIN(ownr->name.length(), 4);
  return ownr->name.before(min_len) + rval;
}

void Event::AddToView() {
  if(!taMisc::iv_active || taMisc::is_loading) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->AddEvent(this, -2);	// -2 is signal to update after, and find button idx
  }
}

void Event::RemoveFromView() {
  if(!taMisc::iv_active) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->RemoveEvent(this);
  }
}

void Event::PresentEvent(TrialProcess* trial_proc, bool new_init) {
  if(trial_proc == NULL) return;
  if(trial_proc->epoch_proc != NULL)
    taBase::SetPointer((TAPtr*)&(trial_proc->epoch_proc->cur_event), this);
  else
    taBase::SetPointer((TAPtr*)&(trial_proc->cur_event), this);
  if(new_init)
    trial_proc->NewInit();
  else
    trial_proc->ReInit();
  trial_proc->Run();
}

void Event::AutoNameEvent(float act_thresh, int max_pat_nm, int max_val_nm) {
  if(spec.spec == NULL) return;
  spec.spec->AutoNameEvent(this, act_thresh, max_pat_nm, max_val_nm);
}


//////////////////////
//   Event_Group    //
//////////////////////

void Event_MGroup::Initialize() {
  SetBaseType(&TA_Event); 
}

void Event_MGroup::InitLinks() {
  PDPMGroup::InitLinks();
  if(taMisc::iv_active)
    AddToView();
}

void Event_MGroup::CutLinks() {
  RemoveFromView();
  PDPMGroup::CutLinks();
}

void Event_MGroup::UpdateAfterEdit() {
  PDPMGroup::UpdateAfterEdit();
  if(taMisc::iv_active)
    AddToView();
}

String Event_MGroup::GetDisplayName() {
  String rval = String(">> ") + name;
  if(!name.empty()) return rval;
   if(super_gp != NULL) {
    int idx = super_gp->gp.Find(this);
    rval += String("Gp[") + String(idx) + "]";
    return rval;
  }
  return rval;		// this shouldn't happen
}

void Event_MGroup::AddToView() {
  if(!taMisc::iv_active || taMisc::is_loading) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->AddEventGp(this, -2);	// -2 is signal to update after, and find button idx
  }
}

void Event_MGroup::RemoveFromView() {
  if(!taMisc::iv_active) return;
  Environment* env = GET_MY_OWNER(Environment);
  if(env == NULL) return;

  EnviroView* evv;
  taLeafItr j;
  FOR_ITR_EL(EnviroView,evv,env->views.,j) {
    if(evv->editor == NULL) continue;
    evv->editor->RemoveEventGp(this);
  }
}

TAPtr Event_MGroup::New(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  int old_sz = size;
  TAPtr rval = PDPMGroup::New(no, typ);
  // actually created elements (not groups)
  if(((typ == NULL) || !typ->InheritsFrom(&TA_taGroup_impl)) && !taMisc::is_loading)
    NewEl_impl(old_sz);
  return rval;
}

TAPtr Event_MGroup::NewEl(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  int old_sz = size;
  TAPtr rval = PDPMGroup::NewEl(no, typ);
  NewEl_impl(old_sz);
  return rval;
}

Event* Event_MGroup::NewFmSpec(int no, TypeDef* typ, EventSpec* es) {
  int old_sz = size;
  TAPtr rval = PDPMGroup::NewEl(no, typ);
  NewEl_impl(old_sz, es);
  return (Event*)rval;
}

void Event_MGroup::NewEl_impl(int old_sz, EventSpec* es) {
  if(es == NULL) {
    Environment* env = GET_MY_OWNER(Environment);
    if(env == NULL) return;
    es = (EventSpec*)env->event_specs.DefaultEl();
    if(es == NULL) return;
  }
  // number with the index implicit..
  int i;
  for(i=old_sz; i<size; i++) {
    Event* new_ev = (Event*)FastEl(i);
    es->NewEvent(new_ev);
    new_ev->name = new_ev->GetTypeDef()->name + "_" + String(i);
  }
}


//////////////////////
//   Environment    //
//////////////////////

void Environment::Initialize() {
  events.SetBaseType(&TA_Event);
  event_specs.SetBaseType(&TA_EventSpec);
  views.SetBaseType(&TA_EnviroView);
  event_ctr = 0;
}

void Environment::InitLinks() {
  taBase::Own(events, this);
  taBase::Own(event_specs, this);
  WinMgr::InitLinks();
}

void Environment::Destroy(){
  CutLinks();
}

// cutlinks is in pdpshell.cc

void Environment::Copy(const Environment& cp) {
  WinMgr::Copy(cp);
  event_specs = cp.event_specs;
  events = cp.events;
  event_ctr = cp.event_ctr;
}

void Environment::UpdateAfterEdit() {
  WinMgr::UpdateAfterEdit();
  UpdateAllEvents();
  if(taMisc::is_loading || !taMisc::iv_active) return;
  InitAllViews();
}

EventSpec* Environment::GetAnEventSpec() {
  if(event_specs.size == 0)
    return (EventSpec*)event_specs.NewEl(1);
  return (EventSpec*)event_specs.DefaultEl();
}

void Environment::UpdateAllEvents() {
  EventSpec* es;
  taLeafItr i;
  FOR_ITR_EL(EventSpec, es, event_specs., i)
    es->UpdateAllEvents();
}

void Environment::UpdateAllEventSpecs() {
  EventSpec* es;
  taLeafItr i;
  FOR_ITR_EL(EventSpec, es, event_specs., i)
    es->UpdateFromLayers();
}

void Environment::UnSetLayers() {
  EventSpec* es;
  taLeafItr i;
  FOR_ITR_EL(EventSpec, es, event_specs., i)
    es->UnSetLayers();
}

int Environment::GroupCount() {
  if(events.gp.size == 0)
    return 0;
  if(events.leaf_gp == NULL) events.InitLeafGp();
  return events.leaf_gp->size;
}

Event_MGroup* Environment::GetGroup(int i) {
  if(events.gp.size == 0)
    return NULL;
  if(events.leaf_gp == NULL) events.InitLeafGp();
  return (Event_MGroup*)events.leaf_gp->SafeEl(i);
}

Event* Environment::GetNextEvent() {
  if(events.leaves > event_ctr)
    return (Event*)events.Leaf(event_ctr++);
  return NULL;
}

void Environment::UnitNamesToNet(EventSpec* es, Network* net) {
  if(es == NULL) es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  if(net == NULL) net = pdpMisc::GetDefNetwork(GET_MY_OWNER(Project));
  if(net == NULL) return;

  es->ApplyNames(net);
}

void Environment::MakeNetIOLayers(EventSpec* es, Network* net) {
  net->RemoveUnits();
  int i;
  for(i=0;i<es->patterns.size;i++) {
    PatternSpec* ps = (PatternSpec*)es->patterns[i];
    Layer* lay = net->FindMakeLayer(ps->layer_name);
    lay->geom = ps->geom;
    lay->n_units = ps->n_vals;
    lay->UpdateAfterEdit();
  }
  winbMisc::DelayedMenuUpdate(net);
}

void Environment::AutoNameAllEvents(float act_thresh, int max_pat_nm, int max_val_nm) {
  Event* ev;
  taLeafItr e;
  FOR_ITR_EL(Event, ev, events., e) {
    ev->AutoNameEvent(act_thresh, max_pat_nm, max_val_nm);
  }
  InitAllViews();
}

void Environment::WriteText(ostream& strm, int pat_no, TextFmt fmt) {
  Event_MGroup* lst_gp = NULL;
  Event* ev;
  taLeafItr i;
  FOR_ITR_EL(Event, ev, events., i) {
    if(events.size != events.leaves) { // has sub-groups
      Event_MGroup* own = (Event_MGroup*)ev->owner;
      if((own != &events) && (own != lst_gp)) {
	strm << "# startgroup\n";
	lst_gp = own; 
      }
    }
    if(fmt == NAME_FIRST)
      strm << ev->name << "\t";
    Pattern* pat;
    if((pat_no >= 0) || (ev->patterns.leaves == 1)) {
      if(ev->patterns.leaves == 1)
	pat_no = 0;
      pat = (Pattern*)ev->patterns.Leaf(pat_no);
      int j;
      for(j=0; j<pat->value.size-1; j++)
	strm << pat->value.FastEl(j) << "\t";
      strm << pat->value.FastEl(j);
      if(fmt == NAME_LAST)
	strm << "\t" << ev->name;
      strm << "\n";
    }
    else {
      taLeafItr pi;
      FOR_ITR_EL(Pattern, pat, ev->patterns., pi) {
	int j;
	for(j=0; j<pat->value.size-1; j++)
	  strm << pat->value.FastEl(j) << "\t";
	strm << pat->value.FastEl(j) << "\n";
      }
      if(fmt == NAME_LAST)
	strm << ev->name << "\n";
    }
  }
}

// this is the reader for Old pdp format and other text formats
void Environment::ReadText(istream& strm, EventSpec* es, TextFmt fmt) {
  if((es == NULL) || (es->patterns.leaves == 0))  {
    taMisc::Error("ReadText: EventSpec was NULL or has no patterns");
    return;
  }

  Pattern* cur_pat = NULL;
  Event_MGroup* cur_gp = &events;
  Event* cur_ev = NULL;
  taLeafItr pat_itr;
  int cur_val_indx = 0;

  strm.seekg(0);
  while(strm.good() && !strm.eof()) {
    int c = taMisc::read_alnum(strm);
    if((c == EOF) || taMisc::LexBuf.empty())
      continue;
    int fc = taMisc::LexBuf.firstchar();
    // skip comments
    if((fc == '#') || (taMisc::LexBuf.before(2) == "//")) {
      if(c != '\n')
	taMisc::read_till_eol(strm);
      taMisc::LexBuf.downcase();
      if(taMisc::LexBuf.contains("startgroup")) {
	cur_gp = (Event_MGroup*)events.NewGp(1);
      }
      continue; // just a comment
    }
    if(cur_pat == NULL) {	// start of an event
      bool cont = false;
      if((cur_ev != NULL) && (fmt == NAME_LAST)) {
	cur_ev->name = taMisc::LexBuf;
	cont = true;
      }
      cur_ev = (Event*) cur_gp->NewFmSpec(1, NULL, es);
      cur_pat = (Pattern*)cur_ev->patterns.FirstEl(pat_itr);
      cur_val_indx = 0;
      if(fmt == NAME_FIRST) {
	cur_ev->name = taMisc::LexBuf;
	continue;		// done reading, get next
      }
      if(cont) continue;	// read in a name, so continue
    }
    if(cur_val_indx >= cur_pat->value.size) {
      cur_pat = (Pattern*)cur_ev->patterns.NextEl(pat_itr);
      cur_val_indx = 0;
      if(cur_pat == NULL) {
	if(fmt == NAME_LAST) {
	  cur_ev->name = taMisc::LexBuf;
	}
	cur_ev = (Event*) cur_gp->NewFmSpec(1, NULL, es);
	cur_pat = (Pattern*)cur_ev->patterns.FirstEl(pat_itr);
	if(fmt == NAME_FIRST) {
	  cur_ev->name = taMisc::LexBuf;
	  continue;		// done reading, get next
	}
	if(fmt == NAME_LAST) continue; // read it in and need to continue
      }
    }
    if(!(isdigit(fc) || (fc == '.') || (fc == '-'))) {
      taMisc::Error("*** ReadText Warning: expecting number and didn't get it:", taMisc::LexBuf,"possible format errors");
    }
    float val = (float)taMisc::LexBuf;
    cur_pat->value.Set(cur_val_indx++, val);
  }
  InitAllViews();
}

void Environment::ReadBinary(istream& strm, EventSpec* es) {
  if((es == NULL) || (es->patterns.leaves == 0))  {
    taMisc::Error("ReadBinary: EventSpec was NULL or has no patterns");
    return;
  }
  Pattern* cur_pat = NULL;
  Event* cur_ev = NULL;
  taLeafItr pat_itr;
  int cur_val_indx = 0;

  strm.seekg(0);
  while(strm.good() && !strm.eof()) {
    if(cur_pat == NULL) {
      cur_ev = (Event*)events.NewFmSpec(1, NULL, es);
      cur_pat = (Pattern*)cur_ev->patterns.FirstEl(pat_itr);
      cur_val_indx = 0;
    }
    if(cur_val_indx >= cur_pat->value.size) {
      cur_pat = (Pattern*)cur_ev->patterns.NextEl(pat_itr);
      cur_val_indx = 0;
      if(cur_pat == NULL)
	continue;		// we're done with this pattern, get new event..
    }
    // just read and plug!
    float cur_val;
    strm.read((char*)&cur_val, sizeof(float));
    cur_pat->value.Set(cur_val_indx++, cur_val);
  }
  InitAllViews();
}

void Environment::WriteBinary(ostream& strm) {
  Event* ev;
  taLeafItr i;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat;
    taLeafItr pi;
    FOR_ITR_EL(Pattern, pat, ev->patterns., pi) {
      int j;
      for(j=0; j<pat->value.size; j++) {
	float cur_val = pat->value.FastEl(j);
	strm.write((char*)&cur_val, sizeof(float));
      }
    }
  }
}

///////////////////////////////
//   Generation Routines     //
///////////////////////////////

void Environment::ReplicateEvents(int n_replicas, bool make_groups) {
  if(make_groups) {
    int i;
    for(i=0; i<events.size; i++) {
      Event* evt = (Event*)events.FastEl(i);
      String base_nm = evt->name;
      Event_MGroup* gp = (Event_MGroup*)events.NewGp(1);
      gp->name = evt->name + "_r";
      gp->Transfer(evt);
      evt->name += "_r0";
      int j;
      for(j=0; j<n_replicas; j++) {
	Event* nw_ev = (Event*)evt->Clone();
	nw_ev->name = base_nm + "_r" + String(j+1);
	gp->Add(nw_ev);
      }
      i--;
    }
  }
  else {
    int i;
    for(i=0; i<events.size; i++) {
      Event* evt = (Event*)events.FastEl(i);
      String base_nm = evt->name;
      evt->name += "_r0";
      int j;
      for(j=0; j<n_replicas; j++) {
	Event* nw_ev = (Event*)evt->Clone();
	nw_ev->name = base_nm + "_r" + String(j+1);
	events.Insert(nw_ev, ++i);
      }
    }
  }
  InitAllViews();
}

void Environment::PermutedBinary(int pat_no, int n_on) {
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      PermutedBinary(pn, n_on);
    }
    return;
  }
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    PermutedBinaryPat(pat, n_on);
  }
  UpdateAllViews();
}

void Environment::PermutedBinary_MinDist(int pat_no, int n_on, float dist,
					 float_RArray::DistMetric metric,
					 bool norm, float tol)
{
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      PermutedBinary_MinDist(pn, n_on, dist, metric, norm, tol);
    }
    return;
  }
  bool larger_further = float_RArray::LargerFurther(metric);
  int bogus_count = 0;
  int ev_ctr = 0;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    int cnt = 100 + (10 * (ev_ctr + 1));   // 100 plus 10 more for every new stim
    bool ok = false;
    float min_d;
    do {
      PermutedBinaryPat(pat, n_on);
      min_d = LastMinDist(ev_ctr, pat_no, metric, norm, tol);
      cnt--;
      if(larger_further)
	ok = (min_d >= dist);
      else
	ok = (min_d <= dist);
    } while(!ok && (cnt > 0));
    
    if(cnt == 0) {
      taMisc::Error("*** Event:", ev->name, "dist of:", (String)min_d,
		     "under dist limit:", (String)dist);
      bogus_count++;
    }
    if(bogus_count > 5) {
      taMisc::Error("Giving up after 5 stimuli under the limit, set limits lower");
      return;
    }
    ev_ctr++;
  }
  UpdateAllViews();
}

float Environment::LastMinDist(int n, int pat_no, float_RArray::DistMetric metric,
			       bool norm, float tol)
{
  bool larger_further = float_RArray::LargerFurther(metric);
  float rval;
  if(larger_further)
    rval = FLT_MAX;
  else
    rval  = -FLT_MAX;
  if(n == 0) return rval;

  n = MIN(events.leaves-1, n);
  Event* trg_ev = (Event*)events.Leaf(n);
  Pattern* trg_pat = (Pattern*)trg_ev->patterns.Leaf(pat_no);
  if(trg_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return rval;
  }

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    if(ev == trg_ev) break;
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return rval;
    }
    float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
    if(larger_further)
      rval = MIN(dst, rval);
    else
      rval = MAX(dst, rval);
  }
  return rval;
}

void Environment::FlipBits(int pat_no, int n_off, int n_on) {
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      FlipBits(pn, n_off, n_on);
    }
    return;
  }
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    FlipBitsPat(pat, n_off, n_on);
  }
  UpdateAllViews();
}

void Environment::FlipBits_MinMax(int pat_no, int n_off, int n_on, float min_dist, float max_dist,
				  float_RArray::DistMetric metric, bool norm, float tol)
{
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      FlipBits_MinMax(pn, n_off, n_on, min_dist, max_dist, metric, norm, tol);
    }
    return;
  }
  float_RArray orig_pat;
  int bogus_count = 0;
  int ev_ctr = 0;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    int cnt = 100 + (10 * (ev_ctr + 1));   // 100 plus 10 more for every new stim
    bool ok = false;
    float min_d, max_d;
    orig_pat.Reset();
    orig_pat = pat->value;
    do {
      FlipBitsPat(pat, n_off, n_on);
      min_d = LastMinMaxDist(ev_ctr, pat_no, max_d, metric, norm, tol);
      cnt--;
      ok = ((min_d >= min_dist) && (max_d <= max_dist));
      if(!ok)			// restore original pattern if not ok..
	pat->value = orig_pat;
    } while(!ok && (cnt > 0));
    
    if(cnt == 0) {
      taMisc::Error("*** Event:", ev->name, "min/max dist of:", String(min_d), 
		    String(max_d), "not within dist limits:", String(min_dist),
		    String(max_dist));
      bogus_count++;
    }
    if(bogus_count > 5) {
      taMisc::Error("Giving up after 5 stimuli under the limit, set limits lower");
      return;
    }
    ev_ctr++;
  }
  UpdateAllViews();
}

float Environment::LastMinMaxDist(int n, int pat_no, float& max_dist,
				  float_RArray::DistMetric metric, bool norm, float tol)
{
  float rval = FLT_MAX;
  max_dist = 0;
  if(n == 0) return rval;

  n = MIN(events.leaves-1, n);
  Event* trg_ev = (Event*)events.Leaf(n);
  Pattern* trg_pat = (Pattern*)trg_ev->patterns.Leaf(pat_no);
  if(trg_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return rval;
  }

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    if(ev == trg_ev) break;
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return rval;
    }
    float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
    rval = MIN(dst, rval);
    max_dist = MAX(dst, max_dist);
  }
  return rval;
}

void Environment::FlipBits_GpMinMax(int pat_no, int n_off, int n_on,
				    float within_min_dist, float within_max_dist,
				    float between_dist, float_RArray::DistMetric metric,
				    bool norm, float tol, int st_gp, int ed_gp)
{
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      FlipBits_GpMinMax(pn, n_off, n_on, within_min_dist, within_max_dist, between_dist,
			metric, norm, tol, st_gp, ed_gp);
    }
    return;
  }
  bool larger_further = float_RArray::LargerFurther(metric);
  float_RArray orig_pat;
  int bogus_count = 0;
  int g;
  int mx_gp = events.gp.size;
  if(ed_gp >= 0)
    mx_gp = MIN(ed_gp, mx_gp);
  for(g = st_gp; g < mx_gp; g++) {
    Event_MGroup* gp = (Event_MGroup*)events.gp.FastEl(g);

    int ev_ctr = 0;
    taLeafItr i;
    Event* ev;
    FOR_ITR_EL(Event, ev, gp->, i) {
      Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      int cnt = 100 + (10 * (ev_ctr + 1));   // 100 plus 10 more for every new stim
      bool ok = false;
      float min_d, max_d, min_w, max_w;
      min_w = FLT_MAX;
      orig_pat.Reset();
      orig_pat = pat->value;
      do {
	FlipBitsPat(pat, n_off, n_on);
	min_d = GpWithinMinMaxDist(gp, ev_ctr, pat_no, max_d, metric, norm, tol);
	cnt--;
	ok = ((min_d >= within_min_dist) && (max_d <= within_max_dist));
	if(!ok)			// restore original pattern if not ok..
	  pat->value = orig_pat;
	else {
	  min_w = GpLastMinMaxDist(g, pat, pat_no, max_w, metric, norm, tol);
	  if(larger_further)
	    ok = (min_w >= between_dist);
	  else
	    ok = (max_w <= between_dist); // todo: check this!
	  if(!ok)
	    pat->value = orig_pat;
	}
      } while(!ok && (cnt > 0));
    
      if(cnt == 0) {
	taMisc::Error("*** Event:", ev->name, "within min/max dist of:", String(min_d), 
		      String(max_d), "not within limits:", String(within_min_dist),
		      String(within_max_dist), "or between:", String(min_w),
		      "over:",String(between_dist));
	bogus_count++;
      }
      if(bogus_count > 5) {
	taMisc::Error("Giving up after 5 stimuli under the limit, set limits lower");
	return;
      }
      ev_ctr++;
    }
  }
  UpdateAllViews();
}

float Environment::GpWithinMinMaxDist(Event_MGroup* gp, int n, int pat_no, float& max_dist,
				      float_RArray::DistMetric metric, bool norm, float tol)
{
  float rval = FLT_MAX;
  max_dist = 0;
  if(n == 0) return rval;

  n = MIN(gp->leaves-1, n);
  Event* trg_ev = (Event*)gp->Leaf(n);
  Pattern* trg_pat = (Pattern*)trg_ev->patterns.Leaf(pat_no);
  if(trg_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return rval;
  }

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, gp->, i) {
    if(ev == trg_ev) break;
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return rval;
    }
    float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
    rval = MIN(dst, rval);
    max_dist = MAX(dst, max_dist);
  }
  return rval;
}

float Environment::GpMinMaxDist(Event_MGroup* gp, Pattern* trg_pat, int pat_no, float& max_dist,
				float_RArray::DistMetric metric, bool norm, float tol)
{
  float rval = FLT_MAX;
  max_dist = 0;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, gp->, i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return rval;
    }
    float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
    rval = MIN(dst, rval);
    max_dist = MAX(dst, max_dist);
  }
  return rval;
}

float Environment::GpLastMinMaxDist(int gp_no, Pattern* trg_pat, int pat_no, float& max_dist,
				    float_RArray::DistMetric metric, bool norm, float tol, int st_gp)
{
  float rval = FLT_MAX;
  max_dist = 0;
  int mx_gp = MIN(gp_no, events.gp.size);
  int g;
  for(g=st_gp; g<mx_gp; g++) {
    Event_MGroup* gp = (Event_MGroup*)events.gp.FastEl(g);
    float mx_dst;
    float dst = GpMinMaxDist(gp, trg_pat, pat_no, mx_dst, metric, norm, tol);
    rval = MIN(dst, rval);
    max_dist = MAX(mx_dst, max_dist);
  }
  return rval;
}

void Environment::Clear(int pat_no, float val) {
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      Clear(pn, val);
    }
    return;
  }
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    pat->value.InitVals(val);
  }
  UpdateAllViews();
}

void Environment::AddNoise(int pat_no, const Random& rnd_spec) {
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      AddNoise(pn, rnd_spec);
    }
    return;
  }
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    AddNoisePat(pat, rnd_spec);
  }
  UpdateAllViews();
}

void Environment::TransformPats(int pat_no, const SimpleMathSpec& trans) {
  if(pat_no < 0) {
    EventSpec* es = (EventSpec*)event_specs.DefaultEl();
    if(es == NULL) return;
    for(int pn = 0; pn < es->patterns.size; pn++) {
      TransformPats(pn, trans);
    }
    return;
  }
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    pat->value.SimpleMath(trans);
  }
  UpdateAllViews();
}

// DistMatrixGrid is in pdpshell.cc

void Environment::DistMatrix(ostream& strm, int pat_no, float_RArray::DistMetric metric,
			     bool norm, float tol, DistMatFmt format, int precision)
{
  if(events.leaves == 0)
    return;
  if(precision < 0) precision = 3;
  if(format == GRID_LOG)
    strm << "_H:\t";
  if(format != PRINT_NO_LABELS) {
    String nm = "$Event";
    PDPLog::LogColumn(strm, nm, 2);
  }

  int dist_wdth = 2;		// width of output
  if(metric != float_RArray::HAMMING)
    dist_wdth = 7;
  else
    precision = 0;

  bool first = true;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    String nm = ev->name;
    if((format == GRID_LOG) && first) {
      nm = String("<") + String(events.leaves) + ">" + nm;
      first = false;
    }
    if((int)nm.length() > dist_wdth)
      nm = nm.before(dist_wdth);
    if(dist_wdth == 2) {
      strm << nm << " ";
      if(nm.length() == 1)
	strm << " ";
    }
    else 
      PDPLog::LogColumn(strm, nm, 1);
  }
  strm << "\n";

  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    if(format == GRID_LOG)
      strm << "_D:\t";
    if(format != PRINT_NO_LABELS)
      PDPLog::LogColumn(strm, ev->name, 2);
    taLeafItr j;
    Event* oev;
    FOR_ITR_EL(Event, oev, events., j) {
      if((format != GRID_LOG) && (oev == ev)) break;
      Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
      if(dist_wdth == 2) {
	String val = taMisc::LeadingZeros((int)dst, dist_wdth);
	strm << val << " ";
	if(val.length() == 1)
	  strm << " ";
      }
      else {
	String val = taMisc::FormatValue(dst, dist_wdth, precision);
	PDPLog::LogColumn(strm, val, 1);
      }
    }
    strm << "\n";
  }
}

void Environment::DistArray(float_RArray& dist_ary, int pat_no,
			    float_RArray::DistMetric metric, bool norm, float tol)
{
  dist_ary.Reset();
  if(events.leaves == 0)
    return;
  for(int i=0;i<events.leaves;i++) {
    Event* ev = (Event*)events.Leaf(i);
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    for(int j=i;j<events.leaves;j++) {
      Event* oev = (Event*)events.Leaf(j);
      Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
      dist_ary.Add(dst);
    }
  }
}

void Environment::GpDistArray(float_RArray& within_dist_ary, float_RArray& between_dist_ary,
			      int pat_no, float_RArray::DistMetric metric, bool norm, float tol)
{
  between_dist_ary.Reset();
  within_dist_ary.Reset();
  if(events.leaves == 0)
    return;

  for(int g=0; g < events.gp.size; g++) {
    Event_MGroup* gp = (Event_MGroup*)events.gp[g];
    for(int i=0; i<gp->size; i++) {
      Event* ev = (Event*)gp->FastEl(i);
      Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
      if(trg_pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      // the next loop computes within-group distance
      for(int j=i; j < gp->size; j++) {	// only does upper-triangle on within
	Event* oev = (Event*)gp->FastEl(j);
	Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
	if(pat == NULL) {
	  taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	  return;
	}
	float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
	within_dist_ary.Add(dst);
      } 

      // the next loop computes between-group distance
      for(int k=g; k < events.gp.size; k++) {
	Event_MGroup* ogp = (Event_MGroup*)events.gp[g];
	for(int l=0; l < ogp->size; l++) {
	  Event* oev = (Event*)ogp->FastEl(l);
	  Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
	  if(pat == NULL) {
	    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	    return;
	  }
	  float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
	  between_dist_ary.Add(dst);
	}
      }
    }
  }
}

void Environment::CmpDistMatrix(ostream& strm, int pat_no, Environment* cmp_env, int cmp_pat_no,
				float_RArray::DistMetric metric, bool norm, float tol, DistMatFmt format)
{
  if((events.leaves == 0) || (cmp_env == NULL) || (cmp_env->events.leaves == 0))
    return;

  int precision = 3;
  if(format == GRID_LOG)
    strm << "_H:\t";
  if(format != PRINT_NO_LABELS) {
    String nm = "$Event";
    PDPLog::LogColumn(strm, nm, 2);
  }

  int dist_wdth = 2;
  if(metric != float_RArray::HAMMING)
    dist_wdth = 7;
  else
    precision = 0;

  bool first = true;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, cmp_env->events., i) {
    String nm = ev->name;
    if((format == GRID_LOG) && first) {
      nm = String("<") + String(cmp_env->events.leaves) + ">" + nm;
      first = false;
    }
    if((int)nm.length() > dist_wdth)
      nm = nm.before(dist_wdth);
    if(dist_wdth == 2) {
      strm << nm << " ";
      if(nm.length() == 1)
	strm << " ";
    }
    else 
      PDPLog::LogColumn(strm, nm, 1);
  }
  strm << "\n";

  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    if(format == GRID_LOG)
      strm << "_D:\t";
    if(format != PRINT_NO_LABELS)
      PDPLog::LogColumn(strm, ev->name, 2);
    taLeafItr j;
    Event* oev;
    FOR_ITR_EL(Event, oev, cmp_env->events., j) {
      Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(cmp_pat_no), "not found");
	return;
      }
      float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
      if(dist_wdth == 2) {
	String val = taMisc::LeadingZeros((int)dst, dist_wdth);
	strm << val << " ";
	if(val.length() == 1)
	  strm << " ";
      }
      else {
	String val = taMisc::FormatValue(dst, dist_wdth, precision);
	PDPLog::LogColumn(strm, val, 1);
      }
    }
    strm << "\n";
  }
}

void Environment::CmpDistArray(float_RArray& dist_ary, int pat_no, Environment* cmp_env, int cmp_pat_no,
			       float_RArray::DistMetric metric, bool norm, float tol)
{
  dist_ary.Reset();
  if(events.leaves == 0)
    return;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    cmp_env->CmpDistArrayPat(dist_ary, trg_pat, cmp_pat_no, metric, norm, tol);
  }
}

void Environment::CmpDistArrayPat(float_RArray& dist_ary, Pattern* trg_pat,
				  int cmp_pat_no, float_RArray::DistMetric metric,
				  bool norm, float tol)
{
  taLeafItr j;
  Event* oev;
  FOR_ITR_EL(Event, oev, events., j) {
    Pattern* pat = (Pattern*)oev->patterns.Leaf(cmp_pat_no);
    if(pat == NULL) {
      taMisc::Error("*** Pattern number:", String(cmp_pat_no), "not found");
      return;
    }
    float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
    dist_ary.Add(dst);
  }
}

void Environment::PermutedBinaryPat(Pattern* pat, int n_on, float on_val, float off_val) {
  int n_max = MIN(pat->value.size, n_on);
  int i;
  for(i=0; i<n_max; i++)
    pat->value.Set(i, on_val);
  for(;i<pat->value.size;i++)
    pat->value.Set(i, off_val);
  pat->value.Permute();
}

void Environment::FlipBitsPat(Pattern* pat, int n_off, int n_on) {
  int_Array on_ary, off_ary;
  int i;
  for(i=0; i<pat->value.size; i++) {
    if(pat->value.FastEl(i) == 1.0)
      on_ary.Add(i);
    else
      off_ary.Add(i);
  }
  on_ary.Permute();
  off_ary.Permute();
  int n_max = MIN(on_ary.size, n_off);
  for(i=0; i<n_max; i++)
    pat->value.Set(on_ary.FastEl(i), 0.0f);
  n_max = MIN(off_ary.size, n_on);
  for(i=0; i<n_max; i++)
    pat->value.Set(off_ary.FastEl(i), 1.0f);
}

void Environment::AddNoisePat(Pattern* pat, const Random& rnd_spec) {
  int i;
  for(i=0; i<pat->value.size; i++)
    pat->value.Set(i, pat->value.FastEl(i) + rnd_spec.Gen());
}

// ClusterPlot is in pdpshell.cc

void Environment::ValOverEventsArray(float_RArray& ary, int pat_no, int val_no) {
  ary.Reset();
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    ary.Add(trg_pat->value.SafeEl(val_no));
  }
}


void Environment::CorrelMatrix(float_RArray& mat, int pat_no, int& dim) {
  if(events.leaves <= 0) return;

  Event* ev = (Event*)events.Leaf(0);
  Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
  if(trg_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return;
  }
  
  dim = trg_pat->value.size;
  mat.AllocTriMatSize(dim); // upper-triangular matrix, incl diagonal

  float_RArray p1vals;
  float_RArray p2vals;

  for(int i=0;i<dim;i++) {
    ValOverEventsArray(p1vals, pat_no, i);
    for(int j=i;j<dim;j++) {
      ValOverEventsArray(p2vals, pat_no, j);
      mat.FastTriMatEl(dim, i, j) = p1vals.Correl(p2vals);
    }
  }
}

void Environment::PCAEigens(float_RArray& evecs, float_RArray& evals, int pat_no, int& dim) {
  float_RArray correl_mat;
  CorrelMatrix(correl_mat, pat_no, dim);
  evecs.CopyFmTriMat(dim, correl_mat);
  evecs.Eigens(dim, evals);
}

void Environment::ProjectPatterns(const float_RArray& prjn_vector, float_RArray& vals, int pat_no) {
  if(events.leaves <= 0) return;
  vals.Reset();
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    vals.Add(trg_pat->value.InnerProd(prjn_vector));
  }
}

void Environment::PatFreqArray(float_RArray& freqs, int pat_no, float act_thresh, bool prop) {
  freqs.Reset();
  float_RArray cnts;
  taLeafItr ei;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., ei) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    for(int i=0;i<trg_pat->value.size;i++) {
      if(freqs.size < trg_pat->value.size) {
	freqs.EnforceSize(trg_pat->value.size);
	cnts.EnforceSize(trg_pat->value.size);
      }
      float val = (trg_pat->value[i] > act_thresh) ? 1.0f : 0.0f;
      freqs[i] += val;
      cnts[i] += 1.0f;
    }
  }
  if(prop) {
    for(int i=0;i<freqs.size;i++) {
      if(cnts[i] > 0.0f) freqs[i] /= cnts[i];
    }
  }
}

void Environment::PatFreqText(float act_thresh, bool prop, ostream& strm) {
  EventSpec* es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  float_RArray freqs;
  int pat_no;
  for(pat_no = 0; pat_no < es->patterns.size; pat_no++) {
    PatternSpec* ps = (PatternSpec*)es->patterns[pat_no];
    PatFreqArray(freqs, pat_no, act_thresh, prop);
    int col_cnt = 0;
    strm << "Pattern: " << pat_no << " " << ps->name << endl;
    for(int i=0; i<freqs.size; i++) {
      col_cnt += 8;
      if(col_cnt > taMisc::display_width) {
	strm << endl;
	col_cnt = 0;
      }
      strm << taMisc::StringMaxLen(ps->value_names.SafeEl(i), 7) << "\t";
    }
    strm << endl;
    col_cnt = 0;
    for(int i=0; i<freqs.size; i++) {
      col_cnt += 8;
      if(col_cnt > taMisc::display_width) {
	strm << endl;
	col_cnt = 0;
      }
      strm << taMisc::FormatValue(freqs[i], 7, 4) << "\t";
    }
    strm << endl;
  }
}

void Environment::PatAggArray(float_RArray& agg_vals, int pat_no, Aggregate& agg) {
  agg_vals.Reset();
  EventSpec* es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  PatternSpec* ps = (PatternSpec*)es->patterns.Leaf(pat_no);
  if(ps == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return;
  }
  agg.Init();
  agg_vals.EnforceSize(ps->n_vals);
  agg_vals.InitVals(agg.InitAggVal());

  taLeafItr ei;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., ei) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    agg_vals.AggToArray(trg_pat->value, agg);
    agg.IncUpdt();
  }
}

void Environment::PatAggText(Aggregate& agg, ostream& strm) {
  EventSpec* es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  float_RArray freqs;
  int pat_no;
  for(pat_no = 0; pat_no < es->patterns.size; pat_no++) {
    PatternSpec* ps = (PatternSpec*)es->patterns[pat_no];
    PatAggArray(freqs, pat_no, agg);
    int col_cnt = 0;
    strm << "Pattern: " << pat_no << " " << ps->name << endl;
    for(int i=0; i<freqs.size; i++) {
      col_cnt += 8;
      if(col_cnt > taMisc::display_width) {
	strm << endl;
	col_cnt = 0;
      }
      strm << taMisc::StringMaxLen(ps->value_names.SafeEl(i), 7) << "\t";
    }
    strm << endl;
    col_cnt = 0;
    for(int i=0; i<freqs.size; i++) {
      col_cnt += 8;
      if(col_cnt > taMisc::display_width) {
	strm << endl;
	col_cnt = 0;
      }
      strm << taMisc::FormatValue(freqs[i], 7, 4) << "\t";
    }
    strm << endl;
  }
}

void Environment::EventFreqText(bool prop, ostream& strm) {
  strm << "\nEvent Frequencies for Environment: " << name << endl;
  float_RArray freqs;
  String_Array nms;
  taLeafItr ei;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., ei) {
    int nmi = nms.Find(ev->name);
    if(nmi < 0) {
      nms.Add(ev->name);
      freqs.Add(1.0f);
    }
    else {
      freqs[nmi]+= 1.0f;
    }
  }
  if(nms.size > 1000) {
    int chs = taMisc::Choice("Over 1,000 events to be displayed (" + String(nms.size)
			     + ") are you sure you want to proceed?", "Ok", "Cancel");
    if(chs == 1) return;
  }
  for(int i=0;i<nms.size;i++) {
    strm << taMisc::StringEnforceLen(nms[i], 63) << "\t";
    if(prop)
      strm << taMisc::FormatValue(freqs[i] / (float)events.leaves, 7, 4);
    else
      strm << freqs[i];
    strm << endl;
  }
}

//////////////////////
//   EnviroView     //
//////////////////////

void EnviroView::Initialize() {
  event_layout = HORIZONTAL;
  auto_scale = false;
  val_disp_mode = COLOR;
  val_text = NAMES;
  no_border = false;
  colorspec = NULL;
  editor = NULL;
  scale_range.min = -1.0f;
  scale_range.max = 1.0f;
  event_header = "Event::name";
  pattern_header = "PatternSpec::layer_name";
  pattern_blocks = "Pattern::values";
}

void EnviroView::InitLinks(){
  taBase::Own(events_displayed, this);
  taBase::Own(ev_gps_displayed, this);
  taBase::Own(specs_displayed, this);
  taBase::Own(scale_range, this);
  taBase::Own(view_font,this);
  taBase::Own(value_font,this);
  PDPView::InitLinks();
}

void EnviroView::CutLinks(){
  events_displayed.CutLinks();
  ev_gps_displayed.CutLinks();
  scale_range.CutLinks();
  view_font.CutLinks();
  value_font.CutLinks();
  taBase::DelPointer((TAPtr*)&colorspec);
  if(editor != NULL) { delete editor; editor = NULL; }
  PDPView::CutLinks();
}

void EnviroView::Destroy(){
  CutLinks();
}

void EnviroView::Copy_(const EnviroView& cp) {
  event_layout = cp.event_layout;
  display_toggle = cp.display_toggle;
  val_disp_mode = cp.val_disp_mode;
  val_text = cp.val_text;
  no_border = cp.no_border;
  view_font = cp.view_font;
  value_font = cp.value_font;
  scale_range = cp.scale_range;
  taBase::SetPointer((TAPtr*)&colorspec, cp.colorspec);
}

void EnviroView::UpdateAfterEdit(){
  PDPView::UpdateAfterEdit();
  if(!taMisc::iv_active)    return;
  if(editor == NULL) return; // loading
  if((colorspec != NULL) && (editor->scale->spec != colorspec)) {
    taBase::SetPointer((TAPtr*)&(editor->scale->spec), colorspec);
    editor->scale->MapColors();
    editor->cbar->SetColorScale(editor->scale);
    editor->envg->background(editor->scale->Get_Background());
  }
  InitDisplay();
}

Network* EnviroView::GetDefaultNetwork() {
  return pdpMisc::GetDefNetwork(GET_MY_OWNER(Project));
}

void EnviroView::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  ivResource::unref(body);      body = NULL;
  PDPView::CloseWindow();
}

void EnviroView::UpdateMenus_impl() {
  PDPView::UpdateMenus_impl();
  InitDisplay();
}

void EnviroView::SetEventSpec(EventSpec* es){
  if(es == NULL) return;
  int i;
  for(i=0;i<events_displayed.size;i++){
    Event* ev = (Event *) events_displayed[i]; 
    ev->spec.SetSpec(es);
    es->UpdateEvent(ev);
  }
  InitDisplay();
}

void EnviroView::ChangeEventType(TypeDef* new_type) {
  if(new_type == NULL) return;
  int i;
  for(i=events_displayed.size-1;i>=0;i--){
    Event* ev = (Event *) events_displayed[i];  
    ev->ChangeMyType(new_type);
  }
  winbMisc::DelayedMenuUpdate(this);
}

void EnviroView::ChangeEventGpType(TypeDef* new_type) {
  if(new_type == NULL) return;
  int i;
  for(i=ev_gps_displayed.size-1;i>=0;i--){
    Event_MGroup* ev = (Event_MGroup*) ev_gps_displayed[i];  
    ev->ChangeMyType(new_type);
  }
  winbMisc::DelayedMenuUpdate(this);
}

void EnviroView::DuplicateEvents() {
  int i;
  for(i=0;i<events_displayed.size;i++){
    Event* ev = (Event *) events_displayed[i];  
    ev->DuplicateMe();
  }
  winbMisc::DelayedMenuUpdate(this);
}

void EnviroView::DuplicateEventGps() {
  int i;
  for(i=0;i<ev_gps_displayed.size;i++){
    Event_MGroup* ev = (Event_MGroup*) ev_gps_displayed[i];  
    ev->DuplicateMe();
  }
  winbMisc::DelayedMenuUpdate(this);
}

void EnviroView::UpdateDisplay(TAPtr updtr) {
  if(!IsMapped() || !display_toggle || editor == NULL) return;
  if(updtr == NULL) { editor->UpdateDisplay(); return;}
  Event* ev = NULL;
  if(updtr->InheritsFrom(&TA_TrialProcess)){ // fast check for common case
    ev = ((TrialProcess *) updtr)->cur_event;
  }
  else {
    MemberDef* mbr;
    if(((mbr = updtr->FindMember(&TA_Event)) != NULL) && // an event member
       (mbr->type->ptr == 1)) { // that is a pointer to an event
      ev = (Event *) mbr->GetOff(updtr);
    }
  }
  if (ev==NULL) return;
  editor->UnDisplayAllEvents();
  editor->DisplayEvent(ev);
  if(anim.capture) CaptureAnimImg();
}	

void EnviroView::InitDisplay() {
  if(!taMisc::iv_active)
    return;
  if(editor != NULL) 
    editor->InitDisplay();
}

void EnviroView::SetColorSpec(ColorScaleSpec* colors) {
  taBase::SetPointer((TAPtr*)&colorspec, colors);
  UpdateAfterEdit();
}

void EnviroView::SetLayout(EventLayout layout) {
  event_layout = layout;
  UpdateAfterEdit();
}

void EnviroView::SetBorder(bool border) {
  no_border = !border;
  UpdateAfterEdit();
}

void EnviroView::SetViewFontSize(int point_size) {
  view_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void EnviroView::SetValueFontSize(int point_size) {
  value_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void EnviroView::SelectEvents(int start, int n_events) {
  Environment* env = (Environment*)mgr;
  if(env == NULL)  return;
  if(n_events == -1)
    n_events = env->events.leaves;
  int i;
  for(i=0; i<n_events; i++) {
    Event* ev = (Event*)env->events.Leaf(start + i);
    if(ev == NULL) break;
    events_displayed.LinkUnique(ev);
  }
  InitDisplay();
}

void EnviroView::DeselectEvents(int start, int n_events) {
  Environment* env = (Environment*)mgr;
  if(env == NULL)  return;
  if(n_events == -1)
    n_events = env->events.leaves;
  int i;
  for(i=0; i<n_events; i++) {
    Event* ev = (Event*)env->events.Leaf(start + i);
    if(ev == NULL) break;
    events_displayed.Remove(ev);
  }
  InitDisplay();
}

void EnviroView::SelectEvent(Event* ev) {
  events_displayed.LinkUnique(ev);
  InitDisplay();
}

void EnviroView::DeselectEvent(Event* ev) {
  events_displayed.Remove(ev);
  InitDisplay();
}

void EnviroView::SelectEventGp(Event_MGroup* eg) {
  ev_gps_displayed.LinkUnique(eg);
  InitDisplay();
}

void EnviroView::DeselectEventGp(Event_MGroup* eg) {
  ev_gps_displayed.Remove(eg);
  Event* ev;
  taLeafItr ei;
  FOR_ITR_EL(Event, ev, eg->, ei) {
    events_displayed.Remove(ev);
  }
  InitDisplay();
}

void EnviroView::SelectGpEvents(Event_MGroup* eg) {
  ev_gps_displayed.Remove(eg);
  Event* ev;
  taLeafItr ei;
  FOR_ITR_EL(Event, ev, eg->, ei) {
    events_displayed.Link(ev);
  }
  InitDisplay();
}

void EnviroView::DeselectAllEvents() {
  events_displayed.Reset();
  ev_gps_displayed.Reset();
  InitDisplay();
}

void EnviroView::SetPatLabel(const char* pat_lbl) {
  pattern_header = pat_lbl;
  editor->SetPatHeadStr(pat_lbl);
  InitDisplay();
}

void EnviroView::SetEventLabel(const char* evt_lbl) {
  event_header = evt_lbl;
  editor->SetEventHeadStr(evt_lbl);
  InitDisplay();
}


void EnviroView::GetBodyRep() {
  if(!taMisc::iv_active) return;
  if(body != NULL) return;
  Environment* env = (Environment*)mgr;
  editor = new EnvEditor(env, this, window);
  editor->Init();
  body = editor->GetLook();
  ivResource::ref(body);
}

ivGlyph* EnviroView::GetPrintData(){
  return editor->print_patch;
}
