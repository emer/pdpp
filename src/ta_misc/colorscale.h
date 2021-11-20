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

#ifndef colorscale_h
#define colorscale_h

#include <ta/ta_group.h>
#include <ta_misc/ta_misc_TA_type.h>
#include <ta_misc/win_base.h>

class ivColor;			// #IGNORE
class ivBrush;			// #IGNORE

class RGBA : public taNBase {
  // ##INLINE ##NO_TOKENS Red Green Blue Alpha color specification
public:
  float	r;			// red
  float	g;			// green
  float	b;			// blue
  float	a;			// alpha (intensity, ratio of fg to bg)
  String desc;			// description of what this color is

  void  Initialize();
  void 	Destroy()		{ };
  void 	Copy_(const RGBA& cp);
  void  UpdateAfterEdit();
  COPY_FUNS(RGBA, taNBase);
  TA_BASEFUNS(RGBA);
  RGBA(float rd, float gr, float bl, float al=1);
};

class RGBA_List : public taList<RGBA> {
  // ##NO_TOKENS #NO_UPDATE_AFTER list of RGBA objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(RGBA_List);
};

class TAColor : public taBase { // ##NO_TOKENS Color
public:
  ivColor* 	color;		// #IGNORE
  ivColor*	contrastcolor;	// #IGNORE
  bool SetColor(float r,float g, float b, float a=1.0,
		RGBA* background=NULL,
		float cr=-1,float cg=1.0,float cb=1.0,float ca=1.0);
  // #USE_RVAL #ARGC=4 #NEW_FUN
  bool SetColor(RGBA* c, RGBA* background= NULL, RGBA* cc=NULL);
  // #IGNORE
  bool SetColor(ivColor* c=NULL, RGBA* background = NULL,ivColor* cc=NULL);
  // #IGNORE
  void Initialize()	{ color = NULL; contrastcolor = NULL;}
  void Destroy();
  TA_BASEFUNS(TAColor);
};

class TAColor_List : public taList<TAColor> {
  // ##NO_TOKENS #NO_UPDATE_AFTER list of TAColor objects
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(TAColor_List);
};

class ColorScaleSpec : public taNBase { // Color Spectrum Data
public:
  RGBA		background;	// background color
  RGBA_List	clr;		// group of colors

  virtual void	GenRanges(TAColor_List* cl, int chunks);

  void 	Initialize();
  void 	Destroy()		{ };
  void 	InitLinks();
  void 	Copy_(const ColorScaleSpec& cp);
  COPY_FUNS(ColorScaleSpec, taNBase);
  TA_BASEFUNS(ColorScaleSpec);
};

class ColorScaleSpec_MGroup : public MenuGroup_impl {
public:
  virtual void 		NewDefaults(); 	// create a set of default colors
  virtual void		SetDefaultColor();// set the default color based on gui
  
  void 	Initialize()	{ };
  void 	Destroy()	{ };

  TA_BASEFUNS(ColorScaleSpec_MGroup);
};

class ivBitmap;			// #IGNORE
class ivColor;			// #IGNORE

class ColorScale : public taNBase {
  // ##NO_TOKENS ##NO_UPDATE_AFTER defines a range of colors to code data values with
public:
  int			chunks;		// number of chunks to divide scale into
  ColorScaleSpec* 	spec;		// specifies the color ranges

  TAColor_List		colors;		// #IGNORE the actual colors
  TAColor		background; 	// #IGNORE background color
  
  TAColor		maxout;		// #IGNORE
  TAColor		minout;		// #IGNORE
  TAColor		nocolor;	// #IGNORE

  virtual ivColor*		GetColor(int idx);  
  virtual ivColor*		GetContrastColor(int idx);
  virtual const ivColor*	Get_Background(); // #IGNORE
  void				DefaultChunks();

  virtual void		MapColors(); 	// generates the colors from spec
  virtual void 		NewDefaults();

  void 	Initialize();
  void 	Destroy();
  void	InitLinks();
  void	CutLinks();
  void  UpdateAfterEdit();
  TA_BASEFUNS(ColorScale);
  ColorScale(int chunk);
};

#endif // colorscale_h



