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

// Type Access: Automatic Access to C Types

#ifndef typea_h
#define typea_h 1

#include <ta/ta_stdef.h>
#include <ta/ta_list.h>

// comment directives which are parsed 'internally' (add a # sign before each)
// "NO_TOKENS"
// "IGNORE"
// "REG_FUN" 
// "INSTANCE"
// "NO_INSTANCE"
// "HIDDEN"
// "READ_ONLY"
// "NO_CSS"
// "NO_MEMBERS"

class EnumSpace;
class MemberSpace;
class MethodSpace;
class TypeSpace;
class EnumDef;
class MemberDef;
class MethodDef;
class TypeDef;
// for iv support
#ifdef NO_IV
class taivType;			// #IGNORE
class taivEdit;			// #IGNORE
class taivMember;		// #IGNORE
class taivMethod;		// #IGNORE
#else
class taivType;
class taivEdit;
class taivMember;
class taivMethod;
#endif
// for css support
class cssEl; 			// #IGNORE 

typedef cssEl* (*css_fun_stub_ptr)(void*, int, cssEl**);
typedef void (*ta_void_fun)();

class ta_memb_ptr_class {
public:
  virtual ~ta_memb_ptr_class()	{ }; // make sure it has a vtable..
};

typedef int ta_memb_ptr_class::* ta_memb_ptr;

class taBase;
typedef taBase* TAPtr;		// pointer to a taBase type

// this goes here so that it already knows about TypeDef's
#include <ta/ta_TA_type.h>

// a plain-array of strings
class String_PArray : public taPlainArray<String> {
public:
  int	FindContains(const char* op, int start=0) const;
  void	Add(const String& it)			{ taPlainArray<String>::Add(it); }
  void	Add(const char* it)			
  { String fad=it; taPlainArray<String>::Add(fad); }

  bool	AddUnique(const String& it)	{ return taPlainArray<String>::AddUnique(it); }
  bool	AddUnique(const char* it)
  { String fad=it; return taPlainArray<String>::AddUnique(fad); }

  void	operator=(const String_PArray& cp)	{ Copy_Duplicate(cp); }
  String_PArray()				{ };
  String_PArray(const String_PArray& cp)	{ InitArray_(); Duplicate(cp); }
  // returns first item which contains given string (-1 if none)
};

typedef taPlainArray<int> 	int_PArray;

class taMisc {
  // #NO_TOKENS #INSTANCE miscellanous global parameters and functions for type access system
public:
  enum ShowMembs {
    ALL_MEMBS = 0x00,
    NO_HIDDEN = 0x01,
    NO_READ_ONLY = 0x02,
    NO_HID_RO = 0x03,
    NO_DETAIL = 0x04,
    NO_HID_DET = 0x05,
    NO_RO_DET = 0x06,
    NO_HID_RO_DET = 0x07
  };

  enum TypeInfo {
    MEMB_OFFSETS,		// display all including member offsets
    ALL_INFO,			// display all type information
    NO_OPTIONS,			// don't display options
    NO_LISTS,			// don't display lists
    NO_OPTIONS_LISTS 		// don't display options or lists
  };

  enum KeepTokens {
    Tokens,			// keep tokens as specified by the type
    NoTokens,			// don't keep any tokens
    ForceTokens 		// force to keep all tokens
  };

  enum SaveFormat {
    PLAIN,			// dump files are not formatted for easy reading
    PRETTY 			// dump files should be more readable by humans
  };

  enum LoadVerbosity {
    QUIET,			// don't say anything except errors
    MESSAGES,			// display informative messages during load
    TRACE,			// and show a trace of objects loaded
    SOURCE 			// and show the source of the load as its loaded
  };

  enum AutoRevert {
    AUTO_APPLY,			// automatically apply changes before auto-reverting
    AUTO_REVERT,		// automatically revert, losing changes
    CONFIRM_REVERT 		// put up a confirmatory message before reverting
  };

  static String		version_no; 	// #READ_ONLY #NO_SAVE #SHOW version number of ta/css
  static TypeSpace 	types;		// #READ_ONLY #NO_SAVE list of all the active types 

  static bool		in_init;	// #READ_ONLY #NO_SAVE true if in ta initialization function
  static bool		not_constr;	// #READ_ONLY true if ta types are not yet constructed (or are destructed)

  static bool		iv_active;	// #READ_ONLY #NO_SAVE if iv has been started up or not
  static bool		is_loading;	// #READ_ONLY #NO_SAVE true if currently loading an object
  static bool		is_saving;	// #READ_ONLY #NO_SAVE true if currently saving an object
  static bool		is_duplicating;	// #READ_ONLY #NO_SAVE true if currently duplicating an object

  static int		dmem_proc; 	// #READ_ONLY #NO_SAVE #SHOW distributed memory process number (rank in MPI, always 0 for no dmem)
  static int		dmem_nprocs; 	// #READ_ONLY #NO_SAVE #SHOW distributed memory number of processes (comm_size in MPI, 1 for no dmem)

