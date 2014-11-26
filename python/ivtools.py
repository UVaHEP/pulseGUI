import os, re, sys
from array import array

########################
def printf(format, *args):
# C-like printf
########################
    sys.stdout.write(format % args)

########################
def calcDelta(x,xbar,dx):
# calculate bin centers and width for derivative calculations
########################
    for i in range(len(x)-1):
        xbar.append((x[i]+x[i+1])/2)
        dx.append(x[i+1]-x[i])


########################
def calc_dVdI(V,I,dIdV,Vbar):
# simple derivative calculation dI/dV
########################
    assert len(I)==len(V), "calc_dIdV: array sizes not equal"
    dV=array("d")
    dI=array("d")
    Ibar=array("d")
    calcDelta(I,Ibar,dI)
    calcDelta(V,Vbar,dV)
    for i in range(len(Ibar)):
        if abs(Vbar[i])<0.1: continue
        print Vbar[i],dV[i],dI[i]
        R=abs(dV[i]/dI[i])
        dIdV.append(R)

########################
def calc_dLogIdV(V,I,dLogIdV,Vbar):
# simple derivative calculation for peak Vbr estimation
########################
    dV=array("d")
    dI=array("d")
    Ibar=array("d")
    calcDelta(I,Ibar,dI)
    calcDelta(V,Vbar,dV)
    for i in range(len(Ibar)):
        dLogIdV.append(1/Ibar[i]*dI[i]/abs(dV[i]))


########################
def readVIfile(file, V, I):
# read CSV file and return V,I data in given arrays 
########################
    VMIN=0  # minimum voltage to scan for Vbr

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
        if len(V)>0 and v==V[len(V)-1] :
            I[len(V)-1]=i # if doing multiple readings, take the last one
            continue
        V.append(v)
        I.append(i)


#######################
# find idx for maximum entry in array
def getMaxIdx(a):
#######################
    amax=-1e20
    idx=-1
    for j in range(len(a)):
        if a[j]>amax:
            amax=a[j]
            idx=j
    return idx

#######################
def getMaxXY(x,y):
# return (x,y) for y_max
#######################
    idx=getMaxIdx(y)
    return x[idx],y[idx]


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
	



        
