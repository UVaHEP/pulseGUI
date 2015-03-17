#include "TFile.h"
#include "TList.h"
#include "TCollection.h"
#include "TObject.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "PSbuffer.h"
#include "TApplication.h"
#include "TString.h"
#include <iostream>
#include <sys/stat.h>
#include <ftw.h>
#include <fnmatch.h>

using namespace std;


void wavePlayer(TString fdat){
  TFile *f = new TFile(fdat);
  if (f->IsZombie()){
    cout << "Cannot open file: " << fdat << endl;
    return;
  }
  cout << "Viewing file: "<< fdat << endl;

  TList *tl=(TList*)f->Get("PSbufferlist");

  TIter next(tl);
  TObject* object = 0;
  TCanvas *c=new TCanvas();
  gStyle->SetOptStat(0);
  c->Draw();
  PSbuffer *psb;
  while ((object = next())) {
    psb = (PSbuffer*)object;
    psb->Draw("trigs");
    psb->Print();
    c->Update();
    gSystem->Sleep(1000);
  }
}

int main(int argc, char **argv) {
  if (argc<2) {
    cout << "Usage: wavePlayer file.root"
	 << endl;
    return 1;
  }
  TString fin=argv[1];
  
  TApplication theApp("App", &argc, argv);

  wavePlayer(fin);
  
  
  cout << "To exit, quit ROOT from the File menu of the GUI (or use control-C)" << endl;
  theApp.Run(true);
  return 0;
}

