#include "PulseAnalysis.h"
#include "pATools.h"
#include "PicoReader.h"
#include <TSystem.h>
#include <TObject.h>
#include <TInterpreter.h>
#include <TApplication.h> 
#include "TString.h"
#include "TSpectrum.h"
#include "TList.h"
#include "TPolyMarker.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFitResultPtr.h"
#include "TF1.h"
#include <TNtuple.h>
#include "TTree.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TQObject.h"
#include <TGButton.h>
#include <TGClient.h>
#include <TGLabel.h>
#include "TGFileBrowser.h"
#include "TROOT.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TArrayF.h"
#include "TArrayC.h"
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream> 


#include "dbg.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using TMath::Sqrt;

PulseAnalysis::PulseAnalysis(TString fName){ 
  debug();
  if (fName=="") return;
  if (!fName.EndsWith(".root")) ConvertFile(fName);
  else _tfName=fName;
  LoadSpectrum();
}

PulseAnalysis::~PulseAnalysis() { 
  debug();
  if (_tf) _tf->Close(); 
} 

void PulseAnalysis::ConvertFile(TString fName) { 
  log_info("Converting file %s",fName.Data());
  debug();
  PicoReader* reader = new PicoReader();
  _tfName=fName;  // name for root file
 
  // Convert file from MATLAB or ASCII format to ROOT binary
  debug("Begin Conversion of: %s",fName.Data());

  // slighty dangerous assumption about file name conventions
  _tfName.ReplaceAll(".txt",".root");
  _tfName.ReplaceAll(".csv",".root");
  _tfName.ReplaceAll(".mat",".root");

  reader->Convert(fName, _tfName);  // to do add error check
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

void PulseAnalysis::LoadSpectrum(TString fName){
  if (fName.Contains(".root")) _tfName = fName;
  else ConvertFile(fName); 
  Reset();  
  LoadSpectrum();
}

void PulseAnalysis::LoadSpectrum() { 
  debug();
  log_info("Loading: %s",_tfName.Data());
  _tf=new TFile(_tfName);
  if (!_tf||_tf->IsZombie()) {
    debug("Invalid spectrum file: ",_tfName);
    return;
  }

  debug("Loading Spectrum: %s",_tf->GetName());

  TArrayF *volts=(TArrayF*)_tf->Get("volts");
  TArrayC *trigger=(TArrayC*)_tf->Get("trigger");

  if (!volts){
    debug("Spectrum data not found"); 
    return;
  }
  if (!trigger) log_info("Trigger data not present");

  _nbins=volts->GetSize();
  debug("got size %d",_nbins);

  Double_t t0=((TH1D*)_tf->Get("T0"))->GetBinContent(1);
  _dT=((TH1D*)_tf->Get("dT"))->GetBinContent(1);
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
}


void PulseAnalysis::DrawSpectrum() { 
  if (_hspect) { 
    _hspect->DrawCopy(); 
  }
}

void PulseAnalysis::SmoothHistogram() {
  debug();
  if (!_hspect) { 
    std::cout << "Spectrum not loaded!" << std::endl; 
    return; 
  }
  std::cout << "Smoothing Histogram" << std::endl; 
  _hspect->Smooth(); 
  
}  
TString PulseAnalysis::FindPeaks(bool nodraw){ 
  debug();
  // Search the data for amplitudes above a given threshold, diplay the peaks,
  // and display the pulse rate.

  if (!_hspect){
    log_info("Load specturm first");
    return TString("");
  }

  debug("search parameters: threshold = %f , sigma(bins) = %f",
	_pThreshold, _pSigma);

  Double_t maxpeak=_hspect->GetBinContent(_hspect->GetMaximumBin());
  //  float minpeak=State::_hspect->GetBinContent(State::_hspect->GetMinimumBin());
  Double_t thresFrac;
  if (_pThreshold<0){ // not initialized by user
    log_info("Threshold not set, using 0.20 of maximum voltage");
    thresFrac=0.20;  // 20% of max peak
  }
  else {thresFrac=_pThreshold/maxpeak;}   // fix me?

  TSpectrum *s = new TSpectrum(MAXPEAKS,2);
  // TH1 *hb = s->Background(State::_hspect,20,"same,kBackOrder8,kBackSmoothing15");

  //  float _pThreshold=_pThreshold/maxpeak;  // fix me?
  //  cout<<"thresholds: "<<threshold<<" "<<threshold<<endl;
  if (nodraw)
    _pNFound = s->Search(_hspect, _pSigma, "nobackground,nomarkov,nodraw",thresFrac);
  else
    _pNFound = s->Search(_hspect,_pSigma,"nobackground,nomarkov",thresFrac);

  // http://root.cern.ch/root/htmldoc/TSpectrum.html#TSpectrum:Background
  // TH1 *hb = s->Background(_hspect,100,"same,kBackOrder8,kBackSmoothing15");
  // http://root.cern.ch/root/html/tutorials/spectrum/peaks.C.html
  cout << "Found " << _pNFound << " peaks" << endl;
  // rate of pulses
  float rate=_pNFound/(_xmax-_xmin)*1000;  // in MHz, time scale is in [ns]
  _pulseRate=rate;   // clean this up later!
  TString rateUnits = "MHz";
  // Adapt displayed units for easier reading 
  if (rate <= 1e-2){    // If rate under 10 kHz display in kHz
    rate = rate*1e3;
    rateUnits.Form("kHz");
  }
  printf("Rate of pulses = %6.2e %s\n",rate, rateUnits.Data());
  TString msg;
  msg.Form("Pulse Rate: %6.2e %s",rate, rateUnits.Data());

  TPaveLabel *tp=new TPaveLabel(0.75,0.85,0.95,0.95,msg,"NDC");
  tp->Draw();  
  msg.Form("Found %d peaks",_pNFound);
  return msg; 
}

void PulseAnalysis::FindPeaksandReduce(Float_t window) { 
// Find Peaks and then eliminate non-peak regions by converting to a zero bin 
  FindPeaks(true); 
  TList *functions = _hspect->GetListOfFunctions(); 
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker"); 
  Double_t *pmarrayX = pm->GetX(); 

  Double_t halfWindow = _dT * window * 0.5; 
  std::cout << "halfWindow:" << halfWindow << std::endl; 

  Int_t *index=new Int_t[_pNFound]; 
  vector<vector<UInt_t> > startEndPairs; 
  TMath::Sort(_pNFound, pmarrayX, index, kFALSE);  // index sort by timestamp
  bool inWindow = false; 

  //Double_t start = 0.0; 
  // Double_t end =  0.0; 
  UInt_t s = 0; 
  //UInt_t c = 0; 

  for (int i = 0; i < _pNFound; i++) { 
    Double_t current = pmarrayX[index[i]];
    
    if (!inWindow) { 
      //  start = current - halfWindow;
      s = _hspect->GetBin(current-halfWindow); 
    }

    //c = _hspect->GetBin(current); 
    
    //end = current + halfWindow; 
    Double_t next = pmarrayX[index[i+1]]; 
    Double_t difference = next - current; 
    std::cout << "Next:Current::" << next << " " << current << " Differece:" << difference << " I:" << i << std::endl; 
    bool check = (difference < halfWindow) && ((i+1) < _pNFound); 
    if (check) {
      if (!inWindow) {
	inWindow = true; 
      }
      std::cout << "Extending window" << std::endl; 

    }
    else {
      std::vector<UInt_t> startEndPair; 
      startEndPair.push_back(s); 
      startEndPair.push_back(_hspect->GetBin(current+halfWindow)); 
      inWindow = false; 
      startEndPairs.push_back(startEndPair); 

    }
    
  }

  std::cout << "Number of Start/End Pairs: " << startEndPairs.size() << std::endl; 
  for (std::vector<vector<UInt_t> >::iterator it = startEndPairs.begin(); it != startEndPairs.end(); ++it) { 
    std::cout << "Start:End:Difference::" << (*it)[0] << ":" << (*it)[1] << ":" << (*it)[1] - (*it)[0] << std::endl; 
  }

  //Double_t t0=((TH1D*)_tf->Get("T0"))->GetBinContent(1);
  TH1F *reduced = new TH1F("_reduced","Pulse Spectrum;Time [ns];Amplitude [mV]",
		    _nbins, _xmin, _xmax);

  for (std::vector<vector<UInt_t> >::iterator it = startEndPairs.begin(); it != startEndPairs.end(); ++it) { 
    Double_t start = (*it)[0]; 
    Double_t end = (*it)[1]; 
    std::cout << "Setting Bins:" << start << ":" << end << std::endl; 
    UInt_t i = start; 
    do { 
      reduced->SetBinContent(i,   _hspect->GetBinContent(i)); 
      i++; 
    }
    while (i < end); 
  }
    
  reduced->Draw(); 
  

}
 


void PulseAnalysis::Analyze(){ 
  debug("");
  if (!_pNFound){
    std::cout << "Find peaks first." << std::endl;return;
  }

  TList *functions = _hspect->GetListOfFunctions();
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
  Double_t *pmarrayX;
  Double_t *pmarrayY;
  pmarrayX=pm->GetX();
  pmarrayY=pm->GetY();
  
  Int_t *index=new Int_t[PulseAnalysis::MAXPEAKS];
  TMath::Sort(_pNFound, pmarrayX, index, kFALSE);  // index sort by timestamp
  AnaClean();

  // Analysis of peak data
  _hdt = new TH1F("hdt","Delta Time;Peak-to-peak time [ns];Entries",
		 201,-_dT/2,3/_pulseRate*1000+_dT/2);
  // Pulse heights
  _hph = new TH1F("hph","Pulse Height;[mV];Entries",100,0,_pMax*1.1);
  // Pulse RMS width
  _hprms =  new TH1F("hprms","Pulse RMW Width;[ns];Entries",100,0,_pWidth*2);
  debug("Pointer Values for histograms, _hdt:%p, _hph:%p, _hprms:%p", _hdt, _hph, _hprms); 
  Double_t maxInt=-1e-30;

  int count0=0, count1=0, count2=0, count3=0;
  int fivepct=_pNFound/20;

  debug("starting peak integrals");
  double sumv, sumt, sum2t;
  for(int i=0;i<_pNFound;++i) {
    if (i && i%fivepct==0) cerr << "*";
    // Fill Delta Time histogram with time between found peaks.
    if (i<_pNFound-1) _hdt->Fill(pmarrayX[index[i+1]]-pmarrayX[index[i]]);
    _hph->Fill(pmarrayY[i]);

    // integrate the peaks, using a simple sum of bins 0.5 sigma above baseline noise
    double cut=_basePar[0]+_basePar[1]/2;
    // locate bin corresponding to peak finder's result
    int pBin=_hspect->FindBin(pmarrayX[index[i]]);
    _pInteg[i]=0;
    _pRMS[i]=0;
    sumv=0; sumt=0; sum2t=0;
    int ib=pBin;
    while (ib>0){                     // left side
      if (_hspect->GetBinContent(ib)>cut){
	double v=_hspect->GetBinContent(ib);
	double t=_hspect->GetBinCenter(ib); 
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;
	ib--;
      }
      else break;
    }
    ib=pBin+1;
    while (ib<=_hspect->GetNbinsX()){  // right side
      if (_hspect->GetBinContent(ib)>cut){
	double v=_hspect->GetBinContent(ib);
	double t=_hspect->GetBinCenter(ib);
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;	
	ib++;
      }
      else break;
    }
    _pInteg[i]=sumv/_dT/1000;                                          // convert mV->V
    _pRMS[i]=Sqrt( TMath::Abs(sum2t/sumv - sumt*sumt/sumv/sumv) );     // RMS width of peak
    cout << _pRMS[i] << "  " << sum2t/sumv << "  "  << sumt*sumt/sumv/sumv << endl;

    if (_pInteg[i]>maxInt) maxInt = _pInteg[i];

    if (pmarrayY[i]>=_pThreshold) count0++;
    if (pmarrayY[i]>=_pThreshold*3) count1++;
    if (pmarrayY[i]>=_pThreshold*3&&pmarrayY[i]<_pThreshold*5) count2++;
    if (pmarrayY[i]>=_pThreshold*5&&pmarrayY[i]<_pThreshold*7) count3++;
  }
  cerr << endl;
  cout << "cross talk (total)" << (float)count1/count0 << endl;
  cout << "cross talk (2x)" << (float)count2/count0 << endl;
  cout << "cross talk (3x)" << (float)count3/count0 << endl;


  // Pulse integrals
  //  hpi=new TH1F("hpi","Pulse Integral;[mV][ns];Entries",200,0,maxInt*1.05);
  _hpi=new TH1F("hpi","Pulse Integral;[Vns];Entries",100,0,maxInt*1.1);  // need to fix upper limit
  for(int i=0;i<_pNFound;i++) { 
    _hpi->Fill(_pInteg[i]); 
    _hprms->Fill(_pRMS[i]); 
  }
  TF1 *xtalball=new TF1("xtalball",XTLBall,0,maxInt*1.1,5);
  xtalball->SetParameters(1,3,_hpi->GetMean(),_hpi->GetRMS(),_hpi->GetMaximum());
  _hpi->Fit("gaus","0");
  _hpi->Fit("xtalball","0");

  delete[] index;
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

void PulseAnalysis::inzoom(){ 
  debug();
  // Reduce the spectrum viewport range in by a factor of 2.
  if (!_hspect) 
    return; 
  int first=_hspect->GetXaxis()->GetFirst();
  int last=_hspect->GetXaxis()->GetLast();
  int range=last-first+1;  // # of bins displayed
  int center=(last+first)/2;
  _hspect->GetXaxis()->SetRange(center-range/4,center+range/4);
}

void PulseAnalysis::unzoom(){ 
  debug();
  if (!_hspect)
    return; 
  _hspect->GetXaxis()->SetRange(2,1);  //if last < first the range is reset
}

void PulseAnalysis::outzoom(){ 
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
