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

# g++ needs a stream library (don't use all of libg++!)
IOS_LIB = -liostream

# if not using g++, need to use 'myiberty' which has alloca, xmalloc
IBERTY_LIB = -liberty
# IBERTY_LIB = -lmyiberty

# any other misc libs that need to be included
GENERIC_LIB = -lm

#default InterViews Library
IV_LIB = -lIV

# this must correspond with the version of the iostream library used
# its for ta which scans modified versions of these include files
# either Cfront or g++ are supported (and the corresponding files
# are found in the src/ta directory.  Note that the CC (cfront) ones are
# symbolic links to /usr/include/CC, and the g++ ones are just copied
# from the 2.6.2 libg++ distribution.
# IOS_INCLUDES = CC-3.1
# IOS_INCLUDES = g++-2.6
IOS_INCLUDES = g++-2.7.1

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

# basic compiling command for shared and non-shared making
# (definitely need PIC or its equivalent for a large GOT)
CC_NONSHARED = gcc -nostdinc
CXX_NONSHARED = g++ -fno-implicit-templates
CC_SHARED = gcc -nostdinc -fPIC
CXX_SHARED = g++ -fPIC -fno-implicit-templates

# extra flags for compiling
MACH_FLAGS =
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
AR_CMD_SHARED = $(CXX_SHARED) -shared -o 
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

# if using shared libs and it costs alot in performance, don't do it
# for the core simulation code (pdp,bp,cs,so,bpso)
# so define these to be non-shared compile flags and archiving, etc
# however, if doing shared and its ok to do shared for all, set to shared.
# they are set to nonshared by default because the default overall is nonshared
SIM_NONSHARED_CXX = $(CXX_NONSHARED)
SIM_NONSHARED_AR_CMD = $(AR_CMD_NONSHARED)
SIM_NONSHARED_LIB_EXT = $(LIB_EXT_NONSHARED)

DEBUG_FLAG = -g -Wparentheses -Wreturn-type -Wunused -Wreorder
# wall is too obnoxious in 2.7.0 (signed-vs-unsigned comparisons..)
# DEBUG_FLAG = -g -Wall -Winline
# debug flags that can be used with optimization
DEBUG_OPT_FLAG = 
# -g tends to eat up a huge amount of space..
#DEBUG_OPT_FLAG = -g
OPT_FLAG = -O2

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
#TI_INST_SRC = $(TI_INST_FILE).cc
#TI_INST_OBJ = $(TI_INST_FILE).o
#TI_INST_DEP = $(TI_INST_FILE).d
TI_INST_SRC = 
TI_INST_OBJ = 
TI_INST_DEP = 

################################
# less-important compiler stuff

# pipe tends to take too much ram...
#PIPEFLAG = -pipe
PIPEFLAG = 

# these defines are passed to every compile stage
DEFINES =
DEFINES_XX = -D$(CPU) $(DEFINES)

# these are the basic flags for compiling C++
CCFLAGS_XX = $(FLAGS) $(MACH_FLAGS) $(PIPEFLAG) $(DEFINES_XX)
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
TA_CCFLAGS_XX = $(FLAGS) $(MACH_TA_CCFLAGS) $(DEFINES_XX)

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
LDIRS_XX = -L$(PDPDIR)/lib/$(CPU) -L$(LOCAL)/lib

# this is what is actually used when compiling
IDIRS = $(IDIRS_EXTRA) $(IDIRS_XX) 
LDIRS = $(LDIRS_EXTRA) $(LDIRS_XX) 

# the name of the executable made in this directory
EXE = $(PROJNAME)++
# this is the 'distribution' executable (i.e. the one that works, not the latest)
DIST_EXE = $(PROJNAME)++

# this is the name of the library made in this directory
LIB = lib$(PROJNAME).$(LIB_EXT)

# extra dependencies for the executable
EXE_DEPENDS =

# these are the basic dependencies 
DEPS_XX = $(DEPS) $(VT_INST_DEP) $(TI_INST_DEP)

