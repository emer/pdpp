######################################################################
# ta/css/pdp++: Standard Definitions for the makefile (Makefile.defs)
#

# default make target is specified in Makefile.in
all:	default

#####################################################################
#			GLOBAL PATHS 				    #
#####################################################################

# it is assumed that most things are installed under LOCAL
LOCAL = /usr/local

# PDPDIR is the path where the distribution is located.  If you change this
# here, ***BE SURE TO CHANGE IT IN THE TOP-LEVEL 'Makefile'***
# (i.e., PDPDIR/Makefile)
PDPDIR = $(LOCAL)/pdp++

# top can be defined to be in a user's home directory: it is where the source is
TOP = $(PDPDIR)/src

# publicly accessable binary directory for linking executables into (install_bin)
BIN_DIR = $(HOME)/bin

# this is where the distribution version of interviews is located (for include
# file paths) this directory should have an "include" subdirectory
IV = $(LOCAL)/interviews

# LOC_OBJS is a directory physically local to each machine where the 
# object files/libraries, etc are put to speed compliation in a distributed
# environment, this directory must be created in advance, and must be given
# read/write permission to whoever will be compiling the software..
LOC_OBJS = /usr/user/pdp++

# standard definitions for path-type variables
# the PROJDIR is the directory the 'project' is in.  This is assumed to be under
# the TOP directory
PROJDIR = $(PROJNAME)
VPATH = $(TOP)/$(PROJDIR)

#####################################################################
#			EXTERNAL LIBRARIES			    #
#####################################################################

EXTERNAL_LIBS = $(LIBS)

# this is no longer relevant -- usually the iostream lib is included in std lib
IOS_LIB = 
# IOS_LIB = -liostream

# if not using g++, need to use 'myiberty' which has alloca, xmalloc
IBERTY_LIB = 
# IBERTY_LIB = -liberty
# IBERTY_LIB = -lmyiberty

# any other misc libs that need to be included
GENERIC_LIB = -lm 

# default InterViews Library: now using the Michael Hines version from the
# NEURON program: http://www.neuron.yale.edu/
IV_LIB = -lIVhines
# if you want debugging information
# IV_LIB = -lIVdhines

# now using zlib for gz de/compression -- just tacking this onto the
# basic X11 lib so people don't have to change their Makefile.in's! 
# default X11 library
X11_LIB = -ljpeg -lX11 -lz

# this must correspond with the version of the iostream library used
# its for ta which scans modified versions of these include files
# either Cfront or g++ are supported (and the corresponding files
# are found in the src/ta directory.  Note that the CC (cfront) ones are
# symbolic links to /usr/include/CC, and the g++ ones are just copied
# from the relevant distribution. Newer ones are abstracted from 
# actual ones to only include relevant info for scanning, as they
# are never included in actual compiling, only in maketa.
# IOS_INCLUDES = CC-3.1
# IOS_INCLUDES = g++-2.8.1
IOS_INCLUDES = g++-3.1

# these are -I flags for additional include directories
IDIRS_EXTRA =
# these are -L flags for additional library directories
LDIRS_EXTRA =

#####################################################################
#			COMPILER SETTINGS			    #
#####################################################################

# make sure the shared library has the right setting (HP uses sl)
LIB_EXT_NONSHARED = a
LIB_EXT_SHARED = so

# MSDOS requires a .exe extension on executables..
EXE_EXT =

# basic compiling command for shared and non-shared making
# (definitely need PIC or its equivalent for a large GOT)
# -fno-exceptions is essential -- it is deadly slow and can't really
# compile with exceptions (takes >512mb of ram for rbp.cc)
# -fno-rtti saves a bit of disk space, but has no performance conseq.
# but the code doesn't use it, so might as well save.
# actually: got rid of -no-rtti because stringstream requires it.
CC_NONSHARED = gcc -nostdinc
# CXX_NONSHARED = g++ -fno-implicit-templates -fno-exceptions -fno-rtti
CXX_NONSHARED = g++ -fno-implicit-templates -fno-exceptions
CC_SHARED = gcc -nostdinc -fPIC
# CXX_SHARED = g++ -fPIC -fno-implicit-templates -fno-exceptions -fno-rtti
CXX_SHARED = g++ -fPIC -fno-implicit-templates -fno-exceptions

# extra flags for compiling, must correspond with appropriate defns
# for interviews include/Dispatch/dispatcher.h
MACH_FLAGS = -DHAVE_POSIX_SIGNALS=1 -DRETSIGTYPE=void
# these are special flags for compiling the huge xxx_TA.cc files
MACH_TA_CCFLAGS = $(MACH_FLAGS)

# for making libraries
AR = ar rc
RANLIB = ranlib
# command to actually use when making a library
AR_PRE_CMD_NONSHARED = echo
AR_CMD_NONSHARED = $(AR)
RANLIB_CMD_NONSHARED = $(RANLIB)

AR_PRE_CMD_SHARED = echo 
AR_CMD_SHARED = $(CXX_SHARED) -shared -Wl,-soname,$@ -o 
RANLIB_CMD_SHARED = echo

# second pass comand for libs
LIB_PASS2_CMD = echo
EXTERNAL_LIBS = $(LIBS)

# this is the GNU preprocessor, which always works
# though it is not standardly installed in /usr/local/bin, so do it yourself
cppC = /usr/local/bin/cccp -lang-c++

# the following four settings determine if shared or non-shared is default
# use noshared as a default, since that will always work
# CC is the C compiler
CC = $(CC_NONSHARED)
# CXX is the C++ compiler
CXX = $(CXX_NONSHARED)
# this is used to create the binary (linking)
BIN_CXX = $(CXX)
# these are used to create the dependency info (-M is added automatically)
DEP_CC = $(CC)
DEP_CXX = $(CXX)
# this is the extension of the library file
LIB_EXT = $(LIB_EXT_NONSHARED)
AR_PRE_CMD = $(AR_PRE_CMD_NONSHARED)
AR_CMD = $(AR_CMD_NONSHARED)
RANLIB_CMD = $(RANLIB_CMD_NONSHARED)

