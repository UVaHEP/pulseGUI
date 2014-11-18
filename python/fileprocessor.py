#!/usr/bin/python
# batch convert MATLAB files in directory

import sys, os, commands, glob
import optparse

ROOTLIBS=commands.getoutput("root-config --libdir")
sys.path.append(ROOTLIBS)

from ROOT import gSystem
gSystem.Load("../build/pulseGUI.so")
gSystem.Load("../build/pythonHelper_C.so")
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


def joinRootFiles(files, output): 
    outputFile = TFile(output, "RECREATE")
    print "Joining Files"
    print files

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
        f.Close()

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
    f.Close()



parser = optparse.OptionParser() 
parser.add_option('-c', '--convert', dest='convert', action="store_true", help='convert all found mat files in the given directory and its subdirectories')
parser.add_option('-j', '--join', dest='join', action="store_true", help='Join files in specified directory and all subdirectories')

(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.print_help()
 
    sys.exit(1)


if options.convert: 

    p=pulseAnalysis()
    for arg in args:
        print arg
        if os.path.isdir(arg):  
            for root, dirs, files in os.walk(arg, False): 
                for dir in dirs:
                    print dir
                    if dir != 'mat': #Avoid already parsed files
                        matfiles = glob.glob(os.path.join(root,dir)+'/*.mat')
                        for file in matfiles:
                            pulseAnalysis(file)
                for file in files:
                    if file.find('.mat') != -1:
                        print 'converting file ' + os.path.join(root,file)
                        pulseAnalysis(os.path.join(root,file))
                        
        if os.path.isfile(arg):
            pulseAnalysis(arg)

if options.join: 
    fileType = '.root'
    combineLimit = 200
    for arg in args: 
        if os.path.isdir(arg):
            for root,dirs,files in os.walk(arg, False): 
                if len(files) > combineLimit: #Probably a large # of small waveforms
                    fileList = []
                    for file in files: 
                        if file.find(fileType) != -1: 
                            fileList.append(file)
                    fileList.sort()

                for dir in dirs: 
                    rfiles = glob.glob(os.path.join(root,dir)+'/*'+fileType)
		    print rfiles
                    rfiles.sort()
                    ofilename = os.path.basename(dir)+'.root'
                    print ofilename
                    joinRootFiles(rfiles, ofilename)
                        
                        

