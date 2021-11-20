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

// ta_group.cc

#include <ta/ta_group.h>
#include <sstream>

#ifndef NO_IV
#include <ta/ta_group_iv.h>
#endif


////////////////////////////
//      taGroup	  	  //
////////////////////////////

// not only sub-groups cause dirtiness, because if you add an el to a
// previously null group, it needs added, etc..

void taSubGroup::Dirty() {
  if(owner != NULL) ((TAGPtr)owner)->Dirty();
}

bool taSubGroup::Transfer(taBase* it) {
  // need to leaf count on parent group
  TAGPtr myown = (TAGPtr)owner;
  taGroup_impl* git = (taGroup_impl*)it;
  if((git->super_gp == myown) || (git->super_gp == NULL))
    return false;
  taGroup_impl* old_own = git->super_gp;
  bool rval = TALOG::Transfer(git);
  if(rval) {
    old_own->UpdateLeafCount_(-git->leaves);
    old_own->Dirty();
    if(myown != NULL) {
      myown->UpdateLeafCount_(git->leaves);
      myown->Dirty();
    }
  }
  return rval;
}

void taGroup_impl::Destroy() {
  if(leaf_gp != NULL) {
    taBase::unRefDone(leaf_gp);
    leaf_gp = NULL;
  }
  RemoveAll();
}

void taGroup_impl::Dirty() {
  if(leaf_gp != NULL) {
    taBase::unRefDone(leaf_gp);
    leaf_gp = NULL;
  }
  if(super_gp != NULL)
    super_gp->Dirty();
}

void taGroup_impl::InitLeafGp() const {
  if(leaf_gp != NULL)
    return;
  taGroup_impl* ncths = (taGroup_impl*)this;
  ncths->leaf_gp = new TALOG;
  taBase::Own(leaf_gp, ncths);
  InitLeafGp_impl(ncths->leaf_gp);
}

void taGroup_impl::InitLeafGp_impl(TALOG* lg) const {
  if(size > 0)
    lg->Push((taGroup_impl*)this);
  int i;
  for(i=0; i<gp.size; i++)
    FastGp_(i)->InitLeafGp_impl(lg);
}

void taGroup_impl::Copy(const taGroup_impl& cp) {
  taList_impl::Copy(cp);
  gp.Copy(cp.gp);
}

void taGroup_impl::Initialize() {
  leaves = 0;
  super_gp = NULL;
  leaf_gp = NULL;
}

void taGroup_impl::InitLinks() {
  taList_impl::InitLinks();
  gp.SetBaseType(GetTypeDef());	// more of the same type of group
  taBase::Own(gp, this);

  super_gp = GetSuperGp_();
  if(super_gp != NULL) {
    SetBaseType(super_gp->el_base);
    el_typ = super_gp->el_typ;
  }
}

bool taGroup_impl::Remove(int i) {
  if(taList_impl::Remove(i)) {
    UpdateLeafCount_(-1);
    return true;
  }
  return false;
}

void taGroup_impl::AddEl_(void* it) {
  taList_impl::AddEl_(it);
  UpdateLeafCount_(1);		// not the most efficient, but gets it at a low level
}
  
TAPtr taGroup_impl::New(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  if(typ == NULL)
    typ = el_typ;

  if(typ->InheritsFrom(TA_taGroup_impl)) {
    if(!typ->InheritsFrom(gp.el_typ))
      typ = GetTypeDef();	// always create one of yourself..
    TAPtr rval = gp.New(no, typ);
//    UpdateAfterEdit();
    return rval;
  }
  if(!typ->InheritsFrom(el_base)) {
    taMisc::Error("*** Attempt to create type:", typ->name,
		   "in list with base type:", el_base->name);
    return NULL;
  }
  TAPtr rval = taList_impl::New(no, typ);
  return rval;
}

TAGPtr taGroup_impl::NewGp_(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active) {
      gpivGroupNew::New(this,typ);
      return NULL;		// not sure if rval is a group or not
    }
#endif
    return NULL;
  }
  if(typ == NULL)
    typ = GetTypeDef();		// always create one of yourself..
  TAGPtr rval = (TAGPtr)gp.New(no, typ);
//  UpdateAfterEdit();
  return rval;
}

