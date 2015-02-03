// Information structure for picoscope data blocks

#ifndef PSBUFFER_H
#define PSBUFFER_H

#include "dbg.h"
#include "TH1F.h"
#include "TFile.h"
#include "TObject.h"


#include <vector>
#include <memory>
#ifndef __CINT__ 
using std::shared_ptr; 
#else 
template<class T> class shared_ptr; 
#endif 


using std::vector;



// to do: better to turn this into a class
// add Read/Write methods to save arrays and read in all pointers from TFile

class PSbuffer : public TObject { 

ClassDef(PSbuffer, 1); 
private: 
  shared_ptr<TH1F> waveBuffer; 
  vector<Int_t> trigs;  /// bins corresponding to triggers
  Double_t t0;
  Double_t dT;
  Double_t dV;
  Double_t dcOffset;
  Double_t noise;  /// gaussian fit to ~+- 2 sigma around 0
  shared_ptr<TH1F> pHD; // pulse height distribution

 public:
  Double_t T0() const {return t0;}
  void SetT0(Double_t t) {t0=t;}
  Double_t Dt() const {return dT;}
  void SetDt(Double_t dt) {dT=dt;}
  /// return voltage resolution
  Double_t DV() {return dV;}
  void SetDV(Double_t dv) {dV=dv;}
  Double_t DCoffset() const {return dcOffset;}
  /// default setting time does from 0..dT*nbins
  void InitWaveform(Int_t nbins, Float_t max=0, Float_t min=0);

  /* 
     Rather than eliminating the ability to get a raw pointer
  I'm following the convention of appending an "R_" to each
  getter that returns a raw pointer version of the internal 
  shared pointer
  */

  TH1F* R_GetWaveform() {return waveBuffer.get();}
  TH1F* R_GetSpectrum() {return pHD.get();}

  shared_ptr<TH1F> GetWaveform() {return waveBuffer;}
  shared_ptr<TH1F> GetSpectrum() {return pHD;}

  Int_t GetTrig(Int_t ntrig=0) const; 
  void AddTrig(Int_t trigBin);
  void Analyze();
  void Print();



}; 


#endif
