#include "G4Version.hh"

#if G4VERSION_NUMBER >= 1000

#include "CupSim/CupPrimaryGeneratorAction.hh"
#include "CupSim/CupRunAction.hh"
#include "CupSim/CupSteppingAction.hh"
#include "CupSim/CupTrackingAction.hh"
#include "CupSim/CupVEventAction.hh"

#include "LscSim/LscActionInitialization.hh"
#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscRootNtuple.hh"

void LscActionInitialization::BuildForMaster() const {
    SetUserAction(new CupRunAction(fRecorders));
}

void LscActionInitialization::Build() const {
    auto p = new CupPrimaryGeneratorAction(fDetConstruction);
    SetUserAction(p);
    SetUserAction(new CupRunAction(fRecorders));
    SetUserAction(new CupVEventAction(fRecorders));
    SetUserAction(new CupTrackingAction(fRecorders));
    SetUserAction(new CupSteppingAction(fRecorders, p));
}

#endif
