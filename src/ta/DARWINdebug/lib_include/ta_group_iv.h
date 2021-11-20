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

// InterViews interface for groups

#ifndef ta_group_iv_h
#define ta_group_iv_h 1

#include <ta/taiv_type.h>
#include <ta/taiv_dialog.h>
#include <ta/ta_group.h>

//////////////////////////
// 	Edit Buttons	//
//////////////////////////

class gpivListEditButton : public taivEditButton {
public:
  gpivListEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  void		SetLabel();
  // display the number of items in the list in the label 
};

class gpivGroupEditButton : public taivEditButton {
public:
  gpivGroupEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  void		SetLabel();
  // display the number of items in the group in the label 
};

// this one sets the name of the sub group..
class gpivSubEditButton : public taivEditButton {
public:
  String 	label;
  gpivSubEditButton(TypeDef* tp, void* base, ivWindow* win, const char* nm,
		     taivDialog* dlg=NULL, taivData* par=NULL);
  void		SetLabel();
};

// link groups don't have the option to create or xfer
class gpivLinkEditButton : public gpivGroupEditButton {
public:
  gpivLinkEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetMethMenus();	// exclude certain methods here..
};

// link lists don't have the option to create or xfer
class gpivListLinkEditButton : public gpivListEditButton {
public:
  gpivListLinkEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetMethMenus();	// exclude certain methods here..
};

class gpivArrayEditButton : public taivEditButton {
public:
  gpivArrayEditButton(TypeDef* tp, void* base, ivWindow* win, taivDialog* dlg=NULL, taivData* par=NULL);
  void		SetLabel();
  // display the number of items in the group in the label 
};



//////////////////////////
//     Element Menus 	//
//////////////////////////

class gpivListEls : public taivData {
  // menu of elements in the list
public:
  taivHierMenu* ta_menu;
  bool		ownflag;
  bool		null_ok;	// true if NULL is an option
  bool		no_lst;		// true if the list itself is not an option
  bool		edit_ok;	// true if edit is an option
  bool		over_max;	// if over max_menu
  TABLPtr	ths;
  TAPtr		chs_obj;	// object chosen by the chooser
  
  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenu_impl(TABLPtr lst, taivMenuAction* actn = NULL);
  
  ivGlyph*	GetRep();
  ivGlyph*	GetLook();
  void		GetImage(TABLPtr base_lst, TAPtr it);
  TAPtr		GetValue();

  virtual void	Edit();		// edit callback
  virtual void	Choose();	// chooser callback

  gpivListEls(int rt, int ft, TABLPtr lst, taivDialog* dlg=NULL, taivData* par=NULL,
	       bool nul_ok = false, bool nolst = false, bool edt_ok=false);
  gpivListEls(taivHierMenu* existing_menu, TABLPtr gp, taivDialog* dlg=NULL, taivData* par=NULL,
	       bool nul_ok = false, bool nolst = false, bool edt_ok=false);
  ~gpivListEls();
};

class gpivGroupEls : public gpivListEls {
  // menu of elements in the group
public:
  bool		no_in_gpmenu;	// true if not including MEMB_IN_GPMENU submenus
  
  virtual void	ChooseGp();	// chooser callback

  void		GetMenu_impl(TABLPtr cur_lst, taivMenuAction* actn = NULL);
  
  gpivGroupEls(int rt, int ft, TABLPtr lst, taivDialog* dlg=NULL, taivData* par=NULL,
		bool nul_itm = false, bool nogp = false, bool nogpm = false, 
		bool edt_ok=false);
  gpivGroupEls(taivHierMenu* existing_menu, TABLPtr gp, taivDialog* dlg=NULL, taivData* par=NULL,
		bool nul_itm = false, bool nogp = false, bool nogpm = false,
		bool edt_ok=false);
};

class gpivSubGroups : public taivData {
  // menu of sub-groups within a group
public:
  taivHierMenu* ta_menu;
  bool		ownflag;
  TAGPtr	ths;
  bool		null_ok;	// null is an option
  bool		edit_ok;	// edit is an option
  bool		over_max;	// if over max_menu
  TAGPtr	chs_obj;	// object chosen by the chooser
  
