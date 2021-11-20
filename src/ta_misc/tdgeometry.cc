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

// tdgeometry.cc

#include <ta_misc/tdgeometry.h>

TwoDCoord::TwoDCoord(const FloatTwoDCoord& cp) {
  Register(); Initialize(); x = (int)cp.x; y = (int)cp.y;
}

void TwoDCoord::operator=(const FloatTwoDCoord& cp) {
  x = (int)cp.x; y = (int)cp.y;
}

bool TwoDCoord::FitN(int n) {
  if((x * y) >= n)	return false;
  y = (int)sqrtf((float)n);
  if(y < 1)
    y = 1;
  x = n / y;
  while((x * y) < n)
    x++;
  return true;
}

bool TwoDCoord::WrapClipOne(bool wrap, int& c, int max) {
  if(c >= max) {
    if(wrap)	c = c % max;
    else	c = -1;
  }
  else if(c < 0) {
    if(wrap)    c = max + (c % max);
    else	c = -1;
  }
  if(c < 0)
    return true;
  return false;
}

void PosTwoDCoord::UpdateAfterEdit() {
  TwoDCoord::UpdateAfterEdit();
  SetGtEq(0);
}


TDCoord::TDCoord(const FloatTDCoord& cp) {
  Register(); Initialize();
  x = (int)cp.x; y = (int)cp.y; z = (int)cp.z;
}

void TDCoord::operator=(const FloatTDCoord& cp) {
  x = (int)cp.x; y = (int)cp.y;	z = (int)cp.z;
}

bool TDCoord::FitNinXY(int n) {
  if((x * y) >= n)	return false;
  y = (int)sqrtf((float)n);
  if(y < 1)
    y = 1;
  x = n / y;
  while((x * y) < n)
    x++;
  return true;
}

void PosTDCoord::UpdateAfterEdit() {
  TDCoord::UpdateAfterEdit();
  SetGtEq(0);
}

FloatTwoDCoord::FloatTwoDCoord(const TwoDCoord& cp) {
  Register(); Initialize(); x = (float)cp.x; y = (float)cp.y;
}

void FloatTwoDCoord::operator=(const TwoDCoord& cp) {
  x = (float)cp.x; y = (float)cp.y;
}

FloatTDCoord::FloatTDCoord(const TDCoord& cp) {
  Register(); Initialize(); 
  x = (float)cp.x; y = (float)cp.y; z = (float)cp.z;
}

void FloatTDCoord::operator=(const TDCoord& cp) {
  x = (float)cp.x; y = (float)cp.y; z = (float)cp.z;
}

