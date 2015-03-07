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

  TList *functions = psbuffer->GetSpectrum()->GetListOfFunctions();
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
  Double_t *pmarrayX;
  Double_t *pmarrayY;
  pmarrayX=pm->GetX();
  pmarrayY=pm->GetY();
  
  Int_t *index=new Int_t[PulseAnalysis::MAXPEAKS];
  TMath::Sort(_pNFound, pmarrayX, index, kFALSE);  // index sort by timestamp
  AnaClean();

  // Analysis of peak data
  Double_t maxInt=-1e-30;

  int count0=0, count1=0, count2=0, count3=0;
  int fivepct=_pNFound/20;

  debug("starting peak integrals");
  double sumv, sumt, sum2t;
  for(int i=0;i<_pNFound;++i) {
    if (i && i%fivepct==0) cerr << "*";
    // Fill Delta Time histogram with time between found peaks.
    if (i<_pNFound-1) _hdt.Fill(pmarrayX[index[i+1]]-pmarrayX[index[i]]);
    _hph.Fill(pmarrayY[i]);

    // integrate the peaks, using a simple sum of bins 0.5 sigma above baseline noise
    double cut=_basePar[0]+_basePar[1]/2;
    // locate bin corresponding to peak finder's result
    int pBin=psbuffer->GetSpectrum()->FindBin(pmarrayX[index[i]]);
    _pInteg[i]=0;
    _pRMS[i]=0;
    sumv=0; sumt=0; sum2t=0;
    int ib=pBin;
    while (ib>0){                     // left side
      if (psbuffer->GetSpectrum()->GetBinContent(ib)>cut){
	double v=psbuffer->GetSpectrum()->GetBinContent(ib);
	double t=psbuffer->GetSpectrum()->GetBinCenter(ib); 
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;
	ib--;
      }
      else break;
    }
    ib=pBin+1;
    while (ib<=psbuffer->GetSpectrum()->GetNbinsX()){  // right side
      if (psbuffer->GetSpectrum()->GetBinContent(ib)>cut){
	double v=psbuffer->GetSpectrum()->GetBinContent(ib);
	double t=psbuffer->GetSpectrum()->GetBinCenter(ib);
	sumv+=v;
	sumt+=t*v;
	sum2t+=t*t*v;	
	ib++;
      }
      else break;
    }
    _pInteg[i]=sumv/psbuffer->Dt()/1000;                                          // convert mV->V
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
  _hpi.SetBins(100,0,maxInt*1.1);  // need to fix upper limit
  for(int i=0;i<_pNFound;i++) { 
    _hpi.Fill(_pInteg[i]); 
    _hprms.Fill(_pRMS[i]); 
  }
  TF1 *xtalball=new TF1("xtalball",XTLBall,0,maxInt*1.1,5);
  xtalball->SetParameters(1,3,_hpi.GetMean(),_hpi.GetRMS(),_hpi.GetMaximum());
  _hpi.Fit("gaus","0");
  _hpi.Fit("xtalball","0");

  delete[] index;
}



// simple peak counter
int  PulseAnalysis::CountPeaksFast(double threshold) const{
  if (threshold==0) threshold=_pThreshold; // use previously set threshold

  // scan for peaks > threshold
  int window=(int)(_pSigma*2);
  int nb=1;  // bin number
  int nbins=psbuffer->GetSpectrum()->GetNbinsX();
  int lastWin=(nbins-window)+1;
  int count=0;
  while ( nb <= lastWin ){
      bool onPeak=true;
      for (int nw=0;nw<window;nw++){  // sliding search window for peak
	nb++;
	if (psbuffer->GetSpectrum()->GetBinContent(nb)<threshold){
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
  double step=(psbuffer->GetSpectrum()->GetMaximum()*0.666-threshold0)/nsteps;
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

  /*  if (!_hspect){
    log_info("Load specturm first");
    return TString("");
    }*/

  debug("search parameters: threshold = %f , sigma(bins) = %f",
	_pThreshold, _pSigma);

  Double_t maxpeak=psbuffer->GetSpectrum()->GetBinContent(psbuffer->GetSpectrum()->GetMaximumBin());
  //  float minpeak=State::_hspect->GetBinContent(State::_hspect->GetMinimumBin());
  Double_t thresFrac;
  if (_pThreshold<0){ // not initialized by user
    log_info("Threshold not set, using 0.20 * maximum voltage");
    thresFrac=0.20;  // 20% of max peak
  }
  else {thresFrac=_pThreshold/maxpeak;}   // fix me?

  TSpectrum *s = new TSpectrum(MAXPEAKS,2);
  // TH1 *hb = s->Background(State::_hspect,20,"same,kBackOrder8,kBackSmoothing15");

  //  float _pThreshold=_pThreshold/maxpeak;  // fix me?
  //  cout<<"thresholds: "<<threshold<<" "<<threshold<<endl;
  if (nodraw)
    _pNFound = s->Search(psbuffer->GetSpectrum(), _pSigma, "nobackground,nomarkov,nodraw",thresFrac);
  else
    _pNFound = s->Search(psbuffer->GetSpectrum(),_pSigma,"nobackground,nomarkov",thresFrac);

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
  TList *functions = psbuffer->GetSpectrum()->GetListOfFunctions(); 
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker"); 
  Double_t *pmarrayX = pm->GetX(); 

  Double_t halfWindow = psbuffer->Dt() * window * 0.5; 
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
      s = psbuffer->GetSpectrum()->GetBin(current-halfWindow); 
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
      startEndPair.push_back(psbuffer->GetSpectrum()->GetBin(current+halfWindow)); 
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
      reduced->SetBinContent(i,   psbuffer->GetSpectrum()->GetBinContent(i)); 
      i++; 
    }
    while (i < end); 
  }
    
  reduced->Draw(); 
  

}
 


