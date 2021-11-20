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

/*
 * Read an idraw drawing from a file
 */

#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_idraw.h>

#include <ta/enter_iv.h>
#include <InterViews/brush.h>
#include <InterViews/color.h>
#include <InterViews/font.h>
#include <InterViews/psfont.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <IV-look/kit.h>
#include <OS/file.h>
#include <OS/list.h>
#include <OS/string.h>
#include <OS/memory.h>
#include <ta/leave_iv.h>

#include <stdlib.h>
#include <ctype.h>

class ivBrushInfo {
public:
    const ivBrush* brush_;
    int width_;
    int pattern_;
};

#include <ta/enter_iv.h>
declareList(ivBrushInfoList,ivBrushInfo)
implementList(ivBrushInfoList,ivBrushInfo)
#include <ta/leave_iv.h>

class Stipple : public virtual ivResource {
public:
    Stipple(float dither);

    float dither_;
protected:
    virtual ~Stipple ();
};

Stipple::Stipple (float dither) {
    dither_ = dither;
}

Stipple::~Stipple () { }

#include <ta/enter_iv.h>
declarePtrList(StippleList,Stipple)
implementPtrList(StippleList,Stipple)
#include <ta/leave_iv.h>

static Stipple* no_stipple = (Stipple*) -1;
static ivBrush* no_brush = (ivBrush*) -1;

struct FigureInfo {
    const char* name;
    bool brush;
    bool foreground;
    bool background;
    bool font;
    bool pattern;
    bool transformer;
    int coords;
    int skip;
};
    
static FigureInfo early_info[] = {
    { "Idraw", 1, 1, 1, 1, 1, 1, 0, 0 },
    { "BSpl",  1, 1, 1, 0, 1, 1, -1, 0 },
    { "Circ",  1, 1, 1, 0, 1, 1, 1, 0 },
    { "CBSpl", 1, 1, 1, 0, 1, 1, -1, 0 },
    { "Elli",  1, 1, 1, 0, 1, 1, 1, 0 },
    { "Line",  1, 1, 1, 0, 1, 1, 2, 0 },
    { "MLine", 1, 1, 1, 0, 1, 1, -1, 0 },
    { "Pict",  1, 1, 1, 1, 1, 1, 0, 0 },
    { "Poly",  1, 1, 1, 0, 1, 1, -1, 0 },
    { "Rect",  1, 1, 1, 0, 1, 1, 2, 0 },
    { "Text", 0, 1, 0, 1, 0, 1, 0, 0 },
    { "eop",  0, 0, 0, 0, 0, 0, 0, 0 },
    { nil, 0, 0, 0, 0, 0, 0, 0 }
};

static FigureInfo version_10_info[] = {
    { "Idraw", 1, 1, 1, 1, 1, 1, 0, 0 },
    { "BSpl",  1, 1, 1, 0, 1, 1, -1, 1 },
    { "Circ",  1, 1, 1, 0, 1, 1, 1, 0 },
    { "CBSpl", 1, 1, 1, 0, 1, 1, -1, 0 },
    { "Elli",  1, 1, 1, 0, 1, 1, 1, 0 },
    { "Line",  1, 1, 1, 0, 1, 1, 2, 1 },
    { "MLine", 1, 1, 1, 0, 1, 1, -1, 1 },
    { "Pict",  1, 1, 1, 1, 1, 1, 0, 0 },
    { "Poly",  1, 1, 1, 0, 1, 1, -1, 0 },
    { "Rect",  1, 1, 1, 0, 1, 1, 2, 0 },
    { "Text", 0, 1, 0, 1, 0, 1, 0, 0 },
    { "eop",  0, 0, 0, 0, 0, 0, 0, 0 },
    { nil, 0, 0, 0, 0, 0, 0, 0, 0 }
};

class IdrawReaderImpl {
    friend class IdrawReader;

