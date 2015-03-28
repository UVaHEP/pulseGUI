// Information structure for picoscope data blocks

#ifndef PSBUFFER_H
#define PSBUFFER_H

#include "dbg.h"
#include "TH1F.h"
#include "TFile.h"
#include "TObject.h"
#include "TString.h"

#include <vector>

using std::vector;

// to do: better to turn this into a class
// add Read/Write methods to save arrays and read in all pointers from TFile

class PSbuffer : public TObject {
  ClassDef(PSbuffer,1);
 private:
  TH1F *waveBuffer;
  vector<Int_t> trigs;  /// bins corresponding to triggers
  Double_t t0;
  Double_t dT;
  Double_t dV;
  Double_t dcOffset;
  Double_t noise;  /// sigma of gaussian fit to ~+- 2 sigma around 0
  TH1F *pHD;   /// pulse height distribution

 public:
  Double_t T0() const {return t0;}
  /// Return time of first sample
  void SetT0(Double_t t) {t0=t;}
  /// Return time resolution
  Double_t Dt() const {return dT;}
  void SetDt(Double_t dt) {dT=dt;}
  /// Return voltage resolution
  Double_t DV() {return dV;}
  void SetDV(Double_t dv) {dV=dv;}
  Double_t DCoffset() const {return dcOffset;}
  /// Return number of samples in signal trace
  Double_t Samples() const {return waveBuffer->GetNbinsX();}
  Double_t Noise() const {return noise;}
  /// default setting time does from 0..dT*nbins
  void InitWaveform(Int_t nbins, Float_t max=0, Float_t min=0);
  /// Return scope trace 
  TH1F* GetWaveform() {return waveBuffer;}
  /// Return pule height distribution of voltage samples 
  TH1F* GetPulseHeights() {return pHD;}
  Int_t GetNtrig() const {return trigs.size();}
  /// Return time corresponding to nth trigger
  Double_t GetTrigT(Int_t ntrig=0) const; 
  /// Return bin corresponding to nth trigger
  Int_t GetTrigBin(Int_t ntrig=0) const; 
  void AddTrig(Int_t trigBin);
  /// Calculate and store derived quantities from signal and trigger traces
  void AnalyzeOld(Double_t scale=-1);
  void Analyze(Double_t scale=-1);
  /// Draw Options
  /// Pass any standard histogram drawing options
  /// Additional options supported:
  /// TRIGS : draw marks for trigger locations
  void Draw(TString options="");
  void Print();
  /// copy this to psb
  void Copy(PSbuffer& psb);
};


#endif
