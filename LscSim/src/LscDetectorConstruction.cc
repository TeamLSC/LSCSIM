#include "globals.hh"

#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscDetectorMessenger.hh"

#include "CupSim/CupInputDataReader.hh"
#include "CupSim/CupParam.hh"

#include "G4Box.hh"
#include "G4Element.hh"
#include "G4IntersectionSolid.hh"
#include "G4Material.hh"
#include "G4OpticalSurface.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"

#include "G4Colour.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4SmartVoxelHeader.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"

#include "G4ios.hh"

#include "G4Types.hh"

#include <fstream>
#include <sstream>

using namespace CLHEP;
using namespace std;

//int LscDetectorConstruction::whichDetGeometry = 99999;
LscDetectorConstruction::eDetGeometry LscDetectorConstruction::whichDetGeometry = kNumDetGeometries;
int LscDetectorConstruction::quenchingmodel   = 1;

LscDetectorConstruction::LscDetectorConstruction() : CupDetectorConstruction() {
    whichDetector = kDetector_LscDetector;
    LscMessenger = new LscDetectorMessenger(this);
}

LscDetectorConstruction::~LscDetectorConstruction() {}

G4VPhysicalVolume *LscDetectorConstruction::Construct() {
    // make materials if needed
    if (!materials_built) {
        ConstructMaterials();
    }

    // delete the old detector if we are constructing a new one
    if (world_phys) {
        delete world_phys;
        world_phys = NULL;
    }

    // construct the new detector
    switch (whichDetector) {
        case kDetector_LscDetector:
            ConstructLscDetector();
            break;
        default:
            CupDetectorConstruction::Construct();
            break;
    }

    return world_phys;
}
// end of LscDetectorConstruction::Construct()

// ----------------------------------------------------------------
G4String LscDetectorConstruction::GetDetectorTypeName(int i) {
    if (i < kNumGenericDetectors)
        return G4String("generic_") + CupDetectorConstruction::GetDetectorTypeName(i);
    switch (i) {
        case kDetector_LscDetector:
            return "LscDetector";
        default:
            return "detector-unknown";
    }
}

// ----------------------------------------------------------------
G4String LscDetectorConstruction::GetDetGeometryTypeName(eDetGeometry i) {
    switch (i) {
        case kDetector_LscYemilab:
            return "lscyemilab";
        default:
            return "detGeometry-unknown";
    }
}

