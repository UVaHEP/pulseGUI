#ifdef CONTROLS

#include "PicoscopeControls.h"


#include <libps6000-1.4/ps6000Api.h>
#include <iostream> 
#include <vector> 
#include <utility> 


static const unsigned int win_x = 1000; 
static const unsigned int win_y = 50; 

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
  _previewF = NULL; 
  _playBtn = NULL; 
  _iter = NULL; 
  _timer = new TTimer(); 
  _cbTimer = new TTimer(); 
  _runs = 1; 

  _hintsn = new TGLayoutHints(kLHintsNormal, 2,2,2,2); 
  _hintsx = new TGLayoutHints(kLHintsExpandX, 0,2,2,2); 
  _hintsy = new TGLayoutHints(kLHintsNormal|kLHintsExpandY, 2,2,2,2); 
  TGLayoutHints *_hintse = new TGLayoutHints(kLHintsTop | kLHintsLeft |
					     kLHintsExpandX | kLHintsExpandY, 
					     5, 5, 5, 5); 



  TGDimension itemSize; 
  std::cout << "Initialising Window" << std::endl; 
  
  _mf = new TGMainFrame(NULL,win_x,win_y, kHorizontalFrame);
  _mf->SetWindowName("PicoScope Controls"); 

  _channelStatusF = new TGGroupFrame(_mf, "Status and Channels", kHorizontalFrame); 

  TGString connected = TGString("Picoscope\nNot connected"); 
  _connectionStatus = new TGLabel(_channelStatusF, connected); 
  _connectionStatus->SetTextColor(0xFF0000); 
  if (_ps.connected()) {
    _connectionStatus->SetText(TGString("Picoscope\nConnected")); 
    _connectionStatus->SetTextColor(0x008000); 
  }

  _channelStatusF->AddFrame(_connectionStatus, _hintse); 


  //Channel Frame

  _channels = new TGVButtonGroup(_channelStatusF, "Channels"); 
  for (auto i = 0; i < channels.size(); i++) { 
    auto channel = new TGRadioButton(_channels, TGHotString(channels[i].first), i); 
  }
  _channels->SetButton(0); 
  _selectedChannel = PS6000_CHANNEL_A; 
  _channels->Connect("Clicked(Int_t)", "PicoscopeControls", this, "channelHandler(Int_t , Int_t)"); 
  _channels->DrawBorder(); 
  _channelStatusF->AddFrame(_channels, _hintsn); 
  
  _channelStatusF->MapSubwindows(); 
  _channelStatusF->Resize(_channelStatusF->GetDefaultSize()); 
  _mf->AddFrame(_channelStatusF, _hintsn); 
  
  _voltageCouplingF = new TGGroupFrame(_mf, "Voltage and Coupling"); 

  // Voltage Frame 
  _voltageF = new TGGroupFrame(_voltageCouplingF, "Voltages", kHorizontalFrame); 
  _voltageB = new TGComboBox(_voltageF, 100); 
  
  _voltageB->AddEntry("Off", -1); 
  
  for (auto i = 0; i < voltages.size(); i++) { 
    _voltageB->AddEntry(voltages[i].first, i); 
  }

  _voltageB->Resize(150, 20); 
  _voltageB->Select(1); 
  _voltageB->Connect("Selected(Int_t)", "PicoscopeControls", this, "voltageHandler(Int_t , Int_t)"); 
  _voltageF->AddFrame(_voltageB, _hintse); 
  _voltageCouplingF->AddFrame(_voltageF,_hintse);  

  //Coupling Frame 

  _couplingF = new TGGroupFrame(_voltageCouplingF, "Coupling", kHorizontalFrame);
  _couplingB = new TGComboBox(_couplingF, 100);
  _couplingB->Connect("Selected(Int_t)", "PicoscopeControls", this, "couplingHandler(Int_t , Int_t)"); 
  for (auto i = 0; i < couplingTypes.size(); i++) { 
    _couplingB->AddEntry(couplingTypes[i].first, i); 
  }
  
  _couplingB->Resize(150, 20); 
  _couplingB->Select(1); 

  _couplingF->AddFrame(_couplingB,_hintse); 

  _voltageCouplingF->AddFrame(_couplingF,_hintse); 

  _mf->AddFrame(_voltageCouplingF, _hintsn); 

  _timeSamplingF = new TGGroupFrame(_mf, "Time and Sampling"); 

  //Sampling Interval
  _sampleInterval = new TGNumberEntry(_timeSamplingF, 2, 8, 5, TGNumberFormat::kNESInteger); 
  _sampleInterval->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "sampleIntervalHandler(Long_t)"); 
  _timeSamplingF->AddFrame(_sampleInterval, _hintsn); 

  _intervalLabel = new TGLabel(_timeSamplingF, TGString(prettyPrintInterval())); 
  
  _timeSamplingF->AddFrame(_intervalLabel, _hintsn); 
  

  //Sampling Number

  _preTriggerNumber = new TGNumberEntry(_timeSamplingF, 0, 8, 5, TGNumberFormat::kNESInteger); 
  _preTriggerNumber->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "preTriggerHandler(Long_t)"); 
  _timeSamplingF->AddFrame(_preTriggerNumber, _hintsn); 
  TGLabel *preTrigLabel = new TGLabel(_timeSamplingF, TGString("Pre-trigger Samples")); 
  _timeSamplingF->AddFrame(preTrigLabel, _hintsn); 

  _postTriggerNumber = new TGNumberEntry(_timeSamplingF, 1e4, 8, 5, TGNumberFormat::kNESInteger); 
  _postTriggerNumber->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "postTriggerHandler(Long_t)"); 
  _timeSamplingF->AddFrame(_postTriggerNumber, _hintsn); 




  TGLabel *sampleLabel = new TGLabel(_timeSamplingF, TGString("Post-Trigger Samples")); 
  _timeSamplingF->AddFrame(sampleLabel, _hintsx); 

  _windowLabel = new TGLabel(_timeSamplingF, TGString(prettyPrintWindow())); 
  _timeSamplingF->AddFrame(_windowLabel, _hintsn); 


  //Baseline capture 
  _DCOffset = new TGNumberEntry(_timeSamplingF, 0.0, 8, 20, TGNumberFormat::kNESReal); 
  _DCOffset->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "DCOffsetHandler(Long_t)"); 
  _timeSamplingF->AddFrame(_DCOffset, _hintsn); 
  _captureBaseline = new TGTextButton(_timeSamplingF, TGHotString("Capture Baseline"), 4); 
  _captureBaseline->Connect("Clicked()", "PicoscopeControls", this, "captureBaseline()"); 
  _timeSamplingF->AddFrame(_captureBaseline, _hintsx); 

  _mf->AddFrame(_timeSamplingF, _hintsy); 





  _triggerRunF = new TGGroupFrame(_mf, "Trigger and Run", kVerticalFrame); 
  // Test Run Button

  _runBtn = new TGTextButton(_triggerRunF, TGHotString("Run"), 4); 
  _runBtn->Connect("Clicked()", "PicoscopeControls", this, "testRun()"); 
  _triggerRunF->AddFrame(_runBtn, _hintsn); 
  _numberRuns = new TGNumberEntry(_triggerRunF, 1, 4, 8, TGNumberFormat::kNESInteger); 
  _numberRuns->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "runNumberHandler(Long_t)"); 
  _triggerRunF->AddFrame(_numberRuns, _hintsn); 
  _numberRunsLabel = new TGLabel(_triggerRunF, TGString("# Blocks")); 
  _triggerRunF->AddFrame(_numberRunsLabel, _hintsn); 
  
  
  //Trigger Button and Entry
  _triggerLevel = new TGNumberEntry(_triggerRunF, 0, 5, 5, TGNumberFormat::kNESInteger); 
  _triggerLevel->Connect("ValueSet(Long_t)", "PicoscopeControls", this, "triggerLevelHandler(Long_t)"); 
  _triggerRunF->AddFrame(_triggerLevel, _hintsn); 

  _triggerLabel = new TGLabel(_triggerRunF, TGString("Trigger Level")); 
  _triggerRunF->AddFrame(_triggerLabel, _hintsn); 
  _triggerEnableBtn = new TGTextButton(_triggerRunF, TGHotString("Trigger Off"), 5); 
  _triggerEnableBtn->Connect("Clicked()", "PicoscopeControls", this, "triggerButton()"); 
  _triggerRunF->AddFrame(_triggerEnableBtn, _hintsn); 

  _triggerRunF->AddFrame(_runBtn, _hintsn); 

  _writeBuffersBtn = new TGTextButton(_triggerRunF, TGHotString("Write Buffers to Disk"), 6); 
  _writeBuffersBtn->Connect("Clicked()", "PicoscopeControls", this, "writeBuffersToDisk()"); 
  _triggerRunF->AddFrame(_writeBuffersBtn, _hintsn); 


  _clearBtn = new TGTextButton(_triggerRunF, TGHotString("Clear Buffers"), 10); 
  _clearBtn->Connect("Clicked()", "PicoscopeControls", this, "clearBuffers()"); 
  _triggerRunF->AddFrame(_clearBtn, _hintsn); 
  
  _previewBtn = new TGTextButton(_triggerRunF, TGHotString("Preview Buffers"), 7); 
  _previewBtn->Connect("Clicked()", "PicoscopeControls", this, "preview()"); 
  _triggerRunF->AddFrame(_previewBtn, _hintsn); 

  
  _mf->AddFrame(_triggerRunF, _hintsy); 
  _mf->Resize(_mf->GetDefaultSize()); 
  _mf->MapSubwindows(); 
  _mf->MapWindow();



}


