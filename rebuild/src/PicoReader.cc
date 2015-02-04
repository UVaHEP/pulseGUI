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
	    << ", values: " << this->nValues << " values." 
	    << ", datatype: " << this->type << std::endl;
}


int PicoReader::Convert(TString infile, TString rootfile){
  bool txtFile=infile.EndsWith(".txt") || infile.EndsWith(".csv");
  bool matFile=infile.EndsWith(".mat");

  if (!txtFile && !matFile) {
    cout << "PicoReader::Convert : Unknown file type: "<<infile<<endl;
    return 1;
  }
  cout << "Converting PicoScope data to ROOT file: " << infile << endl;

  // slighty dangerous assumption about file name conventions
  if (rootfile=="") {
    rootfile=infile;
    rootfile.ReplaceAll(".txt",".root");
    rootfile.ReplaceAll(".csv",".root");
    rootfile.ReplaceAll(".mat",".root");
  }

  if (txtFile) return ConvertTXTFile(infile,rootfile);
  else 
    return ConvertMatFile(infile,rootfile);
}

//----------------------------------------------------------------------
// TMATLAB Files 
//----------------------------------------------------------------------

int PicoReader::ConvertMatFile(TString matfile, TString rootfile) { 
  debug("Converting %s",matfile.Data() );
  TFile f(rootfile, "RECREATE"); 
  psdata *data = ReadMatFile(matfile); 
  debug("Finished Reading .mat File"); 
  f.WriteObject(data->volts, "volts"); 
  if (data->trigger) f.WriteObject(data->trigger, "trigger"); 

  f.Write(); 
  f.Close(); 
  debug("Finished Writing new TFile."); 
  rootfile.ReplaceAll(".root","_2.root");


  PSbuffer *psb=ReadMatFile2(matfile);
  TFile f2(rootfile, "RECREATE"); 
  psb->Write();
  f2.Write(); 
  f2.Close();
  return 0;
}



//----------------------------------------------------------------------
// TXT File 
//----------------------------------------------------------------------
int PicoReader::ConvertTXTFile(TString datfile, TString rootfile) { 
  std::cout << "Converting a CVS file" << std::endl; 
  TFile f(rootfile, "RECREATE"); 
  
  std::ifstream in;
  char delim[]=" ";
  in.open(datfile.Data(), std::ifstream::in);
  if (!in.good()){
    // Add Error function
    printf("Cannot open file: %s", datfile.Data());
    return 1;
  }
  const char *ext = strrchr(datfile.Data(),'.');
  if(ext != NULL && ((strcmp(ext, ".csv") == 0) || (strcmp(ext, ".CSV") == 0))) delim[0]=',';

  //Only accept lines that START with a number.
  TString regexp ("^[-0-9]"); 
  TRegexp cleaner(regexp.Data()); 

  const int lineLength = 100; 
  char buffer[lineLength]; 
  Ssiz_t pos; 

  TString tmp; 
  
  // Unfortunately, TArray does not resize like a normal vector
  // so ehre we figure out the size
  UInt_t nlines=0;
  while (in.good()) {
    if (in.getline(buffer, lineLength)) nlines++;
  }  
  in.close();

  // skipping the header
  in.open(datfile.Data(), std::ifstream::in);
  UInt_t nheader=0;
  while (in.good()) {
    //fill buffer with data from current line 
    in.getline(buffer, lineLength);
    tmp = buffer;
    pos = TString(buffer).Length();
    //Any line beginning with a letter is skipped.
    if (cleaner.Index(TString(buffer), &pos, 0) == -1) {
      std::cout << "Skipping:" << tmp << std::endl; 
      nheader++;
      tmp = in.peek(); // Peek at the next charecter without extracting.
      if (cleaner.Index(TString(tmp), &pos, 0) > -1) {
	break;
      }   //Next line starts new data
    }
    else break;
  }
  cout << "# of Data values: " << nlines-nheader << endl;

  // read the data
  psdata *ps = new psdata(); 
  ps->volts = new TArrayF(nlines-nheader);

  TString tok;
  Double_t t0=0, dT=0;
  Float_t min = 0.0; 
  Float_t max = 0.0; 
  Float_t dV=1e12;
  UInt_t count=0;
  while (in.good()) {
    //fill buffer with data from current line 
    in.getline(buffer, lineLength);
    tmp = buffer;
    if (tmp.Length()==0) break;
    // incomplete: considering two tokens only, Time, Voltage
    pos=0;
    tmp.Tokenize(tok, pos, delim);    
    Float_t t=tok.Atof()*1000;  // convert to ns
    if (count==0) t0=t;
    if (count==1) dT=t-t0;
    tmp.Tokenize(tok, pos, delim);    
    Float_t v=tok.Atof();     // already in mV
    ps->volts->AddAt(v, count);
    if (v < min) min = v; 
    else if (v > max) max = v; 
    // a bit of a hack to find voltage steps, assumes we cross zero...
    if (TMath::Abs(v)>0 && TMath::Abs(v)<dV) dV=TMath::Abs(v);
    
    count++;
    
    // add later maybe: fill trig info, if third column is present
  }
  // majority sign
  ps->sign = new TH1F("sign","sign",1,-1,1); 
  int sign=-1;
  if (TMath::Abs(max)>TMath::Abs(min)) sign=1;
  ps->sign->Fill(0.0,sign); // majority count of +/-signs

  // time info
  ps->t0 = new TH1D("T0", "T0",1,-1,1);
  ps->t0->Fill(0.0, t0);
  ps->dT = new TH1D("dT", "dT",1,-1,1);
  ps->dT->Fill(0.0, dT); 

  // voltage ranges and resolution
  ps->vMax = new TH1F("vMax","vMax",1,-1,1); 
  ps->vMax->Fill(0.0,max); 
  ps->vMin = new TH1F("vMin","vMin",1,-1,1); 
  ps->vMin->Fill(0.0,min); 
  ps->dV = new TH1F("dV","dV",1,-1,1); 
  ps->dV->Fill(0.0,dV);

  f.WriteObject(ps->volts, "volts"); 
  f.Write(); 
  f.Close();

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
  for (auto &b : blocks) { 
    if (b.name == name) { 
      block = b; 
      return true; 
    }
  }
  return false; 

}


