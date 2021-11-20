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

// taiv_dialog.h: InterViews-based dialogs

#ifndef taiv_dialog_h
#define taiv_dialog_h 1

#include <ta/typea.h>
#include <ta/taiv_data.h>

#include <ta/enter_iv.h>
#include <InterViews/dialog.h>
#include <IV-look/button.h> //for script button
#include <InterViews/deck.h> // for hilight Button
#include <ta/leave_iv.h>

class	ivTopLevelWindow;		// #IGNORE
class	ivFont;				// #IGNORE

//////////////////////////
// 	Script Button 	//
//////////////////////////

// this button generates script when pressed
class ScriptButton : public ivButton{ // #IGNORE
public:
  String script;
  ScriptButton(char* name, ivAction *a, char* srp);
  void release(const ivEvent&);
};


//////////////////////////
// 	HiLight Button  //
//////////////////////////

class HiLightButton : public ivDeck { //  #IGNORE
public:
  bool	Set(bool hilight);
  bool	HiLight();
  bool  UnHiLight();
  HiLightButton(char* name, ivAction* action,
		char* srp=NULL, bool unhilight_enable=true);
};

//////////////////////////
// 	taivDialog	//
//////////////////////////

class NoBlockDialog : public ivDialog { // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  taivDialog* 		owner;
  osboolean 		was_accepted;
  ivTopLevelWindow* 	win;
  int			last_focus; // -1 = prev_focus, +1 = next_focus

  osboolean 	post_for_aligned(ivWindow* w, float x_align, float y_align);
  osboolean 	post_at_aligned(ivCoord x, ivCoord y, float x_align, float y_align);
  void		dismiss(osboolean accept);
  virtual void	raise();	// raise the window

  void		keystroke(const ivEvent& e);
  void		prev_focus();
  void		next_focus();

  virtual bool	special_key(const ivEvent& e);
  // processes special keystrokes, returns true if processed, otherwise local
  // handler should do it.  this should be called by any key-processing inputhandler

  NoBlockDialog(taivDialog* dlg, ivGlyph* g, ivStyle* s);
  ~NoBlockDialog();
};

class taivDialog {		// ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  static ivWidgetKit* 	wkit;
  static ivDialogKit* 	dkit;
  static ivLayoutKit* 	layout;
  static ivStyle* 	style;

  enum Dlg_State {
    EXISTS,
    CONSTRUCTED,
    ACTIVE,
    ACCEPTED,
    CANCELED,
    SHOW_CHANGED		// what to show was changed, reconstruct!
  };

  Dlg_State	state;		// indicates state of construction of dialog
  bool		waiting;	// true if dialog is of the waiting variety
  bool		no_can_but;	// no cancel button
  bool		no_ok_but;	// no ok button
  bool          cancel_only;	// apply and ok have no function, must press cancel
  int		mouse_button;	// the mouse button that pressed ok
  bool		modified;	// true if contents were modified
  bool		prev_modified;	// previous modified value
  bool		no_revert_hilight; // do not highlight the revert button
  
  ivGlyph*	prompt;		// these stack in a vbox in this order..
  ivGlyph* 	box;		// this has all user data
  ivButton* 	okbut;
  ivButton* 	canbut;
  HiLightButton* apply_but;	// only use for dialogs that wait around
  HiLightButton* revert_but;
  ivGlyph*	but_box;	// box of buttons on the bottom of the dialog
  ivPatch*	but_patch;

  ivPolyGlyph* 	body_box;	// box for the body
  ivPatch*	body;		// this is the entire glyph for the dialog
  NoBlockDialog* dialog;	// the actual dialog
  ivWindow*	cur_win;	// current window for dialog
  const ivColor* bg_color;	// background color of dialog
  const ivColor* bg_color_dark;	// background color of dialog, darkened

  String	prompt_str;	// string that goes inside as a prompt or label
  String	win_str;	// string that goes on the window frame

  TypeDef*	typ;		// type of object (if relevant)
  void*		cur_base;	// current base pointer of object (if relevant)

  taivDialog();
  virtual ~taivDialog();

  virtual void	CloseWindow();	// close edit dialog window and free current data
  
