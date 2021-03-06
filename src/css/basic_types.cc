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

// basic_types.cc

#include <css/basic_types.h>
#include <css/css_iv.h>		// for cssClass::Edit()

cssInt::operator void*() const {
  if(val == 0)
    return NULL;
  CvtErr("(void*)");
  return NULL;
}

cssInt::operator void**() const {
  if(val == 0)
    return NULL;
  CvtErr("(void**)");
  return NULL;
}

void cssChar::operator=(const cssEl& t) {
  if(t.GetType() == T_String) {
    *this = t.GetStr();		// use string converter
  }
  else {
    val = (Int)t;
  }
}

//////////////////////////
// 	cssString	//
//////////////////////////

cssString::~cssString() {
  if(gf != NULL)
    taRefN::unRefDone(gf);
  gf = NULL;
}

void cssString::Constr() {
  Register();
  gf = NULL;
}
void cssString::TypeInfo(ostream& fh) const {
  TA_taString.OutputType(fh); 
}

void cssString::InheritInfo(ostream& fh) const {
  TA_taString.OutputInherit(fh) << "\n";
}

cssEl* cssString::operator[](int idx) const {
  String nw_val = val.elem(idx);
  return new cssString(nw_val);
}  

int cssString::GetMemberFunNo(const char* memb) const {
  int md;
  TA_taString.methods.FindName(memb, md);
  return md;
}
cssEl* cssString::GetMemberFun(const char* memb) const {
  MethodDef* md = TA_taString.methods.FindName(memb);
  if(md == NULL) {
    cssMisc::Error(prog, "Member function not found:", memb, "in class of type: String");
    return &cssMisc::Void;
  }
  return GetMemberFun_impl(md);
}
cssEl* cssString::GetMemberFun(int memb) const {
  MethodDef* md = TA_taString.methods.SafeEl(memb);
  if(md == NULL) {
    cssMisc::Error(prog, "Member function not found:", String(memb), "in class of type: String");
    return &cssMisc::Void;
  }
  return GetMemberFun_impl(md);
}
cssEl* cssString::GetMemberFun_impl(MethodDef* md) const {
  if(md->stubp != NULL) {
    if(md->fun_argd >= 0)
      return new cssMbrCFun(VarArg, (void*)&val, md->stubp, md->name);
    else
      return new cssMbrCFun(md->fun_argc, (void*)&val, md->stubp, md->name);
  }
  else {
    cssMisc::Error(prog, "Function pointer not callable:", md->name, "of type:", md->type->name, 
	      "in class of type: String");
    return &cssMisc::Void;
  }
}

cssEl* cssString::GetScoped(const char* memb) const {
  EnumDef* ed = TA_taString.FindEnum(memb);
  if(ed != NULL) {
    return new cssInt(ed->enum_no);
  }

  MethodDef* md = TA_taString.methods.FindName(memb);
  if(md == NULL) {
    cssMisc::Error(prog, "Scoped element not found:", memb, "in class of type: String");
    return &cssMisc::Void;
  }

  return GetMemberFun_impl(md);
}

// delete any open files at this point
void cssString::deReferenced() {
  if(gf != NULL)
    taRefN::unRefDone(gf);
  gf = NULL;
}

cssString::operator ostream*() const {
  cssString* ths = (cssString*)this;
  if(ths->gf == NULL) {
    ths->gf = new taivGetFile;
    taRefN::Ref(ths->gf);
  }
  ths->gf->fname = val;
#ifdef DMEM_COMPILE
  // provide different names for all the non-primary processors
  if((taMisc::dmem_nprocs > 1) && (taMisc::dmem_proc > 0)) {
    String fnm = ths->gf->fname;
    if(fnm.contains(taMisc::compress_sfx)) {
      fnm = fnm.before(taMisc::compress_sfx) +
	String(".p") + taMisc::LeadingZeros(taMisc::dmem_proc, 3) + taMisc::compress_sfx;
    }
    else {
      fnm += String(".p") + taMisc::LeadingZeros(taMisc::dmem_proc, 3);
    }
    ths->gf->fname = fnm;
    if(taMisc::dmem_debug)
      cerr << "proc: " << taMisc::dmem_proc << " setting fname to: " << ths->gf->fname << endl;
  }
#endif
  ostream* strm = ths->gf->open_write();
  if((strm == NULL) || !(ths->gf->open_file)) {
    cssMisc::Error(prog, "String -> ostream*: could not open file", val);
    return (ostream*)NULL;
  }
  return strm;
}

cssString::operator istream*() const {
  cssString* ths = (cssString*)this;
  if(ths->gf == NULL) {
    ths->gf = new taivGetFile;
    taRefN::Ref(ths->gf);
  }
  ths->gf->fname = val;
  istream* strm = ths->gf->open_read();
  if((strm == NULL) || !(ths->gf->open_file)) {
    cssMisc::Error(prog, "String -> istream*: could not open file", val);
    return (istream*)NULL;
  }
  return strm;
}

void cssString::Save(ostream& strm) {
  String str = GetStr();
  // quote special characters
  str.gsub("\\", "\\\\");
  str.gsub("\"", "\\\"");
  strm << "\"" << str << "\"";
}

void cssString::Load(istream& strm) {
  int c = taMisc::read_till_quote(strm);
    if(c == '\"')			  // "
      c = taMisc::read_till_quote_semi(strm);// then till second followed by semi

  if(c != ';') {
    taMisc::Error("*** Missing ';' in dump file for string:", name);
    return;
  }
  *this = taMisc::LexBuf;	// set via string assgn
}


//////////////////////////
// 	cssBool 	//
//////////////////////////

String& cssBool::GetStr() const {
  if(val == true)
    ((cssEl*)this)->tmp_str = "true";
  else if(val == false)
    ((cssEl*)this)->tmp_str = "false";
  else
    ((cssEl*)this)->tmp_str=String((int)val);
  return (String&)tmp_str;
}

void cssBool::operator=(const cssEl& s) {
  if(s.GetType() == T_String)
    *this = s.GetStr();		// use string converter
  else
    val = (Int)s;
}

void cssBool::operator=(const String& cp) {
  if(cp == "true")
    val = true;
  else if(cp == "false")
    val = false;
  else
    val = (bool)strtol((const char*)cp, NULL, 0);
}


//////////////////////////////////
// 	cssPtr: Pointer Type	//
//////////////////////////////////


void cssPtr::Constr() {
  el_type = &cssMisc::Void;
  Register();
}

void cssPtr::Copy(const cssPtr& cp) {
  cssEl::Copy(cp);
  SetPtr(cp.ptr);
  SetElType(cp.el_type);
}

cssPtr::cssPtr() {
  Constr();
}
cssPtr::cssPtr(const cssElPtr& it) {
  Constr(); SetPtr(it);  SetElType(it.El()->GetTypeObject());
}
cssPtr::cssPtr(cssEl* typ) {
  Constr(); SetElType(typ);
}
cssPtr::cssPtr(cssEl* typ, const char* nm) {
  Constr(); name = nm; SetElType(typ);
}
cssPtr::cssPtr(cssEl* typ, const cssElPtr& it, const char* nm) {
  Constr(); name = nm; SetElType(typ); SetPtr(it); 
}
cssPtr::cssPtr(const cssPtr& cp) {
  Constr(); Copy(cp);
}
cssPtr::cssPtr(const cssPtr& cp, const char* nm) {
  Constr(); Copy(cp); name = nm;
}
cssPtr::~cssPtr() {
  cssEl::DelRefElPtr(ptr);
  if(el_type != &cssMisc::Void) cssEl::unRefDone(el_type);
  el_type = &cssMisc::Void;
}

