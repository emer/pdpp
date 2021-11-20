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

#ifndef ta_base_tmplt_h
#define ta_base_tmplt_h 1

#include <ta/ta_base.h>

template<class T> class taList : public taList_impl {
  // #NO_TOKENS #INSTANCE #NO_UPDATE_AFTER
public:
  T*		SafeEl(int idx) const		{ return (T*)SafeEl_(idx); }
  // get element at index
  T*		FastEl(int i) const		{ return (T*)el[i]; } 
  // fast element (no range checking)
  T* 		operator[](int i) const	{ return (T*)el[i]; }

  T*		DefaultEl() const		{ return (T*)DefaultEl_(); }
  // returns the element specified as the default for this list

  T*		Edit_El(T* item) const		{ return SafeEl(Find((TAPtr)item)); }
  // #MENU #MENU_ON_Edit #USE_RVAL #ARG_ON_OBJ Edit given list item

  virtual T*	FindName(const char* item_nm, int& idx=Idx) const { return (T*)FindName_(item_nm, idx); }
  // #MENU #USE_RVAL #ARGC_1 #LABEL_Find Find element with given name (item_nm) 
  virtual T* 	FindType(TypeDef* item_tp, int& idx=Idx) const { return (T*)FindType_(item_tp, idx); }
  // find given type element (NULL = not here), sets idx

  virtual T*	Pop()				{ return (T*)Pop_(); }
  // pop the last element off the stack
  virtual T*	Peek() const			{ return (T*)Peek_(); }
  // peek at the last element on the stack

  virtual T*	AddUniqNameOld(T* item)		{ return (T*)AddUniqNameOld_((void*)item); }
  // add so that name is unique, old used if dupl, returns one used
  virtual T*	LinkUniqNameOld(T* item)	{ return (T*)LinkUniqNameOld_((void*)item); }
  // link so that name is unique, old used if dupl, returns one used

  virtual bool	MoveBefore(T* trg, T* item) { return MoveBefore_((void*)trg, (void*)item); }
  // move item so that it appears just before the target item trg in the list
  virtual bool	MoveAfter(T* trg, T* item) { return MoveAfter_((void*)trg, (void*)item); }
  // move item so that it appears just after the target item trg in the list

// when a taList is declared in advance of defining the type within it, this breaks
//  void 	Initialize() 			{ SetBaseType(T::StatTypeDef(1)); }
  void	Initialize()			{ SetBaseType(&TA_taBase); }
  void	Destroy()			{ };

  taList() 				{ Register(); Initialize(); }
  taList(const taList<T>& cp)		{ Register(); Initialize(); Copy(cp); }
  ~taList() 				{ unRegister(); Destroy(); }
  TAPtr Clone() 			{ return new taList<T>(*this); }
  void  UnSafeCopy(TAPtr cp) 		{ if(cp->InheritsFrom(&TA_taList)) Copy(*((taList<T>*)cp));
                                          else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); }
  void  CastCopyTo(TAPtr cp)            { taList<T>& rf = *((taList<T>*)cp); rf.Copy(*this); }
  TAPtr MakeToken()			{ return (TAPtr)(new taList<T>); }
  TAPtr MakeTokenAry(int no)		{ return (TAPtr)(new taList<T>[no]); }
  void operator=(const taList<T>& cp)	{ Copy(cp); }
  TypeDef* GetTypeDef() const		{ return &TA_taList; }
  static TypeDef* StatTypeDef(int)	{ return &TA_taList; }
};

// do not use this macro, since you typically will want ##NO_TOKENS, #NO_UPDATE_AFTER
// which cannot be put inside the macro!
//
// #define taList_of(T)			
// class T ## _List : public taList<T> {
// public:				
//   void Initialize()	{ };		
//   void Destroy()	{ };		
//   TA_BASEFUNS(T ## _List);		
// }

// use the following as a template instead..

