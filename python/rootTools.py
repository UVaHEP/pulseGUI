import os
from ROOT import Double, gPad, TGaxis, TMath

# scale graphis object for plotting on gPad
# assuming same x-axis for now, and no scaling for ymin
def scaleToPad(obj2):
    if gPad==0:
        print "plotOver: No current TPad. Skip drawing."
        return
    isGraph=obj2.InheritsFrom("TGraph")
    isTH1=obj2.InheritsFrom("TH1")
    knownType = isGraph or isTH1
    logy=(gPad.GetLogy()==1)
    if not knownType:
        print "plotOver: unknown plot type. Skip drawing."
        return
    if isGraph:
        xmin=Double(); xmax=Double(); ymin=Double(); ymax=Double()
        obj2.ComputeRange(xmin, ymin, xmax, ymax)
        rightmax=ymax*1.1  # right axis
        scale = gPad.GetUymax()/rightmax;
        if logy: scale=TMath.Power(10,scale)
        for i in range(obj2.GetN()):
            obj2.GetY()[i] = obj2.GetY()[i]*scale
    elif isTH1:
        rightmax = 1.1*obj2.GetMaximum();
        scale = gPad.GetUymax()/rightmax;
        if logy: scale=TMath.Power(10,scale)
        obj2.Scale(scale)
    axisYmax=gPad.GetUymax()
    if logy: axisYmax=TMath.Power(10,axisYmax)
    axis = TGaxis(gPad.GetUxmax(),gPad.GetUymin(),
                      gPad.GetUxmax(),axisYmax,0,rightmax,510,"+L")
    color=obj2.GetLineColor()
    axis.SetLineColor(color)
    axis.SetTextColor(color)
    axis.SetLabelColor(color)
    axis.SetTitle("Gain")
    return obj2,axis





