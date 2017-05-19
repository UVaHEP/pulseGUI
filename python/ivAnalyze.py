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
import time

from ROOT import Double, gStyle, kRed, kBlue, kGreen, kTeal, TFile
from ROOT import TString, TCanvas, TGraph, TLine, TF1, TH2F, TPaveText, TSpectrum
from ROOT import TGraphErrors
from ivtools import *


class ivAnalyze():
    def Reset(self):
        # dark data
        self.V=array("f")      # Voltage points array, dark measurement
        self.Id=array("f")     # Dark Current
        self.vSign=-1          # Sign of bias voltage
        # light data
        self.LV=array("f")     # Voltage points array, light measurement
        self.Itot=array("f")   # Light Current
        # derived parameters
        self.vPeak=0           # Peak of dark current dLnIdV graph
        self.vPeakIp=0         # Peak of Ip current dLnIdV graph
        self.LDRmax=[0,0,0]    # Location of peak light/dark current ratio, est. of Vop [x,y, fwhm]
        self.lowVslope=0       # Slope of dark current before Vbr
        self.MVop=0            # Gain at operating voltage = LDRmax[0]
        # graphs
        self.gIdV=None         # I_dark vs V graph
        self.gItotV=None       # I_light vs V graph
        self.gIpLowV=None      # I_light+leakage-dark (Ip) vs V graph at low voltage
        self.gIpV=None         # Ip vs V graph
        self.gdLnIddV=None     # dark current dLnIdV graph
        self.gd2LnIddV2=None   # dark current d^2logI/dV^2
        self.gdLnIpdV=None     # dLogI/dV for photo current
        self.gdVdLnId=None     # inverse, [dLnIdV]^-1 graph
        self.gLDRatio=None     # light to dark ratio
        self.gGain=None        # gain vs V 
        # parameters
        self.VMIN=10           # default minimum voltage to read
        self.VMAX=80           # default maximum voltage to read
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
    def SetVmax(self, vmax):
        self.VMAX=vmax  
    def Write(self, filename):
        tf=TFile(filename,"recreate")
        self.gIdV.Write()
        self.gItotV.Write()
        self.gIpLowV.Write()
        self.gIpV.Write()
        self.gdLnIddV.Write()
        self.gd2LnIddV2.Write()
        self.gdLnIpdV.Write()
        self.gdVdLnId.Write()
        self.gLDRatio.Write()
        self.gGain.Write()
        tf.Close()
        print "Graphs written to:",filename
        
    # calculate the current Ip for at Gain~1 using a fit to
    # I_light-dark vs V graph at low voltage and extrapolting to V=0
    # return I_p(G=1)
    def CalcIatGain1(self):
        # quick and dirty, return i(light)-i(dark) @ 30V
        ip30=0
        for i in range(len(self.V)):
            if self.V[i]<-30: break
            ip30=self.Itot[i]-self.Id[i]
            #print i, self.V[i],self.Itot[i]-self.Id[i]
        return ip30
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


    # read dark I-V data and estimate Vbr
    def Analyze(self, dorebin=False):
        readVIfile(self.fnIV,self.V,self.Id,self.VMIN,self.VMAX)
        self.gIdV=TGraph(len(self.V), self.V, self.Id)
        self.gIdV.SetName("gIdV")
        self.gIdV.SetTitle("I-V Curve;Volts;Current [Amps]")
        self.gdLnIddV=IV2dLogIdV(self.gIdV)
        self.gdLnIddV.SetName("gdLnIddV")
        self.gdLnIddV.SetTitle("dLn(I_{d})/dV;Volts;dLn(I_{d})/dV")
        # take 2nd derivative
        self.gd2LnIddV2=TGraphDerivative(self.gdLnIddV)      #d^2logI/dV^2
        self.gd2LnIddV2.SetName("gd2LnIddV2")
        self.gd2LnIddV2.SetTitle("d^{2}Ln(I_{d})/d^{2}V;Volts;d^{2}Ln(I_{d})/d^{2}V")
        self.gd2LnIddV2=TGraphScale(self.gd2LnIddV2,self.vSign)
        self.gd2LnIddV2.SetLineStyle(3)
        self.gdVdLnId=TGraphInvert(self.gdLnIddV)
        self.vPeak=GraphMax(self.gdLnIddV)[0]
        self.vSign=copysign(1,self.vPeak/100)
        if self.doLightAnalysis: self.AnalyzeLight()
        results={}
        results["fnLIV"]=self.fnLIV
        results["vPeak"]=self.vPeak
        if self.doLightAnalysis:
            results["vPeakIp"]=self.vPeakIp
            results["LDRmax"]=self.LDRmax
            results["M(Vop)"]=self.gGain.Eval(self.LDRmax[0])
            results["I90"]=self.gIdV.Eval(self.vPeakIp*0.9)  # currents at fixed fractions of Vbr
            results["I60"]=self.gIdV.Eval(self.vPeakIp*0.6)
            results["I30"]=self.gIdV.Eval(self.vPeakIp*0.3)
        else:
            results["vPeakIp"]=0
            results["LDRmax"]=0
            results["M(Vop)"]=0
            results["I90"]=0
            results["I60"]=0
            results["I30"]=0
        return results
        
    def AnalyzeLight(self):
        #generate Light/Dark Ratio
        #Nota bene! I'm doing minimal error checking here, so if you have files with 
        #different voltage ranges or data points this might not work! 
        print "Also analyzing illuminated I-V curve"
        readVIfile(self.fnLIV,self.LV,self.Itot,self.VMIN,self.VMAX)

        # to do: calc [dlogItot/dV]-1, use to estimate Vbr
        
        npoints=len(self.Itot)
        if not npoints==len(self.Id):
            print "Dark/light files of different length:",len(self.Itot),len(self.Id)
            if len(self.Itot)<len(self.Id):
                print "Padding light data to match dark measurements, beware..."
                npoints=len(self.Id)
                for i in range(len(self.Itot),len(self.Id)):
                    self.LV.append(self.V[i])
                    self.Itot.append(1e-2)  # ~current limit
            else: sys.exit(1)

        # define graphs
        self.gItotV=TGraph(len(self.LV), self.LV, self.Itot)
        self.gItotV.SetTitle("I-V Curve (light);Volts;Current [Amps]")
        self.gItotV.SetName("LIV")
            
        # ratio of light to dark IV curves
        self.gLDRatio = TGraphDivide(self.gItotV,self.gIdV)
        self.gLDRatio.SetTitle("LDRatio")
        self.gLDRatio.SetName("LDRatio")
        self.LDRmax=GraphMax(self.gLDRatio,
                             self.vPeak-abs(self.vPeak/4),
                             self.vPeak+abs(self.vPeak/4))        
            
        # make graph of Ip = light+leakage-dark currents
        self.gIpV=TGraphDiff(self.gItotV,self.gIdV)
        self.gIpV.SetLineColor(kBlue)
        self.gdLnIpdV=IV2dLogIdV(self.gIpV)
        self.gdLnIpdV.SetLineColor(kBlue)
        self.vPeakIp=GraphMax(self.gdLnIpdV,-999,-40)[0] # HACK to get around noisy data at low voltages
        self.gIpLowV=TGraph(self.gIpV)
        Vi=Double(); Ii=Double()
        for i in range(npoints):
            self.gIpLowV.GetPoint(i,Vi,Ii)
            if abs(Vi)>abs(self.vPeak)*self.G1FITFRAC:
                self.gIpLowV.RemovePoint(i)
                
        # estimate the light current Ip at Gain~1
        IatGain1=self.CalcIatGain1()
        # gain calculation
        self.gGain=TGraph(npoints)
        for i in range(npoints):
            g=(self.Itot[i]-self.Id[i])/IatGain1
            self.gGain.SetPoint(i,self.V[i],g)

