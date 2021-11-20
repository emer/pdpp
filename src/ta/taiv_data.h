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

// taiv_data.h: InterViews-based graphical data representations

#ifndef taiv_data_h
#define taiv_data_h 1

#include <ta/ta_base.h>

#include <ta/enter_iv.h>
#include <InterViews/glyph.h>
#include <InterViews/monoglyph.h>
#include <InterViews/geometry.h>
#include <ta/leave_iv.h>

#include <unistd.h>

#ifndef CYGWIN
//#include <X11/X.h>
struct _XDisplay;		// #IGNORE
typedef unsigned long XWindow;		// #IGNORE
#endif

class 	ivFieldEditor;		// #IGNORE
class	ivButton;		// #IGNORE
class	ivMenuItem;		// #IGNORE
class	ivAction;		// #IGNORE
class	ivMenu;			// #IGNORE
class	ivMonoGlyph;		// #IGNORE
class	ivTelltaleGroup;	// #IGNORE
class	ivWindow;		// #IGNORE
class   ivManagedWindow;	// #IGNORE
class	ivWidgetKit;		// #IGNORE
class	ivDialogKit;		// #IGNORE
class	ivLayoutKit;		// #IGNORE
class	ivStyle;		// #IGNORE
class	ivPolyGlyph;		// #IGNORE
class	ivPatch;		// #IGNORE
class	ivInputHandler;		// #IGNORE
class	ivFileChooser;		// #IGNORE
class 	ivFileBrowser;		// #IGNORE
class	ivCursor;		// #IGNORE
class	ivCanvas;		// #IGNORE
class	ivEvent;		// #IGNORE
class	ivBitmap;		// #IGNORE
class 	ivDialog;		// #IGNORE
class	ivFont;			// #IGNORE
class	ivColor;		// #IGNORE
class	ivDeck;			// #IGNORE

// pre-declare classes:
class taivData;
class taivDialog;
class taivEditDialog;
class NoBlockDialog;

typedef taPtrList<ivWindow> 	ivWindow_List;  // #IGNORE list of windows

class ivGlyph_List : public taPtrList<ivGlyph> { // #IGNORE list of glyphs
protected:
  void* 	El_Ref_(void* it)	{ ivResource::ref((ivGlyph*)it); return it; }
  void*		El_unRef_(void* it)	{ ivResource::unref((ivGlyph*)it); return it; }

public:
  ~ivGlyph_List() { Reset(); }
};

class taivDialog_List;		// #IGNORE
typedef taPtrList<taivEditDialog> taivEditDialog_List; // #IGNORE

//////////////////////////////
//       taivMisc          //
//////////////////////////////

class taivMisc { // #NO_TOKENS #INSTANCE miscellaneous stuff for taiv
public:
  static bool			iv_active;
  // #READ_ONLY indicates if iv has been started up or not

  static ivWindow_List		active_wins; 	// #IGNORE list of active windows
  static ivWindow_List		delete_wins; 	// #IGONRE list of windows to delete (delayed)
  static taivDialog_List	active_dialogs;	// #IGNORE list of active (NoBlock) dialogs
  static taivEditDialog_List	active_edits;	// #IGNORE list of active edit dialogs

  static NoBlockDialog*		load_dlg;       // #IGNORE load dialog
  static int			busy_count; // levels of busy

  static TypeSpace		arg_types; 	// list of all taivArgTypes

  static ostream*		record_script;
  // stream to use for recording a script of interface activity (NULL if no record)

  static void (*Update_Hook)(TAPtr); 	
  // #IGNORE called after apply in a dialog, etc. obj is the object that was edited

  static void	Initialize();	// #IGNORE initialize iv interface system

  static void	Update(TAPtr obj); // #IGNORE update stuff after modification (uses hook fun)

  static void 	Busy();		// puts system in a 'busy' state (pointer, no input)
  static void	DoneBusy();	// when no longer busy, call this function
  static void	DoneBusy_impl();// #IGNORE implements the done busy function

  static void	StartRecording(ostream* strm); // sets record_strm and record_cursor
  static void   StopRecording();	       // unsets record_strm and record_cursor

  static void	SetWinCursor(ivWindow* win);
  // #IGNORE sets cursor for given window based on busy and record status

  static int	RunIV();	// run iv event loop
  static int	RunIVPending();	// run any pending iv events that might need processed
  static int	FlushIVPending(); // run multiple loops of iv pending to flush out all pending events

  static bool	RecordScript(const char* cmd);
  // record the given script command, if the script is open (just sends cmd to stream)

  static void  ScriptRecordAssignment(taBase* tab,MemberDef* md);
  // record last script assignment of tab's md value;

