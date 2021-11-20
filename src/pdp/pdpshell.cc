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

// PDPShell based on C^c C super script language

#include <pdp/pdpshell.h>
#include <pdp/sched_proc.h>
#include <pdp/net_iv.h>
#include <pdp/pdp_iv.h>
// some functions are instantited here for these extra procs
#include <pdp/procs_extra.h>
#include <pdp/enviro_extra.h>
#include <ta/taiv_type.h>
#include <ta/ta_dump.h>
#include <css/ta_css.h>
#include <css/css_iv.h>
#include <css/css_builtin.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <unistd.h>
#include <sstream>
#include <time.h>

#include <ta/enter_iv.h>
#include <IV-look/kit.h>
#include <IV-look/dialogs.h>
#include <InterViews/window.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/patch.h>
#include <InterViews/stencil.h>
#include <InterViews/background.h>
#include <InterViews/handler.h> // for wm_delete
#ifndef CYGWIN
#include <IV-X11/xwindow.h>		// this is for the window title hack
#endif
#include <InterViews/bitmap.h>
#include <ta/leave_iv.h>

#ifdef DMEM_COMPILE
#include <mpi.h>
#endif

static ivOptionDesc PDP_options[] = {
    { NULL }
};

static ivPropertyData PDP_defs[] = {
  {"PDP++*gui", "sgimotif"},
  {"PDP++*PopupWindow*overlay", "true"},
  {"PDP++*PopupWindow*saveUnder", "on"},
  {"PDP++*TransientWindow*saveUnder", "on"},
  {"PDP++*double_buffered",	"on"},
  {"PDP++*flat",		"#c0c4d3"},
  {"PDP++*background",  	"#70c0d8"},
  {"PDP++*name*flat",		"#70c0d8"},
  {"PDP++*apply_button*flat",	"#c090b0"},
  {"PDP++*FieldEditor*background", "white"},
  {"PDP++*FileChooser*filter", 	"on"},
  {"PDP++*FileChooser.rows", 	"20"},
  {"PDP++*FileChooser.width", 	"300"},
  {"PDP++*taivObjChooser.width", "300"},
  {"PDP++*taivObjChooser.rows",	"20"},
  {"PDP++*PaletteButton*minimumWidth", "72.0"},
  {"PDP++*PushButton*minimumWidth", "72.0"},
  {"PDP++*TaIVButton*SmallWidth", "46.0"},
  {"PDP++*TaIVButton*MediumWidth", "72.0"},
  {"PDP++*TaIVButton*BigWidth", "115.0"},
  {"PDP++*toggleScale",		"1.5"},
#ifdef DARWIN
  {"PDP++*font",		"*-helvetica-medium-r-*-*-11*"},
  {"PDP++*name*font",		"*-helvetica-medium-r-*-*-11*"},
  {"PDP++*title*font",		"*-helvetica-bold-r-*-*-11*"},
  {"PDP++*small_menu*font",	"*-helvetica-medium-r-*-*-11*"},
  {"PDP++*small_submenu*font",	"*-helvetica-medium-r-*-*-11*"},
  {"PDP++*big_menu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"PDP++*big_submenu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"PDP++*big_menubar*font",	"*-helvetica-bold-r-*-*-12*"},  // 18 is next biggest, too big
  {"PDP++*big_italic_menubar*font","*-helvetica-bold-o-*-*-12*"},
#else
#ifdef CYGWIN
  {"PDP++*font",		"*Arial*medium*--12*"},
  {"PDP++*name*font",		"*Arial*medium*--12*"},
  {"PDP++*title*font",		"*Arial*bold*--12*"},
  {"PDP++*small_menu*font",	"*Arial*medium*--12*"},
  {"PDP++*small_submenu*font",	"*Arial*medium*--12*"},
  {"PDP++*big_menu*font",	"*Arial*medium*--12*"},
  {"PDP++*big_submenu*font",	"*Arial*medium*--12*"},
  {"PDP++*big_menubar*font",	"*Arial*bold*--14*"},
  {"PDP++*big_italic_menubar*font","*Arial*italic*--14*"},
  // following are def'd in smf_kit.cpp
  {"PDP++*MenuBar*font", 	"*Arial*bold*--12*"},
  {"PDP++*MenuItem*font", 	"*Arial*bold*--12*"},
  {"PDP++*MenuBar*font", 	"*Arial*bold*--12*"},
  {"PDP++*MenuItem*font", 	"*Arial*bold*--12*"},
  // this sets the scaling of the windows to 1.25 -- much closer to unix pdp sizing
  {"PDP++*mswin_scale",		"1.25"},
#else
  {"PDP++*font",		"*-helvetica-medium-r-*-*-10*"},
  {"PDP++*name*font",		"*-helvetica-medium-r-*-*-10*"},
  {"PDP++*title*font",		"*-helvetica-bold-r-*-*-10*"},
  {"PDP++*small_menu*font",	"*-helvetica-medium-r-*-*-10*"},
  {"PDP++*small_submenu*font",	"*-helvetica-medium-r-*-*-10*"},
  {"PDP++*big_menu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"PDP++*big_submenu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"PDP++*big_menubar*font",	"*-helvetica-bold-r-*-*-14*"},
  {"PDP++*big_italic_menubar*font","*-helvetica-bold-o-*-*-14*"},
#endif
#endif
  { NULL } 
};


//////////////////////////////////////////////////////////////////////////////////
//     	Functions from other files which reference Project or root in passing	//
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////
//	spec.h		//
//////////////////////////

BaseSpec_MGroup* SpecPtr_impl::GetSpecGroup() {
  Project* prj = GET_OWNER(owner,Project);
  if(prj == NULL)
    return NULL;
  return &(prj->specs);
}

//////////////////////////
//	netstru.h	//
//////////////////////////

/////////// Con Spec ////////////

static int conspec_repl_bias_ptr(UnitSpec* us, ConSpec* old, ConSpec* nw) {
  int cnt = 0;
  UnitSpec* u2;
  taLeafItr i;
  FOR_ITR_EL(UnitSpec, u2, us->children., i) {
    if(u2->bias_spec.spec == old) {
      u2->bias_spec.SetSpec(nw); // update to new
      cnt++;
    }
    if(u2->children.leaves > 0)
      cnt += conspec_repl_bias_ptr(u2, old, nw);
  }
  return cnt;
}

void ConSpec::ReplacePointersHook(TAPtr old) {
  ConSpec* spold = (ConSpec*)old;
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  winbMisc::DelayedMenuUpdate(this);
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->ReplaceConSpec(spold, this);
  }

  // now go through specs!
  BaseSpec* sp;
  taLeafItr si;
  FOR_ITR_EL(BaseSpec, sp, prj->specs., si) {
    if(!sp->InheritsFrom(TA_UnitSpec)) continue;
    UnitSpec* us = (UnitSpec*)sp;
    if(us->bias_spec.spec == old) {
      us->bias_spec.SetSpec(this); // update to new
    }
    if(us->children.leaves > 0)
      conspec_repl_bias_ptr(us, spold, this);
  }
  int i;
  for(i=0;i<spold->children.size && i<children.size;i++) {
    children.FastEl(i)->ReplacePointersHook(spold->children.FastEl(i));
  }
  BaseSpec::ReplacePointersHook(old);
}

int ConSpec::UseCount() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if(prj->networks.leaves == 0) return -1; // no networks!
  int cnt = 0;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni) {
    cnt += net->ReplaceConSpec(this, this);
  }
  // now go through specs!
  BaseSpec* sp;
  taLeafItr si;
  FOR_ITR_EL(BaseSpec, sp, prj->specs., si) {
    if(!sp->InheritsFrom(TA_UnitSpec)) continue;
    UnitSpec* us = (UnitSpec*)sp;
    if(us->bias_spec.spec == this)
      cnt++;
    if(us->children.leaves > 0)
      cnt += conspec_repl_bias_ptr(us, this, this);
  }
  return cnt;
}

void ConSpec::InitWeights() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->InitWtState();
    net->UpdateAllViews();
  }
}

void ConSpec::CutLinks() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->deleting) {
    ConSpec* rsp = (ConSpec*)prj->specs.FindSpecTypeNotMe(GetTypeDef(), this);
    if(rsp == NULL) {
      rsp = (ConSpec*)prj->specs.FindSpecInheritsNotMe(&TA_ConSpec, this);
    }
    if(rsp != NULL) {
      int cnt = 0;
      Network* net;
      taLeafItr ni;
      FOR_ITR_EL(Network, net, prj->networks., ni) {
	cnt += net->ReplaceConSpec(this, rsp);
      }
      if(cnt > 0) {
	taMisc::Error("Warning: ConSpec",this->GetPath(),"was used in the network, replaced with",rsp->GetPath());
      }

      // now go through specs!
      BaseSpec* sp;
      taLeafItr si;
      FOR_ITR_EL(BaseSpec, sp, prj->specs., si) {
	if(!sp->InheritsFrom(TA_UnitSpec)) continue;
	UnitSpec* us = (UnitSpec*)sp;
	if(us->bias_spec.spec == this) {
	  us->bias_spec.SetSpec(rsp); // update to new
	}
	if(us->children.leaves > 0)
	  conspec_repl_bias_ptr(us, this, rsp);
      }
    }
    else {
      taMisc::Error("Warning: Deleting ConSpec",this->GetPath(),"and couldn't find replacement - network will have NULL spec and crash!");
    }
  }
  BaseSpec::CutLinks();
}

/////////// Unit Spec ////////////

void UnitSpec::UpdateAfterEdit() {
  BaseSpec::UpdateAfterEdit();
  act_range.UpdateAfterEdit();
  bias_spec.CheckSpec();
  if((bias_con_type != NULL) && (bias_spec.spec != NULL) && !bias_con_type->InheritsFrom(bias_spec.spec->min_con_type)) {
    taMisc::Error("Bias con type of:", bias_con_type->name,
		  "is not of the correct type for the bias con spec,"
		  "which needs at least a:", bias_spec.spec->min_con_type->name);
    return;
  }
  if(taMisc::is_loading) return;
  if(UseCount() == 0) {
    if(!not_used_ok)
      taMisc::Error("Note: you just edited:",GetTypeDef()->name,GetPath(),"but it is not used anywhere!");
    not_used_ok = true;
  }

  if(!taMisc::iv_active) return;
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni) {
    NetView* vw;
    taLeafItr vi;
    FOR_ITR_EL(NetView, vw, net->views., vi) {
      vw->UpdateButtons();	// update buttons to reflect need to build or not..
    }
  }
}

int UnitSpec::UseCount() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if(prj->networks.leaves == 0) return -1; // no networks!
  int cnt = 0;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni) {
    cnt += net->ReplaceUnitSpec(this, this);
  }
  return cnt;
}

void UnitSpec::BuildBiasCons() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->Build();
  }
}

void UnitSpec::CutLinks() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->deleting) {
    UnitSpec* rsp = (UnitSpec*)prj->specs.FindSpecTypeNotMe(GetTypeDef(), this);
    if(rsp == NULL) {
      rsp = (UnitSpec*)prj->specs.FindSpecInheritsNotMe(&TA_UnitSpec, this);
    }
    if(rsp != NULL) {
      int cnt = 0;
      Network* net;
      taLeafItr ni;
      FOR_ITR_EL(Network, net, prj->networks., ni) {
	cnt += net->ReplaceUnitSpec(this, rsp);
      }
      if(cnt > 0) {
	taMisc::Error("Warning: UnitSpec",this->GetPath(),"was used in the network, replaced with",rsp->GetPath());
      }
    }
    else {
      taMisc::Error("Warning: Deleting UnitSpec",this->GetPath(),"and couldn't find replacement - network will have NULL spec and crash!");
    }
  }
  BaseSpec::CutLinks();
  bias_spec.CutLinks();
}

void UnitSpec::ReplacePointersHook(TAPtr old) {
  UnitSpec* spold = (UnitSpec*)old;
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  winbMisc::DelayedMenuUpdate(this); // this will reset flag
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->ReplaceUnitSpec(spold, this);
  }
  int i;
  for(i=0;i<spold->children.size && i<children.size;i++) {
    children.FastEl(i)->ReplacePointersHook(spold->children.FastEl(i));
  }
  BaseSpec::ReplacePointersHook(old);
}

/////////// Layer Spec ////////////

void LayerSpec::CutLinks() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->deleting) {
    LayerSpec* rsp = (LayerSpec*)prj->specs.FindSpecTypeNotMe(GetTypeDef(), this);
    if(rsp == NULL) {
      rsp = (LayerSpec*)prj->specs.FindSpecInheritsNotMe(&TA_LayerSpec, this);
    }
    if(rsp != NULL) {
      int cnt = 0;
      Network* net;
      taLeafItr ni;
      FOR_ITR_EL(Network, net, prj->networks., ni) {
	cnt += net->ReplaceLayerSpec(this, rsp);
      }
      if(cnt > 0) {
	taMisc::Error("Warning: LayerSpec",this->GetPath(),"was used in the network, replaced with",rsp->GetPath());
      }
    }
    else {
      taMisc::Error("Warning: Deleting LayerSpec",this->GetPath(),"and couldn't find replacement - network will have NULL spec and crash!");
    }
  }
  BaseSpec::CutLinks();
}

void LayerSpec::ReplacePointersHook(TAPtr old) {
  LayerSpec* spold = (LayerSpec*)old;
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  winbMisc::DelayedMenuUpdate(this); // this will reset flag
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->ReplaceLayerSpec(spold, this);
  }
  int i;
  for(i=0;i<spold->children.size && i<children.size;i++) {
    children.FastEl(i)->ReplacePointersHook(spold->children.FastEl(i));
  }
  BaseSpec::ReplacePointersHook(old);
}

int LayerSpec::UseCount() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if(prj->networks.leaves == 0) return -1; // no networks!
  int cnt = 0;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni) {
    cnt += net->ReplaceLayerSpec(this, this);
  }
  return cnt;
}

/////////// Projection Spec ////////////

void ProjectionSpec::CutLinks() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->deleting) {
    ProjectionSpec* rsp = (ProjectionSpec*)prj->specs.FindSpecTypeNotMe(GetTypeDef(), this);
    if(rsp == NULL) {
      rsp = (ProjectionSpec*)prj->specs.FindSpecInheritsNotMe(&TA_ProjectionSpec, this);
    }
    if(rsp != NULL) {
      int cnt = 0;
      Network* net;
      taLeafItr ni;
      FOR_ITR_EL(Network, net, prj->networks., ni) {
	cnt += net->ReplacePrjnSpec(this, rsp);
      }
      if(cnt > 0) {
	taMisc::Error("Warning: ProjectionSpec",this->GetPath(),"was used in the network, replaced with",rsp->GetPath());
      }
    }
    else {
      taMisc::Error("Warning: Deleting ProjectionSpec",this->GetPath(),"and couldn't find replacement - network will have NULL spec and crash!");
    }
  }
  BaseSpec::CutLinks();
}

void ProjectionSpec::ReplacePointersHook(TAPtr old) {
  ProjectionSpec* spold = (ProjectionSpec*)old;
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if (prj == NULL) return;
  winbMisc::DelayedMenuUpdate(this); // this will reset flag
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni){
    net->ReplacePrjnSpec(spold, this);
  }
  int i;
  for(i=0;i<spold->children.size && i<children.size;i++) {
    children.FastEl(i)->ReplacePointersHook(spold->children.FastEl(i));
  }
  BaseSpec::ReplacePointersHook(old);
}

int ProjectionSpec::UseCount() {
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if(prj->networks.leaves == 0) return -1; // no networks!
  int cnt = 0;
  Network* net;
  taLeafItr ni;
  FOR_ITR_EL(Network, net, prj->networks., ni) {
    cnt += net->ReplacePrjnSpec(this, this);
  }
  return cnt;
}

void Projection::GridViewWeights(GridLog* disp_log, bool use_swt, int un_x, int un_y, int wt_x, int wt_y) {
  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    disp_log->Clear();
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = (String)"GridViewWeights: " + layer->name + ", " + name;

  DataTable* md = &(disp_log->data);
  md->RemoveAll();
  
  md->NewColString("row");

  int rx = (un_x < 0) ? layer->act_geom.x : un_x;
  int ry = (un_y < 0) ? layer->act_geom.y : un_y;

  int wtx = (wt_x < 0) ? from->act_geom.x : wt_x;
  int wty = (wt_y < 0) ? from->act_geom.y : wt_y;

  int totn = wtx * wty;

  // these are the columns
  int i;
  for(i=0; i<rx; i++) {
    md->NewGroupFloat((String)"c" + (String)i, totn);
    md->AddColDispOpt(String("GEOM_X=") + String(wtx), 0, i); // column 0, subgp i
    md->AddColDispOpt(String("GEOM_Y=") + String(wty), 0, i); // column 0, subgp i
  }

  md->ResetData();

  for(i=0;i<ry;i++) {
    md->AddBlankRow();		// add the rows
    md->SetStringVal(String(ry - i - 1), 0, i);	// col 0, row i
  }

  int uni = 0;
  int ux, uy;
  for(uy=0;uy<ry; uy++) {
    if(uni >= layer->units.leaves)
      break;
    for(ux=0; ux<layer->act_geom.x; ux++, uni++) {
      if(uni >= layer->units.leaves)
	break;
      if(ux >= rx) {
	uni += layer->act_geom.x - rx; break;
      }
      Unit* un = (Unit*)layer->units.Leaf(uni);
      Con_Group* cg = un->recv.FindFrom(from);
      if(cg == NULL)
	break;
      int wi;
      for(wi=0;wi<cg->size;wi++) {
	Unit* su = cg->Un(wi);
	Unit_Group* ownr = ((Unit_Group*)su->owner);
	int sx, sy;
	sx = su->pos.x + ownr->pos.x; 
	sy = su->pos.y + ownr->pos.y;
	if((sx >= wtx) || (sy >= wty)) continue;
	int sidx = (sy * wtx) + sx;
	float wtval = cg->Cn(wi)->wt;
	if(use_swt) {
	  Connection* cn = cg->FindRecipSendCon(un, su);
	  if(cn == NULL) continue;
	  wtval = cn->wt;
	}
	md->SetFloatVal(wtval, sidx, ry-uy-1, ux);
      }
    }
  }
  
  disp_log->ViewAllData();
}

