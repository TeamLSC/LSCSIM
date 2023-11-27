#ifndef LscDetectorConstruction_HH
#define LscDetectorConstruction_HH 1

#include "CupSim/CupDetectorConstruction.hh"

class LscDetectorConstruction : public CupDetectorConstruction {
  public:
    typedef enum {
        kDetector_LscDetector = CupDetectorConstruction::kNumGenericDetectors,
        kNumDetectors
    } eDetector;

    typedef enum {
        //kDetector_LscYemilab = 6,
        //kDetector_LscYemilab = 0,
        kDetector_LscYemilab,
        kNumDetGeometries
    } eDetGeometry;

    LscDetectorConstruction();          // constructor
    virtual ~LscDetectorConstruction(); // destructor

    virtual G4VPhysicalVolume *Construct(); // make the volumes, return ptr to world

    virtual int GetNumDetectorTypes() { return kNumDetectors; }
    virtual G4String GetDetectorTypeName(int i);

    //virtual int GetNumDetGeometryTypes() { return kNumDetGeometries; }
    static eDetGeometry GetNumDetGeometryTypes() { return kNumDetGeometries; }
    virtual G4String GetDetGeometryTypeName(eDetGeometry i);
    eDetGeometry GetWhichDetGeometry(void) { return whichDetGeometry; }
    static inline eDetGeometry GetDetGeometryType() { return whichDetGeometry; }
    virtual void SetWhichDetGeometry(eDetGeometry w) { whichDetGeometry = w; }

    static G4int GetQuenchingModel() { return quenchingmodel; } // EJ
    virtual void SetQuenchingModel(int w) { quenchingmodel = w; }

  protected:
    void ConstructMaterials(); // make all needed materials
    void ConstructLscDetector();
    void ConstructLscYemilab();
    void ConstructLscYemilab_ID();

    G4Material *LS_LAB;
    G4Material *UGF;

    static eDetGeometry whichDetGeometry;
    static int quenchingmodel;

    class LscDetectorMessenger *LscMessenger;
};

#endif