  static void 	SRIAssignment(taBase* tab,MemberDef* md);
  // record inline md assignment

  static void 	SREAssignment(taBase* tab,MemberDef* md);
  // record enum md assignment

  static void	PurgeDialogs();	
  // remove any 'NoBlock' dialogs from active list (& delete them)

  static bool	RevertEdits(void* obj, TypeDef* td);
  // revert any open edit dialogs for given object

  static bool	CloseEdits(void* obj, TypeDef* td);
  // close any open edit dialogs for object or sub-objs

  static bool	NotifyEdits(void* obj, TypeDef* td);
  // notifies any open edit dialogs for given object that a change in its data has occured

  static taivEditDialog* FindEdit(void* obj, TypeDef* td);
  // find first active edit dialog for this object

  static void   CreateLoadDialog();
  static void   RemoveLoadDialog();
  static void	SetLoadDialog(char* tpname);

  static void	DeleteWindows(); // #IGNORE delete windows on the delete list

  static void	Cleanup(int err); // #IGNORE function to be called upon exit to clean stuff up

#ifdef __MAKETA__
};
#else 
  // helper functions for iv
  ivWidgetKit* 	wkit;
  ivDialogKit* 	dkit;
  ivLayoutKit* 	layout;
  ivStyle* 	style;

  ivCoord  	vsep_c;		// separators are for "small spaces" between items
  ivCoord 	hsep_c;
  ivGlyph*	vsep;
  ivGlyph*	hsep;
  ivGlyph*	vfsep;		// fixed versions
  ivGlyph*	hfsep;

  ivCoord	vspc_c;		// spaces are for "large spaces" between items
  ivCoord	hspc_c;
  ivGlyph*	vspc;
  ivGlyph*	hspc;
  ivGlyph*	vfspc;		// fixed (non stretchable) versions
  ivGlyph*	hfspc;

  // make a fixed size button

  ivGlyph*	small_button(ivGlyph* b);
  ivGlyph*	medium_button(ivGlyph* b);
  ivGlyph*	big_button(ivGlyph* b);

  // make a natural size flexible button

  ivGlyph*	small_flex_button(ivGlyph* b);
  ivGlyph*	medium_flex_button(ivGlyph* b);
  ivGlyph*	big_flex_button(ivGlyph* b);
 

  // makes aligning easier, top = top of a box, with a sep, etc.
  ivPolyGlyph*	top_sep(ivGlyph* g);
  ivPolyGlyph*	bot_sep(ivGlyph* g);
  ivPolyGlyph*	mid_sep(ivGlyph* g);
  ivPolyGlyph*	lft_sep(ivGlyph* g);
  ivPolyGlyph*	rgt_sep(ivGlyph* g);
  ivPolyGlyph*	cen_sep(ivGlyph* g);
  
  ivPolyGlyph*	top_spc(ivGlyph* g);
  ivPolyGlyph*	bot_spc(ivGlyph* g);
  ivPolyGlyph*	mid_spc(ivGlyph* g);
  ivPolyGlyph*	lft_spc(ivGlyph* g);
  ivPolyGlyph*	rgt_spc(ivGlyph* g);
  ivPolyGlyph*	cen_spc(ivGlyph* g);
  
  ivPolyGlyph*	wrap(ivGlyph* g);

  const ivColor* font_foreground;
  float 	edit_darkbg_brightness;
  float 	edit_lightbg_brightness;
  const ivColor* edit_darkbg;	// edit dialog darker background color 
  const ivColor* edit_lightbg;	// edit dialog lighter background color (for highlighting)
  String 	color_to_string(const ivColor* clr); // returns a string value (appropriate for setting in a style) for the color

  ivFont* 	name_font;
  ivFont* 	small_menu_font;
  ivFont* 	small_submenu_font;
  ivFont*	big_menu_font;
  ivFont* 	big_submenu_font;
  ivFont* 	big_menubar_font;
  ivFont* 	big_italic_menubar_font;

  double	small_button_width;
  double	medium_button_width;
  double	big_button_width;

  ivStyle* 	name_style;
  ivStyle* 	title_style;
  ivStyle*     	apply_button_style;

  ivCursor*	wait_cursor;	// cursor used for waiting
  ivCursor*	record_cursor;	// cursor used for recording
  ivBitmap*	icon_bitmap;	// icon bitmap

  int GetButton(const ivEvent& e); // applies keyboard mods

