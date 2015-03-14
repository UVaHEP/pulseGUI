{
  gSystem->Load("$PULSEGUI_LIBDIR/libPulseGUI.so");
  //gSystem->SetIncludePath("-I \"$PULSEGUI_INC\"");
  gROOT->SetMacroPath("./script");
  gROOT->ProcessLine(".L waveViewer.C+");
  
}
