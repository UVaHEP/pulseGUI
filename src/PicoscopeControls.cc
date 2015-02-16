#include "PicoscopeControls.h"

#include <iostream> 
#include <vector> 
#include <utility> 


static const unsigned int win_x = 800; 
static const unsigned int win_y = 600; 

static const unsigned int voltage_y = 100; 


static std::vector<std::pair<TString, Int_t> > couplingTypesTest = { 
  std::pair<TString, Int_t>("DC_50R", 50),
  std::pair<TString, Int_t>("DC_1Meg", 1e6),
  std::pair<TString, Int_t>("AC", 0)
}; 
  

static std::vector<TString> couplingTypes = { 
  TString("DC_50R"),
  TString("DC_1Meg"),
  TString("AC")
}; 
 




static std::vector<std::pair<TString, PS6000_RANGE> > voltages = { 
  std::pair<TString, PS6000_RANGE>("+/-10mV", PS6000_10MV),
  std::pair<TString, PS6000_RANGE>("+/-20mV", PS6000_20MV),
  std::pair<TString, PS6000_RANGE>("+/-50mV", PS6000_50MV), 
  std::pair<TString, PS6000_RANGE>("+/-100mV", PS6000_100MV),
  std::pair<TString, PS6000_RANGE>("+/-200mV", PS6000_200MV), 
  std::pair<TString, PS6000_RANGE>("+/-500mV", PS6000_500MV), 
  std::pair<TString, PS6000_RANGE>("+/-1V", PS6000_1V),
  std::pair<TString, PS6000_RANGE>("+/-2V", PS6000_2V), 
  std::pair<TString, PS6000_RANGE>("+/-5V", PS6000_5V), 
  std::pair<TString, PS6000_RANGE>("+/-10V", PS6000_10V),
  std::pair<TString, PS6000_RANGE>("+/-20V", PS6000_20V), 
  std::pair<TString, PS6000_RANGE>("+/-50V", PS6000_50V)

};


/*static std::vector<TString> voltages = { 
TString("+/-10mV"), 
TString("+/-20mV"), 
TString("+/-50mV"), 
TString("+/-100mV"), 
TString("+/-200mV"), 
TString("+/-500mV"), 
TString("+/-1V"), 
TString("+/-2V"), 
TString("+/-5V"), 
TString("+/-10V"), 
TString("+/-20V"), 
TString("+/-50V")
};*/ 


static std::vector<TString> timeDivs = { 
  TString("1 ns"),
  TString("2 ns"),
  TString("5 ns"),
  TString("10 ns"),
  TString("20 ns"),
  TString("50 ns"),
  TString("100 ns"),
  TString("200 ns"),
  TString("500 ns"),
  TString("1 us"),
  TString("2 us"),
  TString("5 us"),
  TString("10 us"),
  TString("20 us"),
  TString("50 us"),
  TString("100 us"),
  TString("200 us"),
  TString("500 us"),
  TString("1 ms"),
  TString("2 ms"),
  TString("5 ms"),
  TString("10 ms"),
  TString("20 ms"),
  TString("50 ms"),
  TString("100 ms"),
  TString("200 ms"),
  TString("500 ms"),
}; 
  
  

PicoscopeControls::PicoscopeControls() { 

  _hintsn = new TGLayoutHints(kLHintsNormal, 2,2,2,2); 
  _hintsy = new TGLayoutHints(kLHintsNormal|kLHintsExpandY, 2,2,2,2); 
  TGLayoutHints *_hintse = new TGLayoutHints(kLHintsTop | kLHintsLeft |
					     kLHintsExpandX | kLHintsExpandY, 
					     5, 5, 5, 5); 

  TGDimension itemSize; 
  std::cout << "Initialising Window" << std::endl; 
  
  _mf = new TGMainFrame(NULL,win_x,win_y, kHorizontalFrame);
  _mf->SetWindowName("PicoScope Controls"); 
  

  // Voltage Frame 
  _voltageF = new TGGroupFrame(_mf, "Voltages", kHorizontalFrame); 
  _voltageB = new TGComboBox(_voltageF, 100); 
  
  for (auto i = 0; i < voltages.size(); i++) { 
    _voltageB->AddEntry(voltages[i].first, i); 
  }

  _voltageB->Resize(150, 20); 
  _voltageB->Select(1); 
  _voltageB->Connect("Selected(Int_t)", "PicoscopeControls", this, "voltageHandler(Int_t , Int_t)"); 
  _voltageF->AddFrame(_voltageB, _hintse); 
  _mf->AddFrame(_voltageF,_hintse);  

  //Coupling Frame 

  _couplingF = new TGGroupFrame(_mf, "Coupling", kHorizontalFrame);
  _couplingB = new TGComboBox(_couplingF, 100);
  _couplingB->Connect("Selected(Int_t)", "PicoscopeControls", this, "couplingHandler(Int_t , Int_t)"); 
  for (auto i = 0; i < couplingTypesTest.size(); i++) { 
    _couplingB->AddEntry(couplingTypesTest[i].first, i); 
  }
  
  _couplingB->Resize(150, 20); 
  _couplingB->Select(1); 

  _couplingF->AddFrame(_couplingB,_hintse); 

  _mf->AddFrame(_couplingF,_hintse); 


  _timeF = new TGGroupFrame(_mf, "Time Div", kHorizontalFrame);
  _timeB = new TGComboBox(_timeF, 100); 
  _timeB->Connect("Selected(Int_t)", "PicoscopeControls", this, "timedivHandler(Int_t , Int_t)"); 
  for (auto i = 0; i < timeDivs.size(); i++) { 

    _timeB->AddEntry(timeDivs[i], i); 

  }
  _timeB->Resize(150, 20); 
  _timeB->Select(1); 
  _timeF->AddFrame(_timeB, _hintse); 
  _mf->AddFrame(_timeF, _hintse); 

  _mf->MapSubwindows(); 
  _mf->Resize(_mf->GetDefaultSize()); 
  _mf->MapWindow();

}


PicoscopeControls::~PicoscopeControls() { 



}



void PicoscopeControls::voltageHandler(Int_t selection, Int_t widgetID) { 

  std::cout << voltages[selection].first << std::endl; 
  
}

void PicoscopeControls::couplingHandler(Int_t selection, Int_t widgetID)  {
  std::cout << couplingTypesTest[selection].second << std::endl; 

}

void PicoscopeControls::timedivHandler(Int_t selection, Int_t widgetID) {


  std::cout << timeDivs[selection] << std::endl; 


}













