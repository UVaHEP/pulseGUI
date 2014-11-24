# Just for testing.  Not needed with main Makefie above

# here we access the root configuration, include files, and libraries
ROOTCFLAGS=$(shell root-config --cflags)
ROOTINC=$(shell root-config --incdir)
ROOTLIBDIR=$(shell root-config --libdir)
ROOTLIBS=$(shell root-config --libs) -lMinuit
ROOTLDFLAGS=$(shell root-config --ldflags)

ROOTC=$(ROOTCFLAGS)
CPPFLAGS=-O -Wall -I../include 
#CPPFLAGS=-g -Wall


ROOTLINK=-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTLDFLAGS) 

default: pulseEXE convertPS

pulseEXE: pulseEXE.cc
	g++ $(CPPFLAGS) $(ROOTC) -o pulseEXE pulseEXE.cc -L../build/lib -lpulseGUI $(ROOTLINK) 

convertPS: convertPS.cc
	g++ $(CPPFLAGS) $(ROOTC) -o convertPS convertPS.cc -L../build/lib -lpulseGUI $(ROOTLINK)

clean: 
	rm -f pulseEXE convertPS *.d *.so