#ifndef CYGWIN
  static void DumpJpeg(_XDisplay* dpy, XWindow win, const char* fnm, int quality=85, int xstart=0, int ystart=0, int width=-1, int height=-1);
  // dump window to jpeg file
  static void DumpJpegIv(ivWindow* win, const char* fnm, int quality=85, int xstart=0, int ystart=0, int width=-1, int height=-1);
  // dump interviews window to jpeg file
  static void DumpTiff(_XDisplay* dpy, XWindow win, const char* fnm, int xstart=0, int ystart=0, int width=-1, int height=-1);
  // dump window to tiff file
  static void DumpTiffIv(ivWindow* win, const char* fnm, int xstart=0, int ystart=0, int width=-1, int height=-1);
  // dump interviews window to tiff file
#endif

  taivMisc();
  ~taivMisc();
};
#endif 

extern taivMisc* taivM;	// this is an instance to use for all seasons..

class IconGlyph : public ivMonoGlyph {
  // #IGNORE a special glyph to put as the first thing in a window, sets pointers..
public:
  ivManagedWindow*	window;		// this is the window
  void*			obj;		// this is the object that owns the window
  int			(*SetIconify)(void*, int on_off);
  // set iconify function: -1 get value, 0 = set off, 1 = set on
  void			(*ScriptIconify)(void* , int on_off);
  // script record iconify: 0 = set off, 1 = set on
  bool			first_draw; // don't set flags on first draw
  ivAllocation  	last_draw_allocation;

  void	SetWindow(ivManagedWindow* w);

  void  draw(ivCanvas*, const ivAllocation&) const;
  void  undraw();

  IconGlyph(ivGlyph* g,ivManagedWindow* w=NULL,void* o=NULL);
};

class HighlightBG : public ivMonoGlyph {
public:
  const ivColor* bg_color;	// default background color
  const ivColor* hi_color;	// highlighted background color
  bool		highlight;	// whether to highlight or not

  HighlightBG(ivGlyph* body, const ivColor* bg, const ivColor* hi);
  virtual ~HighlightBG();

  virtual void allocate(ivCanvas*, const ivAllocation&, ivExtension&);
  virtual void draw(ivCanvas*, const ivAllocation&) const;
  virtual void print(ivPrinter*, const ivAllocation&) const;
};

//////////////////////////////////////////////////////////
// 	taivData: glyphs to represent kinds of data	//
//////////////////////////////////////////////////////////

class taivData : public taRefN {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS base class for data elements
public:
  taivData*		parent;		// if data is contained within data, this the parent container
  taivDialog*		dialog;		// dialog that this belongs to (optional)
  String		orig_val;	// text of original data value
  HighlightBG*		hi_bg;		// a highlight-background if highlighting is used

  taivData(taivDialog* dlg, taivData* par);
  virtual ~taivData();

  virtual void		CreateHiBG(void); // create the highlightbg object for this one
  virtual ivGlyph*	HiBGLook(ivGlyph* body);
  // use this in GetLook call to have hi_bg surround object if hi_bg has been created

  virtual ivGlyph*	GetRep()		{ return NULL; }
  virtual ivGlyph*	GetLook()		{ return NULL; }
  virtual bool	  	NeedsFocus()		{ return false; }

  virtual void		DataChanged(taivData* chld = NULL);
  // indicates something changed in the data from user input, if chld is passed then it called parent->DataChanged(this)

  // each data defines its own GetValue() function to get the value from
  // the data representation, GetImage() generates the image representation
  // from the given value
};

class taivDataList : public taPtrList<taivData> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void*		El_Ref_(void* it)	{ taRefN::Ref((taivData*)it); return it; }
  void* 	El_unRef_(void* it)	{ taRefN::unRef((taivData*)it); return it; }
  void		El_Done_(void* it)	{ taRefN::Done((taivData*)it); }

public:
  ~taivDataList()                       { Reset(); }
};

class taivField : public taivData {
public:
  ivFieldEditor* 	rep;
  ivInputHandler*	input_handler;

  taivField(const char* strval, taivDialog* dlg=NULL,ivInputHandler* ih=NULL,taivData*par=NULL);
  ~taivField();

  virtual void	RemoveFromHandler();
  virtual void	SetHandler(ivInputHandler* ih);

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();
  bool		NeedsFocus()		{ return true; }

  virtual const char* GetValue();
  virtual void 	GetImage(const char* val);
};

// this is for integers and it adds up and down arrow buttons
class taivIncrField : public taivField { // increment/decrement field
public:
  taivIncrField(const char* strval, taivDialog* dlg=NULL,ivInputHandler* ih=NULL,
                taivData*par=NULL,int positive=false);
  ~taivIncrField();

  ivGlyph*	incr_rep;
  ivButton* 	upbutton;
  ivButton* 	downbutton;
  int		pos;