  virtual void	GetMenu(taivMenuAction* actn = NULL);
  virtual void	UpdateMenu(taivMenuAction* actn = NULL);
  virtual void	GetMenu_impl(TAGPtr gp, taivMenuAction* actn = NULL);
  
  ivGlyph*	GetRep();
  ivGlyph*	GetLook();
  void		GetImage(TAGPtr base_gp, TAGPtr gp);
  TAGPtr	GetValue();

  virtual void	Edit();		// edit callback
  virtual void	Choose();	// chooser callback

  gpivSubGroups(int rt, int ft, TAGPtr gp, taivDialog* dlg=NULL, taivData* par=NULL, bool nul_ok=false,
		 bool edt_ok=false);
  gpivSubGroups(taivHierMenu* existing_menu, TAGPtr gp, taivDialog* dlg=NULL, taivData* par=NULL,
		 bool nul_ok=false, bool edt_ok=false);
  ~gpivSubGroups();
};

// TypeHier provides the guts, we just replace the NULL default with "Group"
class gpivElTypes : public taivTypeHier {
public:
  TypeDef*	lst_typd;	// typedef of the list

  gpivElTypes(int rt, int ft, TypeDef* td, TypeDef* lstd, taivDialog* dlg=NULL, taivData* par=NULL);
  gpivElTypes(taivHierMenu* existing_menu, TypeDef* td, TypeDef* lstd, 
	       taivDialog* dlg=NULL, taivData* par=NULL);

  void		GetMenu(taivMenuAction* nact = NULL);
};

class gpivNewFuns : public taivData {
  // functions to call during New
public:
  taivDataList	funs;
  ivGlyph* 		rep;
  TypeDef*	 	typ;

  ivGlyph*		GetRep()	{ return rep; }
  ivGlyph*		GetLook();

  virtual void  	CallFuns(void* obj);

  gpivNewFuns(TypeDef* td, taivDialog* dlg=NULL, taivData* par=NULL);
  ~gpivNewFuns();
}; 

//////////////////////////////////
// 	 gpiv Dialogs		//
//////////////////////////////////

class gpivListNew : public taivDialog {
public:
  TABLPtr		ths;
  int			num;
  TypeDef*		typ;
  taivField*		num_rep;
  gpivElTypes* 	typ_rep;
  gpivNewFuns*		fun_list;
  ivPolyGlyph*		sub_box;

  gpivListNew(TABLPtr lst, TypeDef* td, int n_els=1);
  ~gpivListNew();

  void	Constr_Strings(const char* prompt="", const char* win_title="");
  void	Constr_Box();
  void	GetValue();

  virtual void	Constr_SubGpList()	{ };
  // hook for group new

  // this is the function you actually call..
  static TAPtr 	New(TABLPtr the_lst, TypeDef* td = NULL, ivWindow* win=NULL,
		    int n_els=1);
};

class gpivGroupNew : public gpivListNew {
public:
  TAGPtr		in_gp;		
  gpivSubGroups*	subgp_list;	

  gpivGroupNew(TAGPtr gp, TAGPtr init_gp, TypeDef* td, int n_els=1);
  ~gpivGroupNew();

  void	Constr_SubGpList();
  void	GetValue();

  // this is the function you actually call..
  static TAPtr 	New(TAGPtr the_gp,  TypeDef* td = NULL, TAGPtr init_gp = NULL,
		    ivWindow* win=NULL, int n_els=1);
};


////////////////////////////////
// 	gpivList Edits       //	
////////////////////////////////

class gpivList_ElData {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS contains data_els for one member of List
public:
  TypeDef*	typ;
  TAPtr		cur_base;
  taivDataList data_el;	// data elements
  
  gpivList_ElData(TypeDef* tp, TAPtr base);
  virtual ~gpivList_ElData();
};

class gpivList_ElDataList : public taPtrList<gpivList_ElData> {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
protected:
  void	El_Done_(void* it)	{ delete (gpivList_ElData*)it; }

public:
  ~gpivList_ElDataList()        { Reset(); }
};

class tbScrollBox;		// #IGNORE
class lrScrollBox;		// #IGNORE

