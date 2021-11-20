# super-script language makefile

#DEBUG = -O
DEBUG = -g
#DEBUG = -E

SGIFLAGS = 
SUNFLAGS =  

CC = gcc
GXX = g++  -I./include  -L./lib
GXXN = g++

COMPILER = GNU

# set to proper machine type!!
MCFLAGS = $(SUNFLAGS)

GXXLDFLAGS = $(MCFLAGS)
GXXFLAGS = $(DEBUG) $(MCFLAGS)
CFLAGS = $(DEBUG) $(MCFLAGS)

YACC = bison
YFLAGS = -dy

# g++ files should have extension .cc
.SUFFIXES: .cc
.cc.o:
	$(GXX) $(GXXFLAGS) -c  $<

.y.c:	$(YACC.y) $<
	mv y.tab.c $*.cc
.y.o:	$(YACC.y) $<
	$(YACC) $(YFLAGS) $<
	mv y.tab.c y.tab.cc
	$(GXX) $(GXXFLAGS) -c -o $@ y.tab.cc
	$(RM) y.tab.cc

LIBS	= -lg++ -lm

EXECS =  ssl

OBJECTS = ssl_parse.o ssl.o code.o init.o math.o symbol.o
	  
MYSTRINGOBJ = mystring.o
SRCS = ssl.cc script.cc
GENHEADS = pdp.h
GCCLIBDIR = /usr/local/gcc/ligb++2.2/libg++
LIBRARYHEADER = -I$(GCCLIBDIR) -I$(GCCLIBDIR)/src \
	-I$(GCCLIBDIR)/iostream -I$(GCCLIBDIR)/g++-include

default: ssl

mystring.o: mystring.h mystring.cc
	$(GXXN) $(GCCFLAGS) $(GXXLDFLAGS) $(LIBRARYHEADER) -c mystring.cc


ssl: $(OBJECTS)
	$(GXX) $(GCCFLAGS) $(GXXLDFLAGS) $(OBJECTS) $(LIBS) -o ssl

ssl.o ssl_parse.o code.o init.o symbol.o:	code.h ssl.h

code.o init.o symbol.o:	x.tab.h

x.tab.h: y.tab.h
	-cmp -s x.tab.h y.tab.h || cp y.tab.h x.tab.h

clean:
	- /bin/rm -f *.o *~ #* $(EXECS)
	


