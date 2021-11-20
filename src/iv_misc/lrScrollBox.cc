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

#include <ta/enter_iv.h>
#include <InterViews/canvas.h>
#include <InterViews/hit.h>
#include <InterViews/scrbox.h>
#include <InterViews/transformer.h>
#include <OS/list.h>
#include <OS/math.h>
#include <ta/leave_iv.h>

#include "lrScrollBox.h"

// Left-Right ScrollBox

struct lrScrollBoxInfo {
    /* workaround for g++ bug */
    lrScrollBoxInfo() { }
private:
    friend class lrScrollBox;
    friend class lrScrollBoxImpl;

    ivGlyph* glyph_;
    ivAllocation allocation_;
};

#include <ta/enter_iv.h>
declareList(lrScrollBoxList,lrScrollBoxInfo)
implementList(lrScrollBoxList,lrScrollBoxInfo)
#include <ta/leave_iv.h>

class lrScrollBoxImpl {
private:
    friend class lrScrollBox;

    ivScrollBox* 			scrollbox_;
    ivGlyphIndex 			start_;
    ivGlyphIndex 			end_;
    osboolean 			changed_;
    ivRequisition 		requisition_;
    ivCanvas* 			canvas_;
    ivTransformer 		transformer_;
    ivAllocation 			allocation_;
    ivExtension 			extension_;
    lrScrollBoxList 	visible_;

    void check(ivCanvas*, const ivAllocation&);
    void refresh();
    void reallocate();
    void redraw();
    void undraw_range(ivGlyphIndex begin, ivGlyphIndex end);
};

lrScrollBox::lrScrollBox(ivGlyphIndex size) : ivScrollBox(size) {
    impl_ = new lrScrollBoxImpl;
    lrScrollBoxImpl& sb = *impl_;
    sb.scrollbox_ = this;
    sb.start_ = 0;
    sb.end_ = 0;
    sb.changed_ = ostrue;
    sb.canvas_ = nil;
    naturalnum = -1;
    default_width = 0;
}

lrScrollBox::~lrScrollBox() {
    delete impl_;
}


void  lrScrollBox::request(ivRequisition& req) const {
  ivGlyphIndex n = count();
  lrScrollBoxImpl& sb = *impl_;
  if (sb.changed_) {
    ivRequisition r;
    const ivRequirement& rx = r.x_requirement();
    const ivRequirement& ry = r.y_requirement();
    ivCoord natural_width = 5.0; // a little extra for safety
    ivCoord natural_height = 0.0;
    
    if(naturalnum != -1) 
      n = (naturalnum > n) ? n : naturalnum;
    ivCoord firstwidth = 0;
    
    if(n==0) natural_width = naturalnum * default_width;

    for (ivGlyphIndex i = 0; i < n; i++) {
      ivGlyph* g = component(i);
      if (g != nil) {
	g->request(r);
	if((i!=0) && (i==(n-1))){
	  firstwidth = natural_width;
	}	  
	ivCoord r_height = ry.natural();
	if (r_height > natural_height) {
	  natural_height = r_height;
	}
	natural_width += rx.natural();
      }
    }
    if(naturalnum!=-1) firstwidth = 0;
    ivRequirement& box_x = sb.requisition_.x_requirement();
    box_x.natural(natural_width);
    box_x.stretch(fil);
    box_x.shrink(firstwidth);
//      box_x.shrink(natural_width);
    box_x.alignment(0.0);		
    
    ivRequirement& box_y = sb.requisition_.y_requirement();
    box_y.natural(natural_height);
    box_y.stretch(fil);
    box_y.shrink(0);
//      box_y.shrink(natural_height);
    box_y.alignment(0.0);		
    
    sb.changed_ = osfalse;
  }
  req = sb.requisition_;
}

void lrScrollBox::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
    lrScrollBoxImpl& sb = *impl_;
    if (sb.changed_) {
		ivRequisition req;
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
    notify(Dimension_X);
    notify(Dimension_Y);
}