# path to the 'maketa' type scanning program
MAKETA = $(PDPDIR)/bin/$(CPU)/maketa

MAKETA_FLAGS_XX = "-cpp=$(cppC)" $(MAKETA_FLAGS_CPU) $(MAKETA_FLAGS) $(FLAGS) $(DEFINES_XX) 

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
# LINUX - g++ (ELF)
####################################################

# this version does everything static, and optimized!  
# the extra speed is worth the extra cost in binary size, at least
# on my current system..

# define this to be where the libg++ (stream classes) headers
# are found
LIBG++_INCLUDE_DIR = -I/usr/include/linux -I/usr/include/g++

MACH_FLAGS = -fno-implicit-templates
MACH_TA_CCFLAGS = $(MACH_FLAGS)
MAKETA_FLAGS = -hx -css -instances $(LIBG++_INCLUDE_DIR)

IOS_LIB = 
IBERTY_LIB =

# needs template instantiation files..
TI_INST_SRC = $(TI_INST_FILE).cc
TI_INST_OBJ = $(TI_INST_FILE).o
TI_INST_DEP = $(TI_INST_FILE).d

IOS_INCLUDES = g++-2.7.1

#lets try with the new gcc cpp
# /usr/include headerfiles of LINUX expect __i386__ to be defined
#cppC = /usr/local/bin/cccp -D__i386__
cppC = cpp -D__i386__  $(LIBG++_INCLUDE_DIR)

# LINUX mallocinfo from ta requires mstats from libg.a
# disabled for now
#GENERIC_LIB = -lm -lg
GENERIC_LIB = -lm

IDIRS_EXTRA = -I/usr/include/g++
LDIRS_EXTRA = -L/usr/X11R6/lib/

# no libg++ for binaries 
#BIN_CXX = $(CC)

#AR_CMD_SHARED = g++ -shared -Wl,-soname,$@ -o

# *NOT* using shared libraries
#CC = $(CC_SHARED)
#CXX = $(CXX_SHARED)
#LIB_EXT = $(LIB_EXT_SHARED)
#AR_PRE_CMD = $(AR_PRE_CMD_SHARED)
#AR_CMD = $(AR_CMD_SHARED)
#RANLIB_CMD = $(RANLIB_CMD_SHARED)

# optimization is not compatible with shared
#SIM_NONSHARED_CXX = $(CXX_SHARED)
#SIM_NONSHARED_AR_CMD = $(AR_CMD_SHARED)
#SIM_NONSHARED_LIB_EXT = $(LIB_EXT_SHARED)

TAR = tar
#############################################################
# defines settings for makefiles in user's directory
# 	(i.e. not in the pdp++ main directory)
#
# this is put in the location defined by $(TOP) given below by the 
# script bin/mknewpdp, and is incorporated into the makefile constructed
# in the individual directories below this $(TOP) by the NewMakefiles
# make target, which automatically looks here for these definitions
#

# this must be set to point to the location where pdp++ was actually
# installed!!
PDPDIR = /usr/local/pdp++

# this must be set to point to the location where this file is actually
# located.  
TOP = $(HOME)/mypdp++

# this can optionally be set to a location which will be local to each
# machine, so that the object files can be located there instead of on
# a networked disk, which is slow for compiling.  this is only used
# if the 'make local_cpu_dir' action is performed.
LOC_OBJS = $(HOME)/pdp++_objs
# bleabra makefile for pdp++

##################################
# these are the things to set
#

# this directory should be optimized for speed, so if shared libs slow it
# down, don't use them here
CXX = $(SIM_NONSHARED_CXX)
AR_CMD = $(SIM_NONSHARED_AR_CMD)
LIB_EXT = $(SIM_NONSHARED_LIB_EXT)

LIBS = -lpdp -ltamisc -livgraphic -lcss -ltypea -livmisc -lreadline -ltermcap\
	 -ltastring $(IV_LIB) -lX11 $(IOS_LIB) $(IBERTY_LIB) $(GENERIC_LIB)

