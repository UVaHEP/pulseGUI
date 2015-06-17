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
from math import copysign
import argparse 


from ROOT import Double, gStyle, kRed, kBlue, kGreen, kTeal
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F, TPaveText, TSpectrum
from ROOT import TGraphErrors
from ivtools import *


class ivAnalyze():
    def Reset(self):
        # dark data
        self.V=array("f")      # Voltage points array, dark measurement
        self.Id=array("f")     # Dark Current
        # light data
        self.LV=array("f")     # Voltage points array, light measurement
        self.Itot=array("f")   # for light
        self.ratioMax=[0,0]    # location of peak light/dark ratio 
        self.vPeak=0           # peak of dark current dLnIdV graph
        self.vKnee=0
        self.vPeakIp=0         # peak of Ip current dLnIdV graph
        self.vSign=-1          # sign of bias voltage
        # graphs
        self.gIdV=None         # I_dark vs V graph
        self.gItotV=None       # I_light vs V graph
        self.gIpLowV=None      # I_light+leakage-dark (Ip) vs V graph at low voltage
        self.gIpV=None         # Ip vs V graph
        self.gdLnIddV=None     # dark current dLnIdV graph
        self.gd2LnIddV2=None   # dark current d^2logI/dV^2
        self.doubleD=None
        self.gdLnIpdV=None     # dLnIpdV graph
        self.gdVdLnId=None     # inverse, [dLnIdV]^-1 graph
        self.gRatio=None       # light to dark ratio
        self.gGain=None        # gain vs V 
        # parameters
        self.VMIN=10           # default minimum voltage to read
        self.G1FITFRAC=0.5     # fit range for currents at G~1 as fraction of vPeak 
    def __init__(self,fnIV=None,fnLIV=None):
        self.Reset()
        self.fnIV=fnIV         # dark I-V data file
        self.fnLIV=fnLIV       # light I-V data file
        self.doLightAnalysis = not (fnLIV is None)
    def SetData(self,fnIV,fnLIV=None):
        self.Reset()
        self.fnIV=fnIV  
        self.fnLIV=fnLIV
        self.doLightAnalysis= not (fnLIV is None)
    def SetVmin(self, vmin):
        self.VMIN=vmin

    # calculate the current Ip for at Gain~1 using a fit to
    # I_light-dark vs V graph at low voltage and extrapolting to V=0
    # return I_p(G=1)
    def CalcIatGain1(self):
        # Smooth the graph of light-dark current before fitting
        gsmooth = TGraphSmoother(self.gIpLowV)
        gDeltaIerr=TGraphErrors(self.gIpLowV.GetN()) # assign some *incorrect* errors for stable fit
        xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
        Vi=Double(); Ii=Double()
        self.gIpLowV.ComputeRange(xmin,ymin,xmax,ymax)
        print "***",xmin,ymin,xmax,ymax
        y0=float(ymin)
        wgt0=ymin/20 # HACK! minimum weight for fit values
        for i in range(self.gIpLowV.GetN()):
            gsmooth.GetPoint(i,Vi,Ii)
            gDeltaIerr.SetPoint(i,Vi,Ii)
            gDeltaIerr.SetPointError(i,0,wgt0*Ii/y0)
        self.gIpLowV = TGraph(gsmooth)
        self.gIpLowV = TGraphErrors(gDeltaIerr)
        self.gIpLowV.SetTitle("Ilight-Idark Smoothed")
        # estimate additional current for light at gain=1 
        # use vertex form of parabola, set to constant for |x|<vertex
        # abs(x)<abs(h)?k:a*(x-h)**2+k
        print "Fitting for Ip at G=1: abs(x)<abs(h)?k:a*(x-h)**2+k"
        g1fit=TF1("G1fit","abs(x)<abs([0])?[1]:[2]*(x-[0])*(x-[0])+[1]",
                  self.vPeak*self.G1FITFRAC,self.VMIN)
        g1fit.SetParNames("h","k","a")
        g1fit.SetParameters(self.V[0]*2,self.Id[0],self.Id[0]) # guesstimate params
        self.gIpLowV.Fit(g1fit,"R")
        return g1fit.GetParameter(1) # current Ip at unity gain


    # fit graph of dLogI/dV for knee at first rising edge
    def FitforKnee(self):
        fitFcn=TF1("fitFcn","[0]+exp(-[1]*(x-[2]))",-80,80)
        # guess at starting params
        if self.vSign<0: idxs=range(self.gdLnIddV.GetN()-1,-1,-1)
        else: idxs=range(self.gdLnIddV.GetN())
        xmax=1e-50; ymax=1e-50
        x=Double(); y=Double()
        for i in idxs:
            self.gdLnIddV.GetPoint(i,x,y)
            if float(y)>ymax:
                xmax=float(x)
                ymax=float(y)
            
        fitFcn.SetParameters(0.05,5,self.vPeak) # guess at starting params
        if self.vPeak<0:
            self.gdLnIddV.Fit(fitFcn,"","",self.vPeak,self.vPeak+5)
        else:
            self.gdLnIddV.Fit(fitFcn,"","",self.vPeak-5,self.vPeak)
        self.vKnee=fitFcn.GetParameter(2)

        
    # read dark I-V data and estimate Vbr
    def Analyze(self, dorebin=False):
        readVIfile(self.fnIV,self.V,self.Id,self.VMIN)
        self.gIdV=TGraph(len(self.V), self.V, self.Id)
        self.gIdV.SetTitle("I-V Curve;Volts;Current [Amps]")
        self.gdLnIddV=IV2dLogIdV(self.gIdV)
        # take 2nd derivative
        self.gd2LnIddV2=TGraphDerivative(self.gdLnIddV)      #d^2logI/dV^2
        self.gd2LnIddV2=TGraphScale(self.gd2LnIddV2,self.vSign)
        self.gd2LnIddV2.SetLineStyle(3)
        self.gdVdLnId=TGraphInvert(self.gdLnIddV)
        self.vPeak=GraphMax(self.gdLnIddV)[0]
        self.vSign=copysign(1,self.vPeak/100)
        if self.doLightAnalysis: self.AnalyzeLight()
        self.FitforKnee()

    def AnalyzeLight(self):
        #generate Light/Dark Ratio
        #Nota bene! I'm not doing any error checking here, so if you have files with 
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
        self.gRatio = TGraphDivide(self.gItotV,self.gIdV)
        self.ratioMax=GraphMax(self.gRatio)
            
        # make graph of Ip = light+leakage-dark currents
        self.gIpV=TGraphDiff(self.gItotV,self.gIdV)
        self.gdLnIpdV=IV2dLogIdV(self.gIpV)
        self.vPeakIp=GraphMax(self.gdLnIpdV)[0]
        
        # make restricted range version around Gain=1 (low voltage) region
        # To do, drop this and replace w/ fit range above, as in exmaple....
        # self.gdLnIddV.Fit(fitFcn,"","",self.vPeak,self.vPeak+5)
        self.gIpLowV=TGraph(self.gIpV)
        Vi=Double(); Ii=Double()
        for i in range(npoints):
            self.gIpLowV.GetPoint(i,Vi,Ii)
            if abs(Vi)>abs(self.vPeak)*self.G1FITFRAC:
                self.gIpLowV.RemovePoint(i)
                
        # estimate tle light current Ip at Gain~1
        IatGain1=self.CalcIatGain1()
        # gain calculation
        self.gGain=TGraph(npoints)
        for i in range(npoints):
            g=(self.Itot[i]-self.Id[i])/IatGain1
            self.gGain.SetPoint(i,self.V[i],g)

            
            
            
        # testing...
        # fit for gain from Vpeak_ratio to ~ Vpeak_gain,
        # extrapolate to G=1 to estimate Vbr
        #gxmax,gymax=GraphMax(self.gGain)
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
