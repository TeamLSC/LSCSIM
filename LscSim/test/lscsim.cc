
#include "G4MTRunManager.hh"
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscPhysicsList.hh"
#include "CupSim/CupPrimaryGeneratorAction.hh"
#include "CupSim/CupPhysicsList.hh" 
#include "CupSim/CupVEventAction.hh" 
#include "CupSim/CupTrackingAction.hh" 
#include "CupSim/CupSteppingAction.hh"
#include "CupSim/CupDebugMessenger.hh"
#include "CupSim/CupParam.hh"

#ifdef G4VIS_USE
#include "CupSim/CupVisMessenger.hh"
#include "G4VisExecutive.hh"
#endif
#include <cstdlib>
#include "G4Run.hh"

#include "LscSim/LscRootNtuple.hh"
#include "LscSim/LscPhysicsList.hh"
#include "CupSim/CupRecorderBase.hh"
#include "CupSim/CupRunAction.hh"

int main (int argc,char** argv)
{
    G4cout << "This is Lscsim, version tag $Name:  $" << G4endl;

    // Run manager
/*
#ifdef G4MULTITHREADED
    G4MTRunManager *theRunManager = new G4MTRunManager;
    theRunManager->SetNumberOfThreads(1);
#else
    G4RunManager *theRunManager = new G4RunManager;
#endif
*/
    G4RunManager *theRunManager = new G4RunManager;

    // -- database
    CupParam &db ( CupParam::GetDB() );
    if ( getenv("CupDATA") != NULL )
      db.ReadFile( (G4String(getenv("CupDATA"))+"/settings.dat").c_str() );
    else
      db.ReadFile("data/settings.dat");

    // UserInitialization classes
    LscDetectorConstruction * theLscDetectorConstruction= new LscDetectorConstruction;
    theRunManager -> SetUserInitialization( theLscDetectorConstruction );
//    theRunManager -> SetUserInitialization( new CupPhysicsList );
    theRunManager -> SetUserInitialization( new LscPhysicsList );

    // Do not initialize here:  leave it for /run/initialize in prerun.mac
    // so we can define things at runtime before initialization
    //  theRunManager -> Initialize();

    // Create the LscRecorderBase object
    CupRecorderBase* myRecords = new LscRootNtuple; 

    // UserAction classes
    theRunManager->SetUserAction(new CupPrimaryGeneratorAction( theLscDetectorConstruction ));
    theRunManager->SetUserAction(new CupRunAction(myRecords));
    CupVEventAction* eventAction = new CupVEventAction(myRecords);
    theRunManager->SetUserAction(eventAction);
    theRunManager->SetUserAction(new CupTrackingAction(myRecords));
    theRunManager->SetUserAction(new CupSteppingAction(myRecords));

    // an additional "messenger" class for user diagnostics
    CupDebugMessenger theDebugMessenger( theLscDetectorConstruction );

// Visualization, only if you choose to have it!
#ifdef G4VIS_USE
    G4VisManager *theVisManager      = new G4VisExecutive();
    CupVisMessenger *theVisMessenger = new CupVisMessenger(theVisManager);
    theVisManager->Initialize();
#endif
    // user interface
    G4UImanager *theUI = G4UImanager::GetUIpointer();

    // interactive or batch according to command-line args
    if (argc == 1) {
        // G4UIterminal is a (dumb) terminal.
        // ..but it can be made smart by adding a "shell" to it
        G4UIsession *theSession = new G4UIterminal(new G4UItcsh);
        theSession->SessionStart();
        delete theSession;
    } else { // Batch mode, with optional user interaction
        if (strcmp(argv[1], "gui.mac") == 0 || strcmp(argv[1], "gui") == 0) {
            G4UIExecutive *theSession = new G4UIExecutive(argc, argv);
            theUI->ApplyCommand("/control/execute init_vis.mac");
            if (theSession->IsGUI()) {
                theUI->ApplyCommand("/control/execute gui.mac");
            }

            theSession->SessionStart();
            delete theSession;
        } else {
            G4String command = "/control/execute ";
            for (int iarg = 1; iarg < argc; iarg++)
            {
                G4String fileName = argv[iarg];
                theUI->ApplyCommand(command + fileName);
            }
        }
    }

#ifdef G4VIS_USE
    delete theVisManager;
    delete theVisMessenger;
#endif

    delete theRunManager;
    delete myRecords; 

    return 0;
}