PROJNAME = bleabra

MAKETA_FLAGS = -hx -css -instances

TAHEADS = $(VPATH)/biobel.h $(VPATH)/bleabra.h
HEADS = biobel.h bleabra.h $(PROJNAME)_TA_type.h $(PROJNAME)_TA_inst.h

SRCS = 	biobel.cc bleabra.cc $(PROJNAME)_TA.cc main.cc

OBJS =  biobel.o bleabra.o $(PROJNAME)_TA.o

EXEOBJS = $(OBJS) main.o

DEPS = 	biobel.d bleabra.d $(PROJNAME)_TA.d main.d

# include pdp in library name 
LIB = libpdp_$(PROJNAME).$(LIB_EXT)

#default: dbgBin
default: optBin

# for a long-standing (long-running) executable
longBin:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);\
	$(MAKE) longBin_impl)\
	else $(MAKE) longBin_impl; fi

longBin_impl:
	@echo "Making long-standing Binary: " $(EXE).long " of bin: " $(EXE)
	- $(CP) $(EXE) $(EXE).long

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
# FLAGS		misc compilation flags
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

BasicFiles:
	@if [ ! -f NoMakeTA ];\
	then (if [ ! -f $(PROJNAME)_TA_type.h ]; then $(MAKE) force_ta ; fi;\
	if [ ! -f $(CPU)/$(PROJNAME)_TA.d -a ! -f $(PROJNAME)_TA.d ]; \
	then $(MAKE) depend ; fi; ) fi

Lib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  $(LIB) )\
	else $(MAKE)  $(LIB); fi

LibPass2: BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  LibPass2_impl )\
	else $(MAKE)  LibPass2_impl; fi

Bin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  $(EXE) )\
	else $(MAKE)  $(EXE); fi

ReBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE);$(MAKE)  $(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  $(EXE)); fi

ReLib:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(LIB);$(MAKE)  Lib )\
	else ($(RM) $(LIB); $(MAKE)  Lib); fi

optLib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) optLib_impl)\
	else $(MAKE) optLib_impl; fi

optBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) optBin_impl)\
	else $(MAKE) optBin_impl; fi

optReBin: BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(RM) $(EXE); $(MAKE) optBin_impl)\
	else ($(RM) $(EXE); $(MAKE)  optBin_impl); fi

optBin_impl:
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(OPT_FLAGS)'

optLib_impl:
	$(MAKE)  $(LIB) 'OPT_DEBUG=$(OPT_FLAGS)'

dbgLib:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) dbgLib_impl)\
	else $(MAKE) dbgLib_impl; fi

dbgBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) dbgBin_impl)\
	else $(MAKE) dbgBin_impl; fi

dbgBin_impl:
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)'

ivdbgBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE) ivdbgBin_impl)\
	else $(MAKE) ivdbgBin_impl; fi

ivdbgBin_impl:
	- $(RM) $(EXE)
	$(MAKE)  $(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)' 'IV_LIB=-lIVd'

dbgLib_impl:
	$(MAKE)  $(LIB) 'OPT_DEBUG=$(DEBUG_FLAGS)'

proofBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);\
	$(MAKE)  proofBin_impl)\
	else $(MAKE)  proofBin_impl; fi

proofBin_impl:
	$(MAKE)  proof$(EXE) 'OPT_DEBUG=$(DEBUG_FLAGS)'

distBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);\
	$(MAKE)  distBin_impl)\
	else $(MAKE)  distBin_impl; fi

distBin_impl:
	@echo "Making distribution Binary: " $(DIST_EXE) " of bin: " $(EXE)
	@echo $(DIST_EXE) " is now the default executable produced! "
#	- $(CP) $(EXE) $(DIST_EXE)

stripBin:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  stripBin_impl)\
	else $(MAKE)  stripBin_impl; fi

stripBin_impl:
	@echo "stripping binary: " $(EXE)
	- strip $(EXE)

proof$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ)
	$(PROOF) $(BIN_CXX)  -o $@ $(FLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ)\
		$(TI_INST_OBJ) $(LIBS)