  virtual void	Incr();
  virtual void	Decr();

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();
};

// "read-only" field, just doesn't get focus, doesn't allow typing....  
class taivRO_Field : public taivField { // read only field
public:
  taivRO_Field(const char* strval, taivDialog* dlg=NULL, taivData* par=NULL)
    : taivField(strval, dlg, NULL, par)
  { };
  ~taivRO_Field()					{ };
  
  bool		NeedsFocus()		{ return false; }
};
  
class taivToggle : public taivData {
public:
  ivButton* 	rep;

  taivToggle(taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivToggle();

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	Toggle_Callback();

  virtual int	GetValue();	
  virtual void 	GetImage(int stat);
};

class taivLabel : public taivData {
public:
  ivGlyph*	rep;
  
  taivLabel(taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivLabel();

  ivGlyph*	GetRep()		{ return rep; }
  ivGlyph*	GetLook();

  virtual void 	GetImage(const char* val);	
};

// this is a regular field plus a toggle..
class taivPlusToggle : public taivData {
public:
  taivData*	data;
  ivButton* 	but_rep;
  ivGlyph* 	rep;
  
  taivPlusToggle(taivData* dat, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivPlusToggle();
  
  ivGlyph*	GetRep();
  ivGlyph*	GetLook();
  bool		NeedsFocus()		{ return data->NeedsFocus(); }
  void		DataChanged(taivData* chld=NULL);
  
  virtual void	Toggle_Callback();

  virtual int	GetValue();
  virtual void 	GetImage(int stat);
};


//////////////////////////
//     taivPolyData	//
//////////////////////////

// this class supports the use of hierarchical sub-data within a data item
// its default behavior is to put everything in an hbox with labels

class taivPolyData : public taivData {
public:
  TypeDef* 	typ;
  void*		cur_base;
  taivDataList  data_el;
  ivGlyph*	rep;
  int		show;

  taivPolyData(TypeDef* tp, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivPolyData();

  ivGlyph*	GetRep()		{ return rep; }
  ivGlyph*	GetLook();

  virtual void	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL);

  virtual void  GetImage(void* base, ivWindow* win);
  virtual void	GetValue(void* base);

  virtual bool	ShowMember(MemberDef* md);
};  

//////////////////////////
//     taivDataDeck	//
//////////////////////////

// contains sub-data's within a deck, can toggle between them

class taivDataDeck : public taivData {
public:
  taivDataList  data_el;
  ivGlyph*	rep;
  ivDeck*	deck;
  int		cur_data;

  taivDataDeck(taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivDataDeck();

  ivGlyph*	GetRep()		{ return rep; }
  ivGlyph*	GetLook();

  virtual void	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL);
  virtual void  GetImage(int i);
};  


//////////////////////////////////////////////////////////
// 		taivData: 	Menus			//
//////////////////////////////////////////////////////////

class taivMenu;
class taivMenuEl;

// this is the action for meus that returns the selected menuel
class taivMenuAction : public virtual ivResource {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  taivMenuAction();
  virtual ~taivMenuAction();

  virtual void Select(taivMenuEl*);
};

#define __taivMenuCallback(T) T##_taivMenuCallback
#define taivMenuCallback(T) __taivMenuCallback(T)
#define __taivMenuMemberFunction(T) T##_taivMenuMemberFunction
#define taivMenuMemberFunction(T) __taivMenuMemberFunction(T)

#define declare_taivMenuCallback(T) \
typedef void (T::*taivMenuMemberFunction(T))(taivMenuEl*); \
class taivMenuCallback(T) : public taivMenuAction { \
public: \
    taivMenuCallback(T)( \
	T*, taivMenuMemberFunction(T) Select \
    ); \
    virtual ~taivMenuCallback(T)(); \
\
    virtual void Select(taivMenuEl*); \
private: \
    T* obj_; \
    taivMenuMemberFunction(T) Select_; \
};

#define implement_taivMenuCallback(T) \
taivMenuCallback(T)::taivMenuCallback(T)( \
    T* obj, taivMenuMemberFunction(T) Select \
) { \
    obj_ = obj; \
    Select_ = Select; \
} \
\
taivMenuCallback(T)::~taivMenuCallback(T)() { } \
\
void taivMenuCallback(T)::Select(taivMenuEl* m) { (obj_->*Select_)(m); }


class taivMenuEl {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS holds menu data
public:
  taivMenu*	owner;
  ivMenuItem* 	rep;
  int 		itm_no;
  String	label;
  void*		usr_data;
  ivAction*	action;		// for a direct call-back
  taivMenuAction *men_act;	// for a menu-action call-back (returns item selected)

