##################################################################
# Makefile for the whole ta/css/pdp++ system
#

# paths to various things:
# it is assumed that most things are installed under LOCAL
LOCAL = /usr/local

# PDPDIR is the path where the distribution is located.  If you change this
# here, ***BE SURE TO CHANGE IT IN 'config/Makefile.defs' ***
PDPDIR = $(LOCAL)/pdp++

# top can be defined to be in a user's home directory: it is where the source is
TOP = $(PDPDIR)/src

# publicly accessable binary directory for linking executables into (install_bins)
BIN_DIR = $(LOCAL)/bin

PREV_VERSION = 3.1
VERSION = 3.2a07

# these are fairly standard..
TAR = tar
DU = du
RM = rm -f
CP = cp
CD = cd
LN = ln -s
CAT = cat
MKDIR = mkdir
DIFF = gdiff
GZIP = gzip
MV = mv -f

# library-only directories
LIB_DIRS = ta_string iv_misc ta iv_graphic ta_misc css pdp

# optional dirs
OPT_DIRS = leabra oldlba bp cs so bpso lstm rns

# places where executables get made
BIN_DIRS = css $(OPT_DIRS)

# bin and lib dirs (both library and executable are made here)
LIB_BIN_DIRS = css bp so leabra oldlba lstm rns

TA_COPYRIGHT = ta_string iv_misc ta iv_graphic ta_misc css
PDP_COPYRIGHT = pdp $(OPT_DIRS)

# bin only dirs
BIN_ONLY_DIRS = maketa $(OPT_DIRS)

# all of the directories
DIRS = $(LIB_DIRS) $(BIN_ONLY_DIRS)

MISC_DIST_FILES = pdp++/CopyRight pdp++/INSTALL pdp++/config pdp++/css \
	pdp++/ANNOUNCE pdp++/ANNOUNCE.TA_CSS pdp++/ANNOUNCE.IV_PATCHES \
	pdp++/TODO pdp++/NEWS pdp++/ChangeLog pdp++/ChangeLog.1.2 pdp++/ChangeLog.2.0 \
	pdp++/README pdp++/README.mswindows pdp++/README.darwin pdp++/README.linux \
	pdp++/README.dmem

# "extra" distribution directories (this + bin is usable)
EXT_DIST_DIRS = $(MISC_DIST_FILES) pdp++/defaults pdp++/demo pdp++/manual

EXT_DIST_EXCLUDE =

# directories for source-code distribution 
SRC_DIST_DIRS = pdp++/Makefile pdp++/src pdp++/include \
	pdp++/lib pdp++/bin $(EXT_DIST_DIRS)

SRC_ONLY_DIST_DIRS = pdp++/demo pdp++/Makefile pdp++/ChangeLog pdp++/TODO pdp++/src $(MISC_DIST_DIRS)

SRC_DIST_EXCLUDE = --exclude HP800 --exclude SUN4  --exclude SUN4debug \
                   --exclude SGI   --exclude SGIdebug \
		   --exclude LINUX --exclude LINUXdebug --exclude LINUXprof --exclude LINUXstat\
		   --exclude LINUXPPC --exclude LINUXPPCdebug --exclude LINUXdmem \
		   --exclude LINUXdmemdbg \
		   --exclude IBMaix --exclude IBMaixdebug \
		   --exclude LINKCC --exclude LINKCCdebug --exclude CYGWIN \
		   --exclude iv --exclude TAGS  --exclude oldcode --exclude lib_include \
                   --exclude ta_TA.cc --exclude ta_TA.ccx --exclude src_include

# directories for binary distribution
BIN_DIST_EXCLUDE = --exclude pdpshell --exclude rbp++

#LIB_TARG = Lib
LIB_TARG = optLib

#BIN_TARG = Bin
BIN_TARG = optBin

default:  all
#default:  world

chkcpu:
	@if [ -z "$(CPU)" ];\
	then (echo "ERROR: CPU variable must be set!"; exit 1;) \
	fi

World:	world

world:	chkcpu include_dir lib_dir bin_dir Makefiles new_lib_h Maketa force_ta new_lib_h2 force_ta_misc Libs LibsPass2 distBins
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

all:	chkcpu Libs LibsPass2 distBins
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

re_all:	chkcpu NewMakeDepend force_ta new_lib_h force_ta_misc NewMakeDepend all

makefiles:	Makefiles

