#include "ReadMat.h"
#include "dbg.h"
#include "TMath.h"
#include "TFile.h"
#include "TH1F.h"

#include <iostream>
using std::cout;
using std::endl;
using std::ios;

//--------------------Begin Binary Processing Code, Split into a separate file 


void psblock::Print(){
  std::cout << "Block.name: " << this->name 
	    << ", start: " << this->dataStart
	    << ", values: " << this->nValues << " values." 
	    << ", datatype: " << this->type << std::endl;
}

// get listing of blocks in file
vector<psblock> readBlocks(fstream& fin) { 

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
  }    

  return blocks; 
}

//Really simple helper functions for reading a float or double value
template <class T>
void readNext(fstream &fin, T &channelData) { 
  char data[sizeof(T)]; 
  fin.read(data, sizeof(T)); 
  memcpy(&channelData, data, sizeof(T)); 
  memset(data, 0, sizeof(T)); 
}


bool locateBlock(vector<psblock> &blocks, TString name, psblock &block) {
  for (std::vector<psblock>::iterator it = blocks.begin(); it < blocks.end(); ++it) { 
    if (it->name==name) {
      block=*it;
      return true;
    }
  }
  return false;
}

// replace me w/ readNext call!
Double_t getPSDouble(psblock block, fstream &fin){
  Double_t val;
  fin.seekg(block.dataStart, fin.beg);
  val=0.0;
  readNext(fin, val);
  return val;
}

psdata * readMatFile(TString& filename) { 
  fstream fin; 
  fin.open(filename.Data(), ios::binary | ios::in); 
  if (!fin.is_open()) 
    return NULL; 
 
  psdata *ps = new psdata(); 
  
  vector<psblock> blocks = readBlocks(fin); 

  // get time data
  psblock block;
  if (locateBlock(blocks, "Tstart",block)){
    debug("Reading: Tstart Block");
    double t0=getPSDouble(block,fin)*1e3; // convert to ns
    debug("t0: %20e ns",t0);
    ps->t0 = new TH1D("T0", "T0",1,-1,1);
    ps->t0->Fill(0.0, t0);
  }
  if (locateBlock(blocks, "Tinterval",block)){
    debug("Reading: Tinterval Block");
    double dT=getPSDouble(block,fin)*1e9;  // convert to ns
    debug("dt: %20e ns",dT);
    ps->dT = new TH1D("dT", "dT",1,-1,1);
    ps->dT->Fill(0.0, dT);
  }
  
  // get channel A
  if (locateBlock(blocks, "A", block)){
    fin.seekg(block.dataStart, fin.beg); 
    std::cout << "Reading: Voltage Channel" << std::endl; 
    block.Print();
    ps->volts = new TArrayF(block.nValues); 
    Float_t channelData = 0.0; 
    Float_t min = 1e6; 
    Float_t max = -1e6; 
    Float_t dV=1e12;
    for (UInt_t i = 0; i < block.nValues; i++) { 
      readNext(fin, channelData); 
      channelData*=1000;  // convert to mV
      ps->volts->AddAt(channelData, i); 
      if (channelData < min) min = channelData; 
      else if (channelData > max) max = channelData; 
      // a bit of a hack to find voltage steps, assumes we cross zero...
      if (TMath::Abs(channelData)>0 && TMath::Abs(channelData)<dV) 
	dV=TMath::Abs(channelData);
    }
    debug("Finished reading voltage channel data."); 
    // majority sign
    ps->sign = new TH1F("sign","sign",1,-1,1); 
    int sign=-1;
    if (TMath::Abs(max)>TMath::Abs(min)) sign=1;
    ps->sign->Fill(0.0,sign); // majority count of +/-signs
    
    // voltage ranges and resolution
    ps->vMax = new TH1F("vMax","vMax",1,-1,1); 
    ps->vMax->Fill(0.0,max); 
    ps->vMin = new TH1F("vMin","vMin",1,-1,1); 
    ps->vMin->Fill(0.0,min); 
    ps->dV = new TH1F("dV","dV",1,-1,1); 
    ps->dV->Fill(0.0,dV);
  }

  // get channel B (trigger)
  if (locateBlock(blocks, "B", block)){
    fin.seekg(block.dataStart, fin.beg);
    ps->trigger = new TArrayC(block.nValues); 

    //scan block looking for vMax and vMin
    Float_t channelData = 0.0;
    Float_t min = 0.0; 
    Float_t max = 0.0; 
    for (UInt_t i = 0; i < block.nValues; i++) { 
      readNext(fin, channelData); 
      if (channelData < min) min = channelData; 
      else if (channelData > max) max = channelData; 
    }

    // set trigger point and hysterisis
    Float_t onThreshold = max*0.5; 
    Float_t offThreshold = max*0.4; 
    std::cout << "On Threshold:"   << onThreshold 
	      << " Off Threshold:" << offThreshold << std::endl;

    Bool_t fired = false; 
    // mark triggers
    fin.seekg(block.dataStart, fin.beg); 
    for (UInt_t i = 0; i < block.nValues; i++) { 
      readNext(fin, channelData); 
      if (channelData > onThreshold && !fired) { 
	fired = true; 
	ps->trigger->AddAt(true, i); 
	continue;
      }
      else if (channelData < offThreshold && fired) {
	fired = false; 
	ps->trigger->AddAt(true, i); 
	continue; 
      }
      ps->trigger->AddAt(false, i);
    }
  }

  return ps;  // picoscope data structure
}


//----------------------------------------------------------------------
void ConvertMatFile(TString &matfile, TString &rootfile) { 
  debug("Converting %s",matfile.Data() );
  TFile f(rootfile, "RECREATE"); 
  psdata *data = readMatFile(matfile); 
  debug("Finished Reading .mat File"); 
  f.WriteObject(data->volts, "volts"); 
  if (data->trigger) f.WriteObject(data->trigger, "trigger"); 

  f.Write(); 
  f.Close(); 
  debug("Finished Writing new TFile."); 
}
