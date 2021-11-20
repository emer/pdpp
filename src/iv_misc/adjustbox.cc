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

// AdjustBox.c
// The adjustbox is a set of tools for use in a polyglyph
// These tools allow the dynamic adjusting of the natural size of
// components in a box. Each component must be wrapped in an
// AdjustWrapper. Two AdjustWrapper components may contain an
// AdjustBar between them which will allow the user to adjust
// the natural size of each of the two objects on either side
// of the adjust bar. You also need to create a patch around
// the polyglyph and include that dursing the creation of the
// adjustbar so it can redraw as it moves. 
// Note: The adjust bar only adjusts the objects to the extent
// of their "stretchability" and "shrinkability"

// Example

// PolyGlyph* vb = layout->vbox(3); // Create a polyglyph
// Patch* p = new Patch(vb);        // Create a patch around the polyglyph
//
// // create a Button component
//
// Glyph* g1 = (Glyph*) layout->vflexible(kit->push_button("Hello",NULL),100,50));
//
// vb->append(new AdjustWrapper(g1)); // insert the first button
//
// Create an adjust bar
// AdjustBar* ab = new AdjustBar(vb,p,kit,layout,style); 
//
// vb->append(ab); // insert the adjust bar (it does not need an adjustwrapper)
//
// 
// // create a second Button component with some flexibility
//
// Glyph* g2 = (Glyph*) layout->vflexible(kit->push_button("Goodbye",NULL),100,50));
//
// vb->append(new AdjustWrapper(g1)); // insert the first button
//

#include "adjustbox.h"

#include <InterViews/canvas.h>
#include <InterViews/window.h>
#include <InterViews/event.h>
#include <InterViews/cursor.h>

#ifndef MIN
 #define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

Cursor* AdjustBar::grabber_cursor = nil;

AdjustWrapper::AdjustWrapper(Glyph* g) 
: Patch(g) {newsize = -1; adjusting = 0;}

void AdjustWrapper::SubRequest(Requisition& req) const {
  MonoGlyph::request(req);
}


void AdjustWrapper::modified(GlyphIndex i) {
  Patch::change(i);
  if(body() != nil) body()->change(i);
}


void AdjustWrapper::request(Requisition& req) const {
  Patch::request(req);
  Coord ynat = req.y_requirement().natural();
  Coord ystretch = req.y_requirement().stretch();
  Coord yshrink = req.y_requirement().shrink();
  AdjustWrapper* aw = (AdjustWrapper *) this;
  Coord& ns = aw->newsize;
  if(ns < 0 ) ns = ynat;
  req.y_requirement().natural(ns);
  if(adjusting) {
    req.y_requirement().stretch(0);
    req.y_requirement().shrink(0);
  }
  else {
    Coord newstretch = MAX(0,ynat + ystretch - ns);
    req.y_requirement().stretch(newstretch);
    req.y_requirement().shrink(ns - (ynat - yshrink));
  }
}
	    
void AdjustWrapper::NewSize (ivCoord size) { newsize = size;}

void AdjustWrapper::SetAdjusting(){
  adjusting = ! adjusting;
}


//////////////////
/// Adjust Bar ///
//////////////////

AdjustBar::AdjustBar(PolyGlyph* p, Patch* pt, WidgetKit* kit,LayoutKit* lay,Style* style) 
//: ActiveHandler(lay->flexible(kit->menu_item_separator_look()),style) {
: ActiveHandler(lay->vflexible(kit->inset_frame(lay->vbox(lay->hglue(),lay->vglue(3))),
				  0,0),	style) {
  poly_ = p; patch_ = pt; starty = startx = 0; myindex = 0;
  if(grabber_cursor == nil) {
    Bitmap* grabber_bitmap =
      new Bitmap(grabber_bits, grabber_width, grabber_height,grabber_x_hot,grabber_y_hot);
    Bitmap* grabberMask_bitmap = 
      new Bitmap(grabberMask_bits, grabberMask_width, grabberMask_height,
		 grabberMask_x_hot,grabberMask_y_hot);
    grabber_cursor = new Cursor(grabber_bitmap,grabberMask_bitmap);
  }
  oldcursor = nil;
}

void AdjustBar::enter(){
  if(oldcursor == nil) {
    Window* win = canvas()->window();
    oldcursor = win->cursor();
    win->cursor(grabber_cursor);
  }
}

void AdjustBar::leave(){
  if(oldcursor != nil)    canvas()->window()->cursor(oldcursor);
  oldcursor = nil;
}


