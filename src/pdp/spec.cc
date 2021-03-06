/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the PDP++ software package.			      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted	      //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//									      //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions. 	HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
// 									      //
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

// spec.cc

#include <pdp/spec.h>
#include <ta/ta_group_iv.h>

//////////////////////////////////
//	BaseSpec_MGroup		//
//////////////////////////////////

void BaseSpec_MGroup::Initialize() {
  SetBaseType(&TA_BaseSpec);
}

const ivColor* BaseSpec_MGroup::GetEditColor() {
  return pdpMisc::GetObjColor(GET_MY_OWNER(Project), el_base);
}

BaseSpec* BaseSpec_MGroup::FindSpecType(TypeDef* td) {
  // breadth-first search
  BaseSpec* bs;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(bs->GetTypeDef() == td)
      return bs;    // use equals to find type of spec object
  }
  // then check the children
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(!(bs->InheritsFrom(td) || td->InheritsFrom(bs->GetTypeDef())))
      continue;			// no hope..
    BaseSpec* rval = bs->children.FindSpecType(td);
    if(rval != NULL)
      return rval;
  }
  return NULL;
}

BaseSpec* BaseSpec_MGroup::FindSpecInherits(TypeDef* td, TAPtr for_obj) {
  // breadth-first search
  BaseSpec* bs;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(bs->InheritsFrom(td)) {
      if((for_obj == NULL) || (bs->CheckObjectType_impl(for_obj)))
	return bs;    // object must also be sufficient..
    }
  }
  // then check the children
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(!(bs->InheritsFrom(td) || td->InheritsFrom(bs->GetTypeDef())))
      continue;			// no hope..
    BaseSpec* rval = bs->children.FindSpecInherits(td, for_obj);
    if(rval != NULL)
      return rval;
  }
  return NULL;
}

BaseSpec* BaseSpec_MGroup::FindSpecTypeNotMe(TypeDef* td, BaseSpec* not_me) {
  // breadth-first search
  BaseSpec* bs;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if((bs->GetTypeDef() == td) && (bs != not_me))
      return bs;    // use equals to find type of spec object
  }
  // then check the children
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(!(bs->InheritsFrom(td) || td->InheritsFrom(bs->GetTypeDef()) || (bs != not_me)))
      continue;			// no hope..
    BaseSpec* rval = bs->children.FindSpecTypeNotMe(td, not_me);
    if(rval != NULL)
      return rval;
  }
  return NULL;
}

BaseSpec* BaseSpec_MGroup::FindSpecInheritsNotMe(TypeDef* td, BaseSpec* not_me, TAPtr for_obj) {
  // breadth-first search
  BaseSpec* bs;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(bs->InheritsFrom(td) && (bs != not_me)) {
      if((for_obj == NULL) || (bs->CheckObjectType_impl(for_obj)))
	return bs;    // object must also be sufficient..
    }
  }
  // then check the children
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    if(!(bs->InheritsFrom(td) || td->InheritsFrom(bs->GetTypeDef()) || (bs != not_me)))
      continue;			// no hope..
    BaseSpec* rval = bs->children.FindSpecInheritsNotMe(td, not_me, for_obj);
    if(rval != NULL)
      return rval;
  }
  return NULL;
}


BaseSpec* BaseSpec_MGroup::FindSpecName(const char* nm) {
  int idx;
  BaseSpec* rval = (BaseSpec*)FindLeafName((char*)nm, idx);
  if(rval != NULL)
    return rval;
  BaseSpec* bs;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, bs, this->, i) {
    rval = bs->children.FindSpecName(nm);
    if(rval != NULL)
      return rval;
  }
  return NULL;
}

BaseSpec* BaseSpec_MGroup::FindParent() {
  return GET_MY_OWNER(BaseSpec);
}

// setup callbacks
declare_taivMenuCallback(BaseSpec_MGroup)
implement_taivMenuCallback(BaseSpec_MGroup)

void BaseSpec_MGroup::GetMenu_impl() {
  PDPMGroup::GetMenu_impl();
  
  int cur_no = ta_menu->cur_sno;
  ta_menu->AddSep();

  ta_menu->AddSubMenu("New Child");
  itm_list->GetMenu(new taivMenuCallback(BaseSpec_MGroup)
		    (this, &BaseSpec_MGroup::NewChildSpec_mc));
  ta_menu->SetSub(cur_no);
}

