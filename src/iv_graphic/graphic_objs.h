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


#ifndef graphic_objs_h
#define graphic_objs_h

#include <ta/enter_iv.h>
#include <InterViews/coord.h>
#include <IV-2_6/InterViews/minmax.h>
#include <OS/memory.h>
#include <OS/math.h>
#include <ta/leave_iv.h>

#include <ta/ta_stdef.h>
#include <math.h>

/*
 * Alignment needs to be unsigned so that it can be stored
 * as a bit field.
 */

enum Alignment {
  TopLeft = 0,
  TopCenter = 1,
  TopRight = 2,
  CenterLeft = 3,
  Center = 4,
  CenterRight = 5,
  BottomLeft = 6,
  BottomCenter = 7,
  BottomRight = 8,
  Left = 9,
  Right = 10,
  Top = 11,
  Bottom = 12,
  HorizCenter = 13,
  VertCenter = 14 
};

class PointObj {
public:
    PointObj(ivCoord = 0, ivCoord = 0);
    PointObj(PointObj*);

    ivCoord Distance(PointObj&);
public:
    ivCoord _x, _y;
};

class LineObj {
public:
    LineObj(ivCoord = 0, ivCoord = 0, ivCoord = 0, ivCoord = 0);
    LineObj(LineObj*);

    bool Contains(PointObj&);
    int Same(PointObj& p1, PointObj& p2);
    bool Intersects(LineObj&);
public:
    PointObj _p1, _p2;
};

class BoxObj {
public:
    BoxObj(ivCoord = 0, ivCoord = 0, ivCoord = 0, ivCoord = 0);
    BoxObj(BoxObj*);

    bool operator==(BoxObj&);
    bool Contains(PointObj&);
    bool Intersects(BoxObj&);
    bool Intersects(LineObj&);
    BoxObj operator-(BoxObj&);
    BoxObj operator+(BoxObj&);
    bool Within(BoxObj&);
public:
    ivCoord _left, _right;
    ivCoord _bottom, _top;
};

class MultiLineObj {
public:
    MultiLineObj(ivCoord* = nil, ivCoord* = nil, int = 0);

    void GetBox(BoxObj& b);
    bool Contains(PointObj&);
    bool Intersects(LineObj&);
    bool Intersects(BoxObj&);
    bool Within(BoxObj&);
    void SplineToMultiLine(ivCoord* cpx, ivCoord* cpy, int cpcount);
    void ClosedSplineToPolygon(ivCoord* cpx, ivCoord* cpy, int cpcount);
protected:
    void GrowBuf();
    bool CanApproxWithLine(
	double x0, double y0, double x2, double y2, double x3, double y3
    );
    void AddLine(double x0, double y0, double x1, double y1);
    void AddBezierArc(
        double x0, double y0, double x1, double y1,
        double x2, double y2, double x3, double y3
    );
    void CalcSection(
	ivCoord cminus1x, ivCoord cminus1y, ivCoord cx, ivCoord cy,
	ivCoord cplus1x, ivCoord cplus1y, ivCoord cplus2x, ivCoord cplus2y
    );
public:
    ivCoord* _x, *_y;
    int _count;
};

class FillPolygonObj : public MultiLineObj {
public:
    FillPolygonObj(ivCoord* = nil, ivCoord* = nil, int = 0);
    virtual ~FillPolygonObj();

    bool Contains(PointObj&);
    bool Intersects(LineObj&);
    bool Intersects(BoxObj&);
protected:
    void Normalize();
protected:
    ivCoord* _normx, *_normy;
    int _normCount;
};

class Extent {
public:
    Extent(ivCoord = 0, ivCoord = 0, ivCoord = 0, ivCoord = 0, ivCoord = 0);
    Extent(Extent&);

    bool Undefined();
    bool Within(Extent& e);
    void Merge(Extent&);
public:
    /* defines lower left and center of an object */
    ivCoord _left, _bottom, _cx, _cy, _tol;
};

/*
 * inlines
 */

inline bool Extent::Undefined () { return _left == _cx && _bottom == _cy; }

inline void exch (int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

inline int square(int a) { return a *= a; }
inline ivCoord square(ivCoord a) { return a *= a; }

inline ivCoord degrees(ivCoord rad) { return rad * 180.0 / M_PI; }
inline ivCoord radians(ivCoord deg) { return deg * M_PI / 180.0; }

inline ivCoord Distance(ivCoord x0, ivCoord y0, ivCoord x1, ivCoord y1) {
    return sqrt(ivCoord(square(x0 - x1) + square(y0 - y1)));
}

inline void ArrayCopy (
    const ivCoord* x, const ivCoord* y, int n, ivCoord* newx, ivCoord* newy
) {
    osMemory::copy(x, newx, n * sizeof(ivCoord));
    osMemory::copy(y, newy, n * sizeof(ivCoord));
}

inline void ArrayDup (
    const ivCoord* x, const ivCoord* y, int n, ivCoord*& newx, ivCoord*& newy
) {
    newx = new ivCoord[n];
    newy = new ivCoord[n];
    osMemory::copy(x, newx, n * sizeof(ivCoord));
    osMemory::copy(y, newy, n * sizeof(ivCoord));
}

inline void Midpoint (
    double x0, double y0, double x1, double y1, double& mx, double& my
) {
    mx = (x0 + x1) / 2.0;
    my = (y0 + y1) / 2.0;
}

inline void ThirdPoint (
    double x0, double y0, double x1, double y1, double& tx, double& ty
) {
    tx = (2*x0 + x1) / 3.0;
    ty = (2*y0 + y1) / 3.0;
}

#endif
