#include "TApplication.h"
//#include "TEnv.h"
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
  // highres linux display is a total clusterf**!  Some hack like this is needed
  // but it's not results of this are also f****d up
  //  gEnv->SetValue("Gui.DefaultFont", "-*-helvetica-medium-r-*-*-24-*-*-*-*-*-iso8859-1");
  TApplication theApp("App", &argc, argv);

  PulseGUI *pg=new PulseGUI(fName);
  pg->Print();

  cout << "To exit, quit ROOT from the File menu of the GUI (or use control-C)" << endl;
  theApp.Run(true);
  return 0;
}

