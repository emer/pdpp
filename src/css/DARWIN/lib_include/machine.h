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

// welcome to the machine.h, backbone of the css language

#ifndef machine_h
#define machine_h 1

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#include <fstream>
#include <sstream>

#include <ta_string/ta_string.h>
#include <css/extern_support.h>

// support for typea is neccessary to get file and string member funcs
// TYPEA is an internal type access library

#define CSS_SUPPORT_TYPEA 1
#include <ta/typea.h>
#include <ta/ta_base.h>

#include <signal.h>
#include <setjmp.h>

typedef int Int;
typedef double Real;
typedef int css_progdx;		// program index type

typedef cssEl* (*css_fun_stub_ptr)(void*, int, cssEl**);

class cssElPtr;
class cssEl;
class cssInt;
class cssChar;
class cssReal;
class cssString;
class cssBool;

class cssPtr;
class cssRef;
class cssArray;
class cssArrayType;
class cssClassType;
class cssClassInst;
class cssEnum;
class cssEnumType;

class cssIOStream;
class cssIStream;
class cssOStream;
class cssFStream;

// function types
class cssElFun;
class cssElCFun;
class cssMbrCFun;
class cssScriptFun;
class cssMbrScriptFun;
    
class cssCPtr;
class cssCPtr_int;
class cssCPtr_short;
class cssCPtr_long;
class cssCPtr_char;
class cssCPtr_enum;
class cssCPtr_double;
class cssCPtr_float;
class cssCPtr_String;

class cssDef;

class cssSpace;
class cssInst;
class cssIJump;
class cssProg;
class cssProgSpace;

class cssMisc { // misc stuff for css
public:
  
  static jmp_buf 	begin;

  static cssSpace 	Internal;	// for use in internal, not searched
  static cssSpace	Parse;		// just for parsing stuff
  static cssSpace 	PreProcessor;	// functions of the cpp
  static cssSpace 	TypesSpace;	// Types, searched
  static cssSpace 	Externs;	// external variables (externs)
  static cssSpace 	HardFuns;	// hard-coded functions, searched
  static cssSpace 	HardVars;	// hard-coded variables, searched
  static cssSpace 	Commands;	// commands, activate <cr> based entry
  static cssSpace 	Functions;	// internal functions
  static cssSpace 	Constants;	// constants
  static cssSpace 	Enums;		// enums, searched
  static cssSpace 	Settings;	// user accessible parameters, etc.
  static cssSpace	Defines;	// pre-proc. defines
  static cssSpace	VoidSpace;	// a space with nothing in it (don't search)

  // stuff for parsing..
  static cssProgSpace*	ConstExprTop; 	// top-level control for ConstExpr prog
  static cssProg* 	ConstExpr; 	// constant-expression evaluation space
  static cssProg* 	CDtorProg; 	// constructor-destructor program space
  static cssProg*	CallFunProg;	// program space for dialog callfun
  static cssElPtr  	cur_type;	// current type
  static cssClassType* 	cur_class; 	// current class
  static cssEl* 	cur_scope; 	// current class scope from most recent ::
  static cssMbrScriptFun* cur_method;	// current class method 
  static cssEnumType* 	cur_enum; 	// current enum type
  static int		anon_type_cnt; 	// anonymous type counter
  static cssSpace	default_args;	// space that holds current default arg vals
  static bool           parsing_membdefn;       // true if parsing membdefn
  static bool           parsing_args;   // true if parsing arguments to functions

  static cssProgSpace* 	Top;		// top level space
  static cssProgSpace*	cur_top;	// current top-level (set for parsing, running)
  static cssProgSpace*	code_cur_top;	// cur_top for coding (e.g. if top switched for ConstExpr)
  static cssProgSpace*	delete_me;	// delete this space, couldn't be done earlier
  static cssProgSpace*	next_shell;	// make this the next shell (if not null)

  static cssArray*	s_argv;		// args passed by shell to scripts
  static cssInt*	s_argc;		// number of args passed by shell to scripts
  static String		prompt;		// default prompt for system (defaults to argv[0])
  static String		startup_file;	// file to be run at startup (by -f or -file arg from user)
  static String		startup_code;	// code to be executed at startup (by -e or -exec arg)
  static int		init_debug;	// initial debug level (by -v arg from user)
  static int		init_bpoint;	// initial breakpoint location (-b arg)
  static bool		init_interactive; // user wants to run interactively (-i from arg)

  static cssEl 		Void; 		// a void element
  static cssElPtr 	VoidElPtr;	// a void el pointer (to a void element)
  static cssPtr 	VoidPtr;	// a void pointer (for maketoken)
  static cssRef		VoidRef;	// a void reference (for maketoken)
  static cssArray	VoidArray;	// a void array (for maketoken)
  static cssArrayType   VoidArrayType;  // a void array type (for maketoken)
  static String		VoidStringVal;
  static cssString	VoidString;
  static cssEnumType	VoidEnumType; 	// a void enum type
  static cssClassType	VoidClassType; 	// a void class type

  static void 		Initialize(int argc, const char** argv);
  // this is called to install builtin funcs, setup system, etc.

  static cssProgSpace*	SetCurTop(cssProgSpace* pspc)
  { cssProgSpace* rval = cur_top; cur_top = pspc; return rval; }
  // set cur_top to given value, return previous cur_top, used for PopCurTop
  static void		PopCurTop(cssProgSpace* pspc)	{ cur_top = pspc; }
  // reset cur_top to previous value (returned by SetCurTop)
  static void		CodeConstExpr(); // use const expr for coding
  static void		CodeTop(); 	// use saved cur top for coding

  static void 		Error(cssProg* prg, const char* a, const char* b="", const char* c="", const char* d="",
			      const char* e="", const char* f="", const char* g="", const char* h="",
			      const char* i="", const char* j="", const char* k="", const char* l="");

  static void		Warning(cssProg* prg, const char* a, const char* b="", const char* c="", const char* d="",
				const char* e="", const char* f="", const char* g="", const char* h="",
				const char* i="", const char* j="", const char* k="", const char* l="");
  
  static void 		fpecatch(int);	// floating point exception handling
  static void 		intrcatch(int);	// interrupt exception handling
};


class cssElPtr {
  // a pointer to an el used in code, makes pointers context relative to current frame
public:
  enum	PtrType {
    DIRECT,			// pointer points directly to the cssEl element
    CLASS_MEMBER,		// pointer is cssClassType, index is offset of member
    NVIRT_METHOD,		// pointer is cssClassType, index is offset of non-virtual method
    VIRT_METHOD,		// pointer is cssClassType, index is offset of virutal method
    PROG_AUTO,			// pointer is cssProg, index is offset into autos
    SPACE,			// pointer is cssSpace, index is offset into space
    NULL_PTR			// pointer is NULL
  };

  PtrType	ptr_type;	// type of pointer
  int		dx;		// the index (-1 means void)
  void*		ptr;		// the pointer itself
  
  cssEl* El() const;		// gets the el
  void 	Print(ostream& fh) const;

  bool	IsNull() const	{
    bool nul = false; if(ptr_type == DIRECT) nul = (ptr == NULL);
    else nul = ((ptr == NULL) || (dx < 0)); return nul; }
  bool	NotNull() const	{ return !IsNull(); }

  bool 	operator!=(int cv) const	
  { return (cv == 0) ? !IsNull() : IsNull(); }
  bool 	operator==(int cv) const	
  { return (cv == 0) ? IsNull() : !IsNull(); }
  bool 	operator==(cssElPtr& cv) const	
  { return ((ptr_type == cv.ptr_type) && (dx == cv.dx) && (ptr == cv.ptr)); }
  void 	operator+=(int indx);
  void 	operator-=(int indx);

