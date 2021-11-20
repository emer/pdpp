# include <stdio.h>
# include <math.h>
# define MAXCOLS	80
# define MAXLEN		100
# define MAXVARS	15
# define MAXLEVS	1000
# define MAXFACS	15
int indxc[MAXFACS];
int datc[MAXVARS];
extern int strcmp();
int intcmp();
int cdcmp();
float *sum;
float *ss;
int *count;
int index[MAXFACS];
char indstr[MAXFACS][MAXLEVS][MAXLEN];
int indstrmax[MAXFACS];
char tmpname[BUFSIZ];
int nfacs,ndeps,ncells,tcells,cellno,dcellno;
int nlevs[MAXFACS];
int step[MAXFACS];
int precis = 3;
char precstr[30];
float cellmean,denom,sd,x;
char in[MAXCOLS+1][MAXLEN];
FILE *tmp;
int sort = 1;
int sumsonly,mnsonly, nocount, nosd, second_line, old_sd;

main(argc,argv)
     int argc;
     char *argv[];
{
  register int i,j,k;
  int l, nomatch;
  int iargs;
  int ncols;
  int tc,tsize,jj;
  int len;
  strcpy(in[0],"GM");
  iargs = 0;
  mnsonly = 0;
  sumsonly = 0;
  nocount = nosd = 0;
  second_line = 0;
  old_sd = 0;
  if(argc == 1) {
    fprintf(stderr, "%s: args are:\nt #    tabulate by factor in column #\n\
d #    compute on data in column #\n\
m      generate means only (no sem's or counts)\n\
n      do not sort output\n\
s      generate sums only (no sem's)\n\
p #    use # digits of precision in printing numbers\n\
o      generate 'old' sd's instead of sem's\n\
l      generate a 'long' 2-line format output (means, then sem's)\n\n", argv[0]);
    return;
  }
  while (++iargs < argc)
  {
    switch(*argv[iargs])
    {
    case 't':
      if (nfacs  == MAXFACS)
      {
	fprintf(stderr,"Too many factors\n");
	exit();
      }
      indxc[nfacs] = atoi(argv[++iargs]);
      nfacs++;
      break;
    case 'd':
      if (ndeps == MAXVARS)
      {
	fprintf(stderr,"Too many variables\n");
	exit();
      }
      datc[ndeps] = atoi(argv[++iargs]);
      ndeps++;
      break;
    case 'm':
      mnsonly = 1;
      nocount = 1;
      nosd = 1;
      break;
      /* means only; counts and sd's suppressed */
    case 'n':
      sort = 0;
      break;
      /* do not sort column indexes */
    case 's':
      sumsonly = 1;
      nosd = 1;
      break;
      /* sums only; counts and sd's suppressed */
    case 'p':
      precis = atoi(argv[++iargs]);
      break;
    case 'l':			/* second line */
      second_line = 1;
      break;
    case 'o':
      old_sd = 1;
      break;
    }
  }
  
  sprintf(precstr,"%%7.%dg\t",precis);

  if (nfacs == 0)
  {
    nfacs = 1;
    indxc[0] = 0;
  }
  tmp = fopen(tmpnam(tmpname),"w");
  while ( ( ncols = fstrings(stdin,&in[1][0],MAXCOLS,MAXLEN)) != EOF)
  {

    for (i = 1; i <= ncols; i++)
      fprintf (tmp,"%s ",in[i]);
    fprintf (tmp,"\n");
    for (j = 0; j < nfacs; j++)
    {
      nomatch = 1;
      for (l = 0; l < nlevs[j]; l++)
	if (strcmp(in[indxc[j]],indstr[j][l]) == 0)
	{
	  nomatch = 0;
	  break;
	}
      if (nomatch)
      {
	if (nlevs[j]++ > MAXLEVS)
	{
	  fprintf(stderr,
		  "Too many levels of index %d\n",j);
	  exit();
	}
	len = strlen(in[indxc[j]]);
	if (len > indstrmax[j]) indstrmax[j] = len;
	strcpy(indstr[j][l],in[indxc[j]]);
      }
    }
  }

  ncells = 1;
  for (j = 0; j < nfacs; j++)
  {
    ncells *= nlevs[j];
/*		if (coltype(j))
		qsort(indstr[j],nlevs[j],MAXLEN,intcmp);
		else
		qsort(indstr[j],nlevs[j],MAXLEN,strcmp);
		*/
    if (sort) qsort(indstr[j],nlevs[j],MAXLEN,cdcmp);
    step[j] = 1;
    for (jj = j+1; jj < nfacs; jj++)
    {
      step[j] *= nlevs[jj];
    }
  }
  tcells = ncells*ndeps;
  if ( (sum = (float *) calloc(tcells,sizeof(cellmean)) ) == NULL)
  {
    printf("Cannot allocate core for %d sums\n",tcells);
    exit();
  }
  if ( (ss = (float *) calloc(tcells,sizeof(cellmean)) ) == NULL)
  {
    printf("Cannot allocate core for %d ss's\n",tcells);
    exit();
  }
  if ( ( count = (int *) calloc(ncells,sizeof(i)) ) == NULL )
  {
    printf("Cannot allocate core for %d counts\n",ncells);
    exit();
  }

  for (i = 0; i < ncells; i++)
    count[i] = 0;
  for (i = 0; i < tcells; i++)
    sum[i] = ss[i] = 0.0;
  fclose (tmp);
  tmp = fopen(tmpname,"r");
  while ( ( ncols = strings(tmp,in[1],MAXCOLS,MAXLEN) ) != NULL)
  {
    cellno = 0;
    for (j = 0; j < nfacs; j++)
    {
      for (l = 0; l < nlevs[j]; l++)
      {
	if (strcmp(in[indxc[j]],indstr[j][l]) == 0)
	{
	  cellno += l*step[j];
	  break;
	}
      }
    }

    count[cellno]++;

    for (k = 0; k < ndeps; k++)
    {
      dcellno = (cellno*ndeps) + k;
      x = atof(in[datc[k]]);
      sum[dcellno] = sum[dcellno] + x;
      if (!nosd)
	ss[dcellno] = ss[dcellno] + x*x;
    }

  }

  for (i = 0; i < ncells; i++)
  {
    if (count[i])
    {
      tc = i;
      for (j = 0; j < nfacs; j++)
      {
	tsize = step[j];
	index[j] = tc/tsize;
	tc -= index[j]*step[j];
      }
      for (j = 0; j < nfacs; j++)
	printf("%7s\t",indstr[j][index[j]]);
      if(!nocount && !second_line)
      	printf("%7d\t",count[i]);
      for (k = 0; k < ndeps; k++)
      {
	dcellno = (i*ndeps) + k;
	denom = count[i];
	cellmean = sum[dcellno]/denom; /* needed for sd's */
	if (sumsonly) {
	  printf(precstr, sum[dcellno]);
	}
	else {
	  printf(precstr, cellmean);
	}
	if (!second_line && !nosd) {
	  if (denom > 1.) {
	    sd = (ss[dcellno] - (denom*cellmean*cellmean))/(denom-1.);
	    sd = sqrt(sd);
	    if(!old_sd)		/* doing sem */
	      sd /= sqrt(denom-1.0);
	  }
	  else sd = 0.;
	  printf(precstr,sd);
	}
      }
      if(second_line && !nosd) {
	printf("\n");		/* and count on second line */
	printf("%7d\t",count[i]);
	for (j = 0; j < nfacs-1; j++)
	  printf("\t");
	for (k = 0; k < ndeps; k++) {
	  dcellno = (i*ndeps) + k;
	  denom = count[i];
	  cellmean = sum[dcellno]/denom;
	  if (denom > 1.) {
	    sd = (ss[dcellno] - (denom*cellmean*cellmean))/(denom-1.);
	    sd = sqrt(sd);
	    if(!old_sd) 	/* doing sem */
	      sd /= sqrt(denom-1.0);
	  }
	  else sd = 0.;
	  printf(precstr,sd);
	}
      }
      printf("\n");
    }
  }
  unlink(tmpname);
}

