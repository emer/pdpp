/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the PDP++ software package.			      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted	      //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//									      //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions. 	HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
// 									      //
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

#ifndef spec_tmplt_h
#define spec_tmplt_h 1

#include <pdp/spec.h>

// this template is type-specific

template<class T> class SpecPtr : public SpecPtr_impl {
public:
  T*		spec;		// the actual spec itself

  BaseSpec*	GetSpec() const		{ return spec; } 
  void		SetSpec(BaseSpec* es)   {
    if((es == NULL) || (es->InheritsFrom(base_type) && es->CheckObjectType(owner))) {
      taBase::SetPointer((TAPtr*)&spec,es);
      if(es != NULL) type = es->GetTypeDef();
    }
    else {
      taMisc::Error("SetSpec: incorrect type of Spec:",
		    es->GetPath(), "of type:", es->GetTypeDef()->name,
		    "should be at least:", base_type->name,"in object:",owner->GetPath());
    }
  }
  bool		CheckSpec(TAPtr own_obj)   {
    if(own_obj == NULL) return false;
    if(spec == NULL) {
      taMisc::Error("CheckSpec: spec is NULL in", own_obj->GetPath());
      return false;
    }
    if(!spec->InheritsFrom(base_type)) {
      taMisc::Error("CheckSpec: incorrect type of spec:", spec->GetPath(),
		    "of type:", spec->GetTypeDef()->name,
		    "should be at least:", base_type->name,"in object:",own_obj->GetPath());
      return false;
    }
    return spec->CheckObjectType(own_obj);
  }
  bool		CheckSpec() { return CheckSpec(owner); }

  void		SetDefaultSpec(TAPtr ownr, TypeDef* td) 
  { SpecPtr_impl::SetDefaultSpec(ownr, td); }
  void		SetDefaultSpec(TAPtr ownr)
  { SetDefaultSpec(ownr, T::StatTypeDef(1)); }

  virtual T* 	NewChild()
  {  T* rval = (T*)spec->children.NewEl(1); rval->UpdateSpec(); return rval; }

  T* 		operator->() const	{ return spec; }
  T* 		operator=(T* cp)	{ SetSpec(cp); return cp; }
  bool 		operator!=(T* cp) const	{ return (spec != cp); }
  bool 		operator==(T* cp) const	{ return (spec == cp); }

  operator T*()	const		{ return spec; }
  operator BaseSpec*() const	{ return spec; }

  void 	Initialize()		{ spec = NULL; }
  void	Destroy()		{ CutLinks(); }
  void  CutLinks()		
  { SpecPtr_impl::CutLinks(); taBase::DelPointer((TAPtr*)&spec); }
  void	Copy_(const SpecPtr<T>& cp)	
  { taBase::SetPointer((TAPtr*)&spec, cp.spec); UpdateAfterEdit(); }
  COPY_FUNS(SpecPtr<T>, SpecPtr_impl);
  TA_TMPLT_BASEFUNS(SpecPtr, T);
};


#define SpecPtr_of(T)							      \
class T ## _SPtr : public SpecPtr<T> {					      \
public:									      \
  void 	Initialize() 		{ };					      \
  void	Destroy()		{ };					      \
  TA_BASEFUNS(T ## _SPtr);						      \
}


#endif // spec_h



