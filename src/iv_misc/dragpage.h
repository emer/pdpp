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


// dragpage.h

#include <InterViews/drag.h>
#include <IV-look/kit.h>
#include <InterViews/layout.h>
#include <InterViews/page.h>

///////////////////////////////////////////////////
// Page that can have things moved around in it. //
///////////////////////////////////////////////////

class DragPage : public DragZone {
public:
  // pointer to function to call when an object is dropped
  void (*DropUpdate)(int* dropupdateobject, int dropindex) ;
  int*  dropupdateobject;

  DragPage(WidgetKit* kit,LayoutKit* layout,Glyph* g=nil); // usually g is a BackGround

  virtual void enter(Event&, const char* type, int length);
  virtual void leave(Event&);
  virtual void motion(Event&);

  virtual void drop(Event&, const char* data, int);

  virtual void allocate(Canvas*, const Allocation&, Extension&);
  virtual void reallocate();
  void SetDragObject(Drag*);
  void ToggleEdit();
  int Editable();
  GlyphIndex GetPos(Glyph*);
  Patch* GetPatch();
  Page*  GetPage();

  virtual void AlignToLeftMost();
  virtual void AlignToTopMost();
  virtual void AlignToRightMost();
  virtual void AlignToBottomMost();
  virtual void AutoAlign();
private:
  Glyph* dragged_object;
  int edit_;
  Patch* patch_;
  Page*  page_;
  LayoutKit* layout_;
  WidgetKit* kit_;
  Canvas* canvas_;
  Allocation allocation_;
  Extension extension_;
};

declareActionCallback(DragPage)

//////////////////////////////////////////////////
// Glyph that can be dragged around a dragpage. //
//////////////////////////////////////////////////

class DragPageObject : public Drag {
public:
    DragPageObject(DragPage* p,WidgetKit* kit, boolean useCursor, boolean useGlyph,Glyph* g);

    virtual Cursor* dragCursor();
    virtual Glyph* dragGlyph();
    virtual void dragData(char*& value, int& length);
    virtual Glyph* glyph();
    virtual boolean caught(const Event&) const;
    virtual void pick(Canvas*, const Allocation&, int depth, Hit&);
    virtual void dragOffset(Event& event, int& dx, int& dy);
    virtual void allocate(Canvas*, const Allocation&, Extension&);
    virtual Allocation* GetAllocation();
    virtual ivCoord GetDiffx();
    virtual ivCoord GetDiffy();
protected:
    DragPage* dragpage_;
    char mypos[10];
    WidgetKit* kit_;
    boolean useCursor_;
    boolean useGlyph_;
    Allocation allocation_;
    ivCoord diffx; // offset from object location from grabbed location
    ivCoord diffy; 
};