TAPtr taGroup_impl::NewEl_(int no, TypeDef* typ) {
  if(no == 0) {
#ifndef NO_IV
    if(taMisc::iv_active)
      return gpivGroupNew::New(this,typ);
#endif
    return NULL;
  }
  if(typ == NULL)
    typ = el_typ;
  TAPtr rval = taList_impl::New(no, typ);
  return rval;
}

MemberDef* taGroup_impl::FindMembeR(const char* nm, void*& ptr) const {
  String idx_str = nm;
  idx_str = idx_str.before(']');
  if(idx_str != "") {
    idx_str = idx_str.after('[');
    int idx = atoi(idx_str);
    if((size == 0) || (idx >= size)) {
      ptr = NULL;
      return NULL;
    }
    ptr = el[idx];
    return ReturnFindMd();
  }

  int i;
  if((i = FindLeaf(nm)) >= 0) {
    ptr = Leaf_(i);
    return ReturnFindMd();
  }

  MemberDef* rval;
  if((rval = GetTypeDef()->members.FindNameAddrR(nm, (void*)this, ptr)) != NULL)
    return rval;
  int max_srch = MIN(taMisc::search_depth, size);
  for(i=0; i<max_srch; i++) {
    TAPtr first_el = (TAPtr)FastEl_(i);
    if((first_el != NULL) && // only search owned objects
       ((first_el->GetOwner()==NULL) || (first_el->GetOwner() == (taBase *) this))) {
      return first_el->FindMembeR(nm, ptr);
    }
  }
  ptr = NULL;
  return NULL;
}

MemberDef* taGroup_impl::FindMembeR(TypeDef* it, void*& ptr) const {
  int i;
  if((i = FindLeaf(it)) >= 0) {
    ptr = Leaf_(i);
    return ReturnFindMd();
  }

  MemberDef* rval;
  if((rval = GetTypeDef()->members.FindTypeAddrR(it, (void*)this, ptr)) != NULL)
    return rval;
  int max_srch = MIN(taMisc::search_depth, size);
  for(i=0; i<max_srch; i++) {
    TAPtr first_el = (TAPtr)FastEl_(i);
    if((first_el != NULL) && // only search owned objects
       ((first_el->GetOwner()==NULL) || (first_el->GetOwner() == (taBase *) this))) {
      return first_el->FindMembeR(it, ptr);
    }
  }
  ptr = NULL;
  return NULL;
}

ostream& taGroup_impl::OutputR(ostream& strm, int indent) const {
  taMisc::indent(strm, indent) << name << "[" << size << "] = {\n";
  TypeDef* td = GetTypeDef();
  int i;
  for(i=0; i < td->members.size; i++) {
    MemberDef* md = td->members.FastEl(i);
    if(md->HasOption("EDIT_IN_GROUP"))
      md->Output(strm, (void*)this, indent+1);
  }

  for(i=0; i<size; i++) {
    if(el[i] == NULL)	continue;
    ((TAPtr)el[i])->OutputR(strm, indent+1);
  }

  gp.OutputR(strm, indent+1);

  taMisc::indent(strm, indent) << "}\n";
  return strm;
}

// save the path of all the elements in the group
int taGroup_impl::Dump_Save_PathR_impl(ostream& strm, TAPtr par, int indent) {
  int rval = taList_impl::Dump_Save_PathR_impl(strm, par, indent); // save first-level
  if(rval == false)
    rval = gp.Dump_Save_PathR_impl(strm, par, indent); 
  else
    gp.Dump_Save_PathR_impl(strm, par, indent);
  return rval;
}

int taGroup_impl::Dump_SaveR(ostream& strm, TAPtr par, int indent) {
  taList_impl::Dump_SaveR(strm, par, indent);
  gp.Dump_SaveR(strm, par, indent); // subgroups get saved
  return true;
}


TAGPtr taGroup_impl::GetSuperGp_() {
  if(owner == NULL)
    return NULL;
  if(owner->InheritsFrom(TA_taList)) {
    TAPtr ownr = owner->GetOwner();
    if((ownr != NULL) && (ownr->InheritsFrom(TA_taGroup_impl)))
      return (TAGPtr)ownr;
  }
  return NULL;
}

