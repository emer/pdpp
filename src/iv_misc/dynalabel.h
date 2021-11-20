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

#ifndef dynalabel_h
#define dynalabel_h


#include <ta/enter_iv.h>
#include <InterViews/glyph.h>
#include <OS/string.h>
#include <ta/leave_iv.h>

class ivColor;
class ivFont;


// taken from InterViews/label.h

class DynamicLabel : public ivGlyph { 
public:
  DynamicLabel(const osString&, const ivFont*, const ivColor*, const char* = nil);
  DynamicLabel(const char*, const ivFont*, const ivColor*, const char*form= nil);
  DynamicLabel(float, const ivFont*, const ivColor*, const char* form= nil);
  virtual ~DynamicLabel();

  virtual void request(ivRequisition&) const;
  virtual void allocate(ivCanvas*, const ivAllocation&, ivExtension&);
  virtual void draw(ivCanvas*, const ivAllocation&) const;
  virtual void pick(ivCanvas*, const ivAllocation&, int depth, ivHit&);
  virtual void undraw();

  virtual void set(const char*);
  virtual void set(float);
  virtual void SetColor(const ivColor* col);
protected:    
  void compute_metrics();
protected:
  osString* _text;
  const ivFont* _font;
  const ivColor* _color;
  ivCoord _left;
  ivCoord _right;
  ivCoord _ascent;
  ivCoord _descent;
  ivCoord _width;
  ivCoord* _char_widths;
  char* _format;
  ivCanvas* _c;
  ivAllocation _a;
};

#endif // dynalabel






