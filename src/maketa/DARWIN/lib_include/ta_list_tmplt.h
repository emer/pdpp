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

#ifndef ta_list_tmplt_h
#define ta_list_tmplt_h 1

#include <ta/ta_list.h>


template<class T> class taPtrList : public taPtrList_impl { // #INSTANCE
public:
  taPtrList()					{ };
  taPtrList(const taPtrList<T>& cp) 		{ Borrow(cp); }
  // borrow is guaranteed to work, others require EL_ functions..

  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  // operators
  T*		SafeEl(int i) const		{ return (T*)SafeEl_(i); }
  // element at index
  T*		FastEl(int i) const		{ return (T*)el[i]; } 
  // fast element (no range checking)
  T* 		operator[](int i) const		{ return (T*)el[i]; }
  void		operator=(const taPtrList<T>& cp) { Reset(); Borrow(cp); }
  // borrow is guaranteed to work, others require EL_ functions..

  T*		Edit_El(T* item) const		{ return SafeEl(Find(item)); }
  // #MENU #MENU_ON_Edit #USE_RVAL #ARG_ON_OBJ Edit given list item

  virtual T*	FindName(const char* item_nm, int& idx=Idx) const { return (T*)FindName_(item_nm, idx); }
  // find given named element (NULL = not here), sets idx

  virtual T*	Pop()				{ return (T*)Pop_(); }
  // pop the last element off the stack
  virtual T*	Peek() const			{ return (T*)Peek_(); }
  // peek at the last element on the stack

  virtual T*	AddUniqNameOld(T* item)		{ return (T*)AddUniqNameOld_((void*)item); }
  // add so that name is unique, old used if dupl, returns one used
  virtual T*	LinkUniqNameOld(T* item)		{ return (T*)LinkUniqNameOld_((void*)item); }
  // link so that name is unique, old used if dupl, returns one used

  ////////////////////////////////////////////////
  // 	functions that are passed el of type	//
  ////////////////////////////////////////////////

  virtual int	Find(T* item) const		{ return Find_((void*)item); } 
  // find element in list (-1 if not there)
  virtual int	Find(const char* item_nm) const	{ return taPtrList_impl::Find(item_nm); } 
  virtual int	FindEl(T* item) const		{ return Find(item); }
  // find given element in list (-1 if not there)

  virtual void	AddEl(T* item)			{ AddEl_((void*)item); }
  // append a new pointer to end of list
  virtual void	Add(T* item)	      		{ Add_((void*)item); }
  // add element to the list and "own" item
  virtual bool	AddUnique(T* item)		{ return AddUnique_((void*)item); }
  // add so that object is unique, true if unique
  virtual bool	AddUniqNameNew(T* item)		{ return AddUniqNameNew_((void*)item); }
  // add so that name is unique, true if unique, new replaces existing

  virtual bool	Insert(T* item, int idx)	{ return Insert_((void*)item, idx); }
  // Add or insert element at idx (-1 for end)
  virtual bool 	Replace(T* old_it, T* new_it)		{ return Replace_((void*)old_it, (void*)new_it); }
  virtual bool 	Replace(const char* old_nm, T* new_it)	{ return Replace_(old_nm, (void*)new_it); }
  virtual bool 	Replace(int old_idx, T* new_it)	{ return Replace_(old_idx, (void*)new_it); }
  // replace element at index with the new one
  virtual bool 	ReplaceEl(T* old_it, T* new_it)		{ return Replace(old_it, new_it); }
  // replace given element with the new one
  virtual bool 	ReplaceName(const char* old_nm, T* new_it) { return Replace(old_nm, new_it); }
  // replace named element with the new one

  virtual bool	Remove(const char* item_nm) 	{ return taPtrList_impl::Remove(item_nm);}
  virtual bool	Remove(T* item)			{ return Remove_((void*)item); }
  virtual bool	Remove(int idx)			{ return taPtrList_impl::Remove(idx); }
  // Remove element at given index
  virtual bool	RemoveEl(T* item)		{ return Remove(item); }
  // #MENU #LABEL_Remove #ARG_ON_OBJ #UPDATE_MENUS Remove given item from list

  virtual bool 	DuplicateEl(T* item)		{ return DuplicateEl_((void*)item); }
  // #MENU #ARG_ON_OBJ #UPDATE_MENUS Duplicate given list item and Add to list

  virtual void 	Link(T* item)			{ Link_((void*)item); }
  // Link an item to list without owning it
  virtual bool	LinkUnique(T* item)		{ return LinkUnique_((void*)item); }
  // link so that object is unique, true if unique
  virtual bool	LinkUniqNameNew(T* item)		{ return LinkUniqNameNew_((void*)item); }
  // link so that name is unique, true if unique, new replaces existing
  virtual bool	InsertLink(T* item, int idx= -1) { return InsertLink_((void*)item, idx);}
  // #MENU #LABEL_Link #UPDATE_MENUS Insert a link at index (-1 for end)
  virtual bool 	ReplaceLink(T* old_it, T* new_it)	{ return ReplaceLink_((void*)old_it, (void*)new_it); }
  virtual bool 	ReplaceLink(const char* old_nm, T* new_it) { return ReplaceLink_(old_nm, (void*)new_it); }
  virtual bool 	ReplaceLink(int old_idx, T* new_it)	{ return ReplaceLink_(old_idx, (void*)new_it); }
  // replace element with a link to the new one
  virtual bool 	ReplaceLinkEl(T* old_it, T* new_it)	{ return ReplaceLink(old_it, new_it); }
  // replace given element (if on list) with the new one
  virtual bool	ReplaceLinkName(const char* old_nm, T* new_it){ return ReplaceLink(old_nm, new_it); }
  // replace given named element (if on list) with the new one

  virtual void	Push(T* item)			{ Push_((void*)item); }
  // push item on stack (for temporary use, not "owned")

  virtual bool 	MoveEl(T* from, T* to)		{ return Move(Find(from), Find(to)); }
  // #MENU #LABEL_Move #ARG_ON_OBJ #UPDATE_MENUS Move item (from) to position of (to)
  virtual bool 	Transfer(T* item)  		{ return Transfer_((void*)item); }
  // #MENU #MENU_ON_Edit #NO_SCOPE #UPDATE_MENUS Transfer item to this list

  virtual bool	MoveBefore(T* trg, T* item) { return MoveBefore_((void*)trg, (void*)item); }
  // move item so that it appears just before the target item trg in the list
  virtual bool	MoveAfter(T* trg, T* item) { return MoveAfter_((void*)trg, (void*)item); }
  // move item so that it appears just after the target item trg in the list
};