TAPtr taGroup_impl::Leaf_(int idx) const {
  if(idx >= leaves)
    return NULL;
  if(size && (idx < size))
    return (TAPtr)el[idx];

  int nw_idx = (int)idx - size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if(sbg->leaves && (sbg->leaves > nw_idx))
      return sbg->Leaf_(nw_idx);
    nw_idx -= (int)sbg->leaves;
  }
  return NULL;
}

TAGPtr taGroup_impl::LeafGp_(int idx) const {
  if(idx >= leaves)
    return NULL;
  if(size && (idx < size))
    return (TAGPtr)this;

  int nw_idx = (int)idx - size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if(sbg->leaves && (sbg->leaves > nw_idx))
      return sbg->LeafGp_(nw_idx);
    nw_idx -= (int)sbg->leaves;
  }
  return NULL;
}

void taGroup_impl::UpdateLeafCount_(int no) {
  leaves += no;
  if(super_gp != NULL)
    super_gp->UpdateLeafCount_(no);
}

bool taGroup_impl::RemoveLeaf(int idx) {
  if(idx >= leaves)
    return false;
  if(size && (idx < size))
    return Remove(idx);

  int nw_idx = (int)idx - size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if(sbg->leaves && (sbg->leaves > nw_idx))
      return sbg->RemoveLeaf(nw_idx);
    nw_idx -= (int)sbg->leaves;
  }
  return false;
}

bool taGroup_impl::RemoveLeaf(const char* it) {
  int i;
  if((i = FindLeaf(it)) < 0)
    return false;
  return RemoveLeaf(i);
}

bool taGroup_impl::RemoveLeaf(TAPtr it) {
  int i;
  if((i = FindLeaf(it)) < 0)
    return false;
  return RemoveLeaf(i);
}

void taGroup_impl::RemoveAll() {
  gp.RemoveAll();
  taList_impl::RemoveAll();
  leaves = 0;
}

void taGroup_impl::EnforceLeaves(int sz){
  if(sz > leaves)  New(sz - leaves,el_typ);
  while(leaves > sz) RemoveLeaf(leaves-1);
}

void taGroup_impl::EnforceSameStru(const taGroup_impl& cp) {
  taList_impl::EnforceSameStru(cp);
  gp.EnforceSameStru(cp.gp);
  int i;
  for(i=0; i<gp.size; i++) {
    FastGp_(i)->EnforceSameStru(*(cp.FastGp_(i)));
  }
}

int taGroup_impl::ReplaceType(TypeDef* old_type, TypeDef* new_type) {
  int nchanged = taList_impl::ReplaceType(old_type, new_type);
  nchanged += gp.ReplaceType(old_type, new_type);
  int i;
  for(i=0; i<gp.size; i++) {
    nchanged += FastGp_(i)->ReplaceType(old_type, new_type);
  }
  return nchanged;
}

int taGroup_impl::ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr) {
  int nchg = taList_impl::ReplaceAllPtrsThis(obj_typ, old_ptr, new_ptr);
  int i;
  for(i=0; i<gp.size; i++) {
    nchg += FastGp_(i)->ReplaceAllPtrsThis(obj_typ, old_ptr, new_ptr);
  }
  return nchg;
}

int taGroup_impl::FindLeaf(const char* nm) const {
  int idx;
  if((idx = Find(nm)) >= 0)
    return idx;

  int new_idx = size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if((idx = sbg->FindLeaf(nm)) >= 0)
      return idx + new_idx;
    new_idx += (int)sbg->leaves;
  }
  return -1;
}

int taGroup_impl::FindLeaf(TAPtr it) const {
  int idx;
  if((idx = Find(it)) >= 0)
    return idx;

  int new_idx = size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if((idx = sbg->FindLeaf(it)) >= 0)
      return idx + new_idx;
    new_idx += (int)sbg->leaves;
  }
  return -1;
}

int taGroup_impl::FindLeaf(TypeDef* it) const {
  int idx;
  if((idx = Find(it)) >= 0)
    return idx;

  int new_idx = size;
  int i;
  TAGPtr sbg;
  for(i=0; i<gp.size; i++) {
    sbg = FastGp_(i);
    if((idx = sbg->FindLeaf(it)) >= 0)
      return idx + new_idx;
    new_idx += (int)sbg->leaves;
  }
  return -1;
}

