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

// the lexical analyzer

#include <css/machine.h>
#include <css/basic_types.h>
#include <css/c_ptr_types.h>
#include <css/ta_css.h>
#ifndef CSS_NUMBER
#include <css/css_parse.h>
#endif

static int follow(int expect, int ifyes, int ifno) {
  int c = cssMisc::cur_top->Prog()->Getc();
  
  if(c == expect) return ifyes;
  cssMisc::cur_top->Prog()->unGetc();
  return ifno;
}

static int follow2(int expect1, int if1, int expect2, int if2, int ifno) {
  int c = cssMisc::cur_top->Prog()->Getc();
  
  if(c == expect1) return if1;
  if(c == expect2) return if2;
  cssMisc::cur_top->Prog()->unGetc();
  return ifno;
}

static int follow3(int expect1, int if1, int expect2, int if2, 
		   int expect3, int if3, int ifno) {
  int c = cssMisc::cur_top->Prog()->Getc();
  
  if(c == expect1) return if1;
  if(c == expect2) return if2;
  if(c == expect3) return if3;
  cssMisc::cur_top->Prog()->unGetc();
  return ifno;
}

static int skip_white_nocr() {
  int c;
  while (((c=cssMisc::cur_top->Prog()->Getc()) == ' ') || (c == '\t') || (c == '\r'));
  return c;
}

static int skip_white() {
  int c;
  while (((c=cssMisc::cur_top->Prog()->Getc()) == ' ') || (c == '\t') || (c == '\r') || (c == '\n'));
  return c;
}

static int ctrl_char(int c) {
  int rval = ' ';
  switch (c) {
  case 'n':
    rval = '\n';	break;
  case 't':
    rval = '\t';	break;
  case 'v':
    rval = '\v';	break;
  case 'b':
    rval = '\b';	break;
  case 'r':
    rval= '\r';		break;
  case 'f':
    rval = '\f';	break;
  case '\\':
    rval = '\\';	break;
  case '?':
    rval = '\?';	break;
  case '\'':
    rval = '\'';	break;
  case '\"':
    rval = '\"';	break;	// "
  case '0':
    rval = '\0';	break;
  }
  return rval;
}

