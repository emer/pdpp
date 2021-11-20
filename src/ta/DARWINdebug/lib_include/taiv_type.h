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

// taiv_type.h: easily extendable interface support for all types 

#ifndef taiv_type_h
#define taiv_type_h 1

#include <stdlib.h>
#include <ta/taiv_data.h>

// pre-declare classes:
class taivType;
class taivEdit;
class taivMember;
class taivMethod;
class taivArgType;
// _

//////////////////////////
// 	taivType 	//
//////////////////////////

class taivType {
  // ##INSTANCE ##NO_TOKENS ##NO_CSS ##NO_MEMBERS graphically represents a type 
public:
  TypeDef*	typ;		// typedef of base object
  int		bid;		// its bid
  taivType*    	sub_types;	// lower bid type (which has lower one, and so on)
  String	orig_val;	// original value of the item
  bool		no_setpointer;
  // don't use SetPointer for taBase pointers (ie., for css or other secondary pointers)

  virtual int		BidForType(TypeDef*) { return 1; }
  // bid for (appropriateness) for given type
  virtual void 		AddIv(TypeDef* td);	// add an iv to a type

  virtual taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  // get taivData rep of type

  virtual void		GetImage(taivData* dat, void* base, ivWindow* win);
  // generate the glyph representation of the data
  virtual void		GetValue(taivData* dat, void* base);
  // get the value from the representation

  virtual taivType*	TypInst(TypeDef* td)	{ return new taivType(td); }
  void 	Initialize()	{}; // this is for subclass-constr
  void 	Destroy()	{}; // for subclass-destructor
  taivType(TypeDef* tp);
  taivType();
  virtual ~taivType();
  virtual TypeDef*	GetTypeDef() const	{ return &TA_taivType; }
};

#define TAIV_TYPE_INSTANCE(x,y) x(TypeDef* tp)			\
: y(tp)		{ Initialize(); } 				\
x()		{ Initialize(); } 				\
~x()		{ Destroy(); } 					\
taivType* 	TypInst(TypeDef* td)	 			\
{ return (taivType*) new x(td); }				\
TypeDef*	GetTypeDef() const { return &TA_##x; }


class taivType_List : public taPtrList<taivType> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (taivType*)it; }

public:
  ~taivType_List()              { Reset(); }
};


//////////////////////////////
//       taivTypes         //
//////////////////////////////

class taivIntType : public taivType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivIntType, taivType);
};



class taivEnumType : public taivType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivEnumType, taivType);
};

class taivBoolType : public taivType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivBoolType, taivType);
};

class taivClassType : public taivType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivClassType, taivType);
};

class taivStringType : public taivClassType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);
  TAIV_TYPE_INSTANCE(taivStringType, taivClassType);
};


class taivTokenPtrType : public taivType {
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void 		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivTokenPtrType, taivType);
};

class taivTypePtr : public taivType {
  // typedef pointer
public:
  TypeDef*	orig_typ;

  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  void		Initialize();
  TAIV_TYPE_INSTANCE(taivTypePtr, taivType);
};

class taivFilePtrType : public taivType {// ptr to taiv_getFiles
public:
  int		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow *win);
  void  	GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(taivFilePtrType, taivType);
};


//////////////////////////
//	taivEdit	//
//////////////////////////

// offloads lots of the work to the dialog...

class taivEdit : public taivType {
public:
  virtual int Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgcol = NULL);
  virtual int BidForType(TypeDef*) { return 0;}
  virtual int BidForEdit(TypeDef*) { return 1;}

  virtual void AddIve(TypeDef* td); // add an edit to a type

  TAIV_TYPE_INSTANCE(taivEdit, taivType);
};