    osInputFile* file_;
    int version_;
    FigureInfo* figinfo_;
    const char* start_;
    const char* end_;
    const char* cur_;
    ivBrushInfoList brushes_;
    StippleList stipples_;

    Graphic* load(
	const ivBrush* pb, const ivColor* pfg, const ivColor* pbg,
	const ivFont* pf, const Stipple* ps
    );
    bool fill();
    bool read(osString&);
    bool read(char&);
    bool read(int&);
    bool read(float&);
    const ivBrush* read_brush();
    const ivColor* read_color();
    const ivFont* read_font();
    const Stipple* read_stipple();
    ivTransformer* read_transformer();
    const ivColor* dither_color(const ivColor* fg, const ivColor* bg, float dither);
    void skip();
};

PolyGraphic* IdrawReader::load(osInputFile* f) {
    IdrawReaderImpl impl_;
    impl_.file_ = f;
    impl_.start_ = nil;
    impl_.end_ = nil;
    impl_.cur_ = nil;
    return (PolyGraphic*) impl_.load(nil, nil, nil, nil, nil);
}

PolyGraphic* IdrawReader::load(const char* filename) {
    PolyGraphic* gm = nil;
    osString s(filename);
    osInputFile* input = osInputFile::open(s);
    if (input != nil) {
        gm = load(input);
        delete input;
    }
    return gm;
}

static void Ref_ivBrush(const ivBrush* b) {
    if (b != no_brush) {
        ivResource::ref(b);
    }
}
static void Unref_ivBrush(const ivBrush* b) {
    if (b != no_brush) {
        ivResource::unref(b);
    }
}
static void Ref_Stipple(const Stipple* p) {
    if (p != no_stipple) {
        ivResource::ref(p);
    }
}
static void Unref_Stipple(const Stipple* p) {
    if (p != no_stipple) {
        ivResource::unref(p);
    }
}

