#include <sstream>
#include <string>

#include "CupSim/CupParam.hh"
#include "CupSim/CupPrimaryGeneratorAction.hh"
#include "CupSim/CupScintHit.hh"
#include "CupSim/CupVertexGen.hh"
#include "LscSim/LscDetectorConstruction.hh"
#include "LscSim/LscRootNtuple.hh"
#include "LscSim/LscScintSD.hh"
#include "LscSim/LscScintillation.hh"

// Include files for ROOT.
#include "Rtypes.h"

// Include files for the G4 classes
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4SDManager.hh"
#include "G4Trajectory.hh"
#include "G4VHitsCollection.hh"
#include "G4VProcess.hh"
#include "globals.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UItcsh.hh"
#include "G4UIterminal.hh"

TROOT theROOT("LscSim/Lscsim", "Lsc Geant4 simulation output tree");

LscRootNtuple::LscRootNtuple() : CupRootNtuple() {}

void LscRootNtuple::CreateTree() {
    CupRootNtuple::CreateTree();

    // TG Sensitive Detector
    LscScintSD *tgsd;
    tgsd = (LscScintSD *)(sdman->FindSensitiveDetector("/lsc/TGSD"));
    if (tgsd) {
        fROOTOutputTree->Branch("TGSD", &(Ctgsd), 256000, 2);
    }
}

void LscRootNtuple::RecordStep(const G4Step* a_step)
{
  Int_t trid           = a_step->GetTrack()->GetTrackID();
  Int_t prntid         = a_step->GetTrack()->GetParentID();
  G4String volume      = a_step->GetTrack()->GetVolume()->GetName();
  G4int istep          = a_step->GetTrack()->GetCurrentStepNumber();
  G4String pname       = a_step->GetTrack()->GetDefinition()->GetParticleName();
  G4int atomicmass     = a_step->GetTrack()->GetDefinition()->GetAtomicMass();
  G4int atomicnumber   = a_step->GetTrack()->GetDefinition()->GetAtomicNumber();
  Double_t dep         = a_step->GetTotalEnergyDeposit();

  G4StepPoint* preStep = a_step->GetPreStepPoint();
  G4StepPoint* postStep = a_step->GetPostStepPoint();

  G4String postVol;
  if(postStep->GetPhysicalVolume()==0) postVol = "OutOfWorld";
  else  postVol = postStep->GetPhysicalVolume()->GetName();

  G4String procname    = postStep->GetProcessDefinedStep()->GetProcessName();
  Double_t ke          = postStep->GetKineticEnergy();
  Double_t globaltime  = postStep->GetGlobalTime();
  Double_t localtime   = postStep->GetLocalTime();
  G4ThreeVector  pos   = postStep->GetPosition();
  Float_t xx = (float) pos.x();
  Float_t yy = (float) pos.y();
  Float_t zz = (float) pos.z();

  // TTrack ttr;
  TStep  tst;
  Int_t cuppdgcode;

  char partname[100];
  char prcsname[100];
  char volname[100];

  sprintf(partname,"%s",(char *)pname.data());
  sprintf(volname,"%s",(char *)postVol.data());
  sprintf(prcsname,"%s",(char *)procname.data());

  G4ParticleDefinition* pdef= a_step->GetTrack()->GetDefinition();
  if (G4IonTable::IsIon(pdef)) {
    cuppdgcode= (1000*atomicnumber + atomicmass )*10;
  }
  else {
    cuppdgcode= pdef->GetPDGEncoding();
  }

  if (StatusStep) {
    tst.SetParticleName(partname);
    tst.SetPDGcode(cuppdgcode);
    tst.SetTrackID(trid);
    tst.SetParentID(prntid);
    tst.SetKineticEnergy(ke);
    tst.SetEnergyDeposit(dep);
    tst.SetX(xx);
    tst.SetY(yy);
    tst.SetZ(zz);
    tst.SetGlobalTime(globaltime);
    tst.SetLocalTime(localtime);
    tst.SetProcessName(prcsname);
    tst.SetVolumeName(volname);
    tst.SetStepNo(istep);

    new((*tclst)[nStep]) TStep (tst);
    nStep++;
  }
  // Access physical volume where primary particles generated
  if(istep==1 && prntid==0 && trid==1) {
    G4TouchableHandle theTouchable       = preStep->GetTouchableHandle();
    G4int motherCopyNo                   = theTouchable->GetCopyNumber(1);
    G4VPhysicalVolume* theMotherPhysical = theTouchable->GetVolume(1);

    sprintf(volumeName,"%s",(char *)volume.data());
    copyNo = motherCopyNo;
    G4String motherVolName;
  }

}

void LscRootNtuple::SetTGSD(const G4Event *a_event) {
    //////////////////////////
    // TG Sensitive Detector
    /////////////////////////

    G4String colName;
    G4SDManager *SDman           = G4SDManager::GetSDMpointer();
    TGSDHCID                     = SDman->GetCollectionID(colName = "TGSDColl");
    G4HCofThisEvent *HCE         = a_event->GetHCofThisEvent();
    CupScintHitsCollection *ECHC = 0;
    if (TGSDHCID < 0) return;
    if (HCE) ECHC = (CupScintHitsCollection *)(HCE->GetHC(TGSDHCID));

    int iHit                 = 0;
    double totalE            = 0.;
    double totalEquenched    = 0.;
    int nTotCell             = 0;
    double tgTotEdep         = 0.;
    double tgTotEdepQuenched = 0.;

    int nCell = 0;
    TCell tcell;

    nTotCell = ECHC->entries();
    Ctgsd->SetNTotCell(nTotCell);
    double eDep;
    double eDepQuenched;

    eDetGeometry DetGeometryType;
    DetGeometryType = LscDetectorConstruction::GetDetGeometryType();
    G4cout << "Ntuple DetGeometryType= " << DetGeometryType << G4endl;
    switch (DetGeometryType) {
        case eDetGeometry::kDetector_LscYemilab: {
            G4cout << "###  LSC detector at Yemilab  " << G4endl;
            for (int i = 0; i < nTotCell; i++) {
                tgcellEdep[i]         = 0.;
                tgcellEdepQuenched[i] = 0.;
                CupScintHit *aHit      = (*ECHC)[i];
                eDep                   = aHit->GetEdep();
                eDepQuenched           = aHit->GetEdepQuenched();
                tcell.SetCellID(i);
                tcell.SetEdep(eDep);
                tcell.SetEdepQuenched(eDepQuenched);
                if (eDep > 0.) {
                    iHit++;
                    totalE += eDep;
                    totalEquenched += eDepQuenched;
                }

                new ((*tclcell)[nCell]) TCell(tcell);
                nCell++;
            }
        }   break;
        default:
            G4cout << "### Detector type is not valid!!! Signal is not filled to Cell!!!\n";
            break;
    }
    (void)tgTotEdep;
    (void)tgTotEdepQuenched;
    Ctgsd->SetTotEdep(totalE);
    Ctgsd->SetTotEdepQuenched(totalEquenched);
}
