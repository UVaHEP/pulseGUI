#!/usr/bin/python
#######################################################
# analyze reverse bias I-V data
#######################################################

# keep ROOT TApplication from grabbing -h flag
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
from ROOT import *

import sys, os, commands, re
import getopt, string, math
from array import array
import optparse
from ivAnalyze import ivAnalyze
import argparse

rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

from ivtools import *
from rootTools import *


VMIN=5  # minimum voltage to read
IMIN=1e-9
VMAX=100 # maximum voltage to read


#######################
# main
#######################

#parser = argparse.ArgumentParser(description='I-V Plotter') 
#parser.add_argument('files', nargs='*', help="params: IV data filename(s)")
#args = parser.parse_args()

# TO DO: convert to argparse.ArgumentParser
parser = optparse.OptionParser() 
parser.add_option('-o', '--output', dest='outfn', default=None)
parser.add_option('-p', '--png', dest='png', action="store_true")
parser.add_option('-d', '--dir', dest='dir', default=None)
parser.add_option('-b', '--batch', dest='batch', action="store_true")
parser.add_option('-f', '--darkfn', dest='dfn', default=None)
parser.add_option('-l', '--lightfn', dest='lfn', default=None)
parser.add_option('-w', '--write', dest='writeGraphs', action="store_true")
parser.add_option('-m', '--minV', dest='minV', type="int", default=None)
parser.add_option('-M', '--maxV', dest='maxV', type="int", default=None)
parser.add_option('-0', '--noPlot', dest='noPlot', default=None, action="store_true")
parser.add_option('-P', '--pubStyle', dest='pubStyle', action="store_true")


(options, args) = parser.parse_args()
dfn=None
lfn=None
if options.dir==None: indir=""
else: indir=options.dir+"/"
if options.dfn: dfn=indir+options.dfn
if options.lfn: lfn=indir+options.lfn

if options.dfn==None and len(args)>0: dfn=indir+args[0]
if options.lfn==None and len(args)>1: lfn=indir+args[1]

if dfn is None: 
    print 'No I-V data file to process...quitting'
    exit(0)

if not options.minV==None: VMIN=options.minV
if not options.maxV==None: VMAX=options.maxV


if lfn: ana=ivAnalyze(dfn,lfn)
else: ana=ivAnalyze(dfn)

#ana.SetVmin(VMIN)
#ana.SetVmax(VMAX)
results=ana.Analyze(VMIN,IMIN)



print "\n============= I-V Analysis ============"
if ana.doLightAnalysis:
    VOp=ana.vLDratioMax
    VexOp=VOp-ana.Vbr
    printf("Vpeak dLogIp/DV (Vbr): %4.2f\n",ana.IpVbr)
    #printf("Peak light/dark (Vop): %4.2f   FWHM: %4.2f\n",VOp,results["LDRmax"][2])
    #printf("M(Vop): %6.2e at Vex: %6.2f\n",VOp,VexOp)
#else:    
    printf("Peak dLogId/DV: %4.2f\n",ana.IdVbr)

#printf("Dark Current @ 90%% 60%% 30%% of Vbr: %6.2e %6.2e %6.2e\n",results["I90"],results["I60"],results["I30"])
#printf("Leakage fit exp([0]+[1]*x), Ileak(Vbr) %7.3f %7.3f , %7.3f [nA]\n",results["leakConst"],results["leakSlope"],results["leakAtVbr"]*1e9)
print "======================================="



if (options.noPlot): sys.exit(0)
if options.pubStyle:
    gROOT.SetStyle("Pub")

####################################################
# analysis done, make plots
####################################################

gStyle.SetOptStat(0)
#### graphs
#gIV=ana.gIdV         # dark I-V graph
#gIV.SetLineWidth(2)

#gDV = ana.gdLnIddV   # dark dLogI/dV
#gDV.SetLineWidth(2)

#if ana.doLightAnalysis:
    #gdLnIpdV=ana.gdLnIpdV            # dLogIp/dV for photo current
    #gLDRatio = ana.gLDRatio          # light to dark current ratio
    #gLDRatio.SetName("LD_ratio")
    #gLDRatio.SetLineWidth(2)
    #gGain=ana.gGain
    #gGain.SetName("gGain")
    #gGain.SetLineWidth(2)
    #gGain.SetLineColor(kGreen+1)
    

canvas = r.TCanvas("ivdata",os.path.basename(dfn),1200,600)
if ana.doLightAnalysis:
    canvas.Divide(3,1)
else:
    canvas.Divide(2,1)

#### Canvas 1:  Dark and light I-V curves
    
