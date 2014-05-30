#include "ReadTxt.h"
#include "TMath.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRegexp.h"
#include "psdata.h"

#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ios;



//----------------------------------------------------------------------
void ConvertTXTFile(TString &datfile, TString &rootfile) { 
  std::cout << "Converting a CVS file" << std::endl; 
  TFile f(rootfile, "RECREATE"); 
  
  std::ifstream in;
  char delim[]=" ";
  in.open(datfile.Data(), std::ifstream::in);
  if (!in.good()){
    // Add Error function
    printf("Cannot open file: %s", datfile.Data());
    return;
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
}

/*
TString tmp="abc, def";
TString tok;
Ssiz_t pos = 0;
char delim[]=",";
tmp.Tokenize(tok,pos,delim);
*/
