
void pulseGUI(){
  gSystem->Load("pulseGUI.so");
  pulseGUI* pG=new pulseGUI("../TemperatureData/Package-1-Device-1/10C/Combined/Package-1-Device-1-33.35V-9.5C0.root");
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