void Projection::WeightsToEnv(Environment* env) {
  if(from == NULL) return;
  if(env == NULL) {
    env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project));
  }
  if(env == NULL) return;

  env->events.Reset();
  env->name = (String)"WeightsToEnv: " + layer->name + ", " + name;

  EventSpec* es = env->GetAnEventSpec();
  es->UpdateAfterEdit();	// make sure its all done with internals..
  es->patterns.EnforceSize(1);

  PatternSpec* ps = (PatternSpec*)es->patterns[0];
  ps->n_vals = from->units.leaves;
  ps->geom = from->act_geom;
  ps->UpdateAfterEdit();	// this will sort out cases where nvals > geom
  es->UpdateAllEvents();	// get them all straightened out

  int idx = 0;
  taLeafItr ri;
  Unit* ru;
  FOR_ITR_EL(Unit, ru, layer->units., ri) {
    Con_Group* cg = ru->recv.FindFrom(from);
    if(cg == NULL)
      break;
    Event* ev = (Event*)env->events.NewEl(1);
    if(!ru->name.empty())
      ev->name = ru->name;
    else 
      ev->name = layer->name + String("[") + String(idx) + String("]");
    Pattern* pat = (Pattern*)ev->patterns[0];
    int wi;
    for(wi=0;wi<cg->size;wi++) {
      pat->value.SafeEl(wi) = cg->Cn(wi)->wt;
    }
    idx++;
  }
  env->InitAllViews();
}

///////////////// Network //////////////////////


void Layer::CutLinks() {
  static bool in_repl = false;
  if(in_repl || (owner == NULL)) return; // already replacing or already dead
  DisConnect();
  // remove layer transforms and layer_name transforms
  if(own_net != NULL) {
    int index = own_net->layers.FindLeaf(this);
    Network* prvnet = own_net;
    own_net = NULL;		// this indicates to netview not to display this guy (or anything in it)
    Xform* xf;
    taLeafItr i;
    NetView* view;
    FOR_ITR_EL(NetView, view, prvnet->views., i) {
      if(view->lay_forms != NULL) {
	if((xf = view->lay_forms->FindName(String(index))) != NULL) {
	  view->lay_forms->Remove(xf);
	  int j;
	  for(j=0;j<view->lay_forms->size;j++) {
	    Xform* xf = (Xform*)view->lay_forms->FastEl(j);
	    int nmdx = (int)xf->name;
	    if(nmdx > index) xf->name = (String)(nmdx - 1);
	  }
	}
      }
      if(view->label_forms != NULL) {
	if((xf = view->label_forms->FindName(String(index))) != NULL) {
	  view->label_forms->Remove(xf);
	  int j;
	  for(j=0;j<view->label_forms->size;j++) {
	    Xform* xf = (Xform*)view->label_forms->FastEl(j);
	    int nmdx = (int)xf->name;
	    if(nmdx > index) xf->name = (String)(nmdx - 1);
	  }
	}
      }
      view->InitDisplay();
    }
  }
  send_prjns.CutLinks();
  projections.CutLinks();
  units.CutLinks();
  unit_spec.CutLinks();
  // un-set any other pointers to this object!
  Project* proj = GET_MY_OWNER(Project);
  if((proj != NULL) && !proj->deleting) {
    in_repl = true;
    taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)this, NULL);
    in_repl = false;
  }
  taNBase::CutLinks();
}

void Network::CutLinks() {
  static bool in_repl = false;
  if(in_repl || (owner == NULL)) return; // already replacing or already dead
#ifdef DMEM_COMPILE
  if(dmem_share_units.comm != MPI_COMM_SELF) {
    DMEM_MPICALL(MPI_Comm_free((MPI_Comm*)&dmem_share_units.comm),
		       "Network::CutLinks", "net Comm_free");
    DMEM_MPICALL(MPI_Group_free((MPI_Group*)&dmem_gp),
		       "Network::CutLinks", "net Group_free");
  }
#endif
  views.Reset();
  layers.CutLinks();
  if((proj != NULL) && !proj->deleting) {
    Network* replnet = NULL;
    int nl;
    for(nl=0;nl<proj->networks.leaves;nl++) {
      Network* nt = (Network*)proj->networks.Leaf(nl);
      if(nt != this) {
	replnet = nt;
	break;
      }
    }
    if(replnet == NULL) {
      taMisc::Error("Warning: Deleting Network:",this->GetPath(),"and couldn't find replacement - processes will have NULL network pointers");
    }
    proj->processes.ReplaceNetPtrs(this, replnet);
    // also replace pointers on any netlogviews..
    taLeafItr lgi;
    PDPLog* lg;
    FOR_ITR_EL(PDPLog, lg, proj->logs., lgi) {
      taLeafItr vwi;
      LogView* vw;
      FOR_ITR_EL(LogView, vw, lg->views.,vwi) {
	if(vw->InheritsFrom(TA_NetLogView)) {
	  taBase::SetPointer((TAPtr*)&(((NetLogView*)vw)->network), replnet);
	}
      }
    }
    // un-set any other pointers to this object!
    in_repl = true;
    taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)this, NULL);
    in_repl = false;
  }
  proj = NULL;
  WinMgr::CutLinks();
}

void Network::RemoveMonitors() {
  if(proj == NULL) return;
  taLeafItr pi;
  SchedProcess* sp;
  FOR_ITR_EL(SchedProcess, sp, proj->processes., pi) {
    taLeafItr si;
    Stat* st;
    FOR_ITR_EL(Stat, st, sp->loop_stats., si) {
      if(st->InheritsFrom(TA_MonitorStat))
	((MonitorStat*)st)->ptrs.RemoveAll();
    }
    FOR_ITR_EL(Stat, st, sp->final_stats., si) {
      if(st->InheritsFrom(TA_MonitorStat))
	((MonitorStat*)st)->ptrs.RemoveAll();
    }
  }
}

void Network::UpdateMonitors() {
  if(proj == NULL) return;
  taLeafItr pi;
  SchedProcess* sp;
  FOR_ITR_EL(SchedProcess, sp, proj->processes., pi) {
    taLeafItr si;
    Stat* st;
    FOR_ITR_EL(Stat, st, sp->loop_stats., si) {
      if(st->InheritsFrom(TA_MonitorStat))
	((MonitorStat*)st)->UpdateAfterEdit();
    }
    FOR_ITR_EL(Stat, st, sp->final_stats., si) {
      if(st->InheritsFrom(TA_MonitorStat))
	((MonitorStat*)st)->UpdateAfterEdit();
    }
  }
}

// cfront requires this to be outside class function
enum NetSection {NS_NONE, NS_DEFINITIONS, NS_CONSTRAINTS,
		   NS_NETWORK, NS_BIASES};

ConSpec* GetNSConSpec(char name,BaseSpec_MGroup * bsmg,
		      BaseSpec_MGroup* master, TypeDef* conspec_type,
		      bool skip_dots=true){
  if((skip_dots == true) && (name == '.')) return NULL;
  int i;
  ConSpec* result;
  for(i=0;i<bsmg->leaves;i++){
    result = (ConSpec *) bsmg->Leaf(i);
    if(!result->name.empty() && (result->name[0] == name)) return result;
  }
  if((name == 'r') || (name == 'p') ||
     (name == 'n') || (name == '.')) { // pre-defined specs
    result  = (ConSpec *) master->New(1,conspec_type);
    bsmg->Link(result);
    result->name = name;
    result->rnd.type = Random::UNIFORM;
    if(name == 'r') {result->rnd.mean = 0.0; result->rnd.var = .5;}
    else if(name == 'p') {result->rnd.mean = 0.5; result->rnd.var = .5;}
    else if(name == 'n') {result->rnd.mean = -0.5; result->rnd.var = .5;}
    else if(name == '.') {result->rnd.mean = 0; result->rnd.var = 0;}
    return result;
  }
  // search for uppercase version
  for(i=0;i<bsmg->leaves;i++){
    result = (ConSpec *) bsmg->Leaf(i);
    String upnm = result->name;
    upnm.upcase();
    if(!upnm.empty() && (upnm[0] == name)){
      master->DuplicateEl(result);
      result = (ConSpec *) master->Leaf(master->leaves-1);
      result->name = name;
    }
    // todo set lrate or something to 0 to make upcase
    // version unmodifiable
  }
  taMisc::Error("Connection Specification letter \"", String(name) , "\" not found");
  return NULL;
}


void Network::ReadOldPDPNet(istream& strm, bool skip_dots){
  NetSection ns = NS_NONE;
  int nunits= 0;
  int ninputs = 0;
  int nhiddens = 0;
  int noutputs= 0;
  Unit_Group flat_units;
  BaseSpec_MGroup conspec_group;
  int send_start = 0;
  int send_end = 0;
  int recv_start = 0;
  int recv_end = 0;
  int cur_send = 0;
  int cur_recv = 0;
  char con_char = '\0';
  Project* prj = (Project *) GET_MY_OWNER(Project);
  Layer* lay = (Layer *) layers.New(1);
  Projection* prjn = (Projection *) lay->projections.New(1);
  TypeDef* conspec_type = prjn->con_spec.type;
  lay->projections.Remove(prjn);
  layers.Remove(lay);

  strm.seekg(0);
  while(strm.good() && !strm.eof()){
    int c = taMisc::read_alnum(strm);
    if((c == EOF) || taMisc::LexBuf.empty())
      continue;
    int fc = taMisc::LexBuf.firstchar();
    // skip comments
    if((fc == '#') || (taMisc::LexBuf.before(2) == "//")) {
      taMisc::read_till_eol(strm);
      continue;
    }
    if(ns == NS_NONE){
      if(taMisc::LexBuf == "") continue; // not a section line
      taMisc::LexBuf.upcase();
      if(taMisc::LexBuf == "DEFINITIONS:") { ns = NS_DEFINITIONS; continue;}
      if(taMisc::LexBuf == "CONSTRAINTS:") { ns = NS_CONSTRAINTS; continue;}
      if(taMisc::LexBuf == "NETWORK:") {
	ns = NS_NETWORK;
	// default fully-connected weight configuration
	send_end = recv_end = nunits;
	continue;
      }
      if(taMisc::LexBuf == "BIASES:") { ns = NS_BIASES; continue;}
    }
    else if( ns == NS_DEFINITIONS) {
      if(upcase(taMisc::LexBuf) == "NUNITS") {
	c = taMisc::read_alnum(strm);
	if((c == EOF) || taMisc::LexBuf.empty()){
	  taMisc::Error("Unspecified number of units in OldPDPNet file");
	  return;
	}

	// this is how many units we should have

	nunits = (int) taMisc::LexBuf;
	continue;
      }
      if(upcase(taMisc::LexBuf) == "NINPUTS") {
	c = taMisc::read_alnum(strm);
	if((c == EOF) || taMisc::LexBuf.empty()){
	  taMisc::Error("Unspecified number of inputs in OldPDPNet file");
	  return;
	}

	// this is how many input units we should have

	ninputs = (int) taMisc::LexBuf;
	continue;
      }
      if(upcase(taMisc::LexBuf) == "NOUTPUTS") {
	c = taMisc::read_alnum(strm);
	if((c == EOF) || taMisc::LexBuf.empty()){
	  taMisc::Error("Unspecified number of outputs in OldPDPNet file");
	  return;
	}

	// this is how many output units we should have

	noutputs = (int) taMisc::LexBuf;
	continue;
      }

      if(taMisc::LexBuf == "end") {
	ns = NS_NONE;
	if((ninputs + noutputs) > nunits) {
	  taMisc::Error("ninputs + noutputs > noutputs in OldPDPNet file");
	  return;
	}
	Layer* tlay;
	taLeafItr ti;
	FOR_ITR_EL(Layer, tlay, layers., ti){
	  flat_units.Borrow(tlay->units);
	}
	if(flat_units.leaves > 0) {
	  if(flat_units.leaves != nunits){
	    String nun = (int) nunits;
	      taMisc::Error("Existing network structure does not have ",
			    nun, " units");
	    flat_units.Reset();
	    return;
	  }
	  else { // use existing network;
	    nhiddens = ninputs = noutputs = 0;
	  }
	}
	else {
	  if(ninputs + noutputs != 0)
	    nhiddens = nunits - ninputs - noutputs;
	}
	Layer* input_layer;
	Layer* output_layer;
	Layer* hidden_layer;
	if((ninputs + noutputs + nhiddens == 0) &&
	   (flat_units.leaves == 0) && (nunits > 0)){
	  // we need some units, all in the same layer
	  input_layer = (Layer *) layers.New(1);
	  input_layer->n_units = nunits;
	  input_layer->Build();
	  flat_units.Borrow(input_layer->units);
	  continue;
	}
	if(ninputs > 0) { // user wants an inputs layer, make it the first
	  if(layers.leaves > 0)
	    input_layer = (Layer *) layers.Leaf(0);
	  else {
	    input_layer = (Layer *) layers.New(1);
	    input_layer->name = "Input Layer";
	  }
	  input_layer->n_units = ninputs;
	  input_layer->Build();
	}
	if(nhiddens > 0) { // user wants an hidden layer, make it the second
			  // or first if no input layer
	  if(layers.leaves > (1 - (ninputs == 0)))
	    hidden_layer = (Layer *) layers.Leaf(1 - (ninputs ==0));
	  else {
	    hidden_layer = (Layer *) layers.New(1);
	    hidden_layer->name = "Hidden Layer";
	  }
	  hidden_layer->n_units = nhiddens;
	  hidden_layer->Build();
	}
	if(noutputs > 0) { // user wants an inputs layer, make it the last
			  // but not the hidden layer!
	  if(layers.leaves > ((ninputs != 0) + (nhiddens != 0)))
	    output_layer = (Layer *) layers.Leaf(layers.leaves-1);
	  else {
	    output_layer = (Layer *) layers.New(1);
	    output_layer->name = "Output Layer";
	  }
	  output_layer->n_units = noutputs;
	  output_layer->Build();
	}

	Layer* lay; flat_units.Reset();
	taLeafItr i;
	FOR_ITR_EL(Layer, lay, layers., i){
	  flat_units.Borrow(lay->units);
	}
      }
    }
    else if (ns == NS_NETWORK) {
      if(taMisc::LexBuf == "end") {
	ns = NS_NONE;
	continue;
      }
      if (fc == '%') { // special case
	if(taMisc::LexBuf.length() == 2) // same contype for whole range
	  con_char = taMisc::LexBuf[1];
	c = taMisc::read_alnum_noeol(strm);
        if ((c == EOF) || taMisc::LexBuf.empty()) {
	  taMisc::Error("Improper connection range specification in network file");
	  return;
	}
	recv_start = (int)taMisc::LexBuf;
	c = taMisc::read_alnum_noeol(strm);
        if ((c == EOF) || taMisc::LexBuf.empty()) {
	  taMisc::Error("Improper connection range specification in network file");
	  return;
	}
	recv_end  = recv_start + ( (int) taMisc::LexBuf) -1;
	c = taMisc::read_alnum_noeol(strm);
        if ((c == EOF) ||  taMisc::LexBuf.empty()) {
	  taMisc::Error("Improper connection range specification in network file");
	  return;
	}
	send_start = (int)taMisc::LexBuf;
	c = taMisc::read_alnum_noeol(strm);
        if ((c == EOF) || taMisc::LexBuf.empty()) {
	  taMisc::Error("Improper connection range specification in network file");
	  return;
	}
	send_end  = send_start + ( (int) taMisc::LexBuf) -1;

	if(con_char != '\0') { // same type on con for all of range
	  ConSpec* conspec = GetNSConSpec(con_char,&conspec_group,
					  &prj->specs,conspec_type,skip_dots);
	  if (conspec != NULL) 
	    for(int j=recv_start;j<=recv_end;j++){
	      for(int i =send_start;i<=send_end;i++){
		ConnectUnits(flat_units.Leaf(j),flat_units.Leaf(i),
			     false,conspec);
	      }
	    }
	  con_char = '\0';
	  continue;
	}
        // otherwise prepare to begin a block
	cur_send = send_start;
	cur_recv = recv_start;
	continue;
      }
      // otherwise it must be a line of connections
      // check to make sure that line is send_end - send_start chars in length
      // step cur_send through line for each char

      if((int)taMisc::LexBuf.length() != (send_end - send_start)) {
	String curlen = (int) taMisc::LexBuf.length();
	String expt = send_end - send_start;
	taMisc::Error("Connection Line:", taMisc::LexBuf, " has ",
		      curlen, "sending weights. (expected ", expt, ")");
	return;
      }
      if(cur_recv > recv_end) {
	String cur = cur_recv;
	String expt = recv_end;
	taMisc::Error("Too many connection lines:", cur, 
		      "than expected number:", expt);
	return;
      }
      Unit* recv_unit = flat_units.Leaf(cur_recv);
      Layer* recv_layer = GET_OWNER(recv_unit,Layer);
      for(int i=0;i<(send_end-send_start);i++){
	con_char = taMisc::LexBuf[i];
	ConSpec* conspec = GetNSConSpec(con_char, &conspec_group,
					&prj->specs,conspec_type);
	if(conspec == NULL) continue;
	Unit* send_unit = (Unit *) flat_units.Leaf(i);
	Layer* send_layer = GET_OWNER(send_unit,Layer);
	Projection* pjn = NULL;
	taLeafItr p;
	// check to see if a pjrn already exists
	FOR_ITR_EL(Projection, pjn, recv_layer->projections., p) {
	  if((pjn->from == send_layer) && 
	     (pjn->spec->InheritsFrom(&TA_CustomPrjnSpec)) &&
	     (pjn->con_spec == conspec))
	    break; // ok found one
	}
	if(pjn==NULL) { // no projection
	  pjn = (Projection *) recv_layer->projections.New(1);
	  if(recv_layer == send_layer) // self con
	    pjn->from_type = Projection::SELF;
	  else
	    pjn->from_type = Projection::CUSTOM;

	  pjn->spec.type = &TA_CustomPrjnSpec;
	  pjn->spec.UpdateAfterEdit();
	  pjn->con_spec.SetSpec(conspec);
	  taBase::SetPointer((TAPtr*)&(pjn->from), send_layer);
	  pjn->projected = true;
	  pjn->UpdateAfterEdit();
	}
	pjn->send_idx = pjn->recv_idx = -1; // clear idicies
	// find the connection group on the units
	Con_Group* rgrp = NULL;
	Con_Group* sgrp = NULL;
	int rg,sg;
	FOR_ITR_GP(Con_Group,rgrp,recv_unit->recv.,rg){
	  if(rgrp->prjn == pjn) { pjn->recv_idx = rg; break;}
	}
	FOR_ITR_GP(Con_Group,sgrp,send_unit->send.,sg){
	  if(sgrp->prjn == pjn) { pjn->send_idx = sg; break;}
	}
	recv_unit->ConnectFrom(send_unit,pjn);
	cur_send++;
      }
      con_char = '\0';
      cur_send = send_start;
      cur_recv++;
      continue;
    }
    else if(ns == NS_BIASES) {
      if(taMisc::LexBuf == "end") {ns = NS_NONE;continue;}
    }
    else if (ns == NS_CONSTRAINTS) {
      if(taMisc::LexBuf == "end") {ns = NS_NONE;continue;}
      if(taMisc::LexBuf.length() > 1) { // constriant variable is not a char
	taMisc::Error("Constraint Character:", taMisc::LexBuf,
		      "was not a character");
	return;
      }
      ConSpec* nsc  = (ConSpec *) prj->specs.New(1,conspec_type);
      conspec_group.Link(nsc);
      nsc->name = taMisc::LexBuf;
      nsc->rnd.type = Random::UNIFORM;
      while((c!='\n') && (c != EOF) && !taMisc::LexBuf.empty()){
	c = taMisc::read_alnum_noeol(strm);
	taMisc::LexBuf.upcase();
	if(taMisc::LexBuf == "POSITIVE") {
	  nsc->wt_limits.type = WeightLimits::GT_MIN;
	  nsc->wt_limits.min = 0.0f;
	  continue;
	}
	if(taMisc::LexBuf == "NEGATIVE") {
	  nsc->wt_limits.type = WeightLimits::LT_MAX;
	  nsc->wt_limits.max = 0.0f;
	  continue;
	}
	if(taMisc::LexBuf == "LINKED") {
	  // todo linked
	}
	if(taMisc::LexBuf == "RANDOM") {
	  nsc->rnd.mean = 0;
	  nsc->rnd.var =  0.5;
	  continue;
	}
	// otherwise it must be a number
	nsc->rnd.var = 0.0;
	nsc->rnd.mean = (float) taMisc::LexBuf;
      }
    }
  }
  flat_units.Reset();
  UpdateAfterEdit();
}

