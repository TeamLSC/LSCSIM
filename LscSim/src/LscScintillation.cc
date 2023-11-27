#include "LscSim/LscScintillation.hh"
#include "CupSim/CupScintillation.hh"

#include "G4ParticleTypes.hh"

using namespace CLHEP;

G4double LscScintillation::TotalEnergyDepositQuenched = 0.0;

// Constructor /////////////////////////////////////////////////////////////
LscScintillation::LscScintillation(const G4String &processName, G4ProcessType type)
    : CupScintillation(processName, type) {}

// Destructor //////////////////////////////////////////////////////////////
LscScintillation::~LscScintillation() {}

// PostStepDoIt
// -------------
//
G4VParticleChange *LscScintillation::PostStepDoIt(const G4Track &aTrack, const G4Step &aStep)
// This routine is called for each tracking step of a charged particle
// in a scintillator. A Poisson/Gauss-distributed number of photons is
// generated according to the scintillation yield formula, distributed
// evenly along the track segment and uniformly into 4pi.
{
    aParticleChange.Initialize(aTrack);

    const G4DynamicParticle *aParticle = aTrack.GetDynamicParticle();
    const G4Material *aMaterial        = aTrack.GetMaterial();

    G4StepPoint *pPreStepPoint  = aStep.GetPreStepPoint();
    G4StepPoint *pPostStepPoint = aStep.GetPostStepPoint();

    G4ThreeVector x0 = pPreStepPoint->GetPosition();
    G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
    G4double t0      = pPreStepPoint->GetGlobalTime();

    G4double TotalEnergyDeposit = aStep.GetTotalEnergyDeposit();

    G4MaterialPropertiesTable *aMaterialPropertiesTable = aMaterial->GetMaterialPropertiesTable();
    if (!aMaterialPropertiesTable) return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);

    G4MaterialPropertyVector *Fast_Intensity =
        aMaterialPropertiesTable->GetProperty("FASTCOMPONENT");
    G4MaterialPropertyVector *Slow_Intensity =
        aMaterialPropertiesTable->GetProperty("SLOWCOMPONENT");

    if (!Fast_Intensity && !Slow_Intensity)
        return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);

    G4int nscnt = 1;
    if (Fast_Intensity && Slow_Intensity) nscnt = 2;

    G4double ScintillationYield = 0.;

    if (scintillationByParticleType) {
        // The scintillation response is a function of the energy
        // deposited by particle types.

        // Get the definition of the current particle
        G4ParticleDefinition *pDef                   = aParticle->GetDefinition();
        G4MaterialPropertyVector *Scint_Yield_Vector = NULL;

        // Obtain the G4MaterialPropertyVectory containing the
        // scintillation light yield as a function of the deposited
        // energy for the current particle type

        // Protons
        if (pDef == G4Proton::ProtonDefinition())
            Scint_Yield_Vector = aMaterialPropertiesTable->GetProperty("PROTONSCINTILLATIONYIELD");

        // Deuterons
        else if (pDef == G4Deuteron::DeuteronDefinition())
            Scint_Yield_Vector =
                aMaterialPropertiesTable->GetProperty("DEUTERONSCINTILLATIONYIELD");

        // Tritons
        else if (pDef == G4Triton::TritonDefinition())
            Scint_Yield_Vector = aMaterialPropertiesTable->GetProperty("TRITONSCINTILLATIONYIELD");

        // Alphas
        else if (pDef == G4Alpha::AlphaDefinition())
            Scint_Yield_Vector = aMaterialPropertiesTable->GetProperty("ALPHASCINTILLATIONYIELD");

        // Ions (particles derived from G4VIon and G4Ions)
        // and recoil ions below tracking cut from neutrons after hElastic
        else if (pDef->GetParticleType() == "nucleus" || pDef == G4Neutron::NeutronDefinition())
            Scint_Yield_Vector = aMaterialPropertiesTable->GetProperty("IONSCINTILLATIONYIELD");

        // Electrons (must also account for shell-binding energy
        // attributed to gamma from standard PhotoElectricEffect)
        else if (pDef == G4Electron::ElectronDefinition() || pDef == G4Gamma::GammaDefinition())
            Scint_Yield_Vector =
                aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");

        // Default for particles not enumerated/listed above
        else
            Scint_Yield_Vector =
                aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");

        // If the user has not specified yields for (p,d,t,a,carbon)
        // then these unspecified particles will default to the
        // electron's scintillation yield
        if (!Scint_Yield_Vector) {
            Scint_Yield_Vector =
                aMaterialPropertiesTable->GetProperty("ELECTRONSCINTILLATIONYIELD");
        }

        // Throw an exception if no scintillation yield is found
        if (!Scint_Yield_Vector) {
            G4ExceptionDescription ed;
            ed << "\nLscScintillation::PostStepDoIt(): "
               << "Request for scintillation yield for energy deposit and particle type without "
                  "correct entry in MaterialPropertiesTable\n"
               << "ScintillationByParticleType requires at minimum that ELECTRONSCINTILLATIONYIELD "
                  "is set by the user\n"
               << G4endl;
            G4String comments = "Missing MaterialPropertiesTable entry - No correct entry in "
                                "MaterialPropertiesTable";
            G4Exception("LscSim/LscScintillation::PostStepDoIt", "Scint01", FatalException, ed,
                        comments);
            return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
        }

        if (verboseLevel > 1) {
            G4cout << "\n"
                   << "Particle = " << pDef->GetParticleName() << "\n"
                   << "Energy Dep. = " << TotalEnergyDeposit / MeV << "\n"
                   << "Yield = " << Scint_Yield_Vector->Value(TotalEnergyDeposit) << "\n"
                   << G4endl;
        }

        // Obtain the scintillation yield using the total energy
        // deposited by the particle in this step.

        // Units: [# scintillation photons]
        ScintillationYield = Scint_Yield_Vector->Value(TotalEnergyDeposit);
    } else {
        // The default linear scintillation process
        ScintillationYield = aMaterialPropertiesTable->GetConstProperty("SCINTILLATIONYIELD");

        // Units: [# scintillation photons / MeV]
        ScintillationYield *= YieldFactor;
    }

    G4double ResolutionScale = aMaterialPropertiesTable->GetConstProperty("RESOLUTIONSCALE");

    // Birks law saturation:

    // G4double constBirks = 0.0;

    // constBirks = aMaterial->GetIonisation()->GetBirksConstant();

    G4double MeanNumberOfPhotons;

    //        G4double TotalEnergyDepositQuenched = 0.;

    // Birk's correction via emSaturation and specifying scintillation by
    // by particle type are physically mutually exclusive

    if (scintillationByParticleType) {
        // EJ: start
        TotalEnergyDepositQuenched = ScintillationYield * TotalEnergyDeposit;
        // MeanNumberOfPhotons = TotalEnergyDepositQuenched*40000.; // EJ: 40000pe/MeV
        MeanNumberOfPhotons = TotalEnergyDepositQuenched *
                              (aMaterialPropertiesTable->GetConstProperty("SCINTILLATIONYIELD"));
        // EJ: end
    } else if (emSaturation) {
        // EJ: start
        // TotalEnergyDepositQuenched = emSaturation->VisibleEnergyDeposition(&aStep);
        TotalEnergyDepositQuenched = emSaturation->VisibleEnergyDepositionAtAStep(&aStep);
        // EJ: end
        MeanNumberOfPhotons = ScintillationYield *
                              //   (emSaturation->VisibleEnergyDeposition(&aStep));
                              (emSaturation->VisibleEnergyDepositionAtAStep(&aStep));
    } else {
        MeanNumberOfPhotons = ScintillationYield * TotalEnergyDeposit;
    }

    G4int NumPhotons;

    if (MeanNumberOfPhotons > 10.) {
        G4double sigma = ResolutionScale * std::sqrt(MeanNumberOfPhotons);
        NumPhotons     = G4int(G4RandGauss::shoot(MeanNumberOfPhotons, sigma) + 0.5);
    } else {
        NumPhotons = G4int(G4Poisson(MeanNumberOfPhotons));
    }

    // EJ: start
    totEdep += TotalEnergyDeposit;
    totEdep_quenched += TotalEnergyDepositQuenched;
    nScintPhotons += NumPhotons;

    if (verboseLevel > 1) {
        G4cout << "EJ: LscScintillation: edep= " << TotalEnergyDeposit
               << ", edep_quenched= " << TotalEnergyDepositQuenched
               << ", Yield = " << ScintillationYield << G4endl;
        G4cout << "EJ: totEdep= " << totEdep << ", totEdep_quenched= " << totEdep_quenched
               << ", MeanNumberOfPhotons= " << MeanNumberOfPhotons
               << ", NumPhotons= " << nScintPhotons
               << ", scintillationByParticleType= " << scintillationByParticleType << G4endl;
        G4cout << "EJ: resolutionscale= " << ResolutionScale << G4endl;
    }
    // EJ: end

    if (NumPhotons <= 0) {
        // return unchanged particle and no secondaries

        aParticleChange.SetNumberOfSecondaries(0);

        return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
    }

    // EJ: start
    // now we are done if we are not actually making photons here
    if (!doScintillation) {
        aParticleChange.SetNumberOfSecondaries(0);
        return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
    }
    // EJ: end

    ////////////////////////////////////////////////////////////////

    aParticleChange.SetNumberOfSecondaries(NumPhotons);

    if (fTrackSecondariesFirst) {
        if (aTrack.GetTrackStatus() == fAlive) aParticleChange.ProposeTrackStatus(fSuspend);
    }

    ////////////////////////////////////////////////////////////////

    G4int materialIndex = aMaterial->GetIndex();

    // Retrieve the Scintillation Integral for this material
    // new G4PhysicsOrderedFreeVector allocated to hold CII's

    G4int Num = NumPhotons;

    for (G4int scnt = 1; scnt <= nscnt; scnt++) {

        G4double ScintillationTime                        = 0. * ns;
        G4double ScintillationRiseTime                    = 0. * ns;
        G4PhysicsOrderedFreeVector *ScintillationIntegral = NULL;

        if (scnt == 1) {
            if (nscnt == 1) {
                if (Fast_Intensity) {
                    ScintillationTime =
                        aMaterialPropertiesTable->GetConstProperty("FASTTIMECONSTANT");
                    if (fFiniteRiseTime) {
                        ScintillationRiseTime =
                            aMaterialPropertiesTable->GetConstProperty("FASTSCINTILLATIONRISETIME");
                    }
                    ScintillationIntegral =
                        (G4PhysicsOrderedFreeVector *)((*theFastIntegralTable)(materialIndex));
                }
                if (Slow_Intensity) {
                    ScintillationTime =
                        aMaterialPropertiesTable->GetConstProperty("SLOWTIMECONSTANT");
                    if (fFiniteRiseTime) {
                        ScintillationRiseTime =
                            aMaterialPropertiesTable->GetConstProperty("SLOWSCINTILLATIONRISETIME");
                    }
                    ScintillationIntegral =
                        (G4PhysicsOrderedFreeVector *)((*theSlowIntegralTable)(materialIndex));
                }
            } else {
                G4double YieldRatio = aMaterialPropertiesTable->GetConstProperty("YIELDRATIO");
                if (ExcitationRatio == 1.0) {
                    Num = G4int(std::min(YieldRatio, 1.0) * NumPhotons);
                } else {
                    Num = G4int(std::min(ExcitationRatio, 1.0) * NumPhotons);
                }
                ScintillationTime = aMaterialPropertiesTable->GetConstProperty("FASTTIMECONSTANT");
                if (fFiniteRiseTime) {
                    ScintillationRiseTime =
                        aMaterialPropertiesTable->GetConstProperty("FASTSCINTILLATIONRISETIME");
                }
                ScintillationIntegral =
                    (G4PhysicsOrderedFreeVector *)((*theFastIntegralTable)(materialIndex));
            }
        } else {
            Num               = NumPhotons - Num;
            ScintillationTime = aMaterialPropertiesTable->GetConstProperty("SLOWTIMECONSTANT");
            if (fFiniteRiseTime) {
                ScintillationRiseTime =
                    aMaterialPropertiesTable->GetConstProperty("SLOWSCINTILLATIONRISETIME");
            }
            ScintillationIntegral =
                (G4PhysicsOrderedFreeVector *)((*theSlowIntegralTable)(materialIndex));
        }

        if (!ScintillationIntegral) continue;

        // Max Scintillation Integral

        G4double CIImax = ScintillationIntegral->GetMaxValue();

        for (G4int i = 0; i < Num; i++) {

            // Determine photon energy

            G4double CIIvalue      = G4UniformRand() * CIImax;
            G4double sampledEnergy = ScintillationIntegral->GetEnergy(CIIvalue);

            if (verboseLevel > 1) {
                G4cout << "sampledEnergy = " << sampledEnergy << G4endl;
                G4cout << "CIIvalue =        " << CIIvalue << G4endl;
            }

            // Generate random photon direction

            G4double cost = 1. - 2. * G4UniformRand();
            G4double sint = std::sqrt((1. - cost) * (1. + cost));

            G4double phi  = twopi * G4UniformRand();
            G4double sinp = std::sin(phi);
            G4double cosp = std::cos(phi);

            G4double px = sint * cosp;
            G4double py = sint * sinp;
            G4double pz = cost;

            // Create photon momentum direction vector

            G4ParticleMomentum photonMomentum(px, py, pz);

            // Determine polarization of new photon

            G4double sx = cost * cosp;
            G4double sy = cost * sinp;
            G4double sz = -sint;

            G4ThreeVector photonPolarization(sx, sy, sz);

            G4ThreeVector perp = photonMomentum.cross(photonPolarization);

            phi  = twopi * G4UniformRand();
            sinp = std::sin(phi);
            cosp = std::cos(phi);

            photonPolarization = cosp * photonPolarization + sinp * perp;

            photonPolarization = photonPolarization.unit();

            // Generate a new photon:

            G4DynamicParticle *aScintillationPhoton =
                new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), photonMomentum);
            aScintillationPhoton->SetPolarization(photonPolarization.x(), photonPolarization.y(),
                                                  photonPolarization.z());

            aScintillationPhoton->SetKineticEnergy(sampledEnergy);

            // Generate new G4Track object:

            G4double rand;

            if (aParticle->GetDefinition()->GetPDGCharge() != 0) {
                rand = G4UniformRand();
            } else {
                rand = 1.0;
            }

            G4double delta = rand * aStep.GetStepLength();
            G4double deltaTime =
                delta / ((pPreStepPoint->GetVelocity() + pPostStepPoint->GetVelocity()) / 2.);

            // emission time distribution
            if (ScintillationRiseTime == 0.0) {
                deltaTime = deltaTime - ScintillationTime * std::log(G4UniformRand());
            } else {
                deltaTime = deltaTime + sample_time(ScintillationRiseTime, ScintillationTime);
            }

            G4double aSecondaryTime = t0 + deltaTime;

            G4ThreeVector aSecondaryPosition = x0 + rand * aStep.GetDeltaPosition();

            G4Track *aSecondaryTrack =
                new G4Track(aScintillationPhoton, aSecondaryTime, aSecondaryPosition);

            aSecondaryTrack->SetTouchableHandle(aStep.GetPreStepPoint()->GetTouchableHandle());
            // aSecondaryTrack->SetTouchableHandle((G4VTouchable*)0);

            aSecondaryTrack->SetParentID(aTrack.GetTrackID());

            aParticleChange.AddSecondary(aSecondaryTrack);
        }
    }

    if (verboseLevel > 0) {
        G4cout << "\n Exiting from LscScintillation::DoIt -- NumberOfSecondaries = "
               << aParticleChange.GetNumberOfSecondaries() << G4endl;
    }

    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
}
