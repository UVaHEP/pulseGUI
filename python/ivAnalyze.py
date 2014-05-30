#!/usr/bin/python
#######################################################
# convert CSV file to tree
#######################################################

### 5-30-2014 Updated by Thomas Anderson to add a ratio of Light to Dark I-V curves
### if given an extra csv file 
### Consider the ratio generator in a prototype stage, i.e. no/little error checking


import sys, os, commands, re
import getopt, string
from array import array

import argparse 


rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

from ROOT import TFile, gROOT, gStyle, Double, kRed
from ROOT import TString, TCanvas, TMultiGraph, TGraph, TLine
from ivtools import *

def printf(format, *args):
    sys.stdout.write(format % args)

#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('-f', metavar='-d', type=str, nargs=1, default=None,
			    help="Filename to process")
parser.add_argument('-l', metavar='-l', type=str, nargs=1, default=None,
			    help="Optional illuminated filename to process")
args = parser.parse_args()



if args.f is None: 
    print 'No base file to process...quitting'
    exit(0)


# analysis of of IV data
V=array("d")
LV=array("d")
I=array("d")
LI=array("d")
LdIdVi=array("d")
lvbar=array("d")
ratio=array("d")
dIdVi=array("d")
vbar=array("d")

a=array("d")
b=array("d")

readVIcsv(args.f[0],V,I,vbar,dIdVi)


gRatio = None
ratioMax = [0,0]
#generate Light/Dark Ratio
#Nota bene! I'm not doing any error checking with this, so if you have files with 
#different voltage ranges or data points this won't work! 

if args.l is not None: 
    readVIcsv(args.l[0], LV, LI, lvbar, LdIdVi)
    for x in range(len(V)):
        if (LI[x]/I[x]) > ratioMax[1]:
            ratioMax = [V[x], LI[x]/I[x]]
        ratio.append(LI[x]/I[x])
    gRatio = TGraph(len(V), V, ratio)



    

idx=getMaxIdx(dIdVi)
vbr=vbar[idx]           # estimate from peak

if (False) :   # skip for now
    vFitF=TF1()
    vbr=fitVbr(vbar,dIdVi,vFitF)  # estimate from fit

print "Estimate of Vbr"
printf("Peak 1/I dI/DV: %3.1f\n",vbr)

#print "Knee of 1/I dI/DV", vbr

gIV = TGraph(len(V), V, I)
gDV = TGraph(len(vbar), vbar, dIdVi)


canvas = TCanvas("c2","c2",800,400)

if args.l is None:
    canvas.Divide(2,1)
else:
    canvas.Divide(3,1)

canvas.cd(1).SetLogy()
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
gIV.SetTitle("I-V Curve;Volts;Current [Amps]");
gIV.SetMinimum(10e-9)
gIV.Draw("APC")
Vbreak=TLine(vbr,ymin,vbr,ymax/2)
Vbreak.SetLineColor(kRed)
#Vbreak=TLine(55.75,1e-8,55.75,I[len(V)-1])
Vbreak.Draw("same")

canvas.cd(2)
gDV.ComputeRange(xmin,ymin,xmax,ymax)
gDV.Draw("APC")
gDV.SetTitle("Breakdown analysis;Volts;1/I dI/DV");
Vbreak2=TLine(vbr,ymin,vbr,ymax*1.08)
Vbreak2.SetLineColor(kRed)
Vbreak2.Draw("same")


if args.l is not None: 
    #Draw the Ratio of Light to Dark Curves on canvas 3 
    canvas.cd(3)
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts; Current Ratio [A]")
    gRatio.Draw("APC")
    RatioMax = TLine(ratioMax[0], ymin, ratioMax[0], ymax*1.08)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
    print "%s V, %sx is the max Ratio of Light current to Dark current" % (ratioMax[0], ratioMax[1])

#vfitf.Draw("same")


os.system('sleep 2')

png=args.f[0].replace(".csv",".png")

canvas.Print(png)