class gpivListDialog : public taivEditDialog {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  gpivList_ElDataList lst_data_el;	// list of data elements
  TABLPtr		cur_lst;
  MemberSpace		lst_membs;	// list of members
  int			num_lst_fields; // number of editble list memberdefs with fields

  ivPolyGlyph*		lstel_rep;	// representation of List elements...
  lrScrollBox*		lst_data_g;	// list data glyphs
  ivPatch*		lst_data_patch;	// patch for data glyphs


  gpivListDialog(TypeDef* tp, void* base);
  gpivListDialog() 				{ };
  ~gpivListDialog();
  
  void		CloseWindow();	// close edit dialog window and free current data
  void		Constr_Dialog();
  void		Constr_Strings(const char* prompt="", const char* win_title="");
  void		Constr_Box();
  void		Constr_BodyBox();
  void		GetImage(); 
  void		GetValue();

  int		Edit();
  
  void		Constr_Labels();
  void		Constr_Data(); 
  bool		ShowMember(MemberDef* md);
  void		Scroll();

  virtual void	Constr_ListMembs(); 	// construct list members
  virtual void	Constr_ElData(); 	// construct list elements
  virtual void  Constr_ListData();  	// construct list members themselves

};

class gpivListEdit : public taivEdit {
public:  
  int 		Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  TAIV_EDIT_INSTANCE(gpivListEdit, taivEdit);
};


////////////////////////////////
// 	taivGroup Edits      //	
////////////////////////////////

class gpivGroupDialog : public gpivListDialog {
public:
  taivDataList	sub_data_el;	// list of data elements for sub groups
  tbScrollBox*		sub_data_g;	// data glyphs for sub groups

  gpivGroupDialog(TypeDef* tp, void* base);
  gpivGroupDialog() 				{ };
  ~gpivGroupDialog();
  
  void		CloseWindow();	// close edit dialog window and free current data
  void		Constr_Strings(const char* prompt="", const char* win_title="");
  void		GetImage(); 
  
  int		Edit();

  void		Constr_Data();
  void		Constr_Box();

  virtual void	Constr_SubGpData(); // construct data for subgroups
};

class gpivGroupEdit : public gpivListEdit {
public:  
  int 		Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  TAIV_EDIT_INSTANCE(gpivGroupEdit, gpivListEdit);
};


////////////////////////////////
// 	gpivArray Edits      //	
////////////////////////////////

class gpivArrayEditDialog : public taivEditDialog {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS
public:
  lrScrollBox*	ary_data_g;
  int		n_ary_membs;

  bool 		ShowMember(MemberDef* md);
  void 		GetValue();
  void 		GetImage();
  void		Constr_Data();
  void		Constr_Box();
  virtual void	Constr_AryData();
  void		Scroll();
  int		Edit();

  gpivArrayEditDialog(TypeDef *tp, void* base);
  gpivArrayEditDialog() 		{ };
};

class gpivArrayEdit : public taivEdit {
public:
  int 		Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  TAIV_EDIT_INSTANCE(gpivArrayEdit, taivEdit);
};

//////////////////////////////////////////////////////////
// 		taivType: 	Lists & Groups		//
//////////////////////////////////////////////////////////

class gpivListType : public taivClassType {
public:  
  int 		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(gpivListType, taivClassType);
};

class gpivGroupType : public gpivListType {
public:  
  int 		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(gpivGroupType, gpivListType);
};


//////////////////////////////////////////////////////////
// 		taivType: 	Arrays			//
//////////////////////////////////////////////////////////

class gpivArray_Type : public taivClassType {
public:  
  int 		BidForType(TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetValue(taivData* dat, void* base);

  TAIV_TYPE_INSTANCE(gpivArray_Type, taivClassType);
};


//////////////////////////////////////////////////////////
// 		taivMembers: 	 Groups			//
//////////////////////////////////////////////////////////


class taivROListMember: public taivROMember {
// allows one to view a listing of the members of a ReadOnly object that is a list or array
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);
       
  TAIV_MEMBER_INSTANCE(taivROListMember, taivROMember);
};


class taivROGroupMember: public taivROMember {
// allows one to view a listing of the members of a ReadOnly object that is a Group or array
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);
       
  TAIV_MEMBER_INSTANCE(taivROGroupMember, taivROMember);
};


