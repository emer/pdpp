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

#ifndef ta_list_h
#define ta_list_h 1
// this is the amount of overhead (in 4 byte units) allowed by the alloc
// routines, which allocate a power of two minus this amount

#include <ta/ta_stdef.h>

#define TA_ALLOC_OVERHEAD 2

// pointer list:  semantics of ownership determined by the list functions Own, Ref
// Add = put on list and "own"
// Push = put on list, refer to, but don't own (ie. "link")

class taPtrList_impl;
typedef taPtrList_impl* TALPtr;

class taHashTable;

class taPtrList_impl { // ##NO_TOKENS implementation of the pointer list class
protected:
  static String		 no_el_name;	// when the el has no name..
  static taPtrList_impl scratch_list;	// a list for any temporary processing needs

  // these define what to do with an individual item (overload them!)
  virtual const String& GetListName_() const 		{ return no_el_name; }
  // name of the list
  virtual const String& El_GetName_(void*) const 	{ return no_el_name; }
  // gets name from an element (for list)
  virtual TALPtr El_GetOwner_(void*) const	{ return (TALPtr)this; }
  // who owns the el?
  virtual void*	El_SetOwner_(void* it) 		{ return it; }
  // set owner to this
  virtual void	El_SetIndex_(void*, int) 	{ };
  // sets the element's self-index
  virtual bool  El_FindCheck_(void* it, const char* nm) const
  { return (El_GetName_(it) == nm); }
  virtual int	El_Compare_(void* a, void* b) const
  { int rval=-1; if(El_GetName_(a) > El_GetName_(b)) rval=1;
    else if(El_GetName_(a) == El_GetName_(b)) rval=0; return rval; }
  // compare two items for purposes of sorting

  virtual void*	El_Ref_(void* it)	{ return it; }	// when pushed
  virtual void* El_unRef_(void* it)  	{ return it; }	// when poped
  virtual void	El_Done_(void*)		{ };	// when "done" (delete)
  virtual void*	El_Own_(void* it)	{ El_SetOwner_(El_Ref_(it)); return it; }
  virtual void	El_disOwn_(void* it)	{ El_Done_(El_unRef_(it)); }

  virtual void*	El_MakeToken_(void* it) 	{ return it; }
  // how to make a token of the same type as the argument
  virtual void*	El_Copy_(void* trg, void*) { return trg; }
  // how to copy from given item (2nd arg)

  void		InitList_();	
  virtual int	Scratch_Find_(const char* it) const;
  // find item on the scratch_list using derived El_FindCheck_()
  void		UpdateIndex_(int idx);
  // update the index of the item at given index (i.e., which was just moved)

public:
  void**	el;		// #READ_ONLY #NO_SAVE #HIDDEN the elements themselves
  int 		alloc_size;	// #READ_ONLY #NO_SAVE allocation size
  taHashTable*	hash_table;	// #READ_ONLY #NO_SAVE #HIDDEN a hash table (NULL if not used)
  int		size;		// #READ_ONLY #NO_SAVE number of elements in the list

  taPtrList_impl()			{ InitList_(); }
  taPtrList_impl(const taPtrList_impl& cp)	{ InitList_(); Duplicate(cp); } 
  virtual ~taPtrList_impl();

  static ostream& Indenter(ostream& strm, const char* itm, int no, int prln, int tabs);
  static int	Idx;			// #HIDDEN pass to find if you don't want one


  ////////////////////////////////////////////////
  // 	functions that return the type		//
  ////////////////////////////////////////////////

  void*		SafeEl_(int i) const				
  { void* rval=NULL; if((i >= 0) && (i < size)) rval = el[i]; return rval; } 	// #IGNORE
  void*		FastEl_(int i)	const	{ return el[i]; } 	// #IGNORE
  virtual void*	FindName_(const char* it, int& idx=Idx) const;	// #IGNORE
  virtual void*	Pop_();					// #IGNORE
  void*		Peek_() const
  { void* rval=NULL; if(size > 0) rval = el[size-1]; return rval; }  // #IGNORE

  virtual void*	AddUniqNameOld_(void* it);
  // #IGNORE add so that name is unique, old used if dupl, returns one used

  virtual void*	LinkUniqNameOld_(void* it);		// #IGNORE


  ////////////////////////////////////////////////
  // 	functions that are passed el of type	//
  ////////////////////////////////////////////////

  virtual int	Find_(void* it) const; 			// #IGNORE

