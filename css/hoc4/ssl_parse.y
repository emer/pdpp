/* parser for ssl language */

%{

#include "code.h"

#define Code(c1)	code((Inst)c1);
#define Code2(c1,c2)	Code(c1); Code(c2)
#define Code3(c1,c2,c3)	Code(c1); Code(c2); Code(c3)

%}

%union {				/* stack type */
	Symbol	*sym;			/* symbol table pointer */
	Inst	*inst;			/* machine instruction */
}

%token	<sym>	NUMBER VAR BLTIN UNDEF
%right	'='
%left	'+' '-'	/* left associative, same precedence */
%left	'*' '/'	'%'/* left assoc., higher precedence */
%left	UNARYMINUS
%right	'^'	/* exponentiation */

%%
list:	/* nothing */
	| list '\n'
	| list asgn '\n'	{ Code2(pop, STOP); return 1; }
	| list expr '\n'	{ Code2(print, STOP); return 1; }
	| list error '\n'	{ yyerrok; }
	;
asgn:	VAR '=' expr { Code3(varpush, (Inst)$1, assign); }
	;

expr:	NUMBER		{ Code2(constpush, (Inst)$1); }
        | VAR		{ Code3(varpush, (Inst)$1, eval); }
	| asgn
	| BLTIN '(' expr ')'	{ Code2(bltin, (Inst)$1->u.ptr); }
	| '(' expr ')'		
	| expr '+' expr		{ Code(add); }
	| expr '-' expr		{ Code(sub); }
	| expr '*' expr		{ Code(mul); }
	| expr '/' expr		{ Code(div); }
	| expr '^' expr		{ Code(power); }
	| '-' expr %prec UNARYMINUS	{ Code(negate); }
	;
%%
	/* end of grammar */

#include <stdio.h>

void yyerror(char *s)	/* called for yacc syntax error */
{
	fprintf(stderr, "%s\n", s);
}





