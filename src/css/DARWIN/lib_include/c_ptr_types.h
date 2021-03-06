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

// elaboration of pointers to internal C variables

#ifndef css_ptr_i_h
#define css_ptr_i_h 1

#include <css/machine.h>
#include <css/basic_types.h>


class cssCPtr_int : public cssCPtr {
  // Points to a C integer
public:
  uint		GetSize() const 	{ return sizeof(int); } // use for ptrs
  const char*	GetTypeName() const 	{ return "(c_int)"; }
  String 	PrintStr() const;
  String	PrintFStr() const 	{ return GetStr(); }

  // constructors
  cssCPtr_int() 				: cssCPtr(){};
  cssCPtr_int(void* it, int pc) 		: cssCPtr(it,pc){};
  cssCPtr_int(void* it, int pc, const char* nm)	: cssCPtr(it,pc,nm){};
  cssCPtr_int(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr(it,pc,nm,cp,ro){};
  cssCPtr_int(const cssCPtr_int& cp) 		: cssCPtr(cp){};
  cssCPtr_int(const cssCPtr_int& cp, const char* nm) 	: cssCPtr(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_int, (void*)NULL);

  // converters
  String& GetStr() const	{ ((cssEl*)this)->tmp_str = String((Int)*this); return (String&)tmp_str; }
  operator Int() const		{ return *((int*)GetNonNullVoidPtr()); }
  operator Real() const		{ return (Real)((Int)*this); }
  operator int*() const		{ return (int*)GetNonNullVoidPtr(); }
  operator int**() const	{ return (int**)GetNonNullVoidPtr(2); }

  void operator=(Real cp) 	{ *((int*)GetNonNullVoidPtr()) = (int)cp; }
  void operator=(Int cp)	{ *((int*)GetNonNullVoidPtr()) = cp; }
  void operator=(const String& cp) { *((int*)GetNonNullVoidPtr()) = (int)cp; }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);	

  cssEl* operator+(cssEl &t)   
  { cssInt *r = new cssInt((Int)*this); r->val += (Int)t; return r; }
  cssEl* operator-(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val -= (Int)t; return r; }
  cssEl* operator*()		{ return cssCPtr::operator*(); }
  cssEl* operator*(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val *= (Int)t; return r; }
  cssEl* operator/(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val /= (Int)t; return r; }
  cssEl* operator%(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val %= (Int)t; return r; }
  cssEl* operator<<(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val <<= (Int)t; return r; }
  cssEl* operator>>(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val >>= (Int)t; return r; }
  cssEl* operator&(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val &= (Int)t; return r; }
  cssEl* operator^(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val ^= (Int)t; return r; }
  cssEl* operator|(cssEl &t)	
  { cssInt *r = new cssInt((Int)*this); r->val |= (Int)t; return r; }

  cssEl* operator-()
  { cssInt *r = new cssInt((Int)*this); r->val = -r->val; return r; }
  
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);
  void operator%=(cssEl& t);
  void operator<<=(cssEl& t);
  void operator>>=(cssEl& t);
  void operator&=(cssEl& t);
  void operator^=(cssEl& t);
  void operator|=(cssEl& t);

  bool operator< (cssEl& s) 	{ return ((Int)*this < (Int)s); }
  bool operator> (cssEl& s) 	{ return ((Int)*this > (Int)s); }
  bool operator! () 	    	{ return ( ! (Int)*this); }
  bool operator<=(cssEl& s) 	{ return ((Int)*this <= (Int)s); }
  bool operator>=(cssEl& s) 	{ return ((Int)*this >= (Int)s); }	
  bool operator==(cssEl& s) 	{ return ((Int)*this == (Int)s); }
  bool operator!=(cssEl& s) 	{ return ((Int)*this != (Int)s); }
  bool operator&&(cssEl& s) 	{ return ((Int)*this && (Int)s); }
  bool operator||(cssEl& s) 	{ return ((Int)*this || (Int)s); }
};

#define cssCPtr_int_inst(l,n)		l .Push(new cssCPtr_int(&n,1,#n))
#define cssCPtr_int_inst_nm(l,n,s)	l .Push(new cssCPtr_int(n,1,s))
#define cssCPtr_int_inst_ptr(l,n,x)	l .Push(x = new cssCPtr_int(&n,1,#x))
#define cssCPtr_int_inst_ptr_nm(l,n,x,s) l .Push(x = new cssCPtr_int(n,1,s))


