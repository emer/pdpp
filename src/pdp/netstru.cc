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

// netstru.cc
// Functions specific to structures defined in netstru.h


#include <pdp/netstru.h>
#include <pdp/net_iv.h>
#include <css/css_iv.h>		// for the cssivSession
#include <ta/taiv_data.h>
#include <ta/ta_group_iv.h>

// for NetView::dump_save/load
#include <iv_graphic/graphic_viewer.h>
// for xform stuff
#include <iv_graphic/graphic.h>

#include <ta/enter_iv.h>
#include <InterViews/window.h>
// for view labels
#include <InterViews/font.h>
#include <ta/leave_iv.h>

#ifdef DMEM_COMPILE
#include <mpi.h>
#endif

//////////////////////////
//  	Schedule	//
//////////////////////////

void SchedItem::Initialize() {
  start_ctr = 0;
  start_val = 0.0f;	
  duration = 0;
  step = .01f;
}

void SchedItem::Copy_(const SchedItem& cp) {
  start_ctr = cp.start_ctr;
  start_val = cp.start_val;
  duration = cp.duration;
  step = cp.step;
}

void Schedule::Initialize() {
  last_ctr = -1;
  default_val = 1.0f;
  cur_val = 0.0f;
  interpolate = true;
  SetBaseType(&TA_SchedItem);
}

void Schedule::Copy_(const Schedule& cp) {
  last_ctr = cp.last_ctr;
  default_val = cp.default_val;
  cur_val = cp.cur_val;
  interpolate = cp.interpolate;
}

void Schedule::UpdateAfterEdit() {
  last_ctr = -1;
  cur_val = 0;
  int lst_ctr = -1;
  Schedule temp;
  int i;
  for(i=0; i < size; i++) {
    SchedItem* itm = FastEl(i);
    if(itm->start_ctr < lst_ctr) {
      temp.Transfer(itm);
      i--;
    }
    else {
      lst_ctr = itm->start_ctr;
    }
  }
  for (; temp.size > 0 ;) {
    SchedItem* itm = temp.FastEl(0);
    int j;
    for(j=0; (j < size) && (itm->start_ctr > FastEl(j)->start_ctr); j++);
    Insert(itm,j);		// always insert item in new spot
    temp.Remove(0);
  }
  
  if(size > 0)
    FastEl(0)->start_ctr = 0;

  for(i=0; i < size; i++) {
    SchedItem* itm = FastEl(i);
    if(i == (size - 1)) {
      itm->duration = 1;
      itm->step = 1.0;
      break;
    }
    SchedItem* nxt = FastEl(i+1);
    itm->duration = nxt->start_ctr - itm->start_ctr;
    itm->duration = MAX(itm->duration, 1);
    itm->step = (nxt->start_val - itm->start_val) / (float)itm->duration;
  }
  taList<SchedItem>::UpdateAfterEdit();
}

float Schedule::GetVal(int ctr) {
  if((size <  1) || (ctr < 0)) {
    cur_val = default_val;
    return default_val;
  }

  if(ctr == last_ctr)
    return cur_val;

  last_ctr = ctr;
  int i;
  for(i=0; i < size; i++) {
    SchedItem* itm = FastEl(i);
    if((ctr >= itm->start_ctr) && (ctr < (itm->start_ctr + itm->duration))) {
      if(i == size-1)
	cur_val = itm->start_val;
      else {
	if(interpolate)
	  cur_val = itm->GetVal(ctr);
	else
	  cur_val = itm->start_val;
      }
      return cur_val;
    }
  }
  cur_val = FastEl(size-1)->start_val;
  return cur_val;
}

#define DEFAULT_3D_NETSIZE_X 5
#define DEFAULT_3D_NETSIZE_Y 2
#define DEFAULT_3D_NETSIZE_Z 3
#define DEFAULT_3D_NETVIEW_SKEW .15

#define DEFAULT_2D_NETSIZE_X 5
#define DEFAULT_2D_NETSIZE_Y 12
#define DEFAULT_2D_NETSIZE_Z 0
#define DEFAULT_2D_NETVIEW_SKEW 0

////////////////////////////////////////
//	Connection - Groups & Specs   //
////////////////////////////////////////

void Connection::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
}

bool Connection::ChangeMyType(TypeDef*) {
  taMisc::Error("Cannot change type of connections -- change type setting in projection and reconnect network instead");
  return false;
}

void ConSpec::Initialize() {
  min_obj_type = &TA_Con_Group;
  min_con_type = &TA_Connection;
  rnd.type = Random::UNIFORM;
  rnd.mean = 0.0f;
  rnd.var = .5f;
}

void ConSpec::InitLinks() {
  BaseSpec::InitLinks();
  children.SetBaseType(&TA_ConSpec); // allow all of this general spec type to be created under here
  children.el_typ = GetTypeDef(); // but make the default to be me!
  taBase::Own(rnd,this);
  taBase::Own(wt_limits,this);
}

void ConSpec::Copy_(const ConSpec& cp) {
  //  min_con_type = cp.min_con_type;  // don't do this -- could be going between types
  rnd = cp.rnd;
  wt_limits = cp.wt_limits;
}

void ConSpec::UpdateAfterEdit() {
  BaseSpec::UpdateAfterEdit();
  if(!taMisc::is_loading) {
    if(UseCount() == 0) {
      if(!not_used_ok)
	taMisc::Error("Note: you just edited:",GetTypeDef()->name,GetPath(),"but it is not used anywhere!");
      not_used_ok = true;
    }
  }
}

bool ConSpec::CheckObjectType_impl(TAPtr obj) {
  if(obj->InheritsFrom(TA_Con_Group)) {
    if((obj->GetOwner() != NULL) && obj->GetOwner()->InheritsFrom(TA_Unit))
      return true;		// don't do checking on 1st con group in unit..
  }
  else if(obj->InheritsFrom(TA_Connection)) {
    if(!obj->InheritsFrom(min_con_type))
      return false;
    return true;
  }
  else if(obj->InheritsFrom(TA_Projection)) {
    if(!((Projection*)obj)->con_type->InheritsFrom(min_con_type))
      return false;
    return true;
  }
  return BaseSpec::CheckObjectType_impl(obj);
}

void ConSpec::ApplySymmetry(Con_Group* cg, Unit* ru) {
  if(!wt_limits.sym) return;
  Connection* rc, *su_rc;
  int i;
  for(i=0; i<cg->size;i++) {
    rc = cg->Cn(i);
    su_rc = cg->FindRecipRecvCon(cg->Un(i), ru);
    if(su_rc != NULL)
      su_rc->wt = rc->wt;	// set other's weight to ours (otherwise no random..)
  }
} 

void Con_Group::Initialize() {
  units.SetBaseType(&TA_Unit);
  prjn = NULL;
  other_idx = -1;
  own_cons = false;
  SetBaseType(&TA_Connection);
}

void Con_Group::InitLinks() {
  taBase_Group::InitLinks();
  spec.SetDefaultSpec(this);
  taBase::Own(units, this);
}  

void Con_Group::CutLinks() {
  units.CutLinks();
  taBase::DelPointer((TAPtr*)&prjn);
  spec.CutLinks();
  taBase_Group::CutLinks();
}

void Con_Group::Copy(const Con_Group& cp) {
  Copy_Common(cp);		// just copy values across common connections
  Copy_(cp);			// then copy our stuff.
  gp.Copy(cp.gp);		// copy the sub-groups
}

void Con_Group::Copy_(const Con_Group& cp) {
  if(!cp.name.empty())
    name = cp.name;
  el_base = cp.el_base;
  el_typ = cp.el_typ;
  el_def = cp.el_def;
  spec = cp.spec;

  // stuff that doesn't get copied!
  // taBase::SetPointer((TAPtr*)&prjn, cp.prjn);
  // other_idx = cp.other_idx;
  // own_cons = cp.own_cons;
}

//static TimeUsed debug_time;

void Con_Group::CopyNetwork(Network* net, Network* cn, Con_Group* cp) {
  MemberDef* md;
  String path;
  if(!cp->own_cons) {		// just a link (i.e., send group)
    if(size > 0) {
      Reset();			// get rid of any existing actual cons
      units.Reset();
    }
    // don't copy any elements --- need to get pointers to *our* connections via ReConnect_Load
    own_cons = cp->own_cons;
    other_idx = cp->other_idx;
  }
  else {			// we actually own them
    if(!own_cons && (size > 0)) {
      Reset();
    }
    own_cons = cp->own_cons;
    other_idx = cp->other_idx;
    units.Reset();		// always reset the units, cuz they're going to be wrong..
    taList_impl::Copy(*cp);	// do a standard copy of the connections only
    int i;
    for(i=0; i<cp->units.size; i++) {
      Unit* au = cp->Un(i);
      if(au != NULL) {
	Layer* au_lay = GET_OWNER(au,Layer);
	if(au_lay->own_net != NULL) {
	  int lidx = au_lay->own_net->layers.FindLeaf(au_lay);
	  int uidx = au_lay->units.FindLeaf(au);
	  if((lidx >= 0) && (uidx >= 0)) {
	    Layer* nw_lay = (Layer*)net->layers.Leaf(lidx);
	    if(nw_lay != NULL) {
	      Unit* nw_un = (Unit*)nw_lay->units.Leaf(uidx);
	      if(nw_un != NULL)
		units.Link(nw_un);
	    }
	  }
	}
      }
    }
  }
  if(cp->prjn != NULL) {
    path = cp->prjn->GetPath(NULL, cn);
    Projection* nw_prjn = (Projection*)net->FindFromPath(path, md);
    if(nw_prjn != NULL)
      taBase::SetPointer((TAPtr*)&prjn, nw_prjn);
  }
}

void Con_Group::CopyPtrs(Con_Group* cp) {
  if(!cp->own_cons) {		// just a link (i.e., send group)
    if(size > 0) {
      Reset();			// get rid of any existing actual cons
      units.Reset();
    }
    // don't copy any elements --- need to get pointers to *our* connections via ReConnect_Load
    own_cons = cp->own_cons;
    other_idx = cp->other_idx;
    units.Borrow(cp->units);
    Borrow(*cp);
  }
  else {			// we actually own them
    if(!own_cons && (size > 0)) {
      Reset();
    }
    own_cons = cp->own_cons;
    other_idx = cp->other_idx;
    units.Reset();
    taList_impl::Copy(*cp);	// do a standard copy of the connections only
    units.Borrow(cp->units);
  }
  taBase::SetPointer((TAPtr*)&prjn, cp->prjn);
}

bool Con_Group::ChangeMyType(TypeDef*) {
  taMisc::Error("Cannot change type of con_groups -- change type setting in projection and reconnect network instead");
  return false;
}

void Con_Group::UpdateAfterEdit() {
  taBase_Group::UpdateAfterEdit();
  spec.CheckSpec();
}

Connection* Con_Group::NewCon(TypeDef* typ, Unit* un) {
  units.Link(un);
  Connection* rval;
  if(size > units.size-1)	// connections were preallocated
    rval = Cn(units.size-1);
  else
    rval = (Connection*)taBase_Group::NewEl(1, typ);
  return rval;
}

void Con_Group::LinkCon(Connection* cn, Unit* un) {
  units.Link(un);
  if(size > units.size-1)	// connections were preallocated
    ReplaceLink(units.size-1, cn);
  else
    Link(cn);
}

bool Con_Group::RemoveCon(Unit* un) {
  int idx;
  if((idx = units.Find(un)) < 0)
    return false;
  return Remove(idx);
}

void Con_Group::AllocCon(int no, TypeDef* typ) {
  if(no <= 0) return;
  Alloc(no);
  taBase_Group::NewEl(no, typ);	// subvert the redefinition, call old version of New
}

Connection* Con_Group::FindConFrom(Unit* un, int& idx) const {
  if((idx = units.Find(un)) < 0)
    return NULL;
  return (Connection*)SafeEl(idx);
}

void Con_Group::Alloc(int sz) {
  units.Alloc(sz);
  taBase_Group::Alloc(sz);
}

TAPtr Con_Group::New(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  if(typ == NULL)
    typ = el_typ;

  TAPtr rval = taBase_Group::New(no, typ);

  if(!typ->InheritsFrom(&TA_taGroup_impl)) {
    int i;
    for(i=0; i <no; i++)
      units.Add(NULL);
  }
  return rval;
}
TAPtr Con_Group::NewEl(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  int i;
  for(i=0; i <no; i++)
    units.Add(NULL);
  return taBase_Group::NewEl(no, typ);
}

bool Con_Group::Remove(int i) {
  units.Remove(i);
  return taBase_Group::Remove(i);
}

Con_Group* Con_Group::NewPrjn(Projection* aprjn, bool own) {
  Con_Group* rval = (Con_Group*)NewGp(1, aprjn->con_gp_type);
  taBase::SetPointer((TAPtr*)&(rval->prjn), aprjn);
  spec = aprjn->con_spec;	// set our spec to this one too..
  rval->el_typ = aprjn->con_type; // set type of connection to this type..
  el_typ = aprjn->con_type;	// ditto
  rval->spec = aprjn->con_spec;
  rval->own_cons = own;
  return rval;
}

Con_Group* Con_Group::FindPrjn(Projection* aprjn, int& idx) const {
  int g;
  for(g=0; g<gp.size; g++) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if(cg->prjn == aprjn) {
      idx = g;
      return cg;
    }
  }
  idx = -1;
  return NULL;
}

Con_Group* Con_Group::FindFrom(Layer* from, int& idx) const {
  int g;
  for(g=0; g<gp.size; g++) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if((cg->prjn != NULL) && (cg->prjn->from == from)) {
      idx = g;
      return cg;
    }
  }
  idx = -1;
  return NULL;
}

Con_Group* Con_Group::FindTypeFrom(TypeDef* prjn_td, Layer* from, int& idx) const {
  int g;
  for(g=0; g<gp.size; g++) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if((cg->prjn != NULL) && (cg->prjn->from == from) &&
       (cg->prjn->InheritsFrom(prjn_td)))
    {
      idx = g;
      return cg;
    }
  }
  idx = -1;
  return NULL;
}


Con_Group* Con_Group::FindLayer(Layer* lay, int& idx) const {
  int g;
  for(g=0; g<gp.size; g++) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if((cg->prjn != NULL) && (cg->prjn->layer == lay)) {
      idx = g;
      return cg;
    }
  }
  idx = -1;
  return NULL;
}

bool Con_Group::RemovePrjn(Projection* aprjn) {
  bool rval = false;
  int g;
  for(g=gp.size-1; g>=0; g--) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if(cg->prjn == aprjn) {
      cg->prjn->projected = false;
      gp.Remove(cg); 
      rval = true;
    }
  }
  return rval;
}

bool Con_Group::RemoveFrom(Layer* from) {
  bool rval = false;
  int g;
  for(g=gp.size-1; g>=0; g--) {
    Con_Group* cg = (Con_Group*)gp.FastEl(g);
    if((cg->prjn != NULL) && (cg->prjn->from == from)) {
      cg->prjn->projected = false;
      gp.Remove(cg); 
      rval = true;
    }
  }
  return rval;
}

Connection* Con_Group::FindRecipRecvCon(Unit* su, Unit* ru) {
  Layer* my_lay = GET_OWNER(ru,Layer);
  int g;
  for(g=0; g<su->recv.gp.size; g++) {
    Con_Group* cg = (Con_Group*)su->recv.gp.FastEl(g);
    if((cg->prjn == NULL) && (cg->prjn->from != my_lay)) continue;
    Connection* con = cg->FindConFrom(ru);
    if(con != NULL) return con;
  }
  return NULL;
}

Connection* Con_Group::FindRecipSendCon(Unit* ru, Unit* su) {
  Layer* my_lay = GET_OWNER(su,Layer);
  int g;
  for(g=0; g<ru->send.gp.size; g++) {
    Con_Group* cg = (Con_Group*)ru->send.gp.FastEl(g);
    if((cg->prjn == NULL) && (cg->prjn->layer != my_lay)) continue;
    Connection* con = cg->FindConFrom(su);
    if(con != NULL) return con;
  }
  return NULL;
}

void Con_Group::Copy_Weights(const Con_Group* src) {
  int mx = MIN(size, src->size);
  int i;
  for(i=0; i < mx; i++)
    Cn(i)->wt = src->Cn(i)->wt;
}

void Con_Group::WriteWeights(ostream& strm, Unit*, Con_Group::WtSaveFormat fmt) {
  // this saves writing weights for recv-based linked weights
  if(!own_cons) return;
  int i;
  switch(fmt) {
  case Con_Group::TEXT:
    strm << "#Size " << size << "\n";
    for(i=0; i < size; i++) strm << Cn(i)->wt << "\n";
    break;
  case Con_Group::TEXT_IDX:
    if((prjn == NULL) || (prjn->from == NULL)) {
      taMisc::Error("*** Error in WriteWeights::BINARY_IDX: NULL prjn or prjn->from");
      return;
    }
    strm << "#Size " << size << "\n";
    for(i=0; i < size; i++) {
      int lidx = prjn->from->units.FindLeafEl(Un(i));
      if(lidx < 0) {
	lidx = 0;
	taMisc::Error("*** Error in WriteWeights::BINARY_IDX: can't find unit in connection: ", String(i),
		      "in layer:", prjn->layer->name);
      }
      strm << lidx << " " << Cn(i)->wt << "\n";
    }
    break;
  case Con_Group::BINARY:
    strm << size << "\n";
    for(i=0; i < size; i++) strm.write((char*)&(Cn(i)->wt), sizeof(Cn(i)->wt));
    break;
  case Con_Group::BINARY_IDX:
    if((prjn == NULL) || (prjn->from == NULL)) {
      taMisc::Error("*** Error in WriteWeights::BINARY_IDX: NULL prjn or prjn->from");
      return;
    }
    strm << size << "\n";
    for(i=0; i < size; i++) {
      int lidx = prjn->from->units.FindLeafEl(Un(i));
      if(lidx < 0) {
	lidx = 0;
	taMisc::Error("*** Error in WriteWeights::BINARY_IDX: can't find unit in connection: ", String(i),
		      "in layer:", prjn->layer->name);
      }
      strm.write((char*)&(lidx), sizeof(lidx));
      strm.write((char*)&(Cn(i)->wt), sizeof(Cn(i)->wt));
    }
    break;
  }
}

static int con_group_read_weights_text_size(istream& strm, int size) {
  int sz = size;
  int c = strm.peek();
  while(c == '#') {
    strm.get();
    c = taMisc::read_alnum_noeol(strm);
    if(c == EOF) break;
    if(taMisc::LexBuf == "Size") {
      c = taMisc::read_till_eol(strm);
      sz = (int)taMisc::LexBuf;
      break;
    }
    else {
      c = taMisc::read_till_eol(strm);
    }
    c = strm.peek();
  }
  if(c == EOF) sz = 0;
  if(sz < 0) {
    taMisc::Error("*** Con_Group::ReadWeights: Error in reading weights -- read size < 0");
  }
  return sz;
}

static bool con_group_read_weights_idx_con(Con_Group* ths, int& i, Unit* ru, int lidx ) {
  Unit* su = ths->prjn->from->units.Leaf(lidx);
  if(su == NULL) {
    taMisc::Error("*** Error in ReadWeights::_IDX: unit at leaf index: ",
		  String(lidx), "not found in layer:", ths->prjn->from->name);
    i--; ths->EnforceSize(ths->size-1);
    ru->n_recv_cons--;
    return false;
  }
  Con_Group* send_gp = (Con_Group*)su->send.SafeGp(ths->prjn->send_idx);
  if(send_gp == NULL) {
    taMisc::Error("*** Error in ReadWeights::_IDX: unit at leaf index: ",
		  String(lidx), "does not have proper send group:", String(ths->prjn->send_idx));
    i--; ths->EnforceSize(ths->size-1);
    ru->n_recv_cons--;
    return false;
  }	  
  if(ths->units.size <= i) {
    if(su == NULL) su = ths->prjn->from->units.Leaf(0);
    ths->units.Link(su);
    int sidx = send_gp->units.Find(ru);
    if(sidx >= 0) {
      send_gp->ReplaceLink(sidx, ths->Cn(i));
    }
    else {
      send_gp->LinkCon(ths->Cn(i), ru);
    }
  }
  else if(su != ths->Un(i)) {
    ths->units.ReplaceLink(i, su);
    int sidx = send_gp->units.Find(ru);
    if(sidx >= 0) {
      send_gp->ReplaceLink(sidx, ths->Cn(i));
    }
    else {
      send_gp->LinkCon(ths->Cn(i), ru);
    }
  }
  return true;
}

void Con_Group::ReadWeights(istream& strm, Unit* ru, Con_Group::WtSaveFormat fmt) {
  // this saves writing weights for recv-based linked weights
  if(!own_cons) return;
  int i;
  switch(fmt) {
  case Con_Group::TEXT:
    {
      int sz = con_group_read_weights_text_size(strm, size);
      if(sz <= 0) return ;	// zero size likely = DMEM save (or EOF) so just skip it!
      if(sz != size) {
	taMisc::Error("*** Error in reading weights: read size (", String(sz),
		      ") != current size (", String(size), ")");
      }
      int minsz = MIN(sz, size);
      for(i=0; i < minsz; i++) {
	int c = taMisc::read_alnum_noeol(strm);
	if(c == EOF) break;
	if(taMisc::LexBuf.empty()) {
	  i--; continue;		// re-read it
	}
	if(taMisc::LexBuf.firstchar() == '#') { // skip comments
	  if(c != '\n')
	    taMisc::read_till_eol(strm); // bag rest of line
	  i--;			// re-read this one
	  continue;
	}
	Cn(i)->wt = (float)taMisc::LexBuf;
      }
      if(sz > size) {
	for(;i < sz; i++) {
	  int c = taMisc::read_alnum_noeol(strm);
	  if(c == EOF) break;
	  if(taMisc::LexBuf.empty()) {
	    i--; continue;		// re-read it
	  }
	  if(taMisc::LexBuf.firstchar() == '#') { // skip comments
	    if(c != '\n')
	      taMisc::read_till_eol(strm); // bag rest of line
	    i--;			// re-read this one
	    continue;
	  }
	}
      }
    }
    break;
  case Con_Group::TEXT_IDX:
    {
      int sz = con_group_read_weights_text_size(strm, size);
      if(sz <= 0) return ;	// zero size likely = DMEM save (or EOF) so just skip it!
      ru->n_recv_cons += sz - size;
      EnforceSize(sz);
      for(i=0; i < size; i++) {
	int c = taMisc::read_alnum_noeol(strm);
	if(c == EOF) break;
	if(taMisc::LexBuf.empty()) {
	  i--; continue;		// re-read it
	}
	if(taMisc::LexBuf.firstchar() == '#') { // skip comments
	  if(c != '\n')
	    taMisc::read_till_eol(strm); // bag rest of line
	  i--;			// re-read this one
	  continue;
	}
	int lidx = (int)taMisc::LexBuf;
	c = taMisc::read_till_eol(strm);
	Cn(i)->wt = (float)taMisc::LexBuf;

	con_group_read_weights_idx_con(this, i, ru, lidx);
      }
    }
    break;
  case Con_Group::BINARY:
    {
      int sz;
      strm >> sz;  strm.get();
      if(sz == 0) return;	// zero size likely = DMEM save so just skip it!
      if(sz < 0) {
	taMisc::Error("*** Error in reading weights: read size < 0");
	return;
      }
      if(sz != size) {
	taMisc::Error("*** Error in reading weights: read size (", String(sz),
		      ") != current size (", String(size), ")");
      }
      int minsz = MIN(sz, size);
      for(i=0; i < minsz; i++) strm.read((char*)&(Cn(i)->wt), sizeof(Cn(i)->wt));
      if(sz > size) {
	float dumy;
	for(;i<sz;i++) strm.read((char*)&(dumy), sizeof(dumy));
      }
    }
    break;
  case Con_Group::BINARY_IDX:
    {
      if((prjn == NULL) || (prjn->from == NULL)) {
	taMisc::Error("*** Error in ReadWeights::BINARY_IDX: NULL prjn or prjn->from in WriteWeights::BINARY_IDX");
	return;
      }
      int sz;
      strm >> sz;  strm.get();
      if(sz == 0) return;	// zero size likely = DMEM save so just skip it!
      if(sz < 0) {
	taMisc::Error("*** Error in ReadWeights::BINARY_IDX: read size < 0");
	return;
      }
      ru->n_recv_cons += sz - size;
      EnforceSize(sz);
      for(i=0; i < size; i++) {
	int lidx;
	strm.read((char*)&(lidx), sizeof(lidx));
	strm.read((char*)&(Cn(i)->wt), sizeof(Cn(i)->wt));

	con_group_read_weights_idx_con(this, i, ru, lidx);
      }
    }
    break;
  }
}