  // these functions are called in-order by Constr
  virtual void	Constr_Strings(const char* prompt="", const char* win_title="");
  virtual void	Constr_Buttons();
  virtual void	Constr_WinName(); 
  virtual void	Constr_Prompt(); 
  virtual void	Constr_Box();
  virtual void	Constr_BodyBox();
  virtual void	Constr_Body();	
  virtual void	Constr_Dialog();
  virtual void	Constr_Final();
  virtual void  Constr(ivWindow* win=NULL, bool wait=false, const char* prompt="",
		       const char* win_title="", bool no_cancel=false, const ivColor* bgcol = NULL);
  virtual int 	Edit();
  // returns true if ok, false if cancel (only if waiting) assumes already constr

  virtual void 	Ok();
  virtual void 	Cancel();
  virtual void	GetImage()	{ };
  virtual void	GetValue()	{ };
  virtual void	Apply();
  virtual void	Revert();
  virtual void	Revert_force();	// forcibly (automatically) revert buffer (prompts)
  virtual void  SetRevert();	// set the revert button on
  virtual void  UnSetRevert();	// set the revert button off
  virtual bool  HasChanged();	// returns true if user has changed anything
  virtual void  Changed();	// turns on apply and ok buttons
  virtual void  Unchanged();	// turns off apply and ok buttons
  virtual void	ReDraw();	// re-draw dialog
  virtual void	Raise();	// bring dialog to the front
  virtual void  Scroll(){}	// overload to scroll to field editor
  
  static void	GetPointerPos(ivCoord& x, ivCoord& y); // returns pointer position

  static  int	WaitDialog(ivWindow* win, const char* prompt="",
			   const char* win_title="", bool no_cancel=false, const ivColor* bgcol = NULL);
  // puts up a waiting dialog.  returns value of Edit()
};

class taivDialog_List : public taPtrList<taivDialog> { // #IGNORE list of dialogs
protected:
  void	El_Done_(void* it)	{ delete (taivDialog*)it; }

public:
  ~taivDialog_List()            { Reset(); }
};

//////////////////////////////////
// 	taivChoiceDialog	//
//////////////////////////////////

class taivChoiceDialog : public taivDialog {
public:
  String	but_labels;	// button labels, comma-delimited
  int		result;

  taivChoiceDialog()		{ result = 0; }
  ~taivChoiceDialog()		{ };

  // prompt is comma-delimited (1st entry is prompt) button labels
  void	Constr_Strings(const char* prompt, const char* win_title);
  void	Constr_Buttons();
  void	Constr_Dialog();
  int 	Edit();

#ifndef __MAKETA__
  virtual bool	Constr_OneBut(String& lbl, void (taivChoiceDialog::*fun)());
  // constructs one button, returns false when no more..
#endif

  void	But0Press() { dialog->dismiss(1); result = 0; }
  void	But1Press() { dialog->dismiss(1); result = 1; }
  void	But2Press() { dialog->dismiss(1); result = 2; }
  void	But3Press() { dialog->dismiss(1); result = 3; }
  void	But4Press() { dialog->dismiss(1); result = 4; }
  void	But5Press() { dialog->dismiss(1); result = 5; }
  void	But6Press() { dialog->dismiss(1); result = 6; }
  void	But7Press() { dialog->dismiss(1); result = 7; }
  void	But8Press() { dialog->dismiss(1); result = 8; }
  void	But9Press() { dialog->dismiss(1); result = 9; }

  // puts up a waiting dialog.  returns value of Edit()
  static  int	WaitDialog(ivWindow* win, const char* prompt="",
			   const char* win_title="", bool no_cancel=false, const ivColor* bgcol = NULL);
};	

class taivEditDialog : public taivDialog {
  // // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS edit dialog for classes
public:
  static ivCoord vfix_font_mult;	// how much to stretch default font by to get vfix

  int		show;		// current setting for what to show
  
  ivCoord	vfix;		// amount to fix the vertical size to
  ivCoord	hfix;		// amount to fix labels width to

  ivPolyGlyph*	labels;		// name representation
  ivPolyGlyph*	data_g;		// data glyphs
  ivGlyph*	meth_buts;	// method buttons
  ivGlyph*	meth_butg;	// method button representation

