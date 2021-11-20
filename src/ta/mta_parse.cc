/* A Bison parser, made from /usr/local/pdp++/src/maketa/mta_parse.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	CLASS	257
# define	STRUCT	258
# define	UNION	259
# define	ENUM	260
# define	FUNTYPE	261
# define	STATIC	262
# define	TEMPLATE	263
# define	CONST	264
# define	TYPEDEF	265
# define	TA_TYPEDEF	266
# define	TYPE	267
# define	NUMBER	268
# define	FUNCTION	269
# define	ARRAY	270
# define	NAME	271
# define	COMMENT	272
# define	FUNCALL	273
# define	SCOPER	274
# define	EQUALS	275
# define	PUBLIC	276
# define	PRIVATE	277
# define	PROTECTED	278
# define	OPERATOR	279
# define	FRIEND	280
# define	THISNAME	281
# define	REGFUN	282
# define	VIRTUAL	283

#line 41 "/usr/local/pdp++/src/maketa/mta_parse.y"


#include <ta/maketa.h>

#if defined(SUN4) && !defined(__GNUG__) && !defined(SOLARIS)
#include <alloca.h>
#endif

#if defined(SGI) || defined(SGIdebug)
#include <alloca.h>
#endif

static String_PArray bogus_inh_opts;

#define SETDESC(ty,cm)	mta->SetDesc(cm, ty->desc, ty->inh_opts, ty->opts, ty->lists)

#define SETENUMDESC(ty,cm) mta->SetDesc(cm, ty->desc, bogus_inh_opts, ty->opts, ty->lists)

void yyerror(char *s);
int yylex();


#line 67 "/usr/local/pdp++/src/maketa/mta_parse.y"
#ifndef YYSTYPE
typedef struct {
  TypeDef* 	typ;
  EnumDef* 	enm;
  MemberDef* 	memb;
  MethodDef* 	meth;
  char*    	chr;
  int	   	rval;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 1
#endif



#define	YYFINAL		336
#define	YYFLAG		-32768
#define	YYNTBASE	45

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 283 ? yytranslate[x] : 127)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    43,     2,
      31,    32,    30,    39,    36,    40,    42,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    35,    44,
      38,     2,    37,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    34,     2,    33,    41,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     4,     7,    10,    13,    16,    19,    22,
      25,    27,    29,    31,    34,    37,    42,    46,    51,    54,
      62,    67,    69,    74,    77,    81,    85,    88,    90,    92,
      95,    99,   101,   104,   108,   112,   115,   119,   123,   125,
     129,   132,   135,   137,   139,   141,   145,   147,   150,   152,
     155,   157,   160,   162,   164,   166,   169,   173,   177,   180,
     184,   188,   194,   196,   199,   204,   206,   211,   215,   218,
     220,   223,   225,   228,   230,   233,   235,   237,   241,   243,
     247,   251,   253,   255,   257,   259,   262,   264,   266,   268,
     270,   273,   275,   277,   280,   283,   286,   288,   290,   293,
     297,   300,   305,   310,   312,   316,   318,   323,   325,   329,
     334,   339,   344,   350,   356,   361,   365,   370,   375,   381,
     386,   388,   391,   395,   398,   403,   410,   414,   417,   419,
     421,   423,   426,   429,   432,   436,   438,   441,   445,   447,
     449,   452,   455,   459,   461,   464,   466,   470,   475,   477,
     480,   482,   485,   489,   496,   498,   500,   503,   507,   510,
     512,   514,   517,   519,   521,   523,   526,   528,   530,   533,
     535,   538,   540,   543,   545,   548,   551,   555,   559,   563,
     567,   570,   572,   577,   582,   584,   587,   589,   593,   595,
     597,   599,   602,   604,   606,   608,   611,   613,   615,   617,
     619,   621,   623
};
static const short yyrhs[] =
{
      -1,    45,    47,     0,    45,    51,     0,    45,    55,     0,
      45,    57,     0,    45,    67,     0,    45,    74,     0,    45,
      46,     0,    45,     1,     0,    12,     0,    48,     0,    49,
       0,    49,    18,     0,    11,    50,     0,    11,    59,   119,
     123,     0,   113,   112,   123,     0,   113,    18,   112,   123,
       0,   113,   123,     0,   113,    31,    30,   112,    32,   100,
     123,     0,   113,    20,    30,   112,     0,    52,     0,    53,
      77,    33,   123,     0,    54,    34,     0,    54,    18,    34,
       0,    54,    34,    18,     0,     6,   112,     0,     6,     0,
      56,     0,    62,   123,     0,    62,   123,    18,     0,    58,
       0,    59,   123,     0,    59,   122,   123,     0,    60,    84,
      33,     0,    61,    34,     0,    61,    18,    34,     0,    61,
      34,    18,     0,    62,     0,    62,    35,    63,     0,   126,
     112,     0,   126,    13,     0,   126,     0,   125,     0,    64,
       0,    63,    36,    64,     0,   113,     0,    65,   113,     0,
      17,     0,    65,    17,     0,    66,     0,    65,    66,     0,
     124,     0,    29,     0,    68,     0,    69,   123,     0,    69,
     122,   123,     0,    70,    84,    33,     0,    71,    34,     0,
      71,    18,    34,     0,    71,    34,    18,     0,     9,    72,
      73,    37,    61,     0,    38,     0,     3,   112,     0,    73,
      36,     3,   112,     0,    75,     0,    28,   111,    76,    97,
       0,    28,    76,    97,     0,    96,   100,     0,    78,     0,
      77,    78,     0,    79,     0,    79,    18,     0,    80,     0,
      80,    36,     0,     1,     0,    83,     0,    83,    21,    81,
       0,    82,     0,    81,    39,    82,     0,    81,    40,    82,
       0,    17,     0,    14,     0,    17,     0,    85,     0,    84,
      85,     0,    86,     0,    92,     0,    52,     0,    49,     0,
      59,   123,     0,     1,     0,    87,     0,    22,    35,     0,
      23,    35,     0,    24,    35,     0,    18,     0,    88,     0,
       8,    88,     0,   110,    89,   123,     0,    89,   123,     0,
     110,    90,    16,   123,     0,   110,    91,   100,   123,     0,
      90,     0,    89,    36,    90,     0,    17,     0,    31,    30,
      17,    32,     0,    93,     0,   107,   101,    97,     0,   120,
     107,   101,    97,     0,   107,   101,   105,    97,     0,    41,
     107,   101,    97,     0,   120,    41,   107,   101,    97,     0,
      26,   111,    25,   100,    97,     0,    26,    25,   100,    97,
       0,    26,   111,   123,     0,    26,     3,   111,   123,     0,
      26,     3,    17,   123,     0,    26,   111,    96,   100,    97,
       0,    26,    96,   100,    97,     0,    94,     0,     8,    94,
       0,   110,    95,    97,     0,    95,    97,     0,   110,    25,
     100,    97,     0,   110,    25,    31,    32,   100,    97,     0,
      25,   100,    97,     0,    96,   100,     0,    17,     0,   123,
       0,    98,     0,    99,   123,     0,    18,   123,     0,    18,
      98,     0,    18,    99,   123,     0,    15,     0,    10,    15,
       0,    10,    18,    15,     0,    21,     0,    10,     0,    10,
      21,     0,    31,    32,     0,    31,   102,    32,     0,    32,
       0,   102,    32,     0,   103,     0,   102,    36,   103,     0,
     102,    42,    42,    42,     0,   104,     0,   104,    21,     0,
     113,     0,   113,    17,     0,   113,    17,    16,     0,   113,
      31,    30,    17,    32,   100,     0,    17,     0,   106,     0,
     105,   106,     0,   108,    13,    19,     0,    27,    31,     0,
      35,     0,    30,     0,   109,    30,     0,   111,     0,   113,
       0,   120,     0,   120,   113,     0,    17,     0,   114,     0,
     114,    43,     0,   115,     0,   115,   109,     0,   116,     0,
      10,   116,     0,   117,     0,   125,   117,     0,   125,   112,
       0,    13,    20,    17,     0,    27,    20,    17,     0,    13,
      20,    13,     0,    27,    20,    13,     0,    20,    13,     0,
      27,     0,    13,    72,   118,    37,     0,    27,    72,   118,
      37,     0,    13,     0,   117,    13,     0,    13,     0,   118,
      36,    13,     0,    17,     0,    13,     0,   121,     0,   120,
     121,     0,     7,     0,    29,     0,    17,     0,    17,    16,
       0,    44,     0,    22,     0,    23,     0,    24,     0,     4,
       0,     5,     0,     3,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   136,   137,   140,   142,   145,   148,   150,   152,   154,
     158,   162,   169,   171,   175,   176,   180,   185,   192,   222,
     225,   230,   236,   239,   240,   241,   244,   248,   254,   262,
     264,   267,   275,   277,   280,   286,   290,   292,   296,   298,
     301,   306,   310,   316,   325,   328,   336,   337,   338,   339,
     342,   343,   346,   347,   350,   358,   360,   363,   369,   373,
     375,   379,   387,   390,   392,   395,   401,   403,   407,   412,
     413,   416,   417,   420,   421,   422,   427,   430,   435,   437,
     442,   449,   451,   454,   458,   459,   462,   471,   484,   487,
     490,   494,   497,   499,   500,   501,   502,   521,   523,   528,
     530,   531,   539,   542,   547,   553,   559,   564,   565,   566,
     567,   568,   569,   570,   571,   572,   573,   574,   575,   583,
     593,   595,   598,   600,   601,   602,   603,   606,   613,   617,
     618,   619,   620,   621,   622,   625,   627,   628,   631,   633,
     634,   637,   638,   641,   642,   645,   646,   647,   650,   652,
     659,   664,   668,   672,   676,   682,   684,   687,   690,   695,
     698,   699,   702,   705,   706,   707,   710,   713,   714,   723,
     725,   733,   734,   743,   744,   745,   746,   750,   754,   755,
     756,   757,   758,   775,   781,   782,   791,   793,   796,   797,
     800,   801,   804,   805,   808,   809,   812,   815,   816,   817,
     820,   822,   825
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "CLASS", "STRUCT", "UNION", "ENUM", 
  "FUNTYPE", "STATIC", "TEMPLATE", "CONST", "TYPEDEF", "TA_TYPEDEF", 
  "TYPE", "NUMBER", "FUNCTION", "ARRAY", "NAME", "COMMENT", "FUNCALL", 
  "SCOPER", "EQUALS", "PUBLIC", "PRIVATE", "PROTECTED", "OPERATOR", 
  "FRIEND", "THISNAME", "REGFUN", "VIRTUAL", "'*'", "'('", "')'", "'}'", 
  "'{'", "':'", "','", "'>'", "'<'", "'+'", "'-'", "'~'", "'.'", "'&'", 
  "';'", "list", "preparsed", "typedefn", "typedefns", "typedsub", "defn", 
  "enumdefn", "enumdsub", "enumname", "enumnm", "classdecl", "classdecls", 
  "classdefn", "classdefns", "classdsub", "classname", "classhead", 
  "classnm", "classinh", "classpar", "classptyp", "classpmod", 
  "templdefn", "templdefns", "templdsub", "templname", "templhead", 
  "templopen", "templpars", "fundecl", "funnm", "regfundefn", "enums", 
  "enumline", "enumitm", "enumitms", "enummath", "enummathitm", 
  "enmitmname", "membs", "membline", "membdefn", "basicmemb", 
  "nostatmemb", "membnames", "membname", "membfunp", "methdefn", 
  "basicmeth", "nostatmeth", "mbfundefn", "methname", "fundefn", 
  "funsubdefn", "funsubdecl", "funargs", "constfun", "args", "argdefn", 
  "subargdefn", "constrlist", "constref", "consthsnm", "constcoln", 
  "ptrs", "membtype", "ftype", "tyname", "type", "noreftype", "constype", 
  "subtype", "combtype", "templtypes", "tdname", "funtspec", "funtsmod", 
  "varname", "term", "access", "structstruct", "classkeyword", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      46,    47,    48,    48,    49,    49,    50,    50,    50,    50,
      50,    51,    52,    53,    53,    53,    54,    54,    55,    56,
      56,    57,    58,    58,    59,    60,    60,    60,    61,    61,
      62,    62,    62,    62,    63,    63,    64,    64,    64,    64,
      65,    65,    66,    66,    67,    68,    68,    69,    70,    70,
      70,    71,    72,    73,    73,    74,    75,    75,    76,    77,
      77,    78,    78,    79,    79,    79,    80,    80,    81,    81,
      81,    82,    82,    83,    84,    84,    85,    85,    85,    85,
      85,    85,    86,    86,    86,    86,    86,    87,    87,    88,
      88,    88,    88,    89,    89,    90,    91,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      93,    93,    94,    94,    94,    94,    94,    95,    96,    97,
      97,    97,    97,    97,    97,    98,    98,    98,    99,    99,
      99,   100,   100,   101,   101,   102,   102,   102,   103,   103,
     104,   104,   104,   104,   104,   105,   105,   106,   107,   108,
     109,   109,   110,   111,   111,   111,   112,   113,   113,   114,
     114,   115,   115,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   124,   124,   124,
     125,   125,   126
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     2,     2,     4,     3,     4,     2,     7,
       4,     1,     4,     2,     3,     3,     2,     1,     1,     2,
       3,     1,     2,     3,     3,     2,     3,     3,     1,     3,
       2,     2,     1,     1,     1,     3,     1,     2,     1,     2,
       1,     2,     1,     1,     1,     2,     3,     3,     2,     3,
       3,     5,     1,     2,     4,     1,     4,     3,     2,     1,
       2,     1,     2,     1,     2,     1,     1,     3,     1,     3,
       3,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       2,     1,     1,     2,     2,     2,     1,     1,     2,     3,
       2,     4,     4,     1,     3,     1,     4,     1,     3,     4,
       4,     4,     5,     5,     4,     3,     4,     4,     5,     4,
       1,     2,     3,     2,     4,     6,     3,     2,     1,     1,
       1,     2,     2,     2,     3,     1,     2,     3,     1,     1,
       2,     2,     3,     1,     2,     1,     3,     4,     1,     2,
       1,     2,     3,     6,     1,     1,     2,     3,     2,     1,
       1,     2,     1,     1,     1,     2,     1,     1,     2,     1,
       2,     1,     2,     1,     2,     2,     3,     3,     3,     3,
       2,     1,     4,     4,     1,     2,     1,     3,     1,     1,
       1,     2,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     9,   202,   200,   201,    27,     0,     0,    10,
       0,     8,     2,    11,    12,     3,    21,     0,     0,     4,
      28,     5,    31,     0,     0,     0,    38,     6,    54,     0,
       0,     0,     7,    65,    43,    42,   166,    26,    62,     0,
       0,   184,     0,   181,    14,     0,    38,     0,   167,   169,
     171,   173,    43,   192,   128,   193,     0,     0,     0,   163,
     164,   190,     0,    13,    75,    83,     0,    69,    71,    73,
      76,     0,    23,   194,   196,     0,    32,    91,     0,   105,
      96,     0,     0,     0,     0,     0,   181,     0,    89,    88,
       0,     0,    84,    86,    92,    97,     0,   103,    87,   107,
     120,     0,     0,     0,     0,   162,   164,     0,    35,     0,
      29,     0,    55,     0,     0,    58,    41,    40,     0,     0,
     172,     0,     0,   180,     0,     0,   189,   188,     0,     0,
       0,     0,     0,    18,   168,   160,   170,   185,   184,   175,
     174,   139,   135,     0,   138,    67,   130,     0,   129,     0,
      68,     0,   165,   191,     0,    70,    72,    74,     0,    24,
      25,   195,    33,    98,   121,    93,    94,    95,     0,     0,
       0,     0,     0,   158,     0,     0,    90,    34,    85,     0,
     100,   123,   127,   154,   143,     0,     0,   145,   148,   150,
       0,     0,     0,   103,     0,     0,     0,     0,    36,    37,
      48,   197,   198,   199,    53,    39,    44,     0,    50,    46,
      52,    30,    56,    57,    59,    60,    63,     0,     0,   178,
     176,   186,     0,   179,   177,     0,    15,     0,     0,     0,
      16,   161,   136,     0,   140,   133,     0,   132,   131,   141,
       0,    66,    22,    82,    81,    77,    78,   126,     0,     0,
       0,     0,     0,     0,   115,     0,   105,   104,   159,   108,
       0,   155,     0,   144,     0,     0,   149,   151,     0,     0,
       0,     0,    99,     0,     0,   122,     0,     0,     0,    49,
      51,    47,     0,    61,     0,   182,   183,    17,    20,     0,
     137,   134,   142,     0,     0,   117,   116,   114,   119,     0,
       0,   111,   110,   156,     0,   146,     0,   152,     0,   141,
     124,     0,   101,   102,     0,   109,    45,    64,   187,     0,
      79,    80,   113,   118,   157,   147,     0,     0,   106,   112,
       0,     0,   125,    19,   153,     0,     0
};

static const short yydefgoto[] =
{
       1,    11,    12,    13,    88,    44,    15,    89,    17,    18,
      19,    20,    21,    22,    90,    24,    25,    46,   205,   206,
     207,   208,    27,    28,    29,    30,    31,   125,   119,    32,
      33,    56,    66,    67,    68,    69,   245,   246,    70,    91,
      92,    93,    94,    95,    96,    97,   194,    98,    99,   100,
     101,   102,   145,   146,   147,   150,   185,   186,   187,   188,
     260,   261,   103,   262,   136,   104,   105,   139,    59,    48,
      49,    50,    51,   222,   128,    60,    61,    75,   148,   210,
      62,    35
};

static const short yypact[] =
{
  -32768,   235,-32768,-32768,-32768,-32768,     1,    42,   463,-32768,
     416,-32768,-32768,-32768,   112,-32768,-32768,    38,    53,-32768,
  -32768,-32768,-32768,    26,   133,    88,    19,-32768,-32768,    26,
     133,    89,-32768,-32768,-32768,   135,-32768,-32768,-32768,    34,
     475,     3,    45,     9,-32768,   154,    97,    72,   102,   117,
  -32768,   141,   182,-32768,-32768,-32768,    54,   130,   171,-32768,
     445,-32768,   182,-32768,-32768,-32768,    16,-32768,   189,   184,
     191,   200,   205,   202,-32768,   193,-32768,-32768,   405,   214,
  -32768,   219,   231,   232,   130,   352,    95,   229,-32768,-32768,
     193,   269,-32768,-32768,-32768,-32768,    40,-32768,-32768,-32768,
  -32768,    54,   130,   174,   180,-32768,    15,   234,   260,   363,
     265,   193,-32768,   308,   250,   267,-32768,-32768,     1,   136,
  -32768,   185,   275,-32768,   204,   275,-32768,-32768,   193,     1,
     270,   271,   193,-32768,-32768,-32768,   273,-32768,-32768,-32768,
     141,   201,-32768,    47,-32768,-32768,-32768,   193,-32768,   176,
  -32768,    54,-32768,-32768,   193,-32768,-32768,-32768,    64,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    54,   434,
     130,   130,    68,-32768,   259,   174,-32768,-32768,-32768,   280,
  -32768,-32768,-32768,-32768,-32768,    30,   216,-32768,   278,    96,
     274,   276,    40,   288,   130,    54,   229,   174,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,   272,-32768,   384,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,   314,   227,-32768,
  -32768,-32768,   146,-32768,-32768,   173,-32768,   193,     1,     1,
  -32768,-32768,-32768,   305,-32768,-32768,   193,-32768,-32768,-32768,
     217,-32768,-32768,-32768,-32768,   221,-32768,-32768,   193,   193,
      54,    54,   130,   130,-32768,    54,-32768,-32768,-32768,-32768,
      30,-32768,   309,-32768,   465,   281,-32768,   311,   294,   334,
      54,   312,-32768,   193,   193,-32768,   174,    54,   363,-32768,
  -32768,-32768,     1,-32768,   323,-32768,-32768,-32768,-32768,   310,
  -32768,-32768,-32768,    64,    64,-32768,-32768,-32768,-32768,    54,
      54,-32768,-32768,-32768,   321,-32768,   301,-32768,   328,   130,
  -32768,   316,-32768,-32768,    54,-32768,-32768,-32768,-32768,   130,
  -32768,-32768,-32768,-32768,-32768,-32768,   318,    54,-32768,-32768,
     193,   130,-32768,-32768,-32768,   346,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,-32768,-32768,   351,-32768,-32768,   357,-32768,-32768,
  -32768,-32768,-32768,-32768,   100,-32768,   142,   362,-32768,    75,
  -32768,   157,-32768,-32768,-32768,-32768,-32768,    20,-32768,-32768,
  -32768,   313,-32768,   304,-32768,-32768,-32768,   -29,-32768,   344,
     -53,-32768,-32768,   297,   287,   -93,-32768,-32768,-32768,   300,
     289,    -8,   -85,   239,   241,   -76,  -165,  -140,   131,-32768,
  -32768,   138,   -75,-32768,-32768,-32768,    -6,    -1,    -7,-32768,
  -32768,   356,    73,   277,-32768,    -9,   -47,   370,   -23,-32768,
       6,-32768
};


#define	YYLAST		502


static const short yytable[] =
{
      76,    47,    57,   110,    58,    37,   112,    34,   168,   240,
     255,   193,   175,   153,    52,   106,   181,    64,    36,     4,
       5,   106,    53,   121,   133,    40,   182,    39,    41,   124,
      52,   197,   277,    65,   117,    42,    52,   118,   178,    64,
     141,    38,    86,    73,    55,   142,   132,    38,   143,   154,
      57,   144,   162,   152,   109,    65,   196,   141,   123,   153,
     178,   122,   142,    74,   141,   258,   241,   176,   144,   142,
      74,    71,   143,   180,    74,   144,   179,   171,   243,   172,
      38,   244,   106,   247,    74,    54,   257,    72,   212,    36,
     129,    74,   130,   252,   250,   251,   189,    52,    74,   152,
     259,    23,   209,   131,   106,   226,   107,   114,    45,   230,
     275,   314,    74,   267,   270,   124,    74,   216,   274,    52,
     237,   276,   108,   115,   238,   140,   173,   268,   227,   240,
      63,   242,   109,    38,    77,   140,     3,     4,     5,     6,
      53,    78,   189,    40,     8,   134,    41,   135,   116,   254,
      79,    80,    36,    42,   137,    81,    82,    83,    84,    85,
      86,   149,    55,   249,   253,   297,   298,   126,   189,   272,
     301,   127,   217,   218,    87,   302,   299,   300,     4,     5,
       4,     5,   284,   285,    40,   310,    40,    41,    54,    41,
     189,   183,   315,   183,    42,   138,    42,    79,   219,    36,
     281,    43,   220,    43,   287,   190,   184,   156,   239,   284,
     286,   191,   158,   291,   322,   323,   232,   223,   161,   233,
     157,   224,   234,   160,    34,   295,   296,   288,   289,   329,
       3,     4,     5,   327,   159,   335,     2,    74,     3,     4,
       5,     6,   332,   330,     7,  -128,     8,     9,   263,   292,
     312,   313,   264,   264,   165,   334,   174,   189,   265,   265,
     293,   294,   189,    10,   320,   321,   166,   167,   198,   189,
      77,   209,     3,     4,     5,     6,    53,    78,   199,    40,
       8,   317,    41,   211,   214,   215,    79,    80,   221,    42,
     173,    81,    82,    83,    84,    85,    86,   256,    55,   266,
     228,   229,   177,   231,   273,   269,   271,   333,   278,    77,
      87,     3,     4,     5,     6,    53,    78,   282,    40,     8,
     290,    41,   304,   306,   308,    79,    80,   307,    42,   311,
      81,    82,    83,    84,    85,    86,   318,    55,     4,     5,
     324,   213,   319,   325,    40,   326,   336,    41,   328,    87,
     331,   183,    14,   316,    42,   169,     4,     5,    16,    53,
     283,    43,    40,    26,   280,    41,   309,     4,     5,    54,
     155,   151,    42,    40,   113,   163,    41,   170,   164,    43,
     200,    55,   235,    42,   236,   201,   202,   203,     4,     5,
      43,   192,   204,   195,    40,   305,   120,    41,   303,   111,
       0,   279,   225,     0,    42,     0,   201,   202,   203,     4,
       5,    43,    53,   204,     0,    40,     0,     0,    41,     0,
       4,     5,    79,    53,     0,    42,    40,     0,     0,    41,
      84,     0,    43,    54,    55,     0,    42,     0,     4,     5,
       0,    53,     0,    43,    40,    55,     0,    41,     0,     4,
       5,   248,    53,     0,    42,    40,     0,     0,    41,     0,
       0,    43,     0,    55,     0,    42,     3,     4,     5,     4,
       5,     0,    43,    40,    55,    40,    41,     0,    41,     4,
       5,     0,   183,    42,     0,    42,     0,     0,    41,     0,
      43,     0,    43,     0,     0,    42,     0,     0,     0,     0,
       0,     0,    43
};

static const short yycheck[] =
{
      23,     8,    10,    26,    10,     6,    29,     1,    84,   149,
     175,   104,    87,    60,     8,    24,   101,     1,    17,     4,
       5,    30,     7,    20,    47,    10,   102,     7,    13,    20,
      24,   106,   197,    17,    35,    20,    30,     3,    91,     1,
      10,    38,    27,    17,    29,    15,    47,    38,    18,    33,
      58,    21,    75,    60,    35,    17,    41,    10,    13,   106,
     113,    41,    15,    44,    10,    35,   151,    90,    21,    15,
      44,    18,    18,    96,    44,    21,    36,    85,    14,    85,
      38,    17,    91,   168,    44,    17,   179,    34,   111,    17,
      18,    44,    20,    25,   170,   171,   103,    91,    44,   106,
     185,     1,   109,    31,   113,   128,    18,    18,     8,   132,
     195,   276,    44,    17,   190,    20,    44,   118,   194,   113,
     143,   196,    34,    34,   147,    52,    31,    31,   129,   269,
      18,   154,    35,    38,     1,    62,     3,     4,     5,     6,
       7,     8,   149,    10,    11,    43,    13,    30,    13,   172,
      17,    18,    17,    20,    13,    22,    23,    24,    25,    26,
      27,    31,    29,   169,   172,   250,   251,    13,   175,   192,
     255,    17,    36,    37,    41,   260,   252,   253,     4,     5,
       4,     5,    36,    37,    10,   270,    10,    13,    17,    13,
     197,    17,   277,    17,    20,    13,    20,    17,    13,    17,
     207,    27,    17,    27,   227,    25,    32,    18,    32,    36,
      37,    31,    21,   236,   299,   300,    15,    13,    16,    18,
      36,    17,    21,    18,   218,   248,   249,   228,   229,   314,
       3,     4,     5,   309,    34,     0,     1,    44,     3,     4,
       5,     6,   327,   319,     9,    31,    11,    12,    32,    32,
     273,   274,    36,    36,    35,   331,    27,   264,    42,    42,
      39,    40,   269,    28,   293,   294,    35,    35,    34,   276,
       1,   278,     3,     4,     5,     6,     7,     8,    18,    10,
      11,   282,    13,    18,    34,    18,    17,    18,    13,    20,
      31,    22,    23,    24,    25,    26,    27,    17,    29,    21,
      30,    30,    33,    30,    16,    31,    30,   330,    36,     1,
      41,     3,     4,     5,     6,     7,     8,     3,    10,    11,
      15,    13,    13,    42,    30,    17,    18,    16,    20,    17,
      22,    23,    24,    25,    26,    27,    13,    29,     4,     5,
      19,    33,    32,    42,    10,    17,     0,    13,    32,    41,
      32,    17,     1,   278,    20,     3,     4,     5,     1,     7,
     218,    27,    10,     1,   207,    13,    32,     4,     5,    17,
      66,    58,    20,    10,    30,    78,    13,    25,    78,    27,
      17,    29,   143,    20,   143,    22,    23,    24,     4,     5,
      27,   104,    29,   104,    10,   264,    40,    13,   260,    29,
      -1,    17,   125,    -1,    20,    -1,    22,    23,    24,     4,
       5,    27,     7,    29,    -1,    10,    -1,    -1,    13,    -1,
       4,     5,    17,     7,    -1,    20,    10,    -1,    -1,    13,
      25,    -1,    27,    17,    29,    -1,    20,    -1,     4,     5,
      -1,     7,    -1,    27,    10,    29,    -1,    13,    -1,     4,
       5,    17,     7,    -1,    20,    10,    -1,    -1,    13,    -1,
      -1,    27,    -1,    29,    -1,    20,     3,     4,     5,     4,
       5,    -1,    27,    10,    29,    10,    13,    -1,    13,     4,
       5,    -1,    17,    20,    -1,    20,    -1,    -1,    13,    -1,
      27,    -1,    27,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    27
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
struct yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (struct yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	struct yyalloc *yyptr =
	  (struct yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 136 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->yy_state = MTA::YYRet_Exit; }
    break;
case 2:
#line 137 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_Ok; 
	    return mta->yy_state; }
    break;
case 3:
#line 140 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 4:
#line 142 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ 
	    mta->cur_class = NULL; mta->state = MTA::Find_Item;
	    mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 5:
#line 145 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->cur_class = NULL; mta->state = MTA::Find_Item;
	    mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 6:
#line 148 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 7:
#line 150 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 8:
#line 152 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_Ok; return mta->yy_state; }
    break;
case 9:
#line 154 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->state = MTA::Find_Item; mta->yy_state = MTA::YYRet_NoSrc; return mta->yy_state; }
    break;
case 10:
#line 159 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ }
    break;
case 11:
#line 162 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
  	  if(yyvsp[0].typ != NULL) {
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(yyvsp[0].typ);
	    if(yyval.typ == yyvsp[0].typ) mta->TypeAdded("typedef", sp, yyval.typ); } }
    break;
case 12:
#line 170 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->Burp(); }
    break;
case 13:
#line 171 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    if(yyvsp[-1].typ != NULL)  SETDESC(yyvsp[-1].typ,yyvsp[0].chr); }
    break;
case 14:
#line 175 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 15:
#line 176 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.typ = yyvsp[-2].typ; yyvsp[-2].typ->name = yyvsp[-1].chr; mta->type_stack.Pop(); }
    break;
case 16:
#line 180 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyval.typ = yyvsp[-1].typ; yyvsp[-1].typ->AddParent(yyvsp[-2].typ); yyvsp[-1].typ->ptr = yyvsp[-2].typ->ptr;
	    yyvsp[-1].typ->par_formal.BorrowUnique(yyvsp[-2].typ->par_formal);
	    yyvsp[-1].typ->par_cache.BorrowUnique(yyvsp[-2].typ->par_cache);
	    mta->type_stack.Pop(); }
    break;
case 17:
#line 185 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* annoying place for a comment, but.. */
            yyval.typ = yyvsp[-1].typ; yyvsp[-1].typ->AddParent(yyvsp[-3].typ); yyvsp[-1].typ->ptr = yyvsp[-3].typ->ptr;
	    yyvsp[-1].typ->par_formal.BorrowUnique(yyvsp[-3].typ->par_formal);
	    yyvsp[-1].typ->par_cache.BorrowUnique(yyvsp[-3].typ->par_cache);
	    mta->type_stack.Pop(); }
    break;
