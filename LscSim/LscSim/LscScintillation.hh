#ifndef LscScintillation_h
#define LscScintillation_h 1

#include "CupSim/CupScintillation.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class LscScintillation : public CupScintillation {
  public:
    LscScintillation(const G4String &processName = "Scintillation",
                      G4ProcessType type          = fElectromagnetic);
    ~LscScintillation();

  public:
    G4VParticleChange *PostStepDoIt(const G4Track &aTrack, const G4Step &aStep);

    static G4double GetTotEdepQuenched() { return TotalEnergyDepositQuenched; }

  private:
    static G4double TotalEnergyDepositQuenched;
};

#endif
