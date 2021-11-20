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


// DragPage.cc

#include "dragpage.h"
#include <InterViews/background.h>
#include <InterViews/patch.h>
#include <InterViews/event.h>
#include <InterViews/canvas.h>
#include <InterViews/hit.h>
#include <InterViews/cursor.h> // for crosshairs

// for testing
#include <iostream.h>

#ifndef MIN
 #define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

DragPage::DragPage(WidgetKit* kit,LayoutKit* layout,Glyph* g)
: DragZone(nil), kit_(kit) , layout_(layout){
  page_ = new Page(g);
  patch_ = new Patch(page_); 
  edit_ = false;
  dragged_object = nil;
  body(patch_);
}

void DragPage::AlignToLeftMost() {
  GlyphIndex c = page_->count();
  int i;  ivCoord x = 0; ivCoord y=0 ; ivCoord  edge = 0;
  for(i=0;i<c;i++){    page_->location(i,x,y);    if(i==0 || (x < edge)) edge = x; }
  for(i=0;i<c;i++){    page_->location(i,x,y);    page_->move(i,edge,y); }
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, -1);
  patch_->redraw();
}

void DragPage::AlignToRightMost() {
  GlyphIndex c = page_->count();
  int i;  ivCoord x = 0; ivCoord y=0 ; ivCoord  edge = 0;
  for(i=0;i<c;i++){    page_->location(i,x,y);    if(i==0 || (x > edge)) edge = x; }
  for(i=0;i<c;i++){    page_->location(i,x,y);    page_->move(i,edge,y); }
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, -1);
  patch_->redraw();
}

void DragPage::AlignToTopMost() {
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, -1);
  patch_->redraw();
}

void DragPage::AlignToBottomMost() {
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, -1);
  patch_->redraw();
}

void DragPage::AutoAlign() { // l->r, b->t
  GlyphIndex c = page_->count();
  Coord curx,cury;
  curx = cury = 0;
  Coord xmax = allocation_.x_allotment().end();
  GlyphIndex i;
  Coord maxi = 0; // maximum height on current line
  for(i=0;i<c;i++){
    Requisition req;
    page_->component(i)->request(req);
    Coord newx = curx + req.x_requirement().natural();
    if(newx > xmax) { curx = 0; cury += maxi; maxi = 0;}
    page_->move(i,curx,cury);
    curx = newx;
    maxi = MAX(maxi,req.y_requirement().natural());
  }
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, -1);
  patch_->redraw();
}

void DragPage::SetDragObject(Drag* d){  dragged_object = d;}

GlyphIndex DragPage::GetPos(Glyph* g){
  if((page_ ==  nil) || (g == nil)) return -1;
  GlyphIndex c = page_->count();
  for(int i=0;i<c;i++){    if(page_->component(i) == g) return i;}
  return -1;
}

void DragPage::drop(Event& event, const char* , int ) {
  if(dragged_object == nil ) return;
  DragPageObject* dpo = (DragPageObject *) dragged_object;
  GlyphIndex gi = GetPos(dragged_object);
  Allocation* a = dpo->GetAllocation();
  page_->move(gi,event.pointer_x() - dpo->GetDiffx() - allocation_.x(),
	      event.pointer_y() - dpo->GetDiffy() - allocation_.y());
  dragged_object = nil;
  patch_->redraw();
  if(DropUpdate && dropupdateobject) DropUpdate(dropupdateobject, (int) gi);
}

void DragPage::allocate(Canvas* c, const Allocation& a, Extension& e) {
  DragZone::allocate(c, a, e);
  canvas_ = c;  allocation_ = a;  extension_ = e;
}

void DragPage::reallocate() {
  extension_.clear();
  allocate(canvas_, allocation_, extension_);
  canvas_->damage(extension_);
}


Patch* DragPage::GetPatch(){ return patch_;}
Page* DragPage::GetPage(){  return page_;}
int DragPage::Editable(){  return edit_;};
void DragPage::ToggleEdit(){  edit_ = !edit_;};
void DragPage::enter(Event&, const char*, int) {};
void DragPage::leave(Event&) {};
void DragPage::motion(Event&) {};




////////////////////
// DragPageObject //
////////////////////

DragPageObject::DragPageObject( DragPage* p, WidgetKit* kit, boolean useCursor, boolean useGlyph, Glyph* g=nil)
: Drag(g), kit_(kit), useCursor_(useCursor), useGlyph_(useGlyph) {
  dragpage_ = p; 
}


Allocation* DragPageObject::GetAllocation() {
  return &allocation_;
}

void DragPageObject::dragOffset(Event& event, int& dx, int& dy) {
  Drag::dragOffset(event,dx,dy);
  cout << event.pointer_x() << " - " << allocation_.left() << " = " << dx << "\n";
  cout << event.pointer_y() << " - " << allocation_.top() << " = " << dy << "\n";
//  dy -= (int) ((allocation_.top() - allocation_.bottom()) - 2.0);
//  dx = 0; 
//  dx =  int(event.pointer_x() - rep_->x_ + 1);

}

void DragPageObject::allocate(Canvas* c, const Allocation& a, Extension& ext) {
  Drag::allocate(c,a,ext);
  allocation_ = a;
}

// special verion of pick that does not pass pick down if drag::caught
// opertion occurs...

void DragPageObject::pick(Canvas* c, const Allocation& a, int depth, Hit& hit) {
  const Event* event = hit.event();
  int catchme = caught(*event);
  Glyph* thebody = body();
  if(dragpage_->Editable()) {
    Resource::ref(thebody); body(nil);
  }
  Drag::pick(c,a,depth,hit);
  if(dragpage_->Editable()) {
    body(thebody);
    Resource::unref(thebody);
  }
}


boolean DragPageObject::caught(const Event& event) const {
  if(event.type() == Event::down) {
    if(event.pointer_button() == Event::middle) {
      ((DragPageObject*) this)->diffx  = event.pointer_x() - allocation_.x();
      ((DragPageObject*) this)->diffy  = event.pointer_y() - allocation_.y();
      return true;
    }
  }
  return false;
}

Glyph* DragPageObject::glyph() {
  return body();
}

Cursor* DragPageObject::dragCursor() {
    return useCursor_ ? crosshairs : nil;
}

Glyph* DragPageObject::dragGlyph() {
  return body();
}

void DragPageObject::dragData(char*& value, int& length) {
  length = 0;
  value = nil;
  dragpage_->SetDragObject(this);
}

ivCoord DragPageObject::GetDiffy(){ return diffy; }
ivCoord DragPageObject::GetDiffx(){ return diffx; }




