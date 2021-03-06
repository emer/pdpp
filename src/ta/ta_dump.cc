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

// ta_dump.cc

#include <ta/ta_dump.h>
#include <ta/ta_base.h>
// for process pending events
#ifndef NO_IV
#include <ta/taiv_data.h>
#endif

taBase_List 	dumpMisc::update_after;
DumpPathSubList	dumpMisc::path_subs;
DumpPathTokenList dumpMisc::path_tokens;
VPUList 	dumpMisc::vpus;


//////////////////////////
//	VPUnref		//
//////////////////////////


VPUnref::VPUnref(TAPtr* b, TAPtr par, const String& p, MemberDef* md) {
  base = b; parent = par; path = p; memb_def = md; 
  name = String((long)base);
}

TAPtr VPUnref::Resolve() {
  MemberDef* md;
  TAPtr bs = tabMisc::root->FindFromPath(path, md);
  if((md == NULL) || (bs == NULL))
    return NULL;

  if(md->type->ptr == 1) {
    bs = *((TAPtr*)bs);
    if(bs == NULL)
      return NULL;
  }
  else if(md->type->ptr != 0) {
    taMisc::Error("*** ptr count greater than 1 in path:", path);
    return NULL;
  }

  if((memb_def != NULL) && memb_def->HasOption("OWN_POINTER")) {
    if(parent == NULL)
      taMisc::Error("*** NULL parent for owned pointer:",path);
    else
      taBase::OwnPointer(base, bs, parent);
  }
  else
    taBase::SetPointer(base, bs);

  if(taMisc::verbose_load >= taMisc::MESSAGES)
    taMisc::Error("<== Resolved Reference:",path);
  if(parent != NULL)
    parent->UpdateAfterEdit();
  return *base;
}

void VPUList::Resolve() {
  if(size <= 0)
    return;
  int i=0;
  do {
    if(FastEl(i)->Resolve() != NULL)
      Remove(i);		// take off the list if resolved!
    else {
      VPUnref* vp = (VPUnref*)FastEl(i);
      String par_path;
      if(vp->parent != NULL)
	par_path = vp->parent->GetPath();
      taMisc::Error("*** Warning: Could not resolve following path:",vp->path,
		    "in object:",par_path);
      i++;
    }
  } while(i < size);
}

void VPUList::Add(TAPtr* b, TAPtr par, const String& p, MemberDef* md) {
  AddUniqNameOld(new VPUnref(b,par,p,md));
  if(taMisc::verbose_load >= taMisc::MESSAGES)
    taMisc::Error("==> Unresolved Reference:",p);
}

  
//////////////////////////
//	PathSub		//
//////////////////////////


DumpPathSub::DumpPathSub(TypeDef* td, TAPtr par, const String& o, const String& n) {
  type = td; parent = par;
  old_path = o; new_path = n;
}

void DumpPathSubList::Add(TypeDef* td, TAPtr par, String& o, String& n) {
  int o_i = o.length()-1;
  int n_i = n.length()-1;
  int o_last_sep = -1;		// position of last separator character
  int n_last_sep = -1;
  // remove any trailing parts of name that are common to both..
  // often only the leading part of name is new, so no need to
  // replicate the same fix over and over..
  while ((o_i >= 0) && (n_i >= 0) && (o[o_i] == n[n_i])) {
    if(o[o_i] == '.') {
      o_last_sep = o_i;
      n_last_sep = n_i;
    }
    o_i--; n_i--;
  }
  String* op = (String*)&o;
  String* np = (String*)&n;
  String ot, nt;
  if(o_last_sep > 0) {
    op = &ot;
    np = &nt;
    ot = o.before(o_last_sep);
    nt = n.before(n_last_sep);
  }
  if(taMisc::verbose_load >= taMisc::MESSAGES) {
    String ppath = par->GetPath();
    taMisc::Error("---> New Path Fix, old:",*op,"new:",*np,"in:",ppath);
  }
  DumpPathSub* nwsb = new DumpPathSub(td, par, *op, *np);
  Add(nwsb);
  if(par != tabMisc::root) {	// if local path, then add a global path fix too
    String ppath = par->GetPath();
    unFixPath(td, tabMisc::root, ppath); // un fix the parent if necessary
    String long_o = ppath + *op;
    String long_n = ppath + *np;
    nwsb = new DumpPathSub(td, tabMisc::root, long_o, long_n);
    Add(nwsb);
    if(taMisc::verbose_load >= taMisc::MESSAGES) {
      taMisc::Error("---> New Global Path Fix, old:",long_o,"new:",long_n);
    }
  }
}

void DumpPathSubList::FixPath(TypeDef*, TAPtr par, String& path) {
  // search backwards to get latest fix
  int i;
  for(i=size-1; i>=0; i--) {
    DumpPathSub* dp = FastEl(i);
    if(dp->parent != par) // scope by parent
      continue;
    if(path.contains(dp->old_path)) {
      path.gsub(dp->old_path, dp->new_path);
      return;			// done!
    }
  }
}

void DumpPathSubList::unFixPath(TypeDef*, TAPtr par, String& path) {
  // search backwards to get latest fix
  int i;
  for(i=size-1; i>=0; i--) {
    DumpPathSub* dp = FastEl(i);
    if(dp->parent != par) // scope by parent
      continue;
    if(path.contains(dp->new_path)) {
      path.gsub(dp->new_path, dp->old_path);
      return;			// done!
    }
  }
}


//////////////////////////
//	Path Token	//
//////////////////////////

