######################################################################
# ta/css/pdp++: Standard Definitions for the makefile (Makefile.defs)
#

# paths to various things:
# it is assumed that everything is installed under LOCAL
LOCAL = /usr/local
PDPDIR = $(LOCAL)/pdp++
# top can be defined to be in a user's home directory: it is where the source is
TOP = $(PDPDIR)/src
IV = $(LOCAL)/interviews

# LOC_OBJS is a directory physically local to each machine where the 
# object files/libraries, etc are put to speed compliation in a distributed
# environment, this directory must be created in advance, and must be given
# read/write permission to whoever will be compiling the software..
LOC_OBJS = /usr/user/pdp++

# standard definitions for path-type variables
# the PROJDIR is the directory the 'project' is in
PROJDIR = $(PROJNAME)
VPATH = $(TOP)/$(PROJDIR)
SRC_INC = src_include/$(PROJDIR)
IDIRS_XX = -I./src_include -I./src_include/$(PROJDIR) -I$(PDPDIR)/include/$(CPU)\
	-I$(IV)/include 
IDIRS_EXTRA =
LDIRS_XX = -L$(PDPDIR)/lib/$(CPU) -L$(LOCAL)/lib
LDIRS_EXTRA =

IDIRS = $(IDIRS_XX) $(IDIRS_EXTRA)
LDIRS = $(LDIRS_XX) $(LDIRS_EXTRA)

# the names of the executables, etc
EXE = $(PROJNAME)
# this is the 'distribution' executable (i.e. the one that works, not the latest)
DIST_EXE = $(PROJNAME)++
LIB_EXT = a
LIB = lib$(PROJNAME).$(LIB_EXT)
# extra dependencies for the executable
EXE_DEPENDS =

CC = gcc
CXX = g++
cppC = /usr/lib/cccp -lang-c++

DEBUG_FLAG = -g -Wall -Winline
# debug flags that can be used with optimization
DEBUG_OPT_FLAG = 
# -g tends to eat up a huge amount of space..
#DEBUG_OPT_FLAG = -g
OPT_FLAG = -O2

# controlls the instantiation of virtual tables (cfront)
# instantiate vtable flag (e.g. +e1 for cfront)
VT_INST_FLAG = +e1 
# instantiate vtable file
VT_INST_FILE = $(PROJNAME)_vt
# to actually include the vtable file in executable (cfront only)
#VT_INST_SRC = $(VT_INST_FILE).cc
#VT_INST_OBJ = $(VT_INST_FILE).o
#VT_INST_DEP = $(VT_INST_FILE).d
VT_INST_SRC = 
VT_INST_OBJ = 
VT_INST_DEP = 

# controlls the instantiation of templates (g++)
# instantiate templates flag (maybe not necessary)
TI_INST_FLAG =
# instantiate vtable file
TI_INST_FILE = $(PROJNAME)_ti
# to actually include the template file in executable (g++ only)
#TI_INST_SRC = $(TI_INST_FILE).cc
#TI_INST_OBJ = $(TI_INST_FILE).o
#TI_INST_DEP = $(TI_INST_FILE).d
TI_INST_SRC = 
TI_INST_OBJ = 
TI_INST_DEP = 

AR = ar rc
RANLIB = ranlib

YACC = bison
YFLAGS = -dtvy

RM = rm -f
MV = mv -f
CP = /bin/cp
LN = ln -s
CAT = cat
CD = cd
MKDIR = mkdir
CMP = cmp -s
ED = ed
SED = sed
TOUCH = touch -c
COMPRESS = gzip -f
NICE     = nice +10 

# pipe tends to take too much ram...
#PIPEFLAG = -pipe
PIPEFLAG = 

DEPS_XX = $(DEPS) $(VT_INST_DEP) $(TI_INST_DEP)

DEFINES =
DEFINES_XX = -D$(CPU) $(DEFINES)

CCFLAGS_XX = $(FLAGS) $(MACH_FLAGS) $(PIPEFLAG) $(DEFINES_XX)
CFLAGS_XX =  $(MACH_FLAGS) $(PIPEFLAG) $(DEFINES_XX)