class cssCPtr_short : public cssCPtr_int {
public:
  uint		GetSize() const 	{ return sizeof(short); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_short)"; }

  // constructors
  cssCPtr_short() 				: cssCPtr_int(){};
  cssCPtr_short(void* it, int pc) 		: cssCPtr_int(it,pc){};
  cssCPtr_short(void* it, int pc, const char* nm)	: cssCPtr_int(it,pc,nm){};
  cssCPtr_short(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_int(it,pc,nm,cp,ro){};
  cssCPtr_short(const cssCPtr_short& cp) 		: cssCPtr_int(cp){};
  cssCPtr_short(const cssCPtr_short& cp, const char* nm) 	: cssCPtr_int(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_short, (void*)NULL);

  // converters
  operator Int() const		{ return *((short*)GetNonNullVoidPtr()); }
  operator short*() const	{ return (short*)GetNonNullVoidPtr(); }
  operator short**() const	{ return (short**)GetNonNullVoidPtr(2); }
  operator int*() const		{ CvtErr("(int*)"); return NULL; }
  operator int**() const	{ CvtErr("(int**)"); return NULL; }

  void operator=(Real cp) 	{ *((short*)GetNonNullVoidPtr()) = (short)cp; }
  void operator=(Int cp)	{ *((short*)GetNonNullVoidPtr()) = (short)cp; }
  void operator=(const String& cp) { *((short*)GetNonNullVoidPtr()) = (short)(int)cp; }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);
  void operator%=(cssEl& t);
  void operator<<=(cssEl& t);
  void operator>>=(cssEl& t);
  void operator&=(cssEl& t);
  void operator^=(cssEl& t);
  void operator|=(cssEl& t);

};

class cssCPtr_bool : public cssCPtr_int {
public:
  uint		GetSize() const 	{ return sizeof(bool); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_bool)"; }

  // constructors
  cssCPtr_bool() 				: cssCPtr_int(){};
  cssCPtr_bool(void* it, int pc) 		: cssCPtr_int(it,pc){};
  cssCPtr_bool(void* it, int pc, const char* nm)	: cssCPtr_int(it,pc,nm){};
  cssCPtr_bool(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_int(it,pc,nm,cp,ro){};
  cssCPtr_bool(const cssCPtr_bool& cp) 		: cssCPtr_int(cp){};
  cssCPtr_bool(const cssCPtr_bool& cp, const char* nm) 	: cssCPtr_int(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_bool, (void*)NULL);

  // converters
  String& GetStr() const;
  operator Int() const		{ return *((bool*)GetNonNullVoidPtr()); }
#ifndef NO_BUILTIN_BOOL
  operator bool*() const	{ return (bool*)GetNonNullVoidPtr(); }
  operator bool**() const	{ return (bool**)GetNonNullVoidPtr(2); }
  operator int*() const		{ CvtErr("(int*)"); return NULL; }
  operator int**() const	{ CvtErr("(int**)"); return NULL; }
#endif

  void operator=(Real cp) 	{ *((bool*)GetNonNullVoidPtr()) = (bool)cp; }
  void operator=(Int cp)	{ *((bool*)GetNonNullVoidPtr()) = (bool)cp; }
  void operator=(const String& cp);
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);
};

class cssCPtr_long : public cssCPtr_int {
public:
  uint		GetSize() const 	{ return sizeof(long); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_long)"; }

