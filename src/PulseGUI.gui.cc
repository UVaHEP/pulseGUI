#include "PulseGUI.h"

#include "TCanvas.h"
#include "TStyle.h"


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
  TGHorizontalFrame *bufferNumberFrame = new TGHorizontalFrame(FButton, WIDTH, HEIGHT, kFixedWidth); 
  
  bufferNum = new TGNumberEntry(bufferNumberFrame, 0, 5,25, TGNumberFormat::kNESInteger); 
  bufferNum->SetNumLimits(TGNumberFormat::kNELLimitMax); 
  bufferNum->SetLimitValues(0, 0); 
  bufferNum->Connect("ValueSet(Long_t)", "PulseGUI", this, "SetBufferNumber(Long_t)"); 
  bufferNumberFrame->AddFrame(bufferNum, new TGLayoutHints(kLHintsTop, 2,0,2,2)); 
  bufferCount = new TGLabel(bufferNumberFrame, TGHotString(" /0")); 
  bufferNumberFrame->AddFrame(bufferCount, new TGLayoutHints(kLHintsTop ,2,0,2,2)); 
  vframe1->AddFrame(bufferNumberFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,0,2,2)); 
  
  playbackBN = new TGTextButton(vframe1, "Start Playback"); 
  playbackBN->Connect("Clicked()", "PulseGUI", this, "PlayBack()"); 

  vframe1->AddFrame(playbackBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,0,2,2)); 
  TGHorizontalFrame *playbackButtons = new TGHorizontalFrame(vframe1, WIDTH, HEIGHT, kFixedWidth); 
  prevBN = new TGTextButton(playbackButtons, "Prev Buffer");
  nextBN = new TGTextButton(playbackButtons, "Next Buffer");
  nextBN->Connect("Clicked()", "PulseGUI", this, "NextBuffer()"); 
  prevBN->Connect("Clicked()", "PulseGUI", this, "PrevBuffer()"); 
  playbackButtons->AddFrame(prevBN, new TGLayoutHints(kLHintsTop|kLHintsExpandX, 2,0,2,2)); 
  playbackButtons->AddFrame(nextBN, new TGLayoutHints(kLHintsTop|kLHintsExpandX, 2,0,2,2)); 
  vframe1->AddFrame(playbackButtons, new TGLayoutHints(kLHintsTop, 2,0,2,2)); 

  loadBN = new TGTextButton(vframe1, "&Load Spectrum");
  loadBN->SetTextColor(blue);
  loadBN->Connect("Clicked()", "PulseGUI", this, "OpenFileDialog()");
  // Set hover alt text
  loadBN->SetToolTipText("Select input ROOT file", 2000);
  vframe1->AddFrame(loadBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					      2,0,2,2));
  
  captureBN = new TGTextButton(vframe1, "&Capture Block"); 
  captureBN->SetTextColor(blue); 
  captureBN->Connect("Clicked()", "PulseGUI", this, "OpenPicoscopeControls()"); 
  captureBN->SetToolTipText("Capture a block from a connected Picoscope", 2000); 
  vframe1->AddFrame(captureBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,0,2,2)); 
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
  widthBN = new TGTextButton(cframe1, "Set &Width (ns)");

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
  responseBN->SetTextColor(gray);
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
  UnZoomBN = new TGTextButton(FMotion, "&UnZoom");
  UnZoomBN->SetToolTipText("Zoom all the way out", 2000);

  FMotion->AddFrame(UnZoomBN, new TGLayoutHints(kLHintsTop | kLHintsExpandX,
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
  
  UnZoomBN->Connect("Clicked()", "PulseAnalysis", _analysis, "UnZoom()");
  zoomOutBN->Connect("Clicked()", "PulseAnalysis", _analysis, "ZoomOut()");
  zoomInBN->Connect("Clicked()", "PulseAnalysis", _analysis, "ZoomIn()");
  shiftLeftBN->Connect("Clicked()", "PulseAnalysis", _analysis, "leftShift()");
  shiftRightBN->Connect("Clicked()", "PulseAnalysis", _analysis, "rightShift()");

  UnZoomBN->Connect("Clicked()", "PulseGUI", this, "DrawSpectrum()");
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

//----------------------------------------------------------------------
void PulseGUI::InitWindow(){

  // Initalizes the frame and canvases for the GUI
  gStyle->SetOptStat(kFALSE);
  // Create the main frame
  UInt_t w = 1200;   // main frame width
  UInt_t h = 550;   // main frame height
  FMain = new TGMainFrame(gClient->GetRoot(),w,h, kVerticalFrame);
  FMain->Connect("CloseWindow()", "PulseGUI", this, "CleanupAndClose()"); 
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


//Cleanup and close when main window has been closed
void PulseGUI::CleanupAndClose() { 
  if (dataFile && dataFile->IsOpen()) {
    dataFile->Close(); 
  }
  exit(0); 



}