void BaseSpec_MGroup::NewChildSpec_mc(taivMenuEl* sel) {
  if(win_owner == NULL) return;
  if((sel != NULL) && (sel->usr_data != NULL)) {
    TAPtr itm = (TAPtr)sel->usr_data;
    if(itm->InheritsFrom(TA_BaseSpec)) {
      // call with 0 causes menu to come up..
      TAPtr rval = ((BaseSpec*)itm)->children.New(0);
      if(rval != NULL) {
	winbMisc::DelayedMenuUpdate(this);
	itm->UpdateAfterEdit();
      }
    }
  }
}

bool BaseSpec_MGroup::nw_itm_def_arg = false;

BaseSpec* BaseSpec_MGroup::FindMakeSpec(const char* nm, TypeDef* tp, bool& nw_itm, const char* alt_nm) {
  nw_itm = false;
  BaseSpec* sp = NULL;
  if(nm != NULL) {
    sp = (BaseSpec*)FindName(nm);
    if((sp == NULL) && (alt_nm != NULL)) {
      sp = (BaseSpec*)FindName(alt_nm);
      if(sp != NULL) sp->name = nm;
    }
  }
  else {
    sp = (BaseSpec*)FindType(tp);
  }
  if(sp == NULL) {
    sp = (BaseSpec*)NewEl(1, tp);
    if(nm != NULL)
      sp->name = nm;
    nw_itm = true;
  }
  else if(!sp->InheritsFrom(tp)) {
    Remove(sp);
    sp = (BaseSpec*)NewEl(1, tp);
    if(nm != NULL)
      sp->name = nm;    
    nw_itm = true;
  }
  return sp;
}

bool BaseSpec_MGroup::RemoveSpec(const char* nm, TypeDef* tp) {
  if(nm != NULL)
    return RemoveName(nm);

  int idx = Find(tp);
  if(idx >= 0)
    return Remove(idx);
  return false;
}

bool BaseSpec::nw_itm_def_arg = false;

BaseSpec* BaseSpec::FindMakeChild(const char* nm, TypeDef* td, bool& nw_itm, const char* alt_nm) {
  if(td == NULL) td = children.el_typ;
  return children.FindMakeSpec(nm, td, nw_itm, alt_nm);
}

bool BaseSpec::RemoveChild(const char* nm, TypeDef* td) {
  if(td == NULL) td = children.el_typ;
  return children.RemoveSpec(nm, td);
}

//////////////////////////////////
//	BaseSpec		//
//////////////////////////////////

void BaseSpec::Initialize() {
  not_used_ok = false;
  min_obj_type = &TA_taBase;
  int i;
  for(i=0; i< MAX_SPEC_LONGS; i++)
    unique[i] = 0;
}

void BaseSpec::Copy_(const BaseSpec& cp) {
  //  min_obj_type = cp.min_obj_type;  // don't do this -- could be going between types
  int i;
  for(i=0; i< MAX_SPEC_LONGS; i++)
    unique[i] = cp.unique[i];
  children = cp.children;
}

void BaseSpec::Destroy() {
  CutLinks();
}

void BaseSpec::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(children, this);
  children.SetBaseType(GetTypeDef());
  if(!taMisc::is_loading)
    UpdateSpec();
}

void BaseSpec::CutLinks() {
  children.CutLinks();
  taNBase::CutLinks();
}

void BaseSpec::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  UpdateSpec();
}

BaseSpec* BaseSpec::NewChild() {
  BaseSpec* rval = (BaseSpec*)children.NewEl(1);
  rval->UpdateSpec();
  return rval;
}

BaseSpec* BaseSpec::FindParent() {
  return GET_MY_OWNER(BaseSpec);
}

void BaseSpec::SetUnique(char* memb_nm, bool onoff) {
  MemberDef* md = FindMember(memb_nm);
  if(md != NULL)
    SetUnique(md->idx, onoff);
}

bool BaseSpec::GetUnique(char* memb_nm) {
  MemberDef* md = FindMember(memb_nm);
  if(md != NULL)
    return GetUnique(md->idx);
  return false;
}

