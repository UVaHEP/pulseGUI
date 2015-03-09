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


PicoscopeControls::PicoscopeControls() { 

  _ps.open(); 
  _buffers = new TList(); 

  _hintsn = new TGLayoutHints(kLHintsNormal, 2,2,2,2); 
  _hintsy = new TGLayoutHints(kLHintsNormal|kLHintsExpandY, 2,2,2,2); 
  TGLayoutHints *_hintse = new TGLayoutHints(kLHintsTop | kLHintsLeft |
					     kLHintsExpandX | kLHintsExpandY, 
					     5, 5, 5, 5); 



  TGDimension itemSize; 
  std::cout << "Initialising Window" << std::endl; 
  
  _mf = new TGMainFrame(NULL,win_x,win_y, kHorizontalFrame);
  _mf->SetWindowName("PicoScope Controls"); 

  //Would prefer to have colored text indicating connected/disconnected status
  TGString connected = TGString("Picoscope\nNot connected"); 
  _connectionStatus = new TGLabel(_mf, connected); 
  _connectionStatus->SetTextColor(0xFF0000); 
  if (_ps.connected()) {
    _connectionStatus->SetText(TGString("Picoscope\nConnected")); 
    _connectionStatus->SetTextColor(0x008000); 
  }
    

  _mf->AddFrame(_connectionStatus, _hintse); 
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

  //Sampling Interval
  _sampleInterval = new TGNumberEntry(_mf, 0, 5, 5, TGNumberFormat::kNESInteger); 
  _sampleInterval->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "sampleIntervalHandler(Long_t)"); 
  _mf->AddFrame(_sampleInterval, _hintse); 

  _intervalLabel = new TGLabel(_mf, TGString(prettyPrintInterval())); 
  
  _mf->AddFrame(_intervalLabel, _hintse); 
  

  //Sampling Number
  _sampleNumber = new TGNumberEntry(_mf, 1e4, 5, 5, TGNumberFormat::kNESInteger); 
  _sampleNumber->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "sampleNumberHandler(Long_t)"); 
  _mf->AddFrame(_sampleNumber, _hintse); 


  TGLabel *sampleLabel = new TGLabel(_mf, TGString("# Samples")); 
  _mf->AddFrame(sampleLabel, _hintse); 

  _windowLabel = new TGLabel(_mf, TGString(prettyPrintWindow())); 
  _mf->AddFrame(_windowLabel, _hintse); 
  

  // Test Run Button
  _runBtn = new TGTextButton(_mf, TGHotString("Capture Block!"), 5); 
  _runBtn->Connect("Clicked()", "PicoscopeControls", this, "testRun()"); 

  //Trigger Button and Entry
  _triggerLevel = new TGNumberEntry(_mf, 0, 5, 5, TGNumberFormat::kNESInteger); 
  _triggerLevel->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "triggerLevelHandler(Long_t)"); 
  _mf->AddFrame(_triggerLevel, _hintse); 

  _triggerLabel = new TGLabel(_mf, TGString("Trigger Level")); 
  _mf->AddFrame(_triggerLabel, _hintse); 
  _triggerEnableBtn = new TGTextButton(_mf, TGHotString("Trigger Off"), 5); 
  _triggerEnableBtn->Connect("Clicked()", "PicoscopeControls", this, "triggerButton()"); 
  _mf->AddFrame(_triggerEnableBtn, _hintse); 

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


void PicoscopeControls::sampleIntervalHandler(Long_t val) { 

  picoscope::samplingSettings s = _ps.sampling(); 
  s.setTimebase(_sampleInterval->GetIntNumber()); 
  _intervalLabel->SetText(prettyPrintInterval()); 
  _windowLabel->SetText(prettyPrintWindow()); 
  //  std::cout << "number of samples to capture changed" << std::endl; 

}


void PicoscopeControls::sampleNumberHandler(Long_t val) { 
  picoscope::samplingSettings s = _ps.sampling(); 
  s.setSampleNumber(_sampleNumber->GetIntNumber()); 
  _windowLabel->SetText(prettyPrintWindow()); 
  //  std::cout << "number of samples to capture changed" << std::endl; 

}





void PicoscopeControls::testRun() { 
  
  picoscope::samplingSettings &s = _ps.sampling(); 
  s.setOversample(0); 
  s.setTimebase(_sampleInterval->GetIntNumber()); 
  s.setSegment(1); 
  s.setSampleNumber(_sampleNumber->GetIntNumber()); 
  
  std::cout << "Taking a run and then closing the picoscope." << std::endl; 
  std:: cout << "test Run ps address:" << &_ps << std::endl; 
  std::cout << "Timebase\tSampleNumber\n" << _sampleInterval->GetIntNumber() << "\t" << _sampleNumber->GetIntNumber() << std::endl; 

  _ps.setTrigger(500, PS6000_FALLING); 

  

  /*This is pretty ugly, here's an explanation: 
    I need a pointer to a function to pass in as a callback, so I'm creating a simple lambda that calls the member function callBack of our 
    local PicoscopeControls object. I have to wrap it in the std::function initializer so that it'll get allocated on the heap rather than 
    on the stack. Hence the ugliness. 
   */
  std::function<void(picoscope::picoscopeData *)> *cB = new std::function<void(picoscope::picoscopeData *)>([this](picoscope::picoscopeData *d) { callBack(d);}); 
  _ps.setCallback(cB); 
  _ps.takeRun();

}

