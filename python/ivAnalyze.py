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
        self.results={}
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
        self.VbrMIN=40         # absolute value of lower limit for breakdown region
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
        gsmooth.SetName("gIpLowV_sm")
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


    # read I-V data and estimate Vbr
    def Analyze(self, dorebin=False):
        status=readVIfile(self.fnIV,self.V,self.Id,self.VMIN,self.VMAX)
        if self.doLightAnalysis:
            status=status+readVIfile(self.fnLIV,self.LV,self.Itot,self.VMIN,self.VMAX)
        if not status==0:
            print "Error reading input file(s)",self.fnIV,self.fnLIV
            return None
        if self.V[-1]<0: self.vSign=-1
        else: self.vSign=1
        if self.doLightAnalysis:
            # due to current limit settings dark/light data may have different number of points
            # we assume the starting values and steps are the same and truncate to the shorter length
            if not len(self.V)==len(self.LV):
                nKeep=min(len(self.V),len(self.LV))
                self.V=self.V[0:nKeep-1]
                self.Id=self.Id[0:nKeep-1]
                self.LV=self.LV[0:nKeep-1]
                self.Itot=self.Itot[0:nKeep-1]
                
        self.gIdV=TGraph(len(self.V), self.V, self.Id)
        self.gIdV.SetName("gIdV")
        self.gIdV.SetTitle("I-V Curve;Volts;Current [Amps]")
        self.gdLnIddV=IV2dLogIdV(self.gIdV)
        self.gdLnIddV.SetName("gdLnIddV")
        self.gdLnIddV.SetTitle("dLn(I_{d})/dV;Volts;dLn(I_{d})/dV")
        # take 2nd derivative
        self.gd2LnIddV2=TGraphDerivative(self.gdLnIddV)      #d^2logI/dV^2
        self.gd2LnIddV2=TGraphScale(self.gd2LnIddV2,self.vSign)
        self.gd2LnIddV2.SetName("gd2LnIddV2")
        self.gd2LnIddV2.SetTitle("d^{2}Ln(I_{d})/d^{2}V;Volts;d^{2}Ln(I_{d})/d^{2}V")
        self.gd2LnIddV2.SetLineStyle(3)
        self.gdVdLnId=TGraphInvert(self.gdLnIddV)
        self.gdVdLnId.SetName("gdVdLnId")
        self.gdVdLnId.SetTitle("1/(dLn(I_{d})/dV);Volts;1/(dLn(I_{d})/dV)")
        if self.vSign<0:
            self.vPeak=GraphMax(self.gdLnIddV,umax=-1*self.VbrMIN)[0]
        else: self.vPeak=GraphMax(self.gdLnIddV,umin=self.VbrMIN)[0]

        self.results={}
        self.results["fnLIV"]=self.fnLIV
        self.results["vPeak"]=self.vPeak
       
        
        if self.doLightAnalysis:
            self.AnalyzeLight()
        else:
            self.results["vPeakIp"]=0
            self.results["LDRmax"]=0
            self.results["M(Vop)"]=0
            self.results["I90"]=0
            self.results["I60"]=0
            self.results["I30"]=0
            
        # currents at fixed fractions of Vbr
        vref=self.vPeak
        if self.doLightAnalysis: vref=self.vPeakIp
        self.results["I90"]=self.gIdV.Eval(vref*0.9)
        self.results["I60"]=self.gIdV.Eval(vref*0.6)
        self.results["I30"]=self.gIdV.Eval(vref*0.3)

        # fit for exponential slope of leakage current
        if self.vSign<0:
            xstart=vref*0.95
            xstop=vref*0.6
        else:
            xstop=vref*0.95
            xstart=vref*0.6
        self.gIdV.Fit("expo","q","",xstart,xstop)
        leakFcn=self.gIdV.GetFunction("expo")
        self.results["leakConst"]=leakFcn.GetParameter(0)
        self.results["leakSlope"]=leakFcn.GetParameter(1)
        self.results["leakAtVbr"]=leakFcn.Eval(vref)
        return self.results
        
    def AnalyzeLight(self):
        # generate Light/Dark Ratio        
        # to do: calc [dlogItot/dV]-1, use to estimate Vbr
        
        npoints=len(self.Itot)
        self.results["fnLIV"]=self.fnLIV
            
        # define graphs
        self.gItotV=TGraph(len(self.LV), self.LV, self.Itot)
        self.gItotV.SetTitle("I-V Curve (light);Volts;Current [Amps]")
        self.gItotV.SetName("gItotV")
            

        # make graph of Ip = light+leakage-dark currents
        self.gIpV=TGraphDiff(self.gItotV,self.gIdV)
        self.gIpV.SetName("gIpV"); self.gIpV.SetTitle("I-V Curve (light-dark);Volts;Current [Amps]")
        self.gIpV.SetLineColor(kBlue)
        self.gdLnIpdV=IV2dLogIdV(self.gIpV)
        self.gdLnIpdV.SetName("gdLnIpdV"); self.gdLnIpdV.SetTitle("dLn(I_{dp/dV;Volts;dLn(I_{p})/dV")
        self.gdLnIpdV.SetLineColor(kBlue)
        # HACK to avoid noise at low/high V.  Limit range to middle 80%
        # of voltage range
        self.vPeakIp=GraphMax(self.gdLnIpdV,window=0.9)[0]  
        self.results["vPeakIp"]=self.vPeakIp  # Estimate of Vbr-try w/ pol2 fit to 3 highest points
        self.gIpLowV=TGraph(self.gIpV)
        self.gIpLowV.SetName("gIpLowV")
        Vi=Double(); Ii=Double()
        for i in range(npoints):
            self.gIpLowV.GetPoint(i,Vi,Ii)
            if abs(Vi)>abs(self.vPeak)*self.G1FITFRAC:
                self.gIpLowV.RemovePoint(i)

        # ratio of light to dark IV curves
        self.gLDRatio = TGraphDivide(self.gItotV,self.gIdV)
        #self.gLDRatio = TGraphDivide(self.gIpV,self.gIdV)
        self.gLDRatio.SetTitle("LDRatio")
        self.gLDRatio.SetName("LDRatio")
        self.LDRmax=GraphMax(self.gLDRatio,
                             self.vPeakIp-abs(self.vPeakIp/4),
                             self.vPeakIp+abs(self.vPeakIp/4))
        self.results["LDRmax"]=self.LDRmax

                
        # estimate the light current Ip at Gain~1
        IatGain1=self.CalcIatGain1()
        # gain calculation
        self.gGain=TGraph(npoints)
        for i in range(npoints):
            g=(self.Itot[i]-self.Id[i])/IatGain1
            self.gGain.SetPoint(i,self.V[i],g)
        self.results["M(Vop)"]=self.gGain.Eval(self.LDRmax[0])
