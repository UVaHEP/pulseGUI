#include "dbg.h"
#include "PicoReader.h"
#include "psdata.h"
#include "PSbuffer.h"

#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TRegexp.h"

#include <iostream>
#include <vector>

using TMath::Abs;
using TMath::Max;
using TMath::Min;
using std::cout;
using std::cerr;
using std::endl;
using std::ios;
using std::vector;


void psblock::Print(){
  std::cout << "Block.name: " << this->name 
	    << ", start: " << this->dataStart
	    << ", values: " << this->nValues  
	    << ", datatype: " << this->type << std::endl;
}


//----------------------------------------------------------------------
// TMATLAB Files 
//----------------------------------------------------------------------

int PicoReader::ConvertMatFile(TString matfile, TString rootfile) { 
  debug("Converting %s",matfile.Data() );

  if (! matfile.EndsWith(".mat") ) {
    cout << "PicoReader::Convert : Unknown file type: "<<matfile<<endl;
    return 1;
  }
  cout << "Converting PicoScope data to ROOT file: " << matfile << endl;

  // slighty dangerous assumption about file name conventions
  if (rootfile=="") {
    rootfile=matfile;
    rootfile.ReplaceAll(".mat",".root");
  }

  TFile f(rootfile, "RECREATE"); 

  // new format
  PSbuffer *psb=ReadMatFile(matfile);
  TFile f2(rootfile, "RECREATE"); 
  psb->Write();
  f2.Write(); 
  f2.Close();
  return 0;
}



//----------------------------------------------------------------------
// Utility Functions and Methods 
//----------------------------------------------------------------------

//Really simple helper functions for reading a float or double value
template <class T>
void readNext(fstream &fin, T &channelData) { 
  char data[sizeof(T)]; 
  fin.read(data, sizeof(T)); 
  memcpy(&channelData, data, sizeof(T)); 
  memset(data, 0, sizeof(T)); 
}



// replace me w/ readNext call!
Double_t getPSDouble(psblock block, fstream &fin){
  Double_t val;
  fin.seekg(block.dataStart, fin.beg);
  val=0.0;
  readNext(fin, val);
  return val;
}


bool PicoReader::LocateBlock(vector<psblock> &blocks, 
			     TString name, psblock &block) {

  for (std::vector<psblock>::iterator it = blocks.begin(); it != blocks.end(); it++) { 
    if (it->name == name) { 
      block = *it; 
      return true; 
    }
  }
  return false; 
}



// get listing of blocks in file
vector<psblock> PicoReader::ReadBlocks(fstream& fin) { 

  vector<struct psblock> blocks;
  fin.seekg(0, fin.end); 
  unsigned int end = fin.tellg(); 
  fin.seekg(0, fin.beg); 

  char headerVals[header_sz] = {0}; 
  char name[name_limit] = {0}; 

  debug("Starting to search for headers, Filesize: 0x%x",(unsigned)end);

  while(fin.tellg() < end) { 
    psblock temp; 
    debug("Current position: 0x%x",(unsigned)fin.tellg());
    memset(headerVals, 0, header_sz); 
    memset(name, 0, name_limit); 

    fin.read(headerVals, header_sz); 

    // First we check for a good header 
    UInt_t headerVal = 0; 
    headerVal = headerVals[11] & (0xFF); 
    headerVal <<= 8; 
    headerVal |= headerVals[10] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[9] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[8] & (0xFF);

    if (headerVal != 0x1)  {
      std::cout << "Bad Block Header! Position:" << fin.tellg() << std::endl; 
      blocks.clear(); 
      return blocks; 
    }

    headerVal = headerVals[15] & (0xFF); 
    headerVal <<= 8; 
    headerVal |= headerVals[14] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[13] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[12] & (0xFF);
    
    if (headerVal != 0x0) { 
      std::cout << "Bad Block Header! Position:" << fin.tellg() << std::endl; 
      blocks.clear(); 
      return blocks; 
    } 
    //Get the dataType of this block 
    //0x0 -- Double 8-byte floating point value 
    //0x10 -- Single 4-byte floating point value 
    //0x20 -- 4-byte integer 
    headerVal = headerVals[3] & (0xFF); 
    headerVal <<= 8; 
    headerVal |= headerVals[2] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[1] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[0] & (0xFF);
    temp.type = (MatLabType) headerVal; 

    //Get the number of values in this data block 
    headerVal = headerVals[7] & (0xFF); 
    headerVal <<= 8; 
    headerVal |= headerVals[6] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[5] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[4] & (0xFF);
    temp.nValues = headerVal; 


    // Now we'll grab the size of the Data Block name and grab the name itself
    headerVal = headerVals[19] & (0xFF); 
    headerVal <<= 8; 
    headerVal |= headerVals[18] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[17] & (0xFF);
    headerVal <<= 8; 
    headerVal |= headerVals[16] & (0xFF);
    

    fin.read(name, headerVal); 
    temp.name = TString(name);
    cout << "TString(name): " << temp.name << endl;
    temp.dataStart =fin.tellg(); 
#ifdef DODEBUG
    temp.Print();
#endif

    UInt_t nextBlock = 0; 
    switch(temp.type) { 
    case DOUBLE: {
      nextBlock += sizeof(Double_t)*temp.nValues; 
      break; 
    }
    case FLOAT: {
      nextBlock += sizeof(Float_t)*temp.nValues; 
      break; 
    }
    case INT: {
      nextBlock += sizeof(Int_t)*temp.nValues; 
      break; 
    }
    }
    debug("The next block starts at: 0x%x",(unsigned)nextBlock); 
    fin.seekg(nextBlock, fin.cur); 
    blocks.push_back(temp);
    blocks[blocks.size()-1].Print();
  }    

  return blocks; 
}