  virtual void	Select();	// callback for selecting an item, which then decides
				// if action or men_act should be called subsequently
  taivMenuEl(taivMenu* own, ivMenuItem* rp, int it, const char* lbl,
	      void* usr, ivAction* act);
  taivMenuEl(taivMenu* own, ivMenuItem* rp, int it, const char* lbl,
	      void* usr, taivMenuAction* act);
  virtual ~taivMenuEl();
};

class taivMenuEl_List : public taPtrList<taivMenuEl> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (taivMenuEl*)it; }

public:
  ~taivMenuEl_List()            { Reset(); }
};


//////////////////////////////////
// 	    taivMenu		//
//////////////////////////////////

class taivMenu : public taivData {
  // non-hierarchical menu for selecting a single item
public:
  enum RepType {
    menubar,			// represent menu as a menubar
    menuitem 			// represent menu as menuitem (for use in a bigger bar)
  };
  enum SelType {
    normal,
    normal_update,
    radio,
    radio_update,
    use_default 		// only for additem..
  };
  enum FonType {
    big,
    big_italic,
    small,
    skinny_small
  };

  RepType		rep_type;
  SelType		sel_type;
  FonType		font_type;

  taivMenuEl_List	items;
  taivMenuEl*		cur_sel;  // selection for getting value of menu
  int			cur_item; // for creating menu
  
  ivMenu*		bar;	// the menubar representation
  ivMenu*		mnu;	// the pulldown inside the bar
  ivMenuItem*		mlab;	// the label in the menubar
  ivMonoGlyph*		mlab_g; // the glyph in the label
  String		mlab_str; // string contents of current menu label
  ivTelltaleGroup* 	grp;	// if radio

  void Constr();
  taivMenu(int rt, int  st, int ft, taivDialog* dlg=NULL, taivData* par=NULL);
  taivMenu(int rt, int  st, int ft, const char* lbl, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivMenu();

  ivGlyph*		GetRep();
  ivGlyph*		GetLook();
  ivMenuItem*		GetMenuItem()   { return mlab; }

  virtual taivMenuEl*	GetValue()	{ return cur_sel; }
  virtual bool 		GetImage(int itm);
  virtual bool 		GetImage(void* usr); // set to this usr item..

  virtual ivMenuItem*	NewItem(const char* val, int st);
  virtual taivMenuEl* 	AddItem(const char* val, void* usr = NULL, int st = use_default,
				ivAction* act = NULL);
  virtual taivMenuEl* 	AddItem(const char* val, void* usr, int st, taivMenuAction* act);
  virtual void		AddSep();
  virtual void		SetMLabel(const char* val);	// connect rep & mnu with mlab
  virtual void		Update();		// action-update
  virtual taivMenuEl*	CurEl()		{ return items.FastEl(cur_item); }

  virtual void		ResetMenu();
};

class taivMenu_List : public taPtrList<taivMenu> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (taivMenu*)it; }
  const String& El_GetName_(void* it) const { return ((taivMenu*)it)->mlab_str; }

public:
  ~taivMenu_List()              { Reset(); }
};


//////////////////////////////////
// 	taivHierMenu		//
//////////////////////////////////

class taivHierMenu;

class taivHierEl : public taivMenuEl {
  // data element for hierarchical menus
public:
  int		sub_no;

  taivHierEl(taivHierMenu* own, ivMenuItem* rp, int sb, int it, const char* lbl,
	      void* usr, ivAction* act);
  taivHierEl(taivHierMenu* own, ivMenuItem* rp, int sb, int it, const char* lbl,
	      void* usr, taivMenuAction* act);
  ~taivHierEl() { };
};

class taivHierSub {		// ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  taivMenuEl_List	sub_items;
  ivMenu*		sub_mnu;
  void*			usr_data; // if you want to hang something off of it

  taivHierSub(ivMenu* sm);
  taivHierSub(ivMenu* sm, void* usr);
  ~taivHierSub();
};

class taivHierSub_List : public taPtrList<taivHierSub> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (taivHierSub*)it; }

public:
  ~taivHierSub_List()           { Reset(); }
};

class taivHierMenu : public taivMenu {
  // a hierarchical menu
public:
  taivHierSub_List	subs;	// a list of subitems on the menu
  int			cur_sno; // current menu to add to
  ivMenu*		cur_sub;

  taivHierMenu(int rt, int st, int ft, taivDialog* dlg=NULL, taivData* par=NULL);
  taivHierMenu(int rt, int st, int ft, void* usr, taivDialog* dlg=NULL, taivData* par=NULL);

  ~taivHierMenu();