# this was a way of optimizing sim files when optimiziation caused
# bugs in shared linking.  not relevant anymore so should be excised
# completely.  could cause some makefiles to fail..
# SIM_NONSHARED_CXX = $(CXX)
# SIM_NONSHARED_AR_CMD = $(AR_CMD)
# SIM_NONSHARED_LIB_EXT = $(LIB_EXT)

DEBUG_FLAG = -g -Wall -Winline
# debug flags that can be used with optimization
DEBUG_OPT_FLAG = 
# -g tends to eat up a huge amount of space..
#DEBUG_OPT_FLAG = -g -Wall -Winline
# -O2 now faster in compiling in gcc 3.x..
# OPT_FLAG = -O2 -Wall -Winline
OPT_FLAG = -O2 -Wall
# get compiler to produce dependencies
DEP_FLAG = -M -E

# controls the instantiation of virtual tables (cfront)
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
# instantiate templates file
TI_INST_FILE = $(PROJNAME)_ti
# to actually include the template file in executable (g++ only)
TI_INST_SRC = $(TI_INST_FILE).cc
TI_INST_OBJ = $(TI_INST_FILE).o
TI_INST_DEP = $(TI_INST_FILE).d
# other compiler types should get rid of this TI stuff
#TI_INST_SRC = 
#TI_INST_OBJ = 
#TI_INST_DEP = 

################################
# less-important compiler stuff

# pipe tends to take too much ram...
#PIPEFLAG = -pipe
PIPEFLAG = 

# these defines are passed to every compile stage
DEFINES =
DEFINES_XX = -D$(CPU) $(DEFINES)

# these are the basic flags for compiling C++
CCFLAGS_XX = $(FLAGS) $(IN_FLAGS) $(MACH_FLAGS) $(PIPEFLAG) $(DEFINES_XX)
# and for regular C
CFLAGS_XX =  $(MACH_FLAGS) $(PIPEFLAG) $(DEFINES_XX)

DEBUG_FLAGS = $(DEBUG_FLAG)
OPT_FLAGS = $(DEBUG_OPT_FLAG) $(OPT_FLAG)

# this is the one to set to control optimization vs. debugging
OPT_DEBUG = $(DEBUG_FLAGS)

# these are actually used during compilation
CCFLAGS = $(OPT_DEBUG) $(CCFLAGS_XX)
CFLAGS = $(OPT_DEBUG) $(CFLAGS_XX)

# this is for compiling the _TA.cc file, which is huge..
TA_CCFLAGS_XX = $(FLAGS) $(IN_FLAGS) $(MACH_TA_CCFLAGS) $(DEFINES_XX)

# if you need to debug it..
#TA_CCFLAGS = -g $(TA_CCFLAGS_XX)
TA_CCFLAGS = $(TA_CCFLAGS_XX)


#####################################################################
#			LOCAL PATHS, NAMES			    #
#####################################################################

# src_include is a hack to make local files in the compilation directory
# includeable with the <PROJDIR/xxx.h> syntax.  it is a directory with a
# symbolic link with the name PROJDIR to its parent directory (../)
# it is automatically made by make cpu_dir
SRC_INC = src_include/$(PROJDIR)

# basic include directories
IDIRS_XX = -I./src_include -I./src_include/$(PROJDIR) -I$(PDPDIR)/include/$(CPU)\
	-I$(IV)/include 

# basic library directories
LDIRS_XX = -L$(PDPDIR)/lib/$(CPU) -L$(LOCAL)/lib -L$(IV)/lib/$(CPU)

# this is what is actually used when compiling
IDIRS = $(IDIRS_EXTRA) $(IDIRS_XX) 
LDIRS = $(LDIRS_EXTRA) $(LDIRS_XX) 

# the name of the executable made in this directory
EXE = $(PROJNAME)++$(EXE_EXT)
# this is the 'distribution' executable (i.e. the one that works, not the latest)
DIST_EXE = $(PROJNAME)++$(EXE_EXT)

# this is the name of the library made in this directory
LIB = lib$(PROJNAME).$(LIB_EXT)

# extra dependencies for the executable
EXE_DEPENDS =

# these are the basic dependencies 
DEPS_XX = $(DEPS) $(VT_INST_DEP) $(TI_INST_DEP)

# path to the 'maketa' type scanning program
MAKETA = $(PDPDIR)/bin/$(CPU)/maketa

MAKETA_FLAGS = -hx -css -instances
MAKETA_FLAGS_XX = "-cpp=$(cppC)" $(MAKETA_FLAGS_CPU) $(MAKETA_FLAGS) -I$(PDPDIR)/src/ta/ios-$(IOS_INCLUDES) $(FLAGS) $(IN_FLAGS) $(DEFINES_XX) 

# extra '.hx' file that needs to be copied to the CPU directory  from
# the proj directory  when the cpu_dir is made (typically from a .y
# file, 'maketa' files are handled automatically already).
# DON'T ADD THE .hx HERE!

EXTRA_HX_FILE =

# extra general file that needs to be copied to the CPU directory  from
# the proj directory  when the cpu_dir is made 

EXTRA_CPU_FILE =

# If your compiler generates a .u file when given the -M flag
# set  change the following to
# MAKEDEP_TMP_FILE = /dev/null
# in your Makefile.CPU file

MAKEDEP_TMP_FILE = `echo $@ | $(SED) 's/\.d/.u/g'`

#####################################################################
#			STANDARD DEFINES			    #
#####################################################################

# basic unix commands for manipulating files..
YACC = bison
YFLAGS = -dtvy

MAKE = make

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
NICE = nice +10 
PROOF = proof
TAR = gnutar
####################################################
# machine-dependent stuff and overrides of std defs
# DARWIN - c++ (g++) 3.1 (os X 10.2)
####################################################

# this is for Mac OSX, aka darwin.  it is pretty standard (linux-like), except
# you need to do the following steps:

# all actions below need to be as root (sudo su in osx):

# 0. you need the developer's tools, to get the g++ compiler and all the
# system headers, etc.  find this on the apple website.

# 1. fake malloc.h
# touch /usr/include/malloc.h

# 2. readline is not included, so:
# get readline-4.3.tar.gz from ftp://ftp.gnu.org
# (or our ftp site in darwin)
# ./configure --prefix=/usr/local; make; make install 
# (puts libreadline.a in /usr/local/lib -- the .dylib seems to fail but its ok)