DumpPathToken::DumpPathToken(TAPtr obj, const String& pat, const String& tok_id) {
  object = obj;
  path = pat;
  token_id = tok_id;
}

void DumpPathTokenList::Add(TAPtr obj, const String& pat) {
  String tok_id = "$";
  tok_id += String(size) + "$";
  DumpPathToken* tok = new DumpPathToken(obj, pat, tok_id);
  Add(tok);
}

int DumpPathTokenList::FindObj(TAPtr obj) {
  int i;
  for(i=0; i<size; i++) {
    if(FastEl(i)->object == obj)
      return i;
  }
  return -1;
}

String DumpPathTokenList::GetPath(TAPtr obj) {
  int idx = FindObj(obj);
  if(idx >= 0)
    return FastEl(idx)->token_id;

  String path = obj->GetPath();
  Add(obj, path);
  DumpPathToken* tok = FastEl(size-1);
  path += tok->token_id;	// this marks the token right then and there..
  return path;
}

void DumpPathTokenList::NewLoadToken(String& pat, String& tok_id) {
  int tok_num = (int)tok_id;
  Add(NULL, pat);
  DumpPathToken* tok = FastEl(size-1);
  if(tok_num != size-1) {
    taMisc::Error("*** Path Tokens out of order, on list:", tok->token_id, "in file:",
		   tok_id);
    tok->object = NULL;
    tok->path = "";
    String null_pat;
    while(size <= tok_num) Add(NULL, null_pat);	// catch up if possible
    tok = FastEl(tok_num);
    tok->path = pat;
  }
  if(taMisc::verbose_load >= taMisc::MESSAGES) {
    taMisc::Error("---> New Path Token:",tok_id,"path:",pat);
  }
}

TAPtr DumpPathTokenList::FindFromPath(String& pat, TypeDef* td, void* base,
				      void* par, MemberDef* memb_def)
{
  if(pat[0] == '$') {		// its a token path, not a real one..
    String num = pat(1,pat.index('$',-1)-1); // get the number
    DumpPathToken* tok = SafeEl((int)num);
    if(tok == NULL) {
      taMisc::Error("*** Path Token Not Created Yet:", pat);
      return NULL;
    }
    if((tok->object == NULL) && (base != NULL)) {
      dumpMisc::vpus.Add((TAPtr*)base, (TAPtr)par, pat, memb_def);
      return NULL;
    }
    return tok->object;		// this is what we're looking for
  }
  DumpPathToken* tok = NULL;
  if(pat.lastchar() == '$') {	// path contains definition of new token
    String token_id = pat.after('$');
    token_id = token_id.before('$');
    pat = pat.before('$');
    NewLoadToken(pat, token_id); // make the token
    tok = FastEl(size-1);
  }

  dumpMisc::path_subs.FixPath(td, tabMisc::root, pat);
  MemberDef* md = NULL;
  TAPtr rval = tabMisc::root->FindFromPath(pat, md);
  if((md == NULL) || (rval == NULL)) {
    if(base != NULL)
      dumpMisc::vpus.Add((TAPtr*)base, (TAPtr)par, pat, memb_def);
    return NULL;
  }

  if(md->type->ptr == 1) {
    rval = *((TAPtr*)rval);
    if(rval == NULL) {
      if(base != NULL)
	dumpMisc::vpus.Add((TAPtr*)base, (TAPtr)par, pat, memb_def);
      return NULL;
    }
  }
  else if(md->type->ptr != 0) {
    taMisc::Error("*** ptr count greater than 1 in path:", pat);
    return NULL;
  }
  if(tok != NULL)
    tok->object = rval;
  return rval;
}



//////////////////////////////////////////////////////////
// 			dump save			//
//////////////////////////////////////////////////////////

int MemberSpace::Dump_Save(ostream& strm, void* base, void* par, int indent) {
  int rval = true;
  int i;
  for(i=0; i<size; i++) {
    if(!FastEl(i)->Dump_Save(strm, base, par, indent))
      rval = false;
  }
  return rval;
}

int MemberSpace::Dump_SaveR(ostream& strm, void* base, void* par, int indent) {
  int rval = false;
  int i;
  for(i=0; i<size; i++) {
    if(FastEl(i)->Dump_SaveR(strm, base, par, indent))
      rval = true;
  }
  return rval;
}

int MemberSpace::Dump_Save_PathR(ostream& strm, void* base, void* par, int indent) {
  int rval = false;
  int i;
  for(i=0; i<size; i++) {
    if(FastEl(i)->Dump_Save_PathR(strm, base, par, indent))
      rval = true;
  }
  return rval;
}

bool MemberDef::DumpMember() {
  if((is_static && !HasOption("SAVE")) || HasOption("NO_SAVE"))
    return false;

  if((type->ptr == 0) || type->DerivesFrom(TA_taBase) ||
     type->DerivesFrom(TA_TypeDef) || type->DerivesFrom(TA_MemberDef))
    return true;
  
  // if its a pointer object you own 
  return false;
}   


