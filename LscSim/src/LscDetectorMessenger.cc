#include "LscSim/LscDetectorMessenger.hh"
#include "LscSim/LscDetectorConstruction.hh"

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4ios.hh"
#include "globals.hh"

#include <cstdlib> // for strtol
#include <fstream> // for file streams
#include <sstream>
using namespace std;

LscDetectorMessenger::LscDetectorMessenger(LscDetectorConstruction *Lscdetector)
    : LscDetector(Lscdetector) {
    // the LscDetector directory
    G4UIdirectory *DetectorDir = new G4UIdirectory("/detGeometry/");
    DetectorDir->SetGuidance("Control the detector geometry options.");

    // the select command
    DetGeometrySelectCmd = new G4UIcommand("/detGeometry/select", this);
    DetGeometrySelectCmd->SetGuidance("Select which detector you want to build");
    DetGeometrySelectCmd->SetGuidance(
        "Use with no parameters to get list of available detector styles.");
    DetGeometrySelectCmd->AvailableForStates(G4State_PreInit);
    DetGeometrySelectCmd->SetParameter(new G4UIparameter("which", 's', true));

    // quenching by Birks
    DetGeometryQuenchingCmd = new G4UIcommand("/detGeometry/quenchingModel", this);
    DetGeometryQuenchingCmd->SetGuidance("Set quenching type: 0 by particle type, 1 by Birks");
    DetGeometryQuenchingCmd->AvailableForStates(G4State_PreInit);
    DetGeometryQuenchingCmd->SetParameter(new G4UIparameter("quenching", 's', true));
}

LscDetectorMessenger::~LscDetectorMessenger() {}

void LscDetectorMessenger::SetNewValue(G4UIcommand *command, G4String newValues) {
    // GeometrySelectCmd
    if (command == DetGeometrySelectCmd) {
        if (newValues.length() == 0) {
            G4cout << "Available detector geometries: ";
            for (int i = 0; i < LscDetector->GetNumDetGeometryTypes(); i++) {
		LscDetectorConstruction::eDetGeometry nowEnum = 
			static_cast<LscDetectorConstruction::eDetGeometry>(i);
                G4cout << " " << LscDetector->GetDetGeometryTypeName(nowEnum);
	    }
            G4cout << G4endl;
        } else {
            for (int i = 0; i < LscDetector->GetNumDetGeometryTypes(); i++) {
		LscDetectorConstruction::eDetGeometry nowEnum =
                        static_cast<LscDetectorConstruction::eDetGeometry>(i);
                if (newValues.compare(LscDetector->GetDetGeometryTypeName(nowEnum)) == 0) {
                    // EJ: start
                    G4cout << "detGeometry/select " << LscDetector->GetDetGeometryTypeName(nowEnum)
                           << G4endl;
                    // EJ: end
                    LscDetector->SetWhichDetGeometry(nowEnum);
                    return;
                }
            }
            G4cerr << "Unknown detector geometry style " << newValues << G4endl;
        }
    }

    // Quenching model
    if (command == DetGeometryQuenchingCmd) {
        istringstream is((const char *)newValues);
        int index = -1;
        is >> index;
        if (is.fail()) {
            G4cerr << "/detGeometry/quenchingModel: failed to read" << G4endl;
        }
        if (index < 0) {
            G4cerr << "/detGeometry/quenchingModel: invalid value: arguments are \"" << newValues
                   << "\"" << G4endl;
            return;
        }

        LscDetector->SetQuenchingModel(index);
    }

    // invalid command
    else {
        G4cerr << "invalid detector \"set\" command\n" << flush;
    }
}

G4String LscDetectorMessenger::GetCurrentValue(G4UIcommand *command) {
    // GeometrySelectCmd
    if (command == DetGeometrySelectCmd) {
        return LscDetector->GetDetGeometryTypeName(LscDetector->GetWhichDetGeometry());
    }

    // invalid command
    else {
        return G4String("invalid LscDetectorMessenger \"get\" command");
    }
}
