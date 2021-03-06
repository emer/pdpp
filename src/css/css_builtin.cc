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

// builtin functions, constants, etc.

#include <sstream>
#include <css/machine.h>
#include <css/basic_types.h>
#include <css/c_ptr_types.h>
#include <css/css_builtin.h>
#include <css/ta_css.h>
#include <css/css_iv.h>
#include <ta/ta_base.h>
#include <ta/taiv_data.h>
#include <ta/ta_group.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN	// [I.Noda] added to avoid compiler error 
#endif

#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

// when using the .c version
// extern "C" {
// #include <special_math.h>
// }

// using the .cc version
#include <special_math.h>
#include <css_misc_funs.h>

#ifndef CSS_NUMBER
#include "css_parse.h"
#endif

cssElCFun*	cssBI::asgn=NULL;
cssElCFun*	cssBI::asgn_add=NULL;
cssElCFun*	cssBI::asgn_sub=NULL;
cssElCFun*	cssBI::asgn_mult=NULL;
cssElCFun*	cssBI::asgn_div=NULL;
cssElCFun*	cssBI::asgn_mod=NULL;
cssElCFun*	cssBI::asgn_lshift=NULL;
cssElCFun*	cssBI::asgn_rshift=NULL;
cssElCFun*	cssBI::asgn_and=NULL;
cssElCFun*	cssBI::asgn_xor=NULL;
cssElCFun*	cssBI::asgn_or=NULL;
cssElCFun*	cssBI::init_asgn=NULL;
cssElCFun*	cssBI::asgn_post_pp=NULL;
cssElCFun*	cssBI::asgn_pre_pp=NULL;
cssElCFun*	cssBI::asgn_post_mm=NULL;
cssElCFun*	cssBI::asgn_pre_mm=NULL; 
cssElCFun*	cssBI::new_opr=NULL;
cssElCFun*	cssBI::del_opr=NULL;
cssElCFun*	cssBI::constr=NULL;
cssElCFun*	cssBI::add=NULL;
cssElCFun*	cssBI::sub=NULL;
cssElCFun*	cssBI::mul=NULL;
cssElCFun*	cssBI::div=NULL;
cssElCFun*	cssBI::modulo=NULL;
cssElCFun*	cssBI::lshift=NULL;
cssElCFun*	cssBI::rshift=NULL;
cssElCFun*	cssBI::bit_and=NULL;
cssElCFun*	cssBI::bit_xor=NULL;
cssElCFun*	cssBI::bit_or=NULL;
cssElCFun*	cssBI::neg=NULL;
cssElCFun*	cssBI::addr_of=NULL;
cssElCFun*	cssBI::de_ptr=NULL;
cssElCFun*	cssBI::de_array=NULL;
cssElCFun*	cssBI::points_at=NULL;
cssElCFun*	cssBI::member_fun=NULL;
cssElCFun*	cssBI::scoper=NULL;
cssElCFun*	cssBI::pop=NULL;
cssElCFun*	cssBI::cast=NULL;
cssElCFun*	cssBI::cond=NULL;
cssElCFun*	cssBI::ifcd=NULL;
cssElCFun*	cssBI::switch_jump=NULL;
		
cssElCFun*	cssBI::push_root=NULL;	// pushes a root value on stack
cssElCFun*	cssBI::push_next=NULL; // pushes next program item on stack
cssElCFun*	cssBI::push_cur_this=NULL; // push current this pointer on stack
cssElCFun*	cssBI::arg_swap=NULL; // swaps args
cssElCFun*	cssBI::array_alloc=NULL;
cssElCFun*	cssBI::sstream_rewind=NULL;
		
cssInt*		cssBI::true_int=NULL;
cssInt*		cssBI::false_int=NULL;
cssElCFun*	cssBI::gt=NULL;
cssElCFun*	cssBI::lt=NULL;
cssElCFun*	cssBI::eq=NULL;
cssElCFun*	cssBI::ge=NULL;
cssElCFun*	cssBI::le=NULL;	
cssElCFun*	cssBI::ne=NULL;
cssElCFun*	cssBI::land=NULL;
cssElCFun*	cssBI::lor=NULL;
cssElCFun*	cssBI::lnot=NULL;
cssElCFun*	cssBI::power=NULL;

cssTA_Base*	cssBI::root;		// root script element


//////////////////////////////////
// 	Internal Functions	//
//////////////////////////////////