int MemberDef::Dump_Save(ostream& strm, void* base, void*, int indent) {
  if(!DumpMember())
    return false;
  void* new_base = GetOff(base);
  if((type->ptr == 1) && (type->DerivesFrom(TA_taBase))) {
    TAPtr tap = *((TAPtr*)new_base);
    if((tap != NULL) &&	(tap->GetOwner() == base)) { // wholly owned subsidiary
      return tap->Dump_Save_impl(strm, (TAPtr)base, indent);
    }
    if((tap != NULL) && (tap->GetOwner() == NULL)) { // no owner, fake path name
      strm << tap->GetTypeDef()->name << " @*(." << name << ")";
      tap->Dump_Save_Value(strm, (TAPtr)base, indent);
      taMisc::indent(strm, indent, 1) << "};\n";
      return true;
    }
  }

  taMisc::indent(strm, indent, 1) << name;
  
  if(type->InheritsFormal(TA_class) && !type->InheritsFrom(TA_taString)) {
    if(type->InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)new_base;
      rbase->Dump_Save_inline(strm, (TAPtr)base, indent);
    }
    else
      type->Dump_Save_inline(strm, new_base, base, indent);
  }
  else if(type->InheritsFrom(TA_taString)) {
    // put quotes here because other uses of string vals don't need them
    // also quote special characters
    taString str = type->GetValStr(new_base, base, this);
    str.gsub("\\", "\\\\");
    str.gsub("\"", "\\\"");
    strm << "=\"" << str << "\";\n";
  }
  else {
    strm << "=" << type->GetValStr(new_base, base, this) << ";\n";
  }
  return true;
}

int MemberDef::Dump_SaveR(ostream& strm, void* base, void*, int indent) {
  if(!DumpMember())
    return false;

  int rval = false;
  void* new_base = GetOff(base);

  if((type->InheritsFormal(TA_class)) && !(type->InheritsFrom(TA_taString))) {
    if(type->InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)new_base;
      rval = rbase->Dump_SaveR(strm, (TAPtr)base, indent);
    }
    else
      rval = type->Dump_SaveR(strm, new_base, base, indent);
  }
  else if((type->ptr == 1) && (type->DerivesFrom(TA_taBase))) {
    TAPtr tap = *((taBase **)(new_base));
    if((tap != NULL) &&	(tap->GetOwner() == base)) { // wholly owned subsidiary
      tap->Dump_Save_impl(strm, (TAPtr) base, indent);
      rval = tap->Dump_SaveR(strm, (TAPtr)base, indent);
    }
  }
  return rval;
}

int MemberDef::Dump_Save_PathR(ostream& strm, void* base, void*, int indent) {
  if(!DumpMember())
    return false;
  if(HasOption("NO_SAVE_PATH_R") || HasOption("LINK_GROUP")) // don't save these ones..
    return false;

  int rval = false;
  void* new_base = GetOff(base);

  if((type->InheritsFormal(TA_class)) && !(type->InheritsFrom(TA_taString))) {
    if(type->InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)new_base;
      rval = rbase->Dump_Save_PathR(strm, (TAPtr)base, indent);
    }
    else
      rval = type->Dump_Save_PathR(strm, new_base, (TAPtr)base, indent);
  }
  else if((type->ptr == 1) && (type->DerivesFrom(TA_taBase))) {
    TAPtr tap = *((taBase **)(new_base));
    if((tap != NULL) &&	(tap->GetOwner() == base)) { // wholly owned subsidiary
      strm << "\n";			// actually saving a path: put a newline
      taMisc::indent(strm, indent, 1);
      tap->Dump_Save_Path(strm, (taBase*) base, indent);
      strm << " { ";
      if(tap->Dump_Save_PathR(strm, tap, indent+1))
  	taMisc::indent(strm, indent, 1);
      strm << "};\n";
    }
  }
  return rval;
}

int TypeDef::Dump_Save_Path(ostream& strm, void* base, void* par, int) {
  strm << name << " ";
  if(InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)base;
    if(rbase->GetOwner() == NULL)
      strm << "NULL";
    else {
      // its a relative path if you have a parent (period)
      if(par != NULL)
	strm << "@";
      strm << rbase->GetPath(NULL, (TAPtr)par);
    }
  }
  return true;
}

int TypeDef::Dump_Save_PathR(ostream& strm, void* base, void* par, int indent) {
  if(InheritsFormal(TA_class)) {
    return members.Dump_Save_PathR(strm, base, par, indent);
  }
  return false;
}

int TypeDef::Dump_Save_Value(ostream& strm, void* base, void* par, int indent) {
  if(InheritsFormal(TA_class)) {
    if(HasOption("INLINE")) {
      strm << " " << GetValStr(base, par) << ";\n";
    }
    else {
      strm << " {\n";
      members.Dump_Save(strm, base, par, indent+1);
    }
  }
  else {
    strm << " = " << GetValStr(base, par) << "\n";
  }
  return true;
}

int TypeDef::Dump_Save_impl(ostream& strm, void* base, void* par, int indent) {
  if(base == NULL)
    return false;

  if((ptr == 1) && DerivesFrom(TA_taBase)) {
    TAPtr tap = *((taBase **)(base));
    if((tap != NULL) &&	(tap->GetOwner() == par)) { // wholly owned subsidiary
      return tap->Dump_Save_impl(strm, (TAPtr) par, indent);
    }
    return false;
  }

  taMisc::indent(strm, indent, 1);
  if(InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)base;
    rbase->Dump_Save_Path(strm, (TAPtr)par, indent);
    rbase->Dump_Save_Value(strm, (TAPtr)par, indent);
  }
  else {
    Dump_Save_Path(strm, base, par, indent);
    Dump_Save_Value(strm, base, par, indent);
  }
  if(InheritsFormal(TA_class) && !HasOption("INLINE")) {
    if(InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)base;
      rbase->Dump_SaveR(strm, rbase, indent+1);
    }
    else
      Dump_SaveR(strm, base, base, indent+1);
    taMisc::indent(strm, indent, 1) << "};\n";
  }
