
void pulseGUI(TFile file){
  gSystem->Load("build/lib/libPulseGUI.so");
  PulseGUI* pG=new PulseGUI(file);
//  pulseAnalysis *pA=pG->GetAnalysis()  // get pointer to analysis tool
// _analysis = new pulseAnalysis("Package-1-Device-1-33.6V0.root")
}


/*  Useful cut/paste for interactive work
gSystem->Load("pulseGUI.so");
pulseGUI* pG=new pulseGUI();
pulseAnalysis *pA=pG->GetAnalysis();  // get pointer to analysis tool
pA->Hprms()->Draw();
analysis = new pulseAnalysis("Package-1-Device-1-33.6V0.root")
*/