cdcmp(s1,s2) char *s1, *s2; {
  char ls1[BUFSIZ],ls2[BUFSIZ],cs1[BUFSIZ],cs2[BUFSIZ];
  char *p1, *p2; int c, d1, d2;
  float f1, f2;
  c = 0;
  p1 = s1; p2 = s2;
  while (*p1 && *p2) {
    d1 = isdigit(*p1);
    d2 = isdigit(*p2);
    if (d1 && d2) {	
      strcpy(ls1,p1); p1 = ls1;
      strcpy(ls2,p2); p2 = ls2;
      sscanf(p1,"%f%s",&f1,cs1);
      sscanf(p2,"%f%s",&f2,cs2);
      if (f1 > f2) return(1);
      if (f2 > f1) return(-1);
      p1 = cs1;
      p2 = cs2;
    }
    else {
      if (d1) return(1);
      if (d2) return(-1);
      if (*p1 - *p2 > 0) return(1);
      if (*p2 - *p1 > 0) return(-1);
      p1++; p2++;
    }
  }
  if (*p1) return(1);
  if (*p2) return(-1);
  return(0);
}

intcmp(s1,s2)
     char *s1, *s2;
{
  int t;
  t = atoi(s1) - atoi(s2);
  return(t);
}

coltype(ii)
     int ii;
{
  int jj;
  char * p;
  for (jj = 0; jj < nlevs[ii]; jj++)
  {
    p = indstr[ii][jj];
    while (*p != '\0')
    {
      if ( ( (*p >= '0') && (*p <= '9') ) ||
	   (*p == '-') || (*p == '.') )
      {
	p++;
	continue;
      }
      return (0);
    }
  }
  return (1);
}