class taivROArrayMember: public taivROMember {
// allows one to view a listing of the members of a ReadOnly object that is a array or array
public:
  int		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);
       
  TAIV_MEMBER_INSTANCE(taivROArrayMember, taivROMember);
};


class gpivDefaultEl : public taivMember {
public:  
  int 		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL); 
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(gpivDefaultEl, taivMember);
};

class gpivLinkGP : public taivMember {
public:  
  int 		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL); 
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(gpivLinkGP, taivMember);
};

class gpivLinkList : public taivMember {
public:  
  int 		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL); 
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  TAIV_MEMBER_INSTANCE(gpivLinkList, taivMember);
};

class gpivFromGpTokenPtrMember : public taivTokenPtrMember {
public:  
  int 		BidForMember(MemberDef* md, TypeDef* td);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL); 
  void		GetImage(taivData* dat, void* base, ivWindow* win);
  void		GetMbrValue(taivData* dat, void* base, bool& first_diff);

  virtual MemberDef*	GetFromMd();
  virtual TABLPtr	GetList(MemberDef* from_md, void* base);

  TAIV_MEMBER_INSTANCE(gpivFromGpTokenPtrMember, taivTokenPtrMember);
};



//////////////////////////////////////////////////
// 	 taivArgTypes:   Lists and Groups	//
//////////////////////////////////////////////////

class gpivTAPtrArgType : public taivTokenPtrArgType {
  // for taBase pointers in groups, sets the typedef to be the right one..
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  TAIV_ARGTYPE_INSTANCE(gpivTAPtrArgType, taivTokenPtrArgType);
};

class gpivInObjArgType : public gpivTAPtrArgType {
  // for taBase pointers in groups with ARG_IN_OBJ
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);
  
  TAIV_ARGTYPE_INSTANCE(gpivInObjArgType, gpivTAPtrArgType);
};

class gpivFromGpArgType : public taivTokenPtrArgType {
  // for taBase pointers with FROM_GROUP_xxx
public:
  int 		BidForArgType(int aidx, TypeDef* argt, MethodDef* md, TypeDef* td);
  cssEl*	GetElFromArg(const char* arg_nm, void* base);
  taivData*	GetDataRep(ivInputHandler* ih, taivDialog* dlg=NULL, taivData* par=NULL);
  void		GetImage(taivData* dat, void* base, ivWindow* win); 
  void		GetValue(taivData* dat, void* base);

  virtual MemberDef*	GetFromMd();
  virtual TABLPtr	GetList(MemberDef* from_md, void* base);
  
  TAIV_ARGTYPE_INSTANCE(gpivFromGpArgType, taivTokenPtrArgType);
};


//////////////////////////////////
//	select edit dialog	//
//////////////////////////////////

class SelectEditConfig : public taBase {
  // #EDIT_INLINE ##NO_TOKENS special parameters for controlling the select edit display
public:
  bool		auto_edit;	// automatically bring up edit dialog upon loading
  String_Array	mbr_labels;	// extra labels at the start of each member label for the selected fields
  String_Array	meth_labels;	// extra labels at the start of each method label for the selected functions

  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	Copy_(const SelectEditConfig& cp);
  COPY_FUNS(SelectEditConfig, taBase);
  TA_BASEFUNS(SelectEditConfig);
};

class SelectEdit : public taNBase {
  // ##EXT_edit Selectively edit members from different objects
public:
  SelectEditConfig config;	// special parameters for controlling the display
  bool		edit_on_reopen;	// #READ_ONLY #NO_SAVE if true, edit was open on BaseChangeSave, so edit on ReOpen

  taBase_List	mbr_bases;	// #LINK_GROUP #READ_ONLY #AKA_bases the bases for each element in the list
  String_Array	mbr_strs;	// #READ_ONLY #AKA_member_strs string names of mbrs on bases -- used for saving
  MemberSpace	members;	// #READ_ONLY #NO_SAVE member defs
  String_Array	mbr_base_paths; // #READ_ONLY #NO_SAVE paths to base objects for BaseChangeSave

