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

#ifndef font_spec_h
#define font_spec_h

#include <ta/ta_group.h>
#include <ta_misc/ta_misc_TA_type.h>
#include <iv_graphic/iv_graphic_TA_type.h>	// this makes Xform externally defined..

class NoScale_Text_G; //#IGNORE
class ivFont; // #IGNORE
 
class FontSpec : public taBase {
 // #INLINE a specification of a font (for Unix - X logical font description)
public:
  String		pattern; // the name of the font
  String		prv_pat; // #READ_ONLY #NO_SAVE the previous font that was successfully obtained
  ivFont*		fnt;	 // #IGNORE the actual font
  NoScale_Text_G*	text_g;  // #IGNORE an associated text glyph that can be updated if font changes

  virtual void	XFontSel();
  // #BUTTON launch xfontsel program to find desired font pattern
  virtual void 	SetFont(char* fn);
  virtual void	SetFontSize(int point_size = 10);
  // #BUTTON set font to given point size
  virtual void	SetTextG(NoScale_Text_G* txg); // set the text_g to a new one

  void	UpdateAfterEdit();
  void	CutLinks();
  void	InitLinks();
  void	Initialize();
  void	Copy_(const FontSpec& cp);
  COPY_FUNS(FontSpec,taBase);
  TA_BASEFUNS(FontSpec);
};


class Xform;
class GraphicMaster;		
class GlyphViewer;		
class ivColor;

class ViewLabel : public taNBase {
  // contains a label in a view display
public:
  FontSpec	 spec;		// #EDIT_INLINE specification for the font
  Xform*	 label_xform;	// #HIDDEN xform for name label
  GraphicMaster* master;	// #IGNORE associated graphic master
  GlyphViewer*   viewer;	// #IGNORE associate viewer
  void		(*select_effect)(void*);
  // #IGNORE function to call when text has been selected/unselected
  ivColor*	(*get_color)(void *);
  // #IGNORE function to call to get background contrast color

  virtual void	SetLabelXform(Xform* xf); // #HIDDEN
  virtual void	XFontSel();
  // #BUTTON launch xfontsel program to find desired font pattern
  virtual void	GetMasterViewer()	{ };
  // have to create an overloaded one of these for each type of view label..
  virtual void	MakeText();	// make text object and insert into view
  virtual void	AddToView();	// add text object to view
  virtual bool	UpdateView();	// update the view with any changes
  virtual void  RemoveFromView(); // remove from view

  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	CutLinks();
  void	UpdateAfterEdit();
  void	Copy_(const ViewLabel& cp);
  COPY_FUNS(ViewLabel, taNBase);
  TA_BASEFUNS(ViewLabel);
};

class ViewLabel_List : public taList<ViewLabel> {
  // ##NO_TOKENS #NO_UPDATE_AFTER list of ViewLabel objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(ViewLabel_List);
};

#endif // font_spec_h
