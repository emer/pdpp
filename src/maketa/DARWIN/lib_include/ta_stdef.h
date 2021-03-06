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

#ifndef ta_stdef_h
#define ta_stdef_h 1

// standard definitions, etc

#include <stdlib.h>
#include <string.h>
#ifdef __MAKETA__
#include <fstream.h>
#else
#include <fstream>
#endif
#include <math.h>
#include <ta_string/ta_string.h>

#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifdef List
#undef List
#endif

#ifdef SOLARIS
#include <sys/types.h>
#endif

typedef unsigned int 	uint;
typedef unsigned long   ulong;  

// replicate the builtin bool type for those that don't have it
#ifdef NO_BUILTIN_BOOL
typedef	unsigned int 	bool;

#ifdef true
#undef true
#endif
#define true 1
#ifdef false
#undef false
#endif
#define false 0
#endif

extern "C" {
#ifdef LINUX
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
#endif
#ifdef AIXV4
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
#endif
#if defined (SUN4) || defined(SUN4solaris)
extern "C" {
  extern unsigned sleep(unsigned);
}
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
#endif
// this is needed for pre 6.x IRIX, but not after that..
//  #if defined(SGI)
//  #define SIGNAL_PROC_FUN_TYPE void (*)(...)
//  #define SIGNAL_PROC_FUN_ARG(x) void (*x)(...)
//  #else
#define SIGNAL_PROC_FUN_TYPE void (*)(int)
#define SIGNAL_PROC_FUN_ARG(x) void (*x)(int)
//  #endif
  extern double acosh(double);
  extern double asinh(double);
  extern double atanh(double);
  extern double erf(double);
}

#endif // ta_stdef_h
