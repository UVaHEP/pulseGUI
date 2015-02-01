/* Analysis object: contains all analysis code, may be used w/o GUI */
#ifndef PULSEANALYSIS_H
#define PULSEANALYSIS_H


#include "TFile.h"
#include "TH1F.h"
#include "PSbuffer.h"

class PulseAnalysis {

 private: 
  static const Int_t MAXPEAKS = 100000;  // max peaks considered in spectrum
  static const double NSIGMA = 3.5;      // #_pSigma > noise for peak search threshold
  static const double DEFAULT_ZOOM=1e4;  // default (minimum) zoom [ns]

  TFile *_tf;       // pointer to current data file
  TString _currentTFile;  // name of current TFile
  TH1F *_hspect;    // Histogram with pulse data spectrum
  TH1F *_hfreq;     // pulse data frequency 

  PSbuffer *psbuffer; // pico scope data buffer
  
  // Analysis Histograms
  TH1F  *_hdt;   // delta time between peaks
  TH1F  *_hph;   // spectrum of peak heights
  TH1F  *_hpi;   // spectrum of peak integrals
  TH1F  *_hprms; // RMS width of pulses
  TH1   *_hb;    // pointer to baseline histogram (not implemented)

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

  PulseAnalysis(TString fName=""); 
  PulseAnalysis(PSbuffer *buffer);
  virtual ~PulseAnalysis(); 

  void ConvertFile(TString Filename); 
  void Clear();        // clear internal analysis data
  void AnaClean();     // clear peak analysis histograms
  void Reset();        // reset analysis for current spectrum
  void LoadSpectrum(TString Filename);
  void LoadSpectrum();
  void LoadBuffer(TString Filename);

  // Analysis tools
  void Analyze();
  TString FindPeaks(bool nodraw=false);
  void ScanPeaksFast(int nsteps, double *thresholds, double *count) const;
  int CountPeaksFast(double threshold=0) const;
  void FindPeaksandReduce(Float_t window); 

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
  TString GetSpectName() { if (_tf) return _tf->GetName(); else return "none";}
  Double_t GetThreshold() { return _pThreshold;}
 

  TH1F* Hfreq() {return _hfreq;}
  TH1F* Hspect() {return _hspect;}
  TH1F* Hdt() {return _hdt;}
  TH1F* Hph() {return _hph;}
  TH1F* Hpi() {return _hpi;}
  TH1F* Hprms() {return _hprms;}

  // various setters
  void SetResponse();
  void SetThreshold(Float_t t);
  TString SetWidth(Float_t w);
  void SetXMarks();
  void SetYMarks();

}; 

#endif