  static int		display_width;	// #SAVE width of shell display (in chars)
  static int		sep_tabs;	// #SAVE number of tabs to separate items by
  static int		max_menu;	// #SAVE maximum number of items in a menu
  static int		search_depth;   // #SAVE depth recursive find will search for a path object
  static int		color_scale_size; // #SAVE number of colors to put in a color scale
  static int		mono_scale_size;  // #SAVE number of monochrome bit-patterns to put in a color scale
  static float		window_decor_offset_x; // #SAVE some window managers (e.g., KDE) add an offset to location of windows -- add this amount to x position to compensate
  static float		window_decor_offset_y;  // #SAVE some window managers (e.g., KDE) add an offset to location of windows -- add this amount to y position to compensate
  static float		mswin_scale; 	// #SAVE window size scaling parameter for MS Windows 
  static int		jpeg_quality; 	// #SAVE jpeg quality for dumping jpeg files (1-100; 85 default)

  static ShowMembs	show;		// #SAVE what to show in general (eg. css) 
  static ShowMembs	show_iv;	// #SAVE what to show in interviews (the gui)
  static TypeInfo	type_info;	// #SAVE what to show when displaying type information
  static KeepTokens	keep_tokens;	// #SAVE default for keeping tokens
  static SaveFormat	save_format;	// #SAVE format to use when saving things (dump files)
  static LoadVerbosity	verbose_load;	// #SAVE report the names of things during loading
  static LoadVerbosity  iv_verbose_load; // #SAVE what to report in the load dialog
  static bool		dmem_debug; 	// #SAVE turn on debug messages for distributed memory processing
  static TypeDef*	default_scope;  // type of object to use to determine if two objects are in the same scope

  static bool		auto_edit; 	// #SAVE automatic edit dialog after creation?
  static AutoRevert	auto_revert;    // #SAVE when dialogs are automatically updated (reverted), what to do about changes?

  static String_PArray 	include_paths;  // #SAVE paths to be used for finding files

  static String		tmp_dir;	// #SAVE location of temporary files
  static String		compress_cmd;	// #SAVE command to use for compressing files
  static String		uncompress_cmd;	// #SAVE for uncompressing files
  static String		compress_sfx;	// #SAVE suffix to use for compressing files
  static String		help_file_tmplt; // #SAVE template for converting type name into a help file (%t = type name)
  static String		help_cmd;	// #SAVE how to run html browser to get help, %s is entire path to help file

  void	SaveConfig();		// #BUTTON #CONFIRM save configuration defaults to ~/.taconfig file that is loaded automatically at startup
  void	LoadConfig();		// #BUTTON #CONFIRM load configuration defaults from ~/.taconfig file (which is loaded automatically at startup)

  static void 	Error(const char* a, const char* b="", const char* c="",
		      const char* d="", const char* e="", const char* f="", 
		      const char* g="", const char* h="", const char* i="",
		      const char* j="", const char* k="", const char* l="");
  // displays error either in a window if iv_active or to stdout

  static int 	Choice(const char* text="Choice", const char* a="Ok", const char* b="",
		       const char* c="", const char* d="", const char* e="",
		       const char* f="", const char* g="", const char* h="",
		       const char* i="", const char* j="");
  // allows user to choose among different options in window if iv_active or stdin/out

  static void 	Busy();		// puts system in a 'busy' state 
  static void	DoneBusy();	// when no longer busy, call this function

  static void	Initialize();	// initialize type system, called in ta_TA.cc
  static void	DMem_Initialize(); // #IGNORE initialize distributed memory stuff

  static void	MallocInfo(ostream& strm);
  // generate malloc memory statistic information to given stream
  static void	ListAllTokens(ostream& strm);
  // generate a list and count of all types that keep tokens, with a count of tokens
  static int 	ReplaceAllPtrs(TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  // search through all tokens in all types and replace any pointer to object of type obj_typ address old_ptr with new_ptr
  static int 	ReplaceAllPtrsWithToken(TypeDef* obj_typ, void* old_ptr);
  // search through all tokens in all types and replace any pointer to old_ptr with first other token in same scope of same type

#ifndef __MAKETA__
  static void	Register_Cleanup(SIGNAL_PROC_FUN_ARG(fun));
  // register a cleanup process in response to all terminal signals
#endif

  static void	Decode_Signal(int err);	// printout translation of signal on cerr

  static void	CharToStrArray(String_PArray& sa, const char* ch);
  // convert space-delimeted character string to a string array
  static String	StrArrayToChar(const String_PArray& sa);
  // convert a string array to a space-delimeted character string 

  static void	SpaceLabel(String& lbl);
  // add spaces to a label in place of _'s and upper-lower transitions

  static String	LeadingZeros(int num, int len);
  // returns num converted to a string with leading zeros up to len

  static String	FormatValue(float val, int width, int precision);
  // format output of value according to width and precision

  static String	StringMaxLen(const String& str, int len);
  // returns string up to maximum length given (enforces string to be len or less in length)
  static String	StringEnforceLen(const String& str, int len);
  // returns string enforced to given length (spaces added to make length)

  // path manips
  static String	remove_name(String& path);
  static String	FindFileInclude(const char* fname);
  // try to find file fnm in one of the include paths -- returns complete path to file

  // parsing stuff
  static String	LexBuf;	// #HIDDEN a buffer, contains last thing read by read_ funs

  // return value is the next character in the stream
  // peek=true means that return value was not read, but was just peek'd

  static int	skip_white(istream& strm, bool peek = false);
  static int	skip_white_noeol(istream& strm, bool peek = false); // don't skip end-of-line
  static int	read_word(istream& strm, bool peek = false);
  static int	read_alnum(istream& strm, bool peek = false); 		// alpha-numeric
  static int    read_alnum_noeol(istream& strm, bool peek = false);
  static int	read_till_eol(istream& strm, bool peek = false); 
  static int	read_till_semi(istream& strm, bool peek = false);
  static int	read_till_lbracket(istream& strm, bool peek = false);
  static int	read_till_lb_or_semi(istream& strm, bool peek = false);
  static int	read_till_rbracket(istream& strm, bool peek = false);
  static int	read_till_rb_or_semi(istream& strm, bool peek = false);
  static int	read_till_quote(istream& strm, bool peek = false);      // dbl quote 
  static int	read_till_quote_semi(istream& strm, bool peek = false); // dbl quote followed by a semi
  static int	skip_past_err(istream& strm, bool peek = false);
  // skips to next rb or semi (robust)
  static int	skip_past_err_rb(istream& strm, bool peek = false);
  // skips to next rbracket (robust)

  // output functions 
  static ostream& indent(ostream& strm, int indent, int tsp=2);
  static ostream& fmt_sep(ostream& strm, const String& itm, int no, int indent,
			  int tsp=2);
  static ostream& fancy_list(ostream& strm, const String& itm, int no, int prln,
			     int tabs);
};

class taRefN {
  // #NO_TOKENS #NO_MEMBERS #NO_CSS reference counting base class
protected:
  uint 		refn;
public:
  static uint		RefN(taRefN* it)	{ return it->refn; }
  static void  		Ref(taRefN* it)	{ it->refn++; }
  static void  		Ref(taRefN& it)	{ it.refn++; }
  static void   	unRef(taRefN* it)	{ it->refn--; }
  static void   	Done(taRefN* it)	{ if(it->refn == 0) delete it; }
  static void		unRefDone(taRefN* it)	{ unRef(it); Done(it); }