#define TAIV_EDIT_INSTANCE(x,y) x(TypeDef* tp)		\
: y(tp) 	{ Initialize();}			\
x()             { Initialize();}			\
~x()            { Destroy(); }				\
taivType* 	TypInst(TypeDef* td)			\
{ return (taivType*) new x(td); }			\
TypeDef*	GetTypeDef() const { return &TA_##x; }


//////////////////////////////
//       taivEdits         //
//////////////////////////////

class taivDefaultEdit : public taivEdit {
public:
  int	Edit(void* base, ivWindow* win=NULL, bool wait=false, bool readonly=false, const ivColor* bgclr = NULL);
  int	BidForEdit(TypeDef*) { return 0; }	
  TAIV_EDIT_INSTANCE(taivDefaultEdit, taivEdit);
};


//////////////////////////
// 	taivMember	//
//////////////////////////

class taivMember : public taivType {
public:
  MemberDef*	mbr;

  int		BidForType(TypeDef*) 			{ return 0; }
  // none of the member specific ones should apply types
  virtual int	BidForMember(MemberDef*, TypeDef*) 	{ return 1; }
  // bid for (appropriateness) for given type of member (and members owner type)

  // default member action is to pass thru to the type

  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base)
  { mbr->type->iv->GetValue(dat, mbr->GetOff(base)); }
  virtual void	GetMbrValue(taivData* dat, void* base, bool& first_diff);
  // this is the one to call to get a member value 

  // script-generation code
  virtual void	GetOrigVal(taivData* dat, void* base);
  // gets original value and adds it to the orig_vals list
  virtual void	StartScript(void* base);
  static  void	EndScript(void* base);
  // calling function has to use this function to end script if neccesary
  virtual void	CmpOrigVal(taivData* dat, void* base, bool& first_diff);
  // compares to original value and generates script code to change it 

  taivMember(MemberDef* mb, TypeDef* td) : taivType(td)	{ mbr = mb; }
  taivMember()							{ mbr = NULL; }
  ~taivMember()						{ };

  virtual void 		AddIvm(MemberDef* md);	// add an iv to a member
  virtual taivMember* 	MembInst(MemberDef* md, TypeDef* td)
  { return new taivMember(md, td);}
  TypeDef*	GetTypeDef() const { return &TA_taivMember; }
};

#define TAIV_MEMBER_INSTANCE(x,y) x(MemberDef* md, TypeDef* td) 	\
: y(md,td) 	{ Initialize(); }					\
x()             { Initialize(); }        				\
~x()            { Destroy(); }						\
taivMember* 	MembInst(MemberDef* md, TypeDef* td)			\
{ return (taivMember*) new x(md, td); }					\
TypeDef*	GetTypeDef() const { return &TA_##x; }


//////////////////////////////
//       taivMembers       //
//////////////////////////////

// these have BidforMember() functions which may depend on the opts
// of a member

class taivROMember : public taivMember {
  // read-only member
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);
       
  TAIV_MEMBER_INSTANCE(taivROMember, taivMember);
};


class taivTokenPtrMember : public taivMember {
  // for taBase pointer members (allows scoping by owner obj)
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(taivTokenPtrMember, taivMember);
};

class taivTypePtrMember : public taivMember {
  // typedef ptrs that have member-comment directives
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(taivTypePtrMember, taivMember);
};

class taivSubTokenPtrMember : public taivMember {
  // a token ptr that points to sub-objects of current object
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(taivSubTokenPtrMember, taivMember);
};

class taivMemberDefPtrMember : public taivMember {
  // pointer to a member-def
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);
  
  TAIV_MEMBER_INSTANCE(taivMemberDefPtrMember, taivMember);
};

class taivFunPtrMember : public taivMember {
  // pointer to a function
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(taivFunPtrMember, taivMember);
};