String cssPtr::PrintStr() const {
  String tmp;
  if((ptr.El()->GetType() == T_Class) || (ptr.El()->GetType() == T_ClassType))
    tmp = String(ptr.El()->GetTypeName())+" "+ptr.El()->name+" { }";
  else
    tmp = (ptr.El())->PrintStr();
  return String(GetTypeName()) + "* " + name + " --> " + tmp;
}
String& cssPtr::GetStr() const {
  ((cssEl*)this)->tmp_str = String("--> ") + (ptr.El())->name + " = " + (ptr.El())->GetStr();
  return (String&)tmp_str;
}

cssEl* cssPtr::GetNonPtrTypeObj() const {
  if((el_type->GetType() == T_Ptr) || (el_type->GetType() == T_Array) ||
     (el_type->GetType() == T_ArrayType))
    return ((cssPtr*)el_type)->GetNonPtrTypeObj();
  return el_type;
}

void cssPtr::operator=(const cssEl& s) { 
  if((s.GetType() == T_Ptr) || (s.GetType() == T_Array)) {
    cssPtr* tmp = (cssPtr*)&s;
    cssEl* s_type = tmp->GetNonPtrTypeObj();
    cssEl* nptr_type = GetNonPtrTypeObj();

    // if it's a void pointer, use type of what it points to
    // if it's a class inst, get its class type
    if(s_type->GetType() == T_Void)
      s_type = tmp->ptr.El();
    if(s_type->GetType() == T_Class)
      s_type = ((cssClassInst*)s_type)->type_def;

    // if either one's a Void, then assignment is automatically ok
    bool auto_ok = false;
    if((nptr_type->GetType() == T_Void) || (s_type->GetType() == T_Void))
      auto_ok = true;
    if(!auto_ok) {
      if(nptr_type->GetType() != s_type->GetType()) {
        cssMisc::Error(prog, "Pointer type mismatch between:",nptr_type->GetTypeName(),"and",
                       s_type->GetTypeName());
        return;
      }
      // if we have class types, check for inheritance
      // allow both up and down casting
      else if(nptr_type->GetType() == T_ClassType) {
        cssClassType* ths_class = (cssClassType*) nptr_type;
        cssClassType* s_class = (cssClassType*) s_type;
        if(!ths_class->InheritsFrom(s_class) &&
           !s_class->InheritsFrom(ths_class)) {
          cssMisc::Error(prog, "Pointer type mismatch between:",nptr_type->GetTypeName(),"and",
                         s_type->GetTypeName());
          return;
        }       
      }
    }

    // assignment is now type-correct if you get here
    // if its an array, we refer to it directly, not to its pointer
    if(s.GetType() == T_Array)
      SetPtr(tmp->GetAddr());
    else
      SetPtr(tmp->ptr);
  }
  else {
    int sval = (Int)s;
    if(sval == 0)
      SetPtr(cssMisc::VoidElPtr);
    else
      cssMisc::Error(prog, "Assigning pointer to non-ptr value");
  }
}

cssElPtr& cssPtr::GetOprPtr() const {
  if(ptr.El()->GetType() == T_Array)
    return ((cssArray*)ptr.El())->ptr;
  else
    return (cssElPtr&)ptr;
}

int cssPtr::GetMemberNo(const char* memb) const {
  if(ptr.El() == &cssMisc::Void) {
    if(el_type->GetType() == T_ClassType)
      return el_type->GetMemberNo(memb);
    return -1;
  }
  return ptr.El()->GetMemberNo(memb);
}

int cssPtr::GetMemberFunNo(const char* memb) const {
  if(ptr.El() == &cssMisc::Void) {
    if(el_type->GetType() == T_ClassType)
      return el_type->GetMemberFunNo(memb);
    return -1;
  }
  return ptr.El()->GetMemberFunNo(memb);
}

cssEl* cssPtr::GetMemberFun(const char* memb) const {
  if(el_type->GetType() == T_ClassType) {
    cssEl* rval = el_type->GetMemberFun(memb);
    if(rval->GetType() == T_MbrScriptFun) {
      cssMbrScriptFun* mbrf = (cssMbrScriptFun*)rval;
      if(mbrf->is_virtual)
	return ptr.El()->GetMemberFun(memb); // get from the pointer itself
      else
	return rval;		// get from the declared class of the ptr
    }
  }
  return ptr.El()->GetMemberFun(memb);
}  
  
cssEl* cssPtr::GetMemberFun(int memb) const {
  if(el_type->GetType() == T_ClassType) {
    cssEl* rval = el_type->GetMemberFun(memb);
    if(rval->GetType() == T_MbrScriptFun) {
      cssMbrScriptFun* mbrf = (cssMbrScriptFun*)rval;
      if(mbrf->is_virtual)
	return ptr.El()->GetMemberFun(memb); // get from the pointer itself
      else
	return rval;		// get from the declared class of the ptr
    }
  }
  return ptr.El()->GetMemberFun(memb);
}  

//////////////////////////
//	Array Type	//
//////////////////////////

void cssArray::Constr() {
  items = NULL;
  Register();
}

void cssArray::Constr(int no) {
  items = new cssSpace(no, ""); 
  // we don't actually have any items yet
  ptr.SetSpace(items);
}

void cssArray::Copy(const cssArray& cp) {
  Constr(cp.items->alloc_size); 
  cssEl::Copy(cp);
  SetElType(cp.el_type);
  el_type->prog = prog;
  items->Copy(*(cp.items));
  if (ptr.El() != &cssMisc::Void) cssEl::unRefDone(ptr.El());
  if (items->size > 0) {
    ptr.SetSpace(items, 0);
    cssEl::Ref(ptr.El());
  }
  else {
    ptr.SetSpace(items);
  }
}

cssArray::cssArray() {
  Constr();
}
cssArray::cssArray(int no) {
  Constr(); Constr(no);
}
cssArray::cssArray(int no, cssEl* it) {
  Constr(); Constr(no);
  SetElType(it);
  prog = el_type->prog;
  Fill(el_type);
}
cssArray::cssArray(int no, const char* nm) {
  Constr(); Constr(no); name = nm;
}
cssArray::cssArray(int no, cssEl* it, const char* nm) {
  Constr(); Constr(no); name = nm;
  SetElType(it);
  prog = el_type->prog;
  Fill(el_type);
}
cssArray::cssArray(const cssArray& cp) {
  Constr(); Copy(cp);
}
cssArray::cssArray(const cssArray& cp, const char* nm) {
  Constr(); Copy(cp);  
  name = nm;
}
cssArray::cssArray(const cssArrayType& cp, const char* nm) {
  Constr();
  prog = cp.prog;
  name = nm;
  Constr(cp.size); 
  SetElType(cp.el_type);
  el_type->prog = prog;
  Fill(el_type);
}
cssArray::~cssArray() {
  if (ptr.El() != &cssMisc::Void) cssEl::unRefDone(ptr.El());
  ptr.Reset();
  if(items != NULL)
    delete items;
  items = NULL;
}

