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

#include "hungrybox.h"


#include <ta/enter_iv.h>
#include <InterViews/canvas.h>
#include <InterViews/printer.h>
#include <InterViews/hit.h>
#include <InterViews/scrbox.h>
#include <InterViews/transformer.h>
#include <OS/list.h>
#include <OS/math.h>


HungryBox::HungryBox(int* i, int (*more)(int*, int, HungryBox*),
		     int (*less)(int*,int,HungryBox*), GlyphIndex size)
: PolyGlyph(size) { feeder =  i; ShowMore = more; ShowLess = less;}

HungryBox::~HungryBox() { }

GlyphIndex HungryBox::first_shown() const {
    return 0;
}
GlyphIndex HungryBox::last_shown() const {
    return count() - 1;
}


struct TBHungryBoxInfo {
    /* workaround for g++ bug */
    TBHungryBoxInfo() { }
private:
    friend class TBHungryBox;
    friend class TBHungryBoxImpl;

    Glyph* glyph_;
    Allocation allocation_;
};

declareList(TBHungryBoxList,TBHungryBoxInfo)
implementList(TBHungryBoxList,TBHungryBoxInfo)

class TBHungryBoxImpl {
private:
    friend class TBHungryBox;

    HungryBox* 		hungrybox_;
    int 		est_size;
    GlyphIndex 		start_;
    GlyphIndex 		end_;
    boolean 		changed_;
    Requisition 	requisition_;
    Canvas* 		canvas_;
    Transformer 	transformer_;
    Allocation 		allocation_;
    Extension 		extension_;
    TBHungryBoxList 	visible_;

    void check(Canvas*, const Allocation&);
    void refresh();
    void reallocate();
    void redraw();
    void undraw_range(GlyphIndex begin, GlyphIndex end);
};

/* class TBHungryBoxImpl */

void TBHungryBoxImpl::check(Canvas* c, const Allocation& a) {
    if (canvas_ == nil || canvas_ != c ||
	transformer_ != c->transformer() || !allocation_.equals(a, 1e-4)
    ) {
	Extension ext;
	hungrybox_->allocate(c, a, ext);
    }
}

void TBHungryBoxImpl::refresh() {
    Requisition req;
    hungrybox_->request(req);
    start_ = 0;
    reallocate();
    redraw();
}

void TBHungryBoxImpl::reallocate() {
  if (canvas_ == nil) {
    return;
  }
  boolean done = false;
  int iters;
  for(iters=0;(iters<4 && !done);iters++) {
    HungryBox* s = hungrybox_;
    GlyphIndex n = s->count();
    end_ = n;
    TBHungryBoxList& list = visible_;
    list.remove_all();
    Requisition req;
    TBHungryBoxInfo info;
    Extension e_i;
    const Requirement& r = req.y_requirement();
    Coord p = allocation_.top();
    Coord bottom = allocation_.bottom();
    boolean found_start = false;

    Coord span = 0;
    GlyphIndex i;
    
    for (i = 0; i < n; i++) {
      Glyph* g = s->component(i);
      if (g != nil) {
	g->request(req);
	span = r.natural();
	if (!Math::equal(span, Coord(0), float(1e-2))) {
	  if (!found_start) {
	    start_ = i;
	    found_start = true;
	  }
	  Coord alignment = r.alignment();
	  p -= span;
	  if ((p - bottom) <  -0.5) { // give half a pixel variance
	    end_ = i;
	    break;
	  }
	  info.glyph_ = g;
	  Allotment& ax = info.allocation_.x_allotment();
	  ax = allocation_.x_allotment();
	  Allotment& ay = info.allocation_.y_allotment();
	  ay.span(span);
	  ay.origin(p + Coord(alignment * span));
	  ay.alignment(alignment);
	  list.append(info);
	  //	  ivGlyph* gg = g->component(0);  // for debuging
	  g->allocate(canvas_, info.allocation_, e_i);
	}
      }
    }
    if (i < (n)) {		// we have too many
      done = s->ShowLess(s->feeder,(int)i,s); 
      est_size = (int) i;
    }
    else {
      if (span && (p > (bottom + span))) { // we have more room
	est_size = (int) n + (int) (( p-bottom) / span);
	done = s->ShowMore(s->feeder,est_size,s);
      }
      else {
	done = true;
	est_size = (int)n;
      }
    }
  }
}

void TBHungryBoxImpl::redraw() {
  if (canvas_ != nil) {
	canvas_->damage(extension_);
    }
}

