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


PulseAnalysis::PulseAnalysis(TString fName){ 
  debug();
  if (fName=="") return;
  if (!fName.EndsWith(".root")) ConvertFile(fName);
  else _currentTFile=fName;
  LoadSpectrum();
}

PulseAnalysis::PulseAnalysis(PSbuffer *buffer){
  psbuffer=buffer;
}


PulseAnalysis::~PulseAnalysis() { 
  debug();
  if (_tf) _tf->Close(); 
} 

void PulseAnalysis::ConvertFile(TString fName) { 
  log_info("Converting file %s",fName.Data());
  debug();
  PicoReader* reader = new PicoReader();
  _currentTFile=fName;  // name for root file
 
  // Convert file from MATLAB or ASCII format to ROOT binary
  debug("Begin Conversion of: %s",fName.Data());

  // slighty dangerous assumption about file name conventions
  _currentTFile.ReplaceAll(".txt",".root");
  _currentTFile.ReplaceAll(".csv",".root");
  _currentTFile.ReplaceAll(".mat",".root");

  reader->Convert(fName, _currentTFile);  // to do add error check
  delete reader;

  debug("Conversion Finished");
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

  if (_hspect) {delete _hspect; _hspect=0; debug("deleted _hspect");}
  if (_hfreq) {delete _hfreq; _hfreq=0; debug("deleted _hfreq");}
  
  AnaClean();
}

void PulseAnalysis::Reset() { 
  debug();
  Clear();
  if (_tf) LoadSpectrum(); 
}

void PulseAnalysis::LoadBuffer(TString Filename){
  _currentTFile=Filename;
  _tf = new TFile(_currentTFile);
  psbuffer=(PSbuffer *)_tf->Get("PSbuffer");
}


// convert text data file if necessary, then set data file name and load
void PulseAnalysis::LoadSpectrum(TString fName){
  if (fName.Contains(".root")) _currentTFile = fName;
  else ConvertFile(fName); 
  Reset();  
  LoadSpectrum();
}

// load data from current root file
void PulseAnalysis::LoadSpectrum() { 
  debug();
  log_info("Loading: %s",_currentTFile.Data());
  _tf=new TFile(_currentTFile);
  if (!_tf||_tf->IsZombie()) {
    debug("Invalid spectrum file: %s",_currentTFile.Data());
    return;
  }

  debug("Loading Spectrum: %s",_tf->GetName());

  TArrayF *volts=(TArrayF*)_tf->Get("volts");

  if (!volts){
    debug("Spectrum data not found"); 
    return;
  }

  _nbins=volts->GetSize();
  debug("got size %d",_nbins);

  Double_t t0=((TH1D*)_tf->Get("T0"))->GetBinContent(1);
  _dT=((TH1D*)_tf->Get("dT"))->GetBinContent(1);  // in ns
  Double_t dV=((TH1F*)_tf->Get("dV"))->GetBinContent(1);
  _pMax=((TH1F*)_tf->Get("vMax"))->GetBinContent(1);
  _pMin=((TH1F*)_tf->Get("vMin"))->GetBinContent(1);
  Double_t sign=((TH1F*)_tf->Get("sign"))->GetBinContent(1);
  debug("Finish reading psdata");

  if (sign < 0){ // handle the sign flip
    Double_t tmp=_pMax;
    _pMax=-_pMin;
    _pMin=-tmp;
  }

  _xmin = t0-_dT/2; 
  _xmax = t0+(_nbins-0.5)*_dT;
 
  _hspect = new TH1F("_hspect","Pulse Spectrum;Time [ns];Amplitude [mV]",
		    _nbins, _xmin, _xmax);
  _hfreq = new TH1F("_hfreq","_hfreq",int((_pMax-_pMin)/dV)+1
		    ,_pMin-dV/2,_pMax+dV/2);

  for (UInt_t i = 0; i < (UInt_t)_nbins; i++) { 
    _hspect->Fill(t0+i*_dT, volts->GetAt(i)*sign); 
    _hfreq->Fill(volts->GetAt(i)*sign);
  }

  // defaults
  SetWidth(5); 
  //  SetPeriod(); 
  SetThreshold(0.0); 
 
  // Guestimate threshold based on pulse height frequency distro
  // Assume we have mostly noise samples & fit a gaussian to low amplitude region
  _hfreq->Fit("gaus","0R","",_hfreq->GetBinLowEdge(1),_hfreq->GetRMS()*4);
  TF1 *f=_hfreq->GetFunction("gaus");
  _basePar[0]=f->GetParameter(1);
  _basePar[1]=f->GetParameter(2);
  Double_t cut=f->GetParameter(1)+f->GetParameter(2)*PulseAnalysis::NSIGMA;
  SetThreshold(cut); 
  log_info("Threshold set to: %f",cut);
  
  TString msg=TString("Loaded spectrum from: ")+_tf->GetName();
  //debug(msg);

  cout << "VMin:" << _pMin << " VMax:" << _pMax 
       << " dV:" << dV << " [mV]" << " dT: " << _dT << " [ns]" 
       << endl;

  // set default zoom level
  int bHigh=TMath::Nint(DEFAULT_ZOOM/_dT);
  if (bHigh<_hspect->GetNbinsX()) {
    _hspect->GetXaxis()->SetRange(0,bHigh);
    log_info("Initial Zoom setting max bin to: %d of %d",
	     bHigh,_hspect->GetNbinsX());
  }
}


void PulseAnalysis::SmoothHistogram() {
  debug();
  if (!_hspect) { 
    std::cout << "Spectrum not loaded!" << std::endl; 
    return; 
  }
  std::cout << "Smoothing Histogram" << std::endl; 
  TString title=_hspect->GetTitle();
  if (!title.Contains("smoothed")) title+=" (smoothed)";
  _hspect->SetTitle(title);
  _hspect->Smooth(); 
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
  debug();
  TFile *tf=new TFile("hists.root","recreate");
  if (_hspect) _hspect->Write();
  tf->Close();
  delete tf;
}