TAPtr taGroup_impl::FindLeafName_(const char* it, int& idx) const {
  idx = FindLeaf(it);
  if(idx >= 0) return Leaf_(idx);
  return NULL;
}

TAPtr taGroup_impl::FindLeafType_(TypeDef* it, int& idx) const {
  idx = FindLeaf(it);
  if(idx >= 0) return Leaf_(idx);
  return NULL;
}

void taGroup_impl::Duplicate(const taGroup_impl& cp) {
  taList_impl::Duplicate(cp);
  gp.Duplicate(cp.gp);
}
void taGroup_impl::DupeUniqNameOld(const taGroup_impl& cp) {
  taList_impl::DupeUniqNameOld(cp);
  gp.DupeUniqNameOld(cp.gp);
}
void taGroup_impl::DupeUniqNameNew(const taGroup_impl& cp) {
  taList_impl::DupeUniqNameNew(cp);
  gp.DupeUniqNameNew(cp.gp);
}
void taGroup_impl::Borrow(const taGroup_impl& cp) {
  taList_impl::Borrow(cp);
  gp.Borrow(cp.gp);
}
void taGroup_impl::BorrowUnique(const taGroup_impl& cp) {
  taList_impl::BorrowUnique(cp);
  gp.BorrowUnique(cp.gp);
}
void taGroup_impl::BorrowUniqNameOld(const taGroup_impl& cp) {
  taList_impl::BorrowUniqNameOld(cp);
  gp.BorrowUniqNameOld(cp.gp);
}
void taGroup_impl::BorrowUniqNameNew(const taGroup_impl& cp) {
  taList_impl::BorrowUniqNameNew(cp);
  gp.BorrowUniqNameNew(cp.gp);
}
void taGroup_impl::Copy_Common(const taGroup_impl& cp) {
  taList_impl::Copy_Common(cp);
  gp.Copy_Common(cp.gp);
}
void taGroup_impl::Copy_Duplicate(const taGroup_impl& cp) {
  taList_impl::Copy_Duplicate(cp);
  gp.Copy_Duplicate(cp.gp);
}
void taGroup_impl::Copy_Borrow(const taGroup_impl& cp) {
  taList_impl::Copy_Borrow(cp);
  gp.Copy_Borrow(cp.gp);
}
void taGroup_impl::List(ostream& strm) const {
  taList_impl::List(strm);
  int i;
  for(i=0; i<gp.size; i++)
    FastGp_(i)->List(strm);
}

//////////////////////////////////////////////////////////
// 			DMem Stuff			//
//////////////////////////////////////////////////////////

#ifdef DMEM_COMPILE

#include <mpi.h>

static String dmem_mpi_decode_err(int ercd) {
  char errstr[MPI_MAX_ERROR_STRING];
  int errlen;
  MPI_Error_string(ercd, errstr, &errlen);
  return String(ercd) + " msg: " + String(errstr);
}

void DMemShare::DebugCmd(const char* function, const char* mpi_call) {
//   String fn = function;
//   if(fn.contains("Symmetrize")) return;
//   if(fn.contains("Sync set") || fn.contains("Symmetrize")) return;
  if(taMisc::dmem_debug) {
    cerr << "proc: " << taMisc::dmem_proc << " fun: "
	 << function << " MPI_" << mpi_call
	 << " start..." << endl;
  }
}

bool DMemShare::ProcErr(int ercd, const char* function, const char* mpi_call) {
  if(ercd == MPI_SUCCESS) {
//     String fn = function;
//     if(fn.contains("Symmetrize")) return true;
//     if(fn.contains("Sync set") || fn.contains("Symmetrize")) return true;
    if(taMisc::dmem_debug) {
      cerr << "proc: " << taMisc::dmem_proc << " fun: "
	   << function << " MPI_" << mpi_call
	   << " SUCCESS!" << endl;
    }
    return true;
  }
  cerr << "proc: " << taMisc::dmem_proc << " fun: "
       << function << " MPI_" << mpi_call
       << " FAILED with code: " << dmem_mpi_decode_err(ercd) << endl;
  return false;
}