  void	SetDirect(cssEl* el)	{ ptr_type = DIRECT; ptr = (void*)el; }
  void	SetClassMember(cssClassType* cls, int idx = -1)
  { ptr_type = CLASS_MEMBER; ptr = (void*)cls; if(idx >= 0) dx = idx; }
  void	SetNVirtMethod(cssClassType* cls, int idx = -1)
  { ptr_type = NVIRT_METHOD; ptr = (void*)cls; if(idx >= 0) dx = idx; }
  void	SetVirtMethod(cssClassType* cls, int idx = -1)
  { ptr_type = VIRT_METHOD; ptr = (void*)cls; if(idx >= 0) dx = idx; }
  void	SetProgAuto(cssProg* prg, int idx = -1)
  { ptr_type = PROG_AUTO; ptr = (void*)prg; if(idx >= 0) dx = idx; }
  void	SetSpace(cssSpace* spc, int idx = -1)
  { ptr_type = SPACE; ptr = (void*)spc; if(idx >= 0) dx = idx; }

  void 	Reset() 	{ ptr_type = NULL_PTR; dx = -1; ptr = NULL; }
  cssElPtr()            { Reset(); }
  cssElPtr(cssEl* el)	{ Reset(); SetDirect(el); }
  cssElPtr(cssProg* prg, int idx = -1)	{ Reset(); SetProgAuto(prg, idx); }
  cssElPtr(cssSpace* spc, int idx = -1)	{ Reset(); SetSpace(spc, idx); }
};

class cssElPlusIVal {
  // this is used for parsing only, where a return value and an obj are needed
public:
  cssElPtr	el;
  int		ival;
};

#ifndef CSS_NUMBER
#include <css/css_parse.h>
#endif

// macros for defining the cloning functions 
#define cssCloneFuns(x, init) \
  cssEl* 	Clone()		{ return new x (*this); }	      \
  cssEl* 	AnonClone()  	{ return new x (*this, ""); }	      \
  cssEl*	MakeToken_stub(int, cssEl* arg[])		      \
  { return new x ( init , (const char*)*(arg[1])); }		      \

#define cssCloneOnly(x) \
  cssEl* 	Clone()		  { return new x (*this); }	      \
  cssEl* 	AnonClone() 	  { return new x (*this, ""); }	      \


class cssEl {
public:
  int		refn;		// number of times referred to
  static String no_string;	// when no string val is avail
// todo: tmp_str should be mutable, can get rid of all those stupid casts..
// mutable String tmp_str;	// temporary string when a char* cast is needed
  String 	tmp_str;	// temporary string when a char* cast is needed
  cssElPtr*	addr;		// address of this item (if other than 'this')
public:
  enum cssTypes {
    T_Void,			// No Type
    T_Int,			// Integer
    T_Real,			// real-valued
    T_String,			// character string type
    T_Bool,			// boolean 
    T_Ptr,			// a pointer to an El
    T_Array,			// an array of els
    T_ArrayType,                // an array type definition
    T_Enum,			// instance of an enumerated type
    T_EnumType,			// an enumerated type (collection of enums)
    T_Class,			// class instance
    T_ClassType,		// class type definition
    T_ClassMbr,			// class member definition
    T_ElCFun,                	// C function returning *cssEl
    T_MbrCFun,			// C member function
    T_ScriptFun,                // Script function returning *cssEl
    T_MbrScriptFun,		// Script member function
    T_C_Ptr,			// a pointer to a C object
    T_TA,			// TypeAccess type
    T_PP_Def, 			// pre-processor define
    T_SubShell			// sub-shell prog space
  };

  enum RunStat {	// css running status
    Waiting,		// waiting for input
    Running,		// running normally
    Stopping,		// got to end of program
    Returning,		// received a return
    Breaking,		// received a break
    Continuing,		// received a continue
    BreakPoint,		// stopped in a breakpoint
    ExecError,		// execution error
    Bailing		// stop running due to shell cmd or external exit
  };				

  enum ArgFlags {
    VarArg = -1,	// #IGNORE variable no of args
    NoArg  = -2 	// #IGNORE no args at all
  };

  String 	name;
  cssProg*	prog;		// cur program (filled in each time)

  virtual uint		GetSize() const     	{ return sizeof(*this); }
  virtual const char* 	GetName() const    	{ return (const char*)name; }
  virtual cssElPtr	GetAddr() const;
  virtual void		SetAddr(const cssElPtr& adr);
  virtual cssTypes 	GetType() const   	{ return T_Void; }
  virtual const char*	GetTypeName() const 	{ return "(void)"; }
  virtual cssEl*	GetTypeObject() const; // gets type object corresponding to this
  virtual int		GetParse() const	{ return CSS_VAR; }
  virtual int		IsRef() const	      	{ return false; }
  // check to see if a var is actually a ref to a var
  virtual cssEl*	GetActualObj() 		{ return this; }
  // get non-reference, non-pointer object

  // elaborate printing (with type information, etc..
  virtual String 	PrintStr() const		// the output string
  { return String(GetTypeName()) + " " + name; }
  virtual void 		Print(ostream& fh = cout) const	{ fh << PrintStr(); }

  virtual void		PrintR(ostream& fh = cout) const { Print(fh); }
  // recursive print item (item and all its sub-elements)
  virtual int		Edit(bool =false)	{ return false; }
  // pull up in the interface and edit this one

  // "value" printing 
  virtual String	PrintFStr() const 		 { return "void"; }
  virtual void 		PrintF(ostream& fh = cout) const { fh << PrintFStr(); }

  virtual void 		List(ostream& fh = cout) const	{ Print(fh); }
  virtual void  	TypeInfo(ostream& fh = cout) const
  { fh << GetTypeName() << " " << name; }
  virtual void		InheritInfo(ostream& fh = cout) const { TypeInfo(fh); }

  // saving and loading objects to/from files (special format)
  virtual void		Save(ostream& strm = cout);
  virtual void		Load(istream& strm = cin);

  // token information about a certain type
  virtual void		TokenInfo(ostream&) const	{ }; // show tokens of arg type
  virtual cssEl*	GetToken(int) const		{ return (cssEl*)this; }

  virtual cssEl::RunStat 	Do(cssProg* prg); 	

  // constructors
  void 			Constr();
#ifdef CSS_DEBUG_REGISTER
  virtual void		Register(); 	// for debugging purposes
#else
  virtual void		Register()	{ };  // when not debugging
#endif

  cssEl()			{ name = ""; Constr(); }
  cssEl(const char* nm)		{ name = nm; Constr(); }
  cssEl(const cssEl& cp)	{ Constr(); Copy(cp); }
  cssEl(const cssEl& cp, const char* nm) { Constr(); Copy(cp); name = nm; }

  virtual ~cssEl();

  void   		Copy(const cssEl& cp);		// for copying to existing structs

  virtual cssEl* 	Clone() 			{ return new cssEl(*this); }
  virtual cssEl* 	AnonClone() 			{ return new cssEl(*this, ""); }
  virtual cssEl*	MakeToken_stub(int, cssEl**) 	{ return &cssMisc::Void; }

  virtual cssEl::RunStat 	MakeToken(cssProg* prg); 	
  virtual cssEl::RunStat 	MakeTempToken(cssProg* prg); 	

  // all static just for consistency..
  static void   	Done(cssEl* it)
  { if(it->refn <= 0) delete it; else it->deReferenced(); }
  static void  		Ref(cssEl* it)		{ it->refn++; }
  static void   	unRef(cssEl* it)	{ it->refn--; }
  static void		unRefDone(cssEl* it)	{ unRef(it); Done(it); }

  // routines for reference pointers
  static void		SetRefPointer(cssEl** ptr, cssEl* it)
  { if(*ptr != &cssMisc::Void)  cssEl::unRefDone(*ptr);
    *ptr = it; if(*ptr != &cssMisc::Void)  cssEl::Ref(*ptr); }
  static void		DelRefPointer(cssEl** ptr)
  { if(*ptr != &cssMisc::Void)  cssEl::unRefDone(*ptr); *ptr = &cssMisc::Void; }

