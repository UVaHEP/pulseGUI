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

#ifndef __CINT__
#include "../picoscopeDriver/picoscopeDriver.h"
#else
class picoscope; 
#endif


class PicoscopeControls : public TQObject  { 

  RQ_OBJECT("PicoscopeControls"); 
  

private:
  picoscope _ps; 




  TGLayoutHints *_hintsn; 
  TGLayoutHints *_hintsy; 

  TGGroupFrame *_voltageF; 
  TGComboBox *_voltageB; 

  TGGroupFrame *_couplingF; 
  TGComboBox *_couplingB; 

  TGGroupFrame *_channelF;
  TGComboBox *_channelB; 
  
  TGComboBox *_timeB; 
  TGGroupFrame *_timeF; 


  TGTextButton *_runBtn; 
  
  
  TGMainFrame *_mf; 
  //  TGMainFrame *_otherApp; 

  public: 
  PicoscopeControls(); 
  virtual ~PicoscopeControls(); 
  
  void channelHandler(Int_t selection, Int_t widgetID); 
  void voltageHandler(Int_t selection, Int_t widgetID); 
    

  
  void couplingHandler(Int_t selection, Int_t widgetID);

  void timedivHandler(Int_t selection, Int_t widgetID);
  void testRun(); 

}; 


#endif 