// assign statements
static cssEl* cssElCFun_asgn_stub(int, cssEl* arg[]) {
  *(arg[1]) = *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_init_asgn_stub(int, cssEl* arg[]) {
  (arg[2])->InitAssign(*(arg[1]));
  return arg[2];
}
static cssEl* cssElCFun_asgn_add_stub(int, cssEl* arg[]) {
  *(arg[1]) += *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_sub_stub(int, cssEl* arg[]) {
  *(arg[1]) -= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_mult_stub(int, cssEl* arg[]) {
  *(arg[1]) *= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_div_stub(int, cssEl* arg[]) {
  *(arg[1]) /= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_mod_stub(int, cssEl* arg[]) {
  *(arg[1]) %= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_lshift_stub(int, cssEl* arg[]) {
  *(arg[1]) <<= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_rshift_stub(int, cssEl* arg[]) {
  *(arg[1]) >>= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_and_stub(int, cssEl* arg[]) {
  *(arg[1]) &= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_xor_stub(int, cssEl* arg[]) {
  *(arg[1]) ^= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_or_stub(int, cssEl* arg[]) {
  *(arg[1]) |= *(arg[2]);
  return arg[1];
}
static cssEl* cssElCFun_asgn_post_pp_stub(int, cssEl* arg[]) {
  cssEl* tmp;
  if(arg[1]->IsRef())
    tmp = arg[1]->GetActualObj()->AnonClone(); // save previous value
  else
    tmp = arg[1]->AnonClone(); // save previous value
  *(arg[1]) += *cssBI::true_int;	// an int value of 1
  return tmp;
}
static cssEl* cssElCFun_asgn_pre_pp_stub(int, cssEl* arg[]) {
  *(arg[1]) += *cssBI::true_int;	// an int value of 1
  return arg[1];
}
static cssEl* cssElCFun_asgn_post_mm_stub(int, cssEl* arg[]) {
  cssEl* tmp;
  if(arg[1]->IsRef())
    tmp = arg[1]->GetActualObj()->AnonClone(); // save previous value
  else
    tmp = arg[1]->AnonClone(); // save previous value
  *(arg[1]) -= *cssBI::true_int;	// an int value of 1
  return tmp;
}
static cssEl* cssElCFun_asgn_pre_mm_stub(int, cssEl* arg[]) {
  *(arg[1]) -= *cssBI::true_int;	// an int value of 1
  return arg[1];
}
// arg 1 is conditional, 2 is true, 3 is false
static cssEl* cssElCFun_cond_stub(int, cssEl* arg[]) {
  if((int)*arg[1])
    return arg[2];
  else
    return arg[3];
}

// if na=2 arg 1 is size, arg 2 is type, else arg 1 is type
static cssEl* cssElCFun_new_opr_stub(int na, cssEl* arg[]) {
  if (na == 2) {
    // allocate an array
    cssArray* tmp = new cssArray((int)(*arg[1]), arg[2]);
    return tmp;
  }
  else return arg[1]->NewOpr();
}
static cssEl* cssElCFun_del_opr_stub(int, cssEl* arg[]) {
  arg[1]->DelOpr();
  return &cssMisc::Void;
}

static cssEl* cssElCFun_constr_stub(int, cssEl* arg[]) {
  if(arg[1]->GetType() == cssEl::T_Class)
    ((cssClassInst*)arg[1])->ConstructToken();
  return &cssMisc::Void;
}

// cast operator
static cssEl* cssElCFun_cast_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  (arg[2])->MakeTempToken(cp);		// make the thing
  cssEl* rval = cp->Stack()->Pop(); 	// result is ptr to token
  rval->CastFm(*(arg[1]));
  return rval;
}
  
// push_next

static cssEl* cssElCFun_push_next_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssInst* nxt = cp->Next();
  if(nxt != NULL)
    return nxt->inst.El();
  else
    return &cssMisc::Void;
}

// pointer operators
static cssEl* cssElCFun_addr_of_stub(int, cssEl* arg[]) {
  return new cssPtr(arg[1]->GetAddr());
}
static cssEl* cssElCFun_de_ptr_stub(int, cssEl* arg[]) {
  return *(*arg[1]);
}
static cssEl* cssElCFun_de_array_stub(int, cssEl* arg[]) {
  return (*arg[1])[*arg[2]];
}
static cssEl* cssElCFun_points_at_stub(int, cssEl* arg[]) {
  if(arg[2]->GetType() == cssEl::T_String)
    return (arg[1])->GetMember((const char*)*arg[2]);
  else
    return (arg[1])->GetMember((int)*arg[2]);
}
static cssEl* cssElCFun_scoper_stub(int, cssEl* arg[]) {
  return (arg[1])->GetScoped(*arg[2]);
}
static cssEl* cssElCFun_member_fun_stub(int, cssEl* arg[]) {
#ifdef HAVE_VOLATILE
  volatile cssProg* cp = arg[0]->prog; // volatile because of setjmp
  volatile cssEl* tmp;
#else
  cssProg* cp = arg[0]->prog;
  cssEl* tmp;
#endif
  if(arg[2]->GetType() == cssEl::T_String)
    tmp = (arg[1])->GetMemberFun((const char*)*arg[2]);
  else 
    tmp = (arg[1])->GetMemberFun((int)*arg[2]);
  if((tmp->GetType() != cssEl::T_MbrCFun) && (tmp->GetType() != cssEl::T_MbrScriptFun)) {
    if(tmp != &cssMisc::Void)
      cssEl::Done((cssEl*)tmp);
    cssMisc::Error(cp, "Member is not a function", tmp->name);
    return &cssMisc::Void;
  }
  int old_state = cp->top->state;
  int old_step = cp->top->step_mode;
  cp->top->step_mode = 0;
  cp->Stack()->Push(&cssMisc::Void); // push the arg stop
  cssEl* ths = arg[1]->GetActualObj(); // get past references and pointers
  if(tmp->GetType() == cssEl::T_MbrScriptFun) {	// push the 'this' pointer 
    if(ths->GetType() == cssEl::T_Class)
      cp->Stack()->Push(ths);
    else {
      if(cp->CurThis() == NULL) {
	cssMisc::Error(cp, "No 'this' pointer for member function call");
	return &cssMisc::Void;
      }
      cp->Stack()->Push(cp->CurThis());
    }
  }
  // push the 'this' pointer for builtin functions
  else if(tmp->GetType() == cssEl::T_MbrCFun) {
    if(ths->GetType() == cssEl::T_Class)
      cp->Stack()->Push(ths);
  }
  if(cp->top->debug >= 2) {
    cerr << "\nCalling member function: " << tmp->name << endl;
  }

  cp->top->Cont();		// run rest of args 
  cp->top->state = old_state;
  cp->top->step_mode = old_step;
  
  if(!((cp->top->run==cssEl::Stopping) || (cp->top->run==cssEl::Running)))
    return &cssMisc::Void;
  
  setjmp(cssMisc::begin);		// need to set the break to here for mbr_fun do
  if(cp->top->run != cssEl::ExecError) {
    tmp->Do((cssProg*)cp);
  }
//  arg[0]->prog = cp;		// reset this, see above ^^^^
  cssEl::Done((cssEl*)tmp);	// get rid of the memberfun
  return &cssMisc::Void;	// retv pushed by the previous do
}

// arg[1]..[na-1] = sizes,   arg[na] = array
static cssEl* cssElCFun_array_alloc_stub(int na, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  if(arg[na]->GetType() != cssEl::T_Array &&
     arg[na]->GetType() != cssEl::T_ArrayType) {
    cssMisc::Error(arg[0]->prog, "Array alloc did not get an array");
    rval->val = 0;
    return rval;
  }
  cssArray* tmp = (cssArray*) arg[na];
  rval->val = tmp->AllocAll(na-2, arg+1);
  return rval;
}

static cssEl* cssElCFun_add_stub(int, cssEl* arg[]) {
  return *(arg[1]) + *(arg[2]);
}
static cssEl* cssElCFun_sub_stub(int, cssEl* arg[]) {
  return *(arg[1]) - *(arg[2]);
}
static cssEl* cssElCFun_mul_stub(int, cssEl* arg[]) {
  return *(arg[1]) * *(arg[2]);
}
static cssEl* cssElCFun_div_stub(int, cssEl* arg[]) {
  return *(arg[1]) / *(arg[2]);
}
static cssEl* cssElCFun_modulo_stub(int, cssEl* arg[]) {
  return *(arg[1]) % *(arg[2]);
}
static cssEl* cssElCFun_lshift_stub(int, cssEl* arg[]) {
  if((arg[1]->GetType() == cssEl::T_TA) && (arg[2]->GetType() == cssEl::T_Bool)) {
    cssTA* ta = (cssTA*)arg[1];
    if(ta->type_def->InheritsFrom(TA_ostream)) {
      ostream* strm = (ostream*)ta->GetVoidPtrOfType(&TA_ostream);
      if(strm != NULL) {
	if(arg[2]->name == "flush") {
	  *strm << flush;	  return arg[1];
	}
	if(arg[2]->name == "endl") {
	  *strm << endl;	  return arg[1];
	}
	if(arg[2]->name == "ends") {
	  *strm << ends;	  return arg[1];
	}
	if(arg[2]->name == "dec") {
	  *strm << dec;	  return arg[1];
	}
	if(arg[2]->name == "hex") {
	  *strm << hex;	  return arg[1];
	}
	if(arg[2]->name == "oct") {
	  *strm << oct;	  return arg[1];
	}
      }
    }
  }
  return *(arg[1]) << *(arg[2]);
}
static cssEl* cssElCFun_rshift_stub(int, cssEl* arg[]) {
  if((arg[1]->GetType() == cssEl::T_TA) && (arg[2]->GetType() == cssEl::T_Bool)) {
    cssTA* ta = (cssTA*)arg[1];
    if(ta->type_def->InheritsFrom(TA_istream)) {
      istream* strm = (istream*)ta->GetVoidPtrOfType(&TA_istream);
      if(strm != NULL) {
	if(arg[2]->name == "ws") {
	  *strm >> ws;	  return arg[1];
	}
	if(arg[2]->name == "dec") {
	  *strm >> dec;	  return arg[1];
	}
	if(arg[2]->name == "hex") {
	  *strm >> hex;	  return arg[1];
	}
	if(arg[2]->name == "oct") {
	  *strm >> oct;	  return arg[1];
	}
      }
    }
  }
  return *(arg[1]) >> *(arg[2]);
}
static cssEl* cssElCFun_bit_and_stub(int, cssEl* arg[]) {
  return *(arg[1]) & *(arg[2]);
}
static cssEl* cssElCFun_bit_xor_stub(int, cssEl* arg[]) {
  return *(arg[1]) ^ *(arg[2]);
}
static cssEl* cssElCFun_bit_or_stub(int, cssEl* arg[]) {
  return *(arg[1]) | *(arg[2]);
}
static cssEl* cssElCFun_neg_stub(int, cssEl* arg[]) {
  return -(*(arg[1]));
}

static cssEl* cssElCFun_sstream_rewind_stub(int, cssEl* arg[]) {
  std::stringstream* strm = (std::stringstream*)(void*)*(arg[1]);
  strm->clear();
  strm->seekg(0, ios::beg);
  return arg[1];
}

static cssEl* cssElCFun_pop_stub(int, cssEl**) {
  return &cssMisc::Void;		// the args have the stack contents, will be doneArgs
}
static cssEl* cssElCFun_nop_stub(int, cssEl**) {
  return &cssMisc::Void;
}
static cssEl* cssElCFun_arg_swap_stub(int na, cssEl* arg[]) {
  if(na != 2)
    return &cssMisc::Void;
  cssProg* cp = arg[0]->prog;
  cp->Stack()->Push(arg[2]);	// push this on the stack
  return arg[1];		// and this goes after it
}

// inc after savepc & what happens there
// 0	then
// +1	else
// +2	end
// +3	condition

static cssEl* cssElCFun_ifcd_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  css_progdx savepc = cp->PC();
  cssEl* cond;

  cp->SetPC(savepc + 3);
  cond = cp->top->Cont();
  if((int) *cond) {
    cp->SetPC(savepc);
    cp->EndRunPop();		// get rid of cond..
    cp->top->Cont();		// then part
    if((cp->top->run == cssEl::Stopping) || (cp->top->run == cssEl::Running))
      cp->insts[savepc + 2]->Do(); // reset pc to the end
  }
  else {
    cp->SetPC(savepc + 1);
    cp->EndRunPop();		// get rid of cond..
    cp->top->Cont();
  }
  cp->EndRunPop();
  if((cp->top->run == cssEl::Stopping) || (cp->top->run == cssEl::Running))
    ((cssElFun*)arg[0])->dostat = cssEl::Running;
  else
    ((cssElFun*)arg[0])->dostat = cp->top->run;
  return &cssMisc::Void;
}

static cssEl* cssElCFun_switch_jump_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;

  cssElPtr ptr = cp->literals.FindName(cssSwitchJump_Name);
  cssArray* val_ary = (cssArray*)ptr.El();

  ptr = cp->FindAutoName(cssSwitchVar_Name);
  cssEl* sval = ptr.El();
  
  if((val_ary == &cssMisc::Void) || (sval == &cssMisc::Void)) {
    cssMisc::Error(cp, "case statement not in a valid switch block");
    ((cssElFun*)arg[0])->dostat = cssEl::Breaking; // bail
    return &cssMisc::Void;
  }

  ptr = val_ary->items->FindName((const char*)*sval);
  cssEl* jmp_to = ptr.El();
  if(jmp_to == &cssMisc::Void) { // not found
    ptr = val_ary->items->FindName(cssSwitchDefault_Name);
    jmp_to = ptr.El();
    if(jmp_to == &cssMisc::Void) {
      ((cssElFun*)arg[0])->dostat = cssEl::Breaking; // bail, no other case found
      return &cssMisc::Void;
    }
  }

  cp->SetPC((int)*jmp_to);	// go to that line
  return &cssMisc::Void;
}


static cssEl* cssElCFun_switch_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->Cont();		// run the switch
  cp->EndRunPop();		// get rid of junk

  if(cp->top->run == cssEl::Breaking)
    cp->top->run = cssEl::Running;	// clear the break
  return &cssMisc::Void;
}

// while loop: inc after savepc & what happens there
// 0	body
// +1	end
// +2	condition

static cssEl* cssElCFun_while_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  css_progdx savepc = cp->PC();
  cssEl* cond;

  cp->SetPC(savepc + 2);
  cond = cp->top->Cont();

  while((int) *cond) {
    cp->SetPC(savepc);
    cp->EndRunPop();		// get rid of cond..
    cp->top->Cont();		// body
    if(cp->top->run == cssEl::Breaking) {
      cp->top->run = cssEl::Stopping;
      break;
    }
    else if(cp->top->run == cssEl::Continuing) {
      cp->top->run = cssEl::Stopping;
    }
    else if(!((cp->top->run==cssEl::Stopping) || (cp->top->run==cssEl::Running)))
      break;
    cp->SetPC(savepc + 2);
    cond = cp->top->Cont();
  }
  cp->EndRunPop();		// get rid of cond..
  if((cp->top->run == cssEl::Stopping) || (cp->top->run == cssEl::Running)) {
    cp->insts[savepc + 1]->Do(); // resets pc
    ((cssElFun*)arg[0])->dostat = cssEl::Running;
  }
  else 
    ((cssElFun*)arg[0])->dostat = cp->top->run;
  return &cssMisc::Void;
}

// do loop: inc after savepc & what happens there
// +0	condition
// +1	end
// +2	body

static cssEl* cssElCFun_do_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  css_progdx savepc = cp->PC();
  bool cond;

  do {
    cp->SetPC(savepc+2);
    cp->top->Cont();		// body
    if(cp->top->run == cssEl::Breaking) {
      cp->top->run = cssEl::Stopping;
      break;
    }
    else if(cp->top->run == cssEl::Continuing) {
      cp->top->run = cssEl::Stopping;
    }
    else if(!((cp->top->run==cssEl::Stopping) || (cp->top->run==cssEl::Running)))
      break;
    cp->SetPC(savepc + 0);
    cssEl* cond_el = cp->top->Cont();
    cond = (bool)((int)*cond_el);
    cp->EndRunPop();		// get rid of cond..
  } while(cond);

  if((cp->top->run == cssEl::Stopping) || (cp->top->run == cssEl::Running)) {
    cp->insts[savepc + 1]->Do(); // resets pc
    ((cssElFun*)arg[0])->dostat = cssEl::Running;
  }
  else 
    ((cssElFun*)arg[0])->dostat = cp->top->run;
  return &cssMisc::Void;
}


// inc after savepc & what happens there
// 0	condition
// +1	increments
// +2	body
// +3	end
// +4	inits

static cssEl* cssElCFun_for_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  css_progdx savepc = cp->PC();
  cssEl* cond;

  cp->SetPC(savepc + 4);
  cp->top->Cont();		// run initializers
  cp->EndRunPop();

  cp->SetPC(savepc);
  cond = cp->top->Cont();	// condition

  while((int) *cond) {
    cp->SetPC(savepc + 2);
    cp->EndRunPop();		// get rid of cond..
    cp->top->Cont();		// body
    if(cp->top->run == cssEl::Breaking) {
      cp->top->run = cssEl::Stopping;
      break;
    }
    else if(cp->top->run == cssEl::Continuing) {
      cp->top->run = cssEl::Stopping;
    }
    else if(!((cp->top->run==cssEl::Stopping) || (cp->top->run==cssEl::Running)))
      break;

    cp->SetPC(savepc + 1);
    cp->top->Cont();		// increments
    cp->EndRunPop();

    cp->SetPC(savepc);
    cond = cp->top->Cont();
  }
  cp->EndRunPop();		// get rid of cond..
  if((cp->top->run == cssEl::Stopping) || (cp->top->run == cssEl::Running)) {
    cp->insts[savepc + 3]->Do(); // resets pc to end
    ((cssElFun*)arg[0])->dostat = cssEl::Running;
  }
  else 
    ((cssElFun*)arg[0])->dostat = cp->top->run;
  return &cssMisc::Void;
}

static cssEl* cssElCFun_return_stub(int na, cssEl* arg[]) {
  if(na > 0) {
    cssProg* cp = arg[0]->prog;
    cssScriptFun* sf = cp->top->GetCurrentFun();
    if(sf == NULL)
      cssMisc::Error(cp, "Could not find current function scope for return value!");
    else {
      cssEl* rval = sf->argv[0].El();
      if(rval == &cssMisc::Void)
	cssMisc::Error(cp, "Attempt to return a value in a void function");
      rval->InitAssign(*(arg[1])); // use initassign!
    }
  }
  ((cssElFun*)arg[0])->dostat = cssEl::Returning;
  return &cssMisc::Void;
}
static cssEl* cssElCFun_break_stub(int, cssEl* arg[]) {
  ((cssElFun*)arg[0])->dostat = cssEl::Breaking;
  return &cssMisc::Void;
}
static cssEl* cssElCFun_continue_stub(int, cssEl* arg[]) {
  ((cssElFun*)arg[0])->dostat = cssEl::Continuing;
  return &cssMisc::Void;
}

static cssEl* cssElCFun_gt_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) > *(arg[2])));
}
static cssEl* cssElCFun_lt_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) < *(arg[2])));
}
static cssEl* cssElCFun_eq_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) == *(arg[2])));
}
static cssEl* cssElCFun_ge_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) >= *(arg[2])));
}
static cssEl* cssElCFun_le_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) <= *(arg[2])));
}
static cssEl* cssElCFun_ne_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) != *(arg[2])));
}
static cssEl* cssElCFun_land_stub(int, cssEl* arg[]) {
  return new cssInt((int)(*(arg[1]) && *(arg[2])));
}
static cssEl* cssElCFun_lor_stub(int, cssEl* arg[]) {
  
  return new cssInt((int)(*(arg[1]) || *(arg[2])));
}
static cssEl* cssElCFun_lnot_stub(int, cssEl* arg[]) {
  return new cssInt((int)!(*(arg[1])));
}

static cssEl* cssElCFun_push_root_stub(int, cssEl**) {
  return cssBI::root;
}

static cssEl* cssElCFun_push_cur_this_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(cp->CurThis() == NULL) {
    // find this as hard-coded variable: 
    cssElPtr ths = cp->top->hard_vars.FindName("this");
    if(ths.NotNull()) {
      return ths.El();
    }
    else {
      cssMisc::Error(cp, "No current 'this' pointer");
      return &cssMisc::Void;
    }
  }
  return cp->CurThis();
}

// pre-processor

static cssEl* cssElCFun_include_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetInclude((const char*)*(arg[1]));
  return &cssMisc::Void;
}

//////////////////////////////////
//	Install_Internals	//
//////////////////////////////////

static void Install_Internals() {
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, init_asgn, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_add, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_sub, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_mult, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_div, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_mod, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_lshift, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_rshift, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_and, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_xor, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_or, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_post_pp, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_pre_pp, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_post_mm, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, asgn_pre_mm, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, cond, 3, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, cast, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, add, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, sub, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, mul, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, div, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, modulo, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, lshift, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, rshift, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, bit_and, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, bit_xor, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, bit_or, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, neg, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, addr_of, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, de_ptr, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, de_array, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, points_at, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, scoper, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, member_fun, 2, CSS_FUN);
  cssElCFun_inst_ptr(cssMisc::Internal, pop, cssEl::VarArg, CSS_FUN, " ");
  cssElCFun_inst_ptr(cssMisc::Internal, array_alloc, cssEl::VarArg, CSS_FUN, " ");
  cssElInCFun_inst_ptr(cssMisc::Internal, push_root, cssEl::NoArg, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, push_next, cssEl::NoArg, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, push_cur_this, cssEl::NoArg, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, arg_swap, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, sstream_rewind, 1, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, switch_jump, cssEl::NoArg, CSS_FUN);

  cssElInCFun_inst_ptr(cssMisc::Internal, gt, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, lt, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, eq, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, ge, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, le, 2, CSS_FUN);
  cssElInCFun_inst_ptr(cssMisc::Internal, ne, 2, CSS_FUN);
  cssElInCFun_inst_ptr_nm(cssMisc::Internal, land, 2, "and", CSS_FUN);
  cssElInCFun_inst_ptr_nm(cssMisc::Internal, lor, 2, "or", CSS_FUN);
  cssElInCFun_inst_ptr_nm(cssMisc::Internal, lnot, 1, "not", CSS_FUN);

  // internal functions
  cssElInCFun_inst	(cssMisc::Functions, for, cssEl::NoArg, CSS_FOR);
  cssElInCFun_inst	(cssMisc::Functions, while, cssEl::NoArg, CSS_WHILE);
  cssElInCFun_inst	(cssMisc::Functions, do, cssEl::NoArg, CSS_DO);
  cssElInCFun_inst_ptr_nm (cssMisc::Functions, ifcd, cssEl::NoArg, "if", CSS_IF);
  cssElInCFun_inst_nm	(cssMisc::Functions, ifcd, cssEl::NoArg, "else", CSS_ELSE);
  cssElInCFun_inst	(cssMisc::Functions, switch, cssEl::NoArg, CSS_SWITCH);
  cssElCFun_inst	(cssMisc::Functions, return, cssEl::VarArg, CSS_RETURN, " ");
  cssElInCFun_inst	(cssMisc::Functions, break, cssEl::NoArg, CSS_BREAK);
  cssElInCFun_inst	(cssMisc::Functions, continue, cssEl::NoArg, CSS_CONTINUE);
  cssElCFun_inst_nm     (cssMisc::Functions, new_opr, cssEl::VarArg, "new", CSS_NEW, " ");
  cssElCFun_inst_nm     (cssMisc::Functions, del_opr, 1, "delete", CSS_DELETE, " ");
  cssElInCFun_inst_ptr	(cssMisc::Internal, constr, 1, CSS_FUN);

  // stuff just for parsing
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "class", CSS_CLASS, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "struct", CSS_CLASS, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "union", CSS_CLASS, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "enum", CSS_ENUM, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "extern", CSS_EXTERN, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "static", CSS_STATIC, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "private", CSS_PRIVATE, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "public", CSS_PUBLIC, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "protected", CSS_PROTECTED, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "virtual", CSS_VIRTUAL, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "const", CSS_CONST, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "inline", CSS_INLINE, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "case", CSS_CASE, " ");
  cssElCFun_inst_nm	(cssMisc::Parse, nop, 	cssEl::NoArg, "default", CSS_DEFAULT, " ");

  // pre-processor
  cssElInCFun_inst(cssMisc::PreProcessor, include,  	1, CSS_PP_INCLUDE);
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "define", CSS_PP_DEFINE, " ");
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "undef", CSS_PP_UNDEF, " ");
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "ifdef", CSS_PP_IFDEF, " ");
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "ifndef", CSS_PP_IFNDEF, " ");
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "else", CSS_PP_ELSE, " ");
  cssElCFun_inst_nm(cssMisc::PreProcessor, nop,  	cssEl::NoArg, "endif", CSS_PP_ENDIF, " ");

  cssMisc::Defines.Push(new cssDef(0, "__CSS__"));
}