  // and reference ElPtr's
  static void		SetRefElPtr(cssElPtr& ptr, const cssElPtr& it)
  { if(ptr.El() != &cssMisc::Void)  cssEl::unRefDone(ptr.El());
    ptr = it; if(ptr.El() != &cssMisc::Void)  cssEl::Ref(ptr.El()); }
  static void		DelRefElPtr(cssElPtr& ptr)
  { if(ptr.El() != &cssMisc::Void)  cssEl::unRefDone(ptr.El()); ptr.Reset(); }

  virtual void		deReferenced()		{ };
  // this is a hook for when obj is de-referenced but not actually done

  void NopErr(const char* opr="") const
  { cssMisc::Error(prog, "Operation:", opr, "not defined for type:", GetTypeName()); }

  // use this for conversions that don't work (safer)
  void CvtErr(const char* opr="") const
  { cssMisc::Error(prog, "Conversion:", opr, "not defined for type:", GetTypeName()); }

  // convert from types
  virtual String& GetStr() const 		{ return no_string; }
  virtual operator Real() const	 		{ return 0; }
  virtual operator float() const 		{ return (float)(Real)*this; }
  virtual operator Int() const	 		{ return 0; }
  virtual operator unsigned int() const 	{ return (unsigned int)(Int)*this; } 
  virtual operator char() const	 		{ return (char)(Int)*this; }
  virtual operator unsigned char() const 	{ return (unsigned char)(Int)*this; }
  virtual operator short() const    		{ return (short)(Int)*this; }
  virtual operator unsigned short() const	{ return (unsigned short)(Int)*this; }
  virtual operator long() const	 		{ return (long)(Int)*this; }
  virtual operator unsigned long() const	{ return (unsigned long)(Int)*this; }
  virtual operator long long() const		{ return (long long)(Int)*this; }
#ifndef NO_BUILTIN_BOOL
  virtual operator bool() const			{ return (Int)*this; }
#endif
  virtual operator const char*() const 		{ return (const char*)GetStr(); }
  virtual operator String() const		{ return GetStr(); }
  virtual operator char*() const		{ return (char*)(const char*)GetStr(); }
  virtual operator unsigned char*() const	{ return (unsigned char*)(const char*)*this; }
#ifndef NO_SIGNED
  virtual operator signed char*() const		{ return (signed char*)(const char*)*this; }
#endif
#if NEED_STREAMOFF_CSS_CONV
  virtual operator streamoff() const 		{ return (streamoff)(Int)*this; }
#endif

  virtual operator void*() const		{ CvtErr("(void*)"); return NULL; }
  virtual operator void**() const		{ CvtErr("(void**)"); return NULL; }
  
  virtual void* GetVoidPtrOfType(TypeDef* td) const 	{ CvtErr(td->name); return NULL; }
  virtual void* GetVoidPtrOfType(const char* td) const 	{ CvtErr(td); return NULL; }
  // these are type-safe ways to convert a cssEl into a ptr to object of given type

  virtual operator int*() const		{ CvtErr("(int*)"); return NULL; }
  virtual operator short*() const	{ CvtErr("(short*)"); return NULL; }
  virtual operator long*() const	{ CvtErr("(long*)"); return NULL; }
  virtual operator double*() const	{ CvtErr("(double*)"); return NULL; }
  virtual operator float*() const	{ CvtErr("(float*)"); return NULL; }
  virtual operator String*() const	{ CvtErr("(String*)"); return NULL; }
#ifndef NO_BUILTIN_BOOL
  virtual operator bool*() const	{ CvtErr("(bool*)"); return NULL; }
#endif

  virtual operator int**() const	{ CvtErr("(int**)"); return NULL; }
  virtual operator short**() const	{ CvtErr("(short**)"); return NULL; }
  virtual operator long**() const	{ CvtErr("(long**)"); return NULL; }
  virtual operator double**() const	{ CvtErr("(double**)"); return NULL; }
  virtual operator float**() const	{ CvtErr("(float**)"); return NULL; }
  virtual operator String**() const	{ CvtErr("(String**)"); return NULL; }
#ifndef NO_BUILTIN_BOOL
  virtual operator bool**() const	{ CvtErr("(bool**)"); return NULL; }
#endif

  virtual operator ostream*() const	{ CvtErr("(ostream*)"); return NULL; }
  virtual operator istream*() const	{ CvtErr("(istream*)"); return NULL; }
  virtual operator iostream*() const	{ CvtErr("(iostream*)"); return NULL; }
  virtual operator fstream*() const	{ CvtErr("(fstream*)"); return NULL; }
  virtual operator stringstream*() const { CvtErr("(stringstream*)"); return NULL; }

  virtual operator ostream**() const	{ CvtErr("(ostream**)"); return NULL; }
  virtual operator istream**() const	{ CvtErr("(istream**)"); return NULL; }
  virtual operator iostream**()	const	{ CvtErr("(iostream**)"); return NULL; }
  virtual operator fstream**() const	{ CvtErr("(fstream**)"); return NULL; }
  virtual operator stringstream**() const { CvtErr("(stringstream**)"); return NULL; }

  // support for external types
#ifdef CSS_SUPPORT_TYPEA
  virtual operator TAPtr() const;
  virtual operator TAPtr*() const 		{ CvtErr("(TAPtr*)"); return NULL; }
  virtual operator TypeDef*() const;
  virtual operator MemberDef*() const;
  virtual operator MethodDef*() const;
#endif

  // assign from types
  virtual void operator=(Real)	 		{ CvtErr("(Real)"); }
  virtual void operator=(Int)			{ CvtErr("(Int)"); }
  virtual void operator=(const String&)	 	{ CvtErr("(String)"); }

  virtual void operator=(void*)	 		{ CvtErr("(void*)"); }
  virtual void operator=(void**)		{ CvtErr("(void**)"); }

  virtual void AssignFromType(TypeDef* td, void*)	{ CvtErr(td->name); }
  virtual void AssignFromType(const char* td, void*)	{ CvtErr(td); }
  // type-safe way to assign a void ptr of given type to a cssEl

  // operators
  virtual void operator=(const cssEl&) 	{ NopErr("="); }
  virtual void CastFm(const cssEl& cp)	{ operator=(cp); } // default cast is a copy
  virtual void InitAssign(const cssEl& cp) { operator=(cp); } // default initial assign is cpy

  virtual void UpdateAfterEdit() { };

  virtual cssEl* operator+(cssEl&) { NopErr("+"); return &cssMisc::Void; }
  virtual cssEl* operator-(cssEl&) { NopErr("-"); return &cssMisc::Void; }
  virtual cssEl* operator*(cssEl&) { NopErr("*"); return &cssMisc::Void; }
  virtual cssEl* operator/(cssEl&) { NopErr("/"); return &cssMisc::Void; }
  virtual cssEl* operator%(cssEl&) { NopErr("%"); return &cssMisc::Void; }
  virtual cssEl* operator<<(cssEl&){ NopErr("<<"); return &cssMisc::Void; }
  virtual cssEl* operator>>(cssEl&){ NopErr(">>"); return &cssMisc::Void; }
  virtual cssEl* operator&(cssEl&) { NopErr("&"); return &cssMisc::Void; }
  virtual cssEl* operator^(cssEl&) { NopErr("^"); return &cssMisc::Void; }
  virtual cssEl* operator|(cssEl&) { NopErr("|"); return &cssMisc::Void; }
  virtual cssEl* operator-()       { NopErr("-"); return &cssMisc::Void; } // unary minus
  virtual cssEl* operator*()	   { NopErr("*"); return &cssMisc::Void; } // unary de-ptr
  virtual cssEl* operator[](int) const { NopErr("[]"); return &cssMisc::Void; }
  virtual int	 GetMemberNo(const char*) const { NopErr(".,->"); return -1; }
  virtual cssEl* GetMember(const char*) const  { NopErr(".,->"); return &cssMisc::Void; }
  virtual cssEl* GetMember(int) const  { NopErr(".,->"); return &cssMisc::Void; }
  virtual int	 GetMemberFunNo(const char*) const { NopErr(".,->()"); return -1; }
  virtual cssEl* GetMemberFun(const char*) const { NopErr(".,->()"); return &cssMisc::Void; }
  virtual cssEl* GetMemberFun(int) const { NopErr(".,->()"); return &cssMisc::Void; }
  virtual cssEl* GetScoped(const char*) const { NopErr("::"); return &cssMisc::Void; }
  virtual cssEl* NewOpr();
  virtual void 	 DelOpr()	{ NopErr("delete"); } // delete operator

