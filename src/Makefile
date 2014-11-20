###############################################################################
# Makefile for compiling and linking the ROOT extension to support cloud      #
# storage.                                                                    #
# Based on Makefile in $ROOTSYS/test.                                         #
# Author: Fabio Hernandez (fabio@in2p3.fr)                                    #
# Date:   2013-05-17                                                          #
###############################################################################

#
# Retrieve this ROOT's configuration
#
ROOT_CONFIG := root-config
ifeq ($(shell which $(ROOT_CONFIG) 2>&1 | sed -ne "s@.*/$(ROOT_CONFIG)@$(ROOT_CONFIG)@p"),$(ROOT_CONFIG))
MKARCH := $(wildcard $(shell $(ROOT_CONFIG) --etcdir)/Makefile.arch)
# RCONFIG := $(wildcard $(shell $(ROOT_CONFIG) --incdir)/RConfigure.h)
endif
ROOT_LIBDIR := $(shell $(ROOT_CONFIG) --libdir)
ROOT_ETCDIR := $(shell $(ROOT_CONFIG) --etcdir)

#
# If no general Makefile for this architecture is located in $ROOTSYS/etc
# include the one shipped with the $ROOTSYS/test directory
#
ifeq ($(wildcard $(MKARCH)),)
MKARCH := $(ROOTSYS)/test/Makefile.arch
endif
include $(MKARCH)

#
# Add link flags for ROOT libraries
#
LDFLAGS += $(shell $(ROOT_CONFIG) --libs)

#
# Add compilation and link flags for libNeon. Note that we cannot use libNeon
# explicit libraries in the LDFLAGS variable but instead in the explicit link
# library flags.
# The full path to the 'neon-config' command is expected to be found in the
# environmental variable NEON_CONFIG. If not, we expect this command to be in
# the PATH. If you installed libNeon in a different location, use the command
#
#    $ NEON_CONFIG=/path/to/my/neon-config && make
#
ifeq ($(NEON_CONFIG),)
NEON_CONFIG := neon-config
endif

CXXFLAGS     += $(shell $(NEON_CONFIG) --cflags)
CXXFLAGS     += -I../external/rapidjson/include
EXPLLINKLIBS += $(shell $(NEON_CONFIG) --libs)
NEON_INC     := $(shell $(NEON_CONFIG) --prefix)/include/neon

#------------------------------------------------------------------------------

CLOUDSTOR_INC   =  THttpFile.h \
                   THttpSession.h \
                   THttpRequest.h \
                   THttpHeader.h \
                   THttpHeaderSet.h \
                   TS3Session.h \
                   TS3Request.h \
                   TS3File.h \
                   TSwiftIdentityClient.h \
                   TSwiftSession.h \
                   TSwiftFile.h \
                   TCloudExtension.h

CLOUDSTOR_SRC   =  THttpFile.$(SrcSuf) \
                   THttpSession.$(SrcSuf) \
                   THttpRequest.$(SrcSuf) \
                   THttpHeader.$(SrcSuf) \
                   THttpHeaderSet.$(SrcSuf) \
                   TS3Session.$(SrcSuf) \
                   TS3Request.$(SrcSuf) \
                   TS3File.$(SrcSuf) \
                   TSwiftIdentityClient.$(SrcSuf) \
                   TSwiftSession.$(SrcSuf) \
                   TSwiftFile.$(SrcSuf) \
                   TCloudExtension.$(SrcSuf) \
                   Init.$(SrcSuf) \
                   AllDict.$(SrcSuf)

CLOUDSTOR_OBJ = $(CLOUDSTOR_SRC:%.$(SrcSuf)=%.$(ObjSuf))

CLOUDSTOR_BASE  = libRootCloudStorage
CLOUDSTOR_SHOBJ = $(CLOUDSTOR_BASE).$(DllSuf)

ifeq ($(PLATFORM),win32)
CLOUDSTOR_LIB = $(CLOUDSTOR_BASE).lib
else
CLOUDSTOR_LIB = $(shell pwd)/$(CLOUDSTOR_SHOBJ)
endif


OBJS          = $(CLOUDSTOR_OBJ)
SHOBS         = $(CLOUDSTOR_SHOBJ)

# Directory under which to deploy this extension for a private installation.
# Useful when using a shared (possibly read-only) installation of ROOT. Note
# however that when you deploy this extension in a different location than
# under $ROOTSYS, you need to customize your ROOT configuration file
# (i.e. $HOME/.rootrc) to let ROOT know where it must search for shared
#  libraries and for plugins.
HOME_INSTALL_DIR = $(HOME)/.root

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:  $(OBJS) $(SHOBS)


.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) $(NEON_CXX_FLAGS) -c $<


AllDict.$(SrcSuf): $(CLOUDSTOR_INC) LinkDef.h
	@echo "Generating dictionary $@..."
ifeq ($(ROOTCINT),)
	rootcling -f $@ -c -I$(NEON_INC) -s $(CLOUDSTOR_SHOBJ) $^
else
	rootcint -f $@ -c -I$(NEON_INC) $^
endif


$(CLOUDSTOR_SRC): $(CLOUDSTOR_INC)


$(CLOUDSTOR_SHOBJ): $(CLOUDSTOR_OBJ)
ifeq ($(ARCH),aix5)
	$(MAKESHARED) $(OutPutOpt) $@ $(LIBS) -p 0 $^
else
# On MacOS X we need to build both the .dylib and the .so
ifeq ($(PLATFORM),macosx)
	$(LD) $(SOFLAGS)$@ $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
ifneq ($(subst $(MACOSX_MINOR),,1234),1234)
ifeq ($(MACOSX_MINOR),4)
	ln -sf $@ $(subst .$(DllSuf),.so,$@)
endif
endif
else
ifeq ($(PLATFORM),win32)
	bindexplib $* $^ > $*.def
	lib -nologo -MACHINE:IX86 $^ -def:$*.def $(OutPutOpt)$(CLOUDSTOR_LIB)
	$(LD) $(SOFLAGS) $(LDFLAGS) $^ $*.exp $(LIBS) $(OutPutOpt)$@
	$(MT_DLL)
else
	$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
endif
endif
endif
	@echo "$@ done"


clean:
	@rm -f $(OBJS) core *Dict.* *.so *.dll AllDict*.* *.pcm


buildall: clean all


install: $(CLOUDSTOR_SHOBJ)
	cp ../plugins/P160_CloudStorage.C $(ROOT_ETCDIR)/plugins/TFile
	cp $(wildcard $(CLOUDSTOR_BASE)*.so $(CLOUDSTOR_BASE)*.pcm) $(ROOT_LIBDIR)


uninstall:
	rm -f $(ROOT_ETCDIR)/plugins/TFile/P160_CloudStorage.C
	rm -f $(ROOT_LIBDIR)/$(CLOUDSTOR_BASE)*.{so,pcm}


installhome: $(CLOUDSTOR_SHOBJ) uninstallhome
	mkdir -p $(HOME_INSTALL_DIR)/lib $(HOME_INSTALL_DIR)/plugins/TFile
	cp $(wildcard $(CLOUDSTOR_BASE)*.so $(CLOUDSTOR_BASE)*.pcm) $(HOME_INSTALL_DIR)/lib
	cp ../plugins/P160_CloudStorage.C $(HOME_INSTALL_DIR)/plugins/TFile


uninstallhome:
	rm -f $(HOME_INSTALL_DIR)/lib/$(CLOUDSTOR_BASE)*.{so,pcm}
	rm -f $(HOME_INSTALL_DIR)/plugins/TFile/P160_CloudStorage.C
