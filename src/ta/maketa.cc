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

// maketa: Make TypeAccess Data Structures

#include "maketa.h"
#include "mta_constr.h"
#include <signal.h>
#include <malloc.h>

TypeDef TA_void			("void", 	1, 0, 0, 0, 1);
TypeDef TA_int			("int", 	1, 0, 0, 0, 1);
TypeDef TA_short		("short", 	1, 0, 0, 0, 1);
TypeDef TA_long			("long", 	1, 0, 0, 0, 1);
TypeDef TA_char			("char", 	1, 0, 0, 0, 1);
TypeDef TA_unsigned		("unsigned", 	1, 0, 0, 0, 1);
TypeDef TA_signed		("signed", 	1, 0, 0, 0, 1);
TypeDef TA_float		("float", 	1, 0, 0, 0, 1);
TypeDef TA_double		("double", 	1, 0, 0, 0, 1);
TypeDef TA_bool			("bool", 	1, 0, 0, 0, 1);
TypeDef TA_const		("const", 	1, 0, 0, 0, 1);
TypeDef TA_enum			("enum", 	1, 0, 0, 1, 1); 	// formal
TypeDef TA_struct		("struct", 	1, 0, 0, 1, 1);	// formal
TypeDef TA_union		("union", 	1, 0, 0, 1, 1);	// formal
TypeDef TA_class		("class", 	1, 0, 0, 1, 1);	// formal
TypeDef TA_template		("template", 	1, 0, 0, 1, 1);// formal
TypeDef TA_templ_inst		("templ_inst", 	1, 0, 0, 1, 1);	// formal
TypeDef TA_ta_array		("ta_array", 	1, 0, 0, 1, 1);	// formal
TypeDef TA_taBase("taBase", " Base type for all type-aware classes", 
	"", "", "", 8, (void**)0, 0, 0, 0, 1);
TypeDef TA_taRegFun		("taRegFun", 	1, 0, 0, 0, 1);
TypeDef TA_TypeDef		("TypeDef", 	1, 0, 0, 0, 1);
TypeDef TA_MemberDef		("MemberDef", 	1, 0, 0, 0, 1);
TypeDef TA_MethodDef		("MethodDef", 	1, 0, 0, 0, 1);
TypeDef TA_taString		("taString", "",
				 "", "", "", sizeof(String), (void**)0, 0, 0, 0, 1);
TypeDef TA_taSubString		("taSubString", "",
				 "", "", "", sizeof(SubString), (void**)0, 0, 0, 0, 1);
TypeDef TA_void_ptr		("void_ptr", 	1, 1, 0, 1, 1);

extern int yydebug;
extern "C" int getpid();
MTA* mta;		// holds mta


char MTA::LastLn[8192];

MTA::MTA() {
  spc = NULL;

  cur_enum = NULL;
  cur_class = NULL;
  last_class = NULL;
  cur_mstate = prvt;
  cur_memb = NULL;
  cur_memb_type = NULL;
  cur_meth = NULL;
  last_memb = NULL;
  last_meth = NULL;

  thisname = false;
  constcoln = false;
  burp_fundefn = false;
  gen_css = false;
  gen_iv = false;
  gen_instances = false;
  make_hx = false;
  class_only = true;
  old_cfront = false;
  verbose = 0;
  hash_size = 2000;

  st_line = 0;
  st_col = 0;
  strm_pos = 0;
  st_pos = 0;
  st_line_pos = 0;
  line = 0;
  col = 0; 
  anon_no = 0;

  state = Find_Item;
  yy_state = YYRet_Ok;

  spc_other.name = "Types in Non-Target Header Files";
  spc_extern.name = "External Types Included";
  spc_ignore.name = "Types Ignored";
  spc_pre_parse.name = "Pre-Parsed Types Excluded";
  spc_keywords.name = "Key Words for Searching";
  spc_builtin.name = "Builtin Types";

  InitKeyWords();
  
  InitTypeSpace(spc_other);	// when target not being searched (need base types)
  InitTypeSpace(spc_target);
  
  // only add a subset of things to spc_builtin
  TypeSpace& ts = spc_builtin;
  ts.Add(&TA_int);
  ts.Add(&TA_short);
  ts.Add(&TA_long);
  ts.Add(&TA_char);
  ts.Add(&TA_unsigned);
  ts.Add(&TA_signed);
  ts.Add(&TA_float);
  ts.Add(&TA_double);
#ifndef NO_BUILTIN_BOOL
  ts.Add(&TA_bool);
#endif
  ts.Add(&TA_const);
  ts.Add(&TA_enum);
}

