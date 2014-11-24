#include "PulseGUI.h"
#include <TSystem.h>
#include <TObject.h>
#include <TInterpreter.h>
#include <TApplication.h> 
#include "TString.h"
#include "TSpectrum.h"
#include "TStopwatch.h"
#include "TList.h"
#include "TPolyMarker.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFitResultPtr.h"
#include "TF1.h"
#include <TNtuple.h>
#include "TTree.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TQObject.h"
#include <TGButton.h>
#include <TGClient.h>
#include <TGLabel.h>
#include "TGFileBrowser.h"
#include "TROOT.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TArrayF.h"
#include "TArrayC.h"
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream> 


#include "dbg.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;



PulseGUI::PulseGUI(TString fName) { 
  cout << "Creating PulseGUI " << endl;
  _analysis = new PulseAnalysis(); 
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
  // set width in microseconds

  
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


//////////////////////////////////////////////////
/// Histogram widowing
//////////////////////////////////////////////////

//----------------------------------------------------------------------
void PulseGUI::SetPeriod(){
  // Set the value displayed for the shown period

  //  float xl = _hspect->GetBinCenter(_hspect->GetXaxis()->GetFirst()); 
  //  float xr = _hspect->GetBinCenter(_hspect->GetXaxis()->GetLast());
  //  float display = xr-xl; 
  float display = 0.0; 
  TString s;
  s.Form("Period Displayed: %4.2f ns", display*1e-3);  // ns to us

  infoPad->Modified(); 
  ESpectCanvas->GetCanvas()->Update();
  spectPad->cd(); 
}




//----------------------------------------------------------------------
void PulseGUI::setupInfoPad() {
  // Creates and positions the TPaveLabels that will display
  // information during the analysis.

  infoPad->cd(); 
  info2 = new TPaveLabel(0.05,0.02,0.95,0.47," ...more info....","NDC");
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
  TStopwatch timer;
  timer.Start();
  _analysis->LoadSpectrum(fName); 
  ConnectButtons(); 
  ThreshNum->SetNumber(_analysis->GetThreshold()); 
  WidthNum->SetNumber(_analysis->GetPulseWidth()); 
  if(FAna) { 
    AnaClean();
  }
  setMessage(TString("")); 
  
  // TString name=fileInfo->fFilename;
  // if (name.Last('/')>-1) name.Remove(0,name.Last('/')+1);
  // if (name.Last('.')>-1) name.Remove(name.Last('.'));
  // FMain->SetWindowName("File: "+name);
  TString name=(_analysis->GetSpectName());
  if (name.Length()>50) {
    name.Remove(0,51);
    name="..."+name;
  }
  FMain->SetWindowName("File: "+name);
  DrawSpectrum();
  timer.Stop();
  log_info("Load Complete %lf seconds",timer.RealTime());
}

void PulseGUI::FindPeaks() { 
  if (_analysis) { 
    spectPad->cd();
    TString s = _analysis->FindPeaks(); 
    DrawSpectrum(); 
    setMessage(&s); 
  }
}

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

//////////////////////////////////////////////////
/// GUI
//////////////////////////////////////////////////

//----------------------------------------------------------------------
// Initialize the GUI display
//----------------------------------------------------------------------
void PulseGUI::MakeButtons(){
  // Constructs the button frame, creates all buttons,
  // and connects buttons to functions. Also construct the
  // Info frame.

  const Int_t WIDTH = 200;
  const Int_t HEIGHT = 50;
  // Background colors
  ULong_t red;
  ULong_t green;
  ULong_t blue;
  ULong_t gray;
  ULong_t white;
  gClient->GetColorByName("red", red);
  gClient->GetColorByName("green", green);
  gClient->GetColorByName("blue", blue);
  //  gClient->GetColorByName("gray", gray);
  gClient->GetColorByName("#888888",gray);
  gClient->GetColorByName("white", white);





  //Create and connect buttons
  //Load Spectrum
  TGVerticalFrame *vframe1 = new TGVerticalFrame(FButton, WIDTH, HEIGHT, kFixedWidth);
  loadBN = new TGTextButton(vframe1, "&Load Spectrum");
  loadBN->SetTextColor(blue);
  loadBN->Connect("Clicked()", "PulseGUI", this, "OpenFileDialog()");
  // Set hover alt text
  loadBN->SetToolTipText("Select input ROOT file", 2000);
  vframe1->AddFrame(loadBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					      2,0,2,2));
  FButton->AddFrame(vframe1, new TGLayoutHints(kLHintsCenterX,2,2,5,1));

  /*  // Convert Data File
  TGVerticalFrame *vframe2 = new TGVerticalFrame(FButton, WIDTH, HEIGHT, kFixedWidth);
  convertBN = new TGTextButton(vframe2, "&Convert Data File");
  convertBN->SetTextColor(blue);

  convertBN->SetToolTipText("Convert data to ROOT file", 2000);
  vframe2->AddFrame(convertBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						 2,0,2,2));
  FButton->AddFrame(vframe2, new TGLayoutHints(kLHintsCenterX,2,2,5,1));
  */
  // Number Entry
  // Composite Num
  TGCompositeFrame *cnum = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						kHorizontalFrame | kFixedWidth);
  // Set Threshold
  threshNumBN = new TGTextButton(cnum, "Set &Threshold");

  threshNumBN->SetToolTipText("Set min peak amplitude in mV", 2000);
  cnum->AddFrame(threshNumBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						2,2,2,2));
  // Number entry composite
  ThreshNum = new TGNumberEntry(cnum, 0,9,999, TGNumberFormat::kNESRealTwo,
				TGNumberFormat::kNEANonNegative,TGNumberFormat::kNELLimitMinMax,0,99999);

  cnum->AddFrame(ThreshNum, new TGLayoutHints(kLHintsBottom | kLHintsExpandX,
					      2,2,2,2));
  FButton->AddFrame(cnum, new TGLayoutHints(kLHintsCenterX,2,2,5,1));
  
  // Set Width
  TGCompositeFrame *cframe1 = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						   kHorizontalFrame | kFixedWidth);
  // Set Width
  widthBN = new TGTextButton(cframe1, "Set &Width");

  cframe1->AddFrame(widthBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					       2,2,2,2));

  // Width Number Entry 
  WidthNum =  new TGNumberEntry(cframe1, 0,9,999, TGNumberFormat::kNESRealTwo,
				TGNumberFormat::kNEANonNegative,TGNumberFormat::kNELLimitMinMax,0,99999);



  cframe1->AddFrame(WidthNum, new TGLayoutHints(kLHintsBottom | kLHintsExpandX,
						2,2,2,2)); 
  

  FButton->AddFrame(cframe1, new TGLayoutHints(kLHintsCenterX,2,2,5,1));

  // Measure and Set Resolution
  TGCompositeFrame *cframe2 = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						   kHorizontalFrame | kFixedWidth);

  // Set Response
  responseBN = new TGTextButton(cframe2, "Set &Response");

  cframe2->AddFrame(responseBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						  2,2,2,2));
  FButton->AddFrame(cframe2, new TGLayoutHints(kLHintsCenterX,2,2,5,1));
  
  // Get Background and Subtract Background
  TGCompositeFrame *cframe3 = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						   kHorizontalFrame | kFixedWidth);
  // Get Background
  getBKGBN = new TGTextButton(cframe3, "&Get Background");

  cframe3->AddFrame(getBKGBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						2,2,2,2));
  // Subtract Background
  subBKGBN = new TGTextButton(cframe3, "Subtract BKG");
  subBKGBN->SetTextColor(gray);

  cframe3->AddFrame(subBKGBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						2,2,2,2));
  FButton->AddFrame(cframe3, new TGLayoutHints(kLHintsCenterX,2,2,5,1));
  
  // Find Peaks and Analyze
  TGCompositeFrame *cframe4 = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						   kHorizontalFrame | kFixedWidth);
  // Find Peaks
  findPeaksBN = new TGTextButton(cframe4, "&Find Peaks");

  cframe4->AddFrame(findPeaksBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						   2,2,2,2));
  // Analyze
  analyzeBN = new TGTextButton(cframe4, "&Analyze");
  analyzeBN->ChangeBackground(green);

  cframe4->AddFrame(analyzeBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						 2,2,2,2));
  FButton->AddFrame(cframe4, new TGLayoutHints(kLHintsCenterX,2,2,5,1));

  // Write Peaks, and Smooth Histogram
  TGCompositeFrame *cframe8 = new TGCompositeFrame(FButton, WIDTH, HEIGHT, 
						   kHorizontalFrame | kFixedWidth); 
  writePeaksBN = new TGTextButton(cframe8, "&Write Peaks");

  writePeaksBN->SetToolTipText("Write analysis to .dat file", 2000);
  cframe8->AddFrame(writePeaksBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						    2,2,2,2)); 
  smoothBN = new TGTextButton(cframe8, "&SmoothHistogram"); 

  smoothBN->SetToolTipText("Smooth Histogram", 2000); 
  cframe8->AddFrame(smoothBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 
						2,2,2,2));
  
  //TGVerticalFrame *vframe3 = new TGVerticalFrame(FButton, WIDTH, HEIGHT, kFixedWidth);
  /*  writePeaksBN = new TGTextButton(vframe3, "&Write Peaks");
      writePeaksBN->Connect("Clicked()", "PulseGUI", this, "DumpPeaks()");
      writePeaksBN->SetToolTipText("Write analysis to .dat file", 2000);*/
  //  vframe3->AddFrame(writePeaksBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
  //        2,0,2,2));
  FButton->AddFrame(cframe8, new TGLayoutHints(kLHintsCenterX,2,2,5,1));


  //Reset and Exit
  TGCompositeFrame *cframe7 = new TGCompositeFrame(FButton, WIDTH, HEIGHT,
						   kHorizontalFrame | kFixedWidth);
  // Reset
  resetBN = new TGTextButton(cframe7, "&Reset");

  cframe7->AddFrame(resetBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					       2,2,2,2));
  // Exit
  exitBN = new TGTextButton(cframe7, "&Exit", "gApplication->Terminate(0)");
  exitBN->ChangeBackground(red);
  exitBN->SetToolTipText("Close the application and exit ROOT", 2000);
  cframe7->AddFrame(exitBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					      2,2,2,2));
  FButton->AddFrame(cframe7, new TGLayoutHints(kLHintsCenterX,2,2,5,1));  

  // Motion Panel 
  FMotion = new TGHorizontalFrame(FSpect, 800, 100, kLHintsExpandX | kLHintsExpandY); 

  // Measure
  measureBN = new TGTextButton(FMotion, "&Measure");

  FMotion->AddFrame(measureBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						 2,2,2,2));

  // UnZoom
  unzoomBN = new TGTextButton(FMotion, "&UnZoom");
  unzoomBN->SetToolTipText("Zoom all the way out", 2000);

  FMotion->AddFrame(unzoomBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						    2,2,2,2));

  // Shift Left
  shiftLeftBN = new TGTextButton(FMotion, "<--Shift");

  FMotion->AddFrame(shiftLeftBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						   2,2,2,2));
  // Shift Right
  shiftRightBN = new TGTextButton(FMotion, "Shift-->");

  FMotion->AddFrame(shiftRightBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						3,2,2,2));
  // Zoom In
  zoomInBN = new TGTextButton(FMotion, "&Zoom In");
  zoomInBN->SetToolTipText("Zoom in one cycle", 2000);

  FMotion->AddFrame(zoomInBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						3,2,2,2));

  // Zoom Out
  zoomOutBN = new TGTextButton(FMotion, "Zoom &Out");
  zoomOutBN->SetToolTipText("Zoom out one cycle", 2000);

  FMotion->AddFrame(zoomOutBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
						 3,2,2,2));

  

  FSpect->AddFrame(FMotion, new TGLayoutHints(kLHintsExpandX | kLHintsBottom, 2,2,2,2));

}

