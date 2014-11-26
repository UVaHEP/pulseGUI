import os, re, tempfile
from array import array
from ROOT import TF1, TGraph

########################
def calcDelta(x,xbar,dx):
# calculate bin centers and deltas for derivative calculations
########################
    for i in range(len(x)-1):
        xbar.append((x[i]+x[i+1])/2)
        dx.append(x[i+1]-x[i])

########################
def calc_dLogIdV(V,I,dLogIdV,Vbar):
# simple derivative calculation for peak Vbr estimation
########################
    assert len(I)==len(V), "calc_dLogIdV: array sizes not equal"
    dV=array("d")
    dI=array("d")
    Ibar=array("d")
    calcDelta(I,Ibar,dI)
    calcDelta(V,Vbar,dV)

    for i in range(len(dI)):
        dLogIdV.append(1/Ibar[i]*dI[i]/abs(dV[i]))
        

########################
def readVIfile(file, V, I):
# read CSV file and return V,I data in given arrays 
########################
    VMIN=0  # minimum voltage to scan for Vbr
    # identify file type from header
    isAgilent = os.system("grep Repeat,VAR2 "+file)==0
    tmpINP=tempfile.mktemp()
    # make a 2-column tmp file w/ V, I data
    if isAgilent:
        print "Reading I-V curve from Agilent sourcemeter"
        os.system("cat "+file+" | awk -F',' '{print $4,$5}' | grep -iv Volt >"+tmpINP)
    else:
        print "Reading I-V curve from Keithley sourcemeter"
        os.system("cat "+file+" | awk -F',' '{print $1,$2}' | grep -iv Volt  >"+tmpINP)

    for line in open(tmpINP):
        line=line.strip().split()
        v=(float(line[0]))
        i=abs(float(line[1]))
        if len(V)>0 and v==V[len(V)-1] : 
            I[len(V)-1]=i # if doing multiple readings, take the last one
            continue
        V.append(v)
        I.append(i)
        os.system("rm -f "+tmpINP)


########################
def readVIcsv(file, V, I, vbar, dLogIdV):
# read CSV file and return V,I data in given arrays 
########################
    VMIN=0  # minimum voltage to scan for Vbr
    # identify file type from header
    isAgilent = os.system("grep Repeat,VAR2 "+file)==0
    tmpINP=tempfile.mktemp()
    # make a 2-column tmp file w/ V, I data
    if isAgilent:
        print "Reading I-V curve from Agilent sourcemeter"
        os.system("cat "+file+" | awk -F',' '{print $4,$5}' | grep -iv Volt >"+tmpINP)
    else:
        print "Reading I-V curve from Keithley sourcemeter"
        os.system("cat "+file+" | awk -F',' '{print $1,$2}' | grep -iv Volt  >"+tmpINP)

    for line in open(tmpINP):
        line=line.strip().split()
        v=(float(line[0]))
        i=abs(float(line[1]))
#        if v<0 : continue
        if len(V)>0 and v==V[len(V)-1] : 
            I[len(V)-1]=i # if doing multiple readings, take the last one
            continue
        V.append(v)
        I.append(i)
        # derived quantities
        j=len(V)
        if abs(v)<VMIN or j<2: continue
        j=j-1
        vbar.append((V[j]+V[j-1])/2)
        dV=abs(V[j]-V[j-1])
        ibar=(I[j]+I[j-1])/2
        dI=I[j]-I[j-1]
        delta=1/ibar*dI/dV
        dLogIdV.append(delta)
                
#        print v,i
    os.system("rm -f "+tmpINP)

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

def getMaxXY(x,y):
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
	



        
