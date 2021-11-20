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
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// all unadorned references to string are hereby taString references
// which should be used in the case that there are multiple conflicting string
// types in use.

#ifdef String
#undef String
#endif
#define String taString
#define SubString taSubString
#define StrRep taStrRep

#ifndef ta_string_h
#define ta_string_h 1

// ta_string is the string class used in the TA/CSS software
// it is based on the gnu libg++ string class, with the following
// modifications: 
// - constructors and conversion operators to/from ints, floats, and doubles
// - removal of the regexp versions of functions
// - reordering and commenting of overloaded functions so that the 
// 	const char* version is always the last one, so that it will
//	be the one scanned by TA

#ifdef __MAKETA__
#include <iostream.h>
#else
#include <iostream>
#endif

#include <stdlib.h>

using namespace std;

class StrRep {                   // internal String representations
public:
  unsigned short    len;         // string length 
  unsigned short    sz;          // allocated space
  char              s[1];        // the string starts here 
                                 // (at least 1 char for trailing null)
                                 // allocated & expanded via non-public fcts
};

// primitive ops on StrReps -- nearly all String fns go through these.

StrRep*     Salloc(StrRep*, const char*, int, int);
StrRep*     Scopy(StrRep*, const StrRep*);
StrRep*     Scat(StrRep*, const char*, int, const char*, int);
StrRep*     Scat(StrRep*, const char*, int,const char*,int, const char*,int);
StrRep*     Sprepend(StrRep*, const char*, int);
StrRep*     Sreverse(const StrRep*, StrRep*);
StrRep*     Supcase(const StrRep*, StrRep*);
StrRep*     Sdowncase(const StrRep*, StrRep*);
StrRep*     Scapitalize(const StrRep*, StrRep*);

// These classes need to be defined in the order given

class String;
class SubString;

class SubString
{
  friend class      String;
protected:

  String&           S;        // The String I'm a substring of
  unsigned short    pos;      // starting position in S's rep
  unsigned short    len;      // length of substring

  void              assign(const StrRep*, const char*, int = -1);
                    SubString(String& x, int p, int l);
public:
                    SubString(const SubString& x);

public:

// Note there are no public constructors. SubStrings are always
// created via String operations

                   ~SubString();

  SubString&        operator =  (const String&     y);
  SubString&        operator =  (const SubString&  y);
  SubString&        operator =  (const char* t);
  SubString&        operator =  (char        c);

// return 1 if target appears anywhere in SubString; else 0

  int               contains(char        c) const;
  int               contains(const String&     y) const;
  int               contains(const SubString&  y) const;
  int               contains(const char* t) const;

// return 1 if target matches entire SubString

// IO 

  friend ostream&   operator<<(ostream& s, const SubString& x);

// status

  unsigned int      length() const;
  int               empty() const;
  inline const char*       chars() const;

  int               OK() const; 

};


class String
{
  friend class      SubString;

protected:
  StrRep*           rep;   // Strings are pointers to their representations

// some helper functions

  int               search(int, int, const char*, int = -1) const;
  int               search(int, int, char) const;
  int               match(int, int, int, const char*, int = -1) const;
  int               _gsub(const char*, int, const char* ,int);
  SubString         _substr(int, int);

public:

// constructors & assignment

                    String();
                    String(const String& x);
                    String(const SubString&  x);
                    String(const char* t);
                    String(const char* t, int len);
                    String(char c);

// these are strangely missing in the original
                    String(int i,const char* format = "%d");
                    String(long i,const char* format = "%ld");
                    String(float f,const char* format = "%g");
                    String(double f,const char* format = "%lg");
                    String(void* f,const char* format = "%lg");

                    ~String();

  const char* 	    chars() const { return &(rep->s[0]); }

// these are strangely missing in the original	
  operator int() const	{ return strtol(chars(), NULL, 0); }
  operator long() const	{ return strtol(chars(), NULL, 0); }
  operator float() const { return atof(chars()); }
  operator double() const { return atof(chars()); }
  operator char*() const { return (char*)chars(); }

