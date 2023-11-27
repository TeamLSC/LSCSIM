#ifndef __LscDetectorMessenger_hh__
#define __LscDetectorMessenger_hh__ 1

#include "G4UImessenger.hh"

class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWith3VectorAndUnit;
class LscDetectorConstruction;

class LscDetectorMessenger : public G4UImessenger {
  public:
    LscDetectorMessenger(LscDetectorConstruction *LscDetector);
    ~LscDetectorMessenger();

    void SetNewValue(G4UIcommand *command, G4String newValues);
    G4String GetCurrentValue(G4UIcommand *command);

  private:
    LscDetectorConstruction *LscDetector;

    G4UIcommand *DetGeometrySelectCmd;
    G4UIcommand *DetGeometryQuenchingCmd;

    class LscDetectorMessenger *myMessenger;
};

#endif