  taBase_List	meth_bases;	// #LINK_GROUP #READ_ONLY the bases for each element in the list
  String_Array	meth_strs;	// #READ_ONLY string names of meths on bases -- used for saving
  MethodSpace	methods;	// #READ_ONLY #NO_SAVE method defs
  String_Array	meth_base_paths; // #READ_ONLY #NO_SAVE paths to base objects for BaseChangeSave

  virtual int	FindMbrBase(TAPtr base, MemberDef* md);
  // find a given base and member, returns index
  virtual bool	SelectMember(TAPtr base, MemberDef* md, const char* lbl);
  // add new member to edit if it isn't already here (returns true), otherwise remove (returns false)
  virtual bool	SelectMemberNm(TAPtr base, const char* md, const char* lbl);
  // add new member to edit if it isn't already here (returns true), otherwise remove (returns false)

  virtual int	FindMethBase(TAPtr base, MethodDef* md);
  // find a given base and method, returns index
  virtual bool	SelectMethod(TAPtr base, MethodDef* md, const char* lbl);
  // add new method to edit if it isn't already here (returns true), otherwise remove (returns false)
  virtual bool	SelectMethodNm(TAPtr base, const char* md, const char* lbl);
  // add new method to edit if it isn't already here (returns true), otherwise remove (returns false)

  virtual void	UpdateAllBases();	// perform update-after-edit on all base objects

  virtual void	RemoveField(int idx);
  // #MENU #MENU_ON_SelectEdit remove edit data item at given index
  virtual void	MoveField(int from, int to);
  // #MENU move member to edit from index to index

  virtual void	RemoveFun(int idx);
  // #MENU #MENU_SEP_BEFORE remove function at given index
  virtual void	MoveFun(int from, int to);
  // #MENU move function to edit from index to index

  virtual void	NewEdit();
  // #MENU #MENU_SEP_BEFORE closes current edit dialog and makes a new one (with any changes)

  virtual void	GetMembsFmStrs(); // get members from strings (upon loading)
  virtual void	GetMethsFmStrs(); // get methods from strings (upon loading)
  virtual void	GetAllPaths();	// get paths for all current objects

  virtual bool	BaseClosing(TAPtr base);
  // this base object is about to be closed (removed), if i edit it, then I need to save and reopen (returns true if edited)
  static bool	BaseClosingAll(TAPtr base);
  // calls base closing on all SelectEdit tokens..
  virtual void	BaseChangeSave(); // close edit dialog and save paths to current bases
  virtual void	BaseChangeReOpen(); // re-open the edit dialog loading bases from saved paths

  virtual String GetMbrLabel(int idx); // get full label for member
  virtual String GetMethLabel(int idx);	// get full label for method

  int	Dump_Load_Value(istream& strm, TAPtr par=NULL);
  // reset everything before loading
  int	Dump_Save_Value(ostream& strm, TAPtr par=NULL, int indent = 0);
  // get paths before saving

  void	UpdateAfterEdit();
  void	Initialize();
  void 	InitLinks();
  void	Copy_(const SelectEdit& cp);
  COPY_FUNS(SelectEdit, taNBase);
  TA_BASEFUNS(SelectEdit);
};

class gpivSelectEdit : public taivEdit {
public:  
  int 		Edit(void* base, ivWindow* win=NULL, bool wait=false,bool readonly=false, const ivColor* bgclr = NULL);
  int		BidForEdit(TypeDef* td);
  TAIV_EDIT_INSTANCE(gpivSelectEdit, taivEdit);
};

class gpivSelectEditDialog : public taivEditDialog {
  // ##NO_TOKENS ##NO_CSS ##NO_MEMBERS edit only selected items from a range of ta-base objects
public:
  SelectEdit*	sele;

  gpivSelectEditDialog(TypeDef* td, void* base);
  gpivSelectEditDialog()		{ };
  ~gpivSelectEditDialog();

  bool ShowMember(MemberDef* md);

  void	Constr_Labels_impl(const MemberSpace& ms);
  void	Constr_Data_impl(const MemberSpace& ms, taivDataList& dl);

  void	GetValue(); 
  void	GetImage_impl(const MemberSpace& ms, const taivDataList& dl,
		      void* base, ivWindow* win);
  void	GetValue_impl(const MemberSpace& ms, const taivDataList& dl,void* base);

  void Constr_Methods();
};

#endif // ta_group_iv.h

