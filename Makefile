OS_NAME := $(shell uname -s)
SOURCES = hreeble/Element.cpp hreeble/sop_hreeble.cpp
ICONS = Icons/*.svg

ifeq ($(OS_NAME),Darwin)
	INSTDIR = /Users/$(USER)/Library/Preferences/houdini/15.0
	DSONAME = sop_hreeble.dylib
	OPTIMIZER = -O1
else
	INSTDIR = $(HOME)/houdini15.0
	DSONAME = sop_hreeble.so
	OPTIMIZER = -O2
endif
CXXFLAGS+=-std=c++11
# OPTIMIZER = -g

include $(HFS)/toolkit/makefiles/Makefile.gnu

install:
	@if [ ! -d $(INSTDIR)/dso ]; then mkdir $(INSTDIR)/dso; fi
	@cp $(DSONAME) $(INSTDIR)/dso
	@mkdir -p $(INSTDIR)/config/Icons
	@cp $(ICONS) $(INSTDIR)/config/Icons