cssEl::RunStat cssArray::Do(cssProg* prg) {
  prog = prg;
  el_type->prog = prog;
  prog->Stack()->Push(this);
  return cssEl::Running;
}

String& cssArray::GetStr() const {
  ((cssEl*)this)->tmp_str=PrintFStr();
  return (String&)tmp_str;
}

int cssArray::Alloc(int no) {
  if (ptr.El() != &cssMisc::Void) cssEl::unRefDone(ptr.El());
  if(items != NULL)
    delete items;
  Constr(no);
  Fill(el_type);
  return true;
}

int cssArray::AllocAll(int na, cssEl* arg[]) {
  int n_alloc = (int)*arg[0];
  Alloc(n_alloc);
  if((na == 0) || (el_type->GetType() != T_Array &&
                   el_type->GetType() != T_ArrayType))
    return true;
  int i;
  for(i=0; i<items->size; i++) {
    ((cssArray*)items->FastEl(i))->AllocAll(na-1, arg+1);
  }
  return true;
}

void cssArray::Fill(cssEl* it) {
  int old_size = items->size;
  if(it->GetType() == T_ClassType) {
    int i;
    for(i=items->size; i<items->alloc_size; i++) {
      cssClassInst* tmp = new cssClassInst((cssClassType*)it, "");
      tmp->prog = prog;
      tmp->SetAddr(items->Push(tmp));
      tmp->ConstructToken();
    }
  }
  else if (it->GetType() == T_ArrayType) {
    int i;
    for(i=items->size; i<items->alloc_size; i++) {
      cssArray* tmp = new cssArray(*(cssArrayType*)it, "");
      tmp->SetAddr(items->Push(tmp));
    }
  }
  else if(it->GetType() == T_EnumType) {
    int i;
    for(i=items->size; i<items->alloc_size; i++) {
      cssEnum* tmp = new cssEnum((cssEnumType*)it, 0, "");
      tmp->SetAddr(items->Push(tmp));
    }
  }
  else {
    int i;
    for(i=items->size; i<items->alloc_size; i++) {
      cssEl* tmp = it->AnonClone();
      tmp->SetAddr(items->Push(tmp));
    }
  }
  if (old_size == 0 && items->size > 0) {
    ptr.SetSpace(items, 0);
    cssEl::Ref(ptr.El());
  }
}
// arg[1] = name, arg[2] = type, arg[3] = number of dimensions
cssEl* cssArray::MakeToken_stub(int, cssEl* arg[]) {
  cssEl* artyp = arg[2];
  int n_dims = (int)*(arg[3]);
  int i;
  for(i=0; i<n_dims; i++)
    artyp = new cssArray(0, artyp, (const char*)*(arg[1]));
  return artyp;
}


String cssArray::PrintStr() const {
  String tmp = items->PrintFStr();
  return String(GetTypeName())+" "+name
    +"["+String((int)items->size)+"] {\n" + tmp + "}";
}
String cssArray::PrintFStr() const {
  String tmp = items->PrintFStr();
  return String("[")+String((int)items->size)+"] {" + tmp + "}";
}
void cssArray::TypeInfo(ostream& fh) const {
  fh << GetTypeName() << " " << name << "[" << items->size << "]";
}

void cssArray::operator=(const cssEl& cp) {
  if(cp.GetType() != T_Array) {
    NoAssgn();
    return;
  }
  cssArray* sary = (cssArray*)&cp;
  int i;
  for(i=0; (i<items->size) && (i<sary->items->size); i++) {
    cssEl* trg = items->FastEl(i);
    cssEl* src = sary->items->FastEl(i);
    *trg = *src;		// do an item-wise copy
  }
}

void cssArray::Save(ostream& strm) {
  strm << "[" << items->size << "] { ";
  int i;
  for(i=0; i<items->size; i++) {
    cssEl* itm = items->FastEl(i);
    itm->Save(strm);
    strm << ";";
  }
  strm << "}";
}  

void cssArray::Load(istream& strm) {
  int c = taMisc::skip_white(strm, true);
  if(c != '[') {
    taMisc::Error("Did not get array size");
    taMisc::skip_past_err(strm);
    return;
  }
  strm.get();
  c = taMisc::read_word(strm, true);
  if(c != ']') {
    taMisc::Error("Did not get array size");
    taMisc::skip_past_err(strm);
    return;
  }
  int sz = (int)taMisc::LexBuf;
  Alloc(sz);

  c = taMisc::read_till_lbracket(strm, true);
  if(c != '{') {
    taMisc::Error("Did not get array starting bracket");
    taMisc::skip_past_err(strm);
    return;
  }
  strm.get();			// get the bracket
  int i;
  for(i=0; i<items->size; i++) {
    cssEl* itm = items->FastEl(i);
    itm->Load(strm);
    if(strm.peek() == ';') strm.get(); // skip past ending semi
    if(strm.peek() == '}') 
      break;
  }
  c = taMisc::read_till_rbracket(strm, true);
  if(c != '}') {
    taMisc::Error("Did not get array terminator '}'");
    taMisc::skip_past_err(strm);
    return;
  }
  strm.get();			// get rb
  if(strm.peek() == ';') strm.get(); // skip past ending semi
}  


//////////////////////////
//    Array Def Type    //
//////////////////////////

void cssArrayType::Constr() {
  size = 0;
  Register();
}
void cssArrayType::Copy(const cssArrayType& cp) {
  cssEl::Copy(cp);
  SetSize(cp.size);
  SetElType(cp.el_type);
}
cssArrayType::cssArrayType() {
   Constr();
   SetElType(&cssMisc::Void);
}
cssArrayType::cssArrayType(int no, cssEl* it)
{
  Constr();
  SetSize(no);
  SetElType(it);
}
cssArrayType::cssArrayType(int no, cssEl* it, const char* nm)
{
  Constr();
  name = nm;
  SetSize(no);
  SetElType(it);
}
cssArrayType::cssArrayType(const cssArrayType& cp)
{
  Constr();
  Copy(cp); 
}
cssArrayType::cssArrayType(const cssArrayType& cp, const char* nm)
{
  Constr();
  Copy(cp); 
  name = nm;
}
cssArrayType::~cssArrayType()
{
}
int cssArrayType::AllocAll(int na, cssEl* arg[]) {
  int n_alloc = (int)*arg[0];
  Alloc(n_alloc);
  if((na == 0) || (el_type->GetType() != T_ArrayType))
    return true;
  ((cssArrayType*)el_type)->AllocAll(na-1, arg+1);
  return true;
}