case 18:
#line 192 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeDef* td = yyvsp[-1].typ->parents[1]; mta->type_stack.Pop();
	    TypeSpace* sp = yyvsp[-1].typ->owner;
	    sp->Remove(yyvsp[-1].typ); /* get rid of new one, cuz it is bogus */
	    /* not on list that it would be placed on now.. */
	    if((td->owner != mta->spc) && (mta->spc->FindName(td->name) == NULL)) {
	      if(mta->verbose >= 3)
		cerr << "transfered: " << td->name << " from: " << td->owner->name
		     << " to: " << mta->spc->name << "\n";
	      mta->spc->Transfer(td); /* now check for parent which is a combo of basic types */
	      if((td->parents.size == 1) && (td->parents[0]->owner != mta->spc) && 
		 (td->parents[0]->parents.size == 2)) {
		/* has one parent that is a combo-type which might be basic */
		TypeDef* par = td->parents[0];
		if((mta->spc_builtin.FindName(par->parents[0]->name) != NULL) && 
		   (mta->spc_builtin.FindName(par->parents[1]->name) != NULL)) {
		  if(mta->verbose >= 3)
		    cerr << "transfered: " << par->name << " from: " << par->owner->name
			 << " to: " << mta->spc->name << "\n";
		  TypeDef* already_there = mta->spc->FindName(par->name);
		  if(already_there == NULL)
		    mta->spc->Transfer(par); /* move parent to this list too */
		  else
		    td->parents.ReplaceLink(0, already_there);
		}
	      }
	    }
	    yyval.typ = td; }
    break;