PicoscopeControls::~PicoscopeControls() { 

  _ps.close(); 

}

void PicoscopeControls::channelHandler(Int_t selection, Int_t widgetID) {

  //std::cout << "Channel Selection:" << selection << " Widget: " << widgetID << std::endl; 
  _selectedChannel = channels[selection].second; 

  /* 
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
    }*/

}

void PicoscopeControls::voltageHandler(Int_t selection, Int_t widgetID) { 
  PICO_STATUS status; 
  if (selection == -1) { 
    //Off case 
    status = _ps.disableChannel(channels[_selectedChannel].second); 
    if (status != PICO_OK) 
      std::cout << "Error disabling channel " << channels[_selectedChannel].first << std::endl; 

    return; 

  }
  std::cout << voltages[selection].first << " " << _ps.Channel(channels[_selectedChannel].second).name() << std::endl; 

  picoscope::channel ch = _ps.Channel(channels[_selectedChannel].second); 

  ch.setRange(voltages[selection].second); 

  _ps.setChannelSettings(ch); 
  status = _ps.enableChannel(ch.name()); 
  if (status != PICO_OK) 
    std::cout << "Error setting or enabling channel " << channels[_selectedChannel].first << std::endl; 


  
}

void PicoscopeControls::couplingHandler(Int_t selection, Int_t widgetID)  {

  picoscope::channel ch = _ps.Channel(channels[_selectedChannel].second); 
  std::cout << couplingTypes[selection].first << " Ch:" << ch.name() << std::endl; 
  ch.setCoupling(couplingTypes[selection].second); 
  _ps.setChannelSettings(ch); 
  PICO_STATUS status = _ps.enableChannel(ch.name()); 
  if (status  != PICO_OK) { 
    std::cout << "Failed enabling channel:" << channels[_selectedChannel].first << std::endl; 
  }


}


