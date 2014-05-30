#!/usr/bin/python
# batch convert MATLAB files in directory

import sys, os, commands, glob
import optparse

ROOTLIBS=commands.getoutput("root-config --libdir")
sys.path.append(ROOTLIBS)

from ROOT import gSystem
gSystem.Load("pulseGUI.so")
gSystem.Load("pythonHelper_C.so")
from ROOT import pulseAnalysis, TArrayF, TFile, gatherArray, joinArrays, TString, TH1D, TH1F
from array import array 

def findMinandMax(volts): 

    max = 0
    min = 0
    for i in range(0, volts.GetSize()): 
        cV = volts.At(i)
        if cV < min: 
            min = cV
        if cV > max:
            max = cV

    return (min, max)


def joinRootFiles(path, output): 
    outputFile = TFile(output, "RECREATE")
    print "Joining Files"
    files = glob.glob(path+"*.root")
    files.sort()
    totalArrSize = 0
    if (len(files) == 0): 
        return
    for file in files:
        print file 
        f =  TFile(file)

        key = f.FindKey("volts")
        arr = gatherArray(f, TString("volts"))
        totalArrSize += arr.GetSize()
        print totalArrSize
        f.Close() 


    joinedArr = TArrayF(totalArrSize)

    pos = 0
    minmaxarr = TArrayF(2)
    minmaxarr.AddAt(0.0, 0)
    minmaxarr.AddAt(0.0, 1)

    for file in files:
        print file 
        f =  TFile(file)
        source = gatherArray(f, TString("volts"))
        pos = joinArrays(joinedArr, source, pos, minmaxarr)
        print pos
        if pos > totalArrSize: 
            print "Uh oh...array too big quitting!"
            exit()
    

    f = TFile(files[0])

    outputFile.cd()

    T0 = TH1D(f.Get("T0"))
    dT = TH1D(f.Get("dT"))
    sign = TH1F(f.Get("sign"))
    dV = TH1F(f.Get("dV"))
    
    
    min = minmaxarr.At(0)
    max = minmaxarr.At(1)
    maxTH1 = TH1F("vMax", "vMax", 1,-1,1)
    maxTH1.Fill(0.0, max)
    minTH1 = TH1F("vMin", "vMin", 1,-1,1)
    minTH1.Fill(0.0, min)

    outputFile.WriteObject(joinedArr, "volts")
    outputFile.Write()
    outputFile.Close()
    


#    for i in range(0,62500): 
#        print joinedArr[i]



if len(sys.argv) < 2:
    print "Usage:",sys.argv[0],"directory for file names"
    sys.exit(1)

    
parser = optparse.OptionParser() 
parser.add_option('-j', '--join', dest='join', action="store_true")
parser.add_option('-o', '--outfile', dest='out', default='test.root')
parser.add_option('-p', '--parse', dest='parse', action='store_true')

(options, args) = parser.parse_args()


if options.join: 
    outfile = 'test.root'
    if options.out: 
        outfile = options.out
    joinRootFiles(args[0], outfile)
        
    exit()

if options.parse: 


    p=pulseAnalysis()
    for arg in args:
        if os.path.isdir(arg):  # note search below does not include subdirs now
            for root, dirs, files in os.walk(arg, False): 
                for dir in dirs:
                    if dir != 'mat':
                        matfiles = glob.glob(os.path.join(root,dir)+'/*.mat')
                        for file in matfiles:
                            pulseAnalysis(file)
        if os.path.isfile(arg):
            pulseAnalysis(arg)