MTA::~MTA() {
//   type_stack.Reset();
//   enum_stack.Reset();
//   memb_stack.Reset();
//   meth_stack.Reset();
// 
//   spc_target.Reset();
//   spc_other.Reset();
//   spc_extern.Reset();
//   spc_ignore.Reset();
//   spc_pre_parse.Reset();
//   spc_keywords.Reset();
//   spc_builtin.Reset();
  
//  if(verbose < 1)	return;
//  cerr << "mta object destroyed\n";
}

void MTA::InitKeyWords() {
  TypeDef* ky;
  ky = new TypeDef("typedef");  spc_keywords.Add(ky); ky->idx = TYPEDEF;
  ky = new TypeDef("class"); 	spc_keywords.Add(ky); ky->idx = CLASS;
  ky = new TypeDef("struct"); 	spc_keywords.Add(ky); ky->idx = STRUCT;
  ky = new TypeDef("union"); 	spc_keywords.Add(ky); ky->idx = UNION;
  ky = new TypeDef("template"); spc_keywords.Add(ky); ky->idx = TEMPLATE;
  ky = new TypeDef("enum"); 	spc_keywords.Add(ky); ky->idx = ENUM;
  ky = new TypeDef("TypeDef"); 	spc_keywords.Add(ky); ky->idx = TA_TYPEDEF;
  ky = new TypeDef("public"); 	spc_keywords.Add(ky); ky->idx = PUBLIC;
  ky = new TypeDef("private"); 	spc_keywords.Add(ky); ky->idx = PRIVATE;
  ky = new TypeDef("protected");spc_keywords.Add(ky); ky->idx = PROTECTED;
  ky = new TypeDef("inline"); 	spc_keywords.Add(ky); ky->idx = FUNTYPE;
  ky = new TypeDef("mutable"); 	spc_keywords.Add(ky); ky->idx = FUNTYPE;
  ky = new TypeDef("virtual"); 	spc_keywords.Add(ky); ky->idx = VIRTUAL;
  ky = new TypeDef("static"); 	spc_keywords.Add(ky); ky->idx = STATIC;
  ky = new TypeDef("operator");	spc_keywords.Add(ky); ky->idx = OPERATOR;
  ky = new TypeDef("friend"); 	spc_keywords.Add(ky); ky->idx = FRIEND;
  ky = new TypeDef("REG_FUN"); 	spc_keywords.Add(ky); ky->idx = REGFUN;
}

void MTA::InitTypeSpace(TypeSpace& ts) {
  ts.Add(&TA_void);
  ts.Add(&TA_int);
  ts.Add(&TA_short);
  ts.Add(&TA_long);
  ts.Add(&TA_char);
  ts.Add(&TA_unsigned);
  ts.Add(&TA_signed);
  ts.Add(&TA_float);
  ts.Add(&TA_double);
#ifndef NO_BUILTIN_BOOL
  ts.Add(&TA_bool);
#endif
  ts.Add(&TA_const);
  ts.Add(&TA_enum);
  ts.Add(&TA_struct);
  ts.Add(&TA_union);
  ts.Add(&TA_class);
  ts.Add(&TA_template);
  ts.Add(&TA_templ_inst);
  ts.Add(&TA_ta_array);
  ts.Add(&TA_taBase);
  TA_taBase.AddParFormal(&TA_class);
  ts.Add(&TA_taRegFun);
  ts.Add(&TA_TypeDef);
  ts.Add(&TA_MemberDef);
  ts.Add(&TA_MethodDef);
  ts.Add(&TA_taString);
  TA_taString.AddParFormal(&TA_class);
  ts.Add(&TA_taSubString);
  TA_taSubString.AddParFormal(&TA_class);
  ts.Add(&TA_void_ptr);
  TA_void_ptr.AddParents(&TA_void);
}

