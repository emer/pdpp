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

//
// adjust.h
//
// a movable resizing bar in a box 


#include <InterViews/bitmap.h>
#include <InterViews/cursor.h>
#include <InterViews/patch.h>
#include <InterViews/input.h>
#include <InterViews/polyglyph.h>
#include <InterViews/patch.h>
#include <IV-look/kit.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>

// Adjustwrapper
// Wrab everthing you need to adjust in the box with one of these
//

class AdjustWrapper : public Patch {
public:
 void NewSize(ivCoord size);
 void DeltaAdjust(ivCoord amount);
 void request(Requisition&) const;
 void modified(GlyphIndex i);
 AdjustWrapper(Glyph *);
 void SetAdjusting();
 void SubRequest(Requisition&) const;
private:
 ivCoord newsize;
 int adjusting;
};


// put these inbetween other items as needed
// They dont do anything if they are the first or
// last item in the box


class AdjustBar : public ActiveHandler {
public:
  AdjustBar(PolyGlyph* p, Patch* pt, WidgetKit* kit,LayoutKit* lay,Style* style);
  void drag(const Event&);
  void press(const Event&);
  void release(const Event&);
  void enter();
  void leave();
  void GetMyIndex();
private:
  static Cursor* grabber_cursor;
  Cursor* oldcursor; // storage for previous cursor
  PolyGlyph* poly_;
  Patch* patch_;
  Coord startx;
  Coord starty;
  int myindex;
};

// this is the pointer icon for the bar mover

#include "grabber.bm"
#include "grabberMask.bm"
