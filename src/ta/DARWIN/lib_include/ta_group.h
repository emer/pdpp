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

#ifndef ta_group_h
#define ta_group_h 1

#include <ta/typea.h>
#include <ta/ta_base.h>

#ifndef __MAKETA__
#include <sstream>
#endif

#ifdef List			// prevent interference from iv
#undef List
#endif


class 	taGroup_impl;		// pre-declare
typedef taGroup_impl* 	TAGPtr;

typedef taList<taGroup_impl> TALOG; // list of groups (LOG)

class 	taSubGroup : public TALOG {
  // #INSTANCE ##NO_TOKENS ##NO_UPDATE_AFTER has the sub-groups for a group
public:
  void	Dirty();

  bool	Transfer(taBase* item);

  void	Initialize()	{ };
  void	Destroy()	{ };
  TA_BASEFUNS(taSubGroup);
};

class	taLeafItr {		// contains the indicies for iterating over leafs
public:
  TAGPtr	cgp;		// pointer to current group
  int		g;		// index of current group
  int		i;		// index of current leaf element
};

#define FOR_ITR_EL(T, el, grp, itr) \
for(el = (T*) grp FirstEl(itr); el; el = (T*) grp NextEl(itr))

#define FOR_ITR_GP(T, el, grp, itr) \
for(el = (T*) grp FirstGp(itr); el; el = (T*) grp NextGp(itr))

// move some of the iv interface stuff into this guy..

class taGroup_impl : public taList_impl {
  // #INSTANCE #NO_TOKENS #NO_UPDATE_AFTER implementation of a group
public:
  virtual TAGPtr GetSuperGp_();			// #IGNORE Parent super-group, or NULL
  virtual void	 UpdateLeafCount_(int no); 	// #IGNORE updates the leaves count

public:
  int 		leaves;		// #READ_ONLY #NO_SAVE total number of leaves
  taSubGroup	gp; 		// #HIDDEN #NO_FIND #NO_SAVE sub-groups within this one
  TAGPtr	super_gp;	// #READ_ONLY #NO_SAVE super-group above this
  TALOG* 	leaf_gp; 	// #READ_ONLY #NO_SAVE 'flat' list of leaf-containing-gps 

  void		Dirty();		// when anything changes..
  bool		IsEmpty()	{ return (leaves == 0) ? true : false; }

  TAPtr 	New(int no=0, TypeDef* typ = NULL);

  MemberDef* 	FindMembeR(const char* nm, void*& ptr) const;    // extended to search in the group
  MemberDef* 	FindMembeR(TypeDef* it, void*& ptr) const; // extended to search in the group

  // IO routines
  ostream& 	OutputR(ostream& strm, int indent = 0) const;

  int		Dump_SaveR(ostream& strm, TAPtr par=NULL, int indent=0);
  int		Dump_Save_PathR_impl(ostream& strm, TAPtr par=NULL, int indent=0);

  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  TAGPtr	 Gp_(int i) const	{ return gp.SafeEl(i); } // #IGNORE 
  TAGPtr 	 FastGp_(int i) const	{ return gp.FastEl(i); } // #IGNORE 
  virtual TAPtr  Leaf_(int idx) const;	// #IGNORE DFS through all subroups for leaf i
  virtual TAGPtr LeafGp_(int idx) const; // #IGNORE the group having this leaf in it

  // iterator-like functions
  TAGPtr 	FirstGp_(int& g) const	// #IGNORE first sub-gp
  { g = 0; if(leaf_gp == NULL) InitLeafGp(); return leaf_gp->SafeEl(0); }
  TAGPtr 	NextGp_(int& g)	const	// #IGNORE next sub-gp
  { return leaf_gp->SafeEl(++g); }

  TAPtr	 	FirstEl_(taLeafItr& lf) const	// #IGNORE first leaf
  { TAPtr rval=NULL; lf.i = 0; lf.cgp = FirstGp_(lf.g);
    if(lf.cgp != NULL) rval=(TAPtr)lf.cgp->el[0]; return rval; }
  TAPtr	 	NextEl_(taLeafItr& lf)	const	// #IGNORE next leaf
  { TAPtr rval=NULL; if(++lf.i >= lf.cgp->size) {
    lf.i = 0; lf.cgp = leaf_gp->SafeEl(++lf.g); }
    if(lf.cgp != NULL) rval = (TAPtr)lf.cgp->el[lf.i]; return rval; }

  virtual TAGPtr NewGp_(int no, TypeDef* typ=NULL);	// #IGNORE create sub groups
  virtual TAPtr	 NewEl_(int no, TypeDef* typ=NULL);	// #IGNORE create sub groups

  virtual TAPtr FindLeafName_(const char* it, int& idx=Idx) const; 	// #IGNORE
  virtual TAPtr	FindLeafType_(TypeDef* it, int& idx=Idx) const;	// #IGNORE


  ////////////////////////////////////////////////
  // functions that don't depend on the type	//
  ////////////////////////////////////////////////

