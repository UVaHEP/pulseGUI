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
rSeries=float(args.rs)/1000  # series resistance in kOhm

# ROOT messes with arg parsing, import after instantiating parser
rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)
from ROOT import Double, kRed, kBlue
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F
from plotStyle import *
from ivtools import *

SetPlotStyle() # pretty up plots

# variables for analysis of IV data
V=array("d")
I=array("d")
readVIfile(args.f[0],V,I)
for i in range(len(I)): I[i]=I[i]*1000 # convert to mA for nicer y-axis in graph

gIV=TGraph(len(V), V, I)
gIV.SetTitle("Current vs. voltage;V;I [mA]")

# analysis, fit for resistance
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
fitFcn=TF1("Rfit","[0]+x/[1]",1.5,xmax)
fitFcn.SetParameters(0,(ymax-ymin)/(xmax-xmin)) # guess at starting params
gIV.Fit(fitFcn,"R")
QR=(fitFcn.GetParameter(1)-rSeries)*1000 # convert to Ohms


# plotting
c_IV = TCanvas("cIV","I-V",800,800)
gIV.Draw("APL")
fitFcn.Draw("same")
c_IV.Update()


# print results
print "=== Quench Resistance Analysis ==="
printf("Current supply series resistance: %4.0f Ohms\n",rSeries*1000)
printf("Device resistance: %5.0f Ohms\n",QR)
printf("Average quence resistance / SPAD: %6.2e Ohms\n",QR*nSPADs)
print "===================="




if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')





