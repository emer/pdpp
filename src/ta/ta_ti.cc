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

// regexp for template searching: '<[a-z|A-Z|_]\{2,\}>'

// ta_ti.cc: template instantiation file for ta library

#include <ta/typea.h>
#ifndef NO_TA_BASE
#include <ta/ta_group.h>
#include <ta/ta_dump.h>
#ifndef NO_IV
#include <ta/ta_group_iv.h>
#endif
#endif

template class taPtrList<taHashEl>;
template class taPtrList<taHashBucket>;
template class taPlainArray<String>;
template class taPlainArray<int>;
template class taPtrList<EnumDef>;
template class taPtrList<void>;
template class taPtrList<MemberDef>;
template class taPtrList<MethodDef>;
template class taPtrList<TypeDef>;

#ifndef NO_TA_BASE
template class taPtrList_base<taBase>;
template class taList<taBase>;
template class taArray<int>;
template class taArray<float>;
template class taArray<double>;
template class taArray<String>;
template class taArray<long>;
template class taArray<void*>;
template class taPtrList<VPUnref>;
template class taPtrList<DumpPathSub>;
template class taPtrList<DumpPathToken>;

template class taList<taGroup_impl>;
template class taGroup<taBase>;

#ifndef NO_IV
template class taPtrList<ivGlyph>;
template class taPtrList<ivWindow>;
template class taPtrList<taivDialog>;
template class taPtrList<taivEditDialog>;
template class taPtrList<taivData>;
template class taPtrList<taivMenuEl>;
template class taPtrList<taivMenu>;
template class taPtrList<taivHierSub>;
template class taPtrList<taivHierMenu>;
template class taPtrList<taivType>;
template class taPtrList<gpivList_ElData>;
#endif
#endif