class taivCondEditMember : public taivMember {
  // conditional editing member
public:
  taivMember*	ro_iv;		// member for read-only (no editing)

  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  taivCondEditMember(MemberDef* md, TypeDef* td);
  taivCondEditMember()             { Initialize(); }
  ~taivCondEditMember()            { Destroy(); }
  taivMember* 	MembInst(MemberDef* md, TypeDef* td)
  { return (taivMember*) new taivCondEditMember(md, td); }
  TypeDef*	GetTypeDef() const { return &TA_taivCondEditMember; }
  void		Initialize();
  void		Destroy();
};

class TypeDefault;

class taivTDefaultMember : public taivMember {
// special for the TypeDefault member (add the "active" box)
public:
  TypeDefault*	tpdflt;

  virtual int	BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff); 
  void		Initialize()	{ tpdflt = NULL; }
  void		Destroy()	{ sub_types = NULL; } // prevent from being destroyed

  TAIV_MEMBER_INSTANCE(taivTDefaultMember, taivMember);
};

// Special edit menu for the TDefault's token member
class taivDefaultToken : public taivTokenPtrMember {
public:
  TypeDefault*	tpdflt;

  virtual int	BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff); 
  void		Initialize()	{ tpdflt = NULL; }

  TAIV_MEMBER_INSTANCE(taivDefaultToken, taivTokenPtrMember);
};


//////////////////////////
// 	taivMethod	//
//////////////////////////

// the default method bids 0, so it doesn't get created, and is more of a virtual 
// type

class taivMethod : public taivType {
public:
  MethodDef*	meth;

  int		BidForType(TypeDef*) 			{ return 0; }
  // none of the method specific ones should apply to types
  virtual int	BidForMethod(MethodDef*, TypeDef*) 	{ return 0; }
  // bid for (appropriateness) for given type of method (default is not at all approp.)

  virtual taivMethMenu* GetMethodRep(void*, taivDialog* =NULL, taivData* =NULL) { return NULL; }
  void		GetImage(taivData*, void*, ivWindow*)	{ };
  void		GetValue(taivData*, void*)             { };

  taivMethod(MethodDef* mb, TypeDef* td) : taivType(td)	{ meth = mb; }
  taivMethod()							{ meth = NULL; }
  ~taivMethod()						{ };

  virtual void 		AddIvm(MethodDef* md);	// add an iv to a member

  virtual taivMethod* 	MethInst(MethodDef* md, TypeDef* td)
  { return new taivMethod(md,td);}
  TypeDef*	GetTypeDef() const { return &TA_taivMethod; }
};

