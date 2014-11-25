import os, re, tempfile
from array import array
from ROOT import TF1, TGraph

########################
def readVIcsv(file, V, I, vbar, dIdVi):
# read CSV file and return V,I data in given arrays 
########################
    VMIN=20  # minimum voltage to scan for Vbr
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
        dIdVi.append(delta)
                
#        print v,i
    os.system("rm -f "+tmpINP)

#######################
def getMaxIdx(a):
# find idx for maximum dIdVi
#######################
    amax=-1e20
    idx=-1
    for j in range(len(a)):
        if a[j]>amax:
            amax=a[j]
            idx=j
    return idx



def fitVbr(vbar,dIdVi,fitF):
    ip=getMaxIdx(dIdVi)

    # fit Vbr region w/ a piecewise pair of lines
    # take Vbr to be the cross over point from the fit

    # rising edge
    dy=dIdVi[ip]-dIdVi[ip-2]
    dx=vbar[ip]-vbar[ip-2]
    m1=dy/dx
    b1=dIdVi[ip]-m1*vbar[ip]
    # guess for transition region
    xb=vbar[ip-2]
    # before quick rise, start from Vbr estimate minus ~3V
    # if from Vbr = -3V to -2V
    i1=ip-int(3/dx)
    i2=ip-int(1/dx)
    dy=dIdVi[i2]-dIdVi[i1]
    dx=vbar[i2]-vbar[i1]
    m2=dy/dx
    b2=dIdVi[i2]-m2*vbar[i2]

    print vbar[i1],vbar[ip]
    fcn=TF1("fcn","x>[2]?[0]+[1]*x:[3]+[4]*x",vbar[i1],vbar[ip])
    fcn.SetParameters(b1,m1,xb,b2,m2);

    tg=TGraph(len(vbar),vbar,dIdVi)
    tg.Fit("fcn","R")

    fcn.Copy(fitF)
    return fcn.GetParameter(2)




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
	