#define taPtrList_of(T)						      \
class T ## _List : public taPtrList<T> {				      \
protected:								      \
  void	El_Done_(void* item)	{ delete (T*)item; }			      \
public:                                                                       \
  ~ ## T ## _List()             { Reset(); }                                  \
}

// the hash table is for use in looking up things in PtrList's by name
// it contains hash codes and corresponding indicies for items in the list

typedef unsigned long taHashVal;

class taHashEl {
  // ##NO_TOKENS holds information for one entry of the hash table
public:
  taHashVal	hash_code;	// hash-coded value of name
  int		list_idx;	// index of item in list

  void	Initialize()	{ hash_code = 0; list_idx = -1; }
  taHashEl()		{ Initialize(); }
  taHashEl(taHashVal hash, int idx)	{ hash_code = hash; list_idx = idx; }
};

class taHashBucket : public taPtrList<taHashEl> {
  // holds a set of hash table entries that all have the same hash_code modulo value
protected:
  void	El_Done_(void* it)	{ delete (taHashEl*)it; }
public:
  int		Find(taHashEl* itm) const
  { return taPtrList<taHashEl>::Find(itm); }
  int		Find(const char* itm) const
  { return taPtrList<taHashEl>::Find(itm); }

  int		Find(taHashVal hash) const;
  // find index of item (in the bucket) with given hash code value

  int		FindIndex(taHashVal hash) const;
  // find index of item (list_idx) with given hash code value

  ~taHashBucket()               { Reset(); }
};

class taHashTable : public taPtrList<taHashBucket> {
  // table has a number of buckets, each with some hash values
protected:
  void	El_Done_(void* it)	{ delete (taHashBucket*)it; }
public:

  static int		n_bucket_primes[]; // prime numbers for number of buckets
  static int		n_primes;	 // number of prime numbers (86)

  int			bucket_max;	// maximum size of any bucket 

  static taHashVal	HashCode(const char* string);
  // get a hash code value from given string

  void			Alloc(int sz);
  // allocate in prime-number increments
  void			RemoveAll();

  int			FindIndex(taHashVal hash) const;
  // find index from given hash value (-1 if not found)
  int			FindIndex(const char* string) const
  { return FindIndex(HashCode(string)); }
  // find index from given string