  String&           operator =  (const String&     y);
  String&           operator =  (const char* y);
  String&           operator =  (char        c);
  String&           operator =  (const SubString&  y);

// concatenation

  String&           operator += (const String&     y); 
  String&           operator += (const SubString&  y);
  String&           operator += (const char* t);
  String&           operator += (char        c);

  void              prepend(const String&     y); 
  void              prepend(const SubString&  y);
  void              prepend(char        c);
  void              prepend(const char* t); // add to beginning of string


// procedural versions:
// concatenate first 2 args, store result in last arg

  friend inline void     cat(const String&, const String&, String&);
  friend inline void     cat(const String&, const SubString&, String&);
  friend inline void     cat(const String&, const char*, String&);
  friend inline void     cat(const String&, char, String&);

  friend inline void     cat(const SubString&, const String&, String&);
  friend inline void     cat(const SubString&, const SubString&, String&);
  friend inline void     cat(const SubString&, const char*, String&);
  friend inline void     cat(const SubString&, char, String&);

  friend inline void     cat(const char*, const String&, String&);
  friend inline void     cat(const char*, const SubString&, String&);
  friend inline void     cat(const char*, const char*, String&);
  friend inline void     cat(const char*, char, String&);

// double concatenation, by request. (yes, there are too many versions, 
// but if one is supported, then the others should be too...)
// Concatenate first 3 args, store in last arg

  friend inline void     cat(const String&,const String&, const String&,String&);
  friend inline void     cat(const String&,const String&,const SubString&,String&);
  friend inline void     cat(const String&,const String&, const char*, String&);
  friend inline void     cat(const String&,const String&, char, String&);
  friend inline void     cat(const String&,const SubString&,const String&,String&);
  friend inline void     cat(const String&,const SubString&,const SubString&,String&);
  friend inline void     cat(const String&,const SubString&, const char*, String&);
  friend inline void     cat(const String&,const SubString&, char, String&);
  friend inline void     cat(const String&,const char*, const String&,    String&);
  friend inline void     cat(const String&,const char*, const SubString&, String&);
  friend inline void     cat(const String&,const char*, const char*, String&);
  friend inline void     cat(const String&,const char*, char, String&);

  friend inline void     cat(const char*, const String&, const String&,String&);
  friend inline void     cat(const char*,const String&,const SubString&,String&);
  friend inline void     cat(const char*,const String&, const char*, String&);
  friend inline void     cat(const char*,const String&, char, String&);
  friend inline void     cat(const char*,const SubString&,const String&,String&);
  friend inline void     cat(const char*,const SubString&,const SubString&,String&);
  friend inline void     cat(const char*,const SubString&, const char*, String&);
  friend inline void     cat(const char*,const SubString&, char, String&);
  friend inline void     cat(const char*,const char*, const String&,    String&);
  friend inline void     cat(const char*,const char*, const SubString&, String&);
  friend inline void     cat(const char*,const char*, const char*, String&);
  friend inline void     cat(const char*,const char*, char, String&);


// searching & matching

// return position of target in string or -1 for failure

  int               index(char        c, int startpos = 0) const;      
  int               index(const String&     y, int startpos = 0) const;      
  int               index(const SubString&  y, int startpos = 0) const;      
  int               index(const char* t, int startpos = 0) const;  
// return position of target in string or -1 for failure

// return 1 if target appears anyhere in String; else 0

  int               contains(char        c) const;
  int               contains(const String&     y) const;
  int               contains(const SubString&  y) const;

// return 1 if target appears anywhere after position pos 
// (or before, if pos is negative) in String; else 0

  int               contains(char        c, int pos) const;
  int               contains(const String&     y, int pos) const;
  int               contains(const SubString&  y, int pos) const;
  int               contains(const char* t, int pos) const;
  int               contains(const char* t) const;
// return 1 if target appears anywhere in string

// return 1 if target appears at position pos in String; else 0