void MTA::BuildHashTables() {
  spc_target.BuildHashTable(hash_size);
  spc_other.BuildHashTable(hash_size);
  spc_extern.BuildHashTable(hash_size);
  spc_ignore.BuildHashTable(hash_size);
  spc_pre_parse.BuildHashTable(hash_size);
  spc_keywords.BuildHashTable(100);
}

void MTA::Burp() {
  line = st_line;
  col = st_col;
  fh.seekg(st_pos);
  strm_pos = st_pos;
}

void MTA::Class_ResetCurPtrs() {
  mta->cur_memb = NULL; mta->cur_memb_type = NULL; mta->cur_meth = NULL;
  mta->last_memb = NULL; mta->last_meth = NULL;
}

void MTA::Class_UpdateLastPtrs() {
  mta->last_memb = mta->cur_memb;
  mta->last_meth = mta->cur_meth;
  mta->cur_memb = NULL; mta->cur_memb_type = NULL; mta->cur_meth = NULL;
}
     
TypeSpace* MTA::GetTypeSpace(TypeDef* td) {
  TypeSpace* rval = mta->spc;
  TypeDef* partd;
  if(td->HasOption("IGNORE")) {
    rval = &(spc_ignore);
    if(((partd = td->GetParent()) != NULL) && (partd->InheritsFormal(TA_templ_inst)))
      spc_ignore.Transfer(partd); // put this on the ignore list too
  }
  else if((td->owner != NULL) && (td->owner->owner != NULL)) {
    rval = td->owner;
    if(rval->name == "templ_pars") // don't add new types to template parameters!
      rval = &(td->owner->owner->sub_types);
  }
  if(((partd = td->GetParent()) != NULL) && (partd->InheritsFormal(TA_templ_inst))) {
    partd->opts.DupeUnique(td->opts); // inherit the options..
    partd->inh_opts.DupeUnique(td->inh_opts);
  }
  return rval;
}

void MTA::TypeAdded(const char* typ, TypeSpace* sp, TypeDef* td) {
  if(verbose <= 2)	return;
  cerr << typ << " added: " << td->name << " to: "
       << sp->name << " idx: " << td->idx << "\n";
}
  

void MTA::SetDesc(const char* comnt, String& desc, String_PArray& inh_opts,
		  String_PArray& opts, String_PArray& lists) {
  String tmp = comnt;
  tmp.gsub("\"", "'");		// don't let any quotes get through
  String ud;
  while(tmp.contains('#')) {
    desc += tmp.before('#');
    tmp = tmp.after('#');
    if(tmp.contains(' '))
      ud = tmp.before(' ');
    else
      ud = tmp;
    tmp = tmp.after(' ');
    if(ud(0,5) == "LIST_") {
      ud = ud.after("LIST_");
      lists.AddUnique(ud);
    }
    else {
      if(ud(0,1) == '#') {
	ud = ud.after('#');
	inh_opts.AddUnique(ud);
      }
      opts.AddUnique(ud);
    }
  }
  desc += tmp;
}