// arg[1] = name, arg[2 .. na-2] = sizes, arg[na-1] = n_dims, arg[na] = type
cssEl* cssArrayType::MakeToken_stub(int na, cssEl* arg[]) {
  cssEl* artyp = arg[na];
  int n_dims = (int)*(arg[na-1]);
  int i;
  for(i=0; i<n_dims; i++) {
    artyp = new cssArrayType((int)*(arg[na-2-i]), artyp, (const char*)*(arg[1]));
  }
  return artyp;
}
String cssArrayType::PrintStr() const {
  return String(GetTypeName())+" "+name+
    "["+String(size)+"] { (definition only) }";
}
String cssArrayType::PrintFStr() const {
  return String("[")+String(size)+"] { (definition only) }";
}
void cssArrayType::TypeInfo(ostream& fh) const {
  fh << GetTypeName() << " " << name << "[" << size << "]";
}


/////////////////////////////////
// 	Reference Type	       //
/////////////////////////////////

cssElPtr cssRef::GetAddr() const {
  if(addr != NULL)
    return *addr;
  return ptr;		// its always the thing you refer to, not you!
}

void cssRef::operator=(const cssEl& cp) {
  if(cp.IsRef()) {
    cssRef* tmp = (cssRef*)&cp;
    ptr = tmp->ptr; 
  }
  else {
    ptr.El()->operator=(cp);
  }
}

void cssRef::InitAssign(const cssEl& cp) {
  if(cp.IsRef()) {
    cssRef* tmp = (cssRef*)&cp;
    ptr = tmp->ptr; 
  }
  else {
    ptr = cp.GetAddr();
  }
}

// todo: ref needs base class type to perform correctly for non-virt functions!!
int cssRef::GetMemberNo(const char* memb) const {
  if(ptr.El() == &cssMisc::Void) {
    return -1;
  }
  return ptr.El()->GetMemberNo(memb);
}

int cssRef::GetMemberFunNo(const char* memb) const {
  if(ptr.El() == &cssMisc::Void) {
    return -1;
  }
  return ptr.El()->GetMemberFunNo(memb);
}

//////////////////////////
// 	cssEnumType	//
//////////////////////////

void cssEnumType::Constr() {
  type_name = "(Enum)";
  enums = new cssSpace;
  enums->name = "enums";
  enum_cnt = 0;
  Register();
}

void cssEnumType::Copy(const cssEnumType& cp) {
  cssEl::Copy(cp);
  type_name = cp.type_name;
  enums->Copy(*(cp.enums));
}

cssEnumType::cssEnumType(const cssEnumType& cp) {
  Constr(); Copy(cp);
}
cssEnumType::cssEnumType(const cssEnumType& cp, const char* nm) {
  Constr(); Copy(cp);
  name = nm;
}
cssEnumType::~cssEnumType() {
  delete enums;
}

void cssEnumType::SetTypeName(const char* nm) {
  name = nm; type_name = String("(") + nm + ")";
  enums->name = name + "::enums";
}

void cssEnumType::TypeInfo(ostream& fh) const {
  fh << "enum " << name << " {\n";
  enums->TypeNameValList(fh);
  fh << "}\n";
  fh.flush();
}

cssEl* cssEnumType::GetScoped(const char* memb) const {
  cssElPtr rval = enums->FindName(memb);
  if(rval == 0) {
    cssMisc::Error(prog, "Scoped type not found:", memb, "in enum:",GetTypeName());
    return &cssMisc::Void;
  }
  return rval.El();
}

// the enum type makes an enum instance of itself..
cssEl* cssEnumType::MakeToken_stub(int, cssEl* arg[]) {
  return new cssEnum(this, 0, (const char*)*arg[1]);
}

cssEl* cssEnumType::FindValue(int val) const {
  int i;
  for(i=0; i<enums->size; i++) {
    cssEl* itm = enums->FastEl(i);
    if((int)*itm == val)
      return itm;
  }    
  return &cssMisc::Void;
}



//////////////////////////
// 	cssEnum 	//
//////////////////////////

void cssEnum::Constr() {
  type_def = &cssMisc::VoidEnumType;
}

void cssEnum::Copy(const cssEnum& cp) {
  cssInt::Copy(cp);
  SetEnumType(cp.type_def);
}

cssEnum::cssEnum(const cssEnum& cp) {
  Constr();
  Copy(cp);
}

cssEnum::cssEnum(const cssEnum& cp, const char* nm) {
  Constr();
  Copy(cp);
  name = nm;
}

cssEnum::cssEnum(cssEnumType* cp, Int vl, const char* nm) {
  Constr();
  val = vl;
  name = nm;
  SetEnumType(cp);
}

cssEnum::~cssEnum() {
  if(type_def != &cssMisc::VoidEnumType)
    cssEl::unRefDone(type_def);
  type_def = &cssMisc::VoidEnumType;
}

const char* cssEnum::GetTypeName() const {
  return (const char*)type_def->type_name;
}

String cssEnum::PrintStr() const {
  cssEl* nm = type_def->FindValue(val);
  if(nm == &cssMisc::Void)
    return cssInt::PrintStr();
  return String(GetTypeName())+" " + name + " = " + nm->name;
}

String cssEnum::PrintFStr() const {
  cssEl* nm = type_def->FindValue(val);
  if(nm == &cssMisc::Void)
    return cssInt::PrintFStr();
  return nm->name;
}

String& cssEnum::GetStr() const {
  cssEl* nm = type_def->FindValue(val);
  if(nm == &cssMisc::Void)
    return cssInt::GetStr();
  ((cssEl*)this)->tmp_str = nm->name;
  return (String&)tmp_str;
}
 
void cssEnum::operator=(const String& cp) {
  cssElPtr val_ptr = type_def->enums->FindName((const char*)cp);
  if(val_ptr == 0)
    val = (int)strtol((const char*)cp, NULL, 0);
  else
    val = ((cssEnum*)val_ptr.El())->val;
}

void cssEnum::operator=(const cssEl& s) {
  if(s.GetType() == T_String)
    *this = s.GetStr();		// use string converter
  else
    val = (Int)s;
}

bool cssEnum::operator==(cssEl& s) {
  cssElPtr val_ptr = type_def->enums->FindName((const char*)s);
  if(val_ptr == 0)
    return (val == (Int)s);
  else
    return (val == ((cssEnum*)val_ptr.El())->val);
}

bool cssEnum::operator!=(cssEl& s) {
  cssElPtr val_ptr = type_def->enums->FindName((const char*)s);
  if(val_ptr == 0)
    return (val != (Int)s);
  else
    return (val != ((cssEnum*)val_ptr.El())->val);
}


//////////////////////////////////
// 	css Class Member 	//
//////////////////////////////////

void cssClassMember::Constr() {
  mbr_type = &cssMisc::Void;
}

void cssClassMember::Copy(const cssClassMember& cp) {
  cssEl::Copy(cp);
  SetMbrType(cp.mbr_type);
}

cssClassMember::cssClassMember(cssEl* mbtp) {
  Constr();
  SetMbrType(mbtp);
}

cssClassMember::cssClassMember(cssEl* mbtp, const char* mbnm) {
  Constr();
  SetMbrType(mbtp);
  name = mbnm;
}

cssClassMember::cssClassMember(const cssClassMember& cp) {
  Constr();
  Copy(cp);
}

cssClassMember::cssClassMember(const cssClassMember& cp, const char* nm) {
  Constr();
  Copy(cp);
  name = nm;
}