//////////////////////////
// 	Commands	//
//////////////////////////

static cssEl* cssElCFun_alias_stub(int, cssEl* arg[]) {
  cssElCFun* tmp = (cssElCFun *)((cssRef*)arg[2])->ptr.El();
  cssMisc::Commands.Push(new cssElCFun(tmp->argc, tmp->funp, (const char*)*(arg[1]),
				 tmp->parse));  
  return &cssMisc::Void;
}
static cssEl* cssElCFun_chsh_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    if(arg[1]->GetType() == cssEl::T_SubShell) {
      cp->top->SetShell(&(((cssSubShell*)arg[1])->prog_space));
      return &cssMisc::Void;
    }
    if(arg[1]->GetType() != cssEl::T_TA) {
      cssMisc::Error(cp, "chsh argument is not a pointer to a cssProgSpace");
      return &cssMisc::Void;
    }
    cssTA* ta = (cssTA*)arg[1];
    if(ta->type_def->InheritsFrom("cssProgSpace") && (ta->ptr != NULL)) {
      cssProgSpace* ps = (cssProgSpace*)ta->GetVoidPtr();
      cp->top->SetShell(ps);
    }
    else {
      cssMisc::Error(cp, "chsh argument is not a valid pointer to a cssProgSpace");
    }
  }
  else
    cp->top->SetShell(cp->top);	// shell itself
  return &cssMisc::Void;
}
static cssEl* cssElCFun_clearall_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetClearAll();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_cont_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0)
    cp->top->SetCont((int)*(arg[1]));
  else 
    cp->top->SetCont(-1);
  return &cssMisc::Void;
}
static cssEl* cssElCFun_goto_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetCont((int)*(arg[1]));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_commands_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssMisc::Commands.NameList(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_constants_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssMisc::Constants.List(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_debug_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetDebug((int)*(arg[1])); 
  return &cssMisc::Void;
}
static cssEl* cssElCFun_define_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetDefn();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_defines_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssMisc::Defines.List(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_edit_stub(int na, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  if(na > 1)
    rval->val = arg[1]->Edit((int)*(arg[2]));
  else if(na == 1)
    rval->val = arg[1]->Edit();
  else
    cssMisc::Error(arg[0]->prog, "Edit requires an argument of the object to be edited");
  return rval;
}
static cssEl* cssElCFun_enums_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssMisc::Enums.NameList(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_frame_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    int back = (int)*arg[1];
    if(back >= cp->top->size) {
      cssMisc::Error(cp, "Not that many frames:",String(back),"in program, has only:",
		      String((int)(cp->top->size-1)));
      return &cssMisc::Void;
    }
    if(back == cp->top->size-1)
      cp->top->statics.List(*(cp->top->fout));
    else
      cp->top->Prog(cp->top->size-1-back)->ListSpace();
  }
  else {
    cp->ListSpace();
    cp->top->statics.List(*(cp->top->fout));
  }
  return &cssMisc::Void;
}
static cssEl* cssElCFun_functions_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cssMisc::Functions.NameList(*(cp->top->fout));
  cp->top->hard_funs.NameList(*(cp->top->fout));
  cssMisc::HardFuns.NameList(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_globals_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->hard_vars.List(*(cp->top->fout));
  cssMisc::HardVars.List(*(cp->top->fout));
  cssMisc::Externs.List(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_help_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    if(arg[1]->GetType() == cssEl::T_ElCFun) {
      cssElCFun* fun = (cssElCFun*)arg[1];
      *(cp->top->fout) << "\nHelp for function: " << fun->name << "\n" << fun->name << " ";
      String str = fun->help_str;
      int wdth = 0;
      while(!str.empty()) {
	String wrd;
	if(str.contains(' ')) {
	  wrd = str.before(' ');
	  str = str.after(' ');
	}
	else {
	  wrd = str;
	  str = "";
	}
	if(wdth + (int)wrd.length() > taMisc::display_width) {
	  *(cp->top->fout) << endl;
	  wdth = 0;
	}
	*(cp->top->fout) << wrd << " ";
	wdth += wrd.length() + 1;
      }
      *(cp->top->fout) << endl;
      cp->top->fout->flush();
    }
    else {
      arg[1]->TypeInfo(*(cp->top->fout));
      *(cp->top->fout) << "\n";
      cp->top->fout->flush();
    }
  }
  else
    cp->top->Help();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_inherit_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  arg[1]->InheritInfo(*(cp->top->fout));
  *(cp->top->fout) << "\n";
  cp->top->fout->flush();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_list_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    if((arg[1]->GetType() == cssEl::T_ScriptFun) ||
       (arg[1]->GetType() == cssEl::T_MbrScriptFun))
    {
      cssScriptFun* fe = (cssScriptFun*)arg[1];
      if(na > 1)
	fe->fun->List(*(cp->top->fout), fe->fun->CurSrcLn(0), *arg[2]);
      else
	fe->fun->List(*(cp->top->fout), fe->fun->CurSrcLn(0), 100);	// give it the whole thing..
    }
    else {
      if(na > 1)
	cp->top->List(*arg[1], *arg[2]);
      else
	cp->top->List(*arg[1]);
    }
  }
  else
    cp->top->List();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_load_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetCompile((const char*)*(arg[1]));
  return &cssMisc::Void;
}

static cssEl* cssElCFun_mallinfo_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  taMisc::MallocInfo(*(cp->top->fout));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_print_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  arg[1]->Print(*(cp->top->fout));
  if(cp->top->debug > 1)
    *(cp->top->fout) << "\trefn: " << arg[1]->refn;
  *(cp->top->fout) << "\n";
  cp->top->fout->flush();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_printr_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  arg[1]->PrintR(*(cp->top->fout));
  *(cp->top->fout) << "\n";
  cp->top->fout->flush();
  return &cssMisc::Void;
}

static cssEl* cssElCFun_remove_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(!cp->top->types.Replace(arg[1], &cssMisc::Void)) {
    if(!cssMisc::TypesSpace.Replace(arg[1], &cssMisc::Void)) {
      if(!cp->top->DelVar(arg[1]))
	cssMisc::Error(cp, "Could not delete type/variable:", (char*)arg[1]->GetName());
    }
  }
  return &cssMisc::Void;
}
static cssEl* cssElCFun_reload_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetReCompile(); 
  return &cssMisc::Void;
}
static cssEl* cssElCFun_reset_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetReset();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_restart_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetRestart();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_run_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetRun();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_setbp_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetBreak((int)*(arg[1])); 
  return &cssMisc::Void;
}
static cssEl* cssElCFun_setout_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  ostream* strm = (ostream*)*(arg[1]);
  cp->top->fout = strm; 
  return &cssMisc::Void;
}
static cssEl* cssElCFun_settings_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  int i;
  *(cp->top->fout) << "Include Paths:\n";
  taMisc::include_paths.List(*(cp->top->fout));
  *(cp->top->fout) << "\n";
  for(i=1; i<cssMisc::Settings.size; i++) {
    cssMisc::Settings.FastEl(i)->Print(*(cp->top->fout));
    *(cp->top->fout) << "\n";
  }
  cp->top->fout->flush();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_showbp_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->ShowBreaks();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_source_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->SetSource((const char*)*(arg[1]));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_stack_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->Stack()->List();
  cp->saved_stack.List();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_status_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->Status();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_step_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->step_mode = (int)*(arg[1]);
  return &cssMisc::Void;
}

static cssEl* cssElCFun_system_stub(int, cssEl* arg[]) {
  system((const char*)*arg[1]);
  return &cssMisc::Void;
}

static cssEl* cssElCFun_tokens_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  arg[1]->TokenInfo(*(cp->top->fout));
  *(cp->top->fout) << "\n";
  cp->top->fout->flush();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_trace_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    int tr_lev = (int)*arg[1];
    if(tr_lev > 2)
      cp->top->ListSpace();
    else
      cp->top->Trace(tr_lev);
  }
  else
    cp->top->Trace();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_type_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0) {
    arg[1]->TypeInfo(*(cp->top->fout));
    *(cp->top->fout) << endl;
  }
  else {
    *(cp->top->fout) << "Types local to current top-level program space (" << cp->top->name << "):" << endl;
    cp->top->types.NameList(*(cp->top->fout));
    *(cp->top->fout) << "\n==========================" << endl << 
      "Global types: " << endl;
    cssMisc::TypesSpace.NameList(*(cp->top->fout));
  }
  return &cssMisc::Void;
}
static cssEl* cssElCFun_unsetbp_stub(int, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  cp->top->unSetBreak((int)*(arg[1]));
  return &cssMisc::Void;
}
static cssEl* cssElCFun_undo_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  if(na > 0)
    cp->top->SetUndo((Int)*(arg[1]));
  else
    cp->top->SetUndo(-1);
  return &cssMisc::Void;
}



//////////////////////////////////
//	Install_Commands	//
//////////////////////////////////

