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

// implimentation header for the ta list classes

#include <ta/ta_list.h>	

// needed just for the taMisc::display_width variable..
#include <ta/typea.h>

int taPtrList_impl::Idx = 0;
String taPtrList_impl::no_el_name;
taPtrList_impl taPtrList_impl::scratch_list;

void taPtrList_impl::InitList_() {
  hash_table = NULL;
//   alloc_size = 2;
//   el = (void**)calloc(alloc_size, sizeof(void*));
  // start out un-allocated
  alloc_size = 0;
  el = NULL;
  size = 0;
  Dirty();
}  
taPtrList_impl::~taPtrList_impl() {
  Reset();
  if(el != NULL)
    free((char *) el);
  if(hash_table != NULL)
    delete hash_table;
  hash_table = NULL;
}

// allocate by powers of two, minus the amount of overhead required by the 
// memory manger

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

void taPtrList_impl::BuildHashTable(int sz) {
  if(hash_table == NULL)
    hash_table = new taHashTable();

  hash_table->Alloc(sz);
  int i;
  for(i=0; i<size; i++)
    hash_table->Add(El_GetName_(el[i]), i);
}


void taPtrList_impl::AddEl_(void* it) {
  if(size+1 >= alloc_size)
    Alloc(size+1);
  el[size++] = it;
  Dirty();
}

int taPtrList_impl::Find_(void* it) const {
  int i;
  for(i=0; i < size; i++) {
    if(el[i] == it)
      return i;
  }
  return -1;
}
int taPtrList_impl::Find(const char* it) const {
  if(hash_table != NULL)
    return hash_table->FindIndex(it);
  int i;
  for(i=0; i < size; i++) {
    if(El_FindCheck_(el[i], it))
      return i;
  }
  return -1;
}

void* taPtrList_impl::FindName_(const char* it, int& idx) const {
  idx = Find(it);
  if(idx >= 0) return el[idx];
  return NULL;
}

void taPtrList_impl::UpdateIndex_(int idx) {
  if(el[idx] == NULL)
    return;
  if(El_GetOwner_(el[idx]) == this)
    El_SetIndex_(el[idx], idx);
  if(hash_table != NULL)
    hash_table->UpdateIndex((const char*)El_GetName_(el[idx]), idx);
}

bool taPtrList_impl::Move(int fm, int to) {
  if((size == 0) || (fm >= size) || (to >= size)) return false;

  void* itm = el[fm];
  int j;
  for(j=fm; j < size-1; j++) {		// compact, if necc
    el[j] = el[j+1];
    UpdateIndex_(j);
  }
  for(j=size-1; j>to; j--) {
    el[j] = el[j-1];
    UpdateIndex_(j);
  }
  el[to] = itm;
  UpdateIndex_(to);
  Dirty();
  return true;
}

bool taPtrList_impl::MoveBefore_(void* trg, void* item) {
  int trgi = Find_(trg);
  int iti = Find_(item);
  if((trgi < 0) || (iti < 0) || (iti = trgi - 1)) return false;
  return Move(iti, trgi);
}

bool taPtrList_impl::MoveAfter_(void* trg, void* item) {
  int trgi = Find_(trg);
  int iti = Find_(item);
  if((trgi < 0) || (iti < 0) || (iti = trgi + 1)) return false;
  return Move(iti, trgi+1);
}

bool taPtrList_impl::Swap(int pos1, int pos2) {
  if((size == 0) || (pos1 >= size) || (pos2 >= size)) return false;

  void* tmp = el[pos1];
  el[pos1] = el[pos2];
  UpdateIndex_(pos1);
  el[pos2] = tmp;
  UpdateIndex_(pos2);
  Dirty();
  return true;
}
  

/////////////////////////
//        Add          //
/////////////////////////

void taPtrList_impl::Add_(void* it) {
  AddEl_(it);
  if(it != NULL) {
    El_SetIndex_(El_Own_(it), size-1);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(it), size-1);
  }
}