  taivMenuEl* 	AddItem(const char* val, void* usr = NULL, int st = use_default,
        		ivAction* act = NULL);
  taivMenuEl* 	AddItem(const char* val, void* usr, int st, taivMenuAction* act);
  void		AddSep();
  int		AddSubMenu(const char* val, void* usr = NULL); // returns number of new sub

  void		PopSub()
  { cur_sub = subs.FastEl(--cur_sno)->sub_mnu; }
  void		SetSub(int ns);
  ivMenuItem*	GetSubMenu(int sub, int itm);
  taivHierEl*	GetSubEl(int sub, int itm)
  { return (taivHierEl*)subs.FastEl(sub)->sub_items.FastEl(itm); }
  taivMenuEl*	CurEl()
  { return subs.FastEl(cur_sno)->sub_items.FastEl(cur_item); }
  taivHierEl*	HierCurEl()		{ return (taivHierEl*)CurEl(); }

  bool 		GetImage(int itm)	{ return taivMenu::GetImage(itm); }
  bool		GetImage(int sub, int itm);
  bool		GetImage(void* usr); // set to this usr item

  void		ResetMenu();
};

class taivHierMenu_List : public taPtrList<taivHierMenu> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (taivHierMenu*)it; }

public:
  ~taivHierMenu_List()          { Reset(); }
};


//////////////////////////////////
// 	taivEditButton		//
//////////////////////////////////

class taivEditButton : public taivData {
  // actually an edit menu...
public:
  taivMenu*	rep;
  TypeDef*	typ;
  void*		cur_base;
  ivWindow*	cur_win;
  taivEdit*	ive;
  taivDataList meth_el;	// method elements
  bool		read_only; // create menu which only allows for #EDIT_READ_ONLY members

  taivEditButton(TypeDef* tp, void* base, ivWindow* win, taivEdit *taive=NULL,
		  taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivEditButton();

  virtual void	SetLabel();	// set the edit button label
  virtual void	GetMethMenus();

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetImage(void* base, ivWindow* win);
  virtual void	Edit();		// callback
};

//////////////////////////////////
// 	taivGetFile		//
//////////////////////////////////

class taivGetFile : public taRefN { 
  // ##NO_TOKENS associate this with each file that is managed
public:
  static int	buf_size;	// size of the buffer for input operations
  static String	last_fname;	// last file name processed
  static String	last_dir;	// last directory name processed

  String	filter;
  String	dir;
  String	fname;
  bool		compress;	// true if file should be auto-compressed
  bool		select_only;	// file is to be selected only (not opened)

  ivWindow*	win;		// #IGNORE
  taivDialog*   dlg;		// #IGNORE
  ivFileChooser* chooser;	// #IGNORE the actual file chooser
  ifstream*	ifstrm;		// #READ_ONLY
  char*		iofbuf;		// #IGNORE buffer for input/output
  pid_t		compress_pid;	// #IGNORE process id of compress child proc
  ofstream*	ofstrm;		// #READ_ONLY
  fstream*	fstrm;		// #READ_ONLY
  istream*	istrm;		// #READ_ONLY
  ostream*	ostrm;		// #READ_ONLY
  bool		open_file;	// #READ_ONLY true if there is an open file somewhere
  bool		file_selected;	// #READ_ONLY true if a file was selected last time..

  virtual istream*	UnCompressInput(const char* filename);
  // get an input stream from given compressed file which does uncompression via pipe
  virtual ostream*	CompressOutput(const char* filename, bool append=false);
  // get an output stream to given file which does compression via pipe

  virtual istream*	open_read();
  virtual ostream*	open_write();
  virtual ostream* 	open_append();
  virtual bool		open_write_exist_check(); // returns true if file already exists for writing
  
  virtual istream*	Open(const char* nm = NULL, bool no_dlg = false);
  // to get a file for reading (already exists)
  virtual ostream*	Save(const char* nm = NULL, bool no_dlg = false);
  // to save to an existing file
  virtual ostream*	SaveAs(const char* nm = NULL, bool no_dlg = false);
  // to save with a new file
  virtual ostream*	Append(const char* nm = NULL, bool no_dlg = false);
  // to save a file for appending (already exists)
  virtual void		Close();	// close the stream

  virtual void		FixFileName(); // make sure suffix is right
  virtual void		GetDir();      // get directory from file name

  taivGetFile();	
  taivGetFile(const char* dr, const char* fltr, ivWindow* wn, ivStyle* s, bool comp);
  virtual ~taivGetFile();
};

//////////////////////////////////
// 	taivObjChooser		//
//////////////////////////////////

class taivObjChooser {
// ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
// select objects from a list, much like a file chooser.  can be tokens from typedef or items on a list
public:
  ivWidgetKit* 		kit;
  ivStyle* 		style;

