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

// netstru_extra.cc


#include <pdp/netstru_extra.h>
#include <pdp/net_iv.h>
#include <css/machine.h>


/////////////////////////////
//	    Full	   //
/////////////////////////////

void FullPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  int no = prjn->from->units.leaves;
  if(!self_con && (prjn->from == prjn->layer))
    no--;

  // pre-allocate connections!
  Unit* ru, *su;
  taLeafItr ru_itr, su_itr;
  FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr)
    ru->ConnectAlloc(no, prjn);
  
  FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
    FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
      if(self_con || (ru != su))
	ru->ConnectFrom(su, prjn);
    }  
  }
}

int FullPrjnSpec::ProbAddCons(Projection* prjn, float p_add_con, float init_wt) {
  if(prjn->from == NULL)	return 0;

  int rval = 0;

  int no = prjn->from->units.leaves;
  if(!self_con && (prjn->from == prjn->layer))
    no--;

  int n_new_cons = (int)(p_add_con * (float)no);
  if(n_new_cons <= 0) return 0;
  int_Array new_idxs;
  new_idxs.EnforceSize(no);
  new_idxs.FillSeq();
  Unit* ru;
  taLeafItr ru_itr;
  FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
    new_idxs.Permute();
    for(int i=0;i<n_new_cons;i++) {
      Unit* su = (Unit*)prjn->from->units.Leaf(new_idxs[i]);
      Connection* cn = ru->ConnectFromCk(su, prjn); // check means that it won't add any new connections if already there!
      if(cn != NULL) {
	cn->wt = init_wt;
	rval++;
      }
    }
  }
  return rval;
}

/////////////////////////////
//	  OneToOne	   //
/////////////////////////////

void OneToOnePrjnSpec::Initialize() {
  n_conns = -1;
  recv_start = 0;
  send_start = 0;
  SetUnique("self_con", true);
  self_con = true;		// doesn't make sense to not do self con!
}

void OneToOnePrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  
  int i;
  int max_n = n_conns;
  if(n_conns < 0)
    max_n = prjn->layer->units.leaves - recv_start;
  max_n = MIN(prjn->layer->units.leaves - recv_start, max_n);
  max_n = MIN(prjn->from->units.leaves - send_start, max_n);
  for(i=0; i<max_n; i++) {
    Unit* ru = (Unit*)prjn->layer->units.Leaf(recv_start + i);
    Unit* su = (Unit*)prjn->from->units.Leaf(send_start + i);
    if(self_con || (ru != su))
      ru->ConnectFrom(su, prjn);
  }
}

/////////////////////////////
//	  Tessel	   //
/////////////////////////////

void TessEl::Initialize() {
  wt_val = 1.0f;
}

void TessEl::InitLinks() {
  taOBase::InitLinks();
  taBase::Own(send_off, this);
}

void TessEl::Copy_(const TessEl& cp) {
  send_off = cp.send_off;
  wt_val = cp.wt_val;
}


void TesselPrjnSpec::Initialize() {
  recv_n = -1;
  recv_skip = 1;
  recv_group = 1;
  wrap = true;
  link_type = NO_LINK;
  send_scale = 1.0f;
  send_offs.SetBaseType(&TA_TessEl);
}

void TesselPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(recv_off, this);
  taBase::Own(recv_n, this);
  taBase::Own(recv_skip, this);
  taBase::Own(recv_group, this);
  taBase::Own(link_src, this);
  taBase::Own(send_scale, this);
  taBase::Own(send_border, this);
  taBase::Own(send_offs, this);
}

void TesselPrjnSpec::Copy_(const TesselPrjnSpec& cp) {
  recv_off = cp.recv_off;
  recv_n = cp.recv_n;
  recv_skip = cp.recv_skip;
  recv_group = cp.recv_group;
  wrap = cp.wrap;
  link_type = cp.link_type;
  link_src = cp.link_src;
  send_scale = cp.send_scale;
  send_border = cp.send_border;
  send_offs = cp.send_offs;
}

void TesselPrjnSpec::UpdateAfterEdit() {
  ProjectionSpec::UpdateAfterEdit();
  recv_skip.SetGtEq(1);
  recv_group.SetGtEq(1);
}

void TesselPrjnSpec::MakeEllipse(int half_width, int half_height, int ctr_x, int ctr_y) {
  send_offs.Reset();
  int strt_x = ctr_x - half_width;
  int end_x = ctr_x + half_width;
  int strt_y = ctr_y - half_height;
  int end_y = ctr_y + half_height;
  if(half_width == half_height) { // circle
    int y;
    for(y = strt_y; y <= end_y; y++) {
      int x;
      for(x = strt_x; x <= end_x; x++) {
	int dist = ((x - ctr_x) * (x - ctr_x)) + ((y - ctr_y) * (y - ctr_y));
	if(dist > (half_width * half_width))
	  continue;		// outside the circle
	TessEl* te = (TessEl*)send_offs.New(1, &TA_TessEl);
	te->send_off.x = x;
	te->send_off.y = y;
      }
    }
  }
  else {			// ellipse
    float f1_x, f1_y;		// foci
    float f2_x, f2_y;
    float two_a;			// two times largest axis
    
    if(half_width > half_height) {
      two_a = (float)half_width * 2;
      float c = sqrtf((float)(half_width * half_width) - (float)(half_height * half_height));
      f1_x = (float)ctr_x - c;
      f1_y = (float)ctr_y;
      f2_x = (float)ctr_x + c;
      f2_y = (float)ctr_y;
    }
    else {
      two_a = (float)half_height * 2;
      float c = sqrtf((float)(half_height * half_height) - (float)(half_width * half_width));
      f1_x = (float)ctr_x;
      f1_y = (float)ctr_y - c;
      f2_x = (float)ctr_x;
      f2_y = (float)ctr_y + c;
    }

    int y;
    for(y = strt_y; y <= end_y; y++) {
      int x;
      for(x = strt_x; x <= end_x; x++) {
	float dist = sqrtf((((float)x - f1_x) * ((float)x - f1_x)) + (((float)y - f1_y) * ((float)y - f1_y))) +
	  sqrtf((((float)x - f2_x) * ((float)x - f2_x)) + (((float)y - f2_y) * ((float)y - f2_y)));
	if(dist > two_a)
	  continue;
	TessEl* te = (TessEl*)send_offs.New(1, &TA_TessEl);
	te->send_off.x = x;
	te->send_off.y = y;
      }
    }
  }
}

void TesselPrjnSpec::MakeRectangle(int width, int height, int left, int bottom) {
  send_offs.Reset();
  int y;
  for(y = bottom; y < bottom + height; y++) {
    int x;
    for(x = left; x < left + width; x++) {
      TessEl* te = (TessEl*)send_offs.New(1, &TA_TessEl);
      te->send_off.x = x;
      te->send_off.y = y;
    }
  }
}

void TesselPrjnSpec::MakeFromNetView(NetView* view) {
  if((view == NULL) || (view->editor == NULL) || (view->editor->netg == NULL))
    return;
  if(view->editor->netg->selectgroup.size <= 0) {
    taMisc::Error("Must select some units to get connection pattern from");
    return;
  }
  send_offs.Reset();
  int i;
  taBase* itm;
  Unit* center = NULL;
  for(i=0; i< view->editor->netg->selectgroup.size; i++) {
    itm = view->editor->netg->selectgroup.FastEl(i);
    if(!itm->InheritsFrom(TA_Unit))      continue;
    Unit* un = (Unit*) itm;
    if(center == NULL) {
      center = un;
      continue;
    }
    TessEl* te = (TessEl*)send_offs.New(1, &TA_TessEl);
    te->send_off = un->pos - center->pos;
  }
}

void TesselPrjnSpec::WeightsFromDist(float scale) {
  TwoDCoord zero;
  int i;
  TessEl* te;
  for(i = 0; i< send_offs.size; i++) {
    te = (TessEl*)send_offs.FastEl(i);
    float dist = te->send_off.Dist(zero);
    te->wt_val = scale * (1.0f / dist);
  }
}

void TesselPrjnSpec::WeightsFromGausDist(float scale, float sigma) {
  TwoDCoord zero;
  int i;
  TessEl* te;
  for(i = 0; i< send_offs.size; i++) {
    te = (TessEl*)send_offs.FastEl(i);
    float dist = te->send_off.Dist(zero);
    te->wt_val = scale * exp(-0.5 * dist / (sigma * sigma));
  }
}

// todo: this assumes that things are in order.. (can't really check otherwise)
// which breaks for clipped patterns
void TesselPrjnSpec::C_InitWtState(Projection*, Con_Group* cg, Unit*) {
  int mxi = MIN(cg->size, send_offs.size);
  int i;
  for(i=0; i<mxi; i++) {
    TessEl* te = (TessEl*)send_offs.FastEl(i);
    cg->Cn(i)->wt = te->wt_val;
  }
}

void TesselPrjnSpec::GetCtrFmRecv(TwoDCoord& sctr, TwoDCoord ruc) {
  ruc -= recv_off;
  ruc /= recv_group;	ruc *= recv_group;	// this takes int part of
  ruc += recv_off;	// then re-add offset
  FloatTwoDCoord scruc = ruc;
  scruc *= send_scale;
  sctr = scruc;		// center of sending units
  sctr += send_border;
}

void TesselPrjnSpec::Connect_RecvUnit(Unit* ru_u, const TwoDCoord& ruc, Projection* prjn) {
  PosTDCoord su_geo;  prjn->from->GetActGeomNoSpc(su_geo);
  // positions of center of recv in sending layer
  TwoDCoord sctr;
  GetCtrFmRecv(sctr, ruc);
  int i;
  TessEl* te;
  for(i = 0; i< send_offs.size; i++) {
    te = (TessEl*)send_offs.FastEl(i);
    TwoDCoord suc = te->send_off + sctr;
    if(suc.WrapClip(wrap, su_geo))
      continue;
    Unit* su_u = prjn->from->FindUnitFmCoord(suc);
    if((su_u == NULL) || (!self_con && (su_u == ru_u)))
      continue;
    Connection* cn = ru_u->ConnectFromCk(su_u, prjn);
    if(cn != NULL)
      cn->wt = te->wt_val;
  }
}