void BaseSpec::SetUnique(int memb_no, bool onoff) {
  if(memb_no < TA_BaseSpec.members.size)
    return;
  memb_no -= TA_BaseSpec.members.size; // always relative to the evespec..
  int lng_no = memb_no / (sizeof(long) * 8);
  int bit_no = memb_no % (sizeof(long) * 8);
  if(lng_no >= MAX_SPEC_LONGS) { 
    taMisc::Error("Number of members", String(memb_no), "exceeds spec limit in",
	     GetTypeDef()->name, "of", String(256));
    return;
  }
  if(onoff)
    unique[lng_no] |= 1 << bit_no;
  else
    unique[lng_no] &= ~(1 << bit_no);
}

bool BaseSpec::GetUnique(int memb_no) {
  if(memb_no < TA_BaseSpec.members.size)
    return false;
  memb_no -= TA_BaseSpec.members.size; // always relative to the evespec..
  int lng_no = memb_no / (sizeof(long) * 8);
  int bit_no = memb_no % (sizeof(long) * 8);
  if(lng_no >= MAX_SPEC_LONGS) { 
    taMisc::Error("Number of members", String(memb_no), "exceeds spec limit in",
	     GetTypeDef()->name, "of", String(256));
    return false;
  }
  return unique[lng_no] & (1 << bit_no) ? 1 : 0;
}

void BaseSpec::UpdateSpec() {
  BaseSpec* parent = FindParent();
  if(parent != NULL) {
    TypeDef* td = GetTypeDef();
    int i;
    for(i=TA_BaseSpec.members.size; i< td->members.size; i++)
      UpdateMember(parent, i);
  }
  UpdateSubSpecs();
  UpdateChildren();
}

void BaseSpec::UpdateMember(BaseSpec* from, int memb_no) {
  if((from == NULL) || (memb_no < TA_BaseSpec.members.size))
    return;
  TypeDef* td = GetTypeDef();
  TypeDef* frm_td = from->GetTypeDef();
  if(memb_no < frm_td->members.size) {	// parent must have this member
    MemberDef* md = td->members[memb_no];
    if(frm_td->members[memb_no] == md) { 	// must be the same member
      // don't copy read only or hidden members! (usually set automatically
      // and might depend on local variables)
      if(!GetUnique(memb_no) &&
	 !(md->HasOption("READ_ONLY") || md->HasOption("HIDDEN") ||
	   md->HasOption("NO_INHERIT")))
      {
	if(md->type->InheritsFrom(TA_taList_impl)) {
	  ((taList_impl*)md->GetOff((void*)this))->EnforceSize
	    (((taList_impl*)md->GetOff((void*)from))->size);
	}
	if(md->type->InheritsFrom(TA_taArray_impl)) {
	  ((taArray_impl*)md->GetOff((void*)this))->EnforceSize
	    (((taArray_impl*)md->GetOff((void*)from))->size);
	} 
	MemberCopyFrom(memb_no, from);
	tabMisc::NotifyEdits(this);
      }
    }
  }
}

void BaseSpec::UpdateChildren() {
  BaseSpec* kid;
  taLeafItr i;
  FOR_ITR_EL(BaseSpec, kid, children., i) {
    bool tmp_nuo = kid->not_used_ok;
    kid->not_used_ok = true;
    kid->UpdateAfterEdit();
    kid->not_used_ok = tmp_nuo;
  }
}

bool BaseSpec::CheckType(TypeDef* td) {
  if(td == NULL) {
    taMisc::Error("*** For spec:", name, ", NULL type.",
                  "should be at least:", min_obj_type->name);
    return false;
  }
  if(!CheckType_impl(td)) {
    taMisc::Error("*** For spec:", name, ", incorrect type:", td->name,
                   "should be at least:", min_obj_type->name);
    return false;
  }
  return true;
}
  
bool BaseSpec::CheckObjectType(TAPtr obj) {
  if(obj == NULL) {
    taMisc::Error("*** For spec:", name, ", NULL Object.",
		  "should be at least:", min_obj_type->name);
    return false;
  }
  if(!CheckObjectType_impl(obj)) {
    taMisc::Error("*** For spec:", name, ", incorrect type of obj:", obj->GetPath(),
		   "of type:", obj->GetTypeDef()->name,
		   "should be at least:", min_obj_type->name);
    return false;
  }
  return true;
}
  