bool taPtrList_impl::AddUnique_(void* it) {
  if(Find_(it) >= 0)
    return false;
  Add_(it);
  return true;
}
bool taPtrList_impl::AddUniqNameNew_(void* it) {
  int i;
  if((i=Find(El_GetName_(it))) >= 0) {
    Replace_(i,it);
    return false;
  }
  Add_(it);
  return true;
}
void* taPtrList_impl::AddUniqNameOld_(void* it) {
  int i;
  if((i=Find(El_GetName_(it))) >= 0) {
    return el[i];
  }
  Add_(it);
  return it;
}
bool taPtrList_impl::Remove(int i) {
  if((size == 0) || (i >= size))
    return false;
  if(el[i] != NULL) {
    if(hash_table != NULL)
      hash_table->Remove((const char*)El_GetName_(el[i]));
    El_disOwn_(el[i]);
  }
  int j;
  for(j=i; j < size-1; j++) {		// compact, if necc
    el[j] = el[j+1];
    UpdateIndex_(j);
  }
  size--;
  Dirty();
  return true;
}
bool taPtrList_impl::Remove_(void* it) {
  int i;
  if((i = Find_(it)) < 0)
    return false;
  return Remove(i);
}
bool taPtrList_impl::Remove(const char* it) {
  int i;
  if((i = Find(it)) < 0)
    return false;
  return Remove(i);
}
bool taPtrList_impl::RemoveLast() {
  if(size == 0) return false;
  return Remove(size-1);
}

void taPtrList_impl::RemoveAll() {
  int i;
  for(i=size-1; i>=0; i--)
    RemoveLast();
}

bool taPtrList_impl::Insert_(void* it, int where) {
  if((where >= size) || (where < 0)) {
    Add_(it);
    return true;
  }
  AddEl_(NULL);
  int i;
  for(i=size-1; i > where; i--) {
    el[i] = el[i-1];
    UpdateIndex_(i);
  }
  el[where] = it;
  if(it != NULL) {
    El_SetIndex_(El_Own_(it), where);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(it), where);
  }
  return true;
}
bool taPtrList_impl::Replace_(void* ol, void* nw) {
  int i;
  if((i = Find_(ol)) < 0)
    return false;
  Replace_(i, nw); 
  return true;
}
bool taPtrList_impl::Replace_(const char* ol, void* nw) {
  int i;
  if((i = Find(ol)) < 0)
    return false;
  return Replace_(i, nw); 
}
bool taPtrList_impl::Replace_(int ol, void* nw) {
  if((size == 0) || (ol >= size))
    return false;
  if(el[ol] != NULL) {
    if(hash_table != NULL)
      hash_table->Remove((const char*)El_GetName_(el[ol]));
    El_disOwn_(el[ol]);
  }
  el[ol] = nw;
  if(nw != NULL) {
    El_SetIndex_(El_Own_(nw), ol);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(nw), ol);
  }
  Dirty();
  return true;
}
bool taPtrList_impl::Transfer_(void* it) {
  if(El_GetOwner_(it) == this)
    return false;
  El_Ref_(it);			// extra ref so no delete on remove
  taPtrList_impl* old_own = El_GetOwner_(it);
  El_SetOwner_(it);		// change owner to us so it doesn't call CutLinks with Remove..
  if(old_own != NULL)
    old_own->Remove_(it);
  Add_(it);
  El_unRef_(it);
  return true;
}


/////////////////////////
//        Link         //
/////////////////////////

void taPtrList_impl::Link_(void* it) {
  AddEl_(it);
  if(it != NULL) {
    El_Ref_(it);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(it), size-1);
  }
}

