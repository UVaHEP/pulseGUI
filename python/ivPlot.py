#!/usr/bin/python
#######################################################
# analyze reverse bias I-V data
#######################################################

### 5-30-2014 Updated by Thomas Anderson to add a ratio of Light to Dark I-V curves
### if given an extra csv file 
### Consider the ratio generator in a prototype stage, i.e. no/little error checking

import sys, os, commands, re
import getopt, string, math
from array import array
import optparse
from ivAnalyze import ivAnalyze


rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

from ROOT import Double, gStyle, kGreen, kRed, kTeal, kBlue, kGray, TGraphSmooth
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F, TPaveText
from ivtools import *
from rootTools import *

VMIN=10  # minimum voltage to read



class Thermistor():
    def __init__ (self):
        self.graph=TGraph("KT103J2_TvR.dat")
        
    def Temperature(self,R):
        return self.graph.Eval(R)

    
    

#######################
# main
#######################

# TO DO: convert to argparse.ArgumentParser
parser = optparse.OptionParser() 
parser.add_option('-o', '--output', dest='outfn', default=None)
parser.add_option('-p', '--png', dest='png', action="store_true")
parser.add_option('-b', '--batch', dest='batch', action="store_true")
parser.add_option('-f', '--darkfn', dest='dfn', default=None)
parser.add_option('-l', '--lightfn', dest='lfn', default=None)
parser.add_option('-g', '--gainPoint', dest='gPoint', default=None)
parser.add_option('-x', '--debug', dest='doDebug', default=None)


(options, args) = parser.parse_args()
dfn=None
lfn=None
if options.dfn: dfn=options.dfn
if options.lfn: lfn=options.lfn

if options.dfn==None and len(args)>0: dfn=args[0]
if options.lfn==None and len(args)>1: lfn=args[1]


if dfn is None: 
    print 'No I-V data file to process...quitting'
    exit(0)


doLightAnalysis = not (lfn is None)
doDebug=not (options.doDebug is None)

if doLightAnalysis: ana=ivAnalyze(dfn,lfn)
else: ana=ivAnalyze(dfn)
ana.SetVmin(VMIN)


ana.Analyze()

# analysis done, get with the plots

gStyle.SetOptStat(0)
#### graphs
gIV=ana.gIdV         # dark I-V graph
gIV.SetName("IV")
gIV.SetLineWidth(2)

gDV = ana.gdLnIddV   # dark dLogI/dV
gDV.SetName("dLogI/dV")
gDV.SetLineWidth(2)

if doLightAnalysis:
    gdLnIpdV=ana.gdLnIpdV        # dLogIp/dV for photo current
    gRatio = ana.gRatio          # light to dark current ratio
    gRatio.SetName("LD_ratio")
    gRatio.SetLineWidth(2)
    gGain=ana.gGain
    gGain.SetName("Gain")
    gGain.SetLineWidth(1)
    gGain.SetLineColor(kGreen+3)

#canvas = TCanvas("ivdata","I-V Data",800,400)
canvas = TCanvas("ivdata",os.path.basename(dfn),800,400)
if doLightAnalysis:
    canvas.Divide(3,1)
else:
    canvas.Divide(2,1)

#### Canvas 1:  Dark and light I-V curves
    
IMIN=1e-9
canvas.cd(1).SetLogy()
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
gIV.SetMinimum(IMIN)
hIV=TH2F("hIV","I-V Curve;Volts;Current [Amps]",10,xmin,xmax,10,ymin,ymax*1.1)
hIV.GetYaxis().SetTitleOffset(1.4)
hIV.Draw()
gIV.Draw("L")

if doLightAnalysis:
    gLIV=ana.gItotV
    gLIV.SetLineWidth(2)
    gLIV.SetLineColor(kGreen)
    gLIV.Draw("L")

#### Canvas 2: V_breakdown analysis
# To do: think about possibility of some
#         dark count analysis using dLnI/dV vs dLnIp/dV

canvas.cd(2)
title2=TString()
if doLightAnalysis:
    gdLnIpdV.ComputeRange(xmin,ymin,xmax,ymax) # dLogI/dV for photo current
    gdLnIpdV.SetTitle("Breakdown analysis;Volts;dlog(I_{p})/dV")
    title2=gdLnIpdV.GetTitle()
else:
    gDV.ComputeRange(xmin,ymin,xmax,ymax)          # dark dLogI/dV
    gDV.SetTitle("Breakdown analysis;Volts;dlog(I)/dV")
    title2=gDV.GetTitle()
    
if ana.vPeak<0: 
    gDVframe=TH2F("dvFrame",title2,5,xmin,ana.vPeak/2,5,0,ymax*1.1)
else:
    gDVframe=TH2F("dvFrame",title2,5,xmin/2,xmax,5,0,ymax*1.1)


gDVframe.Draw()

labVbr=TPaveText(0.50,0.76,0.89,0.90,"NDC")
if doLightAnalysis:
    gDV.SetLineColor(kGray)
    gDV.Draw("L")
    msg="VpIp="+("%5.2f" % ana.vPeakIp)
    labVbr.AddText(msg)
    gdLnIpdV.Draw("same")
else:
    gDV.Draw("L")


labVbr.Draw()

##### Canvas 3: Gain

if doLightAnalysis:
    # Draw the Ratio of Light to Dark Curves
    canvas.cd(3) #.SetLogy()
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts;Current Ratio [A]")
    if ana.vPeak<0: 
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,xmin,ana.vPeak/2,10,0,ymax*1.1)
    else:
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,ana.vPeak/2,xmax,10,0,ymax*1.1)
    gRframe.Draw()
    gRatio.Draw("L")
    RatioMax = TLine(ana.ratioMax[0], ymin+(ymax-ymin)/2, ana.ratioMax[0], ymax*1.08)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
    labRat=TPaveText(0.50,0.83,0.89,0.90,"NDC")
    msg="Vmax(L/D)="+("%5.2f" % ana.ratioMax[0])
    labRat.AddText(msg)
    labRat.Draw()
    canvas.Update()
    if options.gPoint:
        gPoint=options.gPoint
        gPoint=math.copysign(float(gPoint),ana.vPeak) # voltage to calculate gain, w/ correct sign convention
        gPointGain=gGain.Eval(gPoint)
    plot,axis=scaleToPad(gGain)
    plot.Draw("L")
    axis.SetTitleOffset(1.3)
    #axis.SetNoExponent(True)
    axis.SetMaxDigits(3)
    axis.Draw()

canvas.Update()

print "=== I-V Analysis ==="
printf("Peak dLogI/DV: %4.2f\n",ana.vPeak)
printf("Knee dLogI/DV: %4.2f\n",ana.vKnee)
if doLightAnalysis:
    printf("Peak light/dark: %4.2f\n",ana.ratioMax[0])
    printf("Vpeak dLogIp/DV: %4.2f\n",ana.vPeakIp)
    if options.gPoint:
        printf("Gain at %4.1f V: %6.0f\n",gPoint,gPointGain)
print "===================="

# diagnostic tests
c2=TCanvas()
ana.gIpLowV.Draw("AP*X")
#ana.gdVdLnId.Draw("AP*L")
canvas.Update()


#os.system('sleep 2')
if not options.batch:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')
    
if options.png:
    png=dfn.replace(".csv",".png")
    canvas.Print(png)
elif options.outfn:
    canvas.Print(options.outfn)

# temporary
#R=float(dfn.split(".c")[0].split("-")[4])*1000
#therm=Thermistor()
#print "Temperature:",therm.Temperature(R)