  virtual void	AddEl_(void* it);
  // #IGNORE just puts the el on the list, doesn't do anything else
  virtual void	Add_(void* it);				// #IGNORE
  virtual bool	AddUnique_(void* it);			// #IGNORE
  virtual bool	AddUniqNameNew_(void* it);
  // #IGNORE add a new object so that list only has one, new replaces existing
  virtual bool	DuplicateEl_(void* it);
  // #IGNORE duplicate given item and add to list
  virtual bool	Insert_(void* it, int where);		// #IGNORE
  virtual bool 	Replace_(void* ol, void* nw);		// #IGNORE
  virtual bool 	Replace_(const char* ol, void* nw);	// #IGNORE
  virtual bool 	Replace_(int ol, void* nw);		// #IGNORE
  virtual bool 	Transfer_(void* it);			// #IGNORE
  virtual bool	Remove_(void* it);			// #IGNORE
  virtual bool	MoveBefore_(void* trg, void* item); 	// #IGNORE
  virtual bool	MoveAfter_(void* trg, void* item); 	// #IGNORE

  virtual void 	Link_(void* it);			// #IGNORE
  virtual bool	LinkUnique_(void* it); 			// #IGNORE 
  virtual bool	LinkUniqNameNew_(void* it);		// #IGNORE
  virtual bool	InsertLink_(void* it, int where);	// #IGNORE
  virtual bool 	ReplaceLink_(void* ol, void* nw);	// #IGNORE 
  virtual bool 	ReplaceLink_(const char* ol, void* nw);	// #IGNORE 
  virtual bool 	ReplaceLink_(int ol, void* nw);		// #IGNORE 

  virtual void	Push_(void* it);			// #IGNORE

  ////////////////////////////////////////////////
  // functions that don't depend on the type	//
  ////////////////////////////////////////////////

  virtual void	Dirty()		{ };	// #IGNORE what to do when the list is modified
  virtual void	Alloc(int sz); 
  // allocate a list big enough for given number of elements (or current size)
  virtual void 	Reset()			{ RemoveAll(); }
  // reset the list (remove all elements)
  virtual bool	IsEmpty()	{ return (size == 0) ? true : false; }

  virtual void	BuildHashTable(int n_buckets);
  // build a hash table with given number of buckets (not dynamic, so make it big)

  virtual int	Find(const char* nm) const; 
  // find named element in list

  virtual bool	Remove(const char* item_nm);
  virtual bool	Remove(int idx);
  // remove (and delete) element from list at index
  virtual bool	RemoveName(const char* item_nm)		{ return Remove(item_nm); }
  // remove given named element from list (if on list)
  virtual bool	RemoveLast();
  // remove the last element on the list
  virtual void	RemoveAll();
  // #MENU #MENU_ON_Edit #CONFIRM #UPDATE_MENUS Remove all elements on the list

  virtual bool	Move(int from, int to);
  // Move element from (from) to position (to) in list
  virtual bool	Swap(int pos1, int pos2);
  // Swap the elements in the two given positions on the list

  virtual void	PopAll();
  // pop all elements off the stack

  virtual void	Permute();
  // #MENU #CONFIRM #UPDATE_MENUS permute the items in the list into a random order
  virtual void	Sort(bool descending=false);
  // #MENU #CONFIRM #UPDATE_MENUS sort the items in the list in alpha order according to name (or El_Compare_)
  virtual void	Sort_(bool descending=false);	// implementation of sorting function
  virtual void 	UpdateAllIndicies();	// update all indicies of elements in list


  /////////////////////////////////////////////////////////////////////////
  // replicating items: either by clone/add (duplicate) or link (borrow) //
  /////////////////////////////////////////////////////////////////////////

  void	Duplicate(const taPtrList_impl& cp);
  // duplicate (clone & add) elements of given list into this one
  void	DupeUniqNameNew(const taPtrList_impl& cp);
  // duplicate so result is unique names, replacing with new ones where dupl
  void	DupeUniqNameOld(const taPtrList_impl& cp);
  // duplicate so result is unique names, using old ones where dupl

  void	Stealth_Borrow(const taPtrList_impl& cp);
  // borrow without referencing the borrowed elements (i.e. use AddEl_())
  void	Borrow(const taPtrList_impl& cp); 
  // borrow (link) elements of given list into this one
  void	BorrowUnique(const taPtrList_impl& cp);
  // borrow so result is unique list
  void	BorrowUniqNameNew(const taPtrList_impl& cp);
  // borrow so result is unique names, replacing with new ones where dupl
  void	BorrowUniqNameOld(const taPtrList_impl& cp);
  // borrow so result is unique names, using old ones where dupl