  taRefN()		{ refn = 0; }
  virtual ~taRefN()	{ };
};


class EnumSpace : public taPtrList<EnumDef> {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS space of enums
protected:
  const String&	GetListName_() const		{ return name; }
  const String&	El_GetName_(void* it) const;
  TALPtr 	El_GetOwner_(void* it) const;
  void*		El_SetOwner_(void* it);
  void		El_SetIndex_(void* it, int i);
  
  void*		El_Ref_(void* it);
  void* 	El_unRef_(void* it);
  void		El_Done_(void* it);
  void*		El_MakeToken_(void* it);
  void*		El_Copy_(void* trg, void* src);

public:
  String 	name;		// of the space
  TypeDef*	owner;		// owner is a typedef

  void		Initialize()	{ owner = NULL; }
  EnumSpace()			{ Initialize(); }
  EnumSpace(const EnumSpace& cp) { Initialize(); Borrow(cp); }
  ~EnumSpace()                  { Reset(); }      

  void operator=(const EnumSpace& cp)	{ Borrow(cp); }

  // adding manages the values of the enum-values
  void			Add(EnumDef* it);
  virtual EnumDef*	Add(const char* nm, const char* dsc="", const char* op="",
			    int eno=0); 	 

  virtual EnumDef*	FindNo(int eno) const;
  // finds for a given enum_no

  virtual ostream&   	OutputType(ostream& strm, int indent = 1) const; 
};


class TokenSpace : public taPtrList<void> {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  const String&	GetListName_() const		{ return name; }
  const String&	El_GetName_(void* it) const;

public:
  static String tmp_el_name;	// for element names that need to be created

  String 	name;		// of the space
  TypeDef*	owner;
  bool		keep;		// true if tokens are kept
  int		sub_tokens;	// number of tokens in sub-types

  virtual void 	Initialize();
  TokenSpace()				{ Initialize(); }
  TokenSpace(const TokenSpace& cp)	{ Initialize(); Borrow(cp); }
  void operator=(const TokenSpace& cp)	{ Borrow(cp); }

  void 		List(ostream& strm=cout) const;
};


class MemberSpace : public taPtrList<MemberDef> {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS space of members
protected:
  const String&	GetListName_() const		{ return name; }
  const String& El_GetName_(void* it) const;
  TALPtr 	El_GetOwner_(void* it) const;
  void*		El_SetOwner_(void* it);
  void		El_SetIndex_(void* it, int i);
  
  void*		El_Ref_(void* it);
  void* 	El_unRef_(void* it);
  void		El_Done_(void* it);
  void*		El_MakeToken_(void* it);
  void*		El_Copy_(void* trg, void* src);

public:
  String 	name;		// of the space
  TypeDef*	owner;		// if a list in a typedef

  void		Initialize()		{ owner = NULL; }
  MemberSpace()				{ Initialize(); }
  MemberSpace(const MemberSpace& cp)	{ Initialize(); Borrow(cp); }
  ~MemberSpace()                        { Reset(); }

  void operator=(const MemberSpace& cp)	{ Borrow(cp); }

  virtual MemberDef*	FindCheck(const char* nm, void* base, void*& ptr) const;
  // breadth-first find pass for the recursive procedures

  int			Find(MemberDef* it) const { return taPtrList<MemberDef>::Find(it); }
  int			Find(const char* nm) const;
  // checks name and type name in 2 passes
  virtual int		FindTypeName(const char* nm) const;
  // find by name of type
  virtual MemberDef*	FindNameR(const char* nm) const;
  // recursive find of name (or type name)
  virtual MemberDef* 	FindNameAddr(const char* nm, void* base, void*& ptr) const;
  // find of name returning address of found member
  virtual MemberDef*	FindNameAddrR(const char* nm, void* base, void*& ptr) const;
  // recursive find of name returning address of found member

