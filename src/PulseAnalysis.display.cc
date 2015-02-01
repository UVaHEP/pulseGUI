#include "PulseAnalysis.h"
#include "dbg.h"

void PulseAnalysis::DrawSpectrum() { 
  if (_hspect) { 
    //    _hspect->DrawCopy(); 
    _hspect->Draw();
  }
}

void PulseAnalysis::ZoomIn(){ 
  debug();
  // Reduce the spectrum viewport range in by a factor of 2.
  if (!_hspect)
    return; 
  int first=_hspect->GetXaxis()->GetFirst();   // current 1st and
  int last=_hspect->GetXaxis()->GetLast();     // last bins displayed
  int range=last-first+1;  // # of bins displayed
  int center=(last+first)/2;
  int bLow=center-range/4;
  int bHigh=center+range/4;
  _hspect->GetXaxis()->SetRange(bLow,bHigh);
}

void PulseAnalysis::UnZoom(){ 
  debug();
  if (!_hspect)
    return; 
  _hspect->GetXaxis()->SetRange(2,1);  //if last < first the range is reset
}

void PulseAnalysis::ZoomOut(){ 
  debug();
  // Expand the spectrum viewport viewring range by a factor of 2.
  if (!_hspect) 
    return; 
  int nbins= _hspect->GetNbinsX();
  int first=_hspect->GetXaxis()->GetFirst();
  int last=_hspect->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  int center=(last+first)/2;
  _hspect->GetXaxis()->SetRange(TMath::Max(0,center-range),TMath::Min(center+range,nbins));
}

void PulseAnalysis::leftShift(){ 
  debug();
  // Shift the spectrum viewport range to the right.
  if (!_hspect)
    return; 
  int first=_hspect->GetXaxis()->GetFirst();
  int last=_hspect->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  first=TMath::Max(1,first-range);
  last=first+range;
  _hspect->GetXaxis()->SetRange(first,last);
}

void PulseAnalysis::rightShift(){ 
  debug();
  // Shift the spectrum viewport range to the left.

  if (!_hspect)
    return; 
  int nbins=_hspect->GetNbinsX();
  int first=_hspect->GetXaxis()->GetFirst();
  int last=_hspect->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  last=TMath::Min(nbins,last+range);
  first=last-range;
  _hspect->GetXaxis()->SetRange(first,last);
}