  bool	        	select_only;	// if true, only for selecting objects from current parent object
  String		caption; 	// current caption at top of chooser
  String		path_str;	// current path string
  TABLPtr		lst_par_obj;	// parent object that is a list object
  TAPtr			reg_par_obj; 	// parent object that is *not* a list object
  TypeDef*		typ_par_obj; 	// parent object that is a typedef (get tokens)
  TAPtr			scope_ref;	// reference object for scoping

  TAPtr			sel_obj;	// current selected object
  String		sel_str; 	// string rep of current selection
  String_Array		items;		// the items in the list

  ivPatch*		body;		// entire glyph of dialog
  ivDialog*		dialog;		
  ivFileBrowser* 	fbrowser; 	// list of items
  ivFieldEditor* 	editor;

  virtual int	Choose();
  // main user interface: this actually puts up the dialog, returns true if Ok, false if cancel

  virtual void 	Build();	// called as constructed
  virtual void 	Clear();	// reset data
  virtual void 	Load();		// reload data
  virtual void 	ReRead();	// update browser for new parent

  virtual void	GetPathStr();	// get current path string
  virtual void  AddItem(const char* itm); // add one item to dialog
  virtual void	AddTokens(TypeDef* td); // add all tokens of given type
  virtual void	UpdateFmSelStr(); // update selection based on sel_str

  // callbacks
  virtual void 	AcceptBrowser();
  virtual void 	CancelBrowser();
  virtual void 	DescendBrowser();
  virtual void 	AcceptEditor(ivFieldEditor*);
  virtual void 	CancelEditor(ivFieldEditor*);

  taivObjChooser(TAPtr parob, ivWidgetKit*, ivStyle*, const char* captn, bool selonly=true);
  taivObjChooser(TypeDef* tpdf, ivWidgetKit*, ivStyle*, const char* captn);
  virtual ~taivObjChooser();
};

//////////////////////////////////
// 	taivFileButton		//
//////////////////////////////////

class taivFileButton : public taivData {
public:
  taivMenu* 	filemenu;	// has the menu options
  ivWindow*	cur_win;
  taivGetFile*	gf;
  bool		read_only;	// only reading streams is an option
  bool		write_only;	// only writing streams are available

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetImage(void* base, ivWindow* win);
  virtual void  GetImage();
  virtual void* GetValue();

  virtual void	GetGetFile();	// make sure we have a getfile..
  virtual void	Open();		// callback
  virtual void	Save();		// callback
  virtual void	SaveAs();	// callback
  virtual void	Append();	// callback
  virtual void	Close();	// callback
  virtual void	Edit();		// callback

  taivFileButton(void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL,
		  bool rd_only = false, bool wrt_only = false);
  ~taivFileButton();
};


//////////////////////////////////
// 	taivToken,Sub		//
//////////////////////////////////

class taivToken : public taivData {
  // for making menus of tokens
public:
  taivHierMenu* ta_menu;
  bool		ownflag;
  TypeDef*	typd;
  TAPtr		scope_ref;	// reference object for scoping
  ivWindow*	cur_win;
  bool		null_not;	// don't include null as an option
  bool		edit_not;	// don't include edit as an option
  bool		over_max;	// over max_menu
  TAPtr		chs_obj;	// return value for choosen object

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetImage(void* ths, TAPtr scp_obj, ivWindow* win);
  virtual void*	GetValue();		

  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenu_impl(TypeDef* td, taivMenuAction* actn = NULL);

  virtual void	Edit();		// for edit callback
  virtual void	Chooser();	// for chooser callback

  taivToken(int rt, int ft, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL,
            bool nul_not=false, bool edt_not=false);
  taivToken(taivHierMenu* existing_menu, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL,
	     bool nul_not=false, bool edt_not=false);
  ~taivToken();
};

class taivSubToken : public taivData {
  // Menu for sub tokens of a giventype
public:
  void*		menubase;
  bool		ownflag;
  TypeDef*	typd;
  ivWindow*	cur_win;
  taivMenu*	ta_menu;
  bool		null_ok;	// null is an option
  bool		edit_not;	// edit is not an option

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenuImpl(void* base,taivMenuAction* actn = NULL);

  virtual void	GetImage(void* ths, ivWindow* win,void* sel=NULL);
  virtual void*	GetValue();		

  virtual void	Edit();		// for edit callback