bool taPtrList_impl::LinkUnique_(void* it) {
  if(Find_(it) >= 0)
    return false;
  Link_(it);
  return true;
}
bool taPtrList_impl::LinkUniqNameNew_(void* it) {
  int i;
  if((i=Find(El_GetName_(it))) >= 0) {
    ReplaceLink_(i,it);	// semantics of LinkUniqName is to update..
    return false;
  }
  Link_(it);
  return true;
}
void* taPtrList_impl::LinkUniqNameOld_(void* it) {
  int i;
  if((i=Find(El_GetName_(it))) >= 0) {
    return el[i];
  }
  Link_(it);
  return it;
}
bool taPtrList_impl::InsertLink_(void* it, int where) {
  if((where >= size) || (where < 0)) {
    Link_(it);
    return true;
  }
  if(size > 0)
    AddEl_(NULL);
  int i;
  for(i=size-1; i>where; i--) {
    el[i] = el[i-1];
    UpdateIndex_(i);
  }
  el[where] = it;
  if(it != NULL) {
    El_Ref_(it);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(it), where);
  }
  return true;
}
bool taPtrList_impl::ReplaceLink_(void* ol, void* nw) {
  int i;
  if((i = Find_(ol)) < 0)
    return false;
  ReplaceLink_(i, nw); 
  return true;
}
bool taPtrList_impl::ReplaceLink_(const char* ol, void* nw) {
  int i;
  if((i = Find(ol)) < 0)
    return false;
  ReplaceLink_(i, nw); 
  return true;
}
bool taPtrList_impl::ReplaceLink_(int ol, void* nw) {
  if((size == 0) || (ol >= size))
    return false;
  if(el[ol] != NULL) {
    if(hash_table != NULL)
      hash_table->Remove((const char*)El_GetName_(el[ol]));
    El_disOwn_(el[ol]);
  }
  el[ol] = nw;
  if(nw != NULL) {
    El_Ref_(nw);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(nw), ol);
  }
  Dirty();
  return true;
}


/////////////////////////
//        Push         //
/////////////////////////

void taPtrList_impl::Push_(void* it) {
  AddEl_(it);
  if(it != NULL) {
    El_Ref_(it);
    if(hash_table != NULL)
      hash_table->Add(El_GetName_(it), size-1);
  }
}

void* taPtrList_impl::Pop_() {
  if(size==0) return NULL;
  void* rval = el[--size];
  if(rval != NULL) {
    if(hash_table != NULL)
      hash_table->Remove((const char*)El_GetName_(rval));
    El_unRef_(rval);
  }
  Dirty();
  return rval;
}

/////////////////////////
//      List-Wise      //
/////////////////////////

void taPtrList_impl::PopAll() {
  int i;
  for(i=size-1; i>=0; i--)
    Pop_();
}

void taPtrList_impl::Permute() {
  int i, nv;
  for(i=0; i<size; i++) {
    nv = (int) ((MTRnd::genrand_int32() % (size - i)) + i); // get someone from the future
    Swap(i, nv);
  }
}

void taPtrList_impl::Sort(bool descending) {
  Sort_(descending);
  UpdateAllIndicies();
}

void taPtrList_impl::Sort_(bool descending) {
  if(size <= 1) return;
  int lt_compval = -1;		// comparison return value for less-than
  if(descending)
    lt_compval = 1;
  // lets do a heap sort since it requires no secondary storage
  int n = size;
  int l,j,ir,i;
  void* tmp;

  l = (n >> 1) + 1;
  ir = n;
  for(;;){
    if(l>1)
      tmp = el[--l -1]; // tmp = ra[--l]
    else {
      tmp = el[ir-1]; // tmp = ra[ir]
      el[ir-1] = el[0]; // ra[ir] = ra[1]
      if(--ir == 1) {
	el[0] = tmp; // ra[1]=tmp
	return;
      }
    }
    i=l;
    j=l << 1;
    while(j<= ir) {
      if(j<ir && (El_Compare_(el[j-1],el[j]) == lt_compval)) j++;
      if(El_Compare_(tmp,el[j-1]) == lt_compval) { // tmp < ra[j]
	el[i-1] = el[j-1]; // ra[i]=ra[j];
	j += (i=j);
      }
      else j = ir+1;
    }
    el[i-1] = tmp; // ra[i] = tmp;
  }
}