//   if(InheritsFormal(TA_class) && !HasOption("INLINE")) {
//     members.Dump_SaveR(strm, base, par, indent+1);
//     taMisc::indent(strm, indent, 1) << "};\n";
//   }
  return true;
}

int TypeDef::Dump_Save_inline(ostream& strm, void* base, void* par, int indent) {
  if(base == NULL)
    return false;

  if((ptr == 1) && DerivesFrom(TA_taBase)) {
    TAPtr tap = *((taBase **)(base));
    if((tap != NULL) &&	(tap->GetOwner() == par)) { // wholly owned subsidiary
      return tap->Dump_Save_impl(strm, (TAPtr) par, indent);
    }
    return false;
  }

  if(InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)base;
    rbase->Dump_Save_Value(strm, (TAPtr)par, indent);
  }
  else {
    Dump_Save_Value(strm, base, par, indent);
  }
  if(InheritsFormal(TA_class) && !InheritsFrom(TA_taString) &&
     !HasOption("INLINE"))
  {
    if(InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)base;
      rbase->Dump_SaveR(strm, rbase, indent+1);
    }
    else
      Dump_SaveR(strm, base, base, indent+1);
    taMisc::indent(strm, indent, 1) << "};\n";
  }
//   if(InheritsFormal(TA_class) && !HasOption("INLINE")) {
//     members.Dump_SaveR(strm, base, par, indent+1);
//     taMisc::indent(strm, indent, 1) << "};\n";
//   }
  return true;
}

int TypeDef::Dump_Save(ostream& strm, void* base, void* par, int indent) {
  if(base == NULL)
    return false;

  taMisc::is_saving = true;
  dumpMisc::path_tokens.Reset();

  strm << "// ta_Dump File v1.0\n";   // be sure to check version with Load
  if(InheritsFrom(TA_taBase)) {
    TAPtr rbase = (TAPtr)base;
    rbase->Dump_Save_Path(strm, (TAPtr)par, indent);
    strm << " { ";
    if(rbase->Dump_Save_PathR(strm, (TAPtr)par, indent+1))
      taMisc::indent(strm, indent, 1);
    strm << "};\n";
    rbase->Dump_Save_impl(strm, (TAPtr)par, indent);
//     rbase->Dump_SaveR(strm, (TAPtr)par, indent); // already in _impl -- not needed!
  }
  else {
    Dump_Save_Path(strm, base, par, indent);
    if(InheritsFormal(TA_class)) {
      strm << " { ";
      if(Dump_Save_PathR(strm, base, par, indent))
	taMisc::indent(strm, indent, 1);
      strm << "};\n";
    }
    else {
      strm << ";\n";
    }

    Dump_Save_impl(strm, base, par, indent);
  }
  taMisc::is_saving = false;
  dumpMisc::path_tokens.Reset();
  return true;
}

int TypeDef::Dump_SaveR(ostream&, void*, void*, int) {
  return false;			// nothing to save
}


//////////////////////////////////////////////////////////
// 			dump load			//
//////////////////////////////////////////////////////////

int MemberSpace::Dump_Load(istream& strm, void* base, void* par,
			   const char* prv_read_nm, int prv_c) {
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (prv_read_nm != NULL) ? prv_read_nm : "NULL";
    cerr << "Entering MemberSpace::Dump_Load, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", prv_read_nm = " << nm
	 << ", prv_c = " << prv_c << endl;
  }

  int rval = 2;			// default assumption is that no members are loaded
  do {
    int c;
    if(prv_read_nm != NULL) {
      taMisc::LexBuf = prv_read_nm;
      c = prv_c;
      prv_read_nm = NULL;
    }
    else {
      c = taMisc::read_word(strm, true);
    }
    if(c == EOF) {
      if(taMisc::verbose_load >= taMisc::MESSAGES)
	taMisc::Error("<<< EOF in MemberSpace::Dump_Load", taMisc::LexBuf);
      return EOF;
    }
    if(c == '}') {
      strm.get();		// get the bracket (above was peek)
      if(strm.peek() == ';') strm.get(); // skip past ending semi
      if(taMisc::verbose_load >= taMisc::TRACE) {
	const char* nm = (prv_read_nm != NULL) ? prv_read_nm : "NULL";
	cerr << "}, Leaving MemberSpace::Dump_Load, type: " << owner->name
	     << ", par = " << String((int)par) << ", base = " << String((int)base)
	     << ", prv_read_nm = " << nm
	     << ", prv_c = " << prv_c << endl;
      }
      return rval;
    }
    if(c == '$') {		// got a path token
      strm.get();
      c = taMisc::read_word(strm);
      String tok_nm = taMisc::LexBuf;
      strm.get();		// read the end of token
      c = taMisc::skip_white(strm); // get the equals
      c = taMisc::read_till_lb_or_semi(strm);
      if(c == EOF) {
	if(taMisc::verbose_load >= taMisc::MESSAGES) 
	  taMisc::Error("<<< EOF in MemberSpace::Dump_Load", taMisc::LexBuf);
	return EOF;
      }
      String path_val = taMisc::LexBuf;
      dumpMisc::path_tokens.NewLoadToken(path_val, tok_nm);
      continue;			// keep on truckin
    }

    String mb_name = taMisc::LexBuf;
    if((c == ' ') || (c == '\n') || (c == '\t')) { // skip past white
      strm.get();
      c = taMisc::skip_white(strm, true);
    }
    int tmp;
    if((c == '@') || (c == '.') || (c == '*')) {	// got a path 
      // pass the mb_name, which is the type name, to the load function...
      tmp = TA_taBase.Dump_Load_impl(strm, NULL, base, mb_name);
    }
    else {
      MemberDef* md = FindName(mb_name);
      if(md == NULL) {		// try to find a name with an aka..
	int a;
	for(a=0;a<size;a++) {
	  MemberDef* amd = FastEl(a);
	  String aka = amd->OptionAfter("AKA_");
	  if(aka.empty()) continue;
	  if(aka == mb_name) {
	    md = amd;
	    break;
	  }
	}
      }
      if(md != NULL) {
	tmp = md->Dump_Load(strm, base, par);
	if(tmp == EOF) {
	  if(taMisc::verbose_load >= taMisc::MESSAGES)
	    taMisc::Error("<<< EOF in MemberSpace::Dump_Load::else:", mb_name);
	  return EOF;
	}
	if(tmp == false)
	  taMisc::skip_past_err(strm);
      }
      else {
	taMisc::Error("*** Member:",mb_name,"not found in type:",
			owner->name, "(this is likely just harmless version skew)");
	int sv_vld = taMisc::verbose_load;
	taMisc::verbose_load = taMisc::SOURCE;
	taMisc::skip_past_err(strm);
	taMisc::verbose_load = (taMisc::LoadVerbosity)sv_vld;
	cerr << endl << endl << flush;
      }
      rval = true;		// member was loaded, do update after edit
    }
  } while (1);
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (prv_read_nm != NULL) ? prv_read_nm : "NULL";
    cerr << "Leaving MemberSpace::Dump_Load, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", prv_read_nm = " << nm
	 << ", prv_c = " << prv_c << endl;
  }
  return rval;
}

