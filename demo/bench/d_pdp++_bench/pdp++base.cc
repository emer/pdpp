/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

#ifdef __GNUG__
#pragma implementation
#endif

#include <mpi.h>

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
  *(u->act) = 0.0f;
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
    *(u->act) = u->ext;
  else
    *(u->act) = u->net;
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


//////////////////////////
//	Unit - dPDP     //
//////////////////////////

int Unit::total_unit_count = 0;         // dPDP - Total # of units
Unit *Unit::unit_llist = NULL;          // dPDP - Head of l-list representing all units on the
                                        //        (both shadow and local) on the current processor
int Unit::proc_rank = -1;               // dPDP - the MPI rank of the current processor
int Unit::comm_size = -1;               // dPDP - the size of the current MPI Communicator
float *Unit::actArray = NULL;           // dPDP - unit activations for all units  (both shadow & local)

// dPDP - divide the units as equally as possible between all of the
//         processors in the current communicator.
// NOTE: THIS PARITIONING IS OPTIMAL IF ALL PROCESSORS IN THE COMMUNICATOR ARE
// APPROXIMATELY EQUAL IN COMPUTATIONAL SPEED AND ARE OTHERWISE UNDER SIMILAR
// LOADS. Optimal partitioning under other conditions would require a moderately
// more sophisticated algorithm.
int Unit::PartitionUnits(int &first_id, int &last_id, int proc_rank) {
   if ((proc_rank < 0) || (comm_size < 0) || (total_unit_count < 1)) {
          first_id = -1;
          last_id = -1;
	  return 0;
   }

   int n_range   = total_unit_count/comm_size;
   int n_range_r = total_unit_count%comm_size;
   if (proc_rank < n_range_r) {
       first_id =  proc_rank*n_range + proc_rank;
       last_id = first_id + n_range;
   } else {
       first_id = proc_rank*n_range + n_range_r;
       last_id = first_id + n_range-1;
   }
   return 1;
}

// dPDP - maintains unit ids and unit l-list
Unit::Unit() {
   global_id = total_unit_count++;   // sets this object's ID & maintains total_unit_count
   llist_next = unit_llist;               // maintains unit l-list
   unit_llist = this;
}

// dPDP - labels local units & allocates blocks of memory used to store
//        unit activations
void Unit::DistributeUnits(int arg_proc_rank, int arg_comm_size){
   int first_id;  // id of the first unit assigned to this processor
   int last_id;   // id of the last unit assigned to this processor

   proc_rank = arg_proc_rank;
   comm_size = arg_comm_size;

   PartitionUnits(first_id, last_id, proc_rank);
   actArray = new float[total_unit_count];
   Unit *u = unit_llist;
   while (u) {
      if ((u->global_id >= first_id) &&
          (u->global_id <= last_id)) {
         u->local = true;
      } else {
         u->local = false;
      }
      u->act = &actArray[u->global_id];
      u = u->llist_next;
   }
}


// dPDP - Given a list containing both local and shadow units, this function
//        returns the subset of the original list consisting of just local units
taGroup_impl* Unit::ExtractLocalFromList(taGroup_impl *global_list) {
    taLeafItr leaf_ittr;
    Unit* u;
    taGroup_impl *local_units = new taGroup_impl;
    FOR_ITR_EL(Unit, u, global_list->, leaf_ittr) {
       if (u->local) {
          local_units->Add_(u);
       }
    }
    return local_units;
}



// dPDP - Synchronizes across all processors all shadow and local units
void Unit::DistributedSync() {
    int first_id, last_id;

    if (comm_size > 2)
    // Note: there are a few other ways of doing this. It's probably worth experimenting
    //       around to see what's generally the fastest.
    for (int rank = 0; rank < comm_size; rank++) {
        PartitionUnits(first_id, last_id, rank);
         MPI_Bcast(&actArray[first_id], first_id-last_id+1, MPI_FLOAT, rank, MPI_COMM_WORLD);
    }

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
Layer *Layer::layer_llist = NULL;   // dPDP - head or layer l-list

Layer::Layer() {
  lesion = false; ext_flag = Unit::NO_EXTERNAL;
  llist_next = layer_llist;                     // dPDP - maintains layer l-list
  layer_llist = this;
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
  FOR_ITR_EL(Unit, u, local_units->, i)
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
  FOR_ITR_EL(Unit, u, local_units->, i)         // dPDP - now, this itterates over the local_units list
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
  FOR_ITR_EL(Unit, u, local_units->, i)        // dPDP - now, this itterates over the local_units list
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
  FOR_ITR_EL(Unit, u, local_units->, i)        // dPDP - now, this itterates over the local_units list
    u->Compute_Act();
}
void Layer::UpdateWeights() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, local_units->, i)       // dPDP - only update Wt values for weights that local units
    u->UpdateWeights();                       //        receive from
}
void Layer::Compute_dWt() {
  Unit* u;
  taLeafItr i;
  FOR_ITR_EL(Unit, u, local_units->, i)      // dPDP - only calculate dWt values for weights that local units
    u->Compute_dWt();                        //        receive from
}


// dPDP - initialize local_units in ALL layer objects
void Layer::InitLocalUnitGroups() {
   Layer *l = layer_llist;
   while(l) {
      l->local_units = Unit::ExtractLocalFromList(&(l->units));
      l = l->llist_next;
   }
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