psdata* PicoReader::ReadMatFile(TString& filename){
  fstream fin; 
  fin.open(filename.Data(), ios::binary | ios::in); 
  if (!fin.is_open()) 
    return NULL; 
 
  psdata *ps = new psdata(); 
  
  vector<psblock> blocks = ReadBlocks(fin); 

  // get time data
  psblock block;
  if (LocateBlock(blocks, "Tstart",block)){
    debug("Reading: Tstart Block");
    double t0=getPSDouble(block,fin)*1e3; // convert to ns
    debug("t0: %20e ns",t0);
    ps->t0 = new TH1D("T0", "T0",1,-1,1);
    ps->t0->Fill(0.0, t0);
  }
  if (LocateBlock(blocks, "Tinterval",block)){
    debug("Reading: Tinterval Block");
    double dT=getPSDouble(block,fin)*1e9;  // convert to ns
    debug("dt: %20e ns",dT);
    ps->dT = new TH1D("dT", "dT",1,-1,1);
    ps->dT->Fill(0.0, dT);
  }
  
  // get channel A
  if (LocateBlock(blocks, "A", block)){
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
  if (LocateBlock(blocks, "B", block)){
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

PSbuffer* PicoReader::ReadMatFile2(TString& filename){
  fstream fin; 
  fin.open(filename.Data(), ios::binary | ios::in); 
  if (!fin.is_open()) {
    cerr << "Open error: " << filename << endl;
    return NULL;
  }
  vector<psblock> blocks = ReadBlocks(fin); 
  shared_ptr<PSbuffer> ps(new PSbuffer());
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

  if (!(LocateBlock(blocks, "A", block))){
    cerr << "Waveform data block not found on Channel A" << endl;
    return ps.get();
  }  

  // get waveform from channel A
  
  fin.seekg(block.dataStart, fin.beg); 
  std::cout << "Reading: Voltage Channel" << std::endl; 
  block.Print();
  ps->InitWaveform(block.nValues);
  shared_ptr<TH1F> wave(ps->GetWaveform()); 
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
  return ps.get();  // picoscope data structure
}