case 19:
#line 222 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.typ = yyvsp[-3].typ; yyval.typ->AddParent(&TA_void_ptr); yyval.typ->ptr = 1; 
	    mta->type_stack.Pop(); }
    break;
case 20:
#line 225 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.typ = yyvsp[0].typ; yyval.typ->AddParent(&TA_void_ptr); yyval.typ->ptr = 1;
	    mta->type_stack.Pop(); }
    break;
case 21:
#line 230 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
            yyval.typ = sp->AddUniqNameOld(yyvsp[0].typ);
	    if(yyval.typ == yyvsp[0].typ) mta->TypeAdded("enum", sp, yyval.typ); }
    break;
case 24:
#line 240 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ SETDESC(yyvsp[-2].typ,yyvsp[-1].chr); }
    break;
case 25:
#line 241 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ SETDESC(yyvsp[-2].typ,yyvsp[0].chr); }
    break;
case 26:
#line 244 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
  	    yyval.typ = yyvsp[0].typ; 
	    yyvsp[0].typ->AddParFormal(&TA_enum); mta->cur_enum = yyvsp[0].typ;
	    mta->type_stack.Pop(); }
    break;
case 27:
#line 248 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    String nm = "enum_"; nm += (String)mta->anon_no++; nm += "_";
	    yyval.typ = new TypeDef(nm); mta->cur_enum = yyval.typ; 
	    yyval.typ->AddParFormal(&TA_enum); yyval.typ->internal = true; }
    break;
