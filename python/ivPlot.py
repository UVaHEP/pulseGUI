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

from ROOT import *
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
    gGain.SetName("gGain")
    gGain.SetLineWidth(2)
    gGain.SetLineColor(kGreen+1)
    

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
leg1=TLegend(0.5,0.75,0.9,0.9)
leg1.AddEntry(gIV,"I-V Dark","l")

if doLightAnalysis:
    gLIV=ana.gItotV
    gLIV.SetLineWidth(2)
    gLIV.SetLineColor(kGreen)
    gLIV.Draw("L")
    ana.gIpV.Draw("L")
    leg1.AddEntry(gLIV,"I-V Light","l")
    leg1.AddEntry(ana.gIpV,"Light-Dark","l")
    
leg1.Draw()
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
    gDVframe=TH2F("dvFrame",title2,5,xmin,min(ana.vPeak*0.75,xmin/2),5,0,ymax*1.1)
else:
    gDVframe=TH2F("dvFrame",title2,5,max(ana.vPeak*0.75,xmax/2),xmax,5,0,ymax*1.1)


gDVframe.Draw()

leg2=TLegend(0.5,0.75,0.9,0.9)
if doLightAnalysis:
    gDV.SetLineColor(kGray+2)
    gDV.Draw("L")
    msg="VpI_p="+("%5.2f" % ana.vPeakIp)
    gdLnIpdV.Draw("same")
    leg2.AddEntry(gdLnIpdV,"dlogI_p/dV","l")
    leg2.AddEntry(0,msg,"")
else:
    msg="VpId="+("%5.2f" % ana.vPeak)
    leg2.AddEntry(0,msg,"")
    gDV.Draw("L")

leg2.AddEntry(gDV,"dlogI_d/dV","l")

leg2.Draw()

##### Canvas 3: Gain

if doLightAnalysis:
    leg3=TLegend(0.5,0.75,0.9,0.9)
    # Draw the Ratio of Light to Dark Curves
    canvas.cd(3) #.SetLogy()
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts;Current Ratio [A]")
    if ana.vPeak<0: 
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,xmin,xmin*0.75,10,0.1,ymax*1.1)
    else:
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,xmax*0,75,xmax,10,0.1,ymax*1.1)
    gRframe.Draw()
    gRatio.Draw("L")
    RatioMax = TLine(ana.ratioMax[0], ymin, ana.ratioMax[0], ymax/2)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
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
    leg3.AddEntry("gRatio","L/D ratio","l")
    leg3.AddEntry("gGain","Gain [I_p/I_p(@30)]","l")
    msg="Vmax(L/D)="+("%5.2f" % ana.ratioMax[0])
    leg3.AddEntry(0,msg,"")
    leg3.Draw()

    canvas.Update()

print "=== I-V Analysis ==="
printf("Peak dLogI/DV: %4.2f\n",ana.vPeak)
printf("Knee dLogI/DV: %4.2f\n",ana.vKnee)
printf("Dark Current @ 30 40 50V: %6.2e %6.2e %6.2e\n",gIV.Eval(-30),gIV.Eval(-40),gIV.Eval(-50))
if doLightAnalysis:
    printf("Peak light/dark: %4.2f\n",ana.ratioMax[0])
    printf("Vpeak dLogIp/DV: %4.2f\n",ana.vPeakIp)
    if options.gPoint:
        printf("Gain at %4.1f V: %6.0f\n",gPoint,gPointGain)
print "===================="

# diagnostic tests
#c2=TCanvas()
#ana.gIpLowV.Draw("AP*X")
#ana.gdVdLnId.Draw("AP*L")
#canvas.Update()


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