static void Install_Commands() {
  cssElCFun_inst(cssMisc::Commands, alias, 		2, CSS_ALIAS,
"<cmd> <new_nm> Gives a new name to an existing command.  This is useful for defining\
 shortcuts (e.g., alias list ls)");
  cssElCFun_inst(cssMisc::Commands, chsh, 		cssEl::VarArg, CSS_COMMAND,
"<script_path> Switches the CSS interface to access the CSS script object pointed to by\
 the given path.  This is for hard-coded objects that have CSS script\
 objects in them (of type cssProgSpace)");
  cssElCFun_inst(cssMisc::Commands, clearall, 		0, CSS_COMMAND,
"Clears out everything from the current program space.  This is like\
 restarting the CSS shell, compared to reset which does not remove\
 any variables defined at the top-level.");
  cssElCFun_inst(cssMisc::Commands, commands,		0, CSS_COMMAND,
"Shows a list of the currently available commands (including any aliases\
 that have been defined, which will appear at the end of the list).");
  cssElCFun_inst(cssMisc::Commands, constants,		0, CSS_COMMAND,
"Shows a list of the pre-defined constants that have been defined in CSS.\
 These are just like globally-defined Int and Real values,\
 and thus they can be assigned to different values (though this is\
 obviously not recommended).");
  cssElCFun_inst(cssMisc::Commands, cont, 		0, CSS_CONT,
"Continues the execution of a program that was stopped either by a\
 breakpoint or by single-stepping.  To continue at a particular line in\
 the code, use the goto command.");
  cssElCFun_inst(cssMisc::Commands, debug,   		1, CSS_COMMAND,
"<level> Sets the debug level.  Level 1 provides a trace of the source lines\
 executed.  Level 2 provides a more detailed, machine-level trace, and\
 causes list to show the program at the machine level instead of\
 at the usual source level. Levels greater than 2 provide increasing\
 amounts of detail about the inner workings of CSS, which should not be\
 relevant to most users.");
  cssElCFun_inst(cssMisc::Commands, define, 		0, CSS_COMMAND,
"Toggles the mode where statements that are typed in become part of the\
 current program to be executed later (define mode), as opposed the\
 default (run mode) where statements are executed immediately after entering\
 them.");
  cssElCFun_inst(cssMisc::Commands, defines, 		0, CSS_COMMAND,
"Shows a list of all of the current #define pre-processor macros.");
  cssElCFun_inst(cssMisc::Commands, edit,   		cssEl::VarArg, CSS_COMMAND,
"If the GUI (graphical user interface) is active (i.e., by using\
 -gui to start up CSS), edit will bring up a graphical edit\
 dialog for the given object, which must be either a script-defined or\
 hard-coded class object.  The optional second argument, if\
 true, will cause the system to wait for the user to close the\
 edit dialog before continuing execution of the script.");
  cssElCFun_inst(cssMisc::Commands, enums,		0, CSS_COMMAND,
"Shows a list of all the current enum types.  Note that most\
 enum types are defined within a class scope, and can be\
 found there by using the type command on the class type.");
  cssElCFun_inst_nm(cssMisc::Commands, nop, 		0, "exit", CSS_EXIT,
"Exits from the program (CSS), or from another program space if\
 chsh (or its GUI equivalent) was called.");
  cssElCFun_inst(cssMisc::Commands, frame,		cssEl::VarArg, CSS_COMMAND,
"[<back>] Shows the variables and their values associated with the current block\
 or frame of processing.  The optional argument gives the number of\
 frames back from the current one to display.  This is most relevant for\
 debugging at a breakpoint, since otherwise there will only be a single,\
 top-level frame to display.");
  cssElCFun_inst(cssMisc::Commands, functions,		0, CSS_COMMAND,
"Shows a list of all of the currently defined functions.");
  cssElCFun_inst(cssMisc::Commands, globals,		0, CSS_COMMAND,
"Shows a list of all of the currently defined global variables, including\
 those in the script and hard-coded ones.");
  cssElCFun_inst(cssMisc::Commands, goto, 		1, CSS_COMMAND,
"<src_ln> Continues execution at the given source line.  ");
  cssElCFun_inst(cssMisc::Commands, help, 		cssEl::VarArg, CSS_HELP,
"[<expr>] Shows a short help message, including lists of commands and functions\
 available.    When passed argument (command, function, class, etc),\
 provides help information for it.");
  cssElCFun_inst(cssMisc::Commands, inherit, 		1, CSS_TYPECMD,
"<object_type> Shows the inheritance path for the given object type.");
  cssElCFun_inst(cssMisc::Commands, list, 		cssEl::VarArg, CSS_LIST,
"[<start_ln> [<n_lns>]] [<function>] Lists the program source (or\
 machine code, if debug is 2 or greater), optionally starting at the\
 given source line number, and continuing for either 20 lines (the\
 initial default) or the number given by the second argument (which\
 then becomes the new default).  Alternatively, a function name can be\
 given, which will start the listing at the beginning of that function\
 (even if the function is extern-al and does not appear in a\
 line-number based list).  list with no arguments will resume where the\
 last one left off.");
  cssElCFun_inst(cssMisc::Commands, load, 		1, CSS_COMMAND,
"<prog_file> Loads and compiles a new program from the given file."); // 
  cssElCFun_inst(cssMisc::Commands, mallinfo, 		0, CSS_COMMAND,
"Generates a listing of the current malloc memory allocation\
 statistics, including changes from the last time the command was called.");
  cssElCFun_inst(cssMisc::Commands, print, 		1, CSS_COMMAND, // "
"<expr> Prints the results of the given expression (which can be any valid CSS\
 expression), giving some type information and following with a new line\
 (\\n).  This is useful for debugging, but not for printing values\
 as part of an executing program.");
  cssElCFun_inst(cssMisc::Commands, printr,		1, CSS_COMMAND, // "
"<object> Prints an object and any of its sub-objects in a indented style\
 output.  This can be very long for objects near the top of the object\
 hierarchy (i.e., the root object), so be careful!");
  cssElCFun_inst(cssMisc::Commands, reload, 		0, CSS_COMMAND,
"Reloads the current program from the last file that was load-ed.\
 This is useful because you do not have to specify the program file when\
 making a series of changes to a program.");
  cssElCFun_inst(cssMisc::Commands, remove,		1, CSS_REMOVE,
"<var_name> Removes given variable from whatever space it was defined in.  This can\
 be useful if a variable was defined accidentally or given the wrong name\
 during interactive use.");
  cssElCFun_inst(cssMisc::Commands, reset, 		0, CSS_COMMAND,
"Reset is like clearall, except that it does not remove any\
 top-level variables that might have been defined.  Neither of these\
 commands will remove anything declared extern.");
  cssElCFun_inst(cssMisc::Commands, restart,   		0, CSS_COMMAND,
"Resets the script to start at the beginning.  This is useful if you want\
 to stop execution of the program after a break point.");
  cssElCFun_inst(cssMisc::Commands, run,  		0, CSS_COMMAND,
"Runs the script from the start (as opposed to cont which\
 continues execution from the current location).");
  cssElCFun_inst(cssMisc::Commands, setbp, 		1, CSS_COMMAND,
"<src_ln> Sets a breakpoint at the given source-code line.  Execution of the\
 program will break when it gets to that line, and you will be able to\
 examine variables, etc.");
  cssElCFun_inst(cssMisc::Commands, setout, 		1, CSS_COMMAND,
"<ostream> Sets the default output of CSS commands to the given stream.  This can\
 be used to redirect listings or program tracing output to a file.");
  cssElCFun_inst(cssMisc::Commands, settings,		0, CSS_COMMAND,
"Shows the current values of various system-level settings or parameters.\
 These settings are all static members of the class ta_Misc, and\
 can be set by using the scoped member name, for example:\
 ta_Misc::display_width = 90;");
  cssElCFun_inst_nm(cssMisc::Commands, system, 		1, "shell", CSS_COMMAND,
"<shell_cmd> Executes the given Unix shell command (i.e., shell \"ls -lt\").");
  cssElCFun_inst(cssMisc::Commands, showbp,  		0, CSS_COMMAND,
"Shows a list of all currently defined breakpoints, and the source code\
 line they point to.");
  cssElCFun_inst(cssMisc::Commands, source, 		1, CSS_COMMAND,
"<cmd_file> Loads a file which contains a series of commands or statements, which\
 are executed exactly as if they were entered from the keyboard.  Note\
 that this is different than load-ing a program, which merely\
 compiles the program but does not execute it immediately thereafter.\
 source uses run mode, while load uses define mode.");
  cssElCFun_inst(cssMisc::Commands, stack, 		0, CSS_COMMAND,
"Displays the current contents of the stack.  This can be useful for\
 debugging."); 
  cssElCFun_inst(cssMisc::Commands, status, 		0, CSS_STATUS,
"Displays a brief listing of various status parameters, such as current\
 source line, depth, etc.");
  cssElCFun_inst(cssMisc::Commands, step, 		1, CSS_COMMAND,
"<step_n> Sets the single-step mode for program execution.  The parameter is the\
 number of lines to step through before pausing.  A value of 0 turns off\
 single stepping.");
  cssElCFun_inst(cssMisc::Commands, tokens, 		1, CSS_TYPECMD,
"<obj_type> Lists the instances of the given object type which are known to have\
 been created.  Many object types do not register tokens, which will be\
 indicated in the results of this command if applicable.  It is possible\
 to refer to the objects by their position in this list with the\
 Token function, which can be a useful shortcut to using the\
 objects path.");
  cssElCFun_inst(cssMisc::Commands, trace, 		cssEl::VarArg, CSS_COMMAND,
"[<level>] Displays a trace of the functions called up to the current one (i.e., as\
 called from within a breakpoint). A trace level of 0 (the default) just\
 gives function names, line numbers, and the source code for the\
 function call, while level 1 adds stack information, level 2 adds\
 stack and auto variable state information, and level 3 gives a complete\
 dump of all available information.");
  cssElCFun_inst(cssMisc::Commands, type, 		cssEl::VarArg, CSS_TYPECMD,
"<type_name> Gives type information about the given type.  This includes full\
 information about classes (both hard-coded and script-defined),\
 including members, functions, scoped types (enums), etc.");
  cssElCFun_inst(cssMisc::Commands, undo, 		cssEl::VarArg, CSS_COMMAND,
"This undoes the previous statement, when in define mode.");
  cssElCFun_inst(cssMisc::Commands, unsetbp, 		1, CSS_COMMAND,
"<src_ln> Removes a breakpoint associated with the given source-code line number.");
}


//////////////////////////
//    Math Functions	//
//////////////////////////

static cssEl* cssElCFun_max_stub(int, cssEl* arg[]) {
  return (*(arg[1]) > *(arg[2])) ? arg[1] : arg[2];
}
static cssEl* cssElCFun_min_stub(int, cssEl* arg[]) {
  return (*(arg[1]) < *(arg[2])) ? arg[1] : arg[2];
}

static cssRealFun_stub1(acos);
static cssRealFun_stub1(asin);
static cssRealFun_stub1(atan);
static cssRealFun_stub2(atan2);
static cssRealFun_stub1(ceil);
static cssRealFun_stub1(cos);
static cssRealFun_stub1(cosh);
static cssEl* cssRealFun_drand48_stub(int, cssEl**) {
  return new cssReal((Real)MTRnd::genrand_res53());
}
static cssEl* cssElCFun_lrand48_stub(int, cssEl**) {
  return new cssInt((int)MTRnd::genrand_int32());
}
static cssRealFun_stub1(exp);
static cssRealFun_stub1(fabs);
static cssRealFun_stub1(floor);
static cssRealFun_stub2(fmod);
static cssRealFun_stub1(log);
static cssRealFun_stub1(log10);
static cssEl* cssRealFun_pow_stub(int, cssEl* arg[]) {
  return new cssReal((Real)pow((Real)*(arg[1]), (Real)*(arg[2])));
}
static cssEl* cssElCFun_random_stub(int, cssEl**) {
  return new cssInt((int)MTRnd::genrand_int31());
}
static cssEl* cssElCFun_srand48_stub(int, cssEl* arg[]) {
  MTRnd::seed((int)*(arg[1]));
  return &cssMisc::Void;
}

static cssRealFun_stub1(sin);
static cssRealFun_stub1(sinh);
static cssRealFun_stub1(sqrt);
static cssRealFun_stub1(tan);
static cssRealFun_stub1(tanh);

static cssRealFun_stub1(acosh);
static cssRealFun_stub1(asinh);
static cssRealFun_stub1(atanh);


// stuff from special math:

//static cssRealFun_stub1(fact_ln);
static cssEl* cssRealFun_fact_ln_stub(int, cssEl* arg[]) {
  return new cssReal((Real)fact_ln((int)*(arg[1])));
}
//static cssRealFun_stub2(bico_ln);
static cssEl* cssRealFun_bico_ln_stub(int, cssEl* arg[]) {
  return new cssReal((Real)bico_ln((int)*(arg[1]), (int)*(arg[2])));
}

//static cssRealFun_stub4(hyperg);
static cssEl* cssRealFun_hyperg_stub(int, cssEl* arg[]) {
  return new cssReal((Real)hyperg((int)*(arg[1]), (int)*(arg[2]), (int)*(arg[3]), (int)*(arg[4])));
}

static cssRealFun_stub1(gamma_ln);
static cssRealFun_stub2(gamma_p);
static cssRealFun_stub2(gamma_q);
static cssRealFun_stub2(beta);
static cssRealFun_stub3(beta_i);

//static cssRealFun_stub3(binom_den);
static cssEl* cssRealFun_binom_den_stub(int, cssEl* arg[]) {
  return new cssReal((Real)binom_den((int)*(arg[1]), (int)*(arg[2]), (double)*(arg[3])));
}
//static cssRealFun_stub3(binom_cum);
static cssEl* cssRealFun_binom_cum_stub(int, cssEl* arg[]) {
  return new cssReal((Real)binom_cum((int)*(arg[1]), (int)*(arg[2]), (double)*(arg[3])));
}
//static cssRealFun_stub2(binom_dev); 
static cssEl* cssRealFun_binom_dev_stub(int, cssEl* arg[]) {
  return new cssReal((Real)binom_dev((int)*(arg[1]), (double)*(arg[2])));
}

//static cssRealFun_stub2(poisson_den);
static cssEl* cssRealFun_poisson_den_stub(int, cssEl* arg[]) {
  return new cssReal((Real)poisson_den((int)*(arg[1]), (double)*(arg[2])));
}
//static cssRealFun_stub2(poisson_cum);
static cssEl* cssRealFun_poisson_cum_stub(int, cssEl* arg[]) {
  return new cssReal((Real)poisson_cum((int)*(arg[1]), (double)*(arg[2])));
}
static cssRealFun_stub1(poisson_dev);

//static cssRealFun_stub3(gamma_den);
static cssEl* cssRealFun_gamma_den_stub(int, cssEl* arg[]) {
  return new cssReal((Real)gamma_den((int)*(arg[1]), (double)*(arg[2]), (double)*(arg[3])));
}
//static cssRealFun_stub3(gamma_cum);
static cssEl* cssRealFun_gamma_cum_stub(int, cssEl* arg[]) {
  return new cssReal((Real)gamma_cum((int)*(arg[1]), (double)*(arg[2]), (double)*(arg[3])));
}
//static cssRealFun_stub1(gamma_dev);
static cssEl* cssRealFun_gamma_dev_stub(int, cssEl* arg[]) {
  return new cssReal((Real)gamma_dev((int)*(arg[1])));
}

static cssRealFun_stub1(gauss_den);
static cssRealFun_stub1(gauss_cum);
static cssRealFun_stub0(gauss_dev);
static cssRealFun_stub1(gauss_inv);
static cssRealFun_stub1(erf);
static cssRealFun_stub1(erf_c);

static cssRealFun_stub2(chisq_p);
static cssRealFun_stub2(chisq_q);
static cssRealFun_stub2(students_cum);
static cssRealFun_stub2(students_den);
static cssRealFun_stub3(Ftest_q);


//////////////////////////////////
//	Install_Math		//
//////////////////////////////////