cssClassMember::~cssClassMember() {
  cssEl::DelRefPointer(&mbr_type);
}

String cssClassMember::PrintStr() const {
  return String(mbr_type->GetTypeName()) + " " + name;
}

String cssClassMember::PrintFStr() const {
  return String(mbr_type->GetTypeName()) + " " + name;
}


//////////////////////////////////
// 	css Class Type  	//
//////////////////////////////////

void cssClassType::Constr() {
  type_name = "(Class)";
  members = new cssSpace;
  methods = new cssSpace;
  parents = new cssSpace;
  types = new cssSpace;
  last_top = NULL;
  multi_space = false;
  members->name = "members";
  methods->name = "methods";
  parents->name = "parents";
  types->name = "types";
  members->el_retv.SetClassMember(this); // any reference to a class member needs updated..
  methods->el_retv.SetNVirtMethod(this); // non-virtual is default assumption
  Register();
}

void cssClassType::AddBuiltins() {
  methods->PushUniqNameOld
    (new cssMbrCFun(2, NULL, &cssClassType::InheritsFrom_stub, "InheritsFrom"));
  methods->PushUniqNameOld
    (new cssMbrCFun(2, NULL, &cssClassType::Load_stub, "Load"));
  methods->PushUniqNameOld
    (new cssMbrCFun(2, NULL, &cssClassType::Save_stub, "Save"));
}

void cssClassType::Copy(const cssClassType& cp) {
  cssEl::Copy(cp);
  type_name = cp.type_name;
  members->Copy(*(cp.members));
  member_desc.Duplicate(cp.member_desc);
  member_opts.Duplicate(cp.member_opts);
  methods->Copy(*(cp.methods));
  parents->Copy(*(cp.parents));
  types->Copy(*(cp.types));
  desc = cp.desc;
  opts = cp.opts;
  last_top = cp.last_top;
  multi_space = cp.multi_space;
}

cssClassType::cssClassType(const cssClassType& cp) {
  Constr();
  Copy(cp);
}
cssClassType::cssClassType(const cssClassType& cp, const char* nm) {
  Constr();
  Copy(cp);
  name = nm;
}
cssClassType::~cssClassType() {
  delete members;
  delete methods;
  delete parents;
  delete types;
}

// the class type makes a class instance of itself unless a membdefn
cssEl* cssClassType::MakeToken_stub(int, cssEl* arg[]) {
  if(cssMisc::parsing_membdefn)
    return Clone();
  else
    return new cssClassInst(this, (const char*)*arg[1]);
}

// invoke the constructor
cssEl* cssClassType::NewOpr() {
  MakeTempToken(cssMisc::cur_top->Prog());
  cssClassInst* itm = (cssClassInst*) cssMisc::cur_top->Prog()->Stack()->Pop();
  itm->ConstructToken();
  cssElPtr ptr((cssEl*) itm);
  return new cssPtr(ptr);       // make it a pointer to the item..
}


void cssClassType::ConstructToken(cssClassInst* tok) {
  int i;
  for(i=0; i<parents->size; i++) {
    ((cssClassType*)parents->FastEl(i))->ConstructToken(tok);
  }
  for(i=0; i<tok->members->size; i++) {
    cssEl* mb = tok->members->FastEl(i);
    mb->prog = tok->prog;
    if(mb->GetType() == T_Class)
      ((cssClassInst*)mb)->ConstructToken();
    // arrays of objects already constructed via cssArray::Fill()
  }
  ConstructToken_impl(tok);
}

void cssClassType::DestructToken(cssClassInst* tok) {
  int i;
//  members will get destructors run when they are unref'ed
  DestructToken_impl(tok);	// destroy me first, then parents..
  for(i=0; i<parents->size; i++) {
    ((cssClassType*)parents->FastEl(i))->DestructToken(tok);
  }
}

void cssClassType::ConstructToken_impl(cssClassInst* tok) {
  String nm = type_name;
  if(nm.contains(')')) {
    nm = nm.before(')');
    nm = nm.after('(');
  }
  cssElPtr ptr = methods->FindName((const char*)nm); // find name of this type..
  if(ptr == 0) return;				       // didn't find it..
  cssMbrScriptFun* ctor = (cssMbrScriptFun*)methods->FastEl(ptr.dx);

  // get the appropriate context to run constructor in: either token top
  // or the generic top-level
  cssProg* prg = cssMisc::CDtorProg;
  cssProgSpace* old_top = NULL;
  cssProgSpace* old_tok_top = NULL;
  cssProgSpace* old_prg_top = NULL;
  if((tok->prog != NULL) && (tok->prog->top != NULL)) {
    old_top = cssMisc::SetCurTop(tok->prog->top);    // reparent to current top
    old_prg_top = prg->SetTop(cssMisc::cur_top);
  }
  else {
    old_top = cssMisc::SetCurTop(cssMisc::Top);
    old_prg_top = prg->SetTop(cssMisc::cur_top);
    tok->prog = prg;
  }

  prg->Stack()->Push(&cssMisc::Void); // argstop
  prg->Stack()->Push(tok);
  cssEl::Ref(tok);		// make sure it doesn't get to 0 refcount in arg del..
  ctor->Do(prg);
  cssEl::unRef(tok);		// undo that
  
  if(old_prg_top != NULL)	prg->PopTop(old_prg_top);
  if(old_tok_top != NULL)	tok->prog->PopTop(old_prg_top);
  if(old_top != NULL)		cssMisc::PopCurTop(old_top);
}

void cssClassType::DestructToken_impl(cssClassInst* tok) {
  String nm = type_name;
  if(nm.contains(')')) {
    nm = nm.before(')');
    nm = nm.after('(');
  }
  nm = "~" + nm;
  cssElPtr ptr = methods->FindName((const char*)nm); // find name of this type..
  if(ptr == 0) return;				       // didn't find it..
  cssMbrScriptFun* dtor = (cssMbrScriptFun*)methods->FastEl(ptr.dx);

  cssProg* prg = cssMisc::CDtorProg;
  cssProgSpace* old_top = cssMisc::SetCurTop(cssMisc::Top);
  cssProgSpace* old_prg_top = prg->SetTop(cssMisc::cur_top);
  tok->prog = prg;

  prg->Stack()->Push(&cssMisc::Void); // argstop
  prg->Stack()->Push(tok);
  cssEl::Ref(tok);		// make sure it doesn't get to 0 refcount in arg del..
  dtor->Do(prg);
  cssEl::unRef(tok);		// undo that

  prg->PopTop(old_prg_top);
  tok->prog->PopTop(old_prg_top);
  cssMisc::PopCurTop(old_top);
}