MonitorStat* NetView::NewMonitor(SchedProcess* proc, MonLoc loc,
			 String variable, Operator op, bool create_aggs) {
  if((proc == NULL) || (editor == NULL) || (editor->netg == NULL)) return NULL;
  Stat_Group* stat_gp = SchedProcess::Default_StatGroup(&TA_MonitorStat, proc);
  if(loc == LOOP)
    stat_gp = &(proc->loop_stats);
  else if(loc == FINAL)
    stat_gp = &(proc->final_stats);
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    taivMisc::RecordScript
      ("{ MonitorStat* stat = " + stat_gp->GetPath() + ".New(1,MonitorStat);\n\
  stat->variable = \"" + variable + "\";\n\
  stat->net_agg.op = " + String(op) + ";\n");
    int i;
    for(i=0;i<editor->netg->selectgroup.size;i++) {
      taivMisc::RecordScript
	("  stat->AddObject(" + editor->netg->selectgroup.FastEl(i)->GetPath() + ");\n");
    }
    taivMisc::RecordScript("  stat->UpdateAfterEdit();\n}\n");
    if(create_aggs)
      taMisc::Error("NewMonitor: error -- cannot create aggs here in dmem gui mode -- do it separately");
    return NULL;
  }
#endif
  MonitorStat* stat = (MonitorStat*) stat_gp->New(1,&TA_MonitorStat);
  stat->variable = variable;
  stat->net_agg.op = (Aggregate::Operator)op;
  int i;
  for(i=0;i<editor->netg->selectgroup.size;i++) {
    stat->objects.LinkUnique(editor->netg->selectgroup.FastEl(i));
  }
  stat->UpdateAfterEdit();
  if(create_aggs)
    stat->CallFun("CreateAggregates");
  winbMisc::DelayedMenuUpdate(proc);
  return stat;
}

//////////////////////////
//	process.h	//
//////////////////////////

void Process::CutLinks() {
  static bool in_repl = false;
  if(in_repl || (owner == NULL)) return; // already replacing or already dead
  taBase::DelPointer((TAPtr*)&network);
  taBase::DelPointer((TAPtr*)&environment);
  if((taMisc::iv_active) && !taMisc::is_loading) { // for linked sub-process
    ctrl_panel.GetPanel();
    if(ctrl_panel.ctrl_panel != NULL)
      ctrl_panel.ctrl_panel->Cancel();
  }
  // un-set any pointers to this object!
  Project* proj = GET_MY_OWNER(Project);  
  if((proj != NULL) && !proj->deleting) {
    in_repl = true;
    taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)this, NULL);
    in_repl = false;
  }
  taNBase::CutLinks();
}

void Process::SetDefaultPNEPtrs() {
  if(project == NULL)
    project = GET_MY_OWNER(Project);
  if(project == NULL)	return;

  if(network == NULL)
    taBase::SetPointer((TAPtr*)&network, project->networks.DefaultEl());
  if(environment == NULL)
    taBase::SetPointer((TAPtr*)&environment, project->environments.DefaultEl());
}

//////////////////////////
//	sched_proc.h	//
//////////////////////////

void SchedProcess::InitAllLogs() {
  if(project == NULL)
    return;
  taLeafItr i;
  PDPLog* lg;
  FOR_ITR_EL(PDPLog, lg, project->logs., i)
    lg->Clear();
}

void ForkProcess::SetDefaultPNEPtrs() {
  SchedProcess::SetDefaultPNEPtrs();
  if(project == NULL) return;
  if(second_network == NULL)
    taBase::SetPointer((TAPtr*)&second_network, project->networks.DefaultEl());
  if(second_environment == NULL)
    taBase::SetPointer((TAPtr*)&second_environment, project->environments.DefaultEl());
}

void SyncEpochProc::SetDefaultPNEPtrs() {
  EpochProcess::SetDefaultPNEPtrs();
  if(project == NULL) return;
  if(second_network == NULL)
    taBase::SetPointer((TAPtr*)&second_network, project->networks.DefaultEl());
}

//////////////////////////
//	enviro.h	//
//////////////////////////

void Environment::CutLinks() {
  static bool in_repl = false;
  if(in_repl || (owner == NULL)) return; // already replacing or already dead
  Project* prj = (Project *) GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->deleting) {
    Environment* replenv = NULL;
    int nl;
    for(nl=0;nl<prj->environments.leaves;nl++) {
      Environment* nt = (Environment*)prj->environments.Leaf(nl);
      if(nt != this) {
	replenv = nt;
	break;
      }
    }
    // replace any actual process.environment ptrs with another one -- could be useful
    prj->processes.ReplaceEnvPtrs(this, replenv);
    // set any misc environment pointers to NULL!
    in_repl = true;
    taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)this, NULL);
    in_repl = false;
  }
  WinMgr::CutLinks();	// winmgrs cut views first..
  event_specs.CutLinks();
  events.CutLinks();
}

void Environment::DistMatrixGrid(GridLog* disp_log, int pat_no, float_RArray::DistMetric metric,
			     bool norm, float tol)
{
  if(events.leaves == 0)
    return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = "DistMatrixGrid: " + name + " Pattern: " + String(pat_no);
  DataTable* dt = &(disp_log->data);

  dt->Reset();
  dt->NewColString("Event");
  DataTable* dtgp = dt->NewGroupFloat("dists", events.leaves);
  bool first = true;
  int cnt = 0;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    String nm = ev->name;
    if(first) {
      nm = String("<") + String(events.leaves) + ">" + nm;
      first = false;
      String geom = "GEOM_X=" + String(events.leaves);
      dtgp->AddColDispOpt(geom, cnt);
      dtgp->AddColDispOpt("GEOM_Y=1", cnt);
      dtgp->AddColDispOpt("USE_EL_NAMES", cnt); // each column has separate element names in gp
    }
    dtgp->SetColName(nm, cnt++);
  }
  dt->ResetData();

  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    dt->AddStringVal(ev->name, 0); // col 0, subgp -1
    cnt = 0;			// column number counter
    taLeafItr j;
    Event* oev;
    FOR_ITR_EL(Event, oev, events., j) {
      Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
      dt->AddFloatVal(dst, cnt, 0); // col cnt, subgp 0
      cnt++;
    }
  }

  disp_log->ViewAllData();
}

void Environment::CmpDistMatrixGrid(GridLog* disp_log, int pat_no, Environment* cmp_env,
				    int cmp_pat_no, float_RArray::DistMetric metric,
				    bool norm, float tol)
{
  if((events.leaves == 0) || (cmp_env == NULL) || (cmp_env->events.leaves == 0))
    return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = "CmpDistMatrixGrid: " + name + " (pat: " + String(pat_no) + ") vs "
    + cmp_env->name + " (pat: " + String(cmp_pat_no) + ")";
  DataTable* dt = &(disp_log->data);

  dt->Reset();
  dt->NewColString("Event");
  DataTable* dtgp = dt->NewGroupFloat("dists", cmp_env->events.leaves);
  bool first = true;
  int cnt = 0;
  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, cmp_env->events., i) {
    String nm = ev->name;
    if(first) {
      nm = String("<") + String(cmp_env->events.leaves) + ">" + nm;
      first = false;
      String geom = "GEOM_X=" + String(cmp_env->events.leaves);
      dtgp->AddColDispOpt(geom, cnt);
      dtgp->AddColDispOpt("GEOM_Y=1", cnt);
      dtgp->AddColDispOpt("USE_EL_NAMES", cnt); // each column has separate element names in gp
    }
    dtgp->SetColName(nm, cnt++);
  }
  dt->ResetData();

  FOR_ITR_EL(Event, ev, events., i) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    dt->AddStringVal(ev->name, 0); // col 0, subgp -1
    cnt = 0;			// column number counter
    taLeafItr j;
    Event* oev;
    FOR_ITR_EL(Event, oev, cmp_env->events., j) {
      Pattern* pat = (Pattern*)oev->patterns.Leaf(pat_no);
      if(pat == NULL) {
	taMisc::Error("*** Pattern number:", String(pat_no), "not found");
	return;
      }
      float dst = trg_pat->value.Dist(pat->value, metric, norm, tol);
      dt->AddFloatVal(dst, cnt, 0); // col cnt, subgp 0
      cnt++;
    }
  }

  disp_log->ViewAllData();
}

void Environment::ClusterPlot(GraphLog* disp_log, int pat_no,
			      float_RArray::DistMetric metric,
			      bool norm, float tol)
{
  if(disp_log == NULL) {
    disp_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  // initialize with leaves
  ClustNode root;
  taLeafItr ei;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., ei) {
    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }
    ClustNode* nd = new ClustNode;
    nd->name = ev->name;
    nd->SetPat(&(trg_pat->value));
    root.AddChild(nd);
  }

  root.Cluster(metric, norm, tol);
  
  disp_log->name = "ClusterPlot: " + name + " Pattern: " + String(pat_no);
  root.GraphData(&(disp_log->data));
  disp_log->ViewAllData();
  disp_log->InitAllViews();
}  

void Environment::CorrelMatrixGrid(GridLog* disp_log, int pat_no) {
  if(events.leaves == 0)
    return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = "CorrelMatrixGrid: " + name + " Pattern: " + String(pat_no);
  DataTable* dt = &(disp_log->data);

  int dim;
  float_RArray correl_mat;
  CorrelMatrix(correl_mat, pat_no, dim);

  dt->Reset();
  dt->NewColString("val");
  DataTable* dtgp = dt->NewGroupFloat("correls", dim);

  bool first = true;
  for(int i=0;i<dim;i++) {
    String nm = String("v") + String(i);
    if(first) {
      nm = String("<") + String(dim) + ">" + nm;
      first = false;
      String geom = "GEOM_X=" + String(dim);
      dtgp->AddColDispOpt(geom, i);
      dtgp->AddColDispOpt("GEOM_Y=1", i);
      dtgp->AddColDispOpt("USE_EL_NAMES", i); // each column has separate element names in gp
    }
    dtgp->SetColName(nm, i);
  }
  dt->ResetData();

  for(int i=0;i<dim;i++) {
    String nm = String("v") + String(i);
    dt->AddStringVal(nm, 0);
    for(int j=0;j<dim;j++) {
      float val = correl_mat.FastTriMatEl(dim, i, j);
      dt->AddFloatVal(val, i, 0); // col cnt, subgp 0
    }
  }

  disp_log->ViewAllData();
}

void Environment::PCAEigenGrid(GridLog* disp_log, int pat_no, bool print_eigens) {
  if(event_specs.size == 0) return;
  PatternSpec* ps = (PatternSpec*)((EventSpec*)event_specs[0])->patterns[pat_no];
  if(ps == NULL) return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    disp_log->Clear();
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = (String)"PCAEigenGrid: " + name  + " Pattern: " + String(pat_no);

  DataTable* md = &(disp_log->data);
  md->RemoveAll();

  md->NewColString("row");

  int ptx = ps->geom.x;
  int pty = ps->geom.y;
  
  int dim;			// dimensionality
  float_RArray evecs;		// eigen vectors
  float_RArray evals;		// eigen values
  PCAEigens(evecs, evals, pat_no, dim);

  if(print_eigens) {
    cerr << "full list of eigen values (sorted lowest-to-highest, reverse of component indicies): " << endl;
    evals.List(cerr);		// these are sorted in ascending order
    cerr << endl;
  }

  TwoDCoord outerg;		// outer-grid geom
  outerg.FitN(dim);		// just find best fit
  int evx = outerg.x; int evy = outerg.y; // evx,y from EnvToGrid

  // these are the columns
  int i;
  for(i=0; i<evx; i++) {
    md->NewGroupFloat((String)"c" + (String)i, dim);
    md->AddColDispOpt(String("GEOM_X=") + String(ptx), 0, i); // column 0, subgp i
    md->AddColDispOpt(String("GEOM_Y=") + String(pty), 0, i); // column 0, subgp i
  }

  md->ResetData();

  for(i=0;i<evy;i++) {
    md->AddBlankRow();		// add the rows
    md->SetStringVal(String(evy - i -1), 0, i);
  }

  float_RArray evec;
  
  int uni = 0;
  int ux, uy;
  for(uy=0;uy<evy; uy++) {
    if(uni >= dim)
      break;
    for(ux=0; ux<evx; ux++, uni++) {
      if(uni >= dim)
	break;
      
      evecs.GetMatCol(dim, evec, dim-uni-1); // get eigen vector = column of correl_matrix 
      DataTable* dt = (DataTable*)md->gp.FastEl(ux);
      dt->PutArrayToRow(evec, evy-uy-1);
    }
  }
  
  disp_log->ViewAllData();
}

void Environment::PCAPrjnPlot(GraphLog* disp_log, int pat_no, int x_axis_c, int y_axis_c, bool print_eigens) {
  if(disp_log == NULL) {
    disp_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  int dim;			// dimensionality
  float_RArray evecs;		// eigen vectors
  float_RArray evals;		// eigen values
  PCAEigens(evecs, evals, pat_no, dim);

  if((x_axis_c < 0) || (x_axis_c >= dim)) {
    taMisc::Error("*** PCA: x_axis component must be between 0 and",String(dim-1));
    return;
  }
  if((y_axis_c < 0) || (y_axis_c >= dim)) {
    taMisc::Error("*** PCA: y_axis component must be between 0 and",String(dim-1));
    return;
  }

  disp_log->name = "PCA: " + name + " Pattern: " + String(pat_no)
    + " X=" + String(x_axis_c) + ", Y=" + String(y_axis_c);

  if(print_eigens) {
    cerr << "full list of eigen values (sorted lowest-to-highest, reverse of component indicies): " << endl;
    evals.List(cerr);		// these are sorted in ascending order
    cerr << endl;
  }

  int x_axis_rev = dim - 1 - x_axis_c; // reverse-order indicies, for accessing data structs
  int y_axis_rev = dim - 1 - y_axis_c;

  float_RArray xevec;		// x eigen vector
  evecs.GetMatCol(dim, xevec, x_axis_rev); // get eigen vector = column of correl_matrix 
  if(print_eigens) {
    cerr << "Component no: " << x_axis_c << " has eigenvalue: " << evals[x_axis_rev] << endl;
  }

  float_RArray yevec;		// x eigen vector
  evecs.GetMatCol(dim, yevec, y_axis_rev); // get eigen vector = column of correl_matrix 
  if(print_eigens) {
    cerr << "Component no: " << y_axis_c << " has eigenvalue: " << evals[y_axis_rev] << endl;
  }

  float_RArray xprjn;
  float_RArray yprjn;

  ProjectPatterns(xevec, xprjn, pat_no);
  ProjectPatterns(yevec, yprjn, pat_no);

  DataTable* dt = &(disp_log->data);
  dt->Reset();
  dt->NewColFloat(String("X = ") + String(x_axis_c));
  dt->NewColFloat(String("Y = ") + String(y_axis_c));
  dt->AddColDispOpt("NEGATIVE_DRAW",1);
  dt->NewColString("Event");
  dt->AddColDispOpt("DISP_STRING", 2);
  dt->AddColDispOpt("AXIS=1", 2); // labels use same axis as y values
  dt->PutArrayToCol(xprjn, 0);
  dt->PutArrayToCol(yprjn, 1);

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    dt->AddStringVal(ev->name, 2);
  }
  disp_log->ViewAllData();
  disp_log->InitAllViews();
}  

void Environment::MDSPrjnPlot(GraphLog* disp_log, int pat_no, int x_axis_c, int y_axis_c,
			      float_RArray::DistMetric metric, bool norm, float tol,
			      bool print_eigens)
{
  if(disp_log == NULL) {
    disp_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  int dim = events.leaves;

  if((x_axis_c < 0) || (x_axis_c >= dim)) {
    taMisc::Error("*** MDSPrjnPlot: x_axis component must be between 0 and",String(dim-1));
    return;
  }
  if((y_axis_c < 0) || (y_axis_c >= dim)) {
    taMisc::Error("*** MDSPrjnPlot: y_axis component must be between 0 and",String(dim-1));
    return;
  }

  disp_log->name = "MDS: " + name + " Pattern: " + String(pat_no)
    + " X=" + String(x_axis_c) + ", Y=" + String(y_axis_c);

  float_RArray dist_ary;	// distance array (tri mat format)
  DistArray(dist_ary, pat_no, metric, norm, tol);
  float_RArray dist_mat;
  dist_mat.CopyFmTriMat(dim, dist_ary);

  float_RArray xcoords;
  float_RArray ycoords;
  dist_mat.MDS(dim, xcoords, ycoords, x_axis_c, y_axis_c, print_eigens);

  DataTable* dt = &(disp_log->data);
  dt->Reset();
  dt->NewColFloat(String("X = ") + String(x_axis_c));
  dt->NewColFloat(String("Y = ") + String(y_axis_c));
  dt->AddColDispOpt("NEGATIVE_DRAW",1);
  dt->NewColString("Event");
  dt->AddColDispOpt("DISP_STRING", 2);
  dt->AddColDispOpt("AXIS=1", 2); // labels use same axis as y values
  dt->PutArrayToCol(xcoords, 0);
  dt->PutArrayToCol(ycoords, 1);

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    dt->AddStringVal(ev->name, 2);
  }
  disp_log->ViewAllData();
  disp_log->InitAllViews();
}  

