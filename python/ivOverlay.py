#!/usr/bin/python
#######################################################
# plot overlay of reverse bias I-V data
#######################################################


import getopt,commands,sys,glob,os
from array import array
from ivAnalyze import ivAnalyze


rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

from ROOT import Double, gStyle, kRed, kBlue, kGreen, kTeal
from ROOT import TString, TCanvas, TGraph, TMultiGraph, TH2F, TPaveText
from ivtools import *

VMIN=10  # minimum voltage to read

#######################
# main
#######################

if __name__ == '__main__': 
    if len(sys.argv)<2: 
        print "No file names given"
        sys.exit()
    files=sys.argv[1:]

    Vs=[]
    Is=[]
    gr=[]
    mg=TMultiGraph()
    n=0
    for file in files:
        V=array("d")
        I=array("d")
        readVIfile(file,V,I,VMIN)
        tg=TGraph(len(V),V,I)
        color=n+2
        tg.SetLineColor(color)
        tg.SetLineWidth(2)
        mg.Add(tg)
        n=n+1

    canvas = TCanvas("ivdata","I-V Data",800,800)
    canvas.SetLogy()

    mg.Draw('APL')

    n=0
    lab=[]
    for file in files:
        name=os.path.basename(file)
        print name
        lab.append( TPaveText(0.5,0.9-0.05*n,0.95,0.95-0.05*n,"NDC") )
        color=n+2
        lab[n].AddText(name)
        lab[n].SetTextColor(color)
        lab[n].Draw()
        n=n+1


    canvas.Update()

    print 'Hit return to exit'
    sys.stdout.flush()
    raw_input('')