void TesselPrjnSpec::Connect_NonLinked(Projection* prjn) {
  PosTDCoord ru_geo;  prjn->layer->GetActGeomNoSpc(ru_geo);

  TwoDCoord use_recv_n = recv_n;

  if(recv_n.x == -1)
    use_recv_n.x = ru_geo.x;
  if(recv_n.y == -1)
    use_recv_n.y = ru_geo.y;

  TwoDCoord ruc, nuc;
  for(ruc.y = recv_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_n.y);
      ruc.y += recv_skip.y, nuc.y++)
  {
    for(ruc.x = recv_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_n.x);
	ruc.x += recv_skip.x, nuc.x++)
    {
      Unit* ru_u = prjn->layer->FindUnitFmCoord(ruc);
      if(ru_u == NULL)
	continue;
      Connect_RecvUnit(ru_u, ruc, prjn);
    }
  }
}

// gp link ignores skipping!!
void TesselPrjnSpec::Connect_GpLinkFmSrc(Projection* prjn) {
  PosTDCoord ru_geo;  prjn->layer->GetActGeomNoSpc(ru_geo);
  PosTDCoord su_geo;  prjn->from->GetActGeomNoSpc(su_geo);

  if((recv_skip.x != 1) || (recv_skip.y != 1)) {
    taMisc::Error("*** recv_skip != 1 is ignored for linked tessel prjn specs in TesselPrjnSpec:",name);
  }

  TwoDCoord use_recv_n = recv_off;
  use_recv_n += recv_group;	// just do the group here

  Unit_List src_gp;		// list of source units!
  TwoDCoord ruc, nuc;
  for(ruc.y = recv_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_n.y);
      ruc.y++, nuc.y++)
  {
    for(ruc.x = recv_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_n.x);
	ruc.x++, nuc.x++)
    {
      Unit* ru_u = prjn->layer->FindUnitFmCoord(ruc);
      if(ru_u == NULL)
	continue;
      src_gp.Link(ru_u);
    }
  }

  use_recv_n = recv_n;
  if((recv_n.x == -1) || (recv_n.y == -1)) {
    use_recv_n = ru_geo;
  }

  for(ruc.y = recv_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_n.y);
      ruc.y++, nuc.y++)
  {
    for(ruc.x = recv_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_n.x);
	ruc.x++, nuc.x++)
    {
      Unit* ru_u = prjn->layer->FindUnitFmCoord(ruc);
      if((ru_u == NULL) || (src_gp.FindEl(ru_u) >= 0))
	continue; // already done it..

      Con_Group* ru_cg = ru_u->recv.FindPrjn(prjn);
      if(ru_cg != NULL) {
	ru_cg->Reset();		// get rid of existing connections if present..
      }

      TwoDCoord srcc = ruc;
      srcc -= recv_off;
      srcc %= recv_group;
      int src_ru_idx = srcc.y * recv_group.x + srcc.x;
      if(src_ru_idx >= src_gp.size)
	continue;		// some kind of bug here...
      Unit* src_ru_u = src_gp.FastEl(src_ru_idx);
      Con_Group* src_ru_cg = src_ru_u->recv.FindPrjn(prjn);
      if(src_ru_cg == NULL)	// must not have been saved with connections..
	continue;

      // positions of center of recv in sending layer
      TwoDCoord sctr;
      GetCtrFmRecv(sctr, ruc);
      int i;
      TessEl* te;
      for(i = 0; i< send_offs.size; i++) {
	te = (TessEl*)send_offs.FastEl(i);
	TwoDCoord suc = te->send_off + sctr;
	if(suc.WrapClip(wrap, su_geo))
	  continue;
	Unit* su_u = prjn->from->FindUnitFmCoord(suc);
	if((su_u == NULL) || (!self_con && (su_u == ru_u)))
	  continue;
	Connection* src_cn = (Connection*)src_ru_cg->SafeEl(i);
	if(src_cn == NULL) {
	  taMisc::Error("*** Source unit:",String(src_ru_idx),"for linking in TesselPrjnSpec:",name,
			"does not have all the connections");
	  continue;
	}

	ru_u->ConnectFromLinkCk(su_u, prjn, src_cn, ru_cg);
	ru_cg->own_cons = false; // doesn't own them..
      }
    }
  }
}

void TesselPrjnSpec::Connect_GpLinked(Projection* prjn) {
  PosTDCoord ru_geo;
  prjn->layer->GetActGeomNoSpc(ru_geo);
  TwoDCoord use_recv_n = recv_off;
  use_recv_n += recv_group;	// just do the group here

  if((recv_skip.x != 1) || (recv_skip.y != 1)) {
    taMisc::Error("*** recv_skip != 1 is ignored for linked tessel prjn specs in TesselPrjnSpec:",name);
  }

  TwoDCoord ruc, nuc;
  for(ruc.y = recv_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_n.y);
      ruc.y++, nuc.y++)
  {
    for(ruc.x = recv_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_n.x);
	ruc.x++, nuc.x++)
    {
      Unit* ru_u = prjn->layer->FindUnitFmCoord(ruc);
      if(ru_u == NULL)
	continue;
      Connect_RecvUnit(ru_u, ruc, prjn);
    }
  }
  Connect_GpLinkFmSrc(prjn);
}

// just connects linked units, assuming link_src is already connected!
void TesselPrjnSpec::Connect_UnLinkFmSrc(Projection* prjn) {
  PosTDCoord ru_geo;  prjn->layer->GetActGeomNoSpc(ru_geo);
  PosTDCoord su_geo;  prjn->from->GetActGeomNoSpc(su_geo);
  TwoDCoord use_recv_n = recv_n;

  if((recv_n.x == -1) || (recv_n.y == -1)) {
    use_recv_n = ru_geo;
  }

  TwoDCoord ruc, nuc;		// receving unit coords
  // the "source" unit for linked weights
  Unit* src_ru_u = prjn->layer->FindUnitFmCoord(link_src);
  if(src_ru_u == NULL) {
    taMisc::Error("Can't find source unit for wt linking in TesselPrjnSpec:", name);
    return;
  }
  Con_Group* src_ru_cg = src_ru_u->recv.FindPrjn(prjn);
  if(src_ru_cg == NULL)	// must not have been saved with connections..
    return;

  for(ruc.y = recv_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_n.y);
      ruc.y += recv_skip.y, nuc.y++)
  {
    for(ruc.x = recv_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_n.x);
	ruc.x += recv_skip.x, nuc.x++)
    {
      Unit* ru_u = prjn->layer->FindUnitFmCoord(ruc);
      if((ru_u == NULL) || (ru_u == src_ru_u))	continue; // already done it..

      Con_Group* ru_cg = ru_u->recv.FindPrjn(prjn);
      if(ru_cg != NULL) {
	ru_cg->Reset();		// get rid of existing connections if present..
      }

      // positions of center of recv in sending layer
      TwoDCoord sctr;
      GetCtrFmRecv(sctr, ruc);
      int i;
      TessEl* te;
      for(i = 0; i< send_offs.size; i++) {
	te = (TessEl*)send_offs.FastEl(i);
	TwoDCoord suc = te->send_off + sctr;
	if(suc.WrapClip(wrap, su_geo))
	  continue;
	Unit* su_u = prjn->from->FindUnitFmCoord(suc);
	if((su_u == NULL) || (!self_con && (su_u == ru_u)))
	  continue;
	Connection* src_cn = (Connection*)src_ru_cg->SafeEl(i);
	if(src_cn == NULL) {
	  taMisc::Error("*** Source unit for linking in TesselPrjnSpec:",name,
			"does not have all the connections");
	  continue;
	}

	ru_u->ConnectFromLinkCk(su_u, prjn, src_cn, ru_cg);
	ru_cg->own_cons = false; // doesn't own them..
      }
    }
  }
}

void TesselPrjnSpec::Connect_UnLinked(Projection* prjn) {
  Unit* src_ru_u = prjn->layer->FindUnitFmCoord(link_src);
  if(src_ru_u == NULL) {
    taMisc::Error("Can't find source unit for wt linking in TesselPrjnSpec:", name);
    return;
  }
  // make connections for the source
  Connect_RecvUnit(src_ru_u, link_src, prjn);
  // then just get everybody else to copy from it!
  Connect_UnLinkFmSrc(prjn);
}

void TesselPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  if(prjn->layer->units.leaves == 0) // an empty layer!
    return;

  if(!wrap && init_wts) {
    taMisc::Error("*** Warning: TesselPrjnSpec non-wrapped tessel prjn spec with init_wts does not usually work!:",name);
  }

#ifdef DMEM_COMPILE
  if(prjn->layer->dmem_dist != Layer::DMEM_DIST_UNITGP) {
    if(link_type != NO_LINK) {
      taMisc::Error("Error: linked connections (TesselPrjnSpec::link_type != NO_LINK) do not work with DMEM dist_units != DMEM_DIST_UNITGP -- NO_LINK will be used!");
    }
    Connect_NonLinked(prjn);
  }
  else
#endif
    {
      switch(link_type) {
      case NO_LINK:
	Connect_NonLinked(prjn);
	break;
      case GP_LINK:
	Connect_GpLinked(prjn);
	break;
      case UN_LINK:
	Connect_UnLinked(prjn);
	break;
      }
    }
}

void TesselPrjnSpec::ReConnect_Load(Projection* prjn) {
#ifdef DMEM_COMPILE
  if(prjn->layer->dmem_dist != Layer::DMEM_DIST_UNITGP) {
    if(link_type != NO_LINK) {
      taMisc::Error("Error: linked connections (TesselPrjnSpec::link_type != NO_LINK) do not work with DMEM dist_units != DMEM_DIST_UNITGP -- NO_LINK will be used!");
    }
  }
  else
#endif
    {
      if(prjn->layer->units.leaves != 0) {
	switch(link_type) {
	case NO_LINK:
	  break;
	case GP_LINK:
	  Connect_GpLinkFmSrc(prjn);
	  break;
	case UN_LINK:
	  Connect_UnLinkFmSrc(prjn);
	  break;
	}
      }
    }
  // then deal with other in a generic way..
  ProjectionSpec::ReConnect_Load(prjn);
}


/////////////////////////////
//	  UniformRnd	   //
/////////////////////////////

void UniformRndPrjnSpec::Initialize() {
  p_con = .25;
  permute = true;
  sym_self = true;
  same_seed = false;
  rndm_seed.GetCurrent();
}

void UniformRndPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(rndm_seed, this);
}

void UniformRndPrjnSpec::UpdateAfterEdit() {
  ProjectionSpec::UpdateAfterEdit();
  if(p_con > 1.0f) p_con = 1.0f;
  if(p_con < 0.0f) p_con = 0.0f;
}

void UniformRndPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(same_seed)
    rndm_seed.OldSeed();

  int no;
  if(!self_con && (prjn->from == prjn->layer))
    no = (int) (p_con * (float)(prjn->from->units.leaves-1));
  else
    no = (int) (p_con * (float)prjn->from->units.leaves);
  if(no <= 0) no = 1;

  if((prjn->from == prjn->layer) && sym_self) {
    Layer* lay = prjn->layer;
    // trick is to divide cons in half, choose recv, send at random
    // for 1/2 cons, then go through all units and make the symmetric cons..
    if(permute) {
      if(p_con > .95f) {
	taMisc::Error("Warning: UniformRndPrjnSpec makes less than complete connectivity for high values of p_con in symmetric, self-connected layers using permute!");
      }
      // pre-allocate connections!
      int first;
      if(!self_con)
	first = (int) (.5f * p_con * (float)(prjn->from->units.leaves-1));
      else
	first = (int) (.5f * p_con * (float)prjn->from->units.leaves);
      if(first <= 0) first = 1;

      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      taPtrList<Unit> ru_list;		// receiver permution list
      taPtrList<Unit> perm_list;	// sender permution list

      FOR_ITR_EL(Unit, ru, lay->units., ru_itr)	// need to permute recvs because of exclusion
	ru_list.Link(ru);			// on making a symmetric connection in first pass
      ru_list.Permute();
      
      int i;
      for(i=0;i<ru_list.size; i++) {
	ru = ru_list.FastEl(i);
	perm_list.Reset();
	FOR_ITR_EL(Unit, su, lay->units., su_itr) {
	  if(!self_con && (ru == su)) continue;
	  // don't connect to anyone who already recvs from me cuz that will make
	  // a symmetric connection which isn't good: symmetry will be enforced later
	  Con_Group* scg = su->recv.FindPrjn(prjn);
	  if(scg->units.Find(ru) >= 0) continue; 
	  perm_list.Link(su);
	}
	perm_list.Permute();
	int j;
	for(j=0; j<first && j<perm_list.size; j++)	// only connect 1/2 of the units
	  ru->ConnectFrom((Unit*)perm_list[j], prjn);
      }
    }
    else {
      int first;
      if(!self_con)
	first = (int) (.5f * p_con * (float)(prjn->from->units.leaves-1) * (float)prjn->from->units.leaves);
      else
	first = (int) (.5f * p_con * (float)prjn->from->units.leaves * (float)prjn->from->units.leaves);
      if(first <= 0) first = 1;

      int cnt = 0;
      while (cnt < first) {
	int ridx = Random::IntZeroN(lay->units.leaves);
	int sidx = Random::IntZeroN(lay->units.leaves);
	if(!self_con && (ridx == sidx)) continue;
	Unit* ru = lay->units.Leaf(ridx);
	Unit* su = lay->units.Leaf(sidx);
	// don't connect to anyone who already recvs from me cuz that will make
	// a symmetric connection which isn't good: symmetry will be enforced later
	Con_Group* scg = su->recv.FindPrjn(prjn);
	if(scg->units.Find(ru) >= 0) continue; 
	if(ru->ConnectFrom(su, prjn) != NULL)
	  cnt++;
      }
    }
    // now go thru and make the symmetric connections
    Unit* ru;
    taLeafItr ru_itr;
    FOR_ITR_EL(Unit, ru, lay->units., ru_itr) {
      Con_Group* scg = ru->send.FindPrjn(prjn);
      if(scg == NULL) continue;
      int i;
      for(i=0;i<scg->size;i++) {
	Unit* su = scg->Un(i);
	ru->ConnectFromCk(su, prjn);
      }
    }
  }
  else {			// not a symmetric self projection
    if(!permute) {
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
	FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
	  if(!self_con && (ru == su)) continue;
	  if(Random::ZeroOne() <= p_con)
	    ru->ConnectFrom(su, prjn);
	}  
      }
    }
    else {
      // pre-allocate connections!
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr)
	ru->ConnectAlloc(no, prjn);

      taPtrList<Unit> perm_list;	// permution list
      FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
	perm_list.Reset();
	FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
	  if(!self_con && (ru == su)) continue;
	  perm_list.Link(su);
	}
	perm_list.Permute();
	int i;
	for(i=0; i<no && i<perm_list.size; i++)
	  ru->ConnectFrom((Unit*)perm_list[i], prjn);
      }
    }
  }
}

/////////////////////////////
//	  PolarRnd	   //
/////////////////////////////

void PolarRndPrjnSpec::Initialize() {
  p_con = .25;

  rnd_dist.type = Random::GAUSSIAN;
  rnd_dist.mean = 0.0f;
  rnd_dist.var = .25f;

  rnd_angle.type = Random::UNIFORM;
  rnd_angle.mean = 0.5f;
  rnd_angle.var = 0.5f;
  
  dist_type = XY_DIST_CENTER_NORM;
  wrap = false;
  max_retries = 1000;

  same_seed = false;
  rndm_seed.GetCurrent();
}

void PolarRndPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(rnd_dist, this);
  taBase::Own(rnd_angle, this);
  taBase::Own(rndm_seed, this);
}

void PolarRndPrjnSpec::UpdateAfterEdit() {
  ProjectionSpec::UpdateAfterEdit();
  if(p_con > 1.0f) p_con = 1.0f;
  if(p_con < 0.0f) p_con = 0.0f;
}

float PolarRndPrjnSpec::UnitDist(UnitDistType typ, Projection* prjn, 
			       const TwoDCoord& ru, const TwoDCoord& su)
{
  FloatTwoDCoord half(.5f);
  PosTDCoord ru_geom; prjn->layer->GetActGeomNoSpc(ru_geom);
  PosTDCoord su_geom; prjn->from->GetActGeomNoSpc(su_geom);
  switch(typ) {
  case XY_DIST:
    return ru.Dist(su);
  case XY_DIST_CENTER: {
    FloatTwoDCoord rctr = ru_geom;   rctr *= half;
    FloatTwoDCoord sctr = su_geom;    sctr *= half;
    TwoDCoord ruc = ru - (TwoDCoord)rctr;
    TwoDCoord suc = su - (TwoDCoord)sctr;
    return ruc.Dist(suc);
  }
  case XY_DIST_NORM: {
    FloatTwoDCoord ruc = ru;	ruc /= (FloatTwoDCoord)ru_geom;
    FloatTwoDCoord suc = su;	suc /= (FloatTwoDCoord)su_geom;
    return ruc.Dist(suc);
  }
  case XY_DIST_CENTER_NORM: {
    FloatTwoDCoord rctr = ru_geom;   rctr *= half;
    FloatTwoDCoord sctr = su_geom;    sctr *= half;
    FloatTwoDCoord ruc = ((FloatTwoDCoord)ru - rctr) / rctr;
    FloatTwoDCoord suc = ((FloatTwoDCoord)su - sctr) / sctr;
    return ruc.Dist(suc);
  }
  }
  return 0.0f;
}

Unit* PolarRndPrjnSpec::GetUnitFmOff(UnitDistType typ, bool wrap, Projection* prjn, 
				   const TwoDCoord& ru, const FloatTwoDCoord& su_off)
{
  FloatTwoDCoord half(.5f);
  PosTDCoord ru_geom; prjn->layer->GetActGeomNoSpc(ru_geom);
  PosTDCoord su_geom; prjn->from->GetActGeomNoSpc(su_geom);
  TwoDCoord suc;		// actual su coordinates
  switch(typ) {
  case XY_DIST: {
    suc = su_off;
    suc += ru;
    break;
  }
  case XY_DIST_CENTER: {	// do everything relative to center
    FloatTwoDCoord rctr = ru_geom;   rctr *= half;
    FloatTwoDCoord sctr = su_geom;    sctr *= half;
    TwoDCoord ruc = ru - (TwoDCoord)rctr;
    suc = su_off;
    suc += ruc;			// add the centerized coordinates
    suc += (TwoDCoord)sctr;	// then add the sending center back into it..
    break;
  }
  case XY_DIST_NORM: {
    FloatTwoDCoord ruc = ru;	ruc /= (FloatTwoDCoord)ru_geom;
    FloatTwoDCoord suf = su_off + ruc; // su_off is in normalized coords, so normalize ru
    suf *= (FloatTwoDCoord)su_geom;
    suc = suf;
    break;
  }
  case XY_DIST_CENTER_NORM: {
    FloatTwoDCoord rctr = ru_geom;   rctr *= half;
    FloatTwoDCoord sctr = su_geom;    sctr *= half;
    FloatTwoDCoord ruc = ((FloatTwoDCoord)ru - rctr) / rctr;
    FloatTwoDCoord suf = su_off + ruc;
    suf *= sctr;    suf += sctr;
    suc = suf;
    break;
  }
  }
  if(suc.WrapClip(wrap, su_geom))
    return NULL;

  Unit* su_u = (Unit*)prjn->from->FindUnitFmCoord(suc);
  return su_u;
}


float PolarRndPrjnSpec::GetDistProb(Projection* prjn, Unit* ru, Unit* su) {
  if(rnd_dist.type == Random::UNIFORM)
    return p_con;
  float prob = p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, su->pos));
  if(wrap) {
    PosTDCoord su_geom; prjn->from->GetActGeomNoSpc(su_geom);
    TwoDCoord suc = su->pos;
    suc.x += su_geom.x; // wrap around in x
    prob += p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
    suc.y += su_geom.y; // wrap around in x & y
    prob += p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
    suc.x = su->pos.x;		// just y
    prob += p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
    suc = su->pos;
    suc.x -= su_geom.x; // wrap around in x
    prob -= p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
    suc.y -= su_geom.y; // wrap around in y
    prob += p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
    suc.x = su->pos.x;		// just y
    prob += p_con * rnd_dist.Density(UnitDist(dist_type, prjn, ru->pos, suc));
  }
  return prob;
}

// todo: could put in some sending limits, and do recvs in random order

void PolarRndPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(same_seed)
    rndm_seed.OldSeed();

  PosTDCoord ru_geom; prjn->layer->GetActGeomNoSpc(ru_geom);
  TwoDCoord ru_pos;		// do this according to act_geom..
  int cnt = 0;
  Unit* ru;
  taLeafItr ru_itr;
  for(ru = (Unit*)prjn->layer->units.FirstEl(ru_itr); ru;
      ru = (Unit*)prjn->layer->units.NextEl(ru_itr), cnt++) {
    ru_pos.y = cnt / ru_geom.x;
    ru_pos.x = cnt % ru_geom.x;
    Con_Group* recv_gp = NULL;
    int n_leaves = prjn->from->units.leaves;
    if(!self_con && (prjn->from == prjn->layer))
      n_leaves--;
    int trg_con = (int)(p_con * (float)n_leaves);
    FloatTwoDCoord suc;
    int n_con = 0;
    int n_retry = 0;
    while((n_con < trg_con) && (n_retry < max_retries)) { // limit number of retries
      float dist = rnd_dist.Gen();		// just get random deviate from distribution
      float angle = 2.0 * 3.14159265 * rnd_angle.Gen(); // same for angle
      suc.x = dist * cos(angle);
      suc.y = dist * sin(angle);
      Unit* su = GetUnitFmOff(dist_type, wrap, prjn, ru_pos, suc);
      if((su == NULL) || (!self_con && (ru == su))) {
	n_retry++;
	continue;
      }
      if(ru->ConnectFromCk(su, prjn, recv_gp) != NULL)
	n_con++;
      else {
	n_retry++;		// already connected, retry
	continue;
      }
    }
    if(n_con < trg_con) {
      taMisc::Error("*** Warning: PolarRndPrjnSpec target no of connections:",String(trg_con),
		    "not made, only made:",String(n_con));
    }
  }
}

void PolarRndPrjnSpec::C_InitWtState(Projection* prjn, Con_Group* cg, Unit* ru) {
  int i;
  for(i=0; i<cg->size; i++) {
    cg->Cn(i)->wt = GetDistProb(prjn, ru, cg->Un(i));
  }
}

/////////////////////////////
//	  Symmetric	   //
/////////////////////////////

void SymmetricPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  int cnt = 0;
  Unit* ru, *su;
  taLeafItr ru_itr, su_itr;
  FOR_ITR_EL(Unit, ru, prjn->layer->units., ru_itr) {
    FOR_ITR_EL(Unit, su, prjn->from->units., su_itr) {
      if(ru->recv.FindRecipRecvCon(su, ru) != NULL) 
	if(ru->ConnectFrom(su, prjn) != NULL)
	  cnt++;
    }  
  }
  if(cnt == 0) {
    taMisc::Error("Warning: SymmetricPrjnSpec on prjn:",prjn->name,"did not make any connections.", 
		  "Note that this layer must be *earlier* in list of layers than the one you are trying to symmetrize from.");
  }
}


//////////////////////////
//	LinkPrjnSpec	//
//////////////////////////

void LinkPrjnConPtr::Initialize() {
  recv_idx = 0;
  send_idx = 0;
}

void LinkPrjnConPtr::Copy_(const LinkPrjnConPtr& cp) {
  recv_layer = cp.recv_layer;
  recv_idx = cp.recv_idx;
  send_layer = cp.send_layer;
  send_idx = cp.send_idx;
}

Con_Group* LinkPrjnConPtr::GetCon(Network* net, int& idx, Unit*& ru, Unit*& su,
				  Con_Group*& su_cg, int& sg_idx)
{
  Layer* recv_lay = (Layer*)net->layers.FindName(recv_layer);
  if(recv_lay == NULL) {
    taMisc::Error("LinkPrjnConPtr: Couldn't find layer named:", recv_layer,
		  "in network:",net->name);
    return NULL;
  }
  Layer* send_lay = (Layer*)net->layers.FindName(send_layer);
  if(send_lay == NULL) {
    taMisc::Error("LinkPrjnConPtr: Couldn't find layer named:", send_layer,
		  "in network:",net->name);
    return NULL;
  }
  ru = recv_lay->units.Leaf(recv_idx);
  su = send_lay->units.Leaf(send_idx);
  if((ru == NULL) || (su == NULL))
    return NULL;

  Con_Group* rval;
  int g;
  FOR_ITR_GP(Con_Group, rval, ru->recv., g) {
    if(rval->prjn->from == send_lay) { 	// receives from sending layer
      idx = rval->units.Find(su);
      if(idx >= 0) {
	int sg;
	for(sg=0; sg < su->send.gp.size; sg++) { // can't use iterator cause empty gps
	  su_cg = (Con_Group*)su->send.gp.FastEl(sg);
	  if((su_cg == NULL) || (su_cg->prjn == NULL) || (su_cg->prjn->layer != recv_lay))
	    continue;
	  sg_idx = su_cg->units.Find(ru);
	  if(sg_idx >= 0)
	    return rval;
	}
      }
    }
  }
  return NULL;
}

void LinkPrjnSpec::Initialize() {
  links.SetBaseType(&TA_LinkPrjnConPtr);
}

void LinkPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(links, this);
}

void LinkPrjnSpec::CutLinks() {
  links.RemoveAll();
  ProjectionSpec::CutLinks();
}

void LinkPrjnSpec::Copy_(const LinkPrjnSpec& cp) {
  links = cp.links;
}

void LinkPrjnSpec::Connect_impl(Projection* prjn) {
  if(links.size < 2)
    return;

  Network* net = prjn->layer->own_net;
  if(net == NULL)
    return;

  Unit* m_ru, *m_su;
  int m_rg_idx, m_sg_idx;
  Con_Group* m_su_cg;
  Con_Group* m_ru_cg = links.FastEl(0)->GetCon(net,m_rg_idx, m_ru, m_su,
					       m_su_cg, m_sg_idx);
  if(m_ru_cg == NULL)
    return;
  Connection* m_cn = m_ru_cg->Cn(m_rg_idx);
  if(m_cn == NULL)
    return;

  int i;
  for(i=1; i<links.size; i++) {
    Unit* ru, *su;
    int rg_idx, sg_idx;
    Con_Group* su_cg;
    Con_Group* ru_cg = links.FastEl(i)->GetCon(net,rg_idx, ru, su, su_cg, sg_idx);
    if(ru_cg == NULL)
      continue;
    ru_cg->ReplaceLink(rg_idx, m_cn); // replace on both sides of the connection
    su_cg->ReplaceLink(sg_idx, m_cn);
  }
}

void LinkPrjnSpec::ReConnect_Load(Projection* prjn) {
  Connect(prjn);
}

void LinkPrjnSpec::CopyNetwork(Network* net, Network* cn, Projection* prjn, Projection* cp) {
  ProjectionSpec::CopyNetwork(net, cn, prjn, cp);
  Connect(prjn);
}


/////////////////////////////
//	  Script	   //
/////////////////////////////

void ScriptPrjnSpec::Initialize() {
  prjn = NULL;
}

void ScriptPrjnSpec::Destroy() {
  prjn = NULL;
}

void ScriptPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(script_file, this);
  taBase::Own(s_args, this);
  if(script_file.fname.empty())	// initialize only on startup up, not transfer
    SetScript("");
}

void ScriptPrjnSpec::Copy_(const ScriptPrjnSpec& cp) {
  s_args = cp.s_args;
  script_file = cp.script_file;
  script_string = cp.script_string;
}

void ScriptPrjnSpec::Connect_impl(Projection* prj) {
  prjn = prj;			// set the arg for the script
  RunScript();
  prjn = NULL;
}

void ScriptPrjnSpec::UpdateAfterEdit() {
  UpdateReCompile();
  if(!script_file.fname.empty()) {
    name = script_file.fname.before(".css");
    if(name.contains('/'))
      name = name.after('/', -1);
    int i;
    for(i=0;i<s_args.size;i++)
      name += String("_") + s_args[i];
  }
}

void ScriptPrjnSpec::Interact() {
  if(script == NULL)   return;
  cssMisc::next_shell = script;
}
  
void ScriptPrjnSpec::Compile() {
  LoadScript();
}

/////////////////////////////
//	  Custom	   //
/////////////////////////////

void CustomPrjnSpec::Connect(Projection* prjn) {
  // make sure i have the correct indicies for my con_groups..
  prjn->recv_idx = -1;
  prjn->send_idx = -1;
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, prjn->layer->units., i) {
    int idx;
    Con_Group* cg = u->recv.FindPrjn(prjn, idx);
    if(cg != NULL) {
      prjn->recv_idx = idx;
      break;
    }
  }
  FOR_ITR_EL(Unit, u, prjn->from->units., i) {
    int idx;
    Con_Group* cg = u->send.FindPrjn(prjn, idx);
    if(cg != NULL) {
      prjn->send_idx = idx;
      break;
    }
  }

  prjn->projected = true;	// don't do anything else..
}

//////////////////////////////////////////
//	UnitGroup-based PrjnSpecs	//
//////////////////////////////////////////

//////////////////////////////////////////
// 		GpFullPrjnSpec		//
//////////////////////////////////////////

void GpFullPrjnSpec::Initialize() {
  n_con_groups = SEND_ONLY;
}

void GpFullPrjnSpec::GetNGroups(Projection* prjn, int& r_n_ugp, int& s_n_ugp) {
  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);
  if(recv_ugp->gp.size > 0)
    r_n_ugp = recv_ugp->gp.size;
  else
    r_n_ugp = 1;
  if(send_ugp->gp.size > 0)
    s_n_ugp = send_ugp->gp.size;
  else
    s_n_ugp = 1;

  prjn->recv_n = s_n_ugp; 
  if(n_con_groups == RECV_SEND_PAIR)
    prjn->send_n = r_n_ugp;	// number sending = number of recv unit groups
  else
    prjn->send_n = 1;		// one sending group
}