void DMemShareVar::Initialize() {
  comm = -1;
  mpi_type = -1;
  max_per_proc = -1;
  n_procs = -1;
  this_proc = -1;
}

void DMemShareVar::InitLinks() {
  taBase::InitLinks();
  taBase::Own(addrs, this);
  taBase::Own(local_proc, this);
  taBase::Own(n_local, this);
  taBase::Own(recv_idx, this);
  taBase::Own(addrs_recv, this);
  taBase::Own(float_send, this);
  taBase::Own(float_recv, this);
  taBase::Own(double_send, this);
  taBase::Own(double_recv, this);
  taBase::Own(int_send, this);
  taBase::Own(int_recv, this);
  taBase::Own(long_send, this);
  taBase::Own(long_recv, this);
}

void DMemShareVar::CutLinks() {
  ResetVar();
  taBase::CutLinks();
}

void DMemShareVar::Copy_(const DMemShareVar& cp) {
  // do we want to actually copy anything here?  ok.
  comm = cp.comm;
  mpi_type = cp.mpi_type;
  max_per_proc = cp.max_per_proc;
  n_procs = cp.n_procs;
  this_proc = cp.this_proc;

  addrs = cp.addrs;
  local_proc = cp.local_proc;
  n_local = cp.n_local;
  recv_idx = cp.recv_idx;
  addrs_recv = cp.addrs_recv;
}

void DMemShareVar::ResetVar() {
  comm = -1;
  mpi_type = -1;
  n_procs = -1;
  this_proc = -1;
  addrs.Reset();
  local_proc.Reset();
  n_local.Reset();
  recv_idx.Reset();
  addrs_recv.Reset();
  float_send.Reset();
  float_recv.Reset();
  double_send.Reset();
  double_recv.Reset();
  int_send.Reset();
  int_recv.Reset();
  long_send.Reset();
  long_recv.Reset();
}  

void DMemShareVar::Compile_Var(MPI_Comm cm) {
  comm = cm;
  n_procs = 0; MPI_Comm_size(comm, &n_procs);
  this_proc = 0; MPI_Comm_rank(comm, &this_proc);

  if(n_procs <= 1) return;

  n_local.EnforceSize(n_procs);
  
  // initialize counts
  for(int i=0;i<n_procs;i++) {
    n_local[i] = 0;
  }

  for(int i=0;i<local_proc.size;i++) {
    n_local[local_proc[i]]++;	// increment counts
  }

  // find max
  max_per_proc = 0;
  for(int i=0;i<n_procs;i++) {
    if(n_local[i] > max_per_proc) max_per_proc = n_local[i];
  }

  static int_Array proc_ctr;
  proc_ctr.EnforceSize(n_procs);
  recv_idx.EnforceSize(n_procs);
  for(int i=0;i<n_procs;i++) {
    proc_ctr[i] = 0;
    recv_idx[i] = max_per_proc * i;
  }

  // allocate the addrs_recv array (max_per_proc * nprocs; 000..111..222...)
  addrs_recv.EnforceSize(max_per_proc * n_procs);
  for(int i=0;i<local_proc.size;i++) {
    int lproc = local_proc[i];
    addrs_recv[recv_idx[lproc] + proc_ctr[lproc]] = addrs[i];
    proc_ctr[lproc]++;
  }

  switch(mpi_type) {
  case MPI_FLOAT: {
    float_send.EnforceSize(addrs_recv.size);
    float_recv.EnforceSize(addrs_recv.size);
    break;
  }
  case MPI_DOUBLE: {
    double_send.EnforceSize(addrs_recv.size);
    double_recv.EnforceSize(addrs_recv.size);
    break;
  }
  case MPI_INT: {
    int_send.EnforceSize(addrs_recv.size);
    int_recv.EnforceSize(addrs_recv.size);
    break;
  }
  case MPI_LONG: {
    long_send.EnforceSize(addrs_recv.size);
    long_recv.EnforceSize(addrs_recv.size);
    break;
  }
  }
}

