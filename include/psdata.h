// Information structure for picoscope data blocks

#ifndef PSDATA_H
#define PSDATA_H

#include "dbg.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TArrayF.h"
#include "TArrayC.h"
#include "TFile.h"
#include <iostream>
using std::cout;
using std::endl;

// to do: better to turn this into a class
// add Read/Write methods to save arrays and read in all pointers from TFile

class psdata { 
 public:
  TH1D *dT; 
  TH1D *t0; 
  TH1F *vMax; 
  TH1F *vMin; 
  TH1F *dV;
  TH1F *dcOffset;
  TH1F *sign;  // primary sign of data +1: >0, -1<0
  TH1F *pHD;   // pulse height distribution
  TArrayF *volts; 
  TArrayC *trigger; 

  bool checkPSData(psdata *ps) {
    if (ps == NULL || ps->dV == NULL || ps->dT == NULL || ps->t0 == NULL
	|| ps->vMax == NULL || ps->vMin == NULL || ps->volts == NULL
	|| ps->trigger == NULL) {
      return false;
    }
    return true;
  }
};


#endif