gDark=ana.GetVIgraph('dark')
xmin=r.Double(); xmax=r.Double(); ymin=r.Double(); ymax=r.Double()
gDark.ComputeRange(xmin,ymin,xmax,ymax)

# set range for plotting IV curves
imin=int( np.log10(ymin.real) )-1 ; imin=np.power(10.,imin)
imax=int( np.log10(ymax.real) )   ; imax=np.power(10.,imax)
imin=max(imin,2e-10)
hIV=TH2F("hIV","I-V Curve;Volts;Current [Amps]",10,xmin,xmax,
         10,imin,imax*1.1)
hIV.GetYaxis().SetTitleOffset(1.4)

canvas.cd(1).SetLogy()
hIV.Draw()
gDark.Draw("L")
leg1=TLegend(0.1,0.75,0.5,0.9)
leg1.AddEntry(gDark,"I-V Dark","l")

if ana.doLightAnalysis:
    gLight=ana.GetVIgraph('light')
    gLight.Draw("L")
    gPhoton=ana.GetVIgraph('photon')
    gPhoton.Draw("L")
    leg1.AddEntry(gLight,"I-V Light","l")
    leg1.AddEntry(gPhoton,"Light-Dark","l")
    
leg1.Draw()

#### Canvas 2: V_breakdown analysis
# To do: think about possibility of some
#         dark count analysis using dLnI/dV vs dLnIp/dV

canvas.cd(2)
title2=TString()

gdLnIddV=ana.GetdLogIgraph("dark",vmin=ana.Vbr/2)             # dark dLogI/dV

leg2=r.TLegend(0.1,0.75,0.5,0.9)
leg2.AddEntry(gdLnIddV,"dlogI_{d}/dV","l")

if ana.doLightAnalysis:
    gdLnIpdV=ana.GetdLogIgraph("photon",vmin=ana.Vbr/2) # dLogI/dV for I_photo
    gdLnIpdV.ComputeRange(xmin,ymin,xmax,ymax)  
    title2=gdLnIpdV.GetTitle()
else:
    gdLnIddV.ComputeRange(xmin,ymin,xmax,ymax)      
    title2=gdLnIddV.GetTitle()

gDVframe=TH2F("dvFrame",title2,2,xmin,xmax,2,0,ymax*1.1)
gDVframe.Draw()

if ana.doLightAnalysis:
    msg="Vbr="+("%5.2f" % ana.IpVbr)
    gdLnIpdV.Draw("l")
    leg2.AddEntry(gdLnIpdV,"dlogI_{p}/dV","l")
    leg2.AddEntry(0,msg,"")
else:
    msg="VpId="+("%5.2f" % ana.IdVbr)
    leg2.AddEntry(0,msg,"")
    
gdLnIddV.Draw("l")
leg2.Draw()

##### Canvas 3: Gain

if ana.doLightAnalysis:
    # Draw the Ratio of Light to Dark Curves
    canvas.cd(3) #.SetLogy()
    gLDRatio=ana.GetLDgraph("ratio",vmin=ana.Vbr*.75)
    gLDRatio.ComputeRange(xmin, ymin, xmax, ymax)
    vLDmax=ana.vLDratioMax
    gRframe=TH2F("grFrame",gLDRatio.GetTitle(),2,xmin,xmax,
                 2,0,ymax*1.1)
    gRframe.Draw()
    gLDRatio.Draw("L")

    RatioMax = TLine(ana.vLDratioMax, gRframe.GetYaxis().GetXmin(), ana.vLDratioMax, ymax)
    RatioMax.SetLineColor(kRed)
    RatioMax.Draw("same")
    canvas.Update()
    ylimit=None
    gGain=ana.GetLDgraph("gain")
    plot,axis=scaleToPad(gGain,ylimit)
    plot.Draw("L")
    axis.SetTitleOffset(1.3)
    axis.SetMaxDigits(3) #; axis.SetNoExponent(True)
    axis.Draw()
    leg3=r.TLegend(0.1,0.75,0.5,0.9)
    leg3.AddEntry("gLDRatio","L/D ratio","l")
    leg3.AddEntry("gGain","Gain [I_{p}/(I_{p}/2)]","l")
    #msg="Vop="+("%5.2f" % ana.LDRmax[0])
    leg3.AddEntry(0,msg,"")
    leg3.Draw()

    canvas.Update()

    

if options.png:
    png=dfn.replace(".csv",".png")
    canvas.Print(png)
elif options.outfn:
    canvas.Print(options.outfn)

if options.writeGraphs:
    tfout=dfn.replace(".csv",".root")
    ana.Write(tfout)
    

if not options.batch:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')
