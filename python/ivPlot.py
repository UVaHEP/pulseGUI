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

from ROOT import Double, gStyle, kGreen, kRed, kTeal, kBlue, TGraphSmooth
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

if doLightAnalysis: ana=ivAnalyze(dfn,lfn)
else: ana=ivAnalyze(dfn)
ana.SetVmin(VMIN)

vPeak,vKnee,ratioMax=ana.Analyze()


# analysis done, get with the plots

gStyle.SetOptStat(0)
# graphs
gIV=ana.gIdV
gIV.SetName("IV")
gIV.SetLineWidth(2)

gDV = ana.gdLnIddV
gDV.SetName("dLogI/dV")
gDV.SetLineWidth(2)

if doLightAnalysis:
    gRatio = ana.gRatio
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

IMIN=1e-9
canvas.cd(1).SetLogy()
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
gIV.SetMinimum(IMIN)
hIV=TH2F("hIV","I-V Curve;Volts;Current [Amps]",10,xmin,xmax,10,ymin,ymax*1.1)
hIV.GetYaxis().SetTitleOffset(1.4)
hIV.Draw()
gIV.Draw("L")
a=TLine(vPeak,IMIN,vPeak,IMIN*2)
a.SetLineColor(kRed)
a.Draw()
b=TLine(vKnee,IMIN,vKnee,IMIN*2)
b.SetLineColor(kBlue)
b.Draw()
if doLightAnalysis:
    gLIV=ana.gItotV
    gLIV.SetLineWidth(2)
    gLIV.SetLineColor(kGreen)
    gLIV.Draw("L")


canvas.cd(2)
gDV.ComputeRange(xmin,ymin,xmax,ymax)
gDV.SetTitle("Breakdown analysis;Volts;dlog(I)/dV");
if vPeak<0: 
    gDVframe=TH2F("dvFrame",gDV.GetTitle(),5,xmin,vPeak/2,5,0,ymax*1.1)
else:
    gDVframe=TH2F("dvFrame",gDV.GetTitle(),5,xmin/2,xmax,5,0,ymax*1.1)
gDVframe.Draw()
gDV.Draw("L")
tlVpeak=TLine(vPeak,ymin+(ymax-ymin)/2,vPeak,ymax*1.08)
tlVpeak.SetLineColor(kRed)
tlVpeak.Draw("same")
fitFcn=gDV.GetFunction("fitFcn")
fitFcn.SetLineColor(kTeal)
tlVknee=TLine(vKnee, gDV.GetYaxis().GetXmin(),vKnee,fitFcn.GetParameter(0))
tlVknee.SetLineColor(kBlue)
tlVknee.SetLineWidth(2)
tlVknee.Draw("same")
labVbr=TPaveText(0.50,0.76,0.89,0.90,"NDC")
msg="Vpeak="+("%5.2f" % vPeak)
labVbr.AddText(msg)
msg="Vknee="+("%5.2f" % vKnee)
labVbr.AddText(msg)
labVbr.Draw()

if doLightAnalysis:
    #Draw the Ratio of Light to Dark Curves on canvas 3 
    canvas.cd(3) #.SetLogy()
    gRatio.ComputeRange(xmin, ymin, xmax, ymax)
    gRatio.SetTitle("Ratio of Light to Dark;Volts;Current Ratio [A]")
    if vPeak<0: 
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,xmin,vPeak/2,10,0,ymax*1.1)
    else:
        gRframe=TH2F("grFrame",gRatio.GetTitle(),10,VPeak/2,xmax,10,0,ymax*1.1)
    gRframe.Draw()
    gRatio.Draw("L")
    RatioMax = TLine(ratioMax[0], ymin+(ymax-ymin)/2, ratioMax[0], ymax*1.08)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
    labRat=TPaveText(0.50,0.83,0.89,0.90,"NDC")
    msg="Vmax(L/D)="+("%5.2f" % ratioMax[0])
    labRat.AddText(msg)
    labRat.Draw()
    canvas.Update()
    if options.gPoint:
        gPoint=options.gPoint
        gPoint=math.copysign(float(gPoint),vPeak) # voltage to calculate gain, w/ correct sign convention
        gPointGain=gGain.Eval(gPoint)
    plot,axis=scaleToPad(gGain)
    plot.Draw("L")
    axis.SetTitleOffset(1.3)
    #axis.SetNoExponent(True)
    axis.SetMaxDigits(3)
    axis.Draw()

canvas.Update()

print "=== I-V Analysis ==="
printf("Peak dLogI/DV: %4.2f\n",vPeak)
printf("Knee dLogI/DV: %4.2f\n",vKnee)
if doLightAnalysis:
    printf("Peak light/dark: %4.2f\n",ratioMax[0])
    if options.gPoint:
        printf("Gain at %4.1f V: %6.0f\n",gPoint,gPointGain)
print "===================="

# diagnostic tests
#ana.gIpV.Draw("AP*X")
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

