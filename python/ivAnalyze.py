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
from ROOT import TGraphErrors
from ivtools import *


class ivAnalyze():
    def Reset(self):
        # dark data
        self.V=array("f")     # Voltage points array, dark measurement
        self.Id=array("f")    # Dark Current
        # light data
        self.LV=array("f")    # Voltage points array, light measurement
        self.Itot=array("f")   # for light

        self.Vbar=array("f")  # V for 1/dV plots
        self.dLogIdV=array("f")   

        self.ratioMax=[0,0]    # location of peak light/dark ratio 
        self.vPeak=0
        self.vKnee=0
        # graphs
        self.gIdV=None         # I_dark vs V graph
        self.gItotV=None       # I_light vs V graph
        self.gIpV=None         # I_light-dark vs V graph
        self.gdLnIddV=None     # dark dLnIdV graph
        self.gdVdLnId=None     # inverse, [dLnIdV]^-1 graph
        self.gIpV=None         # light - dark current vs V
        self.gRatio=None       # light to dark ratio
        self.gGain=None        # gain vs V 
        # parameters
        self.VMIN=10           # default minimum voltage to read
        self.G1FITFRAC=0.5     # fit range for currents at G~1 as fraction of vPeak 
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

    # read dark I-V data and estimate Vbr
    def Analyze(self, dorebin=False):
        readVIfile(self.fnIV,self.V,self.Id,self.VMIN)
        self.gIdV=TGraph(len(self.V), self.V, self.Id)
        self.gIdV.SetTitle("I-V Curve;Volts;Current [Amps]")
        self.gdLnIddV=calc_dLogIdV(self.V,self.Id)
        self.gdVdLnId=InvertGraph(self.gdLnIddV)
        self.vPeak=maxXY(self.gdLnIddV)[0]
        if self.doLightAnalysis: self.AnalyzeLight()
        fitFcn=TF1("fitFcn","[0]+exp(-[1]*(x-[2]))",-80,80)
        fitFcn.SetParameters(0.05,5,self.vPeak) # guess at starting params
        if self.vPeak<0:
            self.gdLnIddV.Fit(fitFcn,"","",self.vPeak,self.vPeak+5)
        else:
            self.gdLnIddV.Fit(fitFcn,"","",self.vPeak-5,self.vPeak)
        self.vKnee=fitFcn.GetParameter(2)
        return self.vPeak,self.vKnee,self.ratioMax

    def AnalyzeLight(self):
        #generate Light/Dark Ratio
        #Nota bene! I'm not doing any error checking with this, so if you have files with 
        #different voltage ranges or data points this won't work! 
        print "Also analyzing illuminated I-V curve"
        readVIfile(self.fnLIV,self.LV,self.Itot,self.VMIN)
        self.gItotV=TGraph(len(self.LV), self.LV, self.Itot)
        self.gItotV.SetTitle("I-V Curve (light);Volts;Current [Amps]")
        self.gItotV.SetName("LIV")

        # to do: calc [dlogItot/dV]-1, use to estimate Vbr
        
        npoints=len(self.Itot)
        if not npoints==len(self.Id):
            print "Dark/light files of different length:",len(self.Itot),len(self.Id)
            sys.exit(1)

        # ratio of light to dark IV curves
        self.gRatio = RatioGraph(self.gItotV,self.gIdV)
        self.ratioMax=maxXY(self.gRatio)
            
        # make graph of Ip = light-dark currents
        # use restricted range around Gain=1 (low voltage) region
        vdeltaI=array("f")
        deltaI=array("f")
        for i in range(npoints):
            if abs(self.V[i])<abs(self.vPeak)*self.G1FITFRAC:
                vdeltaI.append(self.V[i])
                deltaI.append(self.Itot[i]-self.Id[i])
        self.gIpV= TGraph(len(vdeltaI), vdeltaI, deltaI)
                
        # Extrapolate the Ip=Itot-Id curve to 0V to estimate the signal at Gain=1
        # Smooth the graph of light-dark current before fitting
        gsmooth = SmoothGraph(self.gIpV)
        gDeltaIerr=TGraphErrors(npoints) # assign some errors for stable fit
        x=Double(); y=Double()
        gsmooth.GetPoint(len(vdeltaI)-1,x,y) # To do: better to see minimum y!
        y0=float(y)
        wgt0=y/20 # minimum weight for fit values
        for i in range(len(vdeltaI)):
            gsmooth.GetPoint(i,x,y)
            gDeltaIerr.SetPoint(i,x,y)
            gDeltaIerr.SetPointError(i,0,wgt0*y/y0) 
        self.gIpV = TGraph(gsmooth)
        self.gIpV = TGraphErrors(gDeltaIerr)
        self.gIpV.SetTitle("Ilight-Idark Smoothed")
        # estimate additional current for gain=1
        # use vertex form of parabola, set to constanst for |x|<vertex
        # abs(x)<abs(h)?k:a*(x-h)**2+k
        print "Fitting for Ip at G=1: abs(x)<abs(h)?k:a*(x-h)**2+k"
        g1fit=TF1("G1fit","abs(x)<abs([0])?[1]:[2]*(x-[0])*(x-[0])+[1]",
                  self.vPeak*self.G1FITFRAC,self.VMIN)
        g1fit.SetParNames("h","k","a")
        g1fit.SetParameters(self.V[0]*2,self.Id[0],self.Id[0]) # guesstimate params
        self.gIpV.Fit(g1fit,"R")
        IatGain1=g1fit.GetParameter(1) # current Ip at unity gain
        # gain calculation
        self.gGain=TGraph(npoints)
        for i in range(npoints):
            g=(self.Itot[i]-self.Id[i])/IatGain1
            self.gGain.SetPoint(i,self.V[i],g)
        # testing...
        # fit for gain from Vpeak_ratio to ~ Vpeak_gain,
        # extrapolate to G=1 to estimate Vbr
        #gxmax,gymax=maxXY(self.gGain)
        #print gxmax,gymax,self.ratioMax[0],self.gGain.Eval(self.ratioMax[0])
        #print "******",gxmax*0.96,self.ratioMax[0]*0.95
        #g0fit=TF1("G0fit","pol1",gxmax*0.95,self.ratioMax[0])
        #self.gGain.Fit("G0fit","R")
        #p0=g0fit.GetParameter(0)
        #p1=g0fit.GetParameter(1)
        #g0fit=TF1("G1fit","abs(x)<abs([0])?[1]:[2]*(x-[0])*(x-[0])+[1]",
        #          gxmax*0.9,self.ratioMax[0])
        #g0fit.SetParameters(self.ratioMax[0],1,??)
        #self.gGain.Fit("G0fit","R")
        #self.gGain.Print()
