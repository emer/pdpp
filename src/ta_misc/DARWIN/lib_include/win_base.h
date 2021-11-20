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

// win_base.h: basic types for objects that have their own windows
  
#ifndef win_base_h
#define win_base_h 1

#include <ta/ta_group.h>
#include <ta_misc/ta_misc_TA_type.h>

class	ivGlyph;		// #IGNORE
class 	ivFieldEditor;		// #IGNORE
class	ivButton;		// #IGNORE
class	ivMenuItem;		// #IGNORE
class	ivAction;		// #IGNORE
class	ivMenu;			// #IGNORE
class	ivMonoGlyph;		// #IGNORE
class	ivTelltaleGroup;	// #IGNORE
class	ivWindow;		// #IGNORE
class   ivCanvas;		// #IGNORE
class	ivWidgetKit;		// #IGNORE
class	ivDialogKit;		// #IGNORE
class	ivLayoutKit;		// #IGNORE
class	ivStyle;		// #IGNORE
class	ivPolyGlyph;		// #IGNORE
class	ivPatch;		// #IGNORE
class	ivInputHandler;		// #IGNORE
class	ivFileChooser;		// #IGNORE
class   ivManagedWindow;	// #IGNORE

class taivData;
class taivHierMenu;
class taivGetFile;
class taivMenuEl;
class taivMenu_List;
class taivHierMenu_List;
class taivDataList;

// predeclaration of types defined in this header
class MenuGroup_impl;
class WinGeometry;
class WinBase;
class WinView;
class WinMgr;

class winbMisc {
  // #NO_TOKENS miscellaneous things for winbase
public:
  static ivManagedWindow*	group_leader;	
  // #IGNORE group leader window (must be set when leader window is opened)

  static taBase_List		update_menus;
  // #HIDDEN winbase/menugroups that need menus updated in waitproc
  static taBase_List		update_winpos;
  // #HIDDEN winbase's that need positions updated in waitproc
  static taBase_List 		unopened_windows;
  // #HIDDEN unopened windows waiting to be opened

  static void   OpenWindows(); //  open all unopened windows

  static void	DamageCanvas(ivCanvas* can);
  static void	DamageWindow(ivWindow* win);

  static int 	WaitProc();	// waiting process function

  static void	Wait_UpdateMenus(); // update menus in waitproc
  static void	Wait_UpdateWinPos(); // update window position in waitproc

  static void 	MenuUpdate(TAPtr obj);
  // update menus relevant to the given object, which might have changed
  static void	DelayedMenuUpdate(TAPtr obj);
  // add object to list to be updated later (by Wait_UpdateMenus)

  static void	ScriptIconify(void* obj, int onoff); // record iconify command for obj to script
  static int	SetIconify(void* obj, int onoff); // set iconified field of winbase obj to onoff
};


//////////////////////////////////////////////////
// USE_TEMPLATE_GROUPS: macros to allow either  //
//////////////////////////////////////////////////

#ifdef USE_TEMPLATE_GROUPS
// template menu groups allow for type-specific menu groups
#define MENU_GROUP_PARENT	taGroup_impl

#define BaseGroup_of(T)		taGroup_of(T)

#define MenuGroup_of(T)							      	\
class T ## _MGroup : public MenuGroup<T> {				      	\
public:									      	\
  void	Initialize() 		{ };					      	\
  void 	Destroy()		{ };					      	\
  TA_BASEFUNS(T ## _MGroup);						\
}

#else
// no temlate, just use the Base Group and user must cast results..
#define MENU_GROUP_PARENT	taBase_Group

