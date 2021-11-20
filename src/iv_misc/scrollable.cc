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

#include "scrollable.h"


#include <ta/enter_iv.h>
#include <IV-look/kit.h>
#include <InterViews/canvas.h>
#include <InterViews/layout.h>
#include <InterViews/hit.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <InterViews/session.h>
#include <OS/math.h>

Scrollable::Scrollable(Glyph* g, Coord w, Coord h, Coord dx, Coord dy)
: Patch(xform_ = new TformSetter(g)) {
    width_ = w;
    height_ = h;
    scale_ = 1.0;
    dx_ = dx;
    dy_ = dy;
    const Transformer t(
	scale_, 0,
	0, scale_,
	- dx_ * scale_, - dy_ * scale_
    );

    xform_->transformer(t);
    scroll_incr_ = 10;
    page_incr_ = 50;
}

Scrollable::~Scrollable () {
}

void Scrollable::request(Requisition& req) const {
  Scrollable* s = (Scrollable*) this;
  Patch::request(s->requisition_);

  Requirement& rx = s->requisition_.x_requirement();
  Requirement& ry = s->requisition_.y_requirement();

  s->width_ =  rx.natural();
  s->height_ = ry.natural();

  Requirement& box_x = req.x_requirement();
  box_x.natural(width_);
  box_x.stretch(fil);
  box_x.shrink(width_ - 20);	// 20 is for the scrollbar width
  box_x.alignment(0.0);
  
  Requirement& box_y = req.y_requirement();
  box_y.natural(height_);
  box_y.stretch(fil);
  box_y.shrink(height_ - 20);	// 20 is for the scrollbar height
  box_y.alignment(0.0);
}

void Scrollable::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    Patch::allocate(c, a, ext);
    notify(Dimension_X);
    notify(Dimension_Y);
}

void Scrollable::draw(Canvas* c, const Allocation& a) const {
    c->damage(0, 0, width_, height_);
    Extension e;
    e.set(c, a);
    c->push_clipping();
    c->push_transform();
    c->transformer(Transformer());
//    c->clip_rect(e.left(), e.bottom(), e.right(), e.top());
    c->clip_rect(a.left(), a.bottom(), a.right(), a.top());
    c->pop_transform();
    c->pop_clipping();
    Patch::draw(c, a);
}

void Scrollable::reallocate() {
    Patch::reallocate();
}

void Scrollable::pick(Canvas* c, const Allocation& a, int depth, Hit& h) {
    Patch::pick(c, a, depth, h);
}

Coord Scrollable::cur_lower(DimensionName d) const {
    if (d == Dimension_X) return dx_;
    else return dy_;
}

Coord Scrollable::cur_upper(DimensionName d) const {
  if (d == Dimension_X)
    return dx_ + scale_ * 
      (Math::min(allocation().x_allotment().span(),width_));
  else
    return dy_ + scale_ * 
      (Math::min(allocation().y_allotment().span(),height_));
}

Coord Scrollable::cur_length(DimensionName d) const {
  if (d == Dimension_X)
    return scale_ * 
	  (Math::min(allocation().x_allotment().span(),width_));
  else
    return scale_ * 
      (Math::min(allocation().y_allotment().span(),height_));
}

Coord Scrollable::lower(DimensionName) const {
  return 0;
}

Coord Scrollable::upper(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return height_;
}
Coord Scrollable::length(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return height_;
}

void Scrollable::scroll_to(DimensionName d, Coord lower) {
    Coord p = lower;
    constrain(d, p);
    if (p != cur_lower(d)) {
	if (d == Dimension_X) dx_ = p;
	else dy_ = p;
	Transformer t(
	    scale_, 0,
	    0, scale_,
	    - dx_ * scale_, - dy_ * scale_
	);

	redraw();
	xform_->transformer(t);
	reallocate();
	notify(d);
	redraw();
    };
}
    
void Scrollable::scale_to(DimensionName, float fraction_visible) {
    zoom_to(fraction_visible);
}