TypeDef* MTA::FindName(const char* nm, int& lex_token) {
  TypeDef* itm;

  lex_token = TYPE;		// this is the default

  if((state == Parse_inclass) && (cur_class != NULL)) {
    if(cur_class->name == nm) {
      lex_token = THISNAME;
      return cur_class;
    }
    if((itm = cur_class->sub_types.FindName(nm)) != NULL)
      return itm;
    if((itm = cur_class->templ_pars.FindName(nm)) != NULL)
      return itm;
  }
  
  if(((itm = spc_keywords.FindName(nm)) != NULL) && (itm->idx != TA_TYPEDEF)) {
    lex_token = itm->idx;
    return itm;
  }

  TypeDef *rval = NULL;
  if((itm = spc->FindName(nm)) != NULL)
    rval = itm;
  else if((spc != &spc_other) && (itm = spc_other.FindName(nm)) != NULL)
    rval = itm;
  else if((itm = spc_ignore.FindName(nm)) != NULL)
    rval = itm;

  if(rval != NULL) {
    if(rval == &TA_const)
      lex_token = CONST;
  }
  return rval;
}
  

// sets the pre-parsed flag if item is on the pre_parse list...
void MTA::SetPreParseFlag(TypeSpace& aspc, TypeSpace& pplist) {
  int i;
  for(i=0; i<aspc.size; i++) {
    TypeDef* td = aspc.FastEl(i);
    TypeDef* ttd;
    if((ttd = pplist.FindName(td->name)) != NULL)
      td->pre_parsed = true;
  }
}

void mta_cleanup(int err) {
  signal(err, SIG_DFL);
  cerr << "maketa: exiting and cleaning up temp files from signal: ";
  taMisc::Decode_Signal(err);
  cerr << "\n";
  String tmp_file = String("/tmp/mta_tmp.") + String(getpid());
  String rm_tmp = String("/bin/rm ") + tmp_file + " >/dev/null 2>&1";
  system(rm_tmp);
  kill(getpid(), err);		// activate signal 
}

void mta_print_commandline_args(char* argv[]) {
    cerr << "Usage:\t" << argv[0]
      << "\n[-[-]help | -[-]?]     print this argument listing"
      << "\n[-v<level>]         verbosity level, 1-5, 1=results,2=more detail,3=trace,4=source,5=parse"
      << "\n[-hx | -nohx]       generate .hx, .ccx files instead of .h, .cc (for cmp-based updating)"
      << "\n[-css]              generate CSS stub functions"
      << "\n[-instances]        generate instance tokens of types"
      << "\n[-class_only | -struct_union] only scan for class types (else struct and unions)"
      << "\n[-old_cfront        support old cfront style class member pointer initializer"
      << "\n[-I<include>]...    path to include files (one path per -I)"
      << "\n[-D<define>]...     define a pre-processor macro"
      << "\n[-cpp=<cpp command>] explicit path for c-pre-processor"
      << "\n[-hash<size>]       size of hash tables (default 2000), use -v1 to see actual sizes"
      << "\n[-f <filename>]     read list of header files from given file"
      << "\nproject             stub project name (generates project_TA[.cc|_type.h|_inst.h])"
      << "\nfiles...            the header files to be processed\n";
}

