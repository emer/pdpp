####################################################
# machine-dependent stuff and overrides of std defs
# SUN4 - cfront (CC)

# use the CC includes
IOS_INCLUDES = CC-3.1

#include CC iostream stuff
IDIRS_EXTRA = -I/usr/include/CC

# using objectcenter version of cfront
# objectcenter stuff 
OCENTER = $(LOCAL)/ocenter
# use static C libraries for distribution purposes?
LDIRS_EXTRA = -static -L$(OCENTER)/sparc-sunos4/lib
#LDIRS_EXTRA = -L$(OCENTER)/sparc-sunos4/lib

DEFINES =  -Dsparc -D__sparc__ -D__sun__ -DNO_SIGNED -DNO_BUILTIN_BOOL

CXX_SHARED = CC -fPIC +V +a1 +e0 -D__ANSI_CPP__
CC_SHARED = gcc -fPIC -U__GNUG__
CXX_NONSHARED = CC +V +a1 +e0 -D__ANSI_CPP__
CC_NONSHARED = gcc -U__GNUG__

AR_CMD_SHARED = ld -assert pure-text -o 
LIB_EXT_SHARED = so

# use gnu cccp instead cuz it works!
cppC = /usr/local/bin/cccp -lang-c++ -nostdinc++

# use gnu cc insted of clcc for backend c compiler
ccC = /usr/local/bin/gcc -U__GNUG__

# not using shared libraries

DEBUG_FLAG = -g -gdem
DEBUG_OPT_FLAG = 
OPT_FLAG = -O

# vt file needed for e0/e1 stuff (despite docs to contrary..)
VT_INST_SRC = $(VT_INST_FILE).cc
VT_INST_OBJ = $(VT_INST_FILE).o
VT_INST_DEP = $(VT_INST_FILE).d

# not using TI
TI_INST_SRC = 
TI_INST_OBJ = 
TI_INST_DEP = 

# using old cfront for _TA.cc files
MAKETA_FLAGS_CPU = -old_cfront


