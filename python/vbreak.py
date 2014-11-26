#!/usr/bin/python
# Look at gain vs voltage to find Vbreak

import sys, os, commands, glob, time, math
import optparse
from ivtools import *

ROOTLIBS=commands.getoutput("root-config --libdir")
sys.path.append(ROOTLIBS)

from ROOT import gSystem
gSystem.Load("../build/pulseGUI.so")
gSystem.Load("../build/pythonHelper_C.so")

from ROOT import pulseAnalysis, TString, TF1, TGraphErrors, TCanvas, TVirtualFitter
from array import array 

def printf(format, *args):
    sys.stdout.write(format % args)



if len(sys.argv) < 2:
    print "Usage:",sys.argv[0],"[-b] file name list"
    print "example: ./vbreak.py DataDir/Package-1-Device-1-33.*"
    print "-b : runs root in batch mode, no delay at finish"
    sys.exit(1)

parser = optparse.OptionParser() 
parser.add_option('-b', '--batch', dest='batch', action="store_true")
(options, args) = parser.parse_args()


    
p=pulseAnalysis()
V=array("d")
res=array("d")
err=array("d")
zero=array("d")
for arg in args:
    if os.path.isfile(arg):
        p.LoadSpectrum(arg)
        p.FindPeaks()
        p.Analyze()
        V.append( float(getField(arg,"V")) )
        res.append( p.Hpi().GetMean() )
        err.append( p.Hpi().GetRMS()/math.sqrt(p.Hpi().GetEntries()) )
        zero.append(0)

tc=TCanvas("tc","Relative gain")
tg=TGraphErrors(len(V),V,res,zero,err)
tg.SetTitle("Pulse integrals vs voltage")
tg.Fit("pol1")
tg.Draw("AP*")
fitfcn=tg.GetFunction("pol1")
b=fitfcn.GetParameter(0);
m=fitfcn.GetParameter(1);
vbreak=-b/m

# retrieve error matrix
fitter = TVirtualFitter.GetFitter()
npar=2
errm = matrix = [[0]*npar for i in range(npar)]

dfdb=1
dfdm=vbreak

for i in range(npar):
    for j in range (npar):
        errm[i][j]=fitter.GetCovarianceMatrixElement(i,j)
        

deltaVb=math.sqrt(dfdb*dfdb*errm[0][0]+2*dfdb*dfdm*errm[0][1]+dfdm*dfdm*errm[1][1])


printf("Vbreak=%5.2f +- %5.2f\n",vbreak,deltaVb)


if not options.batch:
    time.sleep(5)