case 28:
#line 255 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(yyvsp[0].typ);
	    if(yyval.typ == yyvsp[0].typ) mta->TypeAdded("class", sp, yyval.typ);
	    mta->type_stack.Pop(); }
    break;
case 29:
#line 263 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->Burp(); }
    break;
case 30:
#line 264 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ SETDESC(yyvsp[-2].typ,yyvsp[0].chr); }
    break;
case 31:
#line 268 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(yyvsp[0].typ);
	    if(yyval.typ == yyvsp[0].typ) mta->TypeAdded("class", sp, yyval.typ);
	    mta->type_stack.Pop(); }
    break;
case 34:
#line 281 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    if(yyvsp[-2].typ->HasOption("NO_TOKENS")) yyvsp[-2].typ->tokens.keep = false;
	    else yyvsp[-2].typ->tokens.keep = true; }
    break;
case 35:
#line 287 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyvsp[-1].typ->tokens.keep = true;
	    mta->Class_ResetCurPtrs(); }
    break;
case 36:
#line 290 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    SETDESC(yyvsp[-2].typ,yyvsp[-1].chr); mta->state = MTA::Parse_inclass; mta->Class_ResetCurPtrs(); }
    break;
case 37:
#line 292 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    SETDESC(yyvsp[-2].typ,yyvsp[0].chr); mta->state = MTA::Parse_inclass; mta->Class_ResetCurPtrs(); }
    break;
