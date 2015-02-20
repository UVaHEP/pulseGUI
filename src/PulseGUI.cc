#include "dbg.h"
#include "PulseGUI.h"

#include "TCanvas.h"
#include "TStyle.h"

#include <iostream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;



PulseGUI::PulseGUI(TString fName) { 
  cout << "Creating PulseGUI " << endl;
  InitWindow();
  if (fName!="") LoadSpectrum(fName);
}


PulseGUI::~PulseGUI() {
  //  delete spectPad; 
  //  delete controlPad; 
  //  delete mainCanvas;
  
  if (FMain){
    FMain->SetCleanup(kDeepCleanup);
    FMain->Cleanup();
    delete FMain;
  }
  //Add Analysis Cleanup...? 

}

void PulseGUI::SetStatusText(const char *txt, Int_t pi){
  // Set the text shown in the status bar
  SpectStatusBar->SetText(txt, pi);
}

void PulseGUI::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected){
  // Write the event status in the status bar windows

  const char *text0, *text1, *text3;
  char text2[50];
  text0 = selected->GetTitle();
  SetStatusText(text0,0);
  text1 = selected->GetName();
  SetStatusText(text1,1);
  if (event == kKeyPress) {sprintf(text2, "%c", (char) px);}
  else{ 
    sprintf(text2, "%d,%d", px, py);
    SetStatusText(text2,2);
    text3 = selected->GetObjectInfo(px,py);
    SetStatusText(text3,3);
  }
}

void PulseGUI::Print() { 
  std::cout << "Just a simple gui." << std::endl; 
}

//----------------------------------------------------------------------
//void PulseGUI::PrintInfo(const char *va_(fmt), bool clear, ...){
void PulseGUI::PrintInfo(const char *va_(fmt), ...){
  // Print info to the Info PaveText. If you want to append the info to
  // the previous call, pass clear=false.
  // Clears the TPaveText automatically before filling.

  //  EInfoCanvas->GetCanvas()->cd();

  // Format the string with the information

  //  TString s;
  char buf[256];
  va_list ap;
  va_start(ap, va_(fmt));
  vsnprintf(buf, sizeof(buf), fmt, ap);
  //s.Form(va_(fmt), ap); // problems with this
  va_end(ap);

  // Add the string as text to the info TPaveText
  //  if (clear) tInfo->Clear();
  //  tInfo->Clear();
  //  tInfo->AddText(buf);
  //  tInfo->Draw();

  //  EInfoCanvas->GetCanvas()->Update();

  cout << "PrintInfo: " << buf << endl;  // copy to std out
}


//----------------------------------------------------------------------
void PulseGUI::Show() { 
  //  FSpect->Draw(); 
}

//----------------------------------------------------------------------
void PulseGUI::printState() { 
  printf("xmark : ymark = [ %f : %f ]\n",marks[0].x,marks[0].y);
}






//----------------------------------------------------------------------
void PulseGUI::setupInfoPad() {
  // Creates and positions the TPaveLabels that will display
  // information during the analysis.

  infoPad->cd(); 
  info2 = new TPaveLabel(0.03,0.10,0.97,0.60,"info label","NDC");
  info2->SetTextAlign(11); 
  info2->Draw(); 
}

//----------------------------------------------------------------------
void PulseGUI::setMessage(const TString *msg, bool Alert ) { 
  infoPad->cd(); 
  if (msg != NULL) {
    (Alert) ? info2->SetTextColor(kRed) :  info2->SetTextColor(kBlack); 
    info2->SetLabel(msg->Data()); 
    infoPad->Modified(); 
    ESpectCanvas->GetCanvas()->Update(); 
  }
}

//----------------------------------------------------------------------
void PulseGUI::setMessage(const char *msg, bool Alert ) { 
  TString m(msg);
  setMessage(&m,Alert);
}

//----------------------------------------------------------------------
void PulseGUI::execEvent(Int_t event, Int_t x, Int_t y, TObject *selected) {
  if (selected) {} // prevent compiler warning
  //  TPad *c = (TPad *) gTQSender;

  if (event==1) { 
    Double_t xx  = spectPad->AbsPixeltoX(x);
    Double_t yy  = spectPad->AbsPixeltoY(y);
    
    marks[1] = marks[0]; 
    marks[0].x = xx; 
    marks[0].y = yy;    
  }
}





void PulseGUI::Reset() { 
  _analysis->Reset(); 
  AnaClean(); 
  setMessage(TString("Reset Spectrum"));
  WidthNum->SetNumber(_analysis->GetPulseWidth());
  ThreshNum->SetNumber(_analysis->GetThreshold()); 
  DrawSpectrum(); 

}

void PulseGUI::DrawSpectrum() { 
  spectPad->cd();
  _analysis->DrawSpectrum(); 
  spectPad->Modified(); 
  ESpectCanvas->GetCanvas()->Update();
}

void PulseGUI::AnaClean(){
  // Cleans up and removes the FAna frame and readies for
  // Reset() or Analyze().
  if (FAna) { 
    FAna->SetCleanup(kDeepCleanup);
    FAna->Cleanup();
    EAnaCanvas=0;
    FMain->RemoveFrame(FAna); // Inherited from TGCompositeFrame
    delete FAna; FAna=0;
    _analysis->AnaClean(); 
  }
  FMain->MapSubwindows();
  FMain->Resize(FMain->GetDefaultSize());
  FMain->MapWindow(); 
}


