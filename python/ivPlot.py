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
from ivAnalyze import ivAnalyze


rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

from ROOT import Double, gStyle, kRed, kBlue, kGreen, kTeal
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F, TPaveText
from ivtools import *

VMIN=10  # minimum voltage to read

#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('-f', metavar='-d', type=str, nargs=1, default=None,
			    help="I-V data file to process")
parser.add_argument('-l', metavar='-l', type=str, nargs=1, default=None,
			    help="Optional illuminated I-V data file")
parser.add_argument('-p', '--png',
                    help="Create PNG file from canvas", action="store_true")
parser.add_argument('-a', '--auto',
                    help="Run and exit w/o user interaction", action="store_true")
args = parser.parse_args()


if args.f is None: 
    print 'No I-V data file to process...quitting'
    exit(0)
doLightAnalysis = not (args.l is None)

if doLightAnalysis: ana=ivAnalyze(args.f[0],args.l[0])
else: ana=ivAnalyze(args.f[0])

vPeak,vKnee,ratioMax=ana.Analyze()

# analysis done, get with the plots

gStyle.SetOptStat(0)
# graphs
gIV=ana.gIV
gIV.SetName("IV")
gIV.SetLineWidth(2)

gDV = ana.gDV
gDV.SetName("dLogI/dV")
gDV.SetLineWidth(2)

if doLightAnalysis:
    gRatio = ana.gRatio
    gRatio.SetName("LD_ratio")
    gRatio.SetLineWidth(2)

#canvas = TCanvas("ivdata","I-V Data",800,400)
canvas = TCanvas("ivdata",args.f[0],800,400)
if doLightAnalysis:
    canvas.Divide(3,1)
else:
    canvas.Divide(2,1)

IMIN=1e-9
canvas.cd(1).SetLogy()
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
gIV.SetTitle("I-V Curve;Volts;Current [Amps]");
gIV.SetMinimum(IMIN)
gIV.Draw("AL")
a=TLine(vPeak,IMIN,vPeak,IMIN*2)
a.SetLineColor(kRed)
a.Draw()
b=TLine(vKnee,IMIN,vKnee,IMIN*2)
b.SetLineColor(kBlue)
b.Draw()
if doLightAnalysis:
    gLIV=TGraph(len(ana.V), ana.LV, ana.LI)
    gLIV.SetTitle("I-V Curve (light);Volts;Current [Amps]");
    gLIV.SetName("LIV")
    gLIV.SetLineWidth(2)
    gLIV.SetLineColor(kGreen)
    gLIV.Draw("L")

canvas.cd(2)
gDV.ComputeRange(xmin,ymin,xmax,ymax)
gDV.Draw("AL")
gDV.SetTitle("Breakdown analysis;Volts;dlog(I)/dV");
tlVpeak=TLine(vPeak,ymin+(ymax-ymin)/2,vPeak,ymax*1.08)
tlVpeak.SetLineColor(kRed)
tlVpeak.Draw("same")
fitFcn=gDV.GetFunction("fitFcn")
fitFcn.SetLineColor(kTeal)
tlVknee=TLine(vKnee, gDV.GetYaxis().GetXmin(),vKnee,fitFcn.GetParameter(0))
tlVknee.SetLineColor(kBlue)
tlVknee.SetLineWidth(2)
tlVknee.Draw("same")
labVbr=TPaveText(0.50,0.76,0.90,0.90,"NDC")
msg="Vpeak="+("%5.2f" % vPeak)
labVbr.AddText(msg)
msg="Vknee="+("%5.2f" % vKnee)
labVbr.AddText(msg)
labVbr.Draw()

if doLightAnalysis:
    #Draw the Ratio of Light to Dark Curves on canvas 3 
    canvas.cd(3)
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts;Current Ratio [A]")
    gRframe=TH2F("grFrame",gRatio.GetTitle(),10,xmin,xmax,10,1,ymax*1.1)
    gRframe.Draw()
    gRatio.Draw("L")
    RatioMax = TLine(ratioMax[0], ymin+(ymax-ymin)/2, ratioMax[0], ymax*1.08)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
    labRat=TPaveText(0.50,0.83,0.90,0.90,"NDC")
    msg="Vpeak="+("%5.2f" % ratioMax[0])
    labRat.AddText(msg)
    labRat.Draw()

canvas.Update()

print "=== I-V Analysis ==="
printf("Peak dLogI/DV: %4.2f\n",vPeak)
printf("Knee dLogI/DV: %4.2f\n",vKnee)
if doLightAnalysis: printf("Peak light/dark: %4.2f\n",ratioMax[0])
print "===================="

#os.system('sleep 2')
if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')

if args.png:
    png=args.f[0].replace(".csv",".png")
    canvas.Print(png)


