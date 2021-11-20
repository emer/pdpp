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

#ifndef ta_group_tmplt_h
#define ta_group_tmplt_h 1

#include <ta/ta_group.h>


template<class T> class taGroup : public taGroup_impl {
  // #NO_TOKENS #INSTANCE #NO_UPDATE_AFTER
public:
  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  // operators
  T*		SafeEl(int idx) const		{ return (T*)SafeEl_(idx); }
  // get element at index
  T*		FastEl(int i) const		{ return (T*)el[i]; }
  // fast element (no checking)
  T* 		operator[](int i) const		{ return (T*)el[i]; }

  T*		DefaultEl() const		{ return (T*)DefaultEl_(); }
  // returns the element specified as the default for this group

  // note that the following is just to get this on the menu (it doesn't actually edit)
  T*		Edit_El(T* item) const		{ return SafeEl(Find((TAPtr)item)); }
  // #MENU #MENU_ON_Edit #USE_RVAL #ARG_ON_OBJ Edit given group item

  taGroup<T>*	SafeGp(int idx) const		{ return (taGroup<T>*)gp.SafeEl(idx); }
  // get group at index
  taGroup<T>*	FastGp(int i) const		{ return (taGroup<T>*)gp.FastEl(i); }
  // the sub group at index
  T*		Leaf(int idx) const		{ return (T*)Leaf_(idx); }
  // get leaf element at index
  taGroup<T>* 	LeafGp(int idx) const		{ return (taGroup<T>*)LeafGp_(idx); }
  // the group containing given leaf

  // iterator-like functions
  T*		FirstEl(taLeafItr& lf) const	{ return (T*)FirstEl_(lf); }
  // returns first leaf element and inits indexes
  T*		NextEl(taLeafItr& lf) const 	{ return (T*)NextEl_(lf); }
  // returns next leaf element and incs indexes
  
  taGroup<T>*	FirstGp(int& g)	const		{ return (taGroup<T>*)FirstGp_(g); }
  // returns first leaf group and inits index
  taGroup<T>*	NextGp(int& g) const		{ return (taGroup<T>*)NextGp_(g); }
  // returns next leaf group and incs index

  virtual T* 	NewEl(int n_els=0, TypeDef* typ=NULL) { return (T*)NewEl_(n_els, typ);}
  // Create and add (n_els) new element(s) of given type
  virtual taGroup<T>* NewGp(int n_gps=0, TypeDef* typ=NULL) { return (taGroup<T>*)NewGp_(n_gps, typ);}
  // Create and add (n_gps) new group(s) of given type

  virtual T*	FindName(const char* item_nm, int& idx=Idx)  const { return (T*)FindName_(item_nm, idx); }
  // Find element with given name (nm) (NULL = not here), sets idx
  virtual T* 	FindType(TypeDef* item_tp, int& idx=Idx) const { return (T*)FindType_(item_tp, idx); }
  // find given type element (NULL = not here), sets idx

  virtual T*	Pop()				{ return (T*)Pop_(); }
  // pop the last element off the stack
  virtual T*	Peek()				{ return (T*)Peek_(); }
  // peek at the last element on the stack

  virtual T*	AddUniqNameOld(T* item)		{ return (T*)AddUniqNameOld_((void*)item); }
  // add so that name is unique, old used if dupl, returns one used
  virtual T*	LinkUniqNameOld(T* item)	{ return (T*)LinkUniqNameOld_((void*)item); }
  // link so that name is unique, old used if dupl, returns one used

  virtual bool	MoveBefore(T* trg, T* item) { return MoveBefore_((void*)trg, (void*)item); }
  // move item so that it appears just before the target item trg in the list
  virtual bool	MoveAfter(T* trg, T* item) { return MoveAfter_((void*)trg, (void*)item); }
  // move item so that it appears just after the target item trg in the list

  virtual T* 	FindLeafName(const char* item_nm, int& idx=Idx) const { return (T*)FindLeafName_(item_nm, idx); }
  // #MENU #MENU_ON_Edit #USE_RVAL #ARGC_1 #LABEL_Find Find element with given name (el_nm) 
  virtual T* 	FindLeafType(TypeDef* item_tp, int& idx=Idx) const { return (T*)FindLeafType_(item_tp, idx);}
  // find given type leaf element (NULL = not here), sets idx

  void Initialize() 			{ SetBaseType(T::StatTypeDef(1)); }

  taGroup() 				{ Register(); Initialize(); }
  taGroup(const taGroup<T>& cp)		{ Register(); Initialize(); Copy(cp); }
  ~taGroup() 				{ unRegister(); Destroy(); }
  TAPtr Clone() 			{ return new taGroup<T>(*this); }
  void  UnSafeCopy(TAPtr cp) 		{ if(cp->InheritsFrom(&TA_taGroup)) Copy(*((taGroup<T>*)cp));
                                          else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); }
  void  CastCopyTo(TAPtr cp)            { taGroup<T>& rf = *((taGroup<T>*)cp); rf.Copy(*this); }
  TAPtr MakeToken()			{ return (TAPtr)(new taGroup<T>); }
  TAPtr MakeTokenAry(int no)		{ return (TAPtr)(new taGroup<T>[no]); }
  void operator=(const taGroup<T>& cp)	{ Copy(cp); }
  TypeDef* GetTypeDef() const		{ return &TA_taGroup; }
  static TypeDef* StatTypeDef(int)	{ return &TA_taGroup; }
};


// do not use this macro, since you typically will want ##NO_TOKENS, #NO_UPDATE_AFTER
// which cannot be put inside the macro!
//
// #define taGroup_of(T)			
// class T ## _Group : public taGroup<T> {	
// public:					
//   void Initialize()	{ };			
//   void Destroy()	{ };			
//   TA_BASEFUNS(T ## _Group);			
// }

// use the following as a template instead..

// define default base group to not keep tokens
class taBase_Group : public taGroup<taBase> { 
  // ##NO_TOKENS ##NO_UPDATE_AFTER group of objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(taBase_Group);
};

#endif // ta_group_tmplt_h
