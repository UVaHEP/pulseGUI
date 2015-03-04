#include "PicoscopeControls.h"


#include <libps6000-1.4/ps6000Api.h>
#include <iostream> 
#include <vector> 
#include <utility> 


static const unsigned int win_x = 800; 
static const unsigned int win_y = 600; 

static const unsigned int voltage_y = 100; 



static std::vector<std::pair<TString, PS6000_COUPLING> > couplingTypes = { 
  std::pair<TString, PS6000_COUPLING>("DC_50R", PS6000_DC_50R),
  std::pair<TString, PS6000_COUPLING>("DC_1Meg", PS6000_DC_1M),
  std::pair<TString, PS6000_COUPLING>("AC", PS6000_AC)
}; 

static std::vector<std::pair<TString, PS6000_CHANNEL> > channels = { 
  std::pair<TString, PS6000_CHANNEL>("A", PS6000_CHANNEL_A),
  std::pair<TString, PS6000_CHANNEL>("B", PS6000_CHANNEL_B),
  std::pair<TString, PS6000_CHANNEL>("C", PS6000_CHANNEL_C),
  std::pair<TString, PS6000_CHANNEL>("D", PS6000_CHANNEL_D)
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


static std::vector<std::pair<TString, unsigned long> > timeDivs = { 
  std::pair<TString, unsigned long>("1 ns", 1),
  std::pair<TString, unsigned long>("2 ns", 2),
  std::pair<TString, unsigned long>("5 ns", 5),
  std::pair<TString, unsigned long>("10 ns", 10),
  std::pair<TString, unsigned long>("20 ns", 20),
  std::pair<TString, unsigned long>("50 ns", 50),
  std::pair<TString, unsigned long>("100 ns", 100),
  std::pair<TString, unsigned long>("200 ns", 200),
  std::pair<TString, unsigned long>("500 ns", 500),
  std::pair<TString, unsigned long>("1 us", 1000),
  std::pair<TString, unsigned long>("2 us", 2e3),
  std::pair<TString, unsigned long>("5 us", 5e3),
  std::pair<TString, unsigned long>("10 us", 10e3),
  std::pair<TString, unsigned long>("20 us", 20e3),
  std::pair<TString, unsigned long>("50 us", 50e3),
  std::pair<TString, unsigned long>("100 us",100e3),
  std::pair<TString, unsigned long>("200 us",200e3),
  std::pair<TString, unsigned long>("500 us",500e3),
  std::pair<TString, unsigned long>("1 ms", 1e6),
  std::pair<TString, unsigned long>("2 ms", 2e6),
  std::pair<TString, unsigned long>("5 ms", 5e6),
  std::pair<TString, unsigned long>("10 ms", 10e6),
  std::pair<TString, unsigned long>("20 ms", 20e6),
  std::pair<TString, unsigned long>("50 ms", 50e6),
  std::pair<TString, unsigned long>("100 ms", 100e6),
  std::pair<TString, unsigned long>("200 ms", 200e6),
  std::pair<TString, unsigned long>("500 ms", 500e6)
}; 
  
  

PicoscopeControls::PicoscopeControls() { 

  _ps.open(); 


  _hintsn = new TGLayoutHints(kLHintsNormal, 2,2,2,2); 
  _hintsy = new TGLayoutHints(kLHintsNormal|kLHintsExpandY, 2,2,2,2); 
  TGLayoutHints *_hintse = new TGLayoutHints(kLHintsTop | kLHintsLeft |
					     kLHintsExpandX | kLHintsExpandY, 
					     5, 5, 5, 5); 

  TGDimension itemSize; 
  std::cout << "Initialising Window" << std::endl; 
  
  _mf = new TGMainFrame(NULL,win_x,win_y, kHorizontalFrame);
  _mf->SetWindowName("PicoScope Controls"); 
  

  //Channel Frame
  _channelF = new TGGroupFrame(_mf, "Channels", kHorizontalFrame); 
  _channelB = new TGComboBox(_channelF, 100); 

  for (auto i = 0; i < channels.size(); i++) { 
    _channelB->AddEntry(channels[i].first, i); 
  }

  _channelB->Resize(150, 20); 
  _channelB->Select(1); 
  _channelB->Connect("Selected(Int_t)", "PicoscopeControls", this, "channelHandler(Int_t , Int_t)"); 
  _channelF->AddFrame(_channelB, _hintse); 
  _mf->AddFrame(_channelF,_hintse);  
  

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
  for (auto i = 0; i < couplingTypes.size(); i++) { 
    _couplingB->AddEntry(couplingTypes[i].first, i); 
  }
  
  _couplingB->Resize(150, 20); 
  _couplingB->Select(1); 

  _couplingF->AddFrame(_couplingB,_hintse); 

  _mf->AddFrame(_couplingF,_hintse); 


  _timeF = new TGGroupFrame(_mf, "Time Div", kHorizontalFrame);
  _timeB = new TGComboBox(_timeF, 100); 
  _timeB->Connect("Selected(Int_t)", "PicoscopeControls", this, "timedivHandler(Int_t , Int_t)"); 
  for (auto i = 0; i < timeDivs.size(); i++) { 

    _timeB->AddEntry(timeDivs[i].first, i); 

  }
  _timeB->Resize(150, 20); 
  _timeB->Select(1); 
  _timeF->AddFrame(_timeB, _hintse); 
  _mf->AddFrame(_timeF, _hintse); 

  _sampleNumber = new TGNumberEntry(_mf, 1, 5, 5, TGNumberFormat::kNESInteger); 
  _sampleNumber->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "sampleNumberHandler(Long_t)"); 
  _mf->AddFrame(_sampleNumber, _hintse); 


  TGLabel *sampleLabel = new TGLabel(_mf, TGString("# Samples")); 
  _mf->AddFrame(sampleLabel, _hintse); 

  // Test Run Button
  _runBtn = new TGTextButton(_mf, TGHotString("Capture Block!"), 5); 
  _runBtn->Connect("Clicked()", "PicoscopeControls", this, "testRun()"); 
  _mf->AddFrame(_runBtn, _hintse); 
  _mf->MapSubwindows(); 
  _mf->Resize(_mf->GetDefaultSize()); 
  _mf->MapWindow();

}


