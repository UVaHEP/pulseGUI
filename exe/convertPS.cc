#include "PicoReader.h"
#include <iostream>

using namespace std;



int main(int argc, char **argv) {
  if (argc<2) {
    cout << "Usage: convertPS file.mat [or file.txt or file.csv]" << endl;
    return 1;
  }

  PicoReader* reader = new PicoReader();
  return reader->Convert(argv[1]);
}

