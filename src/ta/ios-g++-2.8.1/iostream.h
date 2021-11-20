/*  This is part of libio/iostream, providing -*- C++ -*- input/output.
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

#ifndef _IOSTREAM_H
#ifdef __GNUG__
#pragma interface
#endif
#define _IOSTREAM_H

#include <streambuf.h>

extern "C++" {
class istream; class ostream;
typedef ios& (*__manip)(ios&);
typedef istream& (*__imanip)(istream&);
typedef ostream& (*__omanip)(ostream&);

extern istream& ws(istream& ins);
extern ostream& flush(ostream& outs);
extern ostream& endl(ostream& outs);
extern ostream& ends(ostream& outs);

class ostream : virtual public ios
{
    // NOTE: If fields are changed, you must fix _fake_ostream in stdstreams.C!
    void do_osfx();
  public:
    ostream() { }
    ostream(streambuf* sb, ostream* tied=NULL);
    int opfx() {
	if (!good()) return 0;
	else { if (_tie) _tie->flush(); _IO_flockfile(_strbuf); return 1;} }
    void osfx() { _IO_funlockfile(_strbuf);
		  if (flags() & (ios::unitbuf|ios::stdio))
		      do_osfx(); }
    ostream& flush();
    ostream& put(char c) { _strbuf->sputc(c); return *this; }
#ifdef _STREAM_COMPAT
    /* Temporary binary compatibility.  REMOVE IN NEXT RELEASE. */
    ostream& put(unsigned char c) { return put((char)c); }
    ostream& put(signed char c) { return put((char)c); }
#endif
    ostream& write(const char *s, streamsize n);
    ostream& write(const unsigned char *s, streamsize n)
      { return write((const char*)s, n);}
    ostream& write(const signed char *s, streamsize n)
      { return write((const char*)s, n);}
    ostream& write(const void *s, streamsize n)
      { return write((const char*)s, n);}
#ifndef CYGWIN
    ostream& seekp(streampos);
    ostream& seekp(streamoff, _seek_dir);
#endif
    streampos tellp();
    ostream& form(const char *format ...);
};

class istream : virtual public ios
{
  public:
    istream(streambuf* sb, ostream*tied=NULL);
    istream& get(char* ptr, int len, char delim = '\n');
    istream& get(unsigned char* ptr, int len, char delim = '\n')
	{ return get((char*)ptr, len, delim); }
    istream& get(char& c);
    istream& get(unsigned char& c) { return get((char&)c); }
    istream& getline(char* ptr, int len, char delim = '\n');
    istream& getline(unsigned char* ptr, int len, char delim = '\n')
	{ return getline((char*)ptr, len, delim); }
    istream& get(signed char& c)  { return get((char&)c); }
    istream& get(signed char* ptr, int len, char delim = '\n')
	{ return get((char*)ptr, len, delim); }
    istream& getline(signed char* ptr, int len, char delim = '\n')
	{ return getline((char*)ptr, len, delim); }
    istream& read(char *ptr, streamsize n);
    istream& read(unsigned char *ptr, streamsize n)
      { return read((char*)ptr, n); }
    istream& read(signed char *ptr, streamsize n)
      { return read((char*)ptr, n); }
    istream& read(void *ptr, streamsize n)
      { return read((char*)ptr, n); }
    istream& get(streambuf& sb, char delim = '\n');
    istream& gets(char **s, char delim = '\n');
    int ipfx(int need = 0) {
	if (!good()) { set(ios::failbit); return 0; }
	else {
	  _IO_flockfile(_strbuf);
	  if (_tie && (need == 0 || rdbuf()->in_avail() < need)) _tie->flush();
	  if (!need && (flags() & ios::skipws)) return _skip_ws();
	  else return 1;
	}
    }
    int ipfx0() { // Optimized version of ipfx(0).
	if (!good()) { set(ios::failbit); return 0; }
	else {
	  _IO_flockfile(_strbuf);
	  if (_tie) _tie->flush();
	  if (flags() & ios::skipws) return _skip_ws();
	  else return 1;
	}
    }
    int ipfx1() { // Optimized version of ipfx(1).
	if (!good()) { set(ios::failbit); return 0; }
	else {
	  _IO_flockfile(_strbuf);
	  if (_tie && rdbuf()->in_avail() == 0) _tie->flush();
	  return 1;
	}
    }
    void isfx() { _IO_funlockfile(_strbuf); }
    int get() { if (!ipfx1()) return EOF;
		else { int ch = _strbuf->sbumpc();
		       if (ch == EOF) set(ios::eofbit);
		       return ch;
		     } }
    int peek();
    _IO_size_t gcount() { return _gcount; }
    istream& ignore(int n=1, int delim = EOF);
    int sync ();
#ifndef CYGWIN
    istream& seekg(streampos);
    istream& seekg(streamoff, _seek_dir);
    streampos tellg();
#endif
    istream& putback(char ch) {
	if (good() && _strbuf->sputbackc(ch) == EOF) clear(ios::badbit);
	return *this;}
    istream& unget() {
	if (good() && _strbuf->sungetc() == EOF) clear(ios::badbit);
	return *this;}
    istream& scan(const char *format ...);
#ifdef _STREAM_COMPAT
    istream& unget(char ch) { return putback(ch); }
    int skip(int i);
    streambuf* istreambuf() const { return _strbuf; }
#endif

};

class iostream : public istream, public ostream
{
  public:
    iostream() { }
    iostream(streambuf* sb, ostream*tied=NULL);
};

#endif /*!_IOSTREAM_H*/
