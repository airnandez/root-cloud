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
RCONFIG := $(wildcard $(shell $(ROOT_CONFIG) --incdir)/RConfigure.h)
endif

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
#    $ NEON_CONFIG=/path/to/my/neon-config make
#
ifeq ($(NEON_CONFIG),)
NEON_CONFIG := neon-config
endif
CXXFLAGS += $(shell $(NEON_CONFIG) --cflags)
EXPLLINKLIBS += $(shell $(NEON_CONFIG) --libs)

#------------------------------------------------------------------------------

CLOUDSTOR_INC   =  THttpFile.h \
                   THttpSession.h \
                   THttpRequest.h \
                   THttpHeader.h \
                   THttpHeaderSet.h \
                   TS3Session.h \
                   TS3Request.h \
                   TS3File.h \
                   TSwiftSession.h \
                   TSwiftFile.h
CLOUDSTOR_SRC   =  THttpFile.$(SrcSuf) \
                   THttpSession.$(SrcSuf) \
                   THttpRequest.$(SrcSuf) \
                   THttpHeader.$(SrcSuf) \
                   THttpHeaderSet.$(SrcSuf) \
                   TS3Session.$(SrcSuf) \
                   TS3Request.$(SrcSuf) \
                   TS3File.$(SrcSuf) \
                   TSwiftSession.$(SrcSuf) \
                   TSwiftFile.$(SrcSuf) \
                   AllDict.$(SrcSuf)
CLOUDSTOR_OBJ   =  THttpFile.$(ObjSuf) \
                   THttpSession.$(ObjSuf) \
                   THttpRequest.$(ObjSuf) \
                   THttpHeader.$(ObjSuf) \
                   THttpHeaderSet.$(ObjSuf) \
                   TS3Session.$(ObjSuf) \
                   TS3Request.$(ObjSuf) \
                   TS3File.$(ObjSuf) \
                   TSwiftSession.$(ObjSuf) \
                   TSwiftFile.$(ObjSuf) \
                   AllDict.$(ObjSuf)
CLOUDSTOR_SHOBJ = libRootCloudStorage.$(DllSuf)

ifeq ($(PLATFORM),win32)
CLOUDSTOR_LIB = libRootCloudStorage.lib
else
CLOUDSTOR_LIB = $(shell pwd)/$(CLOUDSTOR_SHOBJ)
endif


OBJS          = $(CLOUDSTOR_OBJ)
SHOBS         = $(CLOUDSTOR_SHOBJ)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:  $(OBJS) $(SHOBS)


.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) $(NEON_CXX_FLAGS) -c $<


AllDict.$(SrcSuf): $(CLOUDSTOR_INC) LinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -v4 -f $@ -c $^

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
	@rm -f $(OBJS) core *Dict.* *.so *.dll

buildall: clean all

install: $(CLOUDSTOR_SHOBJ)
	cp ../plugins/P160_CloudStorage.C $(ROOTSYS)/etc/plugins/TFile
	cp $(CLOUDSTOR_SHOBJ) $(ROOTSYS)/lib