DEBUG_FLAGS = $(DEBUG_FLAG)
OPT_FLAGS = $(DEBUG_OPT_FLAG) $(OPT_FLAG)

# this is the one to set to control optimization vs. debugging
OPT_DEBUG = $(DEBUG_FLAGS)

CCFLAGS = $(OPT_DEBUG) $(CCFLAGS_XX)
CFLAGS = $(OPT_DEBUG) $(CFLAGS_XX)

TA_CCFLAGS_XX = $(FLAGS) $(MACH_TA_CCFLAGS) $(DEFINES_XX)

#TA_CCFLAGS = -g $(TA_CCFLAGS_XX)
TA_CCFLAGS = $(TA_CCFLAGS_XX)

MAKETA = $(PDPDIR)/bin/$(CPU)/maketa

MAKETA_FLAGS_XX = "-cpp=$(cppC)" $(MAKETA_FLAGS) $(FLAGS) $(DEFINES_XX) 

PROOF = proof

IOS_LIB = -liostream
IBERTY_LIB = -liberty
GENERIC_LIB = -lm

# this must correspond with the version of the iostream library used
# its for ta which scans modified versions of these include files
IOS_INCLUDES = g++-2.6
####################################################
# machine-dependent stuff and overrides of std defs

MACH_FLAGS = 
MACH_TA_CCFLAGS = $(MACH_FLAGS) -w

DEFINES = -DNO_SIGNED -DNO_BUILTIN_BOOL -DCONST_48_ARGS

DEBUG_FLAG = -g
DEBUG_OPT_FLAG = 
OPT_FLAG = -O

CXX = CC +a1 +e0
CC = gcc -U__GNUG__

# use gnu cccp instead cuz it works!
cppC = /usr/lib/cccp -lang-c++ -nostdinc++

# doesn't need ranlib
RANLIB = ls

# to make a shared library on SGI
# AR = /usr/lib/ld -shared -o 
# LIB_EXT = so

VT_INST_SRC = $(VT_INST_FILE).cc
VT_INST_OBJ = $(VT_INST_FILE).o
VT_INST_DEP = $(VT_INST_FILE).d

#include CC iostream stuff
IDIRS_EXTRA = -I/usr/include/CC

IOS_LIB = 
IBERTY_LIB = -lmyiberty

#use opt malloc (otherwise get mysterious bug)
GENERIC_LIB = -lm -lC -lmalloc

# use the CC includes
IOS_INCLUDES = CC-3.1



#####################################################
#
#  Hardcode CSS Makefile
#	    

# this is for any _vt or _ti files that might be needed..

PROJNAME = hard_of_css

LIBS = -lspecmath -ltypea_min -ltastring $(IOS_LIB) $(IBERTY_LIB) $(GENERIC_LIB)

# usually compiling optimized..
OPT_DEBUG = $(OPT_FLAGS)

% : %.c
	$(CC) -o $@ $(CFLAGS) $(LDIRS) $(IDIRS) $<  $(LIBS)

% : %.cc 
	$(CXX) -o $@ $(CCFLAGS) $(LDIRS) $(IDIRS) $< $(VT_INST_OBJ) \
	$(TI_INST_OBJ) $(LIBS)

% : %.css
	$(CXX) -o $@ $(CCFLAGS) $(LDIRS) $(IDIRS) $< $(VT_INST_OBJ) \
	$(TI_INST_OBJ) $(LIBS)

% : %.o
	$(CXX) -o $@ $(CCFLAGS) $(LDIRS) $(IDIRS) $< $(VT_INST_OBJ) \
	$(TI_INST_OBJ) $(LIBS)

.cc.o:
	$(CXX) -o $@ $(IDIRS) $(CCFLAGS) -c $< 

$(VT_INST_FILE).o:	$(VT_INST_FILE).cc
	$(CXX) -o $@ $(IDIRS) $(TA_CCFLAGS) $(VT_INST_FLAG) -c $< 