# 3. libjpeg is not included, so: 
# grab the libjpeg code from the home website:
# www.ijg.org/files/jpegsrc.v6b.tar.gz
# (or get it from our ftp server in darwin)
# ./configure; make; make install-lib

# if you want to compile interviews:
# 1. requires iv-15.tar.gz
# 2. ./configure --prefix=/usr/local; make; make install

# That should be it!
####################################

# also define the standard (non-debug) case, which is what is looked
# for in various files..
DEFINES = -DLINUX -DDARWIN

# always debug, all the time
OPT_FLAG = -g -Wall -Winline
DEBUG_FLAG = -g -Wall -Winline

IDIRS_EXTRA = -I/usr/X11R6/include
LDIRS_EXTRA = -L/usr/X11R6/lib/

# use debug interviews!
#IV_LIB = -lIVd

# define this to be where the libg++ (stream classes) headers are found
LIBG++_INCLUDE_DIR = -I/usr/include/gcc/darwin/4.0/c++

# needs libg++ includes
MAKETA_FLAGS = -hx -css -instances $(LIBG++_INCLUDE_DIR)

IOS_INCLUDES = g++-3.1

# often in /lib/cpp but not on darwin, requires __ppc__ for system includes!
#cppC = /usr/libexec/gcc/darwin/ppc/3.1/cpp -xc++ -D__ppc__
cppC = /usr/libexec/gcc/darwin/ppc/3.3/cpp -xc++ -D__ppc__

# only within-program dependencies: avoid bug in overflow on no of included files
DEP_FLAG = -MM

# darwin does not support shared libs yet (apparently..)
#CC = $(CC_SHARED)
#CXX = $(CXX_SHARED)
#LIB_EXT = $(LIB_EXT_SHARED)
#AR_PRE_CMD = $(AR_PRE_CMD_SHARED)
#AR_CMD = $(AR_CMD_SHARED)
#RANLIB_CMD = $(RANLIB_CMD_SHARED)

TAR = tar
# no .user makefile
# ta_misc makefile for g++

##################################
# these are the things to set
#

# reset opt flags to not do anything for linux, crashes with dynamic linking
#OPT_FLAGS=

LIBS =  -ltamisc -livgraphic -lcss -ltypea -livmisc -lreadline -ltermcap -ltastring\
	$(IV_LIB) -lX11 $(IOS_LIB) $(IBERTY_LIB) $(GENERIC_LIB)

PROJNAME = ta_misc
PROJDIR  = ta_misc

EXE = test

LIB = libtamisc.$(LIB_EXT)

FLAGS =
MAKETA_FLAGS = -hx -css -instances $(FLAGS)

TAHEADS = $(VPATH)/win_base.h $(VPATH)/win_base_tmplt.h $(VPATH)/minmax.h \
	$(VPATH)/random.h $(VPATH)/tdgeometry.h $(VPATH)/aggregate.h\
	$(VPATH)/colorscale.h $(VPATH)/array_iv.h $(VPATH)/fontspec.h \
	$(VPATH)/datatable.h $(VPATH)/datatable_tmplt.h $(VPATH)/datatable_iv.h \
	$(VPATH)/ta_file.h $(VPATH)/script_base.h \
	$(VPATH)/fun_lookup.h

HEADS = win_base.h win_base_tmplt.h minmax.h aggregate.h fontspec.h \
	random.h tdgeometry.h colorscale.h colorbar.h array_iv.h\
	datatable.h datatable_tmplt.h ta_file.h script_base.h fun_lookup.h\
	axis.h picker.bm picker_mask.bm datagraph.h datatable_iv.h\
	$(PROJNAME)_TA_type.h $(PROJNAME)_TA_inst.h

SRCS =  win_base.cc minmax.cc random.cc tdgeometry.cc aggregate.cc fontspec.cc \
	colorscale.cc colorbar.cc datatable.cc datagraph.cc array_iv.cc\
	datatable_iv.cc	axis.cc ta_file.cc script_base.cc fun_lookup.cc test.cc\
	 $(PROJNAME)_TA.cc

OBJS =  win_base.o minmax.o random.o tdgeometry.o aggregate.o fontspec.o\
	colorscale.o colorbar.o datatable.o datagraph.o array_iv.o fun_lookup.o\
	datatable_iv.o axis.o ta_file.o script_base.o $(PROJNAME)_TA.o
EXEOBJS = test.o

DEPS = 	win_base.d minmax.d random.d tdgeometry.d aggregate.d fontspec.d \
	colorscale.d colorbar.d	datatable.d datagraph.d array_iv.d fun_lookup.d\
	axis.d ta_file.d script_base.d datatable_iv.d test.d $(PROJNAME)_TA.d

default: optLib
#default: dbgBin
#default: dbgLib
#####################################
# everything below here is standard..
#

# This makefile requires the availability of pattern rules to work properly
# though some are defined in suffix form, and you might be able to get by
# with just suffix rules if you don't mind recompiling things twice...

# the Makefile.in must supply:

# LIBS		libraries to include for the executable
# DEBUGLIBS	debugging versions of libraries to include for the executable
# EXTERNAL_LIBS needed libraries external to this directory
# PROJNAME	name of the project (for typea scanning)
# PROJDIR	name of the directory included (default is PROJNAME)
# PDPDIR	directory path to pdp library stuff (default is PROJNAME)
# VPATH		directory path to source code (default is $(TOP)/$(PROJDIR))
# EXE		name of the executable (default is PROJNAME)
# LIB		name of the library (default is libPROJNAME.LIB_EXT)
# IN_FLAGS	misc compilation flags
# MAKETA_FLAGS	flags for the maketa program
# TAHEADS	header files to be processed by typea
# HEADS		all of the header files (include TAHEADS & _TA_xxx_.h, if relevant)
# SRCS		all source files
# OBJS		all object files for the library 
# EXEOBJS	all object files for the executable
# DEPS		dependency files
# default	what to make by default (Lib, Bin, optLib, optBin)

