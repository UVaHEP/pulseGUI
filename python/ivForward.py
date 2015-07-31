#!/usr/bin/python
#######################################################
# analyze forward bias I-V data
#######################################################

import sys, os, commands
import getopt, string
from array import array
from math import log
import argparse 

#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('-f', metavar='--filename', type=str, nargs=1, default=None,
                    help="I-V data file to process")
parser.add_argument('-n', default=1,
                    help="Number of SPADs in device")
parser.add_argument('-s', '--rs', default=0,
                    help="Series resistance in device circuit")
parser.add_argument('-a', '--auto', action="store_true",
                    help="Run and exit w/o user interaction")
args = parser.parse_args()

if args.f is None: 
    print 'No I-V data file to process...quitting'
    exit(0)

nSPADs=int(args.n)
rSeries=float(args.rs)  # series resistance

# ROOT messes with arg parsing, import after instantiating parser
rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)
from ROOT import Double, kRed, kBlue
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F
from ROOT import gROOT
from plotStyle import *
from ivtools import *

FIT_FRAC_MIN=0.9 # fraction of highest voltage for start of linear fit

SetPlotStyle() # pretty up plots

# variables for analysis of IV data
V=array("d")
I=array("d")
readVIfile(args.f[0],V,I)

# correct for additional series resistance
if rSeries>0: # V_diode=V_tot-V_Rseries
    for i in range(len(V)): V[i]=V[i]-I[i]*rSeries

gIV=TGraph(len(V), V, I)
gIV.SetTitle("Current vs. voltage;V;I [A]")

# load minuit fitting code
gROOT.ProcessLine(".L python/ForwardIVfitter.C+")
from ROOT import ForwardIVfitter
fitter=ForwardIVfitter(gIV,nSPADs)
par=array("d",[0,0,0])
fitter.Fit(par)

# plotting
c_IV = TCanvas("cIV","I-V",800,800)
gIV.Draw("APL")
fitter.GetFitGraph().Draw("L")

c_IV.Update()

print "=== Quench Resistance Analysis ==="
printf("Number of SPADs: %6d\n",nSPADs)
printf("Current supply series resistance: %4.0f Ohms\n",rSeries)
printf("Average quence resistance / SPAD: %6.2e Ohms\n",par[2])
print "===================="




if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')





