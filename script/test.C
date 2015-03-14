
void test(TString file){
  gSystem->Load("build/lib/libPulseGUI.so");
  TFile *tf=new TFile(file);
  TList *tl=(TList*)tf->Get("PSbufferlist");
  cout << "Found list with " << tl->GetEntries() << " PSbuffers" << endl;
    
  PulseAnalysis *pA=new PulseAnalysis();

  TIter next(tl);
  TObject* object = 0;
  TCanvas *c=new TCanvas();
  gStyle->SetOptStat(0);
  c->Draw();

  TH1F *pulseHeights=new TH1F();

  TIter next(tl);
  TObject* object = 0;
  
  int nbuf=0;
  while ((object = next())) {
    PSbuffer *psb=(PSbuffer*)object;
    pA->SetBuffer(psb);
    pA->FindPeaks();
    pA->Analyze();
    //    psb->Draw("trigs");
    if (nbuf==0) *pulseHeights=*(pA->Hph());
    else pulseHeights->Add(pA->Hph());
    pulseHeights->Draw();
    c->Update();
    gSystem->Sleep(500);
    nbuf++;
  }
    

}




