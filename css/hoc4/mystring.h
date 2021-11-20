#ifndef MYSTRING_H
#define MYSTRING_H

#define DEFAULT_STRING_LENGTH 10

typedef int bool;
#define NO 0
#define YES 1

#ifdef __GNUG__
#include <stream.h>
#endif 

#include <string.h>
#include <stdlib.h>
#include <ctype.h> // for toascii() + others

class String {
  char* p;		// character string
  unsigned len;		// length of string, excluding null character
  unsigned alloc;		// amount of storage allocated
public:
  String();
  String(unsigned extra);
  String(const char*, unsigned extra=0);
  String(const String&, unsigned extra=0);
  String(const char& c, unsigned l=1, unsigned extra=DEFAULT_STRING_LENGTH);
  ~String();
  operator const char*() const	{ return p; }
  char& operator[](unsigned i) {
    if (i < len) 
      return p[i];
    else
      return p[len];
  }
  char operator[](unsigned i) const {
    if (i < len)
      return p[i];
    else
      return p[len];
  }
  
  void operator=(const String&);
  void operator=(const char*);
  
  bool operator<(const String& s) const	{ return strcmp(p, s.p) < 0; }
  bool operator>(const String& s) const	{ return strcmp(p, s.p) > 0; }
  bool operator<=(const String& s) const	{ return strcmp(p, s.p) <= 0; }
  bool operator>=(const String& s) const	{ return strcmp(p, s.p) >= 0; }
  bool operator==(const String& s) const;
  bool operator!=(const String& s) const	{ return !(*this==s); }
  
  bool operator<(const char* cs) const	{ return strcmp(p,cs) < 0; }   
  bool operator>(const char* cs) const	{ return strcmp(p,cs) > 0; }   
  bool operator<=(const char* cs) const	{ return strcmp(p,cs) <= 0; }
  bool operator>=(const char* cs) const	{ return strcmp(p,cs) >= 0; }
  bool operator==(const char* cs) const	{ return strcmp(p,cs) == 0; }
  bool operator!=(const char* cs) const	{ return strcmp(p,cs) != 0; }

  friend String operator+(const String& x, String& cs);
  friend String operator+(const String& x, char* cs);
  friend String operator+(const String& x, char  cs);
  
  void operator +=(const String& t);
  void operator +=(const char* t);
  void operator +=(const char  t);
  
  int contains(char *c);
  String after(char *c);
  String before(char *c);
  char* chars();
  void gsub(const char* p1, const char* p2);

  friend bool operator<(const char* cs, const String& s) {
    return strcmp(cs, s.p) < 0;
  }
  friend bool operator>(const char* cs, const String& s) {	 
    return strcmp(cs, s.p) > 0;
  }
  friend bool operator<=(const char* cs, const String& s) { 
    return strcmp(cs, s.p) <= 0; 
  }
  friend bool operator>=(const char* cs, const String& s) { 
    return strcmp(cs, s.p) >= 0;
  }
  friend bool operator==(const char* cs, const String& s) {
    return strcmp(cs, s.p) == 0; 
  }
  friend bool operator!=(const char* cs, const String& s) {
    return strcmp(cs, s.p) != 0; 
  }
/*  
  String operator&(const String& s) const;
  String operator&(const char* cs) const;
  friend String operator&(const char* cs, const String& s);
*/
  char& at(unsigned i)			{ return (*this)[i]; }
  char at(unsigned i) const		{ return (*this)[i]; }
  unsigned length() const	{ return len; } 
  unsigned reSize(unsigned new_capacity);
  void toAscii();
  void toLower();
  void toUpper();
  unsigned capacity() const;
  unsigned size() const;

  friend String replicate(char,int);

#ifdef __GNUG__
  friend int        readline(istream& s, String& x, 
                             char terminator = '\n',
                             int discard_terminator = 1);
#endif

};

#endif // MYSTRING_H
