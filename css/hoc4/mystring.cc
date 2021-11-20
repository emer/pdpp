
#include "mystring.h"


/* System-independent versions of toupper and tolower */

inline char to_upper(unsigned char c)	{ return (islower(c) ? (c-'a'+'A') : c); }
inline char to_lower(unsigned char c)	{ return (isupper(c) ? (c-'A'+'a') : c); }


// Constructors

String::~String()	{ free(p); }

String::String(const char& c, unsigned l, unsigned extra)
{
  len = l;
  alloc = len + extra + 1;
  p = (char*)malloc(alloc);
  register unsigned i=len;
  p[i] = '\0';
  while (i > 0) p[--i] = c;
}

String::String()
{
  len = 0;
  alloc = DEFAULT_STRING_LENGTH + 1;
  p = (char*)malloc(alloc);
  *p = '\0';
}

String::String(unsigned extra)
{
  len = 0;
  alloc = extra + 1;
  p = (char*)malloc(alloc);
  *p = '\0';
}

String::String(const char* cs, unsigned extra)
{ 
  len = strlen(cs);
  alloc = len + extra + 1;
  p = (char*)malloc(alloc);
  strcpy(p,cs); 
}

String::String(const String& s, unsigned extra)
{
  len = s.len;
  alloc = len + extra +1;
  p = (char*)malloc(alloc);
  strcpy (p,s.p);
}

// operators



void String::operator=(const String& s)
{
  if (p == s.p) return;
  len = s.len;
  if (len >= alloc) {
    free(p);
    p = (char*)malloc(alloc = s.alloc);
  }
  strcpy(p,s.p);
}

void String::operator=(const char* cs)
{
  len = strlen(cs);
  if (len >= alloc) {
    alloc = len + 1;
    free(p);
    p = (char*)malloc(alloc);
  }	
  strcpy(p,cs);
}


bool String::operator==(const String& s) const
{
  unsigned i;
  
  if(s.len!=len) return NO;
  for(i=0;i<len;i++) {
    if(p[i] != s.p[i]) return NO;
  }
  return YES;
}

String operator+(const String& x, String &cs) {
  String r = x;
  r += cs;
  return r;
}

String operator+(const String& x, char* cs) {
  String r = x;
   r += cs;
  return r;
}


String operator+(const String& x, char cs) {
  String r = x;
  r += cs;
  return r;
}

inline void String::operator+=(const String& t) {
  int i;
  for(i=0;i<t.len;i++) {
    *this += t[i];
  }
}

inline void String::operator+=(const char* t) {
  len=len+strlen(t);
  if (len>alloc) {
    alloc = len+1;
    p = realloc(p,alloc);
  }
  strcat(p,t);
}

inline void String::operator+=(const char t) {
  len=len+1;
  if (len>alloc) {
    alloc = len+1;
    p = realloc(p,alloc);
  }
  strcat(p,&t);
}
  

inline String::contains(char *c) {
  if (strchr(p,*c) != NULL) {
    return 1;
  }
  return 0;
}

inline String String::before(char *c) {
  String r;
  char* i;
  char *pos = strchr(p,*c);
  if (pos != NULL) {
    for(i=p;i!=pos;i++) {
      r += *i;
    }
  }
  return r;
}

inline String String::after(char *c) {
  String r;
  char* i;
  char *pos = strchr(p,*c);
  if (pos != NULL) {
    for(i=pos+1;i!=(p + len);i++) {
      r += *i;
    }
  }
  return r;
}

inline void String::gsub(const char* p1, const char* p2){
  int i;
  for(i=0;i<len;i++){
    if (*((char *) (p+i)) == *p1) {
      *((char *) (p+i)) = *p2;
    }
  }
}

inline char* String::chars(){return p;};

/*
String String::operator&(const String& s) const
{
	String t(len+s.len);
	strcpy(t.p,p);
	strcpy(&(t.p[len]), s.p);
	t.len = len+s.len;
	return t;
}

String String::operator&(const char* cs) const
{
	unsigned cslen = strlen(cs);
	String t(len+cslen);
	strcpy (t.p,p);
	strcpy (&(t.p[len]), cs);
	t.len = len+cslen;
	return t;
}

String operator&(const char* cs, const String& s)
{
	int cslen=strlen(cs);
	String t(cslen + s.len);
	strcpy(t.p,cs);
	strcpy(&(t.p[cslen]),s.p);
	t.len = cslen + s.len;
	return t;
}
*/
unsigned String::reSize(unsigned new_capacity)
{
	if (new_capacity < len) new_capacity = len;
	if (alloc != new_capacity+1) {
		p = (char*)realloc(p,alloc = new_capacity+1);
	}
	return alloc - 1;
}


void String::toAscii()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = toascii(*q); q++; }
}

void String::toLower()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = to_lower(*q); q++; }
}

void String::toUpper()
{
	register unsigned i = len;
	register char* q = p;
	while (i--) { *q = to_upper(*q); q++; }
}


unsigned String::size() const		{ return len; }

unsigned String::capacity() const	{ return alloc - 1; }

String  replicate(char c, int num) {
  int i;
  String s;

  for(i=0;i<num;i++) {
    s += c;
  }
  return s;
}

