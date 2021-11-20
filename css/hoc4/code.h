/* antiquated hoc version of the machine */

typedef struct Symbol { /* symbol table entry */
  char	*name;
  short	type;
  union {
    double val;
    double (*ptr)();
  } u;
  struct Symbol *next;
} Symbol;

Symbol *install(char *s, int t, double d); /* install s in the list symbol table */
Symbol *lookup(char *s);		   /* find s in symbol table */

typedef union Datum {
  double val;
  Symbol *sym;
} Datum;

Datum pop(void);

int yylex(void);

typedef int (*Inst)(...);
#define STOP (Inst) 0

Inst *code(Inst f);
void yyerror(char *s);

Inst prog[];
void eval(void), add(void), sub(void),  mul(void);
void div(void), negate(void), power(void);
void assign(void), bltin(void), varpush(void);
void constpush(void), print(void);

void init(void);
void initcode(void);
int yyparse(void);
void execute(Inst *p);
void execerror(char *s, char *t);