  bool			UpdateIndex(taHashVal hash, int index);
  // update index associated with item
  bool			UpdateIndex(const char* string, int index)
  { return UpdateIndex(HashCode(string), index); }
  // update index associated with item

  void			Add(taHashBucket* itm)
  { taPtrList<taHashBucket>::Add(itm); }
  void			Add(const char* string, int index);
  // add a new item to the hash table

  bool			Remove(taHashBucket* itm)
  { return taPtrList<taHashBucket>::Remove(itm); }
  bool			Remove(int idx)
  { return taPtrList<taHashBucket>::Remove(idx); }
  bool			Remove(taHashVal hash);
  // remove given hash code from table
  bool			Remove(const char* string)
  { return Remove(HashCode(string)); }
  // remove given string from table


  void			InitList_();
  taHashTable()			{ InitList_(); }
  taHashTable(const taHashTable& cp)	{ InitList_(); Duplicate(cp); } 
  ~taHashTable()                { Reset(); }
};


template<class T> class taPtrList_base : public taPtrList_impl { // #INSTANCE
public:
  taPtrList_base()					{ };
  taPtrList_base(const taPtrList_base<T>& cp) 		{ Duplicate(cp); };

  ////////////////////////////////////////////////
  // 	functions that are passed el of type	//
  ////////////////////////////////////////////////

  virtual int	Find(const char* item_nm) const	{ return taPtrList_impl::Find(item_nm); } 
  virtual int	Find(T* item) const		{ return Find_((void*)item); } 
  // find element in list (-1 if not there)
  virtual int	FindEl(T* item) const		{ return Find(item); }
  // find given element in list (-1 if not there)

  virtual void	AddEl(T* item)			{ AddEl_((void*)item); }
  // append a new pointer to end of list
  virtual void	Add(T* item)	      		{ Add_((void*)item); }
  // add element to the list and "own" item
  virtual bool	AddUnique(T* item)		{ return AddUnique_((void*)item); }
  // add so that object is unique, true if unique
  virtual bool	AddUniqNameNew(T* item)		{ return AddUniqNameNew_((void*)item); }
  // add so that name is unique, true if unique, new replaces existing

  virtual bool	Insert(T* item, int where)	{ return Insert_((void*)item, where); }
  // insert element at index (-1 for end)
  virtual bool 	Replace(T* old_it, T* new_it)		{ return Replace_((void*)old_it, (void*)new_it); }
  virtual bool 	Replace(const char* old_nm, T* new_it)	{ return Replace_(old_nm, (void*)new_it); }
  virtual bool 	Replace(int old_idx, T* new_it)		{ return Replace_(old_idx, (void*)new_it); }
  // replace element at index with the new one
  virtual bool 	ReplaceEl(T* old_it, T* new_it)		{ return Replace(old_it, new_it); }
  // replace given element with the new one
  virtual bool 	ReplaceName(const char* old_nm, T* new_it) { return Replace(old_nm, new_it); }
  // replace named element with the new one

  virtual bool	Remove(const char* item_nm) 	{ return taPtrList_impl::Remove(item_nm);}
  virtual bool	Remove(T* item)			{ return Remove_((void*)item); }
  virtual bool	Remove(int i)			{ return taPtrList_impl::Remove(i); }
  virtual bool	RemoveEl(T* item)		{ return Remove(item); }
  // #MENU #LABEL_Remove #ARG_ON_OBJ #UPDATE_MENUS Remove given item from list

  virtual bool 	DuplicateEl(T* item)		{ return DuplicateEl_((void*)item); }
  // #MENU #ARG_ON_OBJ #UPDATE_MENUS Duplicate given list item and Add to list

  virtual void 	Link(T* item)			{ Link_((void*)item); }
  // Link an item to list without owning it
  virtual bool	LinkUnique(T* item)		{ return LinkUnique_((void*)item); }
  // link so that object is unique, true if unique
  virtual bool	LinkUniqNameNew(T* item)		{ return LinkUniqNameNew_((void*)item); }
  // link so that name is unique, true if unique, new replaces existing
  virtual bool	InsertLink(T* item, int idx= -1)	{ return InsertLink_((void*)item, idx);}
  // #MENU #LABEL_Link #UPDATE_MENUS Insert a link at index (-1 for end)
  virtual bool 	ReplaceLink(T* old_it, T* new_it)	{ return ReplaceLink_((void*)old_it, (void*)new_it); }
  virtual bool 	ReplaceLink(const char* old_nm, T* new_it) { return ReplaceLink_(old_nm, (void*)new_it); }
  virtual bool 	ReplaceLink(int old_idx, T* new_it)	{ return ReplaceLink_(old_idx, (void*)new_it); }
  // replace element with a link to the new one
  virtual bool 	ReplaceLinkEl(T* old_it, T* new_it)	{ return ReplaceLink(old_it, new_it); }
  // replace given element (if on list) with the new one
  virtual bool	ReplaceLinkName(const char* old_nm, T* new_it){ return ReplaceLink(old_nm, new_it); }
  // replace given named element (if on list) with the new one

  virtual void	Push(T* item)			{ Push_((void*)item); }
  // push item on stack (for temporary use, not "owned")

  virtual bool 	MoveEl(T* from, T* to)		{ return Move(Find(from), Find(to)); }
  // #MENU #LABEL_Move #ARG_ON_OBJ #UPDATE_MENUS Move item (from) to position of (to)
  virtual bool 	Transfer(T* item)  		{ return Transfer_((void*)item); }
  // #MENU #MENU_ON_Edit #NO_SCOPE #UPDATE_MENUS Transfer element to this list

  virtual bool	MoveBefore(T* trg, T* item) { return MoveBefore_((void*)trg, (void*)item); }
  // move item so that it appears just before the target item trg in the list
  virtual bool	MoveAfter(T* trg, T* item) { return MoveAfter_((void*)trg, (void*)item); }
  // move item so that it appears just after the target item trg in the list
};


