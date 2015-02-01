#include "PicoReader.h"
#include <TString.h>
#include <iostream>
#include <sys/stat.h>
#include <ftw.h>
#include <fnmatch.h>

using namespace std;

bool is_file(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}


static const char *filters[] = {"*.mat","*.txt","*.csv"};
static PicoReader reader;

static int callback(const char *fpath, const struct stat *sb, int typeflag) {
  (void)sb;
  /* if it's a file */
    if (typeflag == FTW_F) {
        /* for each filter, */
        for (unsigned i = 0; i < sizeof(filters) / sizeof(filters[0]); i++) {
            /* if the filename matches the filter, */
            if (fnmatch(filters[i], fpath, FNM_CASEFOLD) == 0) {
	      reader.Convert(fpath);
	      break;
            }
        }
    }

    /* tell ftw to continue */
    return 0;
}



int main(int argc, char **argv) {
  if (argc<2) {
    cout << "Usage: convertPS file.mat [or file.txt or file.csv] or directory (for recursive processing)"
	 << endl;
    return 1;
  }
    
  if (is_dir(argv[1])){
    cout << "Processing recursively from directory: " << argv[1] << endl;
    ftw(argv[1], callback, 16);
  }
  else
    return reader.Convert(argv[1]);
}