int main(int argc, char* argv[]) {
  mta = new MTA;

  mta->spc = &(mta->spc_other);

  if(argc < 2) { mta_print_commandline_args(argv); return 1;  } // wrong number of arguments

  taMisc::Register_Cleanup((SIGNAL_PROC_FUN_TYPE) mta_cleanup);

  String cpp = "/usr/lib/cpp";
  String incs;
  mta->basename = "";		// initialize

  int i;
  String tmp;
  for(i=1; i<argc; i++) {
    tmp = argv[i];
    if( (tmp == "-help") || (tmp == "--help") || (tmp == "-?") || (tmp == "--?")) {
      mta_print_commandline_args(argv); return 1; 		// EXIT
    }
    if(tmp == "-css")
      mta->gen_css = true;
    else if(tmp == "-iv")
      mta->gen_iv = true;
    else if(tmp == "-instances")
      mta->gen_instances = true;
    else if(tmp == "-nohx")
      mta->make_hx = false;
    else if(tmp == "-hx")
      mta->make_hx = true;
    else if(tmp == "-class_only")
      mta->class_only = true;
    else if(tmp == "-struct_union")
      mta->class_only = false;
    else if(tmp == "-old_cfront")
      mta->old_cfront = true;
    else if(tmp(0,2) == "-v") {
      mta->verbose = 1;
      int vl;
      String vls;
      vls = tmp.after(1);
      vl = atoi((char*)vls);
      if(vl > 0)
	mta->verbose = vl;
    }
    else if(tmp(0,5) == "-hash") {
      int vl;
      String vls;
      vls = tmp.after("-hash");
      vl = atoi((char*)vls);
      if(vl > 0)
	mta->hash_size = vl;
    }
    else if(tmp(0,2) == "-I")
      incs += tmp + " ";
    else if(tmp(0,2) == "-D")
      incs += String("-D") + tmp.after(1) + " ";
    else if(tmp(0,5) == "-cpp=")
      cpp = tmp.after(4);
    else if(tmp(0,2) == "-f") {
      fstream fh(argv[i+1], ios::in);
      if(fh.bad() || fh.eof()) {
	cerr << argv[0] << " could not open -f file: " << argv[i+1] << "\n";
      }
      else {
	while(fh.good() && !fh.eof()) {
	  String fl;
	  fh >> fl;
	  mta->headv.Add(fl);
	}
      }
      fh.close(); fh.clear();
      i++;			// skip to next one
    }
    else if(tmp[0] == '-')
      cerr << argv[0] << " unknown flag: " << tmp << "\n";
    else if(tmp[0] == '+')
      cerr << argv[0] << " unknown flag: " << tmp << "\n";
    else if(mta->basename.empty())
      mta->basename = tmp;
    else {
      mta->headv.Add(tmp);	// add the header file
    }
  }

  mta->BuildHashTables();	// after getting any user-spec'd hash size
  // parse the file names a bit

  for(i=0; i<mta->headv.size; i++) {
    String nstr = mta->headv.FastEl(i);
    if(nstr.contains('/'))
      nstr = nstr.after('/',-1); // just the file name
    mta->head_fn_only.Add(nstr);
  }

  if(mta->verbose > 0) {
    cerr << "header files to be parsed:\n";
    mta->headv.List(cout);
    cerr << "\nheader files to be parsed (file-name-only):\n";
    mta->head_fn_only.List(cout);
  }

  if(mta->verbose > 4)
    yydebug = 1;			// debug it.
  else
    yydebug = 0;

  if(cpp.contains("cccp"))
    cpp += " -lang-c++";
  String comnd_base = cpp + " " + incs;

  mta->spc_target.name = mta->basename;
  mta->ta_type_h = mta->basename + "_TA_type.h";
  mta->ta_inst_h = mta->basename + "_TA_inst.h";
  mta->ta_ccname = mta->basename + "_TA.cc";

  String tmp_file = String("/tmp/mta_tmp.") + String(getpid());
  String rm_tmp = String("/bin/rm ") + tmp_file;

  String comnd;
  for(i=0; i<mta->headv.size; i++) {
    mta->fname = mta->headv.FastEl(i);
    comnd = comnd_base + " -C -D__MAKETA__ -o " + tmp_file + " " + mta->fname;
    if(mta->verbose > 0)
      cerr << comnd << "\n";
    cout.flush();
    system((char*)comnd);
    mta->fh.open(tmp_file, ios::in);
    mta->state = MTA::Find_Item;
    mta->yy_state = MTA::YYRet_Ok;
    mta->line = 1;
    mta->col = 0;
    mta->strm_pos=0;
    cout << "Processing: " << mta->fname << "\n";
    cout.flush();
    while(mta->yy_state != MTA::YYRet_Exit) yyparse();
    mta->fh.close(); mta->fh.clear();
    mta->included.DupeUnique(mta->tmp_include); // copy over
    system(rm_tmp);
  }

  TypeSpace_Generate_LinkRefs(&(mta->spc_target), &(mta->spc_extern));
  if(mta->verbose > 0)
    mta->spc_extern.List();
  mta->spc_target.BorrowUniqNameOld(mta->spc_extern); // get those types

  cout << "List of pre-parsed files processed:\n";
  mta->pre_parse_inits.List(cout);
  cout << "\n";

  if(mta->verbose > 1) {
    cout << "\nPreParsed Types\n";   mta->spc_pre_parse.List();
  }
  mta->SetPreParseFlag(mta->spc_target, mta->spc_pre_parse);

  if(mta->make_hx) {
    mta->ta_type_h += "x";
    mta->ta_inst_h += "x";
    mta->ta_ccname += "x";
  }

  // give it 5 passes through to try to get everything in order..
  int swp_cnt = 0;
  if(mta->verbose > 0)
    cerr << "Sorting: Pass " << swp_cnt << "\n";
  while ((swp_cnt < 10) && TypeSpace_Sort_Order(&(mta->spc_target))) {
    swp_cnt++;
    if(mta->verbose > 0)
      cerr << "Sorting: Pass " << swp_cnt << "\n";
  }

  if(mta->verbose > 3) {
    mta->spc_target.List();
  }

  fstream out_type_h, out_inst_h, outc;
  
  out_type_h.open((char*)mta->ta_type_h, ios::out);
  out_inst_h.open((char*)mta->ta_inst_h, ios::out);
  outc.open((char*)mta->ta_ccname, ios::out);
  
  TypeSpace_Declare_Types(&(mta->spc_target), out_type_h, mta->headv);
  out_type_h.close();  out_type_h.clear();
  TypeSpace_Declare_Instances(&(mta->spc_target), out_inst_h, mta->headv);
  out_inst_h.close();  out_inst_h.clear();
  TypeSpace_Generate(&(mta->spc_target), outc, mta->headv, mta->pre_parse_inits);
  outc.close();  outc.clear();

  /* update times...why do we have to do this?? */
  comnd = String("touch ") + mta->ta_type_h;
  system(comnd);
  comnd = String("touch ") + mta->ta_inst_h;
  system(comnd);
  comnd = String("touch ") + mta->ta_ccname;
  system(comnd);

  if((mta->verbose > 0) && (mta->spc_target.hash_table != NULL)) {
    cerr << "\n TypeSpace size and hash_table bucket_max values:\n"
	 << "spc_target:\t" << mta->spc_target.size << "\t" << mta->spc_target.hash_table->bucket_max << "\n"
	 << "spc_other:\t" << mta->spc_other.size << "\t" << mta->spc_other.hash_table->bucket_max << "\n"
	 << "spc_extern:\t" << mta->spc_extern.size << "\t" << mta->spc_extern.hash_table->bucket_max << "\n"
	 << "spc_ignore:\t" << mta->spc_ignore.size << "\t" << mta->spc_ignore.hash_table->bucket_max << "\n"
	 << "spc_pre_parse:\t" << mta->spc_pre_parse.size << "\t" << mta->spc_pre_parse.hash_table->bucket_max << "\n"
	 << "spc_keywords:\t" << mta->spc_keywords.size << "\t" << mta->spc_keywords.hash_table->bucket_max << "\n";
  }
  if(mta->verbose > 1) {
    cout << "\nPreParsed Types\n";   mta->spc_pre_parse.List();
    cout << "\nTarget Types\n";   mta->spc_target.List();
    cout << "\nExtern Types\n";   mta->spc_extern.List();
    cout << "\nIgnored Types\n";   mta->spc_ignore.List();
    cout << "\nOther Types\n";   mta->spc_other.List();
  }
  delete mta;
  taMisc::types.RemoveAll();
  return 0;
}
