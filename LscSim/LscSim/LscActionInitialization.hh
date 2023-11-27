#ifndef __LscActionInitialization_h__
#define __LscActionInitialization_h__ 1

#include "G4Version.hh"

#if G4VERSION_NUMBER >= 1000

#include "G4MTRunManager.hh"
#include "G4VUserActionInitialization.hh"
#include "globals.hh"

class LscDetectorConstruction;
class LscRootNtuple;

class LscActionInitialization : public G4VUserActionInitialization {
  public:
    LscActionInitialization(LscRootNtuple *aRec, LscDetectorConstruction *aDet)
        : fDetConstruction(aDet), fRecorders(aRec){};
    virtual ~LscActionInitialization(){};

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    LscDetectorConstruction *fDetConstruction;

    LscRootNtuple *fRecorders;
};
#endif

#endif