void Scrollable::zoom_to(float magnification) {
    if (magnification <= (1./16)) {
	magnification = 1./16;
    } else if (magnification > 16) {
	magnification = 16;
    }
    if (scale_ != magnification) {
	scale_ = magnification;
	Transformer t(
	    scale_, 0,
	    0, scale_,
	    - scale_ * dx_, - scale_ * dy_
	);
	redraw();
	xform_->transformer(t);
	reallocate();
	redraw();
	notify(Dimension_X);
	notify(Dimension_Y);
    }
}

float Scrollable::scale() { return scale_; }

void Scrollable::scroll_forward(DimensionName d) {
  Coord newpos;

  if (d == Dimension_X) {
    newpos = dx_ + scroll_incr_;
  } else {
    newpos = dy_ + scroll_incr_;
  };

  scroll_to(d, newpos);
}

void Scrollable::scroll_backward(DimensionName d) {
  Coord newpos;

  if (d == Dimension_X) {
    newpos = dx_ - scroll_incr_;
  } else {
    newpos = dy_ - scroll_incr_;
  };

  scroll_to(d, newpos);
}

void Scrollable::page_forward(DimensionName d) {
  Coord newpos;

  if (d == Dimension_X) {
    newpos = dx_ + page_incr_;
  } else {
    newpos = dy_ + page_incr_;
  };

  scroll_to(d, newpos);
}

void Scrollable::page_backward(DimensionName d) {
  Coord newpos;

  if (d == Dimension_X) {
    newpos = dx_ - page_incr_;
  } else {
    newpos = dy_ - page_incr_;
  };

  scroll_to(d, newpos);
}


HScrollable::HScrollable(Glyph* g, Coord w, Coord h, Coord dx, Coord dy)
: Scrollable(xform_ = new HTformSetter(g)){
  width_ = w;
  height_ = h;
  scale_ = 1.0;
  dx_ = dx;
  dy_ = dy;
  const Transformer t
    (scale_, 0,
     0, scale_,
     - dx_ * scale_, - dy_ * scale_
     );
  xform_->transformer(t);
  scroll_incr_ = 10;
  page_incr_ = 50;
}


HScrollable::~HScrollable () {}


void HScrollable::request(Requisition& req) const {
  Scrollable* s = (Scrollable*) this;
  Patch::request(s->requisition_);

  Requirement& rx = s->requisition_.x_requirement();
  Requirement& ry = s->requisition_.y_requirement();

  s->width_ =  rx.natural();
  s->height_ = ry.natural();

  Requirement& box_x = req.x_requirement();
  box_x.natural(width_);
  box_x.stretch(fil);
  box_x.shrink(width_ - 20);
  box_x.alignment(0.0);
  
  Requirement& box_y = req.y_requirement();
  box_y = ry;
   box_y.natural(height_);
   box_y.stretch(fil);
   box_y.shrink(height_ - 20);
   box_y.alignment(0.0);
}


/*
 * TformSetter
 */

#include <InterViews/canvas.h>
#include <InterViews/hit.h>
#include <InterViews/printer.h>
#include <InterViews/tformsetter.h>
#include <OS/math.h>

TformSetter::TformSetter(Glyph* g) : MonoGlyph(g) { }

TformSetter::TformSetter(
    Glyph* g, const Transformer& tx
) : MonoGlyph(g) {
    transformer_ = tx;
}

TformSetter::~TformSetter() { }

const Transformer& TformSetter::transformer() const {
    return transformer_;
}

Transformer& TformSetter::transformer() {
    return transformer_;
}

void TformSetter::transformer(const Transformer& tx) {
    transformer_ = tx;
}

// compute_req sets the requirements natural size to be
//  last-first, and makes it unflexible.
//  also it sets the alignment

static void compute_req(Requirement& r, Coord first, Coord last) {
    Coord natural = last - first;
    r.natural(natural);
    r.stretch(0.0);
    r.shrink(0.0);
    if (Math::equal(natural, float(0), float(1e-3))) {
	r.alignment(0.0);
    } else {
	r.alignment(-first / natural);
    }
}

