#!/usr/bin/env python

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
    darkFiles.extend( glob.glob(dir+'/*Dark*csv') )
    darkFiles.extend( glob.glob(dir+'/*-D-*csv') )
    lightFiles=[]
    lightFiles.extend( glob.glob(dir+'/*-L-*csv') )
    lightFiles.extend( glob.glob(dir+'/*Light*csv') )
    lightFiles.extend( glob.glob(dir+'/*urple*csv') )
    lightFiles.extend( glob.glob(dir+'/*reen*csv') )

    ana=ivAnalyze()

    for df in darkFiles:
        lf=None
        for f in lightFiles:
            ln=os.path.basename(f)
            dn=os.path.basename(df)
            if ln[0:3]==dn[0:3]: # not the smartest file matcher, be careful w/ file names!
                lf=f
                break
        print df,lf
        ana.SetData(df,lf)
        results[df]=[lf,ana.Analyze()]
#        print ana.Analyze()
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
    
    for k in results:
        df=k
        lf=results[k][0]
        vPeak=results[k][1][0]
        vKnee=results[k][1][1]
        vRmax=results[k][1][2][0]
        rMax=results[k][1][2][1]
        print k,lf
        df=os.path.basename(k)
        if lf==None: 
            lf="None"
        else:
            lf=os.path.basename(lf)
        print (":: %25s %25s %4.2f %4.2f (%4.2f) %4.2f %5.2f" % (df, lf, vPeak, vKnee, abs(vPeak-vKnee), vRmax, rMax) )