void Environment::EventPrjnPlot(Event* x_axis_e, Event* y_axis_e, int pat_no, GraphLog* disp_log, 
				float_RArray::DistMetric metric, bool norm, float tol)
{
  if(disp_log == NULL) {
    disp_log = (GraphLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GraphLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  if((x_axis_e == NULL) || (y_axis_e == NULL)) return;
  if(x_axis_e->InheritsFrom(TA_Event_MGroup))
    x_axis_e = (Event*)((Event_MGroup*)x_axis_e)->SafeEl(0);
  if(y_axis_e->InheritsFrom(TA_Event_MGroup))
    y_axis_e = (Event*)((Event_MGroup*)y_axis_e)->SafeEl(0);
  if((x_axis_e == NULL) || (y_axis_e == NULL)) return;
  if(!x_axis_e->InheritsFrom(TA_Event) || !y_axis_e->InheritsFrom(TA_Event)) return;

  Pattern* x_axis_pat = (Pattern*)x_axis_e->patterns.Leaf(pat_no);
  if(x_axis_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return;
  }
  Pattern* y_axis_pat = (Pattern*)y_axis_e->patterns.Leaf(pat_no);
  if(y_axis_pat == NULL) {
    taMisc::Error("*** Pattern number:", String(pat_no), "not found");
    return;
  }

  disp_log->name = "EventPrjn: " + name + " Pattern: " + String(pat_no)
    + " X=" + x_axis_e->name + ", Y=" + y_axis_e->name;

  DataTable* dt = &(disp_log->data);
  dt->Reset();
  dt->NewColFloat(String("X = ") + x_axis_e->name);
  dt->NewColFloat(String("Y = ") + y_axis_e->name);
  dt->AddColDispOpt("NEGATIVE_DRAW",1);
  dt->NewColString("Event");
  dt->AddColDispOpt("DISP_STRING", 2);
  dt->AddColDispOpt("AXIS=1", 2); // labels use same axis as y values

  taLeafItr i;
  Event* ev;
  FOR_ITR_EL(Event, ev, events., i) {
    dt->AddStringVal(ev->name, 2);

    Pattern* trg_pat = (Pattern*)ev->patterns.Leaf(pat_no);
    if(trg_pat == NULL) {
      taMisc::Error("*** Pattern number:", String(pat_no), "not found");
      return;
    }

    float xc = trg_pat->value.Dist(x_axis_pat->value, metric, norm, tol);
    float yc = trg_pat->value.Dist(y_axis_pat->value, metric, norm, tol);

    dt->AddFloatVal(xc, 0);
    dt->AddFloatVal(yc, 1);
  }
  disp_log->ViewAllData();
  disp_log->InitAllViews();
}  

void Environment::EnvToGrid(GridLog* disp_log, int pat_no, int ev_x, int ev_y, int pt_x, int pt_y) {
  if(event_specs.size == 0) return;
  PatternSpec* ps = (PatternSpec*)((EventSpec*)event_specs.DefaultEl())->patterns[pat_no];
  if(ps == NULL) return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    disp_log->Clear();
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = (String)"EnvToGrid: " + name  + " Pattern: " + String(pat_no);

  DataTable* md = &(disp_log->data);
  md->RemoveAll();
  
  md->NewColString("row");

  int evx = ev_x;
  int evy = ev_y;
  if(evx < 0) {
    evx = (int)sqrtf((float)events.leaves);
  }
  if((evy < 0) || (evx * evy < events.leaves)) {
    evy = events.leaves / evx;
    while((evx * evy) < events.leaves)
      evy++;
  }

  int ptx = (pt_x < 0) ? ps->geom.x : pt_x;
  int pty = (pt_y < 0) ? ps->geom.y : pt_y;
  int totn = ptx * pty;

  // these are the columns
  int i;
  for(i=0; i<evx; i++) {
    md->NewGroupFloat((String)"c" + (String)i, totn);
    md->AddColDispOpt(String("GEOM_X=") + String(ptx), 0, i); // column 0, subgp i
    md->AddColDispOpt(String("GEOM_Y=") + String(pty), 0, i); // column 0, subgp i
  }

  md->ResetData();

  for(i=0;i<evy;i++) {
    md->AddBlankRow();		// add the rows
    md->SetStringVal(String(evy - i -1), 0, i);
  }

  int uni = 0;
  int ux, uy;
  for(uy=0;uy<evy; uy++) {
    if(uni >= events.leaves)
      break;
    for(ux=0; ux<evx; ux++, uni++) {
      if(uni >= events.leaves)
	break;
      Pattern* pat = (Pattern*)((Event*)events.Leaf(uni))->patterns[pat_no];
      DataTable* dt = (DataTable*)md->gp.FastEl(ux);
      dt->PutArrayToRow(pat->value, evy-uy-1);
    }
  }
  
  disp_log->ViewAllData();
}

void Environment::PatFreqGrid(GridLog* disp_log, float act_thresh, bool prop) {
  if(events.leaves == 0)
    return;

  if(event_specs.size == 0) return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = "PatFreqGrid: " + name;
  DataTable* dt = &(disp_log->data);
  dt->Reset();

  EventSpec* es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  float_RArray freqs;
  int pat_no;
  for(pat_no = 0; pat_no < es->patterns.size; pat_no++) {
    PatternSpec* ps = (PatternSpec*)es->patterns[pat_no];
    PatFreqArray(freqs, pat_no, act_thresh, prop);

    DataTable* dtgp = dt->NewGroupFloat(String("pattern_") + String(pat_no), freqs.size);

    bool first = true;
    for(int i=0;i<freqs.size;i++) {
      String nm;
      if(!ps->value_names[i].empty())
	nm = ps->value_names[i];
      else
	nm = String("v") + String(i);
      if(first) {
	nm = String("<") + String(ps->geom.x) + ">" + nm;
	first = false;
	String xgeom = "GEOM_X=" + String(ps->geom.x);
	dtgp->AddColDispOpt(xgeom, i);
	String ygeom = "GEOM_Y=" + String(ps->geom.y);
	dtgp->AddColDispOpt(ygeom, i);
	dtgp->AddColDispOpt("USE_EL_NAMES", i); // each column has separate element names in gp
      }
      dtgp->SetColName(nm, i);
    }

    for(int i=0;i<freqs.size;i++) {
      dtgp->AddFloatVal(freqs[i], i); // col cnt, subgp 0
    }
  }

  disp_log->ViewAllData();
  GridLogView* glv = (GridLogView*)disp_log->views.SafeEl(0);
  if(glv == NULL) return;
  glv->auto_scale = true;
  glv->AllBlockTextOn();
  glv->SetBlockSizes(20,1);
  glv->UpdateGridLayout();
}

void Environment::PatAggGrid(GridLog* disp_log, Aggregate& agg) {
  if(events.leaves == 0)
    return;

  if(event_specs.size == 0) return;

  if(disp_log == NULL) {
    disp_log = (GridLog*) pdpMisc::GetNewLog(GET_MY_OWNER(Project), &TA_GridLog);
    if(disp_log == NULL) return;
  }
  else {
    LogView* lv = (LogView*)disp_log->views.SafeEl(0);
    if((lv == NULL) || !lv->display_toggle || !lv->IsMapped())
      return;
  }

  disp_log->name = "PatAggGrid: " + name;
  DataTable* dt = &(disp_log->data);
  dt->Reset();

  EventSpec* es = (EventSpec*)event_specs.DefaultEl();
  if(es == NULL) return;
  float_RArray freqs;
  int pat_no;
  for(pat_no = 0; pat_no < es->patterns.size; pat_no++) {
    PatternSpec* ps = (PatternSpec*)es->patterns[pat_no];
    PatAggArray(freqs, pat_no, agg);

    DataTable* dtgp = dt->NewGroupFloat(String("pattern_") + String(pat_no), freqs.size);

    bool first = true;
    for(int i=0;i<freqs.size;i++) {
      String nm;
      if(!ps->value_names[i].empty())
	nm = ps->value_names[i];
      else
	nm = String("v") + String(i);
      if(first) {
	nm = String("<") + String(ps->geom.x) + ">" + nm;
	first = false;
	String xgeom = "GEOM_X=" + String(ps->geom.x);
	dtgp->AddColDispOpt(xgeom, i);
	String ygeom = "GEOM_Y=" + String(ps->geom.y);
	dtgp->AddColDispOpt(ygeom, i);
	dtgp->AddColDispOpt("USE_EL_NAMES", i); // each column has separate element names in gp
      }
      dtgp->SetColName(nm, i);
    }

    for(int i=0;i<freqs.size;i++) {
      dtgp->AddFloatVal(freqs[i], i); // col cnt, subgp 0
    }
  }

  disp_log->ViewAllData();
  GridLogView* glv = (GridLogView*)disp_log->views.SafeEl(0);
  if(glv == NULL) return;
  glv->auto_scale = true;
  glv->AllBlockTextOn();
  glv->SetBlockSizes(20,1);
  glv->UpdateGridLayout();
}

//////////////////////////
//	net_iv.h	//
//////////////////////////

Process_MGroup* NetEditor::GetProcessGroup() {
  Project* proj = GET_OWNER(owner, Project);  
  if(proj == NULL)    return NULL;
  return &(proj->processes);
}

//////////////////////////
//	pdplog.h	//
//////////////////////////


void PDPLog::CutLinks() {
  static bool in_repl = false;
  if(in_repl || (owner == NULL)) return; // already replacing or already dead
  cur_proc = NULL;
  RemoveAllUpdaters();
  // set any misc log pointers to NULL!
  Project* proj = GET_MY_OWNER(Project);  
  if((proj != NULL) && !proj->deleting) {
    in_repl = true;
    taMisc::ReplaceAllPtrs(GetTypeDef(), (void*)this, NULL);
    in_repl = false;
  }
  WinMgr::CutLinks();
}

void NetLogView::CutLinks() {
  Project* proj = GET_MY_OWNER(Project);  
  if((proj != NULL) && !proj->deleting)
    RemoveLabels();
  LogView::CutLinks();
  taBase::DelPointer((TAPtr*)&network);
}


//////////////////////////////////////////
//	PDPRoot: structural root	//
//////////////////////////////////////////

#include <ta/enter_iv.h>
declareActionCallback(PDPRoot)
implementActionCallback(PDPRoot)
#include <ta/leave_iv.h>

String PDPRoot::version_no = "3.2";

void PDPRoot::Initialize() {
  name = "root";
  projects.SetBaseType(&TA_Project);
  colorspecs.SetBaseType(&TA_ColorScaleSpec);
}

void PDPRoot::InitLinks() {
  taBase::Own(projects, this);
  taBase::Own(colorspecs, this);
  WinBase::InitLinks();
}

void PDPRoot::Settings() {
  if(taMisc::iv_active)
    TA_taMisc.ive->Edit(TA_taMisc.GetInstance(),window);
}

void PDPRoot::SaveConfig() {
  if(projects.leaves > 0) {
    taMisc::Error("Error: You cannot save the ~/.pdpconfig file with a project file loaded -- remove the project(s) then try again!");
    return;
  }
  String home_dir = getenv("HOME");
  String cfgfn = home_dir + "/.pdpconfig";
  fstream strm;
  strm.open(cfgfn, ios::out);
  Save(strm);
  strm.close(); strm.clear();
  ((taMisc*)TA_taMisc.GetInstance())->SaveConfig();
}

void PDPRoot::LoadConfig() {
  String home_dir = getenv("HOME");
  String cfgfn = home_dir + "/.pdpconfig";
  fstream strm;
  strm.open(cfgfn, ios::in);
  if(!strm.bad() && !strm.eof())
    Load(strm);
  strm.close(); strm.clear();
  ((taMisc*)TA_taMisc.GetInstance())->LoadConfig();
}

TAPtr PDPRoot::Browse(const char* init_path) {
  if(!taMisc::iv_active) return NULL;
  
  TAPtr iob = this;
  if(init_path != NULL) {
    String ip = init_path;
    iob = FindFromPath(ip);
    if(iob == NULL) iob = this;
  }

  taivObjChooser* chs = new taivObjChooser(iob, taivM->wkit, taivM->wkit->style(),
					   "Browse for Object", false);
  chs->Choose();
  TAPtr retv = chs->sel_obj;
  delete chs;
  return retv;
}

void PDPRoot::Info() {
  String info = "PDP Info\n";
  info += "This is the PDP++ software package, version: ";
  info += version_no + "\n\n";
  info += "Mailing List:       http://www.cnbc.cmu.edu/PDP++/pdp-discuss.html\n";
  info += "Bug Reports:        pdp++bugreport@cnbc.cmu.edu\n";
  info += "WWW Page:           http://www.cnbc.cmu.edu/PDP++/PDP++.html\n";
  info += "Anonymous FTP Site: ftp://grey.colorado.edu/pub/oreilly/pdp++/\n";
  info += "                    or ftp://cnbc.cmu.edu/pub/pdp++/\n";
  info += "\n\n";
    
  info += "Copyright (C) 1995-2001 Randall C. O'Reilly, Chadley K. Dawson,\n\
                    James L. McClelland, and Carnegie Mellon University\n\
 \n\
Permission to use, copy, and modify this software and its documentation\n\
for any purpose other than distribution-for-profit is hereby granted\n\
without fee, provided that the above copyright notice and this permission\n\
notice appear in all copies of the software and related documentation\n\
 \n\
Permission to distribute the software or modified or extended versions\n\
thereof on a not-for-profit basis is explicitly granted, under the\n\
above conditions. HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE\n\
OR MODIFIED OR EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED\n\
EXCEPT BY PRIOR ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS\n\
 \n\
Note that the taString class, which is derived from the GNU String class\n\
is Copyright (C) 1988 Free Software Foundation, written by Doug Lea, and\n\
is covered by the GNU General Public License, see ta_string.h\n\
The iv_graphic library and some iv_misc classes were derived from the\n\
InterViews morpher example and other InterViews code, which is\n\
Copyright (C) 1987, 1988, 1989, 1990, 1991 Stanford University\n\
Copyright (C) 1991 Silicon Graphics, Inc\n\
 \n\
THE SOFTWARE IS PROVIDED 'AS-IS' AND WITHOUT WARRANTY OF ANY KIND\n\
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY\n\
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE\n\
\n\
IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY\n\
SPECIAL INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,\n\
OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,\n\
WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY\n\
OF LIABILITY ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE\n\
OF THIS SOFTWARE";

  taMisc::Choice(info, "Ok");
}

void PDPRoot::Quit() {
  projects.RemoveAll();
  cssMisc::Top->ExitShell();	// just exit from top-level shell
}

void PDPRoot::Destroy() { 
  projects.RemoveAll();
  colorspecs.RemoveAll();
  if(taMisc::iv_active) {
    if(ta_file != NULL)
      taRefN::unRefDone(ta_file);
    ta_file = NULL;
    int i;
    for(i=0; i<10; i++) {
      cssivSession::RunPending();
      if(cssivSession::WaitProc != NULL) 	// then its safe to do some work
	(*cssivSession::WaitProc)();
    }
    CloseWindow();
    cssivSession::Quit();	    // then quit..
  }
  taMisc::types.RemoveAll();	// get rid of all the types before global dtor!
}


void PDPRoot::SetWinName() {
  if(!taMisc::iv_active) return;
  if(window == NULL)    return;
  //  String nw_name = String(ivSession::instance()->classname()) + ": Root";
  // if we give it a common name, windows will be grouped together under KDE!
  String nw_name = String(ivSession::instance()->classname());
  if(nw_name == win_name) return;
  win_name = nw_name;
  if(window->style() == NULL)
    window->style(new ivStyle(wkit->style()));
  window->style()->attribute("title", win_name);
  window->style()->attribute("name", win_name);
  window->style()->attribute("iconName", win_name);
#ifndef CYGWIN
  if(window->is_mapped()) {
    ivManagedWindowRep* mwRep = window->rep();
    mwRep->do_set(window, &ivManagedWindowRep::set_name);
    mwRep->do_set(window, &ivManagedWindowRep::set_icon_name);
  }
#endif
}

class PDPRootWMDeleteHandler : public ivHandler {
public:
  PDPRoot*	rtobj;
  osboolean event(ivEvent& ev);
  PDPRootWMDeleteHandler(PDPRoot* obj);
};

PDPRootWMDeleteHandler::PDPRootWMDeleteHandler(PDPRoot* obj) : ivHandler() {
  rtobj = obj;
}

osboolean PDPRootWMDeleteHandler::event(ivEvent&) {
  int chs = taMisc::Choice("Ok to quit PDP++ and lose all unsaved changes?", "Quit", "Save Then Quit", "Cancel");
  if(chs == 2)
    return true;
  else if(chs == 1) {
    taLeafItr i;
    Project* pr;
    FOR_ITR_EL(Project, pr, rtobj->projects., i) {
      taivGetFile* taf = pr->GetFileDlg();
      ostream* strm = taf->Save();
      if((strm != NULL) && strm->good()) {
	taivMisc::RecordScript(pr->GetPath() + ".Save(" + taf->fname + ");\n");
	pr->SetFileName(taf->fname);
	DMEM_GUI_RUN_IF {
	  pr->Save(*strm);
	}
      }
    }
  }
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    taivMisc::RecordScript(".Quit();\n");
  }
  else
#endif
    {
    rtobj->Quit();
  }
  return true;
}

void PDPRoot::GetWindow() {
  IconGlyph* ig = 
    new IconGlyph(new ivBackground(win_box, wkit->background()),NULL,this);
  ig->SetIconify = winbMisc::SetIconify;
  ig->ScriptIconify= winbMisc::ScriptIconify;
  window = new ivApplicationWindow(ig);
  ivHandler* delh = new PDPRootWMDeleteHandler(this);
  window->wm_delete(delh);
  ig->SetWindow(window);
}

bool PDPRoot::ThisMenuFilter(MethodDef* md) {
  if((md->name == "Load") || (md->name == "Save") || (md->name == "SaveAs") ||
     (md->name == "Close") || (md->name == "Print") || (md->name == "Print_Data") ||
     (md->name == "GetAllWinPos") || (md->name == "ScriptAllWinPos") ||
     (md->name == "SetWinPos") || (md->name == "CopyFrom") || (md->name == "CopyTo") ||
     (md->name == "DuplicateMe") || (md->name == "ChangeMyType") ||
     (md->name == "SelectForEdit") || (md->name == "SelectFunForEdit"))
    return false;
  return true;
}

void PDPRoot::SetWinPos(float left, float bottom, float width, float height) {
  WinGeometry* wn_pos = &win_pos;
  if(projects.leaves > 0)
    wn_pos = &(((Project*)projects.Leaf(0))->root_win_pos);
    
  bool was_set = false;
  if((left != 0) && (bottom != 0)) {
    wn_pos->lft = left;
    wn_pos->bot = bottom;
    was_set = true;
  }
  if((width != 0) && (height != 0)) {
    wn_pos->wd = width;
    wn_pos->ht = height;
    was_set = true;
  }
  if(was_set) {
    wn_pos->UpdateAfterEdit();	// puts it on the update_winpos list
  }
  else {
    wn_pos->SetWinPos();	// does it now
  }
}

void PDPRoot::GetWinPos() {
  WinGeometry* wn_pos = &win_pos;
  if(projects.leaves > 0)
    wn_pos = &(((Project*)projects.Leaf(0))->root_win_pos);
  wn_pos->GetWinPos();
}

//////////////////////////////////////////
//	Project: pdp projects		//
//////////////////////////////////////////

int TypeDefault_MGroup::Dump_Load_Value(istream& strm, TAPtr par) {
  Reset();			// get rid of any existing defaults before loading
  return PDPMGroup::Dump_Load_Value(strm, par);
}

void Wizard_MGroup::AutoEdit() {
  taLeafItr i;
  Wizard* wz;
  FOR_ITR_EL(Wizard, wz, this->, i) {
    if(wz->auto_open)
      wz->Edit();
  }
}

void SelectEdit_MGroup::AutoEdit() {
  taLeafItr i;
  SelectEdit* se;
  FOR_ITR_EL(SelectEdit, se, this->, i) {
    if(se->config.auto_edit)
      se->Edit();
  }
}

void ProjViewState::Initialize() {
  in_use = false;
  iconify = false;
  draw_links = false;
  selected = false;
}

void ProjViewState_List::UnSetInUse() {
  int i;
  for(i=0;i<size;i++)
    FastEl(i)->in_use = false;
}

void ProjViewState_List::RemoveNotInUse() {
  int i;
  for(i=size-1;i>=0;i--) 
    if(!FastEl(i)->in_use) Remove(i);
}

void ProjViewState_List::UnSelectAll() {
  int i;
  for(i=0;i<size;i++)
    FastEl(i)->selected = false;
}


void Project::Initialize() {
  defaults.SetBaseType(&TA_TypeDefault);
  wizards.SetBaseType(&TA_Wizard);
  wizards.el_typ = pdpMisc::def_wizard;
  specs.SetBaseType(&TA_BaseSpec);
  networks.SetBaseType(&TA_Network);
  environments.SetBaseType(&TA_Environment);
  processes.SetBaseType(&TA_SchedProcess);
  logs.SetBaseType(&TA_TextLog);
  scripts.SetBaseType(&TA_Script);
  edits.SetBaseType(&TA_SelectEdit);

  save_rmv_units = false;
  use_sim_log = true;

  editor = NULL;
  obj_width = 11;
  view_colors.SetBaseType(&TA_RGBA);
  the_colors.SetBaseType(&TA_TAColor);
  view_states.SetBaseType(&TA_ProjViewState);
  mnu_updating = false;
  deleting = false;
}

void Project::InitLinks() {
  taBase::Own(defaults, this);
  taBase::Own(wizards, this);
  taBase::Own(specs, this);
  taBase::Own(networks, this);
  taBase::Own(environments, this);
  taBase::Own(processes, this);
  taBase::Own(logs, this);
  taBase::Own(scripts, this);
  taBase::Own(edits, this);
  taBase::Own(root_win_pos, pdpMisc::root); // owned by root..

  taBase::Own(view_font, this);
  taBase::Own(view_colors, this);
  taBase::Own(the_colors, this);
  taBase::Own(view_states, this);

  // need a small font!
#ifndef CYGWIN
  view_font.SetFont("*-Helvetica-medium-r*8*");
#else
  view_font.SetFont("*Arial*medium*--9*");
#endif

  if(taMisc::is_loading)
    view_colors.Reset();		// kill existing colors
  else
    GetDefaultColors();

  LoadDefaults();

  if(!taMisc::is_loading) {
    MakeDefaultWiz(true);	// make default and edit it
  }
  else {
    MakeDefaultWiz(false);	// make default and don't edit it
  }

  WinBase::InitLinks();
}

void Project::GetDefaultColors() {
  view_colors.EnforceSize(COLOR_COUNT);
  view_colors[TEXT]->name = "black";
  view_colors[TEXT]->desc = "Text";
  view_colors[BACKGROUND]->r = .752941;
  view_colors[BACKGROUND]->g = .768627;
  view_colors[BACKGROUND]->b = .827451;
  view_colors[BACKGROUND]->desc = "Background";
  view_colors[NETWORK]->name = "VioletRed1";
  view_colors[NETWORK]->desc = "Network";
  view_colors[ENVIRONMENT]->name = "DarkOliveGreen3";
  view_colors[ENVIRONMENT]->desc = "Env";
  view_colors[SCHED_PROC]->name = "yellow";
  view_colors[SCHED_PROC]->desc = "SchedProc";
  view_colors[STAT_GROUP]->name = "LightSteelBlue2";
  view_colors[STAT_GROUP]->desc = "StatGroup";
  view_colors[SUBPROC_GROUP]->name = "wheat";
  view_colors[SUBPROC_GROUP]->desc = "SubProcGroup";
  view_colors[STAT_PROC]->name = "SlateBlue1";
  view_colors[STAT_PROC]->desc = "Stat";
  view_colors[OTHER_PROC]->name = "gold";
  view_colors[OTHER_PROC]->desc = "Process";
  view_colors[PDPLOG]->name = "burlywood2";
  view_colors[PDPLOG]->desc = "Log";
  view_colors[STAT_AGG]->name = "aquamarine";
  view_colors[STAT_AGG]->desc = "Agg Highlite";
  view_colors[GEN_GROUP]->name = "grey64";
  view_colors[GEN_GROUP]->desc = "Group";
  view_colors[INACTIVE]->name = "grey75";
  view_colors[INACTIVE]->desc = "Inactive";
  view_colors[STOP_CRIT]->name = "red";
  view_colors[STOP_CRIT]->desc = "Stopping Stat";
  view_colors[AGG_STAT]->name = "SlateBlue3";
  view_colors[AGG_STAT]->desc = "Agging Stat";

  view_colors[CON_SPEC]->name = "SpringGreen";
  view_colors[CON_SPEC]->desc = "ConSpec";
  view_colors[UNIT_SPEC]->name = "violet";
  view_colors[UNIT_SPEC]->desc = "UnitSpec";
  view_colors[PRJN_SPEC]->name = "orange";
  view_colors[PRJN_SPEC]->desc = "PrjnSpec";
  view_colors[LAYER_SPEC]->name = "MediumPurple1";
  view_colors[LAYER_SPEC]->desc = "LayerSpec";
  view_colors[WIZARD]->name = "azure";
  view_colors[WIZARD]->desc = "Wizard";
  
  UpdateColors();
}

void Project::UpdateColors() {
  if(view_colors.size != COLOR_COUNT) {
    view_colors.Reset();
    GetDefaultColors();
  }
  the_colors.EnforceSize(COLOR_COUNT);
  if(taMisc::iv_active) {
    int i;
    for(i=0;i<COLOR_COUNT;i++) {
      view_colors[i]->UpdateAfterEdit();
      the_colors[i]->SetColor(view_colors[i]);
    }
  }
}

const ivColor* Project::GetObjColor(TypeDef* td) {
  if(view_colors.size != COLOR_COUNT) {
    view_colors.Reset();
    GetDefaultColors();
  }
  if(the_colors.size != COLOR_COUNT)
    UpdateColors();
  if(td->InheritsFrom(TA_ConSpec))
    return the_colors.FastEl(Project::CON_SPEC)->color;
  else if(td->InheritsFrom(TA_UnitSpec))
    return the_colors.FastEl(Project::UNIT_SPEC)->color;
  else if(td->InheritsFrom(TA_ProjectionSpec))
    return the_colors.FastEl(Project::PRJN_SPEC)->color;
  else if(td->InheritsFrom(TA_LayerSpec))
    return the_colors.FastEl(Project::LAYER_SPEC)->color;
  else if(td->InheritsFrom(TA_Network))
    return the_colors.FastEl(Project::NETWORK)->color;
  else if(td->InheritsFrom(TA_Environment))
    return the_colors.FastEl(Project::ENVIRONMENT)->color;
  else if(td->InheritsFrom(TA_PDPLog))
    return the_colors.FastEl(Project::PDPLOG)->color;
  else if(td->InheritsFrom(TA_SchedProcess))
    return the_colors.FastEl(Project::SCHED_PROC)->color;
  else if(td->InheritsFrom(TA_Stat))
    return the_colors.FastEl(Project::STAT_PROC)->color;
  else if(td->InheritsFrom(TA_Process))
    return the_colors.FastEl(Project::OTHER_PROC)->color;
  else if(td->InheritsFrom(TA_taGroup_impl))
    return the_colors.FastEl(Project::GEN_GROUP)->color;
  else if(td->InheritsFrom(TA_Wizard))
    return the_colors.FastEl(Project::WIZARD)->color;
  else
    return NULL;
}

const ivColor* pdpMisc::GetObjColor(Project* proj, TypeDef* td) {
  if((proj == NULL) || (td == NULL)) return NULL;
  return proj->GetObjColor(td);
}

void Project::LoadDefaults() {
  bool loading = taMisc::is_loading; // cache is loading flag
  fstream def;
  if(!pdpMisc::root->default_file.empty() && cssProgSpace::GetFile(def, pdpMisc::root->default_file)) {
    defaults.Load(def);
    def.close(); def.clear();
    //    cerr << "Successfully loaded default file: " << pdpMisc::root->default_file << "\n";
  }
  else if(pdpMisc::defaults_str != NULL) {
    std::string def_str = pdpMisc::defaults_str;
    std::istringstream sdef(def_str);
    sdef.seekg(0, ios::beg);
    defaults.Load(sdef);
    if(taMisc::dmem_proc == 0)
      cerr << "Using standard pre-compiled defaults file\n";
  }
  else {
    taMisc::Error("Warning: no default file was available - created objects will not automatically be of correct type");
  }
  taMisc::is_loading = loading;
}

void Project::CutLinks() {
  deleting = true;
  int li;
  for(li=0;li<processes.leaves;li++) {
    SchedProcess* sp = (SchedProcess*)processes.Leaf(li);
    if(sp->running)
      sp->Stop();
  }
  if(editor != NULL) { delete editor; editor = NULL; }
  WinBase::CutLinks();	// close windows, etc
  edits.CutLinks();
  scripts.CutLinks();
  logs.CutLinks();
  processes.CutLinks();
  environments.CutLinks();
  networks.CutLinks();
  specs.CutLinks();
  defaults.CutLinks();

  view_font.CutLinks();
  view_colors.CutLinks();
  the_colors.CutLinks();
  view_states.CutLinks();
}

void Project::Copy_(const Project& cp) {
  defaults = cp.defaults;
  specs = cp.specs;
  networks = cp.networks;
  environments = cp.environments;
  processes = cp.processes;
  logs = cp.logs;
  scripts = cp.scripts;
  edits = cp.edits;

  obj_width = cp.obj_width;
  view_font = cp.view_font;
  view_colors = cp.view_colors;
  the_colors = cp.the_colors;
  view_states = cp.view_states;
}

void Project::UpdateAfterEdit() {
  WinBase::UpdateAfterEdit();
  root_win_pos.UpdateAfterEdit();

  UpdateColors();
  if(taMisc::iv_active) {
    InitDisplay();
  }
}

void Project::MakeDefaultWiz(bool auto_opn) {
  Wizard* wiz = (Wizard*)wizards.NewEl(1, pdpMisc::def_wizard);
  if(auto_opn) {
    wiz->auto_open = true;
    wiz->ThreeLayerNet();
    wiz->Edit();
  }
}

void Project::UpdateMenus() {
  mnu_updating = true;
  InitDisplay();
  WinBase::UpdateMenus();
  mnu_updating = false;
}

void Project::GetWinPos() {
  WinBase::GetWinPos();
  root_win_pos.GetWinPos();	// get this..
}

class SimLogEditDialog : public taivEditDialog {
public:
  bool	ShowMember(MemberDef* md) {
    bool rval = (md->ShowMember(show) && (md->iv != NULL));
    if(!rval) return rval;
    if(!(md->name.contains("desc") || (md->name == "use_sim_log") || (md->name == "save_rmv_units")
	 || (md->name == "prev_file_nm"))) return false;
    return true;
  }

  void	Constr_Methods() { };	// no methods

  SimLogEditDialog(TypeDef* tp, void* base) : taivEditDialog(tp, base) { };
};

void Project::UpdateSimLog() {
  SimLogEditDialog* dlg = new SimLogEditDialog(GetTypeDef(), this);
  dlg->Constr(NULL, true, "Update simulation log (SimLog) for this project, storing the name of the project and the description as entered here.  Click off use_sim_log if you are not using this feature");
  if(dlg->Edit() && use_sim_log) {
    time_t tmp = time(NULL);
    String tstamp = ctime(&tmp);
    tstamp = tstamp.before('\n');

    String user;
    char* user_c = getenv("USER");
    if(user_c != NULL) user = user_c;
    char* host_c = getenv("HOSTNAME");
    if(host_c != NULL) user += String("@") + String(host_c);

    fstream fh;
    fh.open("SimLog", ios::out | ios::app);
    fh << endl << endl;
    fh << file_name << " <- " << prev_file_nm << "\t" << tstamp << "\t" << user << endl;
    if(!desc1.empty()) fh << "\t" << desc1 << endl;
    if(!desc2.empty()) fh << "\t" << desc2 << endl;
    if(!desc3.empty()) fh << "\t" << desc3 << endl;
    if(!desc4.empty()) fh << "\t" << desc4 << endl;
    fh.close(); fh.clear();
  }
}

void Project::SetFileName(const char* fname) {
  if(fname == NULL) return;
  prev_file_nm = file_name;
  WinBase::SetFileName(fname);
}

int Project::SaveAs(ostream& strm, TAPtr par, int indent) {
  if(use_sim_log) {
    UpdateSimLog();
  }
  return WinBase::SaveAs(strm, par, indent);
}

int Project::Save(ostream& strm, TAPtr par, int indent) {
  if(taMisc::iv_active) {
    taMisc::Busy();
    GetAllWinPos();
  }
  taMisc::is_saving = true;
  if(save_rmv_units) {
    for(int i=0; i< networks.size; i++) {
      Network* net = (Network*)networks[i];
      net->RemoveUnits();
    }
  }
  dumpMisc::path_tokens.Reset();
  strm << "// ta_Dump File v1.0\n";   // be sure to check version with Load
  int rval = Dump_Save_Path(strm, par, indent);
  if(rval == false) {
    if(taMisc::iv_active)  taMisc::DoneBusy();
    return rval;
  }
  strm << " {\n";

  // save defaults within project save as first item
  defaults.Dump_Save_Path(strm, par, indent+1);
  strm << " { ";
  if(defaults.Dump_Save_PathR(strm, par, indent+2))
    taMisc::indent(strm, indent+1, 1);
  strm << "  };\n";
  defaults.Dump_Save_impl(strm, par, indent+1);
  defaults.Dump_SaveR(strm, par, indent+1);

  if(Dump_Save_PathR(strm, par, indent+1))
    taMisc::indent(strm, indent, 1);
  strm << "};\n";
  Dump_Save_impl(strm, par, indent);
//   Dump_SaveR(strm, par, indent);  // already in the _impl code!  redundant!
  taMisc::is_saving = false;
  dumpMisc::path_tokens.Reset();
  SetWinName();
  if(taMisc::iv_active)  taMisc::DoneBusy();
  return true;
}

int Project::Load(istream& strm, TAPtr par) {
  if((ta_file != NULL) && !ta_file->dir.empty() && (ta_file->dir != ".")) {
    ta_file->GetDir();
    // change directories to where the project was loaded!
    chdir(ta_file->dir);
    String fnm = ta_file->fname.after('/',-1);
    ta_file->dir = "";
    ta_file->fname = fnm;
    file_name = fnm;
  }
  int rval = WinBase::Load(strm, par); // load-em-up
  if(rval) {	 // don't do this as a dump_load_value cuz we need an updateafteredit..
    if(taMisc::iv_active) {
      pdpMisc::post_load_opr.Link(&wizards);
      pdpMisc::post_load_opr.Link(&scripts);
      pdpMisc::post_load_opr.Link(&edits);
    }
    else {
      wizards.AutoEdit();
      scripts.AutoRun();
      edits.AutoEdit();
    }
  }
  return rval;
}

void Project::InitDisplay() {
  if(!taMisc::iv_active || (editor == NULL))
    return;
  editor->InitDisplay();
}

void Project::FixDispButtons() {
  if(!taMisc::iv_active || (editor == NULL))
    return;
  editor->FixEditorButtons();
}

void Project::Select(taNBase* obj, bool add) {
  if(!taMisc::iv_active || (editor == NULL) || (editor->projg == NULL))
    return;
  if((obj == NULL) || !add) {
    editor->projg->selectgroup.RemoveAll();
    view_states.UnSelectAll();
  }
  if(obj == NULL) return;
  ProjViewState* st = editor->GetViewState(obj->GetPath());
  st->selected = true;
  editor->projg->selectgroup.Link(obj);
  editor->InitDisplay();
}

void Project::Minimize() {
  if(!taMisc::iv_active)
    return;
  if(editor != NULL) 
    editor->Minimize();
}

void Project::Maximize() {
  if(!taMisc::iv_active)
    return;
  if(editor != NULL) 
    editor->Maximize();
}

void Project::SetViewFontSize(int point_size) {
  view_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void Project::GetBodyRep() {
  if(!taMisc::iv_active) return;
  if(body != NULL) return;
  UpdateColors();
  editor = new ProjEditor(this, window);
  editor->Init();
  body = editor->GetLook();
  ivResource::ref(body);
}

ivGlyph* Project::GetPrintData(){
  return editor->print_patch;
}

int Project_MGroup::Load(istream& strm, TAPtr par) {
  if((ta_file != NULL) && !ta_file->dir.empty() && (ta_file->dir != ".")) {
    // change directories to where the project was loaded!
    ta_file->GetDir();
    chdir(ta_file->dir);
    String fnm = ta_file->fname.after('/',-1);
    ta_file->dir = "";
    ta_file->fname = fnm;
  }
  int rval = PDPMGroup::Load(strm, par);
  if(rval) {
    Project* p;
    taLeafItr i;
    FOR_ITR_EL(Project, p, this->, i) {
      if(taMisc::iv_active) {
	pdpMisc::post_load_opr.Link(&(p->wizards));
	pdpMisc::post_load_opr.Link(&(p->scripts));
	pdpMisc::post_load_opr.Link(&(p->edits));
      }
      else {
	p->wizards.AutoEdit();
	p->scripts.AutoRun();
	p->edits.AutoEdit();
      }
    }
  }
  return rval;
}

////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// 		Wizard functions	//
//////////////////////////////////////////

bool Project::nw_itm_def_arg = false;
bool pdpMisc::nw_itm_def_arg = false;

BaseSpec_MGroup* Project::FindMakeSpecGp(const char* nm, bool& nw_itm) {
  BaseSpec_MGroup* gp = (BaseSpec_MGroup*)specs.gp.FindName(nm);
  nw_itm = false;
  if(gp == NULL) {
    gp = (BaseSpec_MGroup*)specs.gp.New(1);
    gp->name = nm;
    nw_itm = true;
  }
  return gp;
}

BaseSpec_MGroup* pdpMisc::FindMakeSpecGp(Project* prj, const char* nm, bool& nw_itm) {
  return prj->FindMakeSpecGp(nm, nw_itm);
}

BaseSpec* pdpMisc::FindMakeSpec(Project* prj, const char* nm, TypeDef* td, bool& nw_itm) {
  return (BaseSpec*)prj->specs.FindMakeSpec(nm, td, nw_itm);
}

BaseSpec* pdpMisc::FindSpecName(Project* prj, const char* nm) {
  BaseSpec* rval = (BaseSpec*)prj->specs.FindSpecName(nm);
  if(rval == NULL) {
    taMisc::Error("Error: could not find spec named:", nm);
  }
  return rval;
}

BaseSpec* pdpMisc::FindSpecType(Project* prj, TypeDef* td) {
  BaseSpec* rval = (BaseSpec*)prj->specs.FindSpecType(td);
  if(rval == NULL) {
    taMisc::Error("Error: could not find spec of type:", td->name);
  }
  return rval;
}

Process* pdpMisc::FindMakeProc(Project* prj, const char* nm, TypeDef* td, bool& nw_itm) {
  return (Process*)prj->processes.FindMakeProc(nm, td, nw_itm);
}

Process* pdpMisc::FindProcName(Project* prj, const char* nm) {
  Process* rval = (Process*)prj->processes.FindLeafName(nm);
  if(rval == NULL) {
    taMisc::Error("Error: could not find process named:", nm);
  }
  return rval;
}

Process* pdpMisc::FindProcType(Project* prj, TypeDef* td) {
  Process* rval = (Process*)prj->processes.FindLeafType(td);
  if(rval == NULL) {
    taMisc::Error("Error: could not find process of type:", td->name);
  }
  return rval;
}

PDPLog* pdpMisc::FindMakeLog(Project* prj, const char* nm, TypeDef* td, bool& nw_itm) {
  return (PDPLog*)prj->logs.FindMakeLog(nm, td, nw_itm);
}

PDPLog* pdpMisc::FindLogName(Project* prj, const char* nm) {
  PDPLog* rval = (PDPLog*)prj->logs.FindLeafName(nm);
  if(rval == NULL) {
    taMisc::Error("Error: could not find log named:", nm);
  }
  return rval;
}

PDPLog* pdpMisc::FindLogType(Project* prj, TypeDef* td) {
  PDPLog* rval = (PDPLog*)prj->logs.FindLeafType(td);
  if(rval == NULL) {
    taMisc::Error("Error: could not find log of type:", td->name);
  }
  return rval;
}

SelectEdit* pdpMisc::FindSelectEdit(Project* prj) {
  return (SelectEdit*)prj->edits.DefaultEl();
}

SelectEdit* pdpMisc::FindMakeSelectEdit(Project* prj) {
  SelectEdit* rval = (SelectEdit*)prj->edits.DefaultEl();
  if(rval != NULL) return rval;
  rval = (SelectEdit*)prj->edits.NewEl(1, &TA_SelectEdit);
  return rval;
}


//////////////////////////
// 	Wizard		//
//////////////////////////

void LayerWizEl::Initialize() {
  n_units = 10;
}

void Wizard::Initialize() {
  auto_open = false;
  n_layers = 3;
  layer_cfg.SetBaseType(&TA_LayerWizEl);
  connectivity = FEEDFORWARD;
  event_type = &TA_Event;
}

void Wizard::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(layer_cfg, this);
  layer_cfg.EnforceSize(n_layers);
}

void Wizard::CutLinks() {
  layer_cfg.RemoveAll();
  taNBase::CutLinks();
}

void Wizard::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  layer_cfg.EnforceSize(n_layers);
}

