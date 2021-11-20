/* This is part of libio/iostream, providing -*- C++ -*- input/output.
Copyright (C) 1993 Free Software Foundation

This file is part of the GNU IO Library.  This library is free
software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option)
any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

As a special exception, if you link this library with files
compiled with a GNU compiler to produce an executable, this does not cause
the resulting executable to be covered by the GNU General Public License.
This exception does not however invalidate any other reasons why
the executable file might be covered by the GNU General Public License. */

#ifndef _STREAMBUF_H
#define _STREAMBUF_H
#ifdef __GNUG__
#pragma interface
#endif

/* #define _G_IO_THROW */ /* Not implemented:  ios::failure */

#define _IO_NEW_STREAMS // new optimizated stream representation

extern "C" {
#include <libio.h>
}

#ifndef _IO_wchar_t
#if _G_IO_IO_FILE_VERSION == 0x20001
#define _IO_wchar_t _G_wchar_t
#else
#define _IO_wchar_t short
#endif
#endif

extern "C++" {

class istream; /* Work-around for a g++ name mangling bug. Fixed in 2.6. */
class ostream; class streambuf;

// In case some header files defines these as macros.
#undef open
#undef close

#if defined(_G_IO_IO_FILE_VERSION) && _G_IO_IO_FILE_VERSION == 0x20001
#if __MAKETA__ && !CYGWIN
  typedef signed long long int streampos;
  typedef signed long long int streamoff;
#else
typedef _IO_off64_t streamoff;
typedef _IO_fpos64_t streampos;
#endif
#else
typedef _IO_off_t streamoff;
typedef _IO_fpos_t streampos;
#endif
typedef _IO_ssize_t streamsize;

typedef unsigned long __fmtflags;
typedef unsigned char __iostate;

struct _ios_fields
{ // The data members of an ios.
    streambuf *_strbuf;
    ostream* _tie;
    int _width;
    __fmtflags _flags;
    _IO_wchar_t _fill;
    __iostate _state;
    __iostate _exceptions;
    int _precision;

    void *_arrays; /* Support for ios::iword and ios::pword. */
};

#define _IOS_GOOD	0
#define _IOS_EOF	1
#define _IOS_FAIL	2
#define _IOS_BAD	4

#define _IO_INPUT	1
#define _IO_OUTPUT	2
#define _IO_ATEND	4
#define _IO_APPEND	8
#define _IO_TRUNC	16
#define _IO_NOCREATE	32
#define _IO_NOREPLACE	64
#define _IO_BIN		128

#ifdef _STREAM_COMPAT
enum state_value {
    _good = _IOS_GOOD,
    _eof = _IOS_EOF,
    _fail = _IOS_FAIL,
    _bad = _IOS_BAD };
enum open_mode {
    input = _IO_INPUT,
    output = _IO_OUTPUT,
    atend = _IO_ATEND,
    append = _IO_APPEND };
#endif

class ios : public _ios_fields {
  ios& operator=(ios&);  /* Not allowed! */
  ios (const ios&); /* Not allowed! */
  public:
    typedef __fmtflags fmtflags;
    typedef int iostate;
    typedef int openmode;
    typedef int streamsize;
    enum io_state {
	goodbit = _IOS_GOOD,
	eofbit = _IOS_EOF,
	failbit = _IOS_FAIL,
	badbit = _IOS_BAD };
    enum open_mode {
	in = _IO_INPUT,
	out = _IO_OUTPUT,
	ate = _IO_ATEND,
	app = _IO_APPEND,
	trunc = _IO_TRUNC,
	nocreate = _IO_NOCREATE,
	noreplace = _IO_NOREPLACE,
	bin = _IOS_BIN, // Deprecated - ANSI uses ios::binary.
	binary = _IOS_BIN };
    enum seek_dir { beg, cur, end};
#ifndef __MAKETA__
    typedef enum seek_dir seekdir;
#endif
    // NOTE: If adding flags here, before to update ios::bitalloc().
    enum { skipws=_IO_SKIPWS,
	   left=_IO_LEFT, right=_IO_RIGHT, internal=_IO_INTERNAL,
	   dec=_IO_DEC, oct=_IO_OCT, hex=_IO_HEX,
	   showbase=_IO_SHOWBASE, showpoint=_IO_SHOWPOINT,
	   uppercase=_IO_UPPERCASE, showpos=_IO_SHOWPOS,
	   scientific=_IO_SCIENTIFIC, fixed=_IO_FIXED,
	   unitbuf=_IO_UNITBUF, stdio=_IO_STDIO
#ifndef _IO_NEW_STREAMS
	   , dont_close=_IO_DONT_CLOSE // Don't delete streambuf on stream destruction
#endif
	   };
    enum { // Masks.
	basefield=dec+oct+hex,
	floatfield = scientific+fixed,
	adjustfield = left+right+internal,
    };

    ostream* tie() const { return _tie; }
    ostream* tie(ostream* val) { ostream* save=_tie; _tie=val; return save; }

    fmtflags flags() const { return _flags; }
    fmtflags flags(fmtflags new_val) {
	fmtflags old_val = _flags; _flags = new_val; return old_val; }
    int precision() const { return _precision; }
    int precision(int newp) {
	unsigned short oldp = _precision; _precision = (unsigned short)newp;
	return oldp; }
    fmtflags setf(fmtflags val) {
	fmtflags oldbits = _flags;
	_flags |= val; return oldbits; }
    fmtflags setf(fmtflags val, fmtflags mask) {
	fmtflags oldbits = _flags;
	_flags = (_flags & ~mask) | (val & mask); return oldbits; }
    fmtflags unsetf(fmtflags mask) {
	fmtflags oldbits = _flags;
	_flags &= ~mask; return oldbits; }
    int width() const { return _width; }
    int width(int val) { int save = _width; _width = val; return save; }

    void clear(iostate state = 0) {
	_state = _strbuf ? state : state|badbit;
	if (_state & _exceptions) _throw_failure(); }
    void set(iostate flag) { _state |= flag;
	if (_state & _exceptions) _throw_failure(); }
    void setstate(iostate flag) { _state |= flag; // ANSI
	if (_state & _exceptions) _throw_failure(); }
    int good() const { return _state == 0; }
    int eof() const { return _state & ios::eofbit; }
    int fail() const { return _state & (ios::badbit|ios::failbit); }
    int bad() const { return _state & ios::badbit; }
    iostate rdstate() const { return _state; }
    operator void*() const { return fail() ? (void*)0 : (void*)(-1); }
    int operator!() const { return fail(); }
    iostate exceptions() const { return _exceptions; }
    void exceptions(iostate enable) {
	_exceptions = enable;
	if (_state & _exceptions) _throw_failure(); }

    static fmtflags bitalloc();
    static int xalloc();
    void*& pword(int);
    void* pword(int) const;
    long& iword(int);
    long iword(int) const;
};

#if __GNUG__==1
typedef int _seek_dir;
#else
typedef ios::seek_dir _seek_dir;
#endif

} // extern "C++"
#endif /* _STREAMBUF_H */