  virtual bool operator< (cssEl&) { NopErr("<"); return 0; }
  virtual bool operator> (cssEl&) { NopErr(">"); return 0; }
  virtual bool operator! () 	  { NopErr("!"); return 0; }
  virtual bool operator<=(cssEl&) { NopErr("<="); return 0; }
  virtual bool operator>=(cssEl&) { NopErr(">="); return 0; }	
  virtual bool operator==(cssEl&) { NopErr("=="); return 0; }
  virtual bool operator!=(cssEl&) { NopErr("!="); return 0; }
  virtual bool operator&&(cssEl&) { NopErr("&&"); return 0; }
  virtual bool operator||(cssEl&) { NopErr("||"); return 0; }

  virtual void operator+=(cssEl&) { NopErr("+="); }
  virtual void operator-=(cssEl&) { NopErr("-="); }
  virtual void operator*=(cssEl&) { NopErr("*="); }
  virtual void operator/=(cssEl&) { NopErr("/="); }
  virtual void operator%=(cssEl&) { NopErr("%="); }
  virtual void operator<<=(cssEl&){ NopErr("<<="); }
  virtual void operator>>=(cssEl&){ NopErr(">>="); }
  virtual void operator&=(cssEl&) { NopErr("&="); }
  virtual void operator^=(cssEl&) { NopErr("^="); }
  virtual void operator|=(cssEl&) { NopErr("|="); } 
};


class cssSpace {
  // a name space of some sort, can be a stack, or a list
protected:
friend	class cssArray;
friend  class cssClassType;  
friend  class cssClassInst;  
  int 		alloc_size;		// allocated size of space
public:
  cssElPtr 	el_retv;		// return value for a Find
  String 	name;
  int 		size;			// number of actual elements
  cssEl**	els;			// the elements themselves

  static ostream& fancy_list(ostream& fh, const String& itm, int no, int prln, int tabs);
  static String& fancy_list(String& fh, const String& itm, int no, int prln, int tabs);

  void 		Constr();
  cssSpace()				{ alloc_size = 2;  Constr(); }
  cssSpace(const char* nm)		{ alloc_size = 2;  name = nm;  Constr(); }
  cssSpace(int no, const char* nm)	{ alloc_size = no;  name = nm;  Constr(); }
  cssSpace(const cssSpace& cp)		{ alloc_size = 2; Constr();  Copy(cp); }
  virtual ~cssSpace()			{ Reset();  free(els); }

  void Copy(const cssSpace& cp);
  void CopyUniqNameNew(const cssSpace& cp); // copy only unique items, keep new one
  void CopyUniqNameOld(const cssSpace& cp); // copy only unique items, keep old one

  cssElPtr&	FindName(const char* nm); // lookup by name
  cssElPtr&	Find(Int nm);		// lookup by number value
  cssElPtr&	Find(Real nm);		// lookup by number value
  cssElPtr&	Find(const String& nm); // lookup by string value
  int		GetIndex(cssEl* it);	// find it, return index
  
  void 		List(ostream& fh = cout) const; 	// (elaborate print format)
  void 		NameList(ostream& fh = cout) const;   // just the names
  void		ValList(ostream& fh = cout) const;    // just the values (printf format)
  void		TypeNameList(ostream& fh = cout) const; // "fancy" type/name output
  void		TypeNameValList(ostream& fh = cout) const; // "fancy" type/name/val output
  String	PrintStr() const;
  String	PrintFStr() const;

  void		Alloc(int sz);		// allocate space on the list..
  void 		Reset();		// clear list
  void		Sort();			// sort list

  // for stack-like operations
  cssElPtr&	Push(cssEl* it);
  cssElPtr&	PushUniqNameNew(cssEl* it);
  cssElPtr&	PushUniqNameOld(cssEl* it);
  cssEl*	Pop();			// this pop does not delete
  void		DelPop(); 		// pop and delete item
  bool		Remove(cssEl* it); 	// not a good idea, because ptrs are index based
  bool		Replace(cssEl* old, cssEl* nw); // replace at index with it
  cssEl*	Peek() const
  { cssEl* rval=&cssMisc::Void; if(size > 0) rval = els[size-1]; return rval; }

  // operators
  cssEl* 	FastEl(int i) const	{ return els[i]; }
  cssEl*	El(int i) const
  { cssEl* rval=&cssMisc::Void; if(i < size) rval=els[i]; return rval; }
  cssEl* operator[](int i) const	{ return El(i); }
};


class cssElFun : public cssEl {
  // basic things for function-type objects (this is a base type for other classes)
public:
  static const int ArgMax;      // maximum number of arguments
  cssSpace	arg_defs;	// default values for arguments
  String_Array	arg_vals;	// current arg values (as strings), for gui stuff
  int		def_start;	// where in args do the defaults start
  int		argc;		// number of args desired
  RunStat	dostat;		// return status value for do
  cssEl*	retv_type;	// return value type

  void		SetRetvType(cssEl* rvt) { cssEl::SetRefPointer(&retv_type, rvt); }

  int		GetParse() const	{ return CSS_FUN; }
  cssEl*	GetTypeObject() const	{ return (cssEl*)this; }

  virtual int   BindArgs(cssEl** args, int& act_argc);     // returns actual no of args
  virtual void  DoneArgs(cssEl** args, int& act_argc);     // Done with args
  virtual void	GetArgDefs();	// get argument defaults

  void		Copy(const cssElFun& cp);
  cssElFun();
  ~cssElFun();
};

class cssElCFun : public cssElFun {
  // a C function returning a cssEl pointer
public:
  int		parse;				// parser token code 
  cssEl*	(*funp)(int ac, cssEl* args[]); // function pointer
  String	help_str;			// help string for function

  int		GetParse() const	{ return parse; }
  uint		GetSize() const		{ return sizeof(*this); }
  cssTypes 	GetType() const		{ return T_ElCFun; }
  const char*	GetTypeName() const	{ return "(ElCFun)"; }
  cssEl*	GetTypeObject() const	{ return (cssEl*)this; }

  cssEl::RunStat 	Do(cssProg* prog);

  // constructors
  void 		Constr();
  void		Copy(const cssElCFun& cp);
  cssElCFun(int ac, cssEl* (*fp)(int, cssEl* args[]));
  cssElCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm);
  cssElCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm, int pt, const char* hstr=NULL);
  cssElCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm, cssEl* rtype, const char* hstr=NULL);
  cssElCFun(const cssElCFun& cp);
  cssElCFun(const cssElCFun& cp, const char* nm);
  ~cssElCFun();

  cssCloneOnly(cssElCFun);
  cssEl*	MakeToken_stub(int na, cssEl* arg[]);
  // return retv_type token, else cssInt
  String& GetStr() const	{ return (String&)name; } // types can give strings...
};

// pt can be either the parsetype or the retv_type
#define cssElCFun_inst(l,x,n,pt,hl)	l .Push(new cssElCFun(n, cssElCFun_ ## x ## _stub, #x, pt, hl))
#define cssElCFun_inst_nm(l,x,n,s,pt,hl) l .Push(new cssElCFun(n, cssElCFun_ ## x ## _stub, s, pt, hl))
#define cssElCFun_inst_ptr(l,x,n,pt,hl)	l .Push(cssBI::x = new cssElCFun(n, cssElCFun_ ## x ## _stub, #x, pt, hl))
#define cssElCFun_inst_ptr_nm(l,x,n,s,pt,hl) l .Push(cssBI::x = new cssElCFun(n, cssElCFun_ ## x ## _stub, s, pt, hl))

class cssElInCFun : public cssElCFun {
  // a simple internal C function (having fixed number of parameters)
public:
  int		BindArgs(cssEl** args, int& act_argc);
  // has a simpler, faster version of bind args for internal functions

