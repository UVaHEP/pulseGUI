#include "fourier.h"
#include "PSbuffer.h"

#include "TF1.h"

#include <iostream>

using std::cout;
using std::endl;


void PSbuffer::InitWaveform(Int_t nbins, Float_t max, Float_t min){
  if (max==min) max=dT*nbins;
  waveBuffer=new TH1F("waveform","Waveform;t [ns];V [mV]",nbins,min,max);
}

Int_t PSbuffer::GetTrig(Int_t ntrig) const{
  if (! trigs.size() ) return 0;
  return trigs[ntrig];
}

void PSbuffer::AddTrig(Int_t trigBin){
  trigs.push_back(trigBin);
}



void PSbuffer::Print(){
  cout << "*** Picoscope buffer ***" << endl;
  cout << "t0: " << t0 << " Delta t: " << dT << " nsamples: " << waveBuffer->GetNbinsX();
  cout << " dV: " << dV << " noise: " << noise << endl;
  cout << "nTrigs: " << trigs.size() << "  ( ";
  for (unsigned int i=0; i<trigs.size(); i++) {
    if (i>0) cout <<  " , ";
    cout << trigs[i];
  }
  cout << " )" << endl;
  cout << "DC Offset: " << dcOffset << endl;
  cout << "*** ***";
  cout << endl;
}


void PSbuffer::Analyze(){
  // consider replacing w/ ROOT FFT
  dcOffset=calcDCoffset(waveBuffer);
  double min=waveBuffer->GetBinContent(waveBuffer->GetMinimumBin());  // min/max voltage
  double max=waveBuffer->GetBinContent(waveBuffer->GetMaximumBin());
  min-=dcOffset;
  max-=dcOffset;
  int bins = (int)((max-min)/dV*1.01); // guard against rounding    
  pHD = new TH1F("spectrum","Sample spectrum;Amplitude [mV];# samples",
		 bins,min-dV/2,max+dV/2);
  
  // remove DC offset, fill pulse height spectrum
  for (int i=1; i<=waveBuffer->GetNbinsX(); i++){
    double v=waveBuffer->GetBinContent(i)-dcOffset;
    waveBuffer->SetBinContent( i , v );
    pHD->Fill(v);
  }
  TF1 *g2=new TF1("g2","[0]*exp(-0.5*x*x/[1]/[1])",min-dV/2,max+dV/2);
  g2->SetParameters(pHD->GetMaximum(),pHD->GetRMS());
  pHD->Fit("g2","0");
  // after 1st fit, set fit range to +- 2sigma around 0 and refit
  double sigma=g2->GetParameter(1); 
  g2->SetRange(-2*sigma,2*sigma);
  pHD->Fit("g2","0R");
  noise=g2->GetParameter(1);
}