void cssClassType::UpdateAfterEdit_impl(cssClassInst* tok) {
  cssElPtr ptr = methods->FindName("UpdateAfterEdit");
  if(ptr == 0) return;				       // didn't find it..
  cssMbrScriptFun* ctor = (cssMbrScriptFun*)methods->FastEl(ptr.dx);

  // get the appropriate context to run constructor in: either token top
  // or the generic top-level
  cssProg* prg = cssMisc::CDtorProg;
  cssProgSpace* old_top = NULL;
  cssProgSpace* old_tok_top = NULL;
  cssProgSpace* old_prg_top = NULL;
  if((tok->prog != NULL) && (tok->prog->top != NULL)) {
    old_top = cssMisc::SetCurTop(tok->prog->top);    // reparent to current top
    old_prg_top = prg->SetTop(cssMisc::cur_top);
  }
  else {
    old_top = cssMisc::SetCurTop(cssMisc::Top);
    old_prg_top = prg->SetTop(cssMisc::cur_top);
    tok->prog = prg;
  }

  prg->Stack()->Push(&cssMisc::Void); // argstop
  prg->Stack()->Push(tok);
  cssEl::Ref(tok);		// make sure it doesn't get to 0 refcount in arg del..
  ctor->Do(prg);
  cssEl::unRef(tok);		// undo that
  
  if(old_prg_top != NULL)	prg->PopTop(old_prg_top);
  if(old_tok_top != NULL)	tok->prog->PopTop(old_prg_top);
  if(old_top != NULL)		cssMisc::PopCurTop(old_top);
}

void cssClassType::SetTypeName(const char* nm) {
  name = nm; type_name = String("(") + nm + ")";
  members->name = name + "::members";
  methods->name = name + "::methods";
  parents->name = name + "::parents";
  types->name = name + "::types";
}

String& cssClassType::GetStr() const {
  ((cssEl*)this)->tmp_str=PrintFStr();
  return (String&)tmp_str;
}
void cssClassType::AddParent(cssClassType* par) {
  parents->Push(par);
  members->CopyUniqNameNew(*(par->members));
  // copy in member_desc and member_opts into proper slots
  member_desc.EnforceSize(members->size);
  member_opts.EnforceSize(members->size);
  int i;
  for(i=0; i < par->members->size; i++) {
    int dx = members->FindName(par->members->FastEl(i)->name).dx;
    member_desc[dx] = par->member_desc[i];
    member_opts[dx] = par->member_opts[i];
  }
  methods->CopyUniqNameNew(*(par->methods));
  types->CopyUniqNameNew(*(par->types));
}

bool cssClassType::InheritsFrom(const cssClassType* cp) {
  // identity?
  if(strcmp(GetTypeName(), cp->GetTypeName()) == 0)
    return true;

  // recurse through parents
  int i;
  for (i = 0; i < parents->size; i++) {
    cssClassType* par = (cssClassType*) parents->El(i);
    if(par->InheritsFrom(cp)) return true;
  }
  return false;
}

cssEl* cssClassType::InheritsFrom_stub(void*, int, cssEl* arg[]) {
  // 'this' must be a class or class object to even get here
  cssClassType* ths;
  if(arg[1]->GetType() == T_Class)
    ths = ((cssClassInst*)arg[1])->type_def;
  else
    ths = (cssClassType*)arg[1];

  cssEl* arg2 = arg[2]->GetActualObj();
  cssClassType* that;
  if(arg2->GetType() == T_Class)
    that = ((cssClassInst*)arg2)->type_def;
  else if (arg2->GetType() == T_ClassType)
    that = (cssClassType*)arg2;
  else {
    cssElPtr tp_ptr;
    if((ths->prog != NULL) && (ths->prog->top != NULL))
      tp_ptr = ths->prog->top->FindTypeName((const char*)*arg2);
    else
      tp_ptr = ths->last_top->FindTypeName((const char*)*arg2);
    if((tp_ptr == 0) || (tp_ptr.El()->GetType() != T_ClassType)) {
      cssMisc::Error(ths->prog, "argument is not a class, class object, or class type name");
      return &cssMisc::Void;
    }
    that = (cssClassType*)tp_ptr.El();
  }
  return new cssBool(ths->InheritsFrom(that));
}

cssEl* cssClassType::Load_stub(void*, int, cssEl* arg[]) {
  // 'this' must be a class or class object to even get here
  cssClassInst* ths;
  if(arg[1]->GetType() == T_Class)
    ths = (cssClassInst*)arg[1];
  else {
    cssMisc::Error(NULL, "Load must be called on a class instance, not just a type");
    return &cssMisc::Void;
  }

  istream* fh = (istream*)*(arg[2]);
  if((fh == NULL) || !fh->good())
    return &cssMisc::Void;
  ths->Load(*fh);
  return &cssMisc::Void;
}

cssEl* cssClassType::Save_stub(void*, int, cssEl* arg[]) {
  // 'this' must be a class or class object to even get here
  cssClassInst* ths;
  if(arg[1]->GetType() == T_Class)
    ths = (cssClassInst*)arg[1];
  else {
    cssMisc::Error(NULL, "Save must be called on a class instance, not just a type");
    return &cssMisc::Void;
  }

  ostream* fh = (ostream*)*(arg[2]);
  if((fh == NULL) || !fh->good())
    return &cssMisc::Void;
  ths->Save(*fh);
  return &cssMisc::Void;
}

String	cssClassType::PrintStr() const {
  String tmp = members->PrintStr();
  return String(GetTypeName())+" "+name+" {\n" + tmp + "\n}";
}
String	cssClassType::PrintFStr() const {
  String tmp = members->PrintStr();
  return String("{\n") + tmp + "\n}";
}
void cssClassType::InheritInfo(ostream& fh) const {
  if(parents->size > 0) {
    fh << name << " : ";
    int i;
    (*parents)[0]->InheritInfo(fh);
    for(i=1; i<parents->size; i++) {
      fh << ", ";
      (*parents)[i]->InheritInfo(fh);
    }
  }
  else {
    fh << name;
  }
}

void cssClassType::TypeInfo(ostream& fh) const {
  fh << "class ";
  InheritInfo(fh);
  fh << " {\n  // " << desc << "\n";
  if(types->size > 0) {
    fh << "\n  // types\n";
    types->TypeNameValList(fh);
  }
  fh << "\n  // members\n";
  int i;
  for(i=0; i<members->size; i++) {
    cssClassMember* mbr = (cssClassMember*)members->FastEl(i);
    String tmp = mbr->mbr_type->GetTypeName();
    if(tmp.contains(')')) {
      tmp = tmp.before(')');
      tmp = tmp.after('(');
    }
    tmp = String("  ") + tmp;
    fh << tmp;
    if(tmp.length() >= 24)
      fh << " ";
    else if(tmp.length() >= 16)
      fh << "\t";
    else if(tmp.length() >= 8)
      fh << "\t\t";
    else
      fh << "\t\t\t";
    fh << mbr->name;
    if (mbr->mbr_type->GetType() == cssEl::T_Array) {
      cssArray* ar = (cssArray*) mbr->mbr_type;
      fh << '[' << ar->items->size << ']';
    }
    else if (mbr->mbr_type->GetType() == cssEl::T_ArrayType) {
      cssArrayType* ar = (cssArrayType*) mbr->mbr_type;
      fh << '[' << ar->size << ']';
    }
    if(!member_desc[i].empty())
      fh << "\t// " << member_desc[i];
    fh << "\n";
  }
  fh << "\n  // methods\n";
  for(i=0; i<methods->size; i++) {
    if(methods->FastEl(i)->GetType() != T_MbrScriptFun) {
      fh << "  builtin:\t\t" << methods->FastEl(i)->PrintStr() << "\n";
      continue;
    }
    cssMbrScriptFun* meth = (cssMbrScriptFun*)methods->FastEl(i);
    if((cssMisc::cur_top->debug == 0) && meth->is_tor)
      continue;
    String tmp = String(meth->argv[0].El()->GetTypeName());
    tmp = tmp.before(')');
    tmp = tmp.after('(');
    tmp = String("  ") + tmp;
    fh << tmp;
    if(tmp.length() >= 24)
      fh << " ";
    else if(tmp.length() >= 16)
      fh << "\t";
    else if(tmp.length() >= 8)
      fh << "\t\t";
    else
      fh << "\t\t\t";
    fh << meth->name << "(";
    int j;
    for(j=2; j<=meth->argc; j++) {
      fh << meth->argv[j].El()->GetTypeName() << " " << meth->argv[j].El()->name;
      if(j >= meth->def_start) {
	fh << " = " << meth->arg_defs.El((j-meth->def_start))->PrintFStr();
      }
      if(j < meth->argc)
	fh << ", ";
    }
    fh << ")\t// " << meth->desc;
    if(prog->top->debug > 0)
      fh << " top: " << meth->fun->top->name;
    fh << "\n";
  }
  fh << "}\n";
  fh.flush();
}

