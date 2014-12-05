#!/usr/bin/python
#######################################################
# plot overlay of reverse bias I-V data
#######################################################

import getopt,commands,sys,glob,os
from array import array
import argparse

rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)
    
VMIN=10  # minimum voltage to read

#######################
# main
#######################

if __name__ == '__main__': 
    if len(sys.argv)<2: 
        print "No file names given"
        sys.exit()

    parser = argparse.ArgumentParser(description='I-V Overlay Plotter') 
    parser.add_argument('files', nargs='*')
    parser.add_argument('-t', '--title', type=str, 
                        default='I-VOverlay',
                        help="Title for graph")
    parser.add_argument('-l', '--labels', type=str, nargs='*',
                        default=None,
                        help="Labels for graph")
    parser.add_argument('-d','--dir', type=str, nargs='?', default=None,
                        help="Set search directory for I-V data files")
    args = parser.parse_args()

    # avoid ANNOYING conflict in argument parsing
    
    from ROOT import Double, gStyle
    from ROOT import TCanvas, TGraph, TMultiGraph, TH1F, TPaveText
    from ivtools import readVIfile
    from ivAnalyze import ivAnalyze

    mg=TMultiGraph()
    n=0

    vm=100; vM=-100; im=100; iM=-100  # display ranges
        
    if not args.dir==None:
        for i in range(len(args.files)):
            args.files[i]=args.dir+"/"+args.files[i]

    for file in args.files:
        V=array("d")
        I=array("d")
        readVIfile(file,V,I,VMIN)
        tg=TGraph(len(V),V,I)
        if min(V)<vm: vm=min(V)
        if max(V)>vM: vM=max(V)
        if min(I)<im: im=min(I)
        if max(I)>iM: iM=max(I)
        color=n+2
        tg.SetLineColor(color)
        tg.SetLineWidth(2)
        mg.Add(tg)
        n=n+1

    canvas = TCanvas("ivdata","I-V Overlay",800,800)
    canvas.SetLogy()

    gStyle.SetOptStat(0)
    h=TH1F("ivoverlay",args.title+";V;I [Amps]",2,vm,vM)
    h.SetMinimum(im*0.9)
    h.SetMaximum(iM*1.1)
    h.Draw()
    h.GetYaxis().SetTitleOffset(1.4)
    mg.Draw('PL')

    n=0
    lab=[]
    if vM>0: xlabmin=0.15 # positive voltages
    else: xlabmin=0.5
 
    for file in args.files:
        name=os.path.basename(file)
        lab.append( 
            TPaveText(xlabmin,0.88-0.05*n,xlabmin+0.45,0.93-0.05*n,"NDC") )
        color=n+2
        if args.labels==None: lab[n].AddText(name)
        else: lab[n].AddText(args.labels[n])
        lab[n].SetTextColor(color)
        lab[n].Draw()
        n=n+1


    canvas.Update()

    print 'Hit return to exit'
    sys.stdout.flush()
    raw_input('')