int MemberDef::Dump_Load(istream& strm, void* base, void* par) {
  if(!DumpMember())
    return false;
  
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Entering MemberDef::Dump_Load, member: " << name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }

  void* new_base = GetOff(base);
  int rval;

  if(type->InheritsFormal(TA_class) && !type->InheritsFrom(TA_taString)) {
    if(type->InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)new_base;
      rval = rbase->Dump_Load_impl(strm, (TAPtr)base);
    }
    else
      rval = type->Dump_Load_impl(strm, new_base, base);
    if(taMisc::verbose_load >= taMisc::TRACE) {
      cerr << "Leaving MemberDef::Dump_Load, member: " << name
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << endl;
    }
    return rval;
  }
  else if((type->ptr == 1) && type->DerivesFrom(TA_taBase) 
	  && HasOption("OWN_POINTER"))
  {
    int c = taMisc::skip_white(strm, true);
    if(c == '{') {
      // a taBase object that was saved as a member, but is now a pointer..
      TAPtr rbase = *((TAPtr*)new_base);
      if(rbase == NULL) {	// it's a null object, can't load into it
	taMisc::Error("*** Can't load into NULL pointer object for member:", name,
		      "in type:",GetOwnerType()->name);
	return false;
      }
      // treat it as normal..
      rval = rbase->Dump_Load_impl(strm, (TAPtr)base);
      if(taMisc::verbose_load >= taMisc::TRACE) {
	cerr << "Leaving MemberDef::Dump_Load, member: " << name
	     << ", par = " << String((int)par) << ", base = " << String((int)base)
	     << endl;
      }
      return rval;
    }
  }
  int c = taMisc::skip_white(strm);
  if(c != '=') {
    taMisc::Error("*** Missing '=' in dump file for member:", name,
		  "in type:",GetOwnerType()->name);
    return false;
  }
  if(c == ';') {
    // do nothing
  }
  else if(type->InheritsFrom(TA_taString)) {
    c = taMisc::read_till_quote(strm); // get 1st quote
    if(c == '\"')			  // "
      c = taMisc::read_till_quote_semi(strm);// then till second followed by semi
  }
  else {
    c = taMisc::read_till_rb_or_semi(strm);
  }

  if(c != ';') {
    taMisc::Error("*** Missing ';' in dump file for member:", name,
		  "in type:",GetOwnerType()->name);
    return true;		// don't scan any more after this err..
  }

  type->SetValStr(taMisc::LexBuf, new_base, base, this);
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Leaving MemberDef::Dump_Load, member: " << name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }
  return true;
}