#define TAIV_METHOD_INSTANCE(x,y) x(MethodDef* md,TypeDef*td)		\
: y(md, td) 	{ Initialize(); }					\
x()             { Initialize(); } 					\
~x()            { Destroy(); }						\
taivMethod* 	MethInst(MethodDef* md,TypeDef*td) 			\
{ return (taivMethod*) new x(md,td); }					\
TypeDef*	GetTypeDef() const { return &TA_##x; }



//////////////////////////////
//       taivMethods        //
//////////////////////////////

class taivButtonMethod : public taivMethod {
public:
  int			BidForMethod(MethodDef* md, TypeDef* td);
  taivMethMenu*	GetMethodRep(void* base, taivDialog* dlg=NULL, taivData* par=NULL);
  
  TAIV_METHOD_INSTANCE(taivButtonMethod, taivMethod);
};
  
class taivMenuMethod : public taivMethod {
public:
  int 			BidForMethod(MethodDef* md, TypeDef* td);
  taivMethMenu*	GetMethodRep(void* base, taivDialog* dlg=NULL, taivData* par=NULL);
  
  TAIV_METHOD_INSTANCE(taivMenuMethod, taivMethod);
};

class taivMenuButtonMethod : public taivMethod {
public:
  int			BidForMethod(MethodDef* md, TypeDef* td);
  taivMethMenu*	GetMethodRep(void* base, taivDialog* dlg=NULL, taivData* par=NULL);
  
  TAIV_METHOD_INSTANCE(taivMenuButtonMethod, taivMethod);
};
  
  
//////////////////////////
// 	taivArgType	//
//////////////////////////

class taivArgType : public taivType {
  // unlike taivTypes, these are created and destroyed each time
  // thus, they cache their values
public:
  MethodDef*	meth;		// method that has the args
  TypeDef*	arg_typ;	// which arg typedef this one is
  bool		err_flag;	// true if one of the args was improperly set

  taivType*	use_iv;		// alternate iv type to use
  void*		arg_base;	// base value is computed for typedef
  cssEl*	arg_val;	// argument value (as a css element)
  taBase*	obj_inst;	// instance of taBase object for ptr=0 args

  int		BidForType(TypeDef*) 			{ return 0; }
  // none of the argtype specific ones should apply to types
  virtual int	BidForArgType(int, TypeDef*, MethodDef*, TypeDef*) 	{ return 1; }
  // bid for (appropriateness) for given type of method and argument type

  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  virtual cssEl* GetElFromArg(const char* arg_nm, void* base);
  // this is responsible for setting arg_base and arg_val (base is parent base)

  // base passed here is of the parent object(!)
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  taivArgType(TypeDef* argt, MethodDef* mb, TypeDef* td);
  taivArgType();
  ~taivArgType();

  void 	Initialize()	{};
  void 	Destroy()	{};
  virtual taivArgType*  ArgTypeInst(TypeDef* argt, MethodDef* md, TypeDef* td)
  { return new taivArgType(argt,md,td);}
  TypeDef*	GetTypeDef() const { return &TA_taivArgType; }
};

#define TAIV_ARGTYPE_INSTANCE(x,y) x(TypeDef* argt, MethodDef* md, TypeDef*td) 	\
: y(argt,md,td) { Initialize(); } 						\
x()            	{ Initialize(); } 						\
~x()           	{ Destroy(); }							\
taivArgType* 	ArgTypeInst(TypeDef* argt, MethodDef* md, TypeDef*td) 		\
{ return (taivArgType*) new x(argt,md,td); }					\
TypeDef*	GetTypeDef() const { return &TA_##x; }



//////////////////////////////
//       taivArgTypes	    //
//////////////////////////////

class taivStreamArgType : public taivArgType {
  // for ios derived args (uses a file-requestor)
public:
  taivGetFile*	gf;
  String	filter; 	// our filter if any
  String	old_filter; 	// previous filter
  bool		old_compress;	// previous value of compress flag
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);
  virtual void	GetValueFromGF(); // actually get the value from the getfile
  
  void Initialize();
  void Destroy();
  TAIV_ARGTYPE_INSTANCE(taivStreamArgType, taivArgType);
};
  
class taivBoolArgType : public taivArgType {
  // for bool int types
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);

  TAIV_ARGTYPE_INSTANCE(taivBoolArgType, taivArgType);
};

class taivTokenPtrArgType : public taivArgType {
  // for pointers to tokens (uses appropriate scoping)
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);

  void 	Initialize()	{};
  void 	Destroy()	{};
  TAIV_ARGTYPE_INSTANCE(taivTokenPtrArgType, taivArgType);
};
  
class taivTypePtrArgType : public taivArgType {
  // for typedef ptr types
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);

  void 	Initialize()	{};
  void 	Destroy()	{};
  TAIV_ARGTYPE_INSTANCE(taivTypePtrArgType, taivArgType);
};
  
class taivMemberPtrArgType : public taivArgType {
  // for memberdef ptr types
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);

  void 	Initialize()	{};
  void 	Destroy()	{};
  TAIV_ARGTYPE_INSTANCE(taivMemberPtrArgType, taivArgType);
};

class taivMethodPtrArgType : public taivArgType {
  // for methoddef ptr types
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);

  void 	Initialize()	{};
  void 	Destroy()	{};
  TAIV_ARGTYPE_INSTANCE(taivMethodPtrArgType, taivArgType);
};

#endif // taiv_type_h