  virtual int		Find(TypeDef* it) const;
  virtual MemberDef*	FindType(TypeDef* it, int& idx=Idx) const;
  // find by type, inherits from
  virtual MemberDef*	FindTypeR(TypeDef* it) const; 
  // recursive find of type 
  virtual MemberDef* 	FindTypeAddr(TypeDef* it, void* base, void*& ptr) const;
  // find of type returning address of found member
  virtual MemberDef*	FindTypeAddrR(TypeDef* it, void* base, void*& ptr) const;
  // recursive find of type returning address of found member

  virtual int		FindDerives(TypeDef* it) const;
  virtual MemberDef*	FindTypeDerives(TypeDef* it,  int& idx=Idx) const;
  // find by type, derives from

  virtual int		Find(void* base, void* mbr) const;
  virtual MemberDef*	FindAddr(void* base, void* mbr, int& idx=Idx) const;
  // find by address given base of class and address of member
  virtual int		FindPtr(void* base, void* mbr) const;
  virtual MemberDef*	FindAddrPtr(void* base, void* mbr, int& idx=Idx) const;
  // find by address of a member that is a pointer given base and pointer addr

  virtual void		CopyFromSameType(void* trg_base, void* src_base);
  // copy all members from class of the same type as me
  virtual void		CopyOnlySameType(void* trg_base, void* src_base);
  // copy only those members in my type (no inherited ones)

  // IO
  virtual ostream&   	OutputType(ostream& strm, int indent = 0) const; 

  virtual ostream&   	Output(ostream& strm, void* base, int indent) const; 
  virtual ostream&   	OutputR(ostream& strm, void* base, int indent) const; 

  // for dump files
  virtual int   	Dump_Save(ostream& strm, void* base, void* par, int indent);
  virtual int   	Dump_SaveR(ostream& strm, void* base, void* par, int indent);
  virtual int   	Dump_Save_PathR(ostream& strm, void* base, void* par, int indent);

  virtual int   	Dump_Load(istream& strm, void* base, void* par,
				  const char* prv_read_nm = NULL, int prv_c = 0);
};

class MethodSpace : public taPtrList<MethodDef> {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS space of methods
protected:
  const String&	GetListName_() const		{ return name; }
  const String& El_GetName_(void* it) const;
  TALPtr 	El_GetOwner_(void* it) const;
  void*		El_SetOwner_(void* it);
  void		El_SetIndex_(void* it, int i);
  
  void*		El_Ref_(void* it);
  void* 	El_unRef_(void* it);
  void		El_Done_(void* it);
  void*		El_MakeToken_(void* it);
  void*		El_Copy_(void* trg, void* src);

public:
  String 	name;		// of the space
  TypeDef*	owner;		// if a list in a typedef

  void		Initialize()		{ owner = NULL; }
  MethodSpace()				{ Initialize(); }
  MethodSpace(const MethodSpace& cp)	{ Initialize(); Borrow(cp); }
  ~MethodSpace()                        { Reset(); }

  void operator=(const MethodSpace& cp)	{ Borrow(cp); }

  bool		AddUniqNameNew(MethodDef* it);

  virtual MethodDef*	FindAddr(ta_void_fun funa, int& idx) const;
  // find fun by addr, idx is actual index in method space
  virtual MethodDef*	FindOnListAddr(ta_void_fun funa, const String_PArray& lst, int& lidx) const;
  // find fun on given list by addr, lidx is 'index' of funs on same list
  virtual MethodDef*	FindOnListIdx(int lidx, const String_PArray& lst) const;
  // find fun on given list by index, as given by FindOnListAddr()

  // IO
  virtual ostream&   	OutputType(ostream& strm, int indent = 0) const;
};


class TypeSpace : public taPtrList<TypeDef> {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS space of types
protected:
  const String&	GetListName_() const 		{ return name; }
  const String&	El_GetName_(void* it) const;
  TALPtr 	El_GetOwner_(void* it) const;
  void*		El_SetOwner_(void* it);
  void		El_SetIndex_(void* it, int i);
  
  void*		El_Ref_(void* it);
  void* 	El_unRef_(void* it);
  void		El_Done_(void* it);
  void*		El_MakeToken_(void* it);
  void*		El_Copy_(void* trg, void* src);

public:
  String 	name;		// of the space
  TypeDef*	owner;		// if a list in a typedef

  void		Initialize()		{ owner = NULL; }

  TypeSpace()				{ Initialize(); }
  TypeSpace(const char* nm)		{ Initialize(); name = nm; }
  TypeSpace(const char* nm, int hash_sz) { Initialize(); name = nm; BuildHashTable(hash_sz); }
  TypeSpace(const TypeSpace& cp)	{ Initialize(); Borrow(cp); }
  ~TypeSpace()                          { Reset(); }

  void operator=(const TypeSpace& cp)	{ Borrow(cp); }

  virtual bool	ReplaceLinkAll(TypeDef* ol, TypeDef* nw);
  virtual bool 	ReplaceParents(const TypeSpace& ol, const TypeSpace& nw);
  // replace any parents on the old list with those on the new for all types
  virtual int 	ReplaceAllPtrs(TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  // search through all tokens in all types and replace any pointer to old_ptr of object type obj_typ with new_ptr

  virtual void	ListAllTokens(ostream& strm);
  // list count for all types that are keeping tokens
};

class EnumDef : public taRefN {
// ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS defines an enum member
protected:
  static String	tmp_label;	// for GetLabel() return value
public:
  EnumSpace*	owner;		// the owner of this one
  int		idx;		// the index number for this type