  virtual void	InitLeafGp() const;		// Initialize the leaf group
  virtual void	InitLeafGp_impl(TALOG* lg) const; // #IGNORE impl of init leaf gp
  virtual void	AddEl_(void* it); 		// #IGNORE update leaf count
  virtual bool	Remove(const char* item_nm)	{ return taList_impl::Remove(item_nm); }
  virtual bool	Remove(TAPtr item)		{ return taList_impl::Remove(item); }
  virtual bool	Remove(int idx);

  virtual bool 	RemoveLeaf(TAPtr item); 	// remove given leaf element
  virtual bool	RemoveLeaf(const char* item_nm);
  virtual bool  RemoveLeaf(int idx);	
  // Remove leaf element at leaf index
  virtual bool	RemoveLeafName(const char* item_nm)	{ return RemoveLeaf(item_nm); }
  // remove given named leaf element 
  virtual bool	RemoveLeafEl(TAPtr item)		{ return RemoveLeaf(item); }
  // Remove given leaf element
  virtual void 	RemoveAll();
  // Remove all elements of the group

  virtual bool	RemoveGp(int idx) 			{ return gp.Remove(idx); }
  // remove group at given index
  virtual bool	RemoveGp(TAGPtr group)			{ return gp.Remove(group); }
  // #MENU #FROM_GROUP_gp #MENU_ON_Edit #UPDATE_MENUS remove given group 
  virtual TALOG* EditSubGps() 				{ return &gp; }
  // #MENU #USE_RVAL edit the list of sub-groups (e.g., so you can move around subgroups)

  virtual void	EnforceLeaves(int sz);
  // ensure that sz leaves exits by adding new ones to top group and removing old ones from end
  void	EnforceSameStru(const taGroup_impl& cp);

  int	ReplaceType(TypeDef* old_type, TypeDef* new_type);
  int	ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr);

  virtual int	FindLeaf(TAPtr item) const;  // find given leaf element (-1 = not here)
  virtual int	FindLeaf(TypeDef* item) const;
  virtual int	FindLeaf(const char* item_nm) const;
  // find named leaf element
  virtual int	FindLeafEl(TAPtr item) const	{ return FindLeaf(item); }
  // find given leaf element -1 = not here. 

  void	Duplicate(const taGroup_impl& cp);
  void	DupeUniqNameOld(const taGroup_impl& cp);
  void	DupeUniqNameNew(const taGroup_impl& cp);
  
  void	Borrow(const taGroup_impl& cp);
  void	BorrowUnique(const taGroup_impl& cp);
  void	BorrowUniqNameOld(const taGroup_impl& cp);
  void	BorrowUniqNameNew(const taGroup_impl& cp);

  void	Copy_Common(const taGroup_impl& cp);
  void	Copy_Duplicate(const taGroup_impl& cp);
  void	Copy_Borrow(const taGroup_impl& cp);

  virtual void 	List(ostream& strm=cout) const; // Display list of elements in the group

  void 	Initialize();
  void 	InitLinks();		// inherit the el_typ from parent group..
  void  Destroy();
  void 	Copy(const taGroup_impl& cp);
  TA_BASEFUNS(taGroup_impl);
};

#ifdef DMEM_COMPILE

#include <mpi.h>

// add the following code into any object that is going to be shared across processors
// #ifdef DMEM_COMPILE
//   int 		dmem_local_proc; // #IGNORE processor on which these units are local
//   static int	dmem_this_proc;	// #IGNORE processor rank for this processor RELATIVE TO COMMUNICATOR for the network
//   virtual bool 	DMem_IsLocalProc(int proc)   	{ return dmem_local_proc == proc; } // #IGNORE
//   virtual bool 	DMem_IsLocal()       		{ return dmem_local_proc == dmem_proc; }  // #IGNORE
//   virtual int 	DMem_GetLocalProc() 		{ return dmem_local_proc; } // #IGNORE
//   virtual void 	DMem_SetLocalProc(int lproc) 	{ dmem_local_proc = lproc; } // #IGNORE
//   virtual void 	DMem_SetThisProc(int proc) 	{ dmem_this_proc = proc; } // #IGNORE
// #endif

#define DMEM_MPICALL(mpicmd, fun, mpi_call) \
  DMemShare::DebugCmd(fun, mpi_call); \
  DMemShare::ProcErr(mpicmd, fun, mpi_call)

// use the following to conditionalize running of functions directly as opposed
// to having them be called later via the cmdstream script calls
// it is necessary to do this to sychronize all dmem procs so they all call the
// exact same function scripts at exactly the same time!
#define DMEM_GUI_RUN_IF if(taMisc::dmem_nprocs == 1)

class DMemShareVar : public taBase {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS definition of a variable of a given type (FLOAT, DOUBLE, INT) that each proc has some instances of (can be multiple members of a given object) -- these can all be Allgather'ed to sync across procs
public:
  MPI_Comm	comm;		// #IGNORE communicator associated with these shared objs
  int		mpi_type;	// #IGNORE mpi's type for this variable
  int		max_per_proc;	// #IGNORE maximum number of vars per any one proc
  int		n_procs;	// #IGNORE number of processors in this communicator (set during Compile_Var)
  int		this_proc;	// #IGNORE proc id (rank) of this processor in communicator (set during Compile_Var)