void PicoscopeControls::sampleIntervalHandler(Long_t val) { 

  picoscope::samplingSettings s = _ps.sampling(); 
  s.setTimebase(_sampleInterval->GetIntNumber()); 
  _intervalLabel->SetText(prettyPrintInterval()); 
  _windowLabel->SetText(prettyPrintWindow()); 
  //  std::cout << "number of samples to capture changed" << std::endl; 

}


void PicoscopeControls::postTriggerHandler(Long_t val) { 
  picoscope::samplingSettings s = _ps.sampling(); 
  s.setPostTriggerSamples(_postTriggerNumber->GetIntNumber()); 
  _windowLabel->SetText(prettyPrintWindow()); 
  //  std::cout << "number of samples to capture changed" << std::endl; 

}

void PicoscopeControls::preTriggerHandler(Long_t val) { 
  picoscope::samplingSettings s = _ps.sampling(); 
  s.setPreTriggerSamples(_preTriggerNumber->GetIntNumber()); 
  _windowLabel->SetText(prettyPrintWindow()); 



}



void PicoscopeControls::testRun() { 
  
  picoscope::samplingSettings &s = _ps.sampling(); 
  s.setOversample(0); 
  s.setTimebase(_sampleInterval->GetIntNumber()); 
  s.setSegment(1); 
  s.setPostTriggerSamples(_postTriggerNumber->GetIntNumber()); 
  s.setPreTriggerSamples(_preTriggerNumber->GetIntNumber()); 
  
  /*  std::cout << "Taking a run and then closing the picoscope." << std::endl; 
  std:: cout << "test Run ps address:" << &_ps << std::endl; 
  std::cout << "Timebase\tSampleNumber\n" << _sampleInterval->GetIntNumber() << "\t" << _sampleNumber->GetIntNumber() << std::endl; */
  _ps.enableBandwidthLimit(PS6000_CHANNEL_A); 
  _ps.enableChannel(PS6000_CHANNEL_A); 

  /*This is pretty ugly, here's an explanation: 
    I need a pointer to a function to pass in as a callback, so I'm creating a simple lambda that calls the member function callBack of our 
    local PicoscopeControls object. I have to wrap it in the std::function initializer so that it'll get allocated on the heap rather than 
    on the stack. Hence the ugliness. 
   */
  std::function<void(picoscope::picoscopeData *)> *cB = new std::function<void(picoscope::picoscopeData *)>([this](picoscope::picoscopeData *d) { callBack(d);}); 
  _ps.setCallback(cB); 

  _ps.takeRun();

}