// ----------------------------------------------------------------
void LscDetectorConstruction::ConstructMaterials() {
    CupDetectorConstruction::ConstructMaterials();

    // ... insert any additional material definitions here
    G4String name;
    G4double density;
    G4int nelements;
    G4int natoms;

    // --- LAB-based Liquid Scintillator
    density   = 0.86 * g / cm3;
    nelements = 8;
    LS_LAB    = new G4Material(name = "LS_LAB", density, nelements);

    G4double PPO_fraction    = 3 * g / (m3 * density);   // 3 g/l
    G4double BisMSB_fraction = 30 * mg / (m3 * density); // 30 mg/l

    LS_LAB->AddMaterial(LAB[0], 0.0047 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(LAB[1], 0.097 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(LAB[2], 0.3385 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(LAB[3], 0.3472 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(LAB[4], 0.2083 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(LAB[5], 0.0043 / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(PPO, PPO_fraction / (1.0 + PPO_fraction + BisMSB_fraction));
    LS_LAB->AddMaterial(BisMSB, BisMSB_fraction / (1.0 + PPO_fraction + BisMSB_fraction));

    LS_LAB->GetIonisation()->SetBirksConstant(0.117 * mm / MeV);

    // --- UGF
    density   = 0.96 * g / cm3;
    nelements = 2;

    UGF = new G4Material("UGF", density, nelements);
    UGF->AddElement(_elementH, 20);
    UGF->AddElement(_elementC, 16);

    UGF->GetIonisation()->SetBirksConstant(0.124 * mm / MeV);

/*
    // ............................. NaI crystal ...........................
    G4Material *LscNaI = NaI;
    LscNaI->GetIonisation()->SetBirksConstant(0.003406 * mm / MeV); // 1.25*0.001*g/MeV/cm2

    // EJ: for non-linearity
    G4int const LscNaI_NUMENTRIES_YIELD              = 30;
    G4double energies_alpha[LscNaI_NUMENTRIES_YIELD] = {
        0.0001 * MeV, 0.001 * MeV, 0.002 * MeV, 0.003 * MeV, 0.004 * MeV, 0.005 * MeV,
        0.006 * MeV,  0.007 * MeV, 0.008 * MeV, 0.009 * MeV, 0.01 * MeV,  0.015 * MeV,
        0.020 * MeV,  0.025 * MeV, 0.03 * MeV,  0.035 * MeV, 0.04 * MeV,  0.045 * MeV,
        0.05 * MeV,   0.055 * MeV, 0.06 * MeV,  0.065 * MeV, 0.07 * MeV,  0.075 * MeV,
        0.08 * MeV,   0.085 * MeV, 0.09 * MeV,  0.095 * MeV, 0.1 * MeV,   0.2 * MeV};
    G4double yield_alpha[LscNaI_NUMENTRIES_YIELD] = {
        0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
        0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    G4double energies_electron[LscNaI_NUMENTRIES_YIELD] = {
        0.0001 * MeV, 0.001 * MeV, 0.002 * MeV, 0.003 * MeV, 0.004 * MeV, 0.005 * MeV,
        0.006 * MeV,  0.007 * MeV, 0.008 * MeV, 0.009 * MeV, 0.01 * MeV,  0.015 * MeV,
        0.020 * MeV,  0.025 * MeV, 0.03 * MeV,  0.035 * MeV, 0.04 * MeV,  0.045 * MeV,
        0.05 * MeV,   0.055 * MeV, 0.06 * MeV,  0.065 * MeV, 0.07 * MeV,  0.075 * MeV,
        0.08 * MeV,   0.085 * MeV, 0.09 * MeV,  0.095 * MeV, 0.1 * MeV,   0.2 * MeV};
    G4double yield_electron[LscNaI_NUMENTRIES_YIELD] = {
        0.7,   0.95,  1.055, 1.14,  1.17,  1.20,  1.21,  1.22,  1.23,  1.24,
        1.24,  1.235, 1.225, 1.215, 1.20,  1.18,  1.17,  1.165, 1.155, 1.15,
        1.145, 1.14,  1.135, 1.13,  1.128, 1.125, 1.122, 1.12,  1.118, 1.0};
    G4MaterialPropertiesTable *propLscNaI = new G4MaterialPropertiesTable();
    propLscNaI->AddProperty("PROTONSCINTILLATIONYIELD", energies_alpha, yield_alpha,
                             LscNaI_NUMENTRIES_YIELD);
    propLscNaI->AddProperty("DEUTERONSCINTILLATIONYIELD", energies_alpha, yield_alpha,
                             LscNaI_NUMENTRIES_YIELD);
    propLscNaI->AddProperty("TRITONSCINTILLATIONYIELD", energies_alpha, yield_alpha,
                             LscNaI_NUMENTRIES_YIELD);
    propLscNaI->AddProperty("ALPHASCINTILLATIONYIELD", energies_alpha, yield_alpha,
                             LscNaI_NUMENTRIES_YIELD);
    propLscNaI->AddProperty("IONSCINTILLATIONYIELD", energies_alpha, yield_alpha,
                             LscNaI_NUMENTRIES_YIELD);
    propLscNaI->AddProperty("ELECTRONSCINTILLATIONYIELD", energies_electron, yield_electron,
                             LscNaI_NUMENTRIES_YIELD);
    LscNaI->SetMaterialPropertiesTable(propLscNaI);
*/

    // ..... database .....
    // use materials from database or file
    G4String nowKMString;
    if (getenv("LscDATA") != nullptr) {
        ifstream lsc_mat(nowKMString = (G4String(getenv("LscDATA")) + "/materials_lsc.dat").c_str());
        CupInputDataReader::ReadMaterials(lsc_mat);
    } else if (getenv("SOFTWARE_DIR") != nullptr) {
        ifstream lsc_mat(
            (nowKMString = G4String(getenv("SOFTWARE_DIR")) + "/LscSim/data/materials_lsc.dat")
                .c_str());
        CupInputDataReader::ReadMaterials(lsc_mat);
    } else {
        ifstream lsc_mat(nowKMString = G4String("./data/materials_lsc.dat").c_str());
        CupInputDataReader::ReadMaterials(lsc_mat);
    }

    G4cout << "lsc_mat= " << nowKMString << G4endl;
}

// ----------------------------------------------------------------
void LscDetectorConstruction::ConstructLscDetector() {
// construct the new detector geometry
    switch (whichDetGeometry) {
        case kDetector_LscYemilab:
            ConstructLscYemilab();
            break;
        default:
            G4cerr << "ERROR: INVALID VALUE for LscDetectorConstruction.whichDetGeometry" << G4endl
                   << flush;
            // fall through and make a GenericLAND
    }
}