case 38:
#line 297 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->state = MTA::Parse_inclass; }
    break;
case 39:
#line 298 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->state = MTA::Parse_inclass; }
    break;
case 40:
#line 301 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->state = MTA::Parse_class;
            yyval.typ = yyvsp[0].typ; mta->last_class = mta->cur_class; mta->cur_class = yyvsp[0].typ;
	    yyvsp[0].typ->AddParFormal(&TA_class);
            mta->cur_mstate = MTA::prvt; }
    break;
case 41:
#line 306 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->state = MTA::Parse_class;
	    yyval.typ = yyvsp[0].typ; mta->last_class = mta->cur_class; mta->cur_class = yyvsp[0].typ;
            mta->cur_mstate = MTA::prvt; }
    break;
case 42:
#line 310 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->state = MTA::Parse_class;
	    String nm = yyvsp[0].typ->name + "_" + (String)mta->anon_no++; nm += "_";
	    yyval.typ = new TypeDef(nm); mta->type_stack.Push(yyval.typ);
	    mta->last_class = mta->cur_class; mta->cur_class = yyval.typ;
            mta->cur_mstate = MTA::prvt; }
    break;
case 43:
#line 316 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->state = MTA::Parse_class;
	    String nm = yyvsp[0].typ->name + "_" + (String)mta->anon_no++; nm += "_";
	    yyval.typ = new TypeDef(nm); mta->type_stack.Push(yyval.typ);
	    mta->last_class = mta->cur_class; mta->cur_class = yyval.typ;
	    mta->cur_mstate = MTA::pblc; }
    break;
