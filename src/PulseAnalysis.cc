#include "dbg.h"
#include "PicoReader.h"
#include "PulseAnalysis.h"

#include "TF1.h"
#include "TH1D.h"
#include "TMath.h"
#include "TString.h"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;


PulseAnalysis::PulseAnalysis(PSbuffer *buffer){
  SetBuffer(buffer);
}

PulseAnalysis::PulseAnalysis() {
}

void PulseAnalysis::Init(){
  _hdt.SetName("hdt");
  _hdt.SetTitle("Delta Time;Peak-to-peak time [ns];Entries");
  _hph.SetName("hph");
  _hph.SetTitle("Pulse Height;[mV];Entries");
  _hpi.SetName("hpi");
  _hpi.SetTitle("Pulse Integral;[V#cdot ns];Entries");
  _hprms.SetName("hprms");
  _hprms.SetTitle("Pulse RMW Width;[ns];Entries");
}

PulseAnalysis::~PulseAnalysis() { 
  debug();
  AnaClean();
} 

void PulseAnalysis:: SetBuffer(PSbuffer *buffer){
  debug();
  Clear();
  psbuffer=buffer;
  double dT=psbuffer->Dt();
  _hdt.SetBins(201,-dT/2,3/_pulseRate*1000+dT/2);
  TH1F *waveBuffer=psbuffer->GetWaveform();
  double pMax = waveBuffer->GetBinContent(waveBuffer->GetMaximumBin());
  _hph.SetBins(100,0,pMax*1.1); 
  _hprms.SetBins(100,0,50);  // up to 50 ns width [hack, fix later!]
}

void PulseAnalysis::AnaClean(){ 
  debug();
  _hdt.Reset(); 
  _hph.Reset(); 
  _hpi.Reset();
  _hprms.Reset();
  debug();
}

void PulseAnalysis::Clear() {
  debug();
  // Return all useful variables to default/NULL.
  _pThreshold = -1.0; 
  _pSigma=0;
  _pWidth=0;
  _pNFound=0;
  _pulseRate=0;  
  AnaClean();
}

void PulseAnalysis::Reset() { 
  debug();
  Clear();
  //  if (_tf) LoadSpectrum(); 
}




void PulseAnalysis::DumpPeaks(){
  
  debug();
}

void PulseAnalysis::SubBkg(){
  debug();
}


TString PulseAnalysis::SetWidth(Float_t w){
  debug();
 // set width in microseconds
  _pWidth = w; 
  _pSigma = _pWidth/2/psbuffer->Dt();
  TString s; 
  s.Form("Peak width (FWHM) set to: %e , half width in bins %d",
	 _pWidth,TMath::Nint(_pSigma));
  return s; 
}


void PulseAnalysis::SetThreshold(Float_t t) {
  debug();
  _pThreshold = t;
}

void PulseAnalysis::SetXMarks(){
  debug("n/a");
}

void PulseAnalysis::SetYMarks(){
  debug("n/a");
}

void PulseAnalysis::GetBkg(){ 
  debug("n/a");
}

void PulseAnalysis::SetResponse(){
  debug("n/a");
}

void PulseAnalysis::Print(){
  debug("n/a");
}

void PulseAnalysis::WriteHists(){ 
  /*  debug();
  TFile *tf=new TFile("hists.root","recreate");
  if (_hspect) _hspect->Write();
  tf->Close();
  delete tf;*/
}