  // constructors
  cssCPtr_long() 				: cssCPtr_int(){};
  cssCPtr_long(void* it, int pc) 		: cssCPtr_int(it,pc){};
  cssCPtr_long(void* it, int pc, const char* nm)	: cssCPtr_int(it,pc,nm){};
  cssCPtr_long(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_int(it,pc,nm,cp,ro){};
  cssCPtr_long(const cssCPtr_long& cp) 		: cssCPtr_int(cp){};
  cssCPtr_long(const cssCPtr_long& cp, const char* nm) 	: cssCPtr_int(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_long, (void*)NULL);

  // converters
  operator Int() const		{ return *((long*)GetNonNullVoidPtr()); }
  operator long() const		{ return *((long*)GetNonNullVoidPtr()); }
  operator long*() const	{ return (long*)GetNonNullVoidPtr(); }
  operator long**() const	{ return (long**)GetNonNullVoidPtr(2); }
  operator int*() const		{ CvtErr("(int*)"); return NULL; }
  operator int**() const	{ CvtErr("(int**)"); return NULL; }

  void operator=(Real cp) 	{ *((long*)GetNonNullVoidPtr()) = (long)cp; }
  void operator=(Int cp)	{ *((long*)GetNonNullVoidPtr()) = (long)cp; }
  void operator=(const String& cp) { *((long*)GetNonNullVoidPtr()) = (long)(int)cp; }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);
  void operator%=(cssEl& t);
  void operator<<=(cssEl& t);
  void operator>>=(cssEl& t);
  void operator&=(cssEl& t);
  void operator^=(cssEl& t);
  void operator|=(cssEl& t);
};

class cssCPtr_char : public cssCPtr_int {
public:
  uint		GetSize() const 	{ return sizeof(char); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_char)"; }

  // constructors
  cssCPtr_char() 				: cssCPtr_int(){};
  cssCPtr_char(void* it, int pc) 		: cssCPtr_int(it,pc){};
  cssCPtr_char(void* it, int pc, const char* nm)	: cssCPtr_int(it,pc,nm){};
  cssCPtr_char(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_int(it,pc,nm,cp,ro){};
  cssCPtr_char(const cssCPtr_char& cp) 		: cssCPtr_int(cp){};
  cssCPtr_char(const cssCPtr_char& cp, const char* nm) 	: cssCPtr_int(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_char, (void*)NULL);

  // converters
  String& GetStr() const	{ ((cssEl*)this)->tmp_str = (const char*)GetNonNullVoidPtr(); return (String&)tmp_str; }
  operator Int() const		{ return *((char*)GetNonNullVoidPtr()); }
  operator const char*() const	{ return (const char*)GetNonNullVoidPtr(); }
  operator int*() const		{ CvtErr("(int*)"); return NULL; }
  operator int**() const	{ CvtErr("(int**)"); return NULL; }

  void operator=(Real cp) 	{ *((char*)GetNonNullVoidPtr()) = (char)cp; }
  void operator=(Int cp)	{ *((char*)GetNonNullVoidPtr()) = (char)cp; }
  void operator=(const String& cp)
  { if(!cp.empty()) *((char*)GetNonNullVoidPtr()) = cp[0]; }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);
  void operator%=(cssEl& t);
  void operator<<=(cssEl& t);
  void operator>>=(cssEl& t);
  void operator&=(cssEl& t);
  void operator^=(cssEl& t);
  void operator|=(cssEl& t);
};

class cssCPtr_enum : public cssCPtr_int {
public:
  uint		GetSize() const 	{ return sizeof(int); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_enum)"; }

  MemberDef*	GetEnumType() const;
  // attempts to get member def info from class_parent (use md to get type def)

  // constructors
  cssCPtr_enum() 				: cssCPtr_int(){};
  cssCPtr_enum(void* it, int pc) 		: cssCPtr_int(it,pc){};
  cssCPtr_enum(void* it, int pc, const char* nm)	: cssCPtr_int(it,pc,nm){};
  cssCPtr_enum(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_int(it,pc,nm,cp,ro){};
  cssCPtr_enum(const cssCPtr_enum& cp) 		: cssCPtr_int(cp){};
  cssCPtr_enum(const cssCPtr_enum& cp, const char* nm) 	: cssCPtr_int(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_enum, (void*)NULL);

  // converters 
  String& 	GetStr() const;

  void operator=(Real cp) 	{ *((int*)GetNonNullVoidPtr()) = (int)cp; }
  void operator=(Int cp)	{ *((int*)GetNonNullVoidPtr()) = cp; }
  void operator=(const String& cp);
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);	
};

class cssCPtr_double : public cssCPtr {
public:
  uint		GetSize() const 	{ return sizeof(double); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_double)"; }

  String 	PrintStr() const;
  String	PrintFStr() const 	{ return String((Real)*this); }