PSbuffer *PicoscopeControls::bufferBuilder(int16_t *waveform, int16_t *trigger, uint32_t nsamples, PS6000_RANGE waverange, PS6000_RANGE trigRange, Double_t t0, Double_t dT, Double_t dcOffset) { 

  PSbuffer *ps = new PSbuffer(); 


  ps->SetT0(t0); 
  std::cout << "Time interval ns: " << dT << std::endl; 
  ps->SetDt(dT); 

  ps->InitWaveform(nsamples); 
  TH1F* wave = ps->GetWaveform(); 
  Float_t dV = 1e12; 
  ps->SetDCOffset(dcOffset); 
  float delta = 0; 
  float last = 0; 
  for (unsigned int i = 0; i < nsamples; i++) {

    int32_t mV = picoscope::adcToMv(waveform[i], waverange); 
    delta = TMath::Abs(mV-last); 
    if (delta > 1e-6 && delta < dV) dV = delta; 

    last = mV; 
    wave->SetBinContent(i+1, mV); 

  }

  ps->SetDV(dV); 


  // get channel B (trigger)
  if (trigger) { 
    float min=1e12;
    float max=-1e12;
    vector<float> *vtrig=new vector<float>;
    
    // set trigger point and hysterisis
    // WARNING!  This may fail in case of bad terminations
    // *** TODO:  Use t0 vs t=0  to find 1st trigger bin and gte threshold there ***


    // mark triggers
    Float_t mV; 
    for (UInt_t i = 0; i < nsamples; i++) {

      mV = picoscope::adcToMv(trigger[i], trigRange); 
      min = TMath::Min(min, mV); 
      max = TMath::Max(max, mV); 
    }


    Bool_t fired = false;
    Float_t onThreshold = max*0.5; 
    Float_t offThreshold = max*0.3; 
    std::cout << "On Threshold:"   << onThreshold 
	      << " Off Threshold:" << offThreshold << std::endl;


    for (UInt_t i = 0; i < nsamples; i++) { 
      mV = picoscope::adcToMv(trigger[i], trigRange); 
      if (!fired && mV>onThreshold) {
	fired = true;
	ps->AddTrig(i);
	continue;
      }
    }

  }
  ps->Analyze(); 
  ps->Print(); 
  
  return ps; 

}