# 'cmp' based touching of automatically-generated files:
# the .hx and .ccx files are automatically generated files that are produced
# whenever their source (typically a .y file or from 'maketa') is touched
# however, they can produce the same result even if the source has changed
# so they are compared with the corresponding .h or .cc file before these
# are updated with the new .hx or .ccx version (nothing is touched if not
# different).  This saves some compile time..

.SUFFIXES: .cc .hx .ccx .d

chkcpu:
	@if [ -z "$(CPU)" ];\
	then (echo "ERROR: CPU variable must be set!"; exit 1;) \
	fi

BasicFiles:	chkcpu
	@if [ ! -f NoMakeTA ];\
	then (if [ ! -f $(PROJNAME)_TA_type.h ]; then $(MAKE) force_ta ; fi;\
	if [ ! -f $(CPU)/$(PROJNAME)_TA.d -a ! -f $(PROJNAME)_TA.d ]; \
	then $(MAKE) depend ; fi; ) fi

lib:	Lib

Lib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  $(LIB) )\
	else $(MAKE)  $(LIB); fi

lib_pass2: LibPass2

LibPass2: BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  LibPass2_impl )\
	else $(MAKE)  LibPass2_impl; fi

bin:	Bin

Bin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  $(EXE) )\
	else $(MAKE)  $(EXE); fi

re_bin:	ReBin

ReBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE);$(MAKE)  $(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  $(EXE)); fi

re_lib:	ReLib

ReLib:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(LIB);$(MAKE)  Lib )\
	else ($(RM) $(LIB); $(MAKE)  Lib); fi

opt_lib:     optLib

optLib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) optLib_impl)\
	else $(MAKE) optLib_impl; fi

opt_bin:     optBin

optBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) optBin_impl)\
	else $(MAKE) optBin_impl; fi

opt_re_bin:  optReBin

optReBin: BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(RM) $(EXE); $(MAKE) optBin_impl)\
	else ($(RM) $(EXE); $(MAKE)  optBin_impl); fi

optBin_impl:
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(OPT_FLAGS)'

optLib_impl:
	$(MAKE)  $(LIB) 'OPT_DEBUG=$(OPT_FLAGS)'

dbg_lib:	 dbgLib

dbgLib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) dbgLib_impl)\
	else $(MAKE) dbgLib_impl; fi

dbg_bin:     dbgBin

dbgBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) dbgBin_impl)\
	else $(MAKE) dbgBin_impl; fi

dbgBin_impl:
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)'


iv_dbg_bin:	 ivdbgBin

ivdbgBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) ivdbgBin_impl)\
	else $(MAKE) ivdbgBin_impl; fi

ivdbgBin_impl:
	- $(RM) $(EXE)
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)' 'IV_LIB=-lIVd'

dbgLib_impl:
	$(MAKE)  $(LIB) 'OPT_DEBUG=$(DEBUG_FLAGS)'

proof_bin:	 proofBin

proofBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);\
	$(MAKE)  proofBin_impl)\
	else $(MAKE)  proofBin_impl; fi

proofBin_impl:
	$(MAKE)  proof$(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)'

dist_bin:	 distBin

distBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);\
	$(MAKE)  distBin_impl)\
	else $(MAKE)  distBin_impl; fi

distBin_impl:	      
	@echo "Making distribution Binary: " $(DIST_EXE) " of bin: " $(EXE)
	@echo $(DIST_EXE) " is now the default executable produced! "
#	- $(CP) $(EXE) $(DIST_EXE)

strip_bin:	stripBin

stripBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  stripBin_impl)\
	else $(MAKE)  stripBin_impl; fi

stripBin_impl:	      chkcpu 
	@echo "stripping binary: " $(EXE)
	- strip $(EXE)

proof$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ)
	$(PROOF) $(BIN_CXX)  -o $@ $(FLAGS) $(IN_FLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ)\
		$(TI_INST_OBJ) $(LIBS)

debug_bin:	DebugBin

DebugBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  debug$(EXE) )\
	else $(MAKE)  debug$(EXE); fi

re_debug_bin:	      ReDebugBin

ReDebugBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE); $(MAKE)  debug$(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  debug$(EXE)); fi

re_opt_bin:  ReOptBin

ReOptBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE); $(MAKE)  opt$(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  opt$(EXE));	fi

.cc.d:
	@$(DEP_CXX) $(DEP_FLAG) $(IDIRS) $(CCFLAGS) $< > $(MAKEDEP_TMP_FILE)
	@$(CAT) `echo $@ | $(SED) 's/\.d/.u/g'`  | $(SED) 's/$*\.o/& $@/g' > $@
	@$(RM)  `echo $@ | $(SED) 's/\.d/.u/g'`

.c.d:
	@$(DEP_CC) $(DEP_FLAG) $(IDIRS) $(CCFLAGS) $< > $(MAKEDEP_TMP_FILE)
	@$(CAT) `echo $@ | $(SED) 's/\.d/.u/g'`  | $(SED) 's/$*\.o/& $@/g' > $@
	@$(RM)  `echo $@ | $(SED) 's/\.d/.u/g'`

.c.o:
	@echo ""
	@echo  "Compiling: " $< "on"  `hostname | awk -F. '{printf $$1}'` in directory `pwd`  at `date`
	$(CC) -o $@ $(IDIRS) $(CFLAGS) -c $<

.cc.o:
	@echo ""
	@echo  "Compiling: " $< "on"  `hostname | awk -F. '{printf $$1}'`  in directory `pwd` at `date`
	$(CXX) -o $@ $(IDIRS) $(CCFLAGS) -c $< 

# _vt is the virtual table instantiation file that includes all headers
# and is compiled with the "build external vtables" flag
# (ie. cfront +e1).
# this file is typically named $(PROJNAME)_vt.cc (or empty if not used).
$(VT_INST_FILE).o:	$(VT_INST_FILE).cc
	$(CXX) -o $@ $(IDIRS) $(TA_CCFLAGS) $(VT_INST_FLAG) -c $< 

# why do we do this here?, it always erases any vt already there!
# $(VT_INST_FILE).cc:
# 	@echo "Creating a new virtual template instantiation file"
# 	@echo "// -*- C++ -*-" > $(VPATH)/$@
# 	@echo "// virtual table instantiation file" >> $(VPATH)/$@
# 	@echo " " >> $(VPATH)/$@
# 	@echo "// please include relevant files here.. " >> $(VPATH)/$@

