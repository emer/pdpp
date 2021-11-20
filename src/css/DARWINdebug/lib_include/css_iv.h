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

// css_iv.h: InterViews-css interface

#ifndef cssivh
#define cssivh 1

#include <ta/taiv_type.h>
#include <ta/taiv_dialog.h>

#include <ta/enter_iv.h>
#include <Dispatch/dispatcher.h>
#include <Dispatch/iohandler.h>
#include <InterViews/session.h>
#include <ta/leave_iv.h>

class cssClassInst;
class cssMbrScriptFun;
class cssEnumType;
class cssProgSpace;
class cssClassType;
class cssClassInst;

class cssivSession : public IOHandler { // this handles events for css/iv
public:
  static bool	stdin_event;	// flag for event being detected
  static bool	block_stdin;	// block the processing of stdin input
  static bool	block_in_event; // filter out input events (iv) from processing
  static bool	done_busy;	// true if done being busy after all events proc'd
  static bool	in_session;	// if we are in this session or not 
  static cssivSession* instance; // instance of session
  static taivEditDialog_List	active_edits; // active css edit dialogs

  static void	CancelProgEdits(cssProgSpace* prsp);
  // cancel any active edit dialogs for given prog space
  static void	CancelClassEdits(cssClassType* cltyp);
  // cancel any active edit dialogs for given class type
  static void	CancelObjEdits(cssClassInst* clobj);
  // cancel any active edit dialogs for given class object
  static void	RaiseObjEdits();
  // bring all object edit dialogs to the front (e.g., after loading)

  static void 	Init();		// call this to initialize the handler
  static void 	Stop();		// tell the processing loop to stop
  static void	Quit();		// tell the iv session to quit
  
  static void 	ProcEvent(ivSession* ses); // actually process current event
  static int	Run();		// process events, etc.
  static int	RunPending();	// run any pending events (don't wait..)

  static int	(*WaitProc)();
  // set this to a work process for idle time processing.  return true if something
  // happend that might create new window system events..

  virtual int inputReady(int fd); // this is called when something happens
  virtual int exceptionRaised(int fd);
  virtual int outputReady(int fd);

  cssivSession();
};

//////////////////////////////
//   taivData for css       //
//////////////////////////////

class cssivPolyData : public taivData {
  // supports INLINE members for css
public:
  cssClassInst*	obj;
  taivDataList 	data_el;
  taivType_List type_el;	// type elements (not stored on classes, so kept here)
  ivGlyph*	rep;
  int		show;

  cssivPolyData(cssClassInst* ob, taivDialog* dlg=NULL, taivData* par=NULL);
  ~cssivPolyData();

  ivGlyph*	GetRep()		{ return rep; }
  ivGlyph*	GetLook();

  virtual void	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL);

  virtual void  GetImage(void* base, ivWindow* win);
  virtual void	GetValue(void* base);
};  


//////////////////////////////
//   taivTypes for css      //
//////////////////////////////

class cssivType : public taivType {
  // css types call their corresponding typ for all of the functions
public:
  cssEl*	orig_obj;	// original object that this type is based on (ie., member)
  void*		cur_base;	// thse are used only once, so base is kept here
  taivType*	use_iv;		// if non-null, use this iv instead of typ->iv


  int		BidForType(TypeDef*)	{ return 0; } // don't do any real types!
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetImage(taivData* dat, ivWindow* win)
  { GetImage(dat, cur_base, win); }

  void		GetValue(taivData* dat, void* base);
  void		GetValue(taivData* dat)  { GetValue(dat, cur_base); }

  cssivType(cssEl* orgo, TypeDef* tp, void* bs, bool use_ptr_type = false);
  // if use_ptr_type is true, type given is the base type, make it a pointer
  ~cssivType();
};

class cssivROType : public cssivType {
  // a css read-only type
public:
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetImage(taivData* dat, ivWindow* win)
  { GetImage(dat, cur_base, win); }