int TypeDef::Dump_Load_Path(istream& strm, void*& base, void* par,
			    TypeDef*& td, String& path, const char* typnm)
{
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (typnm != NULL) ? typnm : "NULL";
    cerr << "Entering TypeDef::Dump_Load_Path, type: " << name
	 << ", path = " << path
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", typnm = " << nm
	 << endl;
  }
  td = NULL;
  int c;
  c = taMisc::read_till_lb_or_semi(strm, true);
  if(c == EOF) {
    if(taMisc::verbose_load >= taMisc::MESSAGES)
      taMisc::Error("<<< EOF in Dump_Load_Path:", name, taMisc::LexBuf);
    return EOF;
  }

  // the format is usually: type path, but if tpnm is passed, then it is used
  bool has_type = true;
  if(taMisc::LexBuf.length() == 0)
    has_type = false;		// doesn't have anything
  else if(taMisc::LexBuf.lastchar() == ' ') // trailing spaces..
    taMisc::LexBuf = taMisc::LexBuf.before(' ',-1); // we hope this doesn't happen

  int spc_idx = -1;
  if(has_type && ((spc_idx = taMisc::LexBuf.index(' ')) < 0)) // we still think we have a type but don't
    has_type = false;
    
  String tpnm;
  if(typnm != NULL) {
    tpnm = typnm;
    if(has_type)
      path = taMisc::LexBuf.after(spc_idx);
    else
      path = taMisc::LexBuf;	// just read the whole path..
    has_type = true;		// actually do have type name
  }
  else {
    if(has_type) {
      tpnm = taMisc::LexBuf.before(spc_idx);
      path = taMisc::LexBuf.after(spc_idx); 
    }
    else {
      path = taMisc::LexBuf;
    }
  }
  if(!has_type)
    td = this;			// assume this is the right type..
  else {
    td = taMisc::types.FindName(tpnm);
    if(td == NULL) {
      taMisc::Error("*** Unknown type:",tpnm,"in Dump_Load_Path");
      return false;
    }

    if(!DerivesFrom(TA_taBase)) {
      if(!DerivesFrom(td)) {
	taMisc::Error("*** Type mismatch, expecting:",name,"Got:",td->name);
	return false;
      }
    }
  }

  if((base != NULL) || !(td->DerivesFrom(TA_taBase)) || (path == "")) {
    if(taMisc::verbose_load >= taMisc::TRACE) {
      const char* nm = (typnm != NULL) ? typnm : "NULL";
      cerr << "true Leaving TypeDef::Dump_Load_Path, type: " << name
	   << ", path = " << path
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << ", typnm = " << nm
	   << endl;
    }
    return true;		// nothing left to do here, actually
  }

  int rval = td->Dump_Load_Path_impl(strm, base, par, path);
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (typnm != NULL) ? typnm : "NULL";
    cerr << "Leaving TypeDef::Dump_Load_Path, type: " << name
	 << ", path = " << path
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", typnm = " << nm
	 << ", rval = " << rval
	 << endl;
  }
  return rval;
}

int TypeDef::Dump_Load_Path_impl(istream&, void*& base, void* par, String path) {
  bool ptr_flag = false;	// pointer was loaded
  String orig_path;
  TAPtr find_base = NULL;	// where to find item from
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Entering TypeDef::Dump_Load_Path_impl, type: " << name
	 << ", path = " << path
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }

  if(path.firstchar() == '@') {		// relative to current parent..
    if(par == NULL) {
      taMisc::Error("*** Relative path with NULL parent:", path);
      return false;
    }
    path = path.after('@');
    find_base = (TAPtr)par;	// we better get a taBase
  }

  if(path.firstchar() == '*') {
    ptr_flag = true;
    path = path.after('*');
  }

  // strip any leading parens, etc
  while(path.firstchar() == '(') path = path.after('(');
  while(path.lastchar() == ')') path = path.before(')',-1);

  if(find_base==NULL)
    find_base = tabMisc::root;	// search from top if not relative

  dumpMisc::path_subs.FixPath(NULL, find_base, path);  // fix the path name.. (substitution)
  orig_path = path;				  // original is post-substitution

  if((path == "root") || (path == ".")) {
    base = (void*)find_base;
    if(taMisc::verbose_load >= taMisc::TRACE) {
      cerr << "root Leaving TypeDef::Dump_Load_Path_impl, type: " << name
	   << ", path = " << path
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << endl;
    }
    return true;
  }

  int delim_pos = taBase::GetLastPathDelimPos(path);
  String el_path;
  if(path[delim_pos] == '[')
    el_path = path.from(delim_pos);
  else
    el_path = path.after(delim_pos);
  String ppar_path;
  if(delim_pos > 0)
    ppar_path = path.before(delim_pos);

  MemberDef* ppar_md = NULL;
  TAPtr ppar = find_base->FindFromPath(ppar_path, ppar_md); // path-parent

  if(ppar == NULL) {
    taMisc::Error("*** Could not find a parent for:",el_path,"in",ppar_path);
    return false;
  }

  if((ppar_md != NULL) && !(ppar_md->type->InheritsFrom(TA_taBase))) {
    taMisc::Error("*** Parent must be a taBase type for:",el_path,"in",ppar_path,
	"type:",ppar_md->type->name);
    return false;
  }

  void* tmp_ptr;
  MemberDef* el_md = ppar->FindMembeR(el_path,tmp_ptr);
  TAPtr el = (TAPtr)tmp_ptr;