case 44:
#line 325 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            if(yyvsp[0].typ->InheritsFrom(TA_taBase)) mta->cur_class->AddParCache(&TA_taBase);
	    mta->cur_class->AddParent(yyvsp[0].typ); }
    break;
case 45:
#line 328 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            if(yyvsp[0].typ->InheritsFrom(&TA_taBase)) mta->cur_class->AddParCache(&TA_taBase);
	    mta->cur_class->AddParent(yyvsp[0].typ);
	    if(!mta->cur_class->HasOption("MULT_INHERIT"))
	      mta->cur_class->opts.Add("MULT_INHERIT"); }
    break;
case 47:
#line 337 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 48:
#line 338 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = &TA_class; }
    break;
case 49:
#line 339 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = &TA_class; }
    break;
case 54:
#line 351 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(yyvsp[0].typ);
	    if(yyval.typ == yyvsp[0].typ) mta->TypeAdded("template", sp, yyval.typ);
	    mta->type_stack.Pop(); }
    break;
case 57:
#line 364 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
          if(yyvsp[-2].typ->HasOption("NO_TOKENS")) yyvsp[-2].typ->tokens.keep = false;
	  else yyvsp[-2].typ->tokens.keep = true; }
    break;
case 58:
#line 370 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->state = MTA::Parse_inclass; yyvsp[-1].typ->tokens.keep = true;
	    mta->Class_ResetCurPtrs(); }
    break;
case 59:
#line 373 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    SETDESC(yyvsp[-2].typ,yyvsp[-1].chr); mta->state = MTA::Parse_inclass; mta->Class_ResetCurPtrs(); }
    break;
case 60:
#line 375 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    SETDESC(yyvsp[-2].typ,yyvsp[0].chr); mta->state = MTA::Parse_inclass; mta->Class_ResetCurPtrs(); }
    break;
case 61:
#line 380 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyvsp[0].typ->templ_pars.Reset();
	    yyvsp[0].typ->templ_pars.Duplicate(mta->cur_templ_pars);
	    yyvsp[0].typ->internal = true;
	    yyvsp[0].typ->AddParFormal(&TA_template); yyval.typ = yyvsp[0].typ; }
    break;
case 62:
#line 387 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_templ_pars.Reset(); }
    break;
case 63:
#line 391 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_templ_pars.Link(yyvsp[0].typ); yyval.typ = yyvsp[0].typ; }
    break;
case 64:
#line 392 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_templ_pars.Link(yyvsp[0].typ); yyval.typ = yyvsp[-3].typ; }
    break;
case 65:
#line 395 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            if(mta->spc == &(mta->spc_target)) /* only add reg_funs in target space */
	       TA_taRegFun.methods.AddUniqNameNew(yyvsp[0].meth);
	    mta->meth_stack.Pop(); }
    break;
case 66:
#line 401 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyvsp[-1].meth->type = yyvsp[-2].typ; SETDESC(yyvsp[-1].meth,yyvsp[0].chr); yyval.meth = yyvsp[-1].meth; }
    break;
case 67:
#line 403 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyvsp[-1].meth->type = &TA_int; SETDESC(yyvsp[-1].meth,yyvsp[0].chr); yyval.meth = yyvsp[-1].meth; }
    break;
case 68:
#line 407 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyvsp[-1].meth->is_static = true; /* consider these to be static functions */
            yyvsp[-1].meth->fun_argc = yyvsp[0].rval; yyvsp[-1].meth->arg_types.size = yyvsp[0].rval; mta->burp_fundefn = true; }
    break;
case 72:
#line 417 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ if(yyvsp[-1].enm != NULL) SETENUMDESC(yyvsp[-1].enm,yyvsp[0].chr); }
    break;
case 75:
#line 422 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* trying to do some math */
           yyval.enm = NULL;
	   mta->skiptocommarb(); }
    break;
case 76:
#line 427 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->cur_enum->enum_vals.Add(yyvsp[0].enm);
	    mta->enum_stack.Pop(); }
    break;
case 77:
#line 430 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* using -424242 as a err code (ugly) */
            mta->cur_enum->enum_vals.Add(yyvsp[-2].enm); if(yyvsp[0].rval != -424242) yyvsp[-2].enm->enum_no = yyvsp[0].rval;
	    mta->enum_stack.Pop(); }
    break;
case 79:
#line 437 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	  if((yyvsp[-2].rval != -424242) && (yyvsp[0].rval != -424242))  yyval.rval = yyvsp[-2].rval + yyvsp[0].rval;
	  else if(yyvsp[-2].rval != -424242)	yyval.rval = yyvsp[-2].rval;
	  else if(yyvsp[0].rval != -424242)	yyval.rval = yyvsp[0].rval;
	  else				yyval.rval = -424242; }
    break;
case 80:
#line 442 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	  if((yyvsp[-2].rval != -424242) && (yyvsp[0].rval != -424242)) yyval.rval = yyvsp[-2].rval - yyvsp[0].rval;
	  else if(yyvsp[-2].rval != -424242)	yyval.rval = yyvsp[-2].rval;
	  else if(yyvsp[0].rval != -424242)	yyval.rval = yyvsp[0].rval;
	  else				yyval.rval = -424242; }
    break;
case 81:
#line 450 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = -424242; }
    break;
case 83:
#line 455 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.enm = new EnumDef(yyvsp[0].chr); mta->enum_stack.Push(yyval.enm); }
    break;
case 84:
#line 458 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->Class_UpdateLastPtrs(); }
    break;
case 85:
#line 459 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->Class_UpdateLastPtrs(); }
    break;
case 86:
#line 462 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ 
            if(yyvsp[0].memb != NULL) {
	      if((mta->cur_mstate == MTA::pblc) && !(yyvsp[0].memb->HasOption("IGNORE"))
		 && !(yyvsp[0].memb->type->DerivesFrom(TA_const))) {
		mta->cur_class->members.AddUniqNameNew(yyvsp[0].memb);
		if(mta->verbose >= 3)
		  cerr << "member: " << yyvsp[0].memb->name << " added to: "
		       << mta->cur_class->name << "\n"; } }
	    mta->memb_stack.Pop(); yyval.typ = NULL; }
    break;
case 87:
#line 471 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            if(yyvsp[0].meth != NULL) {
	      if(mta->cur_mstate == MTA::pblc) {
		if(yyvsp[0].meth->HasOption("IGNORE"))
		  mta->cur_class->ignore_meths.AddUnique(yyvsp[0].meth->name);
		else {
		  mta->cur_class->methods.AddUniqNameNew(yyvsp[0].meth);
		  if(mta->verbose >= 3)
		    cerr << "method: " << yyvsp[0].meth->name << " added to: "
			 << mta->cur_class->name << "\n"; } } }
	    else {
	      mta->cur_meth = NULL; }
	    mta->meth_stack.Pop(); yyval.typ = NULL; }
    break;
case 88:
#line 484 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    mta->cur_class->sub_types.AddUniqNameNew(yyvsp[0].typ);
	    mta->state = MTA::Parse_inclass; yyval.typ = NULL; }
    break;
case 89:
#line 487 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ 
	    mta->cur_class->sub_types.AddUniqNameNew(yyvsp[0].typ);
	    mta->state = MTA::Parse_inclass; yyval.typ = NULL; }
    break;
case 90:
#line 490 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* todo: not dealing with sub classes yet.. */
	    mta->last_class->sub_types.AddUniqNameNew(yyvsp[-1].typ);
  	    mta->cur_class = mta->last_class; /* pop back last class.. */
	    mta->state = MTA::Parse_inclass; yyval.typ = NULL; }
    break;
case 91:
#line 494 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = NULL; }
    break;
case 93:
#line 499 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.memb = NULL; mta->cur_mstate = MTA::pblc; }
    break;