void TformSetter::request(Requisition& req) const {
    TformSetter* t = (TformSetter*)this;
    MonoGlyph::request(req);
    Allocation& a = t->natural_allocation_;

    // set the natural_allocation to be the
    // same as the sub glyph's request except origin = 0,0

    Requirement& rx = req.x_requirement();
    Allotment& ax = a.x_allotment();
    ax.origin(0.0);
    ax.span(rx.natural());
    ax.alignment(rx.alignment());

    Requirement& ry = req.y_requirement();
    Allotment& ay = a.y_allotment();
    ay.origin(0.0);
    ay.span(ry.natural());
    ay.alignment(ry.alignment());

    // determines the size of the natural_allocation
    // modified by the transformer and sets
    // the request to be of this size, inflexible,
    // and with an alignment.

    const Transformer& tx = transformer_;
    Coord left = ax.begin(), bottom = ay.begin();
    Coord right = ax.end(), top = ay.end();
    Coord x1, y1, x2, y2, x3, y3, x4, y4;
    tx.transform(left, bottom, x1, y1);
    tx.transform(left, top, x2, y2);
    tx.transform(right, top, x3, y3);
    tx.transform(right, bottom, x4, y4);
    left = Math::min(x1, x2, x3, x4);
    bottom = Math::min(y1, y2, y3, y4);
    right = Math::max(x1, x2, x3, x4);
    top = Math::max(y1, y2, y3, y4);

    compute_req(rx, left, right);
    compute_req(ry, bottom, top);
}

void TformSetter::allocate(
    Canvas* c, const Allocation& a, Extension& ext
) {
    /*
     * Shouldn't need to test for nil canvas, but some old
     * applications (notably doc) pass nil as a canvas
     * when doing certain kinds of allocation.
     */
    if (c != nil) {
	push_transform(c, a, natural_allocation_);
//	MonoGlyph::allocate(c, natural_allocation_, ext);
	Allocation b = a;
	SetFullAllocation(&b);
  	MonoGlyph::allocate(c, b, ext);
	c->pop_transform();
    }

}

void TformSetter::draw(Canvas* c, const Allocation& a) const {
    push_transform(c, a, natural_allocation_);
//  MonoGlyph::draw(c, natural_allocation_);
    Allocation b(a);
    SetFullAllocation(&b);
    MonoGlyph::draw(c, b);
    c->pop_transform();

}

void TformSetter::print(Printer* p, const Allocation& a) const {
    push_transform(p, a, natural_allocation_);
//    MonoGlyph::print(p, natural_allocation_);
    Allocation b = a;
    SetFullAllocation(&b);
    MonoGlyph::print(p,b);
    p->pop_transform();
}

void TformSetter::pick(Canvas* c, const Allocation& a, int depth, Hit& h) {
  Transformer t(transformer_);
  transform(t, a, natural_allocation_);
  c->push_transform();
  c->transform(t);
  h.push_transform();
  h.transform(t);
  Allocation b = a;		
  SetFullAllocation(&b);	
  MonoGlyph::pick(c, b, depth, h);
  c->pop_transform();
  h.pop_transform();
}

void TformSetter::push_transform(
    Canvas* c, const Allocation& a, const Allocation& natural
) const {
    Transformer t(transformer_);
    transform(t, a, natural);
    c->push_transform();
    c->transform(t);
}

void TformSetter::transform(
    Transformer& t, const Allocation& a, const Allocation&
) const {
    t.translate(a.x(), a.y());
}

void TformSetter::SetFullAllocation(Allocation* b) const {
  Allotment& bx = b->x_allotment();
  bx.origin(0.0);
  Allotment& by = b->y_allotment();
  by.origin(0.0);
}


HTformSetter::HTformSetter(Glyph* g) : TformSetter(g){};
HTformSetter::HTformSetter(Glyph* g, const Transformer& t) 
: TformSetter(g,t){};

HTformSetter::~HTformSetter() {};

void HTformSetter::SetFullAllocation(Allocation* b) const {
  Allotment& bx = b->x_allotment();
  if(natural_allocation_.x_allotment().span() > bx.span()) { //only set if greater
    bx = natural_allocation_.x_allotment();
  }
  bx.origin(0.0);
  Allotment& by = b->y_allotment();
  by.origin(0.0);
}

#include <ta/leave_iv.h>