// define default base list to not keep tokens
class taBase_List : public taList<taBase> {
  // ##NO_TOKENS ##NO_UPDATE_AFTER list of objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(taBase_List);
};


template<class T> class taArray : public taArray_base {
  // #NO_TOKENS #INSTANCE #NO_UPDATE_AFTER
protected:
  void		InitArray_()	{ Clear_Tmp_(); alloc_size = 2; el = new T[alloc_size]; }
  void		FreeArray_()	{ if(el != NULL) delete [] el; el = NULL; }

public:
  T*		el;		// #HIDDEN #NO_SAVE Pointer to actual array memory
  T		err;		// #HIDDEN what is returned when out of range

  void*		SafeEl_(int i) const		{ return &(SafeEl(i)); } // #IGNORE
  void*		FastEl_(int i)	const		{ return &(FastEl(i)); }// #IGNORE
  int		El_Compare_(void* a, void* b) const
  { int rval=-1; if(*((T*)a) > *((T*)b)) rval=1; else if(*((T*)a) == *((T*)b)) rval=0; return rval; }
  // #IGNORE
  void		El_Copy_(void* to, void* fm)	{ *((T*)to) = *((T*)fm); } // #IGNORE
  uint		El_SizeOf_() const		{ return sizeof(T); }	 // #IGNORE
  void*		El_GetTmp_() const		{ return (void*)&err; }	 // #IGNORE
  String	El_GetStr_(void* it) const	{ return String(*((T*)it)); } // #IGNORE
  void		El_SetFmStr_(void* it, String& val) 
  { T tmp = (T)val; *((T*)it) = tmp; } // #IGNORE

  ////////////////////////////////////////////////
  // 	functions that manage the list		//
  ////////////////////////////////////////////////

  virtual void	Alloc(int sz) {
    if(alloc_size < sz)	{
      sz = MAX(16, sz);		// once allocating, use a minimum of 16
      alloc_size += TA_ALLOC_OVERHEAD;
      while((alloc_size-TA_ALLOC_OVERHEAD) <= sz) alloc_size <<= 1;
      alloc_size -= TA_ALLOC_OVERHEAD;
      T* nw = new T[alloc_size];
      int i; for(i=0; i<size; i++) nw[i] = el[i]; delete [] el; el = nw; }
  }
  // allocate an array big enough for the given size (exponentially increasing)

  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  T&		SafeEl(int i) const	
  { T* rval=(T*)&err; if((i >= 0) && (i < size)) rval=&(el[i]); return *rval; }
  // #MENU #MENU_ON_Edit #USE_RVAL the element at the given index
  T&		FastEl(int i) const		{ return el[i]; }
  // fast element (no range checking)
  T&		RevEl(int idx) const	{ return SafeEl(size - idx - 1); }
  // reverse (index) element (ie. get from the back of the list first)
  T&		operator[](int i) const		{ return el[i]; }

  virtual T	Pop()	
  { T* rval=(T*)&err; if(size>0) rval=&(el[--size]); return *rval; }
  // pop the last item in the array off
  virtual T&	Peek() const
  { T* rval=(T*)&err; if(size>0) rval=&(el[size-1]); return *rval; }
  // peek at the last item on the array

  ////////////////////////////////////////////////
  // 	functions that are passed el of type	//
  ////////////////////////////////////////////////

  virtual void	Set(int i, const T& item) 	{ SafeEl(i) = item; }
  // use this for assigning values to items in the array (Set should update if needed)
  virtual void	Add(const T& item)		{ Add_((void*)&item); }
  // #MENU add the item to the array
  virtual bool	AddUnique(const T& item)	{ return AddUnique_((void*)&item); }
  // add the item to the array if it isn't already on it, returns true if unique
  virtual void	Push(const T& item)		{ Add(item); }
  // push the item on the end of the array (same as add)
  virtual void	Insert(const T& item, int indx, int n_els=1)	{ Insert_((void*)&item, indx, n_els); }
  // #MENU Insert (n_els) item(s) at indx (-1 for end) in the array
  virtual int	Find(const T& item, int indx=0) const { return Find_((void*)&item, indx); }
  // #MENU #USE_RVAL Find item starting from indx in the array (-1 if not there)
  virtual bool	Remove(const T& item)		{ return Remove_((void*)&item); }
  virtual bool	Remove(uint indx, int n_els=1)	{ return taArray_impl::Remove(indx,n_els); }
  // Remove (n_els) item(s) at indx, returns success
  virtual bool	RemoveEl(const T& item)		{ return Remove(item); }
  // remove given item, returns success
  virtual void	InitVals(const T& item, int start=0, int end=-1) { InitVals_((void*)&item, start, end); }
  // set array elements to specified value starting at start through end (-1 = size)

  void	Initialize()			{ InitArray_(); }
  void	Destroy()			{ FreeArray_(); }

  taArray()				{ Initialize(); } // no_tokens is assumed
  taArray(const taArray<T>& cp)		{ Initialize(); Copy(cp); }
  ~taArray()				{ Destroy(); FreeArray_(); }
  TAPtr Clone() 			{ return new taArray<T>(*this); }
  void  UnSafeCopy(TAPtr cp) 		{ if(cp->InheritsFrom(&TA_taArray)) Copy(*((taArray<T>*)cp));
                                          else if(InheritsFrom(cp->GetTypeDef())) cp->CastCopyTo(this); }
  void  CastCopyTo(TAPtr cp)            { taArray<T>& rf = *((taArray<T>*)cp); rf.Copy(*this); }
  TAPtr MakeToken()			{ return (TAPtr)(new taArray<T>); }
  TAPtr MakeTokenAry(int no)		{ return (TAPtr)(new taArray<T>[no]); }
  void operator=(const taArray<T>& cp)	{ Copy(cp); }
  TypeDef* GetTypeDef() const		{ return &TA_taArray; }
  static TypeDef* StatTypeDef(int)	{ return &TA_taArray; }
};

