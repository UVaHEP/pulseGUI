#include "PulseGUI.h"

#include "TCanvas.h"

#include <iostream>
using std::cout;
using std::endl;


/////////////////
// Button actions
/////////////////

//----------------------------------------------------------------------
void PulseGUI::OpenFileDialog() {
  // Open a browser window and let user select ROOT file to open.
  // Call LoadSpectrum().
 
  fileInfo = new TGFileInfo; 
  fileDialog = new TGFileDialog(gClient->GetRoot(), FButton, kFDOpen, fileInfo); 
  debug("Selected file: %s",fileInfo->fFilename);
  if (fileInfo->fFilename == NULL) {
    std::cout << "File selection cancelled." << std::endl; 
    return; 
  }
  LoadSpectrum(fileInfo->fFilename); 
}

void PulseGUI::LoadSpectrum(TString fName){
  debug();
  if (dataFile != NULL) {
    dataFile->Close(); 
    buffer = NULL; 
  }
  
  dataFile = TFile::Open(fName); 
  buffer = (PSbuffer *) dataFile->Get("PSbuffer"); 
  _analysis = new PulseAnalysis(buffer); 

  ConnectButtons(); 
  ThreshNum->SetNumber(_analysis->GetThreshold()); 
  WidthNum->SetNumber(_analysis->GetPulseWidth()); 
  if(FAna) { 
    AnaClean();
  }
  setMessage(TString("")); 
  TString name=(dataFile->GetName());
  if (name.Length()>50) {
    name.Remove(0,51);
    name="..."+name;
  }
  FMain->SetWindowName("File: "+name);
  DrawSpectrum();
}

//----------------------------------------------------------------------
void PulseGUI::SetThreshold(){
  // Passes the mouse click amplitude in mV to
  // SetThreshold(Float_t t)

  std::cout << "Set Threshold" << std::endl; 
  SetThreshold(marks[0].y);
}


//----------------------------------------------------------------------
void PulseGUI::SetThreshold(Float_t t){
  // Sets the threshold value and updates
  // the spectrum canvas labels.
  if (_analysis) { 
     _analysis->SetThreshold(t);  
  }
  TString s; 
  s.Form("Thresh: %5.2f mV", t); 
  PrintInfo("Threshold set to %5.2f mV",t);
  setMessage(&s);
  infoPad->Modified(); 
  ESpectCanvas->GetCanvas()->Update(); 
  spectPad->cd(); 
  // Update the number entry value
  ThreshNum->SetNumber(t);
  // Update threshold marker if shown
}


void PulseGUI::FindPeaks() { 
  if (_analysis) { 
    spectPad->cd();
    TString s = _analysis->FindPeaks(); 
    DrawSpectrum(); 
    setMessage(&s); 
  }
}

//----------------------------------------------------------------------
void PulseGUI::SetYMarks(){
  // Sets the y-value of marks to the number entry
  // which will be used in SetThreshold()

  marks[0].y = ThreshNum->GetNumberEntry()->GetNumber();
  SetThreshold(marks[0].y);
}

void PulseGUI::SetXMarks() { 
  // Sets the x-value width of marks to the value contained in the width box
  marks[1].x = marks[0].x+WidthNum->GetNumberEntry()->GetNumber(); 
  std::cout << "X-start:" << marks[0].x << 
    " X-End:" << marks[1].x << std::endl; 

  SetWidth(); 

}


//----------------------------------------------------------------------
void PulseGUI::SetWidth(){
  // set width in microseconds

  cout << "x1:" << marks[0].x << " x2:" << marks[1].x << endl; 
  SetWidth(TMath::Abs(marks[0].x-marks[1].x));
  return;
}

//----------------------------------------------------------------------
void PulseGUI::SetWidth(Float_t w){
  // set width in nanoseconds
  
  TString s; 
  if (_analysis)
    s = _analysis->SetWidth(w); 
  else
    s = ""; 
  WidthNum->SetNumber(w); 
  setMessage(&s);
  infoPad->Modified(); 
  ESpectCanvas->GetCanvas()->Update(); 
  spectPad->cd();
}

//----------------------------------------------------------------------
void PulseGUI::measure(){
  Double_t dx=marks[0].x-marks[1].x;
  Double_t dy=marks[0].y-marks[1].y;
  printf("deltaX: %e ns , deltaY: %e mV\n",dx,dy); 
  TString s; 
  s.Form("dX: %3.1f ns   dY: %5.2f mV",dx,dy); 
  setMessage(&s);
  infoPad->Modified(); 
  ESpectCanvas->GetCanvas()->Update();
  spectPad->cd(); 
}


//----------------------------------------------------------------------

void PulseGUI::Analyze() { 
  if (FAna) AnaClean();
  
  //  FAna = new TGHorizontalFrame(FMain, 1200, 350);
  UInt_t AnaW = 1200;
  UInt_t AnaH = 300;
  FAna = new TGMainFrame(FMain, AnaW, AnaH, kHorizontalFrame);
  FAna->SetWindowName("PulseGUI Display");
  FMain->AddFrame(FAna, new TGLayoutHints(kLHintsExpandX
					  | kLHintsExpandY,2,2,5,1));
  EAnaCanvas = new TRootEmbeddedCanvas("Ana Canvas", FAna, 1200,350);
  FAna->AddFrame(EAnaCanvas,
		 new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5,5,5,5));
  //  gStyle->SetOptStat(kTRUE);
  
  //  anaCanvas=new TCanvas("analysis","Data Analysis",0,490,1200,350);
  //  anaCanvas->Connect("Closed()","PulseGUI",this,"NullPointer(&anaCanvas)");
  //  FAna->Connect("Closed()","PulseGUI",this,"NullPointer()");
  EAnaCanvas->GetCanvas()->Divide(3,1);
  // retrive peak locations
  _analysis->Analyze(); 

  EAnaCanvas->GetCanvas()->cd(1);
  _analysis->Hdt()->Draw(); 
  EAnaCanvas->GetCanvas()->cd(2);
  _analysis->Hph()->Draw(); 
  EAnaCanvas->GetCanvas()->cd(3);
  _analysis->Hpi()->Draw(); 
  spectPad->cd();
  EAnaCanvas->GetCanvas()->Update();

  FMain->MapSubwindows();
  FMain->Resize(FMain->GetDefaultSize());
  FMain->MapWindow(); 
}