void taPtrList_impl::UpdateAllIndicies() {
  int i;
  for(i=0; i<size; i++)  {
    if(El_GetOwner_(el[i]) == this)
      El_SetIndex_(el[i], i);
  }
}


/////////////////////////
//      Duplication    //
/////////////////////////


bool taPtrList_impl::DuplicateEl_(void* it) {
  if(it == NULL)
    return false;
  taMisc::is_duplicating = true;
  void* nw = El_MakeToken_(it);
  Add_(nw);
  El_Copy_(nw, it);
  taMisc::is_duplicating = false;
  return true;
}

void taPtrList_impl::Stealth_Borrow(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  int i;
  for(i=0; i < cp.size; i++) {
    AddEl_(cp.el[i]);
  }
}

// this uses the local FindCheck insteaad of scratch_lists
int taPtrList_impl::Scratch_Find_(const char* it) const {
  int i;
  for(i=0; i < scratch_list.size; i++) {
    if(El_FindCheck_(scratch_list.el[i], it))
      return i;
  }
  return -1;
}

void taPtrList_impl::Duplicate(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  int i;
  for(i=0; i < cp.size; i++) {
    if(cp.el[i] == NULL)
      Add_(NULL);
    else {
      taMisc::is_duplicating = true;
      void* it = El_MakeToken_(cp.el[i]);
      Add_(it);
      El_Copy_(it, cp.el[i]);
      taMisc::is_duplicating = false;
    }
  }
}

// the following are optimized to only find duplicates between the two lists
// and not within a given one.  this makes them considerably more efficient!
// they use the scratch_list for keeping a copy of the old list 

void taPtrList_impl::DupeUniqNameNew(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  scratch_list.size = 0;
  scratch_list.Borrow(*this);	// get this into scratch for find (since replacing
				// we need to refer to these items (no stealth)
  int i;
  for(i=0; i < cp.size; i++) {
    if(cp.el[i] == NULL)  continue;
    taMisc::is_duplicating = true;
    void* it = El_MakeToken_(cp.el[i]);
    int idx;
    if((idx=Scratch_Find_(El_GetName_(cp.el[i]))) >= 0) {
      Replace_(idx,it);
      El_Copy_(it, cp.el[i]);
    }      
    else {
      Add_(it);
      El_Copy_(it, cp.el[i]);
    }
    taMisc::is_duplicating = false;
  }
  scratch_list.Reset();
}
void taPtrList_impl::DupeUniqNameOld(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  scratch_list.size = 0;
  scratch_list.Stealth_Borrow(*this);	// get this into scratch for find
  int i;
  for(i=0; i < cp.size; i++) {
    if(cp.el[i] == NULL)  continue;
    if(Scratch_Find_(El_GetName_(cp.el[i])) < 0) {
      taMisc::is_duplicating = true;
      void* it = El_MakeToken_(cp.el[i]);
      Add_(it);
      El_Copy_(it, cp.el[i]);
      taMisc::is_duplicating = false;
    }
  }
  scratch_list.size = 0;
}

void taPtrList_impl::Borrow(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  int i;
  for(i=0; i < cp.size; i++)
    Link_(cp.el[i]);
}
void taPtrList_impl::BorrowUnique(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  scratch_list.size = 0;
  scratch_list.Stealth_Borrow(*this);	// get this into scratch for find
  int i;
  for(i=0; i < cp.size; i++) {
    void* it = cp.el[i];
    if(scratch_list.Find_(it) < 0)
      Link_(it);
  }
  scratch_list.size = 0;
}
void taPtrList_impl::BorrowUniqNameNew(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  scratch_list.size = 0;
  scratch_list.Borrow(*this);	// get this into scratch for find (using replace..)
  int i;
  for(i=0; i < cp.size; i++) {
    void* it = cp.el[i];
    int idx;
    if((idx=Scratch_Find_(El_GetName_(it))) >= 0)
      ReplaceLink_(idx, it);
    else
      Link_(it);
  }
  scratch_list.Reset();
}
void taPtrList_impl::BorrowUniqNameOld(const taPtrList_impl& cp) {
  Alloc(size + cp.size);
  scratch_list.size = 0;
  scratch_list.Stealth_Borrow(*this);	// get this into scratch for find
  int i;
  for(i=0; i < cp.size; i++) {
    void* it = cp.el[i];
    if(Scratch_Find_(El_GetName_(it)) < 0)
      Link_(it);
  }
  scratch_list.size = 0;
}