# _ti is the template instantiation file that explicitly instantiates
# all templates used in the software. this is required by g++2.6.3
# this file is typically named $(PROJNAME)_ti.cc (or empty if not used).
$(TI_INST_FILE).o:	$(TI_INST_FILE).cc
	$(CXX) -o $@ $(IDIRS) $(CCFLAGS) $(TI_INST_FLAG) -c $< 

$(TI_INST_FILE).cc:
	echo "// -*- C++ -*-" > $(VPATH)/$@
	echo "// template instantiation file" >> $(VPATH)/$@
	echo " " >> $(VPATH)/$@
	echo "// please include relevant files and instantiate templates here.. " >> $(VPATH)/$@

# these two pattern dependencies are necessary to make the files in the
# source directory depend on auto-generated files in the local directory
$(SRC_INC)/%.hx : %.hx
	@echo ".hx reduction"

$(VPATH)/%.hx : %.hx
	@echo ".hx reduction"

.hx.h:
	@- $(CMP) $(<F) $(@F) || ($(CP) $(<F) $(@F))
	@- $(CMP) $(<F) $(VPATH)/$(<F) || ($(CP) $(<F) $(VPATH)/$(<F))
	@- $(CMP) $(@F) $(VPATH)/$(@F) || ($(CP) $(@F) $(VPATH)/$(@F))

.ccx.cc:
	@- $(CMP) $(<F) $(@F) || ($(CP) $(<F) $(@F))
	@- $(CMP) $(<F) $(VPATH)/$(<F) || ($(CP) $(<F) $(VPATH)/$(<F))
	@- $(CMP) $(@F) $(VPATH)/$(@F) || ($(CP) $(@F) $(VPATH)/$(@F))

# cancel the direct route
.y.o:

.y.cc:
	$(YACC) $(YFLAGS) $<
	$(MV) y.tab.c $(*F).cc
	$(MV) y.tab.h $(*F).hx
	- $(MV) y.output $(*F).output
	$(SED) -e "s/union/struct/g" $(*F).cc >ttt.uts;\
	$(RM) $(*F).cc; $(MV) ttt.uts $(*F).cc;
	$(SED) -e "s/union/struct/g" $(*F).hx >ttt.uts;\
	 $(RM) $(*F).hx; $(MV) ttt.uts $(*F).hx;
	- $(CP) $(*F).cc $(VPATH)/$(*F).cc
	- $(CP) $(*F).hx $(VPATH)/$(*F).hx

$(LIB): $(OBJS) $(TI_INST_OBJ)
	$(RM) $@
	@- $(AR_PRE_CMD) $(OBJS)
	$(AR_CMD) $@ $(OBJS) $(TI_INST_OBJ)
	- $(RANLIB_CMD) $@
	- if [ $(LIB_EXT) != a ]; then $(MV) lib_include/$@ lib_include/$@.OLD.$$$$; fi
	$(CP) $@ lib_include
	- $(RANLIB_CMD) -t lib_include/$@
	for i in $(HEADS);\
	  do $(CMP) $(VPATH)/$$i lib_include/$$i || $(CP) $(VPATH)/$$i lib_include;\
	done

LibPass2_impl: $(OBJS) $(TI_INST_OBJ)
	$(LIB_PASS2_CMD)

clean_libs:	chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_libs_impl )\
	else $(MAKE)  clean_libs_impl; fi

clean_libs_impl: chkcpu clean_old_libs_impl
	- $(RM) lib*.a lib*.s[ol]* lib_include/lib*.a \
	lib_include/lib*.s[ol]*

clean_old_libs:	chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_old_libs_impl )\
	else $(MAKE)  clean_old_libs_impl; fi

clean_old_libs_impl:  chkcpu 
	- $(RM) lib_include/lib*.OLD*

new_lib_h:	chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  new_lib_h_impl )\
	else $(MAKE)  new_lib_h_impl; fi

new_lib_h_impl:	      chkcpu 
	for i in $(HEADS);\
	  do $(CMP) $(VPATH)/$$i lib_include/$$i || $(CP) $(VPATH)/$$i lib_include;\
	done

$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ) $(EXE_DEPENDS)
	@echo ""
	@echo "Compiling: " $(EXE) "on"  `hostname | awk -F. '{printf $$1}'`  in directory `pwd` at `date`
	- $(RM) -rf ptrepository
	$(BIN_CXX) -o $@ $(CCFLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ) \
		$(TI_INST_OBJ) $(LIBS)

debug$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ) $(EXE_DEPENDS)
	@echo ""
	@echo "Compiling: " Debug version $(EXE) "on" `hostname | awk -F. '{printf $$1}'`  in directory `pwd` at `date`
	$(BIN_CXX) -o $(EXE) $(CCFLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ) \
		$(TI_INST_OBJ) $(DEBUGLIBS)

ta:		chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE)  ta_impl )\
	else $(MAKE)  ta_impl; fi

ta_impl: $(VPATH)/$(PROJNAME)_TA_type.h $(PROJNAME)_TA.cc $(VPATH)/$(PROJNAME)_TA_inst.h
	- touch $(OBJS)
	- rm  $(PROJNAME)_TA.o

force_ta: chkcpu 
	@if [ ! -f NoMakeTA ];\
	then ( if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  force_ta_impl )\
	else $(MAKE)  force_ta_impl; fi); fi

force_ta_impl:	      chkcpu 
	- $(MAKETA) $(MAKETA_FLAGS_XX) $(IDIRS) $(PROJNAME) $(TAHEADS)
	- $(CP) $(PROJNAME)_TA_type.hx $(PROJNAME)_TA.ccx $(PROJNAME)_TA_inst.hx $(VPATH)
	- $(MAKE)  $(PROJNAME)_TA_type.h
	- $(MAKE)  $(PROJNAME)_TA_inst.h
	- $(MAKE)  $(PROJNAME)_TA.cc

no_ta:	chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE)  no_ta_impl )\
	else $(MAKE)  no_ta_impl; fi