  String	name;
  String	desc;		// a description
  int		enum_no;	// number (value) of the enum

  String_PArray	opts;		// user-spec'd options (#xxx)
  String_PArray	lists;		// user-spec'd lists   (#LIST_xxx)

  void		Initialize();
  void		Copy(const EnumDef& cp);

  EnumDef()				{ Initialize(); }
  EnumDef(const char* nm)		{ Initialize(); name = nm; }
  EnumDef(const char* nm, const char* dsc, int eno, const char* op, const char* lis);
  EnumDef(const EnumDef& cp);
  virtual EnumDef*	Clone()		{ return new EnumDef(*this); }
  virtual EnumDef*	MakeToken()	{ return new EnumDef(); }

  virtual TypeDef* 	GetOwnerType() const
  { TypeDef* rval=NULL; if((owner) && (owner->owner)) rval=owner->owner; return rval; }

  virtual bool		HasOption(const char* op) const   // check if option is set
  { String fop = op; return (opts.Find(fop) >= 0) ? true : false; }
  virtual const String&	OptionAfter(const char* op) const;
  // return portion of option after given option header
  virtual bool		CheckList(const String_PArray& lst) const; 
  // check if enum has a list in common with given one
  virtual const String&	GetLabel() const;
  // checks for option of LABEL_xxx and returns it or name
};

class MemberDef : public taRefN {
// ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS defines a class member
protected:
  static String	tmp_label;	// for GetLabel() return value
public:
  MemberSpace*	owner;
  int           idx;		// index in owner memberspace

  TypeDef*	type;		// of this item
  String	name;		// member name
  String        desc;		// description
  String_PArray	inh_opts;	// inherited options ##xxx
  String_PArray	opts;		// user-spec'd options (#xxx)
  String_PArray	lists;		// user-spec'd lists   (#LIST_xxx)
  ta_memb_ptr	off;		// offset of member from owner type
  int		base_off;	// offset for base of owner
  bool		is_static;	// true if this member is static
  void*         addr;		// address of static member
  bool		fun_ptr;	// true if this is a pointer to a function
  taivMember*	iv;		// iv structure for edit representation

  void 		Initialize();
  void		Copy(const MemberDef& cp);
  MemberDef()				{ Initialize(); }
  MemberDef(const char* nm)		{ Initialize(); name = nm; }
  MemberDef(TypeDef* ty, const char* nm, const char* dsc, const char* op, const char* lis,
	    ta_memb_ptr mptr, bool is_stat = false, void* maddr=NULL, bool funp = false);
  MemberDef(const MemberDef& cp);
  virtual ~MemberDef();
  virtual MemberDef*	Clone()		{ return new MemberDef(*this); }
  virtual MemberDef*	MakeToken()	{ return new MemberDef(); }

  void*			GetOff(void* base) const
  { void* rval=addr; if(!is_static) rval=(void*)&((ta_memb_ptr_class*)((char*)base+base_off)->*off); 
    return rval; }

  virtual TypeDef* 	GetOwnerType() const		
  { TypeDef* rval=NULL; if((owner) && (owner->owner)) rval=owner->owner; return rval; }

  virtual bool		HasOption(const char* op) const   // check if option is set
  { String fop = op; return (opts.Find(fop) >= 0) ? true : false; }
  virtual const String&	OptionAfter(const char* op) const;
  // return portion of option after given option header
  virtual bool		CheckList(const String_PArray& lst) const; 
  // check if member has a list in common with given one
  virtual const String&	GetLabel() const;
  // checks for option of LABEL_xxx and returns it or name

  virtual bool		ShowMember(int show=-1) const;
  // decide whether to output or not based on options (READ_ONLY, HIDDEN, etc)

  virtual void		CopyFromSameType(void* trg_base, void* src_base);
  // copy all members from same type
  virtual void		CopyOnlySameType(void* trg_base, void* src_base);
  // copy only those members from same type (no inherited)

  virtual ostream&   	OutputType(ostream& strm, int indent = 1) const;

  virtual ostream& 	Output(ostream& strm, void* base, int indent) const;
  virtual ostream& 	OutputR(ostream& strm, void* base, int indent) const;

  // for dump files
  virtual bool		DumpMember(); 		// decide whether to dump or not
  virtual int		Dump_Save(ostream& strm, void* base, void* par, int indent);
  virtual int	 	Dump_SaveR(ostream& strm, void* base, void* par, int indent);
  virtual int	 	Dump_Save_PathR(ostream& strm, void* base, void* par, int indent);

  virtual int	 	Dump_Load(istream& strm, void* base, void* par);
};

class MethodDef : public taRefN {
// ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS defines a class method
protected:
  static String	tmp_label;	// for GetLabel() return value
public:
  MethodSpace*	owner;
  int           idx;		// index in owner memberspace

  TypeDef*	type;		// of the return value
  String	name;		// member name
  String        desc;		// description
  bool		is_static;	// true if this method is static
  ta_void_fun   addr;		// address (only for static or reg_fun functions) 
  String_PArray	inh_opts;	// inherited options ##xxx
  String_PArray	opts;		// user-spec'd options (#xxx)
  String_PArray	lists;		// user-spec'd lists   (#LIST_xxx)
  taivMethod*	iv;		// iv structure for edit representation

