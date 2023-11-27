#include "LscSim/LscSimGitRevision.hh"

#define STRINGFY(X) #X
#define TOSTRING(X) STRINGFY(X)

const char *LscSimGitRevision::fgceRevisionHash = TOSTRING(LscSim_GIT_COMMIT_HASH);
const char *LscSimGitRevision::fgceBranchName   = TOSTRING(LscSim_GIT_BRANCH);
const char *LscSimGitRevision::fgceLibraryName  = "LscSim library";

const char *LscSimGitRevision::GetRevisionHash() { return fgceRevisionHash; }
const char *LscSimGitRevision::GetBranchName() { return fgceBranchName; }

void LscSimGitRevision::PrintGitInfo() {
    std::cout << fgceLibraryName << ": <Branch: " << fgceBranchName
              << ", Revision hash: " << fgceRevisionHash << ">" << std::endl;
}

int main() {
    std::cout << "LscSim library: "
              << ": <Branch: " << TOSTRING(LscSim_GIT_COMMIT_HASH)
              << ", Revision hash: " << TOSTRING(LscSim_GIT_BRANCH) << ">" << std::endl;
}