void GpFullPrjnSpec::PreConnect(Projection* prjn) {
  prjn->SetFrom();		// justin case
  if(prjn->from == NULL)	return;

  int r_n_ugp, s_n_ugp;
  GetNGroups(prjn, r_n_ugp, s_n_ugp);

  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);

  // make first set of congroups to get indicies
  Unit* first_ru = (Unit*)recv_ugp->Leaf(0);
  Unit* first_su = (Unit*)send_ugp->Leaf(0);
  if((first_ru == NULL) || (first_su == NULL))
    return;
  Con_Group* recv_gp = first_ru->recv.NewPrjn(prjn, true);
  prjn->recv_idx = first_ru->recv.gp.size - 1;
  Con_Group* send_gp = first_su->send.NewPrjn(prjn, false);
  prjn->send_idx = first_su->send.gp.size - 1;
  // set reciprocal indicies
  recv_gp->other_idx = prjn->send_idx;
  send_gp->other_idx = prjn->recv_idx;

  // use basic connectivity routine to set indicies..
  int r, s;
  for(r=0; r<r_n_ugp; r++) {
    Unit_Group* rgp;
    if(recv_ugp->gp.size > 0)
      rgp = (Unit_Group*)recv_ugp->gp.FastEl(r);
    else
      rgp = recv_ugp;
    for(s=0; s<s_n_ugp; s++) {
      Unit_Group* sgp;
      if(send_ugp->gp.size > 0)
	sgp = (Unit_Group*)send_ugp->gp.FastEl(s);
      else
	sgp = send_ugp;

      int recv_idx = prjn->recv_idx + s;
      int send_idx = prjn->send_idx;
      if(n_con_groups == RECV_SEND_PAIR)
	send_idx += r;

      // then its full connectivity..
      Unit* u;
      taLeafItr u_itr;
      FOR_ITR_EL(Unit, u, rgp->, u_itr) {
	if((u == first_ru) && (s == 0))	continue; // skip this one
	recv_gp = u->recv.NewPrjn(prjn, true);
	recv_gp->other_idx = send_idx;
      }
      FOR_ITR_EL(Unit, u, sgp->, u_itr) {
	if((u == first_su) && (r == 0))	continue; // skip this one
	send_gp = u->send.NewPrjn(prjn, false);
	send_gp->other_idx = recv_idx;
      }
    }
  }
}

void GpFullPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  int orig_recv_idx = prjn->recv_idx;
  int orig_send_idx = prjn->send_idx;

  int r_n_ugp, s_n_ugp;
  GetNGroups(prjn, r_n_ugp, s_n_ugp);

  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);
  int r, s;
  for(r=0; r<r_n_ugp; r++) {
    Unit_Group* rgp;
    if(recv_ugp->gp.size > 0)
      rgp = (Unit_Group*)recv_ugp->gp.FastEl(r);
    else
      rgp = recv_ugp;
    for(s=0; s<s_n_ugp; s++) {
      Unit_Group* sgp;
      if(send_ugp->gp.size > 0)
	sgp = (Unit_Group*)send_ugp->gp.FastEl(s);
      else
	sgp = send_ugp;

      prjn->recv_idx = orig_recv_idx + s;
      if(n_con_groups == RECV_SEND_PAIR)
	prjn->send_idx = orig_send_idx + r;

      // then its full connectivity..
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, rgp->, ru_itr) {
	FOR_ITR_EL(Unit, su, sgp->, su_itr) {
	  if(self_con || (ru != su))
	    ru->ConnectFrom(su, prjn);
	}  
      }
    }
  }
  prjn->recv_idx = orig_recv_idx;
  prjn->send_idx = orig_send_idx;
}


//////////////////////////////////////////
// 	GpOneToOnePrjnSpec		//
//////////////////////////////////////////

void GpOneToOnePrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  Unit_Group* recv_gp = &(prjn->layer->units);
  Unit_Group* send_gp = &(prjn->from->units);
  int i;
  int max_n = n_conns;
  if(n_conns < 0)
    max_n = recv_gp->gp.size - recv_start;
  max_n = MIN(recv_gp->gp.size - recv_start, max_n);
  max_n = MIN(send_gp->gp.size - send_start, max_n);
  max_n = MAX(1, max_n);	// lower limit of 1
  for(i=0; i<max_n; i++) {
    Unit_Group* rgp, *sgp;
    // revert to main group if no sub groups
    if(recv_gp->gp.size > 0)
      rgp = (Unit_Group*)recv_gp->gp.FastEl(recv_start + i);
    else
      rgp = recv_gp;
    if(send_gp->gp.size > 0)
      sgp = (Unit_Group*)send_gp->gp.FastEl(send_start + i);
    else
      sgp = send_gp;

    // then its full connectivity..
    Unit* ru, *su;
    taLeafItr ru_itr, su_itr;
    FOR_ITR_EL(Unit, ru, rgp->, ru_itr) {
      FOR_ITR_EL(Unit, su, sgp->, su_itr) {
	if(self_con || (ru != su))
	  ru->ConnectFrom(su, prjn);
      }  
    }
  }
}


//////////////////////////////////////////
// 	RndGpOneToOnePrjnSpec		//
//////////////////////////////////////////

void RndGpOneToOnePrjnSpec::Initialize() {
  p_con = .25;
  permute = true;
  same_seed = false;
  rndm_seed.GetCurrent();
}

void RndGpOneToOnePrjnSpec::InitLinks() {
  GpOneToOnePrjnSpec::InitLinks();
  taBase::Own(rndm_seed, this);
}

void RndGpOneToOnePrjnSpec::UpdateAfterEdit() {
  GpOneToOnePrjnSpec::UpdateAfterEdit();
  if(p_con > 1.0f) p_con = 1.0f;
  if(p_con < 0.0f) p_con = 0.0f;
}

void RndGpOneToOnePrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(same_seed)
    rndm_seed.OldSeed();

  Unit_Group* recv_gp = &(prjn->layer->units);
  Unit_Group* send_gp = &(prjn->from->units);
  int i;
  int max_n = n_conns;
  if(n_conns < 0)
    max_n = recv_gp->gp.size - recv_start;
  if(recv_gp->gp.size > 0)
    max_n = MIN(recv_gp->gp.size - recv_start, max_n);
  if(send_gp->gp.size > 0)
    max_n = MIN(send_gp->gp.size - send_start, max_n);
  max_n = MAX(1, max_n);	// lower limit of 1
  for(i=0; i<max_n; i++) {
    Unit_Group* rgp, *sgp;
    // revert to main group if no sub groups
    if(recv_gp->gp.size > 0)
      rgp = (Unit_Group*)recv_gp->gp.FastEl(recv_start + i);
    else
      rgp = recv_gp;
    if(send_gp->gp.size > 0)
      sgp = (Unit_Group*)send_gp->gp.FastEl(send_start + i);
    else
      sgp = send_gp;

    int no = (int) (p_con * (float)sgp->leaves);
    if(!self_con && (rgp == sgp))
      no--;
    if(no <= 0)
      no = 1;

    if(!permute) {
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, rgp->, ru_itr) {
	FOR_ITR_EL(Unit, su, sgp->, su_itr) {
	  if(!self_con && (ru == su)) continue;
	  if(Random::ZeroOne() <= p_con)
	    ru->ConnectFrom(su, prjn);
	}  
      }
    }
    else {
      // pre-allocate connections!
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, rgp->, ru_itr)
	ru->ConnectAlloc(no, prjn);

      taPtrList<Unit> perm_list;	// permution list
      FOR_ITR_EL(Unit, ru, rgp->, ru_itr) {
	perm_list.Reset();
	FOR_ITR_EL(Unit, su, sgp->, su_itr) {
	  if(!self_con && (ru == su)) continue;
	  perm_list.Link(su);
	}
	perm_list.Permute();
	int i;
	for(i=0; i<no; i++)
	  ru->ConnectFrom((Unit*)perm_list[i], prjn);
      }
    }
  }
}


//////////////////////////////////////////
// 	GpOneToManyPrjnSpec		//
//////////////////////////////////////////

void GpOneToManyPrjnSpec::Initialize() {
  n_con_groups = SEND_ONLY;
}

void GpOneToManyPrjnSpec::GetNGroups(Projection* prjn, int& r_n_ugp, int& s_n_ugp) {
  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);

  s_n_ugp = n_conns;
  if(n_conns < 0)
    s_n_ugp = send_ugp->gp.size - send_start;
  s_n_ugp = MIN(send_ugp->gp.size - send_start, s_n_ugp);
  s_n_ugp = MAX(1, s_n_ugp);	// lower limit of 1

  if(recv_ugp->gp.size > 0)
    r_n_ugp = recv_ugp->gp.size;
  else
    r_n_ugp = 1;

  if(n_con_groups == RECV_SEND_PAIR) {
    prjn->recv_n = s_n_ugp;
    prjn->send_n = r_n_ugp;
  }
  else if(n_con_groups == SEND_ONLY) {
    prjn->recv_n = s_n_ugp;
    prjn->send_n = 1;
  }
  else {
    prjn->recv_n = 1;
    prjn->send_n = 1;
  }
}

// preconnect assumes full interconnectivty in order to maintain 
// homogeneity of all unit con_groups in a layer.  this wastes some, but what can you do..
void GpOneToManyPrjnSpec::PreConnect(Projection* prjn) {
  prjn->SetFrom();		// justin case
  if(prjn->from == NULL)	return;

  int r_n_ugp, s_n_ugp;
  int old_send_start = send_start; // temporarilly resort to full connectivity
  int old_n_conns = n_conns;
  send_start = 0;
  n_conns = -1;
  GetNGroups(prjn, r_n_ugp, s_n_ugp);
  send_start = old_send_start;
  n_conns = old_n_conns;

  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);

  // make first set of congroups to get indicies
  Unit* first_ru = (Unit*)recv_ugp->Leaf(0);
  Unit* first_su = (Unit*)send_ugp->Leaf(0);
  if((first_ru == NULL) || (first_su == NULL))
    return;
  Con_Group* recv_gp = first_ru->recv.NewPrjn(prjn, true);
  prjn->recv_idx = first_ru->recv.gp.size - 1;
  Con_Group* send_gp = first_su->send.NewPrjn(prjn, false);
  prjn->send_idx = first_su->send.gp.size - 1;
  // set reciprocal indicies
  recv_gp->other_idx = prjn->send_idx;
  send_gp->other_idx = prjn->recv_idx;

  // use basic connectivity routine to set indicies..
  int r, s;
  for(r=0; r<r_n_ugp; r++) {
    Unit_Group* rgp;
    if(recv_ugp->gp.size > 0)
      rgp = (Unit_Group*)recv_ugp->gp.FastEl(r);
    else
      rgp = recv_ugp;
    for(s=0; s<s_n_ugp; s++) {
      Unit_Group* sgp;
      if(send_ugp->gp.size > 0)
	sgp = (Unit_Group*)send_ugp->gp.FastEl(s);
      else
	sgp = send_ugp;

      int recv_idx = prjn->recv_idx;
      int send_idx = prjn->send_idx;
      if(n_con_groups != ONE_GROUP)
	recv_idx += s;
      if(n_con_groups == RECV_SEND_PAIR)
	send_idx += r;

      // then its full connectivity..
      Unit* u;
      taLeafItr u_itr;
      FOR_ITR_EL(Unit, u, rgp->, u_itr) {
	if((u == first_ru) && (s == 0))	continue; // skip this one
	recv_gp = u->recv.NewPrjn(prjn, true);
	recv_gp->other_idx = send_idx;
      }
      FOR_ITR_EL(Unit, u, sgp->, u_itr) {
	if((u == first_su) && (r == 0))	continue; // skip this one
	send_gp = u->send.NewPrjn(prjn, false);
	send_gp->other_idx = recv_idx;
      }
    }
  }
}

void GpOneToManyPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;

  int orig_recv_idx = prjn->recv_idx;
  int orig_send_idx = prjn->send_idx;

  int r_n_ugp, s_n_ugp;
  GetNGroups(prjn, r_n_ugp, s_n_ugp);

  Unit_Group* recv_ugp = &(prjn->layer->units);
  Unit_Group* send_ugp = &(prjn->from->units);
  int r, s;
  for(r=0; r<r_n_ugp; r++) {
    Unit_Group* rgp;
    if(recv_ugp->gp.size > 0)
      rgp = (Unit_Group*)recv_ugp->gp.FastEl(r);
    else
      rgp = recv_ugp;
    for(s=0; s<s_n_ugp; s++) {
      Unit_Group* sgp;
      if(send_ugp->gp.size > 0)
	sgp = (Unit_Group*)send_ugp->gp.FastEl(send_start + s);
      else
	sgp = send_ugp;

      if(n_con_groups != ONE_GROUP)
	prjn->recv_idx = orig_recv_idx + s;
      if(n_con_groups == RECV_SEND_PAIR)
	prjn->send_idx = orig_send_idx + r;

      // then its full connectivity..
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, rgp->, ru_itr) {
	FOR_ITR_EL(Unit, su, sgp->, su_itr) {
	  if(self_con || (ru != su))
	    ru->ConnectFrom(su, prjn);
	}  
      }
    }
  }

  prjn->recv_idx = orig_recv_idx;
  prjn->send_idx = orig_send_idx;
}


/////////////////////////////
//	GpRndTessel	   //
/////////////////////////////

void GpTessEl::Initialize() {
  p_con = .25f;
}

void GpTessEl::InitLinks() {
  taOBase::InitLinks();
  taBase::Own(send_gp_off, this);
}

void GpTessEl::Copy_(const GpTessEl& cp) {
  send_gp_off = cp.send_gp_off;
  p_con = cp.p_con;
}


void GpRndTesselPrjnSpec::Initialize() {
  recv_gp_n = -1;
  recv_gp_skip = 1;
  recv_gp_group = 1;
  send_gp_scale = 1.0f;
  send_gp_offs.SetBaseType(&TA_GpTessEl);

  wrap = true;
  def_p_con = .25f;
  permute = true;
  sym_self = true;
  same_seed = false;
  rndm_seed.GetCurrent();
}

void GpRndTesselPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(recv_gp_off, this);
  taBase::Own(recv_gp_n, this);
  taBase::Own(recv_gp_skip, this);
  taBase::Own(recv_gp_group, this);
  taBase::Own(send_gp_scale, this);
  taBase::Own(send_gp_border, this);
  taBase::Own(send_gp_offs, this);
  taBase::Own(rndm_seed, this);
}

void GpRndTesselPrjnSpec::UpdateAfterEdit() {
  ProjectionSpec::UpdateAfterEdit();
  recv_gp_skip.SetGtEq(1);
  recv_gp_group.SetGtEq(1);
}

void GpRndTesselPrjnSpec::MakeEllipse(int half_width, int half_height, int ctr_x, int ctr_y) {
  send_gp_offs.Reset();
  int strt_x = ctr_x - half_width;
  int end_x = ctr_x + half_width;
  int strt_y = ctr_y - half_height;
  int end_y = ctr_y + half_height;
  if(half_width == half_height) { // circle
    int y;
    for(y = strt_y; y <= end_y; y++) {
      int x;
      for(x = strt_x; x <= end_x; x++) {
	int dist = ((x - ctr_x) * (x - ctr_x)) + ((y - ctr_y) * (y - ctr_y));
	if(dist > (half_width * half_width))
	  continue;		// outside the circle
	GpTessEl* te = (GpTessEl*)send_gp_offs.New(1, &TA_GpTessEl);
	te->send_gp_off.x = x;
	te->send_gp_off.y = y;
	te->p_con = def_p_con;
      }
    }
  }
  else {			// ellipse
    float f1_x, f1_y;		// foci
    float f2_x, f2_y;
    float two_a;			// two times largest axis
    
    if(half_width > half_height) {
      two_a = (float)half_width * 2;
      float c = sqrtf((float)(half_width * half_width) - (float)(half_height * half_height));
      f1_x = (float)ctr_x - c;
      f1_y = (float)ctr_y;
      f2_x = (float)ctr_x + c;
      f2_y = (float)ctr_y;
    }
    else {
      two_a = (float)half_height * 2;
      float c = sqrtf((float)(half_height * half_height) - (float)(half_width * half_width));
      f1_x = (float)ctr_x;
      f1_y = (float)ctr_y - c;
      f2_x = (float)ctr_x;
      f2_y = (float)ctr_y + c;
    }

    int y;
    for(y = strt_y; y <= end_y; y++) {
      int x;
      for(x = strt_x; x <= end_x; x++) {
	float dist = sqrtf((((float)x - f1_x) * ((float)x - f1_x)) + (((float)y - f1_y) * ((float)y - f1_y))) +
	  sqrtf((((float)x - f2_x) * ((float)x - f2_x)) + (((float)y - f2_y) * ((float)y - f2_y)));
	if(dist > two_a)
	  continue;
	GpTessEl* te = (GpTessEl*)send_gp_offs.New(1, &TA_GpTessEl);
	te->send_gp_off.x = x;
	te->send_gp_off.y = y;
	te->p_con = def_p_con;
      }
    }
  }
}

void GpRndTesselPrjnSpec::MakeRectangle(int width, int height, int left, int bottom) {
  send_gp_offs.Reset();
  int y;
  for(y = bottom; y < bottom + height; y++) {
    int x;
    for(x = left; x < left + width; x++) {
      GpTessEl* te = (GpTessEl*)send_gp_offs.New(1, &TA_GpTessEl);
      te->send_gp_off.x = x;
      te->send_gp_off.y = y;
      te->p_con = def_p_con;
    }
  }
}

void GpRndTesselPrjnSpec::SetPCon(float p_con, int start, int end) {
  if(end == -1)	end = send_gp_offs.size;  else end = MIN(send_gp_offs.size, end);
  int i;
  for(i=start;i<end;i++) {
    GpTessEl* te = (GpTessEl*)send_gp_offs.FastEl(i);
    te->p_con = p_con;
  }
}


void GpRndTesselPrjnSpec::GetCtrFmRecv(TwoDCoord& sctr, TwoDCoord ruc) {
  ruc -= recv_gp_off;
  ruc /= recv_gp_group;	ruc *= recv_gp_group;	// this takes int part of
  ruc += recv_gp_off;	// then re-add offset
  FloatTwoDCoord scruc = ruc;
  scruc *= send_gp_scale;
  sctr = scruc;		// center of sending units
  sctr += send_gp_border;
}