  /////////////////////////////////
  // copying items between lists //
  /////////////////////////////////

  void	Copy_Common(const taPtrList_impl& cp);
  // apply copy operator to only those items in common between the two lists
  void	Copy_Duplicate(const taPtrList_impl& cp);
  // apply copy operator to items, use duplicate to add new ones from cp (if necc)
  void	Copy_Borrow(const taPtrList_impl& cp);
  // apply copy operator to items, use borrow to add new ones from cp (if necc)

  // output
  virtual void 	List(ostream& strm=cout) const; 	// List the group items
};


class taArray_impl {
  // ##NO_TOKENS Base Type for Arrays, no tokens of which are ever kept
protected:
  int		alloc_size;		// allocated (physical) size

  virtual void	InitArray_()			{ }; // alloc first elements
  virtual void	FreeArray_()			{ }; // free the array

public:
  int 		size;			// #NO_SAVE #READ_ONLY number of elements in the array

  taArray_impl()			{ size = 0; }
  virtual ~taArray_impl()		{ alloc_size = 0; size = 0; }

  virtual void		Alloc(int)		{ }; // allocate of a given size
  virtual void		Reset()			{ size = 0; };
  // reset the list to zero size (does not free memory)
  ////////////////////////////////////////////////
  // 	internal functions that depend on type	//
  ////////////////////////////////////////////////

  virtual void*		SafeEl_(int) const		{ return NULL; }
  // #IGNORE element at posn
  virtual void*		FastEl_(int) const		{ return NULL; }
  // #IGNORE element at posn
  virtual int		El_Compare_(void*, void*) const	{ return 0; }
  // #IGNORE for sorting
  virtual void		El_Copy_(void*, void*)		{ };
  // #IGNORE
  virtual uint		El_SizeOf_() const		{ return 1; }
  // #IGNORE size of element
  virtual void*		El_GetTmp_() const		{ return NULL; }
  // #IGNORE return ptr to Tmp of type 
  virtual String	El_GetStr_(void*) const		{ return ""; } // #IGNORE
  virtual void		El_SetFmStr_(void*, String&) 	{ };       // #IGNORE
  virtual void		Clear_Tmp_();				       // #IGNORE

  virtual void		Add_(void* it);			// #IGNORE
  virtual bool		AddUnique_(void* it);		// #IGNORE
  virtual void		Insert_(void* it, int where, int n=1); // #IGNORE
  virtual int		Find_(void* it, int where=0) const; 	// #IGNORE
  virtual bool		Remove_(void* it);		// #IGNORE
  virtual void		InitVals_(void* it, int start=0, int end=-1);// #IGNORE
  
  ////////////////////////////////////////////////
  // functions that don't depend on the type	//
  ////////////////////////////////////////////////

  virtual void	EnforceSize(int sz);
  // #MENU #MENU_ON_Edit force array to be of given size by inserting blanks or removing

  virtual bool	Remove(uint idx, int n_els=1);
  // #MENU #MENU_ON_Edit Remove (n_els) item(s) at idx, returns success
  virtual bool	Move(int from, int to);
  // #MENU move item from index to index
  virtual void	Permute();
  // #MENU permute the items in the list into a random order
  virtual void	Sort(bool descending=false);
  // #MENU sort the list in ascending order (or descending if switched)
  virtual void	ShiftLeft(int nshift);
  // shift all the elements in the array to the left by given number of items
  virtual void	ShiftLeftPct(float pct);
  // shift the array to the left by given percentage of current size
  virtual int	V_Flip(int width);
  // vertically flip the array as if it was arrange in a matrix of width

  virtual void	Duplicate(const taArray_impl& cp);
  // duplicate the items in the list
  virtual void	DupeUnique(const taArray_impl& cp);
  // duplicate so result is unique list
  virtual void	Copy_Common(const taArray_impl& cp);
  // copy elements in common
  virtual void	Copy_Duplicate(const taArray_impl& cp);
  // copy elements in common, duplicating (if necc) any extra on cp
  virtual void 	CopyVals(const taArray_impl& from, int start=0, int end=-1, int at=0);
  // copy values from other array at given start and end points, and putting at given point in this
  virtual void	List(ostream& strm = cout) const;
  // print out all of the elements in the array
  virtual void	InitFromString(const char* val);
  // initialize an array from given string (does reset first)
};


#include <ta/ta_list_tmplt.h>

#endif // ta_list_h
