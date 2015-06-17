import numpy, os, re, sys
from array import array
from ROOT import TGraph, TGraphSmooth, Double

########################
# C-like printf
def printf(format, *args):
########################
    sys.stdout.write(format % args)


########################
# return x,y for maximum of TGraph
########################
def GraphMax(tg):
    xmax=0
    ymax=-1e50
    npoints=tg.GetN()
    x=Double(); y=Double()
    for i in range(npoints):
        tg.GetPoint(i,x,y)
        if y>ymax:
            ymax=float(y)
            xmax=float(x)
    return xmax,ymax
        


########################
# read CSV file and return V,I data in given arrays 
def readVIfile(file, V, I, Vmin=0):
########################
    f = open(file, 'r')
    # identify file type from header
    isAgilent = "Repeat,VAR2" in f.readline()
    if isAgilent: print "Reading I-V curve from Agilent sourcemeter"
    else: print "Reading I-V curve from Keithley sourcemeter"

    for line in f.readlines():
        line=line.strip().split(',')
        if isAgilent: v,i=line[3:5]
        else: v,i=line[0,2]           # Keithley data format
        v=float(v)
        i=abs(float(i))
        if abs(v)<Vmin or i==0: continue
        if len(V)>0 and v==V[len(V)-1] :
            I[len(V)-1]=i # if doing multiple readings, take the last one
            continue      # TO DO: add averaging option
        V.append(v)
        I.append(i)


#######################
def getField(sname, tgt):
# extract "tgt" field from a file name
# e.g. for temperature field getField(fn,"C") 
#######################
	s=os.path.basename(sname)
	fields=re.split("[-_]|(\.root)|(\.csv)",s)
#	fields=re.split("[_](.csv)",s)
        print fields
	for f in fields:
            if f==None : continue
            ff=re.split(tgt,f)
            if len(ff) > 1: return ff[0]
# use isinstance(val,bool) to check for error
	return False
	
def CalcCelcius(resistance):
    T,R=numpy.genfromtxt(os.getenv("PULSGUIDIR")+"/dat/KT103J2.dat.bz2",unpack=True)
    return numpy.interp(resistance,R,T)


#######################
def TGraphSmoother(graphin,type="super",strength=0):
# return a smoothed version of input TGraph
# strength parameter depends on smoother choosen
# SmoothSuper: strength=bass
# SmoothKernel: strength=bandwidth
#######################
    type=type.lower()
    gs = TGraphSmooth("normal")
    if type=="super":
        graphSm=gs.SmoothSuper(graphin,"",strength,0)
    elif type=="kernel":
        graphSm=gs.SmoothKernel(graphin,"",strength)
    else:
        print "[TGraphSmooth] Smoother",type,"not recognized, returning None"
        return None
    return TGraph(graphSm)

#######################
def TGraphInvert(graphin):
# return a inverted version of input TGraph
#######################
    ginv=TGraph(graphin.GetN())
    x=Double(); y=Double()
    for i in range(graphin.GetN()):
        graphin.GetPoint(i, x, y)
        if y==0:
            print "[TGraphInvert] Cannot invert graph with 0 value, returning None"
            return None
        ginv.SetPoint(i, x, 1/y)
    return TGraph(ginv)

    
#######################
def TGraphDivide(graph1,graph2):
# return ratio of two TGraphs = graph1/graph2
# ASSUMES x values are the same!
#######################
    gr=TGraph(graph1)
    x=Double(); y1=Double(); y2=Double()
    for i in range(graph1.GetN()):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1/y2)
    return TGraph(gr)


#######################
def TGraphDiff(graph1,graph2):
# return difference of two TGraphs = graph1-graph2
# ASSUMES x values are the same!
#######################
    gr=TGraph(graph1)
    x=Double(); y1=Double(); y2=Double()
    for i in range(graph1.GetN()):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1-y2)
    return TGraph(gr)

#######################
def TGraphMultiply(graph1,graph2):
# return new TGraph with points ynew=y1*y2
# ASSUMES x values are the same!
#######################
    gr=TGraph(graph1)
    x=Double(); y1=Double(); y2=Double()
    for i in range(graph1.GetN()):
        graph1.GetPoint(i, x, y1)
        graph2.GetPoint(i, x, y2)
        gr.SetPoint(i, x, y1*y2)
    return TGraph(gr)


########################
# simple derivative calculation of dLogY/dX for peak Vbr estimation
def IV2dLogIdV(tg):
########################
    npoints=tg.GetN()-1
    tgnew=TGraph(npoints)
    v1=Double(); v2=Double(); i1=Double(); i2=Double()
    for i in range(npoints):
        tg.GetPoint(i, v1, i1)
        tg.GetPoint(i+1, v2, i2)
        dI=i2-i1
        Ibar=(i2+i1)/2
        dV=v2-v1
        Vbar=(v2+v1)/2
        dLogIdV=1/Ibar*dI/abs(dV)
        tgnew.SetPoint(i,Vbar,dLogIdV)
    return TGraph(tgnew)

#######################
def TGraphDerivative(tg):
# simple calculation returns derivative of TGraph
#######################
    npoints=tg.GetN()-1
    tgnew=TGraph(npoints)
    x1=Double(); x2=Double(); y1=Double(); y2=Double()
    for i in range(npoints):
        tg.GetPoint(i, x1, y1)
        tg.GetPoint(i+1, x2, y2)
        dX=x2-x1
        Xbar=(x2+x1)/2
        dY=y2-y1
        Ybar=(y2+y1)/2
        tgnew.SetPoint(i,Xbar,dY/dX)
    return TGraph(tgnew)
    
#######################
def TGraphScale(tg,scale):
# simple calculation returns derivative of TGraph
#######################
    npoints=tg.GetN()
    tgnew=TGraph(npoints)
    x=Double(); y=Double();
    for i in range(npoints):
        tg.GetPoint(i, x, y)
        tgnew.SetPoint(i,x,float(y)*scale)
    return TGraph(tgnew)
    
