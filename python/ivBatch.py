#!/usr/bin/env python

# batch process iv data files in given directory

import getopt,commands,sys,glob,os
from ivAnalyze import ivAnalyze

def usage():
    print
    print "Usage: python [OPTION] ivBatch DIR"
    print "      -r             : Recursively process all in DIR"
    print "      -v             : verbose output"
    print 
    sys.exit()

# global keep of all results
results={}

def ProcessDir(dir):
    print "Processing data in:",dir
    darkFiles=[]
    darkFiles.extend( glob.glob(dir+'/*iLED0*csv') )
    lightFiles=[]
    lightFiles.extend( glob.glob(dir+'/*iLED[1-9]*csv') )

    ana=ivAnalyze()

    for df in darkFiles:
        lf=None
        for f in lightFiles:
            ln=os.path.basename(f)
            dn=os.path.basename(df)
            matchto=dn.find("_iLED")
            if ln[0:matchto]==dn[0:matchto]:
                lf=f
                break
        print df,lf
        ana.SetData(df,lf)
        results[df]=ana.Analyze()        
    return

if __name__ == '__main__': 
    try:
        opts, args = getopt.getopt(sys.argv[1:], "rv")
    except getopt.GetoptError as err: usage()

    dirList=[]
    recurse=False
    verbose=False

    for o, a in opts:
         if o == "-r": recurse=True
         elif o == "-v": verbose=true

    if len(args)==0: usage()
    dir=args[0]


    if recurse:
        cmd="find "+dir+" -type d"
        print cmd
        dirList.extend(commands.getoutput(cmd).split())
    else: dirList.append(dir)
    
    for d in dirList: 
        ProcessDir(d)

    # need to print: Vbr, "Vop", LDR(@Vop), Id(Vbr/10,4,2,1), Delta(Vop,VBr)
    #                (I_light-I_dark)/I_dark (@Vop), Gain(@Vop)
    # look at slope of dI/dV vs V, is it ~flat up to Vbr for good devs?
    print ""
    print ("%15s %8s %8s %8s %8s %8s %8s %8s %8s %8s") % ("Dev/chan","Vbr","Vop","Vex","LDRmax","FWHM","DC_Gain",
                                              "I90%", "I60%", "I30%")
    for df in sorted(results.iterkeys()):
        matchto=os.path.basename(df).find("_iLED")
        dev=os.path.basename(df)[0:matchto]
        dat=results[df]
        print ("%15s %8.2f %8.2f %8.2f %8.2f %8.2f %8.1e %8.2e %8.2e %8.2e") %\
        (dev,dat["vPeakIp"],dat["LDRmax"][0],
         dat["LDRmax"][0]-dat["vPeakIp"],
         dat["LDRmax"][1],dat["LDRmax"][2],dat["M(Vop)"],
         dat["I90"],dat["I60"],dat["I30"])

