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

#ifndef datatable_tmplt_h
#define datatable_tmplt_h

#include <ta_misc/datatable.h>

template<class T> class DataArray : public DataArray_impl {
public:
  T*		ar;		// #NO_SAVE the array itself

  taArray_base* AR()		 { return ar; }
  void		NewAR()		
  { taBase::OwnPointer((TAPtr*)&ar, new T, this); }
  void		SetAR(taArray_base* nar)
  { taBase::SetPointer((TAPtr*)&ar, nar); }

  void	Initialize()		{ ar = NULL; }
  void	Destroy()		{ CutLinks(); }
  void	InitLinks() 		{ NewAR(); }
  void	CutLinks()		
  { DataArray_impl::CutLinks(); taBase::DelPointer((TAPtr*)&ar); }
  void	Copy_(const DataArray<T>& cp)  { if(ar == NULL) NewAR(); *ar = *cp.ar; }
  COPY_FUNS(DataArray<T>, DataArray_impl);
  TA_TMPLT_BASEFUNS(DataArray, T);
};

class float_Data : public DataArray<float_RArray> {
  // #NO_UPDATE_AFTER floating point data
public:
  void	Initialize()		{ };
  void	Destroy()		{ };
  TA_BASEFUNS(float_Data);
};

class String_Data : public DataArray<String_Array> {
  // #NO_UPDATE_AFTER string data
public:
  void	Initialize()		{ };
  void	Destroy()		{ };
  TA_BASEFUNS(String_Data);
};

#endif // datatable_tmplt_h