/////////////////////////////
// 	Network Wiz	   //
/////////////////////////////

void Wizard::ThreeLayerNet() {
  n_layers = 3;
  layer_cfg.EnforceSize(n_layers);
  ((LayerWizEl*)layer_cfg[0])->name = "Input";
  ((LayerWizEl*)layer_cfg[0])->io_type = LayerWizEl::INPUT;
  ((LayerWizEl*)layer_cfg[1])->name = "Hidden";
  ((LayerWizEl*)layer_cfg[1])->io_type = LayerWizEl::HIDDEN;
  ((LayerWizEl*)layer_cfg[2])->name = "Output";
  ((LayerWizEl*)layer_cfg[2])->io_type = LayerWizEl::OUTPUT;
}

void Wizard::MultiLayerNet(int n_inputs, int n_hiddens, int n_outputs) {
  n_layers = n_inputs + n_hiddens + n_outputs;
  layer_cfg.EnforceSize(n_layers);
  int i;
  for(i=0;i<n_inputs;i++) {
    ((LayerWizEl*)layer_cfg[i])->name = "Input";
    if(n_inputs > 1) ((LayerWizEl*)layer_cfg[i])->name += "_" + String(i);
    ((LayerWizEl*)layer_cfg[i])->io_type = LayerWizEl::INPUT;
  }
  for(;i<n_inputs + n_hiddens;i++) {
    ((LayerWizEl*)layer_cfg[i])->name = "Hidden";
    if(n_hiddens > 1) ((LayerWizEl*)layer_cfg[i])->name += "_" + String(i-n_inputs);
    ((LayerWizEl*)layer_cfg[i])->io_type = LayerWizEl::HIDDEN;
  }
  for(;i<n_layers;i++) {
    ((LayerWizEl*)layer_cfg[i])->name = "Output";
    if(n_outputs > 1) ((LayerWizEl*)layer_cfg[i])->name += "_" + String(i-(n_inputs+n_hiddens));
    ((LayerWizEl*)layer_cfg[i])->io_type = LayerWizEl::OUTPUT;
  }
}

