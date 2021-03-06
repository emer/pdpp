#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef struct {
  cssElPlusIVal el_ival;
  cssElPtr     	el;		/* for coding */
  int		ival;		/* for program indexes (progdx) and other ints */
  char*        	nm;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	CSS_PP_INCLUDE	257
# define	CSS_PP_DEFINE	258
# define	CSS_PP_UNDEF	259
# define	CSS_PP_IFDEF	260
# define	CSS_PP_IFNDEF	261
# define	CSS_PP_ELSE	262
# define	CSS_PP_ENDIF	263
# define	CSS_NUMBER	264
# define	CSS_STRING	265
# define	CSS_VAR	266
# define	CSS_FUN	267
# define	CSS_PTR	268
# define	CSS_PP_DEF	269
# define	CSS_NAME	270
# define	CSS_COMMENT	271
# define	CSS_TYPE	272
# define	CSS_PTRTYPE	273
# define	CSS_CLASS	274
# define	CSS_ENUM	275
# define	CSS_SCPTYPE	276
# define	CSS_WHILE	277
# define	CSS_DO	278
# define	CSS_IF	279
# define	CSS_ELSE	280
# define	CSS_SWITCH	281
# define	CSS_CASE	282
# define	CSS_DEFAULT	283
# define	CSS_RETURN	284
# define	CSS_BREAK	285
# define	CSS_CONTINUE	286
# define	CSS_FOR	287
# define	CSS_NEW	288
# define	CSS_DELETE	289
# define	CSS_COMMAND	290
# define	CSS_LIST	291
# define	CSS_EXIT	292
# define	CSS_ALIAS	293
# define	CSS_REMOVE	294
# define	CSS_STATUS	295
# define	CSS_TYPECMD	296
# define	CSS_CONT	297
# define	CSS_HELP	298
# define	CSS_EXTERN	299
# define	CSS_STATIC	300
# define	CSS_CONST	301
# define	CSS_PRIVATE	302
# define	CSS_PUBLIC	303
# define	CSS_PROTECTED	304
# define	CSS_VIRTUAL	305
# define	CSS_INLINE	306
# define	CSS_ASGN_ADD	307
# define	CSS_ASGN_SUB	308
# define	CSS_ASGN_MULT	309
# define	CSS_ASGN_DIV	310
# define	CSS_ASGN_MOD	311
# define	CSS_ASGN_LSHIFT	312
# define	CSS_ASGN_RSHIFT	313
# define	CSS_ASGN_AND	314
# define	CSS_ASGN_XOR	315
# define	CSS_ASGN_OR	316
# define	CSS_OR	317
# define	CSS_AND	318
# define	CSS_GT	319
# define	CSS_GE	320
# define	CSS_LT	321
# define	CSS_LE	322
# define	CSS_EQ	323
# define	CSS_NE	324
# define	CSS_LSHIFT	325
# define	CSS_RSHIFT	326
# define	CSS_UNARY	327
# define	CSS_PLUSPLUS	328
# define	CSS_MINMIN	329
# define	CSS_UNARYMINUS	330
# define	CSS_NOT	331
# define	CSS_POINTSAT	332
# define	CSS_SCOPER	333


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