static void Install_Math() {
  // global real functions
  cssMisc::Functions.Push(cssBI::power = new cssElCFun(2, cssRealFun_pow_stub, "pow", CSS_FUN,
"(Real x, y) Returns x to the y power.  This can also be expressed in CSS as x ^ y."));

  // parsed internal real functions
  cssRealFun_inst(cssMisc::Functions, acos, 1,
"(Real X) The arc-cosine (inverse cosine) -- takes an X coordinate and returns\
 the angle (in radians) such that cos(angle)=X.");
  cssRealFun_inst(cssMisc::Functions, asin, 1,
"(Real Y) The arc-sine (inverse sine) -- takes a Y coordinate and returns the\
 angle (in radians) such that sin(angle)=Y.");
  cssRealFun_inst(cssMisc::Functions, atan, 1, 
"(Real Y/X) The arc-tangent (inverse tangent) -- takes a Y/X slope and returns angle\
 (in radians) such that tan(angle)=Y/X.");
  cssRealFun_inst(cssMisc::Functions, atan2, 2,
"(Real Y, Real X) The arc-tangent (inverse tangent) -- takes a Y/X slope and returns angle\
 (in radians) such that tan(angle)=Y/X."); // "
  cssRealFun_inst(cssMisc::Functions, ceil, 1,
"(Real x) Rounds up the value to the next-highest integral value.");
  cssRealFun_inst(cssMisc::Functions, cos, 1,
"(Real x) The cosine of angle x (given in radians).  Use cos(x / DEG) if x\
 is in degrees.");
  cssRealFun_inst(cssMisc::Functions, cosh, 1,
"(Real x) The hyperbolic cosine of angle x.");
  cssRealFun_inst(cssMisc::Functions, drand48, 0,
"Returns a uniformly-distributed random number between 0 and 1 (actually uses MT -- Mersenne Twister).");
  cssElCFun_inst(cssMisc::Functions, lrand48, 0, CSS_FUN,
"Returns a uniformly-distributed random number on the range of the integers (actually uses MT -- Mersenne Twister).");
  cssRealFun_inst(cssMisc::Functions, exp, 1,
"(Real x) The natural exponential (e to the power x).");
  cssRealFun_inst(cssMisc::Functions, fabs, 1,
"(Real x) The absolute value of x.");
  cssRealFun_inst(cssMisc::Functions, floor, 1,
"(Real x) Rounds the value down to the next lowest integral value.");
  cssRealFun_inst(cssMisc::Functions, fmod, 2,
"(Real x, Real y) Returns the value of x modulo y (i.e., x % y) for floating-point values.");
  cssRealFun_inst(cssMisc::Functions, log, 1,
"(Real x) The natural logarithm of x.");
  cssRealFun_inst(cssMisc::Functions, log10, 1,
"(Real x) The logarithm base 10 of x.");
  cssElCFun_inst(cssMisc::Functions, max, 2, CSS_FUN,
"(x,y) Works like the commonly-used #define macro that gives the maximum\
 of the two given arguments.  The return type is that of the\
 maximum-valued argument.");
  cssElCFun_inst(cssMisc::Functions, min, 2, CSS_FUN,
"(x,y) Just like MAX, except it returns the minimum of the two given\
 arguments.");
  cssElCFun_inst_nm(cssMisc::Functions, max, 2, "MAX", CSS_FUN,
"(x,y) Works like the commonly-used #define macro that gives the maximum\
 of the two given arguments.  The return type is that of the\
 maximum-valued argument.");
  cssElCFun_inst_nm(cssMisc::Functions, min, 2, "MIN", CSS_FUN,
"(x,y) Just like MAX, except it returns the minimum of the two given arguments.");
  cssElCFun_inst(cssMisc::Functions, random, 0, CSS_FUN,
"Returns a uniformly-distributed random number on the range of the\
 integers.  CSS actually uses the MT (Mersenne Twister) function to generate the\
 number given the limitations of the standard random generators.");
  cssElCFun_inst(cssMisc::Functions, srand48, 1, CSS_FUN,
"(Int x) Provides a new random seed for the random number generator based on the argument.");
  cssRealFun_inst(cssMisc::Functions, sin, 1,
"(Real x) The sine of angle x (given in radians).  Use sin(x / DEG) if x is in degrees.");
  cssRealFun_inst(cssMisc::Functions, sinh, 1,
"(Real x) The hyperbolic sine of x.");
  cssRealFun_inst(cssMisc::Functions, sqrt, 1,
"(Real x) The square-root of x.");
  cssRealFun_inst(cssMisc::Functions, tan, 1,
"(Real x) The tangent of angle x (given in radians).  Use tan(x / DEG) if x is in degrees.");
  cssRealFun_inst(cssMisc::Functions, tanh, 1,
"(Real x) The hyperbolic tangent of x.");

  cssRealFun_inst(cssMisc::Functions, fact_ln   , 1,
"(Real x) The natural logarithm of the factorial of x (x!).");
  cssRealFun_inst(cssMisc::Functions, bico_ln   , 2,
"(Int n, Int j) The natural logarithm of the binomial coefficient \"n choose j\".  The\
 number of ways of choosing j items out of a set containing n\
 elements: (n j) =  n! / (k! (n-k)!)");
  cssRealFun_inst(cssMisc::Functions, hyperg    , 4,
"(Int j, Int s, Int t, Int n) The hypergeometric probability function for getting j number of the\
 target items in an environment of size n, where there are t\
 targets and a sample (without replacement) of this environment of size\
 s is taken.");

  cssRealFun_inst(cssMisc::Functions, gamma_ln  , 1,
"(Real z) The natural logarithm of the gamma function, which is a generalization\
 of (n-1)! to real-valued arguments. Note that this is not the\
 gamma probability distribution. Gamma(z) = integral_0^x t^(z-1) e^(-t) dt");
  cssRealFun_inst(cssMisc::Functions, gamma_p   , 2,
"(Real a, Real x) The incomplete gamma function: P(a,x) = 1/Gamma(a) \
 integral_0^x t^(a-1) e^(-t) dt (a > 0)");
  cssRealFun_inst(cssMisc::Functions, gamma_q   , 2,
"(Real a, Real x) The incomplete gamma function as the complement of gamma_p:\
 P(a,x) = 1/Gamma(a) integral_x^inf t^(a-1) e^(-t) dt (a > 0)");
  cssRealFun_inst(cssMisc::Functions, beta      , 2,
"(Real z, Real w) The Beta function.");
  cssRealFun_inst(cssMisc::Functions, beta_i    , 3,
"(Real a, Real b, Real x) The incomplete Beta function.");

  cssRealFun_inst(cssMisc::Functions, binom_den , 3,
"(Int n, Int j, Real p)  The binomial probability density function for j successes in n trials,\
 each with probability p of success.  P(n,j,p) = (n j) p^j (1-p)^(n-j)");
  cssRealFun_inst(cssMisc::Functions, binom_cum , 3,
"(Int n, Int j, Int p)  The cumulative binomial probability of getting j or more in n\
 trials of probability p.");
  cssRealFun_inst(cssMisc::Functions, binom_dev , 2,
"(Int n, Real p) The binomial random deviate: produces an integer number of successes for\
 a binomial distribution with p probability over n trials.");

  cssRealFun_inst(cssMisc::Functions, poisson_den, 2,
"(Int j, Real l) The Poisson probability density function for j events given an expected\
 number of events of l (lambda). P(j,l) = (l^j/j!) e^-l");
  cssRealFun_inst(cssMisc::Functions, poisson_cum,2,
"(Int j, Real l) The cumulative Poisson distribution for getting 0 to j-1 events with an\
 exected number of events of l (lambda).");
  cssRealFun_inst(cssMisc::Functions, poisson_dev,1,
"(Real l) A random Poisson deviate with a mean of l (lambda).");

  cssRealFun_inst(cssMisc::Functions, gamma_den , 3,
"(Int j, Real l, Real t) The gamma probability density function for j events, l=lambda, and\
 t=time.  P(j,l,t) = (l^j t^(j-1) / j!) e^-lt (t > 0)");
  cssRealFun_inst(cssMisc::Functions, gamma_cum , 3,
"(Int i, Real l, Real t) The cumulative gamma distribution for event i with parameters l=lambda\
 and t=time, which is the same as gamma_p(j, l * t).");
  cssRealFun_inst(cssMisc::Functions, gamma_dev , 1,
"(Int j) A random gamma deviate: how long it takes to wait until j events occur\
 with a unit lambda (l=1).");

  cssRealFun_inst(cssMisc::Functions, gauss_den , 1,
"(Real x) The Gaussian or normal probability density function at x with sigma = 1\
 and mean = 0.");
  cssRealFun_inst(cssMisc::Functions, gauss_cum , 1,
"(Real x) The cumulative of the Gaussian or normal distribution up to given x\
 (sigma = 1, mean = 0).");
  cssRealFun_inst(cssMisc::Functions, gauss_inv , 1,
"(Real p) Inverse of the cumulative for p: returns z value for given p");
  cssRealFun_inst(cssMisc::Functions, gauss_dev , 0,
"The Gaussian or normal probability density function at x with sigma = 1\
 and mean = 0.");
  cssRealFun_inst(cssMisc::Functions, erf       , 1,
"(Real x) The error function, which provides an approximation to the integral of\
 the normal distribution.");
  cssRealFun_inst(cssMisc::Functions, erf_c     , 1,
"(Real x) The complement of the error function erf.");

  cssRealFun_inst(cssMisc::Functions, chisq_p   , 2,
"(Real X, v) Gives the chi-squared statistic P(X^2 | v).");
  cssRealFun_inst(cssMisc::Functions, chisq_q   , 2,
"(Real X, v) Gives the complement of the chi-squared statistic Q(X^2 | v).");
  cssRealFun_inst(cssMisc::Functions, students_cum, 2,
"(Real t, v) Gives the cumulative Student's distribution for v degrees of freedom t test.");
  cssRealFun_inst(cssMisc::Functions, students_den, 2,
"(Real t, v) Gives the Student's distribution density function for v degrees of freedom t test.");
  cssRealFun_inst(cssMisc::Functions, Ftest_q   , 3,
"(Real F, Real v1, Real v2) Gives the F probability distribution for P(F | (v1 < v2)).  Useful for\
 performing statistical significance tests.  The _q suffix means that\
 this is the complement distribution.");

// hp does not include these unless you have _INCLUDE_HPUX_SOURCE...
#ifndef HP800
  cssRealFun_inst(cssMisc::Functions, acosh, 1,
"(Real x) The hyperbolic arc-cosine.");
  cssRealFun_inst(cssMisc::Functions, asinh, 1,
"(Real x) The hyperbolic arc-sine.");
  cssRealFun_inst(cssMisc::Functions, atanh, 1,
"(Real x) The hyperbolic arc-tangent.");
#endif // HP800
  
  cssReal_inst_nm(cssMisc::Constants, 3.14159265358979323846,  "PI"   );
  cssReal_inst(cssMisc::Constants, 2.71828182845904523536,  E     );
  cssReal_inst(cssMisc::Constants, 0.57721566490153286060,  GAMMA ); // Euler
  cssReal_inst(cssMisc::Constants, 57.29577951308232087680, DEG   ); // deg/radian
  cssReal_inst(cssMisc::Constants, 1.61803398874989484820,  PHI   ); // golden ratio
  cssReal_inst(cssMisc::Constants, 96484.56,                FARADAY); // coulombs/mole
  cssReal_inst(cssMisc::Constants, 8.31441,                 R     );  // molar gas constant, joules/mole/deg-K

  cssReal_inst_nm(cssMisc::Constants, FLT_MAX,             "FLT_MAX");
  cssReal_inst_nm(cssMisc::Constants, FLT_MAX_10_EXP,      "FLT_MAX_10_EXP");
  cssReal_inst_nm(cssMisc::Constants, FLT_MAX_EXP,         "FLT_MAX_EXP");
  cssReal_inst_nm(cssMisc::Constants, FLT_MAX,             "MAXFLOAT");	// compatibiltiy

  cssReal_inst_nm(cssMisc::Constants, FLT_MIN,             "FLT_MIN");
  cssReal_inst_nm(cssMisc::Constants, FLT_MIN_10_EXP,      "FLT_MIN_10_EXP");
  cssReal_inst_nm(cssMisc::Constants, FLT_MIN_EXP,         "FLT_MIN_EXP");
  cssReal_inst_nm(cssMisc::Constants, FLT_EPSILON,         "FLT_EPSILON");

  cssReal_inst_nm(cssMisc::Constants, DBL_MAX,             "DBL_MAX");
  cssReal_inst_nm(cssMisc::Constants, DBL_MAX_10_EXP,      "DBL_MAX_10_EXP");
  cssReal_inst_nm(cssMisc::Constants, DBL_MAX_EXP,         "DBL_MAX_EXP");

  cssReal_inst_nm(cssMisc::Constants, DBL_MIN,             "DBL_MIN");
  cssReal_inst_nm(cssMisc::Constants, DBL_MIN_10_EXP,      "DBL_MIN_10_EXP");
  cssReal_inst_nm(cssMisc::Constants, DBL_MIN_EXP,         "DBL_MIN_EXP");
  cssReal_inst_nm(cssMisc::Constants, DBL_EPSILON,         "DBL_EPSILON");

  cssReal_inst_nm(cssMisc::Constants, INT_MAX,             "INT_MAX");
  cssReal_inst_nm(cssMisc::Constants, INT_MIN,             "INT_MIN");
  cssReal_inst_nm(cssMisc::Constants, LONG_MAX,            "LONG_MAX");
  cssReal_inst_nm(cssMisc::Constants, LONG_MIN,            "LONG_MIN");
}


//////////////////////////////////
// 	Misc Functions  	//
//////////////////////////////////

static cssEl* cssElCFun_CancelEditObj_stub(int, cssEl* arg[]) {
  cssEl* aobj = arg[1]->GetActualObj();
  if(aobj->GetType() != cssEl::T_Class)
    cssMisc::Error(arg[0]->prog, "CancelEditObj: arg not a class instance");
  else {
    cssClassInst* obj = (cssClassInst*)aobj;
    cssivSession::CancelObjEdits(obj);
  }
  return &cssMisc::Void;
}

static cssEl* cssElCFun_Extern_stub(int, cssEl* arg[]) {
  cssElPtr ptr = cssMisc::Externs.FindName((const char*)*arg[1]);
  return ptr.El();
}