case 94:
#line 500 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.memb = NULL; mta->cur_mstate = MTA::prvt; }
    break;
case 95:
#line 501 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.memb = NULL; mta->cur_mstate = MTA::prot; }
    break;
case 96:
#line 502 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.memb = NULL;
	    if(mta->last_memb != NULL) {
	      SETDESC(mta->last_memb, yyvsp[0].chr);
	      if(mta->last_memb->HasOption("IGNORE"))
		mta->cur_class->members.Remove(mta->last_memb); }
	    else if(mta->last_meth != NULL) {
	      SETDESC(mta->last_meth, yyvsp[0].chr);
	      if(mta->last_meth->HasOption("IGNORE")) {
		mta->cur_class->ignore_meths.AddUnique(mta->last_meth->name);
		mta->cur_class->methods.Remove(mta->last_meth);
		mta->last_meth = NULL; }
	      else if((mta->last_meth->opts.size > 0) || (mta->last_meth->lists.size > 0)) {	
		mta->cur_class->methods.AddUniqNameNew(mta->last_meth);
		if(mta->verbose >= 3)
		  cerr << "method: " << mta->last_meth->name << " added to: "
		       << mta->cur_class->name << "\n"; } } }
    break;
case 98:
#line 523 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ 
	  yyval.memb = yyvsp[0].memb; if(yyvsp[0].memb != NULL) yyvsp[0].memb->is_static = true;
	  else if(mta->cur_memb != NULL) mta->cur_memb->is_static = true; }
    break;
case 99:
#line 529 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.memb = yyvsp[-1].memb; }
    break;
case 100:
#line 530 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ }
    break;
case 101:
#line 531 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.memb = yyvsp[-2].memb;
	    String nm = yyvsp[-3].typ->name + "_ary";
	    TypeDef* nty = new TypeDef((char*)nm, true, yyvsp[-3].typ->ptr + 1);
	    nty->AddParFormal(&TA_ta_array); nty->AddParent(yyvsp[-3].typ);
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[-3].typ);
	    TypeDef* uty = sp->AddUniqNameOld(nty); yyvsp[-2].memb->type = uty;
	    if(uty == nty) mta->TypeAdded("array", sp, uty); }
    break;
case 102:
#line 539 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyvsp[-2].memb->type = yyvsp[-3].typ; yyval.memb = yyvsp[-2].memb; }
    break;
case 103:
#line 543 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	     if((mta->cur_mstate == MTA::pblc) && !(yyvsp[0].memb->type->DerivesFrom(TA_const)))
	       mta->cur_class->members.AddUniqNameNew(yyvsp[0].memb);
             mta->memb_stack.Pop(); yyval.memb = NULL; }
    break;
case 104:
#line 547 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	     if((mta->cur_mstate == MTA::pblc) && !(yyvsp[0].memb->type->DerivesFrom(TA_const)))
	       mta->cur_class->members.AddUniqNameNew(yyvsp[0].memb);
             mta->memb_stack.Pop(); yyval.memb = NULL; }
    break;
case 105:
#line 553 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyval.memb = new MemberDef(yyvsp[0].chr); mta->cur_memb = yyval.memb; mta->memb_stack.Push(yyval.memb); 
            if(mta->cur_memb_type != NULL) yyval.memb->type = mta->cur_memb_type;
            else yyval.memb->type = &TA_int; }
    break;
case 106:
#line 559 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.memb = new MemberDef(yyvsp[-1].chr); mta->cur_memb = yyval.memb; mta->memb_stack.Push(yyval.memb);
	    yyval.memb->fun_ptr = 1; }
    break;
case 108:
#line 565 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; mta->thisname = false; }
    break;
case 109:
#line 566 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; mta->thisname = false; }
    break;
case 110:
#line 567 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; mta->thisname = false; }
    break;
case 111:
#line 568 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; mta->thisname = false; }
    break;
case 112:
#line 569 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; mta->thisname = false; }
    break;
case 113:
#line 570 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 114:
#line 571 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 115:
#line 572 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 116:
#line 573 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 117:
#line 574 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 118:
#line 575 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.meth = NULL; String tmp = yyvsp[0].chr;
	    if(tmp.contains("REG_FUN") && (mta->spc == &(mta->spc_target))) {
	      TA_taRegFun.methods.AddUniqNameNew(yyvsp[-2].meth); yyvsp[-2].meth->type = yyvsp[-3].typ;
	      mta->meth_stack.Pop();  yyvsp[-2].meth->fun_argc = yyvsp[-1].rval; yyvsp[-2].meth->arg_types.size = yyvsp[-1].rval;
	      yyvsp[-2].meth->is_static = true; /* consider these to be static functions */
	      SETDESC(yyvsp[-2].meth,yyvsp[0].chr); }
	    else { yyval.meth = NULL; mta->meth_stack.Pop(); } }
    break;
case 119:
#line 583 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.meth = NULL; String tmp = yyvsp[0].chr;
	    if(tmp.contains("REG_FUN") && (mta->spc == &(mta->spc_target))) {
	      TA_taRegFun.methods.AddUniqNameNew(yyvsp[-2].meth); yyvsp[-2].meth->type = &TA_int;
	      mta->meth_stack.Pop();  yyvsp[-2].meth->fun_argc = yyvsp[-1].rval; yyvsp[-2].meth->arg_types.size = yyvsp[-1].rval;
	      yyvsp[-2].meth->is_static = true; /* consider these to be static functions */
	      SETDESC(yyvsp[-2].meth,yyvsp[0].chr); }
	    else { yyval.meth = 0; mta->meth_stack.Pop(); } }
    break;
case 121:
#line 595 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = yyvsp[0].meth; yyvsp[0].meth->is_static = true; }
    break;
case 122:
#line 599 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = yyvsp[-1].meth; yyvsp[-1].meth->type = yyvsp[-2].typ; SETDESC(yyvsp[-1].meth,yyvsp[0].chr); }
    break;
case 123:
#line 600 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyvsp[-1].meth->type = &TA_int; SETDESC(yyvsp[-1].meth,yyvsp[0].chr); }
    break;
case 124:
#line 601 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 125:
#line 602 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 126:
#line 603 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.meth = NULL; }
    break;
case 127:
#line 606 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
             yyvsp[-1].meth->fun_argc = yyvsp[0].rval; yyvsp[-1].meth->arg_types.size = yyvsp[0].rval; mta->burp_fundefn = false;
	     /* argd should always be less than argc, but scanner might screw up
  	        (in fact it does in certain cases..) (if so, then just reset!) */
	     if(yyvsp[-1].meth->fun_argd > yyvsp[-1].meth->fun_argc) yyvsp[-1].meth->fun_argd = -1; }
    break;
case 128:
#line 613 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            yyval.meth = new MethodDef(yyvsp[0].chr); mta->cur_meth = yyval.meth; mta->meth_stack.Push(yyval.meth); }
    break;
case 129:
#line 617 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 130:
#line 618 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[0].chr; }
    break;
case 131:
#line 619 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[-1].chr; }
    break;
case 132:
#line 620 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[-1].chr; }
    break;
case 133:
#line 621 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[-1].chr; }
    break;
case 134:
#line 622 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[-2].chr; }
    break;
case 135:
#line 626 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 136:
#line 627 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 137:
#line 628 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[-1].chr; }
    break;
case 138:
#line 632 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 139:
#line 633 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 140:
#line 634 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = ""; }
    break;
case 141:
#line 637 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = 0; }
    break;
case 142:
#line 638 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = yyvsp[-1].rval; }
    break;