void Wizard::StdNetwork(Network* net) {
  Project* proj = GET_MY_OWNER(Project);
  if(net == NULL)
    net = pdpMisc::GetNewNetwork(proj);
  if(net == NULL) return;
  layer_cfg.EnforceSize(n_layers);
  net->layers.EnforceSize(n_layers);
  int i;
  int n_hid_layers = 0;
  for(i=0;i<layer_cfg.size;i++) {
    if(((LayerWizEl*)layer_cfg[i])->io_type == LayerWizEl::HIDDEN) n_hid_layers++;
  }
  for(i=0;i<layer_cfg.size;i++) {
    LayerWizEl* el = (LayerWizEl*)layer_cfg[i];
    Layer* lay = (Layer*)net->layers[i];
    if(lay->name != el->name) {
      lay->name = el->name;
      if(el->io_type == LayerWizEl::INPUT) {
	lay->pos.z = 0;
	if(i > 0) {
	  Layer* prv = (Layer*)net->layers[i-1];
	  lay->pos.x = prv->pos.x + prv->geom.x + 1;
	}
      }
      else if(el->io_type == LayerWizEl::HIDDEN) {
	if(i > 0) {
	  Layer* prv = (Layer*)net->layers[i-1];
	  lay->pos.z = prv->pos.z + 1;
	}
      }
      else {			// OUTPUT
	lay->pos.z = n_hid_layers + 1;
	if(i > 0) {
	  LayerWizEl* prvel = (LayerWizEl*)layer_cfg[i-1];
	  if(prvel->io_type == LayerWizEl::OUTPUT) {
	    Layer* prv = (Layer*)net->layers[i-1];
	    lay->pos.x = prv->pos.x + prv->geom.x + 1;
	  }
	}
      }
    }
    lay->n_units = el->n_units;
    lay->UpdateAfterEdit();
  }
  int hid_ctr = 0;
  for(i=0;i<layer_cfg.size;i++) {
    LayerWizEl* el = (LayerWizEl*)layer_cfg[i];
    Layer* lay = (Layer*)net->layers[i];
    if(el->io_type == LayerWizEl::INPUT) continue;
    if(el->io_type == LayerWizEl::HIDDEN) {
      for(int j=0;j<layer_cfg.size;j++) {
	LayerWizEl* fmel = (LayerWizEl*)layer_cfg[j];
	Layer* fm = net->FindLayer(fmel->name);
	if(hid_ctr == 0) {
	  if(fmel->io_type == LayerWizEl::INPUT) 
	    net->FindMakePrjn(lay, fm);
	}
	else {
	  if(i==0) continue;
	  LayerWizEl* prvel = (LayerWizEl*)layer_cfg[i-1];
	  Layer* fm = net->FindLayer(prvel->name);
	  net->FindMakePrjn(lay, fm);
	  if(connectivity == BIDIRECTIONAL)
	    net->FindMakePrjn(fm, lay);
	}
      }
      hid_ctr++;
    }
    else {			// OUTPUT
      for(int j=layer_cfg.size-1;j>=0;j--) {
	LayerWizEl* fmel = (LayerWizEl*)layer_cfg[j];
	Layer* fm = net->FindLayer(fmel->name);
	if(fmel->io_type == LayerWizEl::HIDDEN) {
	  net->FindMakePrjn(lay, fm);
	  if(connectivity == BIDIRECTIONAL)
	    net->FindMakePrjn(fm, lay);
	  break;
	}
	else if((n_hid_layers == 0) && (fmel->io_type == LayerWizEl::INPUT)) {
	  net->FindMakePrjn(lay, fm);
	}
      }
    }
  }
  net->Build();
  net->Connect();

  winbMisc::DelayedMenuUpdate(net);
}


//////////////////////////////////
// 	Enviro Wizard		//
//////////////////////////////////

void Wizard::StdEnv(Environment* env, int n_events) {
  if(env == NULL) {
    env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project));
  }
  if(env == NULL) return;
  layer_cfg.EnforceSize(n_layers);
  EventSpec* es = env->GetAnEventSpec();
  int n_io_layers = 0;
  int i;
  for(i=0;i<layer_cfg.size;i++) {
    if(((LayerWizEl*)layer_cfg[i])->io_type != LayerWizEl::HIDDEN) n_io_layers++;
  }
  es->patterns.EnforceSize(n_io_layers);
  int pctr = 0;
  for(i=0;i<layer_cfg.size;i++) {
    LayerWizEl* el = (LayerWizEl*)layer_cfg[i];
    if(el->io_type == LayerWizEl::HIDDEN) continue;
    PatternSpec* ps = (PatternSpec*)es->patterns[pctr];
    ps->name = el->name;
    ps->layer_name = el->name;
    ps->to_layer = PatternSpec::LAY_NAME;
    if(el->io_type == LayerWizEl::INPUT)
      ps->type = PatternSpec::INPUT;
    else
      ps->type = PatternSpec::TARGET;
    ps->SetToLayer();
    pctr++;
  }
  if(event_type->InheritsFrom(env->events.el_base))
    env->events.el_typ = event_type;
  if(n_events > 0) {
    env->events.EnforceSize(n_events);
  }
  winbMisc::DelayedMenuUpdate(env);
}

void Wizard::UpdateEnvFmNet(Environment* env) {
  if(env == NULL) return;
  env->UpdateAllEventSpecs();
}

void Wizard::SequenceEvents(Environment* env, int n_seqs, int events_per_seq) {
  if(env == NULL) {
    taMisc::Error("SequenceEvents: must have basic constructed environment first");
    return;
  }
  if(env->events.size > 0) {
    env->events.EnforceSize(0);	// no top-level dudes
  }
  if(event_type->InheritsFrom(env->events.el_base))
    env->events.el_typ = event_type;
  env->events.gp.EnforceSize(n_seqs);
  int i;
  for(i=0;i<n_seqs;i++) {
    Event_MGroup* egp = (Event_MGroup*)env->events.gp[i];
    egp->el_typ = event_type;
    egp->EnforceSize(events_per_seq);
  }
  winbMisc::DelayedMenuUpdate(env);
}

void Wizard::TimeSeqEvents(TimeEnvironment* env, int n_seqs, int events_per_seq, float start_time, float time_inc) {
  if(env == NULL) {
    taMisc::Error("TimeSeqEvents: must have basic constructed environment first");
    return;
  }
  if(env->events.size > 0) {
    env->events.EnforceSize(0);
  }
  if(event_type->InheritsFrom(env->events.el_base))
    env->events.el_typ = event_type;
  env->events.gp.EnforceSize(n_seqs);
  int i;
  for(i=0;i<n_seqs;i++) {
    TimeEvent_MGroup* egp = (TimeEvent_MGroup*)env->events.gp[i];
    egp->EnforceSize(events_per_seq);
    if(!egp->InheritsFrom(TA_TimeEvent_MGroup)) continue;
    egp->RegularlySpacedTimes(start_time, time_inc);
  }
  winbMisc::DelayedMenuUpdate(env);
}

//////////////////////////////////
// 	Proc Wizard		//
//////////////////////////////////

void Wizard::StdProcs() {
  Project* proj = GET_MY_OWNER(Project);
  BatchProcess* batch = (BatchProcess*)pdpMisc::FindMakeProc(proj, "Batch_0", &TA_BatchProcess);
  batch->CreateSubProcs();
  batch->ControlPanel();
}

void Wizard::NetAutoSave(SchedProcess* proc, bool just_weights) {
  if(proc == NULL) return;
  if(just_weights)
    proc->final_procs.FindMakeProc(NULL, &TA_SaveWtsProc);
  else
    proc->final_procs.FindMakeProc(NULL, &TA_SaveNetsProc);
  winbMisc::DelayedMenuUpdate(proc);
}

EpochProcess* Wizard::AutoTestProc(SchedProcess* tproc, Environment* tenv) {
  if((tproc == NULL) || (tenv == NULL)) return NULL;
  Project* proj = GET_MY_OWNER(Project);
  EpochProcess* cve = (EpochProcess*)pdpMisc::FindMakeProc(proj, "TestEpoch", &TA_EpochProcess);
  cve->CreateSubProcs();
  cve->SetEnv(tenv);
  cve->wt_update = EpochProcess::TEST;
  cve->order = EpochProcess::SEQUENTIAL;
  tproc->loop_procs.Link(cve);
  GetStatsFromProc(cve, tproc, SchedProcess::LOOP_STATS, Aggregate::LAST);
  EpochProcess* trepc = tproc->GetMyEpochProc();
  if(trepc != NULL)
    GetStatsFromProc(cve, trepc, SchedProcess::FINAL_STATS, Aggregate::LAST);
  AddCountersToTest(cve, tproc);
  return cve;
}

EpochProcess* Wizard::CrossValidation(SchedProcess* tproc, Environment* tenv) {
  if((tproc == NULL) || (tenv == NULL)) return NULL;
  EpochProcess* cve = AutoTestProc(tproc, tenv);
  Stat* sst;
  taLeafItr i;
  FOR_ITR_EL(Stat, sst, tproc->loop_stats.,i) {
    if(!sst->InheritsFrom(&TA_SE_Stat) || (sst->time_agg.from == NULL)) continue;
    Stat* fst = sst->time_agg.from;
    if(fst->GetMySchedProc() == cve) {
      ((SE_Stat*)sst)->se.stopcrit.flag = true;	// stop on this one
    }
  }
  return cve;
}

void Wizard::ToSequenceEvents(SchedProcess* proc) {
  if(proc == NULL) return;
  SchedProcess* epc = proc->GetMyEpochProc();
  if(epc == NULL) {
    taMisc::Error("Error: no epoch process found in hierarchy!");
    return;
  }
  if(epc->InheritsFrom(&TA_SequenceEpoch)) return; // already done!
  epc->AddSubProc(&TA_SequenceProcess);
  epc->ChangeMyType(&TA_SequenceEpoch);
}

void Wizard::NoSequenceEvents(SchedProcess* proc) {
  if(proc == NULL) return;
  SchedProcess* epc = proc->GetMyEpochProc();
  if(epc == NULL) {
    taMisc::Error("Error: no epoch process found in hierarchy!");
    return;
  }
  if(!epc->InheritsFrom(&TA_SequenceEpoch)) return; // already done!
  epc->RemoveSubProc();
  epc->ChangeMyType(&TA_EpochProcess);
}

//////////////////////////////////
// 	Stats Wizard		//
//////////////////////////////////

MonitorStat* Wizard::RecordLayerValues(SchedProcess* proc, SchedProcess::StatLoc loc, Layer* lay, const char* var) {
  if(proc == NULL) return NULL;
  Stat_Group* tgp = proc->GetStatGroup(&TA_MonitorStat, loc);  
  MonitorStat* mst = tgp->FindMakeMonitor(lay, var);
  winbMisc::DelayedMenuUpdate(proc);
  return mst;
}