void PulseGUI::ConnectButtons() { 
  if (_analysis) { 

  writePeaksBN->Connect("Clicked()", "PulseAnalysis", _analysis, "DumpPeaks()");
  measureBN->Connect("Clicked()", "PulseGUI", this, "measure()");

  smoothBN->Connect("Clicked()", "PulseAnalysis", _analysis, "SmoothHistogram()"); 
  smoothBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()"); 

  resetBN->Connect("Clicked()", "PulseGUI", this, "Reset()");
  
  unzoomBN->Connect("Clicked()", "PulseAnalysis", _analysis, "unzoom()");
  zoomOutBN->Connect("Clicked()", "PulseAnalysis", _analysis, "outzoom()");
  zoomInBN->Connect("Clicked()", "PulseAnalysis", _analysis, "inzoom()");
  shiftLeftBN->Connect("Clicked()", "PulseAnalysis", _analysis, "leftShift()");
  shiftRightBN->Connect("Clicked()", "PulseAnalysis", _analysis, "rightShift()");

  unzoomBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");
  zoomOutBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");
  zoomInBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");
  shiftLeftBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");
  shiftRightBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");



  analyzeBN->Connect("Clicked()", "PulseGUI", this, "Analyze()");
  findPeaksBN->Connect("Clicked()", "PulseGUI", this, "FindPeaks()");


  //subBKGBN->Connect("Clicked()", "PulseGUI", _analysis, "subBkg()");
  //getBKGBN->Connect("Clicked()", "PulseGUI", _analysis, "getBkg()");
  responseBN->Connect("Clicked()", "PulseAnalysis", _analysis, "SetResponse()");
  (WidthNum->GetNumberEntry())->Connect("ReturnPressed()", "PulseGUI", this, "SetXMarks()"); 
  WidthNum->Connect("ValueSet(Long_t)", "PulseGUI", this, "SetXMarks()"); 
  widthBN->Connect("Clicked()", "PulseGUI", this, "SetWidth()");
  (ThreshNum->GetNumberEntry())->Connect("ReturnPressed()", "PulseGUI", this,
					 "SetYMarks()");    // Fix connection
  ThreshNum->Connect("ValueSet(Long_t)", "PulseGUI", this, "SetYMarks()");   //Fix connection
  threshNumBN->Connect("Clicked()", "PulseGUI", this, "SetThreshold()");
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


//----------------------------------------------------------------------
void PulseGUI::InitWindow(){

  // Initalizes the frame and canvases for the GUI
  gStyle->SetOptStat(kFALSE);
  // Create the main frame
  UInt_t w = 1200;   // main frame width
  UInt_t h = 550;   // main frame height
  FMain = new TGMainFrame(gClient->GetRoot(),w,h, kVerticalFrame);
  // Create the control frame for the buttons and spectrum
  FControl = new TGHorizontalFrame(FMain,w,h);
  // Add the frames to the parent frame
  FMain->AddFrame(FControl,
		  new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2,2,2,2));

  // Button Frame
  // Create the button frame
  FButton = new TGVerticalFrame(FControl, 200, 150, kLHintsLeft | kFixedWidth);
  FControl->AddFrame(FButton, new TGLayoutHints(kFixedWidth | kLHintsExpandY,
						0,2,0,0));

  //Create Spectrum Frame   
  UInt_t SpectW = 800;
  UInt_t SpectH = 450;
  FSpect = new TGVerticalFrame(FControl,SpectW,SpectH,
			       kLHintsExpandX | kLHintsExpandY);
  FControl->AddFrame(FSpect, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
					       2,2,2,2));

  MakeButtons();

  // Create the canvas frame
  // Create canvas widget
  ESpectCanvas = new TRootEmbeddedCanvas(0, FSpect, SpectW,SpectH);
  //  TCanvas *spectC = ESpectCanvas->GetCanvas();
  ESpectCanvas->GetCanvas()->ToggleEventStatus();
  ESpectCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
				     "PulseGUI", this, "execEvent(Int_t, Int_t, Int_t, TObject*)");  
  //  spectC->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
  ESpectCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
				     "PulseGUI", this, "EventInfo(Int_t, Int_t, Int_t, TObject*)");  
  // Embed the canvas in the Spectrum Frame 
  FSpect->AddFrame(ESpectCanvas, new TGLayoutHints(kLHintsTop | kLHintsLeft |
						   kLHintsExpandX | kLHintsExpandY, 0,0,2,2));

  // Spectrum Canvas Status Bar
  Int_t parts[] = {45, 15, 10, 30};
  SpectStatusBar = new TGStatusBar(FSpect, 50, 10, kVerticalFrame);
  SpectStatusBar->SetParts(parts,4);
  SpectStatusBar->Draw3DCorner(kFALSE);
  FSpect->AddFrame(SpectStatusBar, new TGLayoutHints(kLHintsExpandX, 0,0,10,0));

  infoPad = new TPad("infoPad", "Info", 0.0,0.0,1.0,0.1);
  infoPad->SetFillColor(19);
  setupInfoPad();

  spectPad = new TPad("spectPad", "Pulse Data", 0.0,0.1, 1.0, 1.0);
  spectPad->cd();
  TPaveLabel *empty = new TPaveLabel(0.4,0.4,0.6,0.6,"No Spectrum Loaded");
  empty->Draw();
 
  ESpectCanvas->GetCanvas()->Update();
  ESpectCanvas->GetCanvas()->cd();
  spectPad->Draw();
  infoPad->Draw(); 
  spectPad->Update(); 
  ESpectCanvas->GetCanvas()->Update();

  // Make all visible
  FMain->SetWindowName("Analysis Package");
  FMain->MapSubwindows();
  FMain->Resize(FMain->GetDefaultSize());
  FMain->MapWindow(); 
}