bool BaseSpec::CheckType_impl(TypeDef* td) {
  // other specs are allowed to own any kind of other spec,
  // and layers and projections also contain specs..
  if(td->InheritsFrom(TA_BaseSpec) ||
     (td->InheritsFrom(TA_Projection) && InheritsFrom(TA_ConSpec)) ||
     (td->InheritsFrom(TA_Layer) && InheritsFrom(TA_UnitSpec)))
    return true;

  if(!td->InheritsFrom(min_obj_type))
    return false;

  return true;
}

bool BaseSpec::CheckObjectType_impl(TAPtr obj) {
  // other specs are allowed to own any kind of other spec,
  // and layers and projections also contain specs..
  if(obj->InheritsFrom(TA_BaseSpec) ||
     (obj->InheritsFrom(TA_Projection) && InheritsFrom(TA_ConSpec)) ||
     (obj->InheritsFrom(TA_Layer) && InheritsFrom(TA_UnitSpec)))
    return true;

  if(!obj->InheritsFrom(min_obj_type))
    return false;

  return true;
}

int BaseSpec::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  strm << " {\n";
  String tmpstr = "unique";
  taMisc::fmt_sep(strm, tmpstr, 0, indent+1, 1);
  strm << " = {";
  
  TypeDef* td = GetTypeDef();
  int i;
  int cnt=0;
  for(i=TA_BaseSpec.members.size; i< td->members.size; i++) {
    if(GetUnique(i)) {
      strm << td->members[i]->name << "; ";
      cnt++;
      if((cnt % 6) == 0) {
	strm << "\n";
	taMisc::indent(strm, indent+2, 1);
      }
    }
  }
  strm << "};\n";

  GetTypeDef()->members.Dump_Save(strm, (void*)this, (void*)par, indent+1);

  return true;
}


int BaseSpec::Dump_Load_Value(istream& strm, TAPtr par) {
  int c = taMisc::skip_white(strm);
  if(c == EOF)    return EOF;
  if(c == ';')    return 2;  // signal that just a path was loaded..
  if(c == '}') {
    if(strm.peek() == ';') strm.get();
    return 2;
  }

  if(c != '{') {
    taMisc::Error("*** Missing '{' in dump file for spec object:",name);
    return false;
  }

  // this is now like MemberSpace::Dump_Load
  c = taMisc::read_word(strm, true);
  if(c == EOF) return EOF;
  if(c == '}') {
    strm.get();			// get the bracket (above was peek)
    if(strm.peek() == ';') strm.get(); // skip past ending semi
    return 2;
  }

  String mb_name = taMisc::LexBuf;
  if(mb_name == "unique") {
    // first turn all others off, since default is off
    int i;
    for(i=TA_BaseSpec.members.size; i<GetTypeDef()->members.size; i++)
      SetUnique(i, false);
    while(true) {
      c = taMisc::skip_white(strm);
      if(c == '=') {
	c = taMisc::read_till_lb_or_semi(strm);
	if(c == '{') {
	  while(true) {
	    c = taMisc::read_till_rb_or_semi(strm);
	    if(c == EOF)  return EOF;
	    if(c == '}')	break;
	    if(c == ';')
	      SetUnique((char*)taMisc::LexBuf, true);
	  }
	  if(c == EOF)	return EOF;
	  break;		// successful
	}
      }
      taMisc::Error("*** Bad 'unique' Formatting in dump file for type:",
		      GetTypeDef()->name);
      taMisc::skip_past_err(strm);
      break;
    }
  }
  else {			// not 'unique' so pass it to std function
    return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par,
					   (const char*)mb_name, c);
  }
  return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par);
}

//////////////////////////////////
//	BaseSubSpec		//
//////////////////////////////////

void BaseSubSpec::Initialize() {
  int i;
  for(i=0; i< MAX_SPEC_LONGS; i++)
    unique[i] = 0;
}

void BaseSubSpec::Copy_(const BaseSubSpec& cp) {
  int i;
  for(i=0; i< MAX_SPEC_LONGS; i++)
    unique[i] = cp.unique[i];
}

void BaseSubSpec::Destroy() {
}

void BaseSubSpec::InitLinks() {
  taNBase::InitLinks();
  if(!taMisc::is_loading)
    UpdateSpec();
}

void BaseSubSpec::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  UpdateSpec();
}

BaseSpec* BaseSubSpec::FindParentBaseSpec() {
  return GET_MY_OWNER(BaseSpec);
}

