/* Analysis object: contains all analysis code, may be used w/o GUI */
#ifndef PULSEANALYSIS_H
#define PULSEANALYSIS_H


#include "TFile.h"
#include "TH1F.h"


//Have to hide PSbuffer from CINT, otherwise the c++11 code causes it to blow up
#ifndef __CINT__
#include "PSbuffer.h"
#else
class PSbuffer;
template<class T> class shared_ptr; 
#endif 

class PulseAnalysis {

 private: 
  //c++11 uses constexpr for calculated values in classes rather than const static, CINT doesn't support constexpr so this is a quick
  //hack to work around the incompatibility 
  #ifdef __CINT__
  const static  Int_t MAXPEAKS = 100000;  // max peaks considered in spectrum
  const  static double NSIGMA = 3.5;      // #_pSigma > noise for peak search threshold
  const static  double DEFAULT_ZOOM=1e4;  // default (minimum) zoom [ns]
  #else 
  constexpr static  Int_t MAXPEAKS = 100000;  // max peaks considered in spectrum
  constexpr  static double NSIGMA = 3.5;      // #_pSigma > noise for peak search threshold
  constexpr static  double DEFAULT_ZOOM=1e4;  // default (minimum) zoom [ns]
  #endif


  PSbuffer *psbuffer; // pico scope data buffer
  
  // Analysis Histograms
  shared_ptr<TH1F>  _hdt;   // delta time between peaks
  shared_ptr<TH1F>  _hph;   // spectrum of peak heights
  shared_ptr<TH1F>  _hpi;   // spectrum of peak integrals
  shared_ptr<TH1F>  _hprms; // RMS width of pulses

  // Parameters for the current spectrum
  Double_t _pMax; 
  Double_t _pMin; 
  Double_t _dT; 
  Int_t _nbins;
  Double_t _xmin, _xmax;
  
  // simple baseline and noise model
  Double_t _basePar[2];

  // Peak finder variables
  Double_t _pThreshold;    // threshold voltage
  Double_t _pWidth;        // width of pulse in nanoseconds
  Double_t _pSigma;        // half width of pulse in units of bins

  // Analysis of found peaks
  Int_t _pNFound;          // peaks found by TSpectrum
  Double_t _pulseRate;     
  Double_t _pInteg[MAXPEAKS];  // integrals of pulse areas
  Double_t _pRMS[MAXPEAKS];    // RMS width of pulses

public:

  //  PulseAnalysis(TString fName=""); 
  PulseAnalysis(PSbuffer *buffer);
  virtual ~PulseAnalysis(); 

  void Clear();        // clear internal analysis data
  void AnaClean();     // clear peak analysis histograms
  void Reset();        // reset analysis for current spectrum

  // Analysis tools
  void Analyze();
  TString FindPeaks(bool nodraw=false);
  void ScanPeaksFast(int nsteps, double *thresholds, double *count) const;
  int CountPeaksFast(double threshold=0) const;
  void FindPeaksandReduce(Float_t window); 


  // February 03, 2015 To Do: Remove the drawing functionality, most of this needs to get moved to the gui 

  void SmoothHistogram(); 
  void SubBkg(); // tbd

  // manipulate spectrum view
  void DrawSpectrum();
  void ZoomIn(); 
  void UnZoom(); 
  void ZoomOut(); 
  void leftShift(); 
  void rightShift();

  // Access internal data
  void DumpPeaks();
  void Print();
  void WriteHists();

  void GetBkg(); // tbd
  Double_t GetPulseRate() { return _pulseRate; }
  Double_t GetPulseWidth() { return _pWidth; }
  //  TString GetSpectName() { if (_tf) return _tf->GetName(); else return "none";}
  Double_t GetThreshold() { return _pThreshold;}
 


  shared_ptr<TH1F> Hdt() {return _hdt;}
  shared_ptr<TH1F> Hph() {return _hph;}
  shared_ptr<TH1F> Hpi() {return _hpi;}
  shared_ptr<TH1F> Hprms() {return _hprms;}


  TH1F* R_Hdt() {return _hdt.get();}
  TH1F* R_Hph() {return _hph.get();}
  TH1F* R_Hpi() {return _hpi.get();}
  TH1F* R_Hprms() {return _hprms.get();}

  // various setters
  void SetResponse();
  void SetThreshold(Float_t t);
  TString SetWidth(Float_t w);
  void SetXMarks();
  void SetYMarks();

}; 

#endif