void cssClassType::SetDesc_impl(String& dsc, String& opt, const char* des) {
  dsc = "";
  String tmp = des;
  tmp.gsub("\"", "'");		// don't let any quotes get through
  tmp.gsub("\n", " ");		// replace whitespace..
  tmp.gsub("\t", " ");		// replace whitespace..
  String ud;
  while(tmp.contains('#')) {
    dsc += tmp.before('#');
    tmp = tmp.after('#');
    if(tmp.contains(' '))
      ud = tmp.before(' ');
    else
      ud = tmp;
    tmp = tmp.after(' ');
    opt += ud + " ";
  }
  dsc += tmp;
}

String cssClassType::OptionAfter_impl(const String& optns, const char* opt) const {
  if(!optns.contains(opt))
    return "";
  String rval = ((String&)optns).after(opt);
  rval = rval.before(' ');
  return rval;
}

void cssClassType::GetComments(cssEl* obj, cssElPtr cmt) {
  if (cmt == cssMisc::VoidElPtr)
    return;
  member_desc.EnforceSize(members->size);
  member_opts.EnforceSize(members->size);

  int mbridx;
  
  String comment = ((cssString*)cmt.El())->val;
  
  if (obj == this) {    // class description
    SetDesc(comment);
  }
  else if (obj->GetType() == T_MbrScriptFun) {  // method description
    ((cssMbrScriptFun*)obj)->SetDesc(comment);
  }
  else if ((mbridx = GetMemberNo(obj->name)) >= 0) { // member description
    MbrSetDesc(mbridx, comment);
  }
}