/* 	Strings is a safe but silent subroutine which fills
	a string array with successive strings encountered
	on a single line of input.  It only translates up to
	a specified number of characters into a specified number
	of strings -- thus preventing segmentation violations,
	memory faults, etc.  It also skips over the tails of
	strings which are too long thus being certain to get
	the right string in each position even if a string encountered
	is longer than the maximum length.  Silently it discards
	data which exceed its specified capacity.
	Strings takes a FILE pointer (which may be stdin)
	a pointer to an array of
	strings, a maximum number of strings, and the size
	of the array members, and places strings in successive
	array locations from a single input line from FILE.
	Any strings which exceed size -1 characters are
	truncated to size -1 characters.  The last character
	is always '0'.  Whatever is remaining in the input
	of the string is skipped over silently.
	Strings returns NULL if there is nothing
	on the line, otherwise it returns the number of strings
	translated. 
	*/

strings(ioptr,arrayptr,maxstrings,size)
     FILE *ioptr;
     char *arrayptr;
     int maxstrings,size;
{
  char lbuf[BUFSIZ];
  char *p, *q;
  int i, j;

  if (fgets(lbuf,BUFSIZ,ioptr) == NULL)
    return(NULL);
  
  i = 0;
  j = 0;
  p = lbuf;
  q = arrayptr;

  while ((*p != '\n') && (*p != 0 ) )
  {
    if (j >= maxstrings)
      return(j);
    if (i == 0)
    {
      if ( (*p == '\t' ) || ( *p == ' ') )
      {
	p++;
	continue;
      }
    }
    if (i < ( size - 1 ) )
    {
      if ( (*p == '\t' ) || ( *p == ' ') )
      {
	*q = '\0';
	i = 0;
	j++;
	q = arrayptr + j*size;
	p++;
	continue;
      }
      else
      {
	*q++ = *p++;
	i++;
	continue;
      }
    }
    if ( (*p == '\t' ) || ( *p == ' ') )
    {
      *q = '\0';
      i = 0;
      j++;
      q = arrayptr + j*size;
      p++;
      continue;
    }
    p++;
  }
  *q = '\0';
  j++;
  return(j);
}
