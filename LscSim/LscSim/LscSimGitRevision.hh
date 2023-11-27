#ifndef __LscSimLibGitRevision_H_
#define __LscSimLibGitRevision_H_

#include <iostream>

class LscSimGitRevision {
  public:
    static void PrintGitInfo();
    static const char *GetRevisionHash();
    static const char *GetBranchName();

  private:
    LscSimGitRevision(){}; // Don't create an object of this cless!
    ~LscSimGitRevision(){};
    static const char *fgceRevisionHash;
    static const char *fgceBranchName;
    static const char *fgceLibraryName;
};

#endif