  // the following two must be initialized for all the data to be shared prior to calling Compile_Var
  voidptr_Array	addrs;		// #IGNORE addresses for each item to be shared (one for each individual data item)
  int_Array 	local_proc; 	// #IGNORE which proc each guy is local to (one for each individual data item)

  int_Array	n_local; 	// #IGNORE number of local variables for each process (size nprocs)
  int_Array	recv_idx;	// #IGNORE starting indicies into addrs_recv list for each proc (size nprocs)
  voidptr_Array	addrs_recv; 	// #IGNORE addresses in recv format (size nprocs * max_per_proc; 000..111..222...)

  // the following are contiguous data arrays for sending and gathering from all other procs 
  // (size for both = n_procs * max_per_proc; send only needs max_per_proc for allgather, but needs all for allreduce)
  float_Array	float_send; 	// #IGNORE
  float_Array	float_recv; 	// #IGNORE
  double_Array	double_send; 	// #IGNORE
  double_Array	double_recv; 	// #IGNORE
  int_Array	int_send; 	// #IGNORE
  int_Array	int_recv; 	// #IGNORE
  long_Array	long_send; 	// #IGNORE
  long_Array	long_recv; 	// #IGNORE

  virtual void	Compile_Var(MPI_Comm cm); // #IGNORE call this after updating the variable info
  virtual void 	SyncVar();	// #IGNORE synchronize variable across procs (allgather, so all procs have all data)
  virtual void 	AggVar(MPI_Op op); // #IGNORE aggregate variable across procs (allreduce: each sends and recvs all data using op to merge w/ existing vals)

  virtual void 	ResetVar();	// #IGNORE reset variable info

  void 	Initialize();
  void 	Destroy()	{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const DMemShareVar& cp);
  COPY_FUNS(DMemShareVar, taBase);
  TA_BASEFUNS(DMemShareVar);
};


class DMemShare : public taBase_List {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS collection of objects that are shared across distributed processes: link the objects into this list to share them
public:
  taBase_List	vars;		// #IGNORE list of DMemShareVar variables, one for each share_set (set of variables to be shared at the same time)
  // NOTE: a share_set must all have the same variable type (e.g., all must be FLOAT or DOUBLE, etc)!

  MPI_Comm	comm;		// #IGNORE communicator associated with these shared objs

#ifndef __MAKETA__
  static stringstream*	cmdstream;	// #IGNORE command stream: communicating commands across dmem procs
#endif

  static void 	InitCmdStream();	// #IGNORE initialize command stream
  static void 	CloseCmdStream(); 	// #IGNORE close command stream

  static void	DebugCmd(const char* fun, const char* mpi_call);
  // #IGNORE provide debugging trace at start of mpi command call
  static bool	ProcErr(int ercd, const char* fun, const char* mpi_call);
  // #IGNORE process any errors from command, and provide done message if debugging

  virtual void 	SetLocal_Sequential();	// #IGNORE set local processor on shared objects in sequence: 0 1 2..n 0 1 2.. 

  virtual void 	Compile_ShareVar(TypeDef* td, taBase* shr_item, MemberDef* par_md=NULL);
  // #IGNORE compile current set of objects and type info into set of types used in share/aggregate calls (MPI types)
  virtual void 	Compile_ShareTypes();   // #IGNORE compile current set of objects and type info into set of types used in share/aggregate calls (MPI types)

  virtual void 	DistributeItems(); // #IGNORE distribute the items across the nodes: calls above two functions 

  virtual void 	Sync(int share_set);
  // #IGNORE synchronize across all processors for specific set of shared variables
  virtual void 	Aggregate(int share_set, MPI_Op op);
  // #IGNORE aggregate across all processors for specific set of shared variables: this only works for one floating point variable per object

  static void 	ExtractLocalFromList(taPtrList_impl& global_list, taPtrList_impl& local_list);
  // #IGNORE 

  void 	Initialize();
  void 	Destroy()	{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const DMemShare& cp);
  COPY_FUNS(DMemShare, taBase_List);
  TA_BASEFUNS(DMemShare);
};

#else

#define DMEM_GUI_RUN_IF 

// dummy version to keep the _TA files the same..

class DMemShareVar : public taBase {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS definition of a variable of a given type (FLOAT, DOUBLE, INT) that each proc has some instances of (can be multiple members of a given object) -- these can all be Allgather'ed to sync across procs
public:

  void	Dummy(const DMemShareVar&) { };
  void 	Initialize() { };
  void 	Destroy()	{ };
  TA_BASEFUNS(DMemShareVar);
};

class DMemShare : public taBase_List {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS collection of objects that are shared across distributed processes: link the objects into this list to share them
public:
  static void 	ExtractLocalFromList(taPtrList_impl&, taPtrList_impl&) { };
  // #IGNORE 

  void	Dummy(const DMemShare&) { };
  void 	Initialize() 	{ };
  void 	Destroy()	{ };
  TA_BASEFUNS(DMemShare);
};

#endif

#include <ta/ta_group_tmplt.h>

#endif // ta_group_h
