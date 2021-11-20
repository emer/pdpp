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


#include <iv_misc/grid.h>

#include <ta/enter_iv.h>
#include <InterViews/polyglyph.h>
#include <InterViews/aggr.h>
#include <InterViews/box.h>
#include <InterViews/group.h>
#include <InterViews/layout.h>
#include <ta/leave_iv.h>

Grid::Grid (int rows, int columns) : ivGlyph() {
    ivLayoutKit* layout = ivLayoutKit::instance();

    _row_count = rows;
    _column_count = columns;
    _cells = new ivAggregate(_row_count * _column_count);
    for (int i = 0; i < _row_count * _column_count; ++i) {
        _cells->insert(i, nil);
    }
    _columns = layout->hbox();
    for (int c = 0; c < _column_count; ++c) {
        ivGroup* g = new ivGroup(_cells, Dimension_X);
        for (int r = 0; r < _row_count; ++r) {
            g->map(r * _column_count + c);
        }
        _columns->insert(c, g);
    }
    _rows = layout->vbox();
    for (int r = 0; r < _row_count; ++r) {
        ivGroup* g = new ivGroup(_cells, Dimension_Y);
        for (int c = 0; c < _column_count; ++c) {
            g->map(r * _column_count + c);
        }
        _rows->insert(r, g);
    }
    _body = layout->overlay(
        layout->vcenter(_columns, 0.0),
        layout->vcenter(_rows, 0.0),
        _cells
    );
//    _body->ref();
    ivResource::ref(_body);
}

Grid::~Grid () {
//  _body->unref(); _body = nil;
  ivResource::unref(_body); _body = nil;
}

void Grid::cell (int row, int column, ivGlyph* glyph) {
    replace(row * _column_count + column, glyph);
}

void Grid::request(ivRequisition& requisition) const {
    _body->request(requisition);
}

void Grid::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
    _body->allocate(c, a, ext);
}

void Grid::draw(ivCanvas* canvas, const ivAllocation& a) const {
    _body->draw(canvas, a);
}

void Grid::print(ivPrinter* p, const ivAllocation& a) const {
    _body->print(p, a);
}

void Grid::pick(ivCanvas* c, const ivAllocation& a, int depth, ivHit& h) {
    _cells->pick(c, a, depth, h);
}

void Grid::append(ivGlyph*) { }

void Grid::prepend(ivGlyph*) { }

void Grid::insert(ivGlyphIndex, ivGlyph*) { }

void Grid::remove(ivGlyphIndex) { }

void Grid::replace(ivGlyphIndex index, ivGlyph* glyph) {
    _cells->replace(index, glyph);
    change(index);
}

ivGlyphIndex Grid::count() const {
    return _cells->count();
}

ivGlyphIndex Grid::rowcount() const {
  return _row_count;
}

ivGlyphIndex Grid::colcount() const {
  return _column_count;
}

ivGlyph* Grid::component(ivGlyphIndex index) const {
    return _cells->component(index);
}

ivGlyph* Grid::component(ivGlyphIndex row, ivGlyphIndex column) const {
  return _cells->component(row * _column_count + column);
}

void Grid::change(ivGlyphIndex row, ivGlyphIndex column){
  _rows->change(row);
  _columns->change(column);
  _body->change(0);
  _body->change(1);
  _body->change(2);
}  

void Grid::change(ivGlyphIndex index) {
    ivGlyphIndex row = index % _column_count;
    ivGlyphIndex column = index - row * _column_count;
    _rows->change(row);
    _columns->change(column);
    _body->change(0);
    _body->change(1);
    _body->change(2);
}

void Grid::allotment(
    ivGlyphIndex index, DimensionName res, ivAllotment& a
) const {
    _cells->allotment(index, res, a);
}
