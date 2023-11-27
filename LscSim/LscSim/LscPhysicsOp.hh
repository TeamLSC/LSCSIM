#ifndef LscPhysicsOp_h
#define LscPhysicsOp_h 1

#include "G4VPhysicsConstructor.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class LscPhysicsOp : public G4VPhysicsConstructor {
  public:
    LscPhysicsOp(const G4String &name = "standardOp");
    ~LscPhysicsOp();

  public:
    // This method is dummy for physics
    virtual void ConstructParticle(){};

    virtual void ConstructProcess();

  private:
    G4int OpVerbLevel;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
