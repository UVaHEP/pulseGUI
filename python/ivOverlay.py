#!/usr/bin/python
#######################################################
# plot overlay of reverse bias I-V data
#######################################################

import getopt,commands,sys,glob,os
from array import array
import argparse

rootlibs=commands.getoutput("root-config --libdir")
sys.path.append(rootlibs)

# keep ROOT TApplication from grabbing -h flag
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
from ROOT import *

from ivtools import readVIfile
from ivAnalyze import ivAnalyze



VMIN=0  # minimum voltage to read

#######################
# main
#######################

if __name__ == '__main__': 
    if len(sys.argv)<2: 
        print "No file names given"
        print "Usage ivOverlay.py IVfiles [-t title] [-l labels] [-d search dir] -0 (no labels) -a (Do require minPoints)"
        sys.exit()

    parser = argparse.ArgumentParser(description='I-V Overlay Plotter') 
    parser.add_argument('files', nargs='*', help="Give file paths or a [filename].list to \
    specify files.  Use of list overrides labels on command line, use filepath & label in list.")
    parser.add_argument('-t', '--title', type=str, 
                        default='I-VOverlay',
                        help="Title for graph")
    parser.add_argument('-l', '--labels', type=str, nargs='*',
                        default=None,
                        help="Labels for graph")
    parser.add_argument('-d','--dir', type=str, nargs='?', default=None,
                        help="Set search directory for I-V data files")
    parser.add_argument('-o','--output', type=str, nargs='?', default=None,
                        help="Output file for image")
    parser.add_argument("-b", "--batch", help="run in batch mode",
                    action="store_true")
    parser.add_argument("-P", "--pub", help="use publication TStyle",
                    action="store_true")
    parser.add_argument('-m', '--minX', type=float, default=-1,
                        help="Minimum for xrange in plot. Enter abs(minX)")
    parser.add_argument('-M', '--maxX', type=float, default=-1,
                        help="Maximum for xrange in plot. Enter abs(maxX)")
    parser.add_argument('-B', '--beginX', type=float, default=None,
                        help="Start x-axis range here")
    parser.add_argument('-y', '--minY', type=float, default=None,
                        help="Minimum for yrange in plot. Enter abs(minY)")
    parser.add_argument('-Y', '--maxY', type=float, default=None,
                        help="Maximum for yrange in plot. Enter abs(maxY)")
    parser.add_argument("-0", "--nolabels", default=None,
                        help="Do not display labels",
                        action="store_true")
    parser.add_argument("-a", "--plotAll", default=None,
                        help="Do not require min #Points",
                        action="store_true")
    parser.add_argument("-n", "--negPolarity", default=None,
                        help="For plots in negative polarity",
                        action="store_true")
    parser.add_argument("-L", "--linear", default=None,
                        help="Use linear y-axis",
                        action="store_true")
    parser.add_argument('-k','--key', type=str, default="",
                        help="Choose location for key (TL,TR,BL,BR)")
    parser.add_argument('-E','--Vbr', type=float, nargs='*', default=None,
                        help="Plot current as function of Vex based on given Vbr.\n"
                        "Give list of Vbr as positive values.")
    args = parser.parse_args()
    addLabels=(args.nolabels==None)
    plotAll=(not args.plotAll==None)
    if args.pub: 
        gROOT.SetStyle("Pub")
	gStyle.SetPadLeftMargin(0.17);    

    mg=TMultiGraph()
    n=0

    vm=100; vM=-100; im=100; iM=-100  # display ranges
    minPoints=30 # skip plots with less than minPoints
    
    if not args.dir==None:
        for i in range(len(args.files)):
            args.files[i]=args.dir+"/"+args.files[i]

    if ".list" in args.files[0]:
        flist=[]
        args.labels=[]
        print "reading filelist",args.files[0]
        with open(args.files[0]) as f:
            filedata = f.readlines()
        filedata = [x.strip() for x in filedata] 
        for f in filedata:
            if f.startswith("#") or f=="": continue
            if '&' in f:
                f=f.split('&')
                flist.append(f[0].strip())
                args.labels.append(f[1].strip())
    else: flist=args.files

    if not args.Vbr==None:
        if not len(args.Vbr)==len(flist):
            print "Vbr entries much math number of input files",args.Vbr
            sys.exit()

    nfile=0
    for file in flist:
        if not file.endswith(".csv"): continue
        V=array("d")
        I=array("d")
        readVIfile(file,V,I,VMIN)
        if V[-1]>0: sign=1
        else: sign=-1
        if args.Vbr:
            for i in range(len(V)): V[i]=V[i]-sign*args.Vbr[nfile]
        nfile=nfile+1
        if not plotAll and len(V)<minPoints:
            print "Too few points read from file",file,"skipping..."
            print "Use option -a to force plotting"
            continue
        if args.negPolarity and V[-1]>0:
            for i in range(len(V)): V[i]=-1*V[i]
        tg=TGraph(len(V),V,I)
        if min(V)<vm: vm=min(V)
        if max(V)>vM: vM=max(V)
        if min(I)<im: im=min(I)
        if max(I)>iM: iM=max(I)
        if args.minX>0:
            if sign<0: vM=-1.0*args.minX
            else: vm=args.minX
        if args.maxX>0: vM=args.maxX

        color=n+2
        if color==5 : color=809 # get rid of yellow
        if color==3 : color=418 # replace light green with dark green
        if color==10: color=49 #replace white with mauve, will cause probs is >48 curves
        tg.SetLineColor(color)
        tg.SetLineWidth(2)
        mg.Add(tg)
        n=n+1

    canvas = TCanvas("ivdata","I-V Overlay",800,800)
    if args.linear==None: canvas.SetLogy()

    gStyle.SetOptStat(0) #;vM=-50
    if args.beginX: vm=args.beginX
    h=TH1F("ivoverlay",args.title+";V;I [Amps]",2,vm,vM)
    if args.Vbr: h.SetTitle(args.title+";Vex;I [Amps]")
    h.SetMinimum(im*0.9)
    h.SetMaximum(iM*1.1)
    if args.minY: h.SetMinimum(args.minY)
    if args.maxY: h.SetMaximum(args.maxY)
    h.Draw()
    h.GetYaxis().SetTitleOffset(1.4)
    mg.Draw('PL')

    # choose location of the key
    n=0
    lab=[]
    if vM>0 or "L" in args.key: xlabmin=0.15 # positive voltages
    if vM<0 or "R" in args.key: xlabmin=0.5
    if "B" in args.key: ylabmax=0.6
    else: ylabmax=0.93
    
    
    if addLabels:
        for file in flist:
            name=os.path.basename(file)
            lab.append( 
                TPaveText(xlabmin,ylabmax-0.05*(n+1),xlabmin+0.45,ylabmax-0.05*n,"NDC") )
            color=n+2
            if color==5 : color=809 # get rid of yellow
            if color==3 : color=418 # replace light green with dark green
            if color==10: color=49 # replace white with mauve. Probs if >48 curves
            if not args.labels==None and len(args.labels)>n:
                lab[n].AddText(args.labels[n])
            else: lab[n].AddText(name)
            lab[n].SetTextColor(color)
            lab[n].Draw()
            n=n+1


    canvas.Update()
    if args.output: canvas.Print(args.output)
    if args.batch: exit()
    print 'Hit return to exit'
    sys.stdout.flush()
    raw_input('')

