// Information structure for picoscope data blocks

#ifndef PSBUFFER_H
#define PSBUFFER_H

#include "dbg.h"
#include "TH1F.h"
#include "TFile.h"
#include "TObject.h"


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
  Double_t noise;  /// gaussian fit to ~+- 2 sigma around 0
  TH1F *pHD;   /// pulse height distribution

 public:
  Double_t T0() const {return t0;}
  void SetT0(Double_t t) {t0=t;}
  Double_t Dt() const {return dT;}
  void SetDt(Double_t dt) {dT=dt;}
  /// return voltage resolution
  Double_t DV() {return dV;}
  void SetDV(Double_t dv) {dV=dv;}
  Double_t DCoffset() const {return dcOffset;}
  Double_t Samples() const {return waveBuffer->GetNbinsX();}
  /// default setting time does from 0..dT*nbins
  void InitWaveform(Int_t nbins, Float_t max=0, Float_t min=0);
  TH1F* GetWaveform() {return waveBuffer;}
  TH1F* GetSpectrum() {return pHD;}
  Int_t GetTrig(Int_t ntrig=0) const; 
  void AddTrig(Int_t trigBin);
  void Analyze();
  void Print();
};


#endif