Makefiles:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(PDPDIR)/include;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(PDPDIR)/lib;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(PDPDIR)/bin;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  echo "Making Makefiles for" $$i;\
	  $(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		$(PDPDIR)/config/Makefile.init > Makefile.tmp;\
	  if ( $(MAKE) -f Makefile.tmp NewMakefile; ) then echo "";\
	  else exit 1; fi;\
	  $(RM) Makefile.tmp;\
	  if ( $(MAKE) Makefiles; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

include_dir:	chkcpu 
	@echo " "
	@echo "** Making the include links ($@) **"
	@echo " "
	@$(CD) $(PDPDIR)/include;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(PDPDIR)/include; $(MAKE) 
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

lib_dir:	chkcpu 
	@echo " "
	@echo "** Making the lib links ($@) **"
	@echo " "
	@$(CD) $(PDPDIR)/lib;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(PDPDIR)/lib; $(MAKE) 
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

bin_dir:	chkcpu 
	@echo " "
	@echo "** Making the bin links **"
	@echo " "
	@$(CD) $(PDPDIR)/bin;\
	$(CP) Makefile Makefile.bak;\
	$(CAT) $(PDPDIR)/config/Makefile.defs $(PDPDIR)/config/Makefile.$(CPU) \
		Makefile.in >Makefile
	@$(CD) $(PDPDIR)/bin; $(MAKE) 
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

new_makefile:	NewMakefile

NewMakefile:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

make_depend:	MakeDepend

MakeDepend:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) NewMakefile; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) MakeDepend; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

new_make_depend:	MakeDepend

NewMakeDepend:	MakeDepend

depend:	chkcpu 
	@echo " "
	@echo "** Making dependencies in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

force_ta:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

new_lib_h:	chkcpu 
	@echo " "
	@echo "** Making New Library Headers in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

new_lib_h2:	chkcpu 
	@echo " "
	@echo "** Making New Library Headers in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) new_lib_h; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

force_ta_misc:	chkcpu new_lib_h
	@echo " "
	@echo "** Making force_ta in ta_misc **"
	@echo " "
	@$(CD) $(TOP);\
	for i in "ta_misc";\
	  do $(CD) $$i;\
	  if ( $(MAKE) force_ta; ) then echo "* Successfully made force_ta in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

maketa:	Maketa

Maketa:	chkcpu 
	@echo " "
	@echo "** Making the maketa program ($@) **"
	@echo " "
	@$(CD) $(TOP)/ta_string;\
	if ( $(MAKE)  optLib; ) then echo "* Successfully made ta_string/optLib";\
	  else exit 1; fi;\
	$(CD) ../maketa;\
	if ( $(MAKE) optBin; ) then echo "* Successfully made maketa/optBin";\
	  else exit 1; fi;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

libs:	Libs

Libs:	chkcpu 
	@echo " "
	@echo "** Making the libraries ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(LIB_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(LIB_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

libs_pass2:	LibsPass2

LibsPass2:	chkcpu 
	@echo " "
	@echo "** Making pass 2 of the  libraries ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(LIB_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) LibPass2; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

bins:	Bins

Bins:	chkcpu 
	@echo " "
	@echo "** Making the binary executables ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(BIN_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dist_bins:	distBins

distBins:	chkcpu 
	@echo " "
	@echo "** Making the distribution executables ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) optBin; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) distBin; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) optLib; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dist_re_bins:	distReBins

distReBins:	chkcpu 
	@echo " "
	@echo "** re-Making the distribution executables ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) optReBin; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) distBin; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) optLib; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

strip_bins:	stripBins

stripBins:	chkcpu 
	@echo " "
	@echo "** Stripping the executables ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) stripBin; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	-@$(CD) lib/$(CPU);\
        strip lib*.so*;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

lib_min:	LibMin

LibMin:	chkcpu 
	@echo " "
	@echo "** Making the minimal TA library (no gui) ($@) **"
	@echo " "
	@$(CD) $(TOP)/ta_lib_min;\
	if ( $(MAKE) makefiles; ) then echo "";\
	  else exit 1; fi;\
	if ( $(MAKE) ios_includes; ) then echo "";\
	  else exit 1; fi;\
	if ( $(MAKE) lm_force_ta; ) then echo "";\
	  else exit 1; fi;\
	if ( $(MAKE) optLib; ) then echo "* Successfully made $@";\
	  else exit 1; fi;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