Graphic* IdrawReaderImpl::load(
    const ivBrush* pb, const ivColor* pfg, const ivColor* pbg,
    const ivFont* pf, const Stipple* ps
) {
    Ref_ivBrush(pb);
    Ref_Stipple(ps);
    ivResource::ref(pfg);
    ivResource::ref(pbg);
    ivResource::ref(pf);
    
    skip();
    ivTransformer* tx = nil;
    Graphic* glyph = nil;
    osString name;
    if (read(name)) {
	if (name == "Idraw") {
	    read(version_);
	    figinfo_ = version_ < 10 ? early_info : version_10_info;
	}
	FigureInfo* fig;
	for (fig = &figinfo_[0]; fig->name != nil; fig++) {
	    if (name == fig->name) {
		break;
	    }
	}
        const ivBrush* b = fig->brush ? read_brush() : nil;
        const ivColor* fg = fig->foreground ? read_color() : nil;
        const ivColor* bg = fig->background ? read_color() : nil;
        const ivFont* f = fig->font ? read_font() : nil;
        const Stipple* s = fig->pattern ? read_stipple() : nil;
        if (fig->transformer) {
            tx = read_transformer();
        }
        Ref_ivBrush(b);
        Ref_Stipple(s);
        ivResource::ref(fg);
        ivResource::ref(bg);
        ivResource::ref(f);

        if (pb) {
            Unref_ivBrush(b);
	    b = pb;
            Ref_ivBrush(b);
	}
        if (pfg) {
            ivResource::unref(fg);
	    fg = pfg;
            ivResource::ref(fg);
	}
        if (pbg) {
            ivResource::unref(bg);
	    bg = pbg;
            ivResource::ref(bg);
	}
        if (pf) {
            ivResource::unref(f);
	    f = pf;
            ivResource::ref(f);
	}
        if (ps) {
            Unref_Stipple(s);
	    s = ps;
            Ref_Stipple(s);
	}
        if (fig->name == nil) {
	  taMisc::Error("Bad Idraw file! Aborting");
	  return nil;
	}
	osString figname(fig->name);
        if (figname == "Idraw" || figname == "Pict") {
            PolyGraphic* pg;
            if (figname == "Idraw") {
//                pg = new GraphicMaster;
	      pg = new PolyGraphic;
            } else {
                pg = new PolyGraphic;
            }
            pg->transformer(tx);
	    for (;;) {
                Graphic* g = load(b, fg, bg, f, s);
		if (g == nil) {
		    break;
		}
		pg->append_(g);
            }
            glyph = pg;

        } else if (figname == "eop") {
            glyph = nil;
        } else if (figname == "Text") {
            skip();
	    osString s;
	    read(s);
	    char ch;
	    read(ch);
            
            int bufsize = 256;
            char* buf = new char[bufsize];
            int i = 0;
            while (read(ch) && ch != ']') {
                buf[i++] = ch;
                if (i == bufsize) {
                    bufsize = bufsize*2;
                    char* newbuf = new char[bufsize];
                    osMemory::copy(buf, newbuf, i*sizeof(char));
                    delete buf;
                    buf = newbuf;
                }
            }
            buf[i] = '\0';
            glyph = new Text(f, fg, buf, tx);
            delete buf;
        } else {
            skip();
            int c = fig->coords;
            if (c == -1) { 
		read(c);
            }
            ivCoord* x = new ivCoord[c];
            ivCoord* y = new ivCoord[c];
            float xx, yy;
            for (int i = 0; i < c; ++i) {
		read(xx);
		read(yy);
		x[i] = xx;
		y[i] = yy;
            }

            const ivBrush* brush = (b != no_brush) ? b : nil;
            const ivColor* stroke = fg;
            const ivColor* fill = (
                (s != no_stipple) ? dither_color(fg, bg, s->dither_) : nil
            );
            if (figname == "Line") {
                glyph = new Line(
                    brush, stroke, fill, x[0], y[0], x[1], y[1], tx
                );
            } else if (figname == "BSpl") {
                glyph = new Open_BSpline(brush, stroke, fill, x, y, c, tx);
            } else if (figname == "CBSpl") {
                glyph = new Closed_BSpline(brush, stroke, fill, x, y, c, tx);
            } else if (figname == "MLine") {
                glyph = new Polyline(brush, stroke, fill, x, y, c, tx);
            } else if (figname == "Poly") {
                glyph = new Polygon(brush, stroke, fill, x, y, c, tx);
            } else if (figname == "Rect") {
                glyph = new Rectangle(
		    brush, stroke, fill, x[0], y[0], x[1], y[1], tx
		);
            } else if (figname == "Circ") {
		ivCoord radius;
		read(radius);
                glyph = new Circle(
                    brush, stroke, fill, x[0], y[0], radius, tx
                );
            } else if (figname == "Elli") {
		ivCoord r1, r2;
		read(r1);
		read(r2);
                glyph = new Ellipse(
                    brush, stroke, fill, x[0], y[0], r1, r2, tx
                );
            } else {
                glyph = nil;
            }
            delete x;
            delete y;
        }
        for (int extra = fig->skip; extra > 0; --extra) {
            skip();
        }
        Unref_ivBrush(b);
        Unref_Stipple(s);
        ivResource::unref(fg);
        ivResource::unref(bg);
        ivResource::unref(f);
        ivResource::unref(tx);

    }
    Unref_ivBrush(pb);
    Unref_Stipple(ps);
    ivResource::unref(pfg);
    ivResource::unref(pbg);
    ivResource::unref(pf);

    return glyph;
}

bool IdrawReaderImpl::fill() {
    if (cur_ >= end_) {
	int n = file_->read(start_);
	if (n <= 0) {
	    return false;
	}
	cur_ = start_;
	end_ = start_ + n;
    }
    return true;
}

