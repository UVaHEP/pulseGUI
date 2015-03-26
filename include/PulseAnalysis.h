/* Analysis object: contains all analysis code, may be used w/o GUI */
#ifndef PULSEANALYSIS_H
#define PULSEANALYSIS_H


#include "TFile.h"
#include "TH1F.h"


#include "PSbuffer.h"

class PulseAnalysis {

 private: 
  // c++11 uses constexpr for calculated values in classes rather than const static
  // CINT doesn't support constexpr so this is a quick
  // hack to work around the incompatibility 
#if (__cplusplus <= 199711L) || defined (__CINT__)
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
  TH1F  _hdt;   // delta time between peaks
  TH1F  _hph;   // spectrum of peak heights
  TH1F  _hpi;   // spectrum of peak integrals
  TH1F  _hprms; // RMS width of pulses

  // simple baseline and noise model
  Double_t _basePar[2];

  // Peak finder variables
  Double_t _pThreshold;    /// threshold voltage
  Double_t _pWidth;        /// width of pulse in nanoseconds
  Double_t _pSigma;        /// half width of pulse in units of bins
  // Fixed Integration Window
  Int_t _intWindow[2];  /// define bins for fixed integration window
  
  // Analysis of found peaks
  Int_t _pNFound;          /// peaks found by TSpectrum
  Double_t _pulseRate;     /// in MHz 
  Double_t _pInteg[MAXPEAKS];  /// integrals of pulse areas
  Double_t _pRMS[MAXPEAKS];    /// RMS width of pulses

public:
  PulseAnalysis(); 
  PulseAnalysis(PSbuffer *buffer);
  void Init();
  virtual ~PulseAnalysis(); 

  void Clear();        /// clear internal analysis data
  void AnaClean();     /// clear peak analysis histograms
  void Reset();        /// reset analysis for current spectrum
  void SetBuffer(PSbuffer *buffer);  /// 
  
  // Analysis tools
  void Analyze();
  TString FindPeaks(bool nodraw=false);
  void ScanPeaksFast(int nsteps, double *thresholds, double *count) const;
  int CountPeaksFast(double threshold=0) const;
  void FindPeaksandReduce(Float_t window); 
  void SmoothHistogram(); 
  void SubBkg(); // tbd
  void SetIntegrationWindow(Double_t xmin, Double_t xmax);
  void SetWindow(Int_t imin, Int_t imax) {
    _intWindow[0]=imin;
    _intWindow[1]=imax; }
  void SetIntegrationFromTrigger(Double_t width, Double_t offset=0);
  Double_t FixedIntegral() const;
  
  // manipulate spectrum view
  void DrawSpectrum(TString options="");
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
  Double_t GetThreshold() { return _pThreshold;}
 
  // derived histograms
  TH1F* Hdt() {return &_hdt;}
  TH1F* Hph() {return &_hph;}
  TH1F* Hpi() {return &_hpi;}
  TH1F* Hprms() {return &_hprms;}

  // various setters
  void SetResponse();
  void SetThreshold(Float_t t);
  TString SetWidth(Float_t w);
  void SetXMarks();
  void SetYMarks();

  void Dump() const;  // Print information about this instance
}; 

#endif