  taivSubToken(int rt, int ft, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL,
               bool nul_ok=false, bool edt_not=false);
  taivSubToken(taivHierMenu* existing_menu, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL,
		bool nul_ok=false, bool edt_not=false);
  ~taivSubToken();
};

//////////////////////////////////
// 	taivMemberDef		//
//////////////////////////////////

class taivMemberDefMenu : public taivData {
// Menu for memberdefs of a typedef in the object with a MDTYPE_xxx option
public:
  MemberDef*	md;
  void*		menubase;	// the address of the object
  TypeDef*	typd;		// the address of the typedef for the menu
  MemberSpace*  sp;
  ivWindow*	cur_win;
  taivMenu*	ta_menu;

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenuImpl(void* base,taivMenuAction* actn = NULL);

  virtual void 	GetImage(TypeDef* type, MemberDef* memb);
  virtual void 	GetImage(MemberSpace* space, MemberDef* memb);
  virtual void	GetImage(void* ths, ivWindow* win,void* sel=NULL);
  virtual void*	GetValue();		

  taivMemberDefMenu(int rt, int ft,MemberDef* m, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivMemberDefMenu();
};

//////////////////////////////////
// 	taivMethodDef		//
//////////////////////////////////

class taivMethodDefMenu : public taivData {
// Menu for memberdefs of a typedef in the object with a MDTYPE_xxx option
public:
  MethodDef*	md;
  void*		menubase;	// the address of the object
  TypeDef*	typd;		// the address of the typedef for the menu
  MethodSpace*  sp;
  ivWindow*	cur_win;
  taivMenu*	ta_menu;

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenuImpl(void* base,taivMenuAction* actn = NULL);

  virtual void 	GetImage(TypeDef* type, MethodDef* memb);
  virtual void 	GetImage(MethodSpace* space, MethodDef* memb);
  virtual void	GetImage(void* ths, ivWindow* win,void* sel=NULL);
  virtual void*	GetValue();		

  taivMethodDefMenu(int rt, int ft,MethodDef* m, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivMethodDefMenu();
};


//////////////////////////////////
// 	taivTypeHier		//
//////////////////////////////////

class taivTypeHier : public taivData {
// for menus of type hierarchy
public:
  taivHierMenu* ta_menu;
  bool		ownflag;
  TypeDef*	typd;
  bool		null_ok;

  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenu_impl(TypeDef* td, taivMenuAction* actn = NULL);
  virtual int	CountChildren(TypeDef* td);
  virtual bool	AddType(TypeDef* td); 	// true if type should be added

  ivGlyph*	GetRep();
  ivGlyph*	GetLook();

  virtual void		GetImage(TypeDef* ths);
  virtual TypeDef*	GetValue();

  taivTypeHier(int rt, int ft, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL, bool nul_ok=false);
  taivTypeHier(taivHierMenu* existing_menu, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL,
		bool nul_ok=false);
  ~taivTypeHier();
};

//////////////////////////////////
//	taivMethMenu,Button	//
//////////////////////////////////

// unlike real taivData, functions are not subject to updating
// so the constructor is the one that does all the work..

class cssClass;			// #IGNORE
class taivArgDialog;		// #IGNORE

class taivMethMenu : public taivData {
  // all representations of member functions must inherit from this one
public:
  void*		base;		// base is of parent type
  TypeDef*	typ;		// parent type
  MethodDef* 	meth;
  bool		is_menu_item;   // true if this is supposed to be a menu item (else ivGlyph*)
  cssClass*	args;
  taivArgDialog* arg_dlg;
  int		use_argc;

  taivMethMenu(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL);

  ivGlyph*	GetRep()		{ return NULL; }
  ivGlyph*	GetLook()		{ return NULL; }

  virtual void 	CallFun();		// call the function (button callback)
		
  virtual void  ShowReturnVal(cssEl* rval); // show return value after menu call
  virtual void	ApplyBefore();	// apply changes before performing menu call
  virtual void	UpdateAfter();	// update display after performing menu call
		
  virtual void	GenerateScript(); // output script code equivalent if recording
		
  virtual void 	AddToMenu(taivMenu* mnu);
};

class taivMethButton : public taivMethMenu {
  // button representation of a method
public:  
  ivGlyph*	rep;

  taivMethButton(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL); 
  ~taivMethButton();

  ivGlyph*	GetRep()		{ return rep; }
  ivGlyph*	GetLook();
};

class taivMethToggle : public taivMethMenu {
  // toggle representation of a method (does not call directly, but checks flag)
public:
  ivGlyph*	rep;
  taivToggle* 	tog;

  taivMethToggle(void* bs, MethodDef* md, TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL);
  ~taivMethToggle();

  ivGlyph*	GetRep()	{ return rep; }
  ivGlyph*	GetLook();

  void		CallFun();
};

#endif // taiv_data_h