  // constructors
  cssCPtr_double() 				: cssCPtr(){};
  cssCPtr_double(void* it, int pc) 		: cssCPtr(it,pc){};
  cssCPtr_double(void* it, int pc, const char* nm)	: cssCPtr(it,pc,nm){};
  cssCPtr_double(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr(it,pc,nm,cp,ro){};
  cssCPtr_double(const cssCPtr_double& cp) 		: cssCPtr(cp){};
  cssCPtr_double(const cssCPtr_double& cp, const char* nm) : cssCPtr(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_double, (void*)NULL);

  // converters
  String& GetStr() const	{ ((cssEl*)this)->tmp_str=String((Real)*this); return (String&)tmp_str; }
  operator Real() const		{ return *((double*)GetNonNullVoidPtr()); }
  operator Int() const		{ return (Int)(Real)*this; }
  operator double*() const	{ return (double*)GetNonNullVoidPtr(); }
  operator double**() const	{ return (double**)GetNonNullVoidPtr(2); }

  void operator=(Real cp) 	{ *((double*)GetNonNullVoidPtr()) = (double)cp; }
  void operator=(Int cp)	{ *((double*)GetNonNullVoidPtr()) = (double)cp; }
  void operator=(const String& cp) { *((double*)GetNonNullVoidPtr()) = atof((const char*)cp); }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);	

  cssEl* operator+(cssEl &t)   
  { cssReal *r = new cssReal((Real)*this,""); r->val += (Real)t; return r; }
  cssEl* operator-(cssEl &t)	
  { cssReal *r = new cssReal((Real)*this,""); r->val -= (Real)t; return r; }
  cssEl* operator*()		{ return cssCPtr::operator*(); }
  cssEl* operator*(cssEl &t)	
  { cssReal *r = new cssReal((Real)*this,""); r->val *= (Real)t; return r; }
  cssEl* operator/(cssEl &t)	
  { cssReal *r = new cssReal((Real)*this,""); r->val /= (Real)t; return r; }

  // implement the to-the-power of operator as ^
  cssEl* operator^(cssEl &t)	
  { cssReal *r = new cssReal(); r->val = pow((Real)*this, (Real)t); return r; }

  cssEl* operator-()
  { cssReal *r = new cssReal((Real)*this,""); r->val = -r->val; return r; }
  
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);

  bool operator< (cssEl& s) 	{ return ((Real)*this < (Real)s); }
  bool operator> (cssEl& s) 	{ return ((Real)*this > (Real)s); }
  bool operator! () 	    	{ return ( ! (Real)*this); }
  bool operator<=(cssEl& s) 	{ return ((Real)*this <= (Real)s); }
  bool operator>=(cssEl& s) 	{ return ((Real)*this >= (Real)s); }	
  bool operator==(cssEl& s) 	{ return ((Real)*this == (Real)s); }
  bool operator!=(cssEl& s) 	{ return ((Real)*this != (Real)s); }
  bool operator&&(cssEl& s) 	{ return ((Real)*this && (Real)s); }
  bool operator||(cssEl& s) 	{ return ((Real)*this || (Real)s); }
};


class cssCPtr_float : public cssCPtr_double {
public:
  uint		GetSize() const 	{ return sizeof(float); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_float)"; }

  // constructors
  cssCPtr_float() 				: cssCPtr_double(){};
  cssCPtr_float(void* it, int pc) 		: cssCPtr_double(it,pc){};
  cssCPtr_float(void* it, int pc, const char* nm)	: cssCPtr_double(it,pc,nm){};
  cssCPtr_float(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr_double(it,pc,nm,cp,ro){};
  cssCPtr_float(const cssCPtr_float& cp) 		: cssCPtr_double(cp){};
  cssCPtr_float(const cssCPtr_float& cp, const char* nm) 	: cssCPtr_double(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_float, (void*)NULL);

  // converters
  operator Real() const		{ return *((float*)GetNonNullVoidPtr()); }
  operator float*() const	{ return (float*)GetNonNullVoidPtr(); }
  operator float**() const	{ return (float**)GetNonNullVoidPtr(2); }
  operator double*() const	{ CvtErr("(double*)"); return NULL; }
  operator double**() const	{ CvtErr("(double**)"); return NULL; }

  void operator=(Real cp) 	{ *((float*)GetNonNullVoidPtr()) = (float)cp; }
  void operator=(Int cp)	{ *((float*)GetNonNullVoidPtr()) = (float)cp; }
  void operator=(const String& cp) { *((float*)GetNonNullVoidPtr()) = (float)atof((const char*)cp); }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);
  void operator+=(cssEl& t);
  void operator-=(cssEl& t);
  void operator*=(cssEl& t);
  void operator/=(cssEl& t);
};