static cssEl* cssElCFun_Token_stub(int, cssEl* arg[]) {
  return arg[1]->GetToken((int)*(arg[2]));
}

static cssEl* cssElCFun_Type_stub(int, cssEl* arg[]) {
  TypeDef* td = taMisc::types.FindName((const char*)*arg[1]);
  if(td == NULL) {
    cssMisc::Error(arg[0]->prog, "Could not find type:", (const char*)*arg[1]);
    return &cssMisc::Void;
  }
  return new cssTA(td, 1, &TA_TypeDef);
}

static cssEl* cssElCFun_Dir_stub(int na, cssEl* arg[]) {
  String dir_nm = ".";
  if(na > 0)
    dir_nm = (const char*)*(arg[1]);
  String_Array& rval = Dir((const char*)dir_nm);
  static cssTA* rv_ta = NULL;
  if(rv_ta == NULL) {
    rv_ta = new cssTA(NULL, 1, &TA_String_Array);
    cssEl::Ref(rv_ta);
  }
  rv_ta->ptr = (void*)&rval;
  return new cssRef(rv_ta);
}

// this reads a line from a file and returns a string array with
// one entry per column from the file
static cssEl* cssElCFun_ReadLine_stub(int, cssEl* arg[]) {
  istream* fh = (istream*)*(arg[1]);
  if(fh == NULL)
    return &cssMisc::Void;
  String_Array& rval = ReadLine(*fh);
  static cssTA* rv_ta = NULL;
  if(rv_ta == NULL) {
    rv_ta = new cssTA(NULL, 1, &TA_String_Array);
    cssEl::Ref(rv_ta);
  }
  rv_ta->ptr = (void*)&rval;
  return new cssRef(rv_ta);
}

// old-fashioned C file io

static cssEl* cssElCFun_fopen_stub(int, cssEl* arg[]) {
  fstream* strm = new fstream;	// this guy never gets free'd..
  String optype = arg[2]->GetStr();

  if(optype == "r")
    strm->open(*arg[1], ios::in);
  else if(optype == "w")
    strm->open(*arg[1], ios::out);
  else if(optype == "a")
    strm->open(*arg[1], ios::out | ios::app);
  else {
    delete strm;
    cssMisc::Error(arg[0]->prog, "fopen: open type not recognized: ", (char*)optype);
    return &cssMisc::Void;
  }
  return new cssTA((void*)strm, 1, &TA_fstream);
}
static cssEl* cssElCFun_fclose_stub(int, cssEl* arg[]) {
  fstream* fh = (fstream*)*(arg[1]);
  if((fh != NULL) && fh->rdbuf()->is_open()) {
    fh->close(); fh->clear();
  }
  return &cssMisc::Void;
}
static cssEl* cssElCFun_fprintf_stub(int na, cssEl* arg[]) {
  ostream* fh = (ostream*)*(arg[1]);
  if((fh == NULL) || !fh->good())
    return &cssMisc::Void;
  int i;
  for(i=2; i <= na; i++) {
    (arg[i])->PrintF(*fh);
  }
  fh->flush();
  return &cssMisc::Void;
}
static cssEl* cssElCFun_printf_stub(int na, cssEl* arg[]) {
  cssProg* cp = arg[0]->prog;
  int i;
  for(i=1; i <= na; i++) {
    (arg[i])->PrintF(*(cp->top->fout));
  }
  cp->top->fout->flush();
  return &cssMisc::Void;
}

//////////////////////////////////
// 	POSIX Functions  	//
//////////////////////////////////

static cssEl* cssElCFun_access_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = access((const char*)*arg[1], (int)*arg[2]);
  return rval;
}
static cssEl* cssElCFun_alarm_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = alarm((Int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_chdir_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = chdir((const char*)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_chown_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = chown((const char*)*arg[1], (int)*arg[2], (int)*arg[3]);
  return rval;
}
static cssEl* cssElCFun_ctermid_stub(int, cssEl**) {
  cssString* rval = new cssString();
  rval->val = ctermid(NULL);
  return rval;
}
#ifndef DARWIN
static cssEl* cssElCFun_cuserid_stub(int, cssEl**) {
  cssString* rval = new cssString();
  rval->val = cuserid(NULL);
  return rval;
}
#endif
static cssEl* cssElCFun_getcwd_stub(int, cssEl**) {
  cssString* rval = new cssString();
  char* cwd = getcwd(NULL, 1024);
  rval->val = cwd;
  free(cwd);
  return rval;
}
static cssEl* cssElCFun_getenv_stub(int, cssEl* arg[]) {
  cssString* rval = new cssString();
  char* env_val = getenv((const char*)*arg[1]);
  if(env_val != NULL)
    rval->val = env_val;
  return rval;
}
static cssEl* cssElCFun_getegid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)getegid();
  return rval;
}
static cssEl* cssElCFun_geteuid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)geteuid();
  return rval;
}
static cssEl* cssElCFun_getgid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)getegid();
  return rval;
}
static cssEl* cssElCFun_getuid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)getuid();
  return rval;
}
static cssEl* cssElCFun_getlogin_stub(int, cssEl**) {
  cssString* rval = new cssString();
  rval->val = getlogin();
  return rval;
}
static cssEl* cssElCFun_getpgrp_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
// #if defined(SUN4) && !defined(SOLARIS)
//   rval->val = (int)getpgrp(0);
// #else
  rval->val = (int)getpgrp();
// #endif
  return rval;
}
static cssEl* cssElCFun_getpid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)getpid();
  return rval;
}
static cssEl* cssElCFun_getppid_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)getppid();
  return rval;
}
static cssEl* cssElCFun_isatty_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)isatty((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_link_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)link((const char*)*arg[1], (const char*)*arg[2]);
  return rval;
}

#if defined(SUN4)|| defined(SUN4solaris)
extern "C" int symlink(const char*, const char*);
extern "C" int rename(const char*, const char*);
#endif

