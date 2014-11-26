#!/usr/bin/python
#######################################################
# analyse reverse bias I-V data
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

from ROOT import Double, kRed, kBlue
from ROOT import TString, TCanvas, TGraph, TLine, TF1
from ivtools import *


#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('-f', metavar='-d', type=str, nargs=1, default=None,
			    help="I-V data file to process")
parser.add_argument('-l', metavar='-l', type=str, nargs=1, default=None,
			    help="Optional illuminated I-V data file")
parser.add_argument('-a', '--auto',
                    help="Run and exit w/o user interaction", action="store_true")
args = parser.parse_args()


if args.f is None: 
    print 'No I-V data file to process...quitting'
    exit(0)


doLightAnalysis = not (args.l is None)

# variables for analysis of IV data
# dark
V=array("d")
I=array("d")
Vbar=array("d")
dLogIdV=array("d")


# read dark I-V data
readVIfile(args.f[0],V,I)
calc_dLogIdV(V,I,dLogIdV,Vbar)


# with light
LV=array("d")
LI=array("d")

#generate Light/Dark Ratio
#Nota bene! I'm not doing any error checking with this, so if you have files with 
#different voltage ranges or data points this won't work! 

rLD=array("d")  # light to dark current ratio
ratioMax=[0,0]
if doLightAnalysis:
    print "Also analyzing illuminated I-V curve"
    readVIfile(args.l[0],LV,LI)
#    calc_dLogIdV(LV,LI,LdLogIdV,LVbar)

    for x in range(len(V)):
        if (LI[x]/I[x]) > ratioMax[1]:
            ratioMax = [V[x], LI[x]/I[x]]
        rLD.append(LI[x]/I[x])


# graphs
gIV=TGraph(len(V), V, I)
gIV.SetName("IV")
gIV.SetLineWidth(2)

gDV = TGraph(len(Vbar), Vbar, dLogIdV)
gDV.SetName("dLogI/dV")
gDV.SetLineWidth(2)

if doLightAnalysis:
    gRatio = TGraph(len(V), V, rLD)
    gRatio.SetName("LD_ratio")
    gRatio.SetLineWidth(2)


# analysis
vPeak=getMaxXY(Vbar,dLogIdV)[0]  # estimate of Vbr from peak
fitFcn=TF1("fitFcn","[0]+exp(-[1]*(x-[2]))",-80,80)
fitFcn.SetParameters(0.05,5,vPeak) # guess at starting params
if vPeak<0:
    gDV.Fit(fitFcn,"","",vPeak,vPeak+5);
else:
    gDV.Fit(fitFcn,"","",vPeak-5,vPeak);
vKnee=fitFcn.GetParameter(2)


# plotting
canvas = TCanvas("c2","c2",800,400)
if doLightAnalysis:
    canvas.Divide(3,1)
else:
    canvas.Divide(2,1)

canvas.cd(1).SetLogy()
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
gIV.SetTitle("I-V Curve;Volts;Current [Amps]");
gIV.SetMinimum(10e-9)
gIV.Draw("APL")


canvas.cd(2)
gDV.ComputeRange(xmin,ymin,xmax,ymax)
gDV.Draw("APL")
gDV.SetTitle("Breakdown analysis;Volts;dlog(I)/DV");
tlVpeak=TLine(vPeak,ymin+(ymax-ymin)/2,vPeak,ymax*1.08)
tlVpeak.SetLineColor(kRed)
tlVpeak.Draw("same")
tlVknee=TLine(vKnee, gDV.GetYaxis().GetXmin(),vKnee,fitFcn.GetParameter(0))
tlVknee.SetLineColor(kBlue)
tlVknee.SetLineWidth(2)
tlVknee.Draw("same")


if doLightAnalysis:
    #Draw the Ratio of Light to Dark Curves on canvas 3 
    canvas.cd(3)
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts;Current Ratio [A]")
    gRatio.Draw("APC")
    RatioMax = TLine(ratioMax[0], ymin, ratioMax[0], ymax*1.08)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")

canvas.Update()

print "=== I-V Analysis ==="
printf("Peak dLogI/DV: %3.1f\n",vPeak)
printf("Knee dLogI/DV: %3.1f\n",vKnee)
if doLightAnalysis: printf("Peak light/dark: %3.1f\n",ratioMax[0])
print "===================="

#os.system('sleep 2')
if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')


png=args.f[0].replace(".csv",".png")

canvas.Print(png)



