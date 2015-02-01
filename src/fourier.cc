#include "fourier.h"

#include "TMath.h"

#include <iostream>

using TMath::Pi;
using std::cerr;
using std::endl;

void fourier(const TH1F *h, double *r, double *i, int fmax){
  if (fmax >= h->GetNbinsX()) {
    cerr << __FILE__ << " : Max frequency reduced to Nbins-1"  << endl;
  }
  double imag, real;
  int nbins=h->GetNbinsX();
  for (int fidx=0; fidx<nbins; fidx++){  // loop for frequency index
    real=imag=0.0;
    for (int k=0; k<nbins; k++){    // loop for sums
      real+=h->GetBinContent(k+1)*cos((2*Pi()*k*fidx)/nbins);
      imag+=h->GetBinContent(k+1)*sin((2*Pi()*k*fidx)/nbins);
    }
    if (r) r[fidx]=real/nbins;
    if (i) i[fidx]=imag/nbins;
    if (fidx==fmax) return;
  }
}

double calcDCoffset(const TH1F *h){
  double offset;
  fourier(h, &offset, 0, 0);
  return offset;
}