void AdjustBar::release(const Event&) {
  starty = startx = 0;
  ((AdjustWrapper *) poly_->component(myindex -1))->SetAdjusting(); // turn off adjusting
  ((AdjustWrapper *) poly_->component(myindex -1))->modified(0); // turn off adjusting
  ((AdjustWrapper *) poly_->component(myindex +1))->SetAdjusting();
  ((AdjustWrapper *) poly_->component(myindex +1))->modified(0);
  poly_->modified(myindex -1);
  poly_->modified(myindex +1);
  patch_->reallocate();
  patch_->redraw();
  leave(); // remove adjust cursor
}

void AdjustBar::GetMyIndex(){
  GlyphIndex c = poly_->count();
  myindex = 0;
  for(int i=0;i<c;i++){
    if(poly_->component(i) == this) {  myindex = i; break; }
  }
}

void AdjustBar::press(const Event& e){
  GetMyIndex();
  AdjustWrapper* adj1 = (AdjustWrapper *) poly_->component(myindex -1);
  AdjustWrapper* adj2 = (AdjustWrapper *) poly_->component(myindex +1);
  if((adj1 == nil) || (adj2 == nil)) return;
  switch (e.pointer_button()) {
  case Event::left:
    startx = e.pointer_x();
    starty = e.pointer_y();
    adj1->SetAdjusting();
    adj2->SetAdjusting();
    break;
  case Event::middle:
    adj1->NewSize(-1); adj2->NewSize(-1);
    poly_->modified(myindex -1);    poly_->modified(myindex +1);
    patch_->reallocate();
    patch_->redraw();
  case Event::right:
    return;
  }
}

void AdjustBar::drag(const Event& e) {
  ActiveHandler::move(e);
  if(oldcursor == nil) enter(); // ensure adjusting cursor
  GlyphIndex c = poly_->count();
  if((myindex < 1) || (myindex > c-2)) return;

  AdjustWrapper* ad1 = (AdjustWrapper*) poly_->component(myindex -1);
  AdjustWrapper* ad2 = (AdjustWrapper*) poly_->component(myindex +1);
  Requisition req1;
  ad1->SubRequest(req1);
  Requirement& rmt1 = req1.y_requirement();
  Allocation* alctn1 = (ivAllocation *) &(ad1->allocation());
  Coord height1 = alctn1->top() - alctn1->bottom();
  Coord change1 = height1 - rmt1.natural();
  Coord stretch1 = rmt1.stretch();
  Coord shrink1 = rmt1.shrink();
  if(change1 > 0)  { stretch1 = stretch1 - change1; shrink1 = shrink1 + change1;}
  else { stretch1 = stretch1 + change1; shrink1 = shrink1 - change1;}
  if(shrink1 < 0) shrink1 = 0;
  if(stretch1 < 0) stretch1 = 0;

  Requisition req2;
  ad2->SubRequest(req2);
  Requirement& rmt2 = req2.y_requirement();
  Allocation* alctn2  = (ivAllocation *) &(ad2->allocation());
  Coord height2 = alctn2->top() - alctn2->bottom();
  Coord change2 = height2 - rmt2.natural();
  Coord stretch2 = rmt2.stretch();
  Coord shrink2 = rmt2.shrink();
  if(change2 > 0)  { stretch2 = stretch2 - change2; shrink2 = shrink2 + change2;}
  else { stretch2 = stretch2 + change2; shrink2 = shrink2 - change2;}
  if(shrink2 < 0) shrink2 = 0;
  if(stretch2 < 0) stretch2 = 0;

  ivCoord diffy = e.pointer_y() - starty;
  ivCoord adjust_factor;
  if(diffy < 0) { 
    adjust_factor = MIN(stretch1,shrink2);
    adjust_factor = MIN(-diffy,adjust_factor);
    ad1->NewSize(height1+adjust_factor);  ad2->NewSize(height2-adjust_factor);
  }
  else {
    adjust_factor = MIN(shrink1,stretch2);
    adjust_factor = MIN(diffy,adjust_factor);
    ad1->NewSize(height1-adjust_factor);  ad2->NewSize(height2+adjust_factor);
  }
  starty = e.pointer_y();
  ad1->change(0);
  ad2->change(0);
  poly_->modified(myindex -1);
  poly_->modified(myindex +1);
  patch_->reallocate();
  patch_->redraw();
} 

