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


from ROOT import Double, gStyle, kRed, kBlue, kGreen, kTeal
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F, TPaveText, TSpectrum
from ROOT import TGraphErrors, TGraphSmooth
from ivtools import *



class ivAnalyze():
    def Reset(self):
        self.V=array("f")    # I-V
        self.I=array("f")    #   for dark
        self.Vbar=array("f")     # V for 1/dV plots
        self.dLogIdV=array("f")   
        self.LV=array("f")   # I-V
        self.LI=array("f")   #   for light
        self.G=array("f")    # gain
        self.rLD=array("f")  # light to dark current ratio
        self.ratioMax=[0,0]  # location of peak light/dark ratio 
        self.vPeak=0
        self.vKnee=0
        self.gIV=None        # I vs V graph
        self.gDV=None        # dLogIdV graph
        self.gDeltaI=None    # light - dark current vs V
        self.gRatio=None     # light to dark ratio
        self.gGain=None      # gain vs V estimate
        self.VMIN=10         # default minimum voltage to read
        self.G1FITFRAC=0.4   # fit range for current at G=1 as fraction of vPeak 
    def __init__(self,fnIV=None,fnLIV=None):
        self.Reset()
        self.fnIV=fnIV    # dark I-V data
        self.fnLIV=fnLIV  # light I-V data
        self.doLightAnalysis = not (fnLIV is None)
    def SetData(self,fnIV,fnLIV=None):
        self.Reset()
        self.fnIV=fnIV  
        self.fnLIV=fnLIV
        self.doLightAnalysis= not (fnLIV is None)
    def SetVmin(self, vmin):
        self.VMIN=vmin
    def Analyze(self, dorebin=False):
        # read dark I-V data and estimate Vbr
        readVIfile(self.fnIV,self.V,self.I,self.VMIN)
        calc_dLogIdV(self.V,self.I,self.dLogIdV,self.Vbar)
        self.vPeak=getMaxXY(self.Vbar,self.dLogIdV)[0]  # estimate of Vbr from peak
        if self.doLightAnalysis: self.AnalyzeLight()
        self.gIV=TGraph(len(self.V), self.V, self.I)
        self.gDV=TGraph(len(self.Vbar), self.Vbar, self.dLogIdV)
        fitFcn=TF1("fitFcn","[0]+exp(-[1]*(x-[2]))",-80,80)
        fitFcn.SetParameters(0.05,5,self.vPeak) # guess at starting params
        if self.vPeak<0:
            self.gDV.Fit(fitFcn,"","",self.vPeak,self.vPeak+5)
        else:
            self.gDV.Fit(fitFcn,"","",self.vPeak-5,self.vPeak)
        self.vKnee=fitFcn.GetParameter(2)
        return self.vPeak,self.vKnee,self.ratioMax

    def AnalyzeLight(self):
        #generate Light/Dark Ratio
        #Nota bene! I'm not doing any error checking with this, so if you have files with 
        #different voltage ranges or data points this won't work! 
        print "Also analyzing illuminated I-V curve"
        readVIfile(self.fnLIV,self.LV,self.LI,self.VMIN)

        npoints=len(self.LI)
        if not npoints==len(self.I):
            print "Dark/light files of different length:",len(self.LI),len(self.I)
            sys.exit(1)

        dV=999
        iMid=0
        print "Seeking current at VPeak*0.3:",self.vPeak*0.3
        vdeltaI=array("f")
        deltaI=array("f")
        for i in range(npoints):
            r=self.LI[i]/self.I[i]
            #print i,self.V[i],r,self.LI[i],self.I[i]
            if (r) > self.ratioMax[1]:
                self.ratioMax = [self.V[i], r]
            self.rLD.append(r)
                
            if abs(self.V[i])<abs(self.vPeak)*self.G1FITFRAC:
                vdeltaI.append(self.V[i])
                deltaI.append(self.LI[i]-self.I[i])
                
            if abs(self.V[i]-self.vPeak*0.3) < dV:
                dV=abs(self.V[i]-self.vPeak*0.3)
                iMid=self.I[i]    # current at vPeak*0.3

        self.gRatio = TGraph(npoints, self.V, self.rLD)
        # Smooth the graph of light-dark current before fitting
        self.gDeltaI= TGraph(len(vdeltaI), vdeltaI, deltaI)
        gs = TGraphSmooth("normal")
        gsmooth = gs.SmoothSuper(self.gDeltaI,"",0)
        gDeltaIerr=TGraphErrors(npoints)
        x=Double(); y=Double()
        for i in range(len(vdeltaI)):
            gsmooth.GetPoint(i,x,y)
            gDeltaIerr.SetPoint(i,x,y)
            gDeltaIerr.SetPointError(i,0,2e-10) # estimate 200pA measurement errors
        self.gDeltaI = TGraph(gsmooth)
        self.gDeltaI = TGraphErrors(gDeltaIerr)
        # estimate additional current for gain=1
        g1fit=TF1("G1fit","[0]*(1+exp((TMath::Abs(x)-[1])/[2]))",self.vPeak*self.G1FITFRAC,self.VMIN)
        g1fit.SetParameters(self.I[0],abs(self.vPeak/2),abs(self.vPeak/5)) # guesstimate params
        self.gDeltaI.Fit(g1fit,"R")
        IatGain1=g1fit.GetParameter(0)
        # gain analysis
        # (I_light-I_dark) / (I_light-I_dark)@Vbr/2 [iMid]
        for i in range(len(self.V)):
            #self.G.append( (self.LI[i]-self.I[i])/iMid )
            self.G.append( (self.LI[i]-self.I[i])/IatGain1 )
        self.gGain=TGraph(len(self.V), self.V, self.G)