void lrScrollBox::draw(ivCanvas* c, const ivAllocation& a) const {
  lrScrollBoxImpl& sb = *impl_;
  sb.check(c, a);
  ivExtension& e = sb.extension_;
  if (sb.canvas_->damaged(e)) {
    if (sb.changed_) {
      sb.refresh();
    }
    c->push_clipping();
    
    c->clip_rect(a.left(), a.bottom(), a.right(), a.top());
    for (ListItr(lrScrollBoxList) i(sb.visible_); i.more(); i.next()) {
      const lrScrollBoxInfo& info = i.cur_ref();
      ivGlyph* g = info.glyph_;
      g->draw(c, info.allocation_);
    }
    c->pop_clipping();
  }
}

void lrScrollBox::pick(ivCanvas* c, const ivAllocation& a, int depth, ivHit& h) {
  lrScrollBoxImpl& sb = *impl_;
  sb.check(c, a);
  if (h.left() < a.right() && h.right() >= a.left() &&
      h.bottom() < a.top() && h.top() >= a.bottom()) {
    if (sb.changed_) {
      sb.refresh();
    }
    ivGlyphIndex n = sb.start_;
    for (ListItr(lrScrollBoxList) i(sb.visible_); i.more(); i.next()) {
      const lrScrollBoxInfo& info = i.cur_ref();
      ivGlyph* g = info.glyph_;
      h.begin(depth, this, n);
      g->pick(c, info.allocation_, depth + 1, h);
      h.end();
      ++n;
    }
  }
}

void lrScrollBox::undraw() {
  impl_->canvas_ = nil;
  ivScrollBox::undraw();
}

void lrScrollBox::modified(ivGlyphIndex) {
  impl_->changed_ = ostrue;
}

osboolean lrScrollBox::shown(ivGlyphIndex i) const {
  lrScrollBoxImpl& sb = impl();
  return i >= sb.start_ && i < sb.end_;
}

ivGlyphIndex lrScrollBox::first_shown() const {
  lrScrollBoxImpl& sb = impl();
  return sb.start_;
}

ivGlyphIndex lrScrollBox::last_shown() const {
  lrScrollBoxImpl& sb = impl();
  return sb.end_;
}

void lrScrollBox::allotment(ivGlyphIndex i, DimensionName d, ivAllotment& a) const {
  
  lrScrollBoxImpl& sb = impl();
  if (i >= sb.start_ && i < sb.end_) {
    a = sb.visible_.item(i - sb.start_).allocation_.allotment(d);
  }
}

ivCoord lrScrollBox::lower(DimensionName) const {
  return ivCoord(0);
}

ivCoord lrScrollBox::upper(DimensionName) const {
  return ivCoord(count() - 1);
}

ivCoord lrScrollBox::length(DimensionName) const {
  return ivCoord(count());
}

ivCoord lrScrollBox::cur_lower(DimensionName) const {
  lrScrollBoxImpl& sb = impl();
  return ivCoord(sb.start_);
}

ivCoord lrScrollBox::cur_upper(DimensionName) const {
  lrScrollBoxImpl& sb = impl();
  return ivCoord(sb.end_);
}

ivCoord lrScrollBox::cur_length(DimensionName) const {
  lrScrollBoxImpl& sb = impl();
  return ivCoord(sb.end_ - sb.start_);
}

void lrScrollBox::scroll_forward(DimensionName d) {
  scroll_by(d, long(small_scroll(d)));
}

void lrScrollBox::scroll_backward(DimensionName d) {
  scroll_by(d, -long(small_scroll(d)));
}

void lrScrollBox::page_forward(DimensionName d) {
  scroll_by(d, long(large_scroll(d)));
}

void lrScrollBox::page_backward(DimensionName d) {
  scroll_by(d, -long(large_scroll(d)));
}