//Callback for getting an individual Picoscope data buffer 
void PicoscopeControls::bufferCallBack(picoscope::picoscopeData *data) {
  PSbuffer *ps; 
  std::map<PS6000_CHANNEL, std::pair<PS6000_RANGE, int16_t **> > buffers = std::get<0>(*data); 
  uint32_t nsamples = std::get<1>(*data); 
  uint32_t captures = std::get<2>(*data); 
  picoscope::samplingSettings s = std::get<3>(*data); 
  Double_t dT = s.timeIntervalNS(); 
  Double_t t0 = 0; 
  std::cout << "Waveform count:" << nsamples << " Waveform Range:" << buffers[PS6000_CHANNEL_A].first  << std::endl; 
  std::cout << "Trigger count:" << nsamples << " Trigger Range:" << buffers[PS6000_CHANNEL_B].first << std::endl; 

  int16_t **chABuffer = buffers[PS6000_CHANNEL_A].second; 
  int16_t **chBBuffer = NULL; 
  if (buffers.count(PS6000_CHANNEL_B) != 0) 
    chBBuffer = buffers[PS6000_CHANNEL_B].second; 
  

  //Only have one buffer 
  if (chBBuffer) { 
    ps = bufferBuilder(chABuffer[0], chBBuffer[0], nsamples, buffers[PS6000_CHANNEL_A].first, buffers[PS6000_CHANNEL_B].first, t0, dT,0.0); 
    delete[] chBBuffer[0]; 
  }
  else {
    ps =bufferBuilder(chABuffer[0], NULL, nsamples, buffers[PS6000_CHANNEL_A].first, buffers[PS6000_CHANNEL_B].first, t0, dT, 0.0); 
  }
  delete[] chABuffer[0]; 
  delete[] chABuffer; 
  delete[] chBBuffer; 
  delete data; 

  _DCOffset->SetNumber(ps->DCoffset()); 
  delete ps; 

}


void PicoscopeControls::callBack(picoscope::picoscopeData *data) { 

  

  std::map<PS6000_CHANNEL, std::pair<PS6000_RANGE, int16_t **> > buffers = std::get<0>(*data); 
  uint32_t nsamples = std::get<1>(*data); 
  uint32_t captures = std::get<2>(*data); 
  picoscope::samplingSettings s = std::get<3>(*data); 
  Double_t dT = s.timeIntervalNS(); 
  Double_t t0 = 0; 
  std::cout << "Waveform count:" << nsamples << " Waveform Range:" << buffers[PS6000_CHANNEL_A].first  << std::endl; 
  std::cout << "Trigger count:" << nsamples << " Trigger Range:" << buffers[PS6000_CHANNEL_B].first << std::endl; 

  int16_t **chABuffer = buffers[PS6000_CHANNEL_A].second; 
  int16_t **chBBuffer = NULL; 
  if (buffers.count(PS6000_CHANNEL_B) != 0) 
    chBBuffer = buffers[PS6000_CHANNEL_B].second; 
  Double_t dcOffset = _DCOffset->GetNumber(); 
  std::cout << "DC Offset set to:" << dcOffset << std::endl; 
  PSbuffer *ps; 
  for (int i = 0; i < captures; i++) { 
    if (chBBuffer) {
      ps = bufferBuilder(chABuffer[i], chBBuffer[i], nsamples, buffers[PS6000_CHANNEL_A].first, buffers[PS6000_CHANNEL_B].first, t0, dT, dcOffset);
      delete[] chBBuffer[i]; 
    }
    else { 
      ps = bufferBuilder(chABuffer[i], NULL, nsamples, buffers[PS6000_CHANNEL_A].first, buffers[PS6000_CHANNEL_B].first, t0, dT, dcOffset); 
    }
    _buffers->AddLast(ps); 
    delete[] chABuffer[i]; 
    
  }
  delete[] chABuffer; 
  delete[] chBBuffer; 

  delete data;

  //  ps->Analyze();  // calculate DC offset, frequency spectrum, noise, etc*/
  /*  ps->Print(); 


  _buffers->AddLast(ps); 



  /*  _cbTimer->SetTime(30); 
  _cbTimer->Connect("Timeout()", "PicoscopeControls", this, "FinishedCallBack()"); 
  _cbTimer->Start(); */
  

}