DebugBin:	BasicFiles
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  debug$(EXE) )\
	else $(MAKE)  debug$(EXE); fi

ReDebugBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE); $(MAKE)  debug$(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  debug$(EXE)); fi

ReOptBin:	BasicFiles
	@if [ -d $(CPU) ]; then ($(CD) $(CPU); $(RM) $(EXE); $(MAKE)  opt$(EXE) )\
	else ($(RM) $(EXE); $(MAKE)  opt$(EXE));	fi

.cc.d:
	@$(DEP_CXX) -M -E $(IDIRS) $(CCFLAGS) $< > $(MAKEDEP_TMP_FILE)
	@$(CAT) `echo $@ | $(SED) 's/\.d/.u/g'`  | $(SED) 's/$*\.o/& $@/g' > $@
	@$(RM)  `echo $@ | $(SED) 's/\.d/.u/g'`

.c.d:
	@$(DEP_CC) -M -E $(IDIRS) $(CCFLAGS) $< > $(MAKEDEP_TMP_FILE)
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

clean_libs:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_libs_impl )\
	else $(MAKE)  clean_libs_impl; fi

clean_libs_impl: clean_old_libs_impl
	- $(RM) lib*.a lib*.s[ol]* lib_include/lib*.a \
	lib_include/lib*.s[ol]*

clean_old_libs:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_old_libs_impl )\
	else $(MAKE)  clean_old_libs_impl; fi

clean_old_libs_impl:
	- $(RM) lib_include/lib*.OLD*

new_lib_h:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  new_lib_h_impl )\
	else $(MAKE)  new_lib_h_impl; fi

new_lib_h_impl:
	for i in $(HEADS);\
	  do $(CMP) $(VPATH)/$$i lib_include/$$i || $(CP) $(VPATH)/$$i lib_include;\
	done

$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ) $(EXE_DEPENDS)
	- $(RM) -r ptrepository		# repository of problems..
	@echo ""
	@echo "Compiling: " $(EXE) "on"  `hostname | awk -F. '{printf $$1}'`  in directory `pwd` at `date`
	$(BIN_CXX) -o $@ $(CCFLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ) \
		$(TI_INST_OBJ) $(LIBS)

debug$(EXE): $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ) $(EXE_DEPENDS)
	- $(RM) -r ptrepository		# repository of problems..
	@echo ""
	@echo "Compiling: " Debug version $(EXE) "on" `hostname | awk -F. '{printf $$1}'`  in directory `pwd` at `date`
	$(BIN_CXX) -o $(EXE) $(CCFLAGS) $(LDIRS) $(IDIRS) $(EXEOBJS) $(VT_INST_OBJ) \
		$(TI_INST_OBJ) $(DEBUGLIBS)

ta:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE)  ta_impl )\
	else $(MAKE)  ta_impl; fi

ta_impl: $(VPATH)/$(PROJNAME)_TA_type.h $(PROJNAME)_TA.cc $(VPATH)/$(PROJNAME)_TA_inst.h
	- touch $(OBJS)
	- rm  $(PROJNAME)_TA.o

force_ta:
	@if [ ! -f NoMakeTA ];\
	then ( if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  force_ta_impl )\
	else $(MAKE)  force_ta_impl; fi); fi

force_ta_impl:
	- $(MAKETA) $(MAKETA_FLAGS_XX) $(IDIRS) $(PROJNAME) $(TAHEADS)
	- $(CP) $(PROJNAME)_TA_type.hx $(PROJNAME)_TA.ccx $(PROJNAME)_TA_inst.hx $(VPATH)
	- $(MAKE)  $(PROJNAME)_TA_type.h
	- $(MAKE)  $(PROJNAME)_TA_inst.h
	- $(MAKE)  $(PROJNAME)_TA.cc

no_ta:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU);	$(MAKE)  no_ta_impl )\
	else $(MAKE)  no_ta_impl; fi

no_ta_impl:
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