int cssClassType::GetMemberNo(const char* memb) const {
  cssElPtr rval = members->FindName(memb);
  if(rval == 0)
    return -1;
  return rval.dx;
}
cssEl* cssClassType::GetMember(const char* memb) const {
  cssMisc::Warning(prog, "Getting member:",memb,"from class type (not inst)", GetTypeName());
  cssElPtr rval = members->FindName(memb);
  if(rval == 0) {
    cssMisc::Error(prog, "Member not found:", memb, "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  return rval.El();
}
cssEl* cssClassType::GetMember(int memb) const {
  cssEl* rval = members->El(memb);
  if(rval == NULL) {
    cssMisc::Error(prog, "Member not found:", String(memb), "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  return rval;
}

int cssClassType::GetMemberFunNo(const char* memb) const {
  cssElPtr rval = methods->FindName(memb);
  if(rval == 0)
    return -1;
  return rval.dx;
}
cssEl* cssClassType::GetMemberFun(const char* memb) const {
  cssElPtr rval = methods->FindName(memb);
  if(rval == 0) {
    cssMisc::Error(prog, "Member function not found:", memb, "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  if(rval.El()->GetType() != T_MbrScriptFun &&
     rval.El()->GetType() != T_MbrCFun) {
    cssMisc::Error(prog, "Member:", memb, "of type:", rval.El()->GetTypeName(), 
	      "is not a function, in class:", name, "of type:", GetTypeName());
    return &cssMisc::Void;
  }
  return rval.El();
}
cssEl* cssClassType::GetMemberFun(int memb) const {
  cssEl* rval = methods->El(memb);
  if(rval == NULL) {
    cssMisc::Error(prog, "Member function not found:", String(memb), "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  if(rval->GetType() != T_MbrScriptFun &&
     rval->GetType() != T_MbrCFun) {
    cssMisc::Error(prog, "Member:", String(memb), "of type:", rval->GetTypeName(), 
	      "is not a function, in class:", name, "of type:", GetTypeName());
    return &cssMisc::Void;
  }
  return rval;
}
cssEl* cssClassType::GetScoped(const char* memb) const {
  cssElPtr rval = types->FindName(memb);
  if(rval == 0) {
    rval = methods->FindName(memb);
    if(rval == 0) {
      rval = members->FindName(memb);
      if(rval == 0) {
	cssMisc::Error(prog, "Scoped type not found:", memb, "in class:", name, "of type:",
		       GetTypeName());
	return &cssMisc::Void;
      }
      else
	rval = ((cssClassMember*)rval.El())->mbr_type;
    }
  }
  return rval.El();
}


//////////////////////////////////
// 	css Class Inst  	//
//////////////////////////////////

void cssClassInst::Constr() {
  members = new cssSpace;
  members->name = "members";
  type_def = &cssMisc::VoidClassType;
  Register();
}

void cssClassInst::Copy(const cssClassInst& cp) {
  cssEl::Copy(cp);
  SetClassType(cp.type_def);
  members->Copy(*(cp.members));
}

cssClassInst::cssClassInst(const cssClassInst& cp) {
  Constr();
  Copy(cp);
}
cssClassInst::cssClassInst(const cssClassInst& cp, const char* nm) {
  Constr();
  Copy(cp);
  name = nm;
}

cssClassInst::cssClassInst(cssClassType* cp, const char* nm) {
  Constr();
  name = nm;
  SetClassType(cp);
  int i;
  for(i=0; i < cp->members->size; i++) {
    cssClassMember* mb = (cssClassMember*)cp->members->FastEl(i);
    if(mb->mbr_type->GetType() == T_ClassType) {
      members->Push(new cssClassInst((cssClassType*)mb->mbr_type, mb->name));
    }
    else if(mb->mbr_type->GetType() == T_ArrayType) {
      members->Push(new cssArray(*(cssArrayType*)mb->mbr_type, mb->name));
    }
    else if(mb->mbr_type->GetType() == T_EnumType) {
      members->Push(new cssEnum((cssEnumType*)mb->mbr_type, 0, mb->name));
    }
    else {
      cssEl* nwmb = mb->mbr_type->Clone();
      nwmb->name = mb->name;
      members->Push(nwmb);
    }
  }
}

cssClassInst::~cssClassInst() {
  // one could do this, but its probably a bit slow..
  // cssivSession::CancelObjEdits(this);
  DestructToken();
  delete members;
  if(type_def != &cssMisc::VoidClassType)
    cssEl::unRefDone(type_def);
  type_def = &cssMisc::VoidClassType;
}

void cssClassInst::ConstructToken() {
  if(type_def != &cssMisc::VoidClassType)
    type_def->ConstructToken(this);
}

void cssClassInst::DestructToken() {
  if(type_def != &cssMisc::VoidClassType)
    type_def->DestructToken(this);
}

void cssClassInst::UpdateAfterEdit() {
  if(type_def != &cssMisc::VoidClassType)
    type_def->UpdateAfterEdit_impl(this);
}

int cssClassInst::Edit(bool wait) {
  if(!taMisc::iv_active)
    return false;
  
  cssProgSpace* top;
  if(prog != NULL)
    top = prog->top;
  else
    top = cssMisc::cur_top;
  cssivEditDialog* dlg = new cssivEditDialog(this, top);
  dlg->Constr(NULL, wait);
  return dlg->Edit();
}

String& cssClassInst::GetStr() const {
  ((cssEl*)this)->tmp_str=PrintFStr();
  return (String&)tmp_str;
}

String	cssClassInst::PrintStr() const {
  String tmp = members->PrintStr();
  return String(GetTypeName())+" "+name+" {\n" + tmp + "\n}";
}
String	cssClassInst::PrintFStr() const {
  String tmp = members->PrintStr();
  return String("{\n") + tmp + "\n}";
}

const char* cssClassInst::GetTypeName() const {
  return type_def->GetTypeName();
}
void cssClassInst::InheritInfo(ostream& fh) const {
  type_def->InheritInfo(fh);
}

void cssClassInst::TypeInfo(ostream& fh) const {
  type_def->TypeInfo(fh);
}

int cssClassInst::GetMemberNo(const char* memb) const {
  cssElPtr rval = members->FindName(memb);
  if(rval == 0)
    return -1;
  return rval.dx;
}
cssEl* cssClassInst::GetMember(const char* memb) const {
  cssElPtr rval = members->FindName(memb);
  if(rval == 0) {
    cssMisc::Error(prog, "Member not found:", memb, "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  return rval.El();
}
cssEl* cssClassInst::GetMember(int memb) const {
  cssEl* rval = members->El(memb);
  if(rval == NULL) {
    cssMisc::Error(prog, "Member not found:", String(memb), "in class:", name, "of type:",
	      GetTypeName());
    return &cssMisc::Void;
  }
  return rval;
}

int cssClassInst::GetMemberFunNo(const char* memb) const {
  if(type_def != &cssMisc::VoidClassType)
    return type_def->GetMemberFunNo(memb);
  return -1;
}
cssEl* cssClassInst::GetMemberFun(const char* memb) const {
  if(type_def != &cssMisc::VoidClassType)
    return type_def->GetMemberFun(memb);
  cssMisc::Error(prog, "Member function not found:", memb, "in class:", name, "of type:",
		 GetTypeName());
  return &cssMisc::Void;
}
cssEl* cssClassInst::GetMemberFun(int memb) const {
  if(type_def != &cssMisc::VoidClassType)
    return type_def->GetMemberFun(memb);
  cssMisc::Error(prog, "Member function not found:", String(memb), "in class:", name, "of type:",
		 GetTypeName());
  return &cssMisc::Void;
}
cssEl* cssClassInst::GetScoped(const char* memb) const {
  if(type_def != &cssMisc::VoidClassType)
    return type_def->GetScoped(memb);
  cssMisc::Error(prog, "Scoped type not found:", memb, "in class:", name, "of type:",
		 GetTypeName());
  return &cssMisc::Void;
}

void cssClassInst::operator=(const cssEl& s) {
  if(s.GetType() != T_Class) {
    cssMisc::Error(prog, "Assign between a class and a non-class not allowed");
    return;
  }
  cssClassInst* tmp = (cssClassInst*)&s;
  int i;
  cssElPtr pt;
  for(i=0; i<members->size; i++) {
    if((pt = tmp->members->FindName((char*)members->els[i]->name)) != 0) {
      *(members->els[i]) = *(pt.El());
    }
  }
}

void cssClassInst::Load(istream& strm) {
  int c;
  c = taMisc::read_till_lbracket(strm,false); // get the lbrack
  if(c == EOF) return;
  if(c != '{') {
    taMisc::Error("*** left bracket not found for start of class");
    taMisc::skip_past_err(strm);
    return;
  }
  String tp_nm = taMisc::LexBuf;
  tp_nm = tp_nm.before(' ');
  cssElPtr tpptr = prog->top->FindTypeName(tp_nm);
  if((tpptr == 0) || (tpptr.El()->GetType() != T_ClassType) || 
     !type_def->InheritsFrom((cssClassType*)tpptr.El())) {
    taMisc::Error("*** type name:",tp_nm,"not a valid type, or this type:",
		  type_def->name, "does not inherit from it");
    taMisc::skip_past_err(strm);
    return;
  }

  do {
    c = taMisc::read_word(strm, true);
    if(c == EOF) return;
    if(c == '}') {
      strm.get();
      if(strm.peek() == ';') strm.get(); // skip past ending semi
      return;
    }
    
    String mb_name = taMisc::LexBuf;
    if((c == ' ') || (c == '\n') || (c == '\t')) { // skip past white
      strm.get();
      c = taMisc::skip_white(strm, true);
    }
    if(c != '=') {
      taMisc::Error("*** did not get '=' for member");
      taMisc::skip_past_err(strm);
      continue;
    }
    else {
      strm.get();		// get the equals sign
      c = taMisc::skip_white(strm, true); // then skip past any new whitespace
    }

    cssElPtr mb_ptr = members->FindName((const char*)mb_name);
    if(mb_ptr == 0) {
      taMisc::Error("*** Member:", mb_name, "not found in type:", type_def->name);
      taMisc::skip_past_err(strm);
      continue;
    }
    cssEl* mb = mb_ptr.El();

    mb->Load(strm);

  } while(1);
}

void cssClassInst::Save(ostream& strm) {
  strm << type_def->name << " {\n";
  int i;
  for(i=0; i<members->size; i++) {
    cssEl* mb = members->FastEl(i);
    strm << mb->name << " = " ;
    mb->Save(strm);
    strm << ";\n";

  }
  strm << "}";
}



//////////////////////////////////
// 	css Sub Shell	  	//
//////////////////////////////////

String cssSubShell::PrintStr() const {
  return String(GetTypeName()) + " " + name + " -> " + prog_space.name;
}

String cssSubShell::PrintFStr() const {
  return prog_space.name;
}

cssSubShell::cssSubShell() {
  Constr();
}
cssSubShell::cssSubShell(const char* nm) {
  name = nm; Constr();
  prog_space.name = nm;
}
cssSubShell::cssSubShell(const cssSubShell& cp) {
  Constr(); Copy(cp);
}
cssSubShell::cssSubShell(const cssSubShell& cp, const char* nm) {
  Constr(); Copy(cp); name = nm;
  prog_space.name = nm;
}
