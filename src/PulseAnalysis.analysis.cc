#include "dbg.h"
#include "pATools.h"
#include "PulseAnalysis.h"

#include "TF1.h"
#include "TMath.h"
#include "TPaveLabel.h"
#include "TPolyMarker.h"
#include "TSpectrum.h"

#include <iostream>
#include <vector> 
using std::cout;
using std::cerr;
using std::endl;
using TMath::Sqrt;
using std::vector;


void PulseAnalysis::Analyze(){ 
  debug("");
  if (!_pNFound){
    std::cout << "Find peaks first." << std::endl;return;
  }

  TList *functions = psbuffer->GetWaveform()->GetListOfFunctions();
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
  Double_t *pmarrayX=pm->GetX();
  Double_t *pmarrayY=pm->GetY();
  
  Int_t *index=new Int_t[PulseAnalysis::MAXPEAKS];
  TMath::Sort(_pNFound, pmarrayX, index, kFALSE);  // index sort by timestamp
  AnaClean();

  // Analysis of peak data
  Double_t maxInt=-1e-30;

  int count0=0, count1=0, count2=0, count3=0;
  //  int fivepct=_pNFound/20;
  double sumv, sumt, sum2t;
  
  debug("starting peak integrals");
  for(int i=0;i<_pNFound;++i) {
    //if (i && i%fivepct==0) cerr << "*";
    // Fill Delta Time histogram with time between found peaks.
    if (i<_pNFound-1) _hdt.Fill(pmarrayX[index[i+1]]-pmarrayX[index[i]]);
    _hph.Fill(pmarrayY[i]);

    // integrate the peaks, using a simple sum of bins 0.5 sigma above baseline noise
    double cut=_basePar[0]+_basePar[1]/2;
    // locate bin corresponding to peak finder's result
    int pBin=psbuffer->GetWaveform()->FindBin(pmarrayX[index[i]]);
    _pInteg[i]=0;
    _pRMS[i]=0;
    sumv=0; sumt=0; sum2t=0;
    int ib=pBin;
    while (ib>0){                     // left side
      if (psbuffer->GetWaveform()->GetBinContent(ib)>cut){
	double v=psbuffer->GetWaveform()->GetBinContent(ib);
	double t=psbuffer->GetWaveform()->GetBinCenter(ib); 
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;
	ib--;
      }
      else break;
    }
    ib=pBin+1;
    while (ib<=psbuffer->GetWaveform()->GetNbinsX()){  // right side
      if (psbuffer->GetWaveform()->GetBinContent(ib)>cut){
	double v=psbuffer->GetWaveform()->GetBinContent(ib);
	double t=psbuffer->GetWaveform()->GetBinCenter(ib);
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;	
	ib++;
      }
      else break;
    }
    _pInteg[i]=sumv/psbuffer->Dt()/1000;                                          // convert mV->V
    _pRMS[i]=Sqrt( TMath::Abs(sum2t/sumv - sumt*sumt/sumv/sumv) );     // RMS width of peak
    //  cout << _pRMS[i] << "  " << sum2t/sumv << "  "  << sumt*sumt/sumv/sumv << endl;

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
  _hpi.SetBins(100,0,maxInt*1.1);  // need to fix upper limit
  for(int i=0;i<_pNFound;i++) { 
    _hpi.Fill(_pInteg[i]); 
    _hprms.Fill(_pRMS[i]); 
  }
  TF1 *xtalball=new TF1("xtalball",XTLBall,0,maxInt*1.1,5);
  xtalball->SetParameters(1,3,_hpi.GetMean(),_hpi.GetRMS(),_hpi.GetMaximum());
  _hpi.Fit("gaus","0q");
  _hpi.Fit("xtalball","0q");

  delete[] index;
}


// simple peak counter
int  PulseAnalysis::CountPeaksFast(double threshold) const{
  if (threshold==0) threshold=_pThreshold; // use previously set threshold

  // scan for peaks > threshold
  int window=(int)(_pSigma*2);
  int nb=1;  // bin number
  int nbins=psbuffer->GetWaveform()->GetNbinsX();
  int lastWin=(nbins-window)+1;
  int count=0;
  while ( nb <= lastWin ){
      bool onPeak=true;
      for (int nw=0;nw<window;nw++){  // sliding search window for peak
	nb++;

	if (psbuffer->GetWaveform()->GetBinContent(nb)<threshold){
	  onPeak=false;
	  break;
	}
      }
      if (onPeak) count++;
  }
  return count;
}


// simple peak counter scan from ~current threshold to maximum peak
void PulseAnalysis::ScanPeaksFast(int nsteps, 
				   double *thresholds, double *count) const{
  double threshold0=_pThreshold/2;
  double step=(psbuffer->GetWaveform()->GetMaximum()*0.666-threshold0)/nsteps;
  for (int ns=0; ns<=nsteps; ns++){
    thresholds[ns]=threshold0+ns*step;
    cout << "Scanning thresold "<< thresholds[ns] << endl;
    count[ns]=CountPeaksFast(thresholds[ns]);
    cout << "Peaks found: "<< count[ns] << endl;
  }
}



TString PulseAnalysis::FindPeaks(bool nodraw){ 
  debug();
  // Search the data for amplitudes above a given threshold, diplay the peaks,
  // and display the pulse rate.

  debug("search parameters: threshold = %f , sigma(bins) = %f",
	_pThreshold, _pSigma);

  Double_t maxpeak=psbuffer->GetWaveform()->GetBinContent(psbuffer->GetWaveform()->GetMaximumBin());
  //  float minpeak=State::_hspect->GetBinContent(State::_hspect->GetMinimumBin());
  Double_t thresFrac;
  if (_pThreshold<0){ // not initialized by user
    psbuffer->Print();
    thresFrac=3*psbuffer->Noise()/maxpeak;
    log_info("Threshold not set, 3 sigma noise cut %f",thresFrac);
  }
  else {thresFrac=_pThreshold/maxpeak;}   // fix me?

  TSpectrum *s = new TSpectrum(MAXPEAKS);

  if (nodraw)
    _pNFound = s->Search(psbuffer->GetWaveform(), _pSigma, "nobackground,nomarkov,nodraw",thresFrac);
  else
    _pNFound = s->Search(psbuffer->GetWaveform(),_pSigma,"nobackground,nomarkov",thresFrac);

  // http://root.cern.ch/root/htmldoc/TSpectrum.html#TSpectrum:Background
  // TH1 *hb = s->Background(_hspect,100,"same,kBackOrder8,kBackSmoothing15");
  // http://root.cern.ch/root/html/tutorials/spectrum/peaks.C.html
  cout << "Found " << _pNFound << " peaks" << endl;
  // rate of pulses
  double sampleTime=psbuffer->Dt()*psbuffer->Samples();
  _pulseRate=_pNFound/sampleTime*1000;  // in MHz, time scale is in [ns]
  TString rateUnits = "MHz";
  double rateScale=1;
  // Adapt displayed units for easier reading 
  if (_pulseRate <= 1e-2){    // If rate under 10 kHz display in kHz
    rateScale=1e3;
    rateUnits.Form("kHz");
  }
  printf("Rate of pulses = %6.2e %s\n",_pulseRate*rateScale, rateUnits.Data());
  TString msg;
  msg.Form("Pulse Rate: %6.2e %s",_pulseRate*rateScale, rateUnits.Data());

  TPaveLabel *tp=new TPaveLabel(0.75,0.85,0.95,0.95,msg,"NDC");
  tp->Draw();  
  msg.Form("Found %d peaks",_pNFound);
  return msg; 
}

void PulseAnalysis::FindPeaksandReduce(Float_t window) { 
// Find Peaks and then eliminate non-peak regions by converting to a zero bin 
  FindPeaks(true); 

  TList *functions = psbuffer->GetWaveform()->GetListOfFunctions(); 
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker"); 
  Double_t *pmarrayX = pm->GetX(); 

  Double_t halfWindow = psbuffer->Dt() * window * 0.5; 
  std::cout << "halfWindow:" << halfWindow << std::endl; 

  Int_t *index=new Int_t[_pNFound]; 
  vector<vector<UInt_t> > startEndPairs; 
  TMath::Sort(_pNFound, pmarrayX, index, kFALSE);  // index sort by timestamp
  bool inWindow = false; 

  UInt_t s = 0; 

  for (int i = 0; i < _pNFound; i++) { 
    Double_t current = pmarrayX[index[i]];
    
    if (!inWindow) { 
      s = psbuffer->GetWaveform()->GetBin(current-halfWindow); 
    }

    Double_t next = pmarrayX[index[i+1]]; 
    Double_t difference = next - current; 
    std::cout << "Next:Current::" << next << " " << current << " Difference:" << difference << " I:" << i << std::endl; 
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

      startEndPair.push_back(psbuffer->GetWaveform()->GetBin(current+halfWindow)); 
      inWindow = false; 
      startEndPairs.push_back(startEndPair);
    }
    
  }

  std::cout << "Number of Start/End Pairs: " << startEndPairs.size() << std::endl; 
  for (std::vector<vector<UInt_t> >::iterator it = startEndPairs.begin(); it != startEndPairs.end(); ++it) { 
    std::cout << "Start:End:Difference::" << (*it)[0] << ":" << (*it)[1] << ":" << (*it)[1] - (*it)[0] << std::endl; 
  }
  TH1F *reduced = (TH1F*)(psbuffer->GetWaveform()->Clone("_reduced"));
  reduced->Reset();

  for (std::vector<vector<UInt_t> >::iterator it = startEndPairs.begin(); it != startEndPairs.end(); ++it) { 
    Double_t start = (*it)[0]; 
    Double_t end = (*it)[1]; 
    std::cout << "Setting Bins:" << start << ":" << end << std::endl; 
    UInt_t i = start; 
    do { 
      reduced->SetBinContent(i, psbuffer->GetWaveform()->GetBinContent(i)); 
      i++; 
    }
    while (i < end); 
  }
    
  reduced->Draw(); 
}


Double_t PulseAnalysis::FixedIntegral() const{
  // Dump();
  return psbuffer->GetWaveform()->Integral(_intWindow[0],_intWindow[1]);
}