hard_css:	chkcpu 
	@echo " "
	@echo "** Making the hard-coding css library ($@) **"
	@echo " "
	@$(CD) $(TOP)/css;\
	if ( $(MAKE) hard_css; ) then echo "* Successfully made $@";\
	  else exit 1; fi;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

opt_update:	optUpdate

optUpdate:	chkcpu 
	$(MAKE)  Update LIB_TARG=optLib BIN_TARG=optBin
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dbg_update:	dbgUpdate

dbgUpdate:	chkcpu 
	$(MAKE)  Update LIB_TARG=Lib BIN_TARG=Bin
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

update:		Update

Update:	chkcpu 
	@echo " "
	@echo "** Updating Everything (remaking) ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(LIB_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) clean; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) MakeDepend; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) force_ta; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) new_lib_h; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) $(LIB_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@$(CD) $(TOP);\
	for i in $(LIB_BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(BIN_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@$(CD) $(TOP);\
	for i in $(BIN_ONLY_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) clean; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) MakeDepend; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) force_ta; ) then echo "";\
	  else exit 1; fi;\
	  if ( $(MAKE) $(BIN_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

opt_make:	optMake

optMake:	chkcpu 
	$(MAKE) Make LIB_TARG=optLib BIN_TARG=optBin
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dbg_make:	dbgMake

dbgMake:	chkcpu 
	$(MAKE) Make LIB_TARG=Lib BIN_TARG=Bin
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

make:	Make

Make:	chkcpu 
	@echo " "
	@echo "** Doing an appropriate Make in every directory ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(LIB_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(LIB_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@$(CD) $(TOP);\
	for i in $(LIB_BIN_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(BIN_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@$(CD) $(TOP);\
	for i in $(BIN_ONLY_DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $(BIN_TARG); ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

touch_objs:	chkcpu 
	@echo " "
	@echo "** Touching all objects in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

backup:	chkcpu 
	@echo " "
	@echo "** Making backups in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) backup; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

local_cpu_dir:	chkcpu 
	@echo " "
	@echo "** Making local cpu object directory ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) local_cpu_dir; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

rm_cpu_dir:	chkcpu 
	@echo " "
	@echo "** Removing $(CPU) (cpu directory) in all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) rm_cpu_dir; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	$(CD) include; $(RM) -r $(CPU)
	$(CD) lib; $(RM) -r $(CPU)
	$(CD) bin; $(RM) -r $(CPU)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

clean:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

wipe:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dist_clean:	chkcpu 
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

