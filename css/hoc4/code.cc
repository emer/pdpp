
#include "code.h"
#include "y.tab.h"
#include <stdio.h>

#define NSTACK 256
static Datum stack[NSTACK];
static Datum *stackp;

#define NPROG 2000
Inst prog[NPROG];
Inst *progp;
Inst *pc;

void initcode(void)
{

  stackp = stack;
  progp = prog;
}

void push(Datum d)
{
  if(stackp >= &stack[NSTACK])
    execerror("stack overflow", (char *) 0);
  *stackp++ = d;
}

Datum pop()
{
  if(stackp <= stack)
    execerror("stack underflow", (char *) 0);
  return *--stackp;
}

Inst *code(Inst f)
{
  Inst *oprogp = progp;
  if(progp >= &prog[NPROG])
    execerror("program too big", (char *) 0);
  *progp++ = f;
  return oprogp;
}

void execute(Inst *p)
{
  for(pc = p; *pc != STOP; )
    (*(*pc++))();
}

void constpush(void)
{
  Datum d;

  d.val = ((Symbol *)*pc++)->u.val;
  push(d);
}

void varpush(void)
{
  Datum d;

  d.sym = (Symbol *)(*pc++);
  push(d);
}

void add(void)
{
  Datum d1, d2;

  d2 = pop();
  d1 = pop();
  d1.val += d2.val;
  push(d1);
}

void sub(void)
{
  Datum d1, d2;

  d2 = pop();
  d1 = pop();
  d1.val -= d2.val;
  push(d1);
}

void mul(void)
{
  Datum d1, d2;

  d2 = pop();
  d1 = pop();
  d1.val *= d2.val;
  push(d1);
}

void div(void)
{
  Datum d1, d2;

  d2 = pop();
  d1 = pop();
  d1.val /= d2.val;
  push(d1);
}

void power(void)
{
  Datum d1, d2;
  extern double Pow(double x1, double x2);

  d2 = pop();
  d1 = pop();
  d1.val = Pow(d1.val, d2.val);
  push(d1);
}

void negate(void)
{
  Datum d1;

  d1 = pop();
  d1.val = -d1.val;
  push(d1);
}

void eval(void)
{
  Datum d;
  
  d = pop();
  if(d.sym->type == UNDEF)
    execerror("undefined variable", d.sym->name);
  d.val = d.sym->u.val;
  push(d);
}

void assign(void)
{
  Datum d1, d2;

  d1= pop();
  d2 = pop();
  if(d1.sym->type != VAR && d1.sym->type != UNDEF)
    execerror("Assignment to non-variable", d1.sym->name);
  d1.sym->u.val = d2.val;
  d1.sym->type = VAR;
  push(d2);
}

void print(void)
{
  Datum d;

  d=pop();
  printf("\t%.8g\n", d.val);
}

void bltin(void)
{
  Datum d;
  d = pop();
  d.val = (*(double (*)(...))*pc++)(d.val);
  push(d);
}
