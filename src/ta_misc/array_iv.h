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

#ifndef array_iv_h
#define array_iv_h

#include <ta_misc/colorbar.h>
#include <ta/taiv_type.h>
#include <ta/ta_group_iv.h>

class gpivArrayC_Type : public gpivArray_Type {
public:  
  int 		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL); // add the "new" button
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(gpivArrayC_Type, gpivArray_Type);
};


class gpivArrayCEditButton : public gpivArrayEditButton {
  // ##NO_INSTANCE array edit button with coloredit option
public:
  taivEdit*	cive;		// color edit dialog

  gpivArrayCEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  ~gpivArrayCEditButton();

  void		GetMethMenus();
  virtual void	ColorEdit();
};


class gpivArrayCEdit : public gpivArrayEdit {
public:
  int		Edit(void* base, ivWindow* win=NULL, bool wait=false, bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  
  TAIV_EDIT_INSTANCE(gpivArrayCEdit, gpivArrayEdit);
};

#endif // array_iv_h