bool IdrawReaderImpl::read(osString& s) {
    if (!fill()) {
	return false;
    }
    const char* p1 = cur_;
    while (p1 < end_ && isspace(*p1)) {
	++p1;
    }
    const char* p2 = p1;
    while (p2 < end_ && !isspace(*p2)) {
	++p2;
    }
    cur_ = p2;
    s = osString(p1, p2 - p1);
    return true;
}

bool IdrawReaderImpl::read(char& c) {
    if (!fill()) {
	return false;
    }
    c = *cur_++;
    return true;
}

bool IdrawReaderImpl::read(int& i) {
    osString s;
    return read(s) && s.convert(i);
}

bool IdrawReaderImpl::read(float& f) {
    osString s;
    return read(s) && s.convert(f);
}

void IdrawReaderImpl::skip() {
    osString s;
    while (read(s) && s != "%I") { }
}

const ivBrush* IdrawReaderImpl::read_brush() {
    skip();
    osString s;
    read(s);
    read(s);
    if (s == "u") {
        return nil;
    }
    if (s == "n") {
        return no_brush;
    }
    int pattern;
    int width;
    s.convert(pattern);
    read(width);

    for (ListItr(ivBrushInfoList) i(brushes_); i.more(); i.next()) {
	ivBrushInfo& br = i.cur_ref();
	if (br.width_ == width && br.pattern_ == pattern) {
	    return br.brush_;
	}
    }
    ivBrushInfo* b = new ivBrushInfo;
    b->brush_ = new ivBrush(pattern, ivCoord(width));
    ivResource::ref(b->brush_);
    b->width_ = width;
    b->pattern_ = pattern;
    brushes_.append(*b);
    return b->brush_;
}

const ivColor* IdrawReaderImpl::read_color() {
    skip();
    osString s;
    read(s);
    read(s);
    if (s == "u") {
	return nil;
    }
    float r, g, b;
    read(r);
    read(g);
    read(b);
    return new ivColor(r, g, b, 1.0);
}

const ivColor* IdrawReaderImpl::dither_color(
    const ivColor* f, const ivColor* b, float dither
) {
    float fr, fg, fb;
    float br, bg, bb;
    f->intensities(fr, fg, fb);
    b->intensities(br, bg, bb);
    float red = (1 - dither) * fr + dither * br;
    float green = (1 - dither) * fg + dither * bg;
    float blue = (1 - dither) * fb + dither * bb;
    return new ivColor(red, green, blue);
}

const ivFont* IdrawReaderImpl::read_font() {
    skip();
    osString s;
    read(s);
    read(s);
    if (s == "u") {
	return nil;
    }
    osString psname;
    read(psname);
    osNullTerminatedString psname_nt(psname);
    float pointsize;
    read(pointsize);
    if (ivPSFont::exists(psname_nt.string())) {
        osNullTerminatedString s_nt(s);
        return new ivPSFont(psname_nt.string(), pointsize, s_nt.string(), 1.0)
;
    }
// todo: do something better with the font here..
    return ivWidgetKit::instance()->font();
}

const Stipple* IdrawReaderImpl::read_stipple() {
    skip();
    osString s;
    read(s);
    read(s);
    if (s == "u") {
	return nil;
    }
    if (s == "n") {
        return no_stipple;
    }
    float dither;
    s.convert(dither);
    for (ListItr(StippleList) i(stipples_); i.more(); i.next()) {
	const Stipple* st = i.cur();
	if (st->dither_ == dither) {
	    return st;
	}
    }
    Stipple* st = new Stipple(dither);
    ivResource::ref(st);
    stipples_.append(st);
    return st;
}

ivTransformer* IdrawReaderImpl::read_transformer() {
    skip();
    osString s;
    read(s);
    read(s);
    if (s != "u") {
	float a[6];
	for (int i = 0; i < 6; i++) {
	    read(a[i]);
	}
        return new ivTransformer(a[0], a[1], a[2], a[3], a[4], a[5]);
    }
    return nil;
}
