#!/usr/bin/env python

# batch process iv data files in given directory

# keep ROOT TApplication from grabbing -h flag
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
from ROOT import *

import getopt,commands,sys,glob,os
import argparse
from ivAnalyze import ivAnalyze

# global keep of all results
results={}  # dictionary
resList=[]  # list



def MakeFileLists(pathList):
    darkFiles=[]
    lightFiles=[]
    for path in pathList:
        if os.path.isdir(path):
            print "***",path
            darkFiles.extend( glob.glob(path+'/*iLED0*csv') )
            darkFiles.extend( glob.glob(path+'/*iLED-1*csv') )
            lightFiles.extend( glob.glob(path+'/*iLED[1-9]*csv') )
        if not "csv" in path: continue
        elif "iLED0" in path or "iLED-1" in path: darkFiles.extend(path.split())
        elif "iLED" in path: lightFiles.extend(path.split())
    return darkFiles,lightFiles

def ProcessAll(darkFiles,lightFiles):        
    ana=ivAnalyze()

    for df in darkFiles:
        lf=None
        dn=os.path.basename(df)
        matchto=dn.find("_iLED")
        for f in lightFiles: # skip light files if not matched to dark
            ln=os.path.basename(f)
            if ln[0:matchto]==dn[0:matchto]:
                lf=f
                break
        #if lf==None: continue
        ana.SetData(df,lf)
        data=ana.Analyze()
        if data==None: continue
        results[df]=data
        resList.append(data)
    return

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='I-V Curve Batch Processor')
    parser.add_argument('files', nargs='*')
    parser.add_argument("-r", "--recursive", default=None, help="Search subdirectories",
                    action="store_true")
    parser.add_argument("-v", "--verbose", default=None, help="Search subdirectories",
                    action="store_true")
    parser.add_argument("-s", "--sorted", default=None, help="Sort results according to device/channel",
                    action="store_true")
    args = parser.parse_args()
    verbose=args.verbose
    
    pathList=args.files

    if args.recursive:
        if len(pathList)>1:
            print "Enter only one directory name for recursive processing"
            sys.exit()
        cmd="find "+pathList[0]+"/* -type d"
        pathList.extend(commands.getoutput(cmd).split())

    darkFiles,lightFiles=MakeFileLists(pathList) # create lists of dark/light files

    ProcessAll(darkFiles,lightFiles)

    # need to print: Vbr, "Vop", LDR(@Vop), Id(Vbr/10,4,2,1), Delta(Vop,VBr)
    #                (I_light-I_dark)/I_dark (@Vop), Gain(@Vop)
    # look at slope of dI/dV vs V, is it ~flat up to Vbr for good devs?
    print ""
    print ("#%14s %4s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s") % ("Dev", "chan","Vbr","Vop","Vex","LDRmax","FWHM","DC_Gain",
                                                                          "I90%", "I60%", "I30%","leakCnst","Slope","leak@Vbr")
    if args.sorted:
        for df in sorted(results.iterkeys()):
            matchto=os.path.basename(df).find("_iLED")
            dev=os.path.basename(df)[0:matchto]
            dev=dev.split("_Ch")
            name=dev[0]
            chan=dev[1]
            dat=results[df]
            if dat["vPeakIp"]==0: Vbr=dat["vPeak"]
            else: Vbr=dat["vPeakIp"]
            print ("%15s %4s %8.2f %8.2f %8.2f %8.2f %8.2f %8.1e %8.2e %8.2e %8.2e %8.3e %8.3e %8.2f") %\
                (name,chan,Vbr,dat["LDRmax"][0],
                 dat["LDRmax"][0]-dat["vPeakIp"],
                 dat["LDRmax"][1],dat["LDRmax"][2],dat["M(Vop)"],
                 dat["I90"],dat["I60"],dat["I30"],
                 dat["leakConst"],dat["leakSlope"],dat["leakAtVbr"]*1e9)
    else:
        for dat in resList:
            df=dat["fnIV"]
            matchto=os.path.basename(df).find("_iLED")
            dev=os.path.basename(df)[0:matchto]
            dev=dev.split("_Ch")
            name=dev[0]
            chan=dev[1]
            dat=results[df]
            if dat["vPeakIp"]==0: Vbr=dat["vPeak"]
            else: Vbr=dat["vPeakIp"]
            print ("%15s %4s %8.2f %8.2f %8.2f %8.2f %8.2f %8.1e %8.2e %8.2e %8.2e %8.3e %8.3e %8.2f") %\
                (name,chan,Vbr,dat["LDRmax"][0],
                 dat["LDRmax"][0]-dat["vPeakIp"],
                 dat["LDRmax"][1],dat["LDRmax"][2],dat["M(Vop)"],
                 dat["I90"],dat["I60"],dat["I30"],
                 dat["leakConst"],dat["leakSlope"],dat["leakAtVbr"]*1e9)
            
