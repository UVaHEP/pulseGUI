#!/usr/bin/python
#######################################################
# analyse reverse bias I-V data
# for simplicity, all biases will be treated as POSITIVE voltages
#######################################################

import ROOT as r
from ivtools import *

class ivAnalyze():
    
    def __init__(self,fnIV=None,fnLIV=None):
        self.Reset()
        if fnIV: self.SetData(fnIV,fnLIV)

    def Reset(self):
        # raw dark/light data, nupmy arrays
        self.nVd=None     
        self.nId=None     # dark current
        self.nVl=None     
        self.nIl=None     # light current
        self.vSign=1      # Sign of bias voltage
        self.doLightAnalysis=False
        # trimmed data
        self.vmin=0       # cutoffs for small/noisy signal region
        self.imin=0
        self.ntrim=0
        self.tVd=None     
        self.tId=None    
        self.tVl=None
        self.tIl=None
        # derived data
        self.tIp=None     # photon current
        self.nVex=None
        self.dlnIddV=None
        self.dlnIpdV=None
        self.nLDratio=None
        self.Vbr=0          # best Vbr estimate in case of multiple calcs
        self.IdVbr=0        # Vbr from 1st deriv of log(I_dark)
        self.IpVbr=0        # Vbr from 1st deriv of log(I_light-I_dark)
        self.vLDratioMax=0  # voltage of max light/dark current
        

    # set dark(light) data file name(s), read in data
    def SetData(self,fnIV,fnLIV=None):
        self.Reset()
        self.fnIV=fnIV         # dark I-V data file
        self.fnLIV=fnLIV       # light I-V data file
        self.doLightAnalysis = (fnLIV != None)
        try:
            self.nVd,self.nId=readVIfile2(fnIV)
            self.nId=np.abs(self.nId)                    # enforce +I
            if self.doLightAnalysis:
                self.nVl,self.nIl=readVIfile2(fnLIV)
                self.nIl=np.abs(self.nIl)
                self.AlignIV()
        except Exception, e:
            print("Error reading input file(s)",fnIV,fnLIV)
        if np.sign(self.nVd[1])<0:
            self.nVd=-1*self.nVd  # enforce +V
            self.nVl=-1*self.nVl
        
    # retrieve measured data graphs
    def GetVIgraph(self,type="dark"):
        v=np.array(self.tVd,'d')
        if type=="dark":
            tg=r.TGraph(self.ntrim,v,np.array(self.tId,'d'))
            tg.SetLineColor(r.kBlack)
        elif type=="light":
            tg=r.TGraph(self.ntrim,v,np.array(self.tIl,'d'))
            tg.SetLineColor(r.kGreen)
        elif type=="photon":              # light-dark data
            tg=r.TGraph(self.ntrim,v,np.array(self.tIp,'d'))
            tg.SetLineColor(r.kBlue)
        else:
            print("Invalid graph requested:",type)      
        tg.SetLineWidth(2)
        return tg

    # graph for Vbr determination
    def GetdLogIgraph(self,type="dark",vmin=None,imin=None):
        if vmin==None: vmin=self.vmin
        if imin==None: imin=self.imin

        if type=="dark":
            tg=r.TGraph(self.ntrim,np.array(self.tVd,'d'),
                        np.array(self.dlnIddV,'d'))
            tg.SetTitle("Breakdown analysis;Volts;dlog(I_{d})/dV")
            tg.SetLineColor(r.kGray+1)
        elif type=="photon":              # light-dark data
            tg=r.TGraph(self.ntrim,np.array(self.tVd,'d'),
                        np.array(self.dlnIpdV,'d'))
            tg.SetTitle("Breakdown analysis;Volts;dlog(I_{p})/dV")
            tg.SetLineColor(r.kBlue)
        tg.SetLineWidth(2)
        # remove low voltage/current values, these may have large noise
        # use reversed range, b/c NPoints changes!
        for i in range(self.ntrim):
            if self.tVd[i]<vmin or self.tId[i]<imin: tg.RemovePoint(0)
            else: break
        return tg

    # graphs for light/dark ratios and gain
    def GetLDgraph(self,type="ratio",vmin=None, imin=1e-9):
        if vmin==None: vmin=self.IpVbr*0.75
        if type=="ratio":
            tg=r.TGraph(self.ntrim,np.array(self.tVd,'d'),
                        np.array(self.nLDratio,'d'))
            tg.SetTitle("L/D ratio;Volts;light/dark currents")
            tg.SetLineColor(r.kBlack)
        elif type=="gain":
            #estimate gain=1 at Vbr/2
            idx=np.abs(self.tVd - self.IpVbr/2).argmin()
            nM=(self.tIl-self.tId)/(self.tIl[idx]-self.tId[idx])
            tg=r.TGraph(self.ntrim,np.array(self.tVd,'d'),
                        np.array(nM,'d'))
            tg.SetTitle("DC Gain;Volts;Gain")
            tg.SetLineColor(r.kGreen)
        tg.SetLineWidth(2)
        # remove low voltage/current values, these may have large noise
        # use reversed range, b/c NPoints changes!
        for i in range(self.ntrim):
            if self.tVd[i]<vmin or self.tId[i]<imin: tg.RemovePoint(0)
            else: break
        return tg   # how do we force the graph to recale axes after remonving points???

    
    # Due to current limit settings dark/light data may have
    # different number of points. We assume the starting values
    # and steps are the same and truncate to the shorter length
    def AlignIV(self):
        if len(self.nVd)>len(self.nVl):
            nKeep=len(self.nVl)
            self.nVd=self.nVd[0:nKeep]
            self.nId=self.nId[0:nKeep]

    def SetVmin(self, vmin):
        self.VMIN=vmin
    def SetVmax(self, vmax):
        self.VMAX=vmax  

    def TrimVIdata(self):
        vmin=self.vmin
        imin=self.imin
        tVd=filter(lambda x: x>0, map(lambda v,i,vmin=vmin,imin=imin: (v>=vmin)*(i>imin)*v, self.nVd,self.nId))
        tId=filter(lambda x: x>0, map(lambda v,i,vmin=vmin,imin=imin: (v>=vmin)*(i>imin)*i, self.nVd,self.nId))
        self.tVd=np.array(tVd,'d')  # map/filter turns np array into list, grumble!
        self.tId=np.array(tId,'d')
        self.ntrim=len(tVd)
        if self.doLightAnalysis:
            tIl=filter(lambda x: x>0, map(lambda v,i,vmin=vmin: (v>=vmin)*i, self.nVd,self.nIl))
            self.tIl=np.array(tIl,dtype=float)
        
    # read I-V data and estimate Vbr
    def Analyze(self, vmin=5, imin=1e-9):
        # make derived data
        self.vmin=vmin
        self.imin=imin
        self.TrimVIdata()
        lnI=np.log10(self.tId)
        self.dlnIddV=abs(np.gradient(lnI,self.tVd))
        self.IdVbr=self.tVd[ np.argmax(self.dlnIddV) ]
        self.Vbr=self.IdVbr
        if self.doLightAnalysis:
            self.tIp=self.tIl-self.tId
            lnI=np.log10(self.tIp)
            self.dlnIpdV=np.abs(np.gradient(lnI,self.tVd))
            self.IpVbr=self.tVd[ np.argmax(self.dlnIpdV) ]
            self.Vbr=self.IpVbr
            self.nLDratio=self.tIp/self.tId
            self.vLDratioMax=self.tVd[ np.argmax(self.nLDratio) ]