TString PicoscopeControls::prettyPrintWindow() { 

  Long_t samples = _preTriggerNumber->GetIntNumber()+_postTriggerNumber->GetIntNumber(); 
  std::cout << "Samples: " << samples << std::endl; 
  float timebase = _ps.calculateTimebase(_sampleInterval->GetIntNumber()); 
  float result = timebase*samples; 
  std::cout << "timebase * sampleNumber =" << timebase << "*" << samples << "=" << result << std::endl; 
  double lg = TMath::Abs(TMath::Log10(result)); 
  TString units = "";
  bool set = false;  
  float timebaseCorrected = result; 
   if (lg > 9.0) { 
     std::cout << "9" << std::endl; 
     timebaseCorrected *= 1e12; 
     units = "ps";
     set = true; 
   }
   else if (lg > 6.0 && !set) { 
     std::cout << "6" << std::endl; 
     timebaseCorrected *= 1e9; 
     units = "ns"; 
     set = true; 
   }
   else if (lg > 3.0 && !set) { 
     std::cout << "3" << std::endl; 
     timebaseCorrected *= 1e6; 
     units = "us"; 
     set = true; 
   }
   else if (timebase < 1.0 && !set) { 
     std::cout << "1" << std::endl; 
     timebaseCorrected *= 1e3; 
     units = "ms"; 
     set = true; 
   }
   else { 
     units = "seconds"; 
     set = true; 
   }


   return TString::Format("%4.2f %s", timebaseCorrected,units.Data()); 

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
  //  std::cout << "Trigger button text:" << btn << std::endl; 
  if (btn.Contains("Trigger Off")) {
    //    std::cout << "Enabling Trigger" << std::endl;     
    _triggerEnableBtn->SetText("Trigger On"); 
    //    std::cout << "Trigger Level:" << _triggerLevel->GetIntNumber() << std::endl; 
    _ps.setTriggerLevel(_ps.mvToAdc(_triggerLevel->GetIntNumber(), _ps.Channel(PS6000_CHANNEL_B).range())); 
    _ps.setTriggerDirection(PS6000_RISING); 
    _ps.setTriggerChannel(PS6000_CHANNEL_B); 
    _ps.setAutoTriggerDelay(5000); 
    _ps.setTriggerDelay(0); 
    _ps.enableTrigger(); 

  }
  else {
    //    std::cout << "Disabling Trigger" << std::endl;     
    _triggerEnableBtn->SetText("Trigger Off"); 
    _ps.disableTrigger(); 
  }    
  



}

void PicoscopeControls::triggerLevelHandler(Long_t val) {

  //  std::cout << "Trigger Level ADC:" << _ps.mvToAdc(_triggerLevel->GetIntNumber(), _ps.Channel(PS6000_CHANNEL_B).range()) << std::endl; 

  _ps.setTriggerLevel(_ps.mvToAdc(_triggerLevel->GetIntNumber(), _ps.Channel(PS6000_CHANNEL_B).range())); 

}

void PicoscopeControls::writeBuffersToDisk() { 

  TGFileInfo *fileInfo = new TGFileInfo; 
  TGFileDialog *dialog = new TGFileDialog(gClient->GetRoot(), _mf, kFDOpen, fileInfo); 

  if (fileInfo->fFilename == NULL) {
    std::cout << "File selection cancelled." << std::endl; 
    return; 
  }
  

  if (_buffers && _buffers->First()) { 
    TFile *f = new TFile(fileInfo->fFilename, "RECREATE"); 
    _buffers->Write("PSbufferlist", 1); 
    f->Write(); 
    f->Close(); 
  }
  else 
    std::cout << "No buffers to write!" << std::endl; 

}



void PicoscopeControls::preview() { 

  std::cout << "Preview!" << std::endl; 
  if (_previewF == NULL) { 
    _previewF = new TGMainFrame(NULL, 500, 500, kVerticalFrame); 
    _previewF->SetWindowName("Buffers"); 

    TGHorizontalFrame *buttonF = new TGHorizontalFrame(_previewF, 500, 50); 
    _playBtn = new TGTextButton(buttonF, TGHotString("Play")); 
     _playBtn->Connect("Clicked()", "PicoscopeControls", this, "Play()"); 
     buttonF->AddFrame(_playBtn, _hintsn); 
    TGHorizontalFrame *waveFormF = new TGHorizontalFrame(_previewF, 500, 450); 
    _canvas = new TRootEmbeddedCanvas("Preview Canvas", waveFormF, 500, 450); 
    waveFormF->AddFrame(_canvas, _hintsn); 
    _previewF->AddFrame(buttonF, new TGLayoutHints(kLHintsBottom, 2,2,2,2)); 
    _previewF->AddFrame(waveFormF, new TGLayoutHints(kLHintsTop|kLHintsExpandY, 2,2,2,2)); 
   


    _previewF->MapSubwindows(); 
    _previewF->Resize(_previewF->GetDefaultSize()); 
    _previewF->MapWindow(); 
    _previewF->Connect("CloseWindow()", "PicoscopeControls", this, "previewCleanup()"); 
  }
  

}

