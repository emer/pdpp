// SSL: Super-Script Language

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf begin;

#include "code.h"
#include "y.tab.h"

FILE *fin;			/* input file to parser */
int lineno;

int yylex(void)
{
  char c;

  while ((c=getc(fin)) == ' ' || c == '\t');

  if(c == EOF)
    return 0;

  if(c == '.' || isdigit(c)) {	// number
    double d;

    ungetc(c, fin);
    fscanf(fin, "%lf", &d);

    yylval.sym = install("", NUMBER, d);

    return NUMBER;
  }

  if(isalpha(c)) {
    Symbol *s;
    char sbuf[100], *p = sbuf;

    do {
      *p++ = c;
    } while((c=getc(fin)) != EOF && isalnum(c));
    ungetc(c, fin);
    *p = '\0';
    if((s=lookup(sbuf)) == 0)
      s = install(sbuf, UNDEF, 0.0);
    yylval.sym = s;
    return s->type == UNDEF ? VAR : s->type;
  }
    
  if(c == '\n')
    lineno++;
  
  return c;
}

void fpecatch(void)
{
  execerror("floating point exception", (char *) 0);
}

void execerror(char *s, char *t)
{
  fprintf(stderr, "%s %s\n", s, t);
  fseek(fin, 0L, 2);
  longjmp(begin, 0);
}

main(int argc, char *argv[])
{
  fin = stdin;
  
  init();
  
  setjmp(begin);
  signal(SIGFPE, fpecatch);
  for (initcode(); yyparse(); initcode())
    execute(prog);
  return 0;
}




