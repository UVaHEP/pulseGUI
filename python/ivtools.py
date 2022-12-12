import os, re, sys, bz2
import numpy as np
from array import array
#from ROOT import TGraph, TGraphSmooth, Double
import ROOT as r
import time
########################
# C-like printf
def printf(format, *args):
    ########################
    sys.stdout.write(format % args)


########################
# return x,y for maximum of TGraph and approximate FWHM
# optionally search within range [xmin:xmax]
# the window parameter can be used to ignore points near edges
# eg GraphMax(tg,window=0.8) searches in the center 80% of the graph
########################
def GraphMax(tg,xmin=-1e20,xmax=1e20,window=1):
    npoints=tg.GetN()
    tg.Sort()
    ary_x=np.fromiter(tg.GetX(), dtype=np.float, count=npoints)
    ary_y=np.fromiter(tg.GetY(), dtype=np.float, count=npoints)
    if window<1:  # user range
        xmin=ary_x[0]
        xmax=ary_x[-1]
        xRange=xmax-xmin
        xmin=xmin+xRange*(1-window)/2
        xmax=xmin+xRange*window
        
    xMax=0
    yMax=-1e50
    imax=0
    for i in range(npoints):
        if ary_x[i]<xmin or ary_x[i]>xmax: continue
        if ary_y[i]>yMax:
            yMax=ary_y[i]
            xMax=ary_x[i]
            imax=i

    # find approximate FWHM if graph is peaked in the middle
    base=min(ary_y[0],ary_y[-1])  # assume minimum of graph is at one end
    yHalf=(yMax-base)/2+base

    xR=-1
    yR=-1
    try: 
        for i in range(imax-1,-1,-1): #Scan left
            yL=ary_y[i]
            xL=ary_x[i]
            if yL<=yHalf: break
        for i in range(imax+1,npoints-1): #Scan right
            yR=ary_y[i]
            xR=ary_x[i]
            if yR<=yHalf: break
    except Exception as e:
        print ("Cannot esimtate FWHM for graph:",tg.GetName())

    if xR<0 or yR<0:
        return xMax,yMax,0 
    return xMax,yMax,xR-xL



########################
# hack to find right most peak >=20% of highest peak
########################
def GraphMaxRight(tg,xmin=-1e20,xmax=1e20):
    xMax,yMax=GraphMax(tg,xmin,xmax)
    #print "**********",xMax,yMax
    x2=xMax; y2=yMax
    x=Double(); y=Double()
    npoints=tg.GetN()
    for i in range(npoints):
        tg.GetPoint(i,x,y)
        if float(x)>xmax: continue
        if float(x)<xMax: continue
        if float(x)>x2 and float(x)>xMax and float(y)>=yMax*0.2:
            x2=float(x)
            y2=float(y)
    return x2,y2


def readVIfile2(file,sortV="False"):
    v,i = np.loadtxt(file, delimiter=',', usecols = (0,1), unpack=True)
    #    if sortV:
    #        vi=np.stack(v,i) 
    #        vi = np.abs(vi)
    #        vi = np.sort(vi,0)
    #        v,i=np.hsplit(vi,2)
    #        v=np.flatten(v)
    #        i=np.flatten(i)
    return v,i



#######################
def getField(sname, tgt):
    # extract "tgt" field from a file name
    # e.g. for temperature field getField(fn,"C") 
    #######################
    s=os.path.basename(sname)
    fields=re.split("[-_]|(\.root)|(\.csv)",s)
        #	fields=re.split("[_](.csv)",s)
    print( fields)
    for f in fields:
        if f==None : continue
        ff=re.split(tgt,f)
        if len(ff) > 1: return ff[0]
        # use isinstance(val,bool) to check for error
    return False
    
def CalcCelcius(resistance):
    T,R=np.genfromtxt(os.getenv("PULSGUIDIR")+"/dat/KT103J2.dat.bz2",unpack=True)
    return np.interp(resistance,R,T)


#######################
def TGraphSmoother(graphin,type="super",strength=0):
    # return a smoothed version of input TGraph
    # strength parameter depends on smoother choosen
    # SmoothSuper: strength=bass
    # SmoothKernel: strength=bandwidth
    #######################
    type=type.lower()
    gs = r.TGraphSmooth("normal")
    if type=="super":
        graphSm=gs.SmoothSuper(graphin,"",strength,0)
    elif type=="kernel":
        graphSm=gs.SmoothKernel(graphin,"",strength)
    else:
        print ("[TGraphSmooth] Smoother",type,"not recognized, returning None")
        return None
    return r.TGraph(graphSm)

#######################
def TGraphInvert(graphin):
    # return a inverted version of input TGraph
    #######################
    ginv=r.TGraph(graphin.GetN())
    x=r.Double(); y=r.Double()
    prev = 0.1
    for i in range(graphin.GetN()):
        #print 'prev before get point: {0}'.format(prev)
        graphin.GetPoint(i, x, y)
        #print 'prev after get point :{0}'.format(prev)
        #print 'x:{0}, y:{1}, i:{2}'.format(x, y, i)
        if y==0:
            #print 'Found a 0 Value! Using previous {3}, Check Data, x: {0}, y:{1}, i:{2}'.format(x, y, i, prev),graphin.GetName()
            #print 'You may want to do another run'
            y = Double(prev+0.01*prev)
            #print "[TGraphInvert] Cannot invert graph with 0 value, returning None"
            #return None
        ginv.SetPoint(i, x, 1/y)
        prev = float(y)
    return r.TGraph(ginv)

    
