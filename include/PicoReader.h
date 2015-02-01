// Class to Read Picoscope data files

#ifndef PICOREADER_H
#define PICOREADER_H

#include "psdata.h"
#include "PSbuffer.h"
#include "TString.h"

#include <fstream> 
#include <vector>
using std::vector;


static const Int_t header_sz = 20; 
static const Int_t name_limit = 20; 
enum  MatLabType { 
  DOUBLE=0x0,
  FLOAT=0xA,
  INT=0x14
};

class psblock { 
 public:
  TString name;
  MatLabType type; 
  UInt_t dataStart; 
  UInt_t nValues; 
  void Print();
};

class PSBlock {
 private:
  fstream &fin;
 public:
 PSBlock(fstream &f) : fin(f) {;}
};

class PicoReader{
 public:
  //  PicoReader(){;}
  int Convert(TString infile, TString rootfile="");
  // PicoScope Binary MATLAB File
  int ConvertMatFile(TString matfile, TString rootfile);

  void Print(){;}
  // PicoScope Text File
  int ConvertTXTFile(TString datfile, TString rootfile);
 private:
  psdata* ReadMatFile(TString& filename);
  PSbuffer* ReadMatFile2(TString& filename);
  bool LocateBlock(vector<psblock> &blocks, TString name, psblock &block);
  vector<psblock> ReadBlocks(fstream& fin);
};

#endif