  void		GetValue(taivData* dat, void* base);
  void		GetValue(taivData* dat)  { GetValue(dat, cur_base); }

  cssivROType(cssEl* orgo, TypeDef* tp, void* bs, bool use_ptr_type = false);
};

class cssivEnumType : public cssivType {
  // a css enum type
public:
  cssEnumType*	enum_type;

  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetImage(taivData* dat, ivWindow* win)
  { GetImage(dat, cur_base, win); }

  void		GetValue(taivData* dat, void* base);
  void		GetValue(taivData* dat)  { GetValue(dat, cur_base); }

  cssivEnumType(cssEl* orgo, cssEnumType* enm_typ, void* bs);
};

class cssivClassType : public cssivType {
  // a css class type
public:

  taivData*     GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  void          GetImage(taivData* dat, void* base, ivWindow* win);
  void          GetImage(taivData* dat, ivWindow* win)
  { GetImage(dat, cur_base, win); }

  void          GetValue(taivData* dat, void* base);
  void          GetValue(taivData* dat)  { GetValue(dat, cur_base); }

  void		CallEdit();     // invoke an edit dialog
   
  cssivClassType(cssEl* orgo, void* bs);
};

class cssivArrayType : public cssivType {
  // a css array type
public:

  taivData*     GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);

  void          GetImage(taivData* dat, void* base, ivWindow* win);
  void          GetImage(taivData* dat, ivWindow* win)
  { GetImage(dat, cur_base, win); }

  void          GetValue(taivData* dat, void* base);
  void          GetValue(taivData* dat)  { GetValue(dat, cur_base); }

  void		CallEdit();     // invoke an edit dialog
   
  cssivArrayType(cssEl* orgo, void* bs);
};


class cssivMethMenu : public taivMethMenu {
  // css member functions of css classes
public:
  cssClassInst*		obj;		// parent object
  cssProgSpace*		top;		// top-level progspace where edit was called
  cssMbrScriptFun*	css_fun; 	// the function itself
  cssClassInst*		arg_obj; 	// argument object (if necc)
  ivGlyph*		rep;		// representation if a button

  cssivMethMenu(cssClassInst* ob, cssProgSpace* tp, cssMbrScriptFun* cfn,
		taivDialog* dlg=NULL, taivData* par=NULL);
  ~cssivMethMenu();

  void		CallFun();	// call the function..
  void		ShowReturnVal(cssEl* rval);
  void		ApplyBefore();	// apply changes before performing menu call
  void		UpdateAfter();	// update display after performing menu call
	
  void		GenerateScript(); // output script code equivalent if recording

  void		AddToMenu(taivMenu* mnu);
  ivGlyph*	GetLook();
};

class cssivEditDialog : public taivEditDialog {
  // edit dialog for editing css classes
public:
  cssClassInst*	obj;		// class object to edit
  cssProgSpace*	top;		// top-level progspace where edit was called
  taivType_List type_el;	// type elements (not stored on classes, so kept here)

  cssivEditDialog(cssClassInst* ob, cssProgSpace* tp=NULL);
  cssivEditDialog()		{ };
  ~cssivEditDialog();

  void		Constr_Strings(const char* prompt="", const char* win_title="");
  void		GetImage(); 
  void		GetValue();
  int		Edit();

  void		Constr_Labels(); // construct the labels list
  void		Constr_Data(); // construct the static elements of the dialog
  void		Constr_Methods(); // construct the methods (buttons and menus)
  void		Constr_Body();

  ivGlyph* 	GetNameRep(MemberDef* md) { return taivEditDialog::GetNameRep(md); }
  ivGlyph* 	GetNameRep(int idx, cssEl* md);

  void		GetMenuRep(MethodDef* md) { taivEditDialog::GetMenuRep(md); }
  void		GetMenuRep(cssMbrScriptFun* md);

  static cssivType*	GetTypeFromEl(cssEl* el, bool read_only);
  // this decodes cssEl types into cssivTypes, which are then used to render edit
};

#endif // cssivh