void GpRndTesselPrjnSpec::Connect_Gps(Unit_Group* ru_gp, Unit_Group* su_gp, float p_con, Projection* prjn) {
  if((ru_gp->leaves == 0) || (su_gp->leaves == 0)) return;

  if(p_con < 0) {		// this means: make symmetric connections!
    if((prjn->from != prjn->layer) || !sym_self)
      return;			// not applicable otherwise!

    Unit* ru;
    taLeafItr ru_itr;
    FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr) {
      int g;
      for(g=0;g<ru->send.gp.size;g++) {
	Con_Group* scg = (Con_Group*)ru->send.gp.FastEl(g);
	if((scg->prjn->layer != scg->prjn->from) || (scg->prjn->layer != prjn->layer))
	  continue;		// only deal with self projections to this same layer
	int i;
	for(i=0;i<scg->size;i++) {
	  Unit* su = scg->Un(i);
	  if(GET_OWNER(su, Unit_Group) == su_gp) { // this sender is in actual group I'm trying to connect
	    ru->ConnectFromCk(su, prjn);
	  }
	}
      }
    }
  }
  else if((ru_gp == su_gp) && sym_self) {
    // trick is to divide cons in half, choose recv, send at random
    // for 1/2 cons, then go through all units and make the symmetric cons..
    if(permute) {
      // pre-allocate connections!
      if(p_con > .95f) {
	taMisc::Error("Warning: GpRndTesselPrjnSpec makes less than complete connectivity for high values of p_con in symmetric, self-connected layers using permute!");
      }
      int first;
      if(!self_con)
	first = (int) (.5f * p_con * (float)(su_gp->leaves-1));
      else
	first = (int) (.5f * p_con * (float)su_gp->leaves);
      if(first <= 0) first = 1;

      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      taPtrList<Unit> ru_list;		// receiver permution list
      taPtrList<Unit> perm_list;	// sender permution list

      FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr)	// need to permute recvs because of exclusion
	ru_list.Link(ru);			// on making a symmetric connection in first pass
      ru_list.Permute();
      
      int i;
      for(i=0;i<ru_list.size; i++) {
	ru = ru_list.FastEl(i);
	perm_list.Reset();
	FOR_ITR_EL(Unit, su, su_gp->, su_itr) {
	  if(!self_con && (ru == su)) continue;
	  // don't connect to anyone who already recvs from me cuz that will make
	  // a symmetric connection which isn't good: symmetry will be enforced later
	  Con_Group* scg = su->recv.FindPrjn(prjn);
	  if(scg->units.Find(ru) >= 0) continue; 
	  perm_list.Link(su);
	}
	perm_list.Permute();
	int j;
	for(j=0; j<first && j<perm_list.size; j++)	// only connect 1/2 of the units
	  ru->ConnectFromCk((Unit*)perm_list[j], prjn);
      }
    }
    else {
      int first;
      if(!self_con)
	first = (int) (.5f * p_con * (float)(su_gp->leaves-1) * (float)ru_gp->leaves);
      else
	first = (int) (.5f * p_con * (float)su_gp->leaves * (float)ru_gp->leaves);
      if(first <= 0) first = 1;

      int cnt = 0;
      while (cnt < first) {
	int ridx = Random::IntZeroN(ru_gp->leaves);
	int sidx = Random::IntZeroN(su_gp->leaves);
	if(!self_con && (ridx == sidx)) continue;
	Unit* ru = ru_gp->Leaf(ridx);
	Unit* su = su_gp->Leaf(sidx);
	// don't connect to anyone who already recvs from me cuz that will make
	// a symmetric connection which isn't good: symmetry will be enforced later
	Con_Group* scg = su->recv.FindPrjn(prjn);
	if(scg->units.Find(ru) >= 0) continue; 
	if(ru->ConnectFromCk(su, prjn) != NULL)
	  cnt++;
      }
    }
    // now go thru and make the symmetric connections

    Unit* ru;
    taLeafItr ru_itr;
    FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr) {
      Con_Group* scg = ru->send.FindPrjn(prjn);
      if(scg == NULL) continue;
      int i;
      for(i=0;i<scg->size;i++) {
	Unit* su = scg->Un(i);
	ru->ConnectFromCk(su, prjn);
      }
    }
  }
  else {			// not within same con group
    if((prjn->from == prjn->layer) && sym_self) {
      // within the same layer, i want to make connections symmetric: either i'm the
      // first to connect to other group, or other group has already connected to me
      // so I should just make symmetric versions of its connections
      // take first send unit and find if it recvs from anyone in this prjn yet
      Unit* su = (Unit*)su_gp->Leaf(0);
      Con_Group* scg = su->recv.FindPrjn(prjn);
      if((scg != NULL) && (scg->size > 0)) {	// sender has been connected already: try to connect me!
	int n_con = 0;		// number of actual connections made

	Unit* ru;
	taLeafItr ru_itr;
	FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr) {
	  Con_Group* scg = ru->send.FindPrjn(prjn);
	  if(scg == NULL) continue;
	  int i;
	  for(i=0;i<scg->size;i++) {
	    Unit* su = scg->Un(i);
	    if(GET_OWNER(su, Unit_Group) == su_gp) { // this sender is in actual group I'm trying to connect
	      if(ru->ConnectFromCk(su, prjn) != NULL)
		n_con++;
	    }
	  }
	}
	if(n_con > 0)		// made some connections, bail
	  return;
	// otherwise, go ahead and make new connections!
      }
    }

    int no;
    if(!self_con && (ru_gp == su_gp))
      no = (int) (p_con * (float)(su_gp->leaves-1));
    else
      no = (int) (p_con * (float)su_gp->leaves);
    if(no <= 0)  no = 1;

    if(!permute) {
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr) {
	FOR_ITR_EL(Unit, su, su_gp->, su_itr) {
	  if(!self_con && (ru == su)) continue;
	  if(Random::ZeroOne() <= p_con)
	    ru->ConnectFrom(su, prjn);
	}  
      }
    }
    else {
      // pre-allocate connections!
      Unit* ru, *su;
      taLeafItr ru_itr, su_itr;
      FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr)
	ru->ConnectAlloc(no, prjn);

      taPtrList<Unit> perm_list;	// permution list
      FOR_ITR_EL(Unit, ru, ru_gp->, ru_itr) {
	perm_list.Reset();
	FOR_ITR_EL(Unit, su, su_gp->, su_itr) {
	  if(!self_con && (ru == su)) continue;
	  perm_list.Link(su);
	}
	perm_list.Permute();
	int i;
	for(i=0; i<no; i++)
	  ru->ConnectFrom((Unit*)perm_list[i], prjn);
      }
    }
  }
}

void GpRndTesselPrjnSpec::Connect_RecvGp(Unit_Group* ru_gp, const TwoDCoord& ruc, Projection* prjn) {
  TDCoord& su_geo = prjn->from->gp_geom;
  // positions of center of recv in sending layer
  TwoDCoord sctr;
  GetCtrFmRecv(sctr, ruc);
  int i;
  GpTessEl* te;
  for(i = 0; i< send_gp_offs.size; i++) {
    te = (GpTessEl*)send_gp_offs.FastEl(i);
    TwoDCoord suc = te->send_gp_off + sctr;
    if(suc.WrapClip(wrap, su_geo))
      continue;
    Unit_Group* su_gp = prjn->from->FindUnitGpFmCoord(suc);
    if(su_gp == NULL) continue;
    Connect_Gps(ru_gp, su_gp, te->p_con, prjn);
  }
}

void GpRndTesselPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(same_seed)
    rndm_seed.OldSeed();

  if(prjn->layer->units.leaves == 0) // an empty layer!
    return;

  if(prjn->layer->units.gp.size == 0) {
    taMisc::Error("*** Warning: GpRndTesselPrjnSpec requires recv layer to have unit groups!:",name);
    return;
  }
  if(prjn->from->units.gp.size == 0) {
    taMisc::Error("*** Warning: GpRndTesselPrjnSpec requires send layer to have unit groups!:",name);
    return;
  }

  TDCoord& ru_geo = prjn->layer->gp_geom;
  TwoDCoord use_recv_gp_n = recv_gp_n;

  if(recv_gp_n.x == -1)
    use_recv_gp_n.x = ru_geo.x;
  if(recv_gp_n.y == -1)
    use_recv_gp_n.y = ru_geo.y;

  TwoDCoord ruc, nuc;
  for(ruc.y = recv_gp_off.y, nuc.y = 0; (ruc.y < ru_geo.y) && (nuc.y < use_recv_gp_n.y);
      ruc.y += recv_gp_skip.y, nuc.y++)
  {
    for(ruc.x = recv_gp_off.x, nuc.x = 0; (ruc.x < ru_geo.x) && (nuc.x < use_recv_gp_n.x);
	ruc.x += recv_gp_skip.x, nuc.x++)
    {
      Unit_Group* ru_gp = prjn->layer->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;
      Connect_RecvGp(ru_gp, ruc, prjn);
    }
  }
}

///////////////////////////////////////////////////////
//		TiledRFPrjnSpec
///////////////////////////////////////////////////////

void TiledRFPrjnSpec::Initialize() {
  recv_gp_border = 0;
  recv_gp_ex_st = -1;
  recv_gp_ex_n = 0;
  send_border = 0;
  send_adj_rfsz = 0;
  send_adj_sndloc = 0;
}

void TiledRFPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(recv_gp_border, this);
  taBase::Own(recv_gp_ex_st, this);
  taBase::Own(recv_gp_ex_n, this);
  taBase::Own(send_border, this);
  taBase::Own(send_adj_rfsz, this);
  taBase::Own(send_adj_sndloc, this);

  taBase::Own(ru_geo, this);
  taBase::Own(recv_gp_ed, this);
  taBase::Own(recv_gp_ex_ed, this);
  taBase::Own(su_act_geom, this);
  taBase::Own(n_recv_gps, this);
  taBase::Own(n_send_units, this);
  taBase::Own(rf_ovlp, this);
  taBase::Own(rf_move, this);
  taBase::Own(rf_width, this);
}

bool TiledRFPrjnSpec::InitRFSizes(Projection* prjn) {
  if(prjn->from == NULL)	return false;
  if(prjn->layer->units.leaves == 0) // an empty layer!
    return false;
  if(prjn->layer->units.gp.size == 0) {
    taMisc::Error("*** Warning: TiledRFPrjnSpec requires recv layer to have unit groups!:",name);
    return false;
  }

  ru_geo = prjn->layer->gp_geom;
  recv_gp_ed = ru_geo - recv_gp_border;
  recv_gp_ex_ed = recv_gp_ex_st + recv_gp_ex_n;

  prjn->from->GetActGeomNoSpc(su_act_geom);

// 0 1 2 3 4 5 6 7 8 9 a b c d e = 14+1 = 15
// 0 0 0 0 0 0
//       1 1 1 1 1 1
//             2 2 2 2 2 2
//                   3 3 3 3 3 3
//
// 4 gps, ovlp = 3, width = 6 
// ovlp = tot / (nr + 1) = 15 / 5 = 3

// send scale:
// ovlp / recv_gp size 

// send off:
// - recv off * scale

  n_recv_gps = ru_geo - (2 * recv_gp_border);	// total number of recv groups covered
  n_send_units = TwoDCoord(su_act_geom) - (2 * send_border);

  rf_ovlp.x = (int)floor(((float)(n_send_units.x + send_adj_rfsz.x) / (float)(n_recv_gps.x + 1)) + .5f);
  rf_ovlp.y = (int)floor(((float)(n_send_units.y + send_adj_rfsz.y) / (float)(n_recv_gps.y + 1)) + .5f);

  // how to move the receptive fields over the sending layer (floating point)
  rf_move = FloatTwoDCoord(n_send_units + send_adj_sndloc) / FloatTwoDCoord(n_recv_gps + 1);

  rf_width = rf_ovlp * 2;
//   cerr << "prjn: " << name << " layer: " << prjn->layer->name << " from: " << prjn->from->name
//        << " rf size: " << rf_ovlp.x << ", " << rf_ovlp.y
//        << " act send size: " << rf_ovlp.x * (n_recv_gps.x + 1)
//        << ", " << rf_ovlp.y * (n_recv_gps.y + 1)
//        << " trg send size: " << n_send_units.x << ", " << n_send_units.y
//        << endl;
  return true;
}