// this is the original confusing logic
//   if((el != NULL) && (el_md != NULL) && el_md->type->DerivesFrom(TA_taBase)
//      && ((ptr_flag && ((el=(*(TAPtr *)el)) == NULL)) || (el->GetTypeDef() == this)))

  if((el == NULL) || (el_md == NULL)) {   // did not find the thing
    el = ppar->New(1,this);
    if(el == NULL) {
      taMisc::Error("*** New: Could not make a token of:",name,"in:",ppar->GetPath(),
		    path);
      return false;
    }
    if(el->GetOwner() != NULL) {
      String new_path = el->GetPath(NULL, find_base);
      if(new_path != orig_path) {
	dumpMisc::path_subs.Add(this, find_base, orig_path, new_path);
      }
    }
    base = (void*)el; 	// reset the base to be the element
    if(taMisc::verbose_load >= taMisc::TRACE) {
      cerr << "el=NULL Leaving TypeDef::Dump_Load_Path_impl, type: " << name
	   << ", path = " << path
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << endl;
    }
    return true;
  }
  if(ptr_flag) {		// if expecting a pointer
    el = *((TAPtr*)el);
    if(el == NULL) {
      TAPtr* elp = (TAPtr*)tmp_ptr;
      *elp = taBase::MakeToken(this);
      el = *elp;
      if(*elp == NULL) {
	taMisc::Error("*** MakeToken: Could not make a token of:",name,"in:", path);
	return false;
      }
      taBase::Own(el,(TAPtr) par);
      base = (void*)el; 	// reset the base to be the element
      if(taMisc::verbose_load >= taMisc::TRACE) {
	cerr << "ptr_flag Leaving TypeDef::Dump_Load_Path_impl, type: " << name
	     << ", path = " << path
	     << ", par = " << String((int)par) << ", base = " << String((int)base)
	     << endl;
      }
      return true;
    }
  }
  // check for correct type, but allow a list to be created in a group
  // for backwards compatibility with changes from groups to lists
  if((el->GetTypeDef() != this) && 
     !((el->GetTypeDef() == &TA_taBase_List) && (this == &TA_taBase_Group)))
  { 
    // object not the right type, try to create new one..
    if(taMisc::verbose_load >= taMisc::MESSAGES) {
      taMisc::Error("*** Object at path:",ppar->GetPath(),path,
		    "of type:",el->GetTypeDef()->name,"is not the right type:",name,
		    ", attempting to create new one");
    }
    el = ppar->New(1,this);
    if(el == NULL) {
      taMisc::Error("*** Could not make a token of:",name,"in:",ppar->GetPath(),
		    path);
      return false;
    }
    if(el->GetOwner() != NULL) {
      String new_path = el->GetPath(NULL, find_base);
      if(new_path != orig_path) {
	dumpMisc::path_subs.Add(this, find_base, orig_path, new_path);
      }
    }
  }
  base = (void*)el; 	// reset the base to be the element
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Leaving TypeDef::Dump_Load_Path_impl, type: " << name
	 << ", path = " << path
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }
  return true;
}


int TypeDef::Dump_Load_Value(istream& strm, void* base, void* par) {
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Entering TypeDef::Dump_Load_Value, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }
  int c = taMisc::skip_white(strm);
  if(c == EOF) {
    if(taMisc::verbose_load >= taMisc::MESSAGES)
      taMisc::Error("<<< EOF in Dump_Load_Value:", name);
    return EOF;
  }
  if(c == ';')    return 2;  // signal that just a path was loaded..
  if(c == '}') {
    if(strm.peek() == ';') strm.get();
    if(taMisc::verbose_load >= taMisc::TRACE) {
      cerr << "} Leaving TypeDef::Dump_Load_Value, type: " << owner->name
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << endl;
    }
    return 2;
  }
  
  if(HasOption("INLINE")) {
    if(c != '{') {
      taMisc::Error("*** Missing '{' in dump file for inline type:", name);
      return false;
    }
    c = taMisc::read_till_rb_or_semi(strm);
    if(c != '}') {
      taMisc::Error("*** Missing '}' in dump file for inline type:", name);
      return false;
    }
    taMisc::LexBuf = String("{") + taMisc::LexBuf; // put lb back in..
    SetValStr(taMisc::LexBuf, base);
  }
  else if(InheritsFormal(TA_class)) {
    if(c != '{') {
      taMisc::Error("*** Missing '{' in dump file for type:",name);
      return false;
    }
    return members.Dump_Load(strm, base, par);
  }
  else {
    c = taMisc::skip_white(strm);
    if(c == EOF) {
      if(taMisc::verbose_load >= taMisc::MESSAGES)
	taMisc::Error("<<< EOF in Dump_Load_Value::else:", name);
      return EOF;
    }
    if(c != '=') {
      taMisc::Error("*** Missing '=' in dump file for type:",name);
      return false;
    }

    c = taMisc::read_till_rb_or_semi(strm);
    if(c == EOF) {
      if(taMisc::verbose_load >= taMisc::MESSAGES)
	taMisc::Error("<<< EOF in Dump_Load_Value::rb", name);
      return EOF;
    }

    SetValStr(taMisc::LexBuf, base);
  }
  if(taMisc::verbose_load >= taMisc::TRACE) {
    cerr << "Leaving TypeDef::Dump_Load_Value, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << endl;
  }
  return true;
}

int TypeDef::Dump_Load_impl(istream& strm, void* base, void* par, const char* typnm) {
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (typnm != NULL) ? typnm : "NULL";
    cerr << "Entering TypeDef::Dump_Load_impl, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", typnm = " << nm
	 << endl;
  }
  TypeDef* td = NULL;
  String path;
  int rval = Dump_Load_Path(strm, base, par, td, path, typnm);
  if((taMisc::verbose_load >= taMisc::TRACE) && (td != NULL)) 
    cerr << "Loading: " << td->name << " " << path << " rval: " << rval << "\n";
#ifndef NO_IV
  if((taMisc::iv_verbose_load >= taMisc::TRACE) && (td != NULL) &&
     !td->HasOption("NO_UPDATE_AFTER"))
    taivMisc::SetLoadDialog(td->name);
