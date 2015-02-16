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
#include "picoscopeDriver.h"
#endif 

class PicoscopeControls : public TQObject  { 

  RQ_OBJECT("PicoscopeControls"); 
  

  private:

  

  TGLayoutHints *_hintsn; 
  TGLayoutHints *_hintsy; 

  TGGroupFrame *_voltageF; 
  TGComboBox *_voltageB; 

  TGGroupFrame *_couplingF; 
  TGComboBox *_couplingB; 

  
  TGComboBox *_timeB; 
  TGGroupFrame *_timeF; 


  
  TGMainFrame *_mf; 
  //  TGMainFrame *_otherApp; 

  public: 
  PicoscopeControls(); 
  virtual ~PicoscopeControls(); 
  
  void voltageHandler(Int_t selection, Int_t widgetID); 
    

  
  void couplingHandler(Int_t selection, Int_t widgetID);

  void timedivHandler(Int_t selection, Int_t widgetID);


}; 


#endif 