void PicoscopeControls::callBack(picoscope::picoscopeData *data) { 


      PSbuffer *ps = new PSbuffer(); 

      //Most of these objects are probably getting copied around, so after you get it working think about what's going on and make it nicer!

      picoscope::channel waveformSettings = data->first[PS6000_CHANNEL_A].second; 
      picoscope::channel triggerSettings =  data->first[PS6000_CHANNEL_B].second; 
      std::vector<int16_t> *waveform = data->first[PS6000_CHANNEL_A].first; 
      std::vector<int16_t> *trigger = data->first[PS6000_CHANNEL_B].first; 
      picoscope::samplingSettings timeSettings = data->second; 


      /*      for (unsigned int i = 0; i < 2000; i++) { 
	int32_t mV = picoscope::adcToMv(waveform->at(i), waveformSettings.range()); 
	}*/

      std::cout << "Waveform count:" << waveform->size() << " Waveform Range:" << waveformSettings.range() << std::endl; 
      std::cout << "Trigger count:" << trigger->size() << " Trigger Range:" << triggerSettings.range() << std::endl; 


      ps->SetT0(0); 
      ps->SetDt(timeSettings.TimeIntervalNS()); 
      //Fill waveform with data 


      ps->InitWaveform(waveform->size()); 
      TH1F* wave = ps->GetWaveform(); 

      Float_t dV = 1e12; 
      float delta = 0; 
      float last = 0; 
      for (unsigned int i = 0; i < waveform->size(); i++) {

	int32_t mV = picoscope::adcToMv(waveform->at(i), waveformSettings.range()); 
	delta = TMath::Abs(mV-last); 

	if (delta > 1e-6 && delta < dV) dV = delta; 

	last = mV; 
	wave->SetBinContent(i+1, mV); 

      }

      ps->SetDV(dV); 
      // get channel B (trigger)
      float min=1e12;
      float max=-1e12;
      vector<float> *vtrig=new vector<float>;
    
      // set trigger point and hysterisis
      // WARNING!  This may fail in case of bad terminations
      // *** TODO:  Use t0 vs t=0  to find 1st trigger bin and gte threshold there ***
      Float_t onThreshold = max*0.5; 
      Float_t offThreshold = max*0.3; 
      std::cout << "On Threshold:"   << onThreshold 
      << " Off Threshold:" << offThreshold << std::endl;

      Bool_t fired = false;
      // mark triggers
      for (UInt_t i = 0; i < trigger->size(); i++) {
	int32_t mV = picoscope::adcToMv(trigger->at(i), triggerSettings.range()); 
	if (!fired && mV>onThreshold) {
	  fired = true;
	  ps->AddTrig(i);
	  continue;
	}
      }
      ps->Analyze();  // calculate DC offset, frequency spectrum, noise, etc*/
      ps->Print(); 
      

      _buffers->AddLast(ps); 

      delete data; 

      /*      TFile f2("psbuffertest.root", "RECREATE");
      ps->Write(); 
      f2.Write(); 
      f2.Close(); */





}


TString PicoscopeControls::prettyPrintWindow() { 

  float timebase = _ps.calculateTimebase(_sampleInterval->GetIntNumber()); 
  float result = timebase*_sampleNumber->GetIntNumber(); 
  double lg = TMath::Abs(TMath::Log10(result)); 
  TString units = "";
  bool set = false;  
  float timebaseCorrected = result; 
   if (lg > 9.0) { 
     timebaseCorrected *= 1e12; 
     units = "ps";
     set = true; 
   }
   else if (lg > 6.0 && !set) { 
     timebaseCorrected *= 1e9; 
     units = "ns"; 
     set = true; 
   }
   else if (lg > 3.0 && !set) { 
     timebaseCorrected *= 1e6; 
     units = "us"; 
     set = true; 
   }
   else if (timebase < 1.0 && !set) { 
     timebaseCorrected *= 1e3; 
     units = "ms"; 
     set = true; 
   }
   else { 
     units = "seconds"; 
     set = true; 
   }


   return TString::Format("%f %s", result,units.Data()); 

}

TString PicoscopeControls::prettyPrintInterval() { 

   float timebase = _ps.calculateTimebase(_sampleInterval->GetIntNumber()); 
   double lg = TMath::Abs(TMath::Log10(timebase)); 
   TString units = ""; 
   bool set = false; 
   //   std::cout << "Original value:" << timebase << " Log value:" << lg << std::endl; 
   float timebaseCorrected = timebase; 
   if (lg > 9.0) { 
     timebaseCorrected *= 1e12; 
     units = "ps";
     set = true; 
   }
   else if (lg > 6.0 && !set) { 
     timebaseCorrected *= 1e9; 
     units = "ns"; 
     set = true; 
   }
   else if (lg > 3.0 && !set) { 
     timebaseCorrected *= 1e6; 
     units = "us"; 
     set = true; 
   }
   else if (timebase < 1.0 && !set) { 
     timebaseCorrected *= 1e3; 
     units = "ms"; 
     set = true; 
   }
   else { 
     units = "seconds"; 
     set = true; 
   }

   //   std::cout << TString::Format("%.1f %s \n", timebaseCorrected, units.Data()); 
   
   
   return TString::Format("%.1f %s \n", timebaseCorrected, units.Data()); 

 }


void PicoscopeControls::triggerButton() { 



  TString btn = _triggerEnableBtn->GetString(); 
  std::cout << "Trigger button text:" << btn << std::endl; 
  if (btn.Contains("Trigger Off")) {
    std::cout << "Enabling Trigger" << std::endl;     
    _triggerEnableBtn->SetText("Trigger On"); 
    _ps.setTrigger(_triggerLevel->GetIntNumber(), PS6000_FALLING); 

  }
  else {
    std::cout << "Disabling Trigger" << std::endl;     
    _triggerEnableBtn->SetText("Trigger Off"); 
    _ps.disableTrigger(); 
  }    
  



}

void PicoscopeControls::triggerLevelHandler(Long_t val) {

  std::cout << "Trigger Level:" << _triggerLevel->GetIntNumber() << std::endl; 

}




