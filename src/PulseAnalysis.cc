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


/*PulseAnalysis::PulseAnalysis(TString fName){ 
  debug();
  if (fName=="") return;
  if (!fName.EndsWith(".root")) ConvertFile(fName);
  else _currentTFile=fName;
  LoadSpectrum();
  }*/

PulseAnalysis::PulseAnalysis(PSbuffer *buffer){
  psbuffer=buffer;

}

PulseAnalysis::PulseAnalysis() { 


}


PulseAnalysis::~PulseAnalysis() { 
  debug();
  //  if (_tf) _tf->Close(); 
} 


void PulseAnalysis::AnaClean(){ 
  debug();
  if (_hdt) _hdt = NULL; 
  if (_hph) _hph = NULL; 
  if (_hpi) _hpi = NULL;
  if (_hprms) _hprms = NULL;
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
  _pSigma = _pWidth/2/_dT;
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
