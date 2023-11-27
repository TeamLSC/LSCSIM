#include "LscSim/LscScintSD.hh"
#include "CupSim/CupScintSD.hh"

#include "G4EmSaturation.hh"
#include "G4LossTableManager.hh"
#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscDetectorMessenger.hh"
#include "LscSim/LscScintillation.hh"

#include "G4ParticleTypes.hh"

// Constructor /////////////////////////////////////////////////////////////
LscScintSD::LscScintSD(G4String name, int arg_max_tgs) : CupScintSD(name, arg_max_tgs) {}

// Destructor //////////////////////////////////////////////////////////////
LscScintSD::~LscScintSD() {}

G4bool LscScintSD::ProcessHits(G4Step *aStep, G4TouchableHistory * /*ROhist*/) {
    G4ParticleDefinition *particleType = aStep->GetTrack()->GetDefinition();
    G4String particleName              = particleType->GetParticleName();

    G4double edep = aStep->GetTotalEnergyDeposit();
    G4double edep_quenched;
    G4int quenchingModel = LscDetectorConstruction::GetQuenchingModel();
    // G4cout << "quenchingModel= " << quenchingModel << G4endl;
    const G4Material *aMaterial = aStep->GetTrack()->GetMaterial();
    if (quenchingModel == 1) { // using Birks
        G4EmSaturation *emSaturation = G4LossTableManager::Instance()->EmSaturation();
        // edep_quenched = emSaturation->VisibleEnergyDeposition(aStep);
        edep_quenched = emSaturation->VisibleEnergyDepositionAtAStep(aStep);
    } else { // by particle type
        G4MaterialPropertiesTable *aMaterialPropertiesTable =
            aMaterial->GetMaterialPropertiesTable();
        G4MaterialPropertyVector *Scint_Yield_Vector = NULL;
        G4double ScintillationYield                  = 0.;
        if (!aMaterialPropertiesTable) {
            edep_quenched = 0.;
            G4cout << "EJ: No material properties in table" << G4endl;
        } else {
            // Protons
            if (particleType == G4Proton::ProtonDefinition())
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("PROTONSCINTILLATIONYIELD");
            // Deuterons
            else if (particleType == G4Deuteron::DeuteronDefinition())
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("DEUTERONSCINTILLATIONYIELD");
            // Tritons
            else if (particleType == G4Triton::TritonDefinition())
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("TRITONSCINTILLATIONYIELD");
            // Alphas
            else if (particleType == G4Alpha::AlphaDefinition())
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("ALPHASCINTILLATIONYIELD");
            else if (particleType->GetParticleType() == "nucleus" ||
                     particleType == G4Neutron::NeutronDefinition())
                Scint_Yield_Vector = aMaterialPropertiesTable->GetProperty("IONSCINTILLATIONYIELD");
            else if (particleType == G4Electron::ElectronDefinition() ||
                     particleType == G4Gamma::GammaDefinition())
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
            else
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
            if (!Scint_Yield_Vector) {
                Scint_Yield_Vector =
                    aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
            }
            ScintillationYield = Scint_Yield_Vector->Value(edep);
            edep_quenched      = ScintillationYield * edep;
        }
        // G4cout << "particleName= " << particleName << ", particleType= " <<
        // particleType->GetParticleType() << G4endl;
    }

    //if(edep==0.) return true;
    if (edep == 0. || particleName == "opticalphoton") return true;
/*
    G4cout << "EJ: particleName= " << particleName
           << ", particleType= " << particleType->GetParticleType() << ", edep= " << edep
           << ", visible= " << edep_quenched << G4endl;
*/
    G4StepPoint *preStepPoint            = aStep->GetPreStepPoint();
    G4TouchableHandle theTouchable       = preStepPoint->GetTouchableHandle();
    G4int copyNo                         = theTouchable->GetCopyNumber();
    G4VPhysicalVolume *thePhysical       = theTouchable->GetVolume();

    CupScintHit *aHit = (*hitsCollection)[copyNo];
    // check if it is first touch
    if (!(aHit->GetLogV())) {
        // fill volume information
        aHit->SetLogV(thePhysical->GetLogicalVolume());
        G4AffineTransform aTrans = theTouchable->GetHistory()->GetTopTransform();
        aTrans.Invert();
        aHit->SetRot(aTrans.NetRotation());
        aHit->SetPos(aTrans.NetTranslation());
    }

    // add energy deposition
    aHit->AddEdep(edep);
    aHit->AddEdepQuenched(edep_quenched);

    return true;
}