void PicoscopeControls::previewCleanup() { 


  if (_previewF)
    delete _canvas; 
    delete _previewF; 
  _previewF = nullptr; 

}

void PicoscopeControls::Play() { 
  PSbuffer *b = (PSbuffer *) _buffers->First(); 
  if (b != NULL) { 
        _timer->Connect("Timeout()", "PicoscopeControls", this, "NextWaveform()"); 
    _iter = new TListIter(_buffers); 
    _iter->Next(); 
    TCanvas *c = _canvas->GetCanvas(); 
    c->cd(); 
    b->GetWaveform()->Draw(); 
    c->Update(); 
    _playBtn->ChangeText("Stop"); 
    _playBtn->Disconnect("Clicked()"); 
    _playBtn->Connect("Clicked()", "PicoscopeControls", this, "Stop()"); 
    std::cout << "Starting to step through waveforms." << std::endl; 
    _timer->Start(1000, kFALSE); 

  }
  else { 
    std::cout << "No Available Waveforms" << std::endl; 
  }

}

void PicoscopeControls::NextWaveform() { 
  _timer->Stop(); 
  PSbuffer *b = (PSbuffer *) _iter->Next(); 
  if (b != NULL) { 
    _timer->Start(1000, kFALSE); 
    _canvas->GetCanvas()->cd(); 
    b->GetWaveform()->Draw(); 
    _canvas->GetCanvas()->Update(); 

  }
  else {
    _timer->Stop(); 
    std::cout << "Finished stepping through waveforms." << std::endl; 
    delete _iter; 
    _iter = NULL; 
    _playBtn->SetText("Play"); 
    _playBtn->Disconnect("Clicked()"); 
    _playBtn->Connect("Clicked()", "PicoscopeControls", this, "Play()"); 
  }

}

void PicoscopeControls::runNumberHandler(Long_t val) { 
  unsigned long maxSamples = _ps.setCaptures(_numberRuns->GetIntNumber()); 

  
  _runs = 1; //_numberRuns->GetIntNumber(); 
  if (_runs <= 0)
    _runs = 1; 

}

void PicoscopeControls::FinishedCallBack() { 
  _cbTimer->Stop(); 
  std::cout << "Remaining Runs:" << _runs << std::endl; 
  if (_runs > 0) { 
    _ps.takeRun();
  }
  else {
    runNumberHandler(0); 
  }
}
   

void PicoscopeControls::Stop() { 
  _timer->Stop(); 
  _playBtn->SetText("Cont"); 
  _playBtn->Disconnect("Clicked()"); 
  _playBtn->Connect("Clicked()", "PicoscopeControls", this, "NextWaveform()"); 
}


void PicoscopeControls::DCOffsetHandler(Long_t) { 

  std::cout << "DC offset changed value" << std::endl;  

}
void PicoscopeControls::captureBaseline() { 

  std::cout << "capturing baseline" << std::endl;  
  unsigned long maxSamples = _ps.setCaptures(1); 

  picoscope::samplingSettings &s = _ps.sampling(); 
  s.setOversample(0); 
  s.setTimebase(_sampleInterval->GetIntNumber()); 
  s.setSegment(1); 
  s.setPostTriggerSamples(_postTriggerNumber->GetIntNumber()); 
  s.setPreTriggerSamples(_preTriggerNumber->GetIntNumber()); 

  _ps.enableBandwidthLimit(PS6000_CHANNEL_A); 
  _ps.enableChannel(PS6000_CHANNEL_A); 

  std::function<void(picoscope::picoscopeData *)> *cB = new std::function<void(picoscope::picoscopeData *)>([this](picoscope::picoscopeData *d) { bufferCallBack(d);}); 
  _ps.setCallback(cB); 

  _ps.takeRun();



}


void PicoscopeControls::clearBuffers()  {

  _buffers->SetOwner(true); 
  _buffers->Delete(); 
  /*    TList iter = new TListIter(_buffers);   
    PSbuffer *b = (PSbuffer *) iter->Next(); 
    while (b != NULL) { 
      delete b; 
      b = (PSbuffer *) iter->Next(); 
    }

    delete iter; */


}




#endif











