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
#include "TStyle.h"
#include "TCanvas.h"
#include "TGFrame.h"

class PicoscopeControls : public TQObject  { 

  RQ_OBJECT("PicoscopeControls"); 
  

  private:


  TGVerticalFrame *_options; 
  TGWindow *_w; 
  TGMainFrame *_mf; 
  
  

  public: 
  PicoscopeControls(); 
  virtual ~PicoscopeControls(); 
  


}; 


#endif 
