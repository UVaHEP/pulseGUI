#!/usr/bin/python
#######################################################
# analyze forward bias I-V data
#######################################################


import sys, os, commands
import getopt, string
from array import array
from math import log
import argparse 

#######################
# main
#######################
parser = argparse.ArgumentParser(description='IV Curve Analyzer')
parser.add_argument('-f', metavar='--filename', type=str, nargs=1, default=None,
                    help="I-V data file to process")
parser.add_argument('-n', default=1,
                    help="Number of SPADs in device")
parser.add_argument('-a', '--auto',
                    help="Run and exit w/o user interaction")
args = parser.parse_args()


# ROOT messes with arg parsing, to wait to import
rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)
from ROOT import Double, kRed, kBlue
from ROOT import gStyle, TString, TCanvas, TGraph, TLine, TF1, TH2F
from ivtools import *


if args.f is None: 
    print 'No I-V data file to process...quitting'
    exit(0)

nSPADs=int(args.n)

# variables for analysis of IV data
V=array("d")
I=array("d")
readVIfile(args.f[0],V,I)

Vsample=array("d")
R=array("d")

for i in range(len(V)):
    if abs(I[i])<1e-9: continue
    r=abs(V[i]/I[i])
    Vsample.append(abs(V[i]))
    R.append(r)

# graphs
gIV=TGraph(len(Vsample), Vsample, R)
gIV.SetName("IV")
gIV.SetTitle("Resistance Curve;Volts;Resistance [Ohms]");
gIV.SetLineWidth(2)

# plotting
canvas = TCanvas("cR","Resistance",800,800)
canvas.Divide(1,2)

canvas.cd(1).SetLogy()
gIV.Draw("APL")

canvas.cd(2)
# make subsample in range of higher voltages
xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
gIV.ComputeRange(xmin,ymin,xmax,ymax)
V2=array("d")
R2=array("d")

for i in range(len(Vsample)):
    if R[i]<ymin*2: 
        V2.append(Vsample[i])
        R2.append(R[i])

gIV2=TGraph(len(V2), V2, R2)

gIV2.Draw("APL")
gIV2.SetName("IV2")
gIV2.SetTitle("Resistance Curve;Volts;Resistance [Ohms]");
gIV2.SetLineWidth(3)
gIV2.ComputeRange(xmin,ymin,xmax,ymax)

fitFcn=TF1("fitFcn","[0]+exp([1])*exp(-(x*[2]))",xmin,xmax)
print xmin,ymin,xmax,ymax
fitFcn.SetParameters(ymin,log(ymax),1) # guess at starting params
gIV2.Fit(fitFcn,"0")
fitFcn.Draw("same")

canvas.cd(1)
fitFcn.Draw("same")
canvas.Update()

#gIV.ComputeRange(xmin,ymin,xmax,ymax)
#xmin=
#h=TH2F("h",gIV.GetTitle(),10,1,gIV.GetXaxis().GetXmax(),10,0,ymin*10);
#gStyle.SetOptStat(0)
#h.Draw()
#gIV.Draw("PL")



print "=== Quench Resistance Analysis ==="
QR=fitFcn.GetParameter(0)
printf("Estimated total quence resistance: %6.0f\n",QR)
printf("Estimated quence resistance / SPAD: %6.2e\n",QR*nSPADs)
print "===================="

#os.system('sleep 2')
if not args.auto:
    print 'Hit return to exit'
    sys.stdout.flush() 
    raw_input('')


#png=args.f[0].replace(".csv",".png")

#canvas.Print(png)