static cssEl* cssElCFun_symlink_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)symlink((const char*)*arg[1], (const char*)*arg[2]);
  return rval;
}
static cssEl* cssElCFun_unlink_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)unlink((const char*)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_pause_stub(int, cssEl**) {
  cssInt* rval = new cssInt();
  rval->val = (int)pause();
  return rval;
}
static cssEl* cssElCFun_rename_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)rename((const char*)*arg[1], (const char*)*arg[2]);
  return rval;
}
static cssEl* cssElCFun_rmdir_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)rmdir((const char*)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_setgid_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)setgid((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_setpgid_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)setpgid((int)*arg[1], (int)*arg[2]);
  return rval;
}
static cssEl* cssElCFun_setuid_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)setuid((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_sleep_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)sleep((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_tcgetpgrp_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)tcgetpgrp((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_tcsetpgrp_stub(int, cssEl* arg[]) {
  cssInt* rval = new cssInt();
  rval->val = (int)tcsetpgrp((int)*arg[1], (int)*arg[2]);
  return rval;
}
static cssEl* cssElCFun_ttyname_stub(int, cssEl* arg[]) {
  cssString* rval = new cssString();
  rval->val = ttyname((int)*arg[1]);
  return rval;
}
static cssEl* cssElCFun_perror_stub(int, cssEl* arg[]) {
  perror((const char*)*arg[1]);
  return &cssMisc::Void;
}

static cssEl* cssElCFun_putenv_stub(int, cssEl* arg[]) {
  static String_Array env_vals;	// need to keep strings around since putenv uses it..
  String env_val = (const char*)*arg[1];
  String env_cue = env_val.through('=');	// must have an equals..

  int i;
  for(i=0; i<env_vals.size; i++) {
    if(env_vals.FastEl(i).at(0,env_cue.length()) == env_cue) { // redefining existing
      env_vals.FastEl(i) = env_val;
      if(putenv((char*)env_vals.FastEl(i)))
	return cssBI::true_int; // no sense in creating more baggage..
      else
	return cssBI::false_int;
    }
  }
  env_vals.Add(env_val);
  if(putenv((char*)env_vals.Peek()))
    return cssBI::true_int; // no sense in creating more baggage..
  else
    return cssBI::false_int;
}

// possibly non-posix but nonetheless useful functions

// #if defined(SUN4) && !defined(SOLARIS)
// extern "C" {
//   int gettimeofday(struct timeval*, struct timezone*);
//   clock_t clock();
// }
// #define CLOCKS_PER_SEC 1000000
// #endif

static cssEl* cssElCFun_gettimesec_stub(int, cssEl**) {
  struct timezone tzp;
  struct timeval tp;
  gettimeofday(&tp,&tzp);
  return new cssInt((int)tp.tv_sec);
}

static cssEl* cssElCFun_gettimemsec_stub(int, cssEl**) {
  struct timezone tzp;
  struct timeval tp;
  gettimeofday(&tp,&tzp);
  return new cssInt((int)tp.tv_usec);
}

static cssEl* cssElCFun_clock_stub(int, cssEl**) {
  clock_t clk = clock();
  double sec = (double)clk / (double)CLOCKS_PER_SEC;
  return new cssReal(sec);
}


// apparently this is not available on all platforms..
//
// #include <sys/resource.h>
// 
// extern "C" getrusage(int, struct rusage*);
// 
// static cssEl* cssElCFun_getrusage_stub(int, cssEl**) {
//   static int_Array vals;
//   static cssTA* rv_ta = NULL;
//   struct rusage rus;
//   getrusage(RUSAGE_SELF,&rus);
//   vals.EnforceSize(18);
//   vals[0] = (int)rus.ru_utime.tv_sec;  // user time used, secs
//   vals[1] = (int)rus.ru_utime.tv_usec; // user time used, microsecs
//   vals[2] = (int)rus.ru_stime.tv_sec;  // system time used, secs
//   vals[3] = (int)rus.ru_stime.tv_usec; // system time used, microsecs
//   vals[4] = (int)rus.ru_maxrss;	  // max resident size
//   vals[5] = (int)rus.ru_ixrss;	  // integral shared text memory size
//   vals[6] = (int)rus.ru_idrss;	  // integral unshared data size
//   vals[7] = (int)rus.ru_isrss;	  // integral unshared stack size
//   vals[8] = (int)rus.ru_minflt;	  // page reclaims
//   vals[9] = (int)rus.ru_majflt;	  // page faults
//   vals[10]= (int)rus.ru_nswap;	  // swaps
//   vals[11]= (int)rus.ru_inblock;	  // block input operations
//   vals[12]= (int)rus.ru_oublock;	  // block output operations
//   vals[13]= (int)rus.ru_msgsnd;	  // messages sent
//   vals[14]= (int)rus.ru_msgrcv;	  // messages received
//   vals[15]= (int)rus.ru_nsignals;	  // signals received
//   vals[16]= (int)rus.ru_nvcsw;	  // voluntary context switches
//   vals[17]= (int)rus.ru_nivcsw;	  // involuntary context switches
//   if(rv_ta == NULL) {
//     rv_ta = new cssTA(NULL, 1, &TA_int_Array);
//     cssEl::Ref(rv_ta);
//   }
//   rv_ta->ptr = (void*)&vals;
//   return new cssRef(rv_ta);
// }
// 

//////////////////////////////////
//	Install_MiscFun		//
//////////////////////////////////

static void Install_MiscFun() {

  // misc functions 
  cssElCFun_inst_nm(cssMisc::Functions, printr, 1, "PrintR", CSS_FUN,
"(<object>) This is the function version of the printr command.  Prints an\
 object and any of its sub-objects in a indented style output.  This can\
 be very long for objects near the top of the object hierarchy (i.e., the\
 root object), so be careful!");
  cssElCFun_inst_nm(cssMisc::Functions, edit, cssEl::VarArg, "EditObj", CSS_FUN,
"(<object>) This is the function version of the edit command.  If the GUI\
 (graphical user interface) is active (i.e., by using -gui to\
 start up CSS), edit will bring up a graphical edit dialog for the given\
 object, which must be either a script-defined or hard-coded class\
 object.  The optional second argument, if TRUE, will cause the system to\
 wait for the user to close the edit dialog before continuing execution\
 of the script.");
  cssElCFun_inst(cssMisc::Functions, CancelEditObj, 1, CSS_FUN,
"(<object>) Cancels the edit dialog for the given object that would have been opened\
 by EditObj.");
  cssElCFun_inst(cssMisc::Functions, Extern, 1, CSS_FUN,
"(String& name) Returns the object with the given name on the 'extern' variable list.\
 This provides a mechanism for passing arbitrary (i.e., class objects)\
 data across different name spaces (i.e., across different instances of\
 the css program space), since you can pass the name of the extern class\
 object that contains data relevant to another script, and use this\
 function to get that object from its name.");
  cssElCFun_inst(cssMisc::Functions, Token, 2, CSS_FUN,
"(<obj_type>, Int tok_no) Returns the token of the given type of object at index tok_no in\
 the list of tokens.  Use the tokens command to obtain a listing\
 of the tokens of a given type of object.");
  cssElCFun_inst(cssMisc::Functions, Type, 1, new cssTA(NULL, 1, &TA_TypeDef),
"(String& typ_nm | <obj_type>) Returns a type descriptor object (generated by TypeAccess), for the\
 given type name or type object (the type object can be used directly in\
 some situations, but not all).");
  cssElCFun_inst(cssMisc::Functions, Dir, cssEl::VarArg, new cssTA(NULL, 1, &TA_String_Array), 
"([String& dir_nm]) Fills an array with the names of all the files in the given directory\
 (defaults to \".\" if no directory name is passed).  The user should\
 copy the array if they want to keep it around, since the one returned\
 is just a pointer to an internal array object.");
  cssElCFun_inst(cssMisc::Functions, ReadLine, 1, new cssTA(NULL, 1, &TA_String_Array),
"(istream& strm) Reads a line of data from the given stream, and returns a reference to\
 an internal array (which is reused upon a subsequent call to ReadLine)\
 of strings with elements containing the whitespace-delimited columns of\
 the line.  The size of the array gives the number of columns, etc.  This\
 allows one to easily implement much of the functionality of awk.  See\
 the file css_awk.css in css/include for an example.");

  // old file-io
  cssElCFun_inst(cssMisc::Functions, fprintf, cssEl::VarArg, CSS_FUN,
"(FILE strm, v1 [,v2...]) Prints the given arguments (which must be comma separated) to the\
 stream.  Values to be printed can be of any type, and are actually\
 printed with the << operator of the stream classes.  Unlike the\
 standard C function, there is no provision for specifying formatting\
 information.  Instead, the formatting must be specified by changing the\
 parameters of the stream object.  The FILE type is not actually a\
 standard C FILE, but actually a fstream type, so stream\
 operations can be performed on it.");
  cssElCFun_inst(cssMisc::Functions, fopen, 2, CSS_FUN, 
"(String& file_nm, String& mode) Opens given file name in the given mode, where the modes are \"r\",\
 \"w\", and \"a\" for read, write and append. The FILE type is not\
 actually astandard C FILE, but actually a fstream type, so\
 stream operations can be performed on it.");
  cssElCFun_inst(cssMisc::Functions, fclose, 1, CSS_FUN,
"(FILE fh) Closes the file, which was opened by fopen.  The FILE type is not\
 actually a standard C FILE, but actually a fstream type, so\
 stream operations can be performed on it.");
  cssElCFun_inst(cssMisc::Functions, printf, cssEl::VarArg, CSS_FUN,
"(v1 [,v2...]) Prints the given arguments (which must be comma separated) to the\
 standard output stream.  Values to be printed can be of any type, and\
 are actually printed with the @code{<<} operator of the stream classes.\
 Unlike the standard C function, there is no provision for specifying\
 formatting information.  Instead, the formatting must be specified by\
 changing the parameters of the standard stream output object,\
 cout.  The FILE type is not actually a standard C FILE, but\
 actually a fstream type, so stream operations can be performed on\
 it.");

  // POSIX functions
  cssElCFun_inst(cssMisc::Functions, access, 2, CSS_FUN,
"(String fname, int ac_type) This POSIX command determines if the given file name is accessible\
 according to the ac_type argument, which should be some bitwise OR of\
 the enums R_OK W_OK X_OK F_OK.  Returns success and sets errno\
 flag on failure.");
  cssEnum_inst_nm(cssMisc::Enums, R_OK, "R_OK");
  cssEnum_inst_nm(cssMisc::Enums, W_OK, "W_OK");
  cssEnum_inst_nm(cssMisc::Enums, X_OK, "X_OK");
  cssEnum_inst_nm(cssMisc::Enums, F_OK, "F_OK");
  cssElCFun_inst(cssMisc::Functions, alarm, 1, CSS_FUN,
"(int seconds) Generate an alarm signal in the given number of seconds.  Returns\
 success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, chdir, 1, CSS_FUN,
"(String dir_name) Change the current directory to given argument.  Returns success and\
 sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, chown, 3, CSS_FUN,
"(String fname, int user, int group) Changes the ownership of the given file to the given user and group\
 numbers.  Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, ctermid, 0, CSS_FUN,
"Returns the character-id of the current terminal.");
#ifndef DARWIN
  cssElCFun_inst(cssMisc::Functions, cuserid, 0, CSS_FUN,
"Returns the character-id of the current user.");
#endif
  cssElCFun_inst(cssMisc::Functions, getcwd, 0, CSS_FUN,
"Returns the current working directory path.");
  cssElCFun_inst(cssMisc::Functions, getenv, 1, CSS_FUN,
"Returns the environment variable definition for varable var.");
  cssElCFun_inst(cssMisc::Functions, getegid, 0, CSS_FUN,
"Returns the current effective group id number for this process.");
  cssElCFun_inst(cssMisc::Functions, geteuid, 0, CSS_FUN,
"Returns the current effective user id number for this process.");
  cssElCFun_inst(cssMisc::Functions, getgid, 0, CSS_FUN,
"Returns the current group id number for this process.");
  cssElCFun_inst(cssMisc::Functions, getuid, 0, CSS_FUN,
"Returns the current user id number for this process.");
  cssElCFun_inst(cssMisc::Functions, getlogin, 0, CSS_FUN,
"Returns the name the current user logged in as.");
  cssElCFun_inst(cssMisc::Functions, getpgrp, 0, CSS_FUN,
"Returns the process group id for current process.");
  cssElCFun_inst(cssMisc::Functions, getpid, 0, CSS_FUN,
"Returns the process id for current process.");
  cssElCFun_inst(cssMisc::Functions, getppid, 0, CSS_FUN,
"Returns the parent process id for current process.");
  cssElCFun_inst(cssMisc::Functions, isatty, 1, CSS_FUN,
"Returns true if the current input terminal is a tty (as opposed to a\
 file or a pipe or something else).");
  cssElCFun_inst(cssMisc::Functions, link, 2, CSS_FUN,
"(String from, String to) Creates a hard link from given file to other file.  (see also symlink).\
 Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, symlink, 2, CSS_FUN,
"(String from, String to) Creates a symbolic link from given file to other file.  (see also link).\
 Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, unlink, 1, CSS_FUN,
"(String fname) Unlinks (removes) the given file name.");
  cssElCFun_inst(cssMisc::Functions, pause, 0, CSS_FUN,
"Pause (wait) until an alarm or other signal is received.  Returns\
 success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, rename, 2, CSS_FUN,
"(String from, String to) Renames given file.  Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, rmdir, 1, CSS_FUN,
"(String dir_name) Removes given directory.  Returns success and sets errno flag on\
 failure.");
  cssElCFun_inst(cssMisc::Functions, setgid, 1, CSS_FUN,
"(Int id) Sets group id for given process to that given.  Note that only the\
 super-user can in general do this.  Returns success and sets errno flag\
 on failure.");
  cssElCFun_inst(cssMisc::Functions, setpgid, 2, CSS_FUN,
"(Int id) Sets process group id for given process to that given.  Note that only\
 the super-user can in general do this.  Returns success and sets errno\
 flag on failure.");
  cssElCFun_inst(cssMisc::Functions, setuid, 1, CSS_FUN,
"(Int id) Sets user id for given process to that given.  Note that only the\
 super-user can in general do this.  Returns success and sets errno flag\
 on failure.");
  cssElCFun_inst(cssMisc::Functions, sleep, 1, CSS_FUN,
"(Int seconds) Causes the process to wait for given number of seconds.  Returns success\
 and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, tcgetpgrp, 1, CSS_FUN,
"(Int file_no) Gets the process group associated with the given file descriptor.\
 Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, tcsetpgrp, 2, CSS_FUN,
"(Int file_no) Sets the process group associated with the given file descriptor.\
 Returns success and sets errno flag on failure.");
  cssElCFun_inst(cssMisc::Functions, ttyname, 1, CSS_FUN,
"(Int file_no) Returns the terminal name associated with the given file descriptor.");
  cssElCFun_inst(cssMisc::Functions, perror, 1, CSS_FUN,
"(String prompt) Prints out the current error message to stderr (cerr).  The prompt\
 argument is printed before the error message.  Also, the global variable\
 errno can be checked.  Further, there is an include file in\
 css/include called errno.css that defines an enumerated type for the\
 defined values of errno.");
  cssCPtr_int_inst(cssMisc::HardVars, errno);
  cssElCFun_inst(cssMisc::Functions, putenv, 1, CSS_FUN,
"(String env_val) Put the environment value into the list of environment values (avail\
 through getenv).");
  cssElCFun_inst(cssMisc::Functions, system, 1, CSS_FUN,
"(String& cmd) Executes the given command in the Unix shell.");

  cssElCFun_inst(cssMisc::Functions, gettimesec, 0, CSS_FUN,
"Returns current time of day in seconds.");
  cssElCFun_inst(cssMisc::Functions, gettimemsec, 0, CSS_FUN,
"Returns current time of day in microseconds.");
  cssElCFun_inst(cssMisc::Functions, clock, 0, CSS_FUN,
"Returns processor time used by current process in seconds (with\
 fractions expressed in decimals).");
//  cssElCFun_inst(cssMisc::Functions, getrusage, 0, CSS_FUN, " ");

  // parsed internal ptrs
  cssMisc::HardVars.Push(new cssTA((void*)&cout, 1, &TA_ostream, "cout"));
  cssMisc::HardVars.Push(new cssTA((void*)&cin, 1, &TA_istream, "cin"));
  cssMisc::HardVars.Push(new cssTA((void*)&cerr, 1, &TA_ostream, "cerr"));

  cssMisc::HardVars.Push(new cssTA((void*)&cout, 1, &TA_ostream, "stdout"));
  cssMisc::HardVars.Push(new cssTA((void*)&cin, 1, &TA_istream, "stdin"));
  cssMisc::HardVars.Push(new cssTA((void*)&cerr, 1, &TA_ostream, "stderr"));

  // magic stream codes!
  cssMisc::Constants.Push(new cssBool(false, "ws"));
  cssMisc::Constants.Push(new cssBool(false, "flush"));
  cssMisc::Constants.Push(new cssBool(false, "endl"));
  cssMisc::Constants.Push(new cssBool(false, "ends"));
  cssMisc::Constants.Push(new cssBool(false, "dec"));
  cssMisc::Constants.Push(new cssBool(false, "hex"));
  cssMisc::Constants.Push(new cssBool(false, "oct"));
  cssMisc::Constants.Push(new cssBool(false, "lock"));
  cssMisc::Constants.Push(new cssBool(false, "unlock"));

  // constants  
  cssMisc::Enums.Push(new cssBool(true, "true"));
  cssMisc::Enums.Push(new cssBool(false, "false"));
  cssBI::true_int = new cssInt(1, "true");
  cssBI::false_int = new cssInt(0, "false");
  cssMisc::Internal.Push(cssBI::true_int); // keep them on this list (to refn)
  cssMisc::Internal.Push(cssBI::false_int);
  cssInt_inst	(cssMisc::Constants, 0, NULL);
}


//////////////////////////////////
//	Install_Types		//
//////////////////////////////////

static void Install_Types() {
  // basic CSS types
  cssMisc::TypesSpace.Push(&cssMisc::Void);
  cssMisc::TypesSpace.Push(new cssEl("void"));
  cssInt_inst_nm(cssMisc::TypesSpace, 0, "Int");
  cssInt_inst_nm(cssMisc::TypesSpace, 0, "int");
  cssMisc::TypesSpace.Push(new cssBool(false, "bool"));
  cssMisc::TypesSpace.Push(new cssBool(false, "Bool"));
  cssReal_inst_nm(cssMisc::TypesSpace, 0.0, "Real");
  cssReal_inst_nm(cssMisc::TypesSpace, 0.0, "real");
  cssReal_inst_nm(cssMisc::TypesSpace, 0.0, "float");
  cssReal_inst_nm(cssMisc::TypesSpace, 0.0, "double");
  cssString_inst_nm(cssMisc::TypesSpace, "", "String");
  cssString_inst_nm(cssMisc::TypesSpace, "", "string");
  cssMisc::TypesSpace.Push(new cssChar(' ', "char"));
  cssCPtr_inst_nm(cssMisc::TypesSpace, 0, 1, "c_ptr");
  cssMisc::TypesSpace.Push(new cssSubShell("SubShell"));

  // CPtr types
  cssMisc::TypesSpace.Push(new cssCPtr_int(NULL, 1, "c_int"));
  cssMisc::TypesSpace.Push(new cssCPtr_bool(NULL, 1, "c_bool"));
  cssMisc::TypesSpace.Push(new cssCPtr_short(NULL, 1, "c_short"));
  cssMisc::TypesSpace.Push(new cssCPtr_long(NULL, 1, "c_long"));
  cssMisc::TypesSpace.Push(new cssCPtr_char(NULL, 1, "c_char"));
  cssMisc::TypesSpace.Push(new cssCPtr_enum(NULL, 1, "c_enum"));
  cssMisc::TypesSpace.Push(new cssCPtr_double(NULL, 1, "c_double"));
  cssMisc::TypesSpace.Push(new cssCPtr_float(NULL, 1, "c_float"));
  cssMisc::TypesSpace.Push(new cssCPtr_String(NULL, 1, "c_String"));

  // ta types
  cssTA_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_void, "TA");
  cssTA_Base_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_taBase, "TAPtr");
  cssTA_Base_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_taBase, "taBase");
  cssMisc::TypesSpace.Push(new cssFStream("fstream"));
  cssMisc::TypesSpace.Push(new cssSStream("sstream"));
  cssTA_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_fstream, "FILE");
  cssMisc::TypesSpace.Push(new cssLeafItr("taLeafItr"));
  
#ifdef CSS_SUPPORT_TYPEA
  cssTA_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_TypeDef, "TypeDef");
  cssTA_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_MemberDef, "MemberDef");
  cssTA_inst_nm (cssMisc::TypesSpace, NULL, 1, &TA_MethodDef, "MethodDef");