void Con_Group::TransformWeights(const SimpleMathSpec& trans) {
  int i;
  for(i=0; i < size; i++)
    Cn(i)->wt = trans.Evaluate(Cn(i)->wt);
}

void Con_Group::AddNoiseToWeights(const Random& noise_spec) {
  int i;
  for(i=0; i < size; i++)
    Cn(i)->wt += noise_spec.Gen();
}

int Con_Group::PruneCons(Unit* un, const SimpleMathSpec& pre_proc,
			    CountParam::Relation rel, float cmp_val)
{
  CountParam cond;
  cond.rel = rel; cond.val = cmp_val;
  int rval = 0;
  int j;
  for(j=size-1; j>=0; j--) {
    if(cond.Evaluate(pre_proc.Evaluate(Cn(j)->wt))) {
      un->DisConnectFrom(Un(j), prjn);
      rval++;
    }
  }
  return rval;
}

int Con_Group::LesionCons(Unit* un, float p_lesion, bool permute) {
  int rval = 0;
  if(permute) {
    rval = (int) (p_lesion * (float)size);
    if(rval == 0) return 0;
    int_Array ary;
    int j;
    for(j=0; j<size; j++)
      ary.Add(j);
    ary.Permute();
    ary.size = rval;
    ary.Sort();
    for(j=ary.size-1; j>=0; j--)
      un->DisConnectFrom(Un(ary.FastEl(j)), prjn);
  }
  else {
    int j;
    for(j=size-1; j>=0; j--) {
      if(Random::ZeroOne() <= p_lesion) {
	un->DisConnectFrom(Un(j), prjn);
	rval++;
      }
    }
  }
  return rval;
}

void Con_Group::ConValuesToArray(float_RArray& ary, const char* variable) {
  MemberDef* md = el_typ->members.FindName(variable);
  if((md == NULL) || !md->type->InheritsFrom(TA_float)) {
    taMisc::Error("*** Variable:", variable, "not found or not a float on units of type:",
		   el_typ->name, "in ConValuesToArray()");
    return;
  }
  int i;
  for(i=0; i<size; i++) {
    float* val = (float*)md->GetOff((void*)Cn(i));
    ary.Add(*val);
  }
}

void Con_Group::ConValuesFromArray(float_RArray& ary, const char* variable) {
  MemberDef* md = el_typ->members.FindName(variable);
  if((md == NULL) || !md->type->InheritsFrom(TA_float)) {
    taMisc::Error("*** Variable:", variable, "not found or not a float on units of type:",
		   el_typ->name, "in ConValuesToArray()");
    return;
  }
  int mx = MIN(size, ary.size);
  int i;
  for(i=0; i<mx; i++) {
    float* val = (float*)md->GetOff((void*)Cn(i));
    *val = ary[i];
  }
}

int Con_Group::ReplaceConSpec(ConSpec* old_sp, ConSpec* new_sp) {
  if(spec.spec != old_sp) return 0;
  spec.SetSpec(new_sp);
  return 1;
}

bool Con_Group::CheckTypes() {
  if(!spec.CheckSpec())
    return false;
  if((prjn == NULL) && (size > 0)) {
    taMisc::Error("ConGroup:",GetPath(),"has null projection!, do Connect All");
    return false;
  }
  if((size > 0) && (other_idx < 0)) {
    taMisc::Error("ConGroup:", GetPath(), "has unset other_idx, do FixPrjnIndexes or Connect All");
    return false;
  }
  return true;
}

bool Con_Group::CheckOtherIdx_Recv() {
  if(size == 0) return true;
  if((other_idx < 0) || (other_idx != prjn->send_idx)) {
    taMisc::Error("recv ConGroup:", GetPath(), "has unset other_idx or doesn't match prjn, do FixPrjnIndexes or Connect All");
    return false;
  }
  Unit* su = Un(0);
  if(su->send.gp.size <= other_idx) {
    taMisc::Error("recv ConGroup:", GetPath(), "other_idx", String(other_idx),
		  "is out of range on sending unit. Do Actions/Remove Cons, then Build, Connect on Network");
    return false;
  }
  Con_Group* sucg = (Con_Group*)su->send.FastGp(other_idx);
  if(sucg->prjn != prjn) {
    taMisc::Error("recv ConGroup:", GetPath(), "other_idx", String(other_idx),
		  "doesn't have correct prjn on sending unit.  Do Actions/Remove Cons, then Build, Connect on Network");
    return false;
  }
  return true;
}

bool Con_Group::CheckOtherIdx_Send() {
  if(size == 0) return true;
  if((other_idx < 0) || (other_idx != prjn->recv_idx)) {
    taMisc::Error("send ConGroup:", GetPath(), "has unset other_idx or doesn't match prjn, do FixPrjnIndexes or Connect All");
    return false;
  }
  Unit* ru = Un(0);
  if(ru->recv.gp.size <= other_idx) {
    taMisc::Error("send ConGroup:", GetPath(), "other_idx", String(other_idx),
		  "is out of range on recv unit. Do Actions/Remove Cons, then Build, Connect on Network");
    return false;
  }
  Con_Group* rucg = (Con_Group*)ru->recv.FastGp(other_idx);
  if(rucg->prjn != prjn) {
    taMisc::Error("send ConGroup:", GetPath(), "other_idx", String(other_idx),
		  "doesn't have correct prjn on recv unit.  Do Actions/Remove Cons, then Build, Connect on Network");
    return false;
  }
  return true;
}

// only dump sub-group stuff
int Con_Group::Dump_Save_PathR(ostream& strm, TAPtr par, int indent) {
  return gp.Dump_Save_PathR(strm, par, indent);
}
int Con_Group::Dump_SaveR(ostream& strm, TAPtr par, int indent) {
  return gp.Dump_SaveR(strm, par, indent);
}

// have to implement after save_value because we're not saving a real
// path that can be loaded with Load

int Con_Group::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  if(size == 0)			// don't own any if you don't got 'em
    own_cons = false;		// and, own_cons is used as a flag for loading..
  int rval = taBase_Group::Dump_Save_Value(strm, par, indent); // first dump members
  if((size == 0) || !own_cons || !rval || (prjn == NULL))
    return rval;

  // close off the regular members
  taMisc::indent(strm, indent,1) << "};\n";

  // output the units
  taMisc::indent(strm, indent, 1) << "{ units = {";

  int i;
  for(i=0; i<size; i++) {
    String un_path = Un(i)->GetPath(NULL, &(prjn->from->units));
    if(un_path == "root") {
      taMisc::Error("*** Units in projection:", prjn->name,"in layer:",prjn->layer->name,
		    "have a null path.  Do ConnectAll and save again.");
    }
    strm << un_path << "; ";
    if(((i+1) % 8) == 0)
      strm << "\n";
  }
  strm << "};\n";

  // output the connection values
  TypeDef* ctd = Cn(0)->GetTypeDef();
  int j;
  for(j=0; j<ctd->members.size; j++) {
    MemberDef* md = ctd->members.FastEl(j);
    if((md->type->ptr > 0) || (md->HasOption("NO_SAVE")))
      continue;
    taMisc::indent(strm, indent+1,1) << md->name << " = {";
    for(i=0; i<size; i++) {
      strm << md->type->GetValStr(md->GetOff((void*)Cn(i))) << "; ";
      if((((i+1) % 8) == 0) && (i < (size-1)))
	strm << "\n";
    }
    strm << "};\n";
  }
  return true;
}

int Con_Group::Dump_Load_Value(istream& strm, TAPtr) {
  int rval = taBase_Group::Dump_Load_Value(strm); // first dump members
  if(!own_cons || (rval == EOF) || (rval == 2))
    return rval;

  int c = taMisc::read_till_lbracket(strm);	// get past opening bracket
  if(c == EOF) return EOF;
  c = taMisc::read_word(strm);
  if(taMisc::LexBuf != "units") {
    taMisc::Error("*** Con_Group Expecting: 'units' in load file, got:",
		    taMisc::LexBuf,"instead");
    return false;
  }
  // skip =
  c = taMisc::skip_white(strm);
  if(c != '=') {
    taMisc::Error("*** Missing '=' in dump file for unit in Con_Group");
    return false;
  }
  // skip {
  c = taMisc::skip_white(strm);
  if(c != '{') {
    taMisc::Error("*** Missing '{' in dump file for unit in Con_Group");
    return false;
  }

  // prjn has to be loaded in advance of this!
  // (sender-based must have send_prjns own projection, else projection owns it)
  if((prjn == NULL) || (prjn->from == NULL) || (prjn->layer == NULL)) {
    taMisc::Error("*** projection or ->from or ->layer is NULL");
    return false;
  }
  Unit_Group* ug = &(prjn->from->units);

  TypeDef* ctd = prjn->con_type;

  // now read them all in..
  int c_count = 0;		// number of connections
  while(true) {
    c = taMisc::read_till_rb_or_semi(strm);
    if(c == EOF) return EOF;
    if(c == '}') break;

    // find ptr (optimized to search relative to ug_md..
    MemberDef* md;
    Unit* un = (Unit*)ug->FindFromPath(taMisc::LexBuf, md);
    if(un == NULL) {
      taMisc::Error("*** Connection unit not found:", taMisc::LexBuf,
		      "in connection type", ctd->name);
      c = taMisc::skip_past_err(strm); // scan past the error
      if(c == '}') break;
      continue;
    }
    else {
      if(units.size > c_count)
	units.ReplaceLink(c_count, un);
      else {
	units.Link(un);
	if(size < units.size)
	  Add(taBase::MakeToken(ctd)); // keep synced..
      }
    }
    c_count++;
  }

  // now read in the values
  while(true) {
    c = taMisc::read_word(strm);
    if(c == EOF) return EOF;
    if(c == '}') {
      if(strm.peek() == ';') strm.get(); // get the semi
      break;		// done
    }
    MemberDef* md = ctd->members.FindName(taMisc::LexBuf);
    if(md == NULL) {
      taMisc::Error("*** Connection member not found:", taMisc::LexBuf,
		      "in connection type", ctd->name);
      c = taMisc::skip_past_err(strm);
      if(c == '}') break;
      continue;
    }
    // skip =
    c = taMisc::skip_white(strm);
    if(c != '=') {
      taMisc::Error("*** Missing '=' in dump file for unit in Con_Group");
      c = taMisc::skip_past_err(strm);
      continue;
    }
    // skip {
    c = taMisc::skip_white(strm);
    if(c != '{') {
      taMisc::Error("*** Missing '{' in dump file for unit in Con_Group");
      c = taMisc::skip_past_err(strm);
      continue;
    }

    int i = 0;
    while(true) {
      c = taMisc::read_till_rb_or_semi(strm);
      if(c == EOF) return EOF;
      if(c == '}') break;
      if(i >= size) {
	c = taMisc::skip_past_err_rb(strm); // bail to ending rb
	break;
      }
      Connection* cn = Cn(i);
      md->type->SetValStr(taMisc::LexBuf, md->GetOff((void*)cn));
      i++;
    }
  }      
  return true;
}

////////////////////////
//	Unit	      //
////////////////////////

void UnitSpec::Initialize() {
  min_obj_type = &TA_Unit;
  act_range.max = 1.0f; act_range.min = 0.0f;
  act_range.UpdateAfterEdit();
  bias_con_type = NULL;
}

void UnitSpec::InitLinks() {
  BaseSpec::InitLinks();
  children.SetBaseType(&TA_UnitSpec); // allow all of this general spec type to be created under here
  children.el_typ = GetTypeDef(); // but make the default to be me!
  taBase::Own(act_range, this);
  taBase::Own(bias_spec, this);
  bias_spec.owner = this;	// set the owner cuz setdefaultspec won't necc. do so.
  // don't do this if loading -- creates specs in all the wrong places..
  // specs that own specs have this problem
  if(!taMisc::is_loading)
    bias_spec.SetDefaultSpec(this);
}

void UnitSpec::Copy_(const UnitSpec& cp) {
  act_range = cp.act_range;
  bias_con_type = cp.bias_con_type;
  bias_spec = cp.bias_spec;
}

bool UnitSpec::CheckConfig(Unit* un, Layer* lay, TrialProcess* tp, bool quiet) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, un->recv., g) {
    if(!recv_gp->CheckConfig(lay, un, tp, quiet)) return false;
  }
  return true;
}

void UnitSpec::InitState(Unit* u) {
  u->InitExterns();
  u->InitDelta();
  u->act = 0.0f;
}

void UnitSpec::InitWtDelta(Unit* u) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtDelta(u);
  }
  if(u->bias != NULL)
    bias_spec->C_InitWtDelta(NULL, u->bias, u, NULL); // this is a virtual fun
}

void UnitSpec::InitWtState(Unit* u) {
#ifdef DMEM_COMPILE
  if(!u->DMem_IsLocal()) {
    // make up for random numbers not being used for non-local connections.
    for(int i=0; i<u->n_recv_cons; i++) Random::ZeroOne();
  }
  else
#endif
    {
      Con_Group* recv_gp;
      int g;
      FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
	// ignore lesion here because n_recv_cons does not take into account lesioned layers, so dmem would get out of sync
	//    if(!recv_gp->prjn->from->lesion)
	recv_gp->InitWtState(u);
      }
    }

  if(u->bias != NULL) {
    bias_spec->C_InitWtState(NULL, u->bias, u, NULL); // this is a virtual fun
    bias_spec->C_InitWtDelta(NULL, u->bias, u, NULL); // don't forget delta too!!
  }
}

void UnitSpec::InitWtState_post(Unit* u) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtState_post(u);
  }
}

void UnitSpec::Compute_Net(Unit* u) {
  Con_Group* recv_gp;
  u->net = 0.0f;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      u->net += recv_gp->Compute_Net(u);
  }
  if(u->bias != NULL)
    u->net += u->bias->wt;
}

void UnitSpec::Send_Net(Unit* u) {
  // typically the whole point of using sender based net input is that you want to check
  // here if the sending unit's activation (i.e., this one) is above some threshold
  // so you don't send if it isn't above that threshold..  this isn't implemented here though.
  Con_Group* send_gp;
  int g;
  FOR_ITR_GP(Con_Group, send_gp, u->send., g) {
    Layer* tol = send_gp->prjn->layer;
    if(!tol->lesion)
      send_gp->Send_Net(u);
  }
  if(u->bias != NULL)
    u->net += u->bias->wt;
}

void UnitSpec::Send_NetToLay(Unit* u, Layer* tolay) {
  // typically the whole point of using sender based net input is that you want to check
  // here if the sending unit's activation (i.e., this one) is above some threshold
  // so you don't send if it isn't above that threshold..  this isn't implemented here though.
  Con_Group* send_gp;
  int g;
  FOR_ITR_GP(Con_Group, send_gp, u->send., g) {
    Layer* tol = send_gp->prjn->layer;
    if(!tol->lesion && (tol == tolay))
      send_gp->Send_Net(u);
  }
  if(u->bias != NULL)
    u->net += u->bias->wt;
}

void UnitSpec::Compute_Act(Unit* u) {
  if(u->ext_flag & Unit::EXT)
    u->act = u->ext;
  else
    u->act = u->net;
}

void UnitSpec::Compute_dWt(Unit* u) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->Compute_dWt(u);
  }
  // NOTE: derived classes must supply bias->Compute_dWt call because C_Compute_dWt 
  // is not virtual, so if called here, only ConSpec version would be called. 
  // This is not true of InitWtState and InitWtDelta, which are virtual.
}

void UnitSpec::UpdateWeights(Unit* u) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->UpdateWeights(u);
  }
  // NOTE: derived classes must supply bias->UpdateWeights call because C_UpdateWeights
  // is not virtual, so if called here, only ConSpec version would be called. 
  // This is not true of InitWtState and InitWtDelta, which are virtual.
}


/////// Unit ///////

void Unit::Initialize() {
  recv.SetBaseType(&TA_Connection);
  send.SetBaseType(&TA_Connection);
  bias = NULL;
  ext_flag = NO_EXTERNAL;
  targ = 0.0f;
  ext = 0.0f;
  act = 0.0f;
  net = 0.0f;
  n_recv_cons = 0;
}

void Unit::Destroy() {
  CutLinks();
}

void Unit::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(recv, this);	// always own your constitutents
  taBase::Own(send, this);
  taBase::Own(pos, this);
  spec.SetDefaultSpec(this);
  Build();
  Layer* lay = GET_MY_OWNER(Layer);
  if(lay && !taMisc::is_loading)
    lay->LayoutUnits(this);
}

void Unit::CutLinks() {
  recv.CutLinks();
  send.CutLinks();
  taBase::DelPointer((TAPtr*)&bias);
  spec.CutLinks();
  if((owner != NULL) && taMisc::iv_active && (pos.z >= 0)) { // if z = -1, then don't update display
    Layer* lay = ((Unit_Group*)owner)->own_lay;
    owner = NULL;
    if((lay != NULL) && (lay->own_net != NULL) && !lay->own_net->net_will_updt)
      lay->own_net->InitAllViews();
  }
  taNBase::CutLinks();
}

void Unit::Copy_(const Unit& cp) {
  if((bias != NULL) && (cp.bias != NULL))
    *bias = *(cp.bias);
  spec = cp.spec;
  pos = cp.pos;
  ext_flag = cp.ext_flag;
  targ = cp.targ;
  ext = cp.ext;
  act = cp.act;
  net = cp.net;
  recv = cp.recv;
  send = cp.send;
  n_recv_cons = cp.n_recv_cons;
}

void Unit::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  spec.CheckSpec();
  // this updates the unit's representation int the netviews
  // no negative positions
  pos.x = MAX(0,pos.x); pos.y = MAX(0,pos.y);  pos.z = MAX(0,pos.z);
  // stay within layer->geom
  Layer* lay = GET_MY_OWNER(Layer);
  if(lay == NULL) return;
  pos.x = MIN(lay->geom.x-1,pos.x); pos.y = MIN(lay->geom.y-1,pos.y);
  pos.z = MIN(lay->geom.z-1,pos.z);
  if(taMisc::is_loading || !taMisc::iv_active) return;
  Network* mynet = GET_MY_OWNER(Network);
  if(mynet == NULL)	return;
  NetView* view;
  taLeafItr i;
  FOR_ITR_EL(NetView, view, mynet->views., i) {
    if(view->display_toggle)
      view->FixUnit(this,lay);
  }
}

#ifdef DMEM_COMPILE
int Unit::dmem_this_proc = 0;
#endif

void Unit::ReplacePointersHook(TAPtr old) {
  Unit* ou = (Unit*)old;
  CopyPtrs(ou);
  // now go through and replace all pointers to unit
  Network* own_net = GET_MY_OWNER(Network);
  if(own_net != NULL) {
    Layer* l;
    taLeafItr li;
    FOR_ITR_EL(Layer, l, own_net->layers., li) {
      Unit* u;
      taLeafItr ui;
      FOR_ITR_EL(Unit, u, l->units., ui) {
	if(u == ou) continue;
	int g;
	for(g=0; g < u->recv.gp.size; g++) {
	  Con_Group* cg = (Con_Group*)u->recv.gp.FastEl(g);
	  int i;
	  for(i=0;i<cg->units.size;i++) {
	    if(cg->Un(i) == ou)
	      cg->units.ReplaceLink(i, this);
	  }
	}
	for(g=0; g < u->send.gp.size; g++) {
	  Con_Group* cg = (Con_Group*)u->send.gp.FastEl(g);
	  int i;
	  for(i=0;i<cg->units.size;i++) {
	    if(cg->Un(i) == ou)
	      cg->units.ReplaceLink(i, this);
	  }
	}
      }
    }
  }
  ou->recv.Reset();		// get rid of old connectivity
  ou->send.Reset();
  taNBase::ReplacePointersHook(old);
}

void Unit::CopyNetwork(Network* anet, Network* cn, Unit* cp) {
  int g;
  for(g=0; g<recv.gp.size && g<cp->recv.gp.size; g++) {
    ((Con_Group*)recv.gp.FastEl(g))->CopyNetwork(anet, cn, (Con_Group*)cp->recv.gp.FastEl(g));
  }
  for(g=0; g<send.gp.size && g<send.gp.size; g++) {
    ((Con_Group*)send.gp.FastEl(g))->CopyNetwork(anet, cn, (Con_Group*)cp->send.gp.FastEl(g));
  }
}

void Unit::CopyPtrs(Unit* cp) {
  int g;
  for(g=0; g<recv.gp.size && g<cp->recv.gp.size; g++) {
    ((Con_Group*)recv.gp.FastEl(g))->CopyPtrs((Con_Group*)cp->recv.gp.FastEl(g));
  }
  for(g=0; g<send.gp.size && g<send.gp.size; g++) {
    ((Con_Group*)send.gp.FastEl(g))->CopyPtrs((Con_Group*)cp->send.gp.FastEl(g));
  }
}

// the 'New' function is used during loading to create objects if they are
// not of the correct type, this will accomplish that
TAPtr Unit::New(int, TypeDef* type) {
  if(!type->InheritsFrom(TA_Connection))
    return NULL;
  Connection* new_bias = (Connection*)taBase::MakeToken(type);
  taBase::SetPointer((TAPtr*)&bias, new_bias);
  return new_bias;
}

bool Unit::Build() {
  bool rval = false;
  if(spec.spec == NULL)
    return false;
  TypeDef* bstd = spec->bias_con_type;
  if(bstd == NULL) {
    if(bias != NULL) {
      taBase::DelPointer((TAPtr*)&bias);
      rval = true;
    }
  }
  else {
    if((bias == NULL) || (bias->GetTypeDef() != bstd)) {
      rval = true;
      Connection* new_bias = (Connection*)taBase::MakeToken(bstd);
      taBase::SetPointer((TAPtr*)&bias, new_bias);
    }
  }
  return rval;
}

bool Unit::CheckBuild() {
  if(spec.spec == NULL)	return false;
  if(spec->bias_con_type == NULL) {
    if(bias != NULL)
      return true;
  }
  else {
    if((bias == NULL) || (bias->GetTypeDef() != spec->bias_con_type))
      return true;
  }
  return false;
}

bool Unit::CheckTypes() {
  if(!spec.CheckSpec())
    return false;
  if(spec->bias_con_type != NULL) {
    if(!spec->bias_spec.CheckSpec(bias))
      return false;
  }
  Con_Group* cg;
  int i;
  FOR_ITR_GP(Con_Group, cg, recv., i) {
    if(!cg->CheckTypes()) return false;
  }
  FOR_ITR_GP(Con_Group, cg, send., i) {
    if(!cg->CheckTypes()) return false;
  }
  return true;
}

void Unit::RemoveCons() {
  recv.RemoveAll();		// blunt, but effective
  send.RemoveAll();
  n_recv_cons = 0;
}

