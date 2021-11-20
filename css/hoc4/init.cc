
#include <math.h>
#include "code.h"
#include "y.tab.h"

extern double	Log(double x), Log10(double x), Exp(double x);
extern double 	Sqrt(double x), integer(double x), Pow(double x, double y);

static struct {	/* Constants */
	char	*name;
	double cval;
} consts[] = {
	"PI",	3.14159265358979323846,
	"E",	2.71828182845904523536,
	"GAMMA",0.57721566490153286060,	/* Euler */
	"DEG", 57.29577951308232087680,	/* deg/radian */
	"PHI",	1.61803398874989484820,	/* golden ratio */
	"FARADAY", 96484.56,	/*coulombs/mole*/
	"R", 8.31441,		/*molar gas constant, joules/mole/deg-K*/
	0,	0
};

static struct {	/* Built-ins */
	char	*name;
	double	(*func)(...);
} builtins[] = {
	"sin",	sin,
	"cos",	cos,
	"atan",	atan,
	"log",	Log,	/* checks argument */
	"log10",Log10,	/* checks argument */
	"exp",	Exp,	/* checks argument */
	"sqrt",	Sqrt,	/* checks argument */
	"int",	integer,
	"abs",	fabs,
	(char *) 0, (double (*)(...)) 0
};


void init(void)	/* install constants and built-ins table */
{
	int i;
	Symbol *s;

	for (i = 0; consts[i].name; i++)
		install(consts[i].name, VAR, consts[i].cval);
	for (i = 0; builtins[i].name; i++)
	{
		s = install(builtins[i].name, BLTIN, 0.0);
		s->u.ptr = builtins[i].func;
	}
}
