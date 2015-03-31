
void test(TString file){
  gSystem->Load("build/lib/libPulseGUI.so");
  TFile *tf=new TFile(file);
  TList *tl=(TList*)tf->Get("PSbufferlist");
  cout << "Found list with " << tl->GetEntries() << " PSbuffers" << endl;
 
  PulseAnalysis *pA=new PulseAnalysis();
  
  TIter next(tl);
  TObject* object = 0;

  gStyle->SetOptStat(0);
  TCanvas *c=new TCanvas("testCan","test",1000,600);
  TCanvas *c2=new TCanvas("testCan2","PulseHeights",1000,600);
  c->Divide(2,1);
  c->cd(1);
  c->Draw();

  TH1F *pulseHeights=new TH1F();

  TIter next(tl);
  TObject* object = 0;
  
  int nbuf=0;
  while ((object = next())) {
    cout << "Processing buffer " << nbuf << endl;
    PSbuffer *psb=(PSbuffer*)object;
    pA->SetBuffer(psb);
    pA->SetWidth(30);
    
    pA->FindPeaks();
    pA->Analyze();
    c->cd(1);
    pA->DrawSpectrum();
    //c->Update();
    if (nbuf==0) *pulseHeights=*(pA->Hph());
    else pulseHeights->Add(pA->Hph());
    c->cd(2);
    pulseHeights->Draw();
    //   c->Update();
    //gSystem->Sleep(1000);
    nbuf++;
  }
  c2->cd();
  pulseHeights->Draw();
  

}