bool Unit::SetConSpec(ConSpec* sp) {
  if(sp == NULL)	return false;
  int g;
  for(g=0; g<recv.gp.size; g++) {
    Con_Group* recv_gp = (Con_Group*)recv.gp.FastEl(g);
    if(sp->CheckObjectType(recv_gp))
      recv_gp->spec.SetSpec(sp);
    else
      return false;
  }
  return true;
}

int Unit::ReplaceUnitSpec(UnitSpec* old_sp, UnitSpec* new_sp) {
  if(spec.spec != old_sp) return 0;
  spec.SetSpec(new_sp);
  return 1;
}

int Unit::ReplaceConSpec(ConSpec* old_sp, ConSpec* new_sp) {
  int nchg = 0;
  int g;
  nchg += recv.ReplaceConSpec(old_sp, new_sp);
  for(g=0; g<recv.gp.size; g++) {
    Con_Group* recv_gp = (Con_Group*)recv.gp.FastEl(g);
    nchg += recv_gp->ReplaceConSpec(old_sp, new_sp);
  }
  nchg += send.ReplaceConSpec(old_sp, new_sp);
  for(g=0; g<send.gp.size; g++) {
    Con_Group* send_gp = (Con_Group*)send.gp.FastEl(g);
    nchg += send_gp->ReplaceConSpec(old_sp, new_sp);
  }
  return nchg;
}

Con_Group* Unit::rcg_rval = NULL;
Con_Group* Unit::scg_rval = NULL;

void Unit::ConnectAlloc(int no, Projection* prjn, Con_Group*& cgp) {
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal() && !prjn->con_spec->DMem_AlwaysLocal()) return;
#endif
  if((prjn->recv_idx < 0) || ((cgp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL)) {
    cgp = recv.NewPrjn(prjn, true); // owns the connections
    prjn->recv_idx = recv.gp.size-1;
  }
  cgp->AllocCon(no, cgp->prjn->con_type);
}

Connection* Unit::ConnectFrom(Unit* su, Projection* prjn, Con_Group*& recv_gp,
			      Con_Group*& send_gp) {
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal() && !prjn->con_spec->DMem_AlwaysLocal()) return NULL;
#endif
  if((prjn->recv_idx < 0) || ((recv_gp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL)) {
    recv_gp = recv.NewPrjn(prjn, true);
    prjn->recv_idx = recv.gp.size-1;
  }
  if((prjn->send_idx < 0) || ((send_gp = (Con_Group*)su->send.SafeGp(prjn->send_idx)) == NULL)) {
    send_gp = su->send.NewPrjn(prjn, false);
    prjn->send_idx = su->send.gp.size-1;
  }
  if(recv_gp->other_idx < 0)
    recv_gp->other_idx = prjn->send_idx;
  if(send_gp->other_idx < 0)
    send_gp->other_idx = prjn->recv_idx;

  Connection* con = recv_gp->NewCon(prjn->con_type, su);
  send_gp->LinkCon(con, this);
  n_recv_cons++;
  return con;
}

Connection* Unit::ConnectFromLink(Unit* su, Projection* prjn, Connection* src_con,
				  Con_Group*& recv_gp, Con_Group*& send_gp) {
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal() && !prjn->con_spec->DMem_AlwaysLocal()) return NULL;
#endif
  if((prjn->recv_idx < 0) || ((recv_gp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL)) {
    recv_gp = recv.NewPrjn(prjn, false); // doesn't own the connections!
    prjn->recv_idx = recv.gp.size-1;
  }
  if((prjn->send_idx < 0) || ((send_gp = (Con_Group*)su->send.SafeGp(prjn->send_idx)) == NULL)) {
    send_gp = su->send.NewPrjn(prjn, false);
    prjn->send_idx = su->send.gp.size-1;
  }
  if(recv_gp->other_idx < 0)
    recv_gp->other_idx = prjn->send_idx;
  if(send_gp->other_idx < 0)
    send_gp->other_idx = prjn->recv_idx;

  recv_gp->LinkCon(src_con, su);
  send_gp->LinkCon(src_con, this);
  n_recv_cons++;
  return src_con;
}

Connection* Unit::ConnectFromCk(Unit* su, Projection* prjn, Con_Group*& recv_gp,
			      Con_Group*& send_gp) {
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal() && !prjn->con_spec->DMem_AlwaysLocal()) return NULL;
#endif
  if((prjn->recv_idx < 0) || ((recv_gp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL)) {
    recv_gp = recv.NewPrjn(prjn, true);
    prjn->recv_idx = recv.gp.size-1;
  }
  if((prjn->send_idx < 0) || ((send_gp = (Con_Group*)su->send.SafeGp(prjn->send_idx)) == NULL)) {
    send_gp = su->send.NewPrjn(prjn, false);
    prjn->send_idx = su->send.gp.size-1;
  }
  if(recv_gp->other_idx < 0)
    recv_gp->other_idx = prjn->send_idx;
  if(send_gp->other_idx < 0)
    send_gp->other_idx = prjn->recv_idx;

  if(recv_gp->units.Find(su) >= 0) // already connected!
    return NULL;

  Connection* con = recv_gp->NewCon(prjn->con_type, su);
  send_gp->LinkCon(con, this);
  n_recv_cons++;
  return con;
}

Connection* Unit::ConnectFromLinkCk(Unit* su, Projection* prjn, Connection* src_con,
				  Con_Group*& recv_gp, Con_Group*& send_gp) {
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal() && !prjn->con_spec->DMem_AlwaysLocal()) return NULL;
#endif
  if((prjn->recv_idx < 0) || ((recv_gp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL)) {
    recv_gp = recv.NewPrjn(prjn, false); // doesn't own the connections!
    prjn->recv_idx = recv.gp.size-1;
  }

  if((prjn->send_idx < 0) || ((send_gp = (Con_Group*)su->send.SafeGp(prjn->send_idx)) == NULL)) {
    send_gp = su->send.NewPrjn(prjn, false);
    prjn->send_idx = su->send.gp.size-1;
  }

  if(recv_gp->units.Find(su) >= 0) // already connected!
    return NULL;

  recv_gp->LinkCon(src_con, su);
  send_gp->LinkCon(src_con, this);
  return src_con;
}

bool Unit::DisConnectFrom(Unit* su, Projection* prjn) {
  Con_Group* recv_gp, *send_gp;
  if(prjn != NULL) {
    if((prjn->recv_idx < 0) || ((recv_gp = (Con_Group*)recv.SafeGp(prjn->recv_idx)) == NULL))
      return false;
    if((prjn->send_idx < 0) || ((send_gp = (Con_Group*)su->send.SafeGp(prjn->send_idx)) == NULL))
      return false;
  }
  else {
    Layer* su_lay = GET_OWNER(su,Layer);
    if((recv_gp = recv.FindFrom(su_lay)) == NULL)	return false;
    if(recv_gp->other_idx >= 0)
      send_gp = (Con_Group*)su->send.SafeGp(recv_gp->other_idx);
    else
      send_gp = NULL;
    if(send_gp == NULL)
      send_gp = su->send.FindPrjn(recv_gp->prjn);
    if(send_gp == NULL) return false;
    prjn = recv_gp->prjn;
  }

  recv_gp->RemoveCon(su);
  n_recv_cons--;
  return send_gp->RemoveCon(this);
}

void Unit::DisConnectAll() {
  Con_Group* recv_gp, *send_gp;
  int g;
  int i;
  for(g=0; g<recv.gp.size; g++) { // the removes cause the leaf_gp to crash.. 
    recv_gp = (Con_Group*)recv.gp.FastEl(g); 
    for(i=recv_gp->size-1; i>=0; i--) {
      if(recv_gp->other_idx >= 0)
	send_gp = (Con_Group*)recv_gp->Un(i)->send.SafeGp(recv_gp->other_idx);
      else
	send_gp = NULL;
      if(send_gp == NULL)
	send_gp = recv_gp->Un(i)->send.FindPrjn(recv_gp->prjn);
      if(send_gp != NULL)
	send_gp->RemoveCon(this);
      recv_gp->Remove(i);
    }
    recv_gp->other_idx = -1;
  }
  for(g=0; g<send.gp.size; g++) { // the removes cause the leaf_gp to crash.. 
    send_gp = (Con_Group*)send.gp.FastEl(g); 
    for(i=send_gp->size-1; i>=0; i--) {
      if(send_gp->other_idx >= 0)
	recv_gp = (Con_Group*)send_gp->Un(i)->recv.SafeGp(send_gp->other_idx);
      else
	recv_gp = NULL;
      if(recv_gp == NULL)
	recv_gp = send_gp->Un(i)->recv.FindPrjn(send_gp->prjn);
      if(recv_gp != NULL)
	recv_gp->RemoveCon(this);
      send_gp->Remove(i);
    }
    send_gp->other_idx = -1;
  }
  n_recv_cons = 0;
}

int Unit::CountRecvCons() {
  n_recv_cons = 0;
  Con_Group* cg;
  int g;
  FOR_ITR_GP(Con_Group, cg, recv., g) {
    n_recv_cons += cg->size;
  }
  return n_recv_cons;
}

void Unit::Copy_Weights(const Unit* src, Projection* prjn) {
  if((bias != NULL) && (src->bias != NULL)) {
    bias->wt = src->bias->wt;
  }
  Con_Group* cg, *scg;
  int i,si;
  for(cg = (Con_Group*)recv.FirstGp(i), scg = (Con_Group*)src->recv.FirstGp(si);
      (cg != NULL) && (scg != NULL);
      cg = (Con_Group*)recv.NextGp(i), scg = (Con_Group*)src->recv.NextGp(si))
  {
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    cg->Copy_Weights(scg);
  }
}

void Unit::WriteWeights(ostream& strm, Projection* prjn, Con_Group::WtSaveFormat fmt) {
  if(fmt == Con_Group::TEXT)
    strm << "#Unit " << name << "\n";
  if(bias != NULL) {
    switch(fmt) {
    case Con_Group::TEXT:
    case Con_Group::TEXT_IDX:
      strm << bias->wt << "\n";
      break;
    case Con_Group::BINARY:
    case Con_Group::BINARY_IDX:
      strm.write((char*)&(bias->wt), sizeof(bias->wt));
      break;
    }
  }
  // not using ITR here in case of DMEM where we write separate files for
  // each process -- need to include size=0 place holders for non-local units
  int g;
  for(g = 0; g < recv.gp.size; g++) {
    Con_Group* cg = (Con_Group*)recv.gp.FastEl(g);
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    if(fmt == Con_Group::TEXT)
      strm << "#Con_Group " << g << "\n";
    cg->WriteWeights(strm, this, fmt);
  }
}

void Unit::ReadWeights(istream& strm, Projection* prjn, Con_Group::WtSaveFormat fmt) {
  if(bias != NULL) {
    switch(fmt) {
    case Con_Group::TEXT:
    case Con_Group::TEXT_IDX:
      {
	int c;
	while(true) {
	  c = taMisc::read_alnum_noeol(strm);
	  if(c == EOF) return;
	  if(taMisc::LexBuf.empty()) {
	    continue;		// re-read it
	  }
	  if(taMisc::LexBuf.firstchar() == '#') { // skip comments
	    if(c != '\n')			      // only if didn't read last line
	      taMisc::read_till_eol(strm); // bag rest of line
	    continue;
	  }
	  bias->wt = (float)taMisc::LexBuf;
	  break;
	}
      }
      break;
    case Con_Group::BINARY:
    case Con_Group::BINARY_IDX:
      strm.read((char*)&(bias->wt), sizeof(bias->wt));
      break;
    }
  }
#ifdef DMEM_COMPILE
  if(!DMem_IsLocal()) {
    // bypass non-local connections!
    if(fmt == Con_Group::TEXT) {
      int tot_sz = n_recv_cons;
      int read_sz = 0;		// total sizes actually read from the file
      for(int i=0; i<tot_sz; i++) {
	int c = taMisc::read_alnum_noeol(strm);
	if(c == EOF) break;
	if(taMisc::LexBuf.empty()) {
	  i--; continue;		// re-read it
	}
	if(taMisc::LexBuf.firstchar() == '#') { // skip comments
	  if(taMisc::LexBuf == "#Size") {
	    taMisc::read_till_eol(strm);
	    int sz = (int)taMisc::LexBuf;
	    read_sz += sz;
	    if(read_sz > tot_sz) tot_sz = read_sz;
	  }
	  else if(taMisc::LexBuf == "#Unit") {
	    taMisc::read_till_eol(strm);
	    break; // we just went into the second unit, bail!
	  }
	  else {
	    if(c != '\n')
	      taMisc::read_till_eol(strm); // bag rest of line
	    i--;			// re-read this one
	    continue;
	  }
	}
      }
    }
    else {
      int g;
      for(g = 0; g < recv.gp.size; g++) {
	Con_Group* cg = (Con_Group*)recv.gp.FastEl(g);
	if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
	int sz;
	strm >> sz;  strm.get();
	float dumy;
	if(fmt == Con_Group::BINARY) {
	  for(int i=0;i<sz; i++) strm.read((char*)&(dumy), sizeof(dumy));
	}
	else {
	  float lidx;
	  for(int i=0;i<sz; i++) {
	    strm.read((char*)&(lidx), sizeof(lidx));
	    strm.read((char*)&(dumy), sizeof(dumy));
	  }
	}
      }
    }
    return;
  }
#endif
  if(strm.eof()) return;
  int g;
  for(g = 0; g < recv.gp.size; g++) {
    Con_Group* cg = (Con_Group*)recv.gp.FastEl(g);
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    cg->ReadWeights(strm, this, fmt);
    if(strm.eof()) break;
  }
}

void Unit::TransformWeights(const SimpleMathSpec& trans, Projection* prjn) { 
  Con_Group* cg;
  int g;
  FOR_ITR_GP(Con_Group, cg, recv., g) {
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    cg->TransformWeights(trans);
  }
}

void Unit::AddNoiseToWeights(const Random& noise_spec, Projection* prjn) { 
  Con_Group* cg;
  int g;
  FOR_ITR_GP(Con_Group, cg, recv., g) {
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    cg->AddNoiseToWeights(noise_spec);
  }
}

int Unit::PruneCons(const SimpleMathSpec& pre_proc, CountParam::Relation rel,
		       float cmp_val, Projection* prjn)
{
  int rval = 0;
  int g;
  for(g=0; g<recv.gp.size; g++) {
    Con_Group* cg = (Con_Group*)recv.gp.FastEl(g);
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    rval += cg->PruneCons(this, pre_proc, rel, cmp_val);
  }
  n_recv_cons -= rval;
  return rval;
}

int Unit::LesionCons(float p_lesion, bool permute, Projection* prjn) {
  int rval = 0;
  int g;
  for(g=0; g<recv.gp.size; g++) {
    Con_Group* cg = (Con_Group*)recv.gp.FastEl(g);
    if(cg->prjn->from->lesion || ((prjn != NULL) && (cg->prjn != prjn))) continue;
    rval += cg->LesionCons(this, p_lesion, permute);
  }
  n_recv_cons -= rval;
  return rval;
}



////////////////////////////
//	ProjectionSpec    //
////////////////////////////

void ProjectionSpec::UpdateAfterEdit() {
  BaseSpec::UpdateAfterEdit();
  if(!taMisc::is_loading) {
    if(UseCount() == 0) {
      if(!not_used_ok)
	taMisc::Error("Note: you just edited:",GetTypeDef()->name,GetPath(),"but it is not used anywhere!");
      not_used_ok = true;
    }
  }
}

void ProjectionSpec::Initialize() {
  min_obj_type = &TA_Projection;
  self_con = false;
  init_wts = false;
}

void ProjectionSpec::InitLinks() {
  BaseSpec::InitLinks();
  children.SetBaseType(&TA_ProjectionSpec); // allow all of this general spec type to be created under here
  children.el_typ = GetTypeDef(); // but make the default to be me!
}

void ProjectionSpec::RemoveCons(Projection* prjn) {
  Unit* u;
  taLeafItr i;
  if(prjn->layer != NULL) {
    FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
      u->recv.RemovePrjn(prjn);
      u->n_recv_cons = 0;
    }
  }

  if(prjn->from != NULL) {
    FOR_ITR_EL(Unit, u, prjn->from->units., i)
      u->send.RemovePrjn(prjn);
  }

  prjn->recv_idx = -1;
  prjn->send_idx = -1;
  prjn->projected = false;
}

void ProjectionSpec::Connect(Projection* prjn) {
  RemoveCons(prjn);
  prjn->SetFrom();
  PreConnect(prjn);
  Connect_impl(prjn);
  InitWtState(prjn);
  prjn->projected = true;
}

int ProjectionSpec::ProbAddCons(Projection*, float, float) {
  return 0;
}

void ProjectionSpec::InitWtState(Projection* prjn) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
    int g;
    for(g=0; g < u->recv.gp.size; g++) {
      Con_Group* cg = (Con_Group*)u->recv.gp.FastEl(g);
      if(cg->prjn == prjn)
	cg->InitWtState(u);
    }
  }
}

void ProjectionSpec::InitWtState_post(Projection* prjn) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
    int g;
    for(g=0; g < u->recv.gp.size; g++) {
      Con_Group* cg = (Con_Group*)u->recv.gp.FastEl(g);
      if(cg->prjn == prjn)
	cg->InitWtState_post(u);
    }
  }
}

void ProjectionSpec::C_InitWtState(Projection*, Con_Group* cg, Unit* ru) {
  // default is just to do same thing as the conspec would have done..
  CON_GROUP_LOOP(cg, cg->C_InitWtState(cg->Cn(i), ru, cg->Un(i)));
}

void ProjectionSpec::InitWtDelta(Projection* prjn) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
    int g;
    for(g=0; g < u->recv.gp.size; g++) {
      Con_Group* cg = (Con_Group*)u->recv.gp.FastEl(g);
      if(cg->prjn == prjn)
	cg->InitWtDelta(u);
    }
  }
}

// reconnect based on data in the recv groups only (ie after loading)
void ProjectionSpec::ReConnect_Load(Projection* prjn) {
//  prjn->SetFrom();		// justin case

  if((prjn->from == NULL) || (prjn->layer == NULL))
    return;
  
  Con_Group* send_gp;
  Con_Group* recv_gp;
  Unit* ru, *su;
  taLeafItr ru_itr;
  FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
    if(ru->spec.spec == NULL)
      taMisc::Error("Spec is NULL in Unit:",ru->GetPath(),"will crash if not fixed!");
    int rg;
    FOR_ITR_GP(Con_Group, recv_gp, ru->recv., rg) {
      if(recv_gp->prjn != prjn)
	continue;
      if(recv_gp->spec.spec == NULL)
	taMisc::Error("Spec is NULL in Con_Group:",recv_gp->GetPath(),"will crash if not fixed!");
      int j;
      for(j=0; j<recv_gp->units.size; j++) {
	su = recv_gp->Un(j);
	if(su == NULL)
	  continue;

	if(recv_gp->other_idx >= 0)
	  send_gp = (Con_Group*)su->send.SafeGp(recv_gp->other_idx);
	else
	  send_gp = NULL;
	if(send_gp == NULL)
	  send_gp = su->send.FindPrjn(prjn);
	if(send_gp != NULL) {
	  if(send_gp->spec.spec == NULL)
	    taMisc::Error("Spec is NULL in Con_Group:",send_gp->GetPath(),"will crash if not fixed!");
	  send_gp->own_cons = false; // sender doesn't own
	  send_gp->units.LinkUnique(ru);
	  send_gp->LinkUnique(recv_gp->Cn(j));
	}  
      }
    }
  }
}

void ProjectionSpec::CopyNetwork(Network* net, Network* cn, Projection* prjn, Projection* cp) {
  MemberDef* md;
  String path;
  if(cp->from != NULL) {
    path = cp->from->GetPath(NULL, cn); // path of old layer
    Layer* nw_lay = (Layer*)net->FindFromPath(path, md);	// find under nw net
    if(nw_lay != NULL)
      taBase::SetPointer((TAPtr*)&(prjn->from), nw_lay);
  }
  prjn->recv_idx = cp->recv_idx;
  prjn->send_idx = cp->send_idx;
  prjn->recv_n = cp->recv_n;
  prjn->send_n = cp->send_n;
  prjn->projected = cp->projected;
}

void Projection::CopyPtrs(Projection* cp) {
  taBase::SetPointer((TAPtr*)&from, cp->from);
  recv_idx = cp->recv_idx;
  send_idx = cp->send_idx;
  recv_n = cp->recv_n;
  send_n = cp->send_n;
  projected = cp->projected;
}

void ProjectionSpec::PreConnect(Projection* prjn) {
  if(prjn->from == NULL)	return;

  // make first set of congroups to get indicies
  Unit* first_ru = (Unit*)prjn->layer->units.Leaf(0);
  Unit* first_su = (Unit*)prjn->from->units.Leaf(0);
  if((first_ru == NULL) || (first_su == NULL))
    return;
  Con_Group* recv_gp = first_ru->recv.NewPrjn(prjn, true);
  prjn->recv_idx = first_ru->recv.gp.size - 1;
  Con_Group* send_gp = first_su->send.NewPrjn(prjn, false);
  prjn->send_idx = first_su->send.gp.size - 1;
  // set reciprocal indicies
  recv_gp->other_idx = prjn->send_idx;
  send_gp->other_idx = prjn->recv_idx;

  // then crank out for remainder of units..
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
    if(u == first_ru)	continue; // skip over first one..
    recv_gp = u->recv.NewPrjn(prjn, true);
    recv_gp->other_idx = prjn->send_idx;
  }
  FOR_ITR_EL(Unit, u, prjn->from->units., i) {
    if(u == first_su)	continue; // skip over first one..
    send_gp = u->send.NewPrjn(prjn, false);
    send_gp->other_idx = prjn->recv_idx;
  }
}


////////////////////////
//	Projection    //
////////////////////////

void Projection::Initialize() {
  layer = from = NULL;
  from_type = PREV;
  con_type = &TA_Connection;
  con_gp_type = &TA_Con_Group;
  recv_idx = -1;
  send_idx = -1;
  recv_n = 1;
  send_n = 1;
  projected = false;
  proj_points = NULL;
}

void Projection::Destroy(){
  CutLinks();
}

void Projection::CutLinks() {
  if(owner == NULL) return;
  if(from != NULL) {
    // remove from sending links, being sure to protect against a spurious re-delete
    taBase::Ref(this);
    from->send_prjns.Remove(this);
    taBase::unRef(this);
  }
  RemoveCons();		// remove actual connections
  taBase::DelPointer((TAPtr*)&from);
  taBase::DelPointer((TAPtr*)&proj_points);
  spec.CutLinks();
  con_spec.CutLinks();
  if((layer != NULL) && taMisc::iv_active) {
    owner = NULL;		// tell view that we're not really here
    if(layer->own_net != NULL) {
      layer->own_net->RemoveCons(); // get rid of connections in any other layers!
    }
  }
  layer = NULL;
  taNBase::CutLinks();
}

void Projection::InitLinks() {
  taNBase::InitLinks();
  spec.SetDefaultSpec(this);
  con_spec.SetDefaultSpec(this);
  layer = GET_MY_OWNER(Layer);
  Network* mynet = GET_MY_OWNER(Network);
  if(mynet != NULL) {
    int myindex = mynet->layers.FindLeaf(layer);
    if(!(myindex == 0) && (from_type == PREV)) // is it not the first?
      UpdateAfterEdit();
  }
}

void Projection::Copy_(const Projection& cp) {
  from_type = cp.from_type;
  spec = cp.spec;
  con_type = cp.con_type;
  con_gp_type = cp.con_gp_type;
  con_spec = cp.con_spec;
  if(cp.proj_points != NULL) {
    if(proj_points == NULL) {
      proj_points = new Xform();
      taBase::Own(proj_points, this);
    }
    proj_points->Copy(*(cp.proj_points));
  }

  // not to copy!
  // taBase::SetPointer((TAPtr*)&from, cp.from);
  // recv_idx = cp.recv_idx;
  // send_idx = cp.send_idx;
  // recv_n = cp.recv_n;
  // send_n = cp.send_n;
  // projected = cp.projected;
}