  // constructors
  cssElInCFun(int ac, cssEl* (*fp)(int, cssEl* args[]));
  cssElInCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm);
  cssElInCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm, int pt);
  cssElInCFun(int ac, cssEl* (*fp)(int, cssEl* args[]), const char* nm, cssEl* rtype);
  cssElInCFun(const cssElInCFun& cp);
  cssElInCFun(const cssElInCFun& cp, const char* nm);

  cssCloneOnly(cssElInCFun);
};

#define cssElInCFun_inst(l,x,n,pt)	l .Push(new cssElInCFun(n, cssElCFun_ ## x ## _stub, #x, pt))
#define cssElInCFun_inst_nm(l,x,n,s,pt) l .Push(new cssElInCFun(n, cssElCFun_ ## x ## _stub, s, pt))
#define cssElInCFun_inst_ptr(l,x,n,pt) 	l .Push(cssBI::x = new cssElInCFun(n, cssElCFun_ ## x ## _stub, #x, pt))
#define cssElInCFun_inst_ptr_nm(l,x,n,s,pt) l .Push(cssBI::x = new cssElInCFun(n, cssElCFun_ ## x ## _stub, s, pt))

class cssMbrCFun : public cssElFun {
  // a C code member function returning a pointer to a cssEl
public:
  cssEl*	(*funp)(void* ths, int na, cssEl* args[]); // function pointer
  void* 	ths;					   // type instance

  cssTypes 	GetType() const		{ return T_MbrCFun; }
  const char*	GetTypeName() const	{ return "(MbrCFun)"; }
  cssEl*	GetTypeObject() const	{ return (cssEl*)this; }

  cssEl::RunStat Do(cssProg* prog);

  // constructors
  void		Constr();
  void		Copy(const cssMbrCFun& cp);
  cssMbrCFun();
  cssMbrCFun(int ac, void* th, cssEl* (*fp)(void*, int, cssEl**));
  cssMbrCFun(int ac, void* th, cssEl* (*fp)(void*, int, cssEl**), const char* nm);
  cssMbrCFun(const cssMbrCFun& cp);
  cssMbrCFun(const cssMbrCFun& cp, const char* nm);
  ~cssMbrCFun();

  cssCloneOnly(cssMbrCFun);

  cssEl*	MakeToken_stub(int na, cssEl* arg[]);
};


// return variable name
#define cssRetv_Name "_retv_this"
// block function name
#define cssBlock_Name "_block"
// switch block cond variable
#define cssSwitchVar_Name "_switch_this"
// switch block name
#define cssSwitchBlock_Name "_switch_block"
// switch jump table values
#define cssSwitchJump_Name "_switch_jump"
// switch default case name
#define cssSwitchDefault_Name "_switch_default"

class cssScriptFun : public cssElFun {
  // a function defined in the script language
public:
  cssElPtr* 	argv;		// the actual argument holders (0 is retv)
  cssProg*	fun;
  bool		is_block;	// true if this is actually just a block

  uint		GetSize() const		{ return sizeof(*this); }
  cssTypes 	GetType() const 	{ return T_ScriptFun; }
  const char*	GetTypeName() const 	{ return "(ScriptFun)"; }
  cssEl*	GetTypeObject() const	{ return (cssEl*)this; }

  cssEl::RunStat Do(cssProg* prg);

  String	PrintStr() const;
  String	PrintFStr() const		{ return PrintStr(); }
  void 		List(ostream& fh = cout) const;

  virtual void	Define(cssProg* prg, bool decl = false, const char* nm = NULL);
  // initialize the function (decl = true if in declaration, not definition)

  // constructors
  void		Constr();
  void		Copy(const cssScriptFun& cp);
  cssScriptFun();
  cssScriptFun(const char* nm);
  cssScriptFun(const cssScriptFun& cp);
  cssScriptFun(const cssScriptFun& cp, const char* nm);
  ~cssScriptFun();

  cssCloneOnly(cssScriptFun);
  cssEl*	MakeToken_stub(int na, cssEl* arg[]);
  // return retv_type token, else cssInt
};

class cssMbrScriptFun : public cssScriptFun {
  // a function defined in the script language
public:
  cssClassType*	type_def;	// the class fun belongs to
  String	desc;		// a description of this function
  String	opts;		// user-settable comment directives (from desc)
  bool		is_tor;		// is a constructor or destructor
  bool		is_virtual;	// is a virtual function

  uint		GetSize() const		{ return sizeof(*this); }
  cssTypes 	GetType() const 	{ return T_MbrScriptFun; }
  const char*	GetTypeName() const 	{ return "(MbrScriptFun)"; }

  cssEl::RunStat Do(cssProg* prg);

  String	PrintStr() const;

  void		Define(cssProg* prg, bool decl = false, const char* nm = NULL);
  // initialize the function

  void		SetDesc(const char* des); // get options from desc
  bool		HasOption(const char* opt) { return opts.contains(opt); }
  String	OptionAfter(const char* opt);

  void		Constr();
  void		Copy(const cssMbrScriptFun& cp);
  cssMbrScriptFun(const char* nm, cssClassType* cls);
  cssMbrScriptFun(const cssMbrScriptFun& cp);
  cssMbrScriptFun(const cssMbrScriptFun& cp, const char* nm);
  ~cssMbrScriptFun();

  cssCloneOnly(cssMbrScriptFun);
  cssEl*	MakeToken_stub(int na, cssEl* arg[]);
  // return retv_type token, else cssInt
};

#define cssCPtr_CloneFuns(x, init) \
  cssEl* 	Clone()		{ return new x (*this); }	      \
  cssEl* 	AnonClone() 	{ return new x (*this, ""); }	      \
  cssEl*	MakeToken_stub(int, cssEl* arg[])		      \
    { return new x ( init, ptr_cnt, (const char*)*(arg[1])); }	      \


class cssCPtr : public cssEl {
  // base class for ptrs to C objects
protected: 
  cssEl*	class_parent;	// if this pointer was derived from a class structure
public:
  void* 	ptr;
  int		ptr_cnt;	// number of ptrs (1= *, 2= **, etc.)
  bool		read_only;	// true if this object comes from read-only member

  int		GetParse() const	{ return CSS_PTR; }
  uint		GetSize() const		{ return 0; } // use for ptrs
  cssTypes 	GetType() const		{ return T_C_Ptr; }
  const char*	GetTypeName() const 	{ return "(C_Ptr)"; }

  String 	PrintStr() const 
  { return String(GetTypeName())+" "+name + " --> " + String((long)ptr); }
  String	PrintFStr() const { return String((long)ptr); }

  // constructors
  void		Constr();	   
  cssCPtr();
  cssCPtr(void* it, int pc);
  cssCPtr(void* it, int pc, const char* nm);
  cssCPtr(void* it, int pc, const char* nm, cssEl* cp, bool ro);
  cssCPtr(const cssCPtr& cp);
  cssCPtr(const cssCPtr& cp, const char* nm);
  ~cssCPtr();

  cssCPtr_CloneFuns(cssCPtr, (void*)NULL);

  virtual void	SetClassParent(cssEl* cp);

  // converters
  virtual void*	GetVoidPtr(int cnt = 1) const;	// goes down till ptr_cnt == cnt
  virtual void* GetNonNullVoidPtr(int cnt = 1) const; // generates error if null

  String&	GetStr() const	{ ((cssEl*)this)->tmp_str = String((long)ptr); return (String&)tmp_str; }
  operator	Real() const	{ CvtErr("(Real)"); return 0.0; }
  operator 	Int() const	{ return (Int)(long)(ptr); }
  operator 	void*()	const	{ return GetVoidPtr(1); }
  operator 	void**() const	{ return (void**)GetVoidPtr(2); }

  void operator=(Real)	 	{ CvtErr("(Real)"); }
  void operator=(Int)		{ CvtErr("(Int)"); }
  void operator=(const String&)	{ CvtErr("(String)"); }
  void operator=(void* cp)	{ ptr = cp; ptr_cnt = 1; }
  void operator=(void** cp)	{ ptr = (void*)cp; ptr_cnt = 2; }

