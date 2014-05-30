#!/usr/bin/python
#######################################################
# convert CSV file to tree
#######################################################

import sys, os, commands, re
import getopt, string
from array import array

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
if len(sys.argv)<2:
    print "Usage:",sys.argv[0],"csv_file_name"
    sys.exit()

dFile=sys.argv[1]
if not os.path.isfile(dFile):
    print dFile,"not found - exit on error"
    sys.exit()


# optional name of data file w/ illumination 
if len(sys.argv)==3:
    fLight=sys.argv[2]
    if not os.path.isfile(fLight):
        print fLight,"not found - exit on error"
        sys.exit()


# analysis of of IV data
V=array("d")
I=array("d")
dIdVi=array("d")
vbar=array("d")

a=array("d")
b=array("d")

readVIcsv(dFile,V,I,vbar,dIdVi)
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
canvas.Divide(2,1)

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

#vFitF.Draw("same")


os.system('sleep 2')

png=dFile.replace(".csv",".png")

canvas.Print(png)