void Projection::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  spec.CheckSpec();
  con_spec.CheckSpec();
  SetFrom();
  if(from != NULL)
    name = "Fm_" + from->name;
  if(taMisc::is_loading) return;
  ApplyConSpec();
  CheckTypes_impl();
  if(!taMisc::iv_active) return;
  Network* net = GET_MY_OWNER(Network);
  if(net == NULL) return;
  NetView* view;
  taLeafItr i;
  FOR_ITR_EL(NetView, view, net->views., i) {
    if(view->display_toggle) {
      view->FixProjection(this);
      view->UpdateButtons();
    }
  }
}

void Projection::SetFromPoints(float x1,float y1){
  if(proj_points == NULL){
    proj_points = new Xform();
    taBase::Own(proj_points,this);
  }
  proj_points->a00 = x1;
  proj_points->a01 = y1;
}

void Projection::SetToPoints(float x1,float y1){
  if(proj_points == NULL){
    proj_points = new Xform();
    taBase::Own(proj_points,this);
  }
  proj_points->a10 = x1;
  proj_points->a11 = y1;
}

void Projection::SetFrom() {
  if(layer == NULL) {
    from = NULL;
    return;
  }
  Network* mynet = layer->own_net;
  if(mynet == NULL)
    return;
  int myindex = mynet->layers.FindLeaf(layer);

  switch(from_type) { // this is where the projection is coming from
  case NEXT: 
    if (myindex == (mynet->layers.leaves - 1)) { // is it the last layer 
      taMisc::Error("*** Last Layer projects from NEXT layer in prjn:",
		    name);
      return;
    }
    else {
      Layer* nwly = (Layer*)mynet->layers.Leaf(myindex+1);
      if(from == nwly) return;
      taBase::SetPointer((TAPtr*)&from, nwly);
    }
    break;
  case PREV:
    if (myindex == 0) { // is it the first
      taMisc::Error("*** First Layer recieves projection from PREV layer in prjn:",
		    name);
      return;
    }
    else {
      Layer* nwly = (Layer*)mynet->layers.Leaf(myindex-1);
      if(from == nwly) return;
      taBase::SetPointer((TAPtr*)&from, nwly);
    }
    break;
  case SELF: 
    if(from == layer) return;
    taBase::SetPointer((TAPtr*)&from, layer);
    break;	
  case CUSTOM:
    if(from == NULL) {
      taMisc::Error("*** Warning: CUSTOM projection and from is NULL in prjn:",
		    name, "in layer:", layer->name);
    }
    break;
  }
  mynet->UpdtAfterNetMod();
}

void Projection::SetCustomFrom(Layer* fm_lay) {
  taBase::SetPointer((TAPtr*)&from, fm_lay);
  if(fm_lay == layer)
    from_type = SELF;
  else
    from_type = CUSTOM;
  UpdateAfterEdit();
}

bool Projection::SetConSpec(ConSpec* sp) {
  if(sp == NULL)	return false;
  con_spec.SetSpec(sp);
  return ApplyConSpec();
}

bool Projection::SetConType(TypeDef* td) {
  if(con_type == td) return false;
  projected = false;
  con_type = td;
  return true;
}

bool Projection::SetConGpType(TypeDef* td) {
  if(con_gp_type == td) return false;
  projected = false;
  con_gp_type = td;
  return true;
}

bool Projection::ApplyConSpec() {
  if((layer == NULL) || (from == NULL)) return false;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i) {
    int g;
    for(g=0; g<u->recv.gp.size; g++) {
      Con_Group* recv_gp = (Con_Group*)u->recv.gp.FastEl(g);
      if(recv_gp->prjn == this) {
	if(con_spec.spec->CheckObjectType(recv_gp))
	  recv_gp->spec.SetSpec(con_spec.spec);
	else
	  return false;
      }
    }
  }
  // also do the from!
  FOR_ITR_EL(Unit, u, from->units., i) {
    int g;
    for(g=0; g<u->send.gp.size; g++) {
      Con_Group* send_gp = (Con_Group*)u->send.gp.FastEl(g);
      if(send_gp->prjn == this) {
	if(con_spec.spec->CheckObjectType(send_gp))
	  send_gp->spec.SetSpec(con_spec.spec);
	else
	  return false;
      }
    }
  }
  return true;
}

void Projection::FixIndexes() {
  if((layer == NULL) || (from == NULL)) return;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i) {
    int g;
    for(g=0; g<u->recv.gp.size; g++) {
      Con_Group* recv_gp = (Con_Group*)u->recv.gp.FastEl(g);
      if(recv_gp->prjn == this) {
	recv_gp->other_idx = send_idx;
      }
    }
  }
  FOR_ITR_EL(Unit, u, from->units., i) {
    int g;
    for(g=0; g<u->send.gp.size; g++) {
      Con_Group* send_gp = (Con_Group*)u->send.gp.FastEl(g);
      if(send_gp->prjn == this) {
	send_gp->other_idx = recv_idx;
      }
    }
  }
}

int Projection::ReplaceConSpec(ConSpec* old_sp, ConSpec* new_sp) {
  if(con_spec.spec != old_sp) return 0;
  con_spec.SetSpec(new_sp);
  return 1;
}

int Projection::ReplacePrjnSpec(ProjectionSpec* old_sp, ProjectionSpec* new_sp) {
  if(spec.spec != old_sp) return 0;
  spec.SetSpec(new_sp);
  return 1;
}


void Projection::ReConnect_Load() {
  if(spec.spec == NULL) {
    taMisc::Error("Spec is NULL in projection:",GetPath(),"will crash if not fixed!");
    return;
  }
  spec->ReConnect_Load(this);
}

bool Projection::CheckTypes() {
  if(!spec.CheckSpec())
    return false;
  if(CheckTypes_impl())
    return true;
  taMisc::Error("Connection, Con_Group, or spec types for projection:",GetPath(),
		"are not correct, perform 'Connect' to rectify");
  return false;
}

bool Projection::CheckTypes_impl() {
  if((layer == NULL) || (from == NULL)) return true;
  if(spec.spec == NULL)
    return false;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i) {
    int g;
    for(g=0; g<u->recv.gp.size; g++) {
      Con_Group* recv_gp = (Con_Group*)u->recv.gp.FastEl(g);
      if(recv_gp->prjn == this) {
	if(!con_spec.CheckSpec(recv_gp)) {
	  projected = false;
	  return false;
	}
	if(recv_gp->GetTypeDef() != con_gp_type) {
	  projected = false;
	  return false;
	}
	if(recv_gp->el_typ != con_type) {
	  projected = false;
	  return false;
	}
      }
    }
  }
  // also do the from!
  FOR_ITR_EL(Unit, u, from->units., i) {
    int g;
    for(g=0; g<u->send.gp.size; g++) {
      Con_Group* send_gp = (Con_Group*)u->send.gp.FastEl(g);
      if(send_gp->prjn == this) {
	if(!con_spec.CheckSpec(send_gp)) {
	  projected = false;
	  return false;
	}
	if(send_gp->GetTypeDef() != con_gp_type) {
	  projected = false;
	  return false;
	}
	if(send_gp->el_typ != con_type) {
	  projected = false;
	  return false;
	}
      }
    }
  }
  return true;
}

void Projection::Copy_Weights(const Projection* src) {
  Unit* u, *su;
  taLeafItr i,si;
  for(u = (Unit*)layer->units.FirstEl(i), su = (Unit*)src->layer->units.FirstEl(si);
      (u != NULL) && (su != NULL);
      u = (Unit*)layer->units.NextEl(i), su = (Unit*)src->layer->units.NextEl(si))
  {
    u->Copy_Weights(su, this);
  }
}

void Projection::WriteWeights(ostream& strm, Con_Group::WtSaveFormat fmt) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i)
    u->WriteWeights(strm, this, fmt);
}

void Projection::ReadWeights(istream& strm, Con_Group::WtSaveFormat fmt) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i) {
    u->ReadWeights(strm, this, fmt);
    if(strm.eof()) break;
  }
}

void Projection::TransformWeights(const SimpleMathSpec& trans) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i)
    u->TransformWeights(trans, this);
}

void Projection::AddNoiseToWeights(const Random& noise_spec) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i)
    u->AddNoiseToWeights(noise_spec, this);
}

int Projection::PruneCons(const SimpleMathSpec& pre_proc,
			      CountParam::Relation rel, float cmp_val)
{
  int rval = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i)
    rval += u->PruneCons(pre_proc, rel, cmp_val, this);
  return rval;
}

int Projection::LesionCons(float p_lesion, bool permute) {
  int rval = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, layer->units., i)
    rval += u->LesionCons(p_lesion, permute, this);
  return rval;
}


////////////////////////
//	Unit_Group    //
////////////////////////

void Unit_Group::Initialize() {
  own_lay = NULL;
  n_units = 0;
  geom.z = 1;
  units_lesioned = false;
} 

void Unit_Group::InitLinks() {
  taGroup<Unit>::InitLinks();
  taBase::Own(pos,this);
  taBase::Own(geom,this);
  own_lay = GET_MY_OWNER(Layer);
}

void Unit_Group::CutLinks() {
  own_lay = NULL;
  taGroup<Unit>::CutLinks();
}

void Unit_Group::Copy_(const Unit_Group& cp) {
  n_units = cp.n_units;
  pos = cp.pos;
  geom = cp.geom;
}

void Unit_Group::UpdateAfterEdit() {
  taGroup<Unit>::UpdateAfterEdit();
  if((own_lay == NULL) || (own_lay->own_net == NULL)) return;
  RecomputeGeometry();
  if(taMisc::is_loading || !taMisc::iv_active) return;
  NetView* view;
  taLeafItr i;
  FOR_ITR_EL(NetView, view, own_lay->own_net->views., i) {
    if(view->display_toggle)
      view->FixUnitGroup(this, own_lay);
  }
}

void Unit_Group::RemoveAll() {
  units_lesioned = false;
  int i;
  for(i=0; i<size; i++) {
    if(FastEl(i)->owner == this)
      FastEl(i)->pos.z = -1;		// signal not to update display when units are removed
  }
  taGroup<Unit>::RemoveAll();
  // then update here
  if(taMisc::iv_active && (owner != NULL) && (own_lay != NULL) && 
     (own_lay->own_net != NULL) && !own_lay->own_net->net_will_updt)
    own_lay->own_net->InitAllViews();
}

void Unit_Group::RecomputeGeometry() {
  if((own_lay == NULL) || (own_lay->own_net == NULL)) return;
  int use_n_units = n_units;
  if(n_units == 0) {
    geom = own_lay->geom;
    geom.z = 1;
    use_n_units = own_lay->n_units;
  }
  if((use_n_units == 0) && (size > 0)) {
    use_n_units = size;
    n_units = size;
  }
  geom.FitNinXY(use_n_units);
  if (use_n_units == 0) {
    use_n_units = geom.x * geom.y;
    n_units = geom.x * geom.y;
  }
}

void Unit_Group::LayoutUnits(Unit* u) {
  if(owner == own_lay)
    return;			// i'm the layer subgroup, layer takes care
  RecomputeGeometry();
  int i = 0;
  TDCoord mygeo;
  Unit* un = NULL;
  for(mygeo.y=0; mygeo.y < geom.y; mygeo.y++) {
    for(mygeo.x=0; mygeo.x <geom.x; mygeo.x++) {
      if(i >= size)
        break;
      un = (Unit*)FastEl(i++);
      if((un != NULL) && (u==NULL || un==u)) un->pos = mygeo;
      if(un==u) break;
    }
    if(un==u) break;
  }
}  

bool Unit_Group::Build() {
  if((own_lay == NULL) || (owner == own_lay))	return false;
  RecomputeGeometry();
  int use_n_units = n_units;
  if(n_units == 0)
    use_n_units = own_lay->n_units;
  bool units_changed = false;
  if(size != use_n_units)
    units_changed = true;
  EnforceSize(use_n_units);
  EnforceType();
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    u->Build();
  units_lesioned = false;
  return units_changed;
}

bool Unit_Group::CheckBuild() {
  if(n_units == 0) {
    if(own_lay == NULL) return false;
    if(!units_lesioned && (size != own_lay->n_units))
      return true;
  }
  else if(!units_lesioned && (size != n_units))
    return true;
  return false;
}

bool Unit_Group::SetUnitSpec(UnitSpec* sp) {
  if(sp == NULL)	return false;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i) {
    if(sp->CheckObjectType(u))
      u->spec.SetSpec(sp);
    else
      return false;
  }
  return true;
}

bool Unit_Group::SetConSpec(ConSpec* sp) {
  if(sp == NULL)	return false;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i) {
    if(!u->SetConSpec(sp))
      return false;
  }
  return true;
}

void Unit_Group::Copy_Weights(const Unit_Group* src) {
  Unit* u, *su;
  taLeafItr i,si;
  for(u = (Unit*)FirstEl(i), su = (Unit*)src->FirstEl(si);
      (u != NULL) && (su != NULL);
      u = (Unit*)NextEl(i), su = (Unit*)src->NextEl(si))
  {
    u->Copy_Weights(su);
  }
}

void Unit_Group::WriteWeights(ostream& strm, Con_Group::WtSaveFormat fmt) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    u->WriteWeights(strm, NULL, fmt);
}

void Unit_Group::ReadWeights(istream& strm, Con_Group::WtSaveFormat fmt) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i) {
    if(strm.eof()) break;
    u->ReadWeights(strm, NULL, fmt);
  }
}

void Unit_Group::TransformWeights(const SimpleMathSpec& trans) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    u->TransformWeights(trans);
}

void Unit_Group::AddNoiseToWeights(const Random& noise_spec) {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    u->AddNoiseToWeights(noise_spec);
}

int Unit_Group::PruneCons(const SimpleMathSpec& pre_proc,
			CountParam::Relation rel, float cmp_val)
{
  int rval = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    rval += u->PruneCons(pre_proc, rel, cmp_val);
  return rval;
}

int Unit_Group::LesionCons(float p_lesion, bool permute) {
  int rval = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i)
    rval += u->LesionCons(p_lesion, permute);
  return rval;
}

int Unit_Group::LesionUnits(float p_lesion, bool permute) {
  int rval = 0;
  if(permute) {
    rval = (int) (p_lesion * (float)leaves);
    if(rval == 0) return 0;
    int_Array ary;
    int j;
    for(j=0; j<leaves; j++)
      ary.Add(j);
    ary.Permute();
    ary.size = rval;
    ary.Sort();
    for(j=ary.size-1; j>=0; j--) {
      Unit* un = Leaf(ary.FastEl(j));
      un->DisConnectAll();
      un->pos.z = -1;		// don't update yet!
      RemoveLeaf(un);
    }
  }
  else {
    int j;
    for(j=leaves-1; j>=0; j--) {
      if(Random::ZeroOne() <= p_lesion) {
	Unit* un = (Unit*)Leaf(j);
	un->DisConnectAll();
	un->pos.z = -1;		// don't update yet!
	RemoveLeaf(j);
	rval++;
      }
    }
  }
  units_lesioned = true;	// record that units were lesioned
  int k;
  for(k=0; k<gp.size; k++) {
    Unit_Group* ug = (Unit_Group*)gp.FastEl(k);
    ug->units_lesioned = true;
  }
  if(taMisc::iv_active && (owner != NULL) && (own_lay != NULL) && 
     (own_lay->own_net != NULL) && !own_lay->own_net->net_will_updt)
    own_lay->own_net->InitAllViews();
  return rval;
}

void Unit_Group::UnitValuesToArray(float_RArray& ary, const char* variable) {
  MemberDef* md = el_typ->members.FindName(variable);
  if((md == NULL) || !md->type->InheritsFrom(TA_float)) {
    taMisc::Error("*** Variable:", variable, "not found or not a float on units of type:",
		   el_typ->name, "in UnitValuesToArray()");
    return;
  }
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i) {
    float* val = (float*)md->GetOff((void*)u);
    ary.Add(*val);
  }
}

void Unit_Group::UnitValuesFromArray(float_RArray& ary, const char* variable) {
  if(ary.size == 0) return;
  MemberDef* md = el_typ->members.FindName(variable);
  if((md == NULL) || !md->type->InheritsFrom(TA_float)) {
    taMisc::Error("*** Variable:", variable, "not found or not a float on units of type:",
		   el_typ->name, "in UnitValuesToArray()");
    return;
  }
  int cnt=0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, this->, i) {
    float* val = (float*)md->GetOff((void*)u);
    *val = ary[cnt++];
    if(cnt >= ary.size)
      break;
  }
}

Unit* Unit_Group::FindUnitFmCoord(const TwoDCoord& coord) {
  if ((coord.y < 0) || (coord.x < 0)) return NULL;
  if ((coord.y >= geom.y) || (coord.x >=geom.x)) return NULL;
  int idx = coord.y * geom.x + coord.x;
  if(idx < size)
    return FastEl(idx);
  return NULL;
}


////////////////////////
//	Layer	      //
////////////////////////

void LayerSpec::UpdateAfterEdit() {
  BaseSpec::UpdateAfterEdit();
  if(!taMisc::is_loading) {
    if(UseCount() == 0) {
      if(!not_used_ok)
	taMisc::Error("Note: you just edited:",GetTypeDef()->name,GetPath(),"but it is not used anywhere!");
      not_used_ok = true;
    }
  }
}

void LayerSpec::Initialize() {
}

void LayerSpec::InitLinks() {
  BaseSpec::InitLinks();
  children.SetBaseType(&TA_LayerSpec); // allow all of this general spec type to be created under here
  children.el_typ = GetTypeDef(); // but make the default to be me!
}

void Layer::Initialize() {
  own_net = NULL;
  projections.SetBaseType(&TA_Projection);
  units.SetBaseType(&TA_Unit);
  n_units = 0;
  lesion = false;
  ext_flag = Unit::NO_EXTERNAL;
  geom.y =1;  geom.z =1;  geom.x =0;
  act_geom = geom;
  dmem_dist = DMEM_DIST_DEFAULT;
}

void Layer::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(projections, this);
  taBase::Own(send_prjns, this);
  taBase::Own(units, this);
  taBase::Own(pos, this);
  taBase::Own(geom, this);
  taBase::Own(gp_geom, this);
  taBase::Own(gp_spc, this);
  taBase::Own(act_geom, this);
  taBase::Own(sent_already, this);
#ifdef DMEM_COMPILE
  taBase::Own(dmem_share_units, this);
#endif
  unit_spec.SetDefaultSpec(this);
  own_net = GET_MY_OWNER(Network);
  SetDefaultPos();
  units.pos.z = 0;
}

// Layer::CutLinks in pdpshell.cc

void Layer::Copy_(const Layer& cp) {
  n_units = cp.n_units;
  geom = cp.geom;
  pos = cp.pos;
  pos.x+=2; // move it over so it can be seen!
  gp_geom = cp.gp_geom;
  gp_spc = cp.gp_spc;
  projections = cp.projections;
  units = cp.units;
  unit_spec = cp.unit_spec;
  lesion = cp.lesion;
  ext_flag = cp.ext_flag;

  // not copied
  //  send_prjns.BorrowUnique(cp.send_prjns); // link group
}

void Layer::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  //  spec.CheckSpec();  // ADD THIS TO ANY LAYERS WITH SPECS!
  unit_spec.CheckSpec();
  // no negative geoms., y,z must be 1 (for display)
  SyncSendPrjns();
  RecomputeGeometry();

  if(taMisc::is_loading) return;
  if(own_net == NULL) return;
  own_net->UpdtAfterNetMod();
  if(!taMisc::iv_active) return;
  NetView* view;
  taLeafItr i;
  FOR_ITR_EL(NetView, view, own_net->views., i) {
    if(view->display_toggle)
      view->FixLayer(this);
  }
}

void Layer::ReplacePointersHook(TAPtr old) {
  Layer* ol = (Layer*)old;
  CopyPtrs(ol);
  // now go through and replace all pointers to layer
  if(own_net != NULL) {
    Layer* l;
    taLeafItr li;
    FOR_ITR_EL(Layer, l, own_net->layers., li) {
      if(l == ol) continue;
      Projection* p;
      taLeafItr pi;
      FOR_ITR_EL(Projection, p, l->projections., pi) {
	if(p->from == ol)
	  taBase::SetPointer((TAPtr*)&(p->from), this);
      }
      int pip;
      for(pip=l->send_prjns.size-1; pip>=0; pip--) {
	p = (Projection*)l->send_prjns.FastEl(pip);
	if(p == NULL) continue;
	if(p->layer == ol) {
	  int lfidx = ol->projections.FindEl(p);
	  if((lfidx >= 0) && (lfidx < projections.size)) {
	    l->send_prjns.ReplaceLink(pip, projections.FastEl(lfidx));
	  }
	  else
	    l->send_prjns.Remove(pip);	// get rid of it, add in new one
	}
      }
      // now go and see if any send prjns point to units in replaced layer! ak!
      String path;
      MemberDef* md;
      Unit* u;
      taLeafItr i;
      FOR_ITR_EL(Unit, u, l->units., i) {
	int g;
	for(g=0; g < u->recv.gp.size; g++) {
	  Con_Group* cg = (Con_Group*)u->recv.gp.FastEl(g);
	  // prjn could have either layer depending on where it is..
	  if(cg->prjn->layer == ol) {	// replace the projection!
	    int lfidx = ol->projections.FindEl(cg->prjn);
	    if((lfidx >= 0) && (lfidx < projections.size)) {
	      taBase::SetPointer((TAPtr*)&(cg->prjn), projections.FastEl(lfidx));
	    }
	  }
	  if((cg->prjn->from != this) && (cg->prjn->from != ol)) continue;
	  int i;
	  for(i=0;i<cg->units.size;i++) {
	    Unit* au = cg->Un(i);
	    path = au->GetPath(NULL, ol); // path of old unit under old lay
	    Unit* nw_un = (Unit*)FindFromPath(path, md);	// find under nw layer
	    if(nw_un != NULL)
	      cg->units.ReplaceLink(i, nw_un);
	  }
	}
	for(g=0; g < u->send.gp.size; g++) {
	  Con_Group* cg = (Con_Group*)u->send.gp.FastEl(g);
	  // prjn could have either layer depending on where it is..
	  if((cg->prjn->layer != this) && (cg->prjn->layer != ol)) continue; 
	  if(cg->prjn->layer == ol) {	// replace the projection!
	    int lfidx = ol->projections.FindEl(cg->prjn);
	    if((lfidx >= 0) && (lfidx < projections.size)) {
	      taBase::SetPointer((TAPtr*)&(cg->prjn), projections.FastEl(lfidx));
	    }
	  }
	  int i;
	  for(i=0;i<cg->units.size;i++) {
	    Unit* au = cg->Un(i);
	    path = au->GetPath(NULL, ol); // path of old unit under old lay
	    Unit* nw_un = (Unit*)FindFromPath(path, md);	// find under nw layer
	    if(nw_un != NULL)
	      cg->units.ReplaceLink(i, nw_un);
	  }
	}
      }
    }
  }
  Projection* p;
  taLeafItr pi;
  FOR_ITR_EL(Projection, p, ol->projections., pi) {
    p->layer = NULL;
    taBase::DelPointer((TAPtr*)&(p->from));
  }
  // get rid of any apparent connectivity
  ol->projections.Reset();
  ol->send_prjns.Reset();
  tabMisc::DelayedUpdateAfterEdit(this);
  tabMisc::DelayedUpdateAfterEdit(own_net);
  taNBase::ReplacePointersHook(old);
}

