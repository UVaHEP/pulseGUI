#include "TApplication.h"
#include "TString.h"
#include "PulseGUI.h"
#include <iostream>

using namespace std;



int main(int argc, char **argv) {
  TString fName="";
  if (argc>1) {
    fName=argv[1];
    cout << "Loading file: " << fName << endl;
  }
  TApplication theApp("App", &argc, argv);

  PulseGUI *pg=new PulseGUI(fName);
  pg->Print();

  cout << "To exit, quit ROOT from the File menu of the plot (or use control-C)" << endl;
  theApp.Run(true);
  return 0;
}