// do not use this macro, since you typically will want ##NO_TOKENS, #NO_UPDATE_AFTER
// which cannot be put inside the macro!
//
// #define taArray_of(T)						   
// class T ## _Array : public taArray<T> {				   
// public:								   
//   virtual void*		GetTA_Element(int i, TypeDef*& eltd) const 
//   { eltd = &TA_ ## T; return SafeEl_(i); }		      		   
//   void Initialize()	{ };						   
//   void Destroy()	{ };						   
//   TA_BASEFUNS(T ## _Array);						   
// }

// use these as templates instead

// make these ones no_update_after..
class int_Array : public taArray<int> {
  // #NO_UPDATE_AFTER
public:
  virtual void	FillSeq(int start=0, int inc=1);
  // fill array with sequential values starting at start, incrementing by inc

  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_int; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(int_Array);
};
class float_Array : public taArray<float> {
  // #NO_UPDATE_AFTER
public:
  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_float; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(float_Array);
};
class double_Array : public taArray<double> {
  // #NO_UPDATE_AFTER
public:
  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_double; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(double_Array);
};
class String_Array : public taArray<String> {
  // #NO_UPDATE_AFTER
public:
  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_taString; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(String_Array);
};
class long_Array : public taArray<long> {
  // #NO_UPDATE_AFTER
public:
  virtual void	FillSeq(long start=0, long inc=1);
  // fill array with sequential values starting at start, incrementing by inc

  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_long; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(long_Array);
};

#ifdef __MAKETA__
class voidptr_Array : public taArray_base {
#else
class voidptr_Array : public taArray<void*> {
#endif
  // #NO_UPDATE_AFTER ##NO_CSS ##NO_MEMBERS
public:
  virtual void*		GetTA_Element(int i, TypeDef*& eltd) const
  { eltd = &TA_void_ptr; return SafeEl_(i); }
  void Initialize()	{ };
  void Destroy()	{ };
  TA_BASEFUNS(voidptr_Array);
};


#endif // ta_base_tmplt_h

