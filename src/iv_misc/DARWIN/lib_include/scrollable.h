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

#ifndef scrollable_h
#define scrollable_h

#include <ta/enter_iv.h>
#include <InterViews/adjust.h>
#include <InterViews/patch.h>

class TformSetter;

// Scrollable tries to limit the scrolled glyph both horizontally
// and vertically to the size alloacated to the scrollable. 
// It scrolls a region which is the size of the object's request


class Scrollable : public Patch, public Adjustable {
public:
  Scrollable(Glyph* g, Coord w=0, Coord h=0, Coord dx=0, Coord dy=0);
  Scrollable(Patch* p) : Patch(p){};

  Scrollable(TformSetter* t) : Patch((Glyph *) t){};

  virtual ~Scrollable();

  virtual void draw(Canvas*, const Allocation&) const;
  virtual void pick(Canvas*, const Allocation&, int depth, Hit&);
  virtual void reallocate();

  virtual void request(Requisition&) const;
  virtual void allocate(Canvas*, const Allocation&, Extension&);

  
  virtual Coord lower(DimensionName) const;
  virtual Coord upper(DimensionName) const;
  virtual Coord length(DimensionName) const;
  virtual Coord cur_lower(DimensionName) const;
  virtual Coord cur_upper(DimensionName) const;
  virtual Coord cur_length(DimensionName) const;

  virtual void scroll_to(DimensionName, Coord lower);
  virtual void scale_to(DimensionName, float fraction_visible);
  virtual void zoom_to(float magnification);

  virtual float scale();

  virtual void scroll_scr(Coord cnew) { scroll_incr_ = cnew; }
  virtual void page_scr(Coord cnew) { page_incr_ = cnew; }

  virtual void scroll_forward(DimensionName d) ;
  virtual void scroll_backward(DimensionName d);
  virtual void page_forward(DimensionName d);
  virtual void page_backward(DimensionName d);

  Coord width_, height_;
  Coord scale_;
  Coord dx_, dy_;
  Coord scroll_incr_;
  Coord page_incr_;

  Requisition requisition_;
  TformSetter* xform_;
};


// HScrollable allows the scrolled glyph to be its natural size horizontally,
// but limits its vertical size, to the size requested


class HScrollable : public Scrollable {
public:
  HScrollable(Glyph* g, Coord w=0, Coord h=0, Coord dx=0, Coord dy=0);
  virtual ~HScrollable();
  virtual void request(Requisition&) const;
};

// tformsetter

#include <InterViews/monoglyph.h>
#include <InterViews/transformer.h>

class TformSetter : public MonoGlyph {
public:
  TformSetter(Glyph*);
  TformSetter(Glyph*, const Transformer&);
  virtual ~TformSetter();

  virtual const Transformer& transformer() const;
  virtual Transformer& transformer();
  virtual void transformer(const Transformer&);

  virtual void request(Requisition&) const;
  virtual void allocate(Canvas*, const Allocation&, Extension&);
  virtual void draw(Canvas*, const Allocation&) const;
  virtual void print(Printer*, const Allocation&) const;
  virtual void pick(Canvas*, const Allocation&, int depth, Hit&);
  virtual void transform(
			 Transformer&, const Allocation&, const Allocation& natural
			 ) const;
  Transformer transformer_;
  Allocation natural_allocation_;

  void push_transform(
		      Canvas*, const Allocation&, const Allocation& natural
		      ) const;

  virtual void SetFullAllocation(Allocation* b) const;
};


class HTformSetter : public TformSetter {
public:
  HTformSetter(Glyph*);
  HTformSetter(Glyph*, const Transformer&);
  virtual ~HTformSetter();
  virtual void SetFullAllocation(Allocation* b) const;
};

#include <ta/leave_iv.h>

#endif