  int               matches(char        c, int pos = 0) const;
  int               matches(const String&     y, int pos = 0) const;
  int               matches(const SubString&  y, int pos = 0) const;
  int               matches(const char* t, int pos = 0) const;
// return 1 if target appears at position pos in String; else 0

//  return number of occurences of target in String

  int               freq(char        c) const; 
  int               freq(const String&     y) const;
  int               freq(const SubString&  y) const;
  int               freq(const char* t) const;
//  return number of occurences of target in String

// SubString extraction

// Note that you can't take a substring of a const String, since
// this leaves open the possiblility of indirectly modifying the
// String through the SubString

  SubString         operator () (int         pos, int len); // synonym for at

  SubString         at(const String&     x, int startpos = 0); 
  SubString         at(const SubString&  x, int startpos = 0); 
  SubString         at(const char* t, int startpos = 0);
  SubString         at(char        c, int startpos = 0);
  SubString         at(int         pos, int len);
  // substring at position for length

  SubString         before(int          pos);
  SubString         before(const String&      x, int startpos = 0);
  SubString         before(const SubString&   x, int startpos = 0);
  SubString         before(char         c, int startpos = 0);
  SubString         before(const char*  t, int startpos = 0);
  // substring before (not including) target string

  SubString         through(int          pos);
  SubString         through(const String&      x, int startpos = 0);
  SubString         through(const SubString&   x, int startpos = 0);
  SubString         through(char         c, int startpos = 0);
  SubString         through(const char*  t, int startpos = 0);
  // substring through (including) target string

  SubString         from(int          pos);
  SubString         from(const String&      x, int startpos = 0);
  SubString         from(const SubString&   x, int startpos = 0);
  SubString         from(char         c, int startpos = 0);
  SubString         from(const char*  t, int startpos = 0);
  // substring from (including) target string

  SubString         after(int         pos);
  SubString         after(const String&     x, int startpos = 0);
  SubString         after(const SubString&  x, int startpos = 0);
  SubString         after(char        c, int startpos = 0);
  SubString         after(const char* t, int startpos = 0);
  // substring after (not including) target string


// deletion


// delete the first occurrence of target after startpos

  void              del(const String&     y, int startpos = 0);
  void              del(const SubString&  y, int startpos = 0);
  void              del(const char* t, int startpos = 0);
  void              del(char        c, int startpos = 0);
  void              del(int         pos, int len);
// delete len chars starting at pos
  void              remove(const char* t, int startpos = 0) { del(t, startpos); }
// remove target string from string

// global substitution: substitute all occurrences of pat with repl

  int               gsub(const String&     pat, const String&     repl);
  int               gsub(const SubString&  pat, const String&     repl);
  int               gsub(const char* pat, const String&     repl);
  int               gsub(const char* pat, const char* repl);
// global substitution: substitute all occurrences of pat with repl

// friends & utilities

// split string into array res at separators; return number of elements

  friend int        split(const String& x, String res[], int maxn, 
                          const String& sep);

  friend String     common_prefix(const String& x, const String& y, 
                                  int startpos = 0);
  friend String     common_suffix(const String& x, const String& y, 
                                  int startpos = -1);
  friend String     replicate(char        c, int n);
  friend String     replicate(const String&     y, int n);
  friend String     join(String src[], int n, const String& sep);

// simple builtin transformations

  friend inline String     reverse(const String& x);
  friend inline String     upcase(const String& x);
  friend inline String     downcase(const String& x);
  friend inline String     capitalize(const String& x);

// in-place versions of above

  void              reverse();	// reverse order of string
  void              upcase();	// convert all letters to upper case
  void              downcase();	// convert all letters to lower case
  void              capitalize(); // capitalize the first letter of each word

// element extraction