void Layer::CopyNetwork(Network* net, Network* cn, Layer* lay) {
  Projection* p;
  taLeafItr pi;
  Projection* cp;
  taLeafItr cpi;
  for(p = (Projection*)projections.FirstEl(pi), cp = (Projection*)lay->projections.FirstEl(cpi);
      p && cp;
      p = (Projection*)projections.NextEl(pi), cp = (Projection*)lay->projections.NextEl(cpi)) {
    p->CopyNetwork(net, cn, cp);
  }
  Unit* u;
  taLeafItr ui;
  Unit* cu;
  taLeafItr cui;
  for(u = (Unit*)units.FirstEl(ui), cu = (Unit*)lay->units.FirstEl(cui);
      u && cu;
      u = (Unit*)units.NextEl(ui), cu = (Unit*)lay->units.NextEl(cui)) {
    u->CopyNetwork(net, cn, cu);
  }
}

void Layer::CopyPtrs(Layer* lay) {
  Projection* p;
  taLeafItr pi;
  Projection* cp;
  taLeafItr cpi;
  for(p = (Projection*)projections.FirstEl(pi), cp = (Projection*)lay->projections.FirstEl(cpi);
      p && cp;
      p = (Projection*)projections.NextEl(pi), cp = (Projection*)lay->projections.NextEl(cpi)) {
    p->CopyPtrs(cp);
  }
  int pip;
  for(pip=lay->send_prjns.size-1; pip>=0; pip--) {
    Projection* p = (Projection*)lay->send_prjns.FastEl(pip);
    if(p == NULL) continue;
    if(p->layer != lay)
      send_prjns.LinkUnique(p);
    else {
      int lfidx = lay->projections.FindEl(p);
      if((lfidx >= 0) && (lfidx < projections.size)) {
	send_prjns.LinkUnique(projections.FastEl(lfidx));
      }
    }
  }
  Unit* u;
  taLeafItr ui;
  Unit* cu;
  taLeafItr cui;
  for(u = (Unit*)units.FirstEl(ui), cu = (Unit*)lay->units.FirstEl(cui);
      u && cu;
      u = (Unit*)units.NextEl(ui), cu = (Unit*)lay->units.NextEl(cui)) {
    u->CopyPtrs(cu);
  }
}

void Layer::SyncSendPrjns() {
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i) {
    Layer* snd = p->from;
    if(snd == NULL) continue;
    snd->send_prjns.LinkUnique(p); // make sure senders are all represented
  }
  // now make sure that we don't have any spurious ones
  int pi;
  for(pi=send_prjns.size-1; pi>=0; pi--) {
    p = (Projection*)send_prjns.FastEl(pi);
    if(p == NULL) continue;
    if((p->layer == NULL) || (p->from != this))
      send_prjns.Remove(pi);	// get rid of it!
  }
}

void Layer::RecomputeGeometry() {
  geom.y = MAX(1,geom.y);  geom.z = MAX(1,geom.z);
  if((n_units == 0) && (units.size > 0))
    n_units = units.size;
  geom.FitNinXY(n_units);
  if(n_units == 0)
    n_units = geom.x * geom.y;
  if(geom.z > 1)
    gp_geom.FitNinXY(geom.z);	// fit all groups in there..
  else
    act_geom = geom;
}

bool Layer::SetLayerSpec(LayerSpec*) {
  return false;			// no layer spec for layers!
}

void Layer::SetDefaultPos() {
  if (own_net == NULL) return;
  int index = own_net->layers.FindLeaf(this);
  switch(own_net->lay_layout) {
  case Network::THREE_D:
    pos.z = index; pos.y=0;
    return;
  case Network::TWO_D:
    pos.z = 0;
    pos.y = 0;
    int i;
    for(i=0;i<index;i++) {
      pos.y = 
	MAX(pos.y, ((Layer*) own_net->layers.Leaf(i))->pos.y +
	    ((Layer*) own_net->layers.Leaf(i))->geom.y) + 2;
    }
  }
}

void Layer::LayoutUnits(Unit* u) {
  RecomputeGeometry();
  units.pos.z = 0;
  if(geom.z > 1) {		// create groups for each z dimension
    TDCoord mygeo;
    PosTDCoord ungeo;
    act_geom.x = 0; act_geom.y = 0;
    int i = 0;
    int max_y = 0;
    for(mygeo.y=0, ungeo.y=0; mygeo.y < gp_geom.y; mygeo.y++) {
      max_y = 0;
      for(mygeo.x=0, ungeo.x=0; mygeo.x <gp_geom.x; mygeo.x++) {
	if(i >= units.gp.size)
	  break;
	Unit_Group* ug = (Unit_Group*)units.gp.FastEl(i++);
// 	if((ug->pos.x < ungeo.x) || (ug->pos.y < ungeo.y))
	// always update to new position!
	ug->pos = ungeo;
        ug->LayoutUnits();
	ungeo.x = ug->pos.x + MAX(ug->geom.x, geom.x) + gp_spc.x;
	max_y = MAX(ug->pos.y + ug->geom.y, max_y);
	act_geom.x = MAX(act_geom.x, ungeo.x);
      }
      ungeo.y = max_y + gp_spc.y;
      act_geom.y = MAX(act_geom.y, ungeo.y);
    }
    act_geom.y -= gp_spc.y;	// no space at the end!
    act_geom.x -= gp_spc.x;
  }
  else {  
    TDCoord mygeo;
    act_geom = geom;
    units.geom = geom;		// default is to have layer's geom
    int i = 0;
    Unit* un = NULL;
    for(mygeo.y=0; mygeo.y < geom.y; mygeo.y++) {
      for(mygeo.x=0; mygeo.x <geom.x; mygeo.x++) {
	if(i >= units.leaves)
	  break;
	un = (Unit*)units.Leaf(i++);
	if((un != NULL) && (u==NULL || un==u)) un->pos = mygeo;
	if(un==u) break;
      }
      if(un==u) break;
    }
  }
}  

void Layer::Build() {
  taMisc::Busy();
  RecomputeGeometry();
  bool units_changed = false;
  geom.z = MAX(geom.z, 1);
  if(geom.z > 1) {		// create groups for each z dimension
    while(units.size > 0) {
      ((Unit*)units.FastEl(units.size-1))->pos.z = -1; // do not update
      units.Remove(units.size-1); // get rid of any in top-level
    }
    units.gp.EnforceSize(geom.z);
    int k;
    for(k=0; k< units.gp.size; k++) {
      Unit_Group* ug = (Unit_Group*)units.gp.FastEl(k);
      ug->UpdateAfterEdit();
      units_changed = ug->Build();
    }
  }
  else {
    units.gp.RemoveAll();	// in case there were any subgroups..
    if(!units_changed && (units.leaves != n_units))
      units_changed = true;
    units.EnforceLeaves(n_units);
    units.EnforceType();
    Unit* u;
    taLeafItr i;
    FOR_ITR_EL(Unit, u, units., i)
      u->Build();
    units.units_lesioned = false;
  }
  
  LayoutUnits();
  // assign the spec
  taLeafItr i;
  Unit* u;
  FOR_ITR_EL(Unit, u, units.,i){
    u->spec = unit_spec;
  }
  if(units_changed) {
    // tell all projections that they need to be connected
    Projection* pjn;
    taLeafItr j;
    FOR_ITR_EL(Projection, pjn, projections., j) {
      pjn->projected = false;
    }
    FOR_ITR_EL(Projection, pjn, send_prjns., j) {
      pjn->projected = false;
    }
  }
  if(taMisc::iv_active)
    tabMisc::NotifyEdits(this);
  taMisc::DoneBusy();
}

void Layer::LayoutUnitGroups() {
  if(units.gp.size == 0) return;
  TDCoord mygeo;
  PosTDCoord ungeo;
  act_geom.x = 0; act_geom.y = 0;
  int i = 0;
  int max_y = 0;
  for(mygeo.y=0, ungeo.y=0; mygeo.y < gp_geom.y; mygeo.y++) {
    max_y = 0;
    for(mygeo.x=0, ungeo.x=0; mygeo.x <gp_geom.x; mygeo.x++) {
      if(i >= units.gp.size)
	break;
      Unit_Group* ug = (Unit_Group*)units.gp.FastEl(i++);
      ug->pos = ungeo;
      ug->LayoutUnits();
      ungeo.x = ug->pos.x + MAX(ug->geom.x, geom.x) + gp_spc.x;
      max_y = MAX(ug->pos.y + ug->geom.y, max_y);
      act_geom.x = MAX(act_geom.x, ungeo.x);
    }
    ungeo.y = max_y + gp_spc.y;
    act_geom.y = MAX(act_geom.y, ungeo.y);
  }
  act_geom.y -= gp_spc.y;	// no space at the end!
  act_geom.x -= gp_spc.x;
}  

bool Layer::CheckBuild() {
  if(units.gp.size > 0) {
    int g;
    for(g=0; g<units.gp.size; g++) {
      Unit_Group* ug = (Unit_Group*)units.gp.FastEl(g);
      if(ug->CheckBuild())
	return true;
    }
  }
  else {
    if(!units.units_lesioned && (units.size != n_units))
      return true;
  }

  Unit* u;
  taLeafItr ui;
  FOR_ITR_EL(Unit, u, units., ui) {
    if(u->GetTypeDef() != units.el_typ)
      return true;
    if(u->CheckBuild())
      return true;
  }
  return false;
}

bool Layer::CheckConnect() {
  Projection* prjn;
  taLeafItr j;
  FOR_ITR_EL(Projection, prjn, projections.,j) {
    if(!prjn->projected)
      return true;
    if(prjn->con_spec.spec == NULL)
      return false;
    if(!prjn->con_spec->CheckObjectType(prjn))
      return false;
  }
  return false;
}

bool Layer::CheckTypes() {
  // don't check layerspec by default, but layers that have them should!
  Unit* u;
  taLeafItr ui;
  FOR_ITR_EL(Unit, u, units., ui) {
    if(!u->CheckTypes()) return false;
  }
  Projection* prjn;
  taLeafItr j;
  FOR_ITR_EL(Projection, prjn, projections.,j) {
    if(!prjn->CheckTypes()) return false;
  }
  return true;
}

bool Layer::CheckConfig(TrialProcess* tp, bool quiet) {
  // layerspec should take over this function in layers that have them!
  Unit* u;
  taLeafItr ui;
  FOR_ITR_EL(Unit, u, units., ui) {
    if(!u->CheckConfig(this, tp, quiet)) return false;
  }
  return true;
}

void Layer::FixPrjnIndexes() {
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    p->FixIndexes();
}

void Layer::RemoveCons() {
  taMisc::Busy();
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i) {
    if(p->spec.spec != NULL)
      p->RemoveCons();
  }
  Unit* u;
  FOR_ITR_EL(Unit, u, units., i)
    u->RemoveCons();
  taMisc::DoneBusy();
}

void Layer::RemoveUnits() {
  taMisc::Busy();
  if(units.gp.size == 0) {
    units.RemoveAll();
  }
  else {
    int g;
    for(g=0; g<units.gp.size; g++) {
      Unit_Group* ug = (Unit_Group*)units.gp.FastEl(g);
      ug->RemoveAll();
    }
  }
  taMisc::DoneBusy();
}

void Layer::RemoveUnitGroups() {
  taMisc::Busy();
  units.RemoveAll();
  taMisc::DoneBusy();
}

void Layer::PreConnect() {
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    p->PreConnect();
}

void Layer::Connect() {
  taMisc::Busy();
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    p->Connect();
  Unit* u;
  FOR_ITR_EL(Unit, u, units., i)
    u->Build();			// this is for the bias connections!
  taMisc::DoneBusy();
}

void Layer::DisConnect() {
  int pi;
  for(pi=send_prjns.size-1; pi>=0; pi--) {
    Projection* p = (Projection*)send_prjns.FastEl(pi);
    if(p == NULL) continue;
    if(p->layer == NULL) {
      send_prjns.Remove(pi);
      continue;
    }
    p->layer->projections.RemoveLeaf(p);
  }
  send_prjns.Reset();
  projections.Reset();
}

int Layer::CountRecvCons() {
  int n_cons = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i) {
    n_cons += u->CountRecvCons();
  }
  return n_cons;
}

void Layer::ReConnect_Load() {
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    p->ReConnect_Load();
}
 
void Layer::InitExterns() {
  if(ext_flag == Unit::NO_EXTERNAL)
    return;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitExterns();
  ext_flag = Unit::NO_EXTERNAL;
}

void  Layer::InitDelta() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitDelta();
}

void  Layer::InitState() {
  ext_flag = Unit::NO_EXTERNAL;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitState();
}

void  Layer::ModifyState() {
  ext_flag = Unit::NO_EXTERNAL;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->ModifyState();
}

void  Layer::InitWtDelta() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitWtDelta();
}

void Layer::InitWtState() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitWtState();
}

void Layer::InitWtState_post() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->InitWtState_post();
}

void Layer::Compute_Net() {
  if(projections.leaves == 0) return; // if no connections, don't do it!
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Compute_Net();
}

void Layer::Send_Net() {
  if(send_prjns.leaves == 0) return; // no connections, don't do it!
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Send_Net();
}

void Layer::Send_NetToLay(Layer* tolay) {
  if(send_prjns.leaves == 0) return; // no connections, don't do it!
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Send_NetToLay(tolay);
}

void Layer::Send_NetToMe() {
  sent_already.Reset();
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i) {
    if(p->from->lesion) continue;
    int addr = (int)(long)p->from;
    if(sent_already.Find(addr) >= 0) continue;
    p->from->Send_NetToLay(this);
    sent_already.Add(addr);
  }
}

void Layer::Compute_Act() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Compute_Act();
}
void Layer::UpdateWeights() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->UpdateWeights();
}
void Layer::Compute_dWt() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Compute_dWt();
}
void Layer::Copy_Weights(const Layer* src) {
  units.Copy_Weights(&(src->units));
}
void Layer::WriteWeights(ostream& strm, Con_Group::WtSaveFormat fmt) {
  if(fmt == Con_Group::TEXT)
    strm << "#Layer " << name << "\n";
  units.WriteWeights(strm, fmt);
}
void Layer::ReadWeights(istream& strm, Con_Group::WtSaveFormat fmt) {
  units.ReadWeights(strm, fmt);
}

void Layer::TransformWeights(const SimpleMathSpec& trans) {
  units.TransformWeights(trans);
}

void Layer::AddNoiseToWeights(const Random& noise_spec) {
  units.AddNoiseToWeights(noise_spec);
}

int Layer::PruneCons(const SimpleMathSpec& pre_proc,
			CountParam::Relation rel, float cmp_val)
{
  return units.PruneCons(pre_proc, rel, cmp_val);
}

int Layer::ProbAddCons(float p_add_con, float init_wt) {
  int rval = 0;
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    rval += p->ProbAddCons(p_add_con, init_wt);
  return rval;
}

int Layer::LesionCons(float p_lesion, bool permute) {
  return units.LesionCons(p_lesion, permute);
}

int Layer::LesionUnits(float p_lesion, bool permute) {
  return units.LesionUnits(p_lesion, permute);
}

bool Layer::SetUnitSpec(UnitSpec* sp) {
  if(sp == NULL)	return false;
  unit_spec.SetSpec(sp);
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i) {
    if(sp->CheckObjectType(u))
      u->spec.SetSpec(sp);
    else
      return false;
  }
  return true;
}

void Layer::SetUnitType(TypeDef* td) {
  if(td == NULL) return;
  units.el_typ = td;
  if(units.gp.size > 0) {
    int j;
    for(j=0;j<units.gp.size;j++) {
      ((Unit_Group*)units.gp.FastEl(j))->el_typ = td;
    }
  }
}

bool Layer::SetConSpec(ConSpec* sp) {
  if(sp == NULL)	return false;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i) {
    if(!u->SetConSpec(sp))
      return false;
  }
  return true;
}

int Layer::ReplaceUnitSpec(UnitSpec* old_sp, UnitSpec* new_sp) {
  int nchg = 0;
  if(unit_spec.spec == old_sp) {
    unit_spec.SetSpec(new_sp);
    nchg++;
  }
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    nchg += u->ReplaceUnitSpec(old_sp, new_sp);
  return nchg;
}

int Layer::ReplaceConSpec(ConSpec* old_sp, ConSpec* new_sp) {
  int nchg = 0;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    nchg += u->ReplaceConSpec(old_sp, new_sp);
  Projection* p;
  FOR_ITR_EL(Projection, p, projections., i)
    nchg += p->ReplaceConSpec(old_sp, new_sp);
  return nchg;
}

int Layer::ReplacePrjnSpec(ProjectionSpec* old_sp, ProjectionSpec* new_sp) {
  int nchg = 0;
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i)
    nchg += p->ReplacePrjnSpec(old_sp, new_sp);
  return nchg;
}

int Layer::ReplaceLayerSpec(LayerSpec* old_sp, LayerSpec* new_sp) {
  if(GetLayerSpec() != old_sp) return 0;
  SetLayerSpec(new_sp);
  return 1;
}

void Layer::GridViewWeights(GridLog* grid_log, Layer* send_lay, bool use_swt, int un_x, int un_y, int wt_x, int wt_y) {
  if(send_lay == NULL) return;
  bool gotone = false;
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i) {
    if(p->from != send_lay) continue;
    p->GridViewWeights(grid_log, use_swt, un_x, un_y, wt_x, wt_y);
    gotone = true;
  }
  if(!gotone) {
    taMisc::Error("GridViewWeights: No sending projection from: ", send_lay->name);
  }
}

void Layer::WeightsToEnv(Environment* env, Layer* send_lay) {
  if(send_lay == NULL) return;
  bool gotone = false;
  Projection* p;
  taLeafItr i;
  FOR_ITR_EL(Projection, p, projections., i) {
    if(p->from != send_lay) continue;
    p->WeightsToEnv(env);
    gotone = true;
  }
  if(!gotone) {
    taMisc::Error("WeightsToEnv: No sending projection from: ", send_lay->name);
  }
}

Unit* Layer::FindUnitFmCoord(const TwoDCoord& coord) {
  if(units.gp.size == 0)	// no group structure, just do it
    return units.FindUnitFmCoord(coord);
  TwoDCoord gpcoord, uncoord;
  gpcoord.x = coord.x / geom.x;
  gpcoord.y = coord.y / geom.y;
  uncoord.x = coord.x % geom.x;
  uncoord.y = coord.y % geom.y;
  int gpidx = gpcoord.y * gp_geom.x + gpcoord.x;
  if((gpidx >= 0) && (gpidx < units.gp.size))
    return ((Unit_Group*)units.gp.FastEl(gpidx))->FindUnitFmCoord(uncoord);
  return NULL;
}

Unit_Group* Layer::FindUnitGpFmCoord(const TwoDCoord& coord) {
  if((coord.x < 0) || (coord.y < 0)) return NULL;
  int gidx = coord.y * gp_geom.x + coord.x;
  if(gidx >= units.gp.size) return NULL;
  return (Unit_Group*)units.gp.FastEl(gidx);
}

void Layer::GetActGeomNoSpc(PosTDCoord& nospc_geom) {
  nospc_geom = act_geom;
  if(units.gp.size == 0) return;
  nospc_geom.x -= (gp_geom.x - 1) * gp_spc.x;
  nospc_geom.y -= (gp_geom.y - 1) * gp_spc.y;
}

#ifdef DMEM_COMPILE
void Layer::DMem_DistributeUnits() {
  dmem_share_units.Reset();
  DMem_DistributeUnits_impl(dmem_share_units);
  dmem_share_units.Compile_ShareTypes();
}

bool Layer::DMem_DistributeUnits_impl(DMemShare& dms) {
  int np = 0; MPI_Comm_size(dmem_share_units.comm, &np);
  int this_proc = 0; MPI_Comm_rank(dmem_share_units.comm, &this_proc);
  if((dmem_dist == DMEM_DIST_DEFAULT) || (units.gp.size <= 0)) {
    int cnt = 0;
    taLeafItr ui;
    Unit* u;
    FOR_ITR_EL(Unit, u, units., ui) {
      u->DMem_SetLocalProc(cnt % np);
      u->DMem_SetThisProc(this_proc);
      dms.Link(u);
      cnt++;
    }
    return false;
  }
  else {
    int g;
    for(g=0; g<units.gp.size; g++) {
      Unit_Group* ug = (Unit_Group*)units.gp.FastEl(g);
      int cnt = 0;
      taLeafItr ui;
      Unit* u;
      FOR_ITR_EL(Unit, u, ug->, ui) {
	u->DMem_SetLocalProc(cnt % np);
	u->DMem_SetThisProc(this_proc);
	dms.Link(u);
	cnt++;
      }
    }
    return true;
  }
}

void Layer::DMem_SyncNRecvCons() {
  if(own_net->dmem_sync_level != Network::DMEM_SYNC_LAYER) {
    taMisc::Error("Error: attempt to DMem sync nrecv_cons at layer level, should only be at network level!");
    return;
  }
  dmem_share_units.Sync(0);
}

void Layer::DMem_SyncNet() {
  if(own_net->dmem_sync_level != Network::DMEM_SYNC_LAYER) {
    taMisc::Error("Error: attempt to DMem sync netin at layer level, should only be at network level!");
    return;
  }
  dmem_share_units.Sync(1);
}

void Layer::DMem_SyncAct() {
  if(own_net->dmem_sync_level != Network::DMEM_SYNC_LAYER) {
    taMisc::Error("Error: attempt to DMem sync act at layer level, should only be at network level!");
    return;
  }
  dmem_share_units.Sync(2);
}
#endif

////////////////////////
//	Network	      //
////////////////////////

void Network::Initialize() {
  views.SetBaseType(&TA_NetView);
  layers.SetBaseType(&TA_Layer);

  epoch = 0;
  re_init = false;
  dmem_sync_level = DMEM_SYNC_NETWORK;
  dmem_nprocs = 1;
  dmem_nprocs_actual = MIN(dmem_nprocs, taMisc::dmem_nprocs);
  usr1_save_fmt = FULL_NET;
  wt_save_fmt = TEXT;
  lay_layout = THREE_D;

  n_units = 0;
  n_cons = 0;
  max_size.x = GetDefaultX();
  max_size.y = GetDefaultY();
  max_size.z = GetDefaultZ();

  proj = NULL;
  net_will_updt = false;
#ifdef DMEM_COMPILE
  dmem_gp = -1;
  dmem_share_units.comm = (MPI_Comm)MPI_COMM_SELF;
#endif
}

void Network::InitLinks() {
  proj = GET_MY_OWNER(Project);
  taBase::Own(layers, this);
  taBase::Own(max_size, this);
#ifdef DMEM_COMPILE
  taBase::Own(dmem_share_units, this);
#endif
  WinMgr::InitLinks();		// do this after copy to get windows open afterwards
}

// cutlinks is in pdpshell.cc

void Network::Copy_(const Network& cp) {
  layers = cp.layers;
  max_size = cp.max_size;
  epoch = cp.epoch;
  re_init = cp.re_init;
  lay_layout = cp.lay_layout;
  CopyNetwork((Network*)&cp);
  ReConnect_Load();		// set the send cons
#ifdef DMEM_COMPILE
  DMem_DistributeUnits();
#endif
  ((Network&)cp).SyncSendPrjns(); // these get screwed up in there somewhere..
  if(taMisc::iv_active)
    InitAllViews();
}

void Network::UpdateAfterEdit(){
  WinMgr::UpdateAfterEdit();
  UpdtAfterNetMod();
  if(taMisc::is_loading || !taMisc::iv_active) return;
  UpdateAllViews();
}

void Network::UpdtAfterNetMod() {
  SyncSendPrjns();
  CountRecvCons();
#ifdef DMEM_COMPILE
  dmem_nprocs_actual = MIN(dmem_nprocs, taMisc::dmem_nprocs);
  DMem_SyncNRecvCons();
#endif
}

