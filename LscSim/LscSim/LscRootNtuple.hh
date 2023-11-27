#ifndef LscRootNtuple_h
#define LscRootNtuple_h 1

// The LscRecorderBase object.  We're implementing this abstract class
// with methods that fill histograms.
#include "CLHEP/Vector/ThreeVector.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TTree.h"

#include "CupSim/CupRootNtuple.hh"

#include "MCObjs/EvtInfo.hh"
#include "MCObjs/EvtStep.hh"
#include "MCObjs/EvtTrack.hh"
#include "MCObjs/MLCS.hh"
#include "MCObjs/PMTSD.hh"
#include "MCObjs/Photon.hh"
#include "MCObjs/Primary.hh"
#include "MCObjs/Scint.hh"
#include "MCObjs/TGSD.hh"

// Forward declarations for ROOT.
// class TFile;
// class TNtuple;

// Forward declarations of G4 classes that are arguments to
// LscRecorderBase methods.
class G4Event;

// class LscRootNtuple: public LscRecorderBase
class LscRootNtuple : public CupRootNtuple {
  protected:
    using eDetGeometry  = LscDetectorConstruction::eDetGeometry;

  public:
    LscRootNtuple();
    ~LscRootNtuple() {}

    void  RecordStep(const G4Step*);
    virtual void SetTGSD(const G4Event *a_event);

    void CreateTree();

    enum {
        max_primary_particles   = 16,
        max_hits_for_ROOT       = 200000,
        max_gammas              = 10000,
        max_opticalphotons      = 10000,
        max_protons             = 1000,
        max_nTrk                = 1000000,
        max_secondary_particles = 1000
    };
    enum { nLayer_vessel = 4, inout_muon = 2 };
    enum {
        max_muonSecondary           = 10000,
        max_neutronCapture          = 100,
        max_neutronCaptureSecondary = 100,
        max_correlBKG               = 1000000
    };
};

#endif