void DMemShareVar::SyncVar() {
  // basic computation here is to send all of my stuff to all other nodes
  // and for them to send all of their stuff to me
  // 0: send: 0000
  // 1: send: 1111
  // 2: send: 2222
  // all recv: 0000 1111 2222

  if(n_procs <= 1) return;

  if((comm == -1) || (addrs_recv.size != n_procs * max_per_proc)) {
    taMisc::Error("ERROR: SyncVar called before Complie_Var initialized!");
    return;
  }

  int my_idx = recv_idx[this_proc];
  int my_n = n_local[this_proc];
  switch(mpi_type) {
  case MPI_FLOAT: {
    for(int i=0; i< my_n; i++, my_idx++)  float_send[i] = *((float*)addrs_recv[my_idx]);

//     DMEM_MPICALL(MPI_Allgather(float_send.el, max_per_proc, mpi_type, float_recv.el, max_per_proc,
// 			       mpi_type, comm), "SyncVar", "Allgather");

    int errcd = MPI_Allgather(float_send.el, max_per_proc, mpi_type, float_recv.el, max_per_proc,
			       mpi_type, comm);
    if(errcd != MPI_SUCCESS) {
      cerr << "proc: " << taMisc::dmem_proc << " fun: "
	   << "SyncVar" << " MPI_Allgather"
	   << " FAILED with code: " << dmem_mpi_decode_err(errcd)
	   << " max_per_proc: " << max_per_proc
	   << " mpi_type: " << mpi_type
	   << " send.size: " << float_send.size
	   << " recv.size: " << float_recv.size
	   << " comm: " << comm
	   << " this_proc: " << this_proc
	   << " n_procs: " << n_procs
	   << endl;
    }

    for(int proc=0;proc<n_procs;proc++) {
      if(proc == this_proc) continue; 
      int p_idx = recv_idx[proc];
      for(int i=0; i<n_local[proc]; i++, p_idx++) *((float*)addrs_recv[p_idx]) = float_recv[p_idx];
    }
    break;
  }
  case MPI_DOUBLE: {
    for(int i=0;i<my_n;i++, my_idx++)  double_send[i] = *((double*)addrs_recv[my_idx]);
    DMEM_MPICALL(MPI_Allgather(double_send.el, max_per_proc, mpi_type, double_recv.el, max_per_proc,
			       mpi_type, comm), "SyncVar", "Allgather");
    for(int proc=0;proc<n_procs;proc++) {
      if(proc == this_proc) continue; 
      int p_idx = recv_idx[proc];
      for(int i=0; i<n_local[proc]; i++, p_idx++) *((double*)addrs_recv[p_idx]) = double_recv[p_idx];
    }
    break;
  }
  case MPI_INT: {
    for(int i=0;i<my_n;i++, my_idx++)  int_send[i] = *((int*)addrs_recv[my_idx]);
    DMEM_MPICALL(MPI_Allgather(int_send.el, max_per_proc, mpi_type, int_recv.el, max_per_proc,
			       mpi_type, comm), "SyncVar", "Allgather");
    for(int proc=0;proc<n_procs;proc++) {
      if(proc == this_proc) continue; 
      int p_idx = recv_idx[proc];
      for(int i=0; i<n_local[proc]; i++, p_idx++) *((int*)addrs_recv[p_idx]) = int_recv[p_idx];
    }
    break;
  }
  case MPI_LONG: {
    for(int i=0;i<my_n;i++, my_idx++)  long_send[i] = *((long*)addrs_recv[my_idx]);
    DMEM_MPICALL(MPI_Allgather(long_send.el, max_per_proc, mpi_type, long_recv.el, max_per_proc,
			       mpi_type, comm), "SyncVar", "Allgather");
    for(int proc=0;proc<n_procs;proc++) {
      if(proc == this_proc) continue; 
      int p_idx = recv_idx[proc];
      for(int i=0; i<n_local[proc]; i++, p_idx++) *((long*)addrs_recv[p_idx]) = long_recv[p_idx];
    }
    break;
  }
  }
}

