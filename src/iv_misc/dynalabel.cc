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


#include <dynalabel.h>
#include <string.h>

#include <ta/enter_iv.h>
#include <InterViews/glyph.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/font.h>
#include <InterViews/hit.h>
#include <OS/string.h>
#include <ta/leave_iv.h>

#include <ta_string/ta_string.h>

DynamicLabel::DynamicLabel(const osString& s, const ivFont* f, const ivColor* c,
                           const char* form) : ivGlyph()
{
    _text = new osCopyString(s);
    _font = f;
    ivResource::ref(_font);
    _char_widths = NULL;
    _color = c;
    ivResource::ref(_color);
    compute_metrics();
    if (form) {
        _format = new char[strlen(form)+1];
        strcpy(_format,form);
    } else
        _format = nil;
    _c = nil;
}

DynamicLabel::DynamicLabel(const char* text, const ivFont* f, const ivColor* c,
                           const char* form) : ivGlyph()
{
    _text = new osCopyString(text);
    _font = f;
    ivResource::ref(_font);
    _char_widths = NULL;
    _color = c;
    ivResource::ref(_color);
    compute_metrics();
    if (form) {
        _format = new char[strlen(form)+1];
        strcpy(_format,form);
    } else
        _format = nil;
    _c = nil;
}

DynamicLabel::DynamicLabel(float v, const ivFont* f, const ivColor* c,
                           const char* form) : ivGlyph()
{
  String buffer;

  if(form)
    buffer = String(v,form);
  else
    buffer = String(v,"%g");
    
    _text = new osCopyString(buffer);
    _font = f;
    ivResource::ref(_font);
    _char_widths = NULL;
    _color = c;
    ivResource::ref(_color);
    compute_metrics();
    if (form) {
        _format = new char[strlen(form)+1];
        strcpy(_format,form);
    } else
        _format = nil;
    _c = nil;
}

DynamicLabel::~DynamicLabel() {
    delete _text;
    if(_font) ivResource::unref(_font);
    if(_color) ivResource::unref(_color);
    if(_char_widths) delete _char_widths;
    if (_format)
        delete _format;
}

void DynamicLabel::compute_metrics() {
    const ivFont* f = _font;
    const char* str = _text->string();
    int len = _text->length();
    ivFontBoundingBox b;
    f->string_bbox(str, len, b);
    _ascent = b.font_ascent()+1;
    _descent = b.font_descent()+1;
    _left = b.left_bearing()+1;
    _right = b.right_bearing()+1;
    _width = b.width()+2;
    if(_char_widths) delete _char_widths;
    _char_widths = new ivCoord[len];
    for (int i = 0; i < len; i++) {
        _char_widths[i] = f->width(((unsigned char*)str)[i]);
    }
}

void DynamicLabel::request(ivRequisition& requisition) const {
    ivCoord height = _ascent + _descent;
    float alignment = (height == 0) ? 0 : _descent / height;
    ivRequirement rx(_width, 0, 0, 0);
//    ivRequirement ry(height,0,0,0);
    ivRequirement ry(height, 0, 0, alignment);
    requisition.require(Dimension_X, rx);
    requisition.require(Dimension_Y, ry);
}

void DynamicLabel::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
    ivCoord x = a.x();
    ivCoord y = a.y();
    ext.set_xy(c, x - _left, y - _descent, x + _right, y + _ascent);
    DynamicLabel* v = this;
    v->_c = c;
    v->_a = a;
}

void DynamicLabel::draw(ivCanvas* c, const ivAllocation& a) const {
    ivCoord x = a.x();
    ivCoord y = a.y();
    const ivFont* f = _font;
    const ivColor* color = _color;
    const char* p = _text->string();
    const char* q = &p[_text->length()];
    ivCoord* cw = &_char_widths[0];
    for (; p < q; p++, cw++) {
        ivCoord width = *cw;
        c->character(f, *p, width, color, x, y);
        x += width;
    }
    DynamicLabel* v = (DynamicLabel*) this;
    v->_c = c;
    v->_a = a;
}

void DynamicLabel::pick(ivCanvas*, const ivAllocation& a, int depth, ivHit& h) {
    ivCoord x = h.left();
    if (h.right() >= a.left() && x < a.right() &&
        h.top() >= a.bottom() && h.bottom() < a.top()
    ) {
        int index = _font->index(
            _text->string(), _text->length(), x - a.x(), ostrue
        );
        h.target(depth, this, index);
    }
}

void DynamicLabel::set(const char *v) 
{
    delete _text;
    _text = new osCopyString(v);

    if (_c) {
        ivCoord x = _a.x();
        ivCoord y = _a.y();
        _c->damage(x - _left, y - _descent, x + _right, y + _ascent);
    }

    compute_metrics();

    if (_c) {
        ivCoord x = _a.x();
        ivCoord y = _a.y();
        _c->damage(x - _left, y - _descent, x + _right, y + _ascent);
    }
}

void DynamicLabel::set(float num) 
{
    if (_format)
      set((const char*)String(num, _format));
    else
      set((const char*)String(num, "%g"));
}

void DynamicLabel::SetColor(const ivColor* c) {
  ivResource::ref(c);
  if(_color != NULL) ivResource::unref(_color);
  _color = c;
}

void DynamicLabel::undraw()
{
    _c = nil;
}
