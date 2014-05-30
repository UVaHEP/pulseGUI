#include "TMath.h"
#include "TFile.h"
#include "TH1F.h"
#include "TNtuple.h"

TArrayF* gatherArray(TFile *f, TString name) { 

  TArrayF *array = (TArrayF *)f->Get(name); 
  return array; 

}


Int_t joinArrays(TArrayF *sink, TArrayF *source, Int_t pos, TArrayF *minmax) { 
  
  for (Int_t i = 0; i < source->GetSize(); i++) { 
    Float_t c = source->At(i); 
    if (c != TMath::Infinity()) {
      if (c > minmax->At(1))
	minmax->AddAt(c,1); 
      if (c < minmax->At(0)) 
	minmax->AddAt(c,0); 
      sink->AddAt(c, pos); 
      pos++; 
    }

  }

  return pos;

}


TArrayF * findMinandMax(TArrayF *array)  {

  Float_t min = 0.0, max = 0.0; 
  for (Int_t i = 0; i < array->GetSize(); i++) { 
    Float_t c = array->At(i); 
    if (c != TMath::Infinity()) {
      if (c < min)
	min = c; 
      if (c > max)
	max = c; 
    }
  }
  TArrayF *retVal = new TArrayF(2); 
  retVal->AddAt(min, 0); 
  retVal->AddAt(max, 1); 
  return retVal; 

}