void DMemShareVar::AggVar(MPI_Op op) {
  // basic computation here is to send all of my stuff to all other nodes
  // and for them to send all of their stuff to me
  // 0: send: 0000
  // 1: send: 1111
  // 2: send: 2222
  // all recv: 0000 1111 2222

  if(n_procs <= 1) return;

  if((comm == -1) || (addrs_recv.size != n_procs * max_per_proc)) {
    taMisc::Error("ERROR: AggVar called before Complie_Var initialized!");
    return;
  }

  switch(mpi_type) {
  case MPI_FLOAT: {
    for(int i=0; i< addrs.size; i++)  float_send[i] = *((float*)addrs[i]);
    DMEM_MPICALL(MPI_Allreduce(float_send.el, float_recv.el, addrs.size, mpi_type, op, comm),
		 "AggVar", "Allreduce");
    for(int i=0; i< addrs.size; i++) *((float*)addrs[i]) = float_recv[i];
    break;
  }
  case MPI_DOUBLE: {
    for(int i=0; i< addrs.size; i++)  double_send[i] = *((double*)addrs[i]);
    DMEM_MPICALL(MPI_Allreduce(double_send.el, double_recv.el, addrs.size, mpi_type, op, comm),
		 "AggVar", "Allreduce");
    for(int i=0; i< addrs.size; i++) *((double*)addrs[i]) = double_recv[i];
    break;
  }
  case MPI_INT: {
    for(int i=0; i< addrs.size; i++)  int_send[i] = *((int*)addrs[i]);
    DMEM_MPICALL(MPI_Allreduce(int_send.el, int_recv.el, addrs.size, mpi_type, op, comm),
		 "AggVar", "Allreduce");
    for(int i=0; i< addrs.size; i++) *((int*)addrs[i]) = int_recv[i];
    break;
  }
  case MPI_LONG: {
    for(int i=0; i< addrs.size; i++)  long_send[i] = *((long*)addrs[i]);
    DMEM_MPICALL(MPI_Allreduce(long_send.el, long_recv.el, addrs.size, mpi_type, op, comm),
		 "AggVar", "Allreduce");
    for(int i=0; i< addrs.size; i++) *((long*)addrs[i]) = long_recv[i];
    break;
  }
  }
}

//////////////////////////////////////////////////////////

void DMemShare::Initialize() {
  comm = MPI_COMM_WORLD;
  vars.SetBaseType(&TA_DMemShareVar);
}

void DMemShare::InitLinks() {
  taList<taBase>::InitLinks();
  taBase::Own(vars, this);
}

void DMemShare::CutLinks() {
  vars.Reset();
  taBase_List::CutLinks();
}

void DMemShare::Copy_(const DMemShare& cp) {
  vars = cp.vars;
}

void DMemShare::SetLocal_Sequential() {
  int np = 0; MPI_Comm_size(comm, &np);
  if(size <= 0) return;
  int this_proc = 0; MPI_Comm_rank(comm, &this_proc);
  for (int i=0; i < size; i++) {
    taBase* shr_item = FastEl(i);
    shr_item->DMem_SetLocalProc(i % np);
    shr_item->DMem_SetThisProc(this_proc);
  }
}

