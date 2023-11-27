#ifndef LscPhysicsList_h
#define LscPhysicsList_h 1

#include "CupSim/CupPhysicsList.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class LscPhysicsList : public CupPhysicsList {
  public:
    LscPhysicsList();
    ~LscPhysicsList();
    virtual void ConstructProcess();
    virtual void AddPhysicsList(const G4String &name);

  private:
    G4String fEMName;
    G4VPhysicsConstructor*               emPhysList;
    G4String fOpName;
    G4VPhysicsConstructor*               OpPhysList;
    G4String fHadName;
    G4bool hadIsRegisted;
    std::vector<G4VPhysicsConstructor*>  hadronPhys;
};

#endif
