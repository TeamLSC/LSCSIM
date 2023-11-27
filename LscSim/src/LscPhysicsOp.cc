#include "LscSim/LscPhysicsOp.hh"
#include "CupSim/CupOpAttenuation.hh"
#include "CupSim/CupOpBoundaryProcess.hh"
#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscScintillation.hh"

#include "G4LossTableManager.hh"
#include "G4ProcessManager.hh"

#include "G4Cerenkov.hh"
#include "G4EmSaturation.hh"
//#include "G4OpBoundaryProcess.hh"

LscPhysicsOp::LscPhysicsOp(const G4String &name) : G4VPhysicsConstructor(name) {
    OpVerbLevel = 0;
}

LscPhysicsOp::~LscPhysicsOp() {}

void LscPhysicsOp::ConstructProcess() {
    G4int quenchingModel = LscDetectorConstruction::GetQuenchingModel();

    // Scintillation process
    LscScintillation *theScintProcessDef = new LscScintillation("Scintillation");
    if (quenchingModel == 1) { // for Birks
        // theScintProcessDef->DumpPhysicsTable();
        theScintProcessDef->SetTrackSecondariesFirst(true);
        theScintProcessDef->SetScintillationYieldFactor(1.0);     //
        theScintProcessDef->SetScintillationExcitationRatio(1.0); //0.0
        theScintProcessDef->SetVerboseLevel(OpVerbLevel);
        G4EmSaturation *emSaturation = G4LossTableManager::Instance()->EmSaturation();
        theScintProcessDef->AddSaturation(emSaturation);
    } else {
        theScintProcessDef->SetTrackSecondariesFirst(true);
        theScintProcessDef->SetScintillationYieldFactor(1.0);     //
        theScintProcessDef->SetScintillationExcitationRatio(1.0); //0.0
        theScintProcessDef->SetVerboseLevel(OpVerbLevel);
        theScintProcessDef->SetScintillationByParticleType(true); // for by particle type
    }

    // Optical processes
    CupOpAttenuation *theAttenuationProcess = new CupOpAttenuation();
    theAttenuationProcess->UseTimeProfile("exponential");
    theAttenuationProcess->SetVerboseLevel(OpVerbLevel);

    //G4OpBoundaryProcess *theBoundaryProcess = new G4OpBoundaryProcess();
    CupOpBoundaryProcess *theBoundaryProcess = new CupOpBoundaryProcess();
    theBoundaryProcess->SetVerboseLevel(OpVerbLevel);

    // Cerenkov
    G4Cerenkov *theCerenkovProcess = new G4Cerenkov();
    theCerenkovProcess->SetTrackSecondariesFirst(true);

    auto theParticleIterator = GetParticleIterator();

    theParticleIterator->reset();
    while ((*theParticleIterator)()) {
        G4ParticleDefinition *particle = theParticleIterator->value();
        G4ProcessManager *pmanager     = particle->GetProcessManager();
        G4String particleName          = particle->GetParticleName();

        if (theScintProcessDef->IsApplicable(*particle)) {
            pmanager->AddProcess(theScintProcessDef);
            pmanager->SetProcessOrderingToLast(theScintProcessDef, idxAtRest);
            pmanager->SetProcessOrderingToLast(theScintProcessDef, idxPostStep);
        }
        if (theCerenkovProcess->IsApplicable(*particle)) {
            pmanager->AddProcess(theCerenkovProcess);
            pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
        }

        if (particleName == "opticalphoton") {
            pmanager->AddDiscreteProcess(theAttenuationProcess);
            pmanager->AddDiscreteProcess(theBoundaryProcess);
        }
    }
}