src_tar:	
	@echo " "
	@echo "** Making source distribution tar file **"
	@echo " "
	- $(RM) $(PROJNAME)_src.tar*
	$(CD) ..;\
	$(TAR) -cf $(PROJDIR)/$(PROJNAME)_src.tar.gz -z --ignore-failed-read \
		$(PROJDIR)/*.h $(PROJDIR)/*.hx $(PROJDIR)/*.ccx \
		$(PROJDIR)/*.cc $(PROJDIR)/*.css $(PROJDIR)/Makefile.in \
		$(PROJDIR)/Makefile $(PROJDIR)/*.def

bin_tar:	
	@echo " "
	@echo "** Making binary distribution tar file **"
	@echo " "
	- $(RM) $(PROJNAME)_bin_$(CPU).tar.gz
	$(CD) ..;\
	$(TAR) -cf $(PROJDIR)/$(PROJNAME)_bin_$(CPU).tar.gz -z --ignore-failed-read \
		$(PROJDIR)/$(CPU)/$(EXE) $(PROJDIR)/*.def

backup:
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

clean:	clean_impl
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  clean_impl )\
	else $(MAKE)  clean_impl; fi

clean_impl:
	- $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz

real_clean:
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  real_clean_impl )\
	else $(MAKE)  real_clean_impl; fi

real_clean_impl: clean_impl
	- $(RM) *.ccx *.d

dist_clean:	clean_impl
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  dist_clean_impl )\
	else $(MAKE)  dist_clean_impl; fi

dist_clean_impl: real_clean_impl
	- $(RM) lib*.$(LIB_EXT) $(EXE) $(EXE).old $(EXE).new

src_clean:	clean_impl

wipe:
	- $(RM) *~ #* *%

touch:
	- $(TOUCH) $(HEADS) $(SRCS)

touch_objs:	
	@if [ -d $(CPU) ]; then ( $(CD) $(CPU); $(MAKE)  touch_objs_impl )\
	else $(MAKE)  touch_objs_impl; fi

touch_objs_impl:
	- $(TOUCH) $(OBJS) $(EXEOBJS) $(VT_INST_OBJ) $(TI_INST_OBJ)

MakeDepend:	cpu_dir depend

NewMakeDepend:	Makefiles depend

# note the following should not be used when make will not allow a new makefile
# to be run from within the old one!
# in this case, you must make 'NewMakefile' and then 'cpu_dir' in two stages
Makefiles:
	-@if [ -z "$(CPU)" ]; then (echo "ERROR: Need to set CPU variable!!!" )\
	else $(MAKE)  NewMakefile; $(MAKE)  cpu_dir; fi

NewMakefile:
	-@ $(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
	  $(TOP)/Makefile.user Makefile.in $(PDPDIR)/config/Makefile.std >Makefile

cpu_dir:	
	-@if [ -z "$(CPU)" ]; then (echo "ERROR: Need to set CPU variable!!!" )\
	else $(MKDIR) src_include 1>/dev/null 2>&1;\
		$(CD) src_include; $(LN) ../ $(PROJNAME) 1>/dev/null 2>&1;\
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
		then ( $(CP) $(VPATH)/$(EXTRA_CPU_FILE) $(EXTRA_CPU_FILE) ) fi;\
	fi

local_cpu_dir:
	-@if [ -z "$(CPU)" ]; then (echo "ERROR: Need to set CPU variable!!!" )\
	else $(MKDIR) $(LOC_OBJS)/$(PROJNAME) 1>/dev/null 2>&1;\
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
	$(MAKE)  Makefiles; \
	fi

rm_cpu_dir:
	- $(RM) -r $(CPU)
	- $(RM) -r $(LOC_OBJS)/$(PROJNAME)/$(CPU)

depend:	
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
	@$(RM) eddep
	@if [ ! -f $(PROJNAME)_TA.d ]; then echo "# no dependencies" >$(PROJNAME)_TA.d ; fi

# DO NOT DELETE THIS LINE -- make depend depends on it.

