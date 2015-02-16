#include "PicoscopeControls.h"

#include <iostream> 
#include <vector> 

static const unsigned int win_x = 800; 
static const unsigned int win_y = 600; 

static const unsigned int voltage_y = 100; 


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
  //  gStyle->SetOptStat(kFALSE); 
  std::cout << "Initialising Window" << std::endl; 
  
  //  _otherApp = new TGMainFrame(NULL, 500, 500, kVerticalFrame); 
  //  _otherApp->MapWindow(); 

  _mf = new TGMainFrame(NULL,win_x,win_y, kHorizontalFrame);
  _mf->SetWindowName("PicoScope Controls"); 
  

  // Lisp Generated
  // Voltage Frame 
  setupVoltageEntries(); 
  _voltageF = new TGGroupFrame(_mf, "Voltages", kVerticalFrame);
  _voltageL = new TGListBox(_voltageF);
  _voltageC = new TGLBContainer(_voltageL);
  
  itemSize = _voltages[0]->GetDefaultSize(); 

  for (auto i : _voltages) { 
    _voltageL->AddEntry(i, _hintse); 
  }

  _voltageL->Resize(100, 0*itemSize.fHeight); 
  _voltageF->AddFrame(_voltageL,_hintse); 
  _mf->AddFrame(_voltageF,_hintse);  

  //Coupling Frame 

  _couplingF = new TGGroupFrame(_mf, "Coupling", kHorizontalFrame);
  _couplingL = new TGListBox(_couplingF);
  _couplingC = new TGLBContainer(_couplingL);
  DC_50R = new TGTextLBEntry(_couplingC, new TGString("DC 50 Ohms"), 0); 

  _couplingL->AddEntry(DC_50R, _hintse); 
  DC_1M = new TGTextLBEntry(_couplingC, new TGString("DC"), 1); 
  _couplingL->AddEntry(DC_1M, _hintse); 
  AC = new TGTextLBEntry(_couplingC, new TGString("AC"), 2); 
  _couplingL->AddEntry(AC, _hintse); 
  itemSize = DC_50R->GetDefaultSize(); 
  _couplingL->Resize(100, 6*itemSize.fHeight); 
  _couplingF->AddFrame(_couplingL,_hintse); 
  _couplingL->SetMultipleSelections(false); 
  _setCoupling = new TGTextButton(_couplingF, "&Coupling"); 
  _setCoupling->Connect("Pressed()", "PicoscopeControls", this, "updateCoupling()"); 
  _couplingF->AddFrame(_setCoupling, _hintse); 
  _mf->AddFrame(_couplingF,_hintse); 


  _comboF = new TGGroupFrame(_mf, "Combo Test", kHorizontalFrame);
  _comboB = new TGComboBox(_comboF, 100); 

  for (auto i = 0; i < timeDivs.size(); i++) { 

    _comboB->AddEntry(timeDivs[i], i); 

  }
  _comboB->Resize(150, 20); 
  _comboB->Select(2); 
  _comboF->AddFrame(_comboB, _hintse); 
  _mf->AddFrame(_comboF, _hintse); 

  _mf->MapSubwindows(); 
  _mf->Resize(_mf->GetDefaultSize()); 
  _mf->MapWindow();

}


PicoscopeControls::~PicoscopeControls() { 



}

void PicoscopeControls::setupVoltageEntries() { 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-10mV"), 0)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-20mV"), 1)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-50mV"), 2));
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-100mV"), 3)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-200mV"), 4)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-500mV"), 5)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-1V"), 6)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-2V"), 7)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-5V"), 8)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-10V"), 9));
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-20V"), 10)); 
  _voltages.push_back(new TGTextLBEntry(_voltageC, new TGString("+/-50V"), 11));


}


void PicoscopeControls::updateVoltageScale() { 

  std::cout << "Voltage Scale Changed." << std::endl; 

}

void PicoscopeControls::updateCoupling()  {

    TGTextLBEntry *selected = (TGTextLBEntry *) _couplingL->GetSelectedEntry(); 
    if (selected != NULL)  { 
      std::cout  << selected->GetText()->GetString() << std::endl; 
      
      switch(selected->EntryId()) { 
      case 0: 
	std::cout << "50 Ohms" << std::endl; 
	break; 
      case 1: 
	std::cout << "DC 1 MOhm" << std::endl; 
	break; 
      case 2:
	std::cout << "AC" << std::endl; 
	break; 
      default: 
	break;
      };

    }

}