  char&             operator [] (int i);
  const char&       operator [] (int i) const;
  char              elem(int i) const; // get the character at index i
  char              firstchar() const; // get the first character
  char              lastchar() const;  // get the last character

// conversion

                    operator const char*() const;


// IO

  friend inline ostream&   operator<<(ostream& s, const String& x);
  friend ostream&   operator<<(ostream& s, const SubString& x);
  friend istream&   operator>>(istream& s, String& x);

  friend int        readline(istream& s, String& x, 
                             char terminator = '\n',
                             int discard_terminator = 1);

// status

  unsigned int      length() const; // how many characters in the string
  int               empty() const;  // true if the string is empty

  void              alloc(int newsize);
// preallocate some space for String


  int               allocation() const;
// report current allocation (not length!)


  void     error(const char* msg) const;

  int               OK() const;	// check if the string is allocated properly, etc.
};

//typedef String StrTmp; // for backward compatibility

// other externs

int        compare(const String&    x, const String&     y);
int        compare(const String&    x, const SubString&  y);
int        compare(const String&    x, const char* y);
int        compare(const SubString& x, const String&     y);
int        compare(const SubString& x, const SubString&  y);
int        compare(const SubString& x, const char* y);
int        fcompare(const String&   x, const String&     y); // ignore case

extern StrRep  _nilStrRep;
extern String _nilString;

// status reports, needed before defining other things

inline unsigned int String::length() const {  return rep->len; }
inline int         String::empty() const { return rep->len == 0; }
inline int         String::allocation() const { return rep->sz; }

inline unsigned int SubString::length() const { return len; }
inline int         SubString::empty() const { return len == 0; }
inline const char* SubString::chars() const { return &(S.rep->s[pos]); }


// constructors

inline String::String() 
  : rep(&_nilStrRep) {}
inline String::String(const String& x) 
  : rep(Scopy(0, x.rep)) {}
inline String::String(const char* t) 
  : rep(Salloc(0, t, -1, -1)) {}
inline String::String(const char* t, int tlen)
  : rep(Salloc(0, t, tlen, tlen)) {}
inline String::String(const SubString& y)
  : rep(Salloc(0, y.chars(), y.length(), y.length())) {}
inline String::String(char c) 
  : rep(Salloc(0, &c, 1, 1)) {}

inline String::~String() { if (rep != &_nilStrRep) delete rep; }

inline SubString::SubString(const SubString& x)
  :S(x.S), pos(x.pos), len(x.len) {}
inline SubString::SubString(String& x, int first, int l)
  :S(x), pos(first), len(l) {}

inline SubString::~SubString() {}

// assignment

inline String& String::operator =  (const String& y)
{ 
  rep = Scopy(rep, y.rep);
  return *this;
}

inline String& String::operator=(const char* t)
{
  rep = Salloc(rep, t, -1, -1);
  return *this;
}

inline String& String::operator=(const SubString&  y)
{
  rep = Salloc(rep, y.chars(), y.length(), y.length());
  return *this;
}

inline String& String::operator=(char c)
{
  rep = Salloc(rep, &c, 1, 1);
  return *this;
}


inline SubString& SubString::operator = (const char* ys)
{
  assign(0, ys);
  return *this;
}

inline SubString& SubString::operator = (char ch)
{
  assign(0, &ch, 1);
  return *this;
}

inline SubString& SubString::operator = (const String& y)
{
  assign(y.rep, y.chars(), y.length());
  return *this;
}

inline SubString& SubString::operator = (const SubString& y)
{
  assign(y.S.rep, y.chars(), y.length());
  return *this;
}

// Zillions of cats...

inline void cat(const String& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y, -1);
}

inline void cat(const String& x, char y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), &y, 1);
}

inline void cat(const SubString& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const SubString& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const SubString& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), y, -1);
}

inline void cat(const SubString& x, char y, String& r)
{
  r.rep = Scat(r.rep, x.chars(), x.length(), &y, 1);
}