case 143:
#line 641 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = 0; }
    break;
case 144:
#line 642 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = yyvsp[-1].rval; }
    break;
case 145:
#line 645 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = 1; }
    break;
case 146:
#line 646 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = yyvsp[-2].rval + 1; }
    break;
case 147:
#line 647 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = yyvsp[-3].rval; }
    break;
case 148:
#line 650 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            if(mta->cur_meth != NULL) { mta->cur_meth->arg_defs.Add(""); } }
    break;
case 149:
#line 652 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_defs.Add(yyvsp[0].chr);
	      if(mta->cur_meth->fun_argd < 0)
		mta->cur_meth->fun_argd = mta->cur_meth->arg_types.size - 1; } }
    break;
case 150:
#line 660 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.rval = 1; String nm = "na";
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_types.Link(yyvsp[0].typ); mta->cur_meth->arg_names.Add(nm); } }
    break;
case 151:
#line 664 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.rval = 1; String nm = yyvsp[0].chr;
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_types.Link(yyvsp[-1].typ); mta->cur_meth->arg_names.Add(nm); } }
    break;
case 152:
#line 668 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.rval = 1; String nm = String(yyvsp[-1].chr) + "[]";
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_types.Link(yyvsp[-2].typ); mta->cur_meth->arg_names.Add(nm); } }
    break;
case 153:
#line 672 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.rval = 1; String nm = String("(*") + String(yyvsp[-2].chr) + ")";
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_types.Link(yyvsp[-5].typ); mta->cur_meth->arg_names.Add(nm); } }
    break;
case 154:
#line 676 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    yyval.rval = 1; String nm = yyvsp[0].chr;
	    if(mta->cur_meth != NULL) {
	      mta->cur_meth->arg_types.Link(&TA_int); mta->cur_meth->arg_names.Add(nm); } }
    break;
case 158:
#line 691 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
            mta->thisname = true; mta->constcoln = false; }
    break;
case 159:
#line 695 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->constcoln = true; }
    break;
case 160:
#line 698 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = 1; }
    break;
case 161:
#line 699 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.rval = yyvsp[-1].rval + 1; }
    break;
case 162:
#line 702 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_memb_type = yyvsp[0].typ; }
    break;
case 164:
#line 706 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = &TA_int; }
    break;
case 165:
#line 707 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 166:
#line 710 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = new TypeDef(yyvsp[0].chr); mta->type_stack.Push(yyval.typ); }
    break;
case 168:
#line 714 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    String nm = yyvsp[-1].typ->name + "_ref";
	    TypeDef* nty = new TypeDef((char*)nm, true, yyvsp[-1].typ->ptr, true);
	    nty->AddParent(yyvsp[-1].typ);
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[-1].typ);
	    yyval.typ = sp->AddUniqNameOld(nty);
	    if(yyval.typ == nty) mta->TypeAdded("ref", sp, yyval.typ); }
    break;
case 170:
#line 725 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
 	    int i; String nm = yyvsp[-1].typ->name; for(i=0; i<yyvsp[0].rval; i++) nm += "_ptr";
	    TypeDef* nty = new TypeDef((char*)nm, true, yyvsp[0].rval); nty->AddParent(yyvsp[-1].typ);
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[-1].typ);
	    yyval.typ = sp->AddUniqNameOld(nty);
	    if(yyval.typ == nty) mta->TypeAdded("ptr", sp, yyval.typ); }
    break;
case 172:
#line 734 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    String nm = yyvsp[-1].typ->name + "_" + yyvsp[0].typ->name;
	    TypeDef* nty = new TypeDef((char*)nm, true);
	    nty->size = yyvsp[0].typ->size; nty->AddParent(yyvsp[-1].typ); nty->AddParent(yyvsp[0].typ);
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(nty);
	    if(yyval.typ == nty) mta->TypeAdded("const", sp, yyval.typ); }
    break;
case 174:
#line 744 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 175:
#line 745 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 176:
#line 746 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeDef* td; if((td = yyvsp[-2].typ->sub_types.FindName(yyvsp[0].chr)) == NULL) {
	      yyerror("Subtype not found"); YYERROR; }
	    yyval.typ = td; }
    break;
case 177:
#line 750 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    TypeDef* td; if((td = yyvsp[-2].typ->sub_types.FindName(yyvsp[0].chr)) == NULL) {
	      yyerror("Subtype not found"); YYERROR; }
	    yyval.typ = td; }
    break;
case 178:
#line 754 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 179:
#line 755 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 180:
#line 756 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.typ = yyvsp[0].typ; }
    break;
case 182:
#line 758 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* a template */
 	    if(!(yyvsp[-3].typ->InheritsFormal(TA_template))) {
	      yyerror("Template syntax error"); YYERROR; }
	    if((yyvsp[-1].typ->owner != NULL) && (yyvsp[-1].typ->owner->owner != NULL))
	      yyval.typ = yyvsp[-3].typ;	/* don't allow internal types with external templates */
	    else {
	      String nm = yyvsp[-3].typ->GetTemplName(mta->cur_templ_pars);
	      TypeDef* td;
	      int lx_tok;
	      if((td = mta->FindName(nm, lx_tok)) == NULL) {
		td = yyvsp[-3].typ->Clone(); td->name = nm; 
		td->SetTemplType(yyvsp[-3].typ, mta->cur_templ_pars);
		TypeSpace* sp = mta->GetTypeSpace(yyvsp[-3].typ);
 		yyval.typ = sp->AddUniqNameOld(td);
		if(yyval.typ == td) mta->TypeAdded("template instance", sp, yyval.typ); }
	      else 
		yyval.typ = td; } }
    break;
case 183:
#line 775 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ /* this template */
	    if(!(yyvsp[-3].typ->InheritsFormal(TA_template))) {
	      yyerror("Template syntax error"); YYERROR; }
	    yyval.typ = yyvsp[-3].typ; }
    break;
case 185:
#line 782 "/usr/local/pdp++/src/maketa/mta_parse.y"
{
	    String nm = yyvsp[-1].typ->name + "_" + yyvsp[0].typ->name;
	    TypeDef* nty = new TypeDef((char*)nm, true);
	    nty->size = yyvsp[0].typ->size; nty->AddParent(yyvsp[-1].typ); nty->AddParent(yyvsp[0].typ);
	    TypeSpace* sp = mta->GetTypeSpace(yyvsp[0].typ);
	    yyval.typ = sp->AddUniqNameOld(nty);
	    if(yyval.typ == nty) mta->TypeAdded("combo", sp, yyval.typ); }
    break;
case 186:
#line 792 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_templ_pars.Link(yyvsp[0].typ); }
    break;
case 187:
#line 793 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ mta->cur_templ_pars.Link(yyvsp[0].typ); yyval.typ = yyvsp[-2].typ; }
    break;
case 189:
#line 797 "/usr/local/pdp++/src/maketa/mta_parse.y"
{ yyval.chr = yyvsp[0].typ->name; }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 829 "/usr/local/pdp++/src/maketa/mta_parse.y"


	/* end of grammar */

void yyerror(char *s) { 	/* called for yacc syntax error */
  int i;
  if((mta->verbose < 1) && (mta->spc != &(mta->spc_target)))
    return;

  if(strcmp(s, "parse error") == 0) {
    cerr << "Syntax Error, line " << mta->st_line << ":\t" << MTA::LastLn << "\n";
    cerr << "\t\t\t";
    for(i=0; i < mta->st_col + 1; i++)
      cerr << " ";
    cerr << "^\n";
  }
  else {
    cerr << s << "line " << mta->st_line << ":\t" << MTA::LastLn << "\n";
  }
}