void DMemShare::Compile_ShareVar(TypeDef* td, taBase* shr_item, MemberDef* par_md) {
  for(int m=0;m<td->members.size;m++) {
    MemberDef* md = td->members.FastEl(m);
    String shrstr = md->OptionAfter("DMEM_SHARE_SET_");
    if(shrstr.empty()) continue;
    int shrset = (int)shrstr;

    if(shrset >= vars.size) {
      taMisc::Error("WARNING: DMEM_SHARE_SET_# number is greater than max specified in object typedef DMEM_SHARE_SETS_# !");
      continue;
    }

    DMemShareVar* var = (DMemShareVar*)vars[shrset];

    int new_type = -1;
    if(md->type->ptr > 0) {
      taMisc::Error("WARNING: DMEM_SHARE_SET Specified for a pointer in type:",
		    td->name, ", member:", md->name,
		    "Pointers can not be shared.");
      continue;
    }
    if(md->type->InheritsFormal(TA_class)) {
      if(par_md != NULL) {
	taMisc::Error("WARNING: DMEM_SHARE_SET in too many nested objects: only one level of subobject nesting allowed!  type:",
		      td->name, ", member:", md->name);
	continue;
      }
      Compile_ShareVar(md->type, shr_item, md);
      continue;
    }
    else if (md->type->InheritsFrom(TA_double)) {
      new_type = (int)MPI_DOUBLE;
    }
    else if (md->type->InheritsFrom(TA_float)) {
      new_type = (int)MPI_FLOAT;
    }
    else if (md->type->InheritsFrom(TA_int)) {
      new_type = (int)MPI_INT;
    }
    else if (md->type->InheritsFrom(TA_enum)) {
      new_type = (int)MPI_INT;
    }
    else if (md->type->InheritsFrom(TA_long)) {
      new_type = (int)MPI_LONG;
    }
    else {
      taMisc::Error("WARNING: DMEM_SHARE_SET Specified for an unrecognized type.",
		    td->name, ", member:", md->name,
		    "unrecoginized types can not be shared.");
      continue;
    }
    if(var->mpi_type == -1)
      var->mpi_type = new_type;
    if(var->mpi_type != new_type) {
      taMisc::Error("WARNING: Two different types specified for the same DMEM_SHARE_SET.",
		    "All variables in same share set must be of same type!  Type:",
		    td->name, ", member:", md->name);
      continue;
    }

    void* addr = NULL;
    if(par_md == NULL) {
      addr = md->GetOff(shr_item);
    }
    else {
      void* par_base = par_md->GetOff(shr_item);
      addr = md->GetOff(par_base);
    }

    var->addrs.Add(addr);
    var->local_proc.Add(shr_item->DMem_GetLocalProc());
  }
}

void DMemShare::Compile_ShareTypes() {
  if(size <= 0) return;

  int np = 0; MPI_Comm_size(comm, &np);

  int max_share_sets = 0;
  TypeDef* last_type = NULL;
  for(int i=0; i < size; i++) {
    taBase* shr_item = FastEl(i);
    TypeDef* td = shr_item->GetTypeDef();
    if(td == last_type) continue;
    last_type = td;
    for(int j=0;j<td->opts.size;j++) {
      String shrstr = td->opts[j].after("DMEM_SHARE_SETS_");
      if(shrstr.empty()) continue;
      int shrsets = (int)shrstr;
      max_share_sets = MAX(shrsets, max_share_sets);
    }
  }

  vars.EnforceSize(max_share_sets);
  for(int i=0; i < vars.size; i++) {
    DMemShareVar* var = (DMemShareVar*)vars[i];
    var->ResetVar();
    var->n_procs = np;
  }

  if(np <= 1) return;

  for(int i=0; i < size; i++) {
    taBase* shr_item = FastEl(i);
    TypeDef* td = shr_item->GetTypeDef();
    Compile_ShareVar(td, shr_item);
  }

  for(int i=0; i < vars.size; i++) {
    ((DMemShareVar*)vars[i])->Compile_Var(comm);
  }
}

void DMemShare::DistributeItems() {
  SetLocal_Sequential();
  Compile_ShareTypes();
}

void DMemShare::Sync(int share_set) {
  if(share_set >= vars.size) {
    taMisc::Error("DMemShare::Sync -- attempt to sync for share set beyond number allocated:",
		  String(share_set));
    return;
  }
  DMemShareVar* var = (DMemShareVar*)vars[share_set];
  var->SyncVar();
}

void DMemShare::Aggregate(int share_set, MPI_Op op) {
  if(share_set >= vars.size) {
    taMisc::Error("DMemShare::Sync -- attempt to sync for share set beyond number allocated:",
		  String(share_set));
    return;
  }
  DMemShareVar* var = (DMemShareVar*)vars[share_set];
  var->AggVar(op);
}

void DMemShare::ExtractLocalFromList(taPtrList_impl& global_list, taPtrList_impl& local_list) {
  local_list.Reset();
  for (int j=0; j<global_list.size; j++) {
    if (((taBase *)global_list.FastEl_(j))->DMem_IsLocal()) {
      local_list.Link_(global_list.FastEl_(j));
    }
  }
}

stringstream* DMemShare::cmdstream = NULL;

void DMemShare::InitCmdStream() {
  CloseCmdStream();
  cmdstream = new stringstream(ios::in | ios::out);
}

void DMemShare::CloseCmdStream() {
  if(cmdstream != NULL) delete cmdstream;
  cmdstream = NULL;
}

#endif
