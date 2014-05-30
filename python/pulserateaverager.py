import sys, os, commands, glob,math
import optparse

ROOTLIBS=commands.getoutput("root-config --libdir")
sys.path.append(ROOTLIBS)

from ROOT import gSystem, gROOT
gSystem.Load("pulseGUI.so")
gSystem.Load("pythonHelper_C.so")
from ROOT import pulseAnalysis, TArrayF, TFile, gatherArray, joinArrays, TString, TH1D, TH1F
from array import array 


parser = optparse.OptionParser() 

parser.add_option('-a', '--avg', dest='avg', action='store_true')

(options, args) = parser.parse_args()

if options.avg: 

    gROOT.SetBatch()

    for arg in args: 
        if os.path.isdir(arg): 
            processList = []
            resultsTable = {}
            for root, dirs, files in os.walk(arg, False):
                for dir in dirs: 
                    if dir.find('Device') != -1: 
                        processList.append(os.path.join(root, dir))

            p = pulseAnalysis()                    

            for dir in processList: 
                rootFiles = glob.glob(dir+'/*.root')
                rootFiles.sort()
                avg_rate = 0.0
                

                try:
                    for file in rootFiles:
                        p.LoadSpectrum(file)
                        p.SetThreshold(60.0)
                        p.FindPeaks()
                        #                    p.Analyze()
                        avg_rate += p.GetPulseRate()
                        print "File:"+file+" pulse rate:" + str (p.GetPulseRate())
                except Exception as e:
                    print e
                    p = pulseAnalysis()
                    continue

                avg_rate /= len(rootFiles)
                #avg_rate = math.sqrt (avg_rate)
                print "Average Pulse Rate:" +str(avg_rate)
                resultsTable[dir]=avg_rate
                print resultsTable

            keys = resultsTable.keys()
            keys.sort()
            for key in keys:
                splitFile = os.path.basename(key).split('-')
                print '-'.join([splitFile[1], splitFile[3]]) + ' ' + splitFile[4] + ' ' + splitFile[5] + ':' + str(resultsTable[key]) + ' MHz'
#                print ':'.join([key, float(resultsTable[key])])
                
