/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

#ifdef __GNUG__
#pragma implementation
#endif

#include "pdp++base.h"

#define TA_ALLOC_OVERHEAD 2

void taPtrList_impl::Alloc(int sz) {
  if(alloc_size >= sz)	return;	// no need to increase..
  sz = MAX(16-TA_ALLOC_OVERHEAD-1,sz);		// once allocating, use a minimum of 16
  alloc_size += TA_ALLOC_OVERHEAD; // increment to full power of 2
  while((alloc_size-TA_ALLOC_OVERHEAD) <= sz) alloc_size <<= 1;
  alloc_size -= TA_ALLOC_OVERHEAD;
  if(el == NULL)
    el = (void**)malloc(alloc_size * sizeof(void*));
  else
    el = (void**)realloc((char *) el, alloc_size * sizeof(void*));
}

void taPtrList_impl::Add_(void* it) {
  if(size+1 >= alloc_size)
    Alloc(size+1);
  el[size++] = it;
}

int taPtrList_impl::Find(void* it) const {
  int i;
  for(i=0; i < size; i++) {
    if(el[i] == it)
      return i;
  }
  return -1;
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
    bias_spec->C_InitWtDelta(u->bias, u, NULL); // this is a virtual fun
}

void UnitSpec::InitWtState(Unit* u) {
  Con_Group* recv_gp;
  int g;
  FOR_ITR_GP(Con_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtState(u);
  }
  if(u->bias != NULL)
    bias_spec->C_InitWtState(u->bias, u, NULL); // this is a virtual fun
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
  Con_Group* send_gp;
  int g;
  FOR_ITR_GP(Con_Group, send_gp, u->send., g) {
    if(!send_gp->prjn->layer->lesion)
      send_gp->Send_Net(u);
  }
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

Projection::Projection() {
  layer = from = NULL;
  from_type = PREV;
  con_type = NULL;
  con_gp_type = NULL;
  recv_idx = -1;
  send_idx = -1;
  recv_n = 1;
  send_n = 1;
  projected = false;
}

//////////////////////////
//	Layer		// 
//////////////////////////

Layer::Layer() {
  lesion = false; ext_flag = Unit::NO_EXTERNAL;
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

void Layer::ModifyState() {
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
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Compute_Net();
}

void Layer::Send_Net() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, units., i)
    u->Send_Net();
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

//////////////////////////
//	Network		// 
//////////////////////////

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

void Network::InitWtState(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState();
  }
  InitWtState_post();		// done after all initialization (for scaling wts...)
  InitState();			// also re-init state at this point..
  epoch = 0;			// re-init epoch counter..
  re_init = false;		// no need to re-reinit!
}

void Network::InitWtState_post() {
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState_post();
  }
}

void Network::Compute_Net(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Net();
  }
}

void Network::Compute_Act(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Act();
  }
}
void Network::UpdateWeights(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->UpdateWeights();
  }
}
void Network::Compute_dWt(){
  Layer* l;
  taLeafItr i;
  FOR_ITR_EL(Layer, l, layers., i) {
    if(!l->lesion)
      l->Compute_dWt();
  }
}

