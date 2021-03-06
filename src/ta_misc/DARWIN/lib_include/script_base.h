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

// script_base.h: basic types for script-managing objects
  
#ifndef script_base_h
#define script_base_h 1

#include <ta_misc/win_base.h>
#include <ta_misc/ta_file.h>
#include <ta/ta_group_iv.h>

class cssProgSpace;		// #NO_INSTANCE

class ScriptBase {
  // class for adding a script to other objects
public:
  cssProgSpace* script;			// #HIDDEN the script, if defined
  taFile	script_file;		// file to use for the script
  String	script_string;
  // script code to be run, instead of loading from file

  virtual bool	HasScript();
  // returns true if we have some kind of script file or string to run
  virtual bool  RunScript();
  // run the script (returns false for no scr)
  virtual void 	SetScript(const char* file_nm);
  // set the script file (e.g. from the script)
  virtual void	LoadScript(const char* file_nm = NULL);
  // #MENU #LABEL_Compile #MENU_ON_Actions #ARGC_0 compile script from script file into internal runnable format
  virtual void	LoadScriptString(const char* string = NULL);
  // load and recompile the script string
  virtual void	InteractScript();
  // #MENU #LABEL_Interact change to this shell in script (terminal) window to interact, debug etc script

  virtual void	InstallThis();
  // #IGNORE install the this object in the script typespace
  virtual void	UpdateReCompile();
  // #IGNORE recompile if reselected by user
  virtual TypeDef* GetThisTypeDef()	{ return &TA_Script; }
  // #IGNORE overload this function to get the typedef of 'this' object, which must be taBase
  virtual void*	GetThisPtr()		{ return (void*)this; }
  // #IGNORE overload this function to get the 'this' pointer for object (must be taBase)

  ScriptBase();
  virtual ~ScriptBase();
};

class ScriptBase_List : public taPtrList<ScriptBase> {
  // ##NO_TOKENS ##NO_UPDATE_AFTER list of script base objects (doesn't own anything)
public:
  ~ScriptBase_List()	{ Reset(); }
};

class SArg_Array : public String_Array {
  // string argument array: has labels for each argument to make it easier in the interface
public:
  String_Array	labels;		// labels for each argument

  int		Dump_Save_Value(ostream& strm, TAPtr par=NULL, int indent = 0);
  int		Dump_Load_Value(istream& strm, TAPtr par=NULL);

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  void	InitLinks();
  void	Copy_(const SArg_Array& cp);
  COPY_FUNS(SArg_Array, String_Array);
  TA_BASEFUNS(SArg_Array);
};

class SArgEditDialog : public gpivArrayEditDialog {
  // ##NO_INSTANCE
public:
  bool 		ShowMember(MemberDef* md);
  void		Constr_AryData();

  SArgEditDialog(TypeDef *tp, void* base);
  SArgEditDialog() 	{ };
};

class SArgEdit : public gpivArrayEdit {
public:
  int 		Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  TAIV_EDIT_INSTANCE(SArgEdit, gpivArrayEdit);
};

class Script : public taNBase, public ScriptBase {
  // ##EXT_scr an object for maintaining and running arbitrary scripts
public:
  static ScriptBase_List recompile_scripts; // #HIDDEN list of scripts to be recompiled in wait proc
  static bool		 Wait_RecompileScripts(); // wait process for recompiling

  bool		recording;	// #READ_ONLY #NO_SAVE currently recording?
  bool		auto_run;	// run automatically at startup?
  SArg_Array	s_args;		// string-valued arguments to pass to script

  virtual bool  Run();
  // #BUTTON #GHOST_OFF_recording run the script (returns false for no scr)
  virtual void	Record(const char* file_nm = NULL);
  // #BUTTON #GHOST_OFF_recording #ARGC_0 #NO_SCRIPT record script code for interface actions
  virtual void	StopRecording();
  // #BUTTON #LABEL_StopRec #GHOST_ON_recording stop recording script code
  virtual void	Interact();
  // #BUTTON #GHOST_OFF_recording change to this shell in script (terminal) window to interact, debug etc script
  virtual void	Clear();
  // #BUTTON #CONFIRM clear script file
  virtual void	Compile();
  // #BUTTON #GHOST_OFF_recording compile script from script file into internal runnable format

  virtual void	ScriptAllWinPos();
  // #MENU #MENU_ON_Actions #NO_SCRIPT record script code to set window positions, iconified

  virtual void	AutoRun();
  // run this script if auto_run is set

  void		InstallThis();
  TypeDef*	GetThisTypeDef()	{ return GetTypeDef(); }
  void*		GetThisPtr()		{ return (void*)this; }

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const Script& cp);
  COPY_FUNS(Script, taNBase);
  TA_BASEFUNS(Script);
};

class Script_MGroup : public MenuGroup_impl {
public:
  virtual void	StopRecording();
  virtual void	AutoRun();

  void 		GetMenu_impl();		// add a 'run' menu..

  // more callbacks
  virtual void		Run_mc(taivMenuEl* sel);

  void	Initialize();
  void 	Destroy()		{ };
  TA_BASEFUNS(Script_MGroup);
};

#endif // script_base_h