inline void cat(const char* x, const String& y, String& r)
{
  r.rep = Scat(r.rep, x, -1, y.chars(), y.length());
}

inline void cat(const char* x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, x, -1, y.chars(), y.length());
}

inline void cat(const char* x, const char* y, String& r)
{
  r.rep = Scat(r.rep, x, -1, y, -1);
}

inline void cat(const char* x, char y, String& r)
{
  r.rep = Scat(r.rep, x, -1, &y, 1);
}

inline void cat(const String& a, const String& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& a, const String& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& a, const String& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y, -1);
}

inline void cat(const String& a, const String& x, char y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), &y, 1);
}

inline void cat(const String& a, const SubString& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& a, const SubString& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const String& a, const SubString& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), y, -1);
}

inline void cat(const String& a, const SubString& x, char y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x.chars(), x.length(), &y, 1);
}

inline void cat(const String& a, const char* x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x, -1, y.chars(), y.length());
}

inline void cat(const String& a, const char* x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x, -1, y.chars(), y.length());
}

inline void cat(const String& a, const char* x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x, -1, y, -1);
}

inline void cat(const String& a, const char* x, char y, String& r)
{
  r.rep = Scat(r.rep, a.chars(), a.length(), x, -1, &y, 1);
}


inline void cat(const char* a, const String& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const char* a, const String& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const char* a, const String& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y, -1);
}

inline void cat(const char* a, const String& x, char y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), &y, 1);
}

inline void cat(const char* a, const SubString& x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const char* a, const SubString& x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y.chars(), y.length());
}

inline void cat(const char* a, const SubString& x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), y, -1);
}

inline void cat(const char* a, const SubString& x, char y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x.chars(), x.length(), &y, 1);
}

inline void cat(const char* a, const char* x, const String& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x, -1, y.chars(), y.length());
}

inline void cat(const char* a, const char* x, const SubString& y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x, -1, y.chars(), y.length());
}

inline void cat(const char* a, const char* x, const char* y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x, -1, y, -1);
}

inline void cat(const char* a, const char* x, char y, String& r)
{
  r.rep = Scat(r.rep, a, -1, x, -1, &y, 1);
}


// operator versions

inline String& String::operator +=(const String& y)
{
  cat(*this, y, *this);
  return *this;
}

inline String& String::operator +=(const SubString& y)
{
  cat(*this, y, *this);
  return *this;
}

inline String& String::operator += (const char* y)
{
  cat(*this, y, *this);
  return *this;
}

inline String& String:: operator +=(char y)
{
  cat(*this, y, *this);
  return *this;
}

// constructive concatenation

inline String operator + (const String& x, const String& y)
{
  String r;  cat(x, y, r);  return r;
}

inline String operator + (const String& x, const SubString& y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const String& x, const char* y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const String& x, char y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const SubString& x, const String& y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const SubString& x, const SubString& y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const SubString& x, const char* y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const SubString& x, char y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const char* x, const String& y) 
{
  String r; cat(x, y, r); return r;
}

inline String operator + (const char* x, const SubString& y) 
{
  String r; cat(x, y, r); return r;
}

inline String reverse(const String& x) 
{
  String r; r.rep = Sreverse(x.rep, r.rep); return r;
}

inline String upcase(const String& x) 
{
  String r; r.rep = Supcase(x.rep, r.rep); return r;
}

inline String downcase(const String& x) 
{
  String r; r.rep = Sdowncase(x.rep, r.rep); return r;
}

inline String capitalize(const String& x) 
{
  String r; r.rep = Scapitalize(x.rep, r.rep); return r;
}

// prepend

inline void String::prepend(const String& y)
{
  rep = Sprepend(rep, y.chars(), y.length());
}

inline void String::prepend(const char* y)
{
  rep = Sprepend(rep, y, -1); 
}

inline void String::prepend(char y)
{
  rep = Sprepend(rep, &y, 1); 
}

inline void String::prepend(const SubString& y)
{
  rep = Sprepend(rep, y.chars(), y.length());
}