#define BaseGroup_of(T)							      \
class T ## _Group : public taBase_Group {				      \
public:									      \
  void	Initialize() 		{ SetBaseType(&TA_ ## T); }		      \
  void 	Destroy()		{ };					      \
  TA_BASEFUNS(T ## _Group);					      \
}

#define MenuGroup_of(T)							      \
class T ## _MGroup : public MenuGroup_impl {				      \
public:									      \
  void	Initialize() 		{ SetBaseType(&TA_ ## T); }		      \
  void 	Destroy()		{ };					      \
  TA_BASEFUNS(T ## _MGroup);					      \
}

#endif // USE_TEMPLATE_GROUPS


class gpivGroupEls;
class gpivSubGroups;


//////////////////////////////////////////////////
//   MenuGroup: groups that produce menus	//
//////////////////////////////////////////////////

class MenuGroup_impl : public MENU_GROUP_PARENT {
  // ##NO_TOKENS a group that generates menus
public:
  WinBase*		win_owner; 	// #READ_ONLY #NO_SAVE owner that is a win_base
  taivHierMenu* 	ta_menu; 	// #IGNORE menu for this group
  taivGetFile*		ta_file;	// #IGNORE file manager for this group
  gpivGroupEls* 	itm_list; 	// #IGNORE item list object (for callbacks)
  gpivSubGroups* 	grp_list; 	// #IGNORE subgroup list object (for callbacks)
  
  virtual taivHierMenu* GetMenu();	// #IGNORE initialize the menu & keep a copy (ta_menu)
  virtual void		GetMenu_impl();	// #IGNORE actually gets the menu
  virtual void		UpdateMenu();	// #IGNORE update the existing menu
  virtual void		UpdateElMenus(); // #IGNORE update menus of elements
  virtual ivMenuItem*	GetMenuItem();	// #IGNORE returns menu item for this menu
  virtual void		GetAllWinPos();
  // #IGNORE get current window position/size for all windows under me
  virtual void		ScriptAllWinPos();
  // #IGNORE generate script code for all windows under me

  // menu callback functions (mc)
  virtual void		Edit_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		New_mc( taivMenuEl* sel); 	// #IGNORE
  virtual void		Open_in_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		Load_over_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		Save_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		SaveAs_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		Remove_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		Duplicate_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		Move_mc(taivMenuEl* sel);	// #IGNORE
  virtual void		View_mc(taivMenuEl* sel);	// #IGNORE

  bool		Close_Child(TAPtr obj);
  
  void 	Initialize();
  void 	Destroy();
  void	InitLinks();
  TA_BASEFUNS(MenuGroup_impl);
};

//////////////////////////
//   WinGeometry	//
//////////////////////////

class WinGeometry : public taBase {
  // ##NO_TOKENS #INLINE Window geometry (position, size) 
public:
  WinBase* 	owner;   	// #READ_ONLY #NO_SAVE pointer to owner
  float 	lft; 		// left (horizontal)
  float		bot;  		// bottom (vertical)
  float		wd;		// width
  float		ht;		// height

  virtual void	GetWinPos();	// get the window position from parent winbase
  virtual void	SetWinPos();	// set the window position from parent winbase
  virtual void 	ScriptWinPos(ostream& strm = cout);

  TAPtr		GetOwner(TypeDef* tp) const	{ return taBase::GetOwner(tp); }
  TAPtr 	GetOwner() const 		{ return (TAPtr)owner; }
  TAPtr 	SetOwner(TAPtr ta)		{ owner = (WinBase*)ta; return ta; }

  void	UpdateAfterEdit();
  void 	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void	CutLinks()		{ owner = NULL; }
  void 	Copy_(const WinGeometry& cp);
  COPY_FUNS(WinGeometry, taBase);
  TA_BASEFUNS(WinGeometry);
};

class ivBitmap; // #IGNORE

//////////////////////////
//   	WinBase		//
//////////////////////////

class WinBase : public taNBase {
  // #NO_TOKENS the base type for objects with a window and a menu
public: 
  enum PrintFmt {
    POSTSCRIPT,
    JPEG,
    TIFF
  };

  String		win_name;	// #HIDDEN #NO_SAVE name field for the window
  String		file_name;	// #HIDDEN file name used in loading/saving
  WinBase*		win_owner;	// #READ_ONLY #NO_SAVE owner that has a window
  WinGeometry		win_pos;  	// #HIDDEN position/size of the window on the screen

  ivWidgetKit* 		wkit;		// #IGNORE provides widgets
  ivDialogKit*		dkit;		// #IGNORE provides dialogs
  ivLayoutKit* 		layout;		// #IGNORE provides layouts
  ivManagedWindow*	window;		// #IGNORE each project gets a window
  ivPatch*		win_box; 	// #IGNORE outermost glyph for resizing window
  ivPolyGlyph*		main_box;	// #IGNORE box containing everything
  taivMenu_List*	this_menus;	// #IGNORE menus for the "this" menus
  taivDataList*		this_meths;	// #IGNORE menu methods for this
  ivMenu*		menu;		// #IGNORE main_menu in this window
  ivGlyph*		body;		// #IGNORE body of the window 
  taivGetFile*		ta_file;	// #NO_SAVE #HIDDEN file manager for this
  taivGetFile*		print_file;	// #NO_SAVE #HIDDEN print file for this
  bool			iconified;	// #HIDDEN whether window is iconified or not

  int		Save(ostream& strm, TAPtr par=NULL, int indent=0);
  // Save object to a file
  int		SaveAs(ostream& strm, TAPtr par=NULL, int indent=0)
  { return Save(strm,par,indent); }
  // Save object to file
  int  		Load(istream& strm, TAPtr par=NULL);
  // load object from a file
  int		Edit(bool wait=false);	
  void		Close();
  // close this structure/window
  virtual void	Print(PrintFmt format, const char* fname = NULL);
  // #MENU #ARGC_1 #MENU_SEP_BEFORE #LABEL_Print_(Window) Print this object's entire window (including buttons, etc) to file (prompted next) in given format
  virtual void	Print_Data(PrintFmt format, const char* fname = NULL);
  // #MENU #ARGC_1 #LABEL_Print_Data_(Only) Print only specific data associated with this window (not control buttons, etc) to file (prompted next) in given format
  virtual void  UpdateMenus();
  // #MENU #MENU_SEP_BEFORE update all menus under me (inclusive)
  virtual void	GetAllWinPos();
  // #MENU get current window position/size for all windows under me (inclusive)
  virtual void	ScriptAllWinPos();
  // #MENU #NO_SCRIPT generate script code for all windows under me (inclusive)
  virtual void	SetWinPos(float left=0, float bottom=0, float width=0, float height=0);
  // #MENU set current window position and size (0's mean use current vaues)
  virtual void	GetWinPos()		{ win_pos.GetWinPos(); }
  // get current window position
  virtual void 	ScriptWinPos() 		{ win_pos.ScriptWinPos(cout); }
  // #NO_SCRIPT generate script code to position the window
  virtual void  Iconify();		// #MENU iconify the window (saves iconified state)
  virtual void	DeIconify();		// deiconify the window (saves deiconified state)
  virtual bool	IsMapped();		// return the 'mapped' status of the window
  virtual void	ViewWindow(float left=0, float bottom=0, float width=0, float height=0);
  // either de-iconfiy or create a new view to view this object, and then locate/size it

  virtual void 	OpenNewWindow();	// #IGNORE open a new window for this project
  virtual void 	CloseWindow();		// #IGNORE close the window
  virtual void	SetFileName(const char* fname);	// #IGNORE set new file name for object
  virtual void	SetWinName();		// #IGNORE set the window name to object path/name
  virtual void	GetWinBoxes();		// #IGNORE get win/main boxes
  virtual void 	GetBodyRep();		// #IGNORE actually put something in the display
  virtual void	GetWindow();		// #IGNORE get the window
  virtual void 	WinInit();		// #IGNORE initialize basic wkit, etc. pointers

  virtual void  UpdateMenus_impl(); 	// #IGNORE actually does the work
  virtual bool	ThisMenuFilter(MethodDef* md); // #IGNORE filter this menu items
  virtual void	GetThisMenus();		// #IGNORE get the 'this' menus
  virtual void	GetThisMenus_impl(taivMenu_List& ths_men, taivDataList& ths_meth, String prfx);
  // #IGNORE actually gets the menus
  virtual void 	GetMenu();		// #IGNORE get the menu items from MenuGroups

  virtual void	GetFileProps(TypeDef* td, String& fltr, bool& cmprs);
  // #IGNORE get file properties for given type
  taivGetFile*	GetFileDlg();		// #IGNORE for this and its menugroups
  virtual void	GetPrintFileDlg(PrintFmt fmt);	// #IGNORE for the printfile
  virtual String GetPrintFileExt(PrintFmt fmt); // get string of file extension for given fmt

  virtual void  ReSize(float width=0, float height=0);
  virtual void  Place(float left=0, float bottom=0);
  virtual void  Move(float left=0, float bottom=0);
  virtual void	Raise();	// raise window to front
  virtual void	Lower();	// lower window to back

  virtual void  Print_impl(PrintFmt format, ivGlyph*, const char* fname=NULL); // #IGNORE
  virtual ivGlyph* GetPrintData(); // #IGNORE overload this

  int	Dump_Load_Value(istream& strm, TAPtr par=NULL);
  // process any pending iv events after loading..

  void	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy();
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const WinBase& cp);
  COPY_FUNS(WinBase, taNBase);
  TA_BASEFUNS(WinBase);
};

//////////////////////////
//   	WinView		//
//////////////////////////

class AnimImgCapture : public taBase {
  // ##NO_TOKENS #NO_UPDATE_AFTER #INLINE capture images for animations
public:
  bool		  capture;	// capture images of the display every time it is updated
  WinBase::PrintFmt img_fmt;	// format to save images for animation capture
  int		  ctr;		// counter for identifying animation image capture
  String	  subdir;	// subdirectory to store images in

  void 	Initialize();
  void 	Destroy()	{ };
  SIMPLE_COPY(AnimImgCapture);
  COPY_FUNS(AnimImgCapture, taBase);
  TA_BASEFUNS(AnimImgCapture);
};

// the updating object typically will require notification about being added
// as an updater to this view.  this can be done by overloading the
// appropriate AddNotify() and RemoveNotify() functions

class WinView : public WinBase {
  // #NO_TOKENS objects that can be actively updated and have multiple views
public:
  taBase_Group		updaters; 	
  // #HIDDEN #LINK_GROUP list of objects which update view
  taivHierMenu_List* 	gp_ta_menu;
  // #IGNORE storage for ta_menu items for each menugroup
  int			n_mgr_menus;
  // #READ_ONLY #NO_SAVE number of 'this' menus for the manager
  ivMenu*		view_menu;	// #IGNORE menu for this view
  taivMenu_List*	view_menus;	// #IGNORE menu objects for the "this" menus
  WinMgr*		mgr;
  // #READ_ONLY #NO_SAVE the manager for this window
  TypeDef*		updater_type;
  // #HIDDEN #TYPE_taNBase Type of updater to use for this view
  bool  		display_toggle;	// whether the display is being updated
  AnimImgCapture  	anim;		// capture images for making animations

  // redo stuff for view that is different than base
  void  SetWinName();
  void	GetWinBoxes();
  void	OpenNewWindow();
  void 	CloseWindow();
  void 	GetMenu();
  bool	ThisMenuFilter(MethodDef* md);
  void	GetThisMenus();

  virtual void	AddNotify(TAPtr)	{ };
  // #IGNORE notify object that it has been added as an updater of this view
  virtual void	RemoveNotify(TAPtr)	{ };
  // #IGNORE notify object that is no longer an updater of this view
  virtual TypeDef* UpdaterType()	{ return &TA_taBase; }
  // #IGNORE type of object which is used to update views

  virtual void	AddUpdater(TAPtr updtr); 	
  // #MENU #MENU_SEP_BEFORE #TYPE_ON_updater_type Add object as an updater of this view
  virtual void	NotifyAllUpdaters();
  // notify all updaters that they are updating us
  virtual void	RemoveUpdater(TAPtr updtr);
  // #MENU #FROM_GROUP_updaters Remove object from those updating this view
  virtual void	RemoveAllUpdaters(); 	// remove all updaters of this view

  virtual void  InitDisplay()		{ };
  // #MENU re-initialize the display
  virtual void	UpdateDisplay(TAPtr =NULL) 	{ };
  // #MENU #ARGC_0 update the display to reflect changes

  virtual void	CaptureAnimImg();	// capture an image for animations: call this in Updatedisplay if desired

  virtual void 	StartAnimCapture(PrintFmt image_fmt = JPEG, int image_ctr=0, const char* sub_dir = "anim");
  // #MENU #MENU_ON_Actions #CONFIRM start automatic capturing of window updates to files of given type saved in subdirectory sub_dir with incrementing counter starting at image_ctr, for use in creating an animation
  virtual void 	StopAnimCapture();
  // #MENU #CONFIRM stop capturing images

  int	Dump_Load_Value(istream& strm, TAPtr par);

  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const WinView& cp);
  COPY_FUNS(WinView, WinBase);
  TA_BASEFUNS(WinView);
};

BaseGroup_of(WinView);
MenuGroup_of(WinView);
  
//////////////////////////
//   	WinMgr		//
//////////////////////////

class WinMgr : public WinBase {
  // ##MEMB_IN_GPMENU #NO_TOKENS managers own views 
public:
  WinView_MGroup views;			// views of this object

  virtual TypeDef* GetDefaultView()	{ return &TA_WinView; }
  // #IGNORE different WinMgr's overload this to set the default view to open

  virtual void	GetPtrsFromView(); 	// #IGNORE get window pointers from default view

  void		SetWinName();		// set name for all views
  virtual void 	OpenNewWindow();	// open a new window
  virtual void 	OpenNewWindow(TypeDef*);// open a new window of spec'd type
  virtual void 	CloseWindow();		// close last window opened
  virtual void	CloseWindow(WinView*);	// close specified window
  virtual void 	CloseAllWindows();	// close all windows
  bool		ThisMenuFilter(MethodDef* md); // #IGNORE
  void 		GetMenu();		// #IGNORE get the menu items from MenuGroups
  void 		UpdateMenus();
  void		GetAllWinPos();
  void		ScriptAllWinPos();
  void 		GetWinPos();	
  void 		ScriptWinPos();	

  virtual void	UpdateAllViews(); 	// call this when something big happens
  virtual void	InitAllViews(); 	// call this when something biger happens

  void	UpdateAfterEdit();

  void	Initialize();
  void	Destroy();
  void	CutLinks();
  void	InitLinks();
  void	Copy(const WinMgr& cp);
  TA_BASEFUNS(WinMgr);
};

#include <ta_misc/win_base_tmplt.h>

#endif // win_base_h