int yylex()
{
  int c, nxt;
  cssElPtr s;

  // for parsing class comments
  int last_tok_line = cssMisc::cur_top->Prog()->tok_line;
  char* last_tok =
     cssMisc::cur_top->Prog()->GetSrcLC(last_tok_line,
                                        cssMisc::cur_top->Prog()->tok_col);
  do {
    // actual start of the lex
    cssMisc::cur_top->Prog()->st_col = cssMisc::cur_top->Prog()->col;
    cssMisc::cur_top->Prog()->st_line = cssMisc::cur_top->Prog()->line;

    // filter out cr's only when not parsing commands
    if(cssMisc::cur_top->parsing_command || (cssMisc::cur_top->Prog()->col <= 1))
      c = skip_white_nocr();
    else
      c = skip_white();

    // start of the current token
    cssMisc::cur_top->Prog()->tok_col = cssMisc::cur_top->Prog()->col-1;
    cssMisc::cur_top->Prog()->tok_line = cssMisc::cur_top->Prog()->line;

    // for parsing class comments
    bool classcmt = ((cssMisc::cur_class != NULL) &&
                     (cssMisc::cur_method == NULL) &&
                     (cssMisc::cur_top->Prog()->tok_line == last_tok_line ||
                      cssMisc::cur_top->Prog()->tok_line == last_tok_line + 1) &&
                     (*last_tok == ';' || *last_tok == '{'));

    if(c == EOF) {
      return cssProg::YY_Exit;
    }

    if(c == '#') {
      nxt = cssLex::readword(cssMisc::cur_top->Prog(), cssMisc::cur_top->Prog()->Getc());
      if(cssLex::Buf == "!") {		// skip line on #!
	while(((c = cssMisc::cur_top->Prog()->Getc()) != EOF) && (c != '\n'));
	continue;		// restart scan on next line
      }


      if((s = cssMisc::PreProcessor.FindName((char*)cssLex::Buf)) != 0) { // pre-processor
	cssMisc::cur_top->parsing_command = 1;
	yylval.el = s;
	return (s.El())->GetParse();
      }
      yylval.nm = cssLex::Buf;
      return CSS_NAME;
    }
    
    if(c == '.') {
      nxt = cssMisc::cur_top->Prog()->Getc();
      cssMisc::cur_top->Prog()->unGetc(); // look ahead
      if(!(isdigit(nxt)))
	return '.';		// pointsat
    }

    bool got_minus_num = false;
    if(cssMisc::parsing_args) {
      if(c == '-') {
	nxt = cssMisc::cur_top->Prog()->Getc();
	cssMisc::cur_top->Prog()->unGetc(); // look ahead
	if(!(isdigit(nxt) || (nxt == '.')))
	  return follow3('=', CSS_ASGN_SUB, '-', CSS_MINMIN, '>', CSS_POINTSAT, '-');
	got_minus_num = true;
      }
    }

    if((c == '.') || got_minus_num || isdigit(c)) {	// number
      Real r;
      int iv;
      bool gotreal = false;
      bool gothex = false;
      cssLex::Buf = "";
      int prev = 0;
      
      do {
        if(c == '.' || ((c == 'e' || c == 'E') && !gothex)) gotreal = true;
        else if((c == 'x' || c == 'X') && prev == '0') gothex = true;
        cssLex::Buf += (char)c;
	prev = c;
      } while(((c=cssMisc::cur_top->Prog()->Getc()) != EOF) &&
	      ((c == '.') || isdigit(c) || (c == 'x') || (c == 'e') ||
	       (c == 'X') || (c == 'E') || (c == 'a') || (c == 'A') ||
	       (c == 'b') || (c == 'B') || (c == 'c') || (c == 'C') ||
	       (c == 'd') || (c == 'D') || (c == 'f') || (c == 'F') ||
	       (((prev == 'e') || (prev == 'E')) && (c == '-'))));
      
      cssMisc::cur_top->Prog()->unGetc();
      
      const char* startp = (const char*) cssLex::Buf;
      char* endp = NULL;
      if(gotreal) {
        r = strtod(startp, &endp);
	yylval.el = cssMisc::cur_top->AddLiteral(r);
      }
      else {
        iv = (int)strtol(startp, &endp, 0);
	yylval.el = cssMisc::cur_top->AddLiteral(iv);
      }
      if(*endp == 'f') // float (not double) designator
	endp++;
      while (endp++ < startp + cssLex::Buf.length())
         cssMisc::cur_top->Prog()->unGetc();

      return CSS_NUMBER;
    }
    
    if(isalpha(c) || (c == '_')) {

      cssLex::readword(cssMisc::cur_top->Prog(), c);
      
      if(cssLex::Buf.before(3) == "TA_") {
	// parse as a type name
	String nm = cssLex::Buf.after("TA_");
	TypeDef* td = taMisc::types.FindName(nm);
	if(td != NULL) {
	  cssTA* tatd = new cssTA((void*)td, 1, &TA_TypeDef);
	  yylval.el = cssMisc::cur_top->AddLiteral(tatd);
	  return CSS_VAR;
	}
      }
      if((s = cssMisc::cur_top->types.FindName((char*)cssLex::Buf)) != 0) { 
	yylval.el = s;
	if((s.El())->GetParse() == CSS_PTR)
	  return CSS_PTRTYPE;
	else
	  return CSS_TYPE;
      }
      if((s = cssMisc::TypesSpace.FindName((char*)cssLex::Buf)) != 0) { 
	yylval.el = s;
	if((s.El())->GetParse() == CSS_PTR)
	  return CSS_PTRTYPE;
	else
	  return CSS_TYPE;
      }
      if((cssMisc::cur_class != NULL) && 
	 ((s = cssMisc::cur_class->types->FindName((char*)cssLex::Buf)) != 0)) {
	yylval.el = s;
	if(s.El()->GetType() == cssEl::T_Enum)
	  return CSS_VAR;
	else if((s.El())->GetParse() == CSS_PTR)
	  return CSS_PTRTYPE;
	else
	  return CSS_TYPE;
      }
      if(cssMisc::cur_scope != NULL) {
	if(cssMisc::cur_scope->GetType() == cssEl::T_ClassType) {
	  cssClassType* clt = (cssClassType*)cssMisc::cur_scope;
	  if((s = clt->types->FindName((char*)cssLex::Buf)) != 0) {
	    yylval.el = s;
	    if(s.El()->GetType() == cssEl::T_Enum)
	      return CSS_VAR;
	    else if((s.El())->GetParse() == CSS_PTR)
	      return CSS_PTRTYPE;
	    else
	      return CSS_SCPTYPE;
	  }
	}
	else if(cssMisc::cur_scope->GetParse() == CSS_PTR) {
	  if((s = cssMisc::cur_scope->GetScoped((char*)cssLex::Buf)) != 0) {
	    yylval.el = s;
	    cssEl* obj = s.El();
	    if(obj->GetParse() == CSS_PTR) {
	      if(obj->GetType() == cssEl::T_TA) {
		cssTA* tao = (cssTA*)obj;
		if((tao->type_def != NULL) && tao->type_def->DerivesFormal(TA_enum))
		  return CSS_SCPTYPE;
	      }
	      return CSS_PTRTYPE;
	    }
	    else
	      return CSS_VAR;
	  }
	}
      }
      if((s = cssMisc::cur_top->ParseName((char*)cssLex::Buf)) != 0) {
	yylval.el = s;
      	if(s.ptr == (void*)&(cssMisc::Constants)) {
	  cssEl::cssTypes typ = s.El()->GetType();
	  if((typ == cssEl::T_Int) || (typ == cssEl::T_Real) ||
	     (typ == cssEl::T_Bool) || (typ == cssEl::T_Enum))
	  {
	    return CSS_NUMBER;
	  }
	  else if(typ == cssEl::T_String) {
	    return CSS_STRING;
	  }
	}
	return s.El()->GetParse();
      }
      yylval.nm = cssLex::Buf;
      return CSS_NAME;
    }
    
    if(c == '\"') {
      cssLex::Buf = "";
      
      while(((c=cssMisc::cur_top->Prog()->Getc()) != EOF) && (c != '\"')) {
	if(c == '\\') {
	  c=cssMisc::cur_top->Prog()->Getc();
	  if((c == '\n') || (c == '\r'))
	    continue;
	  else
	    cssLex::Buf += (char)ctrl_char(c);
	}
	else 
	  cssLex::Buf += (char)c;
      }
      yylval.el = cssMisc::cur_top->AddLiteral(cssLex::Buf);
      return CSS_STRING;
    }
    
    if(c == '\'') {
      int iv;
      
      while(((c=cssMisc::cur_top->Prog()->Getc()) != EOF) && (c != '\'')) {
	if(c == '\\')
	  iv = ctrl_char(c=cssMisc::cur_top->Prog()->Getc());
	else
	  iv = c;
      }
      yylval.el = cssMisc::cur_top->AddLiteral(new cssInt(iv));
      return CSS_NUMBER;
    }
    
    if(c == '/') {
      if(((c = cssMisc::cur_top->Prog()->Getc()) == '/') || (c == '*')) { 	// comment
        cssLex::Buf = "";
        if(c == '/') {
          while(((c=cssMisc::cur_top->Prog()->Getc()) != EOF) && (c != '\n'))
            if (classcmt && (c != '\r'))
              cssLex::Buf += (char)c;
        }
	else {
	  while(((c=cssMisc::cur_top->Prog()->Getc()) != EOF)) {
            if(c == '*') {
              if((c = cssMisc::cur_top->Prog()->Getc()) == '/')
		break;
              else if (classcmt)
                cssLex::Buf += '*';
	    }
            if (classcmt)
              cssLex::Buf += (char)c;
	  }
        }
        if (classcmt) {
	  yylval.el = cssMisc::cur_top->AddLiteral(cssLex::Buf);
	  return CSS_COMMENT;
	}
	continue;		// redo loop through
      }
      else {
	c = '/';
	cssMisc::cur_top->Prog()->unGetc();
      }
    }

    if(c == ';') {
      while ((nxt=cssMisc::cur_top->Prog()->Getc()) == ' ' || (nxt == '\t') || (nxt == '\r'));
      if(nxt != '\n')
	cssMisc::cur_top->Prog()->unGetc(); // not a newline after semicolon
      return c;
    }
	
    switch (c) {
    case '<': {
      nxt = cssMisc::cur_top->Prog()->Getc();
      if(nxt == '=')	return CSS_LE;
      if(nxt != '<')	{ cssMisc::cur_top->Prog()->unGetc(); return CSS_LT; }
      return follow('=', CSS_ASGN_LSHIFT, CSS_LSHIFT);
    }
    case '>': {
      nxt = cssMisc::cur_top->Prog()->Getc();
      if(nxt == '=')	return CSS_GE;
      if(nxt != '>')	{ cssMisc::cur_top->Prog()->unGetc(); return CSS_GT; }
      return follow('=', CSS_ASGN_RSHIFT, CSS_RSHIFT);
    }
    case '=':
      return follow('=', CSS_EQ, '=');
    case '!':
      return follow('=', CSS_NE, CSS_NOT);
    case '&':
      return follow2('=', CSS_ASGN_AND, '&', CSS_AND, '&');
    case '^':
      return follow('=', CSS_ASGN_XOR, '^');
    case '|':
      return follow2('=', CSS_ASGN_OR, '|', CSS_OR, '|');
    case '+':
      return follow2('=', CSS_ASGN_ADD, '+', CSS_PLUSPLUS, '+');
    case '-':
      return follow3('=', CSS_ASGN_SUB, '-', CSS_MINMIN, '>', CSS_POINTSAT, '-');
    case '*':
      return follow('=', CSS_ASGN_MULT, '*');
    case '/':
      return follow('=', CSS_ASGN_DIV, '/');
    case ':':
      return follow(':', CSS_SCOPER, ':');
    case '\n':
      return '\n';
    default:
      return c;
    }
  } while(1);

  return 0;
}