CopyToEnvStat* Wizard::SaveValuesInDataEnv(MonitorStat* stat) {
  if(stat == NULL) return NULL;
  Project* proj = GET_MY_OWNER(Project);
  Stat_Group* sgp = (Stat_Group*)stat->owner;
  int idx = sgp->Find(stat);
  CopyToEnvStat* cte;
  if(sgp->size <= idx+1) {
    cte = (CopyToEnvStat*)sgp->NewEl(1, &TA_CopyToEnvStat);
  }
  else {
    Stat* nst = (Stat*)sgp->FastEl(idx+1);
    if(nst->InheritsFrom(&TA_CopyToEnvStat))
      cte = (CopyToEnvStat*)nst;
    else {
      cte = (CopyToEnvStat*)sgp->NewEl(1, &TA_CopyToEnvStat);
      sgp->MoveAfter(stat, cte);
    }
  }
  taBase::SetPointer((TAPtr*)&cte->stat, stat);
  if(cte->data_env == NULL) {
    Environment* env = pdpMisc::GetNewEnv(proj, &TA_Environment);
    taBase::SetPointer((TAPtr*)&cte->data_env, env);
  }    
  cte->InitEnv();
  winbMisc::DelayedMenuUpdate(stat->GetMySchedProc());
  return cte;
}

DispDataEnvProc* Wizard::AutoAnalyzeDataEnv(Environment* data_env, int pat_no,
					    DispDataEnvProc::DispType disp_type,
					    SchedProcess* proc, SchedProcess::ProcLoc loc) {
  if((proc == NULL) || (data_env == NULL)) return NULL;
  Process_Group* tgp = proc->GetProcGroup(loc);  
  DispDataEnvProc* ddep = NULL;
  taLeafItr i;
  Process* pr;
  FOR_ITR_EL(Process, pr, tgp->, i) {
    if(!pr->InheritsFrom(TA_DispDataEnvProc)) continue;
    DispDataEnvProc* dp = (DispDataEnvProc*)pr;
    if((dp->data_env == data_env) && (dp->disp_type == disp_type) && (dp->pat_no == pat_no)) {
      ddep = dp;
      break;
    }
  }
  if(ddep == NULL) {
    ddep = (DispDataEnvProc*)tgp->NewEl(1, &TA_DispDataEnvProc);
  }
  taBase::SetPointer((TAPtr*)&ddep->data_env, data_env);
  ddep->pat_no = pat_no;
  ddep->disp_type = disp_type;
  ddep->C_Code();		// run it!
  winbMisc::DelayedMenuUpdate(proc);
  return ddep;
}

DispDataEnvProc* Wizard::AnalyzeNetLayer(SchedProcess* rec_proc, SchedProcess::StatLoc rec_loc,
			     Layer* lay, const char* var,
			     DispDataEnvProc::DispType disp_type, SchedProcess* anal_proc,
			     SchedProcess::ProcLoc anal_loc) {
  MonitorStat* mst = RecordLayerValues(rec_proc, rec_loc, lay, var);
  if(mst == NULL) return NULL;
  CopyToEnvStat* cte = SaveValuesInDataEnv(mst);
  if(cte == NULL) return NULL;
  DispDataEnvProc* ddep = AutoAnalyzeDataEnv(cte->data_env, 0, disp_type, anal_proc, anal_loc);
  return ddep;
}

UnitActRFStat* Wizard::ActBasedReceptiveField(SchedProcess* rec_proc, SchedProcess::StatLoc rec_loc,
					      Layer* recv_layer, Layer* send_layer, Layer* send2_layer,
					      SchedProcess* disp_proc, SchedProcess::ProcLoc disp_loc)
{
  if((rec_proc == NULL) || (disp_proc == NULL)) return NULL;
  Project* proj = GET_MY_OWNER(Project);
  Stat_Group* tgp = rec_proc->GetStatGroup(&TA_UnitActRFStat, rec_loc);
  UnitActRFStat* rfs = (UnitActRFStat*)tgp->FindMakeStat(&TA_UnitActRFStat);
  taBase::SetPointer((TAPtr*)&rfs->layer, recv_layer);
  if(send_layer != NULL)
    rfs->rf_layers.LinkUnique(send_layer);
  if(send2_layer != NULL)
    rfs->rf_layers.LinkUnique(send2_layer);
  if(rfs->data_env == NULL) {
    Environment* env = pdpMisc::GetNewEnv(proj, &TA_Environment);
    taBase::SetPointer((TAPtr*)&rfs->data_env, env);
  }
  rfs->InitRFVals();
  Process_Group* reset_gp = disp_proc->GetProcGroup(SchedProcess::INIT_PROCS);
  UnitActRFStatResetProc* rrf = (UnitActRFStatResetProc*)reset_gp->FindMakeProc(NULL, &TA_UnitActRFStatResetProc);
  taBase::SetPointer((TAPtr*)&rrf->unit_act_rf_stat, rfs);
  AutoAnalyzeDataEnv(rfs->data_env, 0, DispDataEnvProc::RAW_DATA_GRID, disp_proc, disp_loc);
  if(send2_layer != NULL)
    AutoAnalyzeDataEnv(rfs->data_env, 1, DispDataEnvProc::RAW_DATA_GRID, disp_proc, disp_loc);
  winbMisc::DelayedMenuUpdate(rec_proc);
  return rfs;
}

DispNetWeightsProc* Wizard::DisplayNetWeights(Layer* recv_layer, Layer* send_layer,
      SchedProcess* proc, SchedProcess::ProcLoc loc) {
  if((proc == NULL) || (recv_layer == NULL) || (send_layer == NULL)) return NULL;
  Process_Group* tgp = proc->GetProcGroup(loc);
  DispNetWeightsProc* dnw = (DispNetWeightsProc*)tgp->FindMakeProc(NULL, &TA_DispNetWeightsProc);
  dnw->recv_layer_nm = recv_layer->name;
  dnw->send_layer_nm = send_layer->name;
  dnw->C_Code();
  winbMisc::DelayedMenuUpdate(proc);
  return dnw;
}

void Wizard::StopOnActThresh(SchedProcess* proc, Layer* lay, float thresh) {
  if((proc == NULL) || (lay == NULL)) return;
  Stat_Group* sgp = (Stat_Group*)&(proc->loop_stats);
  ActThreshRTStat* rts = (ActThreshRTStat*)sgp->FindMakeStat(&TA_ActThreshRTStat);
  taBase::SetPointer((TAPtr*)&rts->layer, lay);
  rts->act_thresh = thresh;
  rts->max_act.stopcrit.flag = true;
  rts->UpdateAfterEdit();
  winbMisc::DelayedMenuUpdate(proc);
}

void Wizard::AddCountersToTest(SchedProcess* te_proc, SchedProcess* tr_proc) {
  if((te_proc == NULL) || (tr_proc == NULL)) return;
  Stat_Group* sgp = (Stat_Group*)&(te_proc->final_stats);
  ProcCounterStat* st = (ProcCounterStat*)sgp->FindMakeStat(&TA_ProcCounterStat);
  taBase::SetPointer((TAPtr*)&st->proc, tr_proc);
  st->UpdateAfterEdit();
  winbMisc::DelayedMenuUpdate(te_proc);
}

void Wizard::GetStatsFromProc(SchedProcess* sproc, SchedProcess* tproc, SchedProcess::StatLoc tloc, Aggregate::Operator agg_op) {
  if((sproc == NULL) || (tproc == NULL)) return;
  Stat* sst;
  taLeafItr i;
  FOR_ITR_EL(Stat, sst, sproc->loop_stats.,i) {
    Stat_Group* tgp = tproc->GetStatGroup(sst->GetTypeDef(), tloc);
    Stat* trst = tgp->FindAggregator(sst, agg_op);
    if(trst == NULL)
      tproc->MakeAggregator(sst, tloc, agg_op);
  }
  FOR_ITR_EL(Stat, sst, sproc->final_stats.,i) {
    Stat_Group* tgp = tproc->GetStatGroup(sst->GetTypeDef(), tloc);
    Stat* trst = tgp->FindAggregator(sst, agg_op);
    if(trst == NULL)
      tproc->MakeAggregator(sst, tloc, agg_op);
  }
  winbMisc::DelayedMenuUpdate(tproc);
}

TimeCounterStat* Wizard::AddTimeCounter(SchedProcess* inc_proc, SchedProcess::StatLoc loc, SchedProcess* reset_proc) {
  if((inc_proc == NULL) || (reset_proc == NULL)) return NULL;
  Stat_Group* igp = inc_proc->GetStatGroup(&TA_TimeCounterStat, loc);
  TimeCounterStat* st = (TimeCounterStat*)igp->FindMakeStat(&TA_TimeCounterStat);
  Process_Group* rgp = (Process_Group*)&(reset_proc->init_procs);
  TimeCounterStatResetProc* rproc =
    (TimeCounterStatResetProc*)rgp->FindMakeProc(NULL, &TA_TimeCounterStatResetProc);
  taBase::SetPointer((TAPtr*)&rproc->time_ctr_stat, st);
  winbMisc::DelayedMenuUpdate(inc_proc);
  winbMisc::DelayedMenuUpdate(reset_proc);
  return st;
}


//////////////////////////////////
// 	Log Wizard		//
//////////////////////////////////

void Wizard::StdLogs(SchedProcess* proc) {
  if(proc == NULL) return;
  SchedProcess* epc = proc->GetMyEpochProc();
  if(epc != NULL) {
    LogProcess(epc, &TA_GraphLog);

    SchedProcess* bat = epc->FindSuperProc(&TA_BatchProcess);
    if(bat != NULL) {
      LogProcess(bat, &TA_TextLog);
    }
  }
  SchedProcess* trl = proc->GetMyTrialProc();
  if(trl != NULL) {
    LogProcess(trl, &TA_TextLog);
  }
}

void Wizard::LogProcess(SchedProcess* proc, TypeDef* log_type) {
  if((proc == NULL) || (log_type == NULL)) return;
  Project* proj = GET_MY_OWNER(Project);
  String nm = proc->name + "_" + log_type->name;
  PDPLog* plog = pdpMisc::FindMakeLog(proj, nm, log_type);
  if(plog->log_proc.size == 0)
    plog->AddUpdater(proc);
  winbMisc::DelayedMenuUpdate(proj);
}

////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
//	PDPView:  view for pdp system		//
//////////////////////////////////////////////////

void PDPView::AddNotify(TAPtr ud) {
  if((ud != NULL) && ud->InheritsFrom(TA_SchedProcess)) {
    ((SchedProcess*)ud)->displays.LinkUnique(this);
  }
}

void PDPView::RemoveNotify(TAPtr ud) {
  if((ud != NULL) && ud->InheritsFrom(TA_SchedProcess)) {
    ((SchedProcess*)ud)->displays.Remove(this);
  }
}

//////////////////////////////////////////////////
//	PDPMGroup  base mgroup for pdp system	//
//////////////////////////////////////////////////

void PDPMGroup::Initialize() {
}

void PDPMGroup::Destroy() {
}

void PDPMGroup::UpdateMenu() {
  MenuGroup_impl::UpdateMenu();
  Project* prj = GET_MY_OWNER(Project);
  if((prj != NULL) && !prj->mnu_updating)
    prj->InitDisplay();
}

//////////////////////////////////////////
//	pdpMisc: misc stuff for pdp	//
//////////////////////////////////////////

PDPRoot* 	pdpMisc::root = NULL;
char* 		pdpMisc::defaults_str = NULL;
String_Array	pdpMisc::proj_to_load;
taBase_List	pdpMisc::post_load_opr;

TypeDef*	pdpMisc::def_wizard = &TA_Wizard;

void (*pdpMisc::Init_Hook)() = NULL;

ColorScaleSpec* pdpMisc::GetDefaultColor() {
  if((root == NULL) || !taMisc::iv_active)	return NULL;
  return (ColorScaleSpec*)root->colorspecs.DefaultEl();
}

static int get_unique_file_number(int st_no, const char* prefix, const char* suffix) {
  String prfx = prefix;
  String fname;
  int i;
  for(i=st_no; i<10000; i++) {	// stop at 10,000
    fname = prfx + String(i) + suffix;
    int acc = access(fname, R_OK);
    if(acc != 0)
      break;			// its ok..
  }
  fstream strm;
  strm.open(fname, ios::out);	// this should hold the place for the file
  strm.close();	strm.clear();		// while it is being saved, etc..
  return i;
}

// for saving a recovery file if program crashes, is killed, etc.
void pdpMisc::SaveRecoverFile(int err) {
  static bool has_crashed = false;
  signal(err, SIG_DFL);		// disable catcher

  taivMisc::Cleanup(err);	// cleanup stuff in taiv

  if(err == SIGUSR1)
    cerr << "PDP++ saving network file(s) from signal: ";
  else if((err == SIGUSR2) || (err == SIGALRM))
    cerr << "PDP++ saving project file(s) from signal: ";
  else 
    cerr << "PDP++ saving recover file(s) and exiting from signal: ";
  taMisc::Decode_Signal(err);
  cerr << "\n";

  if(has_crashed) {
    cerr << "PDP++ unable to save recover file...sorry\n";
    exit(err);
  }
  has_crashed = true;		// to prevent recursive crashing..

  String net_sfx = ".net";	net_sfx += taMisc::compress_sfx;
  String wts_sfx = ".wts";  	wts_sfx += taMisc::compress_sfx;
  String proj_sfx = ".proj";	proj_sfx += taMisc::compress_sfx;

  String home_dir = getenv("HOME"); // home directory if curent dir does not work.
  taivGetFile gf;		// use a getfile for compressed writes..
  Project* p;
  int cnt = 0;
  taLeafItr i;
  FOR_ITR_EL(Project, p, pdpMisc::root->projects., i) {
    if(err == SIGUSR1) {	// usr1 is to save network at that point
      Network* net;
      taLeafItr ni;
      FOR_ITR_EL(Network, net, p->networks., ni) {
	String use_sfx = net_sfx;
	if(net->usr1_save_fmt == Network::JUST_WEIGHTS)
	  use_sfx = wts_sfx;
	cnt = get_unique_file_number(cnt, "PDP++NetSave.", use_sfx);
	gf.fname = "PDP++NetSave." + String(cnt) + use_sfx;
	ostream* strm = gf.open_write();
	if(((strm == NULL) || strm->bad()) && !home_dir.empty()) {
	  // try it with the home diretory
	  cerr << "Error saving in current directory, trying home directory";
	  gf.Close();
	  gf.fname = home_dir + "/" + gf.fname;
	  strm = gf.open_write();
	}
	if((strm == NULL) || strm->bad())
	  cerr << "SaveNetwork: could not open file: " << gf.fname << "\n";
	else {
	  if(net->usr1_save_fmt == Network::JUST_WEIGHTS)
	    net->WriteWeights(*strm, net->wt_save_fmt);
	  else
	    net->Save(*strm);
	}
	gf.Close();
	cnt++;
      }
    }
    else {
      String prfx;
      if((err == SIGUSR2) || (err == SIGALRM))
	prfx = "PDP++Project.";
      else
	prfx = "PDP++Recover.";
      cnt = get_unique_file_number(cnt, prfx, proj_sfx);
      gf.fname = prfx + String(cnt) + proj_sfx;
      ostream* strm = gf.open_write();
      if(((strm == NULL) || strm->bad()) && !home_dir.empty()) {
	// try it with the home diretory
	cerr << "Error saving in current directory, trying home directory";
	gf.Close();
	gf.fname = home_dir + "/" + gf.fname;
	strm = gf.open_write();
      }
      if((strm == NULL) || strm->bad())
	cerr << "SaveRecoverFile: could not open file: " << gf.fname << "\n";
      else
	p->Save(*strm);
      gf.Close();
      cnt++;
    }
  }

  if((err == SIGALRM) || (err == SIGUSR1) || (err == SIGUSR2)) {
    taMisc::Register_Cleanup((SIGNAL_PROC_FUN_TYPE) SaveRecoverFile);
    has_crashed = false;
  }
  else {
    kill(getpid(), err);		// activate signal 
  }
}

int pdpMisc::WaitProc_LoadProj() {
  taivGetFile* gf = NULL;
  bool delete_gf = true;
  if(taMisc::iv_active) {
    root->GetFileDlg();
    gf = root->projects.ta_file;
    delete_gf = false;
  }
  if(gf == NULL)
    gf = new taivGetFile;

  taivMisc::Busy();
  int i;
  for(i=0; i<proj_to_load.size; i++) {
    String fnm = proj_to_load[i];
    gf->fname = fnm;
    istream* strm = gf->open_read();
    if((strm == NULL) || !(gf->open_file))
      taMisc::Error("Project Load: could not open file", fnm);
    else {
      taivMisc::CreateLoadDialog();
      root->projects.Load(*strm);
      if(root->projects.size > i)
	((Project*)root->projects[i])->file_name = fnm;
      taivMisc::RemoveLoadDialog();
    }
    gf->Close();
    if(taMisc::iv_active) {
      winbMisc::DelayedMenuUpdate(&(root->projects));
    }
  }
  if(delete_gf)
    delete gf;
  proj_to_load.Reset();
  taivMisc::DoneBusy();
  return 0;
}

int pdpMisc::WaitProc_PostLoadOpr() {
  int i;
  for(i=0;i<post_load_opr.size;i++) {
    TAPtr obj = post_load_opr[i];
    if(obj->InheritsFrom(TA_Script_MGroup)) {
      ((Script_MGroup*)obj)->AutoRun();
    }
    else if(obj->InheritsFrom(TA_SelectEdit_MGroup)) {
      ((SelectEdit_MGroup*)obj)->AutoEdit();
    }
    else if(obj->InheritsFrom(TA_Wizard_MGroup)) {
      ((Wizard_MGroup*)obj)->AutoEdit();
    }
  }
  post_load_opr.Reset();
  return 0;
}

int pdpMisc::WaitProc() {
  if(proj_to_load.size > 0) {
    WaitProc_LoadProj();
  }
  else if(post_load_opr.size > 0) {
    WaitProc_PostLoadOpr();
  }

#ifdef DMEM_COMPILE
  if((taMisc::dmem_nprocs > 1) && (taMisc::dmem_proc == 0)) {
    DMem_WaitProc();
  }
#endif

  if(taMisc::iv_active) {
    winbMisc::OpenWindows();
    return winbMisc::WaitProc();
  }
  else {
    return tabMisc::WaitProc();
  }
}