  taivMenu_List ta_menus;	// menu representations (from methods)
  taivMenu*	cur_menu;	// current menu to add to (if not otherwise spec'd)
  ivMenu*	menu;		// menu glyph

  taivDataList data_el;	// data elements
  taivDataList meth_el;	// method elements

  taivEditDialog(TypeDef* tp, void* base);
  taivEditDialog()		{ };
  ~taivEditDialog();

  void		CloseWindow();	// close edit dialog window and free current data

  void		Constr_Strings(const char* prompt, const char* win_title);
  void		Constr_Box();
  void		Constr_BodyBox();
  void		Constr_Body();
  void		Constr_Handlers();
  void		Constr_Dialog();
  int 		Edit();		// add to list of active_edit dialogs too

  void		GetImage(); 
  virtual void	GetImage_impl(const MemberSpace& ms, const taivDataList& dl,
			      void* base, ivWindow* win);
  void		GetValue();
  virtual void	GetValue_impl(const MemberSpace& ms, const taivDataList& dl,
			      void* base);

  virtual bool	ShowMember(MemberDef* md);
  virtual void	GetMembDescRep(MemberDef* md, ivMenu* dsc_menu, String indent);
  virtual ivGlyph* GetNameRep(MemberDef* md);	// returns one name item

  virtual void	Constr_Labels();   // construct the labels list
  virtual void	Constr_Labels_style(); // set the style for the labels
  virtual void	Constr_Labels_impl(const MemberSpace& ms);
  virtual void	Constr_Data();    // construct the data of the dialog
  virtual void	Constr_Data_impl(const MemberSpace& ms, taivDataList& dl);
  virtual void	Constr_Methods(); // construct the methods (buttons and menus)

  virtual void	GetVFix(ivFont* fnt);
  virtual void	GetHFix(ivFont* fnt, const MemberSpace& ms);
  virtual void	FocusOnFirst();
  virtual void	GetButtonRep();
  virtual void	GetMenuRep(MethodDef* md);
  virtual void	GetMenuButtonRep(MethodDef* md);
  virtual void	GetButtonImage();

  virtual void	GetShowMenu(); // make the show/hide menu
  virtual void	ShowChange_mc(taivMenuEl* sel);	// when show/hide menu changes
};

class EditNBDialog : public NoBlockDialog { // #IGNORE
public:
  EditNBDialog(taivEditDialog* ted, ivGlyph* g, ivStyle* s)
  : NoBlockDialog(ted,g,s) {};
  bool special_key(const ivEvent& e);
};

//////////////////////////////
//       taivDialogs        //
//////////////////////////////

class taivTokenDialog : public taivDialog {
  // automatic widget to select a token of a given type
public:
  TypeDef*	ths;
  TAPtr		itm;
  TAPtr		scope_ref;
  taivToken* 	itm_rep;

  taivTokenDialog(TypeDef* td, TAPtr scp_ref=NULL, TAPtr in_itm=NULL);
  ~taivTokenDialog();

  void		Constr_Box();
  void		GetValue();

  // this is the function you actually call..
  static TAPtr GetToken(TypeDef* td, const char* prompt="",
			TAPtr scp_ref=NULL, TAPtr init_itm = NULL, ivWindow* win=NULL);
};

class taivTypeDialog : public taivDialog {
  // select a type 
public:
  TypeDef*	base_typ;
  TypeDef*	sel_typ;
  taivTypeHier* typ_rep;

  taivTypeDialog(TypeDef* td, TypeDef* init_tp=NULL);
  ~taivTypeDialog();

  void		Constr_Box();
  void		GetValue();

  // this is the function you actually call..
  static TypeDef* GetType(TypeDef* td, const char* prompt="", TypeDef* init_tp=NULL,
			  ivWindow* win=NULL);
};

class taivEnumDialog : public taivDialog {
  // select an enum
public:
  TypeDef*	enum_typ;
  int		sel_val;
  taivMenu* 	enm_rep;

  taivEnumDialog(TypeDef* td, int init_vl=0);
  ~taivEnumDialog();

  void		Constr_Box();
  void		GetValue();

  // this is the function you actually call..
  static int 	GetEnum(TypeDef* td, const char* prompt="", int init_vl=0,
			ivWindow* win=NULL);
};

#endif // taiv_dialog_h
