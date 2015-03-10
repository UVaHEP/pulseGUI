#ifndef PICOSCOPECONTROLS_H
#define PICOSCOPECONTROLS_H


#include "RQ_OBJECT.h"
#include "TGCanvas.h"
#include "TGFrame.h"
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TGStatusBar.h"
#include "TGWindow.h"
#include "TString.h"
#include "TGListView.h"
#include "TGListBox.h"
#include "TGComboBox.h" 
#include "TGLabel.h"
#include "TMath.h"
#include "TGButtonGroup.h"
#include "PSbuffer.h"
#include <functional>

#ifndef __CINT__
#include "picoscopeDriver.h"
#else
#include 
class picoscope { 
  class picoscopeData; 
};
#endif



class PicoscopeControls : public TQObject  { 

  RQ_OBJECT("PicoscopeControls"); 
  

private:
  picoscope _ps; 


  TGLabel *_connectionStatus; 

  TGLayoutHints *_hintsn; 
  TGLayoutHints *_hintsy; 

  TGGroupFrame *_voltageF; 
  TGComboBox *_voltageB; 

  TGGroupFrame *_couplingF; 
  TGComboBox *_couplingB; 

  TGVButtonGroup *_channels; 

  TGGroupFrame *_timeSamplingF; 
  TGGroupFrame *_voltageCouplingF; 
  TGGroupFrame *_channelStatusF; 
  TGGroupFrame *_triggerRunF; 

  TGNumberEntry *_sampleInterval; 
  TGLabel *_intervalLabel; 

  TGNumberEntry *_sampleNumber; 
  TGLabel *_windowLabel; 

  TGTextButton *_runBtn; 
  
  TGNumberEntry *_triggerLevel; 
  TGLabel *_triggerLabel; 

  TGTextButton *_triggerEnableBtn; 
  TGTextButton *_writeBuffersBtn; 
  
  TGMainFrame *_mf; 


  TList *_buffers; 
  #ifndef __CINT__
  PS6000_CHANNEL _selectedChannel; 
  #endif

  public: 


  PicoscopeControls(); 
  virtual ~PicoscopeControls(); 
  
  void channelHandler(Int_t selection, Int_t widgetID); 
  void voltageHandler(Int_t selection, Int_t widgetID); 
    
  void sampleNumberHandler(Long_t val); 
  void sampleIntervalHandler(Long_t val); 
  TString prettyPrintInterval();   
  TString prettyPrintWindow();   
  void couplingHandler(Int_t selection, Int_t widgetID);

  void timedivHandler(Int_t selection, Int_t widgetID);
  void triggerButton(); 
  void triggerLevelHandler(Long_t val);
  void testRun(); 
  void writeBuffersToDisk();

  TList* Buffers() { return _buffers; }; 

  void callBack(picoscope::picoscopeData *data); 

}; 


#endif 















