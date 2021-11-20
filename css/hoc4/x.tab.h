typedef union {				/* stack type */
	Symbol	*sym;			/* symbol table pointer */
	Inst	*inst;			/* machine instruction */
} YYSTYPE;
#define	NUMBER	258
#define	VAR	259
#define	BLTIN	260
#define	UNDEF	261
#define	UNARYMINUS	262


extern YYSTYPE yylval;
