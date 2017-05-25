import numpy, os, re, sys
from array import array
from ROOT import TGraph, TGraphSmooth, Double
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
    x=Double(); y=Double()
    tg.Sort()
    if xmin>xmax:
        temp=xmin
        xmin=xmax
        xmax=xmin
    elif window<1:
        tg.GetPoint(0,x,y)
        xmin=float(x)
        tg.GetPoint(npoints-1,x,y)
        xmax=float(x)
        xRange=xmax-xmin
        xmin=xmin+xRange*(1-window)/2
        xmax=xmin+xRange*window
    #tg.Draw()
    #time.sleep(1)
    xMax=0
    yMax=-1e50
    #print tg.GetName()
    #print 'yMax: {0}'.format(yMax)
    imax=0
    for i in range(npoints):
        tg.GetPoint(i,x,y)
        #print 'x:{0}, ymax: {3}, y:{1}, i:{2}'.format(x, y, i, yMax)
        if x<xmin: continue
        if x>xmax: continue
        if y>yMax:
            yMax=float(y)
            xMax=float(x)
            imax=i
    # find approximate FWHM
    x1=Double(); y1=Double()
    x2=Double(); y2=Double()
    #This handles the case where we start on the maximum point
    if imax == 0: imax = 1
    for i in range(imax-1,-1,-1): #Scan left
        tg.GetPoint(i,x1,y1)
        tg.GetPoint(i+1,x2,y2)
        if y1<=yMax/2: break
    #print 'y2: {0}, y1:{1}, x2: {2}, x1: {3}, imax:{4}'.format(y2, y1, x2, x1,imax)
    mL=(y2-y1)/(x2-x1)
    xL=x1+(yMax/2-y1)/mL
    for i in range(imax,npoints-1): #Scan right
        tg.GetPoint(i,x1,y1)
        tg.GetPoint(i+1,x2,y2)
        if y2<=yMax/2: break
    if (x2-x1)==0:
        print "Cannot esimtate FWHM for graph:",tg.GetName()
        return  xMax,yMax,-1
    mH=(y2-y1)/(x2-x1)
    xH=x1+(yMax/2-y1)/mH
    return xMax,yMax,xH-xL



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
        #print x,y
        if float(x)>x2 and float(x)>xMax and float(y)>=yMax*0.2:
            x2=float(x)
            y2=float(y)
    return x2,y2
    



########################
# read CSV file and return V,I data in given arrays 
def readVIfile(file, V, I, Vmin=0, Vmax=200):
########################
    f = open(file, 'r')
    # identify file type from header
    line=f.readline()
    isAgilent = "Repeat,VAR2" in line
    isFromEric="Voltage_1" in line
    if isAgilent: print "Reading I-V curve from Agilent sourcemeter"
    elif isFromEric: print "Reading from Eric's data"
    else: print "Reading I-V curve from Keithley sourcemeter"

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
        if len(V)>0 and v==V[len(V)-1] :
            I[len(V)-1]=i # if doing multiple readings, take the last one
            continue      # TO DO: add averaging option
        V.append(v)
        I.append(i)
    if len(V)==0: return -1
    return 0

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
        if dV==0: # voltage is repeated, use last value for derivative
            v=Double(); deriv=Double()
            tgnew.GetPoint(i-1,v,deriv)
            tgnew.SetPoint(i,v,deriv)
        else:
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
        if dX==0: # X value is repeated, use last value for derivative
            x=Double(); deriv=Double()
            tgnew.GetPoint(i-1,x,deriv)
            tgnew.SetPoint(i,x,deriv)
        else:
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
class Thermistor():
    def __init__ (self):
        self.graph=TGraph("KT103J2_TvR.dat")
        
    def Temperature(self,R):
        return self.graph.Eval(R)
    
