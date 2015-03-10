#include "PulseAnalysis.h"
#include "dbg.h"
#include <iostream> 


void PulseAnalysis::DrawSpectrum() { 

  psbuffer->GetWaveform()->Draw();
}

void PulseAnalysis::SmoothHistogram() {
  debug();

  TH1F *spect = psbuffer->GetWaveform(); 
  std::cout << "Smoothing Histogram" << std::endl; 
  TString title=spect->GetTitle();
  if (!title.Contains("smoothed")) title+=" (smoothed)";
  spect->SetTitle(title);
  spect->Smooth(); 
}  


void PulseAnalysis::ZoomIn(){ 
  debug();
  // Reduce the spectrum viewport range in by a factor of 2.
  int first=psbuffer->GetWaveform()->GetXaxis()->GetFirst();   // current 1st and
  int last=psbuffer->GetWaveform()->GetXaxis()->GetLast();     // last bins displayed
  int range=last-first+1;  // # of bins displayed
  int center=(last+first)/2;
  int bLow=center-range/4;
  int bHigh=center+range/4;
  psbuffer->GetWaveform()->GetXaxis()->SetRange(bLow,bHigh);
}

void PulseAnalysis::UnZoom(){ 
  debug();
  psbuffer->GetWaveform()->GetXaxis()->SetRange(2,1);  //if last < first the range is reset
}

void PulseAnalysis::ZoomOut(){ 
  debug();
  // Expand the spectrum viewport viewring range by a factor of 2.

  int nbins= psbuffer->GetWaveform()->GetNbinsX();
  int first=psbuffer->GetWaveform()->GetXaxis()->GetFirst();
  int last=psbuffer->GetWaveform()->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  int center=(last+first)/2;
  psbuffer->GetWaveform()->GetXaxis()->SetRange(TMath::Max(0,center-range),TMath::Min(center+range,nbins));
}

void PulseAnalysis::leftShift(){ 
  debug();
  // Shift the spectrum viewport range to the right.

  int first=psbuffer->GetWaveform()->GetXaxis()->GetFirst();
  int last=psbuffer->GetWaveform()->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  first=TMath::Max(1,first-range);
  last=first+range;
  psbuffer->GetWaveform()->GetXaxis()->SetRange(first,last);
}

void PulseAnalysis::rightShift(){ 
  debug();
  // Shift the spectrum viewport range to the left.

  int nbins=psbuffer->GetWaveform()->GetNbinsX();
  int first=psbuffer->GetWaveform()->GetXaxis()->GetFirst();
  int last=psbuffer->GetWaveform()->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  last=TMath::Min(nbins,last+range);
  first=last-range;
  psbuffer->GetWaveform()->GetXaxis()->SetRange(first,last);
}

