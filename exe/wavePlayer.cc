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
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <ftw.h>
#include <fnmatch.h>

using namespace std;


void wavePlayer(TString fdat, int delay=1000, TString options=""){
  TFile *f = new TFile(fdat);
  if (f->IsZombie()){
    cout << "Cannot open file: " << fdat << endl;
    return;
  }
  cout << "Viewing file: "<< fdat << endl;

  // parse options
  options.ToLower();
  bool doSumOnly=options.Contains("sumonly");
  bool doSum=options.Contains("sum");
  if (doSumOnly) options.ReplaceAll("sumonly","");
  if (doSum) options.ReplaceAll("sum","");

  // get buffer list
  TList *tl=(TList*)f->Get("PSbufferlist");
  if (!tl) {
    cout << "No buffer list found" << endl;
    return;
  }

  // setup drawing stuff
  TCanvas *c=new TCanvas();
  gStyle->SetOptStat(0);
  c->Draw();
  TH1F *h=0;
  TString title;
  
  TIter next(tl);
  TObject* object = 0;
  PSbuffer *psb;
  int count=0;

  // iterate over list
  while ((object = next())) {
    psb = (PSbuffer*)object;
    count++;
    if (doSum){ // add all waveforms
      if (count==1) h=new TH1F(*(psb->GetWaveform()));
      else h->Add(psb->GetWaveform());
      title.Form("Waveforms added: %d",count);
      h->SetTitle(title);
      if (doSumOnly) continue;
      h->Draw();
    }
    else { // dispaly individual wave forms
      title.Form("Waveform: %d",count);
      psb->GetWaveform()->SetTitle(title);
      psb->Draw("trigs");
    }
    psb->Print();
    c->Update();
    gSystem->Sleep(delay);
  }
  if (doSumOnly) {
    h->Draw();
    c->Update();
  }
}


// options
// -d n : delay time in milliseconds [1000]
// -s   : show running sum instead of individual waveforms
// -t   : just show total sum of waveforms (no animation)

int main(int argc, char **argv) {
  if (argc<2) {
    cout << "Usage: wavePlayer file.root"
	 << endl;
    return 1;
  }
  TString fin=argv[1];
  TString options="";
  int delay=1000;
  int c;
  while ((c = getopt (argc, argv, "std:")) != -1)
    switch (c) {
    case 's':
      options+="sum";
      break;
    case 't':
      options.ReplaceAll("sum","");
      options+="sumonly";
      break;
    case 'd':
      delay = atoi(optarg);
      break;
    default:
      break;
    }

  
  TApplication theApp("App", &argc, argv);

  wavePlayer(fin,delay,options);
  
  
  cout << "To exit, quit ROOT from the File menu of the GUI (or use control-C)" << endl;
  theApp.Run(true);
  return 0;
}