  // operators
  virtual bool	ROCheck();	// do read_only check, true if ok to modify, else err
  virtual void 	PtrAssignPtr(cssCPtr* s); 	// use when assigning ptrs to ptrs
  void operator=(const cssEl& s);    

  void	UpdateAfterEdit();

  cssEl* operator*();
  cssEl* operator*(cssEl&)	{ NopErr("*"); return &cssMisc::Void; }

  virtual bool 	SamePtrLevel(cssCPtr* s); // if this and s have diff cnt, emit warning

  bool operator< (cssEl& s) { return ((Int)(long)(ptr) < (Int)s); }
  bool operator> (cssEl& s) { return ((Int)(long)(ptr) > (Int)s); }
  bool operator! () 	    { return (ptr == 0); }
  bool operator<=(cssEl& s) { return ((Int)(long)(ptr) <= (Int)s); }
  bool operator>=(cssEl& s) { return ((Int)(long)(ptr) >= (Int)s); }	
  bool operator==(cssEl& s);	// these two check for sameptrlevel
  bool operator!=(cssEl& s);
  bool operator&&(cssEl& s) { return ((Int)(long)(ptr) && (Int)s); }
  bool operator||(cssEl& s) { return ((Int)(long)(ptr) || (Int)s); }
};

#define cssCPtr_inst(l,n)		l .Push(new cssCPtr(& n,1,#n))
#define cssCPtr_inst_nm(l,n,c,s)	l .Push(new cssCPtr(n,c, s))
#define cssCPtr_inst_ptr(l,n,x)	l .Push(cssBI::x = new cssCPtr(& n,1,#x))
#define cssCPtr_inst_ptr_nm(l,n,c,x,s) l .Push(cssBI::x = new cssCPtr(n,c,s))


// lexer & preprocessor stuff (#define, etc)

class cssLex {
public:
  static String Buf;

  static int readword(cssProg* prog, int c);
};

class cssDef : public cssElFun {
  // a pre-processor define 
public:
  String_Array	val;		// contains the bits and pieces that surround the args
  int_Array	which_arg;	// determines which arg to insert before each val item

  int		GetParse() const	{ return CSS_PP_DEF; }
  uint		GetSize() const 	{ return sizeof(*this); }
  cssTypes 	GetType() const 	{ return T_PP_Def; }
  const char*	GetTypeName() const  	{ return "#define"; }

  cssEl::RunStat 	Do(cssProg* prog);

  static void	Skip_To_Endif(cssProg* prog); // skip text to next endif

  void	Constr();

  cssDef(int ac);
  cssDef(int ac, const char* nm);
  cssDef(const cssDef& cp);
  cssDef(const cssDef& cp, const char* nm);

  cssCloneFuns(cssDef, 0);
};

class cssInst {
public:
  enum CodeFlags {
    Stop = -1 				// signal to stop execution
  };

  cssProg*	prog;			// program I belong to
  int 		idx;			// index within the program
  int 		line;			// points to source code line number
  int 		col;			// column in line
  cssElPtr	inst;			// instruction this points to
  bool 		isdefn;			// is this the definition of the object?
  css_progdx	previf;			// previous if instr

  virtual int 		Print(ostream& fh = cout) const;
  virtual int 		List(ostream& fh = cout) const;
  virtual cssEl::RunStat 	Do();
  virtual void 		SetLine(css_progdx it) 	{ line = it; }
  virtual css_progdx	GetJump()		{ return -1; }	
  virtual bool		IsJump()		{ return false; }

  void 			SetInst(const cssElPtr& it);
  void			EndIf(css_progdx end = -1);	// end an if statement (and set end of previf)
  void			SetDefn();	// This instruction is a definition (ptr is a fun..)

  // constructors
  void 		Constr();
  void		Copy(const cssInst& cp);
  cssInst();
  cssInst(const cssProg& prg, const cssElPtr& it);
  cssInst(const cssProg& prg, const cssElPtr& it, int lno, int clno);
  cssInst(const cssInst& cp);
  virtual ~cssInst();

  virtual cssInst* Clone() { return new cssInst(*this); }
};

class cssIJump : public cssInst {
public:
  css_progdx    jumpto;			// idx to jump to

  int 		Print(ostream& fh = cout) const;
  int 		List(ostream& fh = cout) const	{ return Print(fh); }
  cssEl::RunStat 	Do();
  void 		SetLine(css_progdx it) 		{ jumpto = it; }
  css_progdx	GetJump()			{ return jumpto; }	
  bool		IsJump()			{ return true; }

  void		Copy(const cssIJump& cp);
  cssIJump(const cssProg &prg, css_progdx jmp);
  cssIJump(const cssProg &prg, css_progdx jmp, int lno, int clno);
  cssIJump(const cssIJump& cp);

  virtual cssInst* Clone() { return new cssIJump(*this); }
};


class cssListEl {
public:
  css_progdx    stpc;		// starting pc for this line
  int		ln;		// line no in source code
  String	src;		// source code for line

  void		Copy(const cssListEl& cp)
  { stpc = cp.stpc; ln = cp.ln; src = cp.src; }

  cssListEl()			{ stpc = 0;   ln = 0; }
  cssListEl(css_progdx pc, int l, const char* cd)
  { stpc = pc;  ln = l;  src = cd;  }
  cssListEl(const cssListEl& cp)	{ Copy(cp); }

  cssListEl* 	Clone()	{ return new cssListEl(*this); }
};


class cssFrame {
public:
  cssProg*	prog;			// program I belong to
  int		fr_no;		        // frame number
  css_progdx 	pc;			// program counter
  cssSpace	stack;			// current stack
  cssSpace	autos;			// current autos
  cssClassInst*	cur_this;		// current this pointer

  cssFrame(cssProg* prg);
};

class cssProg {
  // a single chunk of script code (a block, a function, etc.)
protected:
  friend class	cssProgSpace;
  int 		alloc_size;		// allocated size of program
  int		fr_alloc_size;		// allocated size of frames
  int 		src_alloc_size;		// allocated size of source
  cssElPtr	el_retv;		// return value for lookup, etc
  int		refn;			// reference count
public:
  // flag state values
  enum State_Flags {
    State_Shell		= 1,
    State_Run		= 2,
    State_Cont		= 4,
    State_Defn		= 8, 	// defn = defining, else shelling if in shell
    State_RunLast	= 16, 	// currently executing a runlast
    State_WasBurped	= 32  	// last line was burped, (delete src if necc)
  };

  enum YY_Flags {
    YY_Exit		= 0,	// #IGNORE script is done being parsed
    YY_Ok		= 1,	// #IGNORE everything is fine
    YY_NoSrc		= -2,	// #IGNORE don't code last line as source
    YY_Err		= -3,	// #IGNORE error
    YY_Blank     	= -4,	// #IGNORE blank line
    YY_Parse     	= -5 	// #IGNORE need to parse more
  };
  
  String 	name;			// name of program
  cssScriptFun*	owner;			// if owned by a cssScriptFun
  cssProgSpace* top;			// top-level space holding this one

  cssInst**	insts;			// the instructions themselves
  css_progdx 	size;			// number of instructions
  css_progdx	parse_st_size;		// starting size for parsing
  int		parse_st_src_size;	// source starting size for parsing
  css_progdx	run_st_size;		// starting size for runlast
  int		run_st_src_size;	// source starting size for runlast

  cssFrame** 	frame;			// anything dynamic goes here
  int		fr_size;		// how deep is your frame?

  cssSpace	literals;		// literal constants used during parsing
  cssSpace	statics;		// static variables
  cssSpace	saved_stack;		// saved stack state

  cssListEl**	source;			// keep the source code here
  int 		src_size;		// number of lines of source code
  int 		line;			// current source[] index number (parsing)
  int 		col;			// current source[] column number (parsing)
  int		st_line;		// where started parsing...
  int		st_col;
  int		tok_line;		// where the current token started
  int		tok_col;
  int		state;			// prog-specific state
  