void taPtrList_impl::Copy_Common(const taPtrList_impl& cp) {
  int mx_idx = MIN(size,cp.size);
  int i;
  for(i=0; i < mx_idx; i++) {
    if(cp.el[i] == NULL) continue;
    El_Copy_(el[i], cp.el[i]);
  }
}

void taPtrList_impl::Copy_Duplicate(const taPtrList_impl& cp) {
  int mx_sz = MAX(size, cp.size);
  Alloc(mx_sz);
  Copy_Common(cp);
  int i;
  for(i=size; i < cp.size; i++) {
    if(cp.el[i] == NULL)
      Add_(NULL);
    else {
      TALPtr elo = El_GetOwner_(cp.el[i]);
      if((elo == NULL) || (elo == &cp)) {
	taMisc::is_duplicating = true;
	void* it = El_MakeToken_(cp.el[i]);
	Add_(it);
	El_Copy_(it, cp.el[i]);
	taMisc::is_duplicating = false;
      }
      else {
	// if object is not owned by copying list, then it is linked, so link here!
	Link_(cp.el[i]);
      }
    }
  }
}

void taPtrList_impl::Copy_Borrow(const taPtrList_impl& cp) {
  Copy_Common(cp);
  int i;
  for(i=size; i < cp.size; i++)
    Link_(cp.el[i]);
}

ostream& taPtrList_impl::Indenter(ostream& strm, const char* itm, int no, int prln, int tabs)
{
  strm << itm << " ";
  if((no+1) % prln == 0) {
    strm << "\n";
    return strm;
  }
  int len = strlen(itm) + 1;
  int i;
  for(i=tabs; i>=0; i--) {
    if(len < i * 8)
      strm << "\t";
  }
  return strm;
}

void taPtrList_impl::List(ostream& strm) const {
  int i;
  strm << "\nElements of List: " << GetListName_() << " (" << size << ")\n";
  int names_width = 0;
  for(i=0; i<size; i++) {
    if(el[i] == NULL)
      names_width = MAX(names_width, 4);
    else
      names_width = MAX(names_width, (int)El_GetName_(el[i]).length());
  }
  int tabs = (names_width / 8) + 1;
  int prln = taMisc::display_width / (tabs * 8);  if(prln <= 0) prln = 1;
  for(i=0; i<size; i++) {
    if(el[i] == NULL)
      Indenter(strm, "NULL", i, prln, tabs);
    else
      Indenter(strm, El_GetName_(el[i]), i, prln, tabs);
  }
  strm << "\n";
  strm.flush();
}

///////////////////////////
//        Hash Table     //
///////////////////////////

// some of the following code (the hashing function and the prime number list)
// were taken from the COOL object library package, which has the following
// copyright notice:
//
// Copyright (C) 1991 Texas Instruments Incorporated.
//
// Permission is granted to any individual or institution to use, copy, modify,
// and distribute this software, provided that this complete copyright and
// permission notice is maintained, intact, in all copies and supporting
// documentation.
//
// Texas Instruments Incorporated provides this software "as is" without
// express or implied warranty.

