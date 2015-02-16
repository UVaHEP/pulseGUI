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


class PicoscopeControls : public TQObject  { 

  RQ_OBJECT("PicoscopeControls"); 
  

  private:
  void setupVoltageEntries(); 
  TGLayoutHints *_hintsn; 
  TGLayoutHints *_hintsy; 

  TGGroupFrame *_voltageF; 
  TGLBContainer *_voltageC; 
  TGListBox *_voltageL; 

  std::vector<TGTextLBEntry *> _voltages; 

  TGGroupFrame *_couplingF; 
  TGLBContainer *_couplingC; 
  TGListBox *_couplingL; 

  TGTextLBEntry *DC_50R; 
  TGTextLBEntry *DC_1M; 
  TGTextLBEntry *AC; 
  
  TGComboBox *_comboB; 
  TGGroupFrame *_comboF; 

  TGTextButton *_setVoltage; 
  TGTextButton *_setCoupling; 

  
  TGMainFrame *_mf; 
  //  TGMainFrame *_otherApp; 

  public: 
  PicoscopeControls(); 
  virtual ~PicoscopeControls(); 
  
  void updateVoltageScale();
    

  
  void updateCoupling(); 


}; 


#endif 












