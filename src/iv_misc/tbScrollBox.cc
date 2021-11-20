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

#include <tbScrollBox.h>

// Tob-Bottom ScrollBox

struct tbScrollBoxInfo {
    /* workaround for g++ bug */
    tbScrollBoxInfo() { }
private:
    friend class tbScrollBox;
    friend class tbScrollBoxImpl;

    ivGlyph* glyph_;
    ivAllocation allocation_;
};

#include <ta/enter_iv.h>
declareList(tbScrollBoxList,tbScrollBoxInfo)
implementList(tbScrollBoxList,tbScrollBoxInfo)
#include <ta/leave_iv.h>

class tbScrollBoxImpl {
private:
  
friend class tbScrollBox;
  
  ivScrollBox*			scrollbox_;
  ivGlyphIndex			start_;
  ivGlyphIndex			end_;
  osboolean			changed_;
  ivRequisition			requisition_;
  ivCanvas*			canvas_;
  ivTransformer			transformer_;
  ivAllocation			allocation_;
  ivExtension			extension_;
  tbScrollBoxList		visible_;
  
  void check(ivCanvas*, const ivAllocation&);
  void refresh();
  void reallocate();
  void redraw();
  void undraw_range(ivGlyphIndex begin, ivGlyphIndex end);
};

tbScrollBox::tbScrollBox(ivGlyphIndex size) : ivScrollBox(size) {
    impl_ = new tbScrollBoxImpl;
    tbScrollBoxImpl& sb = *impl_;
    sb.scrollbox_ = this;
    sb.start_ = 0;
    sb.end_ = 0;
    sb.changed_ = ostrue;
    sb.canvas_ = nil;
    naturalnum = -1;
    default_height = 0;
}

tbScrollBox::~tbScrollBox() {
    delete impl_;
}


void tbScrollBox::request(ivRequisition& req) const {
    ivGlyphIndex n = count();
    tbScrollBoxImpl& sb = *impl_;
    if (sb.changed_) {
	ivRequisition r;
	const ivRequirement& rx = r.x_requirement();
	const ivRequirement& ry = r.y_requirement();
	ivCoord natural_width = 0.0;
	ivCoord natural_height = 0.0;

	if(naturalnum != -1) 
	  n = (naturalnum > n) ? n : naturalnum;
	if(n == 0) natural_height = naturalnum* default_height;

	ivCoord firstheight = 0;
	for (ivGlyphIndex i = 0; i < n; i++) {
	    ivGlyph* g = component(i);
	    if (g != nil) {
		g->request(r);
		if((i!=0) && (i==(n-1))){
		  firstheight = natural_height;
		}
		ivCoord r_width = rx.natural();
		if (r_width > natural_width) {
		    natural_width = r_width;
		}
		natural_height += ry.natural();
	    }
	}
	if(naturalnum!=-1) firstheight = 0;
	ivRequirement& box_x = sb.requisition_.x_requirement();
	box_x.natural(natural_width);
	box_x.stretch(fil);
	box_x.shrink(0);
	box_x.alignment(0.0);

	ivRequirement& box_y = sb.requisition_.y_requirement();
	box_y.natural(natural_height);
	box_y.stretch(fil);
	box_y.shrink(firstheight);
//	box_y.shrink(natural_height);
	box_y.alignment(1.0);
	sb.changed_ = osfalse;
    }
    req = sb.requisition_;
}

void tbScrollBox::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
    tbScrollBoxImpl& sb = *impl_;
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

void tbScrollBox::draw(ivCanvas* c, const ivAllocation& a) const {
    tbScrollBoxImpl& sb = *impl_;
    sb.check(c, a);
    ivExtension& e = sb.extension_;
    if (sb.canvas_->damaged(e)) {
	if (sb.changed_) {
	    sb.refresh();
	}
	c->push_clipping();
	c->clip_rect(a.left(), a.bottom(), a.right(), a.top());
	for (ListItr(tbScrollBoxList) i(sb.visible_); i.more(); i.next()) {
	    const tbScrollBoxInfo& info = i.cur_ref();
	    ivGlyph* g = info.glyph_;
	    g->draw(c, info.allocation_);
	}
	c->pop_clipping();
    }
}

void tbScrollBox::pick(ivCanvas* c, const ivAllocation& a, int depth, ivHit& h) {
    tbScrollBoxImpl& sb = *impl_;
    sb.check(c, a);
    if (h.left() < a.right() && h.right() >= a.left() &&
	h.bottom() < a.top() && h.top() >= a.bottom()
    ) {
	if (sb.changed_) {
	    sb.refresh();
	}
	ivGlyphIndex n = sb.start_;
	for (ListItr(tbScrollBoxList) i(sb.visible_); i.more(); i.next()) {
	    const tbScrollBoxInfo& info = i.cur_ref();
	    ivGlyph* g = info.glyph_;
	    h.begin(depth, this, n);
	    g->pick(c, info.allocation_, depth + 1, h);
	    h.end();
	    ++n;
	}
    }
}

void tbScrollBox::undraw() {
    impl_->canvas_ = nil;
    ivScrollBox::undraw();
}

void tbScrollBox::modified(ivGlyphIndex) {
    impl_->changed_ = ostrue;
}

osboolean tbScrollBox::shown(ivGlyphIndex i) const {
    tbScrollBoxImpl& sb = impl();
    return i >= sb.start_ && i < sb.end_;
}