#######################
def TGraphDivide(graph1,graph2):
    # return ratio of two TGraphs = graph1/graph2
    # ASSUMES x values are the same!
    #######################
    gr=r.TGraph(graph1)
    x=r.Double(); y1=r.Double(); y2=r.Double()
    for i in range(graph1.GetN()):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1/y2)
    return r.TGraph(gr)


#######################
def TGraphDiff(graph1,graph2):
    # return difference of two TGraphs = graph1-graph2
    # ASSUMES x values are the same!
    # If graphs are different sizes we truncate the last points
    #######################
    gr=r.TGraph(graph1)
    x=r.Double(); y1=r.Double(); y2=r.Double()
    npoints=graph1.GetN()
    if not graph1.GetN() == graph2.GetN():
        print ("TGraphDiff: Warning - Graphs are different sizes",
        graph1.GetName(), graph2.GetName())
    if graph1.GetN() > graph2.GetN(): npoints=graph2.GetN()
    for i in range(npoints):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1-y2)
    return r.TGraph(gr)

#######################
def TGraphMultiply(graph1,graph2):
    # return new TGraph with points ynew=y1*y2
    # ASSUMES x values are the same!
    #######################
    gr=r.TGraph(graph1)
    x=Double(); y1=Double(); y2=Double()
    for i in range(graph1.GetN()):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1*y2)
    return r.TGraph(gr)


########################
# simple derivative calculation of dLogY/dX for peak Vbr estimation
def IV2dLogIdV(tg):
    ########################
    npoints=tg.GetN()-1
    tgnew=r.TGraph(npoints)
    v1=r.Double(); v2=r.Double(); i1=r.Double(); i2=r.Double()
    for i in range(npoints):
        tg.GetPoint(i, v1, i1)
        tg.GetPoint(i+1, v2, i2)
        dI=i2-i1
        Ibar=(i2+i1)/2
        if Ibar==0: Ibar=1e-11*v1/abs(v1) # in case I is just LSB fluctuations
        dV=v2-v1
        Vbar=(v2+v1)/2
        if dV==0: # voltage is repeated, use last value for derivative
            v=r.Double(); deriv=r.Double()
            tgnew.GetPoint(i-1,v,deriv)
            tgnew.SetPoint(i,v,deriv)
        else:
            dLogIdV=1/Ibar*dI/abs(dV)
            tgnew.SetPoint(i,Vbar,dLogIdV)
    return r.TGraph(tgnew)

#######################
def TGraphDerivative(tg):
    # simple calculation returns derivative of TGraph
    #######################
    npoints=tg.GetN()-1
    tgnew=r.TGraph(npoints)
    x1=r.Double(); x2=r.Double(); y1=r.Double(); y2=r.Double()
    for i in range(npoints):
        tg.GetPoint(i, x1, y1)
        tg.GetPoint(i+1, x2, y2)
        dX=x2-x1
        Xbar=(x2+x1)/2
        dY=y2-y1
        Ybar=(y2+y1)/2
        if dX==0: # X value is repeated, use last value for derivative
            x=Double(); deriv=Double()
            tgnew.GetPoint(i-1,x,deriv)
            tgnew.SetPoint(i,x,deriv)
        else:
            tgnew.SetPoint(i,Xbar,dY/dX)
    return r.TGraph(tgnew)

#######################
def TGraphShift(tg,shiftx,shifty=0):
    # shift point by shiftx, shifty
    #######################
    x=r.Double(); y=r.Double()
    npoints=tg.GetN()
    print ("TGSHIFT",npoints)
    for i in range(npoints):
        tg.GetPoint(i, x, y)
        tg.SetPoint(i,x-shiftx,y-shifty)
        print (tg)
    return tg

#######################
def TGraphScale(tg,scale):
    # scale y values in TGraph
    #######################
    npoints=tg.GetN()
    tgnew=r.TGraph(npoints)
    x=r.Double(); y=r.Double();
    for i in range(npoints):
        tg.GetPoint(i, x, y)
        tgnew.SetPoint(i,x,float(y)*scale)
    return r.TGraph(tgnew)

class Thermistor():
    def __init__ (self):
        datdir = os.environ.get('PULSGUIDIR')+'/dat'
        rtable = datdir+"/KT103J2.dat.bz2"
        f_p=bz2.BZ2File(rtable,"r")
        self.graph=r.TGraph()
        n=0
        while 1:
            line=f_p.readline()
            if not line: break
            if line.startswith('#') : continue
            if not "." in line : continue
            T,R=line.split()
            self.graph.SetPoint(n,float(R),float(T))
            n=n+1
    def Temperature(self,R):
        return self.graph.Eval(R)
    


################################################################
# Depricated
################################################################


########################
# read CSV file and return V,I data in given arrays 
def readVIfile(file, V, I, Vmin=0, Vmax=200):
    ########################
    f = open(file, 'r')
    # identify file type from header
    line=f.readline()
    isAgilent = "Repeat,VAR2" in line
    isFromEric="Voltage_1" in line
    if isAgilent: print ("Reading I-V curve from Agilent sourcemeter")
    #elif isFromEric: print "Reading from Eric's data"
    else: print ("Reading I-V curve from Keithley sourcemeter")

    for line in f.readlines():
        line=line.strip().split(',')
        if isAgilent: v,i=line[3:5]
        elif isFromEric: v,i=line[0:2]
        else: v,i=line[0:2]           # Keithley data format
        v=float(v)
        i=abs(float(i))
        if abs(v)<Vmin or i==0:
            continue
        if abs(v)>Vmax:
            continue
        # if doing multiple readings, take the last one
        # TO DO: add averaging option
        if len(V)>0 and abs(v-V[-1])<1e-5:
            I[-1]=i
            continue
        V.append(v)
        I.append(i)
    if len(V)<20: return -1 # Fail of short files. Warning magic number!
    return 0