  int		fun_overld;	// number of times function is overloaded
  int		fun_argc;	// nofun, or # of parameters to the function
  int		fun_argd;	// indx for start of the default args (-1 if none)
  TypeSpace	arg_types;	// argument types
  String_PArray	arg_names;	// argument names
  String_PArray	arg_defs;	// argument default values
  String_PArray	arg_vals;	// argument values (previous)

  css_fun_stub_ptr stubp;	// css function stup pointer

  void		Initialize();
  void		Copy(const MethodDef& cp);
  MethodDef()				{ Initialize(); }
  MethodDef(const char* nm)		{ Initialize(); name = nm; }
  MethodDef(TypeDef* ty, const char* nm, const char* dsc, const char* op, const char* lis,
	    int fover, int farc, int fard, bool is_stat = false, ta_void_fun funa = NULL, 
	    css_fun_stub_ptr stb = NULL);
  MethodDef(const MethodDef& md);	// copy constructor
  virtual MethodDef*	Clone()		{ return new MethodDef(*this); }
  virtual MethodDef*	MakeToken()	{ return new MethodDef(); }

  virtual TypeDef* 	GetOwnerType() const
  { TypeDef* rval=NULL; if((owner) && (owner->owner)) rval=owner->owner; return rval; }

  virtual bool		HasOption(const char* op) const   // check if option is set
  { String fop = op; return (opts.Find(fop) >= 0) ? true : false; }
  virtual const String&	OptionAfter(const char* op) const;
  // return portion of option after given option header
  virtual bool		CheckList(const String_PArray& lst) const; 
  // check if method has a list in common with given one
  virtual const String&	GetLabel() const;
  // checks for option of LABEL_xxx and returns it or name

  virtual bool		CompareArgs(MethodDef* it) const;	// true if same, false if not

  virtual ostream&   	OutputType(ostream& strm, int indent = 1) const;