int taHashTable::n_bucket_primes[] = 
                     {3, 7, 13, 19, 29, 41, 53, 67, 83, 97, 113, 137,
                      163, 191, 223, 263, 307, 349, 401, 461, 521,
		      653, 719, 773, 839, 911, 983, 1049, 1123, 1201,
		      1279, 1367, 1459, 1549, 1657, 1759, 1861, 1973,
		      2081, 2179, 2281, 2383, 2503, 2617, 2729, 2843,
		      2963, 3089, 3203, 3323, 3449, 3571, 3697, 3833,
		      3967, 4099, 4241, 4391, 4549, 4703, 4861, 5011,
		      5171, 5333, 5483, 5669, 5839, 6029, 6197, 6361,
		      6547, 6761, 6961, 7177, 7393, 7517, 7727, 7951,
		      8101, 8209, 16411, 32771, 65537, 131301, 262147,
		      524287};

int taHashTable::n_primes = 86;

taHashVal taHashTable::HashCode(const char* string) {
  // from TCL, seems to be a tiny bit faster..
  unsigned int hash = 0;
  while (1) {
    unsigned int c = *string;
    string++;
    if (c == 0) {
      break;
    }
    hash += (hash<<3) + c;
  }
  return (taHashVal)hash;

  // from COOL
#if 0
  unsigned int hash = *string++;
  if(*string != '\0') {
    hash = (hash << 7) ^ *string++;
    if (*string != '\0') {
      hash = (hash << 7) ^ *string++;
      if (*string != '\0') {
	hash = (hash << 7) ^ *string++;
	while (*string != '\0') {// rotate hash left 7 bits & xor char
	  int rest = hash >> 25;
	  hash = ((hash << 7) | rest) ^ *string++;
	}
      }
    }
  }
  hash &= 0x7fffffffL;				// Make sure bit 32 is zero
  return (taHashVal)hash;
#endif
}

void taHashTable::InitList_() {
  Alloc(3);
}

void taHashTable::Alloc(int sz) {
  Reset();			// get rid of any existing ones
  bucket_max = 0;
  int act_sz = 0;
  int cnt = 0;
  while((cnt < n_primes) && (act_sz < sz))	act_sz = n_bucket_primes[cnt++];
  taPtrList<taHashBucket>::Alloc(act_sz);
  int i;
  for(i=0; i<act_sz; i++)	// initialize with nulls
    AddEl_(NULL);
}

int taHashBucket::Find(taHashVal hash) const {
  int i;
  for(i=0; i < size; i++) {
    if(FastEl(i)->hash_code == hash)
      return i;
  }
  return -1;
}

int taHashBucket::FindIndex(taHashVal hash) const {
  int i;
  for(i=0; i < size; i++) {
    if(FastEl(i)->hash_code == hash)
      return FastEl(i)->list_idx;
  }
  return -1;
}

int taHashTable::FindIndex(taHashVal hash) const {
  if(size == 0)	return -1;
  int buck_no = (int)(hash % size);
  taHashBucket* bucket = FastEl(buck_no);
  if(bucket == NULL) return -1;
  return bucket->FindIndex(hash);
}

bool taHashTable::UpdateIndex(taHashVal hash, int index) {
  if(size == 0)	return false;
  int buck_no = (int)(hash % size);
  taHashBucket* bucket = FastEl(buck_no);
  if(bucket == NULL) return false;
  int idx = bucket->Find(hash);
  if(idx < 0)    return false;
  bucket->FastEl(idx)->list_idx = index;
  return true;
}

void taHashTable::Add(const char* string, int index) {
  if(size == 0)	return;		// this shouldn't happen, but justin case..
  taHashVal hash = HashCode(string);
  int buck_no = (int)(hash % size);
  taHashBucket* bucket = FastEl(buck_no);
  if(bucket == NULL) {
    bucket = new taHashBucket();
    Replace(buck_no, bucket);
  }
  bucket->Add(new taHashEl(hash, index));
  bucket_max = MAX(bucket_max, bucket->size);
}

bool taHashTable::Remove(taHashVal hash) {
  if(size == 0)	return false;
  int buck_no = (int)(hash % size);
  taHashBucket* bucket = FastEl(buck_no);
  if(bucket == NULL) return false;
  int idx = bucket->Find(hash);
  if(idx == -1)
    return false;
  return bucket->Remove(idx);
}

