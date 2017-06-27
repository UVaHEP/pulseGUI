import sys
from ROOT import *

def pausePlot(delay=0):
    if delay==0:
        print 'Hit return to exit'
        sys.stdout.flush() 
        raw_input('')
        return
    else: time.sleep(delay)


def setTG(tg,col=-1,mark=-1,line=-1):
    if mark<0: mark=i
    #if mark<20: mark=mark+20
    if line<-1: line=1
    colors=[kRed+1,kBlack,kGreen+2,kBlue+1,kMagenta+1,kYellow-2,kCyan+2,kOrange+9]
    if col>20: color=col
    else: color=colors[col]
    tg.SetLineColor(color)
    tg.SetLineWidth(2)
    tg.SetMarkerStyle(mark)
    tg.SetMarkerSize(1.18)
    tg.SetMarkerColor(color)
    tg.SetLineStyle(line)

