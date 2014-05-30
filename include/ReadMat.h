// Interface to matlab data file format

#ifndef READMAT_H
#define READMAT_H

#include "psdata.h"
#include <vector>
#include <fstream> 
#include <iostream>
using std::cout;
using std::endl;
using std::vector;


static const Int_t header_sz = 20; 
static const Int_t name_limit = 20; 


enum  MatLabType { 
  DOUBLE=0x0,
  FLOAT=0xA,
  INT=0x14
}; 

struct psblock { 
  TString name;
  MatLabType type; 
  UInt_t dataStart; 
  UInt_t nValues; 
  void Print();
}; 

/* Note: unless you generate a dictionary for the vector<psblock> 
   you won't be able to use it with CINT, it doesn't matter in the 
   compiled file, it'll work fine here.  Just something to watch out for, 
   also modern compilers usually have return value optimization 
   so returning the vector by value shouldn't cause any 
   performance/memory usage issues
*/
vector<psblock> readBlocks(fstream& fin);

psdata * readMatFile(TString& filename);

void ConvertMatFile(TString &matfile, TString &rootfile);

#endif

