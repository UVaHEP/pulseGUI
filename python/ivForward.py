#!/usr/bin/python
#######################################################
# analyze forward bias I-V data
#######################################################
import commands,sys
# keep ROOT TApplication from grabbing -h flag
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
from ROOT import *

rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

import os
import getopt, string
from array import array
from math import log
import argparse 
from plotStyle import *
from ivtools import *


#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('file', nargs='*', default=None,
                    help="I-V data file to process")
parser.add_argument('-f', metavar='--filename', type=str, nargs=1, default=None,
                    help="I-V data file to process [-f is optional]")
parser.add_argument('-n', default=1,
                    help="Number of SPADs in device")
parser.add_argument('-s', '--rs', default=0,
                    help="Series resistance in device circuit")
parser.add_argument('-a', '--auto', action="store_true",
                    help="Run and exit w/o user interaction")
parser.add_argument('-x', '--xmin', default=0,
                    help="starting point of fit")
parser.add_argument('-X', '--xmax', default=0,
                    help="ending point of fit")
args = parser.parse_args()

if args.f is None and args.file is None: 
    print 'No I-V data file to process...quitting'
    exit(1)

if args.f is None: filename=args.file[0]
else: filename=args.f[0]
print "filename",filename
    
xmin=float(args.xmin)
xmax=float(args.xmax)

if (xmax-xmin)<=0:
    print "Undefined fit range, specify xmin,xmax"
    exit(1)

    
nSPADs=int(args.n)
rSeries=float(args.rs)  # series resistance


FIT_FRAC_MIN=0.9 # fraction of highest voltage for start of linear fit

SetPlotStyle() # pretty up plots

# variables for analysis of IV data
V=array("d")
I=array("d")
#readVIfile(args.f[0],V,I)
readVIfile(filename,V,I)
for i in range(len(V)):
    V[i]=abs(V[i])
    I[i]=abs(I[i])

# correct for additional series resistance
if rSeries>0: # V_diode=V_tot-V_Rseries
    for i in range(len(V)): V[i]=V[i]-I[i]*rSeries

gIV=TGraph(len(V), V, I)
gIV.SetTitle("Current vs. forward voltage;V;I [A]")

# do linear fit to graph
gIV.Fit("pol1","","",xmin,xmax)
pol1=gIV.GetFunction("pol1")
Rfit=1.0/abs(pol1.GetParameter(1))
print "***",gIV.GetFunction("pol1"),Rfit

# load minuit fitting code
#gROOT.ProcessLine(".L python/ForwardIVfitter.C+")
#from ROOT import ForwardIVfitter
#fitter=ForwardIVfitter(gIV,nSPADs)
#par=array("d",[0,0,0])
#fitter.Fit(par)


# plotting
c_IV = TCanvas("cIV","I-V",800,800)
gIV.Draw("APL")
#fitter.GetFitGraph().Draw("L")

c_IV.Update()

print "=== Quench Resistance Analysis ==="
printf("Number of SPADs: %6d\n",nSPADs)
printf("Current supply series resistance: %4.0f Ohms\n",rSeries)
printf("Average quench resistance / SPAD: %6.2e Ohms\n",Rfit*nSPADs)
print "===================="




if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')





