// simple fourier transform code

#ifndef FOURIER_h
#define FOURIER_H

#include "TH1F.h"

// do a simple fourier transform
// h:      histogram to operate on
// fmax:   max "frequency" in units of hbins 0 << max freq < nbins-1
// r,i:    normalizations vs. frequency
// size of r,i arrays must be >= fmax

void fourier(const TH1F *h, double *r, double *i=0, int fmax=-1);

// calculate DC offset based on 0th Fourrier component
double calcDCoffset(const TH1F *h);

#endif
