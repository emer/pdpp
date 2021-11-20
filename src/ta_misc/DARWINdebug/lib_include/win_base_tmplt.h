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

// win_base_tmplt.h
  
#ifndef win_base_tmplt_h
#define win_base_tmplt_h 1

#include <ta_misc/win_base.h>

#ifdef USE_TEMPLATE_GROUPS

template<class T> class MenuGroup : public MenuGroup_impl { // #INSTANCE
public:
  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  // operators
  T*		SafeEl(int i) const	{ return (T*)SafeEl_(i); }
  // element at index
  T*		FastEl(int i) const	{ return (T*)el[i]; }
  // fast element (no checking)
  T* 		operator[](int i) const	{ return (T*)el[i]; }

  T*		DefaultEl() const		{ return (T*)DefaultEl_(); }
  // returns the element specified as the default for this group

  MenuGroup<T>*	Gp(int i) const		{ return (MenuGroup<T>*)gp.El(i); }
  // the sub group at index
  MenuGroup<T>*	FastGp(int i) const		{ return (MenuGroup<T>*)gp.FastEl(i); }
  // the sub group at index
  T*		Leaf(int idx) const		{ return (T*)Leaf_(idx); }
  // leaf element at leaf-index
  MenuGroup<T>* 	LeafGp(int idx) const	{ return (MenuGroup<T>*)LeafGp_(idx); }
  // the group containing given leaf

  // iterator-like functions
  T*		FirstEl(taLeafItr& lf) 	{ return (T*)FirstEl_(lf); }
  // returns first leaf element and inits indexes
  T*		NextEl(taLeafItr& lf)  	{ return (T*)NextEl_(lf); }
  // returns next leaf element and incs indexes
  
  MenuGroup<T>*	FirstGp(int& g)			{ return (MenuGroup<T>*)FirstGp_(g); }
  // returns first leaf group and inits index
  MenuGroup<T>*	NextGp(int& g)			{ return (MenuGroup<T>*)NextGp_(g); }
  // returns next leaf group and incs index

  virtual T* 	NewEl(int no, TypeDef* typ=NULL) { return (T*)NewEl_(no, typ);}
  // create and add a new element
  virtual MenuGroup<T>* NewGp(int no, TypeDef* typ=NULL)
  { return (MenuGroup<T>*)NewGp_(no, typ);} // create and add a new sub-group 

  virtual T*	FindName(char* it, int& idx=Idx)  const { return (T*)FindName_(it, idx); }
  // find given named element (NULL = not here), sets idx
  virtual T* 	FindType(TypeDef* it, int& idx=Idx) const { return (T*)FindType_(it, idx); }
  // find given type element (NULL = not here), sets idx

  virtual T*	Pop()				{ return (T*)Pop_(); }
  // pop the last element off the stack
  virtual T*	Peek()				{ return (T*)Peek_(); }
  // peek at the last element on the stack

  virtual T*	AddUniqNameOld(T* it)		{ return (T*)AddUniqNameOld_((void*)it); }
  // add so that name is unique, old used if dupl, returns one used
  virtual T*	LinkUniqNameOld(T* it)		{ return (T*)LinkUniqNameOld_((void*)it); }
  // link so that name is unique, old used if dupl, returns one used

  virtual bool	MoveBefore(T* trg, T* item) { return MoveBefore_((void*)trg, (void*)item); }
  // move item so that it appears just before the target item trg in the list
  virtual bool	MoveAfter(T* trg, T* item) { return MoveAfter_((void*)trg, (void*)item); }
  // move item so that it appears just after the target item trg in the list

  virtual T* 	FindLeafName(char* it, int& idx=Idx) const { return (T*)FindLeafName_(it, idx); }
  // find given named leaf element (NULL = not here), sets idx
  virtual T* 	FindLeafType(TypeDef* it, int& idx=Idx) const { return (T*)FindLeafType_(it, idx);}
  // find given type leaf element (NULL = not here), sets idx

  void Initialize() 			{ SetBaseType(T::StatTypeDef(1)); }

  MenuGroup() 				{ Register(); Initialize(); }
  MenuGroup(const MenuGroup<T>& cp)	{ Register(); Initialize(); Copy(cp); }
  ~MenuGroup() 				{ unRegister(); Destroy(); }
  TAPtr Clone() 			{ return new MenuGroup<T>(*this); }
  void  UnSafeCopy(TAPtr cp) 		{ if(cp->InheritsFrom(&TA_MenuGroup)) Copy(*((MenuGroup<T>*)cp));
                                          else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); }
  void  CastCopyTo(TAPtr cp)            { MenuGroup<T>& rf = *((MenuGroup<T>*)cp); rf.Copy(*this); }
  TAPtr MakeToken()			{ return (TAPtr)(new MenuGroup<T>); }
  TAPtr MakeTokenAry(int no)		{ return (TAPtr)(new MenuGroup<T>[no]); }
  void operator=(const MenuGroup<T>& cp){ Reset(); Copy(cp); }
  TypeDef* GetTypeDef() const		{ return &TA_MenuGroup; }
  static TypeDef* StatTypeDef(int)	{ return &TA_MenuGroup; }
};

#endif // USE_TEMPLATE_GROUPS

#endif // win_base_tmplt_h



