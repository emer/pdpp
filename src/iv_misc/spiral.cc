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

#include "spiral.h"

#include <InterViews/color.h>
#include <InterViews/canvas.h>
#include <InterViews/brush.h>
#include <OS/math.h>
#include <math.h>

Spiral::Spiral(float percent, const Color* dc,const Color* bc,int w, int h,int b) {
  border = b;
  drawcolor = dc;
  Resource::ref(drawcolor);
  backcolor = bc;
  Resource::ref(backcolor);
  width = w;
  height = h;
  if(percent > 1.0f) percent = 1.0f;
  else {
    if(percent > 0.0f) {
      area = (int) (percent * (width - (border *2)) * (height - (border *2)));
    }
    else area = 0; // percent <= 0.0f;
  }
  brush = new Brush(1);
  Resource::ref(brush);
}

Spiral::~Spiral(){
  Resource::unref(drawcolor);
  Resource::unref(backcolor);
  Resource::unref(brush);
}

void Spiral::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    ext.set(c, a);
}

void Spiral::request(Requisition& req) const {
  Requirement& rx = req.x_requirement();
  Requirement& ry = req.y_requirement();
  rx.natural(width);
  rx.stretch(0.0);
  rx.shrink(0.0);
  rx.alignment(0);
  ry.natural(height);
  ry.stretch(0.0);
  ry.shrink(0.0);
  ry.alignment(0);
}

void Spiral::draw(Canvas* c, const Allocation& a) const {
  if (c != nil) {

    c->fill_rect(a.left(),a.bottom(),a.right(),a.top(),backcolor);

    if(area == 0) return;
    Coord xspan = a.x_allotment().span();
    Coord yspan = a.y_allotment().span();
    Coord xorig = a.x_allotment().origin();
    Coord yorig = a.y_allotment().origin();

    PixelCoord pxspan = c->to_pixels(xspan) - border;
    PixelCoord pyspan = c->to_pixels(yspan) - border;
    PixelCoord pxorig = c->to_pixels(xorig) + border;
    PixelCoord pyorig = c->to_pixels(yorig) + border;

    PixelCoord boxsize = (PixelCoord) sqrt((double) area);
    PixelCoord d = boxsize * boxsize;

    PixelCoord	xboxoff,yboxoff;

    if(d%2 == 0){ 
      xboxoff = (pxorig + ((pxspan - boxsize)/2));
      yboxoff = (pyorig + ((pyspan - boxsize)/2)) ;
    }
    else {
      xboxoff = (pxorig + ((pxspan - boxsize)/2));
      yboxoff = (pyorig + ((pyspan - boxsize)/2));
    }
    c->fill_rect(c->to_coord(xboxoff),
		 c->to_coord(yboxoff + boxsize),
		 c->to_coord(xboxoff + boxsize),
		 c->to_coord(yboxoff),drawcolor);

    if(d==area) return;
    
    PixelCoord x,xmod;
    PixelCoord y,ymod;

    PixelCoord linedist;

    if((boxsize%2) == 0) {
      linedist = boxsize + 1;
      linedist = (PixelCoord) Math::min((int) linedist,(int) (area - d));
      x = xboxoff + boxsize  ; xmod = -linedist;
      y = yboxoff;
    }
    else {
      linedist = boxsize;
      linedist = (PixelCoord) Math::min((int)linedist, (int) (area - d));
      x = xboxoff; xmod = linedist;
      y = yboxoff + boxsize +1;
    }

    c->new_path();
    c->move_to(c->to_coord(x),c->to_coord(y));
    x += xmod;
    c->line_to(c->to_coord(x),c->to_coord(y));
    d += (int) linedist;

    if(d!=area)  {
      linedist = (PixelCoord)Math::min((int) linedist,(int)(area -d));
      if((boxsize%2)==0){
	ymod = linedist;
      }
      else {
	ymod = -linedist;
      }
      y += ymod;
      c->line_to(c->to_coord(x),c->to_coord(y));
    }
    c->stroke(drawcolor,brush);
  }
}
    

void Spiral::print(Printer* c, const Allocation& a) const {
  if (c != nil) {

    ((Canvas*) c)->fill_rect(a.left(),a.bottom(),a.right(),a.top(),backcolor);

    if(area == 0) return;
    Coord xspan = a.x_allotment().span() - border;
    Coord yspan = a.y_allotment().span() - border;
    Coord xorig = a.x_allotment().origin() + border;
    Coord yorig = a.y_allotment().origin() + border;


    int boxsize = (int) sqrt((double) area);
    int d = boxsize * boxsize;

    Coord	xboxoff,yboxoff;

    if(d % 2 == 0){ 
      xboxoff = (xorig + ((xspan - boxsize)/2));
      yboxoff = (yorig + ((yspan - boxsize)/2)) ;
    }
    else {
      xboxoff = (xorig + ((xspan - boxsize)/2));
      yboxoff = (yorig + ((yspan - boxsize)/2));
    }
    ((Canvas *) c)->fill_rect(xboxoff,(yboxoff + boxsize),
		 (xboxoff + boxsize),
		 (yboxoff),drawcolor);

    if(d==area) return;
    
    Coord x,xmod;
    Coord y,ymod;

    Coord linedist;

    if((boxsize%2) == 0) {
      linedist = boxsize + 1;
      linedist = (Coord) Math::min((int) linedist,(int) (area - d));
      x = xboxoff + boxsize  ; xmod = -linedist + .5;
      y = yboxoff;
    }
    else {
      linedist = boxsize;
      linedist = (Coord) Math::min((int)linedist, (int) (area - d));
      x = xboxoff; xmod = linedist ;
      y = yboxoff + boxsize +.5;
    }

    ((Canvas *) c)->new_path();
    ((Canvas *) c)->move_to((x),(y));
    x += xmod;
    ((Canvas *) c)->line_to((x),(y));
    d += (int) linedist;

    if(d!=area)  {
      linedist = (Coord)Math::min((int) linedist,(int)(area -d));
      if((boxsize%2)==0){
	ymod = linedist ;
      }
      else {
	ymod = -linedist;
      }
      y += ymod;
      ((Canvas *) c)->line_to((x),(y));
    }
    ((Canvas *) c)->stroke(drawcolor,brush);
  }
}
    

void Spiral::Set(float percent, const Color* dc, const Color *bc){
  area = (width - (border *2)) * (height - (border *2));
  if(percent > 1.0f) percent = 1.0f;
  else if( percent < 0.0f) percent = 0.0f;
  area = (int) (area * percent);
  Resource::ref(dc);
  Resource::unref(drawcolor);
  drawcolor = dc;
  if(bc) {
    Resource::ref(bc);
    Resource::unref(backcolor);
    backcolor = bc;
  }
}
