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

#ifndef ta_base_h
#define ta_base_h 1

#include <ta/typea.h>

#ifdef NO_IV
class taivGetFile;		// #IGNORE
#else
class taivGetFile;
#endif
class taBase_List;

class tabMisc {
  // #NO_TOKENS #INSTANCE miscellaneous useful stuff for taBase
public:
  static TAPtr		root;	
  // root of the structural object hierarchy
  static taBase_List	delayed_remove;
  // list of objs to be removed in the wait process (e.g. when objs delete themselves)
  static taBase_List	delayed_updateafteredit;
  // list of objs to be update-after-edit'd in the wait process

  static void	Close_Obj(TAPtr obj);
  // call this to implement closing object function
  static int	WaitProc();
  // wait process function: remove objects from groups, update others
  static bool	NotifyEdits(TAPtr obj);
  // notify any edit dialogs of a taptr object that object has changed
  static void	DelayedUpdateAfterEdit(TAPtr obj);
  // call update-after-edit on object in wait process (in case this does other kinds of damage..)
};


#define TA_BASEFUNS(y) 	y () { Register(); Initialize(); SetDefaultName(); } \
			y (const y& cp) { Register(); Initialize(); Copy(cp); } \
			~y () { unRegister(); Destroy(); } \
			TAPtr Clone() { return new y(*this); }  \
			void  UnSafeCopy(TAPtr cp) { if(cp->InheritsFrom(&TA_##y)) Copy(*((y*)cp)); \
						     else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); } \
			void  CastCopyTo(TAPtr cp) { y& rf = *((y*)cp); rf.Copy(*this); } \
			TAPtr MakeToken(){ return (TAPtr)(new y); } \
			TAPtr MakeTokenAry(int no){ return (TAPtr)(new y[no]); } \
			void operator=(const y& cp) { Copy(cp); } \
			TypeDef* GetTypeDef() const { return &TA_##y; } \
			static TypeDef* StatTypeDef(int) { return &TA_##y; }

// use this when you have consts in your class and can't use the generic constrs
#define TA_CONST_BASEFUNS(y) ~y () { unRegister(); Destroy(); } \
			TAPtr Clone() { return new y(*this); }  \
			void  UnSafeCopy(TAPtr cp) { if(cp->InheritsFrom(&TA_##y)) Copy(*((y*)cp)); \
						     else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); } \
			void  CastCopyTo(TAPtr cp) { y& rf = *((y*)cp); rf.Copy(*this); } \
			TAPtr MakeToken(){ return (TAPtr)(new y); } \
			TAPtr MakeTokenAry(int no){ return (TAPtr)(new y[no]); } \
			void operator=(const y& cp) { Copy(cp); } \
			TypeDef* GetTypeDef() const { return &TA_##y; } \
			static TypeDef* StatTypeDef(int) { return &TA_##y; }

// for use with templates
#define TA_TMPLT_BASEFUNS(y,T) y () { Register(); Initialize(); SetDefaultName(); } \
			y (const y<T>& cp) { Register(); Initialize(); Copy(cp); } \
			~y () { unRegister(); Destroy(); } \
			TAPtr Clone() { return new y<T>(*this); } \
			void  UnSafeCopy(TAPtr cp) { if(cp->InheritsFrom(&TA_##y)) Copy(*((y<T>*)cp)); \
						     else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); } \
			void  CastCopyTo(TAPtr cp) { y<T>& rf = *((y<T>*)cp); rf.Copy(*this); } \
			TAPtr MakeToken(){ return (TAPtr)(new y<T>); }  \
			TAPtr MakeTokenAry(int no){ return (TAPtr)(new y<T>[no]); }  \
			void operator=(const y<T>& cp) { Copy(cp); } \
			TypeDef* GetTypeDef() const { return &TA_##y; } \
			static TypeDef* StatTypeDef(int) { return &TA_##y; }

#define COPY_FUNS(T, P)   void Copy(const T& cp)	{ P::Copy(cp); Copy_(cp); }

#define SIMPLE_COPY(T)	  void Copy_(const T& cp)	{ TA_ ## T.CopyOnlySameType((void*)this, (void*)&cp); }

// simplified Get owner functions B = ta_base object, T = class name
#define GET_MY_OWNER(T) (T *) GetOwner(&TA_ ## T)
#define GET_OWNER(B,T)  (T *) B ->GetOwner(&TA_ ## T)

#ifndef NO_IV
class ivColor;			// #IGNORE
#endif
class SelectEdit;

class taBase {
  // #NO_TOKENS #INSTANCE #NO_UPDATE_AFTER Base type for all type-aware classes
  // has auto instances for all taBases unless NO_INSTANCE
protected:
  uint			refn;		// number of references to this object
  static String		no_name; 	// return this for no names
  static int		no_idx;		// return this for no index
  static MemberDef*	no_mdef;	// return this for no memberdef ptr
public:
  void			Initialize()		{ refn = 0; }
  // #IGNORE the constructor, do not call parent because it is called during construction
  virtual void		Destroy()		{ };
  // #IGNORE the destructor, do not call parent
  virtual void		InitLinks()		{ };
  // #IGNORE initialize links to other objs, called after construction & SetOwner, call parent
  virtual void		CutLinks();
  // #IGNORE cut any links to other objs, called upon removal from a group or owner
  void			Copy(const taBase&)	{ };
  // #IGNORE the copy (=) operator, call parent

  virtual void 		Register()			// #IGNORE 
  { if(!taMisc::not_constr) GetTypeDef()->Register((void*)this); }
  virtual void 		unRegister()			// #IGNORE
  { if(!taMisc::not_constr) GetTypeDef()->unRegister((void*)this); }

  // these are all "free" with the TA_BASEFUNS
  taBase()					{ Register(); Initialize(); }
  taBase(taBase& cp)				{ Register(); Initialize(); Copy(cp); }
  taBase(int)					{ Initialize(); } // don't register...
  virtual ~taBase() 				{ Destroy(); }
  virtual TAPtr		Clone()			{ return new taBase(*this); } // #IGNORE 
  virtual void		UnSafeCopy(TAPtr)	{ };
  virtual void		CastCopyTo(TAPtr)	{ };
  virtual TAPtr 	MakeToken()		{ return new taBase; }	// #IGNORE 
  virtual TAPtr 	MakeTokenAry(int no)	{ return new taBase[no]; } // #IGNORE 
  void 			operator=(const taBase& cp)	{ Copy(cp); }
  static  TypeDef*	StatTypeDef(int)	{ return &TA_taBase; }	// #IGNORE 
  virtual TypeDef*	GetTypeDef() const	{ return &TA_taBase; }	// #IGNORE 

  static TAPtr 		MakeToken(TypeDef* td);
  // #IGNORE make a token of the given type
  static TAPtr 		MakeTokenAry(TypeDef* td, int no);
  // #IGNORE make an array of tokens of the given type

  // Reference counting mechanisms, all static just for consistency..
  static uint		GetRefn(TAPtr it)	{ return it->refn; } // #IGNORE
  static void  		Ref(taBase& it)	{ it.refn++; }	     // #IGNORE
  static void  		Ref(TAPtr it)		{ it->refn++; }	     // #IGNORE
  static void   	unRef(TAPtr it)		{ it->refn--; }	     // #IGNORE
  static void   	Done(TAPtr it)		{ if(it->refn == 0) delete it; } // #IGNORE
  static void		unRefDone(TAPtr it)	{ unRef(it); Done(it); }	 // #IGNORE
  static void		Own(taBase& it, TAPtr onr);	// #IGNORE
  static void		Own(TAPtr it, TAPtr onr);	// #IGNORE

  // Pointer management routines (all pointers should be ref'd!!)
  // NOTE: TAPtr& versions are not working for gcc 3.3.2: type conversion might be
  // making a new object instead of setting the pointer;  pointer versions are safer!
  static void 		InitPointer(TAPtr* ptr) { *ptr = NULL; } // #IGNORE 
//   static void 		InitPointer(TAPtr& ptr) { ptr = NULL; } // #IGNORE 
  static void 		SetPointer(TAPtr* ptr, TAPtr new_val);	 // #IGNORE 
//   static void 		SetPointer(TAPtr& ptr, TAPtr new_val);	 // #IGNORE 
  static void 		OwnPointer(TAPtr* ptr, TAPtr new_val, TAPtr onr); // #IGNORE 
//   static void 		OwnPointer(TAPtr& ptr, TAPtr new_val, TAPtr onr); // #IGNORE 
  static void 		DelPointer(TAPtr* ptr);				  // #IGNORE 
//   static void 		DelPointer(TAPtr& ptr);				  // #IGNORE 

  // these are to be overloaded if names are available 
  virtual bool		SetName(const char*)    { return 0; } // #IGNORE
  virtual const String&	GetName() const 	{ return no_name; } // #IGNORE 
  virtual void 		SetDefaultName();			    // #IGNORE
  virtual void		SetTypeDefaults();			    // #IGNORE 
  virtual void		SetTypeDefaults_impl(TypeDef* ttd, TAPtr scope); // #IGNORE
  virtual void		SetTypeDefaults_parents(TypeDef* ttd, TAPtr scope); // #IGNORE

  virtual void* 	GetTA_Element(int, TypeDef*& eltd) const
  { eltd = NULL; return NULL; } // #IGNORE a bracket opr
  virtual TAPtr 	SetOwner(TAPtr)		{ return(NULL); } // #IGNORE 
  virtual TAPtr 	GetOwner() const	{ return(NULL); }
  virtual TAPtr		GetOwner(TypeDef* td) const;
  virtual String 	GetPath_Long(TAPtr ta=NULL, TAPtr par_stop=NULL) const;
  // #IGNORE get path from root (default), but stop at par_stop if non-null
  virtual String	GetPath(TAPtr ta=NULL, TAPtr par_stop=NULL) const;
  // get path without name informtation, stop at par_stop if non-null
  virtual TAPtr		FindFromPath(String& path, MemberDef*& ret_md=no_mdef, int start=0) const;
  // find object from path (starting from this, and position start of the path)

  // functions for managing structural scoping
  virtual TypeDef*	GetScopeType();
  // #IGNORE gets my scope type (if NULL, it means no scoping, or root)
  virtual TAPtr		GetScopeObj(TypeDef* scp_tp=NULL);
  // #IGNORE gets the object that is at the scope type above me (uses GetScopeType() or scp_tp)
  virtual bool		SameScope(TAPtr ref_obj, TypeDef* scp_tp=NULL);
  // #IGNORE determine if this is in the same scope as given ref_obj (uses my scope type)
  static int		NTokensInScope(TypeDef* type, TAPtr ref_obj, TypeDef* scp_tp=NULL);
  // #IGNORE number of tokens of taBase objects of given type in same scope as ref_obj

  // utility functions for doing path stuff
  static int		GetNextPathDelimPos(const String& path, int start);
  // #IGNORE get the next delimiter ('.' or '[') position in the path
  static int		GetLastPathDelimPos(const String& path);
  // #IGNORE get the last delimiter ('.' or '[') position in the path

  virtual TAPtr		New(int n_objs=0, TypeDef* type=NULL);
  // Create n_objs objects of given type (type is optional)
  virtual uint		GetSize() const		{ return GetTypeDef()->size; }  // #IGNORE 

  virtual void		UpdateAfterEdit();
  // called after editing, or any user change to members (eg. in the interface, script)

  // the following are selected functions from TypeDef
  bool		HasOption(const char* op) const		// #IGNORE 
  { return GetTypeDef()->HasOption(op); }
  bool		CheckList(const String_PArray& lst) const // #IGNORE 
  { return GetTypeDef()->CheckList(lst); }
  
  bool 		InheritsFrom(TypeDef* it) const		{ return GetTypeDef()->InheritsFrom(it); }
  bool		InheritsFrom(const TypeDef& it) const	{ return GetTypeDef()->InheritsFrom(it); }
  bool 		InheritsFrom(const char* nm) const	// does this inherit from given type?
  { return GetTypeDef()->InheritsFrom(nm); }
  
  bool		InheritsFormal(TypeDef* it) const	// #IGNORE 
  { return GetTypeDef()->InheritsFormal(it); }
  bool		InheritsFormal(const TypeDef& it) const	// #IGNORE 
  { return GetTypeDef()->InheritsFormal(it); }

  virtual MemberDef*	FindMember(const char* nm, int& idx=no_idx) const // #IGNORE 
  { return GetTypeDef()->members.FindName(nm, idx); }	
  virtual MemberDef*	FindMember(TypeDef* it, int& idx=no_idx) const 	// #IGNORE 
  { return GetTypeDef()->members.FindType(it, idx); }	
  virtual MemberDef* 	FindMember(void* mbr, int& idx=no_idx) const 	// #IGNORE 
  { return GetTypeDef()->members.FindAddr((void*)this, mbr, idx); }
  virtual MemberDef* 	FindMemberPtr(void* mbr, int& idx=no_idx) const	// #IGNORE 
  { return GetTypeDef()->members.FindAddrPtr((void*)this, mbr, idx); }	// #IGNORE

  // these get overwritten by groups, lists, etc.
  virtual MemberDef* 	FindMembeR(const char* nm, void*& ptr) const 	// #IGNORE 
  { return GetTypeDef()->members.FindNameAddrR(nm, (void*)this, ptr); }
  virtual MemberDef* 	FindMembeR(TypeDef* it, void*& ptr) const	// #IGNORE 
  { return GetTypeDef()->members.FindTypeAddrR(it, (void*)this, ptr); }

  virtual bool		FindCheck(const char* nm) const // #IGNORE check this for the name
  { return ((GetName() == nm) || InheritsFrom(nm)); }

  virtual String	GetEnumString(const char* enum_tp_nm, int enum_val) const
  { return GetTypeDef()->GetEnumString(enum_tp_nm, enum_val); }
  // get the name corresponding to given enum value in enum type enum_tp_nm
  virtual int		GetEnumVal(const char* enum_nm, String& enum_tp_nm = no_name) const
  { return GetTypeDef()->GetEnumVal(enum_nm, enum_tp_nm); }
  // get the enum value corresponding to the given enum name (-1 if not found), and sets enum_tp_nm to name of type this enum belongs in (empty if not found)

  virtual void		CopyFromSameType(void* src_base) 	// #IGNORE 
  { GetTypeDef()->CopyFromSameType((void*)this, src_base); }
  virtual void		CopyOnlySameType(void* src_base)
  { GetTypeDef()->CopyOnlySameType((void*)this, src_base); }
  // #IGNORE copy only those members from same type (no inherited)
  virtual void		MemberCopyFrom(int memb_no, void* src_base) // #IGNORE 
  { GetTypeDef()->MemberCopyFrom(memb_no, (void*)this, src_base); }

  virtual ostream&  	OutputType(ostream& strm) const		// #IGNORE 
  { return GetTypeDef()->OutputType(strm); }
  virtual ostream&  	OutputInherit(ostream& strm) const 	// #IGNORE 
  { return GetTypeDef()->OutputInherit(strm); }
  virtual ostream&  	OutputTokens(ostream& strm) const	//#IGNORE 
  { GetTypeDef()->tokens.List(strm); return strm; }

  virtual ostream& 	Output(ostream& strm, int indent = 0) const // #IGNORE 
  { return GetTypeDef()->Output(strm, (void*)this, indent); }
  virtual ostream& 	OutputR(ostream& strm, int indent = 0) const // #IGNORE 
  { return GetTypeDef()->OutputR(strm, (void*)this, indent); }

  virtual taivGetFile*	GetFileDlg();	// #IGNORE gets file dialog for this object

  virtual int	 	Load(istream& strm, TAPtr par=NULL)
  // #MENU #MENU_ON_Object #ARGC_1 #UPDATE_MENUS Load object data from a file
  { return GetTypeDef()->Dump_Load(strm, (void*)this, par); }
  virtual int	 	Dump_Load_impl(istream& strm, TAPtr par=NULL) // #IGNORE 
  { return GetTypeDef()->Dump_Load_impl(strm, (void*)this, par); }
  virtual int	 	Dump_Load_Value(istream& strm, TAPtr par=NULL) // #IGNORE 
  { return GetTypeDef()->Dump_Load_Value(strm, (void*)this, par); }

  virtual int 		Save(ostream& strm, TAPtr par=NULL, int indent=0)
  // #MENU #ARGC_1 #QUICK_SAVE Save object data to a file
  { return GetTypeDef()->Dump_Save(strm, (void*)this, par, indent); }
  virtual int 		SaveAs(ostream& strm, TAPtr par=NULL, int indent=0)
  // #MENU #ARGC_1 Save object data to a new file
  { return Save(strm,par,indent); }
  virtual int 		Dump_Save_impl(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_Save_impl(strm, (void*)this, par, indent); } // #IGNORE 
  virtual int 		Dump_Save_inline(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_Save_inline(strm, (void*)this, par, indent); } // #IGNORE 
  virtual int 		Dump_Save_Path(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_Save_Path(strm, (void*)this, par, indent); } // #IGNORE 
  virtual int 		Dump_Save_Value(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_Save_Value(strm, (void*)this, par, indent); } // #IGNORE 

  virtual int		Dump_SaveR(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_SaveR(strm, (void*)this, par, indent); } 	// #IGNORE 
  virtual int 		Dump_Save_PathR(ostream& strm, TAPtr par=NULL, int indent=0)
  { return GetTypeDef()->Dump_Save_PathR(strm, (void*)this, par, indent); } // #IGNORE 

  virtual int		Edit(bool wait=false);
  // #MENU #ARGC_0 #MENU_ON_Object #NO_SCRIPT Edit this object using the gui
  virtual bool		CloseEdit();	// close any open edit dialogs for this object
#ifndef NO_IV
  virtual const ivColor* GetEditColor() { return NULL; } // #IGNORE background color for edit dialog
  virtual const ivColor* GetEditColorInherit(); // #IGNORE background color for edit dialog, include inherited colors from parents
#endif
  virtual void		Close();
  // #MENU #CONFIRM #NO_REVERT_AFTER #LABEL_Close_(Destroy) PERMANENTLY Destroy this object!  This is not Iconify.
  virtual bool		Close_Child(TAPtr obj);
  // #IGNORE actually closes a child object (should be immediate child)
  virtual bool		CopyFrom(TAPtr cpy_from);
  // #MENU #MENU_SEP_BEFORE #TYPE_ON_this #NO_SCOPE #UPDATE_MENUS Copy from given object into this object
  // this is a safe interface to UnSafeCopy
  virtual bool		CopyTo(TAPtr cpy_to);
  // #MENU #TYPE_ON_this #NO_SCOPE #UPDATE_MENUS Copy to given object from this object
  // need both directions to more easily handle scoping of types on menus
  virtual bool		DuplicateMe();
  // #MENU #CONFIRM #UPDATE_MENUS Make another copy of myself (done through owner)
  virtual bool		ChangeMyType(TypeDef* new_type);
  // #MENU #TYPE_this #UPDATE_MENUS Change me into a different type of object, copying current info (done through owner)
  virtual bool		SelectForEdit(MemberDef* member, SelectEdit* editor, const char* extra_label);
  // #MENU select a given member for editing in a select edit dialog -- if already on dialog, removes it & returns false (else true)
  virtual bool		SelectForEditNm(const char* memb_nm, SelectEdit* editor, const char* extra_label);
  // #IGNORE hard code interface for updating editors
  virtual bool		SelectFunForEdit(MethodDef* function, SelectEdit* editor, const char* extra_label);
  // #MENU select a given function (method) for calling in a select edit dialog -- if already on dialog, removes it & returns false (else true)
  virtual bool		SelectFunForEditNm(const char* function_nm, SelectEdit* editor, const char* extra_label);
  // #IGNORE hard code interface for updating editors
  virtual void		Help();
  // #MENU get help on using this object

  virtual void		ReplacePointersHook(TAPtr old_ptr);
  // #IGNORE hook to replace all pointers that might have pointed to old version of this object to this: default is to call taMisc::ReplaceAllPtrs
  virtual int		ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  // #IGNORE replace all pointers to old_ptr with new_ptr (calls TypeDef::ReplaceAllPtrsThis by default)

  virtual void		CallFun(const char* fun_name);
  // call function of given name on this object, prompting for args using gui interface

#ifdef DMEM_COMPILE
  virtual bool DMem_IsLocal()      // #IGNORE check if local given stored "this process" number
  { return true; } 
  virtual bool DMem_IsLocalProc(int proc)  // #IGNORE check if local given external process number
  { return true; }
  virtual int  DMem_GetLocalProc()  // #IGNORE get local processor number for this object
  { return 0; }
  virtual void DMem_SetLocalProc(int lproc) // #IGNORE set the local process number for this object
  { };
  virtual void DMem_SetThisProc(int proc) // #IGNORE set the local processor number RELATIVE to relevant communicator
  { } 
#endif
};

inline istream& operator>>(istream &strm, taBase &obj)
{ obj.Load(strm); return strm; }
inline ostream& operator<<(ostream &strm, taBase &obj)
{ obj.Save(strm); return strm; }

class taNBase : public taBase { // #NO_TOKENS Named, owned base class of taBase
public:
  TAPtr		owner;		// #READ_ONLY #NO_SAVE pointer to owner
  String	name;		// name of the object
  
  bool 		SetName(const char* nm)    	{ name = nm; return true; }
  const String&	GetName() const		{ return name; }
  TAPtr 	GetOwner() const	{ return owner; }
  TAPtr		GetOwner(TypeDef* tp) const { return taBase::GetOwner(tp); }
  TAPtr 	SetOwner(TAPtr ta)	{ owner = ta; return ta; }

  virtual void		UpdateAfterEdit(); // notify set the edit

  void 	Initialize()			{ owner = NULL; }
  void	Destroy()			{ CutLinks(); }
  void	CutLinks();
  void	Copy_(const taNBase& cp);
  COPY_FUNS(taNBase, taBase);
  TA_BASEFUNS(taNBase);
};

typedef taNBase* TANPtr;


class taOBase : public taBase {
  // #NO_TOKENS #NO_UPDATE_AFTER Named, owned base class of taBase
public:
  TAPtr		owner;		// #READ_ONLY #NO_SAVE pointer to owner
  
  TAPtr 	GetOwner() const	{ return owner; }
  TAPtr		GetOwner(TypeDef* tp) const { return taBase::GetOwner(tp); }
  TAPtr 	SetOwner(TAPtr ta)	{ owner = ta; return ta; }

  virtual void		UpdateAfterEdit(); // notify the edit dialogs

  void 	Initialize()			{ owner = NULL; }
  void	Destroy()			{ CutLinks(); }
  void	CutLinks();
  TA_BASEFUNS(taOBase);
};

typedef taPtrList_base<taBase>  taPtrList_ta_base;

class taList_impl;
typedef taList_impl* TABLPtr;

class taList_impl : public taBase, public taPtrList_ta_base {
  // #INSTANCE #NO_TOKENS #NO_UPDATE_AFTER implementation for a taBase list class
protected:
  const String&	GetListName_() const	{ return name; }
  const String&	El_GetName_(void* it) const { return ((TAPtr)it)->GetName(); }
  TALPtr	El_GetOwner_(void* it) const { return (TABLPtr)((TAPtr)it)->GetOwner(); }
  void*		El_SetOwner_(void* it)	{ ((TAPtr)it)->SetOwner(this); return it; }
  bool		El_FindCheck_(void* it, const char* nm) const
  { return (((TAPtr)it)->FindCheck(nm) &&
	    ((El_GetOwner_(it) != NULL) || (El_GetOwner_(it) == (TALPtr) this))); }

  void*		El_Ref_(void* it)	{ taBase::Ref((TAPtr)it); return it; }
  void*		El_unRef_(void* it)	{ taBase::unRef((TAPtr)it); return it; }
  void		El_Done_(void* it)	{ taBase::Done((TAPtr)it); }
  void*		El_Own_(void* it)	{ taBase::Own((TAPtr)it,this); return it; }
  void		El_disOwn_(void* it)	
  { if(El_GetOwner_(it) == this) ((TAPtr)it)->CutLinks(); El_Done_(El_unRef_(it)); }
  // cut links to other objects when removed from owner group 

  void*		El_MakeToken_(void* it) { return (void*)((TAPtr)it)->MakeToken(); }
  void*		El_Copy_(void* trg, void* src)
  { ((TAPtr)trg)->UnSafeCopy((TAPtr)src); return trg; }
 
public:
  static MemberDef* find_md;	// #HIDDEN #NO_SAVE return value for findmember of data

  TypeDef*	el_base;	// #HIDDEN #NO_SAVE Base type for objects in group
  TypeDef* 	el_typ;		// #TYPE_ON_el_base Default type for objects in group
  int		el_def;		// Index of default element in group
  String	name;		// name of the list
  TAPtr		owner;		// #READ_ONLY #NO_SAVE owner of the list

  // stuff for the taBase
  bool		SetName(const char* nm)			{ name = nm; return true; }
  const String&	GetName() const				{ return name; }
  void*   	GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = el_typ; return (void*)SafeEl_(i); }
  TAPtr		SetOwner(TAPtr ow)			{ owner = ow; return ow; }
  TAPtr		GetOwner(TypeDef* tp) const		{ return taBase::GetOwner(tp); }
  TAPtr		GetOwner() const			{ return owner; }

  TAPtr		New(int n_objs=0, TypeDef* typ=NULL);
  // #MENU #MENU_ON_Object #ARGC_0 #UPDATE_MENUS #NO_SCRIPT create n_objs new objects of given type

  String 	GetPath_Long(TAPtr ta=NULL, TAPtr par_stop = NULL) const;
  String 	GetPath(TAPtr ta=NULL, TAPtr par_stop = NULL) const;

  bool 		FindCheck(const char* nm) const // also check for el_typ
  { return ((name == nm) || InheritsFrom(nm) || el_typ->InheritsFrom(nm)); }

  MemberDef* 	FindMembeR(const char* nm, void*& ptr) const;    // extended to search in the list 
  MemberDef* 	FindMembeR(TypeDef* it, void*& ptr) const; // extended to search in the list 

  void		Close();
  bool		Close_Child(TAPtr obj);

  // IO
  ostream& 	OutputR(ostream& strm, int indent = 0) const;

  int		Dump_SaveR(ostream& strm, TAPtr par=NULL, int indent=0);
  int		Dump_Save_PathR(ostream& strm, TAPtr par=NULL, int indent=0);
  virtual int	Dump_Save_PathR_impl(ostream& strm, TAPtr par=NULL, int indent=0);
  int		Dump_Load_Value(istream& strm, TAPtr par=NULL);

  TAPtr		DefaultEl_() const	{ return (TAPtr)SafeEl_(el_def); } // #IGNORE

  virtual int	SetDefaultEl(TypeDef* it);
  virtual int	SetDefaultEl(const char* nm);
  virtual int	SetDefaultEl(TAPtr it);
  // set the default element to be given item
  virtual int	SetDefaultElName(const char* nm)	{ return SetDefaultEl(nm); }
  // set the default element to be item with given name
  virtual int	SetDefaultElType(TypeDef* it)	{ return SetDefaultEl(it); }
  // set the default element to be item with given type

  virtual int	Find(TAPtr item) const		{ return taPtrList_ta_base::Find(item); }
  virtual int	Find(TypeDef* item) const;
  // find element of given type
  virtual int	Find(const char* item_nm) const	{ return taPtrList_ta_base::Find(item_nm); }

  virtual bool	Remove(const char* item_nm)	{ return taPtrList_ta_base::Remove(item_nm); }
  virtual bool	Remove(TAPtr item)	{ return taPtrList_ta_base::Remove(item); }
  virtual bool	Remove(int idx);
  // Remove object at given index on list
  virtual bool	RemoveEl(TAPtr item)	{ return Remove(item); }
  // Remove given item from the list

  virtual void	EnforceSize(int sz);
  // add or remove elements to force list to be of given size
  virtual void	EnforceType();
  // enforce current type (all elements have to be of this type)
  void	EnforceSameStru(const taList_impl& cp);
  // make the two lists identical in terms of size and types of objects

  virtual bool	ChangeType(int idx, TypeDef* new_type);
  // change type of item at index
  virtual bool	ChangeType(TAPtr itm, TypeDef* new_type);
  // #MENU #MENU_ON_Object #UPDATE_MENUS #ARG_ON_OBJ #TYPE_ON_el_base change type of item to new type, copying current info
  virtual int	ReplaceType(TypeDef* old_type, TypeDef* new_type);
  // #MENU #MENU_ON_Object #USE_RVAL #UPDATE_MENUS #TYPE_ON_el_base replace all items of old type with new type (returns number changed)
  
  int	ReplaceAllPtrsThis(TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  virtual TAPtr	FindType_(TypeDef* item_tp, int& idx) const; 	// #IGNORE

  void	SetBaseType(TypeDef* it); // set base (and default) type to given td

  MemberDef*	ReturnFindMd() const;	// return the find_md variable, initialized if necessary

  void 	Initialize();
  void	Destroy();
  void 	CutLinks();
  void 	Copy(const taList_impl& cp);
  TA_BASEFUNS(taList_impl);
};

class taArray_base : public taBase, public taArray_impl {
  // #INSTANCE #NO_TOKENS #NO_UPDATE_AFTER base for arrays (from taBase)
public:
  TAPtr		owner;		// #READ_ONLY #NO_SAVE owner of the list

  ostream& 	Output(ostream& strm, int indent = 0) const;
  ostream& 	OutputR(ostream& strm, int indent = 0) const
  { return Output(strm, indent); }

  int		Dump_Save_Value(ostream& strm, TAPtr par=NULL, int indent = 0);
  int		Dump_Load_Value(istream& strm, TAPtr par=NULL);

  TAPtr		SetOwner(TAPtr ow)			{ owner = ow; return ow; }
  TAPtr		GetOwner(TypeDef* tp) const		{ return taBase::GetOwner(tp); }
  TAPtr		GetOwner() const			{ return owner; }

  void 	Initialize()	{ owner = NULL; }
  void 	Destroy()	{ CutLinks(); }
  void	CutLinks();
  void	Copy(const taArray_base& cp)		{ Copy_Duplicate(cp); }
  TA_BASEFUNS(taArray_base);
};

#include <ta/ta_base_tmplt.h>

// define selectedit if no iv
#ifdef NO_IV
class SelectEdit {
  bool	do_nothing;
};
#endif

#endif // ta_base_h
