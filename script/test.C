
void test(TString file){
  gSystem->Load("build/lib/libPulseGUI.so");
  TFile *tf=new TFile(file);
  TList *tl=(TList*)tf->Get("PSbufferlist");
  cout << "Found list with " << tl->GetEntries() << " PSbuffers" << endl;
 
  PulseAnalysis *pA=new PulseAnalysis();
  //  pA->SetWidth(30e-9);
  
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
    cout << "Processing buffer " << nbuf << endl;
    PSbuffer *psb=(PSbuffer*)object;
    pA->SetBuffer(psb);

    cout << "Set Width " << pA->SetWidth(30) << endl;
    pA->FindPeaks();
    pA->Analyze();
    pA->DrawSpectrum(); c->Update();
    gSystem->Sleep(1000);  
    //    psb->Draw("trigs");
    if (nbuf==0) *pulseHeights=*(pA->Hph());
    else pulseHeights->Add(pA->Hph());
    pulseHeights->Draw();
    c->Update();
    gSystem->Sleep(100);
    nbuf++;
  }

  pulseHeights->Draw();
  c->Update();

}




