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

/*
 * HungryBox
 */

#ifndef hungrybox_h
#define hungrybox_h 1

#include <ta/enter_iv.h>
#include <InterViews/polyglyph.h>
#include <InterViews/monoglyph.h>


class HungryBox : public PolyGlyph { // #IGNORE
public:
  HungryBox(int* i, int (*more)(int*,int, HungryBox*),
	    int (*less)(int*,int,HungryBox*),GlyphIndex size = 10);
  virtual ~HungryBox();

  virtual GlyphIndex first_shown() const;
  virtual GlyphIndex last_shown() const;

  int*	feeder;

  int (*ShowMore)(int*, int, HungryBox*);	// function which adds int to this
  int (*ShowLess)(int*, int, HungryBox*);	// function which adds int to this
};

class TBHungryBoxImpl; // #IGNORE


class TBHungryBox : public HungryBox { // #IGNORE
public:
  TBHungryBox(int*, int (*more)(int*, int, HungryBox*),
	      int (*less)(int*,int,HungryBox*),GlyphIndex size = 10);
  virtual ~TBHungryBox();

  virtual void remove(GlyphIndex);

  virtual void request(Requisition&) const;
  virtual void allocate(Canvas*, const Allocation&, Extension&);
  virtual void draw(Canvas*, const Allocation&) const;
  virtual void print(Printer*, const Allocation&) const;
  virtual void pick(Canvas*, const Allocation&, int depth, Hit&);
  virtual void undraw();

  virtual void modified(GlyphIndex);
  virtual boolean shown(GlyphIndex) const;
  virtual GlyphIndex first_shown() const;
  virtual GlyphIndex last_shown() const;
  virtual void allotment(GlyphIndex, DimensionName, Allotment&) const;

  virtual Coord lower(DimensionName) const;
  virtual Coord upper(DimensionName) const;
  virtual Coord length(DimensionName) const;
  virtual Coord cur_lower(DimensionName) const;
  virtual Coord cur_upper(DimensionName) const;
  virtual Coord cur_length(DimensionName) const;
  virtual int   est_size();
				       
  TBHungryBoxImpl* impl_;

  TBHungryBoxImpl& impl() const;
};

class ShapeOfx : public MonoGlyph {
public:
  ShapeOfx(Glyph* theglyph,Glyph* theshape);
  virtual ~ShapeOfx();
  virtual void request(Requisition&) const;
private:
  Glyph* x_;
  
};


#include <ta/leave_iv.h>
#endif