#endif

  // add all user-defined types..
  int i, j;
  for(i=0; i<taMisc::types.size; i++) {
    TypeDef* tmp = taMisc::types.FastEl(i);
    if(tmp->InheritsFormal(TA_class)) {
      // don't add any template instances that have a single further subclass
      // (use the subclass instead)
      if(tmp->InheritsFormal(TA_templ_inst)) {
	if((tmp->children.size != 1) || (tmp->children.FastEl(0)->parents.size > 1)) {
	  if(tmp->InheritsFrom(TA_taBase))
	    cssTA_Base_inst_nm(cssMisc::TypesSpace, tmp->GetInstance(), 1, tmp, tmp->name);
	  else
	    cssTA_inst_nm(cssMisc::TypesSpace, tmp->GetInstance(), 1, tmp, tmp->name);
	}
      }
      else if(!((tmp->members.size==0) && (tmp->methods.size==0)) &&
	      (tmp->name != "fstream") && (tmp->name != "class") && 
	      (tmp->name != "taString") && (tmp->name != "taLeafItr") &&
	      (tmp->name != "sstream"))
      {
	if(tmp->InheritsFrom(TA_taBase))
	  cssTA_Base_inst_nm(cssMisc::TypesSpace, tmp->GetInstance(), 1, tmp, tmp->name);
	else
	  cssTA_inst_nm(cssMisc::TypesSpace, tmp->GetInstance(), 1, tmp, tmp->name);
      }
    }
    else if(tmp->InheritsFormal(TA_enum)) {
      cssEnum_inst_nm(cssMisc::Enums, 0, tmp->name);
      for(j=0; j < tmp->enum_vals.size; j++)
	cssInt_inst_nm(cssMisc::Enums, tmp->enum_vals.FastEl(j)->enum_no,
			tmp->enum_vals.FastEl(j)->name);
    }
    else if(tmp->InheritsFrom(TA_taRegFun)) {
      for(j=0; j < tmp->methods.size; j++) {
	MethodDef* md = tmp->methods.FastEl(j);
	if(md->stubp != NULL)
	  cssMisc::HardFuns.Push(new cssMbrCFun(md->fun_argc, NULL, md->stubp,
					     (const char*)md->name));
      }
    }
  }
}

//////////////////////////////////
//   Completion in Readline	//
//////////////////////////////////

extern "C" {
  typedef int rl_function(void);
  typedef char* rl_generator_fun(char*, int);
  extern int (*rl_attempted_completion_function)(void);
  extern char** completion_matches (char* text, rl_generator_fun* gen);
  extern char** css_attempted_completion(char* text, int start, int end);
  extern char* css_path_generator(char* text, int state);
  extern char* css_scoped_generator(char* text, int state);
  extern char* css_keyword_generator(char* text, int state);
  extern char *rl_line_buffer;
}

char** css_attempted_completion(char* text, int start, int) {
  char** matches = NULL;
  String txt_str = text;

  // if starting with a quote, assume its a file name
  if((start > 0) && (rl_line_buffer[start-1] == '\"')) // "
    return matches;
  // if it starts with a dot, then its a path
  if(text[0] == '.')
    matches = completion_matches(text, css_path_generator);
  // check for a scoped type
  else if(txt_str.contains("::"))
    matches = completion_matches(text, css_scoped_generator);
  // otherwise, it must be a keyword of some sort..
  else
    matches = completion_matches(text, css_keyword_generator);

  return matches;
}

static char* css_malloc_string(const String& str) {
  char* rval = (char*)malloc((str.length()+1) * sizeof(char*));
  strcpy(rval, (const char*)str);
  return rval;
}

static char* css_cmd_gen_get_nm(cssSpace* spc, int& idx, char* text, int len) {
  while(idx < spc->size) {
    cssEl* el = spc->FastEl(idx++);
    if(el->name(0,len) == text)
      return css_malloc_string(el->name);
  }
  return NULL;
}

char* css_keyword_generator(char* text, int state) {
  static int spc_idx;		// index in spaces searching through
  static int item_idx;		// index in items searching through
  static int len;
  
  if(state == 0) {
    spc_idx = 0;
    item_idx = 0;
    len = strlen(text);
  }

  cssSpace* spc = NULL;
  int extra_spcs = 2;		// both type spaces
  if(cssMisc::cur_class != NULL)
    extra_spcs++;

  if(spc_idx == 0)
    spc = &(cssMisc::cur_top->types);
  else if(spc_idx == 1)
    spc = &cssMisc::TypesSpace;
  else if((cssMisc::cur_class != NULL) && (spc_idx == 2))
    spc = cssMisc::cur_class->types;
  else
    spc = cssMisc::cur_top->GetParseSpace(spc_idx-extra_spcs);

  char* rval = NULL;
  while((spc != NULL) && ((rval = css_cmd_gen_get_nm(spc, item_idx, text, len)) == NULL)) {
    item_idx = 0;
    spc_idx++;			// move to next index
    if(spc_idx == 1)
      spc = &cssMisc::TypesSpace;
    else if((cssMisc::cur_class != NULL) && (spc_idx == 2))
      spc = cssMisc::cur_class->types;
    else
      spc = cssMisc::cur_top->GetParseSpace(spc_idx - extra_spcs);
  }
  return rval;
}

static String* css_path_search(TypeDef* td, int lst_idx, int& idx, String& mb_name) {
  if(lst_idx == 0) {
    while(idx < td->members.size) {
      MemberDef* md = td->members.FastEl(idx++);
      if(md->name(0,mb_name.length()) == mb_name)
	return &(md->name);
    }
    return NULL;
  }
  else {
    while(idx < td->methods.size) {
      MethodDef* md = td->methods.FastEl(idx++);
      if(md->name(0,mb_name.length()) == mb_name)
	return &(md->name);
    }
    return NULL;
  }
}

char* css_path_generator(char* text, int state) {
  static int lst_idx;		// list index
  static int item_idx;		// index for searching within list
  static String mb_name;
  static String par_path;
  static TAPtr parent;
  static TypeDef* par_td;

  if(cssBI::root == NULL)
    return NULL;

  if(state == 0) {
    par_path = text;
    par_path = par_path.before('.',-1);
    if(par_path == "") {
      parent = (TAPtr)cssBI::root->ptr;
      if(parent != NULL)
	par_td = parent->GetTypeDef();
    }
    else {
      MemberDef* md = NULL;
      TAPtr root = (TAPtr)cssBI::root->ptr;
      if(root == NULL)
	parent = NULL;
      else {
	parent = root->FindFromPath(par_path, md);
	if(parent != NULL) {
	  par_td = parent->GetTypeDef();
// no need to do this..
//	  par_path = parent->GetPath();
	}
      }
    }
    mb_name = text;
    mb_name = mb_name.after('.', -1);
    item_idx = 0;
    lst_idx = 0;
  }

  if(parent == NULL)
    return NULL;

  String* rval = NULL;
  while((lst_idx < 2) &&
	((rval = css_path_search(par_td, lst_idx, item_idx, mb_name)) == NULL))
  {
    item_idx = 0;
    lst_idx++;
  }

  if(rval == NULL)
    return NULL;

  String full_val = par_path;
  full_val += ".";
  full_val += *rval;
  return css_malloc_string(full_val);
}

static String* css_scope_search(TypeDef* td, int lst_idx, int& idx, int& sub_itm, 
				String& mb_name)
{
  if(lst_idx == 0) {
    while(idx < td->members.size) {
      MemberDef* md = td->members.FastEl(idx++);
      if(md->name(0,mb_name.length()) == mb_name)
	return &(md->name);
    }
    return NULL;
  }
  else if(lst_idx == 1) {
    while(idx < td->methods.size) {
      MethodDef* md = td->methods.FastEl(idx++);
      if(md->name(0,mb_name.length()) == mb_name)
	return &(md->name);
    }
    return NULL;
  }
  else {
    while(idx < td->sub_types.size) {
      TypeDef* st = td->sub_types.FastEl(idx);
      if(st->InheritsFormal(TA_enum)) {
	while(sub_itm < st->enum_vals.size) {
	  EnumDef* en = st->enum_vals.FastEl(sub_itm++);
	  if(en->name(0,mb_name.length()) == mb_name)
	    return &(en->name);
	}
      }
      idx++;
      sub_itm = 0;
      if(st->name(0,mb_name.length()) == mb_name)
	return &(st->name);
    }
    return NULL;
  }
}

char* css_scoped_generator(char* text, int state) {
  static int lst_idx;		// list index
  static int item_idx;		// index for searching within list
  static int sub_item_idx;	// index of sub-item within item (if applic)
  static String mb_name;
  static String par_path;
  static TypeDef* par_td;

  if(state == 0) {
    par_path = text;
    par_path = par_path.before("::",-1);
    par_td = taMisc::types.FindName(par_path);
    mb_name = text;
    mb_name = mb_name.after("::", -1);
    item_idx = 0;
    lst_idx = 0;
    sub_item_idx = 0;
  }

  if(par_td == NULL)
    return NULL;

  String* rval = NULL;
  while((lst_idx < 3) &&
	((rval = css_scope_search(par_td, lst_idx, item_idx, sub_item_idx, mb_name))
	 == NULL)) {
    item_idx = 0;
    sub_item_idx = 0;
    lst_idx++;
  }

  if(rval == NULL)
    return NULL;

  String full_val = par_path;
  full_val += "::";
  full_val += *rval;
  return css_malloc_string(full_val);
}


//////////////////////////////////
//	Initialize		//
//////////////////////////////////

extern "C" {
  extern int (*rl_event_hook)(void);	// this points to the waitproc if running IV
  extern int readline_nogui_waitproc(void);
}

int readline_nogui_waitproc() {
  return tabMisc::WaitProc();
}

void cssMisc::Initialize(int argc, const char** argv) {
  setlocale(LC_ALL, "");

  // use our completion function
  rl_attempted_completion_function = (rl_function*)css_attempted_completion;
 // set the hook to our nogui "waitproc" -- can be overridden later
  rl_event_hook = readline_nogui_waitproc;

  // functions
  cssEl::Ref(&cssMisc::Void);		// reference this to keep it around

  cssMisc::Top = new cssProgSpace("C^c Top Level");	// top level
  cssMisc::ConstExprTop = new cssProgSpace("Constant Expression Top Level");
  cssMisc::ConstExpr = cssMisc::ConstExprTop->Prog(); // this guy is child of top
  cssMisc::CDtorProg = new cssProg("Constructor/Destructor Prog");
  cssMisc::CallFunProg = new cssProg("CallFun Prog");
  cssProg::Ref(cssMisc::CDtorProg);
  cssProg::Ref(cssMisc::CallFunProg);
  cssMisc::cur_top = cssMisc::Top;

  Install_Internals();
  Install_Commands();
  Install_Types();
  Install_Math();
  Install_MiscFun();

  cssMisc::s_argv = new cssArray(1, cssMisc::VoidString.Clone(), "argv");
  cssMisc::s_argc = new cssInt(0, "argc");
  cssMisc::HardVars.Push(cssMisc::s_argv);
  cssMisc::HardVars.Push(cssMisc::s_argc);

  // setting variables
  cssMisc::Settings.Push(new cssTA(&taMisc::include_paths, 1, &TA_String_PArray,
				    "include_paths"));

  cssMisc::Settings.Push(new cssTA(TA_taMisc.GetInstance(), 1, &TA_taMisc,
				    "cssSettings"));

  cssMisc::TypesSpace.Sort();		// must sort before anything happens
  cssMisc::Commands.Sort();
  cssMisc::Functions.Sort();		// must sort before anything happens
  cssMisc::Constants.Sort();		// must sort before anything happens

  String home = getenv("HOME");
  taMisc::include_paths.AddUnique(home); // this is first on the list..

#ifndef CYGWIN
  String css_dir = "/usr/local/pdp++"; // default css home directory
#else
  String css_dir = "C:/PDP++"; // default pdp home directory
#endif
  char* css_dir_env = getenv("CSSDIR");
  if(css_dir_env != NULL)
    css_dir = css_dir_env;

  taMisc::include_paths.AddUnique(css_dir);
  taMisc::include_paths.AddUnique(css_dir+"/css/include");

  if(argc > 0) {
    // args can get munged together when #!/usr/local/css, so need to pre-process
    String_Array act_args;
    int i;
    for(i=0;i<argc;i++) {
      String tmp = argv[i];
      if(tmp.contains('-') && tmp.contains(' ')) { // args munged from auto start script
	while(tmp.contains('-') && tmp.contains(' ')) {
	  String t2 = tmp.before(' ');
	  tmp = tmp.after(' ');
	  act_args.Add(t2);
	}
	if(tmp.contains('-'))
	  act_args.Add(tmp);
      }
      else
	act_args.Add(tmp);
    }
    cssMisc::s_argv->Alloc(act_args.size); // allocate proper number of args
    *cssMisc::s_argc = 0;
    i = 0;
    while(i < act_args.size) {
      String tmp = act_args[i];
      if(((tmp == "-f") || (tmp == "-file")) && (i+1 < act_args.size)) { // compile the file
	cssMisc::startup_file = act_args[++i];
      }
      else if(((tmp == "-e") || (tmp == "-exec")) && (i+1 < act_args.size)) { // compile the file
	cssMisc::startup_code = act_args[++i];
//	cssMisc::startup_code.gsub(";\\n", ";\n"); // translate cr.s
	cssMisc::startup_code += "\n";		  // add a final cr for good measure
      }
      else if((tmp == "-i") || (tmp == "-interactive")) {
	cssMisc::init_interactive = true;
      }
      else if(tmp(0,2) == "-v") {
	String vls = tmp.after(1);
	int vl = (int)vls;
	if(vl > 0)
	  cssMisc::init_debug = vl;
	else
	  cssMisc::init_debug = 1;
      }
      else if(((tmp == "-b") || (tmp == "-bp")) && (i+1 < act_args.size)) {
	int vl = (int)act_args[++i];
	cssMisc::init_bpoint = vl;
      }
      else {	   // pass the arg to the script
	int ac = (int)(*cssMisc::s_argc);
	cssString* av = (cssString*)(*cssMisc::s_argv)[ac];
	*av = tmp;
	*cssMisc::s_argc = (int)(*cssMisc::s_argc) + 1;
	if(i == 0) {
	  if(tmp.contains('/'))
	    tmp = tmp.after('/',-1);
	  cssMisc::prompt = tmp; // default prompt is this program
	}
      }
      i++;
    }
  }
}
