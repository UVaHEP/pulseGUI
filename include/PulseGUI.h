#ifndef PULSEGUI_H
#define PULSEGUI_H


#include "PulseAnalysis.h"
#include "PSbuffer.h" 
#include "RQ_OBJECT.h"
#include "TGCanvas.h"
#include "TGFrame.h"
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TGStatusBar.h"
#include "TPad.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include "TRootEmbeddedCanvas.h"
#include "TString.h"

#ifdef CONTROLS
#include "PicoscopeControls.h"
#endif 


class PulseGUI : public TQObject { 
  RQ_OBJECT("PulseGUI"); 

  struct clickPosition_t { 
    Double_t x; 
    Double_t y; 
  };


private: 
  // Class global objects
  TFile                 *dataFile; 
  TGMainFrame           *FMain;
  TGHorizontalFrame     *FControl;
  TGVerticalFrame       *FButton;
  TGHorizontalFrame     *FInfo;
  TGHorizontalFrame     *FMotion; 
  TRootEmbeddedCanvas   *EInfoCanvas;
  TGVerticalFrame       *FSpect;
  TRootEmbeddedCanvas   *ESpectCanvas;
  TGMainFrame           *FAna;
  TRootEmbeddedCanvas   *EAnaCanvas;

  TGStatusBar           *SpectStatusBar;
  TGNumberEntry         *ThreshNum;
  TGNumberEntry         *WidthNum;
  
  //Storage + Main root stuff 
  //  TCanvas *mainCanvas; 
  //  TCanvas *anaCanvas;

  // Button Frame and buttons
  TGTextButton  *loadBN, *convertBN, *threshBN, *threshNumBN, *widthBN, *measureBN,
    *responseBN, *getBKGBN, *subBKGBN, *findPeaksBN, *analyzeBN,
    *writePeaksBN, *zoomOutBN,*UnZoomBN, *zoomInBN, *shiftLeftBN,
    *shiftRightBN, *resetBN, *exitBN, *smoothBN, *captureBN;

  TGTextButton  *ShowHideBN;
  Bool_t        show;
  TArrow        *arThresh;

  TGFileDialog  *fileDialog; 
  TGFileInfo    *fileInfo;

  // Error message frame
  TGMainFrame *errorFrame;
  TGCanvas    *errorCanvas;       

  TPad *spectPad; 
  TPad *controlPad; 
  TPad *infoPad;
  
  clickPosition_t marks[2]; //Buffer containing previous click locations
  
  //Canvas Related 
  TPad *pspect; //TPad w/Pulse Spectrum 
  //InfoPad Labels
  TPaveLabel *info2; 
  
  TPaveText *tInfo; 

  static const bool ERRMSG=true;

  //Analysis object accesses/manages data 
  PulseAnalysis *_analysis; 

  PSbuffer * buffer; 

#ifdef CONTROLS
  PicoscopeControls *_pscontrols; 
#endif
  // member functions 
public:
  PulseGUI(TString fName="");
  virtual ~PulseGUI(); 

  void InitWindow();
  void MakeButtons();   // replacement for setupControls
  void ConnectButtons(); 

  void OpenFileDialog(); 
  void LoadSpectrum(TString fName);
  void OpenPicoscopeControls(); 
  void AnaClean();
  void CleanupAndClose(); 
  void setupInfoPad(); 
  void Show();
  void DrawSpectrum(); 
  void Reset(); 

  void PrintInfo(const char *va_(fmt), ...);

  void SetStatusText(const char *txt, Int_t pi);
  void EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected); 

  void measure(); 
  void SetWidth();
  void SetWidth(Float_t w);
  void SetThreshold();
  void SetThreshold(Float_t t);
  void SetXMarks();
  void SetYMarks();
  void printState(); 
  
  void FindPeaks();
  void Analyze(); 


  void setMessage(const TString *msg, bool Alert = false);
  void setMessage(const char *msg, bool Alert = false);
  void Print();

  PulseAnalysis* GetAnalysis() {return _analysis;}

  void execEvent(Int_t event, Int_t x, Int_t y, TObject *selected); 

};

#endif