#endif
  if((rval > 0) && (base != NULL)) {
    if(td->InheritsFrom(TA_taBase)) {
      TAPtr rbase = (TAPtr)base;
      rval = rbase->Dump_Load_Value(strm, (TAPtr)par);
      if(rval==1) {
	if(rbase->HasOption("IMMEDIATE_UPDATE"))
	  rbase->UpdateAfterEdit();
	else if(!rbase->HasOption("NO_UPDATE_AFTER")) {
	  dumpMisc::update_after.Link(rbase);
	}
      }
    }
    else {
      rval = td->Dump_Load_Value(strm, base, par);
    }
    if(rval == EOF) {
      if(taMisc::verbose_load >= taMisc::MESSAGES)
	taMisc::Error("<<< EOF in Dump_Load_impl:", name);
      return EOF;
    }
    if(rval == false) {
      // read till next rbracket, (skipping over inner ones..)
      // be sure to revise dump_loads to return false on error
      int c = taMisc::skip_past_err(strm);
      if(c == EOF) return EOF;
      if(taMisc::verbose_load >= taMisc::TRACE) {
	const char* nm = (typnm != NULL) ? typnm : "NULL";
	cerr << "err Leaving TypeDef::Dump_Load_impl, type: " << owner->name
	     << ", par = " << String((int)par) << ", base = " << String((int)base)
	     << ", typnm = " << nm
	     << endl;
      }
      return true;		// we already scanned past error
    }
  }
  else {
    // make sure we skip until we hit the end of this one (rbracket)
    // not just the semicolon at the end of some data item..
    if(strm.peek() == '{')
      strm.get();		// if we're at the start of a bracket, get it first
    if(strm.peek() == '=') {	// if we're reading a path and it didn't work, need to get lb!
      taMisc::read_till_lbracket(strm, false);
    }
    int c;
    //    if((path.length() > 1) && (path[1] == '*')) // for path items, use non-peek, else use peek 
    //    cerr << "***HERE***" << endl;
    c = taMisc::skip_past_err_rb(strm, false);
//     else
//       c = taMisc::skip_past_err_rb(strm, true);
    if(c == EOF) return EOF;
    if(taMisc::verbose_load >= taMisc::TRACE) {
      const char* nm = (typnm != NULL) ? typnm : "NULL";
      cerr << "err rb Leaving TypeDef::Dump_Load_impl, type: " << owner->name
	   << ", par = " << String((int)par) << ", base = " << String((int)base)
	   << ", typnm = " << nm
	   << endl;
    }
    return true;		// already scanned past error
  }
  if(taMisc::verbose_load >= taMisc::TRACE) {
    const char* nm = (typnm != NULL) ? typnm : "NULL";
    cerr << "Leaving TypeDef::Dump_Load_impl, type: " << owner->name
	 << ", par = " << String((int)par) << ", base = " << String((int)base)
	 << ", typnm = " << nm
	 << ", rval = " << rval
	 << endl;
  }
  return rval;
}

int TypeDef::Dump_Load(istream& strm, void* base, void* par) {
  if(base == NULL) {
    taMisc::Error("*** Cannot load into NULL");
    return false;
  }

  dumpMisc::path_subs.Reset();
  dumpMisc::path_tokens.Reset();
  dumpMisc::vpus.Reset();
  dumpMisc::update_after.Reset();
    
  int c;
  c = taMisc::read_till_eol(strm);
  if(c == EOF) return EOF;
  if(!(taMisc::LexBuf.contains("// ta_Dump File v1.0"))) {
    taMisc::Error("*** Dump file does not have proper format id:", taMisc::LexBuf);
    return false;
  }

  TypeDef* td;
  String path;
  int rval = Dump_Load_Path(strm, base, par, td, path); // non-null base just gets type
  if(rval <= 0) {
    taMisc::Error("*** Dump load aborted due to errors");
    return false;
  }
  
  if(!td->InheritsFrom(TA_taBase)) {
    taMisc::Error("*** Only taBase objects may be loaded, not:", td->name);
    return false;
  }

  if(taMisc::verbose_load >= taMisc::TRACE) {
    cout << "Loading: " << td->name << " " << path << " rval: " << rval << "\n";
  }

  taMisc::is_loading = true;

  TAPtr el;				// the loaded element
  if(InheritsFrom(td)) {		// we are the same as load token
    el = (TAPtr)base;			// so use the given base
  }
  else {
    TAPtr par = (TAPtr)base;		// given base must be a parent
    el = par->New(1,td);		// create one of the saved type
    if(el == NULL) {
      taMisc::Error("*** Could not make a:",td->name,"in:",par->GetPath());
      taMisc::is_loading = false;
      return false;
    }
  }


  String new_path = el->GetPath();
  if(new_path != path)
    dumpMisc::path_subs.Add(td, tabMisc::root, path, new_path);
      
  rval = el->Dump_Load_Value(strm, (TAPtr)par); // read it

  while(rval != EOF) {
    rval = Dump_Load_impl(strm, NULL, par); 	// base is given by the path..
  }

  dumpMisc::vpus.Resolve(); 			// try to cache out references.
  
  int i;
  for(i=0; i<dumpMisc::update_after.size; i++) {
    TAPtr tmp = dumpMisc::update_after.FastEl(i);
    if(taBase::GetRefn(tmp) <= 1) {
      taMisc::Error("*** Object: of type:",
		      tmp->GetTypeDef()->name,"named:",tmp->GetName(),"is unowned!");
      taBase::Ref(tmp);
    }
    tmp->UpdateAfterEdit();
#ifndef NO_IV
    taivMisc::RunIVPending();	// process events as they happen in updates..
#endif
  }

  taMisc::is_loading = false;

  dumpMisc::update_after.Reset();
  dumpMisc::path_subs.Reset();
  dumpMisc::path_tokens.Reset();
  dumpMisc::vpus.Reset();
  return true;
}
   