demo_clean:	chkcpu 
	@echo " "
	@echo "** Cleaning up the demo directory **"
	@echo " "
	-@ $(CD) $(PDPDIR)/demo;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) bp; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../bp_srn; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../bridge; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../cs; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../rbp; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../so; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz;\
		$(CD) ../demo; $(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

src_clean:	chkcpu demo_clean
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	-@ $(CD) $(PDPDIR)/include;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	-@ $(CD) $(PDPDIR)/lib;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	-@ $(CD) $(PDPDIR)/bin;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	-@ $(CD) $(PDPDIR)/config;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	-@ $(CD) $(PDPDIR)/css/include;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	-@ $(CD) $(PDPDIR)/css/tests;\
		$(RM) *.o *~ #* core *.output PDP++Recover.*.proj.gz
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

clean_libs:	chkcpu 
	@echo " "
	@echo "** Getting rid of existing libraries ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i/$(CPU);\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

clean_old_libs:	chkcpu 
	@echo " "
	@echo "** Getting rid of OLD libraries saved for shared ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(DIRS);\
	  do $(CD) $$i/$(CPU);\
	  if ( $(MAKE) $@; ) then echo "* Successfully made $@ in $$i";\
	  else exit 1; fi;\
	  $(CD) $(TOP);\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

clean_config:	chkcpu 
	@echo " "
	@echo "** Removing current makefile links ($@) **"
	@echo " "
	@$(CD) config;\
	$(RM) Makefile.SUN4; \
	$(RM) Makefile.HP800; \
	$(RM) Makefile.SGI;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

std_config:	chkcpu 
	@echo " "
	@echo "** Remaking standard makefile links ($@) **"
	@echo " "
	@$(CD) config;\
	$(LN) Makefile.SUN4.g++ Makefile.SUN4; \
	$(LN) Makefile.HP800.CC Makefile.HP800; \
	$(LN) Makefile.SGI.CC Makefile.SGI;
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

diffs:	chkcpu 
	@echo " "
	@echo "** Making source diffs on blank directories ($@) **"
	@echo " "
	@echo "** from version pdp++-$(PREV_VERSION) to pdp++-$(VERSION) ** "
	@echo " "
	- $(RM) pdp++_$(PREV_VERSION)-$(VERSION)_diffs*
	$(CD) ..;\
	$(DIFF) -crN -xsrc_include -xmanual -xinterviews -xMakefile -xMakefile.bak \
	  pdp++-$(PREV_VERSION) pdp++-$(VERSION) \
	  > pdp++/pdp++_$(PREV_VERSION)-$(VERSION)_diffs; \
	$(CD) pdp++; \
	$(GZIP) pdp++_$(PREV_VERSION)-$(VERSION)_diffs
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

src_tar:	chkcpu src_clean
	@echo " "
	@echo "** Making source distribution tar file (does a src_clean first!!) ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_src.tar*
	$(CD) ..;\
	$(TAR) -cf pdp++/pdp++_$(VERSION)_src.tar.gz -z $(SRC_DIST_EXCLUDE) $(SRC_DIST_DIRS);\
	$(CD) pdp++
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

src_only_tar:	chkcpu src_clean
	@echo " "
	@echo "** Making source distribution tar file (does a src_clean first!!) ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_src_only.tar*
	$(CD) ..;\
	$(TAR) -cf pdp++/pdp++_$(VERSION)_src_only.tar.gz -z \
               $(SRC_DIST_EXCLUDE) $(SRC_ONLY_DIST_DIRS);\
	$(CD) pdp++
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

ta_css_tar:	chkcpu src_clean
	@echo " "
	@echo "** Making TA/CSS distribution tar file (does a src_clean first!!) ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_ta_css.tar*
	$(CD) ..;\
	$(TAR) -cf pdp++/pdp++_$(VERSION)_ta_css.tar.gz -z $(SRC_DIST_EXCLUDE) \
	$(MISC_DIST_FILES) pdp++/manual/ta_css* pdp++/manual/html/ta_css* \
	pdp++/Makefile pdp++/include pdp++/lib pdp++/bin \
	pdp++/src/ta_string pdp++/src/iv_misc pdp++/src/ta \
	pdp++/src/css pdp++/src/iv_graphic pdp++/src/ta_misc \
	pdp++/src/Makefile.init pdp++/src/Makefile.user
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

# To make .rpm:
# 1. make the binext_tar, then su
# 2. mv /usr/local/pdp++ /usr/local/pdp++.good
# 3. copy binext_tar to /usr/src/redhat/SOURCES
# 4. copy pdp++-binext-version.spec /usr/src/redhat/SPECS and cd there
# 5. do rpm -bb pdp++-binext-2.0-0.spec
# 6. /bin/rm -rf /usr/local/pdp++
# 7. mv /usr/local/pdp++.good /usr/local/pdp++
# 8. /bin/rm -rf /usr/src/redhat/BUILD/*, SOURCES/*
# 9. mv /usr/src/redhat/RPM/i386/pdp++-binext-version-0.i386.rpm /usr/local/pdp++
# a. mine /usr/local/pdp++/pdp++-binext-version-0.i386.rpm
# b. un-su 

binext_tar:	chkcpu stripBins
	@echo " "
	@echo "** Making binaries and extra stuff distribution tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_binext_$(CPU).tar*
	$(CD) ..;\
	$(TAR) -chf pdp++/pdp++_$(VERSION)_binext_$(CPU).tar.gz -z $(BIN_DIST_EXCLUDE) \
		$(EXT_DIST_EXCLUDE) pdp++/bin/$(CPU) pdp++/lib/$(CPU)/lib*.s[ol]* \
		pdp++/lib/$(CPU)/lib*.dylib \
		pdp++/interviews/lib/$(CPU)/libIVhines.s[ol]* $(EXT_DIST_DIRS) \
		pdp++/interviews/lib/$(CPU)/libIVhines.dylib 
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

bin_tar:	chkcpu stripBins
	@echo " "
	@echo "** Making binaries distribution tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_bin_$(CPU).tar*
	$(CD) ..;\
	$(TAR) -chf pdp++/pdp++_$(VERSION)_bin_$(CPU).tar.gz -z $(BIN_DIST_EXCLUDE) \
		pdp++/bin/$(CPU) pdp++/lib/$(CPU)/lib*.s[ol]* \
		pdp++/lib/$(CPU)/lib*.dylib \
		pdp++/interviews/lib/$(CPU)/libIVhines.s[ol]* \
		pdp++/interviews/lib/$(CPU)/libIVhines.dylib
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

css_bin_tar:	chkcpu stripBins
	@echo " "
	@echo "** Making CSS binaries distribution tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_css_$(CPU).tar*
	$(CD) ..;\
	$(TAR) -chf pdp++/pdp++_$(VERSION)_css_$(CPU).tar.gz -z $(BIN_DIST_EXCLUDE) \
		pdp++/bin/$(CPU)/css pdp++/lib/$(CPU)/libtastring.s[ol]* \
		pdp++/lib/$(CPU)/libivmisc.s[ol]* pdp++/lib/$(CPU)/libtypea.s[ol]* \
		pdp++/interviews/lib/$(CPU)/libIVhines.s[ol]* \
		pdp++/interviews/lib/$(CPU)/libIVhines.dylib
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

ext_tar:	chkcpu 
	@echo " "
	@echo "** Making extras distribution tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_ext.tar*
	$(CD) ..;\
	$(TAR) -chf pdp++/pdp++_$(VERSION)_ext.tar.gz -z $(EXT_DIST_EXCLUDE) $(EXT_DIST_DIRS)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

ivlib_tar:	chkcpu 
	@echo " "
	@echo "** Making interviews library tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_ivlib_$(CPU).tar*
	- $(TAR) -czf pdp++_$(VERSION)_ivlib_$(CPU).tar.gz \
		--exclude libUnidraw.a interviews/lib/$(CPU)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

ivinc_tar:	chkcpu 
	@echo " "
	@echo "** Making interviews include tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_ivinc.tar*
	$(TAR) -chf pdp++_$(VERSION)_ivinc.tar.gz -z --exclude Unidraw interviews/include
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

ivpatch_tar:	chkcpu 
	@echo " "
	@echo "** Making interviews patch tar file ($@) **"
	@echo " "
	- $(RM) pdp++_$(VERSION)_ivpatch.tar*
	$(CD) lib; \
	$(TAR) -chf ../pdp++_$(VERSION)_ivpatch.tar.gz -z interviews
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

dist_size:	chkcpu 
	@echo " "
	@echo "** Maximum size of the distribution tar file (do dist_clean first!!) ($@) **"
	@echo " "
	$(DU) -s $(SRC_DIST_DIRS)
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

copyrightify:	chkcpu 
	@echo " "
	@echo "** Marking all files with copyright notice! ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	for i in $(TA_COPYRIGHT);\
	  do $(CD) $$i;\
	  $(PDPDIR)/bin/copyrightify $(PDPDIR)/ta_copyright;\
	  $(CD) $(TOP);\
	done
	@$(CD) $(TOP);\
	for i in $(PDP_COPYRIGHT);\
	  do $(CD) $$i;\
	  $(PDPDIR)/bin/copyrightify $(PDPDIR)/pdp_copyright;\
	  $(CD) $(TOP);\
	done	
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

tags:	chkcpu 
	@echo " "
	@echo "** Making TAGS file for all directories ($@) **"
	@echo " "
	@$(CD) $(TOP);\
	$(RM) TAGS;\
	for i in $(DIRS);\
	  do etags --c++ -a -o TAGS $$i/*.h $$i/*.cc;\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

find_changed:	chkcpu 
	@echo " "
	@echo "** Find changed files since src tar file ($@) **"
	@echo " "
	find . -maxdepth 3 -newer pdp++_$(VERSION)_src.tar.gz -type f -print | grep -v Makefile.bak \
	 | grep -v _TA
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

# this version is for 'find' commands without the -maxdepth 3 command..
find_changed_cpu:	chkcpu 
	@echo " "
	@echo "** Find changed files since src tar file ($@) **"
	@echo " "
	find . -name HP800 -prune -o -name HP800debug -prune -o -name SGI -prune -o \
	-name SGIdebug -prune -o -name SUN4 -prune -o -name SUN4debug -prune -o \
	-name LINUX -prune -o -name LINUXdebug -prune -o -name LINUXprof -prune -o \
	-newer pdp++_$(VERSION)_src.tar.gz -type f -print | grep -v Makefile.bak \
	 | grep -v _TA
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

txt_to_win:
	@echo " "
	@echo "** Convert text files to windows format ($@) **"
	@echo " "
	unix2dos -k -n CopyRight CopyRight.txt
	unix2dos -k -n README README.txt
	unix2dos -k -n README.mswindows README_mswindows.txt
	unix2dos -k -n INSTALL INSTALL.txt
	unix2dos -k -n ANNOUNCE ANNOUNCE.txt
	unix2dos -k -n ANNOUNCE.TA_CSS ANNOUNCE_TA_CSS.txt
	unix2dos -k -n ANNOUNCE.IV_PATCHES ANNOUNCE_IV_PATCHES.txt
	unix2dos -k -n TODO TODO.txt
	unix2dos -k -n ChangeLog ChangeLog.txt
	unix2dos -k -n ChangeLog.1.2 ChangeLog_1_2.txt
	unix2dos -k -n NEWS NEWS.txt
	$(RM) CopyRight
	$(RM) README
	$(RM) README.mswindows
	$(RM) INSTALL
	$(RM) ANNOUNCE
	$(RM) ANNOUNCE.TA_CSS
	$(RM) ANNOUNCE.IV_PATCHES
	$(RM) TODO
	$(RM) ChangeLog
	$(RM) ChangeLog.1.2
	$(RM) NEWS
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

txt_fm_win:
	@echo " "
	@echo "** Convert text files not to windows format ($@) **"
	@echo " "
	dos2unix -k -n CopyRight.txt CopyRight
	dos2unix -k -n README.txt README
	dos2unix -k -n README_mswindows.txt README.mswindows
	dos2unix -k -n INSTALL.txt INSTALL
	dos2unix -k -n ANNOUNCE.txt ANNOUNCE
	dos2unix -k -n ANNOUNCE_TA_CSS.txt ANNOUNCE.TA_CSS
	dos2unix -k -n ANNOUNCE_IV_PATCHES.txt ANNOUNCE.IV_PATCHES
	dos2unix -k -n TODO.txt TODO
	dos2unix -k -n ChangeLog.txt ChangeLog
	dos2unix -k -n ChangeLog_1_2.txt ChangeLog.1.2
	dos2unix -k -n NEWS.txt NEWS
	$(RM) CopyRight.txt
	$(RM) README.txt
	$(RM) README_mswindows.txt
	$(RM) INSTALL.txt
	$(RM) ANNOUNCE.txt
	$(RM) ANNOUNCE_TA_CSS.txt
	$(RM) ANNOUNCE_IV_PATCHES.txt
	$(RM) TODO.txt
	$(RM) ChangeLog.txt
	$(RM) ChangeLog_1_2.txt
	$(RM) NEWS.txt
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

install_bins:
	@echo " "
	@echo "** Make links in BIN_DIR for executables ($@) **"
	@echo " "
	@$(CD) $(PDPDIR)/bin/$(CPU);\
	for i in *;\
	  do echo $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/$$i ;\
	  $(RM) $(BIN_DIR)/$$i ;\
	  $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/$$i ;\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

install_dbg_bins:
	@echo " "
	@echo "** Make links in BIN_DIR with dbg_ prefix for executables ($@) **"
	@echo " "
	@$(CD) $(PDPDIR)/bin/$(CPU);\
	for i in *;\
	  do echo $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/dbg_$$i ;\
	  $(RM) $(BIN_DIR)/dbg_$$i ;\
	  $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/dbg_$$i ;\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

install_dmem_bins:
	@echo " "
	@echo "** Make links in BIN_DIR with dm_ prefix for executables ($@) **"
	@echo " "
	@$(CD) $(PDPDIR)/bin/$(CPU);\
	for i in *;\
	  do echo $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/dm_$$i ;\
	  $(RM) $(BIN_DIR)/dm_$$i ;\
	  $(LN) $(PDPDIR)/bin/$(CPU)/$$i $(BIN_DIR)/dm_$$i ;\
	done
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

linecount:
	@echo " "
	@echo "** Making $@ in all directories **"
	@echo " "
	@$(CD) $(TOP);\
	$(RM) $(TOP)/../linecount.txt ;\
	for i in $(DIRS);\
	  do $(CD) $$i;\
	  wc --lines *.h *.cc | tail -1 >> $(TOP)/../linecount.txt ;\
	  $(CD) $(TOP);\
	done;\
	dtsem d 1 s < $(TOP)/../linecount.txt
	@echo " "
	@echo "** Make $@ successfully completed! **"
	@echo " "

