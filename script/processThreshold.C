

void processThreshold(TString fname, Float_t width, Float_t threshold) { 
  gSystem->Load("pulseGUI.so"); 
  pulseAnalysis* ana = new pulseAnalysis(fname); 
  ana->SetThreshold(threshold); 
  ana->SetWidth(width); 
  ana->FindPeaks(true); 
  ana->Analyze(); 
  ana->Hph()->Draw(); 


}


					