################# Depricated #################

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
        print("Graphs written to:",filename)
        
    # calculate the current Ip for at Gain~1 using a fit to
    # I_light-dark vs V graph at low voltage and extrapolting to V=0
    # return I_p(G=1)
    def CalcIatGain1(self):
        # quick and dirty, return i(light)-i(dark) @ 30V
        ip30=0
        for i in range(len(self.V)):
            if self.V[i]<-11: break
            ip30=self.Itot[i]-self.Id[i]
        return ip30
        # Smooth the graph of light-dark current before fitting
        gsmooth = TGraphSmoother(self.gIpLowV)
        gsmooth.SetName("gIpLowV_sm")
        gDeltaIerr=r.TGraphErrors(self.gIpLowV.GetN()) # assign some *incorrect* errors for stable fit
        xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
        Vi=Double(); Ii=Double()
        self.gIpLowV.ComputeRange(xmin,ymin,xmax,ymax)
        #print "***",xmin,ymin,xmax,ymax
        y0=float(ymin)
        wgt0=ymin/20 # HACK! minimum weight for fit values
        for i in range(self.gIpLowV.GetN()):
            gsmooth.GetPoint(i,Vi,Ii)
            gDeltaIerr.SetPoint(i,Vi,Ii)
            gDeltaIerr.SetPointError(i,0,wgt0*Ii/y0)
        self.gIpLowV = r.TGraph(gsmooth)
        self.gIpLowV = r.TGraphErrors(gDeltaIerr)
        self.gIpLowV.SetTitle("Ilight-Idark Smoothed")
        # estimate additional current for light at gain=1 
        # use vertex form of parabola, set to constant for |x|<vertex
        # abs(x)<abs(h)?k:a*(x-h)**2+k
        print("Fitting for Ip at G=1: abs(x)<abs(h)?k:a*(x-h)**2+k")
        g1fit=TF1("G1fit","abs(x)<abs([0])?[1]:[2]*(x-[0])*(x-[0])+[1]",
                  self.vPeak*self.G1FITFRAC,self.VMIN)
        g1fit.SetParNames("h","k","a")
        g1fit.SetParameters(self.V[0]*2,self.Id[0],self.Id[0]) # guesstimate params
        self.gIpLowV.Fit(g1fit,"R")
        return g1fit.GetParameter(1) # current Ip at unity gain