void TBHungryBoxImpl::undraw_range(GlyphIndex begin, GlyphIndex end) {
    HungryBox* s = hungrybox_;
    for (GlyphIndex i = begin; i <= end; i++) {
	Glyph* g = s->component(i);
	if (g != nil) {
	    g->undraw();
	}
    }
}


TBHungryBox::TBHungryBox(int* i, int (*more)(int*, int, HungryBox*),
 int (*less)(int*, int, HungryBox*), GlyphIndex size)
: HungryBox(i, more,less,size) {
  impl_ = new TBHungryBoxImpl;
  TBHungryBoxImpl& sb = *impl_;
  sb.est_size = 0;
  sb.hungrybox_ = this;
  sb.start_ = 0;
  sb.end_ = 0;
  sb.changed_ = true;
  sb.canvas_ = nil;
}

TBHungryBox::~TBHungryBox() {
    delete impl_;
}

int TBHungryBox::est_size(){
  return impl_->est_size;
}

void TBHungryBox::request(Requisition& req) const {
  GlyphIndex n = count();
  TBHungryBoxImpl& sb = *impl_;
  if (sb.changed_) {
    Requisition r;
    const Requirement& rx = r.x_requirement();
    const Requirement& ry = r.y_requirement();
    Coord natural_width = 0.0;
    Coord natural_height = 0.0;
    Coord first_height = 0;
    for (GlyphIndex i = 0; i < n; i++) {
      Glyph* g = component(i);
      if (g != nil)	g->request(r);
      Coord r_width = rx.natural();
      if (r_width > natural_width) natural_width = r_width;
      natural_height += ry.natural();
      if(i==0) first_height = natural_height;
    }
    Requirement& box_x = sb.requisition_.x_requirement();
    box_x.natural(natural_width);
    box_x.stretch(fil);
    box_x.shrink(natural_width);
    box_x.alignment(0.0);

    Requirement& box_y = sb.requisition_.y_requirement();
    box_y.natural(first_height);
    box_y.stretch(fil);
    box_y.shrink(first_height);
//    box_y.shrink(0);
    box_y.alignment(1.0);
    sb.changed_ = false;
  }
  req = sb.requisition_;
}

void TBHungryBox::remove(GlyphIndex i) {
  PolyGlyph::remove(i);
  ivResource::flush();
}

// void TBHungryBox::request(Requisition& req) const {
//     GlyphIndex n = count();
//     TBHungryBoxImpl& sb = *impl_;
//     if (sb.changed_) {
// 	Requisition r;
// 	const Requirement& rx = r.x_requirement();
// 	const Requirement& ry = r.y_requirement();
// 	Coord natural_width = 0.0;
// 	Coord natural_height = 0.0;
// 	Coord first_height = 0;
// 	for (GlyphIndex i = 0; i < n; i++) {
// 	    Glyph* g = component(i);
// 	    if (g != nil) {
// 		g->request(r);
// 		if((i!=0) && (i==(n-1))){
// 		  first_height = natural_height;
// 		}
// 		Coord r_width = rx.natural();
// 		if (r_width > natural_width) {
// 		    natural_width = r_width;
// 		}
// 		natural_height += ry.natural();
// 	    }
// 	}
// 	Requirement& box_x = sb.requisition_.x_requirement();
// 	box_x.natural(natural_width);
// 	box_x.stretch(fil);
// 	box_x.shrink(natural_width);
// 	box_x.alignment(0.0);
// 
// 	Requirement& box_y = sb.requisition_.y_requirement();
// 	box_y.natural(natural_height);
// 	box_y.stretch(fil);
// 	box_y.shrink(first_height);
// //	box_y.shrink(natural_height);
// 	box_y.alignment(1.0);
// 	sb.changed_ = false;
//     }
//     req = sb.requisition_;
// }

void TBHungryBox::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    TBHungryBoxImpl& sb = *impl_;
    if (sb.changed_) {
	Requisition req;
	request(req);
    }
    ext.set(c, a);
    sb.canvas_ = c;
    if (c != nil) {
	sb.transformer_ = c->transformer();
    }
    sb.allocation_ = a;
    sb.extension_ = ext;
    sb.reallocate();
}