PicoscopeControls::~PicoscopeControls() { 

  _ps.close(); 

}

void PicoscopeControls::channelHandler(Int_t selection, Int_t widgetID) {
  PICO_STATUS status; 
  TGLBEntry *box = _channelB->GetSelectedEntry(); 
  if (_ps.Channel(channels[selection].second).enabled()) {
    status = _ps.disableChannel(channels[selection].second); 
    box->SetBackgroundColor(0xffffff); 
  }
  else {
    status = _ps.enableChannel(channels[selection].second); 
    box->SetBackgroundColor(0x00ff00); 
  }

  if (status  != PICO_OK) { 
    std::cout << "Channel failed to get enabled:" << channels[selection].first << std::endl; 
  }

}

void PicoscopeControls::voltageHandler(Int_t selection, Int_t widgetID) { 

  std::cout << voltages[selection].first << " " << _ps.Channel(channels[_channelB->GetSelected()].second).name() << std::endl; 

  picoscope::channel ch = _ps.Channel(channels[_channelB->GetSelected()].second); 
  ch.setChannelRange(voltages[selection].second); 

  PICO_STATUS status = _ps.setAndEnableChannel(ch); 
  if (status != PICO_OK) 
    std::cout << "Error setting or enabling channel " << channels[_channelB->GetSelected()].first << std::endl; 


  
}

void PicoscopeControls::couplingHandler(Int_t selection, Int_t widgetID)  {
  std::cout << couplingTypes[selection].first << std::endl; 
  picoscope::channel ch = _ps.Channel(channels[_channelB->GetSelected()].second); 
  ch.setChannelCoupling(couplingTypes[selection].second); 
  PICO_STATUS status = _ps.setAndEnableChannel(ch); 

  if (status  != PICO_OK) { 
    std::cout << "Failed enabling channel:" << channels[_channelB->GetSelected()].first << std::endl; 
  }


}


void PicoscopeControls::sampleNumberHandler(Long_t val) { 

  //  std::cout << "number of samples to capture changed" << std::endl; 

}

void PicoscopeControls::timedivHandler(Int_t selection, Int_t widgetID) {
  //I'm working off the assumption of capturing 10 divisions similar 
  //to an oscilloscope. There's no reason to keep limited to this range
  //so consider making this more adaptable in the future. 

  unsigned int divisions = 10; 
  
  std::cout << timeDivs[selection].first << std::endl; 


}

void PicoscopeControls::testRun() { 
  int32_t sampleCount = 50000; 
  std::cout << "Taking a run and then closing the picoscope." << std::endl; 

  _ps.takeRun(sampleCount); 

}