int Network::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = WinMgr::Dump_Load_Value(strm, par);
  if(rval) 
    ReConnect_Load();
  re_init = false;		// never re-init a just-loaded network

#ifdef DMEM_COMPILE
  DMem_DistributeUnits();
  DMem_PruneNonLocalCons();
#endif

  return rval;
}

int Network::GetDefaultX(){
  int rval = 1;
  switch(lay_layout) {
  case THREE_D:	rval = DEFAULT_3D_NETSIZE_X; break;
  case TWO_D:	rval = DEFAULT_2D_NETSIZE_X; break;
  }
  return rval;
}

int Network::GetDefaultY(){
  int rval = 1;
  switch(lay_layout) {
  case THREE_D:	rval = DEFAULT_3D_NETSIZE_Y; break;
  case TWO_D:	rval = DEFAULT_2D_NETSIZE_Y; break;
  }
  return rval;
}

int Network::GetDefaultZ(){
  int rval = 1;
  switch(lay_layout) {
  case THREE_D:	rval = DEFAULT_3D_NETSIZE_Z; break;
  case TWO_D:	rval = DEFAULT_2D_NETSIZE_Z; break;
  }
  return rval;
}

void Network::Build() {
  taMisc::Busy();
  net_will_updt = true;
  UpdateMax();
  Layer* l;
  taLeafItr i;

  FOR_ITR_EL(Layer, l, layers., i)
    l->Build();
  net_will_updt = false;
  UpdateMonitors();
  InitAllViews();
  taMisc::DoneBusy();
  UpdtAfterNetMod();
#ifdef DMEM_COMPILE
  DMem_DistributeUnits();
#endif
  if(!taMisc::iv_active)    return;
  NetView* nv;
  FOR_ITR_EL(NetView, nv, views., i) {
    if(nv->ordered_uvg_list.size == 0) {
      nv->SelectVar("act");
      if(nv->editor != NULL)
	nv->editor->SelectActButton(Tool::select);
    }
  }

}

void Network::PreConnect() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->PreConnect();
}

void Network::Connect() {
  taMisc::Busy();
  RemoveCons();
  SyncSendPrjns();
  // go in reverse order so that symmetric prjns can be made in
  // response to receiver-based projections
  Layer* l;
  int i;
  for(i=layers.leaves-1;i>=0;i--) {
    l = (Layer*)layers.Leaf(i);
    l->Connect();
  }
  UpdtAfterNetMod();
  UpdateMonitors();
  InitAllViews();
  taMisc::DoneBusy();
}

void Network::CountRecvCons() {
  n_units = 0;
  n_cons = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    n_cons += l->CountRecvCons();
    n_units += l->units.leaves;
  }
}

bool Network::CheckBuild() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(l->CheckBuild())
      return true;
  }
  return false;
}

bool Network::CheckConnect() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(l->CheckConnect())
      return true;
  }
  return false;
}

bool Network::CheckTypes() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->CheckTypes()) return false;
  }
  return true;
}

bool Network::CheckConfig(TrialProcess* tp, bool quiet) {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->CheckConfig(tp, quiet)) return false;
  }
  return true;
}

void Network::SyncSendPrjns() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    l->SyncSendPrjns();
  }
}

void Network::FixPrjnIndexes() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    l->FixPrjnIndexes();
  }
}

void Network::ConnectUnits(Unit* u_to, Unit* u_from, bool record,
			   ConSpec* conspec)
{
  if(u_to == NULL) return; // must have reciever
  if(u_from == NULL)    u_from = u_to; // assume self con if no from

  Layer* lay = GET_OWNER(u_to,Layer);
  Layer* l_from = GET_OWNER(u_from,Layer);
  Projection* pjn = NULL;
  taLeafItr p;
  // check to see if a pjrn already exists
  FOR_ITR_EL(Projection, pjn, lay->projections., p) {
    if((pjn->from == l_from) &&
       (pjn->spec->InheritsFrom(&TA_CustomPrjnSpec)) &&
       ((conspec == NULL) || (pjn->con_spec == conspec)))
      break; // ok found one
  }
  if(pjn==NULL) { // no projection
#ifdef DMEM_COMPILE
    if(record && (taMisc::dmem_nprocs == 1)) { // don't actually run under gui in dmem mode
#endif
      pjn = (Projection*) lay->projections.New(1);
      pjn->SetCustomFrom(l_from);
      pjn->spec.type = &TA_CustomPrjnSpec;
      if(conspec != NULL)
	pjn->con_spec.SetSpec(conspec);
      pjn->spec.UpdateAfterEdit();
      pjn->projected = true;
      pjn->UpdateAfterEdit();
#ifdef DMEM_COMPILE
    }
#endif
    if(record) {
      taivMisc::RecordScript(lay->projections.GetPath() + ".NewEl(1);\n");
      taivMisc::SREAssignment(pjn,pjn->FindMember("from_type"));
      taivMisc::ScriptRecordAssignment(pjn,pjn->FindMember("from"));
      taivMisc::RecordScript(pjn->GetPath() + ".spec.type = CustomPrjnSpec;");
    }
  }
  
  u_to->ConnectFromCk(u_from,pjn);
  if(record == true) taivMisc::RecordScript(u_to->GetPath() + ".ConnectFromCk(" + u_from->GetPath() +
					    ", " + pjn->GetPath() + ");\n");
  lay->UpdateAfterEdit();
}

void Network::ReConnect_Load() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->ReConnect_Load();
}

void Network::CopyNetwork(Network* net) {
  Layer* l;
  taLeafItr li;
  Layer* cl;
  taLeafItr cli;
  for(l = (Layer*)layers.FirstEl(li), cl = (Layer*)net->layers.FirstEl(cli);
      l && cl;
      l = (Layer*)layers.NextEl(li), cl = (Layer*)net->layers.NextEl(cli)) {
    l->CopyNetwork(this, net, cl);
  }
  UpdtAfterNetMod();
}

void Network::RemoveUnits() {
  taMisc::Busy();
  net_will_updt = true;
  RemoveMonitors();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->RemoveUnits();
  net_will_updt = false;
  InitAllViews();
  n_cons = 0;
  taMisc::DoneBusy();
}

void Network::RemoveUnitGroups() {
  taMisc::Busy();
  net_will_updt = true;
  RemoveMonitors();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->RemoveUnitGroups();
  net_will_updt = false;
  InitAllViews();
  taMisc::DoneBusy();
}

void Network::LayoutUnitGroups() {
  taMisc::Busy();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->LayoutUnitGroups();
  net_will_updt = false;
  InitAllViews();
  taMisc::DoneBusy();
}

void Network::RemoveCons() {
  taMisc::Busy();
  RemoveMonitors();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i)
    l->RemoveCons();
  InitAllViews();
  taMisc::DoneBusy();
}

void Network::InitExterns(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitExterns();
  }
}

void Network::InitDelta(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitDelta();
  }
}

void Network::InitState(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitState();
  }
}

#ifdef DMEM_COMPILE

void Network::DMem_SyncNRecvCons() {
  if(n_cons <= 0) return;
  if(dmem_sync_level == DMEM_SYNC_LAYER) {
    Layer* l;
    taLeafItr i;
    FOR_ITR_EL(Layer, l, layers., i) {
      if(!l->lesion)
	l->DMem_SyncNRecvCons();
    }
  }
  else {
    dmem_share_units.Sync(0);
  }
}

void Network::DMem_SyncNet() {
  if(dmem_sync_level != DMEM_SYNC_NETWORK) {
    taMisc::Error("Error: attempt to DMem sync netin at network level, should only be at layer level!");
    return;
  }
  dmem_share_units.Sync(1);
}

void Network::DMem_SyncAct() {
  if(dmem_sync_level != DMEM_SYNC_NETWORK) {
    taMisc::Error("Error: attempt to DMem sync act at network level, should only be at layer level!");
    return;
  }
  dmem_share_units.Sync(2);
}

void Network::DMem_DistributeUnits() {
  //  cerr << "proc " << taMisc::dmem_proc << " in distribunits" << endl;
  dmem_nprocs_actual = MIN(dmem_nprocs, taMisc::dmem_nprocs);

  if(dmem_nprocs_actual > 1) {
    MPI_Group worldgp;
    DMEM_MPICALL(MPI_Comm_group(MPI_COMM_WORLD, &worldgp),
		       "Network::DMem_DistributeUnits", "Comm_group");

    if(dmem_share_units.comm != MPI_COMM_SELF) {
      DMEM_MPICALL(MPI_Comm_free((MPI_Comm*)&dmem_share_units.comm),
			 "Network::DMem_DistributeUnits", "net Comm_free");
      DMEM_MPICALL(MPI_Group_free((MPI_Group*)&dmem_gp),
			 "Network::DMem_DistributeUnits", "net Group_free");
    }

    int myepc = taMisc::dmem_proc / dmem_nprocs_actual;
    // outer-loop is epoch-wise: if there are extra procs they are for that
    int stnet = myepc * dmem_nprocs_actual;
    int net_ranks[dmem_nprocs_actual];
    for(int i = 0; i<dmem_nprocs_actual; i++)
      net_ranks[i] = stnet + i;

    DMEM_MPICALL(MPI_Group_incl(worldgp, dmem_nprocs_actual, net_ranks, (MPI_Group*)&dmem_gp),
		       "Network::DMem_DistributeUnits", "net Group_incl");
    DMEM_MPICALL(MPI_Comm_create(MPI_COMM_WORLD, (MPI_Group)dmem_gp, (MPI_Comm*)&dmem_share_units.comm),
		       "Network::DMem_DistributeUnits", "net Comm_create");
  }
  else {
    dmem_share_units.comm = MPI_COMM_SELF;
    dmem_gp = -1;
  }

  dmem_share_units.Reset();
  bool any_custom_distrib = false;
  Layer* lay;
  taLeafItr li;
  FOR_ITR_EL(Layer, lay, layers., li) {
    lay->dmem_share_units.comm = dmem_share_units.comm;
    if(dmem_sync_level == DMEM_SYNC_LAYER) {
      lay->DMem_DistributeUnits();
    }
    else {
      if(lay->DMem_DistributeUnits_impl(dmem_share_units))
	any_custom_distrib = true;
    }
  }
  if(dmem_sync_level == DMEM_SYNC_NETWORK) {
    if(!any_custom_distrib) {
      dmem_share_units.DistributeItems(); // use more efficient full distribution
    }
    else
      dmem_share_units.Compile_ShareTypes(); // use custom distribution: just compile it
  }
}

void Network::DMem_PruneNonLocalCons() {
  if(dmem_nprocs_actual <= 1) return;
  taLeafItr li, ui;
  Unit* u;
  Layer *l;
  FOR_ITR_EL(Layer, l, layers., li) {
    FOR_ITR_EL(Unit, u, l->units., ui) {
      if(u->DMem_IsLocal()) {
	continue;
      }
      // only non-local
      Con_Group* recv_gp;
      int g;
      for (g = 0; g < u->recv.gp.size; g++) {
	recv_gp = (Con_Group *)u->recv.FastGp(g);
	if(recv_gp->spec->DMem_AlwaysLocal()) continue;
	for (int sui = recv_gp->size-1; sui >= 0; sui--) {
	  u->DisConnectFrom(recv_gp->Un(sui), NULL);
	}
      }
    }
  }
}

void Network::DMem_SyncWts(MPI_Comm comm, bool sum_dwts) {
  static float_Array values;
  static float_Array results;

  int np = 0; MPI_Comm_size(comm, &np);
  if(np <= 1) return;

//   if(taMisc::dmem_debug)
//     cerr << "proc: " << taMisc::dmem_proc << " at syncwts barrier!" << endl;
//   DMEM_MPICALL(MPI_Barrier(comm),"Network::SyncWts", "Barrier");
//   if(taMisc::dmem_debug && (taMisc::dmem_proc == 0))
//     cerr << "---------- past syncwts barrier ---------" << endl;

  values.EnforceSize(n_cons + n_units);

  int cidx = 0;
  Layer* lay;
  taLeafItr li;
  FOR_ITR_EL(Layer, lay, layers., li) {
    if(lay->projections.size == 0) continue;
    int dwt_off = 0;
    if(sum_dwts) {
      Projection* prjn = (Projection*)lay->projections.FastEl(0);
      MemberDef* md = prjn->con_spec->DMem_EpochShareDwtVar();
      if(md == NULL) continue;
      dwt_off = (int)md->GetOff((void*)0x100) - 0x100;
    }
    Unit* un;
    taLeafItr ui;
    FOR_ITR_EL(Unit, un, lay->units., ui) {
      if(un->bias != NULL) {
	if(sum_dwts) {
	  values.FastEl(cidx++) = *((float*)((char*)(un->bias) + dwt_off));
	}
	else {
	  values.FastEl(cidx++) = un->bias->wt;
	}
      }

      Con_Group* cg;
      int gi;
      FOR_ITR_GP(Con_Group, cg, un->recv., gi) {
	if(sum_dwts) {
	  for(int i = 0;i<cg->size;i++) values.FastEl(cidx++) = *((float*)((char*)(cg->Cn(i)) + dwt_off));
	}
	else {
	  for(int i = 0;i<cg->size;i++) values.FastEl(cidx++) = cg->Cn(i)->wt;
	}
      }
    }
  }

  results.EnforceSize(cidx);
  DMEM_MPICALL(MPI_Allreduce(values.el, results.el, cidx, MPI_FLOAT, MPI_SUM, comm),
		     "Network::SyncWts", "Allreduce");

  float avg_mult = 1.0f / (float)np;
  cidx = 0;
  FOR_ITR_EL(Layer, lay, layers., li) {
    if(lay->projections.size == 0) continue;
    int dwt_off = 0;
    if(sum_dwts) {
      Projection* prjn = (Projection*)lay->projections.FastEl(0);
      MemberDef* md = prjn->con_spec->DMem_EpochShareDwtVar();
      if(md == NULL) continue;
      dwt_off = (int)md->GetOff((void*)0x100) - 0x100;
    }
    Unit* un;
    taLeafItr ui;
    FOR_ITR_EL(Unit, un, lay->units., ui) {
      if(un->bias != NULL) {
	if(sum_dwts) {
	  *((float*)((char*)(un->bias) + dwt_off)) = results.FastEl(cidx++);
	}
	else {
	  un->bias->wt = avg_mult * results.FastEl(cidx++);
	}
      }
      Con_Group* cg;
      int gi;
      FOR_ITR_GP(Con_Group, cg, un->recv., gi) {
	if(sum_dwts) {
	  for(int i = 0;i<cg->size;i++) *((float*)((char*)(cg->Cn(i)) + dwt_off)) = results.FastEl(cidx++);
	}
	else {
	  for(int i = 0;i<cg->size;i++) cg->Cn(i)->wt = avg_mult * results.FastEl(cidx++);
	}
      }
    }
  }
}

void Network::DMem_SymmetrizeWts() {
  MPI_Comm comm = dmem_share_units.comm;
  int np = 0; MPI_Comm_size(comm, &np);

  if(np <= 1) return;

  static int_Array unit_idxs;
  static float_Array wt_vals;

  static int_Array all_unit_idxs;
  static float_Array all_wt_vals;

  Layer* lay;
  taLeafItr li;
  FOR_ITR_EL(Layer, lay, layers., li) {
    if(lay->projections.size == 0) continue;
    Unit* un;
    taLeafItr ui;
    FOR_ITR_EL(Unit, un, lay->units., ui) {
      int gi;
      for(gi=0;gi<un->recv.gp.size;gi++) {
	Con_Group* cg = (Con_Group*)un->recv.gp[gi];
	if(!cg->spec->wt_limits.sym) continue;

	// check for presence of reciprocal connections in the first place..
	Layer* fmlay = cg->prjn->from;
	bool has_recip_prjn = false;
	Projection* fmpj;
	taLeafItr pji;
	FOR_ITR_EL(Projection, fmpj, fmlay->projections., pji) {
	  if(fmpj->from == lay) {
	    has_recip_prjn = true;
	    break;
	  }
	}
	if(!has_recip_prjn) continue; // no sym cons there anyway

	if(un->DMem_IsLocal()) {
	  // I'm local: I recv values from all other procs: each sends unit index and wt
	  all_unit_idxs.Reset();
	  all_wt_vals.Reset();
	  int proc;
	  for(proc = 0; proc < np; proc++) {
	    if(proc == un->dmem_local_proc) continue;
	    // recv the number of connection values obtained
	    int msgsize = 0;
	    MPI_Status status;
	    DMEM_MPICALL(MPI_Recv((void*)&msgsize, 1, MPI_INT, proc, 101, comm, &status),
			 "DMem_SymmetrizeWts", "MPI_Recv msgsize");
	    if(msgsize > 0) {
	      unit_idxs.EnforceSize(msgsize);
	      wt_vals.EnforceSize(msgsize);
	      DMEM_MPICALL(MPI_Recv(unit_idxs.el, msgsize, MPI_INT, proc, 102, comm, &status),
			   "DMem_SymmetrizeWts", "MPI_Recv unit_idxs");
	      DMEM_MPICALL(MPI_Recv(wt_vals.el, msgsize, MPI_FLOAT, proc, 103, comm, &status),
			   "DMem_SymmetrizeWts", "MPI_Recv wt_vals");

	      all_unit_idxs.CopyVals(unit_idxs, 0, -1, all_unit_idxs.size);
	      all_wt_vals.CopyVals(wt_vals, 0, -1, all_wt_vals.size);
	    }
	  }
	  // now have all the data collected, to through and get the sym values!
	  int i;
	  for(i=0;i<cg->size;i++) {
	    Unit* fm = cg->Un(i);
	    int uidx = cg->prjn->from->units.FindLeaf(fm);
	    if(uidx < 0) continue;
	    int sidx = all_unit_idxs.Find(uidx);
	    if(sidx < 0) continue;
	    cg->Cn(i)->wt = all_wt_vals[sidx];
	  }
	}
	else {
	  // collect my data and send it off!
	  unit_idxs.Reset();
	  wt_vals.Reset();
	  int uni;
	  for(uni=0;uni<fmlay->units.leaves;uni++) {
	    Unit* fm = fmlay->units.Leaf(uni);
	    if(!fm->DMem_IsLocal()) continue;
	    int fmgi;
	    Con_Group* fmg;
	    FOR_ITR_GP(Con_Group, fmg, fm->recv., fmgi) {
	      if(fmg->prjn->from != lay) continue;
	      Connection* con = fmg->FindConFrom(un);
	      if(con != NULL) {
		unit_idxs.Add(uni);
		wt_vals.Add(con->wt);
	      }
	    }
	  }
	  // send the number, then the data
	  int msgsize = unit_idxs.size;
	  DMEM_MPICALL(MPI_Send((void*)&msgsize, 1, MPI_INT, un->dmem_local_proc, 101, comm),
		       "DMem_SymmetrizeWts", "MPI_Send msgsize");

	  if(msgsize > 0) {
	    DMEM_MPICALL(MPI_Send(unit_idxs.el, msgsize, MPI_INT, un->dmem_local_proc, 102, comm),
			 "DMem_SymmetrizeWts", "MPI_Send unit_idxs");
	    DMEM_MPICALL(MPI_Send(wt_vals.el, msgsize, MPI_FLOAT, un->dmem_local_proc, 103, comm),
			 "DMem_SymmetrizeWts", "MPI_Send wt_vals");
	  }
	}
      }
    }
  }
//   if(taMisc::dmem_debug)
//     cerr << "proc: " << taMisc::dmem_proc << " at sym_wts barrier!" << endl;
//   DMEM_MPICALL(MPI_Barrier(comm),"Network::SymmetrizeWts", "Barrier");
//   if(taMisc::dmem_debug && (taMisc::dmem_proc == 0))
//     cerr << "---------- past sym_wts barrier ---------" << endl;
}

#endif

void Network::ModifyState(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->ModifyState();
  }

}

void Network::InitWtDelta(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitWtDelta();
  }
}

void Network::InitWtState() {
  // do lots of checking here to make sure, cuz often 1st thing that happens
  if(!CheckTypes()) return;
  if(CheckBuild()) {
    taMisc::Error("Network:",name,"Needs the 'Build' command to be run");
    return;
  }
  if(CheckConnect()) {
    taMisc::Error("Network:",name, "Needs the 'Connect' command to be run");
    return;
  }

  taMisc::Busy();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState();
  }
  InitWtState_post();		// done after all initialization (for scaling wts...)

#ifdef DMEM_COMPILE
  // do the dmem weight symmetrizing!
  DMem_SymmetrizeWts();
#endif

  InitState();			// also re-init state at this point..
  epoch = 0;			// re-init epoch counter..
  re_init = false;		// no need to re-reinit!
  UpdateAllViews();
  taMisc::DoneBusy();
}

void Network::InitWtState_post() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState_post();
  }
}

void Network::Compute_Net() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Net();
  }
#ifdef DMEM_COMPILE
  DMem_SyncNet();
#endif
}

void Network::Send_Net() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Send_Net();
  }
#ifdef DMEM_COMPILE
  DMem_SyncNet();
#endif
}

void Network::Compute_Act() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Act();
  }
}

void Network::UpdateWeights() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->UpdateWeights();
  }
}

void Network::Compute_dWt() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_dWt();
  }
}

void Network::Copy_Weights(const Network* src) {
  taMisc::Busy();
  Layer* l, *sl;
  taLeafItr i,si;
  for(l = (Layer*)layers.FirstEl(i), sl = (Layer*)src->layers.FirstEl(si);
      (l != NULL) && (sl != NULL);
      l = (Layer*)layers.NextEl(i), sl = (Layer*)src->layers.NextEl(si))
  {
    if(!l->lesion && !sl->lesion)
      l->Copy_Weights(sl);
  }
  UpdateAllViews();
  taMisc::DoneBusy();
}

void Network::WriteWeights(ostream& strm, Network::WtSaveFormat fmt) {
  taMisc::Busy();
  strm << "#Fmt " << fmt << "\n"
       << "#Name " << name << "\n"
       << "#Epoch " << epoch << "\n"
       << "#ReInit " << (int)re_init << "\n";
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->WriteWeights(strm, (Con_Group::WtSaveFormat)fmt);
  }
  taMisc::DoneBusy();
}
  
void Network::ReadWeights(istream& strm) {
  taMisc::Busy();
  int c = strm.peek();
  if(c != '#') {
    taMisc::Error("ReadWeights: file format is incorrect, expected #");
    return;
  }
  strm.get();
  c = strm.peek();
  if(!((c == 'F') || (c == 'N'))) {
    taMisc::Error("ReadWeights: file format is incorrect, expected Fmt or Name");
    return;
  }
  if(c == 'F') {
    c = taMisc::read_alnum_noeol(strm);
    c = taMisc::read_till_eol(strm);
    wt_save_fmt = (WtSaveFormat)(int)taMisc::LexBuf;
  }
  else {
    wt_save_fmt = TEXT;		// default is text for old files that don't have Fmt saved
  }
  c = taMisc::read_alnum_noeol(strm);
  c = taMisc::read_till_eol(strm);
  name = taMisc::LexBuf;
  c = taMisc::read_alnum_noeol(strm); // get #Epoch
  c = taMisc::read_till_eol(strm); // get epoch
  epoch = (int)taMisc::LexBuf;
  c = taMisc::read_alnum_noeol(strm); // get #ReInit
  c = taMisc::read_till_eol(strm); // get re_init
  re_init = (int)taMisc::LexBuf;

  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->ReadWeights(strm, (Con_Group::WtSaveFormat)wt_save_fmt);
    if(strm.eof()) break;
  }
  re_init = false;		// never re-init after loading weights
  UpdateAllViews();
  taMisc::DoneBusy();
}