void taHashTable::RemoveAll() {
  taPtrList<taHashBucket>::RemoveAll();
  bucket_max = 0;
}

/////////////////////////
//        Arrays       //
/////////////////////////


void taArray_impl::Clear_Tmp_() {
  String val;
  El_SetFmStr_(El_GetTmp_(), val);
}

void taArray_impl::Add_(void* it) {
  if(size+1 >= alloc_size) Alloc(size+1);
  El_Copy_(FastEl_(size++), it);
}

bool taArray_impl::AddUnique_(void* it) {
  if(Find_(it) >= 0)
    return false;
  Add_(it);
  return true;
}
  
bool taArray_impl::Remove_(void* it) {
  int i;
  if((i = Find_(it)) < 0)
    return false;
  return Remove(i);
}

void taArray_impl::EnforceSize(int sz) {
  if(size < sz) {
    Clear_Tmp_();
    Insert_(El_GetTmp_(), size, sz - size);
  }
  else if(size > sz)
    size = sz;			// that's easy!
}

bool taArray_impl::Remove(uint i, int n) {
  if((int)i >= size) return false;
  n = MIN(n, size-(int)i);
  int j;
  for(j=i; j < size-n; j++)
    El_Copy_(FastEl_(j), FastEl_(j+n));
  size-=n;
  return true;
}

void taArray_impl::Insert_(void* it, int where, int n) {
  if((where > size) || (n == 0)) return;
  if(size+n >= alloc_size)  Alloc(size + n);	// pre-add stuff
  if((where==size) || (where < 0)) {
    int i;
    for(i=0; i<n; i++) Add_(it);
    return;
  }
  int i;
  int n_mv = size - where;	// number that must be moved
  size += n;
  int trg_o = size-1;
  int src_o = size-1-n;
  for(i=0; i<n_mv; i++)		// shift everyone over
    El_Copy_(FastEl_(trg_o - i), FastEl_(src_o - i));
  for(i=where; i<where+n; i++)
    El_Copy_(FastEl_(i), it);
}

int taArray_impl::Find_(void* it, int where) const {
  int i;
  for(i=where; i<size; i++) {
    if(El_Compare_(it, FastEl_(i)) == 0)
      return i;
  }
  return -1;
}

void taArray_impl::InitVals_(void* it, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  int i;
  for(i=start;i<end;i++) {
    El_Copy_(FastEl_(i), it);
  }
}

bool taArray_impl::Move(int fm, int to) {
  if((size == 0) || (fm >= size) || (to >= size)) return false;

  void* tmp = El_GetTmp_();
  El_Copy_(tmp, FastEl_(fm));
  int j;
  for(j=fm; j < size-1; j++) {		// compact, if necc
    El_Copy_(FastEl_(j), FastEl_(j+1));
  }
  for(j=size-1; j>to; j--) {
    El_Copy_(FastEl_(j), FastEl_(j-1));
  }
  El_Copy_(FastEl_(to), tmp);
  return true;
}

void taArray_impl::Permute() {
  int i, nv;
  void* tmp = El_GetTmp_();
  for(i=0; i<size; i++) {
    nv = (int) ((MTRnd::genrand_int32() % (size - i)) + i); // get someone from the future
    El_Copy_(tmp, FastEl_(i));
    El_Copy_(FastEl_(i), FastEl_(nv));	// swap with yourself
    El_Copy_(FastEl_(nv), tmp);
  }
}

