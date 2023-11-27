#include <iomanip>

#include "LscSim/LscPhysicsList.hh"
#include "LscSim/LscPhysicsOp.hh"

#include "G4VPhysicsConstructor.hh"

#include "G4EmLivermorePhysics.hh"
#include "G4EmStandardPhysics.hh"

#include "G4HadronElasticPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsQGSP_BERT_HP.hh"
#include "G4ThermalNeutrons.hh"

// Constructor /////////////////////////////////////////////////////////////
LscPhysicsList::LscPhysicsList() : CupPhysicsList() {
    hadIsRegisted = false;
}

// Destructor //////////////////////////////////////////////////////////////
LscPhysicsList::~LscPhysicsList() {
    for(size_t i=0; i<hadronPhys.size(); i++) {delete hadronPhys[i];}
}

void LscPhysicsList::ConstructProcess() {

    AddTransportation();

    AddParameterisation();

    // -- ConstructEM
    if (fEMName == "livermore") {
	emPhysList->ConstructProcess();
    } else {
        ConstructEM();
        G4cout << "\n EM Physics is ConstructEM(): default! \n" << G4endl;
    }
    // -- ConstructOp
    if (fOpName == "lscphysicsOp") {
	OpPhysList->ConstructProcess();
    } else {
        ConstructOp();
	G4cout << "\n Op Physics is ConstructOp(): default! \n" << G4endl;
    }
    // -- ConstructHad
    if (fHadName == "lscphysicsHad") {
	// hadronic physics lists
  	for(size_t i=0; i<hadronPhys.size(); i++) {
    	hadronPhys[i]->ConstructProcess();
  	}
    } else {
    	ConstructHad();
	G4cout << "\n Had Physics is ConstructHad(): default! \n" << G4endl;
    }	

    // -- ConstructGeneral
    ConstructGeneral();
}

// Add Physics List ////////////////////////////////////////////////////////
void LscPhysicsList::AddPhysicsList(const G4String &name) {
    G4int verb = 1;
    SetVerboseLevel(verb);

    G4cout << "\n>>>   LscPhysicsList::AddPhysicsList: <<<" << name << ">>> \n" << G4endl;

    // EM physics
    if (name == "livermore") {
        G4cout << "Physics : EmLivermore is selected \n" << G4endl;
	delete emPhysList;
	emPhysList = new G4EmLivermorePhysics();
        fEMName = name;
    // Op physics
    } else if (name == "lscphysicsOp") {
        G4cout << "Physics : LscPhysicsOp is selected \n" << G4endl;
	delete OpPhysList;
	OpPhysList = new LscPhysicsOp();
        fOpName = name;
    // Had physics
    } else if (name == "lscphysicsHad" && !hadIsRegisted) {
        G4cout << "Physics : LscPhysicsHad is selected \n" << G4endl;
        fHadName = name;
	hadronPhys.push_back(new G4HadronElasticPhysicsHP());
	//hadronPhys.push_back(new G4HadronElasticPhysics());
	hadronPhys.push_back(new G4HadronPhysicsQGSP_BERT_HP());
	hadronPhys.push_back(new G4ThermalNeutrons(0));
	hadIsRegisted = true;
    } else {
        G4cout << "LscPhysicsList::AddPhysicsList: <" << name << ">"
               << " is not defined" << G4endl;
    }
}