void Network::TransformWeights(const SimpleMathSpec& trans) {
  taMisc::Busy();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->TransformWeights(trans);
  }
  UpdateAllViews();
  taMisc::DoneBusy();
}

void Network::AddNoiseToWeights(const Random& noise_spec) {
  taMisc::Busy();
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->AddNoiseToWeights(noise_spec);
  }
  UpdateAllViews();
  taMisc::DoneBusy();
}

int Network::PruneCons(const SimpleMathSpec& pre_proc,
			  CountParam::Relation rel, float cmp_val)
{
  taMisc::Busy();
  int rval = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      rval += l->PruneCons(pre_proc, rel, cmp_val);
  }
  InitAllViews();
  taMisc::DoneBusy();
  return rval;
}

int Network::ProbAddCons(float p_add_con, float init_wt) {
  taMisc::Busy();
  int rval = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      rval += l->ProbAddCons(p_add_con, init_wt);
  }
  InitAllViews();
  taMisc::DoneBusy();
  return rval;
}

int Network::LesionCons(float p_lesion, bool permute) {
  taMisc::Busy();
  int rval = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      rval += l->LesionCons(p_lesion, permute);
  }
  InitAllViews();
  taMisc::DoneBusy();
  return rval;
}

int Network::LesionUnits(float p_lesion, bool permute) {
  taMisc::Busy();
  net_will_updt = true;
  int rval = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      rval += l->LesionUnits(p_lesion, permute);
  }
  net_will_updt = false;
  InitAllViews();
  taMisc::DoneBusy();
  return rval;
}

void Network::UpdateMax() {
  max_size.x = 1;  max_size.y = 1;  max_size.z = 0;

  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    max_size.x = MAX(max_size.x, l->act_geom.x + l->pos.x);
    max_size.y = MAX(max_size.y, l->act_geom.y + l->pos.y);
    max_size.z = MAX(max_size.z, 1 + l->pos.z);
  }
  max_size.y += 1;		// increment max y by one (for extra spacing)
  if(max_size.x == 1) max_size.x = GetDefaultX();
  if(max_size.y == 2) max_size.y = GetDefaultY();
  if(max_size.z == 0) max_size.z = GetDefaultZ();
}

void Network::FixLayerViews(Layer* lay){
  NetView* view;
  taLeafItr i;
  FOR_ITR_EL(NetView, view, views., i) {
    view->FixLayer(lay);
  }
}

void Network::TwoD_Or_ThreeD(LayerLayout lo){
  lay_layout = lo;
  Layer * lay;
  taLeafItr j;
  FOR_ITR_EL(Layer, lay, layers., j){
    lay->SetDefaultPos();
  }
  NetView* nv;
  taLeafItr i;
  FOR_ITR_EL(NetView, nv, views., i) {
    nv->SetDefaultSkew();
    nv->AutoPositionPrjnPoints();
    nv->UpdateAfterEdit();
  }
}

int Network::ReplaceUnitSpec(UnitSpec* old_sp, UnitSpec* new_sp) {
  int nchg = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      nchg += l->ReplaceUnitSpec(old_sp, new_sp);
  }
  return nchg;
}

int Network::ReplaceConSpec(ConSpec* old_sp, ConSpec* new_sp) {
  int nchg = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      nchg += l->ReplaceConSpec(old_sp, new_sp);
  }
  return nchg;
}

int Network::ReplacePrjnSpec(ProjectionSpec* old_sp, ProjectionSpec* new_sp) {
  int nchg = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      nchg += l->ReplacePrjnSpec(old_sp, new_sp);
  }
  return nchg;
}

int Network::ReplaceLayerSpec(LayerSpec* old_sp, LayerSpec* new_sp) {
  int nchg = 0;
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      nchg += l->ReplaceLayerSpec(old_sp, new_sp);
  }
  return nchg;
}

void Network::GridViewWeights(GridLog* grid_log, Layer* recv_lay, Layer* send_lay,
			      bool use_swt, int un_x, int un_y, int wt_x, int wt_y)
{
  if(recv_lay == NULL) return;
  recv_lay->GridViewWeights(grid_log, send_lay, use_swt, un_x, un_y, wt_x, wt_y);
}

void Network::WeightsToEnv(Environment* env, Layer* recv_lay, Layer* send_lay)
{
  if(recv_lay == NULL) return;
  recv_lay->WeightsToEnv(env, send_lay);
}

////////////////////////////
//   	NetViewGraphic    //
////////////////////////////

void NetViewGraphic::Initialize(){
  graphic_xform = NULL;
  master = NULL;
  viewer = NULL;
  net_graphic = NULL;
  idraw_file = NULL;
  graphic_xform = NULL;
}


void NetViewGraphic::InitLinks(){
  taNBase::InitLinks();
  NetView* nv = GET_MY_OWNER(NetView);
  if((nv != NULL) && (nv->editor != NULL)) {
    viewer = nv->editor->viewer;
    master = nv->editor->netg;
  }
  else return;
  idraw_file = new taivGetFile(".","*.*",nv->window,nv->wkit->style(),false);
  taRefN::Ref(idraw_file);
  idraw_file->select_only = true;
}

void NetViewGraphic::Destroy() {
  CutLinks();
  master = NULL;
  viewer = NULL;
}

void NetViewGraphic::CutLinks(){
  taNBase::CutLinks();
  RemoveFromView();
  if(idraw_file != NULL)
    taRefN::unRefDone(idraw_file);
  idraw_file = NULL;
  if(graphic_xform != NULL) {
    taBase::DelPointer((TAPtr*)&graphic_xform);
    graphic_xform = NULL;
  }
}

void NetViewGraphic::Copy_(const NetViewGraphic& cp){
  name = cp.name;
  if(cp.graphic_xform != NULL) SetGraphicXform(cp.graphic_xform);
}

void NetViewGraphic::GetMasterViewer(){
  NetView* nv = GET_MY_OWNER(NetView);
  if((nv != NULL) && (nv->editor != NULL)) {
    viewer = nv->editor->viewer;
    master = nv->editor->netg;
  }
}

void NetViewGraphic::SetGraphicXform(Xform* xf){
  if(graphic_xform != NULL) taBase::DelPointer((TAPtr*)&graphic_xform);
  graphic_xform = xf;
  if(xf != NULL) taBase::Own(xf,this);
}

void NetViewGraphic::SetGraphicXform(ivTransformer* t){
  if(t == NULL) return;
  if(graphic_xform == NULL) {
    graphic_xform = new Xform(t);
    taBase::Own(graphic_xform, this);
  }
  else {
    graphic_xform->Set(t);
  }
}
    
void NetViewGraphic::UpdateAfterEdit(){
  GetMasterViewer();
  Network_G* netg = (Network_G *) master;
  NetView* nv = GET_MY_OWNER(NetView);
  if((nv == NULL) || (nv->window == NULL)) return;
  if(idraw_file == NULL) {
    idraw_file = new taivGetFile(".","*.*",nv->window,nv->wkit->style(),false);
    idraw_file->select_only = true;
    taRefN::Ref(idraw_file);
  }
  if(!name.empty() && (idraw_file != NULL) && (idraw_file->fname.empty()))
    idraw_file->fname = name;
  if(idraw_file->fname.empty()) return;
  if((net_graphic == NULL) || (!name.empty() && (name != idraw_file->fname))) {
    if(net_graphic) RemoveFromView();
    if(netg->LoadGraphic(idraw_file->fname.chars()) == false) return; // error
    net_graphic = master->component_(0); // graphic was prepended    
    ((NetViewGraphic_G *) net_graphic)->netvg = this;
  }    
  name = idraw_file->fname;
  if(graphic_xform != NULL) {
    if(net_graphic->transformer() == NULL) {
      ivTransformer* t = graphic_xform->transformer();
      net_graphic->transformer(t);
      ivResource::unref(t);
    }
  }
  else {
    graphic_xform = new Xform(net_graphic->transformer());
    taBase::Own(graphic_xform,this);
  }
  nv->has_graphic = true;
  
}

void NetViewGraphic::RemoveFromView(){
  if((net_graphic == NULL) || (viewer == NULL) || (master == NULL))  return;
  Graphic* vlg;
  ivGlyphIndex j;
  for(j=master->count_()-1; j>=0; j--) {
    vlg = master->component_(j);
    if(vlg == net_graphic) {
      if(viewer->canvas() != NULL) 
	vlg->damage_me(viewer->canvas());
      master->remove_(j);
      break; // we've found and removed the graphic no need to keep searching
    }
  }
}


/////////////////////////
//   	NetView Misc   //
/////////////////////////

void NetViewScaleRange::UpdateAfterEdit(){
  if(taMisc::is_loading || !taMisc::iv_active) return;
  NetView* nv = GET_MY_OWNER(NetView);
  if((nv == NULL)||(nv->editor == NULL)||(nv->editor->netg == NULL)) return;
  if(nv->ordered_uvg_list.size == 1) {
    MemberDef* md =
      (MemberDef *) nv->editor->netg->membs[nv->ordered_uvg_list[0]];
    if((md != NULL) && (md->name == name)){
      nv->auto_scale = nv->editor->auto_scale = auto_scale;
      nv->editor->as_ckbox->state()->set(ivTelltaleState::is_chosen,auto_scale);
      nv->scale_min = minmax.min;
      nv->scale_max = minmax.max;
      nv->editor->cbar->SetMinMax(minmax.min,minmax.max);
      nv->editor->UpdateDisplay();
    }
  }
} 

void NetViewLabel::Initialize() {
  label_gp = 0;
}

void NetViewLabel::GetMasterViewer() {
  NetView* nv = GET_MY_OWNER(NetView);
  if((nv != NULL) && (nv->editor != NULL)) {
    viewer = nv->editor->viewer;
    master = nv->editor->netg;
  }
}

void NetViewLabel::AddToView() {
  if(label_gp <= 0) {
    ViewLabel::AddToView();
    return;
  }
  else {
    if(UpdateView()) return;
    int gp_cnt = 0;
    int i;
    for(i=0;i<master->count_();i++) {
      Graphic* gr = master->component_(i);
      if(gr->InheritsFrom(&TA_NetViewLabelGroup_G)) {
	gp_cnt++;
	if(gp_cnt == label_gp) {
	  NetViewLabelGroup_G* grp = (NetViewLabelGroup_G*)gr;
	  grp->append_(spec.text_g);
	  return;		// done!
	}
      }
    }
    gp_cnt++;
    label_gp = gp_cnt;
    NetViewLabelGroup_G* grp = new NetViewLabelGroup_G(NULL);
    grp->append_(spec.text_g);
    master->append_(grp);
  }
}

bool NetViewLabel::UpdateView() {
  if((viewer == NULL) || (master == NULL) || (spec.text_g == NULL))  return false;

  if(label_gp <= 0) {
    return ViewLabel::UpdateView();
  }
  else {
    spec.text_g->select_effect = select_effect;
    spec.text_g->text(name);
    if(label_xform != NULL) {
      ivTransformer* tx = label_xform->transformer();
      spec.text_g->transformer(tx);
      ivResource::unref(tx);	// unref the self-ref from transform
    }
    int gp_cnt = 0;
    int i;
    for(i=0;i<master->count_();i++) {
      Graphic* gr = master->component_(i);
      if(gr->InheritsFrom(&TA_NetViewLabelGroup_G)) {
	gp_cnt++;
	if(gp_cnt == label_gp) {
	  NetViewLabelGroup_G* grp = (NetViewLabelGroup_G*)gr;

	  Graphic* vlg;
	  ivGlyphIndex j;
	  for(j=grp->count_()-1; j>=0; j--) {
	    vlg = grp->component_(j);
	    if ((vlg->InheritsFrom(&TA_NoScale_Text_G)) &&
		(((NoScale_Text_G *) vlg)->obj == this)) {
	      if(viewer->canvas() != NULL) 
		vlg->damage_me(viewer->canvas());
	      return true;
	    }
	  }
	}
      }
    }
  }
  return false;
} 

void NetViewLabel::RemoveFromView() {
  if((viewer == NULL) || (master == NULL))  return;

  if(label_gp <= 0) {
    ViewLabel::RemoveFromView();
    return;
  }
  else {
    int gp_cnt = 0;
    int i;
    for(i=0;i<master->count_();i++) {
      Graphic* gr = master->component_(i);
      if(gr->InheritsFrom(&TA_NetViewLabelGroup_G)) {
	gp_cnt++;
	if(gp_cnt == label_gp) {
	  NetViewLabelGroup_G* grp = (NetViewLabelGroup_G*)gr;

	  Graphic* vlg;
	  ivGlyphIndex j;
	  for(j=grp->count_()-1; j>=0; j--) {
	    vlg = grp->component_(j);
	    if ((vlg->InheritsFrom(&TA_NoScale_Text_G)) &&
		(((NoScale_Text_G *) vlg)->obj == this)) {
	      if(viewer->canvas() != NULL) 
		vlg->damage_me(viewer->canvas());
	      grp->remove_(j);
	      return;
	    }
	  }
	}
      }
    }
  }
} 

/////////////////////
//   	NetView    //
/////////////////////

void NetView::Initialize() {
  editor = NULL;
  skew = 0.0f;
  shape = THREE_D;
  prjn_arrow_angle = 0.6f;
  prjn_arrow_size = 8.0f;
  colorspec = NULL;
  unit_text = NONE;
  unit_layout = TOP_BOTTOM;
  auto_scale = true;
  spacing.x = .1f;
  spacing.y = .15f;
  spacing.z = .05f;
  unit_border = 0.0f;
  scale_min = 0.0f;
  scale_max = 1.0f;
  lay_forms = NULL;
  label_forms = NULL;
  frozen_net_form = NULL;
  has_graphic = false;
  labels.SetBaseType(&TA_NetViewLabel);
  idraw_graphics.SetBaseType(&TA_NetViewGraphic);
  scale_ranges.SetBaseType(&TA_NetViewScaleRange);
}

void NetView::InitLinks() {
  taBase::Own(anim,this);
  taBase::Own(labels,this);
  taBase::Own(idraw_graphics,this);
  taBase::Own(scale_ranges,this);
  taBase::Own(layer_font,this);
  taBase::Own(unit_font,this);
  taBase::Own(ordered_uvg_list, this);
  taBase::Own(spacing, this);
// leave this as null by default
//  taBase::SetPointer((TAPtr*)&colorspec, pdpMisc::GetDefaultColor());
  PDPView::InitLinks();
  SetDefaultSkew(); // need mgr (network) from above to get 2d/3d skew
  if(taMisc::iv_active && (editor != NULL))
    editor->SelectActButton(Tool::select); // select act button!

  // need a small font!
#ifndef CYGWIN
  layer_font.SetFont("*-Helvetica-medium-r*18*");
#else
  layer_font.SetFont("*Arial*medium*--18*");
#endif
}
  

void NetView::CutLinks() {
  labels.RemoveAll();		// first get rid of labels
  labels.CutLinks();
  idraw_graphics.RemoveAll();
  idraw_graphics.CutLinks();
  scale_ranges.RemoveAll();
  scale_ranges.CutLinks();
  layer_font.CutLinks();
  unit_font.CutLinks();
  ordered_uvg_list.CutLinks();
  PDPView::CutLinks();		// view closes window first, then delete this stuff..
  taBase::DelPointer((TAPtr*)&colorspec);
  taBase::DelPointer((TAPtr*)&frozen_net_form);
  taBase::DelPointer((TAPtr*)&lay_forms);
  taBase::DelPointer((TAPtr*)&label_forms);
  if(editor != NULL) { delete editor; editor = NULL; }
}

void NetView::Copy_(const NetView& cp) {
  skew = cp.skew;
  shape = cp.shape;
  prjn_arrow_size = cp.prjn_arrow_size;
  prjn_arrow_angle = cp.prjn_arrow_angle;
  taBase::SetPointer((TAPtr*)&colorspec,cp.colorspec);
  layer_font = cp.layer_font;
  unit_font = cp.unit_font;
  labels = cp.labels;
  scale_ranges = cp.scale_ranges;
  idraw_graphics = cp.idraw_graphics;
  unit_text = cp.unit_text;
  unit_layout = cp.unit_layout;
  auto_scale = cp.auto_scale;
  spacing = cp.spacing;
  scale_min = cp.scale_min;
  scale_max = cp.scale_max;
  ordered_uvg_list = cp.ordered_uvg_list;
  if(cp.lay_forms != NULL) {
    MakeLayXforms();
    lay_forms->Copy(*(cp.lay_forms));
  }
  if(cp.label_forms != NULL) {
    MakeLayLabelXforms();
    label_forms->Copy(*(cp.label_forms));
  }
  if(cp.frozen_net_form != NULL) {
    MakeNetXform();
    frozen_net_form->Copy(*(cp.frozen_net_form));
  }
  if(taMisc::iv_active && (editor != NULL))
    editor->SelectActButton(Tool::select); // select act button!
  InitDisplay();
}

int NetView::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  return PDPView::Dump_Save_Value(strm, par, indent);
}

int NetView::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = PDPView::Dump_Load_Value(strm, par);
  if(rval) {
    NotifyAllUpdaters();
  }
  return rval;
}

void NetView::UpdateAfterEdit(){
  PDPView::UpdateAfterEdit();
  labels.SetBaseType(&TA_NetViewLabel); // make sure to use this type..
  idraw_graphics.SetBaseType(&TA_NetViewGraphic);
  if(!taMisc::iv_active) return;
  if(editor == NULL) return; //loading
  if(editor->netg->skew != skew)
    editor->netg->skew = skew;
  if((colorspec != NULL) && (editor->scale->spec != colorspec)) {
    taBase::SetPointer((TAPtr*)&(editor->scale->spec), colorspec);
    editor->scale->MapColors();
    editor->netg->background(editor->scale->Get_Background());
  }
  if(editor->display_toggle != display_toggle) editor->ToggleDisplay(false);
  if(editor->auto_scale != auto_scale) editor->ToggleAutoScale(false);
  if(editor->unit_text != unit_text) editor->UpdateUTextMenu(unit_text);
  if(editor->shape != shape) editor->UpdateDispMdMenu(shape);

  InitDisplay();
}  

void NetView::GetBodyRep() {
  if(!taMisc::iv_active) return;

  editor = new NetEditor((Network*)mgr, this, window);
  editor->owner = this;
  editor->shape = shape;
  editor->Init();
  body = editor->GetLook();
  ivResource::ref(body);
}

void NetView::CloseWindow() {
  if(!taMisc::iv_active || (window == NULL)) return;
  ivResource::unref(body);	body = NULL;
  PDPView::CloseWindow();
}

void NetView::InitDisplay() {
  if(mgr) ((Network*)mgr)->UpdateMax();
  if(!taMisc::iv_active || (editor == NULL)) return;
  editor->InitDisplay();
}

void NetView::UpdateDisplay(TAPtr) {
  if(!IsMapped() || !display_toggle || (editor == NULL)) return;
  editor->update_from_state();
  if(anim.capture) CaptureAnimImg();
}

ivGlyph* NetView::GetPrintData() {
  if(editor == NULL)
    return win_box;
  return editor->print_patch;
}

void NetView::GetMenu() {
  PDPView::GetMenu();
  if(editor != NULL) {
    editor->UpdateMStatMenu();
  }
//  InitDisplay();
}

void NetView::OpenNewWindow() { 
  if(!taMisc::iv_active) return;
  PDPView::OpenNewWindow() ;
}

void NetView::ToggleDisplay() {
  if(editor) {
    editor->ToggleDisplay();
    display_toggle = editor->display_toggle;
  }
}

void NetView::ToggleAutoScale() {
  if(editor) {
    editor->ToggleDisplay();
    auto_scale = editor->auto_scale;
  }
}

void NetView::SetDefaultSkew() {
  Network* net = (Network*)mgr;
  if (net==NULL) { skew = 0; return; }
  switch(net->lay_layout){
  case Network::THREE_D: skew = DEFAULT_3D_NETVIEW_SKEW; break;
  case Network::TWO_D: skew = DEFAULT_2D_NETVIEW_SKEW;
  }
  if(editor != NULL) {
    editor->netg->skew = skew;
    editor->netg->SetMax(net->max_size);
  }  
}

///////////////////////////////
// net xform

void NetView::MakeNetXform() {
  if(frozen_net_form == NULL) {
    frozen_net_form = new Xform();
    taBase::Own(frozen_net_form,this);
  }
}

///////////////////////////////
// layer xforms

void NetView::MakeLayXforms() {
  if(lay_forms == NULL) {
    lay_forms = new Xform_List();
    taBase::Own(lay_forms,this);
  }
}

Xform* NetView::GetLayerXform(Layer* lay) {
  if(lay_forms == NULL) return NULL;
  Network* net = (Network*)mgr;
  int index = net->layers.FindLeaf(lay);
  Xform* xf = lay_forms->FindName(String(index));
  return xf;
}

void NetView::SetLayerXform(Layer* lay,float b00,float b01, float b10, float b11,
			    float b20, float b21)
{
  MakeLayXforms();
  Xform* xf = GetLayerXform(lay);
  if(xf == NULL) {
    xf = (Xform*) lay_forms->New(1);
    Network* net = (Network*)mgr;
    int index = net->layers.FindLeaf(lay);
    xf->name = String(index);
  }
  xf->Set(b00,b01,b10,b11,b20,b21);
}

///////////////////////////////
// layer label xforms

void NetView::MakeLayLabelXforms() {
  if(label_forms == NULL) {
    label_forms = new Xform_List();
    taBase::Own(label_forms,this);
  }
}

Xform* NetView::GetLayerLabelXform(Layer* lay) {
  if(label_forms == NULL) return NULL;
  Network* net = (Network*)mgr;
  int index = net->layers.FindLeaf(lay);
  Xform* xf = label_forms->FindName(String(index));
  return xf;
}

void NetView::SetLayerLabelXform(Layer* lay,float b00,float b01,
				 float b10, float b11,
				 float b20, float b21)
{
  MakeLayLabelXforms();
  Xform* xf = GetLayerLabelXform(lay);
  if(xf == NULL) {
    xf = (Xform*) label_forms->New(1);
    Network* net = (Network*)mgr;
    int index = net->layers.FindLeaf(lay);
    xf->name = String(index);
  }
  xf->Set(b00,b01,b10,b11,b20,b21);
}

/////////////////////////////////////////
// 	Fix display of stuff

void NetView::FixUnit(Unit* u, Layer* lay){
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  Network_G* ng  = (Network_G *) editor->netg;
  ng->FixUnit(u, lay);
}

void NetView::FixUnitGroup(Unit_Group* ug, Layer* lay){
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  Network_G* ng  = (Network_G *) editor->netg;
  ng->FixUnitGroup(ug, lay);
}

void NetView::FixLayer(Layer* lay) {
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  Network_G* ng  = (Network_G *) editor->netg;
  if(lay==NULL) {
    Network* net = (Network*)mgr;
    taLeafItr j;
    FOR_ITR_EL(Layer, lay, net->layers., j){
      ng->FixLayer(lay);
    }
  }
  else {
    ng->FixLayer(lay);
  }
}

void NetView::FixProjection(Projection* p,int index){
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  Projection* pjn = p;
  Network* net = (Network*)mgr;
  Layer* lay;
  int idx=0;
  if(p==NULL){
    taLeafItr i;
    FOR_ITR_EL(Layer, lay, net->layers., i){
      taLeafItr j;
      FOR_ITR_EL(Projection, pjn, lay->projections., j){
	FixProjection(pjn,idx);
	idx++;
      }
    }
    return;
  }

  Network_G* ng  = (Network_G *) editor->netg;
  if(ng == NULL) return;
  ng->FixProjection(pjn,index);
}

void NetView::UpdateButtons() {
  if(editor == NULL) return;
  editor->FixEditorButtons();
}

////////////////////////////////
// 	labels

