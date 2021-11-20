#include "code.h"
#include "y.tab.h"

static Symbol *symlist = 0;


Symbol *lookup(char *s)	/* find s in symbol table */
{
  Symbol *sp;

  for(sp = symlist; sp != (Symbol *) 0; sp = sp->next)
    if(strcmp(sp->name, s) == 0)
      return sp;
  return 0;
}

Symbol *install(char *s, int t, double d)	/* install s in the list symbol table */
{
  Symbol *sp;

  sp = (Symbol *) malloc(sizeof(Symbol));
  sp->name = (char *) malloc((unsigned)(strlen(s)+1));	/* +1 for '\0' */
  strcpy(sp->name, s);
  sp->type = t;
  sp->u.val = d;
  sp->next = symlist;
  symlist = sp;
  return sp;
}