// the plainarray is not a taBase..

template<class T> class taPlainArray : public taArray_impl { // #INSTANCE
protected:
  void		InitArray_()	{ Clear_Tmp_(); alloc_size = 2; el = new T[alloc_size]; }
  void		FreeArray_()	{ if(el != NULL) delete [] el; el = NULL; }

public:
  T*		el;		// #HIDDEN #NO_SAVE Pointer to actual array memory
  T		err;		// #HIDDEN what is returned when out of range

  void*		SafeEl_(int i) const		{ return &(SafeEl(i)); } // #IGNORE
  void*		FastEl_(int i)	const		{ return &(FastEl(i)); } // #IGNORE
  int		El_Compare_(void* a, void* b) const
  { int rval=-1; if(*((T*)a) > *((T*)b)) rval=1; else if(*((T*)a) == *((T*)b)) rval=0; return rval; }
  // #IGNORE
  void		El_Copy_(void* to, void* fm)	{ *((T*)to) = *((T*)fm); } // #IGNORE
  uint		El_SizeOf_() const		{ return sizeof(T); }	 // #IGNORE
  void*		El_GetTmp_() const		{ return (void*)&err; }	 // #IGNORE
  String	El_GetStr_(void* it) const	{ return String(*((T*)it)); } // #IGNORE
  void		El_SetFmStr_(void* it, String& val) { *((T*)it) = (T)val; } // #IGNORE

  taPlainArray()				{ InitArray_(); }
  taPlainArray(const taPlainArray<T>& cp)	{ InitArray_(); Duplicate(cp); }
  virtual ~taPlainArray()			{ FreeArray_(); }

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
  // the element at the given index
  T&		FastEl(int i) const		{ return el[i]; }
  // fast element (no range checking)
  T&		RevEl(int idx) const		{ return SafeEl(size - idx - 1); }
  // reverse (index) element (ie. get from the back of the list first)
  T&		operator[](int i) const		{ return el[i]; }
  void		operator=(const taPlainArray<T>& cp)	{ Reset(); Duplicate(cp); }

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
  virtual void	Insert(const T& item, int idx, int n_els=1) { Insert_((void*)&item, idx, n_els); }
  // #MENU Insert (n_els) item(s) at idx (-1 for end) in the array
  virtual int	Find(const T& item, int i=0) const { return Find_((void*)&item, i); }
  // #MENU #USE_RVAL Find item starting from idx in the array (-1 if not there)
  virtual bool	Remove(const T& item)		{ return Remove_((void*)&item); }
  virtual bool	Remove(uint idx, int n_els=1)	{ return taArray_impl::Remove(idx,n_els); }
  // Remove (n_els) item(s) at idx, returns success
  virtual bool	RemoveEl(const T& item)		{ return Remove(item); }
  // remove given item, returns success
};


#endif // ta_list_tmplt_h