no_ta_impl:  chkcpu 
	- touch $(VPATH)/$(PROJNAME)_TA_type.hx $(VPATH)/$(PROJNAME)_TA.ccx $(VPATH)/$(PROJNAME)_TA_inst.hx
	- touch $(PROJNAME)_TA_type.hx $(PROJNAME)_TA.ccx $(PROJNAME)_TA_inst.hx

$(PROJNAME)_TA_type.hx $(PROJNAME)_TA.ccx $(PROJNAME)_TA_inst.hx: $(TAHEADS)
	$(MAKETA) $(MAKETA_FLAGS_XX) $(IDIRS) $(PROJNAME) $(TAHEADS)
	- $(CP) $(PROJNAME)_TA_type.hx $(PROJNAME)_TA.ccx $(PROJNAME)_TA_inst.hx $(VPATH)

# this has to compile without optimization or symbols
$(PROJNAME)_TA.o: 	$(PROJNAME)_TA.cc
	@echo ""
	@echo "Compiling: " $< on  `hostname | awk -F. '{printf $$1}'` in directory `pwd`  in directory `pwd` at `date`
	$(CXX) -o $(PROJNAME)_TA.o $(IDIRS) $(TA_CCFLAGS) -c $(PROJNAME)_TA.cc 

src_tar:	chkcpu 
	@echo " "
	@echo "** Making source distribution tar file **"
	@echo " "
	- $(RM) $(PROJNAME)_src.tar*
	$(CD) ..;\
	$(TAR) -cf $(PROJDIR)/$(PROJNAME)_src.tar.gz -z --ignore-failed-read \
		$(PROJDIR)/*.h $(PROJDIR)/*.hx $(PROJDIR)/*.ccx \
		$(PROJDIR)/*.cc $(PROJDIR)/*.css $(PROJDIR)/Makefile.in \
		$(PROJDIR)/Makefile $(PROJDIR)/*.def
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

bin_tar:	chkcpu 
	@echo " "
	@echo "** Making binary distribution tar file **"
	@echo " "
	- $(RM) $(PROJNAME)_bin_$(CPU).tar.gz
	$(CD) ..;\
	$(TAR) -cf $(PROJDIR)/$(PROJNAME)_bin_$(CPU).tar.gz -z --ignore-failed-read \
		$(PROJDIR)/$(CPU)/$(EXE) $(PROJDIR)/*.def
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

install_bin:	chkcpu
	@echo " "
	@echo "** Make links in BIN_DIR for executable ($@) **"
	@echo " "
	$(CD) $(BIN_DIR);\
	$(RM) $(EXE);\
	$(LN) $(VPATH)/$(CPU)/$(EXE) $(EXE)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

install_dbg_bin:	chkcpu
	@echo " "
	@echo "** Make links in BIN_DIR for executable with dbg_ prefix ($@) **"
	@echo " "
	$(CD) $(BIN_DIR);\
	$(RM) dbg_$(EXE);\
	$(LN) $(VPATH)/$(CPU)/$(EXE) dbg_$(EXE)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

backup:		chkcpu 
	@echo "Backing up the directory"
	-@ $(MKDIR) oldcode
	-@ $(MKDIR) `date "+oldcode/%H-%d-%h-%y"`
	-@ $(RM) oldcode/mostrecent
	-@ $(CD) oldcode; $(LN) `date "+%H-%d-%h-%y"` mostrecent
	- $(CP) *.cc oldcode/mostrecent
	- $(CP) *.h oldcode/mostrecent
	- $(CP) *.y oldcode/mostrecent
	- $(CP) *.css oldcode/mostrecent
	- $(CP) *.def oldcode/mostrecent
	- $(CP) *akefile* oldcode/mostrecent
	- $(CP) .*rc oldcode/mostrecent
	-@ $(COMPRESS) oldcode/mostrecent/* &

clean:	chkcpu clean_impl
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_impl )\
	else $(MAKE)  clean_impl; fi

clean_impl:  chkcpu 
	- $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz

real_clean:	chkcpu 
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  real_clean_impl )\
	else $(MAKE)  real_clean_impl; fi

real_clean_impl: chkcpu clean_impl
	- $(RM) *.ccx *.d

dist_clean:	chkcpu clean_impl
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  dist_clean_impl )\
	else $(MAKE)  dist_clean_impl; fi

dist_clean_impl: chkcpu real_clean_impl
	- $(RM) lib*.$(LIB_EXT) $(EXE) $(EXE).old $(EXE).new

src_clean:	chkcpu clean_impl

wipe:  chkcpu
	- $(RM) *~ #* *%

touch:	chkcpu
	- $(TOUCH) $(HEADS) $(SRCS)

touch_objs:	chkcpu
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  touch_objs_impl )\
	else $(MAKE)  touch_objs_impl; fi

touch_objs_impl: chkcpu
	- $(TOUCH) $(OBJS) $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ)

make_depend:	   MakeDepend

MakeDepend:	chkcpu cpu_dir depend

new_make_depend:       NewMakeDepend

NewMakeDepend:	chkcpu Makefiles depend

makefiles:	Makefiles

# note the following should not be used when make will not allow a new makefile
# to be run from within the old one!
# in this case, you must make 'NewMakefile' and then 'cpu_dir' in two stages
Makefiles:	chkcpu
	-@$(MAKE)  NewMakefile; $(MAKE)  cpu_dir

new_makefile:	   NewMakefile

NewMakefile: chkcpu
	-@ $(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
	  $(TOP)/Makefile.user Makefile.in $(PDPDIR)/config/Makefile.std >Makefile

cpu_dir:	chkcpu
	-@$(MKDIR) src_include 1>/dev/null 2>&1;\
	$(CD) src_include; $(LN) ../ $(PROJDIR) 1>/dev/null 2>&1;\
	$(CD) ..;\
	$(MKDIR) $(CPU) 1>/dev/null 2>&1;\
	$(CD) $(CPU);\
	$(RM) Makefile 1>/dev/null 2>&1;\
	$(CP) $(VPATH)/Makefile .;\
	$(RM) src_include;\
	$(LN) $(VPATH)/src_include src_include ;\
	$(RM) $(VPATH)/src_include/src_include 1>/dev/null 2>&1;\
	$(MKDIR) lib_include 1>/dev/null 2>&1;\
	for i in $(HEADS);\
	  do $(CMP) $(VPATH)/$$i lib_include/$$i || $(CP) $(VPATH)/$$i lib_include;\
	done;\
	if [ ! -f $(VPATH)/NoMakeTA ];\
	then ($(CMP) $(VPATH)/$(PROJNAME)_TA_type.hx $(PROJNAME)_TA_type.hx \
	 || ($(RM) $(PROJNAME)_TA_type.hx $(PROJNAME)_TA_type.h ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA_type.hx $(PROJNAME)_TA_type.hx ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA_type.h $(PROJNAME)_TA_type.h ) ;\
	$(CMP) $(VPATH)/$(PROJNAME)_TA_inst.hx $(PROJNAME)_TA_inst.hx \
	 || ($(RM) $(PROJNAME)_TA_inst.hx $(PROJNAME)_TA_inst.h ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA_inst.hx $(PROJNAME)_TA_inst.hx ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA_inst.h $(PROJNAME)_TA_inst.h );\
	$(CMP) $(VPATH)/$(PROJNAME)_TA.ccx $(PROJNAME)_TA.ccx \
	 || ($(RM) $(PROJNAME)_TA.ccx $(PROJNAME)_TA.cc ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA.ccx $(PROJNAME)_TA.ccx ;\
	     $(CP) $(VPATH)/$(PROJNAME)_TA.cc $(PROJNAME)_TA.cc ) )\
	fi;\
	if [ -f $(VPATH)/$(EXTRA_HX_FILE).hx ];\
	then ($(CMP) $(VPATH)/$(EXTRA_HX_FILE).hx $(EXTRA_HX_FILE).hx \
	 || ($(RM) $(EXTRA_HX_FILE).hx $(EXTRA_HX_FILE).h ;\
	     $(CP) $(VPATH)/$(EXTRA_HX_FILE).hx $(EXTRA_HX_FILE).hx ;\
	     $(CP) $(VPATH)/$(EXTRA_HX_FILE).h $(EXTRA_HX_FILE).h ) ) fi;\
	if [ -f $(VPATH)/$(EXTRA_CPU_FILE) ];\
	then ( $(CP) $(VPATH)/$(EXTRA_CPU_FILE) $(EXTRA_CPU_FILE) ) fi

local_cpu_dir: chkcpu
	-@$(MKDIR) $(LOC_OBJS)/$(PROJNAME) 1>/dev/null 2>&1;\
	$(CD) $(LOC_OBJS)/$(PROJNAME);\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
	  $(PDPDIR)/config/Makefile.init > Makefile;\
	$(CP) $(TOP)/$(PROJNAME)/Makefile.in Makefile.in;\
	$(MKDIR) $(CPU) 1>/dev/null 2>&1;\
	$(CD) $(TOP)/$(PROJNAME);\
	$(RM) -r $(CPU);\
	$(LN) $(LOC_OBJS)/$(PROJNAME)/$(CPU) $(CPU) 1>/dev/null 2>&1;\
	$(CD) $(LOC_OBJS)/$(PROJNAME);\
	$(LN) $(TOP)/$(PROJNAME)/src_include src_include 1>/dev/null 2>&1;\
	$(CD) $(TOP)/$(PROJNAME);\
	$(MAKE)  Makefiles

rm_cpu_dir:	chkcpu
	- $(RM) -r $(CPU)
	- $(RM) -r $(LOC_OBJS)/$(PROJNAME)/$(CPU)

depend:	chkcpu
	@echo "Making Dependencies"
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(RM) *.d; $(MAKE)  depend_impl )\
	else ( $(RM) *.d; $(MAKE)  depend_impl ) fi

# this is much easier, but not guaranteed to work..
#depend_impl:
#	makedepend -f Makefile $(IDIRS) $(VPATH)/*.cc

# this works always..
depend_impl:	$(DEPS_XX)
	@echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	@echo 'w' >>eddep
	@$(CP) Makefile Makefile.bak
	@$(ED) - Makefile < eddep
	@$(CAT) $(DEPS_XX) >>Makefile
#	@$(RM) eddep
	@if [ ! -f $(PROJNAME)_TA.d ]; then echo "# no dependencies" >$(PROJNAME)_TA.d ; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.
win_base.o win_base.d: /usr/local/pdp++/src/ta_misc/win_base.cc \
  src_include/ta_misc/win_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  src_include/ta_misc/win_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h \
  src_include/ta_misc/script_base.h src_include/ta_misc/ta_file.h \
  /usr/local/pdp++/include/DARWINdebug/css/css_iv.h
minmax.o minmax.d: /usr/local/pdp++/src/ta_misc/minmax.cc \
  src_include/ta_misc/minmax.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h
random.o random.d: /usr/local/pdp++/src/ta_misc/random.cc \
  src_include/ta_misc/random.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/css/special_math.h
tdgeometry.o tdgeometry.d: /usr/local/pdp++/src/ta_misc/tdgeometry.cc \
  src_include/ta_misc/tdgeometry.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h
aggregate.o aggregate.d: /usr/local/pdp++/src/ta_misc/aggregate.cc \
  src_include/ta_misc/aggregate.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h
fontspec.o fontspec.d: /usr/local/pdp++/src/ta_misc/fontspec.cc \
  src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_text.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/css/ta_css.h \
  /usr/local/pdp++/include/DARWINdebug/css/machine.h \
  /usr/local/pdp++/include/DARWINdebug/css/extern_support.h \
  /usr/local/pdp++/include/DARWINdebug/css/css_parse.h \
  /usr/local/pdp++/include/DARWINdebug/css/basic_types.h
colorscale.o colorscale.d: /usr/local/pdp++/src/ta_misc/colorscale.cc \
  src_include/ta_misc/colorscale.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h
colorbar.o colorbar.d: /usr/local/pdp++/src/ta_misc/colorbar.cc \
  src_include/ta_misc/colorbar.h src_include/ta_misc/colorscale.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  src_include/ta_misc/painter.bm src_include/ta_misc/painter_mask.bm \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dastepper.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h
datatable.o datatable.d: /usr/local/pdp++/src/ta_misc/datatable.cc \
  src_include/ta_misc/datatable.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/tdgeometry.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/colorscale.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  src_include/ta_misc/datatable_tmplt.h src_include/ta_misc/datagraph.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  src_include/ta_misc/colorbar.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  src_include/ta_misc/datatable_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_text.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h
datagraph.o datagraph.d: /usr/local/pdp++/src/ta_misc/datagraph.cc \
  src_include/ta_misc/datagraph.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  src_include/ta_misc/datatable.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/tdgeometry.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/colorscale.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  src_include/ta_misc/datatable_tmplt.h src_include/ta_misc/colorbar.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_text.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dastepper.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/grid.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/requestbox.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/tbScrollBox.h \
  src_include/ta_misc/axis.h
array_iv.o array_iv.d: /usr/local/pdp++/src/ta_misc/array_iv.cc \
  src_include/ta_misc/array_iv.h src_include/ta_misc/colorbar.h \
  src_include/ta_misc/colorscale.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h
fun_lookup.o fun_lookup.d: /usr/local/pdp++/src/ta_misc/fun_lookup.cc \
  src_include/ta_misc/fun_lookup.h src_include/ta_misc/datatable.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/tdgeometry.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/colorscale.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  src_include/ta_misc/datatable_tmplt.h
axis.o axis.d: /usr/local/pdp++/src/ta_misc/axis.cc src_include/ta_misc/axis.h \
  src_include/ta_misc/minmax.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h
ta_file.o ta_file.d: /usr/local/pdp++/src/ta_misc/ta_file.cc \
  src_include/ta_misc/ta_file.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h
script_base.o script_base.d: /usr/local/pdp++/src/ta_misc/script_base.cc \
  src_include/ta_misc/script_base.h src_include/ta_misc/win_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  src_include/ta_misc/win_base_tmplt.h src_include/ta_misc/ta_file.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h \
  /usr/local/pdp++/include/DARWINdebug/css/ta_css.h \
  /usr/local/pdp++/include/DARWINdebug/css/machine.h \
  /usr/local/pdp++/include/DARWINdebug/css/extern_support.h \
  /usr/local/pdp++/include/DARWINdebug/css/css_parse.h \
  /usr/local/pdp++/include/DARWINdebug/css/basic_types.h \
  /usr/local/pdp++/include/DARWINdebug/css/css_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/lrScrollBox.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/tbScrollBox.h
datatable_iv.o datatable_iv.d: /usr/local/pdp++/src/ta_misc/datatable_iv.cc \
  src_include/ta_misc/datatable_iv.h src_include/ta_misc/colorscale.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h src_include/ta_misc/colorbar.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  src_include/ta_misc/datatable.h src_include/ta_misc/tdgeometry.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/datatable_tmplt.h src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_text.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/lrScrollBox.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/tbScrollBox.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dastepper.h
test.o test.d: /usr/local/pdp++/src/ta_misc/test.cc \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/lrScrollBox.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/tbScrollBox.h \
  src_include/ta_misc/picker.bm src_include/ta_misc/picker_mask.bm \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/scrollable.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h
ta_misc_TA.o ta_misc_TA.d: ta_misc_TA.cc \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea_constr.h \
  /usr/local/pdp++/include/DARWINdebug/css/basic_types.h \
  /usr/local/pdp++/include/DARWINdebug/css/machine.h \
  /usr/local/pdp++/include/DARWINdebug/css/extern_support.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/css/css_parse.h \
  /usr/local/pdp++/include/DARWINdebug/css/c_ptr_types.h \
  /usr/local/pdp++/include/DARWINdebug/css/ta_css.h \
  src_include/ta_misc/win_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h \
  src_include/ta_misc/win_base_tmplt.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h src_include/ta_misc/minmax.h \
  src_include/ta_misc/random.h \
  /usr/local/pdp++/include/DARWINdebug/css/special_math.h \
  src_include/ta_misc/tdgeometry.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/colorscale.h src_include/ta_misc/array_iv.h \
  src_include/ta_misc/colorbar.h src_include/ta_misc/colorscale.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h \
  src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  src_include/ta_misc/datatable.h src_include/ta_misc/tdgeometry.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/datatable_tmplt.h src_include/ta_misc/datatable.h \
  src_include/ta_misc/datatable_tmplt.h \
  src_include/ta_misc/datatable_iv.h src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_text.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/dynalabel.h \
  src_include/ta_misc/ta_file.h src_include/ta_misc/script_base.h \
  src_include/ta_misc/ta_file.h src_include/ta_misc/fun_lookup.h \
  ta_misc_TA_type.h ta_misc_TA_inst.h
ta_misc_ti.o ta_misc_ti.d: /usr/local/pdp++/src/ta_misc/ta_misc_ti.cc \
  src_include/ta_misc/tdgeometry.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base.h \
  /usr/local/pdp++/include/DARWINdebug/ta/typea.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_stdef.h \
  /usr/local/pdp++/include/DARWINdebug/ta_string/ta_string.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_list_tmplt.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_base_tmplt.h \
  src_include/ta_misc/ta_misc_TA_type.h src_include/ta_misc/datagraph.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_type.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_data.h \
  /usr/local/pdp++/include/DARWINdebug/ta/enter_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/leave_iv.h \
  src_include/ta_misc/datatable.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_tmplt.h \
  src_include/ta_misc/minmax.h src_include/ta_misc/aggregate.h \
  src_include/ta_misc/colorscale.h src_include/ta_misc/win_base.h \
  src_include/ta_misc/win_base_tmplt.h \
  src_include/ta_misc/datatable_tmplt.h src_include/ta_misc/colorbar.h \
  /usr/local/pdp++/include/DARWINdebug/iv_misc/spiral.h \
  src_include/ta_misc/fontspec.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/iv_graphic_TA_type.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_objs.h \
  /usr/local/pdp++/include/DARWINdebug/iv_graphic/graphic_viewer.h \
  src_include/ta_misc/script_base.h src_include/ta_misc/ta_file.h \
  /usr/local/pdp++/include/DARWINdebug/ta/ta_group_iv.h \
  /usr/local/pdp++/include/DARWINdebug/ta/taiv_dialog.h