BaseSubSpec* BaseSubSpec::FindParent() {
  BaseSpec* bso = FindParentBaseSpec();
  if(bso == NULL)	return NULL;
  BaseSpec* bsoo = bso->FindParent(); // parent's owner
  if(bsoo == NULL)	return NULL;

  String my_path = GetPath(NULL, bso); // get my path to owner..
  MemberDef* md;
  BaseSubSpec* from = (BaseSubSpec*)bsoo->FindFromPath(my_path, md);
  if((from == NULL) || !from->InheritsFrom(TA_BaseSubSpec))
    return NULL;			// corresponding subspec object not found..
  return from;
}

void BaseSubSpec::SetUnique(char* memb_nm, bool onoff) {
  MemberDef* md = FindMember(memb_nm);
  if(md != NULL)
    SetUnique(md->idx, onoff);
}

bool BaseSubSpec::GetUnique(char* memb_nm) {
  MemberDef* md = FindMember(memb_nm);
  if(md != NULL)
    return GetUnique(md->idx);
  return false;
}

void BaseSubSpec::SetUnique(int memb_no, bool onoff) {
  if(memb_no < TA_BaseSubSpec.members.size)
    return;
  memb_no -= TA_BaseSubSpec.members.size; // always relative to the evespec..
  int lng_no = memb_no / (sizeof(long) * 8);
  int bit_no = memb_no % (sizeof(long) * 8);
  if(lng_no >= MAX_SPEC_LONGS) { 
    taMisc::Error("Number of members", String(memb_no), "exceeds spec limit in",
	     GetTypeDef()->name, "of", String(256));
    return;
  }
  if(onoff)
    unique[lng_no] |= 1 << bit_no;
  else
    unique[lng_no] &= ~(1 << bit_no);
}

bool BaseSubSpec::GetUnique(int memb_no) {
  if(memb_no < TA_BaseSubSpec.members.size)
    return false;
  memb_no -= TA_BaseSubSpec.members.size; // always relative to the evespec..
  int lng_no = memb_no / (sizeof(long) * 8);
  int bit_no = memb_no % (sizeof(long) * 8);
  if(lng_no >= MAX_SPEC_LONGS) { 
    taMisc::Error("Number of members", String(memb_no), "exceeds spec limit in",
	     GetTypeDef()->name, "of", String(256));
    return false;
  }
  return unique[lng_no] & (1 << bit_no) ? 1 : 0;
}

void BaseSubSpec::UpdateSpec() {
  BaseSubSpec* parent = FindParent();
  if(parent != NULL) {
    TypeDef* td = GetTypeDef();
    int i;
    for(i=TA_BaseSubSpec.members.size; i< td->members.size; i++)
      UpdateMember(parent, i);
  }
}

void BaseSubSpec::UpdateMember(BaseSubSpec* from, int memb_no) {
  if((from == NULL) || (memb_no < TA_BaseSubSpec.members.size))
    return;
  TypeDef* td = GetTypeDef();
  TypeDef* frm_td = from->GetTypeDef();
  if(memb_no < frm_td->members.size) {	// parent must have this member
    MemberDef* md = td->members[memb_no];
    if(frm_td->members[memb_no] == md) { 	// must be the same member
      // don't copy read only or hidden members! (usually set automatically
      // and might depend on local variables)
      if(!GetUnique(memb_no) &&
	 !(md->HasOption("READ_ONLY") || md->HasOption("HIDDEN") || 
	   md->HasOption("NO_INHERIT")))
      {
	MemberCopyFrom(memb_no, from);
	tabMisc::NotifyEdits(this);
      }
    }
  }
}

int BaseSubSpec::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  strm << " {\n";
  String tmpstr = "unique";
  taMisc::fmt_sep(strm, tmpstr, 0, indent+1, 1);
  strm << " = {";
  
  TypeDef* td = GetTypeDef();
  int i;
  int cnt=0;
  for(i=TA_BaseSubSpec.members.size; i< td->members.size; i++) {
    if(GetUnique(i)) {
      strm << td->members[i]->name << "; ";
      cnt++;
      if((cnt % 6) == 0) {
	strm << "\n";
	taMisc::indent(strm, indent+2, 1);
      }
    }
  }
  strm << "};\n";

  GetTypeDef()->members.Dump_Save(strm, (void*)this, (void*)par, indent+1);

  return true;
}


