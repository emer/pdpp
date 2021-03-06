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

#ifndef tdefaults_h
#define tdefaults_h

#include <ta/typea.h>
#include <ta/ta_base.h>

class NameValue : public taNBase {
  // #INSTANCE #NO_TOKENS #NO_UPDATE_AFTER name/value pair
public:
  String 	value;			// Value for name

  void 	Copy_(const NameValue& cp)		{ value = cp.value; }
  COPY_FUNS(NameValue, taNBase);
  TA_BASEFUNS(NameValue);
};

#define MAX_DFT_LONGS (int)((256 / (sizeof(long) * 8)) + 1)

class TypeDefault : public taNBase {
  // ##EXT_def #INSTANCE #NO_TOKENS #NO_UPDATE_AFTER Contains a default object of a type
public:
  TypeDef*	old_type;	// #HIDDEN #NO_SAVE has previous type (if default_type changes)
  TypeDef*	default_type;	// #TYPE_taBase type of the default
  TAPtr		token;		// #DEFAULT_EDIT #NO_SAVE token which has default values
  taBase_List 	active_membs;	// MemberName / MemberValue pairs
  unsigned long active[MAX_DFT_LONGS]; // #HIDDEN #NO_SAVE bits representing a memberdefs activity in a default

  virtual void	SetActive(int memb_no, bool onoff); // set active bit
  virtual void	SetActive(char* memb_nm, bool onoff); // set active flag for member
  virtual bool	GetActive(int memb_no);              // check active bit
  virtual bool 	GetActive(char* memb_nm);	     // check active flag for member
  virtual void	UpdateToNameValue();		   // set the name value data from token
  virtual void	UpdateFromNameValue();		   // set the token form the name values
  virtual void	SetTypeDefaults()		{ taNBase::SetTypeDefaults(); }
  virtual void	SetTypeDefaults(TAPtr tok);	   // set defaults for a given token

  int	Dump_Load_Value(istream& strm, TAPtr par=NULL);

  void	UpdateAfterEdit();
  void  Initialize();
  void	InitLinks();
  void	Destroy();
  TA_BASEFUNS(TypeDefault);
};


#endif // tdefaults_h