void TBHungryBox::draw(Canvas* c, const Allocation& a) const {
    TBHungryBoxImpl& sb = *impl_;
    sb.check(c, a);
    Extension& e = sb.extension_;
    if (sb.canvas_->damaged(e)) {
	if (sb.changed_) {
	    sb.refresh();
	}
	c->push_clipping();
	c->clip_rect(a.left(), a.bottom(), a.right(), a.top());
	for (ListItr(TBHungryBoxList) i(sb.visible_); i.more(); i.next()) {
	    const TBHungryBoxInfo& info = i.cur_ref();
	    Glyph* g = info.glyph_;
	    if(g==nil) continue; // should not happen
	    g->draw(c, info.allocation_);
	}
	c->pop_clipping();
    }
}

void TBHungryBox::print(Printer* c, const Allocation& a) const {
    TBHungryBoxImpl& sb = *impl_;
    sb.check(c, a);
    Extension& e = sb.extension_;
    if (sb.canvas_->damaged(e)) {
	if (sb.changed_) {
	    sb.refresh();
	}
	c->push_clipping();
	c->clip_rect(a.left(), a.bottom(), a.right(), a.top());
	for (ListItr(TBHungryBoxList) i(sb.visible_); i.more(); i.next()) {
	    const TBHungryBoxInfo& info = i.cur_ref();
	    Glyph* g = info.glyph_;
	    if(g==nil) continue; // should not happen
	    g->print(c, info.allocation_);
	}
	c->pop_clipping();
    }
}

void TBHungryBox::pick(Canvas* c, const Allocation& a, int depth, Hit& h) {
    TBHungryBoxImpl& sb = *impl_;
    sb.check(c, a);
    if (h.left() < a.right() && h.right() >= a.left() &&
	h.bottom() < a.top() && h.top() >= a.bottom()
    ) {
	if (sb.changed_) {
	    sb.refresh();
	}
	GlyphIndex n = sb.start_;
	for (ListItr(TBHungryBoxList) i(sb.visible_); i.more(); i.next()) {
	    const TBHungryBoxInfo& info = i.cur_ref();
	    Glyph* g = info.glyph_;
	    if(g==nil) continue; // should not happen
	    h.begin(depth, this, n);
	    g->pick(c, info.allocation_, depth + 1, h);
	    h.end();
	    ++n;
	}
    }
}

void TBHungryBox::undraw() {
    impl_->canvas_ = nil;
    HungryBox::undraw();
}

void TBHungryBox::modified(GlyphIndex) {
    impl_->changed_ = true;
}

boolean TBHungryBox::shown(GlyphIndex i) const {
    TBHungryBoxImpl& sb = impl();
    return i >= sb.start_ && i < sb.end_;
}

GlyphIndex TBHungryBox::first_shown() const {
    TBHungryBoxImpl& sb = impl();
    return sb.start_;
}

GlyphIndex TBHungryBox::last_shown() const {
    TBHungryBoxImpl& sb = impl();
    return sb.end_ - 1;
}

void TBHungryBox::allotment(
    GlyphIndex i, DimensionName d, Allotment& a
) const {
    TBHungryBoxImpl& sb = impl();
    if (i >= sb.start_ && i < sb.end_) {
	a = sb.visible_.item(i - sb.start_).allocation_.allotment(d);
    }
}

Coord TBHungryBox::lower(DimensionName) const {
    return Coord(0);
}

Coord TBHungryBox::upper(DimensionName) const {
    return Coord(count() - 1);
}

Coord TBHungryBox::length(DimensionName) const {
    return Coord(count());
}

Coord TBHungryBox::cur_lower(DimensionName) const {
    TBHungryBoxImpl& sb = impl();
    return Coord(count() - sb.end_);
}

Coord TBHungryBox::cur_upper(DimensionName) const {
    TBHungryBoxImpl& sb = impl();
    return Coord(count() - 1 - sb.start_);
}

Coord TBHungryBox::cur_length(DimensionName) const {
    TBHungryBoxImpl& sb = impl();
    return Coord(sb.end_ - sb.start_);
}

TBHungryBoxImpl& TBHungryBox::impl() const {
    TBHungryBoxImpl& sb = *impl_;
    if (sb.changed_) {
	sb.refresh();
    }
    return sb;
}

ShapeOfx::ShapeOfx(Glyph* theglyph, Glyph* theshape) : MonoGlyph(theglyph){
  x_ = theshape;
  Resource::ref(x_);
}

ShapeOfx::~ShapeOfx() {
    Resource::unref(x_);
}

void ShapeOfx::request(Requisition& requisition) const {
  MonoGlyph::request(requisition);
  if(!x_) return;
  Requisition req;
  x_->request(req);
  requisition.require_x(req.x_requirement());
}

#include <ta/leave_iv.h>