ivGlyphIndex tbScrollBox::first_shown() const {
    tbScrollBoxImpl& sb = impl();
    return sb.start_;
}

ivGlyphIndex tbScrollBox::last_shown() const {
    tbScrollBoxImpl& sb = impl();
    return sb.end_ - 1;
}

void tbScrollBox::allotment(
    ivGlyphIndex i, DimensionName d, ivAllotment& a
) const {
    tbScrollBoxImpl& sb = impl();
    if (i >= sb.start_ && i < sb.end_) {
	a = sb.visible_.item(i - sb.start_).allocation_.allotment(d);
    }
}

ivCoord tbScrollBox::lower(DimensionName) const {
    return ivCoord(0);
}

ivCoord tbScrollBox::upper(DimensionName) const {
    return ivCoord(count() - 1);
}

ivCoord tbScrollBox::length(DimensionName) const {
    return ivCoord(count());
}

ivCoord tbScrollBox::cur_lower(DimensionName) const {
    tbScrollBoxImpl& sb = impl();
    return ivCoord(count() - sb.end_);
}

ivCoord tbScrollBox::cur_upper(DimensionName) const {
    tbScrollBoxImpl& sb = impl();
    return ivCoord(count() - 1 - sb.start_);
}

ivCoord tbScrollBox::cur_length(DimensionName) const {
    tbScrollBoxImpl& sb = impl();
    return ivCoord(sb.end_ - sb.start_);
}

void tbScrollBox::scroll_forward(DimensionName d) {
    scroll_by(d, -long(small_scroll(d)));
}

void tbScrollBox::scroll_backward(DimensionName d) {
    scroll_by(d, long(small_scroll(d)));
}

void tbScrollBox::page_forward(DimensionName d) {
    scroll_by(d, -long(large_scroll(d)));
}

void tbScrollBox::page_backward(DimensionName d) {
    scroll_by(d, long(large_scroll(d)));
}

void tbScrollBox::scroll_to(DimensionName d, ivCoord lower) {
    tbScrollBoxImpl& sb = impl();
    ivGlyphIndex max_end = count();
    ivGlyphIndex new_end = max_end - osMath::round(lower);
    ivGlyphIndex new_start = new_end - sb.end_ + sb.start_;
    do_scroll(d, new_start, new_end);
}

tbScrollBoxImpl& tbScrollBox::impl() const {
    tbScrollBoxImpl& sb = *impl_;
    if (sb.changed_) {
	sb.refresh();
    }
    return sb;
}

void tbScrollBox::scroll_by(DimensionName d, long offset) {
    tbScrollBoxImpl& sb = impl();
    do_scroll(d, sb.start_ + offset, sb.end_ + offset);
}

void tbScrollBox::do_scroll(
    DimensionName d, ivGlyphIndex new_start, ivGlyphIndex new_end
) {
    tbScrollBoxImpl& sb = *impl_;
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

/* class tbScrollBoxImpl */

void tbScrollBoxImpl::check(ivCanvas* c, const ivAllocation& a) {
    if (canvas_ == nil || canvas_ != c ||
	transformer_ != c->transformer() || !allocation_.equals(a, 1e-4)
    ) {
	ivExtension ext;
	scrollbox_->allocate(c, a, ext);
    }
}

void tbScrollBoxImpl::refresh() {
    ivRequisition req;
     scrollbox_->request(req);
    start_ = 0;
    reallocate();
    redraw();
}

void tbScrollBoxImpl::reallocate() {
    if (canvas_ == nil) {
	return;
    }
    ivScrollBox* s = scrollbox_;
    ivGlyphIndex n = s->count();
    end_ = n;
    tbScrollBoxList& list = visible_;
    list.remove_all();
    ivRequisition req;
    tbScrollBoxInfo info;
    ivExtension e_i;
    const ivRequirement& r = req.y_requirement();
    ivCoord p = allocation_.top();
    ivCoord bottom = allocation_.bottom();

    // vv is a hack which makes netview\'s Vars work on startup but
    // vv probably isn\'t always legal

    if (bottom <0) bottom = 0;

    osboolean found_start = osfalse;
    for (ivGlyphIndex i = start_; i < n; i++) {
	ivGlyph* g = s->component(i);
	if (g != nil) {
	    g->request(req);
	    ivCoord span = r.natural();
	    if (!osMath::equal(span, ivCoord(0), float(1e-2))) {
		if (!found_start) {
		    start_ = i;
		    found_start = ostrue;
		}
		ivCoord alignment = r.alignment();
		p -= span;
		if (p < bottom) {
		    end_ = i;
		    break;
		}
		info.glyph_ = g;
		ivAllotment& ax = info.allocation_.x_allotment();
		ax = allocation_.x_allotment();
		ivAllotment& ay = info.allocation_.y_allotment();
		ay.span(span);
		ay.origin(p + ivCoord(alignment * span));
		ay.alignment(alignment);
		list.append(info);
		g->allocate(canvas_, info.allocation_, e_i);
	    }
	}
    }
}

void tbScrollBoxImpl::redraw() {
    if (canvas_ != nil) {
	canvas_->damage(extension_);
    }
}

void tbScrollBoxImpl::undraw_range(ivGlyphIndex begin, ivGlyphIndex end) {
    ivScrollBox* s = scrollbox_;
    for (ivGlyphIndex i = begin; i <= end; i++) {
	ivGlyph* g = s->component(i);
	if (g != nil) {
	    g->undraw();
	}
    }
}
