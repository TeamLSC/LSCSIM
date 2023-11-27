#ifndef LscScintSD_h
#define LscScintSD_h 1

#include "CupSim/CupScintSD.hh"

class G4TouchableHistory;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class LscScintSD : public CupScintSD {
  public:
    LscScintSD(G4String name, int max_tgs = 1000);
    ~LscScintSD();

  public:
    virtual G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist);
};

#endif