PSbuffer* PicoReader::ReadMatFile(TString& filename){
  fstream fin; 
  fin.open(filename.Data(), ios::binary | ios::in); 
  if (!fin.is_open()) {
    cerr << "Open error: " << filename << endl;
    return NULL;
  }
  vector<psblock> blocks = ReadBlocks(fin); 
  PSbuffer *ps = new PSbuffer();
  //  PSbuffer *ps = new PSbuffer(); 
  
  // get time data
  psblock block;
  if (LocateBlock(blocks, "Tstart",block)){
    debug("Reading: Tstart Block");
    double t0=getPSDouble(block,fin)*1e3;  // convert to ns
    debug("t0: %20e ns",t0);
    ps->SetT0(t0);
  }


  if (LocateBlock(blocks, "Tinterval",block)){
    debug("Reading: Tinterval Block");
    double dT=getPSDouble(block,fin)*1e9;  // convert to ns
    debug("dt: %20e ns",dT);
    ps->SetDt(dT);
  }
  else {
    log_err("Tinterval block not found");
  }

  if (!(LocateBlock(blocks, "A", block))){
    cerr << "Waveform data block not found on Channel A" << endl;
    return ps;
  }  

  // get waveform from channel A
  
  fin.seekg(block.dataStart, fin.beg); 
  std::cout << "Reading: Voltage Channel" << std::endl; 
  block.Print();
  ps->InitWaveform(block.nValues);
  TH1F* wave= ps->GetWaveform(); 
  //  TH1F *wave=ps->R_GetWaveform();

  Float_t channelData = 0.0; 
  Float_t dV=1e12;  

  float last=0;  
  for (UInt_t i = 0; i < block.nValues; i++) { 
    readNext(fin, channelData); 
    channelData*=1000;  // convert to mV
    wave->SetBinContent(i+1,channelData);
    // find minimal voltage step
    float delta=Abs(channelData-last);
    if ( delta>1e-6 && delta<dV ) dV=delta;
    last=channelData;
  }
  ps->SetDV(dV);
  debug("Finished reading voltage channel data."); 
    

  // get channel B (trigger)
  float min=1e12;
  float max=-1e12;
  vector<float> *vtrig=new vector<float>;
  if (LocateBlock(blocks, "B", block)){
    fin.seekg(block.dataStart, fin.beg);
    // temporarily store trigger data
    for (UInt_t i = 0; i < block.nValues; i++) { 
      readNext(fin, channelData); 
      vtrig->push_back(channelData);
      min=Min(min,channelData);
      max=Max(max,channelData);
    }
    
    // set trigger point and hysterisis
    // WARNING!  This may fail in case of bad terminations
    // *** TODO:  Use t0 vs t=0  to find 1st trigger bin and gte threshold there ***
    Float_t onThreshold = max*0.5; 
    Float_t offThreshold = max*0.3; 
    std::cout << "On Threshold:"   << onThreshold 
	      << " Off Threshold:" << offThreshold << std::endl;

    Bool_t fired = false;
    // mark triggers
    for (UInt_t i = 0; i < block.nValues; i++) {
      if (!fired && vtrig->at(i)>onThreshold){
	fired = true;
	ps->AddTrig(i);
	continue;
      }
    }
    fired = !(channelData < offThreshold); 
  }

  ps->Analyze();  // calculate DC offset, frequency spectrum, noise, etc
  ps->Print();
  return ps;  // picoscope data structure
}