void taArray_impl::Sort(bool descending) {
  if(size <= 1) return;
  int lt_compval = -1;		// comparison return value for less-than
  if(descending)
    lt_compval = 1;
  // lets do a heap sort since it requires no secondary storage
  int n = size;
  int l,j,ir,i;
  void* tmp = El_GetTmp_();

  l = (n >> 1)+1;
  ir = n;
  for(;;){
    if(l>1)
      El_Copy_(tmp,FastEl_(--l -1)); // tmp = ra[--l]
    else {
      El_Copy_(tmp,FastEl_(ir-1)); // tmp = ra[ir]
      El_Copy_(FastEl_(ir-1),FastEl_(0)); // ra[ir] = ra[1]
      if(--ir == 1) {
	El_Copy_(FastEl_(0),tmp); // ra[1]=tmp
	return;
      }
    }
    i=l;
    j=l << 1;
    while(j<= ir) {
      if(j<ir && (El_Compare_(FastEl_(j-1),FastEl_(j)) == lt_compval)) j++;
      if(El_Compare_(tmp,FastEl_(j-1)) == lt_compval) { // tmp < ra[j]
	El_Copy_(FastEl_(i-1),FastEl_(j-1)); // ra[i]=ra[j];
	j += (i=j);
      }
      else j = ir+1;
    }
    El_Copy_(FastEl_(i-1),tmp); // ra[i] = tmp;
  }
}

void taArray_impl::ShiftLeft(int nshift) {
  if((nshift <= 0) || (nshift >= size)) return;

  int i;
  for(i=0; i < size - nshift; i++) {
    El_Copy_(FastEl_(i), FastEl_(i+nshift)); // move left..
  }
  size = size-nshift;		// update the size now..
}

void taArray_impl::ShiftLeftPct(float pct) {
  int nshift = (int)(pct * (float)size);
  if(nshift == 0)    nshift = 1;
  ShiftLeft(nshift);
}

int taArray_impl::V_Flip(int width){
  if (size % width) return false;
  int from_end,from_start;
  for(from_end = size-width,from_start = 0;
      (from_start <from_end);
      from_end -= width, from_start += width){
    int j;
    for(j=0;j<width;j++){
      Add_(FastEl_(from_start+j));
      El_Copy_(FastEl_(from_start+j),FastEl_(from_end+j));
      El_Copy_(FastEl_(from_end+j),FastEl_(size-1));
      Remove(size-1);
    }
  }
  return true;
}

void taArray_impl::Duplicate(const taArray_impl& cp) {
  int i;
  for(i=0; i<cp.size; i++)
    Add_(cp.FastEl_(i));
}

void taArray_impl::DupeUnique(const taArray_impl& cp) {
  int i;
  for(i=0; i<cp.size; i++)
    AddUnique_(cp.FastEl_(i));
}

void taArray_impl::Copy_Common(const taArray_impl& cp) {
  int i;
  int mx_idx = MIN(size, cp.size);
  for(i=0; i<mx_idx; i++)
    El_Copy_(FastEl_(i), cp.FastEl_(i));
}

void taArray_impl::Copy_Duplicate(const taArray_impl& cp) {
  Copy_Common(cp);
  int i;
  for(i=size; i<cp.size; i++)
    Add_(cp.FastEl_(i));
}

void taArray_impl::CopyVals(const taArray_impl& from, int start, int end, int at) {
  if(end == -1)	end = from.size;  else end = MIN(from.size, end);
  int len = end - start;
  if(size < at + len)
    EnforceSize(at + len);
  int i, trg;
  for(i=start, trg=at;i<end;i++, trg++)
    El_Copy_(FastEl_(trg), from.FastEl_(i));
}

void taArray_impl::List(ostream& strm) const {
  strm << "[" << size << "] {";
  int i;
  for(i=0;i<size;i++) {
    strm << " " << El_GetStr_(FastEl_(i)) << ",";
  }
  strm << "}";
}

void taArray_impl::InitFromString(const char* val) {
  String tmp = val;
  Reset();
  String tmp2;
  tmp2 = tmp.after("{");		// find starting point
  while(tmp.length() > 0) {
    String el_val = tmp.before(',');
    if(el_val.empty()) {
      el_val = tmp.before('}');
      if(el_val.empty())
	break;
    }
    tmp = tmp.after(',');
    if(el_val.contains(' '))
      el_val = el_val.after(' ');
    Add_(El_GetTmp_());		// add a blank
    El_SetFmStr_(SafeEl_(size-1), el_val);
  }
}