  virtual void		CallFun(void* base) const;
  // call the function, using IV dialog if need to get args
};

class taBase_List;

#define IF_ENUM_STRING(enm_var, enm_val) \
((enm_var == enm_val) ? #enm_val : "")


class TypeDef : public taRefN {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  static String	tmp_label;	// for GetLabel() function
public:
#if !defined(NO_TA_BASE) && defined(DMEM_COMPILE)
  int_PArray	dmem_type;
  virtual int 	GetDMemType(int share_set);
#endif
  TypeSpace	*owner;		// the owner of this one
  int		idx;		// the index number for this type

  String	name;
  String	desc;		// a description
  uint          size;		// size (in bytes) of item
  int		ptr;		// number of pointers
  bool 		ref;		// true if a reference variable
  bool		internal;	// true if an internal type (auto generated)
  bool		formal;		// true if a formal type (e.g. class, const, enum..)
  bool		pre_parsed;	// true if previously parsed by maketa

  String_PArray	inh_opts;	// inherited options (##xxx)
  String_PArray	opts;		// user-spec'd options (#xxx)
  String_PArray	lists;		// user-spec'd lists   (#LIST_xxx)

  TypeSpace	parents;	// type(s) this inherits from
  int_PArray	par_off;	// parent offsets
  TypeSpace	par_formal;	// formal parents (e.g. class, const, enum..)
  TypeSpace	par_cache;	// cache of certain parent types for speedup
  TypeSpace	children;	// inherited from this

  void**	instance;	// pointer to the instance ptr of this type
  TokenSpace	tokens;		// tokens of this type (if kept)

  taivType*	iv;		// single glyph representation of type	
  taivEdit*	ive;		// editing window rep. of type

  taBase_List*	defaults;	// default values registered for this type

  // the following only apply to enums or classes
  EnumSpace	enum_vals;	// if type is an enum, this is are the labels
  TypeSpace	sub_types;	// sub types scoped within class (incl enums)
  MemberSpace	members;	// member variables for class
  MethodSpace	methods;	// member functions (methods) for class
  String_PArray	ignore_meths;	// methods to be ignored
  TypeSpace	templ_pars;	// template parameters

  void 		Initialize();
  void		Copy(const TypeDef& cp);
  TypeDef()				{ Initialize(); }
  TypeDef(const char* nm) 		{ Initialize(); name = nm; }
  TypeDef(const char* nm, const char* dsc, const char* inop, const char* op, const char* lis, 
	  uint siz, void** inst, bool toks=false, int ptrs=0, bool refnc=false,
	  bool global_obj=false); // global_obj=true for global (non new'ed) typedef objs
  TypeDef(const char* nm, bool intrnl, int ptrs=0, bool refnc=false, bool forml=false,
	  bool global_obj=false); // global_obj=ture for global (non new'ed) typedef objs
  TypeDef(const TypeDef& td);
  virtual ~TypeDef();
  virtual TypeDef*	Clone()		{ return new TypeDef(*this); }
  virtual TypeDef*	MakeToken()	{ return new TypeDef(); }

  virtual void		DuplicateMDFrom(const TypeDef* old);
  // duplicates members, methods from given type
  virtual void		UpdateMDTypes(const TypeSpace& ol, const TypeSpace& nw);
  // updates pointers within members, methods to new types from old

  virtual bool		HasOption(const char* op) const // check if option is set
  { String fop = op; return (opts.Find(fop) >= 0) ? true : false; }
  virtual const String&	OptionAfter(const char* op) const;
  // return portion of option after given option header
  virtual bool		CheckList(const String_PArray& lst) const; 
  // check if have a list in common
  virtual const String&	GetLabel() const;
  // checks for option of LABEL_xxx and returns it or name

  virtual void*		GetInstance() const
  { void* rval=NULL; if(instance != NULL) rval = *instance; return rval; }
  TypeDef*		GetParent() const { return parents.SafeEl(0); }
  // gets (first) parent of this type (assumes no multiple inheritance)

  virtual TypeDef* 	GetNonPtrType() const;
  // gets base type (ptr=0) parent of this type
  virtual TypeDef* 	GetNonRefType() const;
  // gets base type (not ref) parent of this type
  virtual TypeDef* 	GetNonConstType() const;
  // gets base type (not const) parent of this type
  virtual TypeDef* 	GetTemplType() const;
  // gets base template parent of this type
  virtual TypeDef* 	GetTemplInstType() const;
  // gets base template instantiation parent of this type
  virtual String 	GetPtrString() const;
  // gets a string of pointer symbols (*) corresponding to the number ptrs
  virtual String	Get_C_Name() const;
  // returns the actual c-code name for this type

  // you inherit from yourself.  This ensures that you are a "base" class (ptr == 0)
  bool 			InheritsFrom(const char *nm) const
  { bool rval=0; if((ptr == 0) && ((name == nm) || (par_cache.Find(nm)>=0) || FindParent(nm))) rval=1; return rval; }
  bool			InheritsFrom(TypeDef* td) const	
  { bool rval=0; if((ptr == 0) && ((this == td) || (par_cache.Find(td)>=0) || FindParent(td))) rval=1; return rval; }
  bool 			InheritsFrom(const TypeDef& it) const { return InheritsFrom((TypeDef*)&it); }

  // pointers to a type, etc, can be Derives from a given type (looser than inherits)
  bool 			DerivesFrom(const char *nm) const	
  { bool rval=0; if((name == nm) || (par_cache.Find(nm)>=0) || FindParent(nm)) rval=1; return rval; }
  bool 			DerivesFrom(TypeDef* td) const	
  { bool rval=0; if((this == td) || (par_cache.Find(td)>=0) || FindParent(td)) rval=1; return rval; }
  bool 			DerivesFrom(const TypeDef& it) const { return DerivesFrom((TypeDef*)&it); }

  // inheritance from a formal class (e.g. const, static, class)
  bool			InheritsFormal(TypeDef* it) const
  { bool rval=0; if((ptr == 0) && (par_formal.Find(it)>=0)) rval=1; return rval; }
  bool 			InheritsFormal(const TypeDef& it) const { return InheritsFormal((TypeDef*)&it); }
  bool			DerivesFormal(TypeDef* it) const
  { bool rval=0; if(par_formal.Find(it)>=0) rval=1; return rval; }
  bool 			DerivesFormal(const TypeDef& it) const { return DerivesFormal((TypeDef*)&it); }

  virtual TypeDef*	AddParent(TypeDef* it, int p_off=0);
  // adds parent and inherits all the stuff from it

  // these are for construction
  virtual void	AddParents(TypeDef* p1=NULL, TypeDef* p2=NULL,
			   TypeDef* p3=NULL, TypeDef* p4=NULL,
			   TypeDef* p5=NULL, TypeDef* p6=NULL);
  virtual void	AddClassPar(TypeDef* p1=NULL, int p1_off=0, TypeDef* p2=NULL, int p2_off=0,
			    TypeDef* p3=NULL, int p3_off=0, TypeDef* p4=NULL, int p4_off=0,
			    TypeDef* p5=NULL, int p5_off=0, TypeDef* p6=NULL, int p6_off=0);
  virtual void	AddParFormal(TypeDef* p1=NULL, TypeDef* p2=NULL,
			     TypeDef* p3=NULL, TypeDef* p4=NULL,
			     TypeDef* p5=NULL, TypeDef* p6=NULL);
  virtual void	AddParCache(TypeDef* p1=NULL, TypeDef* p2=NULL,
			    TypeDef* p3=NULL, TypeDef* p4=NULL,
			    TypeDef* p5=NULL, TypeDef* p6=NULL);

  virtual void		ComputeMembBaseOff();
  // only for MI types, after adding parents, get new members & compute base_off
  virtual bool		IgnoreMeth(const String& nm) const;
  // check if given method should be ignored (also checks parents, etc)

  virtual bool	 	FindParent(const char* nm) const;	
  virtual bool 		FindParent(TypeDef* it) const;
  // recursively tries to find parent, returns true if successful
  virtual void*		GetParAddr(const char* par, void* base) const;
  virtual void*		GetParAddr(TypeDef* par, void* base) const;
  // return the given parent's address given the base address (par must be a parent!)
  virtual int		GetParOff(TypeDef* par, int boff=-1) const;
  // return the given parent's offset (par must be a parent!)
  virtual bool 		ReplaceParent(TypeDef* old_tp, TypeDef* new_tp);
  // replace parent of old_tp with parent of new_tp (recursive)
  virtual bool	 	FindChild(const char* nm) const;
  virtual bool 		FindChild(TypeDef* it) const;
  // recursively tries to  find child, returns true if successful

  TypeDef*		GetTemplParent() const;
  // returns immediate parent which is a template (or NULL if none found)
  virtual String	GetTemplName(const TypeSpace& inst_pars) const;
  virtual void		SetTemplType(TypeDef* templ_par, const TypeSpace& inst_pars);
  // set type of a template class

  virtual EnumDef*	FindEnum(const char* enum_nm) const;
  // find an enum and return its definition (or NULL if not found).  searches in enum_vals, then subtypes 
  virtual int		GetEnumVal(const char* enum_nm, String& enum_tp_nm = tmp_label) const;
  // find an enum and return its enum_no value, and set enum_tp_nm at the type name of the enum.  if not found, returns -1 and enum_tp_nm is empty
  virtual String	GetEnumString(const char* enum_tp_nm, int enum_val) const;
  // get the name of enum with given value in enum list of given type (e.g., enum defined within class)
  virtual int		FindTokenR(void* addr, TypeDef*& ptr) const;
  virtual int		FindTokenR(const char* nm, TypeDef*& ptr) const;
  // recursive search for token among children

  // for token management
  virtual void 		Register(void* it);	 
  virtual void 		unRegister(void* it);

  virtual String	GetValStr(void* base, void* par=NULL,
				  MemberDef* memb_def = NULL) const;
  // get a string representation of value
  virtual void		SetValStr(const char* val, void* base, void* par=NULL,
				  MemberDef* memb_def = NULL); 
  // set the value from a string representation

  virtual void		CopyFromSameType(void* trg_base, void* src_base,
					 MemberDef* memb_def = NULL);
  // copy all mmbers from same type
  virtual void		CopyOnlySameType(void* trg_base, void* src_base,
					 MemberDef* memb_def = NULL);
  // copy only those members from same type (no inherited)
  virtual void		MemberCopyFrom(int memb_no, void* trg_base, void* src_base);
  // copy a particular member from same type
  virtual int		ReplaceAllPtrs(TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  // replace any member pointers that point to old_ptr with new_ptr (returns no. changed)
  virtual int		ReplaceAllPtrsThis(void* base, TypeDef* obj_typ, void* old_ptr, void* new_ptr);
  // replace any member pointers that point to old_ptr with new_ptr (returns no. changed)

  // IO
  virtual ostream& 	Output(ostream& strm, void* base, int indent=0) const;
  // output value information for display purposes
  virtual ostream& 	OutputR(ostream& strm, void* base, int indent=0) const;
  // output value information for display purposes, recursive

  virtual ostream&  	OutputType(ostream& strm, int indent = 0) const;
  // output type information only 
  virtual ostream&  	OutputInherit(ostream& strm) const;
  virtual ostream&  	OutputInherit_impl(ostream& strm) const;

  // saving and loading of type instances to/from streams
  virtual int	Dump_Save(ostream& strm, void* base, void* par=NULL, int indent=0);
  // called by the user to save an object
  virtual int 	Dump_Save_impl(ostream& strm, void* base, void* par=NULL, int indent=0);
  virtual int 	Dump_Save_inline(ostream& strm, void* base, void* par=NULL, int indent=0);
  // for saving objects as members of other objects
  virtual int 	Dump_Save_Path(ostream& strm, void* base, void* par=NULL, int indent=0);
  // save the path of the object
  virtual int 	Dump_Save_Value(ostream& strm, void* base, void* par=NULL, int indent=0);
  // save the value of this object (i.e. the members)
  virtual int	Dump_SaveR(ostream& strm, void* base, void* par=NULL, int indent=0);
  // if there are sub-elements (i.e. groups), save them (return false if not)
  virtual int 	Dump_Save_PathR(ostream& strm, void* base, void* par=NULL, int indent=0);
  // if there are sub-elements, save the path to them (return false if not)
  
  virtual int	Dump_Load(istream& strm, void* base, void* par=NULL);
  // called by the user to load an object
  virtual int	Dump_Load_impl(istream& strm, void* base, void* par=NULL,
			       const char* typnm=NULL);
  virtual int	Dump_Load_Path(istream& strm, void*& base, void* par, TypeDef*& td,
			       String& path, const char* typnm=NULL);
  // loads a path (typename path) and fills in the base and td of object (false if err)
  virtual int	Dump_Load_Path_impl(istream& strm, void*& base, void* par, String path);
  virtual int	Dump_Load_Value(istream& strm, void* base, void* par=NULL);
  // loads the actual member values of the object (false if error)

  TypeDef*	FindTypeWithMember(const char* nm, MemberDef** md); // returns the type or child type with memberdef md
};

class MTRnd {
  // #IGNORE A container for the Mersenne Twister (MT19937) random number generator by Makoto Matsumoto and Takuji Nishimura
public:
#ifndef __MAKETA__
  static const int N = 624;
  static const int M = 397;
  static const ulong MATRIX_A = 0x9908b0dfUL;   /* constant vector a */
  static const ulong UPPER_MASK = 0x80000000UL; /* most significant w-r bits */
  static const ulong LOWER_MASK = 0x7fffffffUL; /* least significant r bits */

  static ulong mt[N];
  static int mti;				/* mti==N+1 means mt[N] is not initialized */
#endif

  static void seed(ulong s); 	// seed the generator with given seed value
  static ulong seed_time_pid();	// seed the generator with a random seed produced from the time and the process id
  static void seed_array(ulong init_key[], int key_length); // seed with given initial key

  static ulong genrand_int32();	// generates a random number on [0,0xffffffff]-interval
  static long  genrand_int31();	// generates a random number on [0,0x7fffffff]-interval
  static double genrand_real1(); // generates a random number on [0,1]-real-interval
  static double genrand_real2(); // generates a random number on [0,1)-real-interval
  static double genrand_real3(); // generates a random number on (0,1)-real-interval
  static double genrand_res53(); // generates a random number on [0,1) with 53-bit resolution
};

#endif //typea_h