class cssCPtr_String : public cssCPtr {
public:
  uint		GetSize() const 	{ return sizeof(String); } // use for ptrs
  const char*	GetTypeName() const  	{ return "(c_String)"; }

  String 	PrintStr() const;
  String	PrintFStr() const 	{ return String(*((String*)ptr)); }

  // constructors
  cssCPtr_String() 				: cssCPtr(){};
  cssCPtr_String(void* it, int pc) 		: cssCPtr(it,pc){};
  cssCPtr_String(void* it, int pc, const char* nm)	: cssCPtr(it,pc,nm){};
  cssCPtr_String(void* it, int pc, const char* nm, cssEl* cp, bool ro)
  : cssCPtr(it,pc,nm,cp,ro){};
  cssCPtr_String(const cssCPtr_String& cp) 		: cssCPtr(cp){};
  cssCPtr_String(const cssCPtr_String& cp, const char* nm) : cssCPtr(cp,nm){};

  cssCPtr_CloneFuns(cssCPtr_String, (void*)NULL);

  // converters
  String& GetStr() const	{ return *((String*)GetNonNullVoidPtr()); }
  operator Real() const 	{ Real r = atof((const char*)GetStr()); return r; }
  operator Int() const		{ Int r = (int)GetStr(); return r; }
  operator String*() const	{ return (String*)GetNonNullVoidPtr(); }
  operator String**() const	{ return (String**)GetNonNullVoidPtr(2); }

  void operator=(Real cp) 	{ *((String*)GetNonNullVoidPtr()) = String(cp); }
  void operator=(Int cp)	{ *((String*)GetNonNullVoidPtr()) = String(cp); }
  void operator=(const String& cp) { *((String*)GetNonNullVoidPtr()) = cp; }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  void operator=(const cssEl& t);

  cssEl* operator+(cssEl &t)   
  { cssString *r = new cssString(GetStr(),""); r->val += t.GetStr(); return r; }
  cssEl* operator-(cssEl&)		{ NopErr("-"); return &cssMisc::Void; }
  cssEl* operator*()			{ return cssCPtr::operator*(); }
  cssEl* operator*(cssEl&)		{ NopErr("*"); return &cssMisc::Void; }	
  cssEl* operator/(cssEl&)		{ NopErr("/"); return &cssMisc::Void; }	
  cssEl* operator%(cssEl&)		{ NopErr("%"); return &cssMisc::Void; }	
  cssEl* operator-()    		{ NopErr("-"); return &cssMisc::Void; }
  
  void operator+=(cssEl& t);

  bool operator< (cssEl& s) { return (*((String*)GetNonNullVoidPtr()) < s.GetStr()); }
  bool operator> (cssEl& s) { return (*((String*)GetNonNullVoidPtr()) > s.GetStr()); }
  bool operator! () 	    { return ((String*)GetNonNullVoidPtr())->length(); }
  bool operator<=(cssEl& s) { return (*((String*)GetNonNullVoidPtr()) <= s.GetStr()); }
  bool operator>=(cssEl& s) { return (*((String*)GetNonNullVoidPtr()) >= s.GetStr()); }	
  bool operator==(cssEl& s) { return (*((String*)GetNonNullVoidPtr()) == s.GetStr()); }
  bool operator!=(cssEl& s) { return (*((String*)GetNonNullVoidPtr()) != s.GetStr()); }
  bool operator&&(cssEl& s) { return (((String*)GetNonNullVoidPtr())->length() && (Int)s); }
  bool operator||(cssEl& s) { return (((String*)GetNonNullVoidPtr())->length() || (Int)s); }

  // these use the TA info to perform actions
  cssEl* operator[](int idx) const;
  cssEl* GetMemberFun_impl(MethodDef* md) const;
  int	 GetMemberFunNo(const char*) const;
  cssEl* GetMemberFun(const char* memb) const;
  cssEl* GetMemberFun(int memb) const;
  cssEl* GetScoped(const char*) const;
};


#endif // css_ptr_i_h