// misc transformations


inline void String::reverse()
{
  rep = Sreverse(rep, rep);
}


inline void String::upcase()
{
  rep = Supcase(rep, rep);
}


inline void String::downcase()
{
  rep = Sdowncase(rep, rep);
}


inline void String::capitalize()
{
  rep = Scapitalize(rep, rep);
}

// element extraction

inline char&  String::operator [] (int i) 
{ 
  if (((unsigned)i) >= length()) error("invalid index");
  return rep->s[i];
}

inline const char&  String::operator [] (int i) const
{ 
  if (((unsigned)i) >= length()) error("invalid index");
  return rep->s[i];
}

inline char  String::elem (int i) const
{ 
  if (((unsigned)i) >= length()) error("invalid index");
  return rep->s[i];
}

inline char  String::firstchar() const
{ 
  return elem(0);
}

inline char  String::lastchar() const
{ 
  return elem(length() - 1);
}

// searching

inline int String::index(char c, int startpos) const
{
  return search(startpos, length(), c);
}

inline int String::index(const char* t, int startpos) const
{   
  return search(startpos, length(), t);
}

inline int String::index(const String& y, int startpos) const
{   
  return search(startpos, length(), y.chars(), y.length());
}

inline int String::index(const SubString& y, int startpos) const
{   
  return search(startpos, length(), y.chars(), y.length());
}

inline int String::contains(char c) const
{
  return search(0, length(), c) >= 0;
}

inline int String::contains(const char* t) const
{   
  return search(0, length(), t) >= 0;
}

inline int String::contains(const String& y) const
{   
  return search(0, length(), y.chars(), y.length()) >= 0;
}

inline int String::contains(const SubString& y) const
{   
  return search(0, length(), y.chars(), y.length()) >= 0;
}

inline int String::contains(char c, int p) const
{
  return match(p, length(), 0, &c, 1) >= 0;
}

inline int String::contains(const char* t, int p) const
{
  return match(p, length(), 0, t) >= 0;
}

inline int String::contains(const String& y, int p) const
{
  return match(p, length(), 0, y.chars(), y.length()) >= 0;
}

inline int String::contains(const SubString& y, int p) const
{
  return match(p, length(), 0, y.chars(), y.length()) >= 0;
}


inline int String::matches(const SubString& y, int p) const
{
  return match(p, length(), 1, y.chars(), y.length()) >= 0;
}

inline int String::matches(const String& y, int p) const
{
  return match(p, length(), 1, y.chars(), y.length()) >= 0;
}

inline int String::matches(const char* t, int p) const
{
  return match(p, length(), 1, t) >= 0;
}

inline int String::matches(char c, int p) const
{
  return match(p, length(), 1, &c, 1) >= 0;
}


inline int SubString::contains(const char* t) const
{   
  return S.search(pos, pos+len, t) >= 0;
}

inline int SubString::contains(const String& y) const
{   
  return S.search(pos, pos+len, y.chars(), y.length()) >= 0;
}

inline int SubString::contains(const SubString&  y) const
{   
  return S.search(pos, pos+len, y.chars(), y.length()) >= 0;
}

inline int SubString::contains(char c) const
{
  return S.search(pos, pos+len, c) >= 0;
}


inline int String::gsub(const String& pat, const String& r)
{
  return _gsub(pat.chars(), pat.length(), r.chars(), r.length());
}

inline int String::gsub(const SubString&  pat, const String& r)
{
  return _gsub(pat.chars(), pat.length(), r.chars(), r.length());
}

inline int String::gsub(const char* pat, const String& r)
{
  return _gsub(pat, -1, r.chars(), r.length());
}

inline int String::gsub(const char* pat, const char* r)
{
  return _gsub(pat, -1, r, -1);
}



inline  ostream& operator<<(ostream& s, const String& x)
{
   s << x.chars(); return s;
}

// a zillion comparison operators

