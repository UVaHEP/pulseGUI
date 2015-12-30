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
    darkFiles.extend( glob.glob(dir+'/*iLED0-*csv') )
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
        ana.Analyze()
        results[df]=[lf,ana.vPeakIp]

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

    # ouch!

    for df in sorted(results.iterkeys()):
        matchto=os.path.basename(df).find("_iLED")
        dev=os.path.basename(df)[0:matchto]
        lf=results[df][0]
        vPeak=results[df][1]
        print ("%s %4.2f") % (dev,vPeak)

        #if lf==None: 
        #    lf="None"
        #else:
        #    lf=os.path.basename(lf)
        #print (":: %25s %25s %4.2f %4.2f (%4.2f) %4.2f %5.2f" % (df, lf, vPeak, vKnee, abs(vPeak-vKnee), vRmax, rMax) )
