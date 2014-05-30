

SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .cxx .cc 
VPATH=src:include:build:build/lib:../src:/include:../include:lib:script
ROOTLIB=$(shell root-config --libdir)
ROOTCFLAGS=$(shell root-config --cflags)
ROOTLFLAGS=$(shell root-config --ldflags)
ROOTLIBS=$(shell root-config --libs) -lSpectrum -lGui
INC=../include
DEBUG=-DDODEBUG
PROFILE=-pg
CXXFLAGS=-Wall -fPIC -O2 $(DEBUG) $(PROFILE) -rdynamic
SHAREDCFLAGS= -shared $(ROOTCFLAGS) $(CXXFLAGS) -I$(CURDIR)/include
NONSHARED = -c -I$(CURDIR)/include -pipe -Wshadow -W -Woverloaded-virtual $(ROOTCFLAGS) $(CXXFLAGS) -DR__HAVE_CONFIG
BUILDDIR = $(CURDIR)/build
LIB = $(BUILDDIR)/lib

all: pulseGUI.so 

clean:
	rm -f AnalysisEventDict.cxx AnalysisEventDict.h *.so *.o *~ $(LIB)/* $(BUILDDIR)/pulseGUI.so

pulseGUI.so: pulseGUI.o pulseAnalysis.so AnalysisEventDict.so
	LIBS=$(wildcard $(LIB)/ReadMat.so)
ifeq ($(LIBS),)
	$(CC) $(SHAREDCFLAGS) $(ROOTLIBS) $(ROOTLFLAGS) -o $@ $(abspath $(patsubst %.so, $(LIB)/%.so,$^))
else
	$(CC) $(SHAREDCFLAGS) $(ROOTLIBS) $(ROOTLFLAGS) -o $@ $^ 
endif
	mv $@ $(BUILDDIR)/


AnalysisEventDict.so: AnalysisEventDict.cxx
	$(CC) $(SHAREDCFLAGS) -I. $^ -o $@
	mv $@ $(LIB)/

AnalysisEventDict.cxx:  pulseGUI.h pulseAnalysis.h LinkDef.h
	rm -f ./AnalysisEventDict.cxx ./AnalysisEventDict.h
	rootcint  $@ -c $^ 

pulseAnalysis.so: pulseAnalysis.cxx ReadMat.so ReadTXT.so pATools.o
	LIBS=$(wildcard $(LIB)/ReadMat.so)
ifeq ($(LIBS),)
	$(CC) $(SHAREDCFLAGS) -o $@  $(ROOTLIBS) $(abspath $(patsubst %.so, $(LIB)/%.so,$^)) 
else
	$(CC) $(SHAREDCFLAGS) -o $@  $(ROOTLIBS) $^
endif
	mv $@ $(LIB)/


pATools.o: pATools.cxx 
	$(CC) $(NONSHARED) $(abspath $(patsubst %.so, $(LIB)/%.so,$^)) 


pulseGUI.o: pulseGUI.cxx
	$(CC) $(NONSHARED) $^


ReadMat.so: ReadMat.cxx 
	$(CC) $(SHAREDCFLAGS) $^ -o $@
	mv $@ $(LIB)/

ReadTXT.so: ReadTxt.cxx 
	$(CC) $(SHAREDCFLAGS) $^ -o $@
	mv $@ $(LIB)/

pythonHelper_C.so: Compile.C pythonHelper.C
	root -b $^