void TiledRFPrjnSpec::Connect_impl(Projection* prjn) {
  if(!InitRFSizes(prjn)) return;
  TwoDCoord ruc;
  for(ruc.y = recv_gp_border.y; ruc.y < recv_gp_ed.y; ruc.y++) {
    for(ruc.x = recv_gp_border.x; ruc.x < recv_gp_ed.x; ruc.x++) {
      if((ruc.y >= recv_gp_ex_st.y) && (ruc.y < recv_gp_ex_ed.y) &&
	 (ruc.x >= recv_gp_ex_st.x) && (ruc.x < recv_gp_ex_ed.x)) continue;
      Unit_Group* ru_gp = prjn->layer->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;

      TwoDCoord su_st;
      su_st.x = send_border.x + (int)floor((float)(ruc.x - recv_gp_border.x) * rf_move.x);
      su_st.y = send_border.y + (int)floor((float)(ruc.y - recv_gp_border.y) * rf_move.y);

      TwoDCoord suc;
      for(suc.y = su_st.y; suc.y < su_st.y + rf_width.y; suc.y++) {
	for(suc.x = su_st.x; suc.x < su_st.x + rf_width.x; suc.x++) {
	  Unit* su_u = prjn->from->FindUnitFmCoord(suc);
	  if(su_u == NULL) continue;

	  for(int rui=0;rui<ru_gp->size;rui++) {
	    Unit* ru_u = (Unit*)ru_gp->FastEl(rui);
	    if(!self_con && (su_u == ru_u)) continue;

	    ru_u->ConnectFrom(su_u, prjn); // don't check: saves lots of time!
	  }
	}
      }
    }
  }
}

int TiledRFPrjnSpec::ProbAddCons(Projection* prjn, float p_add_con, float init_wt) {
  if(!InitRFSizes(prjn)) return 0;
  int rval = 0;

  int n_cons = rf_width.x * rf_width.y;
  int n_new_cons = (int)(p_add_con * (float)n_cons);
  if(n_new_cons <= 0) return 0;
  int_Array new_idxs;
  new_idxs.EnforceSize(n_cons);
  new_idxs.FillSeq();

  TwoDCoord ruc;
  for(ruc.y = recv_gp_border.y; ruc.y < recv_gp_ed.y; ruc.y++) {
    for(ruc.x = recv_gp_border.x; ruc.x < recv_gp_ed.x; ruc.x++) {
      if((ruc.y >= recv_gp_ex_st.y) && (ruc.y < recv_gp_ex_ed.y) &&
	 (ruc.x >= recv_gp_ex_st.x) && (ruc.x < recv_gp_ex_ed.x)) continue;
      Unit_Group* ru_gp = prjn->layer->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;

      TwoDCoord su_st;
      su_st.x = send_border.x + (int)floor((float)(ruc.x - recv_gp_border.x) * rf_move.x);
      su_st.y = send_border.y + (int)floor((float)(ruc.y - recv_gp_border.y) * rf_move.y);

      for(int rui=0;rui<ru_gp->size;rui++) {
	Unit* ru_u = (Unit*)ru_gp->FastEl(rui);

	new_idxs.Permute();

	for(int i=0;i<n_new_cons;i++) {
	  int su_idx = new_idxs[i];
	  TwoDCoord suc;
	  suc.y = su_idx / rf_width.x;
	  suc.x = su_idx % rf_width.x;
	  suc += su_st;
	  Unit* su_u = prjn->from->FindUnitFmCoord(suc);
	  if(su_u == NULL) continue;
	  if(!self_con && (su_u == ru_u)) continue;
	  Connection* cn = ru_u->ConnectFromCk(su_u, prjn); // gotta check!
	  if(cn != NULL) {
	    cn->wt = init_wt;
	    rval++;
	  }
	}
      }
    }
  }
  return rval;
}

void TiledRFPrjnSpec::SelectRF(Projection* prjn) {
  if(!InitRFSizes(prjn)) return;

  Network* net = prjn->layer->own_net;
  if(net == NULL) return;
  NetView* nv = (NetView*)net->views.DefaultEl();
  if(nv == NULL) return;

  taBase_List* selgp = nv->GetSelectGroup();
  selgp->Reset();

  TwoDCoord ruc;
  for(ruc.y = recv_gp_border.y; ruc.y < recv_gp_ed.y; ruc.y++) {
    for(ruc.x = recv_gp_border.x; ruc.x < recv_gp_ed.x; ruc.x++) {
      if((ruc.y >= recv_gp_ex_st.y) && (ruc.y < recv_gp_ex_ed.y) &&
	 (ruc.x >= recv_gp_ex_st.x) && (ruc.x < recv_gp_ex_ed.x)) continue;
      Unit_Group* ru_gp = prjn->layer->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;

      selgp->LinkUnique(ru_gp);

      TwoDCoord su_st;
      su_st.x = send_border.x + (int)floor((float)(ruc.x - recv_gp_border.x) * rf_move.x);
      su_st.y = send_border.y + (int)floor((float)(ruc.y - recv_gp_border.y) * rf_move.y);

      TwoDCoord suc;
      for(suc.y = su_st.y; suc.y < su_st.y + rf_width.y; suc.y++) {
	for(suc.x = su_st.x; suc.x < su_st.x + rf_width.x; suc.x++) {
	  Unit* su_u = prjn->from->FindUnitFmCoord(suc);
	  if(su_u == NULL) continue;

	  selgp->LinkUnique(su_u);
	}
      }
    }
  }
  nv->UpdateSelect();
}

//////////////////////////////////////////////////////////
//  	TiledGpRFPrjnSpec

void TiledGpRFPrjnSpec::Initialize() {
  send_gp_size = 4;
  send_gp_skip = 2;
  reciprocal = false;
}

void TiledGpRFPrjnSpec::InitLinks() {
  ProjectionSpec::InitLinks();
  taBase::Own(send_gp_size, this);
  taBase::Own(send_gp_skip, this);
}

void TiledGpRFPrjnSpec::Connect_impl(Projection* prjn) {
  if(prjn->from == NULL)	return;
  if(prjn->layer->units.leaves == 0) // an empty layer!
    return;
  if(prjn->layer->units.gp.size == 0) {
    taMisc::Error("*** Warning: TiledGpRFPrjnSpec requires recv layer to have unit groups!:",name);
    return;
  }
  if(prjn->from->units.gp.size == 0) {
    taMisc::Error("*** Warning: TiledGpRFPrjnSpec requires send layer to have unit groups!:",name);
    return;
  }
  
  Layer* recv_lay = prjn->layer;
  Layer* send_lay = prjn->from;
  if(reciprocal) {
    recv_lay = prjn->from;
    send_lay = prjn->layer;
  }

  TwoDCoord ru_geo = recv_lay->gp_geom;
  TwoDCoord su_geo = send_lay->gp_geom;

  TwoDCoord ruc;
  for(ruc.y = 0; ruc.y < ru_geo.y; ruc.y++) {
    for(ruc.x = 0; ruc.x < ru_geo.x; ruc.x++) {
      Unit_Group* ru_gp = recv_lay->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;

      TwoDCoord su_st = ruc * send_gp_skip;

      TwoDCoord suc;
      for(suc.y = su_st.y; suc.y < su_st.y + send_gp_size.y; suc.y++) {
	for(suc.x = su_st.x; suc.x < su_st.x + send_gp_size.x; suc.x++) {
	  Unit_Group* su_gp = send_lay->FindUnitGpFmCoord(suc);
	  if(su_gp == NULL) continue;

	  for(int rui=0;rui<ru_gp->size;rui++) {
	    for(int sui=0;sui<su_gp->size;sui++) {
	      Unit* ru_u = (Unit*)ru_gp->FastEl(rui);
	      Unit* su_u = (Unit*)su_gp->FastEl(sui);
	      if(!self_con && (su_u == ru_u)) continue;

	      if(!reciprocal)
		ru_u->ConnectFrom(su_u, prjn);
	      else
		su_u->ConnectFrom(ru_u, prjn);
	    }
	  }
	}
      }
    }
  }
}

int TiledGpRFPrjnSpec::ProbAddCons(Projection* prjn, float p_add_con, float init_wt) {
  if(prjn->from == NULL)	return 0;
  if(prjn->layer->units.leaves == 0) // an empty layer!
    return 0;
  if(prjn->layer->units.gp.size == 0) {
    taMisc::Error("*** Warning: TiledGpRFPrjnSpec requires recv layer to have unit groups!:",name);
    return 0;
  }
  if(prjn->from->units.gp.size == 0) {
    taMisc::Error("*** Warning: TiledGpRFPrjnSpec requires send layer to have unit groups!:",name);
    return 0;
  }
  
  int rval = 0;

  Layer* recv_lay = prjn->layer;
  Layer* send_lay = prjn->from;
  if(reciprocal) {
    recv_lay = prjn->from;
    send_lay = prjn->layer;
  }

  TwoDCoord ru_geo = recv_lay->gp_geom;
  TwoDCoord su_geo = send_lay->gp_geom;

  TwoDCoord ruc;
  for(ruc.y = 0; ruc.y < ru_geo.y; ruc.y++) {
    for(ruc.x = 0; ruc.x < ru_geo.x; ruc.x++) {
      Unit_Group* ru_gp = recv_lay->FindUnitGpFmCoord(ruc);
      if(ru_gp == NULL) continue;

      TwoDCoord su_st = ruc * send_gp_skip;

      TwoDCoord suc;
      for(suc.y = su_st.y; suc.y < su_st.y + send_gp_size.y; suc.y++) {
	for(suc.x = su_st.x; suc.x < su_st.x + send_gp_size.x; suc.x++) {
	  Unit_Group* su_gp = send_lay->FindUnitGpFmCoord(suc);
	  if(su_gp == NULL) continue;

	  for(int rui=0;rui<ru_gp->size;rui++) {
	    for(int sui=0;sui<su_gp->size;sui++) {
	      Unit* ru_u = (Unit*)ru_gp->FastEl(rui);
	      Unit* su_u = (Unit*)su_gp->FastEl(sui);
	      if(!self_con && (su_u == ru_u)) continue;

	      // just do a basic probabilistic version: too hard to permute..
	      if(Random::ZeroOne() > p_add_con) continue; // no-go

	      Connection* cn;
	      if(!reciprocal)
		cn = ru_u->ConnectFromCk(su_u, prjn); // gotta check!
	      else
		cn = su_u->ConnectFromCk(ru_u, prjn);
	      if(cn != NULL) {
		cn->wt = init_wt;
		rval++;
	      }
	    }
	  }
	}
      }
    }
  }
  return rval;
}