int BaseSubSpec::Dump_Load_Value(istream& strm, TAPtr par) {
  int c = taMisc::skip_white(strm);
  if(c == EOF)    return EOF;
  if(c == ';')    return 2;  // signal that just a path was loaded..
  if(c == '}') {
    if(strm.peek() == ';') strm.get();
    return 2;
  }

  if(c != '{') {
    taMisc::Error("*** Missing '{' in dump file for spec object:",name);
    return false;
  }

  // this is now like MemberSpace::Dump_Load
  c = taMisc::read_word(strm, true);
  if(c == EOF) return EOF;
  if(c == '}') {
    strm.get();			// get the bracket (above was peek)
    if(strm.peek() == ';') strm.get(); // skip past ending semi
    return 2;
  }

  String mb_name = taMisc::LexBuf;
  if(mb_name == "unique") {
    // first turn all others off, since default is off
    int i;
    for(i=TA_BaseSubSpec.members.size; i<GetTypeDef()->members.size; i++)
      SetUnique(i, false);
    while(true) {
      c = taMisc::skip_white(strm);
      if(c == '=') {
	c = taMisc::read_till_lb_or_semi(strm);
	if(c == '{') {
	  while(true) {
	    c = taMisc::read_till_rb_or_semi(strm);
	    if(c == EOF)  return EOF;
	    if(c == '}')	break;
	    if(c == ';')
	      SetUnique((char*)taMisc::LexBuf, true);
	  }
	  if(c == EOF)	return EOF;
	  break;		// successful
	}
      }
      taMisc::Error("*** Bad 'unique' Formatting in dump file for type:",
		      GetTypeDef()->name);
      taMisc::skip_past_err(strm);
      break;
    }
  }
  else {			// not 'unique' so pass it to std function
    return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par,
					   (const char*)mb_name, c);
  }
  return GetTypeDef()->members.Dump_Load(strm, (void*)this, (void*)par);
}


//////////////////////////////////
//	SpecPtr_impl		//
//////////////////////////////////

void SpecPtr_impl::Initialize() {
  owner = NULL;
  type = NULL;
  base_type = &TA_BaseSpec;
}

void SpecPtr_impl::Copy_(const SpecPtr_impl& cp) {
  type = cp.type;
  base_type = cp.base_type;
}

void SpecPtr_impl::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();

  if((owner == NULL) || (type == NULL))
    return;

  BaseSpec* sp = GetSpec();
  if(sp != NULL) {
    if(sp->GetTypeDef() == type)
      return;
    else
      SetSpec(NULL);		// get rid of existing spec
  }

  GetSpecOfType();
}

void SpecPtr_impl::SetBaseType(TypeDef* td) {
  type = td;
  base_type = td;		// this doesn't get set by defaults
}

void SpecPtr_impl::SetDefaultSpec(TAPtr ownr, TypeDef* td) {
  if(type == NULL)
    type = td;

  if(base_type == &TA_BaseSpec)
    base_type = td;

  owner = (TAPtr)ownr;
  if(taBase::GetRefn(this) == 0) {
    taBase::Ref(this);		// refer to this object..
  }

  BaseSpec* sp = GetSpec();
  if((sp != NULL) && (sp->GetTypeDef() == type))
    return;

  // this is just like GetSpecOfType(), except it uses inherits!
  // thus, this won't create a new object unless absolutely necessary
  BaseSpec_MGroup* spgp = GetSpecGroup();
  if(spgp == NULL)
    return;
  // pass the ownr to this, so that min_obj_type can be checked
  sp = spgp->FindSpecInherits(type, owner);
  if((sp != NULL) && sp->InheritsFrom(type)) {
    SetSpec(sp);
    return;
  }

  sp = (BaseSpec*)spgp->NewEl(1, type);
  if(sp != NULL) {
    SetSpec(sp); 
    winbMisc::DelayedMenuUpdate(sp);
  }
}

void SpecPtr_impl::GetSpecOfType() {
  BaseSpec_MGroup* spgp = GetSpecGroup();
  if(spgp == NULL)
    return;
  BaseSpec* sp = spgp->FindSpecType(type);
  if((sp != NULL) && (sp->GetTypeDef() == type)) {
    SetSpec(sp);
    return;
  }

  sp = (BaseSpec*)spgp->NewEl(1, type);
  if(sp != NULL) {
    SetSpec(sp); 
    winbMisc::DelayedMenuUpdate(sp);
  }
}