  int_Array	breaks;			// breakpoints
  css_progdx	lastif;			// last if statment index (for else)
  css_progdx	elseif;			// last if for the else-if construct
  css_progdx	lastdo;			// last do statment index (for while)

  void 		Constr();
  void		Copy(const cssProg& cp);
  cssProg();
  cssProg(const char* nm);
  cssProg(const cssProg& cp);
  virtual ~cssProg();

  // all static just for consistency..
  static void   Done(cssProg* it)	{ if(it->refn <= 0) delete it; }
  static void  	Ref(cssProg* it)	{ it->refn++; }
  static void   unRef(cssProg* it)	{ it->refn--; }
  static void	unRefDone(cssProg* it)	{ unRef(it); Done(it); }
  
  // internal functions
  void		AllocInst(int sz);
  void		AllocFrame(int sz);
  void		AllocSrc(int sz);

  cssInst* 	Inst(int i) const
  { cssInst* rval = NULL; if((int)i<size) rval = insts[i]; return rval; }

  cssFrame*	Frame() const 		{ return frame[fr_size-1]; }
  cssFrame*	Frame(int frdx) const 	{ return frame[frdx]; }
  cssSpace*	Autos() const		{ return &(Frame()->autos); }
  cssSpace*	Autos(int frdx) const	{ return &(Frame(frdx)->autos); }
  cssSpace*	Stack() const		{ return &(Frame()->stack); }
  cssSpace*	Stack(int frdx) const 	{ return &(Frame(frdx)->stack); }
  css_progdx 	PC() const	     	{ return Frame()->pc; }
  css_progdx	PC(int frdx) const     	{ return Frame(frdx)->pc; }
  cssClassInst* CurThis() const		{ return Frame()->cur_this; }
  cssClassInst* CurThis(int frdx) const	{ return Frame(frdx)->cur_this; }
  void		SetCurThis(cssClassInst* ths)	{ Frame()->cur_this = ths; }

  int 		AddFrame();		// copies autos to new frame
  int		DelFrame();		// removes a frame
  void 		Reset();		// reset code, autos, frames, etc
  void		ResetCode();		// get rid of all insts and source

  cssProgSpace*	SetTop(cssProgSpace* pspc)
  { cssProgSpace* rval = top; top = pspc; return rval; }
  // set the top pointer, returning current one, which should be saved and used to pop
  void		PopTop(cssProgSpace* pspc)  	{ top = pspc; }
  // pop the top pointer, set to value returned by SetTop upon exit of scope

  // source, debugging
  char*		GetSrc() const
  { char* rval=""; if(line < src_size) rval = (char*)(source[line]->src) + col; return rval; }
  char*		GetSrcLC(int ln, int cl=0) const
  { char* rval=""; if(ln < src_size) rval = (char*)(source[ln]->src) + cl; return rval; }
  int		CurSrcLn(css_progdx pcval); 	// extrnl srcln
  int		CurSrcLn()			{ return CurSrcLn(PC()); }
  int		CurSrcLC(css_progdx pcval); 	// internal ln cnt
  int		CurSrcLC()			{ return CurSrcLC(PC()); }
  int		FindSrcLn(int ln);		// find the particular one
  int		ClosestSrcLn(int ln);		// find closest to ln
  int		HasSrcLn(int st, int ed);	// find one in the given range
  int           CurSrcCharsLeft();              // number of chars remaining in cur src ln
  int 		SubList(int sln, int eln, ostream& fh = cout);
  int 		List(ostream& fh = cout);
  int 		List(ostream& fh, int st, int nlines);
  void		ListSpace(ostream& fh = cout, int frdx = -1);
  void		SubPrint(css_progdx pcdx, ostream& fh = cout); // elaborate print
  int 		Print(css_progdx pcdx, ostream& fh = cout); // returns source cd line

  // coding
  int 		AddCode(cssInst* it);
  cssElPtr&	AddAuto(cssEl* it);
  cssElPtr&	AddLiteral(cssEl* it);
  int 		AddSrc(const char* line);
  void		MarkParseStart()
  { parse_st_size = size; parse_st_src_size = src_size; }
  void		MarkRunStart()
  { run_st_size = size; run_st_src_size = src_size; }
  void		ZapFrom(int zp_size, int zp_src_size); // zap from
  void          ZapFromSrc(int zp_src_size); // zap only source, not inst 
  void		ZapLastParse()		{ ZapFrom(parse_st_size, parse_st_src_size); }
  void		ZapLastRun()		{ ZapFrom(run_st_size, run_st_src_size); }
  int 		Getc();
  void 		unGetc()		{ col--; }
  int 		ReadLn();		// read the line in from filein

  int 		Code(cssEl* it); 
  int 		Code(cssElPtr &it); 
  int 		Code(const char* nm); 
  int 		Code(css_progdx it);
  int 		Code(cssIJump* it)	{ return AddCode(it); }
  void		UnCode()		{ if(size > 0) delete insts[--size]; }
  void		ResetLasts() 		{ lastif = -1; lastdo = -1; }
  void		BurpSrc(); 		// source was read in advance, burp it
  int		Undo(int srcln);
  
  cssElPtr&	FindAutoName(const char* nm);	// lookup by name
  cssElPtr&	FindLiteral(Int it)	{ return literals.Find(it); }
  cssElPtr&	FindLiteral(Real it)	{ return literals.Find(it); }
  cssElPtr&	FindLiteral(const String& it) { return literals.Find(it); }

  // execution
  void 		SetPC(css_progdx npc);
  cssProg*	SetSrcPC(int srcln); 	// this one goes by src ln
  cssInst* 	Next();
  inline cssInst* Next_Peek();
  cssEl*	Cont();	 		// continue with rest of program
  cssEl*	Cont(css_progdx st)	{ SetPC(st); return Cont(); }
  cssEl*	ContSrc(int srcln);	// continue with rest of program
  cssEl*	Run()	 		{ Restart(); return Cont(); }
  cssEl*	RunLast();
  void 		Restart();		// restart execution
  void		EndRunPop()		{ Stack()->DelPop(); }
  void		SaveStack();		// save current stack
  void		ReloadStack();		// reload current stack from saved

  // breakpoints
  int 		SetBreak(int srcln);
  bool		IsBreak(css_progdx pcval)
  { bool rval = false; if(breaks.Find(pcval) >= 0) rval = true; return rval; }

  bool		IsBreak()		{ return IsBreak(PC()); }
  void		ShowBreaks(ostream& fh = cout);
  bool		unSetBreak(int srcln);
};

class cssProgStack {
  // contains an index of what frame a given program is in 
public:
  cssProg*	prog;
  int		fr_no;
};


class cssProgSpace {
  // an entire program space, including all functions defined, variables etc.
protected:
friend class cssProg;

  enum ShellCmds {	// these flag that the given command was requested to run
    SC_None,
    SC_Compile,			// file name is in sc_compile_this
    SC_reCompile,
    SC_Source,			// file name is in sc_compile_this
    SC_Defn,
    SC_Shell,			// shell object is in sc_shell_this
    SC_Run,
    SC_Restart,
    SC_Reset,
    SC_ClearAll,
    SC_Undo
  };

  enum CompileCtrl {		// compiling control state (delayed compile actions)
    CC_None,
    CC_Push,			// prog to push in cc_push_this
    CC_Pop,
    CC_Include			// file name is in cc_include_this
  };

  int 		alloc_size;		// allocated number of prog_stacks
  int		old_debug;		// saved version
  cssElPtr	el_retv;		// return value for getel

  bool		cont_pending;		// a continue is pending
  css_progdx	cont_here;		// continue at this point
    
  CompileCtrl	compile_ctrl;		// control flags for delayed compile actions
  cssProg*	cc_push_this;		// push this object
  String	cc_include_this;	// filename to be included

  ShellCmds	shell_cmds;		// shell command to run next
  int		sc_undo_this;		// undo this program index
  String	sc_compile_this;	// filename to be compiled
  cssProgSpace* sc_shell_this;		// shell to shell
public:

  String 	name;
  String	prompt;
  String	act_prompt;		// the actual prompt

  int 		size;
  cssProgStack** progs;

  int		state;
  int		depth;			// number of levels in shells, etc.

  cssSpace	hard_vars;		// space-specific extern vars
  cssSpace	hard_funs;		// space-specific extern funs
  cssSpace	statics;		// global variables (in space)
  cssSpace	types;			// types defined in space
  
  istream*	fin;			// input file (current)
  ostream*	fout;			// output file
  ostream*	ferr;			// error file
  istream*	fshell;			// shell input file
  int		step_mode;		// step mode
  cssEl::RunStat run;			// flag to tell if running
  int 		debug;			// debug level
  bool		external_exit;		// set to true to break out of a shell...
  bool		in_readline;		// currently reading input from shell

  int 		src_ln;			// present source line no (parsing)
  int		st_src_ln;		// starting source line no
  int		list_ln;		// present source line no (listing)
  int		list_n;			// number of lines to list (default 20)
  int		st_list_ln;		// saved list ln
  int		lstop_ln;		// stopping line
  int	 	prev_ln;		// previous line no (listing)

  bool		parsing_command; 	// true if we are presently parsing a command


  void Constr();			
  cssProgSpace();
  cssProgSpace(const char* nm);
  virtual ~cssProgSpace();
  
  bool		DeleteOk();		// checks if its ok to delete this one
  void		DeferredDelete();	// call this if not ok to delete now..
  void		ExitShell();		// cause shell to exit

  cssProgStack* ProgStack()		{ return progs[size-1]; }
  cssProgStack* ProgStack(int prdx)	{ return progs[prdx]; }
  cssProg*	Prog()			{ return ProgStack()->prog; }
  cssProg*	Prog(int prdx)		{ return ProgStack(prdx)->prog; }
  int		Prog_Fr()		{ return ProgStack()->fr_no; }
  int		Prog_Fr(int prdx)	{ return ProgStack(prdx)->fr_no; }

  // internal coding, programs
  void		SetName(const char* nm); 		// updates all names
  void 		Reset();
  void		ClearAll();
  void		AllocProg(int sz);
  void		AddProg(cssProg* it);
  void		DelProg(); 		// delete program
  cssProg*	PopProg();		// pop (no delete) program
  void 		Push(cssProg* it);
  void		Shove(cssProg* it); 	// pushing while running
  cssProg*	Pop();
  cssProg*	Pull();			// popping while running
  cssScriptFun*	GetCurrentFun();	// find current function (or null if not in one)

  // internal coding, variables
  cssElPtr&	AddAuto(cssEl* it) 	{ return Prog()->AddAuto(it); }
  cssElPtr&	AddLiteral(cssEl* it) 	{ return Prog()->AddLiteral(it); }
  cssElPtr&	AddLiteral(String& str);
  cssElPtr&	AddLiteral(int itm);
  cssElPtr&	AddLiteral(Real itm);
  cssElPtr&	AddPtrType(cssEl* base_type); // add new pointer type of base_type
  cssElPtr&	AddRefType(cssEl* base_type); // add new reference type of base_type
  cssElPtr&	AddVar(cssEl* it); 	// for variables, not literals, etc.
  cssElPtr&	AddStaticVar(cssEl* it);// for variables declared static
  cssElPtr&	AddStatic(cssEl* it);	// for other static objects (not variables)
  bool		ReplaceVar(cssEl* old, cssEl* nw); 
  bool		RemoveVar(cssEl* it);	// this is somewhat dangerous..
  bool		DelVar(cssEl* it)	{ return ReplaceVar(it, &cssMisc::Void); }
  cssElPtr&	FindName(const char* nm); // lookup object by name (in autos, static, hards)
  cssSpace*	GetParseSpace(int idx);		// get parse spaces in order by index, NULL if over
  cssElPtr&	ParseName(const char* nm);	// parse name in all spaces (in order)
  cssElPtr&	FindTypeName(const char* nm); // find name based on type

  // compiling
  static  int	GetFile(fstream& fh, const char* fname); // get the file
  int		MarkListStart()			// where to start listing
  { st_list_ln = list_ln; st_src_ln = src_ln; return src_ln; }
  void		RestoreListStart(int old_src_ln)
  { src_ln = old_src_ln; list_ln = st_list_ln; }
  int		CompileLn(istream& fh = cin);	// parse next line of file
  void 		Compile(istream& fh = cin);	// parse a file and produce a program
  void 		Compile(const char* fname);	// parse a file and produce a program
  void 		CompileCode(const String& code);// parse string of code, produce a program
  void		Include(const char* fname);	// include a file
  void		CompileRunClear(const char* fname); // complile, run and clearall
  void 		reCompile();			// parse same file and produce a program
  void		Undo(int st);
  void		Undo()			{ Undo(src_ln-2); }
  void		ResetParseFlags();

  // compile control actions
  void		ClearCompileCtrl()	{ compile_ctrl = CC_None; }
  void		SetPop()		{ compile_ctrl = CC_Pop; }
  void		SetPush(cssProg* it)	{ compile_ctrl = CC_Push; cc_push_this = it; }
  void		SetInclude(const char* it) { compile_ctrl = CC_Include; cc_include_this = it; }
  bool		DoCompileCtrl(); 	// do compile control action based on flag

  // execution
  void 		Restart();
  cssEl* 	Cont();
  cssEl* 	Cont(css_progdx st);
  cssEl* 	ContSrc(int srcln);
  cssEl*	Run();
  void		SetCont(int it=-1)	{ cont_pending = true; cont_here = it; }
  void		EndRunPop()		{ Prog()->EndRunPop(); }

  // shell execution and commands
  bool		InShell()		{ return (state & cssProg::State_Shell); }
  void		Shell(istream& fhi = cin);
  void		CtrlShell(istream& fhi = cin, ostream& fho = cout, const char* prmpt = NULL);
  void		StartupShell(istream& fhi = cin, ostream& fho = cout);
  void 		Source(const char* fname);	// run a file as if in a shell

  bool		ShellCmdPending()	{ return (shell_cmds != SC_None); }
  void		SetSC(ShellCmds sc)	{ shell_cmds = sc; run = cssEl::Bailing; }
  void		ClearShellCmds()	{ SetSC(SC_None); }
  void		SetCompile(const char* it) { SetSC(SC_Compile); sc_compile_this = it; }
  void		SetReCompile()		{ SetSC(SC_reCompile); }
  void		SetSource(const char* it)  { SetSC(SC_Source); sc_compile_this = it; }
  void		SetDefn()		{ SetSC(SC_Defn); }
  void		SetShell(cssProgSpace* it) { SetSC(SC_Shell); sc_shell_this = it; }
  void		SetRun()		{ SetSC(SC_Run); }
  void		SetRestart()		{ SetSC(SC_Restart); }
  void		SetReset()		{ SetSC(SC_Reset); }
  void		SetClearAll()		{ SetSC(SC_ClearAll); }
  void		SetUndo(int it)		{ SetSC(SC_Undo); sc_undo_this = it; }
  bool		DoShellCmd();		// execute shell command based on flag

  // display, status
  int		ListDebug()
  { int rval=debug;
    if((debug >= 2) || ((state & cssProg::State_RunLast) && (old_debug >= 2))) rval=2;
    return rval; }
  void		SetDebug(int dblev);
  void		SetListStop();
  void 		List();
  void 		List(int st);
  void 		List(int st, int nlines);
  void		ListSpace();
  void		Status();
  void		Trace(int level=0);
  void		Help();

  // breakpoints
  void 		SetBreak(int srcln);
  void		ShowBreaks();
  void		unSetBreak(int srcln);
};

inline cssInst* cssProg::Next_Peek() {
  cssInst* rval = NULL; if(PC() >= size) top->run = cssEl::Stopping;
  else rval = insts[PC()]; return rval;
}

#endif // machine_h