PDPLog* pdpMisc::GetNewLog(Project* prj, TypeDef* typ) {
  if((prj == NULL) || (typ == NULL)) return NULL;
  PDPLog* rval = (PDPLog*)prj->logs.NewEl(1, typ);
  taivMisc::RunIVPending();
  winbMisc::DelayedMenuUpdate(prj);
  return rval;
}

Environment* pdpMisc::GetNewEnv(Project* prj, TypeDef* typ) {
  if(prj == NULL) return NULL;
  Environment* rval = (Environment*)prj->environments.NewEl(1, typ);
  taivMisc::RunIVPending();
  winbMisc::DelayedMenuUpdate(prj);
  return rval;
}  

Network* pdpMisc::GetNewNetwork(Project* prj, TypeDef* typ) {
  if(prj == NULL) return NULL;
  Network* rval = (Network*)prj->networks.NewEl(1, typ);
  taivMisc::RunIVPending();
  winbMisc::DelayedMenuUpdate(prj);
  return rval;
}  

Network* pdpMisc::GetDefNetwork(Project* prj) {
  if(prj == NULL) return NULL;
  return (Network*)prj->networks.DefaultEl();
}  

// for pdp icon (See base.h)

#define pdp_bitmap_width 64
#define pdp_bitmap_height 64
static unsigned char pdp_bitmap_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x83, 0xff,
  0x1f, 0xfc, 0xff, 0x00, 0x10, 0x00, 0x82, 0x00, 0x10, 0x04, 0x80, 0x00,
  0x10, 0x00, 0x82, 0x00, 0x10, 0x04, 0x80, 0x00, 0x10, 0x1f, 0x82, 0xf8,
  0x10, 0xc4, 0x87, 0x00, 0x10, 0x32, 0x82, 0x10, 0x11, 0x84, 0x8c, 0x00,
  0x10, 0x22, 0x82, 0x10, 0x11, 0x84, 0x88, 0x00, 0x10, 0x32, 0x82, 0x10,
  0x11, 0x84, 0x8c, 0x00, 0x10, 0x1e, 0x82, 0x10, 0x11, 0x84, 0x87, 0x00,
  0x10, 0x02, 0x82, 0x10, 0x11, 0x84, 0x80, 0x00, 0x10, 0x02, 0x82, 0x10,
  0x11, 0x84, 0x80, 0x00, 0x10, 0x0f, 0x82, 0xf8, 0x10, 0xc4, 0x83, 0x00,
  0x10, 0x00, 0x82, 0x00, 0x10, 0x04, 0x80, 0x00, 0x10, 0x00, 0x82, 0x00,
  0x10, 0x04, 0x80, 0x00, 0x10, 0x00, 0x82, 0x00, 0x10, 0x04, 0x80, 0x00,
  0xf0, 0xff, 0x83, 0xff, 0x1f, 0xfc, 0xff, 0x00, 0x00, 0x78, 0x00, 0x50,
  0x00, 0xf0, 0x00, 0x00, 0x00, 0x90, 0x03, 0x88, 0x00, 0x4e, 0x00, 0x00,
  0x00, 0x20, 0x0c, 0x04, 0xc1, 0x21, 0x00, 0x00, 0x00, 0x40, 0x70, 0x02,
  0x3a, 0x10, 0x00, 0x00, 0x00, 0x40, 0x80, 0x03, 0x06, 0x08, 0x00, 0x00,
  0x00, 0x80, 0x80, 0xdc, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x41, 0xf8,
  0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x22, 0x07, 0x13, 0x01, 0x00, 0x00,
  0x00, 0x00, 0xf4, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x07,
  0xff, 0x3f, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00,
  0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04,
  0x01, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00,
  0x00, 0x20, 0x08, 0x04, 0x41, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x04,
  0x41, 0x20, 0x00, 0x00, 0x00, 0x20, 0x3e, 0x04, 0xf1, 0x21, 0x00, 0x00,
  0x00, 0x20, 0x08, 0x04, 0x41, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x04,
  0x41, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00,
  0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04,
  0x01, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x01, 0x20, 0x00, 0x00,
  0x00, 0xe0, 0xff, 0x07, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x01,
  0x7c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x42, 0x9e, 0x23, 0x06, 0x00, 0x00,
  0x00, 0x00, 0x81, 0xf9, 0x11, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x07,
  0x1e, 0x10, 0x00, 0x00, 0x00, 0x40, 0xf0, 0x0c, 0xe2, 0x21, 0x00, 0x00,
  0x00, 0x20, 0x0e, 0x10, 0x01, 0xde, 0x00, 0x00, 0x00, 0xf0, 0x01, 0xe0,
  0x00, 0xe0, 0x01, 0x00, 0xe0, 0xff, 0x07, 0xff, 0x3f, 0xf8, 0xff, 0x01,
  0x20, 0x00, 0x04, 0x01, 0x20, 0x08, 0x00, 0x01, 0x20, 0x00, 0x04, 0x01,
  0x20, 0x08, 0x00, 0x01, 0x20, 0x3e, 0x04, 0xf1, 0x21, 0x88, 0x0f, 0x01,
  0x20, 0x64, 0x04, 0x21, 0x22, 0x08, 0x19, 0x01, 0x20, 0x44, 0x04, 0x21,
  0x22, 0x08, 0x11, 0x01, 0x20, 0x64, 0x04, 0x21, 0x22, 0x08, 0x19, 0x01,
  0x20, 0x3c, 0x04, 0x21, 0x22, 0x08, 0x0f, 0x01, 0x20, 0x04, 0x04, 0x21,
  0x22, 0x08, 0x01, 0x01, 0x20, 0x04, 0x04, 0x21, 0x22, 0x08, 0x01, 0x01,
  0x20, 0x1e, 0x04, 0xf1, 0x21, 0x88, 0x07, 0x01, 0x20, 0x00, 0x04, 0x01,
  0x20, 0x08, 0x00, 0x01, 0x20, 0x00, 0x04, 0x01, 0x20, 0x08, 0x00, 0x01,
  0x20, 0x00, 0x04, 0x01, 0x20, 0x08, 0x00, 0x01, 0xe0, 0xff, 0x07, 0xff,
  0x3f, 0xf8, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


// this is the main that should be called..

int pdpMisc::Main(int argc, char *argv[]) {
#ifdef OPT_MALLOC
  mallopt(M_MXFAST, 48); // optimize allocation for objs up to 48 bytes..
  // todo: the problem is that this comes well after first small block is
  // allocated, so it probably doesn't do any good (and may cause crashing!)
#endif

  bool	use_gui = true;
  String user_spec_def;
  int ac;

#ifdef DMEM_COMPILE
  MPI_Init(&argc, &argv);
  taMisc::DMem_Initialize();
#endif

  for(ac = 1; ac < argc; ac++) {
    String tmp = argv[ac];
    if(tmp == "-nogui")
      use_gui = false;
    // project is either flagged with a -p flag or the first or second argument
    if((tmp == "-p") || ((ac <= 2) && tmp.contains(".proj"))) {
      if(tmp == "-p") {
	if(argc > ac+1) {
	  proj_to_load.Add(argv[ac+1]);
	  ac++;			// skip over it so as to not load 2x
	}
      }
      else
	proj_to_load.Add(tmp);
    }
    if((tmp == "-d") || (tmp == "-def")) {
      if(argc > ac+1)
	user_spec_def = argv[ac+1];
    }
  }

  if(Init_Hook != NULL)
    (*Init_Hook)();	// call the user's init function (which will call pdp)
  else
    ta_Init_pdp();		// always has to be first

  if(use_gui && (taMisc::dmem_proc == 0))
    new ivSession("PDP++", argc, argv, PDP_options, PDP_defs);

  root = new PDPRoot();
  taBase::Ref(root);
  root->InitLinks();	// normally the owner would do this, but..
  root->name = "root";

  // tabMisc stuff
  tabMisc::root = (TAPtr)root;
  taMisc::default_scope = &TA_Project;

  // cssMisc stuff
  cssMisc::HardVars.Push(cssBI::root = new cssTA_Base(root, 1, &TA_PDPRoot,"root"));
  cssMisc::Initialize(argc, (const char**)argv);

  ((taMisc*)TA_taMisc.GetInstance())->LoadConfig();
  // need this config to get mswin_scale (in taivMisc::Initialize) before opening root window.

  if(use_gui && (taMisc::dmem_proc == 0)) {
    taivMisc::Initialize();
    taivM->icon_bitmap = new ivBitmap(pdp_bitmap_bits,pdp_bitmap_width,
				       pdp_bitmap_height);
    ivResource::ref(taivM->icon_bitmap);

    root->WinInit();
    root->OpenNewWindow();
    // set the update action (taken after Ok or Apply in the Edit dialog)
    taivMisc::Update_Hook = winbMisc::DelayedMenuUpdate;
    winbMisc::group_leader = root->window;
    winbMisc::DelayedMenuUpdate(root); 	// get menus after startup..
  }
  // create colorspecs even if nogui, since they are referred to in projects
  root->colorspecs.SetDefaultColor();	// set color after starting up..

  if(cssivSession::WaitProc == NULL)
    cssivSession::WaitProc = pdpMisc::WaitProc;

#ifndef DMEM_COMPILE
  taMisc::Register_Cleanup((SIGNAL_PROC_FUN_TYPE) SaveRecoverFile);
#endif

#ifndef CYGWIN
  String pdp_dir = "/usr/local/pdp++"; // default pdp home directory
#else
  String pdp_dir = "C:/PDP++"; // default pdp home directory
#endif
  char* pdp_dir_env = getenv("PDPDIR");
  if(pdp_dir_env != NULL)
    pdp_dir = pdp_dir_env;

  String home_dir;
  char* home_dir_env = getenv("HOME");
  if(home_dir_env != NULL)
    home_dir = home_dir_env;

  taMisc::include_paths.AddUnique(pdp_dir);
  taMisc::include_paths.AddUnique(pdp_dir+"/css/include");
  taMisc::include_paths.AddUnique(pdp_dir+"/defaults");
  if(!home_dir.empty()) {
    taMisc::include_paths.AddUnique(home_dir+"/mypdp++");
    taMisc::include_paths.AddUnique(home_dir+"/pdp++");
    taMisc::include_paths.AddUnique(home_dir+"/mypdp++/defaults");
    taMisc::include_paths.AddUnique(home_dir+"/pdp++/defaults");
  }
  
  String prognm = argv[0];
  if(prognm.contains('/'))
    prognm = prognm.after('/',-1);
  if(prognm.contains("++"))	// the distribution version of an executable
    prognm = prognm.before("++");
  if(prognm.contains('.'))	// some kind of extention
    prognm = prognm.before('.', -1);

  // set the defaults to be the given executable's name
  if((prognm == "pdpshell") || (prognm == "pdp") || (prognm == "bp"))
    root->default_file = "bp.def";
  else
    root->default_file = prognm + ".def";

  if(!user_spec_def.empty()) {
    root->default_file = user_spec_def;
    if(!root->default_file.contains(".def"))
      root->default_file += ".def"; // justin case
  }

  String vers = "-version";
  int i;
  for(i=1; i<=argc; i++) {
    if(vers == (const char*)argv[i]) {
      root->Info();
      break;
    }
  }

  root->LoadConfig();

  cssMisc::Top->CompileRunClear(".pdpinitrc");

  if((proj_to_load.size > 0) && !use_gui)
    pdpMisc::WaitProc_LoadProj();	// load file manually, since it won't go thru waitproc
  if(taivMisc::iv_active) winbMisc::OpenWindows();

#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    if(use_gui) {
      if(taMisc::dmem_proc == 0) {
	DMemShare::InitCmdStream();
	// need to have some initial string in the stream, otherwise it goes EOF and is bad!
	*(DMemShare::cmdstream) << "cerr << \"proc no: \" << taMisc::dmem_proc << endl;" << endl;
	taivMisc::StartRecording((ostream*)(DMemShare::cmdstream));
	cssMisc::Top->StartupShell(cin, cout);
	DMemShare::CloseCmdStream();
	cerr << "proc: 0 quitting!" << endl;
      }
      else {
	use_gui = false;	// not for subguys
	if(proj_to_load.size > 0) {
	  if(taMisc::dmem_debug)
	    cerr << "proc " << taMisc::dmem_proc << " loading projects." << endl;
	  pdpMisc::WaitProc_LoadProj();	// load file manually, since it won't go thru waitproc
	}

	cssMisc::init_interactive = false; // don't stay in startup shell
// 	if(taMisc::dmem_debug)
// 	  cerr << "proc " << taMisc::dmem_proc << " starting shell." << endl;
	// get rid of wait proc for rl -- we call it ourselves
	extern int (*rl_event_hook)(void);
 	rl_event_hook = NULL;
 	cssMisc::Top->StartupShell(cin, cout);
	//	cssMisc::Top->debug = 2;
	DMem_SubEventLoop();
	cerr << "proc: " << taMisc::dmem_proc << " quitting!" << endl;
      }
    }
    else {
      cssMisc::Top->StartupShell(cin, cout);
    }
  }
  else {
    cssMisc::Top->StartupShell(cin, cout);
  }
  MPI_Finalize();

#else
  cssMisc::Top->StartupShell(cin, cout);
#endif

  delete root;
  return 0;
}

#ifdef DMEM_COMPILE

static cssProgSpace* dmem_space1 = NULL;
static cssProgSpace* dmem_space2 = NULL;

int pdpMisc::DMem_WaitProc(bool send_stop_to_subs) {
  if(dmem_space1 == NULL) dmem_space1 = new cssProgSpace;
  if(dmem_space2 == NULL) dmem_space2 = new cssProgSpace;

  if(DMemShare::cmdstream->bad() || DMemShare::cmdstream->eof()) {
    taMisc::Error("DMem: Error! cmstream is bad or eof.",
		  "Software will not respond to any commands, must quit!!");
  }
  while(DMemShare::cmdstream->tellp() > DMemShare::cmdstream->tellg()) {
    DMemShare::cmdstream->seekg(0, ios::beg);
    string str = DMemShare::cmdstream->str();
    String cmdstr = str.c_str();
    cmdstr = cmdstr.before((int)(DMemShare::cmdstream->tellp() - DMemShare::cmdstream->tellg()));
    // make sure to only get the part that is current -- other junk might be in there.
    cmdstr += '\n';
    if(taMisc::dmem_debug) {
      cerr << "proc 0 sending cmd: " << cmdstr;
    }
    DMemShare::cmdstream->seekp(0, ios::beg);

    int cmdlen = cmdstr.length();

    DMEM_MPICALL(MPI_Bcast((void*)&cmdlen, 1, MPI_INT, 0, MPI_COMM_WORLD),
		 "Proc 0 WaitProc", "MPI_Bcast - cmdlen");

    DMEM_MPICALL(MPI_Bcast((void*)(const char*)cmdstr, cmdlen, MPI_CHAR, 0, MPI_COMM_WORLD),
		 "Proc 0 WaitProc", "MPI_Bcast - cmd");

    if(taMisc::dmem_debug) {
      cerr << "proc 0 running cmd: " << cmdstr << endl;
    }
    // now run the command: it wasn't run before!
    cssProgSpace* sp = dmem_space1; // if first space is currently running, use another
    if(sp->state & (cssProg::State_Run | cssProg::State_Cont)) {
      if(taMisc::dmem_debug) 
	cerr << "proc 0 using 2nd space!" << endl;
      sp = dmem_space2;
    }

    sp->CompileCode(cmdstr);
    sp->Run();
    sp->ClearAll();
  }
  if(send_stop_to_subs) {
    String cmdstr = "stop";
    int cmdlen = cmdstr.length();
    DMEM_MPICALL(MPI_Bcast((void*)&cmdlen, 1, MPI_INT, 0, MPI_COMM_WORLD),
		 "Proc 0 WaitProc, SendStop", "MPI_Bcast - cmdlen");
    DMEM_MPICALL(MPI_Bcast((void*)(const char*)cmdstr, cmdlen, MPI_CHAR, 0, MPI_COMM_WORLD),
		 "Proc 0 WaitProc, SendStop", "MPI_Bcast - cmdstr");
  }
  return 0;
}

int pdpMisc::DMem_SubEventLoop() {
  if(taMisc::dmem_debug) {
    cerr << "proc: " << taMisc::dmem_proc << " event loop start" << endl;
  }

  if(dmem_space1 == NULL) dmem_space1 = new cssProgSpace;
  if(dmem_space2 == NULL) dmem_space2 = new cssProgSpace;

  while(true) {
    int cmdlen;
    DMEM_MPICALL(MPI_Bcast((void*)&cmdlen, 1, MPI_INT, 0, MPI_COMM_WORLD),
		 "Proc n SubEventLoop", "MPI_Bcast");
    char* recv_buf = new char[cmdlen+2];
    DMEM_MPICALL(MPI_Bcast(recv_buf, cmdlen, MPI_CHAR, 0, MPI_COMM_WORLD),
		 "Proc n SubEventLoop", "MPI_Bcast");
    recv_buf[cmdlen] = '\0';
    String cmd = recv_buf;
    delete recv_buf;

    if(cmd.length() > 0) {
      if(taMisc::dmem_debug) {
       cerr << "proc " << taMisc::dmem_proc << " recv cmd: " << cmd << endl << endl;
      }
      if(cmd == "stop") {
	if(taMisc::dmem_debug)
	  cerr << "proc " << taMisc::dmem_proc << " got stop command, stopping out of sub event processing loop." << endl;
	return 1;
      }
      else if(!cmd.contains("Save(") && !cmd.contains("SaveAs(")) {
	if(taMisc::dmem_debug) {
	  cerr << "proc " << taMisc::dmem_proc << " running cmd: " << cmd << endl;
	}

	cssProgSpace* sp = dmem_space1; // if first space is currenntly running, use another
	if(sp->state & (cssProg::State_Run | cssProg::State_Cont)) {
	  if(taMisc::dmem_debug)
	    cerr << "proc " << taMisc::dmem_proc << " using 2nd space!" << endl;
	  sp = dmem_space2;
	}

	sp->CompileCode(cmd);
	sp->Run();
	sp->ClearAll();

	if(cmd.contains("Quit()")) {
	  if(taMisc::dmem_debug)
	    cerr << "proc " << taMisc::dmem_proc << " got quit command, quitting." << endl;
	  return 1;
	}
      }
    }
    else {
      cerr << "proc " << taMisc::dmem_proc << " received null command!" << endl;
    }
    // do basic wait proc here..
    tabMisc::WaitProc();
  }
  return 0;
}

#endif