void NetViewSelectEffect(void* obj){
  ((Network_G*) obj)->owner->FixEditorButtons();
}

ivColor*  NetViewGetLabelColor(void* obj){
  return ((Network_G *) obj)->GetLabelColor();
}

void NetView::NewLabel(String text) {
  NetViewLabel* vl = (NetViewLabel*) labels.New(1, &TA_NetViewLabel);
  vl->select_effect = NetViewSelectEffect;
  vl->get_color = NetViewGetLabelColor;
  vl->name = text;
  vl->MakeText();
  InitDisplay();
}

void NetView::RemoveLabel(ViewLabel* vl) {
  labels.Remove(vl);
}


///////////////////////////////////////////////
// functions to manipulate what is selected/picked

void NetView::ScriptSelectCurrent() {
  if(taivMisc::record_script == NULL) return;
  *taivMisc::record_script << "{ NetView* ths = " << GetPath() << ";\n"
			   << "  ths->Select(NULL);" << endl;
  int i;
  for(i=0; i<editor->netg->selectgroup.size; i++) {
    *taivMisc::record_script << "  ths->Select(" << editor->netg->selectgroup[i]->GetPath()
			     << ");\n";
  }
  *taivMisc::record_script << "}" << endl;
}

void NetView::Select(TAPtr obj, bool add){
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  if((obj == NULL) || !add) {
    editor->netg->selectgroup.RemoveAll();
    editor->netg->UpdateSelect();
    if(obj == NULL)
      return;
  }
  if(obj->InheritsFrom(&TA_Unit)  ||
     obj->InheritsFrom(&TA_Unit_Group) ||
     obj->InheritsFrom(&TA_Layer)  ||
     obj->InheritsFrom(&TA_Projection)  ||
     obj->InheritsFrom(&TA_ViewLabel))
  {
//     editor->netg->selectgroup.LinkUnique(obj);
//     editor->netg->UpdateSelect();
    editor->netg->SelectObject(obj); // this should be much faster!
  }
}

void NetView::Pick(TAPtr obj, bool add) {
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  if((obj == NULL) || !add) {
    editor->netg->pickgroup.RemoveAll();
    editor->netg->UpdatePick();
    if(obj == NULL)
      return;
  }
  if(obj->InheritsFrom(&TA_Unit)) {
    editor->netg->pickgroup.LinkUnique(obj);
    editor->netg->UpdatePick();
  }
}

taBase_List* NetView::GetSelectGroup() {
  return &editor->netg->selectgroup;
}

taBase_List* NetView::GetPickGroup() {
  return &editor->netg->pickgroup;
}

void NetView::UpdateSelect() {
  editor->netg->UpdateSelect();
}

void NetView::UpdatePick() {
  editor->netg->UpdatePick();
}

void NetView::SelectVar(const char* var_name, bool add) {
  if((editor == NULL) || (editor->netg == NULL)) return;
  int idx;
  MemberDef* md =
    (MemberDef *) editor->netg->membs.FindName(var_name, idx);
  if(!add)
    ordered_uvg_list.Reset();
  if(md == NULL) return;
  ordered_uvg_list.Add(idx);
  InitDisplay();
}


///////////////////////////////////////////////
// user interface actions to control display

void NetView::SetColorSpec(ColorScaleSpec* colors) {
  taBase::SetPointer((TAPtr*)&colorspec, colors);
  UpdateAfterEdit();
}

void NetView::AutoPositionPrjnPoints() {
  if(editor == NULL) return;
  // clear all proj_points transforms
  Network* net = (Network*)mgr;
  taLeafItr i;
  Layer* lay;
  FOR_ITR_EL(Layer, lay, net->layers., i) {
    taLeafItr j;
    Projection* pjn;
    FOR_ITR_EL(Projection, pjn, lay->projections., j) {
      taBase::DelPointer((TAPtr*)&pjn->proj_points);
      pjn->proj_points = NULL;
    }
  }
  UpdateAfterEdit();
}

void NetView::AutoPositionLayerNames() {
  if(label_forms != NULL)
    label_forms->Reset();
  UpdateAfterEdit();
}

void NetView::ZoomLayer(Layer* lay, float mag_factor_x, float mag_factor_y) {
  if(!taMisc::iv_active) return;
  Network_G* ng  = (Network_G*)editor->netg;
  ivGlyphIndex count = ng->count_();
  ivGlyphIndex i;
  Layer_G* lg = NULL;
  for(i=0;i<count;i++) {
    Graphic* gr = ng->component_(i);
    if(gr->InheritsFrom(&TA_Layer_G) && (((Layer_G*)gr)->lay == lay)) {
      lg = (Layer_G*)gr;
      break;
    }
  }
  if(lg == NULL) return;
  float a00 = 0.0f; float a01 = 0.0f; float a10 = 0.0f;
  float a11 = 0.0f; float a20 = 0.0f; float a21 = 0.0f;
  ivTransformer* tx = lg->transformer();
  if(tx != NULL) {
    tx->matrix(a00,a01,a10,a11,a20,a21);
  }
  SetLayerXform(lay, mag_factor_x, a01, a10, mag_factor_y, a20, a21);
  UpdateAfterEdit();
}

void NetView::ResetLayerZoom() {
  if(lay_forms != NULL)
    lay_forms->Reset();
  UpdateAfterEdit();
}

void NetView::FreezeNetZoom() {
  if((editor == NULL) || (editor->viewer == NULL) || (editor->netg == NULL)) return;
  MakeNetXform();
  frozen_net_form->Set(editor->viewer->root()->transformer());
  UpdateAfterEdit();
}

void NetView::AutoNetZoom() {
  taBase::DelPointer((TAPtr*)&frozen_net_form);
  frozen_net_form = NULL;
  UpdateAfterEdit();
}

void NetView::SetLayerFontSize(int point_size) {
  layer_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void NetView::SetUnitFontSize(int point_size) {
  unit_font.SetFontSize(point_size);
  UpdateAfterEdit();
}

void NetView::RemoveGraphics() {
  if(has_graphic) {
    idraw_graphics.RemoveAll();
    has_graphic = false;
  }
}

void NetView::LoadGraphic(const char* nm) {
  String new_name = nm;
  if(editor == NULL) return;
  ivCoord c= editor->netg->count_();
  if((nm == NULL) || (*nm == '\0')) { // get filename
    taivGetFile f;
    f.select_only = true;
    f.Open();
    if( f.file_selected == true)  {
      new_name = f.fname;
    }
  }
  NetViewGraphic* netvg = (NetViewGraphic*)(idraw_graphics.New(1));
  netvg->name = new_name;
  netvg->UpdateAfterEdit();
  if (editor->netg->count_() != c) has_graphic = true;
  UpdateDisplay();
}


///////////////////////////////////////////////
// user interface actions to operate on selected objects

void NetView::SetUnitSpec(UnitSpec* unit_spec) {
  if((editor == NULL) || (editor->netg == NULL) || (unit_spec == NULL))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some units or layers to apply UnitSpec to");
    return;
  }
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    String usp = unit_spec->GetPath();
    taBase* un;
    for(int i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Unit)) {
	if(unit_spec->CheckObjectType(un)) {
	  taivMisc::RecordScript(un->GetPath() + "->spec.SetSpec(" + usp + ");\n");
	}
	else
	  break;
      }
      else if(un->InheritsFrom(TA_Layer)) {
	taivMisc::RecordScript(un->GetPath() + "->SetUnitSpec(" + usp + ");\n");
      }
      else if(un->InheritsFrom(TA_Unit_Group)) {
	taivMisc::RecordScript(un->GetPath() + "->SetUnitSpec(" + usp + ");\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  int i;
  taBase* un;
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Unit)) {
      if(unit_spec->CheckObjectType(un))
	((Unit*)un)->spec.SetSpec(unit_spec);
      else
	break;
    }
    else if(un->InheritsFrom(TA_Layer)) {
      if(!((Layer*)un)->SetUnitSpec(unit_spec))
	break;
    }
    else if(un->InheritsFrom(TA_Unit_Group)) {
      if(!((Unit_Group*)un)->SetUnitSpec(unit_spec))
	break;
    }
  }
}

void NetView::SetConSpec(ConSpec* con_spec) {
  if((editor == NULL) || (editor->netg == NULL) || (con_spec == NULL))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some units or projections to apply ConSpec to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    String usp = con_spec->GetPath();
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Unit) || un->InheritsFrom(TA_Projection) ||
	 un->InheritsFrom(TA_Layer) || un->InheritsFrom(TA_Unit_Group)) {
	taivMisc::RecordScript(un->GetPath() + "->SetConSpec(" + usp + ");\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Unit)) {
      if(!((Unit*)un)->SetConSpec(con_spec))
	break;
    }
    else if(un->InheritsFrom(TA_Projection)) {
      if(!((Projection*)un)->SetConSpec(con_spec))
	break;
    }
    else if(un->InheritsFrom(TA_Layer)) {
      if(!((Layer*)un)->SetConSpec(con_spec))
	break;
    }
    else if(un->InheritsFrom(TA_Unit_Group)) {
      if(!((Unit_Group*)un)->SetConSpec(con_spec))
	break;
    }
  }
}

void NetView::SetPrjnSpec(ProjectionSpec* prjn_spec) {
  if((editor == NULL) || (editor->netg == NULL) || (prjn_spec == NULL))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some projections to apply spec to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    String usp = prjn_spec->GetPath();
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Projection)) {
	if(prjn_spec->CheckObjectType(un))
	  taivMisc::RecordScript(un->GetPath() + "->spec.SetSpec(" + usp + ");\n");
	else
	  break;
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Projection)) {
      if(prjn_spec->CheckObjectType(un))
	((Projection*)un)->spec.SetSpec(prjn_spec);
      else
	break;
    }
  }
}

void NetView::SetPrjnConType(TypeDef* con_type) {
  if((editor == NULL) || (editor->netg == NULL) || (con_type == NULL)
     || !con_type->InheritsFrom(TA_Connection))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some projections to apply con_type to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Projection)) {
	taivMisc::RecordScript(un->GetPath() + "->SetConType(" + con_type->name + ");\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Projection)) {
      Projection* prjn = (Projection*)un;
      prjn->SetConType(con_type);
    }
  }
}

void NetView::SetPrjnConGpType(TypeDef* con_gp_type) {
  if((editor == NULL) || (editor->netg == NULL) || (con_gp_type == NULL)
     || !con_gp_type->InheritsFrom(TA_Con_Group))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some projections to apply con_gp_type to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Projection)) {
	taivMisc::RecordScript(un->GetPath() + "->SetConGpType(" + con_gp_type->name + ");\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Projection)) {
      Projection* prjn = (Projection*)un;
      prjn->SetConGpType(con_gp_type);
    }
  }
}

void NetView::SetLayerSpec(LayerSpec* layer_spec) {
  if((editor == NULL) || (editor->netg == NULL) || (layer_spec == NULL))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some layers to apply spec to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    String usp = layer_spec->GetPath();
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Layer)) {
	taivMisc::RecordScript(un->GetPath() + "->SetLayerSpec(" + usp + ");\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Layer)) {
      if(!((Layer*)un)->SetLayerSpec(layer_spec))
	break;
    }
  }
}

void NetView::SetLayerUnitType(TypeDef* unit_type) {
  if((editor == NULL) || (editor->netg == NULL) || (unit_type == NULL)
     || !unit_type->InheritsFrom(TA_Unit))
    return;
  if(editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some layers to apply unit_type to");
    return;
  }
  int i;
  taBase* un;
#ifdef DMEM_COMPILE
  if(taMisc::dmem_nprocs > 1) {
    for(i=0; i< editor->netg->selectgroup.size; i++) {
      un = editor->netg->selectgroup.FastEl(i);
      if(un->InheritsFrom(TA_Layer)) {
	taivMisc::RecordScript(un->GetPath() + "->SetUnitType(" + unit_type->name + ");\n");
      }
      else if(un->InheritsFrom(TA_Unit_Group)) {
	taivMisc::RecordScript(un->GetPath() + "->el_typ=" + unit_type->name + ";\n");
      }
    }
    return;
  }
#endif
  ScriptSelectCurrent();
  for(i=0; i< editor->netg->selectgroup.size; i++) {
    un = editor->netg->selectgroup.FastEl(i);
    if(un->InheritsFrom(TA_Layer)) {
      ((Layer*)un)->SetUnitType(unit_type);
    }
    else if(un->InheritsFrom(TA_Unit_Group))
      ((Unit_Group*)un)->el_typ = unit_type;
  }
}

void NetView::ShowUnitSpec(UnitSpec* unit_spec){
  if((editor == NULL) || (editor->netg == NULL)) return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    if(l->unit_spec.GetSpec() == unit_spec) {
      editor->netg->selectgroup.LinkUnique(l);
    }
    else {
      Unit* u;
      taLeafItr j;
      FOR_ITR_EL(Unit, u, l->units., j){
	if(u->spec.GetSpec() == unit_spec){
	  editor->netg->selectgroup.LinkUnique(u);
	}
      }
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowConSpec(ConSpec* con_spec){
  if((editor == NULL) || (editor->netg == NULL)) return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Projection* p;
    taLeafItr j;
    FOR_ITR_EL(Projection, p, l->projections., j){
      if(p->con_spec.GetSpec() == con_spec){
	editor->netg->selectgroup.LinkUnique(p);
      }
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowPrjnSpec(ProjectionSpec* prjn_spec) {
  if((editor == NULL) || (editor->netg == NULL)) return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Projection* p;
    taLeafItr j;
    FOR_ITR_EL(Projection, p, l->projections., j){
      if(p->spec.spec == prjn_spec){
	editor->netg->selectgroup.LinkUnique(p);
      }
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowPrjnConType(TypeDef* con_type) {
  if((editor == NULL) || (editor->netg == NULL) || (con_type == NULL)
     || !con_type->InheritsFrom(TA_Connection))
    return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Projection* p;
    taLeafItr j;
    FOR_ITR_EL(Projection, p, l->projections., j) {
      if(p->con_type == con_type) {
	editor->netg->selectgroup.LinkUnique(p);
      }
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowPrjnConGpType(TypeDef* con_gp_type) {
  if((editor == NULL) || (editor->netg == NULL) || (con_gp_type == NULL)
     || !con_gp_type->InheritsFrom(TA_Con_Group))
    return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Projection* p;
    taLeafItr j;
    FOR_ITR_EL(Projection, p, l->projections., j) {
      if(p->con_gp_type  == con_gp_type) {
	editor->netg->selectgroup.LinkUnique(p);
      }
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowLayerSpec(LayerSpec* layer_spec){
  if((editor == NULL) || (editor->netg == NULL)) return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    if(l->GetLayerSpec() == layer_spec){
      editor->netg->selectgroup.LinkUnique(l);
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::ShowLayerUnitType(TypeDef* unit_type) {
  if((editor == NULL) || (editor->netg == NULL) || (unit_type == NULL)
     || !unit_type->InheritsFrom(TA_Unit))
    return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    if(l->units.el_typ == unit_type)
      editor->netg->selectgroup.LinkUnique(l);
    int ug;
    for(ug=0;ug<l->units.gp.size; ug++) {
      Unit_Group* ugg = (Unit_Group*)l->units.gp.FastEl(ug);
      if(ugg->el_typ == unit_type)
	editor->netg->selectgroup.LinkUnique(ugg);
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::SelectAllPrjns() {
  if((editor == NULL) || (editor->netg == NULL))
    return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Projection* p;
    taLeafItr j;
    FOR_ITR_EL(Projection, p, l->projections., j) {
      editor->netg->selectgroup.LinkUnique(p);
    }
  }
  editor->netg->UpdateSelect();
}

void NetView::SelectAllLayers() {
  if((editor == NULL) || (editor->netg == NULL))
    return;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    editor->netg->selectgroup.LinkUnique(l);
  }
  editor->netg->UpdateSelect();
}

void NetView::SelectUnits(const char* variable, CountParam::Relation rel, float cmp_val) {
  if((editor == NULL) || (editor->netg == NULL))
    return;
  String var = variable;
  String varname = var;
  MemberDef* md = NULL;
  CountParam cond;
  cond.rel = rel; cond.val = cmp_val;
  editor->netg->selectgroup.RemoveAll();
  Layer* l;
  taLeafItr i;
  Network* net = (Network*)mgr;
  FOR_ITR_EL(Layer, l, net->layers., i) {
    Unit* u;
    taLeafItr ui;
    FOR_ITR_EL(Unit, u, l->units., ui) {
      TAPtr ths = u;
      if(var.contains('.')) {
	varname = var.before('.');
	if((varname=="r") || (varname=="s")) {
	  Unit* pick_u = (Unit*)GetPickGroup()->SafeEl(0);
	  if((pick_u == NULL) || !pick_u->InheritsFrom(TA_Unit)) return;
	  Con_Group* cg;
	  if(varname == "r")
	    cg = &(pick_u->recv);
	  else
	    cg = &(pick_u->send);
	  varname = var.after('.');
	  Con_Group* tcg;
	  int g;
	  FOR_ITR_GP(Con_Group, tcg, cg->, g) {
	    md = tcg->prjn->con_type->members.FindName((const char*)varname);
	    if(md == NULL) continue;
	    Connection* con = tcg->FindConFrom(u);
	    if(con == NULL) continue;
	    float val = *((float*)md->GetOff((void*)con));
	    if(cond.Evaluate(val))
	      editor->netg->selectgroup.LinkUnique(u);
	  }
	  continue;		// done..
	}
	void* temp;
	md = u->FindMembeR(varname,temp);
	if((md == NULL) || (temp == NULL)) continue;
	if(md->type->ptr == 1) {
	  ths = *((TAPtr*)temp);
	  if(ths == NULL) continue;
	}
	else
	  ths = (TAPtr) temp;
	varname = var.after('.');
      }
      md = ths->FindMember((const char*)varname);
      if((md == NULL) || !md->type->InheritsFrom(TA_float))
	continue;
      float val = *((float*)md->GetOff((void*)ths));
      if(cond.Evaluate(val))
	editor->netg->selectgroup.LinkUnique(u);
    }
  }
  editor->netg->UpdateSelect();
}

// new monitor is in pdpshell.cc

////////////////////////////////////////////
//  	Wizard functions
////////////////////////////////////////////

bool Network::nw_itm_def_arg = false;

Layer* Network::FindMakeLayer(const char* nm, TypeDef* td, bool& nw_itm, const char* alt_nm) {
  nw_itm = false;
  Layer* lay = (Layer*)layers.FindName(nm);
  if((lay == NULL) && (alt_nm != NULL)) {
    lay = (Layer*)layers.FindName(alt_nm);
    if(lay != NULL) lay->name = nm;
  }
  if(lay == NULL) {
    lay = (Layer*)layers.NewEl(1, td);
    lay->name = nm;
    nw_itm = true;
  }
  if((td != NULL) && !lay->InheritsFrom(td)) {
    layers.Remove(lay);
    lay = (Layer*)layers.NewEl(1, td);
    lay->name = nm;
    nw_itm = true;
  }
  return lay;
}

Projection* Network::FindMakePrjn(Layer* recv, Layer* send, ProjectionSpec* ps, ConSpec* cs, bool& nw_itm) {
  Projection* use_prj = NULL;
  int i;
  for(i=0;i<recv->projections.size;i++) {
    Projection* prj = (Projection*)recv->projections[i];
    if(prj->from == send) {
      if((ps == NULL) && (cs == NULL)) {
	nw_itm = false;
	return prj;
      }
      if((ps != NULL) && (prj->spec.spec != ps)) {
	use_prj = prj;
	break;
      }
      if((cs != NULL) && (prj->con_spec.spec != cs)) {
	use_prj = prj;
	break;
      }
      nw_itm = false;
      return prj;
    }
  }
  nw_itm = true;
  if(use_prj == NULL)
    use_prj = (Projection*)recv->projections.NewEl(1);
  use_prj->SetCustomFrom(send);
  if(ps != NULL) {
    use_prj->spec.SetSpec(ps);
  }
  if(cs != NULL) {
    use_prj->SetConType(cs->min_con_type);
    use_prj->con_spec.SetSpec(cs);
  }
  return use_prj;
}

Projection* Network::FindMakePrjnAdd(Layer* recv, Layer* send, ProjectionSpec* ps, ConSpec* cs, bool& nw_itm) {
  int i;
  for(i=0;i<recv->projections.size;i++) {
    Projection* prj = (Projection*)recv->projections[i];
    if((prj->from == send)
       && ((ps == NULL) || (prj->spec.spec == ps) ||
	   (prj->spec.spec->InheritsFrom(TA_FullPrjnSpec) &&
	    ps->InheritsFrom(TA_FullPrjnSpec)))
       && ((cs == NULL) || (prj->con_spec.spec == cs))) {
      nw_itm = false;
      return prj;
    }
  }
  nw_itm = true;
  Projection* prj = (Projection*)recv->projections.NewEl(1);
  prj->SetCustomFrom(send);
  if(ps != NULL) {
    prj->spec.SetSpec(ps);
  }
  if(cs != NULL) {
    prj->SetConType(cs->min_con_type);
    prj->con_spec.SetSpec(cs);
  }
  return prj;
}

Projection* Network::FindMakeSelfPrjn(Layer* recv, ProjectionSpec* ps, ConSpec* cs, bool& nw_itm) {
  Projection* use_prj = NULL;
  int i;
  for(i=0;i<recv->projections.size;i++) {
    Projection* prj = (Projection*)recv->projections[i];
    if(prj->from == recv) {
      if((ps == NULL) && (cs == NULL)) {
	nw_itm = false;
	return prj;
      }
      if((ps != NULL) && (prj->spec.spec != ps)) {
	use_prj = prj;
	break;
      }
      if((cs != NULL) && (prj->con_spec.spec != cs)) {
	use_prj = prj;
	break;
      }
      nw_itm = false;
      return prj;
    }
  }
  nw_itm = true;
  if(use_prj == NULL)
    use_prj = (Projection*)recv->projections.NewEl(1);
  use_prj->from_type = Projection::SELF;
  taBase::SetPointer((TAPtr*)&(use_prj->from), recv);
  if(ps != NULL)
    use_prj->spec.SetSpec(ps);
  if(cs != NULL)
    use_prj->con_spec.SetSpec(cs);
  return use_prj;
}

Projection* Network::FindMakeSelfPrjnAdd(Layer* recv, ProjectionSpec* ps, ConSpec* cs, bool& nw_itm) {
  int i;
  for(i=0;i<recv->projections.size;i++) {
    Projection* prj = (Projection*)recv->projections[i];
    if((prj->from == recv)
       && ((ps == NULL) || (prj->spec.spec == ps))
       && ((cs == NULL) || (prj->con_spec.spec == cs))) {
      nw_itm = false;
      return prj;
    }
  }
  nw_itm = true;
  Projection* prj = (Projection*)recv->projections.NewEl(1);
  prj->from_type = Projection::SELF;
  taBase::SetPointer((TAPtr*)&(prj->from), recv);
  if(ps != NULL)
    prj->spec.SetSpec(ps);
  if(cs != NULL)
    prj->con_spec.SetSpec(cs);
  return prj;
}

bool Network::RemovePrjn(Layer* recv, Layer* send, ProjectionSpec* ps, ConSpec* cs) {
  int i;
  for(i=recv->projections.size-1;i>=0;i--) {
    Projection* prj = (Projection*)recv->projections[i];
    if((prj->from == send)
       && ((ps == NULL) || (prj->spec.spec == ps) ||
	   (prj->spec.spec->InheritsFrom(TA_FullPrjnSpec) &&
	    ps->InheritsFrom(TA_FullPrjnSpec)))
       && ((cs == NULL) || (prj->con_spec.spec == cs))) {
      recv->projections.Remove(prj);
      return true;
    }
  }
  return false;
}