inline int operator==(const String& x, const String& y) 
{
  return compare(x, y) == 0; 
}

inline int operator!=(const String& x, const String& y)
{
  return compare(x, y) != 0; 
}

inline int operator>(const String& x, const String& y)
{
  return compare(x, y) > 0; 
}

inline int operator>=(const String& x, const String& y)
{
  return compare(x, y) >= 0; 
}

inline int operator<(const String& x, const String& y)
{
  return compare(x, y) < 0; 
}

inline int operator<=(const String& x, const String& y)
{
  return compare(x, y) <= 0; 
}

inline int operator==(const String& x, const SubString&  y) 
{
  return compare(x, y) == 0; 
}

inline int operator!=(const String& x, const SubString&  y)
{
  return compare(x, y) != 0; 
}

inline int operator>(const String& x, const SubString&  y)      
{
  return compare(x, y) > 0; 
}

inline int operator>=(const String& x, const SubString&  y)
{
  return compare(x, y) >= 0; 
}

inline int operator<(const String& x, const SubString&  y) 
{
  return compare(x, y) < 0; 
}

inline int operator<=(const String& x, const SubString&  y)
{
  return compare(x, y) <= 0; 
}

inline int operator==(const String& x, const char* t) 
{
  return compare(x, t) == 0; 
}

inline int operator!=(const String& x, const char* t) 
{
  return compare(x, t) != 0; 
}

inline int operator>(const String& x, const char* t)  
{
  return compare(x, t) > 0; 
}

inline int operator>=(const String& x, const char* t) 
{
  return compare(x, t) >= 0; 
}

inline int operator<(const String& x, const char* t)  
{
  return compare(x, t) < 0; 
}

inline int operator<=(const String& x, const char* t) 
{
  return compare(x, t) <= 0; 
}

inline int operator==(const SubString& x, const String& y) 
{
  return compare(y, x) == 0; 
}

inline int operator!=(const SubString& x, const String& y)
{
  return compare(y, x) != 0;
}

inline int operator>(const SubString& x, const String& y)      
{
  return compare(y, x) < 0;
}

inline int operator>=(const SubString& x, const String& y)     
{
  return compare(y, x) <= 0;
}

inline int operator<(const SubString& x, const String& y)      
{
  return compare(y, x) > 0;
}

inline int operator<=(const SubString& x, const String& y)     
{
  return compare(y, x) >= 0;
}

inline int operator==(const SubString& x, const SubString&  y) 
{
  return compare(x, y) == 0; 
}

inline int operator!=(const SubString& x, const SubString&  y)
{
  return compare(x, y) != 0;
}

inline int operator>(const SubString& x, const SubString&  y)      
{
  return compare(x, y) > 0;
}

inline int operator>=(const SubString& x, const SubString&  y)
{
  return compare(x, y) >= 0;
}

inline int operator<(const SubString& x, const SubString&  y) 
{
  return compare(x, y) < 0;
}

inline int operator<=(const SubString& x, const SubString&  y)
{
  return compare(x, y) <= 0;
}

inline int operator==(const SubString& x, const char* t) 
{
  return compare(x, t) == 0; 
}

inline int operator!=(const SubString& x, const char* t) 
{
  return compare(x, t) != 0;
}

inline int operator>(const SubString& x, const char* t)  
{
  return compare(x, t) > 0; 
}

inline int operator>=(const SubString& x, const char* t) 
{
  return compare(x, t) >= 0; 
}

inline int operator<(const SubString& x, const char* t)  
{
  return compare(x, t) < 0; 
}

inline int operator<=(const SubString& x, const char* t) 
{
  return compare(x, t) <= 0; 
}


// a helper needed by at, before, etc.

inline SubString String::_substr(int first, int l)
{
  if (first < 0 || (unsigned)(first + l) > length() )
    return SubString(_nilString, 0, 0) ;
  else 
    return SubString(*this, first, l);
}

#endif