void lrScrollBox::scroll_to(DimensionName d, ivCoord lower) {
  lrScrollBoxImpl& sb = impl();
  
  ivGlyphIndex new_start = osMath::round(lower);
  ivGlyphIndex new_end = new_start + (sb.end_ - sb.start_);
  
  do_scroll(d, new_start, new_end);
}

lrScrollBoxImpl& lrScrollBox::impl() const {
  lrScrollBoxImpl& sb = *impl_;
  if (sb.changed_) {
    sb.refresh();
  }
  return sb;
}

void lrScrollBox::scroll_by(DimensionName d, long offset) {
  lrScrollBoxImpl& sb = impl();
  do_scroll(d, sb.start_ + offset, sb.end_ + offset);
}

void lrScrollBox::do_scroll(DimensionName d, ivGlyphIndex new_start, ivGlyphIndex new_end) {
  
  lrScrollBoxImpl& sb = *impl_;
  ivGlyphIndex max_end = count();
  if (new_start < 0) {
    new_start = 0;
  }
  if (new_end > max_end) {
    new_start -= (new_end - max_end);
    new_end = max_end;
  }
  if (new_start != sb.start_ || new_end != sb.end_) {
    sb.undraw_range(sb.start_, new_start - 1);
    ivGlyphIndex old_end = sb.end_;
    sb.start_ = new_start;
    sb.end_ = new_end;
    sb.reallocate();
    sb.undraw_range(sb.end_, old_end - 1);
    sb.redraw();
    notify(d);
  }
}

/* class lrScrollBoxImpl */

void lrScrollBoxImpl::check(ivCanvas* c, const ivAllocation& a) {
  if (canvas_ == nil || canvas_ != c || 
      transformer_ != c->transformer() || !allocation_.equals(a, 1e-4)) {
    
    ivExtension ext;
    scrollbox_->allocate(c, a, ext);
  }
}

void lrScrollBoxImpl::refresh() {
  ivRequisition req;
  scrollbox_->request(req);
  start_ = 0;
  reallocate();
  redraw();
}

void lrScrollBoxImpl::reallocate() {
  if (canvas_ == nil) {
    return;
  }
  ivScrollBox* s = scrollbox_;
  ivGlyphIndex n = s->count();
  end_ = n;
  lrScrollBoxList& list = visible_;
  list.remove_all();
  ivRequisition req;
  lrScrollBoxInfo info;
  ivExtension e_i;
  const ivRequirement& r = req.x_requirement();
  
  ivCoord p = allocation_.left();
  ivCoord nextp = p;
  ivCoord right = allocation_.right();
  osboolean found_start = osfalse;
  
  for (ivGlyphIndex i = start_; i < n; i++) {
    ivGlyph* g = s->component(i);
    if (g != nil) {
      // get the width of this glyph
      g->request(req);
      ivCoord span = r.natural();
      if (!osMath::equal(span, ivCoord(0), float(1e-2))) {
	if (!found_start) {
	  start_ = i;
	  found_start = ostrue;
	}
	ivCoord alignment = r.alignment();
	p = nextp;
	nextp = p + span;
	if (nextp > right) {
	  end_ = i;
	  break;
	}
	info.glyph_ = g;
	
	ivAllotment& ay = info.allocation_.y_allotment();
	ay = allocation_.y_allotment();
	
	ivAllotment& ax = info.allocation_.x_allotment();
	ax.span(span);
	ax.origin(p + ivCoord(alignment * span));
	ax.alignment(alignment);
	list.append(info);
	g->allocate(canvas_, info.allocation_, e_i);
      }
    }
  }
}

void lrScrollBoxImpl::redraw() {
  if (canvas_ != nil) {
    canvas_->damage(extension_);
  }
}

void lrScrollBoxImpl::undraw_range(ivGlyphIndex begin, ivGlyphIndex end) {
  ivScrollBox* s = scrollbox_;
  for (ivGlyphIndex i = begin; i <= end; i++) {
    ivGlyph* g = s->component(i);
    if (g != nil) {
      g->undraw();
    }
  }
}
