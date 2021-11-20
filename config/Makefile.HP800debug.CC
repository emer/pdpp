####################################################
# machine-dependent stuff and overrides of std defs
# HP800 - cfront (CC) debug version

# use the CC includes
IOS_INCLUDES = CC-3.1

#include CC iostream stuff
IDIRS_EXTRA = -I/opt/CC/include/CC

#include x libs
LDIRS_EXTRA = -L/usr/lib/X11R5

DEFINES = -DNO_SIGNED -DNO_BUILTIN_BOOL -DHP800

# +T use cfront to translate to c and then compile
#  this results in a typically faster executable, but -much- longer compile
#  times when compiling debug so we removed it!
# +Z genereate position independed code for shared libs
# +a1 generate ansi C code
# +e0 genereate virtual tables elsewhere (vt file)

CXX_NONSHARED = CC +a1 +e0 -D__ANSI_CPP__
CC_NONSHARED = gcc -U__GNUG__ -nostdinc -I/usr/include

LIB_EXT_SHARED = .a

#cppC = /usr/lib/Cpp.ansi -I/usr/include/CC
# use gnu cccp instead cuz it works! (and don't include standard stuff..)
cppC = /usr/local/bin/cccp -lang-c++ -nostdinc++

# debug g1 only does debug info once!
DEBUG_FLAG = -g1
DEBUG_OPT_FLAG = 
OPT_FLAG = -g1

# vt file needed for e0/e1 stuff
VT_INST_SRC = $(VT_INST_FILE).cc
VT_INST_OBJ = $(VT_INST_FILE).o
VT_INST_DEP = $(VT_INST_FILE).d

# not using TI
TI_INST_SRC = 
TI_INST_OBJ = 
TI_INST_DEP = 

# using old cfront for _TA.cc files
MAKETA_FLAGS_CPU = -old_cfront

IV_LIB = -lIVCCdebug

# end.o is for _DLD_HOOK_ for debugging with gdb?
# GENERIC_LIB = -lm /opt/langtools/lib/end.o
GENERIC_LIB = -lm

# HP's CC does not have -M flag
DEP_CXX = g++
DEP_CC = gcc
